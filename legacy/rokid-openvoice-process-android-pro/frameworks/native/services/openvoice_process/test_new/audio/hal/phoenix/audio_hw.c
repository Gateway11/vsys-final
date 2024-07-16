/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "audio_hw_x9"
// #define LOG_NDEBUG 0
#include "audio_hw.h"

#include <audio_effects/effect_aec.h>
#include <audio_extra.h>
#include <audio_utils/echo_reference.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/str_parms.h>
#include <errno.h>
#include <hardware/audio.h>
#include <hardware/audio_alsaops.h>
#include <hardware/audio_effect.h>
#include <hardware/hardware.h>
#include <hfp.h>
#include <inttypes.h>
#include <malloc.h>
#include <math.h>
#include <pthread.h>
#include <sound/asound.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <system/audio.h>
#include <system/audio_effects/effect_loudnessenhancer.h>

#include "PerfServiceApi.hpp"
#include "ecnr_fake.h"
#include "remote.h"
#include "audio_dsp_common.h"
#include "audio_control_wrap.h"

#define MAX_DSP_INIT_COUNT 3
#define OUT_MAX_PROCESSED_BUF_PERIOD (OUT_PERIOD_MS * 3)  // ms
const char *default_effect_lib = "/vendor/lib/soundfx/libsdldnhncr.so";
const char *STR_DEV_BUS1_ADDR = "bus0_media_out";
const char *STR_DEV_BUS2_ADDR = "bus1_voice_command_out";
const char *STR_DEV_BUS3_ADDR = "bus2_phone_out";
const char *STR_DEV_BUS4_ADDR = "bus3_navi_out";
const char *STR_DEV_BUS5_ADDR = "bus4_call_ring_out";
const char *STR_DEV_BUS6_ADDR = "bus5_alarm_out";
const char *STR_DEV_BUS7_ADDR = "bus6_notification_out";
const char *STR_DEV_BUS8_ADDR = "bus7_system_sound_out";
const char *STR_DEV_BUS100_ADDR = "bus100_rear_seat1";

//For audio track sharing virtual i2s switch.
const char *STR_VIRT_I2S_SWITCH = "Virt-i2s Switch";

