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

#define LOG_TAG "audio_hw_btcall"
// #define LOG_NDEBUG 0

#include <audio_utils/resampler.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <hardware/audio.h>
#include <hardware/audio_alsaops.h>
#include <hardware/hardware.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <system/audio.h>
#include <system/thread_defs.h>
#include <tinyalsa/asoundlib.h>

#include "audio_extra.h"
#include "audio_hw.h"
#include "ecnr_fake.h"
#include "hfp.h"
#include "remote.h"
#define TMP_BUF_SZ 15360

extern struct pcm_config *pcm_main_config;
extern struct pcm_config *pcm_main_cap_config;

struct pcm_config pcm_config_bt_in = {
    .channels = CHANNEL_MONO, // ahub stereo. slot map mono
    .rate = SAMPLING_RATE_16K,
    .period_size = (SAMPLING_RATE_16K * IN_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config pcm_config_bt_out = {
    .channels = CHANNEL_MONO,
    .rate = SAMPLING_RATE_16K,
    .period_size = (SAMPLING_RATE_16K * OUT_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config pcm_config_codec_out = {
    .channels = CHANNEL_STEREO,
    .rate = SAMPLING_RATE_16K,
    .period_size = (SAMPLING_RATE_16K * OUT_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config pcm_config_codec_in = {
    .channels = CHANNEL_2POINT0POINT2,
    .rate = SAMPLING_RATE_16K,
    .period_size = (SAMPLING_RATE_16K * IN_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct s_btcall_rt_map {
    const char *name;
    btcall_rt id;
};

static const struct s_btcall_rt_map btcall_rt_map[BTCALL_RT_MAX] = {
    {"bt_in", BTCALL_BT_INPUT},
    {"bt_out", BTCALL_BT_OUTPUT},
    {"codec_in", BTCALL_CODEC_INPUT},
    {"codec_out", BTCALL_CODEC_OUTPUT},
};

ecnr_param_t param = {
    .mic_in_cfg.rate = 16000,  // default, will update by adev->hfp_rate
    .mic_in_cfg.ch = 2,
    .mic_in_cfg.fmt = PCM_FORMAT_S16_LE,

    .recv_in_cfg.rate = 16000,  // default, will update by adev->hfp_rate
    .recv_in_cfg.ch = 1,
    .recv_in_cfg.fmt = PCM_FORMAT_S16_LE,

    .recv_out_cfg.rate = 16000,
    .recv_out_cfg.ch = 1,
    .recv_out_cfg.fmt = PCM_FORMAT_S16_LE,

    .ref_in_cfg.rate = 16000,
    .ref_in_cfg.ch = 2,
    .ref_in_cfg.fmt = PCM_FORMAT_S16_LE,

    .mic_out_cfg.rate = 16000,
    .mic_out_cfg.ch = 1,  // Real ECNR lib may support arbitrary parameters(mono,stereo...)
    .mic_out_cfg.fmt = PCM_FORMAT_S16_LE,
};

static FILE *rec_in_fp;
static FILE *mic_in_fp;
static FILE *mic_out_fp;
static FILE *ref_in_fp;

static size_t get_input_frame_size(const struct btcall_data *data) {
    size_t chan_samp_sz;
    audio_format_t format = audio_format_from_pcm_format(data->config->format);
    audio_channel_mask_t channel_mask = audio_channel_in_mask_from_count(data->config->channels);

    if (audio_has_proportional_frames(format)) {
        chan_samp_sz = audio_bytes_per_sample(format);
        return audio_channel_count_from_in_mask(channel_mask) * chan_samp_sz;
    }

    return sizeof(int8_t);
}

static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                           struct resampler_buffer *buffer) {
    struct btcall_data *data;
    int frame_sz = pcm_format_to_bits(param.recv_in_cfg.fmt) / 8;
    char rx_processed[TMP_BUF_SZ];
    int rx_processed_len;
    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    data = (struct btcall_data *)((char *)buffer_provider -
                                  offsetof(struct btcall_data, buf_provider));
    struct alsa_audio_device *adev = data->adev;

    int sz = data->config->period_size * get_input_frame_size(data);
    if (data->frames_in == 0) {
        if (data->pcm) {
            data->read_status = pcm_read(data->pcm, (void *)data->buffer, sz);
        } else if (data->in->pcm_hdl) {
            data->read_status = hal_streamer_read(data->in, (void *)data->buffer, sz);
            // dump_out_data((void *)data->buffer, sz, NULL, 1);
        }
        if (BTCALL_BT_INPUT == data->direction && adev->ecnr_hdl) {
            struct timeval start, end;
            gettimeofday(&start, NULL);
            adev->ecnr_ops->ecnr_recv_in(
                *(int32_t *)adev->ecnr_hdl, (void *)data->buffer,
                sz / frame_sz /* s16 */ / param.recv_in_cfg.ch /*channels*/, rx_processed,
                &rx_processed_len, 0);
            gettimeofday(&end, NULL);
            int32_t interval =
                1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
            AALOGV(
                "recv_primary %f ms, "
                "rx_processed_len: %d",
                interval / 1000.0, rx_processed_len);
            FP_WRITE(rx_processed, rx_processed_len * 2, rec_in_fp);
        }
        if (data->read_status != 0) {
            AALOGE("pcm_read error %d, %s", data->read_status, strerror(errno));
            buffer->raw = NULL;
            buffer->frame_count = 0;
            return data->read_status;
        }
        data->frames_in = data->config->period_size;
    }

    buffer->frame_count =
        (buffer->frame_count > data->frames_in) ? data->frames_in : buffer->frame_count;
    buffer->i16 =
        data->buffer + (data->config->period_size - data->frames_in) * data->config->channels;

    return data->read_status;
}

static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                           struct resampler_buffer *buffer) {
    struct btcall_data *data;

    if (buffer_provider == NULL || buffer == NULL)
        return;

    data = (struct btcall_data *)((char *)buffer_provider -
                                  offsetof(struct btcall_data, buf_provider));

    data->frames_in -= buffer->frame_count;
}

/* read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer
 * specified */
static ssize_t read_frames(struct btcall_data *in_data, void *buffer, ssize_t frames) {
    ssize_t frames_wr = 0;

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;
        if (in_data->resampler != NULL) {
            in_data->resampler->resample_from_provider(
                in_data->resampler,
                (int16_t *)((char *)buffer + frames_wr * get_input_frame_size(in_data)),
                &frames_rd);
        } else {
            struct resampler_buffer buf = {
                {
                    .raw = NULL,
                },
                .frame_count = frames_rd,
            };
            get_next_buffer(&in_data->buf_provider, &buf);
            if (buf.raw != NULL) {
                memcpy((char *)buffer + frames_wr * get_input_frame_size(in_data), buf.raw,
                       buf.frame_count * get_input_frame_size(in_data));
                frames_rd = buf.frame_count;
            }
            release_buffer(&in_data->buf_provider, &buf);
        }
        /* in->read_status is updated by getNextBuffer() also called by
         * in->resampler->resample_from_provider() */
        if (in_data->read_status != 0)
            return in_data->read_status;
        frames_wr += frames_rd;
    }
    return frames_wr;
}

static int btcall_create_resampler(struct alsa_audio_device *adev, int direction) {
    int ret = 0;
    unsigned int in_rate = 0;
    unsigned int out_rate = 0;
    unsigned int ch_cnt = 0;
    struct btcall_data *bt_data = NULL;

    if (direction >= BTCALL_BT_INPUT && direction < BTCALL_RT_MAX) {
        bt_data = adev->bt_data[direction];
    } else {
        ALOGE("btcall direction is out of range : %d", direction);
        return -1;
    }

    in_rate = bt_data->rsp_in_rate;
    out_rate = bt_data->rsp_out_rate;
    ch_cnt = bt_data->config->channels;

    bt_data->buf_provider.get_next_buffer = get_next_buffer;
    bt_data->buf_provider.release_buffer = release_buffer;

    if (in_rate == out_rate) {
        ALOGI("%s, do not use resampler", btcall_rt_map[direction].name);
        return 0;
    }

    ret = create_resampler(in_rate, out_rate, ch_cnt, RESAMPLER_QUALITY_DEFAULT,
                           &bt_data->buf_provider, &bt_data->resampler);

    if (ret != 0) {
        ALOGE("%s, create resampler failed, %d -> %d", btcall_rt_map[direction].name, in_rate,
              out_rate);
        return ret;
    }

    AALOGI("%s, create resampler OK, %d -> %d", btcall_rt_map[direction].name, in_rate, out_rate);

    if (bt_data->resampler) {
        bt_data->resampler->reset(bt_data->resampler);
        bt_data->frames_in = 0;
    }

    return ret;
}

int btcall_init_rx(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data[BTCALL_RT_MAX];
    struct btcall_thread_data *bt_thread_data;

    for (int i = 0; i < BTCALL_RT_MAX; i++) {
        bt_data[i] = calloc(1, sizeof(struct btcall_data));
        if (!bt_data[i]) {
            ALOGE("failed to allocate bt_data memory");
            return -ENOMEM;
        }
        memset(bt_data[i], 0x00, sizeof(struct btcall_data));
        adev->bt_data[i] = bt_data[i];
    }

    bt_thread_data = calloc(1, sizeof(struct btcall_thread_data));
    if (!bt_thread_data) {
        ALOGE("failed to allocate bt_data memory");
        return -ENOMEM;
    }
    memset(bt_thread_data, 0x00, sizeof(struct btcall_thread_data));
    adev->bt_thread_data = bt_thread_data;

    // init mutex and condition
    if (pthread_mutex_init(&bt_thread_data->lock_rx, NULL)) {
        ALOGE("failed to create mutex lock_rx (%d): %m", errno);
        return -EINVAL;
    }

    if (pthread_cond_init(&bt_thread_data->cond_rx, NULL)) {
        ALOGE("failed to create cond(%d): %m", errno);
        return -EINVAL;
    }

    if (!bt_thread_data->thread_rx) {  // define: private data: bt_data
        // pthread_attr_t attr;
        // struct sched_param sched = {0};
        // sched.sched_priority = ANDROID_PRIORITY_AUDIO;
        // pthread_attr_init(&attr);
        // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        // pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        // pthread_attr_setschedparam(&attr, &sched);
        if (pthread_create(&bt_thread_data->thread_rx, NULL, btcall_thread_rx, adev)) {
            ALOGE(
                "%s() pthread_create btinput to codec output "
                "thread failed!!!",
                __func__);
            //   pthread_attr_destroy(&attr);
            return -EINVAL;
        }
        ALOGI("###thread_rx done.");
        // pthread_attr_destroy(&attr);
    }
    return 0;
}

int btcall_init_tx(struct alsa_audio_device *adev) {
    struct btcall_thread_data *bt_thread_data;
    bt_thread_data = adev->bt_thread_data;
    if (!bt_thread_data) {
        ALOGE("failed to allocate bt_data memory");
        return -ENOMEM;
    }

    // init mutex and condition
    if (pthread_mutex_init(&bt_thread_data->lock_tx, NULL)) {
        ALOGE("failed to create mutex lock_tx (%d): %m", errno);
        return -EINVAL;
    }

    if (pthread_cond_init(&bt_thread_data->cond_tx, NULL)) {
        ALOGE("failed to create cond out_cond (%d): %m", errno);
        return -EINVAL;
    }

    if (!bt_thread_data->thread_tx) {  // define: private data: bt_data
        // pthread_attr_t attr;
        // struct sched_param sched = {0};
        // sched.sched_priority = ANDROID_PRIORITY_AUDIO;
        // pthread_attr_init(&attr);
        // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        // pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        // pthread_attr_setschedparam(&attr, &sched);
        if (pthread_create(&bt_thread_data->thread_tx, NULL, btcall_thread_tx, adev)) {
            ALOGE(
                "%s() pthread_create codec input to bt output "
                "thread failed!!!",
                __func__);
            // pthread_attr_destroy(&attr);
            return -EINVAL;
        }
        ALOGI("###thread_tx done.");
        // pthread_attr_destroy(&attr);
    }
    return 0;
}

static int open_bt_inpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_BT_INPUT];

    bt_data->config = &pcm_config_bt_in;
    if ((bt_data->card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
        AALOGE("get card id failed");
        return -EINVAL;
    }
    bt_data->adev = adev;
    bt_data->port = PORT_BT;
    bt_data->direction = BTCALL_BT_INPUT;

    // If there is no configuration information in audio_platform_info.xml,
    // the default configuration will be used.
    sd_platform_get_card_info(adev->platform, BT_CALL_HFP_IN_NAME, &bt_data->card, &bt_data->port,
                              bt_data->config);
    if (!bt_data->pcm) {
        bt_data->pcm = pcm_open_prepare_helper(bt_data->card, bt_data->port, PCM_IN,
                                               PROXY_OPEN_RETRY_COUNT, bt_data->config);

        if (!pcm_is_ready(bt_data->pcm)) {
            ALOGE("%s, cannot open driver: %s", btcall_rt_map[bt_data->direction].name,
                  pcm_get_error(bt_data->pcm));
            bt_data->pcm = NULL;
            return -ENOMEM;
        }
    }

    bt_data->rsp_in_rate = bt_data->config->rate;
    bt_data->rsp_out_rate = HIFI_SAMPLING_RATE;
    if (!adev->platform->info.misc.ahub_en) {
        if (btcall_create_resampler(adev, bt_data->direction)) {
            ALOGE("%s, create resampler failed", btcall_rt_map[bt_data->direction].name);
            return -EINVAL;
        }
    }

    AALOGV("open success");

    return 0;
}

static void close_bt_inpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_BT_INPUT];

    if (!bt_data)
        return;

    if (bt_data->resampler) {
        release_resampler(bt_data->resampler);
        bt_data->resampler = NULL;
    }

    if (bt_data->pcm) {
        pcm_close(bt_data->pcm);
        bt_data->pcm = NULL;
        AALOGV("close success");
    }
}

static int open_codec_outpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_CODEC_OUTPUT];

