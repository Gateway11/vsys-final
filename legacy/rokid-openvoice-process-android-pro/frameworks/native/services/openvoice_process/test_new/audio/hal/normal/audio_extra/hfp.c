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

#define LOG_TAG "audio_hw_hfp"
// #define LOG_NDEBUG 0

#include "audio_extra.h"
#include "platform.h"
#include "remote.h"
#include <cutils/str_parms.h>
#include <errno.h>
#include <hfp.h>
#include <log/log.h>
#include <math.h>
#include <stdlib.h>
#include <tinyalsa/asoundlib.h>
#include <math.h>

#define AUDIO_PARAMETER_HFP_VOL_MIXER_CTL "hfp_vol_mixer_ctl"
#define AUDIO_PARAMATER_HFP_VALUE_MAX 128

#define HFP_ENABLE_MIXER_PATH "hfp"
#define HFP_MIC_MUTE_MIXER_PATH "hfp-mic-mute"

#define AUDIO_PARAMETER_KEY_HFP_MIC_VOLUME "hfp_mic_volume"
#define CAPTURE_VOLUME_DEFAULT (15.0)
#define MIN_HFP_VOLUME_DB (-115.5)  // db
#define HFP_VOLUME_SLOPE (0.5)

static struct audio_gain g_btcall_sw_gain = {
    .min_value = -4800,
    .max_value = 600,
    .step_value = 300,
};

// this definition will equal to audio_policy_configuration_car.xml bus2_phone_out gain setting.
#define HFP_VOLUME_MIN_mDB (-2800)
#define HFP_VOLUME_MAX_mDB (200)
#define VOL_PRECENT(vol_mdb) \
    (vol_mdb - HFP_VOLUME_MIN_mDB) * 1.0 / (HFP_VOLUME_MAX_mDB - HFP_VOLUME_MIN_mDB) * 100
struct hfp_module {
    struct pcm *hfp_sco_rx;
    struct pcm *hfp_sco_tx;
    int hfp_volume;
    float amplitude_ratio;
    float mic_volume;
    char hfp_vol_mixer_ctl[AUDIO_PARAMATER_HFP_VALUE_MAX];
    bool is_hfp_running;
    bool mic_mute;
    bool wb;
    int32_t (*hfp_set_volume_hook)(struct alsa_audio_device *adev, int volume /*mB*/);
    void (*restore_ms_hfp_volume)(struct alsa_audio_device *adev);
    int32_t ms_hfp_vol_tmp[4];
};

/* Remote */
static int32_t set_remote_mic_mute(struct alsa_audio_device *adev, bool mute);
static int32_t enable_remote_hfp(struct alsa_audio_device *adev, bool enable);
static int32_t set_remote_hfp_volume_dsp(struct alsa_audio_device *adev, int volume);
static int32_t set_remote_hfp_volume_soc(struct alsa_audio_device *adev, int volume);
/* end */

static int32_t hfp_set_volume_ms_board(struct alsa_audio_device *adev, int volume);
static int32_t hfp_set_volume_ref_board(struct alsa_audio_device *adev, int volume /*mB*/);
static struct hfp_module hfpmod = {
    .wb = true,
    .hfp_sco_rx = NULL,
    .hfp_sco_tx = NULL,
    .hfp_volume = 0,
    .mic_volume = CAPTURE_VOLUME_DEFAULT,
    .hfp_vol_mixer_ctl =
        {
            0,
        },
    .is_hfp_running = 0,
    .mic_mute = 0,
    .hfp_set_volume_hook = hfp_set_volume_ref_board,
};
static struct pcm_config pcm_config_hfp = {.channels = 1,
                                           .rate = 16000,
                                           .period_size = 192,
                                           .period_count = 4,
                                           .format = PCM_FORMAT_S16_LE,
                                           .start_threshold = 0,
                                           .stop_threshold = 0,
                                           .avail_min = 0};

