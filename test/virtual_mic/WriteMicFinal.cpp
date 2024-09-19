/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "AHAL: VirtualMic"
#define LOG_NDDEBUG 0

#include <thread>
#include <fstream>
#include <list>
#include <shared_mutex>
#if 1
#include <log/log.h>

#include "VirtualMic.h"
#include "MemoryManager.h"
#include "AudioCommon.h"

#ifdef DYNAMIC_LOG_ENABLED
#include <log_xml_parser.h>
#define LOG_MASK HAL_MOD_FILE_FM
#include <log_utils.h>
#endif
#else
#include "MemoryManager.h"
enum track_type_t {
    TRACK_DEFAULT = 1,
};

#define AHAL_DBG printf
#define AHAL_WARN printf
#endif

#define BUFFER_SIZE 3840
#define MAX_DELAY 19000

struct Track {
    Track(track_type_t type) : type(type) {
        //size_t total_memorysize = 1 * 1024 * 1024;  // 1MB
        size_t total_memorysize = BUFFER_SIZE * 6;
        size_t blockSize = BUFFER_SIZE;  // Block size of 3840 bytes

        memory = std::make_shared<MemoryManager>(total_memorysize, blockSize);
    }

    std::shared_ptr<MemoryManager> memory;
    std::mutex mutex;
    std::list<uint8_t*> data;
    std::chrono::steady_clock::time_point tp;
    track_type_t type;
};

std::shared_mutex mutex;
std::list<std::shared_ptr<Track>> list_tracks;
std::chrono::steady_clock::time_point tp_write;

uint32_t session = 0;
std::ofstream output;

uint32_t calculate_wait_time(size_t block_size, size_t total_size, uint32_t wait_time) {
#define max(a, b)  ((a) > (b) ? (a) : (b))
    //return (block_size >= total_size * 2 / 3) ? std::max(0, wait_time - 500) : wait_time;
    return (block_size >= total_size - 1) ? max(0, wait_time - 500) : wait_time;
}

void time_check(std::chrono::steady_clock::time_point& tp, uint32_t max_delay, char* tag) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - tp);

    AHAL_DBG("%s: time %02lld ms, %d", tag, elapsed.count(), max_delay);

    if (elapsed.count() < max_delay && elapsed.count() >= 0)
        std::this_thread::sleep_for(std::chrono::microseconds(max_delay) - elapsed);

    tp = std::chrono::steady_clock::now();

#if 0
    if (elapsed.count() > max_delay) {
        auto adjustment = std::chrono::microseconds(elapsed.count() - max_delay);
        tp += adjustment;
    }
#else
    auto adjustment = elapsed.count() - max_delay;
    if (adjustment > 0 && adjustment <= max_delay) {
        tp -= std::chrono::microseconds(adjustment);
    }
#endif
}

void virtual_mic_write(const uint8_t* buf, size_t bytes) {
    uint32_t num_tracks = 0;

    AHAL_DBG("Enter, %zu", bytes);
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        num_tracks = list_tracks.size();

        if (num_tracks && bytes == BUFFER_SIZE) {
            for (auto& track: list_tracks) {
                void* block = track->memory->allocate();
                if (block != nullptr) {
                    memcpy(block, buf, BUFFER_SIZE);
                    track->data.push_back((uint8_t *)block);
                } else {
                    AHAL_WARN("overrun\n");
                }
            }
        }
    }
    //if (num_tracks) {
        output.write((const char *)buf, bytes);
        time_check(tp_write, MAX_DELAY - 500, "write");
    //}
    AHAL_DBG("Exit");
}

ssize_t virtual_mic_read(track_type_t type, uint8_t* buf, size_t size) {
    ssize_t bytes_read = 0, time = 3;
    std::unique_lock<std::mutex> locker;
    std::chrono::steady_clock::time_point* tp;

    while (time--) {
        {
            std::shared_lock<std::shared_mutex> lock(mutex);
            for (auto& track: list_tracks) {
                AHAL_DBG("############ %zd, %zu", time, track->data.size());
                if (track->type == type && track->data.size()) {
                    uint8_t* block = track->data.front();
                    memcpy(buf, block, size);

                    track->data.pop_front();
                    track->memory->deallocate(block);
                    bytes_read = size;

                    locker = std::unique_lock<std::mutex>(track->mutex);
                    tp = &(track->tp);
                    goto done;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(MAX_DELAY / 2));
    }
done:
    if (time < 0) {
        AHAL_WARN("underrun\n");
    } else
        time_check(*tp, MAX_DELAY, "read ");

    return bytes_read;
}

void virtual_mic_start(track_type_t type) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    list_tracks.emplace_front(std::make_shared<Track>(type));

    if (list_tracks.size() == 1) {
        char path[64];
        snprintf(path, sizeof(path), "/data/virtual_mic/48000.2.16bit_%02d.pcm", session++);
        output.open(path, std::ios::out | std::ios::binary);
    }
}

void virtual_mic_stop(track_type_t type) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    for (auto it = list_tracks.begin(); it != list_tracks.end(); ++it) {
        if ((*it)->type == type) {
            std::lock_guard<std::mutex> lg((*it)->mutex);
            list_tracks.erase(it);
            break;
        }
    }
    if (list_tracks.empty())
        output.close();
}

#if 0
int32_t main() {
    uint8_t buf[BUFFER_SIZE];

    virtual_mic_start(TRACK_DEFAULT);
    while (true)
        virtual_mic_read(TRACK_DEFAULT, buf, BUFFER_SIZE);
    virtual_mic_stop(TRACK_DEFAULT);
}
#endif