    if (adev->platform->info.misc.ahub_en) {
        bt_data->config = &pcm_config_codec_out;
        bt_data->config->start_threshold = PLAYBACK_PERIOD_START_THRESHOLD * bt_data->config->period_size;
        bt_data->config->stop_threshold = PLAYBACK_PERIOD_STOP_THRESHOLD * bt_data->config->period_size;
    } else {
        bt_data->config = pcm_main_config;
    }

    if ((bt_data->card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
        AALOGE("get card id failed");
        return -EINVAL;
    }
    bt_data->port = PORT_CODEC;
    bt_data->direction = BTCALL_CODEC_OUTPUT;

#if 0
    if (!bt_data->pcm) {
        bt_data->pcm = pcm_open(bt_data->card, bt_data->port, PCM_OUT, &bt_data->config);

        if (!pcm_is_ready(bt_data->pcm)) {
            ALOGE("%s, cannot open driver: %s", btcall_rt_map[bt_data->direction].name,
                  pcm_get_error(bt_data->pcm));
            bt_data->pcm = NULL;
            return -ENOMEM;
        }
        ALOGW("%s() ", __func__);
    }
#else
    if (!bt_data->pcm) {
        // spk open
        if (!bt_data->bt_out) {
            bt_data->bt_out = (struct alsa_stream_out *)calloc(1, sizeof(struct alsa_stream_out));
        }
        if (bt_data->bt_out) {
            memcpy(bt_data->bt_out, bt_data->out, sizeof(struct alsa_stream_out));
            bt_data->bt_out->bus_address = BT_CALL_BUS_NAME;  // bt call private
            bt_data->bt_out->bus_num = atoi(bt_data->bt_out->bus_address + 3);
        } else {
            AALOGE("bt private out alloc failed!!!");
            return -1;
        }
        if (adev->platform->info.misc.ahub_en) {
            // If there is no configuration information in audio_platform_info.xml,
            // the default configuration will be used.
            sd_platform_get_card_info(adev->platform, bt_data->bt_out->bus_address,
                                             &bt_data->card, &bt_data->port, bt_data->config);
            bt_data->pcm = pcm_open_prepare_helper(
                bt_data->card, bt_data->port, PCM_OUT, PROXY_OPEN_RETRY_COUNT, bt_data->config);
            bt_data->bt_out->pcm = bt_data->pcm;
        } else {
            bt_data->pcm = hal_streamer_open((struct audio_stream *)bt_data->bt_out, bt_data->card,
                                             bt_data->port, PCM_OUT, bt_data->config);
            if (NULL == bt_data->pcm) {
                ALOGE("ext_pcm_open failed !!!");
                return -1;
            }
            bt_data->bt_out->pcm_hdl = bt_data->pcm;
        }
    }
#endif

    AALOGV("open success");

    return 0;
}

static void close_codec_outpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_CODEC_OUTPUT];

    if (!bt_data)
        return;

    if (bt_data->resampler) {
        release_resampler(bt_data->resampler);
        bt_data->resampler = NULL;
    }

    if (bt_data->bt_out->pcm_hdl) {
        hal_streamer_close((struct audio_stream *)bt_data->bt_out, PCM_OUT);
        bt_data->pcm = NULL;
        if (bt_data->bt_out) {
            free(bt_data->bt_out);
            bt_data->bt_out = NULL;
        }
        AALOGV("close success");
    } else if (bt_data->bt_out->pcm) {
        pcm_close(bt_data->bt_out->pcm);
        bt_data->pcm = NULL;
        if (bt_data->bt_out) {
            free(bt_data->bt_out);
            bt_data->bt_out = NULL;
        }
        AALOGV("close success");
    }
}

