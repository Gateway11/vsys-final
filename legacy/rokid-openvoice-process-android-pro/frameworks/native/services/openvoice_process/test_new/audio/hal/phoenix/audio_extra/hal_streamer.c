/*
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author: mengmeng.chang
 * @Date: 2022-04-08 15:20:22
 * @Descripttion:
 * @Revision History:
 * ----------------
 */

#define LOG_TAG "audio_hw_streamer"
// #define LOG_NDEBUG 0
#include "audio_extra.h"
#include <log/log.h>
#include <semaphore.h>
#include <unistd.h>

#include "audio_bt_call.h"
#include "audio_hw.h"
#include "hal_streamer.h"
#include "platform.h"
#include "remote.h"
#include "audio_dsp_common.h"
#define READ_TIMEOUT_LIMITS 10

// #define DUMP_PCM_WRITE

#define MAX_STREAMERS 8
#define BUS_RBUF_LEN 5

hal_streamer_t g_out_stream[MAX_STREAMERS];
hal_streamer_t g_in_stream[MAX_STREAMERS];

static pthread_mutex_t g_alloc_lock = PTHREAD_MUTEX_INITIALIZER;

int32_t hal_streamer_bind_bus(const char *name, int32_t len, int32_t bind_bus) {
    AALOGV("card name: %s, bind bus: %d", name, bind_bus);
    hal_streamer_t *hdl = NULL;
    int32_t ret = 0;
    pthread_mutex_lock(&g_alloc_lock);
    {
        for (int32_t i = 0; i < ARRAY_SIZE(g_out_stream); i++) {
            if (g_out_stream[i].allocated && !strcmp(g_out_stream[i].name, name)) {
                hdl = &g_out_stream[i];
                goto find;
            }
        }
        for (int32_t i = 0; i < ARRAY_SIZE(g_out_stream); i++) {
            if (!g_out_stream[i].allocated) {
                hdl = &g_out_stream[i];
            }
        }
        if (!hdl) {
            AALOGE("no valid streamer");
            ret = -ENOMEM;
            goto bind_err;
        }
        hdl->allocated = true;

        pthread_mutex_init(&hdl->lock, NULL);
        strncpy(hdl->name, name, len);
    }
find:
    pthread_mutex_unlock(&g_alloc_lock);
    pthread_mutex_lock(&hdl->lock);
    {
        hdl->bind_bus[hdl->bind_bus_len++] = bind_bus;
        AALOGI("bind bus %d, len: %d", bind_bus, hdl->bind_bus_len);
    }
    pthread_mutex_unlock(&hdl->lock);
    AALOGV("hdl: %p", hdl);
    return ret;

bind_err:
    pthread_mutex_unlock(&g_alloc_lock);
    return ret;
}

static hal_streamer_t *probe_in(struct alsa_stream_in *in) {
    hal_streamer_t *hdl = NULL;
    for (int32_t i = 0; i < ARRAY_SIZE(g_in_stream); i++) {
        if (!g_in_stream[i].allocated) {
            hdl = &g_in_stream[i];
            goto out;
        }
    }
    AALOGE("Not found allocated handler, please check out platform.xml");
out:
    if (hdl)
        hdl->dir = CAPTURE_STREAM;
    return hdl;
}

static hal_streamer_t *probe_out(struct alsa_stream_out *out) {
    hal_streamer_t *hdl = NULL;
    for (int32_t i = 0; i < ARRAY_SIZE(g_out_stream); i++) {
        if (g_out_stream[i].allocated) {
            hdl = &g_out_stream[i];
            for (int32_t k = 0; k < hdl->bind_bus_len; k++) {
                AALOGI("hdl: %p, bus: %d, k: %d val: %d", hdl, out->bus_num, k, hdl->bind_bus[k]);
                if (out->bus_num == hdl->bind_bus[k])
                    goto out;
            }
        }
    }
    AALOGE("Not found allocated handler, please check out platform.xml");
out:
    if (hdl)
        hdl->dir = PLAYBACK_STREAM;
    return hdl;
}

