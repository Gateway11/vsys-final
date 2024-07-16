/*
 * Remote processor messaging sockets
 *
 * Copyright (C) 2011-2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Ohad Ben-Cohen <ohad@wizery.com>
 * Suman Anna <s-anna@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _AUDIO_DSP_COMMON_H
#define _AUDIO_DSP_COMMON_H

#include "dsp.h"

__BEGIN_DECLS

typedef enum {
    /* media_i2s channel*/
    CHANNEL_MEDIA_MUSIC,
    /* tuner_i2s/adc2_tuner channel*/
    CHANNEL_MEDIA_TUNER,
    /* bt_i2s channel*/
    CHANNEL_BT_PHONE,
    /* carplay channel*/
    CHANNEL_CARPLAY_PHONE,
    /* tbox channel*/
    CHANNEL_TBOX,
    /* siri channel*/
    CHANNEL_SIRI,
	/* navi channel*/
    CHANNEL_NAVI,
	/* beep channel*/
    CHANNEL_BEEP,
    /* vr channel*/
    CHANNEL_VR,

    CHANNEL_NULL
} dsp_channel_type_t;

int dspdev_set_channel(dsp_channel_type_t channel_type);

int dspdev_get_channel(dsp_channel_type_t* channel);

int dspdev_set_tuner_mixer(bool on);

int dspdev_set_volume(dsp_channel_type_t channel_type, int value);

int dspdev_get_volume(dsp_channel_type_t channel_type, int *value);

int dspdev_set_volume_offset(dsp_channel_type_t channel_type,int value);

int dspdev_get_volume_offset(dsp_channel_type_t type, int *value);

static int dspdev_set_variable_max_volume(dsp_channel_type_t channel_type,int value);

int dspdev_get_variable_max_volume(dsp_channel_type_t type, int *value);

int dspdev_get_max_volume(dsp_channel_type_t type, int *value);

int dspdev_set_music_mute(bool mute);

int dspdev_get_music_mute(bool *mute);

int dspdev_set_media_mute(bool mute);

int dspdev_set_navi_mute(bool mute);

int dspdev_get_navi_mute(bool *mute);

int dspdev_set_tbox_mute(bool mute);

int dspdev_get_tbox_mute(bool *mute);

int dspdev_set_beep_mute(bool mute);

int dspdev_get_beep_mute(bool *mute);

int dspdev_set_phone_mute(bool mute);

int dspdev_get_phone_mute(bool *mute);

int dspdev_set_tuner_mute(bool mute);

int dspdev_get_tuner_mute(bool *mute);

int dspdev_set_mic_mute(bool mute);

int dspdev_get_mic_mute(bool *mute);

int dspdev_set_amplifier_mute(bool mute);

int dspdev_set_mic_volume(int value);

int dspdev_get_mic_volume(int *value);

int dspdev_set_mcu_app_mic_test(bool on_off);

int dspdev_get_mcu_app_mic_test(bool *on_off);

int dspdev_set_loudness(bool enable);

int dspdev_get_loudness(bool *enable);

int dspdev_set_sound_effect(dsp_sound_effect_type_t sound_type);

int dspdev_get_sound_effect(dsp_sound_effect_type_t *sound_type);

int dspdev_set_bass_mid_treble(dsp_bmt_type_t bmt_type, int value);

int dspdev_get_bass_mid_treble(dsp_bmt_type_t bmt_type, int *value);

int dspdev_set_geq(dsp_geq_type_t geq_type, int value);

int dspdev_get_geq(dsp_geq_type_t geq_type, int *value);

int dspdev_set_sound_field(int balance, int fader);

int dspdev_get_sound_field(int *balance, int *fader);

int dspdev_set_dtcs_mode(dsp_dtcs_mode_t mode);

int dspdev_get_dtcs_mode(dsp_dtcs_mode_t* mode);

int dspdev_set_dtcs_status(bool status);

int dspdev_get_dtcs_status(bool* status);

int dspdev_set_ale_status(bool status);

int dspdev_get_ale_status(bool* status);

int dspdev_set_bex_status(dsp_bex_switch_t status);

int dspdev_get_bex_status(dsp_bex_switch_t* status);

int dspdev_set_tts_status(dsp_tts_switch_t status);

int dspdev_get_tts_status(dsp_tts_switch_t* status);

int dspdev_set_vcworkmode(dsp_vcworkmode_type_t workmode);

int dspdev_get_vcworkmode(dsp_vcworkmode_type_t* workmode);

int dspdev_set_vcfunc(dsp_vcfunc_type_t func);

int dspdev_get_vcfunc(dsp_vcfunc_type_t* func);

int dspdev_set_vcvolume(int value);

int dspdev_get_vcvolume(int *value);

int dspdev_set_pa_pwm_frequency(dsp_pa_pwm_frequency_t frequency);

int dspdev_get_pa_pwm_frequency(dsp_pa_pwm_frequency_t* frequency);

int dspdev_set_pa_spread_spectrum(dsp_pa_spread_spectrum_t spread_spectrum);

int dspdev_get_pa_spread_spectrum(dsp_pa_spread_spectrum_t* spread_spectrum);

int dspdev_set_pa_phase_staggering_ch(int index, dsp_pa_phase_staggering_ch_t phase_staggering);

int dspdev_get_pa_phase_staggering_ch(int index, dsp_pa_phase_staggering_ch_t* phase_staggering);

int dspdev_set_pa_am_frequency(dsp_pa_am_frequency_t frequency);

int dspdev_get_pa_am_frequency(dsp_pa_am_frequency_t* frequency);

int dspdev_set_sound_stage(dsp_sound_stage_t stage);

int dspdev_get_sound_stage(dsp_sound_stage_t *stage);

int dspdev_set_audio_balance(dsp_balance_source_t source);

int dspdev_set_sound_channel_mute(sound_channel_type_t sound_channel, bool mute);

int dspdev_set_ecnr_type(dsp_ecnr_type_t type);

int dspdev_get_ecnr_type(dsp_ecnr_type_t *type);

int dspdev_set_mcu_app_speaker_test(dsp_pdc_channel_t channel, bool on_off);

int dspdev_get_mcu_app_speaker_test(dsp_pdc_channel_t channel, bool *on_off);

int dspdev_get_dsp_firmware_version(int *version);

int dspdev_open_external_amp_test(bool on);

int dspdev_set_mcu_reset_dsp(bool on);

static int dspdev_set_diag_sound_effect_off(bool on);

int dspdev_set_secondary_mono_all(bool all);

int dspdev_get_secondary_mono_all(bool *all);

int dspdev_set_secondary_steoro_all(bool all);

int dspdev_get_secondary_steoro_all(bool *all);

int dspdev_set_navi_mono_all(bool all);

int dspdev_get_navi_mono_all(bool *all);

int dspdev_set_phone_mono_all(bool all);

int dspdev_get_phone_mono_all(bool *all);

__END_DECLS

#endif /* _AUDIO_DSP_COMMON_H */