static int open_codec_inpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_CODEC_INPUT];

    if (adev->platform->info.misc.ahub_en)
        bt_data->config = &pcm_config_codec_in;
    else
        bt_data->config = pcm_main_cap_config;
    if ((bt_data->card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
        AALOGE("get card id failed");
        return -EINVAL;
    }
    bt_data->port = CODEC_DEV;
    bt_data->direction = BTCALL_CODEC_INPUT;

    AALOGI("%s, card id = %d, port id: %d, config rate = %d, channels = %d",
           btcall_rt_map[bt_data->direction].name, bt_data->card, bt_data->port,
           bt_data->config->rate, bt_data->config->channels);

    if (!bt_data->in) {
        // mic open
        if (!bt_data->in) {
            bt_data->in = (struct alsa_stream_in *)calloc(1, sizeof(struct alsa_stream_in));
        }
        if (!bt_data->in)
            return -ENOMEM;
        bt_data->in->config.channels = CHANNEL_2POINT0POINT2;
        bt_data->in->config.rate = SAMPLING_RATE_16K;
        bt_data->in->config.format = PCM_FORMAT_S16_LE;
        bt_data->in->devices = AUDIO_DEVICE_IN_TELEPHONY_RX;
        bt_data->in->dev = adev;
        bt_data->in->pcm_hdl = hal_streamer_open((struct audio_stream *)bt_data->in, bt_data->card,
                                                 bt_data->port, PCM_IN, bt_data->config);
        AALOGI("bt_data: %p, codec in pcm: %p", bt_data, bt_data->in->pcm_hdl);
        if (bt_data->in->pcm_hdl == NULL) {
            AALOGE("%s, cannot open ext pcm", btcall_rt_map[bt_data->direction].name);
            return -ENOMEM;
        }
    }

    bt_data->rsp_in_rate = bt_data->config->rate;
    bt_data->rsp_out_rate = pcm_config_bt_out.rate;
    if (!adev->platform->info.misc.ahub_en) {
        if (btcall_create_resampler(adev, bt_data->direction)) {
            AALOGE("%s, create resampler failed", btcall_rt_map[bt_data->direction].name);
            return -EINVAL;
        }
    }
    AALOGV("open success");

    return 0;
}

