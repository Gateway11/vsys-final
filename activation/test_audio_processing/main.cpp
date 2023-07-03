//
//  main.cpp
//  aec
//
//  Created by 薯条 on 2018/10/30.
//  Copyright © 2018 薯条. All rights reserved.
//

#include <fstream>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <semaphore.h>
#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>

#include "debug.h"

uint32_t num_mics = 2;
uint32_t num_speakers = 2;
uint32_t num_channels = num_mics + num_speakers;

uint32_t sample_rate = 16000;
uint32_t frame_size = sample_rate / 100;

void test_audio_processing(){
    std::ifstream input_stream("../data/lujnan_G_0020.16000.4.16bit.pcm", std::ios::in | std::ios::binary);
    std::ofstream output_stream("../data/tmp/lujnan_G_0020.16000.2.16bit.pcm", std::ios::out | std::ios::binary);
    
    SpeexPreprocessState** dens(new SpeexPreprocessState*[num_mics]);
    SpeexEchoState** echo_states(new SpeexEchoState*[num_speakers]);
    for(uint32_t i = 0; i < num_mics; i++){
        dens[i] = speex_preprocess_state_init(frame_size, sample_rate);
        
        echo_states[i] = speex_echo_state_init_mc(frame_size, 1024, 1, num_speakers);
        speex_echo_ctl(echo_states[i], SPEEX_ECHO_SET_SAMPLING_RATE, &sample_rate);
        speex_preprocess_ctl(dens[i], SPEEX_PREPROCESS_SET_ECHO_STATE, echo_states[i]);
    }
    
    sem_t *proc_sem, *sync_sem;
#if defined(__APPLE__) && defined(__MACH__)
#define __STR(x) #x
    sem_unlink(__STR(proc_sem));
    sem_unlink(__STR(sync_sem));
    proc_sem = sem_open(__STR(proc_sem), O_CREAT|O_EXCL, 0644, 0);
    sync_sem = sem_open(__STR(sync_sem), O_CREAT|O_EXCL, 0644, 0);
#elif
    sem_t __proc_sem, __sync_sem;
    sem_init(&__proc_sem, 0, 0);
    sem_init(&__sync_sem, 0, 0);
    proc_sem = &__proc_sem;
    sync_sem = &__sync_sem;
#endif
    
    std::vector<std::thread> threads;
    std::atomic_int tasks(0);
    std::atomic_bool thread_exit(false);
    
    short *data(new short[frame_size * num_channels]);
    spx_int16_t* input(new spx_int16_t[num_channels * frame_size]);
    spx_int16_t* output(new spx_int16_t[num_mics * frame_size]);
    
    for (uint32_t i = 0; i < num_mics; i++) {
        threads.emplace_back([&]{
            while (true) {
                sem_wait(proc_sem);
                if(thread_exit.load()) break;
                
                uint32_t task = tasks.fetch_sub(1, std::memory_order_seq_cst) - 1;
                
                speex_echo_cancellation(echo_states[task], input + task * frame_size,
                                        input + frame_size * num_mics, output + task * frame_size);
                speex_preprocess_run(dens[task], output + task * frame_size);
                sem_post(sync_sem);
            }
        });
    }
    
    auto mfor_each = [](uint32_t first, uint32_t last, std::function<void()> function){
        for (uint32_t i = first; i < last; i++) {function();}
    };
    while (input_stream.good()) {
        input_stream.read((char *)data, frame_size * num_channels * sizeof(short));
        for (uint32_t i = 0; i < num_mics; i++) {
            for (uint32_t j = 0; j < frame_size; j++) {
                input[i * frame_size + j] = data[i + j * num_channels];
            }
        }
        for (uint32_t i = num_mics, k = 0; i < num_channels; i++, k++) {
            for (uint32_t j = 0; j < frame_size; j++) {
                input[k + j * num_mics + num_mics * frame_size] = data[i + j * num_channels];
            }
        }
        
        tasks.store(num_mics, std::memory_order_release);
        mfor_each(0, num_mics, [&proc_sem]{sem_post(proc_sem);});
        mfor_each(0, num_mics, [&sync_sem]{sem_wait(sync_sem);});
        
        for (uint32_t i = 0; i < num_mics; i++) {
            for (uint32_t j = 0; j < frame_size; j++) {
                data[j * num_mics + i] = output[i * frame_size + j];
            }
        }
        output_stream.write((char *)data, num_mics * frame_size * sizeof(short));
    }
    thread_exit = true;
    mfor_each(0, num_mics, [&proc_sem]{sem_post(proc_sem);});
    std::for_each(threads.begin(), threads.end(), [](std::thread& thread){thread.join();});
#if defined(__APPLE__) && defined(__MACH__)
    sem_close(proc_sem);
    sem_close(sync_sem);
    sem_unlink(__STR(proc_sem));
    sem_unlink(__STR(sync_sem));
#else
    sem_destroy(proc_sem);
    sem_destroy(sync_sem);
#endif
    for (uint32_t i = 0; i < num_mics; i++) {
        speex_echo_state_destroy(echo_states[i]);
        speex_preprocess_state_destroy(dens[i]);
    }
}

int main(int argc, const char * argv[]) {
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    test_audio_processing();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp);
    VSYS_DEBUGI("已运行%lld毫秒\n", elapsed.count());
    return 0;
}
