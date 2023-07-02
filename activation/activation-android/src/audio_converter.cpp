//
//  audio_converter.cpp
//  vsys
//
//  Created by 薯条 on 2018/1/21.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include "audio_converter.h"
#include "vsys_types.h"

namespace vsys {
    
int32_t AudioConverter::convert(void *input, uint32_t input_size, float **&output, uint32_t &output_size){
    
    uint32_t samples = 0;
    switch (format) {
        case AUDIO_FORMAT_ENCODING_PCM_16BIT:{
            short *data = (short *)input;
            samples = input_size / (num_channels * sizeof(short));
            check_buffer(samples);
            for (uint32_t i = 0; i < num_mics; i++) {
                for (uint32_t j = 0; j < samples; j++) {
                    buff[i][j] = data[mic_ids[i] + j * num_channels];
                }
            }
            break;
        }
        case AUDIO_FORMAT_ENCODING_PCM_24BIT:{
            int24_t *data = (int24_t *)input;
            samples = input_size / (num_channels * sizeof(int24_t));
            check_buffer(samples);
            for(uint32_t i = 0; i < num_mics; i++){
                for (uint32_t j = 0; j < samples; j++) {
                    buff[i][j] = data[mic_ids[i] + j * num_channels].toint() / 4.0f;
                }
            }
            break;
        }
        case AUDIO_FORMAT_ENCODING_PCM_32BIT:{
            float *data = (float *)input;
            samples = input_size / (num_channels * sizeof(float));
            check_buffer(samples);
            for(uint32_t i = 0; i < num_mics; i++){
                for (uint32_t j = 0; j < samples; j++) {
                    buff[i][j] = data[mic_ids[i] + j * num_channels] / 1024.0f;
                }
            }
            break;
        }
        case AUDIO_FORMAT_ENCODING_PCM_FLOAT:{
            float *data = (float *)input;
            samples = input_size / (num_channels * sizeof(float));
            check_buffer(samples);
            for(uint32_t i = 0; i < num_mics; i++){
                for (uint32_t j = 0; j < samples; j++) {
                    buff[i][j] = data[mic_ids[i] + j * num_channels] * 32768.0f;
                }
            }
            break;
        }
    }
    output = buff;
    output_size = samples;
    return 0;
}
    
void AudioConverter::check_buffer(uint32_t length){
    if(length > buff_len){
        buff_len = length;
        buff = malloc_buffer2(num_mics, buff_len);
    }
}
    
}
