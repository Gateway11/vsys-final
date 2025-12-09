#include "microphone.h"

uint64_t Microphone::getCurrentTSCTicks() {
    uint64_t tscTicks {0u};
    __asm__ __volatile__(
        "isb\n\t"
        "mrs %0, cntvct_el0\n\t"
        "isb\n\t"
        : "=r"(tscTicks)
        :
        : "memory");
    return tscTicks;
}

uint64_t Microphone::ticksToNanoseconds(uint64_t ticks) {
    uint64_t hz;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(hz));
    __uint128_t ns = ( (__uint128_t)ticks * 1000000000ull );
    return (uint64_t)(ns / hz);
}

Microphone::Microphone(const std::string& device_name)
    : device_name_(device_name)
{
    init();
}

Microphone::~Microphone() {
    stop_capture();
    if (pcm_handle_ != nullptr) {
        snd_pcm_close(pcm_handle_);
        pcm_handle_ = nullptr;
    }
}

void Microphone::init()
{
    if (snd_pcm_open(&pcm_handle_, device_name_.c_str(), SND_PCM_STREAM_CAPTURE, 0) < 0) {
        std::cout << "Failed to open PCM device: " << device_name_ << std::endl;
        pcm_handle_ = nullptr;
        return;
    }
    
    snd_pcm_hw_params_t* hw_params = nullptr;
    snd_pcm_hw_params_malloc(&hw_params);
    if (snd_pcm_hw_params_any(pcm_handle_, hw_params) < 0) {
        std::cout << "Failed to initialize PCM hardware parameters" << std::endl;
        if (pcm_handle_ != nullptr) {
            snd_pcm_close(pcm_handle_);
            pcm_handle_ = nullptr;
        }
        snd_pcm_hw_params_free(hw_params);
        return;
    }
    snd_pcm_hw_params_set_access(pcm_handle_, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle_, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate(pcm_handle_, hw_params, kSampleRate, 0);
    snd_pcm_hw_params_set_channels(pcm_handle_, hw_params, kChannels);
#if 1
    snd_pcm_hw_params_set_period_size(pcm_handle_, hw_params, kPeriodSize, 0);

    // Set buffer size to 4 periods (1024 frames)
    snd_pcm_hw_params_set_buffer_size(pcm_handle_, hw_params, kBufferSize);
#else
    unsigned period_time = 0;
    unsigned buffer_time = 0;
    snd_pcm_uframes_t period_frames = 0;
    snd_pcm_uframes_t buffer_frames = 0;

    if (buffer_time == 0 && buffer_frames == 0) {
        int err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time, 0);
        assert(err >= 0);
        if (buffer_time > 500000) buffer_time = 500000;
    }
    if (period_time == 0 && period_frames == 0) {
        if (buffer_time > 0) period_time = buffer_time / 4;
        else period_frames = buffer_frames / 4;
    }

    if (period_time > 0) {
        snd_pcm_hw_params_set_period_time_near(pcm_handle_, hw_params, &period_time, 0);
    } else {
        snd_pcm_hw_params_set_period_size_near(pcm_handle_, hw_params, &period_frames, 0);
    }

    if (buffer_time > 0) {
        snd_pcm_hw_params_set_buffer_time_near(pcm_handle_, hw_params, &buffer_time, 0);
    } else {
        snd_pcm_hw_params_set_buffer_size_near(pcm_handle_, hw_params, &buffer_frames);
    }
#endif
        
    if (snd_pcm_hw_params(pcm_handle_, hw_params) < 0) {
        std::cout << "Failed to set hardware parameters" << std::endl;
        snd_pcm_hw_params_free(hw_params);
        return;
    }

    // Get the actual negotiated period size and buffer size
    snd_pcm_hw_params_get_period_size(hw_params, &period_frames_, 0);
    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size_);
    std::cout << "=== Hardware parameters ===" << std::endl;
    std::cout << "Period size: " << period_frames_ << std::endl;
    std::cout << "Buffer size: " << buffer_size_ << std::endl;

    const size_t interleaved_buffer_size =
        static_cast<size_t>(period_frames_) * kChannels * kBytesPerSample;

    std::cout << "Interleaved buffer size: " << interleaved_buffer_size << std::endl;
    
    interleaved_buffer_.resize(interleaved_buffer_size);

    if (snd_pcm_prepare(pcm_handle_) < 0) {
        std::cout << "Failed to prepare PCM" << std::endl;
        snd_pcm_hw_params_free(hw_params);
        return;
    }

    snd_pcm_start(pcm_handle_);
    snd_pcm_hw_params_free(hw_params);
}

