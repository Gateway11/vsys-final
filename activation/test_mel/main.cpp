//
//  main.cpp
//  test_mel
//
//  Created by 薯条 on 2018/3/24.
//  Copyright © 2018年 薯条. All rights reserved.
//

#define LOG_TAG "mel"

#define FRAME_SIZE 160
#define FRAMES_PER_BUFFER 400
#define NOT_OVERLAP 200
#define PI 3.1415926
#define LEN_SPECTRUM 512
#define NUM_FILTER 40
#define LEN_MELREC 13

#include <thread>
#include <fstream>
#include <math.h>

#include "fftw3.h"
#include "debug.h"

std::ifstream input_stream("/Users/daixiang/Desktop/vsys/data/sounds/baomao_M_0020.16000.1.16bit.pcm", std::ios::in | std::ios::binary);
std::ofstream output_stream("/Users/daixiang/Documents/MATLAB/exp/data.txt", std::ios::out);

char* buff;
float* buff_emp;
float mul_mel_rec[300][LEN_MELREC];

template<typename T>
void print_array(T data, uint32_t length, std::ofstream& output_stream){
    for (uint32_t i = 0; i < length; i++) {
        output_stream << data[i] << " ";
    }
}

//y(n)=x(n)-ax(n-1)
void pre_emphasizing(char* input, uint32_t input_size, float factor, float*& output, uint32_t& output_size){
    
    short* data = (short *)input;
    uint32_t samples = input_size / 2;
    
    buff_emp[0] = (float)data[0];
    
    for (uint32_t i = 1; i < samples; i++) {
        buff_emp[i] = data[i] - factor * data[i - 1];
    }
    output = buff_emp;
    output_size = samples;
}

void set_hamming_window(float* frame_win){
    for (uint32_t i = 0; i < FRAMES_PER_BUFFER; i++) {
        frame_win[i] = 0.54 - 0.46 * cos(2 * PI * i / (FRAMES_PER_BUFFER -1));
    }
}

void set_hanning_window(float* frame_win){
    for (uint32_t i = 0; i < FRAMES_PER_BUFFER; i++) {
        frame_win[i] = 0.5 - 0.5 * cos(2 * PI * i / (FRAMES_PER_BUFFER -1));
    }
}

void set_blackman_window(float* frame_win){
    for (uint32_t i = 0; i < FRAMES_PER_BUFFER; i++) {
        frame_win[i] = 0.42 - 0.5 * cos(2 * PI * i / (FRAMES_PER_BUFFER -1)) + 0.08 * cos(4 * PI * i / (FRAMES_PER_BUFFER -1));
    }
}

void fft_power(float* input, float* output){
    fftwf_complex* fft_buff = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * LEN_SPECTRUM);
    fftwf_plan fftw = fftwf_plan_dft_r2c_1d(LEN_SPECTRUM, input, fft_buff, FFTW_ESTIMATE);
    fftwf_execute(fftw);
    for (uint32_t i = 0; i < LEN_SPECTRUM; i++) {
        output[i] = fft_buff[i][0] * fft_buff[i][0] + fft_buff[i][1] * fft_buff[i][1];
    }
    fftwf_destroy_plan(fftw);
    fftwf_free(fft_buff);
}

void compute_mel(float* input, int sample_rate, float* output){
    uint32_t max = sample_rate / 2;
    float max_mel_freq = 1125 * log(1 + max / 700);
    uint32_t delta = (uint32_t)(max_mel_freq / (NUM_FILTER + 1));
    
    float** mel_filter = new float*[NUM_FILTER];
    for (uint32_t i = 0; i < NUM_FILTER; i++) {
        mel_filter[i] = new float[3];
    }
    float* m = new float[NUM_FILTER + 2];
    float* h = new float[NUM_FILTER + 2];
    float* f = new float[NUM_FILTER + 2];
    //计算频谱到梅尔谱的映射关系
    for (uint32_t i = 0; i < NUM_FILTER; i++) {
        m[i] = i * delta;
        h[i] = 700 * (exp(m[i] / 1125) -1);
        f[i] = floor((256 + 1) * h[i] / sample_rate);
    }
    //计算梅尔滤波参数
    for (uint32_t i = 0; i < NUM_FILTER; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            mel_filter[i][j] = f[i + j];
        }
    }
    //梅尔滤波
    for (uint32_t i = 0; i < NUM_FILTER; i++) {
        for (uint32_t j = 0; j < 256; j++) {
            if(j >= mel_filter[i][0] && j <= mel_filter[i][1]){
                output[i] += ((j - mel_filter[i][0]) / (mel_filter[i][1] - mel_filter[i][0])) * input[j];
            }else if(j > mel_filter[i][1] && j <= mel_filter[i][2]){
                output[i] += ((mel_filter[i][2] - j) / (mel_filter[i][2] - mel_filter[i][1])) * input[j];
            }
        }
    }
    for (uint32_t i = 0; i < 3; i++) {
        delete[] mel_filter[i];
    }
    delete[] mel_filter;
    delete[] m;
    delete[] h;
    delete[] f;
}

