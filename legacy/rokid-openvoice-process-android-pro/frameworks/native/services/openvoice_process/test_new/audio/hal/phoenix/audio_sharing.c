/*
 * audio_sharing.c
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author: mengmeng.chang
 * @Date: 2022-03-14 19:36:37
 * @Descripttion:
 * @Revision History:
 * ----------------
 */

#define LOG_TAG "audio_hw_sharing"
// #define LOG_NDEBUG 0
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "audio_extra.h"
#include "audio_hw.h"
#include "audio_sharing.h"
#include <cutils/str_parms.h>
#include <log/log.h>

au_sharing_t g_sharing = {
    .name = "common",
};

extern struct pcm_config *pcm_main_config;
extern struct pcm_config rear_100_cap_cfg_ref;
extern struct pcm_config main_config_ref_virtual;

uint32_t mute_period = 100 /*ms*/ / OUT_PERIOD_MS;

static bool check_pcm_open(struct pcm *pcm) {
    if (!pcm || !pcm_is_ready(pcm)) {
        AALOGE("Unable to open PCM device (%s)\n", pcm_get_error(pcm));
        return false;
    }
    return true;
}

#if 1
static int start_ne_sharing_stream(au_shr_dst_t *dst) {
    struct alsa_audio_device *adev = dst->dev;
    int card = 0, port = 0;

    if (is_x9u_ref_b()) {
        card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR2);
        port = REFB_VI2S_DEV;
    } else if (is_x9u_ref_a()) {
        card = sd_platform_get_card_id(adev->platform, AUDIO_NAME_REAR1);
        port = REFA_VI2S_DEV;
    }
    if (card < 0) {
        AALOGE("get card id failed");
        return -1;
    }

    dst->pcm_vi2s = pcm_open(card, port, PCM_OUT, &main_config_ref_virtual);
    if (dst->pcm_vi2s) {
        AALOGI("open virtual i2s card: %d , port: %d", card, port);
        pcm_prepare(dst->pcm_vi2s);
    }
    return 0;
}

static int stop_ne_sharing_stream(au_shr_dst_t *dst) {
    if (dst->pcm_vi2s) {
        pcm_close(dst->pcm_vi2s);
        AALOGI("close virtual i2s");
        dst->pcm_vi2s = NULL;
    }
    return 0;
}