// default value for pcm config.
// use platform_info xml to customize it, which located in
// device\semidrive\common\audio\audio_platform_info_xx.xml
static struct pcm_config default_output_config = {
    .channels = CHANNEL_7POINT1,
    .rate = HIFI_SAMPLING_RATE,
    .period_size = (HIFI_SAMPLING_RATE * OUT_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};
static struct pcm_config default_input_config = {
    .channels = CHANNEL_7POINT1,
    .rate = HIFI_SAMPLING_RATE,
    .period_size = (HIFI_SAMPLING_RATE * OUT_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config rear_100_cap_cfg_ref = {
    .channels = CHANNEL_STEREO,
    .rate = HIFI_SAMPLING_RATE,
    .period_size = (HIFI_SAMPLING_RATE * OUT_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};
/* Next is virtual configuration */
struct pcm_config main_config_ref_virtual = {
    .channels = CHANNEL_STEREO,
    .rate = HIFI_SAMPLING_RATE,
    .period_size = (HIFI_SAMPLING_RATE * OUT_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config *pcm_main_config = &default_output_config;
struct pcm_config *pcm_main_cap_config = &default_input_config;

struct audio_bus_status current_audio_bus_status;

static void set_btcall_status(struct alsa_audio_device *adev);
static void select_mode(struct alsa_audio_device *adev);
static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode);

extern bool use_channel_map;
static int create_effect(struct alsa_stream_out *out) {
    if (out->effect_desc == NULL || out == NULL)
        return -EINVAL;
    effect_uuid_t loudnessenhancer_uuid = {
        0xfa415329, 0x2034, 0x4bea, 0xb5dc, {0x5b, 0x38, 0x1c, 0x8d, 0x1e, 0x2c}};
    return out->effect_desc->create_effect(&loudnessenhancer_uuid, 0, 0, &out->effect_hdl);
}
static int out_effect_config(struct alsa_stream_out *out, const void *in_buffer,
                             const void *out_buffer, size_t out_frames) {
    uint32_t cmd_code = EFFECT_CMD_SET_CONFIG;
    uint32_t cmd_size = sizeof(effect_config_t);
    effect_config_t cmd_data;
    uint32_t reply_size = sizeof(uint32_t);
    int32_t reply_data = 0;
    int ret = 0;
    if (out == NULL || in_buffer == NULL || out_buffer == NULL)
        return -EINVAL;
    /*Input/Output config*/
    cmd_data.inputCfg.samplingRate = out->config.rate;
    cmd_data.outputCfg.samplingRate = out->config.rate;
    cmd_data.inputCfg.channels = out->config.channels;
    cmd_data.outputCfg.channels = out->config.channels;
    switch (out->config.format) {
        case PCM_FORMAT_S16_LE:
            cmd_data.inputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
            cmd_data.outputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
            break;
        default:
            cmd_data.inputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
            cmd_data.outputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
            ALOGE("Unsupport format\n");
            break;
    }
    cmd_data.inputCfg.buffer.s16 = (int16_t *)in_buffer;
    cmd_data.outputCfg.buffer.s16 = (int16_t *)out_buffer;
    cmd_data.inputCfg.buffer.frameCount = out_frames;
    cmd_data.outputCfg.buffer.frameCount = out_frames;
    cmd_data.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_WRITE;
    // cmd_data.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_ACCUMULATE;
    ret = (*out->effect_hdl)
              ->command(out->effect_hdl, cmd_code, cmd_size, &cmd_data, &reply_size, &reply_data);
    ALOGD("%s() ret: %d", __func__, ret);
    return 0;
}
static int out_effect_set_param(struct alsa_stream_out *out) {
    // struct alsa_audio_device *adev = out->dev;
    int param = LOUDNESS_ENHANCER_PARAM_TARGET_GAIN_MB;
    int value = out->target_gain_mb;  // mB
    int ret = 0;
    uint32_t cmd_code = EFFECT_CMD_SET_PARAM;
    effect_param_t *cmd_data;
    uint32_t cmd_size = sizeof(effect_param_t) + sizeof(uint32_t) + sizeof(int32_t);
    int32_t reply_data = 0;
    uint32_t reply_size = sizeof(int32_t);

    if (out == NULL)
        return -EINVAL;

    cmd_data = (effect_param_t *)malloc(cmd_size);

    cmd_data->psize = sizeof(uint32_t);
    cmd_data->vsize = sizeof(uint32_t);
    memset(cmd_data->data, 0, sizeof(uint32_t) * 2);
    memcpy((cmd_data->data), &param, sizeof(int32_t));
    memcpy((cmd_data->data + sizeof(int32_t)), &value, sizeof(int32_t));

    ret = (*out->effect_hdl)
              ->command(out->effect_hdl, cmd_code, cmd_size, cmd_data, &reply_size, &reply_data);
    ALOGI("%s() value:%d, ret:%d reply_data: %d", __func__, value, ret, reply_data);
    if (cmd_data)
        free(cmd_data);
    return reply_data;
}
static int out_effect_enable(struct alsa_stream_out *out) {
    // struct alsa_audio_device *adev = out->dev;
    uint32_t cmd_code = EFFECT_CMD_ENABLE;
    uint32_t cmd_size = sizeof(effect_config_t);
    effect_config_t cmd_data;
    uint32_t reply_size = sizeof(uint32_t);
    uint32_t reply_data = 0;
    int ret = 0;
    if (out == NULL || out->effect_hdl == NULL)
        return -EINVAL;

    ret = (*out->effect_hdl)
              ->command(out->effect_hdl, cmd_code, cmd_size, &cmd_data, &reply_size, &reply_data);
    ALOGI("enable effect ret: %d", ret);
    return ret;
}

static bool out_is_rear_seat_or_sharing_addr(const char *address) {
    if ((!strncmp(address, "bus100_", 7)) || (!strncmp(address, "bus101_", 7)) ||
        (!strncmp(address, "bus102_", 7)) || (!strncmp(address, "bus103_", 7))
        || (!strncmp(address, "bus9_", 5))) {
        return true;
    }
    return false;
}

static bool out_is_rear_seat_addr(const char *address) {
    if ((!strncmp(address, "bus100", 6)) || (!strncmp(address, "bus101", 6)) ||
        (!strncmp(address, "bus102", 6))) {
        return true;
    }
    return false;
}

static bool sd_virtual_i2s_switch(int32_t card_id, int32_t enable) {
    int ret = 0;
    struct mixer *mixer = mixer_open(card_id);
    struct mixer_ctl *ctl = mixer_get_ctl_by_name(mixer, STR_VIRT_I2S_SWITCH);
    if (!ctl) {
        ALOGW("%s: Could not get ctl for mixer cmd - %s", __func__, STR_VIRT_I2S_SWITCH);
        return -ENOSYS;
    }

    if (enable) {
        if (mixer_ctl_set_value(ctl, 0, 1)) {
            ALOGE("apply bus-sharing vitrual i2s ctrl failed, errorno:%d", ret);
            return ret;
        }
    } else {
        if (mixer_ctl_set_value(ctl, 0, 0)) {
            ALOGE("apply bus-sharing vitrual i2s ctrl failed, errorno:%d", ret);
            return ret;
        }
    }
    return ret;
}

static bool out_is_main_seat_addr(const char *address) {
    if (!strncmp(address, "bus101", 6)) {
        return true;
    }
    return false;
}
struct path_state {
    char *name;
    int32_t ref;
};

struct path_state paths[] = {
    {.name = HIFI_PLAYBACK_TO_MAIN_SPK_48K, .ref = 0},
};

int set_bus_volume( uint32_t which_bus )
{
    int ret = 0;
    uint8_t bus_volume = 0;
    bool is_ducked;
    bool is_muted;

    pthread_mutex_lock(&current_audio_bus_status.lock[which_bus]);
    is_ducked = current_audio_bus_status.is_ducked[which_bus];
    is_muted  = current_audio_bus_status.is_muted[which_bus];
    pthread_mutex_unlock(&current_audio_bus_status.lock[which_bus]);

    if( current_audio_bus_status.is_chime_on == 1)
    {
        if(is_muted == 1)
        {
            bus_volume = 0;
            switch (current_audio_bus_status.slot_index[which_bus])
            {
                case 0:
                {
                    ret = dspdev_set_volume(CHANNEL_MEDIA_MUSIC, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 2:
                {
                    ret = dspdev_set_volume(CHANNEL_BEEP, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 4:
                {
                    ret = dspdev_set_volume(CHANNEL_BT_PHONE, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 6:
                {
                    ret = dspdev_set_volume(CHANNEL_NAVI, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                default:
                break;
            }
        }
        else
        {
            if(current_audio_bus_status.volume[which_bus] > 8)
            {
                bus_volume = 8;
            }
            else
            {
                bus_volume = current_audio_bus_status.volume[which_bus];
            }
            switch (current_audio_bus_status.slot_index[which_bus])
            {
                case 0:
                {
                    ret = dspdev_set_volume(CHANNEL_MEDIA_MUSIC, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 2:
                {
                    ret = dspdev_set_volume(CHANNEL_BEEP, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 4:
                {
                    ret = dspdev_set_volume(CHANNEL_BT_PHONE, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 6:
                {
                    ret = dspdev_set_volume(CHANNEL_NAVI, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                default:
                break;
            }
        }
    }
    else
    {
        if(is_muted == 1)
        {
            bus_volume = 0;
            switch (current_audio_bus_status.slot_index[which_bus])
            {
                case 0:
                {
                    ret = dspdev_set_volume(CHANNEL_MEDIA_MUSIC, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 2:
                {
                    ret = dspdev_set_volume(CHANNEL_BEEP, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 4:
                {
                    ret = dspdev_set_volume(CHANNEL_BT_PHONE, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 6:
                {
                    ret = dspdev_set_volume(CHANNEL_NAVI, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                default:
                break;
            }
        }
        else if(is_ducked == 1)
        {
            for(int i = 0; i < 4; i++)
            {
                if(current_audio_bus_status.focus_bus[i] != 0xFF)
                {
                    if(current_audio_bus_status.volume[current_audio_bus_status.focus_bus[i]] >= bus_volume)
                    {
                        bus_volume = current_audio_bus_status.volume[current_audio_bus_status.focus_bus[i]];
                    }
                }
            }
            if((bus_volume - current_audio_bus_status.volume[which_bus]) > 12)
            {
                bus_volume = current_audio_bus_status.volume[which_bus];
            }
            else
            {
                bus_volume = current_audio_bus_status.volume[which_bus] * 0.8;
            }
            switch (current_audio_bus_status.slot_index[which_bus])
            {
                case 0:
                {
                    ret = dspdev_set_volume(CHANNEL_MEDIA_MUSIC, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 2:
                {
                    ret = dspdev_set_volume(CHANNEL_BEEP, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 4:
                {
                    ret = dspdev_set_volume(CHANNEL_BT_PHONE, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 6:
                {
                    ret = dspdev_set_volume(CHANNEL_NAVI, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                default:
                break;
            }
        }
        else
        {
            bus_volume = current_audio_bus_status.volume[which_bus];
            switch (current_audio_bus_status.slot_index[which_bus])
            {
                case 0:
                {
                    ret = dspdev_set_volume(CHANNEL_MEDIA_MUSIC, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 2:
                {
                    ret = dspdev_set_volume(CHANNEL_BEEP, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 4:
                {
                    ret = dspdev_set_volume(CHANNEL_BT_PHONE, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                case 6:
                {
                    ret = dspdev_set_volume(CHANNEL_NAVI, bus_volume);
                    if(ret < 0)
                    {
                        ALOGE("set MediaVol failed! --- %d", ret);
                    }
                    break;
                }
                default:
                break;
            }
        }
    }

    ALOGD("bus: %d bus_volume: %d, current_vol:%d", which_bus, current_audio_bus_status.volume[which_bus], bus_volume);
    return ret;
}

void set_device_address_is_ducked(uint32_t device_address, bool is_ducked)
{
    uint32_t bus_id = device_address;
    int ret = 0;

    pthread_mutex_lock(&current_audio_bus_status.lock[bus_id]);
    current_audio_bus_status.is_ducked[bus_id] = is_ducked;
    pthread_mutex_unlock(&current_audio_bus_status.lock[bus_id]);

    if(current_audio_bus_status.bus_active[bus_id] == 1)
    {
        ret = set_bus_volume(bus_id);
        if(ret)
        {
            ALOGE("set_bus_volume failed! --- %d", ret);
        }
    }
}

void set_device_address_is_muted(uint32_t device_address, bool is_muted)
{
    uint32_t bus_id = device_address;

    pthread_mutex_lock(&current_audio_bus_status.lock[bus_id]);
    current_audio_bus_status.is_muted[bus_id] = is_muted;
    pthread_mutex_unlock(&current_audio_bus_status.lock[bus_id]);

    set_bus_volume(bus_id);
}

void open_output_path(struct alsa_stream_out *out) {
    char *select_path = NULL;
    int32_t vol = HIFI_PATH_FIXED_VOL;
    bool apply = false;
    switch (out->bus_num) {
        case BUS0:
        case BUS5:
        case BUS9:
            select_path = HIFI_PLAYBACK_TO_MAIN_SPK_48K;
            break;
        case BUS3:
            select_path = NAV_PLAYBACK_TO_MAIN_SPK_48K;
            apply = true;
            break;

        default:
            AALOGE("Invalid bus: %d", out->bus_num);
            break;
    }
    if (select_path) {
        for (int32_t i = 0; i < ARRAY_SIZE(paths); i++) {
            if (!strcmp(paths[i].name, select_path)) {
                paths[i].ref++;
                if (paths[i].ref > 0)
                    apply = true;
                else
                    apply = false;
                break;
            }
        }
    }
    if (apply)
        audio_remote_start(out->dev, select_path, vol);
}

void close_output_path(struct alsa_stream_out *out) {
    char *select_path = NULL;
    bool apply = false;
    switch (out->bus_num) {
        case BUS0:
        case BUS5:
        case BUS9:
            select_path = HIFI_PLAYBACK_TO_MAIN_SPK_48K;
            break;
        case BUS3:
            select_path = NAV_PLAYBACK_TO_MAIN_SPK_48K;
            apply = true;
            break;

        default:
            AALOGI("unsupport bus");
            return;
    }

    if (select_path) {
        for (int32_t i = 0; i < ARRAY_SIZE(paths); i++) {
            if (!strcmp(paths[i].name, select_path)) {
                paths[i].ref--;
                if (paths[i].ref == 0)
                    apply = true;
                else
                    apply = false;
                if (paths[i].ref < 0)
                    paths[i].ref = 0;
                break;
            }
        }
    }
    if (apply)
        audio_remote_stop(out->dev, select_path);
}
struct pcm *pcm_open_prepare_helper(unsigned int snd_card, unsigned int pcm_device_id,
                                    unsigned int flags, unsigned int pcm_open_retry_count,
                                    struct pcm_config *config) {
    struct pcm *pcm = NULL;
    AALOGI(
        "[%d : %d] pcm config: %d %d %d period_size: %d, period_count: %d, start_threshold: %d, "
        "stop_threshold: %d, silence_threshold: %d, silence_size: %d, avail_min: %d",
        snd_card, pcm_device_id, config->format, config->rate, config->channels,
        config->period_size, config->period_count, config->start_threshold, config->stop_threshold,
        config->silence_threshold, config->silence_size, config->avail_min);
    while (1) {
        pcm = pcm_open(snd_card, pcm_device_id, flags, config);
        if (pcm == NULL || !pcm_is_ready(pcm)) {
            ALOGE("%s: %s", __func__, pcm_get_error(pcm));
            if (pcm != NULL) {
                pcm_close(pcm);
                pcm = NULL;
            }
            if (pcm_open_retry_count-- == 0)
                return NULL;

            usleep(PROXY_OPEN_WAIT_TIME * 1000);
            continue;
        }
        break;
    }

    if (pcm_is_ready(pcm)) {
        int ret = pcm_prepare(pcm);
        if (ret < 0) {
            ALOGE("%s: pcm_prepare returned %d", __func__, ret);
            pcm_close(pcm);
            pcm = NULL;
        }
    }

    return pcm;
}

static bool out_is_ssa_sharing_bus(const char *address)
{
	//TODO: strncmp have compare issue such as strncmp(address, "bus1", 4) and addr == bus101
	if (!strncmp(address, "bus9_", 5)) {
		return true;
	}
	return false;
}

static bool out_is_ssb_sharing_bus(const char *address)
{
	if (!strncmp(address, "bus103_", 7)) {
		return true;
	}
	return false;
}
/* must be called with hw device and output stream mutexes locked */

static int start_output_stream(struct alsa_stream_out *out) {
    struct alsa_audio_device *adev = out->dev;
    int card = 0, port = 0;
    struct pcm_config config;

    memset(&config, 0x00, sizeof(struct pcm_config));

    /* default to low power: will be corrected in out_write if necessary
     * before first write to tinyalsa.
     */
    /* out->write_threshold = PLAYBACK_PERIOD_COUNT * PERIOD_SIZE;
    out->config.start_threshold =
        PLAYBACK_PERIOD_START_THRESHOLD * PERIOD_SIZE; */
    /* Only used if the stream is opened in mmap mode. */
    /* out->config.avail_min = PERIOD_SIZE; */
    ALOGI("%s: Playing sample: %u ch, %u hz, %s format\n", out->bus_address, out->config.channels,
          out->config.rate, audio_extra_get_alsa_format_name(out->config.format));
    if (adev->active_input == NULL) {
        ALOGV(" acquirePerfLock for output.\n");
        acquirePerfLock(0, 0);
    }

    adev->active_output = out;
    if (out_is_rear_seat_or_sharing_addr(out->bus_address)) {
        if (adev->is_usb_exist) {
            card = adev->usb_card_id;
            port = adev->usb_port_id;
            usleep(100000);  // waitting for usb audio device to
                             // stabilize
        } else {
            if (!strncmp(out->bus_address, "bus100", 6)) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR1);
                port = CODEC_DEV;
            } else if (!strncmp(out->bus_address, "bus101", 6)) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2);
                port = CODEC_DEV;
            } else if (!strncmp(out->bus_address, "bus102", 6)) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR3);
                port = CODEC_DEV;
            }
#ifdef ENABLE_AUDIO_SHAREING
            else if (out_is_ssb_sharing_bus(out->bus_address)) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2);
                port = REFB_VI2S_DEV;
                /*set vitural i2s control to 1 fot audio track sharing*/
                sd_virtual_i2s_switch(card, 1);
            } else if (out_is_ssa_sharing_bus(out->bus_address)) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR1);
                port = REFA_VI2S_DEV;
                /*set vitural i2s control to 1 fot audio track sharing*/
                sd_virtual_i2s_switch(card, 1);
            }
#endif
        }
        if (card < 0) {
            goto err_out;
        }
        AALOGI("open card %d, device:%d for address: %s", card, port, out->bus_address);
        // out->pcm_wrapper =
        //     pcm_wrapper_open(card, port, PCM_OUT, &out->config);
        // if (out->pcm_wrapper) {
        // 	pcm_wrapper_prepare(out->pcm_wrapper);
        // }

        out->pcm =
            pcm_open_prepare_helper(card, port, PCM_OUT, PROXY_OPEN_RETRY_COUNT, &out->config);
        if (out->pcm == NULL || !pcm_is_ready(out->pcm)) {
            ALOGE("cannot open pcm_out driver: %s", pcm_get_error(out->pcm));
            goto err_out;
        }
    } else if (adev->platform->info.misc.ahub_en) {
        sd_platform_get_card_info(adev->platform, out->bus_address, &card, &port, &out->config);
        out->config.start_threshold = PLAYBACK_PERIOD_START_THRESHOLD * out->config.period_size;
        out->config.stop_threshold = PLAYBACK_PERIOD_STOP_THRESHOLD * out->config.period_size;
        out->pcm =
            pcm_open_prepare_helper(card, port, PCM_OUT, PROXY_OPEN_RETRY_COUNT, &out->config);
        if (out->pcm == NULL || !pcm_is_ready(out->pcm)) {
            AALOGE("cannot open pcm_out driver: %s", pcm_get_error(out->pcm));
            goto err_out;
        }
    } else {
        if ((card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
            goto err_out;
        }
        port = CODEC_DEV;
        open_output_path(out);
        AALOGI("open card %d, device:%d for address: %s", card, port, out->bus_address);
        sd_platform_get_card_pcm_config(adev->platform, AUDIO_NAME_PRIMARY, &config, 0,
                                        AUDIO_HAL_PLAYBACK);
        out->pcm_hdl = hal_streamer_open((struct audio_stream *)out, card, port, PCM_OUT, &config);
        if (NULL == out->pcm_hdl) {
            ALOGE("hal_streamer_open failed !!!");
            goto err_out;
        }
    }
    if (adev->platform->info.misc.sw_out_src) {
        int ret = 0;
        if (HIFI_SAMPLING_RATE != out->config.rate) {
            ret = create_resampler(out->config.rate, HIFI_SAMPLING_RATE, out->config.channels,
                                   RESAMPLER_QUALITY_DEFAULT, NULL, &out->resampler);
            if (ret != 0) {
                ALOGE("create out resampler failed, %d -> %d, channels: %d", out->config.rate,
                      HIFI_SAMPLING_RATE, out->config.channels);
                return ret;
            }

            ALOGI("create out resampler OK, %d -> %d, channels: %d", out->config.rate,
                  HIFI_SAMPLING_RATE, out->config.channels);
        } else
            ALOGI("do not use out resampler");
    }
    return 0;

err_out:
    adev->active_output = NULL;
    if (adev->active_input == NULL) {
        ALOGV("releasePerfLock for output\n \n");
        releasePerfLock(CpuBoostCancelRequest);
    }
    return -ENODEV;
}

static uint32_t out_get_sample_rate(const struct audio_stream *stream) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    return out->config.rate;
}
/* Fix hifi sample rate to 48k, don't support set rate*/
static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
    DEBUG_ITEM_PRT(0)
    return -ENOSYS;
}

static size_t out_get_buffer_size(const struct audio_stream *stream) {
    /* return the closest majoring multiple of 16 frames, as
     * audioflinger expects audio buffers to be a multiple of 16 frames
     */
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    size_t size = ((out->config.period_size + 15) / 16) * 16 *
                  audio_stream_out_frame_size((struct audio_stream_out *)stream);
    DEBUG_ITEM_PRT(size);
    return size;
}

static audio_channel_mask_t out_get_channels(const struct audio_stream *stream) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    return audio_channel_out_mask_from_count(out->config.channels);
}

static audio_format_t out_get_format(const struct audio_stream *stream) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    return audio_format_from_pcm_format(out->config.format);
}

static int out_set_format(struct audio_stream *stream, audio_format_t format) {
    DEBUG_ITEM_PRT(format)
    return -ENOSYS;
}

static int do_output_standby(struct alsa_stream_out *out) {
    DEBUG_FUNC_PRT
    struct alsa_audio_device *adev = out->dev;
    if (!out->standby) {
        ALOGI("out pcm_close %s! \n", out->bus_address);
        if (out_is_rear_seat_or_sharing_addr(out->bus_address) && out->pcm) {
            if (0 == pcm_close(out->pcm)) {
                ALOGD("%s pcm_close success!", out->bus_address);
                out->pcm = NULL;
            } else {
                AALOGE("%s pcm_close failed!", out->bus_address);
            }
#ifdef ENABLE_AUDIO_SHAREING
            int card = -1;
            if (out_is_ssb_sharing_bus(out->bus_address)) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2);
                sd_virtual_i2s_switch(card, 0);
            } else if (out_is_ssa_sharing_bus(out->bus_address)) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR1);
                sd_virtual_i2s_switch(card, 0);
            }
#endif
        } else if (out->pcm_hdl) {
            hal_streamer_close((struct audio_stream *)out, PCM_OUT);
            close_output_path(out);
        } else if (out->pcm) { // ahub usecase
            if (0 == pcm_close(out->pcm)) {
                AALOGI("%s pcm_close success!", out->bus_address);
                out->pcm = NULL;
            } else {
                AALOGE("%s pcm_close failed!", out->bus_address);
            }
        }

        if (out->resampler) {
            release_resampler(out->resampler);
            out->resampler = NULL;
        }

        adev->active_output = NULL;

#ifdef ENABLE_AUDIO_SHAREING
        if (out->repeater_src) {
            au_sharing_release_src(out->repeater_src);
            out->repeater_src = NULL;
        }
#endif
        if (out->effect_desc)
            out->effect_desc->release_effect(out->effect_hdl);

        out->standby = 1;
        if (adev->active_input == NULL) {
            ALOGI("releasePerfLock for output\n \n");
            releasePerfLock(CpuBoostCancelRequest);
        }
    }
    return 0;
}

static int out_standby(struct audio_stream *stream) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    int status;
    pthread_mutex_lock(&out->dev->lock);
    pthread_mutex_lock(&out->lock);
    status = do_output_standby(out);
    pthread_mutex_unlock(&out->lock);

    pthread_mutex_unlock(&out->dev->lock);
    AALOGI("%s stream close completed", out->bus_address);
    return status;
}

static int out_dump(const struct audio_stream *stream, int fd) {
    DEBUG_FUNC_PRT
    return 0;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    struct alsa_audio_device *adev = out->dev;
    struct str_parms *parms;
    char value[32];
    int ret = -ENOSYS;
    int val = 0;
    if (!strcmp(kvpairs, "")) {
        ALOGW("empty params!!!!");
        return 0;
    }
    ALOGI("%s kvpairs: %s", __func__, kvpairs);

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value, sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        pthread_mutex_lock(&adev->lock);
        pthread_mutex_lock(&out->lock);
        if (((adev->devices & AUDIO_DEVICE_OUT_ALL) != val) && (val != 0)) {
            adev->devices &= ~AUDIO_DEVICE_OUT_ALL;
            adev->devices |= val;
        }
        pthread_mutex_unlock(&out->lock);
        pthread_mutex_unlock(&adev->lock);
    } else {
        ret = -ENOSYS;
    }

    str_parms_destroy(parms);

    return ret;
}
static bool stream_get_parameter_channels(struct str_parms *query, struct str_parms *reply,
                                          audio_channel_mask_t *supported_channel_masks) {
    int ret = -1;
    char value[8 * 32 /* max channel name size */];
    bool first = true;
    size_t i;
    const char *str;
    DEBUG_FUNC_PRT

    if (str_parms_has_key(query, AUDIO_PARAMETER_STREAM_SUP_CHANNELS)) {
        ret = 0;
        value[0] = '\0';
        i = 0;
        while (supported_channel_masks[i] != 0) {
            str = audio_extra_get_channel_name(supported_channel_masks[i]);
            if (str) {
                if (!first) {
                    strcat(value, "|");
                }
                strcat(value, str);
                first = false;
            }
            i++;
        }
        str_parms_add_str(reply, AUDIO_PARAMETER_STREAM_SUP_CHANNELS, value);
    }
    return ret >= 0;
}

static bool stream_get_parameter_formats(struct str_parms *query, struct str_parms *reply,
                                         audio_format_t *supported_formats) {
    int ret = -1;
    char value[256];
    ALOGV("%s supports_format:%d", __func__, supported_formats[0]);
    if (str_parms_has_key(query, AUDIO_PARAMETER_STREAM_SUP_FORMATS)) {
        ret = 0;
        value[0] = '\0';
        switch (supported_formats[0]) {
            case AUDIO_FORMAT_PCM_16_BIT:
                strcat(value, "AUDIO_FORMAT_PCM_16_BIT");
                break;
            // case AUDIO_FORMAT_PCM_24_BIT_PACKED:
            // 	strcat(value, "AUDIO_FORMAT_PCM_24_BIT_PACKED");
            // 	break;
            // case AUDIO_FORMAT_PCM_32_BIT:
            // 	strcat(value, "AUDIO_FORMAT_PCM_32_BIT");
            // 	break;
            default:
                ALOGE("%s: unsupported format %#x", __func__, supported_formats[0]);
                break;
        }
        str_parms_add_str(reply, AUDIO_PARAMETER_STREAM_SUP_FORMATS, value);
    }
    return ret >= 0;
}

static bool stream_get_parameter_rates(struct str_parms *query, struct str_parms *reply,
                                       uint32_t *supported_sample_rates) {
    int i;
    char value[256];
    int ret = -1;
    DEBUG_FUNC_PRT
    if (str_parms_has_key(query, AUDIO_PARAMETER_STREAM_SUP_SAMPLING_RATES)) {
        ret = 0;
        value[0] = '\0';
        i = 0;
        int cursor = 0;
        while (supported_sample_rates[i]) {
            int avail = sizeof(value) - cursor;
            ret = snprintf(value + cursor, avail, "%s%d", cursor > 0 ? "|" : "",
                           supported_sample_rates[i]);
            if (ret < 0 || ret >= avail) {
                // if cursor is at the last element of the array
                //    overwrite with \0 is duplicate work as
                //    snprintf already put a \0 in place.
                // else
                //    we had space to write the '|' at value[cursor]
                //    (which will be overwritten) or no space to fill
                //    the first element (=> cursor == 0)
                value[cursor] = '\0';
                break;
            }
            cursor += ret;
            ++i;
        }
        str_parms_add_str(reply, AUDIO_PARAMETER_STREAM_SUP_SAMPLING_RATES, value);
    }
    return ret >= 0;
}

static char *out_get_parameters(const struct audio_stream *stream, const char *keys) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    struct str_parms *query = str_parms_create_str(keys);
    struct str_parms *reply = str_parms_create();
    char *str;
    bool replied = false;
    ALOGV("%s: enter: keys - %s", __func__, keys);

    replied |= stream_get_parameter_channels(query, reply, &out->supported_channel_masks[0]);
    replied |= stream_get_parameter_formats(query, reply, &out->supported_formats[0]);
    replied |= stream_get_parameter_rates(query, reply, &out->supported_sample_rates[0]);
    if (replied) {
        str = str_parms_to_str(reply);
    } else {
        str = strdup("");
    }
    str_parms_destroy(query);
    str_parms_destroy(reply);
    ALOGD("%s: exit: returns - %s", __func__, str);
    return str;
}

static uint32_t out_get_latency(const struct audio_stream_out *stream) {
    // DEBUG_FUNC_PRT
    uint32_t latency = 0;
    latency = (pcm_main_config->period_size * PLAYBACK_PERIOD_COUNT * 1000) / pcm_main_config->rate;
    // AALOGV("addr: %s HW latency %d ms", out->bus_address, latency);
    return latency;
}

/* Don't support this function*/
static int out_set_volume(struct audio_stream_out *stream, float left, float right) {
    ALOGV("out_set_volume: Left:%f Right:%f", left, right);
    return 0;
}

static bool filter_bus(struct audio_stream_out *stream) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    if (strstr(out->bus_address, "phone"))
        return false;
    return true;
}

static int32_t start_sw_effect(struct alsa_stream_out *out, const void *buffer, size_t bytes,
                               size_t out_frames) {
    int32_t ret = 0;
    if (bytes <= out->processed_buf_len && out->effect_desc) {
        ret = create_effect(out);
        ALOGV("create_effect ret: %d\n", ret);
        out_effect_enable(out);
        out_effect_config(out, buffer, out->processed_buf, out_frames);
        out_effect_set_param(out);
    }
    return ret;
}

static void *sw_resample_process(struct alsa_stream_out *out, const void *buffer,
                                 size_t *out_frames, size_t *rsp_out_frames) {
    void *buf;
    if (out->resampler) {
        out->resampler->resample_from_input(out->resampler, (int16_t *)buffer, out_frames,
                                            (int16_t *)out->rsp_out_buffer, rsp_out_frames);
        buf = (void *)out->rsp_out_buffer;
        *out_frames = *rsp_out_frames;
    } else {
        // rsp_out_frames = out_frames;
        buf = (void *)buffer;
    }
    return buf;
}

static void *sw_effect_process(struct alsa_stream_out *out, void *buf, size_t out_frames) {
    audio_buffer_t in_buf;
    audio_buffer_t out_buf;
    if (out->effect_desc) {
        out_effect_set_param(out);
        in_buf.raw = (void *)buf;
        in_buf.frameCount = out_frames;
        out_buf.raw = out->processed_buf;
        out_buf.frameCount = out_frames;
        memset(out_buf.raw, 0, out->processed_buf_len);
        (*out->effect_hdl)->process(out->effect_hdl, &in_buf, &out_buf);
    } else {
        out_buf.raw = (void *)buf;
    }
    return out_buf.raw;
}
static void sw_gain_process(struct alsa_stream_out *out, const void *buffer, size_t bytes) {
    struct alsa_audio_device *adev = out->dev;
    if (adev->platform->info.misc.sw_gain) {
        audio_extra_sw_gain_16(out->amplitude_ratio, buffer, bytes);
    }
}
// #define DEBUG_PCM_DATA
static int32_t pcm_write_helper(struct alsa_stream_out *out, void *buffer, size_t bytes) {
#ifdef DEBUG_PCM_DATA
    char test_buf[16 * 1024];
    for (int i = 0; i < bytes; i++) {
        test_buf[i] = (i % 8) + 1;
    }
    buffer = test_buf;
#endif
    sw_gain_process(out, buffer, bytes);

    if (out_is_rear_seat_or_sharing_addr(out->bus_address)) {
        return pcm_write(out->pcm, buffer, bytes);
    } else if (out->pcm_hdl) {
        return hal_streamer_write(out, buffer, bytes);
    } else {
        return pcm_write(out->pcm, buffer, bytes);
    }
}

/* Out put . */
static ssize_t _out_write(struct audio_stream_out *stream, const void *buffer, size_t bytes) {
    int ret = 0;
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    struct alsa_audio_device *adev = out->dev;
    audio_buffer_t out_buf;
    size_t frames_written = 0;
    size_t frame_size = audio_stream_out_frame_size(stream);
    size_t out_frames = bytes / frame_size;
    size_t rsp_out_frames = RESAMPLER_BUFFER_SIZE / frame_size;
    void *buf;

    /* acquiring hw device mutex systematically is useful if a low priority
     * thread is waiting on the output stream mutex - e.g. executing
     * select_mode() while holding the hw device mutex
     */
    // int size = 0;
    // dump_out_data(buffer, bytes, NULL, 1);
    // AALOGV("bus: %s, bytes: %zu, standby: %d", out->bus_address, bytes,
    // out->standby); if (strstr(out->bus_address, "phone"))
    // 	dump_out_data(buffer, bytes, NULL, 1);
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&out->lock);

//    if (!filter_bus(stream)) {
//        pthread_mutex_unlock(&adev->lock);
//        ret = -ENOSYS;
//        goto exit;
//    }
#ifdef ENABLE_AUDIO_SHAREING
    if (!au_sharing_get_state(adev->platform->sharing,
                              out->bus_address))  // sharing
    {
        if (!out->standby) {  // ongoing
            AALOGV("bus: %d, exit", out->bus_num);
            pthread_mutex_unlock(&adev->lock);
            pthread_mutex_unlock(&out->lock);
            out_standby((struct audio_stream *)out);
            usleep(OUT_PERIOD_MS * 1000);
            return bytes;
        } else {  // standby
            AALOGV("bus: %d, exit", out->bus_num);
            pthread_mutex_unlock(&adev->lock);
            frames_written = out_frames;
            usleep(OUT_PERIOD_MS * 1000);
            goto exit;
        }
    }
#endif
    if (out->standby) {
        ALOGV("%s() %d, current state is standby.", __func__, __LINE__);
        ret = start_output_stream(out);
        if (ret != 0) {
            pthread_mutex_unlock(&adev->lock);
            ALOGV("Error out \n");
            goto exit;
        }
        start_sw_effect(out, buffer, bytes, frame_size);
#ifdef ENABLE_AUDIO_SHAREING
        if (out->repeater_src == NULL) {
            au_shr_cfg_t cfg = {
                .pcm_cfg.channels = out->config.channels,
                .stream = out,
            };
            out->repeater_src = au_sharing_get_src(&cfg);
        }
#endif
        out->standby = 0;
    }

    buf = sw_resample_process(out, buffer, &out_frames, &rsp_out_frames);

    out_buf.raw = sw_effect_process(out, buf, out_frames);

    pthread_mutex_unlock(&adev->lock);
    frames_written = out_frames;
    /* we don't support mmap now
     * ret = pcm_mmap_write(out->pcm, buffer, out_frames * frame_size);
     * */
#ifdef ENABLE_AUDIO_SHAREING
    if (out_is_ssa_sharing_bus(out->bus_address)) {
        //AALOGE("ssa write dummy"); //Do nothing
    } else if (out_is_ssb_sharing_bus(out->bus_address)) {
        //AALOGE("ssb write dummy"); //Do nothing
    } else if (out_is_rear_seat_addr(out->bus_address)) {
        if (is_x9u_ref_a()) {
            au_sharing_duplicate(out->repeater_src, out_buf.raw, out_frames * frame_size,
                                 SSA_SRC_SEC_EP, adev->direct);
        } else if (is_x9u_ref_b()) {
            au_sharing_duplicate(
                out->repeater_src, out_buf.raw, out_frames * frame_size,
                out_is_main_seat_addr(out->bus_address) ? SSB_SRC_PRI_EP : SSB_SRC_SEC_EP,
                adev->direct);
        }
    } else {
        if (is_x9u_ref_a()) { /*SSA*/
            au_sharing_duplicate(out->repeater_src, out_buf.raw, out_frames * frame_size,
                                 SSA_SRC_PRI_EP, adev->direct);
        }
    }
#endif
    // hal streamer send pcm data

    pcm_write_helper(out, out_buf.raw, out_frames * frame_size);
    if (ret == 0) {
        out->written += out_frames;
    }
exit:
    pthread_mutex_unlock(&out->lock);
    if (ret != 0) {
        ALOGE("pcm write failed %d", ret);
        usleep((int64_t)bytes * 1000000 / audio_stream_out_frame_size(stream) /
               out_get_sample_rate(&stream->common));
        /* TODO: Need restart */
    }
    if (frames_written < out_frames) {
        ALOGV(
            "Hardware backing HAL too slow, could only write %zu of "
            "%zu frames",
            frames_written, out_frames);
    }
    // AALOGV("out(%s) write done", out->bus_address);
    return bytes;
}

static int out_get_render_position(const struct audio_stream_out *stream, uint32_t *dsp_frames) {
    *dsp_frames = 0;
    DEBUG_ITEM_PRT(dsp_frames);
    return -ENOSYS;
}

static int out_get_presentation_position(const struct audio_stream_out *stream, uint64_t *frames,
                                         struct timespec *timestamp) {
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    int ret = -1;

    size_t kernel_buffer_size;
    unsigned int avail;
    uint64_t signed_frames;
    if (out->pcm && !out->standby) {
        if (pcm_get_htimestamp(out->pcm, &avail, timestamp) == 0) {
            kernel_buffer_size = out->config.period_size * out->config.period_count;
            signed_frames = out->written - kernel_buffer_size + avail;
            if (signed_frames >= 0) {
                *frames = signed_frames;
                ret = 0;
            } else {
                AALOGV("get signed_frames err: %" PRId64 " ", signed_frames);
            }
        } else {
            AALOGV("get pcm_get_htimestamp err!");
        }
    } else {
        AALOGV("get out_get_presentation_position err!");
    }
#if 1
    // FIXME: Temp fake timestamp for mediaplayer timestamp sync issue.
    if (clock_gettime(CLOCK_MONOTONIC, timestamp) >= 0) {
        //
    }
#endif

    //	ALOGV("%s ----> %d %ld.%03d !",
    //__FUNCTION__,__LINE__,timestamp->tv_sec, (int)timestamp->tv_nsec /
    // 1000000);

    return ret;
}

static int out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
    ALOGV("out_add_audio_effect: %p", effect);
    return 0;
}

static int out_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
    ALOGV("out_remove_audio_effect: %p", effect);
    return 0;
}

static int out_get_next_write_timestamp(const struct audio_stream_out *stream, int64_t *timestamp) {
    DEBUG_FUNC_PRT
    *timestamp = 0;
    ALOGV("out_get_next_write_timestamp: %ld", (long int)(*timestamp));
    return -ENOSYS;
}

/** audio_stream_in implementation  only support 48k if input resampler is
 * disable */
static uint32_t in_get_sample_rate(const struct audio_stream *stream) {
    DEBUG_FUNC_PRT
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    struct alsa_audio_device *adev = in->dev;
    if (adev->platform->info.misc.sw_in_src)
        return in->requested_rate;
    else
        return in->config.rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
    DEBUG_ITEM_PRT(rate)
    /*     struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
        in->sample_rate = rate; */
    return -ENOSYS;
}
/* get_input_buffer_size if there is conversation */
static size_t get_input_buffer_size(uint32_t sample_rate, audio_format_t format,
                                    uint32_t channels) {
    size_t size;
    DEBUG_FUNC_PRT
    size = sample_rate * IN_PERIOD_MS / 1000;
    /* Audioflinger expects audio buffers to be multiple of 16 frames */
    size = ((size + 15) / 16) * 16;
    size *= sizeof(short) * channels;
    return size;
}

static size_t in_get_buffer_size(const struct audio_stream *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    /* 	int size = get_input_buffer_size(in->config.rate,
       in->config.format, in->config.channels); */
    size_t size = ((in->config.period_size + 15) / 16) * 16 *
                  audio_stream_in_frame_size((struct audio_stream_in *)stream);
    DEBUG_ITEM_PRT(size)
    return size;
}

static audio_channel_mask_t in_get_channels(const struct audio_stream *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    ALOGV("%s, in->config.channels:%d, channel_mask:%#x", __func__, in->config.channels,
          audio_channel_in_mask_from_count(in->config.channels));
    return audio_channel_in_mask_from_count(in->config.channels);
}

static audio_format_t in_get_format(const struct audio_stream *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    ALOGV("%s, pcm format:%d, audio format:%#x", __func__, in->config.format,
          audio_format_from_pcm_format(in->config.format));
    return audio_format_from_pcm_format(in->config.format);
}

static int in_set_format(struct audio_stream *stream, audio_format_t format) {
    DEBUG_FUNC_PRT
    return -ENOSYS;
}
static int do_in_standby(struct alsa_stream_in *in) {
    struct alsa_audio_device *adev = in->dev;

    if (!in->standby) {
        if (in->pcm_wrapper) {
            if (0 == pcm_wrapper_close(in->pcm_wrapper)) {
                ALOGD("input pcm_wrapper_close success!");
                in->pcm = NULL;
                in->pcm_wrapper = NULL;
            } else {
                ALOGE("input pcm_wrapper_close failed!");
                return -1;
            }
        } else if (in->pcm_hdl) {
            hal_streamer_close((struct audio_stream *)in, PCM_IN);
            in->pcm_hdl = NULL;
            if (adev->platform->remote > 0) {
                audio_remote_stop(adev, HIFI_CAPTURE_FROM_MAIN_MIC_48K);
            }
        }

        if (in->resampler) {
            release_resampler(in->resampler);
            in->resampler = NULL;
        }
        adev->active_input = NULL;
        in->standby = 1;
    }
    return 0;
}
static int in_standby(struct audio_stream *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    int status;

    pthread_mutex_lock(&in->dev->lock);
    pthread_mutex_lock(&in->lock);
    status = do_in_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->dev->lock);
    AALOGI("in_device: %#x close status: %d", in->devices, status);
    return status;
}

static int in_dump(const struct audio_stream *stream, int fd) {
    return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs) {
    DEBUG_FUNC_PRT
    return 0;
}
static int32_t pcm_read_hw(struct alsa_stream_in *in, void *buffer, size_t bytes) {
    int32_t ret = 0;

    if (in->pcm_wrapper) {
        AALOGV("pcm read: %zu bytes", bytes);
        ret = pcm_wrapper_read(in->pcm_wrapper, buffer, bytes);
        // int32_t size = 0;
        // dump_out_data(buffer, bytes, NULL, 2);
    } else if (in->pcm_hdl) {
        AALOGV("pcm read: %zu bytes", bytes);
        ret = hal_streamer_read(in, buffer, bytes);
    } else {
        ret = -EINVAL;
    }
    if (ret < 0) {
        AALOGE("Failed to read w/err %d, (%s)", ret, strerror(ret));
    }
    return ret;
}
static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                           struct resampler_buffer *buffer) {
    struct alsa_stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    in = (struct alsa_stream_in *)((char *)buffer_provider -
                                   offsetof(struct alsa_stream_in, buf_provider));

    if (in->frames_in == 0) {
        in->read_status =
            pcm_read_hw(in, (void *)in->buffer,
                        in->config.period_size * audio_stream_in_frame_size(&in->stream));
        /*in->read_status = pcm_read(in->pcm,
                       (void*)in->buffer,
                       in->config.period_size *
                           audio_stream_in_frame_size(&in->stream));*/
        if (in->read_status != 0) {
            ALOGE("get_next_buffer() pcm_read error %d, %s", in->read_status, strerror(errno));
            buffer->raw = NULL;
            buffer->frame_count = 0;
            return in->read_status;
        }
        in->frames_in = in->config.period_size;
    }

    buffer->frame_count =
        (buffer->frame_count > in->frames_in) ? in->frames_in : buffer->frame_count;
    buffer->i16 = in->buffer + (in->config.period_size - in->frames_in) * in->config.channels;

    return in->read_status;
}

static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                           struct resampler_buffer *buffer) {
    struct alsa_stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return;

    in = (struct alsa_stream_in *)((char *)buffer_provider -
                                   offsetof(struct alsa_stream_in, buf_provider));

    in->frames_in -= buffer->frame_count;
}

/* read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer
 * specified */
static ssize_t read_frames(struct alsa_stream_in *in, void *buffer, ssize_t frames) {
    ssize_t frames_wr = 0;

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;
        if (in->resampler != NULL) {
            in->resampler->resample_from_provider(
                in->resampler,
                (int16_t *)((char *)buffer + frames_wr * audio_stream_in_frame_size(&in->stream)),
                &frames_rd);
        } else {
            struct resampler_buffer buf = {
                {
                    .raw = NULL,
                },
                .frame_count = frames_rd,
            };
            get_next_buffer(&in->buf_provider, &buf);
            if (buf.raw != NULL) {
                memcpy((char *)buffer + frames_wr * audio_stream_in_frame_size(&in->stream),
                       buf.raw, buf.frame_count * audio_stream_in_frame_size(&in->stream));
                frames_rd = buf.frame_count;
            }
            release_buffer(&in->buf_provider, &buf);
        }
        /* in->read_status is updated by getNextBuffer() also called by
         * in->resampler->resample_from_provider() */
        if (in->read_status != 0)
            return in->read_status;

        frames_wr += frames_rd;
    }
    return frames_wr;
}

static char *in_get_parameters(const struct audio_stream *stream, const char *keys) {
    DEBUG_FUNC_PRT
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    struct str_parms *query = str_parms_create_str(keys);
    char *str;
    struct str_parms *reply = str_parms_create();
    bool replied = false;
    ALOGV("%s: enter: keys - %s", __func__, keys);

    replied |= stream_get_parameter_channels(query, reply, &in->supported_channel_masks[0]);
    replied |= stream_get_parameter_formats(query, reply, &in->supported_formats[0]);
    replied |= stream_get_parameter_rates(query, reply, &in->supported_sample_rates[0]);
    if (replied) {
        str = str_parms_to_str(reply);
    } else {
        str = strdup("");
    }
    str_parms_destroy(query);
    str_parms_destroy(reply);
    ALOGI("%s exit, key: %s - value: %s", __func__, keys, str);
    return str;
}

static int in_set_gain(struct audio_stream_in *stream, float gain) {
    return 0;
}

int start_input_stream(struct alsa_stream_in *in) {
    struct pcm_config *config = &default_input_config;
    struct pcm_config in_pcm_config;
    struct alsa_audio_device *adev = in->dev;
    int card = CARD_DEFAULT, port = CODEC_DEV_IN;

    if (adev->mode == AUDIO_MODE_IN_CALL) {
        ALOGD("in call mode , do not start input stream.");
        return 0;
    }

    if (adev->active_output == NULL) {
        ALOGV(" acquirePerfLock for input.\n");
        acquirePerfLock(0, 0);
    }

    adev->active_input = in;
    if (is_x9u_ref_b()) {
        int active_device = get_active_hs_device(adev);
        if (active_device & REAR_SEAT3_HS_STATUS) {
            card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR3);
        } else {
            card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2);
        }
        AALOGI("get card id nearend");
        if (card >= 0) {
            config = &rear_100_cap_cfg_ref;
        } else {
            AALOGE("get card id failed");
            return -ENODEV;
        }
    } else if (AUDIO_DEVICE_IN_WIRED_HEADSET == in->devices) {  // 0x80000010u
        if (is_x9u_ref_b()) {
            int active_device = get_active_hs_device(adev);
            if (active_device & REAR_SEAT3_HS_STATUS) {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR3);

            } else {
                card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2);
            }
        } else {
            card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR1);
        }
        if (card >= 0) {
            config = &rear_100_cap_cfg_ref;
        } else {
            return -ENODEV;
        }

    } else if ((card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
        return -ENODEV;
    }
    port = CODEC_DEV_IN;
#ifdef ENABLE_AUDIO_SHAREING
    in->is_audiosharing_input_node = false;
    if (adev->audio_sharing_fe == true) {
        config = &main_config_ref_virtual;
        if (is_x9u_ref_b()) {
            if ((card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2)) < 0) {
                return -ENODEV;
            }
            AALOGI("get card id farend");
            port = REFB_VI2S_DEV;
            in->is_audiosharing_input_node = true;
        } else {
            if ((card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR1)) < 0) {
                return -ENODEV;
            }
            AALOGI("get card id farend");
            port = REFA_VI2S_DEV;
            in->is_audiosharing_input_node = true;
        }
    }
#endif
    AALOGI("device: %#x, card: %d, port: %d", in->devices, card, port);
#ifdef ENABLE_AUDIO_SHAREING
    if ((AUDIO_DEVICE_IN_WIRED_HEADSET == in->devices) || (adev->audio_sharing_fe == true)) {
        AALOGI("enter pcm warpper");
#else
    if ((AUDIO_DEVICE_IN_WIRED_HEADSET == in->devices)) {  // second card record
#endif
        in->pcm_wrapper = pcm_wrapper_open(card, port, PCM_IN, config);
        in->pcm = pcm_wrapper_get_handler(in->pcm_wrapper);
        if (in->pcm_wrapper) {
            pcm_wrapper_prepare(in->pcm_wrapper);
            pcm_wrapper_start(in->pcm_wrapper);
        }
        if (in->pcm == NULL || !pcm_is_ready(in->pcm)) {
            ALOGE("%s: pcm stream not ready", __func__);
            adev->active_input = NULL;
            if (adev->active_output == NULL) {
                ALOGI("releasePerfLock for output\n \n");
                releasePerfLock(CpuBoostCancelRequest);
            }
            return -ENODEV;
        }
    } else if (is_x9u_ref_b()) {  // main card record
        in->pcm_wrapper = pcm_wrapper_open(card, port, PCM_IN, config);
        in->pcm = pcm_wrapper_get_handler(in->pcm_wrapper);
        if (in->pcm_wrapper) {
            pcm_wrapper_prepare(in->pcm_wrapper);
            pcm_wrapper_start(in->pcm_wrapper);
        }
        if (in->pcm == NULL || !pcm_is_ready(in->pcm)) {
            ALOGE("%s: pcm stream not ready", __func__);
            adev->active_input = NULL;
            if (adev->active_output == NULL) {
                ALOGI("releasePerfLock for output\n \n");
                releasePerfLock(CpuBoostCancelRequest);
            }
            return -ENODEV;
        }
    } else {
        if (adev->platform->remote > 0) {
            audio_remote_start(adev, HIFI_CAPTURE_FROM_MAIN_MIC_48K, 100);
        }
        AALOGI("in %d %d %d", in->config.rate, in->config.channels, in->config.format);
        if (adev->platform->info.misc.ahub_en) {
            memcpy(&in_pcm_config, config, sizeof(in_pcm_config));
            in_pcm_config.rate = in->config.rate;
            in_pcm_config.period_size = (in->config.rate * IN_PERIOD_MS) / 1000;
            in->pcm_hdl = hal_streamer_open((struct audio_stream *)in, card, port, PCM_IN,
                                            &in_pcm_config);
        } else {
            in->pcm_hdl = hal_streamer_open((struct audio_stream *)in, card, port, PCM_IN,
                                            &default_input_config);
        }
        if (in->pcm_hdl == NULL) {
            AALOGE("hal_streamer_open open failed");
            adev->active_input = NULL;
            if (adev->active_output == NULL) {
                ALOGI("releasePerfLock for output\n \n");
                releasePerfLock(CpuBoostCancelRequest);
            }
            return -ENODEV;
        }
    }

    if (adev->platform->info.misc.sw_in_src) {
        int ret = 0;
        /* if no supported sample rate is available, use the resampler
         */
        if (in->requested_rate != in->config.rate) {
            in->buf_provider.get_next_buffer = get_next_buffer;
            in->buf_provider.release_buffer = release_buffer;

            ret = create_resampler(in->config.rate, in->requested_rate, in->config.channels,
                                   RESAMPLER_QUALITY_DEFAULT, &in->buf_provider, &in->resampler);
            if (ret != 0) {
                ALOGE("create in resampler failed, %d -> %d", in->config.rate, in->requested_rate);
                return -EINVAL;
            }

            ALOGI("create in resampler OK, %d -> %d", in->config.rate, in->requested_rate);
        } else
            ALOGI("do not use in resampler");

        if (in->resampler) {
            in->resampler->reset(in->resampler);
            in->frames_in = 0;
        }
    }

    return 0;
}

static int in_stop(const struct audio_stream_in *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    struct alsa_audio_device *adev = in->dev;
    int ret = -ENOSYS;

    DEBUG_FUNC_PRT
    pthread_mutex_lock(&adev->lock);
    /* Do something*/
    if (in->pcm) {
        pcm_close(in->pcm);
        in->pcm = NULL;
    }
    pthread_mutex_unlock(&adev->lock);
    return ret;
}

static int in_start(const struct audio_stream_in *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    struct alsa_audio_device *adev = in->dev;

    int ret = -1;

    DEBUG_FUNC_PRT
    pthread_mutex_lock(&adev->lock);
    /* Do something*/
    if (in->pcm != NULL && !in->standby /*capture_started and MMAP record*/)
        ret = start_input_stream(in);
    pthread_mutex_unlock(&adev->lock);
    return ret;
}

static ssize_t in_read(struct audio_stream_in *stream, void *buffer, size_t bytes) {
    size_t frames_rq = 0;
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    struct alsa_audio_device *adev = in->dev;
    ALOGV("in_read: bytes %zu", bytes);
    int ret;
    /* TODO: here need handle side tone session */

    if (adev->mode == AUDIO_MODE_IN_CALL) {
        usleep(10000);
        return 1;
    }

    pthread_mutex_lock(&in->lock);

    /* Standby session */
    if (in->standby) {
        ret = start_input_stream(in);
        if (ret != 0) {
            goto exit;
        }
        in->standby = false;
    }

    /* place after start_input_stream, because start_input_stream() change frame size */
    frames_rq = bytes / audio_stream_in_frame_size(stream);

    if (in->resampler != NULL) {
        ret = read_frames(in, buffer, frames_rq);
    } else {
        ret = pcm_read_hw(in, buffer, bytes);
    }
    if (ret < 0) {
        AALOGE("Failed to read w/err %d, (%s)", ret, strerror(ret));
    }

    // int size = 0;
    // dump_out_data(buffer, bytes, NULL, 9);
exit:
    pthread_mutex_unlock(&in->lock);
#ifdef ENABLE_AUDIO_SHAREING
    // If audio set
    if (in->reset_stream_input_node == true) {
        AALOGE("AudioSharing Input need reset: %d, (%s)", ret, strerror(ret));
        in_standby((struct audio_stream *)in);
        in->reset_stream_input_node = false;
    }
#endif
    if (ret < 0) {
        AALOGE("Failed to read w/err %d, (%s)---0", ret, strerror(ret));
        in_standby((struct audio_stream *)in);
        // clear return data
        memset(buffer, 0x0, bytes);
        usleep(bytes * 1000000 / audio_stream_in_frame_size(stream) /
               in_get_sample_rate((const struct audio_stream *)in));
    }
    return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream) {
    DEBUG_FUNC_PRT
    return 0;
}

static int in_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
    DEBUG_FUNC_PRT
    return 0;
}

static int in_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
    DEBUG_FUNC_PRT
    return 0;
}
static bool register_uint(uint32_t value, uint32_t *list, size_t list_length) {
    DEBUG_FUNC_PRT
    for (size_t i = 0; i < list_length; i++) {
        if (list[i] == value)
            return true;     // value is already present
        if (list[i] == 0) {  // no values in this slot
            list[i] = value;
            return true;  // value inserted
        }
    }
    return false;  // could not insert value
}
static void register_channel_mask(
    audio_channel_mask_t channel_mask,
    audio_channel_mask_t supported_channel_masks[static MAX_SUPPORTED_CHANNEL_MASKS]) {
    DEBUG_FUNC_PRT
    ALOGE_IF(!register_uint(channel_mask, supported_channel_masks, MAX_SUPPORTED_CHANNEL_MASKS),
             "%s: stream can not declare supporting its channel_mask %x", __func__, channel_mask);
}

static void register_format(audio_format_t format,
                            audio_format_t supported_formats[static MAX_SUPPORTED_FORMATS]) {
    DEBUG_FUNC_PRT

    ALOGE_IF(!register_uint(format, supported_formats, MAX_SUPPORTED_FORMATS),
             "%s: stream can not declare supporting its format %x", __func__, format);
}

static void register_sample_rate(
    uint32_t sample_rate, uint32_t supported_sample_rates[static MAX_SUPPORTED_SAMPLE_RATES]) {
    DEBUG_FUNC_PRT

    ALOGE_IF(!register_uint(sample_rate, supported_sample_rates, MAX_SUPPORTED_SAMPLE_RATES),
             "%s: stream can not declare supporting its sample rate %x", __func__, sample_rate);
}

/* copied from libcutils/str_parms.c */
static bool str_eq(void *key_a, void *key_b) {
    return !strcmp((const char *)key_a, (const char *)key_b);
}

/**
 * use djb hash unless we find it inadequate.
 * copied from libcutils/str_parms.c
 */
#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
static int

str_hash_fn(void *str) {
    uint32_t hash = 5381;
    char *p;
    for (p = str; p && *p; p++) {
        hash = ((hash << 5) + hash) + *p;
    }
    return (int)hash;
}

static void force_all_standby(struct alsa_audio_device *adev) {
    struct alsa_stream_in *in;
    struct alsa_stream_out *out;

    if (adev->active_output) {
        out = adev->active_output;
        /*pthread_mutex_lock(&out->dev->lock);
        pthread_mutex_lock(&out->lock);
        do_output_standby(out);
        pthread_mutex_unlock(&out->lock);
        pthread_mutex_unlock(&out->dev->lock);*/
    }
    if (adev->active_input) {
        in = adev->active_input;
        pthread_mutex_lock(&in->dev->lock);
        pthread_mutex_lock(&in->lock);
        do_in_standby(in);
        pthread_mutex_unlock(&in->lock);
        pthread_mutex_unlock(&in->dev->lock);
    }
}

static void set_btcall_status(struct alsa_audio_device *adev) {
    int i = 0;
    int wait_cnt = 300;

    for (i = 0; i < BTCALL_RT_MAX; i++) {
        if (!adev->bt_data[i]) {
            ALOGI("btcall is not inited.");
            return;
        }
    }

    if (!adev->bt_thread_data) {
        ALOGI("bt call thread is not inited.");
        return;
    }

    if (adev->in_call == 1) {
        for (i = 0; i < wait_cnt; i++) {
            if (!adev->bt_thread_data->is_running_tx && !adev->bt_thread_data->is_running_rx) {
                break;
            }
            usleep(10000);
        }
        usleep(50000);
        AALOGI("open bt call");
        if (btcall_open(adev)) {
            AALOGE("bt call open failed");
            btcall_close(adev);
            return;
        }

        pthread_mutex_lock(&adev->bt_thread_data->lock_rx);
        adev->bt_thread_data->start_work_rx = 1;
        adev->bt_thread_data->exit_work_rx = 0;
        pthread_cond_signal(&adev->bt_thread_data->cond_rx);
        AALOGI("pthread_cond_signal");
        pthread_mutex_unlock(&adev->bt_thread_data->lock_rx);

        pthread_mutex_lock(&adev->bt_thread_data->lock_tx);
        adev->bt_thread_data->start_work_tx = 1;
        adev->bt_thread_data->exit_work_tx = 0;
        pthread_cond_signal(&adev->bt_thread_data->cond_tx);
        AALOGI("pthread_cond_signal2");
        pthread_mutex_unlock(&adev->bt_thread_data->lock_tx);
    } else if (adev->in_call == 0) {
        pthread_mutex_lock(&adev->bt_thread_data->lock_rx);
        //pthread_cond_signal(&adev->bt_thread_data->cond_rx);
        adev->bt_thread_data->start_work_rx = 0;
        adev->bt_thread_data->exit_work_rx = 0;
        ALOGD("pthread_rx waiting...");
        pthread_mutex_unlock(&adev->bt_thread_data->lock_rx);
        pthread_mutex_lock(&adev->bt_thread_data->lock_tx);
        //pthread_cond_signal(&adev->bt_thread_data->cond_tx);
        adev->bt_thread_data->start_work_tx = 0;
        adev->bt_thread_data->exit_work_tx = 0;
        ALOGD("pthread_tx waiting...");
        pthread_mutex_unlock(&adev->bt_thread_data->lock_tx);
        ALOGV("pthread_tx exited");

        for (i = 0; i < wait_cnt; i++) {
            if (!adev->bt_thread_data->is_running_tx && !adev->bt_thread_data->is_running_rx) {
                break;
            }
            usleep(10000);
        }
        if (i >= wait_cnt) {
            ALOGE("waitting for bt thread stop failed!");
        }
        ALOGD("%d ms waitted for bt thread stop.", 10 * i);
        if((adev->bt_thread_data->start_work_rx == 1) || (adev->bt_thread_data->start_work_tx == 1))
        {
            ALOGD("next call have started");
            return;
        }
        btcall_close(adev);
        if (adev->ecnr_ops && adev->ecnr_hdl) {
            if (adev->ecnr_ops->ecnr_deinit(adev->ecnr_hdl)) {
                adev->ecnr_hdl = NULL;
            }
        }
    } else {
        ALOGE("unknown in call status!!");
    }
}

static void select_mode(struct alsa_audio_device *adev) {
    if (adev->mode == AUDIO_MODE_IN_CALL) {
        AALOGI("Entering IN_CALL mode, in_call=%d", adev->in_call);
        if (!adev->in_call) {
            force_all_standby(adev);
            // select_output_device(adev);
            adev->in_call = 1;
            AALOGI("begin btcall status set");
            set_btcall_status(adev);
        }
    } else {
        AALOGI("Leaving IN_CALL mode, in_call=%d, mode=%d", adev->in_call, adev->mode);
        if (adev->in_call) {
            adev->in_call = 0;
            // force_all_standby(adev);
            // select_output_device(adev);
            // select_input_device(adev);
            set_btcall_status(adev);
        }
    }
}

static int adev_open_output_stream(struct audio_hw_device *dev, audio_io_handle_t handle,
                                   audio_devices_t devices, audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out, const char *address) {
    ALOGI(
        "%s: enter: format(%s) sample_rate(%d) channel_mask(%#x) "
        "devices(%#x) flags(%#x) addr(%p: %s)",
        __func__, audio_extra_get_android_format_name(config->format), config->sample_rate,
        config->channel_mask, devices, flags, address, address);
    struct alsa_audio_device *ladev = (struct alsa_audio_device *)dev;
    struct alsa_stream_out *out;
    struct pcm_params *params;
    int ret = 0;
    int card = CARD_DEFAULT, port = CODEC_DEV;
    bool is_rear_seat = out_is_rear_seat_or_sharing_addr(address);

    if (!is_rear_seat) {
        if ((card = sd_platform_get_card_id(ladev->platform, AUDIO_NAME_PRIMARY)) >= 0) {
            port = CODEC_DEV;
            params = pcm_params_get(card, port, PCM_OUT);
            if (!params)
                return -ENOSYS;
        } else {
            AALOGE("get card id failed");
        }
    }

    if (!sd_platform_check_output(config, NULL))
        return -EINVAL;

    out = (struct alsa_stream_out *)calloc(1, sizeof(struct alsa_stream_out));
    if (!out)
        return -ENOMEM;

    out->rsp_out_buffer = malloc(RESAMPLER_BUFFER_SIZE); /* todo: allow for reallocing */
    if (!out->rsp_out_buffer) {
        ret = -ENOMEM;
        goto err_exit;
    }

    memset(out->rsp_out_buffer, 0x00, RESAMPLER_BUFFER_SIZE);

    out->stream.common.get_sample_rate = out_get_sample_rate;
    out->stream.common.set_sample_rate = out_set_sample_rate;
    out->stream.common.get_buffer_size = out_get_buffer_size;
    out->stream.common.get_channels = out_get_channels;
    out->stream.common.get_format = out_get_format;
    out->stream.common.set_format = out_set_format;
    out->stream.common.standby = out_standby;
    out->stream.common.dump = out_dump;
    out->stream.common.set_parameters = out_set_parameters;
    out->stream.common.get_parameters = out_get_parameters;
    out->stream.common.add_audio_effect = out_add_audio_effect;
    out->stream.common.remove_audio_effect = out_remove_audio_effect;
    out->stream.get_latency = out_get_latency;
    out->stream.set_volume = out_set_volume;
    out->stream.get_render_position = out_get_render_position;
    out->stream.get_next_write_timestamp = out_get_next_write_timestamp;
    out->stream.get_presentation_position = out_get_presentation_position;
    out->stream.write = _out_write;

    out->bus_num = atoi(address + 3);
    /* ALSA configration*/
    switch (out->bus_num) {
        case BUS100: /* rear seat */
        case BUS101:
        case BUS102:
        case BUS103:
            out->config.channels = CHANNEL_STEREO;
            out->config.rate = HIFI_SAMPLING_RATE;
            break;
        case BUS0: /* media */
        case BUS1: /* voice_command_out */
        case BUS3: /* navi */
        case BUS4: /* call_ring */
        case BUS5: /* alarm */
        case BUS6: /* notification */
        case BUS7: /* system_sound */
        case BUS9: /* sharing */
            out->config.channels = audio_channel_count_from_out_mask(config->channel_mask);
            out->config.rate = config->sample_rate;
            break;
        case BUS2: /* phone */
            //out->config.channels = CHANNEL_MONO;
            out->config.channels = audio_channel_count_from_out_mask(config->channel_mask);
            out->config.rate = config->sample_rate;
            break;
        case BUS8: /*bt call private*/
            AALOGE("bus num 8 is btcall private , not support for application");
            ret = -EINVAL;
            goto err_exit;
        default:
            out->config.rate = config->sample_rate;
            break;
    }

    out->config.format = pcm_format_from_audio_format(config->format);
    out->config.period_size = (out->config.rate * OUT_PERIOD_MS) / 1000;
    out->config.period_count = PLAYBACK_PERIOD_COUNT;

    if (is_rear_seat &&
        (out->config.rate != config->sample_rate ||
         audio_channel_count_from_out_mask(config->channel_mask) != CHANNEL_STEREO ||
         out->config.format != pcm_format_from_audio_format(config->format))) {
        config->sample_rate = out->config.rate;
        config->format = audio_format_from_pcm_format(out->config.format);
        config->channel_mask = audio_channel_out_mask_from_count(CHANNEL_STEREO);
        ret = -EINVAL;
    } else if (!strcmp(address, "")) {
        ALOGW("address is non-value, not car usecase!!!");
        out->config.rate = config->sample_rate;
        out->config.channels = audio_channel_count_from_out_mask(config->channel_mask);
        out->config.format = pcm_format_from_audio_format(config->format);
    }

    ALOGI("adev_open_output_stream :: select channels=%d rate=%d format=%s", out->config.channels,
          out->config.rate, audio_extra_get_alsa_format_name(out->config.format));

    out->dev = ladev;
    out->standby = true;
    if (address && strcmp(address, "")) {
        out->bus_address = calloc(strlen(address) + 1, sizeof(char));
        // strncpy(out->bus_address, address, strlen(address));
        strncpy((char *)out->bus_address, address, strlen(address));
        hashmapPut(ladev->out_bus_stream_map, (void *)out->bus_address, out);
        out->amplitude_ratio = 1.0;
        out->gain_stage = (struct audio_gain){
            .min_value = -4800,
            .max_value = 6900,
            .step_value = 300,
        };
    }

    /* Set output stream configration */
    config->format = out_get_format(&out->stream.common);
    config->channel_mask = out_get_channels(&out->stream.common);
    config->sample_rate = out_get_sample_rate(&out->stream.common);

    *stream_out = &out->stream;

    if (!strcmp(address, STR_DEV_BUS2_ADDR)) {
        if (ladev->bt_data[BTCALL_CODEC_OUTPUT]) {
            ladev->bt_data[BTCALL_CODEC_OUTPUT]->out = out;
        }
        out->processed_buf = NULL;
        return ret;
    }

    out->processed_buf_len = audio_bytes_per_sample(out->config.format) * out->config.channels *
                             (out->config.rate * OUT_MAX_PROCESSED_BUF_PERIOD /*ms*/ / 1000);
    if (out->processed_buf_len)
        out->processed_buf = malloc(out->processed_buf_len);

    register_format(config->format, out->supported_formats);
    register_channel_mask(config->channel_mask, out->supported_channel_masks);
    register_sample_rate(out->config.rate, out->supported_sample_rates);

    sd_effect_init((struct audio_stream *)out, STREAM_OUT_EFFECT, default_effect_lib);

    /* TODO The retry mechanism isn't implemented in
     * AudioPolicyManager/AudioFlinger. */
    return ret;

err_exit:
    if (out)
        free(out);

    return ret;
}

static void adev_close_output_stream(struct audio_hw_device *dev, struct audio_stream_out *stream) {
    DEBUG_FUNC_PRT
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;

    pthread_mutex_lock(&out->lock);
    do_output_standby(out);
    pthread_mutex_unlock(&out->lock);

    if (out && out->processed_buf)
        free(out->processed_buf);

    if (out && out->rsp_out_buffer)
        free(out->rsp_out_buffer);

    if (out->resampler)
        release_resampler(out->resampler);

    if (out->bus_address) {
        hashmapRemove(adev->out_bus_stream_map, (void *)out->bus_address);
        free((void *)out->bus_address);
    }
    free(stream);
}

static int adev_handle_connect(struct alsa_audio_device *adev, bool connect) {
    int ret = 0;
    int card_id = -1;

    if (is_x9u_ref_b()) {
        if (connect) {
            adev->hs_device = get_hs_connect_status();
        }
        ALOGD("adev_handle_connect hs device = %d", adev->hs_device);

        if (adev->hs_device & REAR_SEAT2_HS_STATUS) {
            card_id = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2);
        } else if (adev->hs_device & REAR_SEAT3_HS_STATUS) {
            card_id = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR3);
        }
    } else {
        card_id = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR1);
    }

    ALOGD("adev_handle_connect card id = %d", card_id);

    if (card_id < 0) {
        return -1;
    }

    if (connect) {
        ret = audio_route_apply_and_update_path(audio_extra_get_audio_route(adev, card_id),
                                                "bus100-headset");
        if (ret < 0) {
            ALOGE("apply headset ctrl failed, errorno:%d", ret);
            return ret;
        }
    } else {
        adev->hs_device = 0;
        ret = audio_route_reset_and_update_path(audio_extra_get_audio_route(adev, card_id),
                                                "bus100-headset");
        if (ret < 0) {
            ALOGE("reset headset ctrl failed, errorno:%d", ret);
            return ret;
        }
    }

    return ret;
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs) {
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;
    struct str_parms *parms;
    char value[32];
    int ret = -ENOSYS;
    bool replied = false;
    int32_t mode = AUDIO_MODE_NORMAL;

    ALOGD("%s, param : %s", __func__, kvpairs);
    if (!strcmp(kvpairs, "")) {
        ALOGW("kvpairs is non value , do nothing");
        return 0;
    }
    parms = str_parms_create_str(kvpairs);
#if 0
    //bus duck
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_DUCK_BUS_NUM, value, sizeof(value));
    if (ret >= 0)
    {
        ALOGD("value:%s", value);
        char* bus_ptr = strstr(value, "]") + 1;
        ALOGD("bus_ptr:%s ", bus_ptr);

        int i = 0;
        int bus_num = 0;
        int numbers[8]={0,0,0,0,0,0,0,0};

        char *token = strtok(bus_ptr, "[] ,");
        ALOGD("token:%s ",token);
        while (token != NULL && i < 8)
        {
            // 使用 atoi 函数将字符串转换为整数
            ALOGD("i:%d token:%s ",i,token);
            bus_num = atoi(token);
            numbers[bus_num] = 1;
            i++;
            token = strtok(NULL, "[] ,");
        }

        i = 0;
        char focus[20];
        strncpy(focus, value, bus_ptr - value);
        token = strtok(focus, "[] ,");
        ALOGD("token:%s ",token);
        for(i = 0; i < 4; i++)
        {
            current_audio_bus_status.focus_bus[i] = -1;
        }

        i = 0;
        while (token != NULL && i < 4)
        {
            // 使用 atoi 函数将字符串转换为整数
            ALOGD("i:%d token:%s ",i,token);
            current_audio_bus_status.focus_bus[i] = atoi(token);
            i++;
            token = strtok(NULL, "[] ,");
        }

        for (int j = 0; j < 8; j++)
        {
            switch (j)
            {
                case BUS0:
                case BUS1:
                case BUS2:
                case BUS3:
                case BUS6:
                {
                    ALOGD("numbers%d = %d ", j, numbers[j]);
                    if(numbers[j] == 1)
                    {
                        set_device_address_is_ducked(j, 1);
                    }
                    else
                    {
                        set_device_address_is_ducked(j, 0);
                    }
                    break;
                }
                default:
                    break;
            }
        }
        replied |= true;
    }
    //bus mute
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_MUTE_BUS_NUM, value, sizeof(value));
    if (ret >= 0)
    {
        ALOGD("kvpairs:%s", kvpairs);
        ALOGD("value:%s", value);
        char dst_str[32];
        int bus_mute_values[8]={0};
        int i = 0;
        int bus_num = 0;
        bool mute_value = false;
        strcpy(dst_str, kvpairs);

        char* bus_mute_ptr = strstr(kvpairs, "bus_mute");
        if (bus_mute_ptr != NULL)
        {
            char* start_ptr = strchr(bus_mute_ptr, '[') + 1;
            char* end_ptr = strchr(bus_mute_ptr, ']');
            char* token = strtok(start_ptr, ",");

            while (token != NULL && i < 8 && token < end_ptr)
            {
                bus_num = atoi(token);
                bus_mute_values[bus_num] = 1;
                i++;
                token = strtok(NULL, ",");
            }
        }

        char* mute_ptr = strstr(dst_str, ",mute");
        if (mute_ptr != NULL)
        {
            char* value_ptr = strchr(mute_ptr, '=') + 1;
            mute_value = (bool)atoi(value_ptr);
            ALOGD("value_ptr values: %s\n",value_ptr);
        }

        for (i = 0; i < 8; i++)
        {
            switch (i)
            {
                case BUS0:
                case BUS1:
                case BUS2:
                case BUS3:
                case BUS6:
                {
                    ALOGD("bus_mute_values: %d = %d ", i, bus_mute_values[i]);
                    if(bus_mute_values[i] == 1)
                    {
                        set_device_address_is_muted(i, mute_value);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        replied |= true;
    }
#endif
    // Samp rate
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_HFP_SET_SAMPLING_RATE, value, sizeof(value));
    if (ret >= 0) {
        adev->hfp_sample_rate = atoi(value);
        pthread_mutex_lock(&adev->lock);
        ALOGV("hfp_set_sampling_rate: %d", adev->hfp_sample_rate);
        pthread_mutex_unlock(&adev->lock);
        replied |= true;
    }

    // HFP Enable
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_HFP_ENABLE, value, sizeof(value));
    if (ret >= 0) {
        if (adev->platform->bt_type == BT_CONNECT_WITH_DSP) {
            if (!strcmp(value, "true")) {
                mode = AUDIO_MODE_IN_CALL;
                ret = start_hfp(adev);
            } else if (!strcmp(value, "false")) {
                mode = AUDIO_MODE_NORMAL;
                ret = stop_hfp(adev);
            }
        }
        if (adev->platform->bt_type == BT_CONNECT_WITH_SOC) {
            if (!strcmp(value, "true")) {
                mode = AUDIO_MODE_IN_CALL;
            } else if (!strcmp(value, "false")) {
                mode = AUDIO_MODE_NORMAL;
            }
            if (adev->mode != mode) {
                pthread_mutex_lock(&adev->lock);
                AALOGI("select mode (%d -> %d)", adev->mode, mode);
                adev->mode = mode;
                pthread_mutex_unlock(&adev->lock);
                select_mode(adev);
            }
        }
        replied |= true;
        ALOGV("hfp ret: %d", ret);
    }

    // HFP Volume
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_HFP_VOLUME, value, sizeof(value));
    if (ret >= 0) {
        pthread_mutex_lock(&adev->lock);
        adev->hfp_volume = atoi(value);
        // hfp_set_volume(adev);
        pthread_mutex_unlock(&adev->lock);
        replied |= true;
        ALOGV("hfp_volume: %d", adev->hfp_volume);
    }

    // headset connect
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DEVICE_CONNECT, value, sizeof(value));
    if (ret >= 0) {
        pthread_mutex_lock(&adev->lock);
        audio_devices_t device = (audio_devices_t)strtoul(value, NULL, 10);
        AALOGI("device: %#x", device);

        if (AUDIO_DEVICE_IN_WIRED_HEADSET == device) {
            ALOGI("Headset Connect");
            adev_handle_connect(adev, true);
        } else {
            replied |= false;
        }
        replied |= true;
        pthread_mutex_unlock(&adev->lock);
    }

    // headset disconnect
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_DEVICE_DISCONNECT, value, sizeof(value));
    if (ret >= 0) {
        pthread_mutex_lock(&adev->lock);
        audio_devices_t device = (audio_devices_t)strtoul(value, NULL, 10);
        AALOGI("device: %x", device);

        if (AUDIO_DEVICE_IN_WIRED_HEADSET == device) {
            ALOGI("Headset Disconnect");
            adev_handle_connect(adev, false);
            replied |= true;
        }
        pthread_mutex_unlock(&adev->lock);
    }
#ifdef ENABLE_AUDIO_SHAREING
    // audio sharing nearend, Only X9U support this function
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_AUDIO_SHARING_NE, value, sizeof(value));
    if (ret >= 0) {
        pthread_mutex_lock(&adev->lock);
        if (!strcmp(value, "false")) {
            au_sharing_disable(AUDIO_SHARING_NE, adev);
            adev->direct = MIRROR_CLOSE;
            replied |= true;
        } else if (!strcmp(value, AUDIO_PARAMETER_AUDIO_SHARING_PRI_SHARING)) {
            au_sharing_enable(AUDIO_SHARING_NE, adev);
            adev->direct = MIRROR_PRI2VDEV;  // ssa_1-->ssb, ssb1-->ssa
            replied |= true;
        } else if (!strcmp(value, AUDIO_PARAMETER_AUDIO_SHARING_SEC_SHARING)) {
            au_sharing_enable(AUDIO_SHARING_NE, adev);
            adev->direct = MIRROR_SEC2VDEV;
            replied |= true;
        } else if (!strcmp(value, AUDIO_PARAMETER_AUDIO_SHARING_SEC_TO_PRI)) {
            adev->direct = MIRROR_SEC2PRI;
            replied |= true;
        } else if (!strcmp(value, AUDIO_PARAMETER_AUDIO_SHARING_PRI_TO_SEC)) {
            adev->direct = MIRROR_PRI2SEC;
            replied |= true;
        }
        pthread_mutex_unlock(&adev->lock);
        AALOGI("after unlock adev lock");
        ALOGI("sharing nearend setting, adev->direct: %d", adev->direct);
    }
    // audio sharing farend , Only X9U support this function

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_AUDIO_SHARING_FE, value, sizeof(value));
    if (ret >= 0) {
        pthread_mutex_lock(&adev->lock);
        AALOGI("after unlock adev lock");
        if (!strcmp(value, "false")) {
            au_sharing_disable(AUDIO_SHARING_FE, adev);
            adev->direct = MIRROR_CLOSE;
            if (adev->active_input)
                adev->active_input->reset_stream_input_node = false;
            replied |= true;
        } else if (!strcmp(value, AUDIO_PARAMETER_AUDIO_SHARING_PRI_SHARING)) {
            au_sharing_enable(AUDIO_SHARING_FE, adev);
            // adev->direct = MIRROR_AFE2ANE;
            if (adev->active_input) {
                // if already have active input need reset input stream.
                if (adev->active_input->is_audiosharing_input_node == false) {
                    ALOGE("AudioSharing Input need reset for AUDIO_SHARING_FE");
                    adev->active_input->reset_stream_input_node = true;
                }
            }
            replied |= true;
        } else if (!strcmp(value, AUDIO_PARAMETER_AUDIO_SHARING_SEC_SHARING)) {
            au_sharing_enable(AUDIO_SHARING_FE, adev);
            // if already have active input need reset input stream.
            // need add lock
            if (adev->active_input) {
                // if already have active input need reset input stream.
                if (adev->active_input->is_audiosharing_input_node == false) {
                    ALOGE("AudioSharing Input need reset for AUDIO_SHARING_FE");
                    adev->active_input->reset_stream_input_node = true;
                }
            }
            replied |= true;
        }
        pthread_mutex_unlock(&adev->lock);

        ALOGI("sharing farend setting, adev->direct: %d", adev->direct);
    }
#endif
    str_parms_destroy(parms);
    if (!replied) {
        ALOGE("Unsupport params: %s", kvpairs);
        ret = -ENOSYS;
    } else {
        ALOGI("%s(), %s successed", __func__, kvpairs);
        ret = 0;
    }

