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

#define LOG_TAG "audio_hw_ecnr_fake"
// #define LOG_NDEBUG 0

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <system/thread_defs.h>

#include <cutils/log.h>
#include <hardware/audio.h>
#include <hardware/audio_alsaops.h>
#include <hardware/hardware.h>
#include <system/audio.h>

#include "audio_extra.h"
#include "audio_hw.h"
#include "ecnr_fake.h"
#include "hfp.h"
#include "remote.h"
#include <audio_utils/resampler.h>
#include <fcntl.h>
#include <tinyalsa/asoundlib.h>

#define DSP_FUNC_IN AALOGV("     DSP In...");
#define ECNR_FUNC_IN AALOGV("  ECNR In...");

static void *x9_ecnr_init(ecnr_param_t *param);
static int32_t x9_ecnr_ref_in(int32_t hdl, void *ref, int32_t ref_frames, void *args);
static int32_t x9_ecnr_process(int32_t hdl, void *in, int32_t in_frames, void *processed,
                               int32_t *processed_frames, void *args);
static int32_t x9_ecnr_recv_in(int32_t hdl, void *recv, int32_t recv_frames, void *processed,
                               int32_t *processed_frames, void *args);
static bool x9_ecnr_deinit(void *hld);

static struct ecnr_common_ops x9_ecnr = {.ecnr_init = x9_ecnr_init,
                                         .ecnr_ref_in = x9_ecnr_ref_in,
                                         .ecnr_process = x9_ecnr_process,
                                         .ecnr_recv_in = x9_ecnr_recv_in,
                                         .ecnr_deinit = x9_ecnr_deinit};

/* shoule implement in ecnr lib*/
/* ============================================================================
 */
static ecnr_param_t fake_ecnr_param;

static int fake_dsp_alloc(void) {
    DSP_FUNC_IN
    return 0;
}

static int fake_dsp_close(int handle) {
    DSP_FUNC_IN
    return 0;
}

static int fake_dsp_load_config(int handle, char *filename) {
    DSP_FUNC_IN
    return 0;
}

static int fake_dsp_recv_primary(int handle, void *SpkIn, int in_len, void *SpkOut, int *out_len,
                                 int64_t curr_timestamp) {
    DSP_FUNC_IN
    memcpy(SpkOut, SpkIn, in_len);
    *out_len = in_len;
    return 0;
}

static int fake_dsp_recv_primary_fmt(int handle, int in_channel, int in_rate, int out_channel,
                                     int out_rate) {
    DSP_FUNC_IN
    return 0;
}

static int fake_dsp_ref(int handle, void *RefIn, int in_len, int64_t curr_timestamp) {
    DSP_FUNC_IN
    return 0;
}

static int fake_dsp_ref_fmt(int handle, int in_channel, int in_rate) {
    DSP_FUNC_IN
    fake_ecnr_param.ref_in_cfg.ch = in_channel;
    return 0;
}

static int fake_dsp_release(int handle) {
    DSP_FUNC_IN
    return 0;
}
static int fake_dsp_send_primary(int handle, void *MicIn, int in_len, void *MicOut, int *out_len,
                                 int64_t curr_timestamp) {
    DSP_FUNC_IN
    int32_t tx_processed_len;
    tx_processed_len = audio_extra_select_slot_16(
        (int16_t *)MicOut, fake_ecnr_param.mic_out_cfg.ch /* ch */, MicIn,
        fake_ecnr_param.mic_in_cfg.ch /* ch */, in_len * fake_ecnr_param.mic_in_cfg.ch);
    *out_len = tx_processed_len;
    return 0;
}
static int fake_dsp_send_primary_fmt(int handle, int in_channel, int in_rate, int out_channel,
                                     int out_rate) {
    DSP_FUNC_IN
    fake_ecnr_param.mic_in_cfg.ch = in_channel;
    fake_ecnr_param.mic_out_cfg.ch = out_channel;
    return 0;
}

