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
#ifndef SEMIDRIVE_AUDIO_HW_H
#define SEMIDRIVE_AUDIO_HW_H

//#include <cutils/bitops.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <cutils/hashmap.h>
#include <audio_route/audio_route.h>
#include <audio_utils/resampler.h>
#include <tinyalsa/asoundlib.h>
#include <cutils/list.h>
#include <system/audio.h>
#include <hardware/audio.h>
#include <utils/AndroidThreads.h>
#include "platform.h"
#include <system/thread_defs.h>
#include "pcm_wrapper.h"
#include "audio_bt_call.h"
#include "hal_streamer.h"

#define AALOGV(fmt, ...) ALOGV("%d %s: " fmt " ", __LINE__, __func__, ##__VA_ARGS__)
#define AALOGI(fmt, ...) ALOGI("%d %s: " fmt " ", __LINE__, __func__, ##__VA_ARGS__)
#define AALOGE(fmt, ...) ALOGE("###ERROR %s, %d: " fmt " .", __func__, __LINE__, ##__VA_ARGS__)

//#undef offsetof
//#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member)                    \
    ({                                                     \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })

#define AUDIO_FMT_2_BYTES(fmt) (pcm_format_to_bits(fmt) >> 3)

#define get_bit(dat, i) ((dat & (0x0001 << i)) ? 1 : 0)
#define set_bit(dat, i) ((dat) |= (0x0001 << (i)))
#define clear_bit(dat, i) ((dat) &= (~(0x0001 << (i))))
#define HIFI_PATH_FIXED_VOL 100
#define PHONE_PATH_FIXED_VOL 100

/* Sound card definition */
#define CARD_DEFAULT 0
#define CODEC_DEV 2
#define CODEC_DEV_IN 0
#define PHONE_DEV 1

// audio sharing
#define REFA_VI2S_DEV 1
#define REFB_VI2S_DEV 1

#define RS_HP_1_DEV 0
#define RS_HP_2_DEV 0

#define CODEC_NUM 2

/* Parameter definitions */

#define CODEC_DEV_FRAME_COUNT 32

#define PERIOD_MULTIPLIER 32
/* codec number of frames per short period (low latency) */
#define PERIOD_SIZE (CODEC_DEV_FRAME_COUNT * PERIOD_MULTIPLIER)

#define RESAMPLER_BUFFER_SIZE (8 * PERIOD_SIZE)

/* number of pseudo periods for low latency playback */
#define CAPTURE_PERIOD_COUNT 4
#define PLAYBACK_PERIOD_COUNT 6
#define PLAYBACK_PERIOD_START_THRESHOLD 4
#define PLAYBACK_PERIOD_STOP_THRESHOLD PLAYBACK_PERIOD_COUNT

#define IN_PERIOD_MS 20
/* A short period (low latency) 20 ms */
#define OUT_PERIOD_MS 20

#define SAMPLING_RATE_8K 8000
#define SAMPLING_RATE_16K 16000
#define SAMPLING_RATE_24K 24000
#define SAMPLING_RATE_32K 32000
#define SAMPLING_RATE_48K 48000

#define SAMPLING_RATE_11K 11025
#define SAMPLING_RATE_22K 22050
#define SAMPLING_RATE_44K 44100

#define LOFI_SAMPLING_RATE SAMPLING_RATE_16K
#define HIFI_SAMPLING_RATE SAMPLING_RATE_48K

/* It's different form tiny alsa pcm format*/
#define HIFI_AUDIO_FORMAT AUDIO_FORMAT_PCM_16_BIT
#define CHANNEL_MONO 1
#define CHANNEL_STEREO 2

/*
    OUT_FRONT_LEFT | OUT_FRONT_RIGHT |
    OUT_TOP_SIDE_LEFT | OUT_TOP_SIDE_RIGHT
*/
#define CHANNEL_2POINT0POINT2 4

/*
    OUT_FRONT_LEFT | OUT_FRONT_RIGHT |
    OUT_FRONT_CENTER | OUT_LOW_FREQUENCY |
    OUT_BACK_LEFT | OUT_BACK_RIGHT |
    OUT_SIDE_LEFT | OUT_SIDE_RIGHT
*/
#define CHANNEL_7POINT1 8
#define MIN_WRITE_SLEEP_US 5000

/*support params*/
#define MAX_SUPPORTED_CHANNEL_MASKS (2 * FCC_8) /* support positional and index masks to 8ch */
#define MAX_SUPPORTED_FORMATS 15
#define MAX_SUPPORTED_SAMPLE_RATES 7

#define DEBUG_FUNC_PRT ALOGV(" %s:%i <-- \n", __func__, __LINE__);