#endif
static void *ssa_rear_loop(void *context) {
    au_shr_dst_t *dstp = (au_shr_dst_t *)context;
    uint32_t periods = 0;
    char tmp[16 * 1024];
    char write_buf[16 * 1024];
    uint32_t period_bytes = (4 /*ch*/ * 2 /*s16*/ * OUT_PERIOD_MS * HIFI_SAMPLING_RATE) / 1000;
    int32_t read_sz = 0, convert_frames;
    struct timeval start, end;
    uint32_t card = 1;
    uint32_t port = CODEC_DEV;

    while (1) {
        sem_wait(&dstp->sem);
        gettimeofday(&start, NULL);
        if (dstp->pcm == NULL) {
            card = sd_platform_get_card_id(dstp->dev->platform, AUDIO_NAME_REAR1);
            dstp->pcm = pcm_open(card, port, PCM_OUT, &rear_100_cap_cfg_ref);
            if (!check_pcm_open(dstp->pcm)) {
                AALOGE("open pcm failed");
                pcm_close(dstp->pcm);
                dstp->pcm = NULL;
            }
        }
        gettimeofday(&end, NULL);
        int32_t interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        AALOGI("pcmC%dD%dp open cost: %f ms, dstp: %p", card, port, interval / 1000.0, dstp);

        dstp->first_done = false;
        periods = 0;
        while (1) {
            read_sz = audio_circlebuf_read(dstp->rbuf, (u_char *)tmp, period_bytes);
            if (periods < mute_period) {
                memset(tmp, 0, period_bytes);
                periods++;
                continue;
            }
            convert_frames = audio_extra_select_slot_16(
                (int16_t *)write_buf, rear_100_cap_cfg_ref.channels, (int16_t *)tmp,
                // src->pcm_cfg.channels,
                4, period_bytes / 2);
            if (dstp->pcm && read_sz == period_bytes) {
                // dump_out_data(tmp, period_bytes, NULL, 1);
                AALOGV("SSA 1 Second card playback %d bytes ", convert_frames * 2);
                pcm_write(dstp->pcm, write_buf,  // ssa second card
                          convert_frames * 2);
                dstp->first_done = true;
            } else {
                usleep(5 * 1000);
                AALOGV("pcm not active");
                if (dstp->pcm == NULL) {
                    dstp->pcm = pcm_open(card, port, PCM_OUT, &rear_100_cap_cfg_ref);
                    if (!check_pcm_open(dstp->pcm)) {
                        AALOGE("open pcm failed");
                        pcm_close(dstp->pcm);
                        dstp->pcm = NULL;
                    }
                }
            }
            if (dstp->hold) {
                audio_circlebuf_clear(dstp->rbuf);
                break;
            }
        }
        if (dstp->hold) {
            AALOGI("hold");
            if (dstp->pcm) {
                pcm_close(dstp->pcm);
                dstp->pcm = NULL;
                audio_circlebuf_clear(dstp->rbuf);
            }
        }
    }
}
static void *ssb_rear_loop(void *context) {
    au_shr_dst_t *dstp = (au_shr_dst_t *)context;
    uint32_t periods = 0;
    char tmp[16 * 1024];
    uint32_t period_bytes = (2 /*ch*/ * 2 /*s16*/ * OUT_PERIOD_MS * HIFI_SAMPLING_RATE) / 1000;
    int32_t read_sz = 0;
    struct timeval start, end;
    uint32_t card = 1;
    uint32_t port = CODEC_DEV;

    while (1) {
        sem_wait(&dstp->sem);
        gettimeofday(&start, NULL);
        if (dstp->pcm == NULL) {
            card = sd_platform_get_card_id(dstp->dev->platform, AUDIO_NAME_REAR3);
            dstp->pcm = pcm_open(card, port, PCM_OUT, &rear_100_cap_cfg_ref);
            if (!check_pcm_open(dstp->pcm)) {
                AALOGE("open pcm failed");
                pcm_close(dstp->pcm);
                dstp->pcm = NULL;
            }
        }
        gettimeofday(&end, NULL);
        int32_t interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        AALOGI("pcmC%dD%dp open cost: %f ms", card, port, interval / 1000.0);

        dstp->first_done = false;
        periods = 0;
        while (1) {
            read_sz = audio_circlebuf_read(dstp->rbuf, (u_char *)tmp, period_bytes);
            if (periods < mute_period) {
                memset(tmp, 0, period_bytes);
                periods++;
                // continue;
            }
            if (dstp->pcm && read_sz == period_bytes) {
                // dump_out_data(tmp, period_bytes, NULL, 1);
                pcm_write(dstp->pcm, tmp, read_sz);  // playback ssb second card
                dstp->first_done = true;
            } else {
                if (dstp->pcm == NULL) {
                    dstp->pcm = pcm_open(card, port, PCM_OUT, &rear_100_cap_cfg_ref);
                    if (!check_pcm_open(dstp->pcm)) {
                        AALOGE("open pcm failed");
                        pcm_close(dstp->pcm);
                        dstp->pcm = NULL;
                    }
                } else if (read_sz < period_bytes) {
                    AALOGV("No enough pcm data");
                    usleep(5 * 1000);
                }
            }
            if (dstp->hold) {
                audio_circlebuf_clear(dstp->rbuf);
                break;
            }
        }
        if (dstp->hold) {
            AALOGI("hold");
            if (dstp->pcm) {
                pcm_close(dstp->pcm);
                dstp->pcm = NULL;
                audio_circlebuf_clear(dstp->rbuf);
            }
        }
    }
}
static void *ssb_main_loop(void *context) {
    au_shr_dst_t *dstp = (au_shr_dst_t *)context;
    uint32_t periods = 0;
    char tmp[16 * 1024];
    uint32_t period_bytes = (2 /*ch*/ * 2 /*s16*/ * OUT_PERIOD_MS * HIFI_SAMPLING_RATE) / 1000;
    int32_t read_sz = 0;
    struct timeval start, end;
    uint32_t card = 1;
    uint32_t port = CODEC_DEV;

    while (1) {
        sem_wait(&dstp->sem);
        gettimeofday(&start, NULL);
        if (dstp->pcm == NULL) {
            card = sd_platform_get_card_id(dstp->dev->platform, AUDIO_NAME_REAR2);
            dstp->pcm = pcm_open(card, port, PCM_OUT, &rear_100_cap_cfg_ref);
            if (!check_pcm_open(dstp->pcm)) {
                AALOGE("open pcm failed");
                pcm_close(dstp->pcm);
                dstp->pcm = NULL;
            }
        }
        gettimeofday(&end, NULL);
        int32_t interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        AALOGI("pcmC%dD%dp open cost: %f ms", card, port, interval / 1000.0);

        dstp->first_done = false;
        periods = 0;
        while (1) {
            read_sz = audio_circlebuf_read(dstp->rbuf, (u_char *)tmp, period_bytes);
            if (periods < mute_period) {
                memset(tmp, 0, period_bytes);
                periods++;
                // continue;
            }
            if (dstp->pcm && read_sz == period_bytes) {
                // dump_out_data(tmp, period_bytes, NULL, 1);
                pcm_write(dstp->pcm, tmp, read_sz);  // write to ssb main card
                dstp->first_done = true;
            } else {
                if (dstp->pcm == NULL) {
                    dstp->pcm = pcm_open(card, port, PCM_OUT, &rear_100_cap_cfg_ref);
                    if (!check_pcm_open(dstp->pcm)) {
                        AALOGE("open pcm failed");
                        pcm_close(dstp->pcm);
                        dstp->pcm = NULL;
                    }
                } else if (read_sz < period_bytes) {
                    AALOGV("No enough pcm data");
                    usleep(5 * 1000);
                }
            }
            if (dstp->hold) {
                audio_circlebuf_clear(dstp->rbuf);
                break;
            }
        }
        if (dstp->hold) {
            AALOGI("hold");
            if (dstp->pcm) {
                pcm_close(dstp->pcm);
                dstp->pcm = NULL;
                audio_circlebuf_clear(dstp->rbuf);
            }
        }
    }
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

static void *ssa_main_loop(void *context) {
    au_shr_dst_t *dstp = (au_shr_dst_t *)context;
    uint32_t periods = 0;
    char tmp[16 * 1024];
    char write_buf[16 * 1024];
    uint32_t period_bytes = (2 /*ch*/ * 2 /*s16*/ * OUT_PERIOD_MS * HIFI_SAMPLING_RATE) / 1000;
    int32_t read_sz = 0;
    struct timeval start, end;
    uint32_t card = sd_platform_get_card_id(dstp->dev->platform, AUDIO_NAME_PRIMARY);
    uint32_t port = CODEC_DEV;
    androidSetThreadPriority(0, ANDROID_PRIORITY_URGENT_AUDIO);

    while (1) {
        sem_wait(&dstp->sem);
        gettimeofday(&start, NULL);
        if (dstp->pcm_hdl == NULL) {
            open_output_path(dstp->stream);
            dstp->pcm_hdl = hal_streamer_open((struct audio_stream *)dstp->stream, card, port,
                                              PCM_OUT, pcm_main_config);
            dstp->stream->pcm_hdl = dstp->pcm_hdl;
        }
        gettimeofday(&end, NULL);
        int32_t interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        AALOGI("pcmC%dD%dp open cost: %f ms", card, port, interval / 1000.0);

        dstp->first_done = false;
        periods = 0;
        while (1) {
            read_sz = audio_circlebuf_read(dstp->rbuf, (u_char *)tmp, period_bytes);
            if (periods < mute_period) {
                memset(tmp, 0, period_bytes);
                periods++;
            }
            // 2 --> 4
            pcm_16_areas_copy(period_bytes / 2 /*2 ch frames*/, tmp, 2, write_buf, 4, 0);
            if (dstp->pcm_hdl && read_sz == period_bytes) {
                // dump_out_data(tmp, period_bytes, NULL, 1);
                hal_streamer_write(dstp->stream, write_buf, period_bytes * 2);
                dstp->first_done = true;
            } else {
                usleep(5 * 1000);
                AALOGV("pcm not active");
            }
            if (dstp->hold) {
                audio_circlebuf_clear(dstp->rbuf);
                break;
            }
        }
        if (dstp->hold) {
            AALOGI("hold");
            if (dstp->pcm_hdl) {
                hal_streamer_close((struct audio_stream *)dstp->stream, PCM_OUT);
                dstp->pcm_hdl = NULL;
                audio_circlebuf_clear(dstp->rbuf);
                close_output_path(dstp->stream);
            }
        }
    }
}

static int32_t au_sharing_close_dst(au_shr_src_t *src) {
    // close dup
    AALOGI("close");
    if (src->duplicate) {
        au_shr_dst_t *dst_p = &g_sharing.dsts[src->dup_sink];
        if (dst_p->hold == false) {
            dst_p->ref_cnt--;
            if (dst_p->ref_cnt == 0)
                dst_p->hold = true;
        }
    }
    return 0;
}

struct alsa_stream_out out = {
    .bus_num = 9,
    .config.channels = 4,
    .config.rate = 48000,
    .config.format = PCM_FORMAT_S16_LE,
    .config.period_size = (HIFI_SAMPLING_RATE * OUT_PERIOD_MS) / 1000,
    .bus_address = "bus9_media_out",
};

void dsts_init(au_shr_dst_t *arrary, struct alsa_audio_device *dev) {
    uint32_t total_ep = ARRAY_SIZE(g_sharing.dsts);
    int32_t rbuf_items_max = 3;
    AALOGI("arrary: %p", arrary);
    for (int32_t i = 0; i < total_ep; i++) {
        arrary[i].pcm = NULL;
        arrary[i].pcm_vi2s = NULL;
        arrary[i].hold = true;
        arrary[i].ref_cnt = 0;
        arrary[i].first_done = false;
        arrary[i].addr = i;
        arrary[i].dev = dev;
        sem_init(&arrary[i].sem, 0, 0);
        switch (i) {
            case SSA_SRC_SEC_EP:
                arrary[i].rbuf = audio_circlebuf_create_pcm(4, PCM_FORMAT_S16_LE,
                                                            HIFI_SAMPLING_RATE, rbuf_items_max);
                out.dev = dev;
                arrary[i].stream = &out;
                pthread_create(&arrary[i].looper, (const pthread_attr_t *)NULL, ssa_main_loop,
                               &arrary[i]);
                break;
            case SSA_SRC_PRI_EP:
                AALOGI("dst: %p", &arrary[i]);
                arrary[i].rbuf = audio_circlebuf_create_pcm(4, PCM_FORMAT_S16_LE,
                                                            HIFI_SAMPLING_RATE, rbuf_items_max);
                pthread_create(&arrary[i].looper, (const pthread_attr_t *)NULL, ssa_rear_loop,
                               &arrary[i]);
                break;
            case SSB_SRC_PRI_EP:
                AALOGI("dst: %p", &arrary[i]);
                arrary[i].rbuf = audio_circlebuf_create_pcm(2, PCM_FORMAT_S16_LE,
                                                            HIFI_SAMPLING_RATE, rbuf_items_max);
                pthread_create(&arrary[i].looper, (const pthread_attr_t *)NULL, ssb_rear_loop,
                               &arrary[i]);
                break;
            case SSB_SRC_SEC_EP:
                arrary[i].rbuf = audio_circlebuf_create_pcm(2, PCM_FORMAT_S16_LE,
                                                            HIFI_SAMPLING_RATE, rbuf_items_max);
                out.dev = dev;
                arrary[i].stream = &out;
                pthread_create(&arrary[i].looper, (const pthread_attr_t *)NULL, ssb_main_loop,
                               &arrary[i]);
                break;

            default:
                break;
        }
    }
    total_ep = ARRAY_SIZE(g_sharing.srcs);
    for (int32_t i = 0; i < total_ep; i++) {
        AALOGI("init src: %p", &g_sharing.srcs[i]);
        g_sharing.srcs[i].allocated = false;
    }
}
au_sharing_t *au_sharing_init(au_shr_cfg_t *cfg) {
    AALOGI("g_sharing.dsts: %p", &g_sharing.dsts[0]);
    dsts_init(&g_sharing.dsts[0], cfg->dev);
    AALOGI("init done");
    return &g_sharing;
}
#if 0
static rbuf_write_sync(audio_circlebuf_t *rbuf, const char *data, uint32_t count) {
    int32_t avail;
    while (1) {
        if (rbuf) {
            avail = audio_circlebuf_available(rbuf);
            if (avail >= count)
                break;
            else
                usleep(5 * 1000);
        } else {
            AALOGI("rb is null");
            return -1;
        }
    }
    audio_circlebuf_write(rbuf, (u_char *)data, count);

    AALOGV("avail: %d, buf : %p, write %d , avail %d", avail, data, count, avail);
    return 0;
}
#endif
static int32_t rbuf_write_async(audio_circlebuf_t *rbuf, void *data, uint32_t count) {
    audio_circlebuf_write(rbuf, (u_char *)data, count);

    return 0;
}

au_shr_src_t *au_sharing_get_src(au_shr_cfg_t *cfg) {
    au_shr_src_t *src_p = NULL;
    for (int32_t i = 0; i < ARRAY_SIZE(g_sharing.srcs); i++) {
        if (!g_sharing.srcs[i].allocated)
            src_p = &g_sharing.srcs[i];
    }
    if (src_p) {
        src_p->bus_num = cfg->stream->bus_num;
        src_p->pcm_cfg.channels = cfg->pcm_cfg.channels;
        src_p->duplicate = false;
        src_p->allocated = true;
    }
    return src_p;
}

int32_t au_sharing_release_src(au_shr_src_t *src) {
    src->allocated = false;
    return au_sharing_close_dst(src);
}
int32_t virtual_i2s_sync_write(au_shr_dst_t *dst_p, void *data, uint32_t count) {
    struct timeval start, end;
    if (dst_p->pcm_vi2s == NULL) {
        start_ne_sharing_stream(dst_p);
    }
    gettimeofday(&start, NULL);
    pcm_write(dst_p->pcm_vi2s, data, count);
    gettimeofday(&end, NULL);
    int32_t interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    AALOGV("VI2S write done, cost: %f ms", interval / 1000.0);
    return 0;
}
static int32_t au_sharing_duplicate_update(au_shr_src_t *src, enum au_shr_addr target,
                                           enum mirror_direct direct, bool enable) {
    au_shr_dst_t *dst_p = &g_sharing.dsts[target];
    AALOGV("enable: %d, target: %d, direct: %d, src->duplicate: %d", enable, target, direct,
           src->duplicate);
    if (!enable && src->duplicate == true)  // close dup
    {
        AALOGI("close dup dst_p: %p, addr: %d", dst_p, dst_p->addr);
        src->duplicate = false;
        dst_p = &g_sharing.dsts[src->dup_sink];
        if (dst_p->pcm_vi2s != NULL) {
            stop_ne_sharing_stream(dst_p);
        } else {
            dst_p->ref_cnt--;
            dst_p->hold = true;
        }
    } else if (enable && src->duplicate == false) {
        if ((MIRROR_PRI2SEC == direct && (src->bus_num == 0 || src->bus_num == 101)) ||
            (MIRROR_SEC2PRI == direct && (src->bus_num == 100 || src->bus_num == 102)) ||
            (MIRROR_SEC2VDEV == direct && (src->bus_num == 100 || src->bus_num == 102))) {
            AALOGI("open dup, target: --> %d, bus: %d", target, src->bus_num);

            src->dup_sink = target;
            dst_p->ref_cnt++;
            AALOGI("dst_p->ref_cnt: %d", dst_p->ref_cnt);
            if (dst_p->ref_cnt == 1) {
                dst_p->hold = false;
                sem_post(&dst_p->sem);
                AALOGI("dup to %d", target);
            }
            src->duplicate = true;
        } else if (MIRROR_PRI2VDEV == direct) {
            src->dup_sink = target;
            src->duplicate = true;
        }
    }
    return 0;
}
static float get_amplitude_ratio(struct alsa_audio_device *adev, char *bus_address) {
    float ret = 0;
    struct alsa_stream_out *out = hashmapGet(adev->out_bus_stream_map, (void *)bus_address);
    ret = out->amplitude_ratio;
    AALOGV("get %s amplitude_ratio: %f", bus_address, ret);
    return ret;
}
static void copy_buf_and_apply_volume(struct alsa_audio_device *adev, char *write_buf,
                                      char *source, int32_t len, char *bus_address) {
    float amplitude_ratio = 0;
    amplitude_ratio = get_amplitude_ratio(adev, bus_address);
    memcpy(write_buf, source, len);
    audio_extra_sw_gain_16(amplitude_ratio, write_buf, len);
}
int32_t au_sharing_duplicate(au_shr_src_t *src, void *buf, uint32_t len, enum au_shr_addr location,
                             enum mirror_direct direct) {
    bool onoff = (direct == MIRROR_CLOSE ? false : true);
    char write_buf[16 * 1024];
    int32_t convert_frames;
    AALOGV("direct: %d, location: %d", direct, location);
    au_sharing_duplicate_update(src, location, direct, onoff);  // got dst
    if (src->duplicate) {
        switch (location) {
            case SSA_SRC_SEC_EP:
                if (direct == MIRROR_SEC2PRI) {  // SSA_2 ---> SSA_1
                    copy_buf_and_apply_volume(g_sharing.dsts[src->dup_sink].dev, write_buf, buf,
                                              len, "bus0_media_out");
                    rbuf_write_async(g_sharing.dsts[src->dup_sink].rbuf, (u_char *)write_buf, len);
                } else if (direct == MIRROR_SEC2VDEV) {  // SSA_2 ---> SSB_1/2
                    AALOGI("SSA-2 Sendto");
                    virtual_i2s_sync_write(&g_sharing.dsts[src->dup_sink], buf, len);
                }
                break;
            case SSA_SRC_PRI_EP:
                if (direct == MIRROR_PRI2SEC) {  // SSA_1  ---> SSA_2
                    copy_buf_and_apply_volume(g_sharing.dsts[src->dup_sink].dev, write_buf, buf,
                                              len, "bus100_rear_seat1");
                    rbuf_write_async(g_sharing.dsts[src->dup_sink].rbuf, (u_char *)write_buf, len);
                } else if (direct == MIRROR_PRI2VDEV) {  // SSA_1 ---> SSB_1/2
                    AALOGV("SSA-1 Sendto");
                    // 4ch -> 2ch
                    convert_frames = audio_extra_select_slot_16(
                        (int16_t *)write_buf, rear_100_cap_cfg_ref.channels, (int16_t *)buf,
                        // src->pcm_cfg.channels,
                        4, len / 2);
                    virtual_i2s_sync_write(&g_sharing.dsts[src->dup_sink], write_buf,
                                           convert_frames * 2);
                }
                break;
            case SSB_SRC_SEC_EP:
                if (direct == MIRROR_SEC2PRI) {
                    AALOGV("SSB_2 ---> SSB_1");
                    copy_buf_and_apply_volume(g_sharing.dsts[src->dup_sink].dev, write_buf, buf,
                                              len, "bus101_rear_seat");
                    rbuf_write_async(g_sharing.dsts[src->dup_sink].rbuf, (u_char *)write_buf, len);
                } else if (direct == MIRROR_PRI2VDEV) {
                    AALOGV("SSB-1 Sendto");
                    virtual_i2s_sync_write(&g_sharing.dsts[src->dup_sink], buf, len);
                }
                break;
            case SSB_SRC_PRI_EP:
                if (direct == MIRROR_PRI2SEC) {
                    AALOGV("SSB_1 ---> SSB_2");
                    copy_buf_and_apply_volume(g_sharing.dsts[src->dup_sink].dev, write_buf, buf,
                                              len, "bus102_rear_seat");
                    rbuf_write_async(g_sharing.dsts[src->dup_sink].rbuf, (u_char *)write_buf, len);
                } else if (direct == MIRROR_PRI2VDEV) {
                    AALOGV("SSB-2 Sendto");
                    // dump_out_data(buf, len, NULL, 21);
                    virtual_i2s_sync_write(&g_sharing.dsts[src->dup_sink], buf, len);
                }
                break;
            default:
                AALOGE("Unsupport location %d", location);
                break;
        }
    }
    return 0;
}

int au_sharing_enable(int endpoint, struct alsa_audio_device *adev) {
    AALOGI("au_sharing_enable  %d", endpoint);
    adev->audio_sharing_fe = false;
    adev->audio_sharing_ne = false;
    if (endpoint == AUDIO_SHARING_NE) {
        adev->audio_sharing_ne = true;
    } else if (endpoint == AUDIO_SHARING_FE) {
        adev->audio_sharing_fe = true;
    }
    return 0;
}

int32_t au_sharing_disable(int endpoint, struct alsa_audio_device *adev) {
    ALOGI("au_sharing_disable  %d", endpoint);
    if (endpoint == AUDIO_SHARING_NE) {
        adev->audio_sharing_ne = false;
    } else if (endpoint == AUDIO_SHARING_FE) {
        adev->audio_sharing_fe = false;
    }

    return 0;
}

bool au_sharing_get_state(au_sharing_t *handler, const char *bus_addr) {
    bool ret = true;
    au_shr_dst_t *dst = NULL;
    if (!strcmp(bus_addr, "bus100_rear_seat1")) {
        dst = &handler->dsts[SSA_SRC_PRI_EP];
    } else if (!strcmp(bus_addr, "bus101_rear_seat")) {
        dst = &handler->dsts[SSB_SRC_SEC_EP];
    } else if (!strcmp(bus_addr, "bus102_rear_seat")) {
        dst = &handler->dsts[SSB_SRC_PRI_EP];
    }
    if (dst) {
        ret = dst->hold;
        AALOGV("%s, dst: %p, hold: %d", bus_addr, dst, ret);
    } else {
        AALOGV("Invalid bus: %s", bus_addr);
    }
    return ret;
}