static int hfp_ctrl_enable(struct alsa_audio_device *adev, bool enable) {
    int ret = 0;
    int card_id = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY);

    if (card_id < 0) {
        return -1;
    }

    if (enable) {
        ret = audio_route_apply_and_update_path(audio_extra_get_audio_route(adev, card_id),
                                                HFP_ENABLE_MIXER_PATH);
        if (ret < 0) {
            ALOGE("apply phone ctrl failed, errorno:%d", ret);
            return ret;
        }
    } else {
        ret = audio_route_reset_and_update_path(audio_extra_get_audio_route(adev, card_id),
                                                HFP_ENABLE_MIXER_PATH);
        if (ret < 0) {
            ALOGE("reset phone ctrl failed, errorno:%d", ret);
            return ret;
        }
    }

    return ret;
}
static void store_hfp_volume(struct alsa_audio_device *adev) {
    ALOGD("%s: enter", __func__);
    struct mixer_ctl *ctl = NULL;
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC1 Digital Volume L");
    hfpmod.ms_hfp_vol_tmp[0] = mixer_ctl_get_value(ctl, 0);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC1 Digital Volume R");
    hfpmod.ms_hfp_vol_tmp[1] = mixer_ctl_get_value(ctl, 0);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC2 Digital Volume L");
    hfpmod.ms_hfp_vol_tmp[2] = mixer_ctl_get_value(ctl, 0);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC2 Digital Volume R");
    hfpmod.ms_hfp_vol_tmp[3] = mixer_ctl_get_value(ctl, 0);
}
static void restore_hfp_volume(struct alsa_audio_device *adev) {
    struct mixer_ctl *ctl = NULL;

    ALOGD("%s: enter", __func__);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC1 Digital Volume L");
    mixer_ctl_set_value(ctl, 0, hfpmod.ms_hfp_vol_tmp[0]);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC1 Digital Volume R");
    mixer_ctl_set_value(ctl, 0, hfpmod.ms_hfp_vol_tmp[1]);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC2 Digital Volume L");
    mixer_ctl_set_value(ctl, 0, hfpmod.ms_hfp_vol_tmp[2]);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC2 Digital Volume R");
    mixer_ctl_set_value(ctl, 0, hfpmod.ms_hfp_vol_tmp[3]);
}

static void restore_hfp_remote_vol(struct alsa_audio_device *adev) {
    ALOGD("%s: enter", __func__);
    audio_remote_set_volume(adev, HIFI_PLAYBACK_TO_MAIN_SPK_48K,
                            100);  // hifi path sw gain, restore 100%, if phone
                                   // path support should delete this
}

int32_t init_hfp(struct alsa_audio_device *adev) {
    char linux_name[CARD_NAME_LENGTH];
    ALOGD("%s: enter", __func__);
    memset(linux_name, 0x00, CARD_NAME_LENGTH);
    sd_platform_get_card_name(adev->platform, AUDIO_NAME_PRIMARY, linux_name);
    if (!strcmp(linux_name, "x9core01ak7738")) {
        store_hfp_volume(adev);
        hfpmod.hfp_set_volume_hook = hfp_set_volume_ms_board;
        hfpmod.restore_ms_hfp_volume = restore_hfp_volume;
        ALOGD("use %s adjust hfp volume", linux_name);
    }
    if (strstr(linux_name, "mach")) {
        store_hfp_volume(adev);
        if (adev->platform->bt_type == BT_CONNECT_WITH_DSP) {
            hfpmod.hfp_set_volume_hook = set_remote_hfp_volume_dsp;
        } else if (adev->platform->bt_type == BT_CONNECT_WITH_SOC) {
            hfpmod.hfp_set_volume_hook = set_remote_hfp_volume_soc;
        }
        ALOGD("use %s adjust hfp volume", linux_name);
    }
    return 0;
}

int32_t start_hfp(struct alsa_audio_device *adev) {
    int32_t ret = 0;
    int32_t card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY);
    int32_t pcm_dev_rx_id = 1;  // TODO: will support dynamic update by platform
    int32_t pcm_dev_tx_id = 1;

    ALOGV("%s: enter", __func__);

    if (card < 0) {
        return -1;
    }

    if (adev->enable_hfp == true) {
        ALOGD("%s: HFP is already active!\n", __func__);
        return 0;
    }
    adev->enable_hfp = true;
    if (adev->platform->remote > 0) {
        AALOGI("Remote audiomanager is enable");
        hfpmod.is_hfp_running = true;
        ret = enable_remote_hfp(adev, true);
        hfp_set_volume(adev, hfpmod.hfp_volume);
        return ret;
    }
    hfp_ctrl_enable(adev, true);
    pcm_config_hfp.rate = adev->hfp_sample_rate;
    if (pcm_dev_rx_id < 0 || pcm_dev_tx_id < 0) {
        ALOGE("%s: Invalid PCM devices (rx: %d tx: %d )", __func__, pcm_dev_rx_id, pcm_dev_tx_id);
        ret = -EIO;
        goto exit;
    }

    ALOGD(
        "%s: Opening PCM playback device card_id(%d) form platform device_id(%d) "
        "rate:(%d)",
        __func__, card, pcm_dev_rx_id, pcm_config_hfp.rate);
    hfpmod.hfp_sco_rx = pcm_open(card, pcm_dev_rx_id, PCM_OUT, &pcm_config_hfp);

    if (!hfpmod.hfp_sco_rx || !pcm_is_ready(hfpmod.hfp_sco_rx)) {
        ALOGE("%s: %s", __func__, pcm_get_error(hfpmod.hfp_sco_tx));
        ret = -EIO;
        goto exit;
    }
    ALOGD(
        "%s: Opening PCM capture device card_id(%d) device_id(%d) "
        "rate:(%d)",
        __func__, card, pcm_dev_tx_id, pcm_config_hfp.rate);

    hfpmod.hfp_sco_tx = pcm_open(card, pcm_dev_tx_id, PCM_IN, &pcm_config_hfp);
    if (!hfpmod.hfp_sco_tx || !pcm_is_ready(hfpmod.hfp_sco_tx)) {
        ALOGE("%s: %s", __func__, pcm_get_error(hfpmod.hfp_sco_tx));
        ret = -EIO;
        goto exit;
    }
    pcm_start(hfpmod.hfp_sco_rx);
    pcm_start(hfpmod.hfp_sco_tx);

    hfpmod.is_hfp_running = true;

    /*
    TODO: update phone volume and mic mute.
    */
    hfp_set_volume(adev, hfpmod.hfp_volume);
    // hfp_set_mic_mute(adev, adev->mic_muted);
    ALOGV("%s: exit: status(%d)", __func__, ret);
    return 0;