    return ret;
}

static char *adev_get_parameters(const struct audio_hw_device *dev, const char *keys) {
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;
    struct str_parms *reply = str_parms_create();
    struct str_parms *query = str_parms_create_str(keys);
    char *str = NULL;
    char value[128];
    int ret = 0;
    ALOGV("%s() %d: keys:%s", __func__, __LINE__, keys);

    if (!query)
        return NULL;

    pthread_mutex_lock(&adev->lock);
    ret = str_parms_get_str(query, AUDIO_PARAMETER_STREAM_HW_AV_SYNC, value, sizeof(value));
    if (ret >= 0) {
        str_parms_add_int(reply, AUDIO_PARAMETER_STREAM_HW_AV_SYNC, 0);
        str = str_parms_to_str(reply);
    } else {
        str = strdup("");
    }
    if (!strcmp(keys, "active_hs_device")) {
        if (is_x9u_ref_b()) {
            int device = get_active_hs_device(adev);
            if (device == REAR_SEAT2_HS_STATUS) {
                str = strdup("rear_seat2");
            } else if (device == REAR_SEAT3_HS_STATUS) {
                str = strdup("rear_seat3");
            } else {
                str = strdup("");
            }
        } else {
            str = strdup("");
        }
    }
    str_parms_destroy(query);
    str_parms_destroy(reply);

    pthread_mutex_unlock(&adev->lock);
    ALOGI("%s exit, keys: %s - value: %s", __func__, keys, str);
    return str;
}

