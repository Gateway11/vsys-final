//
//  main.cpp
//  test_audio_processing
//
//  Created by 薯条 on 2018/3/24.
//  Copyright © 2018年 薯条. All rights reserved.
//

#define LOG_TAG "audio_procissing"

#define MIC_CHANNEL 2
#define SPEAKER_CHANNEL 2

#define FRAME_SIZE 160
#define FRAME_SIZE_SPEEX 256

#define SAMPLE_RATE 48000
#define SPEEX_AEC_TAIL 1024

#include <fstream>
#include <thread>
#include <list>
#include <vector>
#include <semaphore.h>
#include "debug.h"
#include "speex_preprocess.h"
#include "speex_echo.h"

std::ifstream input_stream("/Users/daixiang/Desktop/vsys/data/sounds/lujnan_M_0020.48000.4.16bit.pcm", std::ios::in | std::ios::binary);
std::ofstream output_stream("/Users/daixiang/Desktop/vsys/data/sounds/lujnan_M_0020.48000.2.16bit.pcm", std::ios::out | std::ios::binary);

char buff[8192];
float input[8192];
float output[FRAME_SIZE_SPEEX];

void test_audio_processing(){
    SpeexPreprocessState** preprocess_states = new SpeexPreprocessState*[MIC_CHANNEL];
    SpeexEchoState** echo_states = new SpeexEchoState*[SPEAKER_CHANNEL];
    
    int sample_rate = SAMPLE_RATE;
    for(uint32_t i = 0; i < MIC_CHANNEL; i++){
        preprocess_states[i] = speex_preprocess_state_init(FRAME_SIZE_SPEEX, SAMPLE_RATE);
        echo_states[i] = speex_echo_state_init_mc(FRAME_SIZE_SPEEX, SPEEX_AEC_TAIL, 1, SPEAKER_CHANNEL);
        
        speex_echo_ctl(echo_states[i], SPEEX_ECHO_SET_SAMPLING_RATE, &sample_rate);
        speex_preprocess_ctl(preprocess_states[i], SPEEX_PREPROCESS_SET_ECHO_STATE, echo_states[i]);
    }
    
    std::list<uint32_t> tasks;
    
    sem_t resume, pause;
    sem_t *resume_ref, *pause_ref;
#if defined(__APPLE__) || defined(__MACH__)
    sem_unlink("sem_speex_resume");
    sem_unlink("sem_speex_pause");
    resume_ref = sem_open("sem_speex_resume", O_CREAT | O_EXCL, 0644, 0);
    pause_ref = sem_open("sem_speex_pause", O_CREAT | O_EXCL, 0644, 0);
#else
    sem_init(&resume, 0, 0);
    sem_init(&pause, 0, 0);
    resume_ref = &resume;
    pause_ref = &pause;
#endif
    
    bool thread_exit = false;
    std::mutex task_mutex;
    auto thread_loop  = [&]{
        std::unique_lock<decltype(task_mutex)> locker(task_mutex, std::defer_lock);
        while (true) {
            sem_wait(resume_ref);
            if(thread_exit) break;
            
            locker.lock();
            uint32_t task_id = *tasks.begin();
            tasks.pop_front();
            VSYS_DEBUGI("-------------------------------");
            locker.unlock();
            
            speex_echo_cancellation(echo_states[task_id],
                                    input + task_id * FRAME_SIZE_SPEEX,         //mic
                                    input + MIC_CHANNEL * FRAME_SIZE_SPEEX,     //speaker
                                    output + task_id * FRAME_SIZE_SPEEX);
            speex_preprocess_run(preprocess_states[task_id], output + task_id * FRAME_SIZE_SPEEX);
            
            sem_post(pause_ref);
        }
    };
    
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < 2; i++) {
        threads.emplace_back(thread_loop);
    }
    
    uint32_t num_channels = MIC_CHANNEL + SPEAKER_CHANNEL;
    uint32_t total = num_channels * FRAME_SIZE_SPEEX;
    
    while (input_stream.good()) {
        input_stream.read(buff, total * sizeof(short));
        for (uint32_t i = 0; i < num_channels; i++) {
            for (uint32_t j = 0; j < total; j++) {
                input[i * FRAME_SIZE_SPEEX + j] = ((short *)buff)[j * num_channels + i];
            }
        }
        for (uint32_t i = FRAME_SIZE_SPEEX; i < num_channels; i++) {
            for (uint32_t j = 0; j < MIC_CHANNEL; j++) {
                input[i * SPEAKER_CHANNEL +j] = input[j * FRAME_SIZE_SPEEX + i];
            }
        }
        for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
            tasks.push_back(i);
            sem_post(resume_ref);
        }
        for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
            sem_wait(pause_ref);
        }
        output_stream.write((char *)output, FRAME_SIZE_SPEEX * sizeof(float));
    }
    
    thread_exit = true;
    for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
        sem_post(resume_ref);
        threads[i].join();
    }
    for (uint32_t i = 0; i < MIC_CHANNEL; i++) {
        speex_echo_state_destroy(echo_states[i]);
        speex_preprocess_state_destroy(preprocess_states[i]);
    }
    
#if defined(__APPLE__) || defined(__MACH__)
    sem_close(resume_ref);
    sem_close(pause_ref);
    sem_unlink("sem_speex_resume");
    sem_unlink("sem_speex_pause");
#else
    sem_destroy(resume_ref);
    sem_destroy(pause_ref);
#endif
}

int main(int argc, const char * argv[]) {
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    test_audio_processing();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp);
    VSYS_DEBUGI("已运行%lld毫秒\n", elapsed.count());
    return 0;
}
