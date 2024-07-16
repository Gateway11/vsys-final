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

#ifndef __AUDIO_BT_CALL__
#define __AUDIO_BT_CALL__

#define PORT_CODEC 0
#define PORT_BT 1

#define BT_CALL_BUS_NAME "bus8_bt_phone_out"
#define BT_CALL_HFP_OUT_NAME "hfp_out"
#define BT_CALL_HFP_IN_NAME "hfp_in"

int btcall_open(struct alsa_audio_device *adev);
int btcall_close(struct alsa_audio_device *adev);

int btcall_init_rx(struct alsa_audio_device *adev);
int btcall_init_tx(struct alsa_audio_device *adev);

void *btcall_thread_rx(void *data);
void *btcall_thread_tx(void *data);

typedef enum _btcall_rt {
    BTCALL_BT_INPUT = 0,  // bt in
    BTCALL_BT_OUTPUT,     // bt out
    BTCALL_CODEC_INPUT,   // codec in
    BTCALL_CODEC_OUTPUT,  // codec out
    BTCALL_RT_MAX,
} btcall_rt;

struct btcall_data {
    int card;
    int port;
    void *pcm;
    struct alsa_stream_in *in;
    struct pcm_config *config;
    struct alsa_stream_out *out;     // out bus
    struct alsa_stream_out *bt_out;  // bt private out

    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
    size_t frames_in;
    unsigned int rsp_in_rate;
    unsigned int rsp_out_rate;
    int direction;
    int read_status;
    struct alsa_audio_device *adev;
};

struct btcall_thread_data {
    int start_work_rx;
    int exit_work_rx;
    int start_work_tx;
    int exit_work_tx;
    int is_running_tx;
    int is_running_rx;
    pthread_t thread_rx;
    pthread_t thread_tx;
    pthread_cond_t cond_rx;
    pthread_cond_t cond_tx;
    pthread_mutex_t lock_rx;
    pthread_mutex_t lock_tx;

    void *rsp_out_buffer_tx;
    void *rsp_out_buffer_rx;
    void *buffer_tx;
    void *buffer_rx;
    int buffer_txsize;
    int buffer_rxsize;
};

#endif