static int adev_init_check(const struct audio_hw_device *dev) {
    DEBUG_FUNC_PRT
    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume) {
    ALOGV("adev_set_voice_volume: %f", volume);
    return 0;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume) {
    ALOGV("adev_set_master_volume: %f", volume);
    return -ENOSYS;
}

static int adev_get_master_volume(struct audio_hw_device *dev, float *volume) {
    ALOGV("adev_get_master_volume: %f", *volume);
    return -ENOSYS;
}

static int adev_set_master_mute(struct audio_hw_device *dev, bool muted) {
    ALOGV("adev_set_master_mute: %d", muted);
    return -ENOSYS;
}

static int adev_get_master_mute(struct audio_hw_device *dev, bool *muted) {
    ALOGV("adev_get_master_mute: %d", *muted);
    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode) {
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;
    AALOGI("set %d, cur mode: %d", mode, adev->mode);
    for (int i = 0; i < BTCALL_RT_MAX; i++) {
        if (!adev->bt_data[i]) {
            AALOGI("btcall is not inited.");
            return 0;
        }
    }

    if (!adev->bt_thread_data) {
        AALOGI("bt call thread is not inited.");
        return 0;
    }
    if (mode > AUDIO_MODE_IN_COMMUNICATION || mode < AUDIO_MODE_INVALID) {
        AALOGE("mode :%d is not valid", mode);
        return -EINVAL;
    }
    if (adev->mode != mode) {
        if (BT_CONNECT_WITH_SOC == adev->platform->bt_type) {
            pthread_mutex_lock(&adev->lock);
            AALOGI("select mode");
            adev->mode = mode;
            pthread_mutex_unlock(&adev->lock);
            select_mode(adev);
        }
    }
    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state) {
    int ret = 0;
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;

    if (!adev)
        return -EINVAL;
    ALOGI("adev_set_mic_mute: %d", state);
    adev->mic_mute = state;
    ret = hfp_set_mic_mute(adev, state);
    if (ret < 0) {
        ALOGE("set mic mute failed : %d", ret);
    }
    return ret;
}