void Microphone::prime_capture() {
    if (pcm_handle_ == nullptr) {
        std::cout << "PCM handle is null; initialization failed" << std::endl;
        return;
    }
    // Failed the initialization
    if (snd_pcm_state(pcm_handle_) != SND_PCM_STATE_RUNNING) {
        std::cout << "Failed to start capture" << std::endl;
        return;
    }

    // Read data from the microphone    
    const snd_pcm_sframes_t frames_read = snd_pcm_readi(
        pcm_handle_, interleaved_buffer_.data(), period_frames_);

    // Failed to read data
    if (frames_read < 0) {
        std::cout << "Failed to read data" << std::endl;
        return;
    }

    if (frames_read != static_cast<snd_pcm_sframes_t>(period_frames_)) {
        std::cout << "Failed to read full buffer" << std::endl;
        return;
    }
}

void Microphone::start_capture() {
    if (pcm_handle_ == nullptr) {
        std::cout << "PCM handle is null; initialization failed" << std::endl;
        return;
    }
    // Failed the initialization
    if (snd_pcm_state(pcm_handle_) != SND_PCM_STATE_RUNNING) {
        std::cout << "Failed to start capture" << std::endl;
        return;
    }

#if 1
    // Read data from the microphone    
    const snd_pcm_sframes_t frames_read = snd_pcm_readi(
        pcm_handle_, interleaved_buffer_.data(), period_frames_);

    uint64_t current_tsc_ticks = Microphone::getCurrentTSCTicks();

    // Initialize the previous TSC ticks
    static bool first_time = true;
    if (first_time) {
        previous_tsc_ticks_ = current_tsc_ticks;
        first_time = false;
        return;
    }

    uint64_t elapsed_ticks = current_tsc_ticks - previous_tsc_ticks_;
    previous_tsc_ticks_ = current_tsc_ticks;
#else
    uint64_t tsc_ticks = Microphone::getCurrentTSCTicks();

    // Read data from the microphone    
    const snd_pcm_sframes_t frames_read = snd_pcm_readi(
        pcm_handle_, interleaved_buffer_.data(), period_frames_);

    uint64_t current_tsc_ticks = Microphone::getCurrentTSCTicks();
    uint64_t elapsed_ticks = current_tsc_ticks - tsc_ticks;
#endif

    // std::cout << "Elapsed ticks: " << elapsed_ticks << std::endl;
    uint64_t elapsed_nanoseconds = Microphone::ticksToNanoseconds(elapsed_ticks);
    
    times_stamps_.push_back(elapsed_nanoseconds);
    //file_ << "Elapsed time of snd_pcm_readi: " << elapsed_nanoseconds << "ns" << std::endl;
    //file_.flush();

    // Failed to read data
    if (frames_read < 0) {
        std::cout << "Failed to read data" << std::endl;
        return;
    }

    if (frames_read != static_cast<snd_pcm_sframes_t>(period_frames_)) {
        std::cout << "Failed to read full buffer" << std::endl;
        return;
    }
}

void Microphone::stop_capture() {
    if (pcm_handle_ != nullptr) {
        snd_pcm_drop(pcm_handle_);
    }
}

double Microphone::calculate_value_at_percentile_ms(double percentile) {
    if (times_stamps_.empty()) {
        return 0.0;
    }
    
    std::sort(times_stamps_.begin(), times_stamps_.end());
    
    size_t n = times_stamps_.size();
    size_t index = static_cast<size_t>(std::ceil(percentile * n)) - 1;
    index = std::min(index, n - 1);
    
    return times_stamps_[index] / 1000000.0;
}

double Microphone::calculate_average_ms() {
    if (times_stamps_.empty()) {
        return 0.0;
    }
    return std::accumulate(times_stamps_.begin(), times_stamps_.end(), 0.0) / times_stamps_.size() / 1000000.0;
}

double Microphone::expected_interval_ms() {
    // Calculate the expected call interval in milliseconds
    // (256/48000)*1000 = 5.333333333333333ms
    return (static_cast<double>(period_frames_) / kSampleRate) * 1000.0;
}

void Microphone::save_times_stamps_to_file(std::string filename) {
    std::ofstream file(filename.c_str());

    if (file.is_open()) {
        std::sort(times_stamps_.begin(), times_stamps_.end());
        for (const auto& timestamp : times_stamps_) {
            file << timestamp << std::endl;
        }
        file.flush();
    }
}
