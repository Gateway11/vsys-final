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

#define LOG_TAG "AHAL: DPin"
#define LOG_NDDEBUG 0

#include <errno.h>
#include <math.h>
#include <log/log.h>
#include <unistd.h>
#include <cutils/properties.h>
#include "PalApi.h"
#include "AudioDevice.h"
#include "AudioCommon.h"

#ifdef DYNAMIC_LOG_ENABLED
#include <log_xml_parser.h>
#define LOG_MASK HAL_MOD_FILE_DPIN
#include <log_utils.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_PARAMETER_KEY_HANDLE_DPIN "handle_dpin"
#define AUDIO_PARAMETER_KEY_DPIN_VOLUME "dpin_volume"
#define AUDIO_PARAMETER_KEY_REC_PLAY_CONC "rec_play_conc_on"
#define AUDIO_PARAMETER_KEY_DPIN_MUTE "dpin_mute"
#define AUDIO_PARAMETER_KEY_DPIN_RESTORE_VOLUME "dpin_restore_volume"
#define AUDIO_PARAMETER_KEY_DPIN_ROUTING "dpin_routing"
#define AUDIO_PARAMETER_KEY_DPIN_STATUS "dpin_status"
#define DPIN_LOOPBACK_DRAIN_TIME_MS 2

#define CHANNELS 2
#define BIT_WIDTH 16
#define SAMPLE_RATE 48000

struct dpin_module {
    bool running;
    bool muted;
    bool restart;
    float volume;
    audio_devices_t device;
    pal_stream_handle_t* stream_handle;
};

static struct dpin_module dpin = {
    .running = 0,
    .muted = 0,
    .restart = 0,
    .volume = 0,
    .device = (audio_devices_t)0,
    .stream_handle = 0
};

int32_t dpin_set_volume(float value, bool persist=false)
{
    int32_t ret = 0;
    struct pal_volume_data *pal_volume = NULL;

    AHAL_DBG("Enter: volume = %f, persist: %d", value, persist);

    if (value < 0.0) {
       AHAL_DBG("(%f) Under 0.0, assuming 0.0", value);
        value = 0.0;
    } else if (value > 1.0) {
        AHAL_DBG("(%f) Over 1.0, assuming 1.0", value);
        value = 1.0;
    }

    if(persist)
        dpin.volume = value;

    if (dpin.muted && value > 0) {
        AHAL_DBG("dpin is muted, applying '0' volume instead of %f", value);
        value = 0;
    }

    if (!dpin.running) {
        AHAL_VERBOSE(" dpin not active, ignoring set_volume call");
        return -EIO;
    }

    AHAL_DBG("Setting dpin volume to %f", value);

    pal_volume = (struct pal_volume_data *) malloc(sizeof(struct pal_volume_data) + sizeof(struct pal_channel_vol_kv));

    if (!pal_volume)
       return -ENOMEM;

    pal_volume->no_of_volpair = 1;
    pal_volume->volume_pair[0].channel_mask = 0x03;
    pal_volume->volume_pair[0].vol = value;

    ret = pal_stream_set_volume(dpin.stream_handle, pal_volume);
    if (ret)
        AHAL_ERR("set volume failed: %d", ret);

    free(pal_volume);
    AHAL_DBG("exit");
    return ret;
}

