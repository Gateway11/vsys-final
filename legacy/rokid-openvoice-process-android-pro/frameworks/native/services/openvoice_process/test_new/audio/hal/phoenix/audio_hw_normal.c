/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "audio_hw_normal"
// #define LOG_NDEBUG 0

#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <log/log.h>

#include <hardware/audio.h>
#include <hardware/audio_alsaops.h>
#include <hardware/hardware.h>
#include <system/audio.h>

#include "audio_hw.h"
#include <audio_extra.h>

struct pcm_config main_config_ref = {
    .channels = CHANNEL_STEREO,
    .rate = HIFI_SAMPLING_RATE,
    .period_size = (HIFI_SAMPLING_RATE * OUT_PERIOD_MS) / 1000,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config *pcm_main_config = &main_config_ref;
struct pcm_config *pcm_main_cap_config = &main_config_ref;

static size_t samples_per_milliseconds(size_t milliseconds, uint32_t sample_rate,
                                       size_t channel_count) {
    return milliseconds * sample_rate * channel_count / 1000;
}

static uint32_t normal_out_get_sample_rate(const struct audio_stream *stream) {
    const struct alsa_stream_out *out = (const struct alsa_stream_out *)stream;

    ALOGV("out_get_sample_rate: %u", out->config.rate);
    return out->config.rate;
}

static int normal_out_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
    // struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    // out->config.rate = rate;

    ALOGV("out_set_sample_rate: %d", rate);
    ALOGD("Fix sample rate to 48k, don't support set rate");

    return -ENOSYS;
}

static size_t normal_out_get_buffer_size(const struct audio_stream *stream) {
    const struct alsa_stream_out *out = (const struct alsa_stream_out *)stream;
    size_t buffer_size =
        samples_per_milliseconds(OUT_PERIOD_MS, out->config.rate, out->config.channels) *
        audio_stream_out_frame_size(&out->stream);

    ALOGV("out_get_buffer_size: %zu", buffer_size);
    return buffer_size;
}

static audio_channel_mask_t normal_out_get_channels(const struct audio_stream *stream) {
    const struct alsa_stream_out *out = (const struct alsa_stream_out *)stream;
    return audio_channel_out_mask_from_count(out->config.channels);
}

static audio_format_t normal_out_get_format(const struct audio_stream *stream) {
    const struct alsa_stream_out *out = (const struct alsa_stream_out *)stream;
    return audio_format_from_pcm_format(out->config.format);
}

static int normal_out_set_format(struct audio_stream *stream, audio_format_t format) {
    // struct alsa_stream_out *out = (struct alsa_stream_out *)stream;

    ALOGD("Fix rormat to PCM_FORMAT_S16_LE, don't support set format");
    return -ENOSYS;
}

/* must be called with hw device and output stream mutexes locked */
static int normal_do_output_standby(struct alsa_stream_out *out) {
    struct alsa_audio_device *adev = out->dev;

    if (!out->standby) {
        if (out->pcm) {
            pcm_close(out->pcm);
            out->pcm = NULL;
        }

        if (out->resampler) {
            release_resampler(out->resampler);
            out->resampler = NULL;
        }

        adev->active_output = 0;

        out->standby = 1;
    }
    return 0;
}

static int normal_out_standby(struct audio_stream *stream) {
    ALOGV("out_standby");
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;

    pthread_mutex_lock(&out->dev->lock);
    pthread_mutex_lock(&out->lock);
    normal_do_output_standby(out);
    pthread_mutex_unlock(&out->lock);
    pthread_mutex_unlock(&out->dev->lock);
    return 0;
}

static int normal_out_dump(const struct audio_stream *stream, int fd) {
    ALOGV("out_dump");
    return 0;
}

static int normal_out_set_parameters(struct audio_stream *stream, const char *kvpairs) {
    ALOGV("out_set_parameters");
    return 0;
}

static char *normal_out_get_parameters(const struct audio_stream *stream, const char *keys) {
    ALOGV("out_get_parameters");
    return strdup("");
}

static uint32_t normal_out_get_latency(const struct audio_stream_out *stream) {
    ALOGV("out_get_latency");
    return OUT_PERIOD_MS;
}

static int normal_out_set_volume(struct audio_stream_out *stream, float left, float right) {
    ALOGV("out_set_volume: Left:%f Right:%f", left, right);
    return 0;
}

