//
//  main.cpp
//  vsys
//
//  Created by 薯条 on 2017/12/24.
//  Copyright © 2017年 薯条. All rights reserved.
//
#define LOG_TAG "examples"

#define FRAME_SIZE 160

#include <string.h>
#include <fstream>
#include <thread>

#include "debug.h"
#include "vsys_activation.h"

#define FRAME_SIZE 160
#define CHANNEL_NUM 1
#define AUDIO_TYPE float

std::ifstream pcm_in("./data/sounds/pcm_16k_32f_1.pcm", std::ios::in | std::ios::binary);
char buff[8192];

void test_activation(){
    activation_param_t param;
    memset(&param, 0, sizeof(activation_param_t));
    param.sample_rate = AUDIO_SAMPLT_RATE_16K;
    param.sample_size_bits = AUDIO_FORMAT_ENCODING_PCM_FLOAT;
    param.num_mics = 1;
    param.num_channels = CHANNEL_NUM;
    VsysActivationInst handle =  VsysActivation_Create(&param, "./thirdparty/model/dnn", true);

//    vt_word_t vt_word;
//    memset(&vt_word, 0, sizeof(vt_word_t));
//    vt_word.type = VSYS_WORD_AWAKE;
//    strcpy(vt_word.phone, "r|l|r_B|l_B|# w o4|o4_E|## q|q_B|# i2|i2_E|##");
//    strcpy(vt_word.word_utf8, "若琪");
//    strcpy(vt_word.nnet_path, "./thirdparty/model/dnn/workdir_cn/final.ruoqi.mod");
//    vt_word.mask |= VT_WORD_USE_OUTSIDE_PHONE_MASK
//                | VT_WORD_LOCAL_CLASSIFY_CHECK_MASK;
//    VsysActivation_AddVtWord(handle, &vt_word);

    bool thread_exit = false;
    srand(time(nullptr));

    std::thread thread([&]{
        while (thread_exit) {
            VsysActivation_Control(handle, (rand() % 2) ? ACTIVATION_SET_STATE_AWAKE : ACTIVATION_SET_STATE_SLEEP);
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000 + 1));
        }
    });

    while(pcm_in.good()){
        pcm_in.read(buff, FRAME_SIZE * CHANNEL_NUM * sizeof(AUDIO_TYPE));
        VsysActivation_Process(handle, (uint8_t *)buff, FRAME_SIZE * CHANNEL_NUM * sizeof(AUDIO_TYPE));
    }
    thread_exit = true;
    thread.join();
    VsysActivation_Free(handle);
}

int main(int argc, const char * argv[]){
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    test_activation();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp);
    VSYS_DEBUGI("已运行%lld毫秒\n", elapsed.count());
    return 0;
}