int32_t dpin_start(std::shared_ptr<AudioDevice> adev, int device_id)
{
    int32_t ret = 0;
    const int num_pal_devs = 2;
    struct pal_stream_attributes stream_attr;
    struct pal_channel_info ch_info;
    struct pal_device pal_devs[num_pal_devs];
    pal_device_id_t pal_device_id = PAL_DEVICE_OUT_USB_HEADSET;
    dynamic_media_config_t dynamic_media_config;
    size_t payload_size = 0;

    AHAL_DBG("Enter");

    if(device_id == AUDIO_DEVICE_OUT_USB_HEADSET)
        pal_device_id = PAL_DEVICE_OUT_USB_HEADSET;
    else {
        AHAL_ERR("Unsupported device_id %d",device_id);
        return -EINVAL;
    }

    ch_info.channels = CHANNELS;
    ch_info.ch_map[0] = PAL_CHMAP_CHANNEL_FL;
    ch_info.ch_map[1] = PAL_CHMAP_CHANNEL_FR;

    stream_attr.type = PAL_STREAM_LOOPBACK;
    stream_attr.info.opt_stream_info.loopback_type = PAL_STREAM_LOOPBACK_KARAOKE;
    stream_attr.direction = PAL_AUDIO_INPUT_OUTPUT;
    stream_attr.in_media_config.sample_rate = SAMPLE_RATE;
    stream_attr.in_media_config.bit_width = BIT_WIDTH;
    stream_attr.in_media_config.ch_info = ch_info;
    stream_attr.in_media_config.aud_fmt_id = PAL_AUDIO_FMT_PCM_S16_LE;
    //stream_attr.in_media_config.aud_fmt_id = PAL_AUDIO_FMT_DEFAULT_PCM;

    stream_attr.out_media_config.sample_rate = SAMPLE_RATE;
    stream_attr.out_media_config.bit_width = BIT_WIDTH;
    stream_attr.out_media_config.ch_info = ch_info;
    stream_attr.out_media_config.aud_fmt_id = PAL_AUDIO_FMT_PCM_S16_LE;
    //stream_attr.out_media_config.aud_fmt_id = PAL_AUDIO_FMT_DEFAULT_PCM;


    for(int i = 0; i < 2; ++i){
        //pal_devs[i].id = i ? PAL_DEVICE_IN_AUX_DIGITAL : pal_device_id;
        pal_devs[i].id = i ? PAL_DEVICE_IN_HANDSET_MIC : pal_device_id;
        // TODO: remove hardcoded device id & pass adev to getPalDeviceIds instead
        if (device_out == PAL_DEVICE_OUT_USB_HEADSET) {
            //Configure USB Digital Headset parameters
            pal_param_device_capability_t *device_cap_query = (pal_param_device_capability_t *)
                                                       malloc(sizeof(pal_param_device_capability_t));
            if (!device_cap_query) {
                AHAL_ERR("Failed to allocate mem for device_cap_query");
                return 0;
            }

            device_cap_query->id = PAL_DEVICE_OUT_USB_DEVICE;
            device_cap_query->is_playback = true;
            device_cap_query->addr.card_id = adev->usb_card_id_;
            device_cap_query->addr.device_num = adev->usb_dev_num_;
            device_cap_query->config = &dynamic_media_config;
            pal_get_param(PAL_PARAM_ID_DEVICE_CAPABILITY,
                                 (void **)&device_cap_query,
                                 &payload_size, nullptr);
            pal_devs[i].address.card_id = adev->usb_card_id_;
            pal_devs[i].address.device_num = adev->usb_dev_num_;
            pal_devs[i].config.sample_rate = dynamic_media_config.sample_rate[0];
            pal_devs[i].config.ch_info = ch_info;
            pal_devs[i].config.aud_fmt_id = (pal_audio_fmt_t)dynamic_media_config.format[0];
            free(device_cap_query);
        } else {
            pal_devs[i].config.sample_rate = SAMPLE_RATE;
            pal_devs[i].config.bit_width = BIT_WIDTH;
            pal_devs[i].config.ch_info = ch_info;
            pal_devs[i].config.aud_fmt_id = PAL_AUDIO_FMT_PCM_S16_LE;
            //pal_devs[i].config.aud_fmt_id = PAL_AUDIO_FMT_DEFAULT_PCM;
        }
    }

    ret = pal_stream_open(&stream_attr,
            num_pal_devs, pal_devs,
            0,
            NULL,
            NULL,
            0,
            &dpin.stream_handle);

    if (ret) {
        AHAL_ERR("stream open failed with: %d", ret);
        return ret;
    }

    ret = pal_stream_start(dpin.stream_handle);
    if (ret) {
        AHAL_ERR("stream start failed with %d", ret);
        pal_stream_close(dpin.stream_handle);
        return ret;
    }

    dpin.running = true;
    dpin_set_volume(dpin.volume, true);
    AHAL_DBG("Exit");
    return ret;
}