exit:
    stop_hfp(adev);
    ALOGE("%s: Problem in HFP start: status(%d)", __func__, ret);
    return ret;
}

int32_t stop_hfp(struct alsa_audio_device *adev) {
    int32_t ret = 0;
    if (!hfpmod.is_hfp_running) {
        ALOGW("Invalid state for stop hfp");
        goto exit;
    }
    ALOGV("%s: enter", __func__);
    hfpmod.is_hfp_running = false;
    if (adev->platform->remote > 0) {
        AALOGI("Remote audiomanager is enable");
        adev->enable_hfp = false;
        return enable_remote_hfp(adev, false);
    }
    /* 1. Close the PCM devices */
    if (hfpmod.hfp_sco_rx) {
        pcm_close(hfpmod.hfp_sco_rx);
        hfpmod.hfp_sco_rx = NULL;
    }
    if (hfpmod.hfp_sco_tx) {
        pcm_close(hfpmod.hfp_sco_tx);
        hfpmod.hfp_sco_tx = NULL;
    }
    /* 2. Get and set stream specific mixer controls */
    hfp_ctrl_enable(adev, false);

    /* 3. Update status */
    adev->enable_hfp = false;
exit:
    if (hfpmod.restore_ms_hfp_volume)
        hfpmod.restore_ms_hfp_volume(adev);
    ALOGD("%s: exit: status(%d)", __func__, ret);
    return ret;
}
static int32_t hfp_set_volume_ms_board(struct alsa_audio_device *adev, int vol_index) {
    struct mixer_ctl *ctl = NULL;
    int32_t ret = 0;
    if (!adev)
        return -EINVAL;

    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC1 Digital Volume L");
    ret = mixer_ctl_set_value(ctl, 0, vol_index);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC1 Digital Volume R");
    ret = mixer_ctl_set_value(ctl, 0, vol_index);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC2 Digital Volume L");
    ret = mixer_ctl_set_value(ctl, 0, vol_index);
    ctl = mixer_get_ctl_by_name(adev->mixers[0], "DAC2 Digital Volume R");
    ret = mixer_ctl_set_value(ctl, 0, vol_index);

    ALOGD("%s, set volume %d to kernel %s", __func__, vol_index, strerror(ret));
    return ret;
}

static int32_t hfp_set_volume_ref_board(struct alsa_audio_device *adev, int vol_index /*mB*/) {
    struct mixer_ctl *ctl = NULL;
    int32_t ret = 0;

    if (!adev)
        return -EINVAL;

    ctl = mixer_get_ctl_by_name(adev->mixers[0], CTRL_DSP1_HFP_VOL);
    ret = mixer_ctl_set_value(ctl, 0, vol_index);

    ALOGD("%s, set volume %d to kernel %s", __func__, vol_index, strerror(ret));
    return ret;
}

/*return 0-100 index*/
int32_t hfp_get_volume_idx(struct alsa_audio_device *adev) {
    int32_t vol_index = -1;
    if (adev->platform->remote > 0)
        vol_index = VOL_PRECENT(hfpmod.hfp_volume);
    return vol_index;
}