static int fake_dsp_start(int handle) {
    DSP_FUNC_IN
    return 0;
}
/* ============================================================================
 */

static void *x9_ecnr_init(ecnr_param_t *param) {
    ECNR_FUNC_IN
    int32_t ret;
    int *ecnr_handler = calloc(sizeof(int32_t), 1);
    if (ecnr_handler == NULL) {
        AALOGE("ecnr alloc failed");
        return NULL;
    }
    /* update params ? */
    *ecnr_handler = fake_dsp_alloc();
    switch (param->mic_out_cfg.rate) {
        case 8000:
            fake_dsp_load_config(*ecnr_handler, "/vendor/etc/8k_ecnr_config");
            break;
        case 16000:
            fake_dsp_load_config(*ecnr_handler, "/vendor/etc/16k_ecnr_config");
            break;
        case 24000:
            fake_dsp_load_config(*ecnr_handler, "/vendor/etc/24k_ecnr_config");
            break;
        case 32000:
            fake_dsp_load_config(*ecnr_handler, "/vendor/etc/32k_ecnr_config");
            break;
        default:
            AALOGE("invalid pamrams");
            goto exit;
    }

    ret = fake_dsp_recv_primary_fmt(*ecnr_handler, param->recv_in_cfg.ch, param->recv_in_cfg.rate,
                                    param->recv_out_cfg.ch, param->recv_out_cfg.rate);
    if (ret < 0) {
        AALOGE("set recv fmt faild");
        goto exit;
    }

    ret = fake_dsp_ref_fmt(*ecnr_handler, param->ref_in_cfg.ch, param->ref_in_cfg.rate);
    if (ret < 0) {
        AALOGE("set ref fmt faild");
        goto exit;
    }

    ret = fake_dsp_send_primary_fmt(*ecnr_handler, param->mic_in_cfg.ch, param->mic_in_cfg.rate,
                                    param->mic_out_cfg.ch, param->mic_out_cfg.rate);
    if (ret < 0) {
        AALOGE("set mic fmt faild");
        goto exit;
    }

    ret = fake_dsp_start(*ecnr_handler);
    if (ret < 0) {
        AALOGE("ecnr start faild");
        goto exit;
    }

    return (void *)ecnr_handler;

exit:
    if (ecnr_handler)
        free(ecnr_handler);
    return NULL;
}

static int32_t x9_ecnr_ref_in(int32_t hdl, void *ref, int32_t ref_frames, void *args) {
    ECNR_FUNC_IN
    AALOGV("hdl: %d, ref_frames: %d", hdl, ref_frames);
    return fake_dsp_ref(hdl, ref, ref_frames, 0);
}

static int32_t x9_ecnr_process(int32_t hdl, void *in, int32_t in_frames, void *processed,
                               int32_t *processed_frames, void *args) {
    ECNR_FUNC_IN
    int32_t ret = 0;
    AALOGV("hdl: %d, in_frames: %d, processed_frames: %d", hdl, in_frames, *processed_frames);
    ret = fake_dsp_send_primary(hdl, in, in_frames, processed, processed_frames, 0);
    if (ret < 0) {
        AALOGE("mic process faild");
    }
    AALOGV("ret : %d, processed_frames: %d", ret, *processed_frames);
    return ret;
}

static int32_t x9_ecnr_recv_in(int32_t hdl, void *recv, int32_t recv_frames, void *processed,
                               int32_t *processed_frames, void *args) {
    ECNR_FUNC_IN
    return fake_dsp_recv_primary(hdl, recv, recv_frames, processed, processed_frames, 0);
}

static bool x9_ecnr_deinit(void *hdl) {
    ECNR_FUNC_IN
    if (!hdl) {
        AALOGE("invalid arguments");
        return false;
    }

    fake_dsp_close(*(int32_t *)hdl);
    fake_dsp_release(*(int32_t *)hdl);
    free(hdl);
    return true;
}

struct ecnr_common_ops *get_ecnr_ops(void) {
    return &x9_ecnr;
}