static int pcm_16_areas_copy(uint32_t frames, const char *src, uint32_t src_ch, char *dst,
                             uint32_t dst_ch, uint32_t dst_offset) {
    int32_t dst_inx, src_inx;
    int16_t *srcp = (int16_t *)src;
    int16_t *dstp = (int16_t *)dst;
    if (dst_offset < 0 || !srcp || !dstp)
        return -EINVAL;
    for (int i = 0; i < frames / src_ch; i++) {
        dst_inx = i * dst_ch + (dst_offset * src_ch);
        src_inx = (i * src_ch);
        memcpy((void *)(dstp + dst_inx), (const void *)(srcp + src_inx), src_ch * 2);
    }
    return 0;
}

/* TODO: Normalize with do_mix_period interface
 * should support 8-channel one-mix internal sound.
 * example 2-channel map/mix to 8-channel slot 0/1 or slot 2/3
 */
static uint32_t /* return dst frames */
pcm_16_areas_mix(uint32_t frames, int16_t *dstp,
		 int16_t *srcp, /*volatile signed int *sum,*/
		 size_t src_ch, size_t dst_ch/*,
		 int32_t dst_offset , size_t sum_step*/)
{
    int32_t dst_inx, src_inx;
    int16_t mixed = 0;
    int i = 0;
    if (!srcp || !dstp)
        return 0;
    for (; i < frames / src_ch; i++) {
        mixed = 0;
        dst_inx = i * dst_ch + (0 /*TODO offset, not used*/ * src_ch);
        src_inx = (i * src_ch);
        mixed = dstp[dst_inx] + srcp[src_inx];
        if (mixed > INT16_MAX)
            dstp[dst_inx] = INT16_MAX;
        else if (mixed < INT16_MIN)
            dstp[dst_inx] = INT16_MIN;
        else
            dstp[dst_inx] = mixed;
    }
    return frames * (dst_ch / src_ch);
}

static int32_t slot_map_process_buf(hal_streamer_t *hdl, int16_t *out, int32_t out_bytes,
                                    int16_t *in, int32_t in_frames, int32_t bus) {
    int32_t out_oft;
    uint32_t ch = audio_circlebuf_get_ch(hdl->bus_rbuf[bus]);
    // int32_t target_inx, src_inx;
    struct platform_data *pf = hdl->adev->platform;
    out_oft = pf->info.output_maps[bus].slots[0];
    if(current_audio_bus_status.slot_index[bus] == pf->info.output_maps[bus].slots[0]) {
        out_oft = pf->info.output_maps[bus].slots[0];
    } else {
        out_oft = current_audio_bus_status.slot_index[bus];
    }
#if 0
    // TODO: Every slot copy
    for (int32_t i = 0; i < in_frames / ch; i++) {
        target_inx = i * hdl->pcm_cfg.channels + out_oft;
        src_inx = (i * ch);
        // AALOGI("target_inx: %d, src_inx: %d, in_frames: %d, ch: %d",
        //        target_inx, src_inx, in_frames, ch);
        memcpy(out + target_inx, in + src_inx, (ch) * (pcm_format_to_bits(fmt) >> 3));
    }
#endif
    AALOGV("bus ch: %d", ch);
    for (int32_t i = 0; i < ch; i++) {
        pcm_16_areas_mix(in_frames, out + out_oft + i, in + i, ch, hdl->pcm_cfg.channels);
    }

    return 0;
}

static int32_t do_channel_map(void *context, int16_t *out_tmp, int32_t out_len) {
    hal_streamer_t *hdl = (hal_streamer_t *)context;
    audio_circlebuf_t *rb = NULL;
    int32_t buf_size, ret = 0;
    char in_tmp[PCM_DATA_DEFAULT_BYTES];  // every bus rbuf data

    for (int32_t active_bus = 0; active_bus < MAX_CLIENT_NUM; active_bus++) {
        if (get_bit(hdl->bus_state, active_bus)) {
            if (active_bus == BUS2)  // bt phone bus direct write, skiped.
                continue;
            rb = hdl->bus_rbuf[active_bus];
            buf_size = hdl->bus_buf_sz[active_bus];
            memset(in_tmp, 0, buf_size);
            ret = audio_circlebuf_used(rb);
            if (ret >= buf_size) {
                AALOGV("[    READY %d] used: %d bytes, buf_size %d", active_bus, ret, buf_size);
                ret = audio_circlebuf_read(rb, (u_char *)in_tmp,
                                           buf_size);  // period data
                ret = slot_map_process_buf(hdl, (int16_t *)out_tmp, out_len, (int16_t *)in_tmp,
                                           buf_size / 2 /* s16 */, active_bus);
            }
        }
    }
    return 0;
}

