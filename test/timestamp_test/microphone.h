#pragma once

#include <alsa/asoundlib.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <cmath>

constexpr int kBitsPerSample = 16;
constexpr size_t kBytesPerSample = kBitsPerSample / 8;
constexpr size_t kChannels = 32;
constexpr size_t kSampleRate = 48000;
constexpr size_t kPeriodSize = 256;
constexpr size_t kBufferSize = 1024;

class Microphone 
{
public:
    Microphone(const std::string& device_name);
    ~Microphone();

    void init();
    void prime_capture();
    void start_capture();
    void stop_capture();
    double calculate_value_at_percentile_ms(double percentile);
    double calculate_average_ms();
    double expected_interval_ms();
    void save_times_stamps_to_file(std::string filename);

    // Timing measurement functions
    static uint64_t getCurrentTSCTicks();
    static uint64_t ticksToNanoseconds(uint64_t ticks);
    
private:
    std::string device_name_;
    snd_pcm_t* pcm_handle_ = nullptr;
    snd_pcm_format_t format_;
    std::vector<uint8_t> interleaved_buffer_;
    snd_pcm_uframes_t period_frames_ = 0;
    snd_pcm_uframes_t buffer_size_ = 0;

    uint64_t previous_tsc_ticks_ = 0;
    std::vector<uint64_t> times_stamps_;
};
