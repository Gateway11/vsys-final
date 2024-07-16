/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AUDIO_EXTRA_H
#define AUDIO_EXTRA_H

#include <audio_route/audio_route.h>
#include <semaphore.h>

#include "audio_hw.h"

#define HW_INFO_ARRAY_MAX_SIZE 32
#define MAX_SND_CARD 8
#define OS_SYNC_RELEASED 0
#define OS_SYNC_TIMEOUT 1
#define OS_SYNC_FAILED 0xffffffffu
#define FP_CLOSE(fp) \
    if (fp) {        \
        fclose(fp);  \
        fp = NULL;   \
    }

#define FP_WRITE(buf, sz, fp)   \
    if (fp) {                   \
        fwrite(buf, sz, 1, fp); \
        fflush(fp);             \
    }
struct snd_card_split {
    char chip[HW_INFO_ARRAY_MAX_SIZE];
    char codec[HW_INFO_ARRAY_MAX_SIZE];
    char board[HW_INFO_ARRAY_MAX_SIZE];
};
struct snd_card_split *audio_extra_get_snd_card_split();
void audio_extra_set_snd_card_split(const char *in_snd_card_name);
bool audio_extra_utils_resolve_config_file(char[]);
int audio_extra_utils_get_snd_card_num(struct alsa_audio_device *adev);
struct audio_route *audio_extra_get_audio_route(struct alsa_audio_device *adev, const int card_id);
const char *audio_extra_get_android_format_name(audio_format_t format);
const char *audio_extra_get_alsa_format_name(enum pcm_format format);
const char *audio_extra_get_channel_name(audio_channel_mask_t mask);
uint32_t audio_extra_semwait(sem_t *sem, uint32_t timeout);
void dump_out_data(const void *buffer, size_t bytes, int *size, int id);
int32_t audio_extra_select_slot_16(int16_t *dst, int32_t dst_ch, int16_t *src, int32_t src_ch,
                                   int32_t src_frames);
void audio_extra_sw_slot_gain_16(int32_t slots, uint32_t slots_mask, float amplitude_ratio,
                                   const void *buffer, size_t bytes);

void audio_extra_sw_gain_16(float amplitude_ratio, const void *buffer, size_t bytes);
#endif /* AUDIO_EXTRA_H */