static void close_codec_inpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_CODEC_INPUT];
    int32_t ret;
    if (!bt_data)
        return;

    if (bt_data->resampler) {
        release_resampler(bt_data->resampler);
        bt_data->resampler = NULL;
    }

    if (bt_data->pcm) {
        pcm_close(bt_data->pcm);
        bt_data->pcm = NULL;
        AALOGV("close success");
    }
    if (bt_data->in->pcm_hdl) {
        AALOGI("close codec input pcm");
        ret = hal_streamer_close((struct audio_stream *)bt_data->in, PCM_IN);
        if (ret < 0) {
            AALOGE("hal stream close failed: %d", ret);
        }
        bt_data->in->pcm_hdl = NULL;
        if (bt_data->in) {
            free(bt_data->in);
            bt_data->in = NULL;
        }
        AALOGV("close success");
    }
}

static int open_bt_outpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_BT_OUTPUT];

    bt_data->config = &pcm_config_bt_out;
    bt_data->direction = BTCALL_BT_OUTPUT;

    if ((bt_data->card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
        AALOGE("get card id failed");
        return -EINVAL;
    }
    bt_data->port = PORT_BT;

    // If there is no configuration information in audio_platform_info.xml,
    // the default configuration will be used.
    sd_platform_get_card_info(adev->platform, BT_CALL_HFP_OUT_NAME, &bt_data->card, &bt_data->port,
                              bt_data->config);

    if (!bt_data->pcm) {
        bt_data->pcm = pcm_open_prepare_helper(bt_data->card, bt_data->port, PCM_OUT,
                                               PROXY_OPEN_RETRY_COUNT, bt_data->config);

        if (!pcm_is_ready(bt_data->pcm)) {
            ALOGE("%s, cannot open driver: %s", btcall_rt_map[bt_data->direction].name,
                  pcm_get_error(bt_data->pcm));
            bt_data->pcm = NULL;
            return -ENOMEM;
        }
    }

    AALOGV("open success");

    return 0;
}

