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
#include <map>
#include <shared_mutex>
#include <log/log.h>

#include "VirtualMic.h"
#include "MemoryManager.h"
#include "AudioCommon.h"

#ifdef DYNAMIC_LOG_ENABLED
#include <log_xml_parser.h>
#define LOG_MASK HAL_MOD_FILE_FM
#include <log_utils.h>
#endif

#define BUFFER_SIZE 3840
#define MAX_DELAY 18

static std::map<track_type_t, std::pair<std::list<uint8_t*>, std::shared_ptr<MemoryManager>>> map_tracks;
static std::shared_mutex mutex;
static std::chrono::steady_clock::time_point tp_write, tp_read;

static uint32_t session = 0;
static std::ofstream output;

void time_check(std::chrono::steady_clock::time_point& tp, uint32_t max_delay) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - tp);

    AHAL_DBG("time %d ms", elapsed.count());

    if (elapsed.count() < max_delay && elapsed.count() > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(max_delay - elapsed.count()));

    tp = std::chrono::steady_clock::now();

    if (elapsed.count() > max_delay) {
        auto adjustment = std::chrono::milliseconds(elapsed.count() - max_delay);
        tp += adjustment;
    }
}

void virtual_mic_write(const uint8_t* buf, size_t bytes) {
    uint32_t num_tracks = 0;
    AHAL_DBG("Enter, %zu", bytes);
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        num_tracks = track.size();

        if (num_tracks && bytes == BUFFER_SIZE) {
            for (auto& track: map_tracks) {
                void* block = track.second.second->allocate();
                if (block != nullptr) {
                    memcpy(block, buf, BUFFER_SIZE);
                    track.second.first.push_back((uint8_t *)block);
                } else {
                    AHAL_WARN("overrun\n");
                }
            }
        }
    }
    if (num_tracks) {
        output.write((const char *)buf, bytes);
        time_check(tp_write, MAX_DELAY);
    }
    AHAL_DBG("Exit");
}

ssize_t virtual_mic_read(track_type_t type, uint8_t* buf, size_t size) {
    ssize_t bytes_read = 0, time = 3;
    while (time--) {
        {
            std::shared_lock<std::shared_mutex> lock(mutex);
            auto& track = map_tracks[type];

            AHAL_DBG("############ %zd, %zu", time, track.first.size());
            if (track.first.size()) {
                uint8_t* block = track.first.front();
                memcpy(buf, block, size);

                track.first.pop_front();
                track.second->deallocate(block);
                bytes_read = size;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(6));
    }

    if (time <= 0)
        AHAL_WARN("underrun\n");

    time_check(tp_read, MAX_DELAY);
    return bytes_read;
}

void virtual_mic_start(track_type_t type) {
    //size_t total_memorysize = 1 * 1024 * 1024;  // 1MB
    size_t total_memorysize = BUFFER_SIZE * 6;
    size_t blockSize = BUFFER_SIZE;  // Block size of 3840 bytes

    std::unique_lock<std::shared_mutex> lock(mutex);
    map_tracks.try_emplace(type, std::list<uint8_t*>(),
            std::make_shared<MemoryManager>(total_memorysize, blockSize));

    if (map_tracks.size() == 1) {
        char path[64];
        snprintf(path, sizeof(path), "/data/virtual_mic/48000.2.16bit_%02d.pcm", session++);
        output.open(path, std::ios::out | std::ios::binary);
    }
}

void virtual_mic_stop(track_type_t type) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    map_tracks.erase(type);

    if (map_tracks.empty())
        output.close();
}