static int adev_get_microphones(const struct audio_hw_device *dev,
                                struct audio_microphone_characteristic_t *mic_array,
                                size_t *mic_count) {
    DEBUG_FUNC_PRT

    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;

    pthread_mutex_lock(&adev->lock);
    int ret = sd_platform_get_microphones(adev->platform, mic_array, mic_count);
    pthread_mutex_unlock(&adev->lock);
    ALOGV("%s() mic_count: %zu", __func__, *mic_count);
    return ret;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state) {
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;

    if (!adev)
        return -EINVAL;
    ALOGD("%s, mic_mute: %d", __func__, adev->mic_mute);
    *state = adev->mic_mute;
    return 0;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config) {
    // size_t size = get_input_buffer_size(config->sample_rate,
    // config->format, config->channel_mask);
    int32_t ch = audio_channel_count_from_in_mask(config->channel_mask);

    size_t size = get_input_buffer_size(config->sample_rate, config->format,
                                        audio_channel_count_from_in_mask(config->channel_mask));
    ALOGV("adev_get_input_buffer_size: %zu, ch: %d, ch_mask: %#x", size, ch, config->channel_mask);
    return size;
}

static int adev_open_input_stream(struct audio_hw_device __unused *dev, audio_io_handle_t handle,
                                  audio_devices_t devices, struct audio_config *config,
                                  struct audio_stream_in **stream_in,
                                  audio_input_flags_t flags __unused, const char *address __unused,
                                  audio_source_t source __unused) {
    struct alsa_audio_device *ladev = (struct alsa_audio_device *)dev;
    struct alsa_stream_in *in;
    int ret = 0;
    ALOGI(
        "%s: enter: format(%#x) sample_rate(%d) channel_mask(%#x) "
        "devices(%#x) flags(%#x) addr(%s)",
        __func__, config->format, config->sample_rate, config->channel_mask, devices, flags,
        address);

    if (!sd_platform_check_input(config, NULL))
        return -EINVAL;

    in = (struct alsa_stream_in *)calloc(1, sizeof(struct alsa_stream_in));
    if (!in)
        return -ENOMEM;

    in->stream.common.get_sample_rate = in_get_sample_rate;
    in->stream.common.set_sample_rate = in_set_sample_rate;
    in->stream.common.get_buffer_size = in_get_buffer_size;
    in->stream.common.get_channels = in_get_channels;
    in->stream.common.get_format = in_get_format;
    in->stream.common.set_format = in_set_format;
    in->stream.common.standby = in_standby;
    in->stream.common.dump = in_dump;
    in->stream.common.set_parameters = in_set_parameters;
    in->stream.common.get_parameters = in_get_parameters;
    in->stream.common.add_audio_effect = in_add_audio_effect;
    in->stream.common.remove_audio_effect = in_remove_audio_effect;
    in->stream.set_gain = in_set_gain;
    in->stream.read = in_read;
    in->stream.start = in_start;
    in->stream.stop = in_stop;
    in->stream.get_input_frames_lost = in_get_input_frames_lost;

    in->devices = devices;
    in->config.channels = audio_channel_count_from_in_mask(config->channel_mask);
    if (!ladev->platform->info.misc.sw_in_src) {
        in->config.rate = config->sample_rate;
    } else {
        in->config.rate = HIFI_SAMPLING_RATE;
    }
    in->config.format = pcm_format_from_audio_format(config->format);
    in->config.period_size = (in->config.rate * IN_PERIOD_MS) / 1000;
    in->config.period_count = CAPTURE_PERIOD_COUNT;
    in->requested_rate = config->sample_rate;

    in->buffer = malloc(in->config.period_size * audio_stream_in_frame_size(&in->stream) * 8);

    if (!in->buffer) {
        return -ENOMEM;
    }

    ALOGI("Capture sample: %u ch, %u hz, %u bit\n", in->config.channels, in->config.rate,
          in->config.format);
    ALOGI("Config Capture sample: %u frame, %u hz, %u bit\n", config->frame_count,
          config->sample_rate, config->format);
    *stream_in = &in->stream;
    in->dev = ladev;
    in->source = source;
    in->standby = true;
    in->capture_handle = handle;
    in->flags = flags;
    in->pcm = NULL;
    in->pcm_hdl = NULL;
    register_format(config->format, in->supported_formats);
    register_channel_mask(config->channel_mask, in->supported_channel_masks);
    register_sample_rate(in->config.rate, in->supported_sample_rates);
    return ret;
}

