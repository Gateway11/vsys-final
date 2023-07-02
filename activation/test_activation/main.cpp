//
//  main.cpp
//  test_activation
//
//  Created by 薯条 on 2018/3/24.
//  Copyright © 2018年 薯条. All rights reserved.
//

#define LOG_TAG "examples"

#define FRAME_SIZE 160

#include <fstream>
#include <thread>

#include "debug.h"
#include "vsys_activation.h"

std::ifstream input_stream("/Users/daixiang/Desktop/vsys/data/sounds/baomao_M_0020.16000.8.float.pcm", std::ios::in | std::ios::binary);

char buff[8192];

float mic_pos[] = {
    0.0425000000, 0.0000000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000,
    0.0000000000, 0.0425000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000,
    0.0425000000, 0.0000000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000,
    0.0000000000, 0.0425000000, 0.0000000000,
    0.0300520382, 0.0300520382, 0.0000000000
};

void test_activation(){
    mic_param_t channel_param[8];
    for (uint32_t i = 0; i < 8; i++) {
        channel_param[i].position.x = mic_pos[i * 3 + 0];
        channel_param[i].position.y = mic_pos[i * 3 + 1];
        channel_param[i].position.z = mic_pos[i * 3 + 2];
        channel_param[i].id = i;
        //        channel_param[0].delay = 0.0000000000;
    }
    
    activation_param_t param;
    param.mic_params = channel_param;
    param.sample_rate = AUDIO_SAMPLT_RATE_16K;
    param.sample_size_bits = AUDIO_FORMAT_ENCODING_PCM_FLOAT;
    param.num_mics = 8;
    param.num_channels = 8;
    param.mask |= MIC_PARAM_POSTION_MASK;
    //    param.mask |= MIC_PARAM_DELAY_MASK;
    
    bool loop = true;
    srand(time(nullptr));
    VsysActivationInst handle =  VsysActivation_Create(&param, "/Users/daixiang/workspace/thirdlib", true);
    
    vt_word_t vt_word;
    memset(&vt_word, 0, sizeof(vt_word_t));
    vt_word.type = VSYS_WORD_AWAKE;
    strcpy(vt_word.phone, "r|l|r_B|l_B|# w o4|o4_E|## q|q_B|# i2|i2_E|##");
    strcpy(vt_word.word_utf8, "若琪");
    strcpy(vt_word.nnet_path, "/Users/daixiang/workspace/thirdlib/workdir_cn/final.ruoqi.mod");
    vt_word.mask |= VT_WORD_USE_OUTSIDE_PHONE_MASK
    | VT_WORD_LOCAL_CLASSIFY_CHECK_MASK;
    VsysActivation_AddVtWord(handle, &vt_word);
    
    std::thread thread([&]{
        while (loop) {
            VsysActivation_Control(handle, (rand() % 2) ? ACTIVATION_SET_STATE_AWAKE : ACTIVATION_SET_STATE_SLEEP);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000 + 1));
        }
    });
    
    for (uint32_t i = 0; i < FRAME_SIZE * 100; i++) {
        while(input_stream.good()){
            input_stream.read(buff, FRAME_SIZE * 8 * sizeof(float));
            VsysActivation_Process(handle, (uint8_t *)buff, FRAME_SIZE * 8 * sizeof(float));
        }
        printf("#############################################{%d/%d}#############################################\n", i, FRAME_SIZE * 100);
        input_stream.clear();
        input_stream.seekg(0, std::ios::beg);
    }
    loop = false;
    thread.join();
    VsysActivation_Free(handle);
}

int main(int argc, const char * argv[]) {
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    test_activation();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp);
    VSYS_DEBUGI("已运行%lld毫秒\n", elapsed.count());
    return 0;
}
