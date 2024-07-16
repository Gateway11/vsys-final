/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "audio_hw_extra"
// #define LOG_NDEBUG 0

#include "audio_extra.h"

#include <cutils/properties.h>
#include <dlfcn.h>
#include <errno.h>
#include <log/log.h>
#include <stdlib.h>

#include "audio_hw.h"
#include "platform.h"
#define MAX_LENGTH_MIXER_CONTROL_IN_INT 128

static const char *kConfigLocationList[] = {"/odm/etc", "/vendor/etc", "/system/etc"};
static const int kConfigLocationListSize =
    (sizeof(kConfigLocationList) / sizeof(kConfigLocationList[0]));

struct snd_card_split cur_snd_card_split = {
    .chip = {0},
    .codec = {0},
    .board = {0},
};

static const struct string_to_enum channels_name_to_enum_table[] = {
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_STEREO),   STRING_TO_ENUM(AUDIO_CHANNEL_OUT_5POINT1),
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_7POINT1),  STRING_TO_ENUM(AUDIO_CHANNEL_IN_MONO),
    STRING_TO_ENUM(AUDIO_CHANNEL_IN_STEREO),    STRING_TO_ENUM(AUDIO_CHANNEL_IN_FRONT_BACK),
    STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_1), STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_2),
    STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_3), STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_4),
    STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_5), STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_6),
    STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_7), STRING_TO_ENUM(AUDIO_CHANNEL_INDEX_MASK_8),
};

static const struct string_to_enum android_format_name_to_enum_table[] = {
    STRING_TO_ENUM(AUDIO_FORMAT_PCM_16_BIT),
    STRING_TO_ENUM(AUDIO_FORMAT_PCM_24_BIT_PACKED),
    STRING_TO_ENUM(AUDIO_FORMAT_PCM_8_24_BIT),
    STRING_TO_ENUM(AUDIO_FORMAT_PCM_32_BIT),
};

static const struct string_to_enum alsa_format_name_to_enum_table[] = {
    STRING_TO_ENUM(PCM_FORMAT_S16_LE),
    STRING_TO_ENUM(PCM_FORMAT_S24_3LE),
    STRING_TO_ENUM(PCM_FORMAT_S24_LE),
    STRING_TO_ENUM(PCM_FORMAT_S32_LE),
};
struct snd_card_split *audio_extra_get_snd_card_split() {
    return &cur_snd_card_split;
}
bool audio_extra_utils_resolve_config_file(char file_name[MIXER_PATH_MAX_LENGTH]) {
    char full_config_path[MIXER_PATH_MAX_LENGTH];
    for (int i = 0; i < kConfigLocationListSize; i++) {
        snprintf(full_config_path, MIXER_PATH_MAX_LENGTH, "%s/%s", kConfigLocationList[i],
                 file_name);
        if (F_OK == access(full_config_path, 0)) {
            strcpy(file_name, full_config_path);
            return true;
        }
    }
    return false;
}
void audio_extra_set_snd_card_split(const char *in_snd_card_name) {
    /* sound card name follows below mentioned convention
       <target name>-<sound card name>-<form factor>-snd-card
       parse target name, sound card name and form factor
    */
    char *snd_card_name = strdup(in_snd_card_name);
    char *tmp = NULL;
    char *chip = NULL;
    char *codec = NULL;
    char *board = NULL;

    if (in_snd_card_name == NULL) {
        ALOGE("%s: snd_card_name passed is NULL", __func__);
        goto on_error;
    }

    chip = strtok_r(snd_card_name, "-", &tmp);
    if (chip == NULL) {
        ALOGE("%s: called on invalid chip name", __func__);
        goto on_error;
    }
    strlcpy(cur_snd_card_split.chip, chip, HW_INFO_ARRAY_MAX_SIZE);

    board = strtok_r(NULL, "-", &tmp);
    if (board == NULL) {
        ALOGE("%s: called on invalid board name", __func__);
        goto on_error;
    }
    strlcpy(cur_snd_card_split.board, board, HW_INFO_ARRAY_MAX_SIZE);

    codec = strtok_r(NULL, "-", &tmp);
    if (codec == NULL) {
        ALOGE("%s: called on invalid codec name", __func__);
        goto on_error;
    }
    strlcpy(cur_snd_card_split.codec, codec, HW_INFO_ARRAY_MAX_SIZE);

    ALOGV("%s: snd_card_name(%s) chip(%s) codec(%s) board(%s)", __func__, in_snd_card_name, chip,
          codec, board);

on_error:
    if (snd_card_name)
        free(snd_card_name);
}

