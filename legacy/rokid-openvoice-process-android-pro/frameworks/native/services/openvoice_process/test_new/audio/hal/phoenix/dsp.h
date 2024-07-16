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


#ifndef ANDROID_DSP_HAL_INTERFACE_H
#define ANDROID_DSP_HAL_INTERFACE_H

#include <stdbool.h>
#include <hardware/hardware.h>

__BEGIN_DECLS

/**
 * The id of this module
 */
#define DSP_HARDWARE_MODULE_ID "dsp"

/**
 * Name of the dsp devices to open
 */
#define DSP_HARDWARE_INTERFACE "dsp_hw_if"


/**
 * Module versioning information for the Dsp hardware module, based on
 */
#define DSP_MODULE_API_VERSION_0_1 HARDWARE_MODULE_API_VERSION(0, 1)
#define DSP_MODULE_API_VERSION_CURRENT DSP_MODULE_API_VERSION_0_1

/* First generation of audio devices had version hardcoded to 0. all devices with versions < 1.0
 * will be considered of first generation API.
 */
#define DSP_DEVICE_API_VERSION_1_0 HARDWARE_DEVICE_API_VERSION(1, 0)
#define DSP_DEVICE_API_VERSION_CURRENT DSP_DEVICE_API_VERSION_1_0
/* Minimal dsp HAL version supported by the dsp framework */
#define DSP_DEVICE_API_VERSION_MIN DSP_DEVICE_API_VERSION_1_0


typedef enum{
    /*自然原声*/
    SOUND_EFFECT_NATURAL_ACOUSTIC,

    /*清亮人声*/
    SOUND_EFFECT_CLEAR_VOICE,

    /*超重低音*/
    SOUND_EFFECT_MEGA_BASS,

    /*休息模式*/
    SOUND_EFFECT_REST_MODE
}dsp_sound_effect_type_t;

typedef enum{
    /*BMT1*/
    BMT_BASS,
    /*BMT2*/
    BMT_MIDDLE,
    /*BMT3*/
    BMT_TREBLE,
}dsp_bmt_type_t;

typedef enum {
    GEQ_BASS,

    GEQ_MIDDLE_BASS,

    GEQ_MIDDLE,

    GEQ_MIDDLE_HIGH,

    GEQ_HIGH
}dsp_geq_type_t;

typedef enum{
    /*speaker balance*/
    BALANCE,
    /*speaker fader*/
    FADER
}dsp_speaker_type_t;

//typedef enum{
//    STREAM_TYPE_MEDIA_MUSIC,
//    STREAM_TYPE_MEDIA_NAVI,
//    STREAM_TYPE_MEDIA_TUNER,
//    STREAM_TYPE_MEDIA_AUX_IN,
//    STREAM_TYPE_BT_PHONE,
//}dsp_stream_type_t;

typedef enum {
    EQUALIZER_TYPE_FLAT,
    EQUALIZER_TYPE_JAZZ,
    EQUALIZER_TYPE_VOCAL,
    EQUALIZER_TYPE_POP,
    EQUALIZER_TYPE_CLASSIC,
    EQUALIZER_TYPE_ROCK,
}dsp_equalizer_type_t;

typedef enum {
    /*EQ1*/
    EQUALIZER_LOW_FREQUENCY,
    /*EQ2*/
    EQUALIZER_MIDDLE_FREQUENCY,
    /*EQ3*/
    EQUALIZER_HIGH_FREQUENCY,
}dsp_equalizer_freq_type_t;

typedef enum {
	PERIPHERAL,
	TOP_LIGHT,
	HOST,
	PAD,
	NARROW_BAND,
	WIDE_BAND,
	BLUETOOTH_NARROW_BAND,
	BLUETOOTH_WIDE_BAND,
	EAR,
	NOISE_REDUCTION_24K,
}dsp_vcworkmode_type_t;

typedef enum {
	BYPASS,
	NOISE_CLEAN,
	PHONE,
	WAKEUP,
}dsp_vcfunc_type_t;

