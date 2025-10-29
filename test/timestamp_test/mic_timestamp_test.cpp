#include "microphone.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <algorithm>

constexpr double kMaxPercentileDifference = 0.1;

int main(int argc, char* argv[]) {
    int kMinutesToTest = 5;
    if (argc == 2) {
        kMinutesToTest = std::stoi(argv[1]);
    }

    Microphone microphone("hw:0,4");

    // Prime the capture, to ensure buffer is filled.
    for (int i = 0; i < 20; i++) {
        microphone.prime_capture();
    }

    std::cout << "=== Starting capture for " << kMinutesToTest << " minutes ===" << std::endl;

    // Capture for 5 minutes
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
    int sample_count = 0;
    int test_duration_seconds = kMinutesToTest * 60;
    while (std::chrono::high_resolution_clock::now() - start_time < std::chrono::minutes(kMinutesToTest)) {
        microphone.start_capture();
        sample_count++;

        // Progress indicator every 5000 samples
        if (sample_count % 5000 == 0) {
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count();
            std::cout << "Progress: " << std::fixed << std::setprecision(1) 
                        << elapsed_seconds << "s / " << test_duration_seconds << "s "
                        << "(" << sample_count << " samples)" << std::endl;
        }
    }

    // Close the microphone.
    std::cout << "=== Test complete ===" << std::endl;

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Total test duration: " << total_duration.count() << "ms" << std::endl;

    // Calculate the average time taken to capture audio.
    double average_time = microphone.calculate_average_ms();
    std::cout << "Average time taken to capture audio: " << std::fixed << std::setprecision(6) << average_time << "ms" << std::endl;
    
    // Calculate the average time taken to capture audio.
    double expected_time = microphone.expected_interval_ms();
    std::cout << "Expected time between calls: " << std::fixed << std::setprecision(6) << expected_time << "ms" << std::endl;

    const auto print_percentile_error = [&](double percentile) -> double {
        return std::max(0.0, (microphone.calculate_value_at_percentile_ms(percentile) - expected_time));
    };

    std::cout << "Error at 50th percentile: " << print_percentile_error(0.5) << " ms" << std::endl;
    std::cout << "Error at 99th percentile: " << print_percentile_error(0.99) << " ms" << std::endl;
    std::cout << "Error at 99.999th percentile: " << print_percentile_error(0.99999) << " ms" << std::endl;
    std::cout << "Error at 100th percentile: " << print_percentile_error(1.0) << " ms" << std::endl;

    std::cout << "Raw sorted timestamps in file: time_elapsed.txt" << std::endl;
    microphone.save_times_stamps_to_file("time_elapsed.txt");

    return 0;
}