static void adev_close_input_stream(struct audio_hw_device *dev, struct audio_stream_in *stream) {
    DEBUG_FUNC_PRT
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    if (in->buffer) {
        free(in->buffer);
        in->buffer = 0;
    }
    in_standby(&stream->common);

    free(stream);
    return;
}

static int adev_dump(const audio_hw_device_t *device, int fd) {
    DEBUG_FUNC_PRT
    return 0;
}
static int adev_create_audio_patch(struct audio_hw_device *dev, unsigned int num_sources,
                                   const struct audio_port_config *sources, unsigned int num_sinks,
                                   const struct audio_port_config *sinks,
                                   audio_patch_handle_t *handle) {
    DEBUG_FUNC_PRT
    struct alsa_audio_device *audio_dev = (struct alsa_audio_device *)dev;
    for (int i = 0; i < num_sources; i++) {
        ALOGD("%s: source[%d] type=%d address=%s", __func__, i, sources[i].type,
              sources[i].type == AUDIO_PORT_TYPE_DEVICE ? sources[i].ext.device.address : "");
    }
    for (int i = 0; i < num_sinks; i++) {
        ALOGD("%s: sink[%d] type=%d address=%s", __func__, i, sinks[i].type,
              sinks[i].type == AUDIO_PORT_TYPE_DEVICE ? sinks[i].ext.device.address : "N/A");
    }
    if (num_sources == 1 && num_sinks == 1 && sources[0].type == AUDIO_PORT_TYPE_DEVICE &&
        sinks[0].type == AUDIO_PORT_TYPE_DEVICE) {
        pthread_mutex_lock(&audio_dev->lock);
        audio_dev->last_patch_id += 1;
        pthread_mutex_unlock(&audio_dev->lock);
        *handle = audio_dev->last_patch_id;
        ALOGD("%s: handle: %d", __func__, *handle);
    }
    return 0;
}