/* must be called with hw device and output stream mutexes locked */

static int normal_start_output_stream(struct alsa_stream_out *out) {
    struct alsa_audio_device *adev = out->dev;
    int card = 1;
    int port = PORT_CODEC;

    if ((card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_PRIMARY)) < 0) {
        return -ENODEV;
    }

    adev->active_output = out;
    ALOGI("Playing sample: %u ch, %u hz, %s format\n", out->config.channels, out->config.rate,
          audio_extra_get_alsa_format_name(out->config.format));

    out->pcm = pcm_open(card, port, PCM_OUT, &out->config);
    if (!pcm_is_ready(out->pcm)) {
        ALOGE("cannot open pcm driver: %s", pcm_get_error(out->pcm));
        pcm_close(out->pcm);
        out->pcm = NULL;
        adev->active_output = NULL;
        return -ENOMEM;
    }

    return 0;
}

static ssize_t normal_out_write(struct audio_stream_out *stream, const void *buffer, size_t bytes) {
    ALOGV("out_write: bytes: %zu", bytes);

    int ret = 0;
    struct alsa_stream_out *out = (struct alsa_stream_out *)stream;
    struct alsa_audio_device *adev = out->dev;

    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&out->lock);
    if (out->standby) {
        ret = normal_start_output_stream(out);
        if (ret != 0) {
            pthread_mutex_unlock(&adev->lock);
            AALOGE("Failed to start output stream, err %d, (%s)", ret, strerror(ret));
            goto exit;
        }
        out->standby = 0;
    }
    pthread_mutex_unlock(&adev->lock);

    ret = pcm_write(out->pcm, buffer, bytes);

exit:
    pthread_mutex_unlock(&out->lock);
    if (ret != 0) {
        ALOGE("##############write fail, reopen it. ret = %d #######################", ret);
        normal_out_standby((struct audio_stream *)out);
        usleep((int64_t)bytes * 1000000 / audio_stream_out_frame_size(stream) /
               normal_out_get_sample_rate(&stream->common));
    }
    return bytes;
}

static int normal_out_get_render_position(const struct audio_stream_out *stream,
                                          uint32_t *dsp_frames) {
    *dsp_frames = 0;
    ALOGV("out_get_render_position: dsp_frames: %p", dsp_frames);
    return -EINVAL;
}

static int normal_out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
    ALOGV("out_add_audio_effect: %p", effect);
    return 0;
}

static int normal_out_remove_audio_effect(const struct audio_stream *stream,
                                          effect_handle_t effect) {
    ALOGV("out_remove_audio_effect: %p", effect);
    return 0;
}

static int normal_out_get_next_write_timestamp(const struct audio_stream_out *stream,
                                               int64_t *timestamp) {
    *timestamp = 0;
    ALOGV("out_get_next_write_timestamp: %ld", (long int)(*timestamp));
    return -EINVAL;
}

/** audio_stream_in implementation **/
static uint32_t normal_in_get_sample_rate(const struct audio_stream *stream) {
    const struct alsa_stream_in *in = (const struct alsa_stream_in *)stream;
    struct alsa_audio_device *adev = in->dev;
    uint32_t rate = 0;

    if (adev->platform->info.misc.sw_in_src)
        rate = in->requested_rate;
    else
        rate = in->config.rate;

    ALOGV("in_get_sample_rate: %u", rate);
    return rate;
}

static int normal_in_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
    // struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    // in->sample_rate = rate;

    ALOGV("in_set_sample_rate: %u", rate);

    return -ENOSYS;
}

static size_t normal_in_get_buffer_size(const struct audio_stream *stream) {
    const struct alsa_stream_in *in = (const struct alsa_stream_in *)stream;
    struct alsa_audio_device *adev = in->dev;
    uint32_t rate = 0;

    if (adev->platform->info.misc.sw_in_src)
        rate = in->requested_rate;
    else
        rate = in->config.rate;

    size_t buffer_size = samples_per_milliseconds(IN_PERIOD_MS, rate, in->config.channels) *
                         audio_stream_in_frame_size(&in->stream);

    ALOGV("in_get_buffer_size: %zu", buffer_size);
    return buffer_size;
}

static audio_channel_mask_t normal_in_get_channels(const struct audio_stream *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    ALOGV("%s, in->config.channels:%d, channel_mask:%#x", __func__, in->config.channels,
          audio_channel_in_mask_from_count(in->config.channels));
    return audio_channel_in_mask_from_count(in->config.channels);
}