static int32_t select_cap_slot_s16(int16_t *dst, int32_t dst_ch, int16_t *src, int32_t src_ch,
                                   int32_t src_frames) {
    int32_t src_idx = 0;
    // TODO: src_slot_base can be customized in the future
    int32_t src_slot_base = 0;
    int32_t dst_idx = 0;
    int32_t i;
    for (i = 0; i < src_frames / src_ch; i++) {
        src_idx = i * src_ch + src_slot_base;
        dst_idx = i * dst_ch;
        memcpy(dst + dst_idx, src + src_idx, sizeof(short));
    }
    return i;
}
static void *capture_thread_loop(void *context) {
    androidSetThreadPriority(0, ANDROID_PRIORITY_URGENT_AUDIO);
    pthread_detach(pthread_self());

    hal_streamer_t *hdl = (hal_streamer_t *)context;
    char pcm_buf[PCM_DATA_DEFAULT_BYTES];        // read form kernel
    char processed_buf[PCM_DATA_DEFAULT_BYTES];  // select slot
    int32_t dst_device = -1;
    audio_circlebuf_t *target_rbuf;
    int32_t device_channels, device_fmt = 0;
    // TODO: use config params
    int32_t src_bytes = CAP_THRESHOLD_BYTES_BY_RATE(hdl->pcm_cfg.channels, hdl->pcm_cfg.format,
                                                    hdl->pcm_cfg.rate, IN_PERIOD_MS);
    int32_t src_frames = MIX_THRESHOLD_FRAMES(src_bytes, hdl->pcm_cfg.format);
    int32_t dst_frames = 0;
    int32_t dst_bytes;

    AALOGI("capture data bytes: %d, src_frames: %d", src_bytes, src_frames);

    while (!hdl->cap_exit) {
        dst_device = -1;
        if (get_bit(hdl->cap_state, BUILD_IN_MIC_CAP)) {
            target_rbuf = hdl->cap_rbuf[BUILD_IN_MIC_CAP];
        } else if (get_bit(hdl->cap_state, EXT_MIC_CAP)) {
            target_rbuf = hdl->cap_rbuf[EXT_MIC_CAP];
        } else {
            ALOGW("Unsupport MIC, use default four mic");
            target_rbuf = hdl->cap_rbuf[FOUR_MIC_CAP];
        }
        AALOGV("target_rbuf: %p, hdl->cap_state: %#x", target_rbuf, hdl->cap_state);
        device_channels = audio_circlebuf_get_ch(target_rbuf);
        device_fmt = audio_circlebuf_get_fmt(target_rbuf);
        AALOGV(
            "capture %d bytes data , cap_sta: %#x, src_frames: %d. "
            "device_channels: %d, device_fmt: %d",
            src_bytes, hdl->cap_state, src_frames, device_channels, device_fmt);
        pcm_read(hdl->pcm, pcm_buf, src_bytes);
        if (hdl->pcm_cfg.channels != device_channels) {
            for (int c = 0; c < device_channels; c++) {
                dst_frames =
                    select_cap_slot_s16(((int16_t *)processed_buf) + c,  // out buffer
                                        device_channels,                 // out channels
                                        (int16_t *)pcm_buf, hdl->pcm_cfg.channels, src_frames);
            }
            dst_bytes = dst_frames * device_channels * AUDIO_FMT_2_BYTES(device_fmt);

            audio_circlebuf_write(target_rbuf, (u_char *)processed_buf, dst_bytes);
        } else {
            audio_circlebuf_write(target_rbuf, (u_char *)pcm_buf, src_bytes);
        }
    }
    /* singal */
    sem_post(&hdl->cap_sem);
    AALOGI("capture thread exit");
    pthread_exit(0);
    return NULL;
}