int audio_extra_utils_get_snd_card_num(struct alsa_audio_device *adev) {
    int snd_card_num = 0;
    int i = 0;

    if (!adev) {
        ALOGE("%s() %d: invalid arg", __func__, __LINE__);
        return -1;
    }

    for (i = 0; i < MAX_AUDIO_DEVICES; i++) {
        if (adev->platform->dev_manager[i].flag_exist) {
            ++snd_card_num;
        }
    }
    return snd_card_num;
}

struct audio_route *audio_extra_get_audio_route(struct alsa_audio_device *adev, const int card_id) {
    struct audio_route_l *ar_l = NULL;
    struct listnode *p = NULL;
    struct listnode *q = NULL;
    struct audio_route *ar = NULL;
    if (!adev) {
        ALOGE("%s() %d: invalid arg", __func__, __LINE__);
        return NULL;
    }

    if (!list_empty(&adev->audio_route_list)) {
        list_for_each_safe(p, q, &adev->audio_route_list) {
            ar_l = node_to_item(p, struct audio_route_l, list);
            if (card_id == ar_l->card_id) {
                ar = ar_l->audio_route;
                break;
            }
        }
    }

    return ar;
}
static const char *get_format_name(struct string_to_enum *table, size_t len, uint32_t format) {
    const char *name = NULL;
    size_t i;
    for (i = 0; i < len; i++) {
        if (table[i].value == format) {
            name = table[i].name;
            break;
        }
    }
    return name;
}
const char *audio_extra_get_android_format_name(audio_format_t format) {
    return get_format_name((struct string_to_enum *)android_format_name_to_enum_table,
                           ARRAY_SIZE(android_format_name_to_enum_table), format);
}
const char *audio_extra_get_alsa_format_name(enum pcm_format format) {
    return get_format_name((struct string_to_enum *)alsa_format_name_to_enum_table,
                           ARRAY_SIZE(alsa_format_name_to_enum_table), format);
}
const char *audio_extra_get_channel_name(audio_channel_mask_t mask) {
    int j;
    for (j = 0; j < ARRAY_SIZE(channels_name_to_enum_table); j++) {
        if (channels_name_to_enum_table[j].value == mask) {
            return channels_name_to_enum_table[j].name;
        }
    }
    return NULL;
}
uint32_t audio_extra_semwait(sem_t *sem, uint32_t timeout_ms /*ms*/) {
    uint32_t result = OS_SYNC_RELEASED;
    struct timespec tm;
    long one_sec_in_ns = (int)1e9;
    bool got_result = false;

    clock_gettime(CLOCK_REALTIME, &tm);

    /* add timeout_ms (can't overflow): */
    tm.tv_sec += (timeout_ms / 1000);
    tm.tv_nsec += ((timeout_ms % 1000) * 1000000);

    /* make sure nanoseconds are below a million */
    if (tm.tv_nsec >= one_sec_in_ns) {
        tm.tv_sec++;
        tm.tv_nsec -= one_sec_in_ns;
    }

    while (!got_result) {
        if (sem_timedwait(sem, &tm) == -1) {
            int e = errno;

            if (e == EINTR) {
                /* interrupted by signal? repeat sem_wait again
                 */
                continue;
            }

            if (e == ETIMEDOUT) {
                result = OS_SYNC_TIMEOUT;
                got_result = true;
            } else {
                result = OS_SYNC_FAILED;
                got_result = true;
            }
        } else {
            got_result = true;
        }
    }
    return result;
}