#define DEBUG_ITEM_PRT(item) \
    ALOGV(" %s:%i [%s]: --> 0x%x(%d) \n", __func__, __LINE__, #item, (int)item, (int)item);

#define STRING_TO_ENUM(string) \
    { #string, string }
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define REAR_SEAT1_HS_STATUS 0x01
#define REAR_SEAT2_HS_STATUS 0x02
#define REAR_SEAT3_HS_STATUS 0x04
#define AUDIO_HAL_PLAYBACK 0
#define AUDIO_HAL_CAPTURE 1
#define MAX_BUS_NUM 16

enum bus_num {
    BUS0 = 0,
    BUS1,
    BUS2,
    BUS3,
    BUS4,
    BUS5,
    BUS6,
    BUS7,
    BUS8,
    BUS9,//SSA sharing bus
    BUS100 = 100,
    BUS101 = 101,
    BUS102 = 102,
    BUS103 = 103,//SSB sharing bus
};

struct string_to_enum {
    const char *name;
    uint32_t value;
};

union stream_ptr {
    struct stream_in *in;
    struct stream_out *out;
};

struct audio_route_l {
    struct listnode list;
    struct audio_route *audio_route;
    int card_id;
};

struct audio_bus_status {
    pthread_mutex_t lock[MAX_BUS_NUM];
    bool is_ducked[MAX_BUS_NUM];
    bool is_muted[MAX_BUS_NUM];
	int32_t in_zoneId[MAX_BUS_NUM];
	uint8_t volume[MAX_BUS_NUM];
	uint8_t bus_active[MAX_BUS_NUM];
	uint8_t slot_index[MAX_BUS_NUM];
	uint8_t slot_in_use[4];
	bool is_chime_on;
	uint8_t focus_bus[4];
};

struct alsa_stream_in {
    struct audio_stream_in stream;
    pthread_mutex_t lock;
    struct pcm_config config;
    struct pcm *pcm;
    audio_devices_t devices;
    int standby;
    struct alsa_audio_device *dev;
    int source;
    audio_io_handle_t capture_handle;
    audio_input_flags_t flags;
    const char *bus_address; /*  Extended field. Constant after init for car */
    // int snd_card;
    // int snd_dev;

    /*+1 so that the last entry is always 0 */
    audio_channel_mask_t supported_channel_masks[MAX_SUPPORTED_CHANNEL_MASKS + 1];
    audio_format_t supported_formats[MAX_SUPPORTED_FORMATS + 1];
    uint32_t supported_sample_rates[MAX_SUPPORTED_SAMPLE_RATES + 1];
    pcm_wrapper_t *pcm_wrapper;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
    unsigned int requested_rate;
    size_t frames_in;
    int read_status;
    struct hal_streamer *pcm_hdl;

#ifdef ENABLE_AUDIO_SHAREING
    bool reset_stream_input_node;
    bool is_audiosharing_input_node;
#endif
};

struct alsa_stream_out {
    struct audio_stream_out stream;
    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    struct pcm_config config;
    struct pcm *pcm;
    struct hal_streamer *pcm_hdl;
    int standby;
    struct alsa_audio_device *dev;
    int write_threshold;
    uint64_t written;
    const char *bus_address; /*  Extended field. Constant after init for car */
    int32_t bus_num;
    struct audio_gain gain_stage; /* Constant after init */
    float amplitude_ratio;        /* Protected by this->lock */

    // Time & Position Keeping
    uint64_t underrun_position;
    struct timespec underrun_time;
    uint64_t last_write_time_us;
    uint64_t frames_total_buffered;
    uint64_t frames_written;
    uint64_t frames_rendered;

    // Effect
    audio_effect_library_t *effect_desc;
    effect_handle_t effect_hdl;
    char *processed_buf;
    uint32_t processed_buf_len;
    int32_t target_gain_mb;
    struct resampler_itfe *resampler;
    void *rsp_out_buffer;

    /* +1 so that the last entry is always 0 */
    audio_channel_mask_t supported_channel_masks[MAX_SUPPORTED_CHANNEL_MASKS + 1];
    audio_format_t supported_formats[MAX_SUPPORTED_FORMATS + 1];
    uint32_t supported_sample_rates[MAX_SUPPORTED_SAMPLE_RATES + 1];

#ifdef ENABLE_AUDIO_SHAREING
    au_shr_src_t *repeater_src;
#endif
};

struct alsa_audio_device {
    struct audio_hw_device hw_device;

    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    int devices;
    struct alsa_stream_in *active_input;
    struct alsa_stream_out *active_output;
    struct platform_data *platform;
    bool mic_mute;
    bool is_usb_exist;
    struct mixer *mixers[CODEC_NUM];
    unsigned int last_patch_id;
    int snd_card;
    int usb_card_id;
    int usb_port_id;
    Hashmap *out_bus_stream_map;
    struct listnode audio_route_list;
    // hfp
    int hfp_sample_rate;
    bool enable_hfp;
    int hfp_volume;

    struct btcall_data *bt_data[BTCALL_RT_MAX];
    struct btcall_thread_data *bt_thread_data;
    audio_mode_t mode;
    int in_call;
    int hs_device;
    struct ecnr_common_ops *ecnr_ops;
    void *ecnr_hdl;

    pthread_t ipcc_thread;

#ifdef ENABLE_AUDIO_SHAREING
    // audio sharing 0 off. 1. Sharing Near End, 2 Sharing Far End.
    bool audio_sharing_fe;
    bool audio_sharing_ne;
    enum mirror_direct direct;
#endif
};

/*
 * frameworks/base/include/media/AudioSystem.h definitions
 */

static inline int is_x9u_ref_a() {
#ifdef X9U_REF_A
    return 1;
#else
    return 0;
#endif
}
static inline int is_x9u_ref_b() {
#ifdef X9U_REF_B
    return 1;
#else
    return 0;
#endif
}
extern struct audio_bus_status current_audio_bus_status;

struct pcm *pcm_open_prepare_helper(unsigned int snd_card, unsigned int pcm_device_id,
                                    unsigned int flags, unsigned int pcm_open_retry_count,
                                    struct pcm_config *config);
void open_output_path(struct alsa_stream_out *out);
void close_output_path(struct alsa_stream_out *out);
int set_bus_volume( uint32_t which_bus );

__BEGIN_DECLS
void set_device_address_is_ducked(uint32_t device_address, bool is_ducked);
void set_device_address_is_muted(uint32_t device_address, bool is_muted);
__END_DECLS
#endif /* SEMIDRIVE_AUDIO_HW_H */