void dct(float* input, float* output){
    for (uint32_t i = 0; i < LEN_MELREC; i++) {
        for (uint32_t j = 0; j < NUM_FILTER; j++) {
            if(input[j] <= -0.0001 || input[j] >= 0.0001){
                output[i] = log(input[j]) * cos(PI * i / (2 * NUM_FILTER) * (2 * j + 1));
            }
        }
    }
}

int main(int argc, const char * argv[]) {
    input_stream.seekg(0, std::ios::end);
    uint32_t file_size = input_stream.tellg();
    input_stream.seekg(0, std::ios::beg);

    output_stream << file_size / 2 << std::endl;
    
    buff = new char[file_size];
    buff_emp = new float[file_size * 2];

    float* data_emp;
    uint32_t data_emp_size;

    input_stream.read(buff, file_size);
//    print_array(((short *)buff), file_size / 2, output_stream);

    //step 1: 预加重
    pre_emphasizing(buff, file_size, 0.95, data_emp, data_emp_size);
//    print_array(data_emp, file_size / 2, output_stream);
    
    uint32_t num_frames = ceil((file_size - FRAMES_PER_BUFFER) / NOT_OVERLAP) + 1;
    
    float* frame_win = new float[FRAMES_PER_BUFFER];
    float* after_win = new float[LEN_SPECTRUM];
    float* energy_spectrum = new float[LEN_SPECTRUM];
    float* mel = new float[NUM_FILTER];
    float* mel_rec = new float[LEN_MELREC];
    float* sum_mel_rec = new float[LEN_MELREC];
    memset(sum_mel_rec, 0, LEN_MELREC * sizeof(float));
    memset(mul_mel_rec, 0, num_frames * LEN_MELREC * sizeof(float));
    
    uint32_t offset = 0;
    
    //step 2: 设置窗参数
    set_hamming_window(frame_win);
    
    for (uint32_t i = 0; i < num_frames; i++) {
        //加窗操作
        for (uint32_t j = 0; j < LEN_SPECTRUM; j++) {
            after_win[j] = j < FRAMES_PER_BUFFER ? data_emp[j + offset] * frame_win[j] : 0;
        }
        if(i == 100){
//            print_array(after_win, LEN_SPECTRUM, output_stream);
        }
        //step 3: 计算能量谱
        fft_power(after_win, energy_spectrum);
        if(i == 100){
//            print_array(energy_spectrum, LEN_SPECTRUM, output_stream);
        }
        //step 4: 计算梅尔谱
        memset(mel, 0, NUM_FILTER * sizeof(float));
        compute_mel(energy_spectrum, 16000, mel);
        if(i == 100){
//            print_array(mel, NUM_FILTER, output_stream);
        }
        //step 5: 计算离散余弦变换
        memset(mel_rec, 0, LEN_MELREC * sizeof(float));
        dct(mel, mel_rec);
        if(i == 100){
            print_array(mel_rec, LEN_MELREC, output_stream);
        }
        //step 6: 累计总值
        for (uint32_t j = 0; j < LEN_MELREC; j++) {
            mul_mel_rec[i][j] = mel_rec[j];
            sum_mel_rec[j] += mel_rec[j] * mel_rec[j];
        }
        offset += NOT_OVERLAP;
    }
    //step 6: 归一化处理
    for (uint32_t i = 0; i < LEN_MELREC; i++) {
        sum_mel_rec[i] = sqrt(sum_mel_rec[i] / num_frames);
    }
    for (uint32_t i = 0; i < num_frames; i++) {
        for (uint32_t j = 0; j < LEN_MELREC; j++) {
//            output_stream << mul_mel_rec[i][j] << " ";
            mul_mel_rec[i][j] /= sum_mel_rec[j];
//            output_stream << mul_mel_rec[i][j] << " ";
        }
    }
    VSYS_DEBUGD("%d         %d", file_size, num_frames * LEN_MELREC);
    return 0;
}