/* Debug for dump data, use prop for control. */
void dump_out_data(const void *buffer, size_t bytes, int *size, int id) {
    int sz = 0;
    static FILE *fd = NULL;
    static int offset = 0;
    char name[1024];
    snprintf(name, 1024, "/data/misc/audioserver/dump_audio_%d.pcm", id);
    if (fd == NULL) {
        fd = fopen(name, "wb+");
        if (fd == NULL) {
            ALOGE("open %s fail,(%s)%d, maybe adb shell touch %s", name, strerror(errno), errno,
                  name);
            offset = 0;
            return;
        }
    }
    sz = fwrite(buffer, bytes, 1, fd);
    ALOGD("write %s -> %zu bytes", name, bytes);
    offset += bytes;
    fflush(fd);
    /* Stop dump if over than 100M */
    if (offset >= (100 * 1024 * 1024)) {
        if (size)
            *size = 0;
        fclose(fd);
        offset = 0;
        system("setprop media.audio.record 0");
        ALOGD("TEST playback pcmfile end");
    }
}

int32_t audio_extra_select_slot_16(int16_t *dst, int32_t dst_ch, int16_t *src, int32_t src_ch,
                                   int32_t src_frames) {
    int32_t src_idx = 0;
    // TODO: src_slot_base can be customized in the future
    int32_t src_slot_base = 0;
    int32_t dst_idx = 0;
    int32_t i;
    int32_t cp_sz = sizeof(int16_t) * dst_ch;

    for (i = 0; i < src_frames / src_ch; i++) {
        src_idx = i * src_ch + src_slot_base;
        dst_idx = i * dst_ch;
        memcpy(dst + dst_idx, src + src_idx, cp_sz);
    }
    return i * dst_ch;
}

#define CAL_SW_GAIN_16(ratio, buffer, multiplied)    \
({    \
    (multiplied) = (buffer) * (ratio);    \
    if ((multiplied) > INT16_MAX) {    \
        (buffer) = INT16_MAX;    \
    } else if ((multiplied) < INT16_MIN) {    \
        (buffer) = INT16_MIN;    \
    } else {    \
        (buffer) = (int16_t)(multiplied);    \
    }    \
})

// int32_t slots: total slots of per frame.
// uint32_t slots_mask: slots to be set gain.
// for example,if total slots=8, set slot 1, slot 3,and slot 5's gain, slots_mask=0x2A.
void audio_extra_sw_slot_gain_16(int32_t slots, uint32_t slots_mask, float amplitude_ratio, const void *buffer, size_t bytes) {
    if (amplitude_ratio < 0)
        return;
    if (slots <= 0)
        return;
    int16_t *int16_buffer = (int16_t *)buffer;
    size_t int16_size = bytes / sizeof(int16_t);
    int j = 0;
    float multiplied = 1.0;
    float ratio = 1.0;
    for (int i = 0; i < int16_size; i++) {
        j = i % slots;
        if ((slots_mask >> j) & 1) {
            ratio = amplitude_ratio;
        } else {
            ratio = 1.0;
        }
        CAL_SW_GAIN_16(ratio, int16_buffer[i], multiplied);
    }
}

void audio_extra_sw_gain_16(float amplitude_ratio, const void *buffer, size_t bytes) {
    if (amplitude_ratio < 0)
        return;
    int16_t *int16_buffer = (int16_t *)buffer;
    size_t int16_size = bytes / sizeof(int16_t);
    float multiplied = 1.0;
    for (int i = 0; i < int16_size; i++) {
        CAL_SW_GAIN_16(amplitude_ratio, int16_buffer[i], multiplied);
    }
}