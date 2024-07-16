/*
 * audio_repeater.h
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author: mengmeng.chang
 * @Date: 2022-03-14 19:39:57
 * @Descripttion:
 * @Revision History:
 * ----------------
 */
#ifndef AUDIO_SHARING_H
#define AUDIO_SHARING_H

#include "audio_hw.h"
#include "ext_circlebuffer.h"
#include "pcm_wrapper.h"
#include <cutils/hashmap.h>
#include <pthread.h>
#include <semaphore.h>
#include <system/thread_defs.h>
#include <tinyalsa/asoundlib.h>
#include <utils/AndroidThreads.h>

#define AUDIO_PARAMETER_AUDIO_SHARING_NE "audio_sharing_nearend"
#define AUDIO_PARAMETER_AUDIO_SHARING_FE "audio_sharing_farend"
#define AUDIO_PARAMETER_AUDIO_SHARING_PRI_SHARING "pri_sharing"
#define AUDIO_PARAMETER_AUDIO_SHARING_SEC_SHARING "sec_sharing"
#define AUDIO_PARAMETER_AUDIO_SHARING_PRI_TO_SEC "pri_to_sec"
#define AUDIO_PARAMETER_AUDIO_SHARING_SEC_TO_PRI "sec_to_pri"

#define AUDIO_SHARING_NE 0
#define AUDIO_SHARING_FE 1

enum mirror_direct {
    MIRROR_CLOSE,
    MIRROR_PRI2SEC,   // ssa1 -> ssa2, ssb1 -> ssb2
    MIRROR_SEC2PRI,   // ssa2 -> ssa1, ssb2 -> ssb1
    MIRROR_PRI2VDEV,  // ssa1 -> ssb1/ssb2, ssb1 -> ssa1/ssa2
    MIRROR_SEC2VDEV,  // ssa2 -> ssb1/ssb2, ssb2 -> ssa1/ssa2
};

enum au_shr_addr {
    SSA_SRC_PRI_EP,
    SSA_SRC_SEC_EP,
    SSB_SRC_PRI_EP,
    SSB_SRC_SEC_EP,
    ADDR_MAX,
};

typedef struct au_shr_cfg {
    struct pcm_config pcm_cfg;
    struct alsa_stream_out *stream;
    struct alsa_audio_device *dev;
} au_shr_cfg_t;

typedef struct au_shr_dst {
    audio_circlebuf_t *rbuf;
    sem_t sem;
    pthread_t looper;
    struct hal_streamer *pcm_hdl;
    // pcm_wrapper_t *pcm_wrapper;
    struct pcm *pcm_vi2s;
    struct pcm *pcm;
    bool hold;
    int32_t ref_cnt;
    struct alsa_stream_out *stream;
    bool first_done;
    enum au_shr_addr addr;
    struct alsa_audio_device *dev;
} au_shr_dst_t;

typedef struct au_shr_src {
    uint32_t bus_num;
    struct pcm_config pcm_cfg;
    struct alsa_stream_out *stream;
    au_shr_dst_t *sink;
    enum au_shr_addr dup_sink;
    bool duplicate;
    bool allocated;
} au_shr_src_t;

typedef struct au_sharing {
    char name[128];
    au_shr_dst_t dsts[ADDR_MAX];
    au_shr_src_t srcs[ADDR_MAX];
} au_sharing_t;

au_sharing_t *au_sharing_init(au_shr_cfg_t *cfg);
au_shr_src_t *au_sharing_get_src(au_shr_cfg_t *cfg);

// int32_t au_sharing_queue(au_shr_src_t *src, void *buf, uint32_t len);
int32_t au_sharing_duplicate(au_shr_src_t *src, void *buf, uint32_t len, enum au_shr_addr dst,
                             enum mirror_direct direct);
int32_t au_sharing_release_src(au_shr_src_t *src);
int32_t au_sharing_disable(int endpoint, struct alsa_audio_device *adev);
int au_sharing_enable(int endpoint, struct alsa_audio_device *adev);
bool au_sharing_get_state(au_sharing_t *handler, const char *bus_addr);

#endif