static void *playback_thread_loop(void *context) {
    hal_streamer_t *hdl = (hal_streamer_t *)context;
    char out_tmp[PCM_DATA_DEFAULT_BYTES];  // pcm write buf
    int32_t buf_size = 0;
    if (!hdl) {
        AALOGE("invalid args");
        return NULL;
    }

    buf_size = CALCULATE_BYTES(hdl->pcm_cfg.channels, PCM_FORMAT_S16_LE, 48000, 20);

    AALOGI("%s: __enter__", __func__);
    androidSetThreadPriority(0, ANDROID_PRIORITY_URGENT_AUDIO);
    pthread_detach(pthread_self());

    while (!hdl->plbk_exit) {
        if (hdl->plbk_hold) {
            usleep(1 * 1000);
            continue;
        }
        if (hdl->pcm) {
            memset(out_tmp, 0, buf_size);
            do_channel_map(hdl, (int16_t *)out_tmp, buf_size);
            if((pcm_write(hdl->pcm, out_tmp, buf_size)) != 0 )
            {
                ALOGD("%s: write failed", __func__);
                usleep(20 * 1000);
            }
#ifdef DUMP_PCM_WRITE
            dump_out_data(out_tmp, buf_size, NULL, 0);
#endif
        }
    }
    /* singal */
    sem_post(&hdl->plbk_sem);
    AALOGI("playback thread exit");
    pthread_exit(0);
    return NULL;
}

static int32_t stream_capture_prepare(hal_streamer_t *hdl) {
    int32_t ret = 0;
    hdl->cap_exit = false;
    sem_init(&hdl->cap_sem, 0, 0);
    ret = pthread_create(&hdl->cap_thread, (const pthread_attr_t *)NULL, capture_thread_loop, hdl);

    return ret;
}

static int32_t stream_playback_prepare(hal_streamer_t *hdl) {
    int32_t ret = 0;
    hdl->plbk_exit = false;
    hdl->plbk_hold = false;
    sem_init(&hdl->plbk_sem, 0, 0);
    ret =
        pthread_create(&hdl->plbk_thread, (const pthread_attr_t *)NULL, playback_thread_loop, hdl);

    return ret;
}

static int32_t prepare_rbuf(hal_streamer_t *hdl, struct audio_stream *stream) {
    int32_t ret = 0;
    if (!hdl)
        return -EINVAL;
    struct alsa_stream_out *out;
    struct alsa_stream_in *in;
    audio_circlebuf_t *rb;
    if (PLAYBACK_STREAM == hdl->dir) {
        AALOGV("prepare output stream rbuf");
        out = (struct alsa_stream_out *)stream;
        hdl->bus_rbuf[out->bus_num] = audio_circlebuf_create_pcm(
            out->config.channels, out->config.format, hdl->pcm_cfg.rate, BUS_RBUF_LEN);

    } else if (CAPTURE_STREAM == hdl->dir) {
        AALOGV("prepare input stream rbuf");
        in = (struct alsa_stream_in *)stream;

        rb = audio_circlebuf_create_pcm(in->config.channels, in->config.format, in->config.rate,
                                        BUS_RBUF_LEN);
        AALOGV("create rb: %p for device : 0x%08x", rb, in->devices);
        // hdl->pcm_cfg.channels, hdl->pcm_cfg.format, hdl->pcm_cfg.rate,
        // BUS_RBUF_LEN);

        if (AUDIO_DEVICE_IN_BUILTIN_MIC == in->devices)
            hdl->cap_rbuf[BUILD_IN_MIC_CAP] = rb;
        else if (AUDIO_DEVICE_IN_TELEPHONY_RX == in->devices)
            hdl->cap_rbuf[EXT_MIC_CAP] = rb;
    }
    return ret;
}