static int adev_release_audio_patch(struct audio_hw_device *dev, audio_patch_handle_t handle) {
    DEBUG_FUNC_PRT;
    return 0;
}

static int adev_get_audio_port(struct audio_hw_device *dev, struct audio_port *port)
{
    AALOGI("name: %s, id: 0x%08x, role: 0x%08x, type: 0x%08x, address: %s", port->name,
           port->id, port->role, port->type, port->ext.device.address);
    return 0;
}

static int adev_set_audio_port_config(struct audio_hw_device *dev,
                                      const struct audio_port_config *config) {
    int ret = 0;
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;
    const char *bus_address = config->ext.device.address;
    struct alsa_stream_out *out = hashmapGet(adev->out_bus_stream_map, (void *)bus_address);
#if 0
    if (out && !strcmp(out->bus_address, STR_DEV_BUS2_ADDR)) {
        pthread_mutex_lock(&out->lock);
        ALOGD("%s, hfp volume: %d", __func__, config->gain.values[0]);
        ret = hfp_set_volume(adev, config->gain.values[0]);
        pthread_mutex_unlock(&out->lock);
    } else if (out && (!strcmp(out->bus_address, STR_DEV_BUS1_ADDR) ||
                       !strncmp(out->bus_address, "bus", 3) ||
                       !strcmp(out->bus_address, STR_DEV_BUS100_ADDR))) {
#else
    if (out && (!strcmp(out->bus_address, STR_DEV_BUS1_ADDR)
            || !strcmp(out->bus_address, STR_DEV_BUS2_ADDR)
            || !strcmp(out->bus_address, STR_DEV_BUS3_ADDR)
            || !strcmp(out->bus_address, STR_DEV_BUS4_ADDR)
            || !strcmp(out->bus_address, STR_DEV_BUS5_ADDR)
            || !strcmp(out->bus_address, STR_DEV_BUS7_ADDR)
            || !strcmp(out->bus_address, STR_DEV_BUS100_ADDR))) {
#endif
        pthread_mutex_lock(&out->lock);
        int gainIndex =
            (config->gain.values[0] - out->gain_stage.min_value) / out->gain_stage.step_value;
        int totalSteps =
            (out->gain_stage.max_value - out->gain_stage.min_value) / out->gain_stage.step_value;
        int minDb = out->gain_stage.min_value / 100;
        int maxDb = out->gain_stage.max_value / 100;
        // curve: 10^((minDb + (maxDb - minDb) * gainIndex / totalSteps) / 20)
        out->amplitude_ratio =
            pow(10, (minDb + (maxDb - minDb) * (gainIndex / (float)totalSteps)) / 20);
        out->target_gain_mb = config->gain.values[0];
        pthread_mutex_unlock(&out->lock);
        ALOGD("%s: set audio gain: %f(%d) on %s steps %d:%d", __func__, out->amplitude_ratio,
              config->gain.values[0], bus_address, gainIndex, totalSteps);

        current_audio_bus_status.volume[out->bus_num] = gainIndex;
        if (current_audio_bus_status.bus_active[out->bus_num] == 1) {
            ret = set_bus_volume(out->bus_num);
            if(ret) {
                ALOGE("set_bus_volume failed! --- %d", ret);
            }
        }
        ALOGD("bus: %d volume: %d, bus_active:%d", out->bus_num, current_audio_bus_status.volume[out->bus_num], current_audio_bus_status.bus_active[out->bus_num]);
    } else {
        ALOGE("%s: can not find output stream by bus_address:%s", __func__, bus_address);
        ret = -EINVAL;
    }
    return ret;
}

/* Set audio port param configuration */
static int adev_set_audio_param_config(struct audio_hw_device *dev,
                           const struct audio_param_config *config)
{
    int ret = 0;
#if 0
    struct alsa_audio_device *adev = (struct alsa_audio_device *)dev;

    switch (config->command) {
    case AUDIO_COMMAND_TYPE_GEQ:
    {
        switch (config->mode.equalizer.type)
        {
            ALOGI("AUDIO_COMMAND_TYPE_GEQ:%d\n", config->mode.equalizer.type);
            case AUDIO_GEQ_EFFECT_TYPE:
            {
                ALOGI("AUDIO_GEQ_EFFECT_TYPE:%s\n", config->mode.equalizer.eq_effect_type);
                if (!strcmp(config->mode.equalizer.eq_effect_type, "Default"))
                {
                    ret = dspdev_set_geq(GEQ_BASS, -2);
                    ret = dspdev_set_geq(GEQ_MIDDLE_BASS, -1);
                    ret = dspdev_set_geq(GEQ_MIDDLE, 1);
                    ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 2);
                    ret = dspdev_set_geq(GEQ_HIGH, 3);
                }
                else if(!strcmp(config->mode.equalizer.eq_effect_type, "Classic"))
                {
                    ret = dspdev_set_geq(GEQ_BASS, 2);
                    ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 2);
                    ret = dspdev_set_geq(GEQ_MIDDLE, 0);
                    ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 3);
                    ret = dspdev_set_geq(GEQ_HIGH, 4);
                }
                else if(!strcmp(config->mode.equalizer.eq_effect_type, "Pop"))
                {
                    ret = dspdev_set_geq(GEQ_BASS, 4);
                    ret = dspdev_set_geq(GEQ_MIDDLE_BASS, -2);
                    ret = dspdev_set_geq(GEQ_MIDDLE, 4);
                    ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 2);
                    ret = dspdev_set_geq(GEQ_HIGH, 2);
                }
                else if(!strcmp(config->mode.equalizer.eq_effect_type, "Jazz"))
                {
                    ret = dspdev_set_geq(GEQ_BASS, 2);
                    ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 0);
                    ret = dspdev_set_geq(GEQ_MIDDLE, 4);
                    ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 0);
                    ret = dspdev_set_geq(GEQ_HIGH, 2);
                }
                else if(!strcmp(config->mode.equalizer.eq_effect_type, "Percussion"))
                {
                    ret = dspdev_set_geq(GEQ_BASS, -4);
                    ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 2);
                    ret = dspdev_set_geq(GEQ_MIDDLE, 2);
                    ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, -2);
                    ret = dspdev_set_geq(GEQ_HIGH, -4);
                }
                else if(!strcmp(config->mode.equalizer.eq_effect_type, "Rock"))
                {
                    ret = dspdev_set_geq(GEQ_BASS, 6);
                    ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 0);
                    ret = dspdev_set_geq(GEQ_MIDDLE, 2);
                    ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 0);
                    ret = dspdev_set_geq(GEQ_HIGH, 6);
                }
                else if(!strcmp(config->mode.equalizer.eq_effect_type, "Customize"))
                {
                    ALOGI("Customize: will set frequency_index after\n");
                }
                else
                {
                    ret = dspdev_set_geq(GEQ_BASS, -2);
                    ret = dspdev_set_geq(GEQ_MIDDLE_BASS, -1);
                    ret = dspdev_set_geq(GEQ_MIDDLE, 1);
                    ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 2);
                    ret = dspdev_set_geq(GEQ_HIGH, 3);
                }
                break;
            }

            case AUDIO_GEQ_FREQUENCY_INDEX:
            {
                ALOGI("AUDIO_GEQ_FREQUENCY_INDEX:%d, %d\n", config->mode.equalizer.frequency_index, config->values);
                switch (config->mode.equalizer.frequency_index)
                {
                    case 0:
                    {
                        ret = dspdev_set_geq(GEQ_BASS, config->values);
                        break;
                    }
                    case 1:
                    {
                        ret = dspdev_set_geq(GEQ_MIDDLE_BASS, config->values);
                        break;
                    }
                    case 2:
                    {
                        ret = dspdev_set_geq(GEQ_MIDDLE, config->values);
                        break;
                    }
                    case 3:
                    {
                        ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, config->values);
                        break;
                    }
                    case 4:
                    {
                        ret = dspdev_set_geq(GEQ_HIGH, config->values);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }

            case AUDIO_GEQ_UNKNOWN:
            case AUDIO_GEQ_CLOSE:
            case AUDIO_GEQ_MAX:
            default:
            {
                ret = dspdev_set_geq(GEQ_BASS, 0);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 0);
                ret = dspdev_set_geq(GEQ_MIDDLE, 0);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 0);
                ret = dspdev_set_geq(GEQ_HIGH, 0);
                break;
            }
        }
        break;
    }

    case AUDIO_COMMAND_TYPE_FIELD:
    {
        ret = dspdev_set_sound_field(config->extra_data[0], config->extra_data[1]);
        break;
    }

    case AUDIO_COMMAND_TYPE_SURROUND:
    {

        break;
    }

    case AUDIO_COMMAND_TYPE_RESTORE:
    {

        break;
    }

    case AUDIO_COMMAND_TYPE_DTS:
    {

        break;
    }

    case AUDIO_COMMAND_TYPE_UNKOWN:
    default:
        break;
    }