static void close_bt_outpcm(struct alsa_audio_device *adev) {
    struct btcall_data *bt_data = adev->bt_data[BTCALL_BT_OUTPUT];

    if (!bt_data)
        return;

    if (bt_data->resampler) {
        release_resampler(bt_data->resampler);
        bt_data->resampler = NULL;
    }

    if (bt_data->pcm) {
        pcm_close(bt_data->pcm);
        bt_data->pcm = NULL;
        AALOGV("close success");
    }
}

int btcall_open(struct alsa_audio_device *adev) {
    int size = 0;
    int ret = 0;
    bool ecnr_en = true;
    char prop_value[PROPERTY_VALUE_MAX];
    struct btcall_thread_data *bt_thread_data = adev->bt_thread_data;

    property_get("media.audio.btcall_dump", prop_value, NULL);
    if (!strcmp(prop_value, "enable")) {
        rec_in_fp = fopen("/data/recv_in.pcm", "wb+");
        mic_in_fp = fopen("/data/mic_in.pcm", "wb+");
        mic_out_fp = fopen("/data/mic_out.pcm", "wb+");
        ref_in_fp = fopen("/data/ref_in.pcm", "wb+");
    }

    if (adev->platform->info.misc.sw_ecnr) {
        ecnr_en = true;
        property_get("media.audio.btcall_ecnr", prop_value, NULL);
        if (!strcmp(prop_value, "disable")) {
            ecnr_en = false;
        }
    } else {
        ecnr_en = false;
    }

    if (ecnr_en && adev->ecnr_ops) {
        adev->ecnr_hdl = adev->ecnr_ops->ecnr_init(&param);
        AALOGI("bt call ECNR handler: %p", adev->ecnr_hdl);
    } else {
        AALOGI("Warning bt call not support ECNR");
    }

    if (!adev->bt_data[BTCALL_BT_INPUT]->pcm) {
        ret = open_bt_inpcm(adev);
        if (ret != 0) {
            ALOGE("open bt inpcm pcm failed.");
            return -EINVAL;
        }
    }

    if (!adev->bt_data[BTCALL_CODEC_OUTPUT]->pcm) {
        ret = open_codec_outpcm(adev);
        if (ret != 0) {
            ALOGE("open output pcm failed.");
            return -EINVAL;
        }
    }

    if (!adev->bt_data[BTCALL_CODEC_INPUT]->pcm) {
        ret = open_codec_inpcm(adev);
        if (ret != 0) {
            ALOGE("open codec_inpcm pcm failed.");
            return -EINVAL;
        }
    }

    if (!adev->bt_data[BTCALL_BT_OUTPUT]->pcm) {
        ret = open_bt_outpcm(adev);
        if (ret != 0) {
            ALOGE("open bt output pcm failed.");
            return -EINVAL;
        }
    }

    if (!bt_thread_data->buffer_rx) {
        size = ((adev->bt_data[BTCALL_BT_INPUT]->config->period_size + 15) / 16) * 16;
        size *= get_input_frame_size(adev->bt_data[BTCALL_BT_INPUT]);
        int multiple = adev->bt_data[BTCALL_BT_INPUT]->rsp_out_rate /
                               adev->bt_data[BTCALL_BT_INPUT]->rsp_in_rate;
        bt_thread_data->buffer_rx = malloc(size * multiple);
        if (!bt_thread_data->buffer_rx) {
            ALOGE("malloc failed, out of memory.");
            return -ENOMEM;
        }
        bt_thread_data->buffer_rxsize = size;
    }

    if (!bt_thread_data->buffer_tx) {
        size = ((adev->bt_data[BTCALL_CODEC_INPUT]->config->period_size + 15) / 16) * 16 *
               get_input_frame_size(adev->bt_data[BTCALL_CODEC_INPUT]);

        bt_thread_data->buffer_tx = malloc(size);
        if (!bt_thread_data->buffer_tx) {
            ALOGE("malloc failed, out of memory.");
            return -ENOMEM;
        }
        bt_thread_data->buffer_txsize = size;
    }

    if (!bt_thread_data->rsp_out_buffer_tx) {
        size = bt_thread_data->buffer_txsize * 8;
        bt_thread_data->rsp_out_buffer_tx = malloc(size); /* todo: allow for reallocing */
        if (!bt_thread_data->rsp_out_buffer_tx) {
            ALOGE("malloc failed, out of memory.");
            return -ENOMEM;
        }

        memset(bt_thread_data->rsp_out_buffer_tx, 0x00, size);
    }

    if (!bt_thread_data->rsp_out_buffer_rx) {
        size = bt_thread_data->buffer_rxsize * 8;
        bt_thread_data->rsp_out_buffer_rx = malloc(size); /* todo: allow for reallocing */
        if (!bt_thread_data->rsp_out_buffer_rx) {
            ALOGE("malloc failed, out of memory.");
            return -ENOMEM;
        }

        memset(bt_thread_data->rsp_out_buffer_rx, 0x00, size);
    }

    for (int i = 0; i < BTCALL_RT_MAX; i++) {
        if (!adev->bt_data[i]->buffer) {
            adev->bt_data[i]->buffer = malloc(adev->bt_data[i]->config->period_size *
                                              get_input_frame_size(adev->bt_data[i]) * 8);

            if (!adev->bt_data[i]->buffer) {
                ALOGE("malloc failed, out of memory.");
                return -ENOMEM;
            }
        }
    }

    ALOGI("btcall open success");

    return 0;
}