static float hfp_mdb_2_ratio(int32_t gain) {
    int gainIndex = (gain - g_btcall_sw_gain.min_value) / g_btcall_sw_gain.step_value;
    int totalSteps =
        (g_btcall_sw_gain.max_value - g_btcall_sw_gain.min_value) / g_btcall_sw_gain.step_value;
    int minDb = g_btcall_sw_gain.min_value / 100;
    int maxDb = g_btcall_sw_gain.max_value / 100;
    return powf((float)10,
                (float)((minDb + (maxDb - minDb) * (gainIndex / (float)totalSteps)) / 20));
}

int32_t hfp_set_volume(struct alsa_audio_device *adev, int volume /*mB*/) {
    int32_t ret = 0;
    int32_t vol_index = 0;  // 0-255

    if (!adev)
        return -EINVAL;
    if (adev->platform->remote > 0)
        vol_index = VOL_PRECENT(volume);
    else
        vol_index = ((volume / 100.0 /*mB -> dB*/) - MIN_HFP_VOLUME_DB) / HFP_VOLUME_SLOPE;

    hfpmod.hfp_volume = volume;

    ret = hfpmod.hfp_set_volume_hook(adev, vol_index);
    return ret;
}
static int32_t set_remote_mic_mute(struct alsa_audio_device *adev, bool mute) {
    AALOGI("%s: enter", __func__);
    if (!mute) {
        audio_remote_start(
            adev, hfpmod.wb ? BT_CAPTURE_FROM_MAIN_MIC_16K : BT_CAPTURE_FROM_MAIN_MIC_8K, 100);

    } else {
        audio_remote_stop(adev,
                          hfpmod.wb ? BT_CAPTURE_FROM_MAIN_MIC_16K : BT_CAPTURE_FROM_MAIN_MIC_8K);
    }
    return 0;
}

int32_t hfp_set_mic_mute(struct alsa_audio_device *adev, bool state) {
    int ret = 0;
    int card_id = CARD_DEFAULT;
    struct audio_route *ar = NULL;
    if (!adev)
        return -EINVAL;

    if (!hfpmod.is_hfp_running) {
        ALOGW("Unsupported hfp status: %d", hfpmod.is_hfp_running);
        return -EPIPE;
    }

    if (adev->platform->remote > 0) {
        return set_remote_mic_mute(adev, state);
    }

    card_id = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY);
    if (card_id < 0) {
        return -EINVAL;
    }
    ar = audio_extra_get_audio_route(adev, card_id);
    if (state) {
        ret = audio_route_apply_and_update_path(ar, HFP_MIC_MUTE_MIXER_PATH);
    } else {
        ret = audio_route_reset_and_update_path(ar, HFP_MIC_MUTE_MIXER_PATH);
    }
    if (ret < 0) {
        ALOGE("apply hfp-mic-mute path failed, errorno:%d", ret);
    }
    return ret;
}

static int32_t enable_remote_hfp(struct alsa_audio_device *adev, bool enable) {
    AALOGI("%s: enter", __func__);
    if (enable) {
        set_remote_mic_mute(adev, false);
        audio_remote_start(adev,
                           hfpmod.wb ? BT_PLAYBACK_TO_MAIN_SPK_16K : BT_PLAYBACK_TO_MAIN_SPK_8K,
                           0);  // volume will been set by hfp_set_volume when start hfp
    } else {
        set_remote_mic_mute(adev, true);
        audio_remote_stop(adev,
                          hfpmod.wb ? BT_PLAYBACK_TO_MAIN_SPK_16K : BT_PLAYBACK_TO_MAIN_SPK_8K);
    }
    return 0;
}

static int32_t set_remote_hfp_volume_dsp(struct alsa_audio_device *adev, int volume /*index*/) {
    uint32_t vol;
    vol = (2800 + volume) / 24;
    audio_remote_set_volume(
        adev, hfpmod.wb ? BT_PLAYBACK_TO_MAIN_SPK_16K : BT_PLAYBACK_TO_MAIN_SPK_8K, volume);
    return 0;
}

// sw gain
static int32_t set_remote_hfp_volume_soc(struct alsa_audio_device *adev, int volume /*index*/) {
    hfpmod.amplitude_ratio = hfp_mdb_2_ratio(hfpmod.hfp_volume);
    AALOGI("amplitude_ratio: %f", hfpmod.amplitude_ratio);
    // TODO:
    // if audiomanager support phone path use audiomanager adjust volume
    // sw gain will bring extra CPU consumption and pop noise
    return 0;
}

float hfp_get_amplitude_ratio(void) {
    return hfpmod.amplitude_ratio;
}