typedef enum {
    pwm_frequency_501_55khz,
    pwm_frequency_599_41khz,
}dsp_pa_pwm_frequency_t;

typedef enum {
    spread_spectrum_0,
    spread_spectrum_2,
    spread_spectrum_5,
}dsp_pa_spread_spectrum_t;

typedef enum {
    phase_staggering_0ns,
    phase_staggering_81_4ns,
    phase_staggering_162_8ns,
    phase_staggering_244_2ns,
    phase_staggering_325_6ns,
    phase_staggering_407_0ns,
    phase_staggering_488_4ns,
    phase_staggering_569_8ns,
    phase_staggering_651_2ns,
    phase_staggering_732_6ns,
    phase_staggering_814_0ns,
    phase_staggering_895_4ns,
    phase_staggering_976_8ns,
    phase_staggering_1058_2ns,
    phase_staggering_1139_6ns,
    phase_staggering_1221_0ns,
}dsp_pa_phase_staggering_ch_t;

typedef enum {
    am_frequency_522_805KHZ,
    am_frequency_805_965KHZ,
    am_frequency_965_1179KHZ,
    am_frequency_1179_1397KHZ,
    am_frequency_1397_1554KHZ,
    am_frequency_1554_1710KHZ,
    am_frequency_point_999_1008_1503KHZ,
}dsp_pa_am_frequency_t;

typedef enum {
    /*dtcs front only*/
    DTCS_DRIVER_MODE,
    /*dtcs whole vehicle*/
    DTCS_ALL_PASSENGERS,
    /*dtcs rear only*/
    DTCS_PASSENGERS_MODE,
}dsp_dtcs_mode_t;

typedef enum {
    /*bex off*/
    BEX_OFF,
    /*bex on*/
    BEX_ON,
}dsp_bex_switch_t;

typedef enum {
    /*tts off*/
    TTS_OFF,
    /*tts on*/
    TTS_ON,
}dsp_tts_switch_t;

typedef enum {
    /*关闭*/
    SOUND_STAGE_OFF,
    /*驾驶员模式*/
    SOUND_STAGE_DRIVER_MODE,
    /*全车模式*/
    SOUND_STAGE_ALL_PASSENGER_MODE,
}dsp_sound_stage_t;

typedef enum {
    AUDIO_BALANCE_MUSIC,

    AUDIO_BALANCE_FM,

    AUDIO_BALANCE_AM,
}dsp_balance_source_t;

typedef enum {
    SOUND_CHANNEL_RR,
    SOUND_CHANNEL_RL,
    SOUND_CHANNEL_FR,
    SOUND_CHANNEL_FL,
}sound_channel_type_t;

typedef enum {
    ECNR_TYPE_VR,
    ECNR_TYPE_BT_PHONE,
    ECNR_TYPE_CP_PHONE,
}dsp_ecnr_type_t;

typedef enum{
    PDC_CHANNEL_NONE = 0x00,
    PDC_CHANNEL_FL = 0x01,          // bit 0
    PDC_CHANNEL_FR = 0x02,          // bit1
    PDC_CHANNEL_FL_FR,
    PDC_CHANNEL_RL = 0x04,          // bit2
    PDC_CHANNEL_FL_RL,
    PDC_CHANNEL_FR_RL,
    PDC_CHANNEL_FL_FR_RL,
    PDC_CHANNEL_RR = 0x08,          // bit3
    PDC_CHANNEL_FL_RR,
    PDC_CHANNEL_FR_RR,
    PDC_CHANNEL_FL_FR_RR,
    PDC_CHANNEL_RL_RR,
    PDC_CHANNEL_FL_RL_RR,
    PDC_CHANNEL_FR_RL_RR,
    PDC_CHANNEL_ALL = 0x0F,
}dsp_pdc_channel_t;

typedef enum{
    PDC_TYPE_STOP,
    PDC_TYPE_SLOW,
    PDC_TYPE_MID,
    PDC_TYPE_FAST,
    PDC_TYPE_CONT,
}dsp_pdc_type_t;



__END_DECLS

#endif  // ANDROID_DSP_INTERFACE_H