hal_streamer_t *hal_streamer_open(struct audio_stream *stream, int32_t card, int32_t port,
                                  int32_t flag, struct pcm_config *pcm_cfg) {
    DEBUG_FUNC_PRT
    int32_t ret = 0;
    struct alsa_stream_out *out;
    struct alsa_stream_in *in;
    hal_streamer_t *hdl = NULL;
    AALOGI("flag: %#x", flag);
    if (flag & PCM_IN) {
        in = (struct alsa_stream_in *)stream;
        hdl = probe_in(in);
        AALOGI("in->config.channels: %d, in->config.format: %d, pcm_cfg->rate: %d",
               in->config.channels, pcm_format_to_bits(in->config.format) >> 3, pcm_cfg->rate);
    } else {
        pthread_mutex_lock(&g_alloc_lock);
        out = (struct alsa_stream_out *)stream;
        // open_output_path(out);
        hdl = probe_out(out);
        hdl->adev = out->dev;
        AALOGI("out->config.channels: %d, out->config.format: %d, pcm_cfg->rate: %d",
               out->config.channels, pcm_format_to_bits(out->config.format) >> 3, pcm_cfg->rate);

        hdl->bus_buf_sz[out->bus_num] = CALCULATE_BYTES(out->config.channels, out->config.format,
                                                        pcm_cfg->rate /* all ---> 48000*/, 20);
        pthread_mutex_unlock(&g_alloc_lock);
        AALOGV("bus %d, buf size: %d", out->bus_num, hdl->bus_buf_sz[out->bus_num]);
        if (current_audio_bus_status.slot_in_use[current_audio_bus_status.slot_index[out->bus_num] / 2 ] == 1 )
        {
            for(int i=1; i < 4; i++)
            {
                if(current_audio_bus_status.slot_in_use[i] == 1)
                {
                    continue;
                }
                else
                {
                    current_audio_bus_status.slot_in_use[i] = 1;
                    switch (i)
                    {
                        case 1:
                        {
                            current_audio_bus_status.slot_index[out->bus_num] = 2;
                            break;
                        }
                        case 2:
                        {
                            current_audio_bus_status.slot_index[out->bus_num] = 4;
                            break;
                        }
                        case 3:
                        {
                            current_audio_bus_status.slot_index[out->bus_num] = 6;
                            break;
                        }
                        default:
                        break;
                    }
                }
            }
        }
        else
        {
            current_audio_bus_status.slot_in_use[current_audio_bus_status.slot_index[out->bus_num] / 2 ] = 1;
        }
        ret = set_bus_volume(out->bus_num);
        if(ret)
        {
            ALOGE("set_bus_volume failed! --- %d", ret);
        }

        switch (out->bus_num)
        {
            case BUS1:
            {
                switch (current_audio_bus_status.slot_index[out->bus_num])
                {
                    case 2:
                    {
                        dspdev_set_secondary_mono_all(1);
                        break;
                    }
                    case 4:
                    {
                        dspdev_set_phone_mono_all(1);
                        break;
                    }
                    case 6:
                    {
                        dspdev_set_navi_mono_all(1);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case BUS2:
            {
                switch (current_audio_bus_status.slot_index[out->bus_num])
                {
                    case 2:
                    {
                        dspdev_set_secondary_mono_all(0);
                        break;
                    }
                    case 4:
                    {
                        dspdev_set_phone_mono_all(0);
                        break;
                    }
                    case 6:
                    {
                        dspdev_set_navi_mono_all(0);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case BUS3:
            {
                switch (current_audio_bus_status.slot_index[out->bus_num])
                {
                    case 2:
                    {
                        dspdev_set_secondary_mono_all(0);
                        break;
                    }
                    case 4:
                    {
                        dspdev_set_phone_mono_all(0);
                        break;
                    }
                    case 6:
                    {
                        dspdev_set_navi_mono_all(0);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case BUS6:
            {
                switch (current_audio_bus_status.slot_index[out->bus_num])
                {
                    case 2:
                    {
                        dspdev_set_secondary_mono_all(1);
                        break;
                    }
                    case 4:
                    {
                        dspdev_set_phone_mono_all(1);
                        break;
                    }
                    case 6:
                    {
                        dspdev_set_navi_mono_all(1);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case BUS0:
            default:
            {
                break;
            }
        }
        current_audio_bus_status.bus_active[out->bus_num] = 1;

        ALOGD("out->bus_num: %d, slot_index: %d, bus_active:%d", out->bus_num, current_audio_bus_status.slot_index[out->bus_num], current_audio_bus_status.bus_active[out->bus_num]);
    }

    if (hdl) {
        pthread_mutex_lock(&hdl->lock);
        {
            AALOGI("hdl: %p, ref cnt: %d", hdl, hdl->ref_cnt);
            // playback thread not start
            if (hdl->ref_cnt == 0) {
                hdl->card = card;
                hdl->port = port;
                hdl->pcm_cfg.format = pcm_cfg->format;
                hdl->pcm_cfg.rate = pcm_cfg->rate;
                hdl->pcm_cfg.channels = pcm_cfg->channels;
                hdl->pcm =
                    pcm_open_prepare_helper(card, port, flag, PROXY_OPEN_RETRY_COUNT, pcm_cfg);
                if (prepare_rbuf(hdl, stream) < 0) {
                    AALOGE("prepare rbuf failed");
                    goto error_open;
                }

                if (hdl->pcm == NULL) {
                    goto error_open;
                }
                if (flag & PCM_IN) {  // start capture thread

                    if (stream_capture_prepare(hdl)) {
                        AALOGE("Prepare capture Failed");
                        goto error_open;
                    }
                } else {
                    if (stream_playback_prepare(hdl))  // start playback thread
                    {
                        AALOGE("Prepare playback Failed");
                        goto error_open;
                    }
                }
            } else if (prepare_rbuf(hdl, stream) < 0) {
                AALOGE("prepare rbuf failed");
                goto error_open;
            }

            if (flag & PCM_IN) {  // input
                if (AUDIO_DEVICE_IN_BUILTIN_MIC == in->devices)
                    set_bit(hdl->cap_state, BUILD_IN_MIC_CAP);
                else if (AUDIO_DEVICE_IN_TELEPHONY_RX == in->devices)
                    set_bit(hdl->cap_state, EXT_MIC_CAP);
            } else {  // output
                set_bit(hdl->bus_state, out->bus_num);
                out->pcm = hdl->pcm;
            }
            hdl->ref_cnt++;
        }

        pthread_mutex_unlock(&hdl->lock);
    } else {
        AALOGE("open failed");
        return hdl;
    }

    return hdl;

error_open:
    out->pcm = NULL;
    pthread_mutex_unlock(&hdl->lock);
    return NULL;
}

static bool is_low_latency_bus(struct alsa_stream_out *out) {
    if (strstr(out->bus_address, BT_CALL_BUS_NAME)) {
        // AALOGI("out bus: %s", out->bus_address);
        return true;
    } else {
        return false;
    }
}

#define FULL_SLOT_FRAMES ((HIFI_SAMPLING_RATE / 1000) * CHANNEL_7POINT1 * OUT_PERIOD_MS)

static int32_t direct_write_pcm(hal_streamer_t *hdl, struct alsa_stream_out *out, const void *data,
                                uint32_t count) {
    // AALOGI("bus: %s ch: %d", out->bus_address, out->config.channels);
    uint8_t dst_offset = 0;
    int32_t frame_count = 0;
    int32_t phone_ch = 0;
    int16_t out_tmp[FULL_SLOT_FRAMES] = {0};
    int32_t buf_size = CALCULATE_BYTES(8, PCM_FORMAT_S16_LE, 48000, 20);
    if (!hdl->plbk_hold) {
        hdl->plbk_hold = true;
    }
    phone_ch = BT_CHANNEL_CNT;//out->config.channels;
    frame_count = count / (pcm_format_to_bits(out->config.format) / 8);
    if(out->bus_num == 2)
    {
        switch (current_audio_bus_status.slot_index[out->bus_num])
        {
            case 2:
            {
                dst_offset = 1;
                break;
            }
            case 4:
            {
                dst_offset = 2;
                break;
            }
            case 6:
            {
                dst_offset = 3;
                break;
            }
            default:
            {
                dst_offset = 2;
                break;
            }
        }
    }
    pcm_16_areas_copy(frame_count, data, phone_ch, (char *)out_tmp, hdl->pcm_cfg.channels, dst_offset);
    do_channel_map(hdl, out_tmp, buf_size);
    // dump_out_data(out_tmp, buf_size, NULL, 1);
    return pcm_write(hdl->pcm, out_tmp, buf_size);
}

int32_t hal_streamer_write(struct alsa_stream_out *out, const void *data, uint32_t count) {
    DEBUG_FUNC_PRT

    hal_streamer_t *hdl = NULL;
    int32_t ret = count;
    int32_t avail = 0;
    audio_circlebuf_t *rbuf = NULL;

    if (!out || !out->pcm || !data) {
        AALOGE("Invalid argument");
        return -EINVAL;
    }

    hdl = out->pcm_hdl;
    if (!hdl) {
        AALOGE("Invalid argument");
        return -EINVAL;
    }

    if (is_low_latency_bus(out)) {
        ret = direct_write_pcm(hdl, out, data, count);
    } else {
        rbuf = hdl->bus_rbuf[out->bus_num];
        while (1) {
            if (rbuf) {
                avail = audio_circlebuf_available(rbuf);
                if (avail >= count)
                    break;
                else {
                    usleep(500);
                }
            } else {
                AALOGI("rb is null");
                return 0;
            }
        }
        audio_circlebuf_write(rbuf, (u_char *)data, count);
        AALOGV("bus : %d, write %d , avail %d", out->bus_num, count, avail);
        ret = 0;
    }

    return ret;
}

int32_t hal_streamer_read(struct alsa_stream_in *in, const void *data, uint32_t count) {
    int32_t policy = 0;
    hal_streamer_t *hdl = in->pcm_hdl;
    audio_circlebuf_t *rb;
    int32_t used = 0;
    int32_t ret = 0;
    uint32_t limit = READ_TIMEOUT_LIMITS;

    if (!hdl) {
        AALOGE("invalid pcm");
        ret = -EINVAL;
        goto read_err;
    }
    // AALOGI("in devices: 0x%08x", in->devices);
    DEBUG_FUNC_PRT
    if (AUDIO_DEVICE_IN_BUILTIN_MIC == in->devices)
        policy = BUILD_IN_MIC_CAP;
    else if (AUDIO_DEVICE_IN_TELEPHONY_RX == in->devices)
        policy = EXT_MIC_CAP;
    rb = hdl->cap_rbuf[policy];

    while (1) {
        used = audio_circlebuf_used(rb);
        // AALOGV("read form rbuf: %p, count: %d, used:%d", rb, count, used);
        if (used >= count) {
            ret = audio_circlebuf_read(rb, (u_char *)data, count);
            break;
        } else {
            usleep(IN_PERIOD_MS / 2 * 1000);
            if (!limit--)
                break;
        }
    }

    if (ret > 0)
        ret = 0;
    return ret;
read_err:
    return ret;
}

int32_t hal_streamer_close(struct audio_stream *stream, int32_t flag) {
    DEBUG_FUNC_PRT
    struct alsa_stream_out *out = NULL;
    struct alsa_stream_in *in = NULL;
    hal_streamer_t *hdl = NULL;
    int32_t exit_timeout_ms = 315;
    int32_t result;

    if (flag & PCM_IN) {
        in = (struct alsa_stream_in *)stream;
        hdl = in->pcm_hdl;
        AALOGI("capture stream(%p) close", hdl);
    } else {
        out = (struct alsa_stream_out *)stream;
        hdl = out->pcm_hdl;
        AALOGI("playback stream(%p) close", hdl);
        current_audio_bus_status.slot_in_use[current_audio_bus_status.slot_index[out->bus_num] / 2 ] = 0;
        current_audio_bus_status.bus_active[out->bus_num] = 0;
        current_audio_bus_status.slot_index[out->bus_num] = out->dev->platform->info.output_maps[out->bus_num].slots[0];
    }

    if (!hdl)
        return -EINVAL;

    pthread_mutex_lock(&hdl->lock);
    {
        hdl->ref_cnt--;
        if (hdl->ref_cnt == 0) {
            if (flag & PCM_IN) {
                hdl->cap_exit = true;
                AALOGI("wait for capture thread exited, timeout is %d ms", exit_timeout_ms);
                result = audio_extra_semwait(&hdl->cap_sem, exit_timeout_ms /*ms*/);
                if (OS_SYNC_TIMEOUT == result) {
                    ALOGW("playback thread exit too slow");
                }
                if (AUDIO_DEVICE_IN_BUILTIN_MIC == in->devices)
                    clear_bit(hdl->cap_state, BUILD_IN_MIC_CAP);
                else if (AUDIO_DEVICE_IN_TELEPHONY_RX == in->devices)
                    clear_bit(hdl->cap_state, EXT_MIC_CAP);
                if (hdl->pcm) {
                    pcm_close(hdl->pcm);
                    hdl->pcm = NULL;
                    in->pcm = NULL;
                }
            } else {
                hdl->plbk_exit = true;
                AALOGI("wait for playback thread exited, timeout is %d ms", exit_timeout_ms);
                result = audio_extra_semwait(&hdl->plbk_sem, exit_timeout_ms /*ms*/);
                if (OS_SYNC_TIMEOUT == result) {
                    ALOGW("playback thread exit too slow");
                }
                if (hdl->pcm) {
                    pcm_close(hdl->pcm);
                    hdl->pcm = NULL;
                    out->pcm = NULL;
                }
            }
        } else {
            if (flag & PCM_IN) {
            } else {
                if (is_low_latency_bus(out))
                    hdl->plbk_hold = false;
            }
        }
    }
    pthread_mutex_unlock(&hdl->lock);
    AALOGI("ref cnt: %d", hdl->ref_cnt);
    return 0;
}
