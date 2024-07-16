/*
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author: mengmeng.chang
 * @Date: 2022-04-08 15:20:36
 * @Descripttion:
 * @Revision History:
 * ----------------
 */

#ifndef HAL_STREAMER_H
#define HAL_STREAMER_H

#include "audio_hw.h"
#define MAX_CLIENT_NUM 16
#define PCM_DATA_DEFAULT_BYTES (16 * 1024)
#define PROXY_OPEN_RETRY_COUNT 100
#define PROXY_OPEN_WAIT_TIME 20

enum stream_direction {
    PLAYBACK_STREAM,
    CAPTURE_STREAM,
};
enum cap_dst_e {
    FOUR_MIC_CAP,
    BACK_MIC_CAP,
    BUILD_IN_MIC_CAP,
    EXT_MIC_CAP,
};
typedef struct hal_streamer {
    char name[128];
    int32_t card;
    int32_t port;
    int32_t ref_cnt;
    pthread_mutex_t lock;
    bool allocated;
    int32_t bind_bus[MAX_AUDIO_DEVICES];
    int32_t bind_bus_len;
    struct pcm *pcm;
    struct pcm_config pcm_cfg;
    pthread_t plbk_thread;
    bool plbk_exit;
    bool plbk_hold;  // for low latency
    sem_t plbk_sem;
    pthread_t cap_thread;
    bool cap_exit;
    sem_t cap_sem;
    audio_circlebuf_t *bus_rbuf[MAX_CLIENT_NUM];
    audio_circlebuf_t *cap_rbuf[MAX_CLIENT_NUM];
    uint16_t bus_state;
    uint16_t cap_state;
    enum stream_direction dir;
    struct alsa_audio_device *adev;
    int32_t bus_buf_sz[MAX_CLIENT_NUM];  // do not calculate every time
    int32_t cap_buf_sz[MAX_CLIENT_NUM];
} hal_streamer_t;

/*============ api ===============*/
int32_t hal_streamer_bind_bus(const char *name, int32_t len, int32_t bind_bus);
hal_streamer_t *hal_streamer_open(struct audio_stream *stream, int32_t card, int32_t port,
                                  int32_t flag, struct pcm_config *pcm_cfg);
int32_t hal_streamer_write(struct alsa_stream_out *out, const void *data, uint32_t count);
int32_t hal_streamer_read(struct alsa_stream_in *in, const void *data, uint32_t count);
int32_t hal_streamer_close(struct audio_stream *stream, int32_t flag);

#endif  // HAL_STREAMER_H