static audio_format_t normal_in_get_format(const struct audio_stream *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    ALOGV("%s, pcm format:%d, audio format:%#x", __func__, in->config.format,
          audio_format_from_pcm_format(in->config.format));
    return audio_format_from_pcm_format(in->config.format);
}

static int normal_in_set_format(struct audio_stream *stream, audio_format_t format) {
    DEBUG_FUNC_PRT
    return -ENOSYS;
}

static int normal_do_in_standby(struct alsa_stream_in *in) {
    struct alsa_audio_device *adev = in->dev;

    if (!in->standby) {
        if (in->pcm) {
            pcm_close(in->pcm);
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

static int normal_in_standby(struct audio_stream *stream) {
    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;

    pthread_mutex_lock(&in->dev->lock);
    pthread_mutex_lock(&in->lock);
    normal_do_in_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->dev->lock);

    return 0;
}

static int normal_in_dump(const struct audio_stream *stream, int fd) {
    return 0;
}

static int normal_in_set_parameters(struct audio_stream *stream, const char *kvpairs) {
    return 0;
}

static char *normal_in_get_parameters(const struct audio_stream *stream, const char *keys) {
    return strdup("");
}

static int normal_in_set_gain(struct audio_stream_in *stream, float gain) {
    return 0;
}

/* must be called with hw device and input stream mutexes locked */
static int normal_start_input_stream(struct alsa_stream_in *in) {
    struct alsa_audio_device *adev = in->dev;

    adev->active_input = in;

    int card = 0;

    if ((card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_CAPTURE)) < 0) {
        return -ENODEV;
    }

    ALOGI("capturing sample: %u ch, %u hz, %s format\n", in->config.channels, in->config.rate,
          audio_extra_get_alsa_format_name(in->config.format));
    in->pcm = pcm_open(card, PORT_CODEC, PCM_IN, &in->config);

    if (!pcm_is_ready(in->pcm)) {
        ALOGE("cannot open pcm_in driver: %s", pcm_get_error(in->pcm));
        pcm_close(in->pcm);
        adev->active_input = NULL;
        return -ENOMEM;
    }
    if (adev->platform->info.misc.sw_in_src) {
        int ret = 0;
        /* if no supported sample rate is available, use the resampler */
        if (in->requested_rate != in->config.rate) {
            in->buf_provider.get_next_buffer = NULL;  // get_next_buffer;
            in->buf_provider.release_buffer = NULL;   // release_buffer;

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

    adev->active_input = in;

    return 0;
}

static ssize_t normal_in_read(struct audio_stream_in *stream, void *buffer, size_t bytes) {
    size_t frames_rq = 0;

    struct alsa_stream_in *in = (struct alsa_stream_in *)stream;
    // struct alsa_audio_device *adev = in->dev;
    ALOGV("in_read: bytes %zu", bytes);
    int ret;

    pthread_mutex_lock(&in->lock);

    /* Standby session */
    if (in->standby) {
        ret = normal_start_input_stream(in);
        if (ret != 0) {
            AALOGE("Failed to start input stream, err %d, (%s)", ret, strerror(ret));
            goto exit;
        }
        in->standby = false;
    }
    frames_rq = bytes / audio_stream_in_frame_size(stream);

    if (in->resampler != NULL) {
        // ret = read_frames(in, buffer, frames_rq);
    } else {
        ret = pcm_read(in->pcm, buffer, bytes);
    }

    if (ret < 0) {
        AALOGE("Failed to read w/err %d, (%s)", ret, strerror(ret));
    }

    // int size = 0;
    // dump_out_data(buffer, bytes, NULL, 9);
exit:
    pthread_mutex_unlock(&in->lock);
    if (ret < 0) {
        normal_in_standby((struct audio_stream *)in);
        // clear return data
        memset(buffer, 0x0, bytes);
        usleep(bytes * 1000000 / audio_stream_in_frame_size(stream) /
               normal_in_get_sample_rate((const struct audio_stream *)in));
    }
    return bytes;
}

static uint32_t normal_in_get_input_frames_lost(struct audio_stream_in *stream) {
    return 0;
}

static int normal_in_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect) {
    return 0;
}

static int normal_in_remove_audio_effect(const struct audio_stream *stream,
                                         effect_handle_t effect) {
    return 0;
}

struct pcm *pcm_open_prepare_helper(unsigned int snd_card, unsigned int pcm_device_id,
                                    unsigned int flags, unsigned int pcm_open_retry_count,
                                    struct pcm_config *config) {
    return NULL;
}

static int adev_normal_open_output_stream(struct audio_hw_device *dev, audio_io_handle_t handle,
                                          audio_devices_t devices, audio_output_flags_t flags,
                                          struct audio_config *config,
                                          struct audio_stream_out **stream_out,
                                          const char *address __unused) {
    ALOGV("adev_normal_open_output_stream...");
    struct alsa_audio_device *ladev = (struct alsa_audio_device *)dev;

    *stream_out = NULL;
    struct alsa_stream_out *out =
        (struct alsa_stream_out *)calloc(1, sizeof(struct alsa_stream_out));
    if (!out)
        return -ENOMEM;

    out->stream.common.get_sample_rate = normal_out_get_sample_rate;
    out->stream.common.set_sample_rate = normal_out_set_sample_rate;
    out->stream.common.get_buffer_size = normal_out_get_buffer_size;
    out->stream.common.get_channels = normal_out_get_channels;
    out->stream.common.get_format = normal_out_get_format;
    out->stream.common.set_format = normal_out_set_format;
    out->stream.common.standby = normal_out_standby;
    out->stream.common.dump = normal_out_dump;
    out->stream.common.set_parameters = normal_out_set_parameters;
    out->stream.common.get_parameters = normal_out_get_parameters;
    out->stream.common.add_audio_effect = normal_out_add_audio_effect;
    out->stream.common.remove_audio_effect = normal_out_remove_audio_effect;
    out->stream.get_latency = normal_out_get_latency;
    out->stream.set_volume = normal_out_set_volume;
    out->stream.write = normal_out_write;
    out->stream.get_render_position = normal_out_get_render_position;
    out->stream.get_next_write_timestamp = normal_out_get_next_write_timestamp;
    out->config.rate = config->sample_rate;
    out->dev = ladev;
    out->standby = true;
    if (out->config.rate == 0)
        out->config.rate = HIFI_SAMPLING_RATE;

    if (config->channel_mask == AUDIO_CHANNEL_NONE)
        out->config.channels = CHANNEL_STEREO;
    else
        out->config.channels = audio_channel_count_from_out_mask(config->channel_mask);

    if (config->format == AUDIO_FORMAT_DEFAULT)
        out->config.format = pcm_format_from_audio_format(HIFI_AUDIO_FORMAT);
    else
        out->config.format = pcm_format_from_audio_format(config->format);

    out->config.period_size =
        samples_per_milliseconds(OUT_PERIOD_MS, out->config.rate, out->config.channels);
    out->config.period_count = PLAYBACK_PERIOD_COUNT;

    ALOGD("adev_normal_open_output_stream: sample_rate: %d, channels: %d, format: %s, frames: %d",
          out->config.rate, out->config.channels,
          audio_extra_get_alsa_format_name(out->config.format), out->config.period_size);
    *stream_out = &out->stream;
    return 0;
}

static void adev_normal_close_output_stream(struct audio_hw_device *dev,
                                            struct audio_stream_out *stream) {
    ALOGV("adev_normal_close_output_stream...");
    free(stream);
}

static int adev_normal_set_parameters(struct audio_hw_device *dev, const char *kvpairs) {
    ALOGV("adev_normal_set_parameters");
    return -ENOSYS;
}

static char *adev_normal_get_parameters(const struct audio_hw_device *dev, const char *keys) {
    ALOGV("adev_normal_get_parameters");
    return strdup("");
}

static int adev_normal_init_check(const struct audio_hw_device *dev) {
    ALOGV("adev_normal_init_check");
    return 0;
}

static int adev_normal_set_voice_volume(struct audio_hw_device *dev, float volume) {
    ALOGV("adev_normal_set_voice_volume: %f", volume);
    return -ENOSYS;
}

static int adev_normal_set_master_volume(struct audio_hw_device *dev, float volume) {
    ALOGV("adev_normal_set_master_volume: %f", volume);
    return -ENOSYS;
}

static int adev_normal_get_master_volume(struct audio_hw_device *dev, float *volume) {
    ALOGV("adev_normal_get_master_volume: %f", *volume);
    return -ENOSYS;
}

static int adev_normal_set_master_mute(struct audio_hw_device *dev, bool muted) {
    ALOGV("adev_normal_set_master_mute: %d", muted);
    return -ENOSYS;
}

static int adev_normal_get_master_mute(struct audio_hw_device *dev, bool *muted) {
    ALOGV("adev_normal_get_master_mute: %d", *muted);
    return -ENOSYS;
}

static int adev_normal_set_mode(struct audio_hw_device *dev, audio_mode_t mode) {
    ALOGV("adev_normal_set_mode: %d", mode);
    return 0;
}

static int adev_normal_set_mic_mute(struct audio_hw_device *dev, bool state) {
    ALOGV("adev_normal_set_mic_mute: %d", state);
    return -ENOSYS;
}

static int adev_normal_get_mic_mute(const struct audio_hw_device *dev, bool *state) {
    ALOGV("adev_normal_get_mic_mute");
    return -ENOSYS;
}

static size_t adev_normal_get_input_buffer_size(const struct audio_hw_device *dev,
                                                const struct audio_config *config) {
    DEBUG_FUNC_PRT
    int32_t ch = audio_channel_count_from_in_mask(config->channel_mask);

    /* Audioflinger expects audio buffers to be multiple of 16 frames */

    size_t size = (config->sample_rate * IN_PERIOD_MS / 1000 + 15) / 16 * 16;

    size *= sizeof(short) * ch;
    ALOGV("adev_normal_get_input_buffer_size: %zu, ch: %d, ch_mask: %#x", size, ch,
          config->channel_mask);
    return size;
}

//#define FIX_CAPTURE_SAMPLE_RATE
static int adev_normal_open_input_stream(struct audio_hw_device *dev, audio_io_handle_t handle,
                                         audio_devices_t devices, struct audio_config *config,
                                         struct audio_stream_in **stream_in,
                                         audio_input_flags_t flags __unused,
                                         const char *address __unused,
                                         audio_source_t source __unused) {
    ALOGI(
        "%s: enter: format(%#x) sample_rate(%d) channel_mask(%#x) "
        "devices(%#x) flags(%#x)",
        __func__, config->format, config->sample_rate, config->channel_mask, devices, flags);

    *stream_in = NULL;
    struct alsa_audio_device *ladev = (struct alsa_audio_device *)dev;
    struct alsa_stream_in *in = (struct alsa_stream_in *)calloc(1, sizeof(struct alsa_stream_in));
    if (!in)
        return -ENOMEM;

    in->stream.common.get_sample_rate = normal_in_get_sample_rate;
    in->stream.common.set_sample_rate = normal_in_set_sample_rate;
    in->stream.common.get_buffer_size = normal_in_get_buffer_size;
    in->stream.common.get_channels = normal_in_get_channels;
    in->stream.common.get_format = normal_in_get_format;
    in->stream.common.set_format = normal_in_set_format;
    in->stream.common.standby = normal_in_standby;
    in->stream.common.dump = normal_in_dump;
    in->stream.common.set_parameters = normal_in_set_parameters;
    in->stream.common.get_parameters = normal_in_get_parameters;
    in->stream.common.add_audio_effect = normal_in_add_audio_effect;
    in->stream.common.remove_audio_effect = normal_in_remove_audio_effect;
    in->stream.set_gain = normal_in_set_gain;
    in->stream.read = normal_in_read;
    in->stream.get_input_frames_lost = normal_in_get_input_frames_lost;
    in->config.rate = config->sample_rate;
    in->dev = ladev;
    in->standby = true;
    in->requested_rate = config->sample_rate;

#ifdef ENABLE_FIX_CAPTURE_SAMPLE_RATE
    in->config.rate = HIFI_SAMPLING_RATE;
#else
    int in_ajust_rate = in->requested_rate;
    if (HIFI_SAMPLING_RATE == SAMPLING_RATE_48K) {
        if (!(in->requested_rate % SAMPLING_RATE_8K)) {
            in_ajust_rate = in->requested_rate;
        } else {
            in_ajust_rate = SAMPLING_RATE_8K * (in->requested_rate / SAMPLING_RATE_11K + 1);
            if (in_ajust_rate > SAMPLING_RATE_48K)
                in_ajust_rate = SAMPLING_RATE_48K;
            ALOGI("out/in stream should be both 48K serial, force capture rate: %d", in_ajust_rate);
        }
    } else if (HIFI_SAMPLING_RATE == SAMPLING_RATE_44K) {
        if (!(in->requested_rate % SAMPLING_RATE_11K)) {
            in_ajust_rate = in->requested_rate;
        } else {
            in_ajust_rate = SAMPLING_RATE_11K * in->requested_rate / SAMPLING_RATE_8K;
            if (in_ajust_rate > SAMPLING_RATE_44K)
                in_ajust_rate = SAMPLING_RATE_44K;
            ALOGI("out/in stream should be both 44.1K serial, force capture rate: %d",
                  in_ajust_rate);
        }
    }
    in->config.rate = in_ajust_rate;
#endif

    if (config->channel_mask == AUDIO_CHANNEL_NONE)
        in->config.channels = CHANNEL_STEREO;
    else
        in->config.channels = audio_channel_count_from_in_mask(config->channel_mask);
    in->config.format = config->format;
    if (config->format == AUDIO_FORMAT_DEFAULT)
        in->config.format = pcm_format_from_audio_format(HIFI_AUDIO_FORMAT);
    else
        in->config.format = pcm_format_from_audio_format(config->format);

    in->config.period_size =
        samples_per_milliseconds(IN_PERIOD_MS, in->config.rate, in->config.channels);
    in->config.period_count = CAPTURE_PERIOD_COUNT;

    ALOGD(
        "adev_normal_open_input_stream: config rate: %d, request rate: %d, channels: %d, format: "
        "%d bit,"
        "frames: %d",
        in->config.rate, in->requested_rate, in->config.channels, in->config.format,
        in->config.period_size);
    *stream_in = &in->stream;
    return 0;
}

static void adev_normal_close_input_stream(struct audio_hw_device *dev,
                                           struct audio_stream_in *in) {
    ALOGV("adev_normal_close_input_stream...");
    return;
}

static int adev_normal_dump(const audio_hw_device_t *device, int fd) {
    ALOGV("adev_normal_dump");
    return 0;
}

static int adev_normal_close(hw_device_t *device) {
    ALOGV("adev_normal_close");
    free(device);
    return 0;
}

static int adev_normal_open(const hw_module_t *module, const char *name, hw_device_t **device) {
    ALOGV("adev_normal_open: %s", name);

    int ret = 0;
    struct alsa_audio_device *adev;

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = calloc(1, sizeof(struct alsa_audio_device));
    if (!adev)
        return -ENOMEM;

    adev->hw_device.common.tag = HARDWARE_DEVICE_TAG;
    adev->hw_device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
    adev->hw_device.common.module = (struct hw_module_t *)module;
    adev->hw_device.common.close = adev_normal_close;

    adev->hw_device.init_check = adev_normal_init_check;
    adev->hw_device.set_voice_volume = adev_normal_set_voice_volume;
    adev->hw_device.set_master_volume = adev_normal_set_master_volume;
    adev->hw_device.get_master_volume = adev_normal_get_master_volume;
    adev->hw_device.set_master_mute = adev_normal_set_master_mute;
    adev->hw_device.get_master_mute = adev_normal_get_master_mute;
    adev->hw_device.set_mode = adev_normal_set_mode;
    adev->hw_device.set_mic_mute = adev_normal_set_mic_mute;
    adev->hw_device.get_mic_mute = adev_normal_get_mic_mute;
    adev->hw_device.set_parameters = adev_normal_set_parameters;
    adev->hw_device.get_parameters = adev_normal_get_parameters;
    adev->hw_device.get_input_buffer_size = adev_normal_get_input_buffer_size;
    adev->hw_device.open_output_stream = adev_normal_open_output_stream;
    adev->hw_device.close_output_stream = adev_normal_close_output_stream;
    adev->hw_device.open_input_stream = adev_normal_open_input_stream;
    adev->hw_device.close_input_stream = adev_normal_close_input_stream;
    adev->hw_device.dump = adev_normal_dump;

    *device = &adev->hw_device.common;

#ifdef ENABLE_INPUT_RESAMPLER
    adev->is_input_resampler_enable = false;  // TODO: input resample is not avaible now
#else
    adev->is_input_resampler_enable = false;
#endif

    ret = sd_platform_init(adev);

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_normal_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common =
        {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = AUDIO_HARDWARE_MODULE_ID,
            .name = "normal audio HW HAL",
            .author = "The Android Open Source Project",
            .methods = &hal_module_methods,
        },
};