int btcall_close(struct alsa_audio_device *adev) {
    struct btcall_thread_data *bt_thread_data = adev->bt_thread_data;

    if (adev->bt_data[BTCALL_BT_INPUT]->pcm)
        close_bt_inpcm(adev);

    if (adev->bt_data[BTCALL_CODEC_OUTPUT]->pcm)
        close_codec_outpcm(adev);

    if (adev->bt_data[BTCALL_CODEC_INPUT]->in->pcm_hdl) {
        close_codec_inpcm(adev);
        free(adev->bt_data[BTCALL_CODEC_INPUT]->in);
        adev->bt_data[BTCALL_CODEC_INPUT]->in = NULL;
    }

    if (adev->bt_data[BTCALL_BT_OUTPUT]->pcm)
        close_bt_outpcm(adev);

    if (!adev->platform->info.misc.ahub_en) {
        audio_remote_stop(adev, PHONE_CAPTURE_FROM_MAIN_MIC_48K);
        audio_remote_stop(adev, PHONE_PLAYBACK_TO_MAIN_SPK_48K);
    }

    if (bt_thread_data->buffer_rx) {
        free(bt_thread_data->buffer_rx);
        bt_thread_data->buffer_rx = NULL;
        bt_thread_data->buffer_rxsize = 0;
    }
    if (bt_thread_data->buffer_tx) {
        free(bt_thread_data->buffer_tx);
        bt_thread_data->buffer_tx = NULL;
        bt_thread_data->buffer_txsize = 0;
    }

    if (bt_thread_data->rsp_out_buffer_tx) {
        free(bt_thread_data->rsp_out_buffer_tx);
        bt_thread_data->rsp_out_buffer_tx = NULL;
    }
    if (bt_thread_data->rsp_out_buffer_rx) {
        free(bt_thread_data->rsp_out_buffer_rx);
        bt_thread_data->rsp_out_buffer_rx = NULL;
    }

    for (int i = 0; i < BTCALL_RT_MAX; i++) {
        if (adev->bt_data[i]->buffer) {
            free(adev->bt_data[i]->buffer);
            adev->bt_data[i]->buffer = NULL;
        }
    }

    ALOGI("btcall close success");

    return 0;
}