#endif
    return ret;
}

static int adev_close(hw_device_t *device) {
    /* 	struct alsa_audio_device *adev = (struct alsa_audio_device
     * *)device; */

    DEBUG_FUNC_PRT
    struct alsa_audio_device *adev = (struct alsa_audio_device *)device;
    struct audio_route_l *ar_l = NULL;
    struct listnode *p = NULL;
    struct listnode *q = NULL;
    int i;
    list_for_each_safe(p, q, &adev->audio_route_list) {
        ar_l = node_to_item(p, struct audio_route_l, list);
        if (ar_l) {
            list_remove(&ar_l->list);
            audio_route_free(ar_l->audio_route);
            ar_l->audio_route = NULL;
            free(ar_l);
            ar_l = NULL;
        }
    }

    for (i = 0; i < sizeof(adev->mixers) / sizeof(struct mixer *); i++) {
        if (adev->mixers[i])
            mixer_close(adev->mixers[i]);
    }
    sd_platform_deinit(adev);
    free(device);
    return 0;
}

static int boot_dsp(struct alsa_audio_device *adev, int32_t dsp_id) {
    int ret = 0;
    int card = CARD_DEFAULT;
    struct audio_route *ar = NULL;
    char dsp_path_name[32] = {0};
    if (!adev)
        return -EINVAL;

    snprintf(dsp_path_name, sizeof(dsp_path_name), "boot-dsp%d", dsp_id);
    if ((card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
        return -EINVAL;
    }
    ar = audio_extra_get_audio_route(adev, card);
    ret = audio_route_apply_and_update_path(ar, dsp_path_name);
    if (ret < 0) {
        ALOGW("apply %s ctrl failed, errorno:%d", dsp_path_name, ret);
        return ret;
    }
    AALOGI("dsp_path_name : %s", dsp_path_name);
    return ret;
}

static void sd_dsp_init(struct alsa_audio_device *adev, int32_t dsp_count) {
    while (dsp_count) {
        boot_dsp(adev, dsp_count);
        dsp_count--;
    }
}

static int adev_open(const hw_module_t *module, const char *name, hw_device_t **device) {
    struct alsa_audio_device *adev;
    int ret = 0;

    ALOGI("adev_open: %s", name);

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = calloc(1, sizeof(struct alsa_audio_device));
    if (!adev)
        return -ENOMEM;
    adev->hw_device.common.tag = HARDWARE_DEVICE_TAG;
    adev->hw_device.common.version = AUDIO_DEVICE_API_VERSION_3_0;
    adev->hw_device.common.module = (struct hw_module_t *)module;
    adev->hw_device.common.close = adev_close;
    adev->hw_device.init_check = adev_init_check;
    adev->hw_device.set_voice_volume = adev_set_voice_volume;
    adev->hw_device.set_master_volume = adev_set_master_volume;
    adev->hw_device.get_master_volume = adev_get_master_volume;
    adev->hw_device.set_master_mute = adev_set_master_mute;
    adev->hw_device.get_master_mute = adev_get_master_mute;
    adev->hw_device.set_mode = adev_set_mode;
    adev->hw_device.set_mic_mute = adev_set_mic_mute;
    adev->hw_device.get_mic_mute = adev_get_mic_mute;
    adev->hw_device.set_parameters = adev_set_parameters;
    adev->hw_device.get_parameters = adev_get_parameters;
    adev->hw_device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->hw_device.open_output_stream = adev_open_output_stream;
    adev->hw_device.close_output_stream = adev_close_output_stream;
    adev->hw_device.open_input_stream = adev_open_input_stream;
    adev->hw_device.close_input_stream = adev_close_input_stream;
    adev->hw_device.dump = adev_dump;
    adev->hw_device.get_microphones = adev_get_microphones;
    /* Next is AUDIO_DEVICE_API_VERSION_3_0 API*/
    adev->devices = AUDIO_DEVICE_NONE;
    adev->hw_device.get_audio_port = adev_get_audio_port;
    adev->hw_device.set_audio_port_config = adev_set_audio_port_config;
    adev->hw_device.set_audio_param_config = adev_set_audio_param_config;
    adev->hw_device.create_audio_patch = adev_create_audio_patch;
    adev->hw_device.release_audio_patch = adev_release_audio_patch;
    *device = &adev->hw_device.common;
    adev->mode = AUDIO_MODE_NORMAL;
    adev->is_usb_exist = false;
    adev->bt_thread_data = NULL;

    if (is_x9u_ref_b()) {
        adev->hs_device = get_active_hs_device(adev);
    }

    set_audio_hal_bridge();

    // Initialize the bus address to output stream map
    adev->out_bus_stream_map = hashmapCreate(5, str_hash_fn, str_eq);
    list_init(&adev->audio_route_list);

    ret = sd_platform_init(adev);
    if (!ret) {
        if (sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY) >= 0) {
            sd_dsp_init(adev, MAX_DSP_INIT_COUNT);
            init_hfp(adev);
        } else {
            AALOGI("dsp card is not exist");
        }
    }

    for (int i = 0; i < BTCALL_RT_MAX; i++) {
        adev->bt_data[i] = NULL;
    }
    current_audio_bus_status.is_chime_on = 0;
    for (int j = 0; j < MAX_BUS_NUM; j++) {
        current_audio_bus_status.is_ducked[j] = 0;
        current_audio_bus_status.is_muted[j] = 0;
        current_audio_bus_status.in_zoneId[j] = 0xf;
        current_audio_bus_status.volume[j] = 0;
        current_audio_bus_status.slot_index[j] = adev->platform->info.output_maps[j].slots[0];
        if(j < 4)
        {
            current_audio_bus_status.slot_in_use[j] = 0;
            current_audio_bus_status.focus_bus[j] = 0xFF;
        }
        pthread_mutex_init(&current_audio_bus_status.lock[j], (const pthread_mutexattr_t *) NULL);
    }

    if (adev->platform->info.misc.btcall) {
        adev->ecnr_hdl = NULL;
        adev->ecnr_ops = get_ecnr_ops();
        // btcall_init_rx Must be initialized before btcall_init2
        if (!btcall_init_rx(adev)) {
            if (btcall_init_tx(adev)) {
                ALOGE("bt all tx init failed : %s", strerror(errno));
            }
        } else {
            ALOGE("bt all rx init failed : %s", strerror(errno));
        }
    }

    return ret;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common =
        {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = AUDIO_HARDWARE_MODULE_ID,
            .name = "x9 ref audio HW HAL",
            .author = "The Android Open Source Project",
            .methods = &hal_module_methods,
        },
};