int32_t dpin_stop()
{
    AHAL_DBG("enter");

    if(!dpin.running){
        AHAL_ERR("dpin not in running state...");
        return -EINVAL;
    }

    if (dpin.stream_handle) {
        pal_stream_stop(dpin.stream_handle);
        pal_stream_close(dpin.stream_handle);
    }
    dpin.stream_handle = NULL;
    dpin.running = false;
    AHAL_DBG("exit");
    return 0;
}

void dpin_get_parameters(std::shared_ptr<AudioDevice> adev __unused, struct str_parms *query, struct str_parms *reply)
{
    int ret;
    char value[32] = {0};

    AHAL_DBG("enter");

    if(query && reply){
        ret = str_parms_get_str(query, AUDIO_PARAMETER_KEY_DPIN_STATUS, value, sizeof(value));
        if (ret >= 0)
            str_parms_add_int(reply, AUDIO_PARAMETER_KEY_DPIN_STATUS, dpin.running);
    }
    AHAL_DBG("exit");
}

inline void hal2vec(audio_devices_t hdev, std::vector<audio_devices_t>& hdevs){
    audio_devices_t out_devs = (audio_devices_t)(hdev & AUDIO_DEVICE_OUT_ALL);
    audio_devices_t in_devs = (audio_devices_t)(hdev & AUDIO_DEVICE_IN_ALL);

    for(audio_devices_t i = (audio_devices_t)0x1; i < AUDIO_DEVICE_OUT_DEFAULT; i = (audio_devices_t)(i << 1))
        if(out_devs & i)
            hdevs.push_back(i);

    for(audio_devices_t i = (audio_devices_t)0x10000; i < AUDIO_DEVICE_IN_DEFAULT; i = (audio_devices_t)(i << 1))
        if(out_devs & i)
            hdevs.push_back(i);
}

void dpin_set_parameters(std::shared_ptr<AudioDevice> adev, struct str_parms *parms)
{
    int ret, val, num_pal_devs;
    pal_device_id_t *pal_devs;
    char value[32] = {0};
    float vol = 0.0;

    AHAL_DBG("Enter");

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_HANDLE_DPIN,
                            value, sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        AHAL_DBG("dpin usecase");
        if (val)
        {
            // AUDIO_DEVICE_IN_AUX_DIGITAL | AUDIO_DEVICE_OUT_USB_HEADSET
            if(val & AUDIO_DEVICE_IN_AUX_DIGITAL && !dpin.running)
                dpin_start(adev, val & ~AUDIO_DEVICE_IN_AUX_DIGITAL);
            else if (!(val & AUDIO_DEVICE_IN_AUX_DIGITAL) && dpin.running) {
                dpin_set_volume(0, false);
                usleep(DPIN_LOOPBACK_DRAIN_TIME_MS*1000);
                dpin_stop();
            }
        }
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_DPIN_ROUTING, value, sizeof(value));
    if (ret >= 0 && dpin.running) {
        val = atoi(value);
       AHAL_DBG("dpin usecase");
        if (val && (val & AUDIO_DEVICE_IN_AUX_DIGITAL)){
            dpin_set_volume(0, false);
            dpin_stop();
            dpin_start(adev, val & ~AUDIO_DEVICE_IN_AUX_DIGITAL);
        }
    }
    memset(value, 0, sizeof(value));

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_DPIN_VOLUME, value, sizeof(value));
    if (ret >= 0) {
       AHAL_DBG("Param: set volume");
        if (sscanf(value, "%f", &vol) != 1){
            AHAL_ERR("error in retrieving dpin volume");
            return;
        }
        dpin_set_volume(vol, true);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_DPIN_MUTE, value, sizeof(value));
    if (ret >= 0) {
        AHAL_DBG("Param: mute");
        dpin.muted = (value[0] == '1');
        if(dpin.muted)
           dpin_set_volume(0);
        else
           dpin_set_volume(dpin.volume);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_DPIN_RESTORE_VOLUME, value, sizeof(value));
    if (ret >= 0) {
       AHAL_DBG("Param: restore volume");
        if (value[0] == '1')
            dpin_set_volume(dpin.volume);
    }

    AHAL_DBG("exit");
}

#ifdef __cplusplus
}
#endif