// for bt call read from bt in pcm, send data to codec output
void *btcall_thread_rx(void *data) {
    struct alsa_audio_device *adev = (struct alsa_audio_device *)data;
    struct btcall_thread_data *bt_thread_data = adev->bt_thread_data;
    int ret = 0;
    int size = 0;
    size_t in_frames = 0;
    void *buf;
    int multiple = 1;
    while (1) {
        pthread_mutex_lock(&bt_thread_data->lock_rx);
        if (!bt_thread_data->start_work_rx && !bt_thread_data->exit_work_rx) {
            AALOGI("wait in line = %d", __LINE__);
            // audio_remote_stop(adev, PHONE_PLAYBACK_TO_MAIN_SPK_48K);
            bt_thread_data->is_running_rx = 0;
            pthread_cond_wait(&bt_thread_data->cond_rx, &bt_thread_data->lock_rx);

            if (!adev->platform->info.misc.ahub_en) {
                audio_remote_start(adev, PHONE_PLAYBACK_TO_MAIN_SPK_48K,
                                   PHONE_PATH_FIXED_VOL /* fixed volume, adjust by sw gain*/);
            }
            multiple = adev->bt_data[BTCALL_BT_INPUT]->rsp_out_rate /
                       adev->bt_data[BTCALL_BT_INPUT]->rsp_in_rate;
            AALOGI("start.");
            bt_thread_data->is_running_rx = 1;
        }

        if (bt_thread_data->exit_work_rx) {
            AALOGI("exit");
            pthread_mutex_unlock(&bt_thread_data->lock_rx);
            break;
        }

        if (adev->bt_data[BTCALL_BT_INPUT] && adev->bt_data[BTCALL_CODEC_OUTPUT] &&
            (bt_thread_data->buffer_rx != NULL)) {
            in_frames = bt_thread_data->buffer_rxsize /
                        get_input_frame_size(adev->bt_data[BTCALL_BT_INPUT]);
            // get form bt pcm in
            if (adev->bt_data[BTCALL_BT_INPUT]->resampler != NULL) {  // SRC
                if (adev->bt_data[BTCALL_BT_INPUT]->rsp_out_rate >
                    adev->bt_data[BTCALL_BT_INPUT]->rsp_in_rate) {
                    in_frames *= multiple;
                    ret = read_frames(adev->bt_data[BTCALL_BT_INPUT], bt_thread_data->buffer_rx,
                                      in_frames);
                    size = in_frames * get_input_frame_size(adev->bt_data[BTCALL_BT_INPUT]);
                    buf = bt_thread_data->buffer_rx;
                    // dump_out_data(tmp, size, NULL, 2);
                }
            } else {
                ret = pcm_read(adev->bt_data[BTCALL_BT_INPUT]->pcm, bt_thread_data->buffer_rx,
                               bt_thread_data->buffer_rxsize);
                size = bt_thread_data->buffer_rxsize;
                buf = (void *)bt_thread_data->buffer_rx;
            }
            pthread_mutex_unlock(&bt_thread_data->lock_rx);
            if (ret < 0)
                AALOGE("bt in, pcm_read failed, ret : %d\n", ret);
            ret = 0;

            audio_extra_sw_gain_16(hfp_get_amplitude_ratio(), buf, size);

            if (adev->bt_data[BTCALL_CODEC_OUTPUT]->bt_out->pcm_hdl) {
                ret = hal_streamer_write(adev->bt_data[BTCALL_CODEC_OUTPUT]->bt_out, buf, size);
                if (ret < 0)
                    AALOGE("codec out, pcm_write failed");
            } else if (adev->bt_data[BTCALL_CODEC_OUTPUT]->bt_out->pcm) {
                ret = pcm_write(adev->bt_data[BTCALL_CODEC_OUTPUT]->bt_out->pcm, buf, size);
                if (ret)
                    AALOGE("codec out, pcm_write failed");
            }
        }
    }

    bt_thread_data->exit_work_rx = 1;
    bt_thread_data->start_work_rx = 0;
    return NULL;
}

// for bt call read from codec mic ,send data to bt pcm
void *btcall_thread_tx(void *data) {
    AALOGI("enter");
    int ret = 0;
    int size = 0;
    size_t in_frames = 0;
    size_t out_frames = 0;
    int64_t curr_timestamp = 0;
    void *buf;

    struct alsa_audio_device *adev = (struct alsa_audio_device *)data;
    struct btcall_thread_data *bt_thread_data = adev->bt_thread_data;

    int frame_sz = AUDIO_FMT_2_BYTES(
        pcm_main_cap_config->format);  // pcm_format_to_bits(pcm_main_cap_config->format) / 8;
    char tx_processed[TMP_BUF_SZ];
    char ref_in[TMP_BUF_SZ];
    int ref_in_frames;
    char mic_in[TMP_BUF_SZ];
    int mic_in_frames;
    int32_t tx_processed_len = 0;

    while (1) {
        pthread_mutex_lock(&bt_thread_data->lock_tx);
        if (!bt_thread_data->start_work_tx && !bt_thread_data->exit_work_tx) {
            AALOGI("wait in line = %d", __LINE__);
            bt_thread_data->is_running_tx = 0;
            pthread_cond_wait(&bt_thread_data->cond_tx, &bt_thread_data->lock_tx);
            if (!adev->platform->info.misc.ahub_en) {
                audio_remote_start(adev, PHONE_CAPTURE_FROM_MAIN_MIC_48K, PHONE_PATH_FIXED_VOL);
            }
            AALOGI("start.");
            bt_thread_data->is_running_tx = 1;
        }
        if (bt_thread_data->exit_work_tx) {
            AALOGI("exit");
            pthread_mutex_unlock(&bt_thread_data->lock_tx);
            break;
        }
        pthread_mutex_unlock(&bt_thread_data->lock_tx);
        if (adev->bt_data[BTCALL_CODEC_INPUT]->in->pcm_hdl &&
            adev->bt_data[BTCALL_BT_OUTPUT]->pcm && (bt_thread_data->buffer_tx != NULL)) {
            in_frames = bt_thread_data->buffer_txsize /
                        get_input_frame_size(adev->bt_data[BTCALL_CODEC_INPUT]);
            out_frames = 8 * in_frames;

            if (adev->bt_data[BTCALL_CODEC_INPUT]->resampler != NULL) {
                ret = read_frames(adev->bt_data[BTCALL_CODEC_INPUT], bt_thread_data->buffer_tx,
                                  in_frames);
            } else if (adev->bt_data[BTCALL_CODEC_INPUT]->in) {
                // codec mic read
                ret = hal_streamer_read(adev->bt_data[BTCALL_CODEC_INPUT]->in,
                                        bt_thread_data->buffer_tx, bt_thread_data->buffer_txsize);
            }
            if (ret < 0)
                ALOGE("codec in, pcm_read failed, ret: %d", ret);
            ret = 0;
            // dump_out_data(bt_thread_data->buffer_tx,
            // 	      bt_thread_data->buffer_txsize, NULL, 1);
            size = bt_thread_data->buffer_txsize;
            buf = (void *)bt_thread_data->buffer_tx;

            if (adev->mic_mute) {
                memset(buf, 0x00, size);
            }

            // got mic in and reference in
            int32_t ecnr = adev->ecnr_hdl == NULL ? -1 : *(int32_t *)adev->ecnr_hdl;
            if (ecnr >= 0)  // ecnr enable
            {
                /* 4ch -> 2ch mic & 2ch ref */
                // get mic in
                mic_in_frames = audio_extra_select_slot_16(
                    (int16_t *)mic_in, param.mic_in_cfg.ch /*ch*/, buf,
                    pcm_main_cap_config->channels /*ch*/, size / frame_sz /*s16*/);
                // get ref in
                ref_in_frames = audio_extra_select_slot_16(
                    (int16_t *)ref_in, param.ref_in_cfg.ch /*ch*/,
                    (int16_t *)buf + param.mic_in_cfg.ch /* base */,
                    pcm_main_cap_config->channels /* ch */, size / frame_sz /*s16*/);
                FP_WRITE(ref_in, ref_in_frames * frame_sz, ref_in_fp);
                FP_WRITE(mic_in, mic_in_frames * frame_sz, mic_in_fp);
                // AALOGI("sent ref begin");
                adev->ecnr_ops->ecnr_ref_in(ecnr, ref_in,
                                            ref_in_frames / param.ref_in_cfg.ch /*ch*/,
                                            (void *)curr_timestamp);
                // AALOGI("sent ref end");
                // AALOGI("sent micin begin");
                adev->ecnr_ops->ecnr_process(
                    ecnr, mic_in, mic_in_frames / param.mic_in_cfg.ch /*ch*/, tx_processed,
                    &tx_processed_len, (void *)curr_timestamp);
                // AALOGI("sent micin end");
            } else {  // ecnr disable
                tx_processed_len = audio_extra_select_slot_16(
                    (int16_t *)tx_processed, param.mic_out_cfg.ch /* ch */, buf,
                    pcm_main_cap_config->channels /* ch */, size / frame_sz /* s16 */);
            }
            if (adev->bt_data[BTCALL_BT_OUTPUT]->pcm) {
                FP_WRITE(tx_processed, tx_processed_len * frame_sz, mic_out_fp);
                ret = pcm_write(adev->bt_data[BTCALL_BT_OUTPUT]->pcm, tx_processed,
                                tx_processed_len * frame_sz);
                if (ret) {
                    ALOGE("bt out, pcm_write failed");
                }
            }
        }
    }

    bt_thread_data->exit_work_tx = 1;
    bt_thread_data->start_work_tx = 0;
    return NULL;
}
