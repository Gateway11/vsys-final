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
#define LOG_TAG "dsp_hal"
#define LOG_NDEBUG 0

#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>

#include <tinyalsa/asoundlib.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <sys/system_properties.h>
#include <hardware/hardware.h>

#include "audio_dsp_common.h"

#if defined(PLATFORM_CHERY_AUTOMOBILE) || defined(PLATFORM_CHERY_COMMERCIAL)
#define PLATFORM_CHERY
#endif

#define CAR_DOWNLINE_CONFIG ""
#define	MEDIA_VOL_MAX      39
#define PHONE_VOL_MAX	   39

#define	NAVI_VOL_MAX       39
#define	MEDIA_VOL_INIT     0
#define	PHONE_VOL_INIT     0
#define BMT_VALUE_MAX	20
#define GEQ_VALUE_MAX 14

static struct pcm_config config = {
        .channels = 2,
        .rate = 48000,
        .period_size = 512,
        .period_count = 4,
        .format = PCM_FORMAT_S16_LE,
};

static const char* tinymix_list_controls[5]={"SEC_MI2S_TX Bit Format","SEC_MI2S_RX Audio Mixer MultiMedia1","MultiMedia1 Mixer SEC_MI2S_TX","TERT_MI2S_TX Bit Format","TERT_MI2S_RX Audio Mixer MultiMedia2"};
static const char* tinymix_control_MediaVol = "Media_Vol";	//Media_vol:0 (dsrange 0->120)
static const char* tinymix_control_NaviVol = "Navi_Vol";	//Navi_vol: 0 (dsrange 0->66)
static const char* tinymix_control_BeepVol = "Beep_Vol";
static const char* tinymix_control_TboxVol = "Tbox_Vol";	//Tbox_vol: 0 (dsrange 0->66)
static const char* tinymix_control_PhoneVol = "Phone_Vol";	//Phone_Vol: 0 (dsrange 0->10)
static const char* tinymix_control_Source_Select = "Source_Select";		//Input SW: >Music Tuner AUX_IN ALL0
static char* tinymix_control_Source_Select_Music = "Music";
static char* tinymix_control_Source_Select_Tuner = "Tuner";
static char* tinymix_control_Source_Select_BT = "BT";
static char* tinymix_control_Source_Select_CP_Phone = "CP Phone";
static char* tinymix_control_Source_Select_Tbox = "Tbox";
static char* tinymix_control_Source_Select_Siri = "Siri";
static char* tinymix_control_Source_Select_VR = "VR";
static char* tinymix_control_Source_Select_NULL = "NULL";
static char* tinymix_control_Mute_On = "1";
static char* tinymix_control_Mute_Off = "0";
static const char* tinymix_control_MicVol = "Mic_vol";	//Mic_vol: 10 (dsrange 0->10)
static const char* tinymix_control_MediaMute = "Media_Mute";	//Media_Mute: Off/On
static const char* tinymix_control_NaviMute = "Navi_Mute";	//Navi_Mute: Off/On
static const char* tinymix_control_TboxMute = "Tbox_Mute";	//Tbox_Mute: Off/On
static const char* tinymix_control_BeepMute = "Beep_Mute";	//Beep_Mute: Off/On
static const char* tinymix_control_PhoneMute = "Phone_Mute";	//Phone_Mute: Off/On
static const char* tinymix_control_TunerMute = "Tuner_Mute";	//Tuner_Mute: Off/On
static const char* tinymix_control_MicMute = "MIC_Mute";	//MIC_Mute: Off/On
static const char* tinymix_control_Loudness = "Loudness_SW";	//Loudness_SW: Off/On

static const char* tinymix_control_Bass = "Bass";	//Bass: (dsrange 0->20)
static const char* tinymix_control_Middle = "Middle";	//Middle: (dsrange 0->20)
static const char* tinymix_control_Treble = "Treble";	//Treble: (dsrange 0->20)
static const char* tinymix_control_Bass_EQ = "Sony_Geq1";		//dsrange 0->14
static const char* tinymix_control_Middle_Bass_EQ = "Sony_Geq2";	//dsrange 0->14
static const char* tinymix_control_Middle_EQ = "Sony_Geq3";		//dsrange 0->14
static const char* tinymix_control_Middle_High_EQ = "Sony_Geq4";	//dsrange 0->14
static const char* tinymix_control_High_EQ = "Sony_Geq5";		//dsrange 0->14

static const char* tinymix_control_Balance = "Balance";		//Balance: 0 (dsrange 0->20)
static const char* tinymix_control_Fader  = "Fader";		//Fader: 0 (dsrange 0->20)
static const char* tinymix_control_VCWorkMode = "VCWorkMode";	//VCWorkMode: Default Peripheral >Top light Host Pad Narrow band Wide band Bluetooth narrow band Bluetooth wide band Ear Noise reduction 24K
static const char* tinymix_control_VCFunc = "VCFunc";	//Bypass >Noise clean Phone Wakeup
static const char* tinymix_control_VCVolume = "VCVolume";	//VCVolume: -10 (dsrange 0->29)
static const char* tinymix_control_DtcsModeSw = "DTCS MODE SW";	//DTCS MODE SW: >Driver Mode All PAssengers Passengers Mode
static const char* tinymix_control_DtcsSw = "DTCS SW";	//DTCS SW: Off/On
static const char* tinymix_control_AleSw = "ALE A SW";	//ALE A SW: Off/On
static const char* tinymix_control_BexSw = "BEX SW";	//BEX SW: >BEX OFF BEX ON
static const char* tinymix_control_TtsSw = "TTS SW";	//TTS SW: >TTS OFF TTS ON
static const char* tinymix_control_PA_pwm_frequency = "PA_pwm_frequency";	//PA_pwm_frequency: >501.55khz 599.41khz
static const char* tinymix_control_PA_spread_spectrum = "PA_spread_spectrum";	//PA_spread_spectrum : 0 2% >5%
static const char* tinymix_control_PA_phase_staggering_ch1 = "PA_phase_staggering_ch1";	//PA_phase_staggering_ch1: >0ns 81.4ns 162.8ns 244.2ns 325,6ns 407.0ns 488.4ns 569.8ns 651.2ns 732.6ns 814.0ns 895.4ns 976.8ns 1058.2ns 1139.6ns 1221.0ns
static const char* tinymix_control_PA_phase_staggering_ch2 = "PA_phase_staggering_ch2";	//PA_phase_staggering_ch2: >0ns 81.4ns 162.8ns 244.2ns 325,6ns 407.0ns 488.4ns 569.8ns 651.2ns 732.6ns 814.0ns 895.4ns 976.8ns 1058.2ns 1139.6ns 1221.0ns
static const char* tinymix_control_PA_phase_staggering_ch3 = "PA_phase_staggering_ch3";	//PA_phase_staggering_ch3: >0ns 81.4ns 162.8ns 244.2ns 325,6ns 407.0ns 488.4ns 569.8ns 651.2ns 732.6ns 814.0ns 895.4ns 976.8ns 1058.2ns 1139.6ns 1221.0ns
static const char* tinymix_control_PA_phase_staggering_ch4 = "PA_phase_staggering_ch4";	//PA_phase_staggering_ch4: >0ns 81.4ns 162.8ns 244.2ns 325,6ns 407.0ns 488.4ns 569.8ns 651.2ns 732.6ns 814.0ns 895.4ns 976.8ns 1058.2ns 1139.6ns 1221.0ns
static const char* tinymix_control_PA_AM_frequency = "PA_AM_frequency";	//PA_AM_frequency: >522-935KHZ 953-1153KHZ 1153-1429KHZ 1429-1538KHZ 1538-1710KHZ
static const char* tinymix_control_PA_Mute = "PA_Mute";	//PA_Mute: Off/On
static const char* tinymix_control_PA_caculate_caribration = "PA_caculate_caribration";
static const char* tinymix_control_PA_set_caribration = "PA_set_caribration";
static const char* tinymix_control_Sound_Effect = "Sound_Mode";
static const char* tinymix_control_Sound_Stage = "Sound_Stage";
static const char* tinymix_control_ExtPaConfig = "ExtPa_Config";
static const char* tinymix_control_SpeakerNum = "Speaker_Num";
static const char* tinymix_control_SpeakerType = "Speaker_Type";
static const char* tinymix_contorl_BalanceSource = "Media_Offset";
static const char* tinymix_contorl_SoundChannel_Mute = "Test_Channel_Mute";
static const char* tinymix_control_ExternalPaTest = "DIAG_AM8_EXT_ADC";
static const char* tinymix_control_DiagMicTest = "DIAG_MIC_Test";
static const char* tinymix_control_EcnrType = "Ecnr_Type";
static const char* tinymix_control_DiagPDCVol = "DIAG_PDC_Vol";
static const char* tinymix_control_DiagPDCChannel = "DIAG_PDC_Channel";
static const char* tinymix_control_DiagPDCType = "DIAG_PDC_Type";
static const char* tinymix_control_dsp_firmware_version = "Get_Firmware_Version";
static const char* tinymix_control_MCU_Reset_DSP = "MCU_Reset_DSP";
static const char* tinymix_control_DiagSoundEffectOff = "DIAG_Sound_Effect_Off";
static const char* tinymix_control_secondary_mono_all = "Secondary_Mono_All";
static const char* tinymix_control_secondary_steoro_all = "Secondary_Steoro_All";
static const char* tinymix_control_navi_mono_all = "Navi_Mono_All";
static const char* tinymix_control_phone_mono_all = "Phone_Mono_All";

static int play_sound(unsigned char* sound_array, int size);
static int dsp_init();
static int dsp_control(const char* control, char* values);
static int dsp_get_value(const char* control, int* value);
static int tinymix_detail_control(struct mixer *mixer, const char *control,int *value,
                                  int prefix, int print_all);
static int tinymix_set_value(struct mixer *mixer, const char *control,
                             char **values, unsigned int num_values);
static void tinymix_print_enum(struct mixer_ctl *ctl, const char *space,
                               int print_all);
static int media_volume_cache = MEDIA_VOL_INIT;
static int phone_volume_cache = PHONE_VOL_INIT;
static int media_volume_offset_cache = 0;
static int phone_volume_offset_cache = 0;
static int media_variable_max_volume_cache = MEDIA_VOL_MAX;
static int bmt_value_cache = BMT_VALUE_MAX;
static bool s_media_muted = false;

int dspdev_set_audio_balance(dsp_balance_source_t source);

int dspdev_set_channel(dsp_channel_type_t channel_type)
{
    static int channel_cache = -1;
    if(channel_cache == channel_type){
        ALOGI("channel_type the same with last setted, return;");
        return 0;
    }
    int ret = 0;
    switch (channel_type) {
	case CHANNEL_MEDIA_MUSIC:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_Music);
		dspdev_set_audio_balance(AUDIO_BALANCE_MUSIC);
		break;
	case CHANNEL_MEDIA_TUNER:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_Tuner);
		dspdev_set_audio_balance(AUDIO_BALANCE_FM);
		break;
	case CHANNEL_BT_PHONE:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_BT);
		break;
	case CHANNEL_CARPLAY_PHONE:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_CP_Phone);
		break;
	case CHANNEL_TBOX:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_Tbox);
		break;
	case CHANNEL_SIRI:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_Siri);
		break;
	case CHANNEL_VR:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_VR);
		break;
	case CHANNEL_NULL:
		ret = dsp_control(tinymix_control_Source_Select, tinymix_control_Source_Select_NULL);
		break;
	default:
		ALOGE("set_channel: set an error channel type :%d", channel_type);
		return EINVAL;
	}

    if(ret < 0){
	    ALOGE("set_channel failed! ret = %d", ret);
	    return -1;
    }
    channel_cache = channel_type;
    return 0;
}

int dspdev_get_channel(dsp_channel_type_t* channel)
{
    ALOGI("%s,channel pointer addr: %p;", __func__,channel);
    int ret = 0;
    int native_channel = 0;
    ret = dsp_get_value(tinymix_control_Source_Select, &native_channel);
    switch (native_channel) {
	case 0:
		*channel = CHANNEL_MEDIA_MUSIC;
		break;
	case 1:
		*channel = CHANNEL_MEDIA_TUNER;
		break;
	case 2:
		*channel = CHANNEL_BT_PHONE;
		break;
	case 3:
		*channel = CHANNEL_CARPLAY_PHONE;
		break;
	case 4:
		*channel = CHANNEL_TBOX;
		break;
	case 5:
		*channel = CHANNEL_SIRI;
		break;
	case 6:
		*channel = CHANNEL_VR;
		break;
	default:
		*channel = CHANNEL_MEDIA_MUSIC;
		return EINVAL;
	}
    ALOGD("get_channel --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_tuner_mixer(bool on)
{
    ALOGI("%s,to turn mixer %d;", __func__,on);
    // char s_vol_value[4];
    // sprintf(s_vol_value,"%d",value);
   // int ret = 0;
	// ret = dsp_control(tinymix_control_MediaVol, s_vol_value);

    //if(ret < 0){
     //   return -1;
    //}

    return 0;
}

int dspdev_set_volume(dsp_channel_type_t channel_type, int value)
{
    if((channel_type == CHANNEL_MEDIA_MUSIC) || (channel_type == CHANNEL_MEDIA_TUNER)) {
	value += media_volume_offset_cache;
	if(value > media_variable_max_volume_cache){
	    ALOGI("to be set volume bigger than media_variable_max_volume_cache..");
	    value = media_variable_max_volume_cache;
	}
    } else if((channel_type == CHANNEL_BT_PHONE)) {
	if((value + phone_volume_offset_cache) > PHONE_VOL_MAX){
	    value = PHONE_VOL_MAX;
	}else if((value + phone_volume_offset_cache) <= 0){
	    if(value > 0) {
		value = 1;
	    }
	}else{
	    value += phone_volume_offset_cache;
	}
    }

    char s_vol_value[4];
    sprintf(s_vol_value, "%d", value);
    int ret = 0;
    switch (channel_type) {
	case CHANNEL_MEDIA_MUSIC:
	case CHANNEL_MEDIA_TUNER:
		media_volume_cache = value - media_volume_offset_cache;
		ret = dsp_control(tinymix_control_MediaVol, s_vol_value);
		break;
	case CHANNEL_BT_PHONE:
		phone_volume_cache = value;
		ret = dsp_control(tinymix_control_PhoneVol, s_vol_value);
		break;
	case CHANNEL_TBOX:
		ret = dsp_control(tinymix_control_TboxVol, s_vol_value);
		break;
	case CHANNEL_NAVI:
		ret = dsp_control(tinymix_control_NaviVol, s_vol_value);
		break;
	case CHANNEL_BEEP:
		ret = dsp_control(tinymix_control_BeepVol, s_vol_value);
		break;
	default:
		return -EINVAL;
	}

    if(ret < 0){
	    ALOGE("set_volume failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_get_volume(dsp_channel_type_t channel_type, int *value)
{
    ALOGI("%s, dsp_channel_type_t %d, volume value pointer addr: %p;", __func__, channel_type, value);
    int ret = 0;
    switch (channel_type) {
	case CHANNEL_MEDIA_MUSIC:
	case CHANNEL_MEDIA_TUNER:
	    ret = dsp_get_value(tinymix_control_MediaVol, value);
	    *value -= media_volume_offset_cache;
	    break;
	case CHANNEL_BT_PHONE:
	    ret = dsp_get_value(tinymix_control_PhoneVol, value);
	    *value -= phone_volume_offset_cache;
	    break;
	case CHANNEL_TBOX:
	    ret = dsp_get_value(tinymix_control_TboxVol, value);
	    break;
	default:
	    ret = dsp_get_value(tinymix_control_MediaVol, value);
	    *value -= media_volume_offset_cache;
	    return EINVAL;
    }

    if(ret < 0){
	ALOGE("get_volume failed! --- %d", ret);
	return -1;
    }
    return 0;
}

int dspdev_set_volume_offset(dsp_channel_type_t channel_type,int value)
{
    char s_vol_after_offset_value[4];
    int ret = 0;
    int current_vol = MEDIA_VOL_INIT;
    switch (channel_type) {
	case CHANNEL_MEDIA_MUSIC:
	case CHANNEL_MEDIA_TUNER:
	    media_volume_offset_cache = value;
	    current_vol = media_volume_cache;
	    if((current_vol+value)>MEDIA_VOL_MAX){
		current_vol = MEDIA_VOL_MAX;
	    }else if((current_vol+value)<0){
		current_vol = 0;
	    }else{
		current_vol = current_vol+value;
	    }
	    if(current_vol>media_variable_max_volume_cache){
		ALOGI("to be set volume with offset bigger than media_variable_max_volume_cache..");
		current_vol = media_variable_max_volume_cache;
	    }
	    sprintf(s_vol_after_offset_value,"%d",current_vol);
	    ret = dsp_control(tinymix_control_MediaVol, s_vol_after_offset_value);
	    break;
	case CHANNEL_VR:
	    current_vol = media_volume_cache;
	    if((current_vol+value)>NAVI_VOL_MAX){
		current_vol = NAVI_VOL_MAX;
	    }else if((current_vol+value)<0){
		current_vol = 0;
	    }else{
		current_vol = current_vol+value;
	    }
	    if(current_vol>media_variable_max_volume_cache){
		ALOGI("to be set navi volume with offset bigger than media_variable_max_volume_cache..");
		current_vol = media_variable_max_volume_cache;
	    }
	    sprintf(s_vol_after_offset_value,"%d",current_vol);
	    ret = dsp_control(tinymix_control_NaviVol, s_vol_after_offset_value);
	    break;
	case CHANNEL_BT_PHONE:
	    phone_volume_offset_cache = value;
	    current_vol = phone_volume_cache;
	    if((current_vol+value)>PHONE_VOL_MAX){
		current_vol = PHONE_VOL_MAX;
	    }else if((current_vol+value) <= 0){
		if(current_vol > 0) {
		    current_vol = 1;
		}
	    }else{
		current_vol = current_vol + value;
	    }
	    sprintf(s_vol_after_offset_value,"%d",current_vol);
	    ret = dsp_control(tinymix_control_PhoneVol, s_vol_after_offset_value);
	    break;
	default:
	    return EINVAL;
    }

    if(ret < 0){
	ALOGE("set_volume_offset failed! --- %d", ret);
	return -1;
    }

    return 0;
}

int dspdev_get_volume_offset(dsp_channel_type_t type, int *value)
{
    ALOGI("%s, dsp_channel_type_t %d, volume offset value pointer addr: %p;", __func__, type, value);
    *value = media_volume_offset_cache;

    return 0;
}

static int dspdev_set_variable_max_volume(dsp_channel_type_t channel_type,int value)
{
    ALOGI("%s, dsp_channel_type_t %d, to set_variable_max_volume value %d;", __func__, channel_type, value);
    media_variable_max_volume_cache = value;
    return 0;
}

int dspdev_get_variable_max_volume(dsp_channel_type_t type, int *value)
{
    ALOGI("%s, dsp_channel_type_t %d, variable max volume value pointer addr: %p;", __func__, type, value);
    *value = media_variable_max_volume_cache;
    return 0;
}

int dspdev_get_max_volume(dsp_channel_type_t type, int *value)
{
    ALOGI("%s, dsp_channel_type_t %d, volume max value pointer addr: %p;", __func__, type, value);
    *value = MEDIA_VOL_MAX;
    return 0;
}

int dspdev_set_music_mute(bool mute)
{
    char mute_status[4];
    int ret = 0;
    if(s_media_muted)
	return 0;
    sprintf(mute_status,"%d", mute);
    ret = dsp_control(tinymix_control_MediaMute, mute_status);

    if(ret < 0){
	    ALOGE("set_music_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_get_music_mute(bool *mute)
{
    ALOGI("%s, music mute pointer addr: %p", __func__, mute);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_MediaMute, &i_mute);
    *mute = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("get_music_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_set_media_mute(bool mute)
{
    char mute_status[4];
    int ret;
    s_media_muted = mute;
    sprintf(mute_status,"%d", mute);
    ret = dsp_control(tinymix_control_MediaMute, mute_status);

    if(ret < 0){
	    ALOGE("set_music_mute failed! --- %d", ret);
	    return -1;
    }
    return 0;
}

int dspdev_set_navi_mute(bool mute)
{
    char s_navi_mute_status[4];
    sprintf(s_navi_mute_status,"%d",mute);
    int ret = 0;
	ret = dsp_control(tinymix_control_NaviMute, s_navi_mute_status);
    ALOGD("set_navi_mute --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_navi_mute(bool *mute)
{
    ALOGI("%s, navi_mute pointer addr: %p;", __func__,mute);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_NaviMute, &i_mute);
    *mute = (i_mute)?true:false;
    ALOGD("get_navi_mute --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_tbox_mute(bool mute)
{
    char s_tbox_mute_status[4];
    sprintf(s_tbox_mute_status,"%d",mute);
    int ret = 0;
	ret = dsp_control(tinymix_control_TboxMute, s_tbox_mute_status);

    if(ret < 0){
	    ALOGD("set_tbox_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_get_tbox_mute(bool *mute)
{
    ALOGI("%s, tbox_mute pointer addr: %p;", __func__,mute);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_TboxMute, &i_mute);
    *mute = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("get_tbox_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_set_beep_mute(bool mute)
{
    char s_beep_mute_status[4];
    sprintf(s_beep_mute_status,"%d",mute);
    int ret = 0;
	ret = dsp_control(tinymix_control_BeepMute, s_beep_mute_status);

    if(ret < 0){
	    ALOGD("set_beep_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_get_beep_mute(bool *mute)
{
    ALOGI("%s, beep_mute pointer addr: %p;", __func__,mute);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_BeepMute, &i_mute);
    *mute = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("get_beep_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_set_phone_mute(bool mute)
{
    char s_phone_mute_status[4];
    sprintf(s_phone_mute_status,"%d",mute);
    int ret = 0;
	ret = dsp_control(tinymix_control_PhoneMute, s_phone_mute_status);

    if(ret < 0){
	    ALOGD("set_phone_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_get_phone_mute(bool *mute)
{
    ALOGI("%s, phone_mute pointer addr: %p;", __func__,mute);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_PhoneMute, &i_mute);
    *mute = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("get_phone_mute failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_set_tuner_mute(bool mute)
{
	char s_tuner_mute_status[4];
	int ret = 0;
	sprintf(s_tuner_mute_status, "%d", mute);
	ret = dsp_control(tinymix_control_TunerMute, s_tuner_mute_status);	
	if(ret < 0) {
		ALOGE("set tuner mute failed! --- %d", ret);
		return -1;
	}
	return 0;
}


int dspdev_get_tuner_mute(bool *mute)
{
	ALOGI("%s, get tuner mute status!", __func__);
	int ret = 0;
	int i_mute = 0;
	ret = dsp_get_value(tinymix_control_TunerMute, &i_mute);
	*mute = (i_mute) ? true : false;
	if(ret < 0) {
		ALOGE("get tuner mute failed! --- %d", ret);
		return -1;
	}
	return 0;
}

int dspdev_set_mic_mute(bool mute)
{
	char s_mic_mute_status[4];
	int ret = 0;
	sprintf(s_mic_mute_status, "%d", mute);
	ret = dsp_control(tinymix_control_MicMute, s_mic_mute_status);	
	if(ret < 0) {
		ALOGE("set mic mute failed! --- %d", ret);
		return -1;
	}
	return 0;
}


int dspdev_get_mic_mute(bool *mute)
{
	ALOGI("%s, get mic mute status!", __func__);
	int ret = 0;
	int i_mute = 0;
	ret = dsp_get_value(tinymix_control_MicMute, &i_mute);
	*mute = (i_mute) ? true : false;
	if(ret < 0) {
		ALOGE("get mic mute failed! --- %d", ret);
		return -1;
	}
	return 0;
}

int dspdev_set_amplifier_mute(bool mute)
{
    char s_amplifier_mute_status[4];
    sprintf(s_amplifier_mute_status,"%d",mute);
    int ret = 0;
    ret = dsp_control(tinymix_control_PA_Mute, s_amplifier_mute_status);

    if(ret < 0){
	    ALOGE("set_amplifier_mute failed! --- %d", ret);
	    return -1;
    }
    return 0;
}

int dspdev_set_mic_volume(int value)
{
    char s_vol_mic_value[4];
    sprintf(s_vol_mic_value,"%d",value);
    int ret = 0;
	ret = dsp_control(tinymix_control_MicVol, s_vol_mic_value);
    ALOGD("set_mic_volume --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_mic_volume(int *value)
{
    ALOGI("%s, get_mic_volume value pointer addr: %p;", __func__, value);
    int ret = 0;
    *value = 2;
    ret = dsp_get_value(tinymix_control_MicVol, value);
    ALOGD("get_mic_volume --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_mcu_app_mic_test(bool on_off)
{
	ALOGI("%s : for mcu app mic test. test on/off : %d \n", __func__, on_off);
    char s_mcu_mic_value[4];
    sprintf(s_mcu_mic_value,"%d", on_off);
	int ret = 0;
	ret = dsp_control(tinymix_control_DiagMicTest, s_mcu_mic_value);
	if(ret < 0)
        return -1;

    return 0;
}

int dspdev_get_mcu_app_mic_test(bool *on_off)
{
    int ret = 0;
    int temp_get_mic_test = 0;
	ret = dsp_get_value(tinymix_control_DiagMicTest, &temp_get_mic_test);
	*on_off = (temp_get_mic_test)?true:false;

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_loudness(bool enable)
{
    char s_loudness_enable[4];
    sprintf(s_loudness_enable,"%d",enable);
    int ret = 0;
    ret = dsp_control(tinymix_control_Loudness, s_loudness_enable);
    if(ret < 0){
	    ALOGE("set_loudness failed! --- %d", ret);
	    return -1;
    }
    return 0;
}

int dspdev_get_loudness(bool *enable)
{
    ALOGI("%s, loudness status pointer addr: %p;", __func__, enable);
    int ret = 0;
    int i_enable = 0;
    ret = dsp_get_value(tinymix_control_Loudness, &i_enable);
    *enable = (i_enable)?true:false;

    if(ret < 0){
	    ALOGD("get_loudness failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_set_sound_effect(dsp_sound_effect_type_t sound_type)
{
    if(sound_type > SOUND_EFFECT_REST_MODE) {
	ALOGE("%s, wrong sound_type:%d, abandon!", __func__, sound_type);
	return -EINVAL;
    }
    char s_sound_effect[4];
    sprintf(s_sound_effect,"%d",sound_type);
    int ret = 0;
    ret = dsp_control(tinymix_control_Sound_Effect, s_sound_effect);

    if(ret < 0){
	ALOGE("%s, set sound effect failed! --- %d", __func__, ret);
        return -1;
    }
    return 0;
}

int dspdev_get_sound_effect(dsp_sound_effect_type_t *sound_type)
{
    ALOGI("%s, sound_type pointer addr:%p;", __func__, sound_type);
    int ret = 0;
    ALOGD("get_sound_effect --- %d", ret);
    *sound_type = SOUND_EFFECT_NATURAL_ACOUSTIC;
    ret = dsp_get_value(tinymix_control_Sound_Effect, (int *)sound_type);
    if(ret < 0){
	ALOGE("%s, get sound effect failed! --- %d", __func__, ret);
        return -1;
    }
    return 0;
}


int dspdev_set_bass_mid_treble(dsp_bmt_type_t bmt_type, int value)
{
    if(value > bmt_value_cache)
	value = bmt_value_cache;
    char s_bmt_value[4];
    sprintf(s_bmt_value,"%d",value);
    int ret = 0;
    switch (bmt_type) {
	case BMT_BASS:
		ret = dsp_control(tinymix_control_Bass, s_bmt_value);
		break;
	case BMT_MIDDLE:
		ret = dsp_control(tinymix_control_Middle, s_bmt_value);
		break;
	case BMT_TREBLE:
		ret = dsp_control(tinymix_control_Treble, s_bmt_value);
		break;
	default:
		return EINVAL;
	}
    ALOGD("set_bass_mid_treble --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_bass_mid_treble(dsp_bmt_type_t bmt_type, int *value)
{
    ALOGI("%s, bmt_type:%d,bmt value pointer addr: %p;", __func__,bmt_type,value);
    int ret = 0;
    switch (bmt_type) {
	case BMT_BASS:
		ret = dsp_get_value(tinymix_control_Bass, value);
		break;
	case BMT_MIDDLE:
		ret = dsp_get_value(tinymix_control_Middle, value);
		break;
	case BMT_TREBLE:
		ret = dsp_get_value(tinymix_control_Treble, value);
		break;
	default:
		return -EINVAL;
	}
    ALOGD("get_bass_mid_treble --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

/*
 * Chery project: set Bass/MID/TREBLE
 **/
int dspdev_set_geq(dsp_geq_type_t geq_type, int value)
{
    char s_geq_value[4];
    if(value > GEQ_VALUE_MAX)
	value = GEQ_VALUE_MAX;
    sprintf(s_geq_value, "%d", value);
    int ret = 0;
    switch(geq_type) {
	case GEQ_BASS:
	    ret = dsp_control(tinymix_control_Bass_EQ, s_geq_value);
	    break;
	case GEQ_MIDDLE_BASS:
	    ret = dsp_control(tinymix_control_Middle_Bass_EQ, s_geq_value);
	    break;
	case GEQ_MIDDLE:
	    ret = dsp_control(tinymix_control_Middle_EQ, s_geq_value);
	    break;
	case GEQ_MIDDLE_HIGH:
	    ret = dsp_control(tinymix_control_Middle_High_EQ, s_geq_value);
	    break;
	case GEQ_HIGH: 
	    ret = dsp_control(tinymix_control_High_EQ, s_geq_value);
	    break;
	default:
	    ALOGE("%s, Set (BMT) geq_type failed, do not have type %d", __func__, geq_type);
	    return -1;
    }
    if(ret < 0) {
        ALOGE("%s, dsp_control failed. %d", __func__, ret);
	return -1;
    }
    return 0;
}

/*
 * Chery project: get Bass/MID/TREBLE
 **/
int dspdev_get_geq(dsp_geq_type_t geq_type, int *value)
{
    ALOGI("%s, (BMT)geq_type: %d", __func__, geq_type);
    int ret = 0;
    switch(geq_type) {
	case GEQ_BASS:
	    ret = dsp_get_value(tinymix_control_Bass_EQ, value);
	    break;
	case GEQ_MIDDLE_BASS:
	    ret = dsp_get_value(tinymix_control_Middle_Bass_EQ, value);
	    break;
	case GEQ_MIDDLE:
	    ret = dsp_get_value(tinymix_control_Middle_EQ, value);
	    break;
	case GEQ_MIDDLE_HIGH:
	    ret = dsp_get_value(tinymix_control_Middle_High_EQ, value);
	    break;
	case GEQ_HIGH: 
	    ret = dsp_get_value(tinymix_control_High_EQ, value);
	    break;
	default:
	    ALOGE("%s, Get (BMT) geq_type failed, do not have type %d", __func__, geq_type);
	    return -1;
    }
    if(ret < 0) {
        ALOGE("%s, dsp_control failed. %d", __func__, ret);
	return -1;
    }
    return 0;
}


int dspdev_set_sound_field(int balance, int fader)
{
    char s_sound_balance_value[4];
    char s_sound_fader_value[4];
    sprintf(s_sound_balance_value,"%d", balance);
    sprintf(s_sound_fader_value,"%d", fader);
    int ret = 0;

    ret = dsp_control(tinymix_control_Balance, s_sound_balance_value);
    ret = dsp_control(tinymix_control_Fader, s_sound_fader_value);

    if(ret < 0){
	    ALOGE("set_sound_field failed! --- %d", ret);
	    return -1;
    }

    return 0;
}

int dspdev_get_sound_field(int *balance, int *fader)
{
    ALOGI("%s, sound value pointer addr: balance %p fader %p;", __func__, balance, fader);
    int ret = 0;

    ret = dsp_get_value(tinymix_control_Balance, balance);
    ret = dsp_get_value(tinymix_control_Fader, fader);

    if(ret < 0){
	    ALOGE("get_sound_field failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_set_dtcs_mode(dsp_dtcs_mode_t mode)
{
    char s_mode_value[4];
    sprintf(s_mode_value,"%d",mode);
    int ret = 0;
    ret = dsp_control(tinymix_control_DtcsModeSw, s_mode_value);
    ALOGD("set_dtcs_mode --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_dtcs_mode(dsp_dtcs_mode_t* mode)
{
    ALOGI("%s,mode pointer addr: %p;", __func__, mode);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_DtcsModeSw, (int*)mode);
    ALOGD("get_dtcs_mode --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_dtcs_status(bool status)
{
    char s_dtcs_status[4];
    sprintf(s_dtcs_status,"%d",status);
    int ret = 0;
    ret = dsp_control(tinymix_control_DtcsSw, s_dtcs_status);
    ALOGD("set_dtcs_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_dtcs_status(bool* status)
{
    ALOGI("%s, dtcs_status pointer addr: %p;", __func__,status);
    int ret = 0;
    int i_status = 0;
    ret = dsp_get_value(tinymix_control_DtcsSw, &i_status);
    *status = (i_status)?true:false;
    ALOGD("get_dtcs_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_ale_status(bool status)
{
    char s_ale_status[4];
    sprintf(s_ale_status,"%d",status);
    int ret = 0;
    ret = dsp_control(tinymix_control_AleSw, s_ale_status);
    ALOGD("set_ale_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_ale_status(bool* status)
{
    ALOGI("%s, ale_status pointer addr: %p;", __func__,status);
    int ret = 0;
    int i_status = 0;
    ret = dsp_get_value(tinymix_control_AleSw, &i_status);
    *status = (i_status)?true:false;
    ALOGD("get_ale_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_bex_status(dsp_bex_switch_t status)
{
    char s_status_value[4];
    sprintf(s_status_value,"%d",status);
    int ret = 0;
    ret = dsp_control(tinymix_control_BexSw, s_status_value);
    ALOGD("set_bex_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_bex_status(dsp_bex_switch_t* status)
{
    ALOGI("%s,bex_status pointer addr: %p;", __func__, status);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_BexSw, (int*)status);
    ALOGD("get_bex_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_tts_status(dsp_tts_switch_t status)
{
    char s_status_value[4];
    sprintf(s_status_value,"%d",status);
    int ret = 0;
    ret = dsp_control(tinymix_control_TtsSw, s_status_value);
    ALOGD("set_tts_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_tts_status(dsp_tts_switch_t* status)
{
    ALOGI("%s,tts_status pointer addr: %p;", __func__, status);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_TtsSw, (int*)status);
    ALOGD("get_tts_status --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_vcworkmode(dsp_vcworkmode_type_t workmode)
{
    char s_mode_value[4];
    sprintf(s_mode_value,"%d",workmode);
    int ret = 0;
    ret = dsp_control(tinymix_control_VCWorkMode, s_mode_value);
    ALOGD("set_vcworkmode --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_vcworkmode(dsp_vcworkmode_type_t* workmode)
{
    ALOGI("%s,vcworkmode pointer addr: %p;", __func__, workmode);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_VCWorkMode, (int*)workmode);
    ALOGD("get_vcworkmode --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_vcfunc(dsp_vcfunc_type_t func)
{
    char s_vcfunc_value[4];
    sprintf(s_vcfunc_value,"%d",func);
    int ret = 0;
    ret = dsp_control(tinymix_control_VCFunc, s_vcfunc_value);
    ALOGD("set_vcfunc --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_vcfunc(dsp_vcfunc_type_t* func)
{
    ALOGI("%s,dsp_vcfunc pointer addr: %p;", __func__, func);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_VCFunc, (int*)func);
    ALOGD("get_vcfunc --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_vcvolume(int value)
{
    char s_vcvolume_value[4];
    sprintf(s_vcvolume_value,"%d",value);
    int ret = 0;
    ret = dsp_control(tinymix_control_VCVolume, s_vcvolume_value);
    ALOGD("set_vcvolume --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_vcvolume(int *value)
{
    ALOGI("%s, get_vcvolume value pointer addr: %p;", __func__, value);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_VCVolume, value);
    ALOGD("get_vcvolume --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_pa_pwm_frequency(dsp_pa_pwm_frequency_t frequency)
{
    char s_pa_pwm_frequency_value[4];
    sprintf(s_pa_pwm_frequency_value,"%d",frequency);
    int ret = 0;
    ret = dsp_control(tinymix_control_PA_pwm_frequency, s_pa_pwm_frequency_value);
    ALOGD("set_pa_pwm_frequency --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_pa_pwm_frequency(dsp_pa_pwm_frequency_t* frequency)
{
    ALOGI("%s,pa_pwm_frequency pointer addr: %p;", __func__, frequency);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_PA_pwm_frequency, (int*)frequency);
    ALOGD("get_pa_pwm_frequency --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_pa_spread_spectrum(dsp_pa_spread_spectrum_t spread_spectrum)
{
    char s_pa_spread_spectrum_value[4];
    sprintf(s_pa_spread_spectrum_value,"%d",spread_spectrum);
    int ret = 0;
    ret = dsp_control(tinymix_control_PA_spread_spectrum, s_pa_spread_spectrum_value);
    ALOGD("set_pa_spread_spectrum --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_pa_spread_spectrum(dsp_pa_spread_spectrum_t* spread_spectrum)
{
    ALOGI("%s,spread_spectrum pointer addr: %p;", __func__, spread_spectrum);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_PA_spread_spectrum, (int*)spread_spectrum);
    ALOGD("get_pa_spread_spectrum --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_pa_phase_staggering_ch(int index, dsp_pa_phase_staggering_ch_t phase_staggering)
{
    char s_pa_phase_staggering_ch_value[4];
    sprintf(s_pa_phase_staggering_ch_value,"%d",phase_staggering);
    int ret = 0;
    switch (index) {
	case 0:
		ret = dsp_control(tinymix_control_PA_phase_staggering_ch1, s_pa_phase_staggering_ch_value);
		break;
	case 1:
		ret = dsp_control(tinymix_control_PA_phase_staggering_ch2, s_pa_phase_staggering_ch_value);
		break;
	case 2:
		ret = dsp_control(tinymix_control_PA_phase_staggering_ch3, s_pa_phase_staggering_ch_value);
		break;
	case 3:
		ret = dsp_control(tinymix_control_PA_phase_staggering_ch4, s_pa_phase_staggering_ch_value);
		break;
	default:
		return EINVAL;
	}
    ALOGD("set_pa_phase_staggering_ch --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_pa_phase_staggering_ch(int index, dsp_pa_phase_staggering_ch_t* phase_staggering)
{
    ALOGI("%s, pa_phase_staggering_ch index:%d, value pointer addr: %p;", __func__,index,phase_staggering);
    int ret = 0;
    switch (index) {
	case 0:
		ret = dsp_get_value(tinymix_control_PA_phase_staggering_ch1, (int*)phase_staggering);
		break;
	case 1:
		ret = dsp_get_value(tinymix_control_PA_phase_staggering_ch2, (int*)phase_staggering);
		break;
	case 2:
		ret = dsp_get_value(tinymix_control_PA_phase_staggering_ch3, (int*)phase_staggering);
		break;
	case 3:
		ret = dsp_get_value(tinymix_control_PA_phase_staggering_ch4, (int*)phase_staggering);
		break;
	default:
		return EINVAL;
	}
    ALOGD("get_pa_phase_staggering_ch --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_pa_am_frequency(dsp_pa_am_frequency_t frequency)
{
    char s_pa_am_frequency_value[4];
    sprintf(s_pa_am_frequency_value,"%d",frequency);
    int ret = 0;
    ret = dsp_control(tinymix_control_PA_AM_frequency, s_pa_am_frequency_value);
    ALOGD("set_pa_am_frequency --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_get_pa_am_frequency(dsp_pa_am_frequency_t* frequency)
{
    ALOGI("%s,frequency pointer addr: %p;", __func__, frequency);
    int ret = 0;
    ret = dsp_get_value(tinymix_control_PA_AM_frequency, (int*)frequency);
    ALOGD("get_pa_am_frequency --- %d", ret);

    if(ret < 0){
        return -1;
    }

    return 0;
}

int dspdev_set_sound_stage(dsp_sound_stage_t stage)
{
    if(stage > SOUND_STAGE_ALL_PASSENGER_MODE) {
	ALOGE("%s, wrong stage: %d, abandon!", __func__, stage);
	return -EINVAL;
    }
    char s_sound_stage_value[4];
    sprintf(s_sound_stage_value, "%d", stage);
    int ret = 0;
    ret = dsp_control(tinymix_control_Sound_Stage, s_sound_stage_value); 
    if(ret < 0) {
	ALOGE("%s, set sound stage failed! --- %d", __func__, ret);
	return -1;
    }

    return 0;
}

int dspdev_get_sound_stage(dsp_sound_stage_t *stage)
{
    ALOGI("%s, sound stage pointer addr: %p", __func__, stage);
    int ret = 0;
    * stage = SOUND_STAGE_DRIVER_MODE;
    ret = dsp_get_value(tinymix_control_Sound_Stage, (int *) stage);
    if(ret < 0) {
	ALOGE("%s, get sound stage failed! --- %d", __func__, ret);
	return -1;
    }
    return 0;
}

int dspdev_set_audio_balance(dsp_balance_source_t source)
{
    if(source > AUDIO_BALANCE_AM) {
    	ALOGE("%s, wrong source type: %d, abandon!", __func__, source);
	return -EINVAL;
    }

    char s_audio_balance_source[4];
    sprintf(s_audio_balance_source, "%d", source);
    int ret = 0;
    ret = dsp_control(tinymix_contorl_BalanceSource, s_audio_balance_source); 
    if(ret < 0) {
	ALOGE("%s, set audio balance failed! --- %d", __func__, ret);
	return -1;
    }

    return 0;
}

int dspdev_set_sound_channel_mute(sound_channel_type_t sound_channel, bool mute)
{
    static uint8_t s_mute_status = 0x0;
    if((sound_channel < SOUND_CHANNEL_RR) || (sound_channel > SOUND_CHANNEL_FL)) {
    	ALOGE("%s, wrong sound channel type: %d, abandon!", __func__, sound_channel);
	return -EINVAL;
    }

    if(mute)
	s_mute_status |= ((1 << sound_channel) & 0xf); 
    else
	s_mute_status &= ((1 << sound_channel) ^ 0xf); 
    char s_sound_channel_mute[4];
    sprintf(s_sound_channel_mute, "%d", s_mute_status);
    int ret = 0;
    ret = dsp_control(tinymix_contorl_SoundChannel_Mute, s_sound_channel_mute); 
    if(ret < 0) {
	ALOGE("%s, set sound channel mute failed! --- %d", __func__, ret);
	return -1;
    }

    return 0;
}

int dspdev_set_ecnr_type(dsp_ecnr_type_t type)
{
    if((type < ECNR_TYPE_VR) || (type > ECNR_TYPE_CP_PHONE)) {
	ALOGE("%s: wrong ecnr type %d, abandon!", __func__, type);
    }
    char s_ecnr_type[4];
    int ret;
    sprintf(s_ecnr_type, "%d", type);
    ret = dsp_control(tinymix_control_EcnrType, s_ecnr_type);
    if(ret < 0) {
	ALOGE("%s, set ecnr type failed! --- %d", __func__, ret);
	return -1;
    }
    return 0;
}

int dspdev_get_ecnr_type(dsp_ecnr_type_t *type)
{
    int ret = 0;
    *type = ECNR_TYPE_VR;
    ret = dsp_get_value(tinymix_control_EcnrType, (int *) type);
    if(ret < 0) {
	ALOGE("%s, get ecnr type failed! --- %d", __func__, ret);
	return -1;
    }
    return 0;
}

int dspdev_set_mcu_app_speaker_test(dsp_pdc_channel_t channel, bool on_off)
{
    ALOGI("%s : for mcu app speaker test. test channel : %d ; test on/off : %d \n", __func__, channel, on_off);
    char s_channel_value[4];
    char s_on_off_value[4];
    sprintf(s_channel_value,"%d", channel);
    int ret = 0;
    ret = dsp_control(tinymix_control_DiagPDCVol, "42");        // -24dB
    ret = dsp_control(tinymix_control_DiagPDCChannel, s_channel_value);
    if(on_off) {
        sprintf(s_on_off_value,"%d", PDC_TYPE_SLOW);    // slow test  
    } else {
        sprintf(s_on_off_value,"%d", PDC_TYPE_STOP);     // stop
    }
    ret = dsp_control(tinymix_control_DiagPDCType, s_on_off_value);
    if(ret < 0) {
        return -1;
    }

    return 0;
}

int dspdev_get_mcu_app_speaker_test(dsp_pdc_channel_t channel, bool *on_off)
{
    int ret = 0;
    int temp_get_speaker_test = 0;
    ret = dsp_get_value(tinymix_control_DiagPDCType, &temp_get_speaker_test);
    *on_off = (temp_get_speaker_test) ? true : false;

    if(ret < 0) {
        return -1;
    }

    return 0;
}

int dspdev_get_dsp_firmware_version(int *version)
{
    int ret = 0;
    ret = dsp_get_value(tinymix_control_dsp_firmware_version, version);

    ALOGI(" %s : dsp firmware version 0x%x", __func__, *version);

    if(ret < 0) {
        return -1;
    }

    return 0;
}

int dspdev_open_external_amp_test(bool on)
{
   char s_test_status[4];
   sprintf(s_test_status, "%d", on);
   int ret = 0;
   ret = dsp_control(tinymix_control_ExternalPaTest, s_test_status);
   if(ret < 0) {
       ALOGE("%s, open external amp test failed! --- %d", __func__, ret);
       return -1;
   }
   return 0;
}

int dspdev_set_mcu_reset_dsp(bool on)
{
   char s_on_status[4];
   sprintf(s_on_status, "%d", on);
   int ret = 0;
   ret = dsp_control(tinymix_control_MCU_Reset_DSP, s_on_status);
   if(ret < 0) {
       ALOGE("%s, mcu reset dsp failed! --- %d", __func__, ret);
       return -1;
   }
   return 0;
}

static int dspdev_set_diag_sound_effect_off(bool on)
{
   char s_on_status[4];
   sprintf(s_on_status, "%d", on);
   int ret = 0;
   ret = dsp_control(tinymix_control_DiagSoundEffectOff, s_on_status);
   if(ret < 0) {
       ALOGE("%s, diag set sound effect off! --- %d", __func__, ret);
       return -1;
   }
   return 0;
}


int dspdev_set_secondary_mono_all(bool all)
{
    char speaker_status[4];
    int ret = 0;

    sprintf(speaker_status,"%d", all);
    ret = dsp_control(tinymix_control_secondary_mono_all, speaker_status);

    if(ret < 0){
	    ALOGE("dspdev_set_secondary_mono_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_set_secondary_steoro_all(bool all)
{
    char speaker_status[4];
    int ret = 0;

    sprintf(speaker_status,"%d", all);
    ret = dsp_control(tinymix_control_secondary_steoro_all, speaker_status);

    if(ret < 0){
	    ALOGE("dspdev_set_secondary_steoro_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_set_navi_mono_all(bool all)
{
    char speaker_status[4];
    int ret = 0;

    sprintf(speaker_status,"%d", all);
    ret = dsp_control(tinymix_control_navi_mono_all, speaker_status);

    if(ret < 0){
	    ALOGE("dspdev_set_navi_mono_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_set_phone_mono_all(bool all)
{
    char speaker_status[4];
    int ret = 0;

    sprintf(speaker_status,"%d", all);
    ret = dsp_control(tinymix_control_phone_mono_all, speaker_status);

    if(ret < 0){
	    ALOGE("dspdev_set_phone_mono_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_get_secondary_mono_all(bool *all)
{
    ALOGI("%s, dspdev_get_secondary_mono_all addr: %p", __func__, all);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_secondary_mono_all, &i_mute);
    *all = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("dspdev_get_secondary_mono_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_get_secondary_steoro_all(bool *all)
{
    ALOGI("%s, dspdev_get_secondary_steoro_all addr: %p", __func__, all);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_secondary_steoro_all, &i_mute);
    *all = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("dspdev_get_secondary_steoro_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_get_navi_mono_all(bool *all)
{
    ALOGI("%s, dspdev_get_navi_mono_all addr: %p", __func__, all);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_navi_mono_all, &i_mute);
    *all = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("dspdev_get_navi_mono_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


int dspdev_get_phone_mono_all(bool *all)
{
    ALOGI("%s, dspdev_get_phone_mono_all addr: %p", __func__, all);
    int ret = 0;
    int i_mute = 0;
    ret = dsp_get_value(tinymix_control_phone_mono_all, &i_mute);
    *all = (i_mute)?true:false;

    if(ret < 0){
	    ALOGE("dspdev_get_phone_mono_all failed! --- %d", ret);
	    return -1;
    }

    return 0;
}


static int isnumber(const char *str)
{
    char *end;

    if (str == NULL || strlen(str) == 0)
        return 0;

    strtol(str, &end, 0);
    return strlen(end) == 0;
}


static void tinymix_print_enum(struct mixer_ctl *ctl, const char *space,
                               int print_all)
{
    unsigned int num_enums;
    unsigned int i;
    const char *string;
    int control_value = mixer_ctl_get_value(ctl, 0);
    ALOGI("%s,control_value: %d;", __func__, control_value);

    if (print_all) {
        num_enums = mixer_ctl_get_num_enums(ctl);
        for (i = 0; i < num_enums; i++) {
            string = mixer_ctl_get_enum_string(ctl, i);
            printf("%s%s%s",
                   control_value == (int)i ? ">" : "", string,
                   (i < num_enums - 1) ? space : "");
        }
    }
    else {
        string = mixer_ctl_get_enum_string(ctl, control_value);
        printf("%s", string);
    }
}

static int tinymix_detail_control(struct mixer *mixer, const char *control, int *value,
                                  int prefix, int print_all)
{
    ALOGI("%s,value pointer addr: %p;", __func__, value);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values;
    unsigned int i;
    int min, max;
    int ret;
    char *buf = NULL;
    size_t len;
    unsigned int tlv_header_size = 0;
    const char *space = 0 ? "\t" : " ";

    if (isnumber(control))
        ctl = mixer_get_ctl(mixer, atoi(control));
    else
        ctl = mixer_get_ctl_by_name(mixer, control);

    if (!ctl) {
        fprintf(stderr, "Invalid mixer control: %s\n", control);
        return ENOENT;
    }

    type = mixer_ctl_get_type(ctl);
    num_values = mixer_ctl_get_num_values(ctl);

    if (type == MIXER_CTL_TYPE_BYTE) {
        if (mixer_ctl_is_access_tlv_rw(ctl)) {
            tlv_header_size = TLV_HEADER_SIZE;
        }
        buf = calloc(1, num_values + tlv_header_size);
        if (buf == NULL) {
            fprintf(stderr, "Failed to alloc mem for bytes %d\n", num_values);
            return ENOENT;
        }

        len = num_values;
        ret = mixer_ctl_get_array(ctl, buf, len + tlv_header_size);
        if (ret < 0) {
            fprintf(stderr, "Failed to mixer_ctl_get_array\n");
            free(buf);
            return ENOENT;
        }
    }

    if (prefix)
        printf("%s:%s", mixer_ctl_get_name(ctl), space);

    for (i = 0; i < num_values; i++) {
        switch (type)
        {
        case MIXER_CTL_TYPE_INT:
            ALOGI("%s,mixer_ctl_get_value: %d;TYPE_INT", __func__, mixer_ctl_get_value(ctl, i));
            *value = mixer_ctl_get_value(ctl, i);
            printf("%d", mixer_ctl_get_value(ctl, i));
            break;
        case MIXER_CTL_TYPE_BOOL:
            ALOGI("%s,mixer_ctl_get_value: %d;TYPE_BOOL", __func__, mixer_ctl_get_value(ctl, i));
            *value = mixer_ctl_get_value(ctl, i);
            printf("%s", mixer_ctl_get_value(ctl, i) ? "On" : "Off");
            break;
        case MIXER_CTL_TYPE_ENUM:
            ALOGI("%s,mixer_ctl_get_value: %d;TYPE_ENUM", __func__, mixer_ctl_get_value(ctl, i));
            *value = mixer_ctl_get_value(ctl, i);
            tinymix_print_enum(ctl, space, print_all);
            break;
        case MIXER_CTL_TYPE_BYTE:
            /* skip printing TLV header if exists */
            printf(" %02x", buf[i + tlv_header_size]);
            break;
        default:
            printf("unknown");
            break;
        }

        if (i < num_values - 1)
            printf("%s", space);
    }

    if (print_all) {
        if (type == MIXER_CTL_TYPE_INT) {
            min = mixer_ctl_get_range_min(ctl);
            max = mixer_ctl_get_range_max(ctl);
            printf("%s(dsrange %d->%d)", space, min, max);
        }
    }

    free(buf);

    printf("\n");
    return 0;
}

static void tinymix_set_byte_ctl(struct mixer_ctl *ctl,
    char **values, unsigned int num_values)
{
    int ret;
    char *buf;
    char *end;
    unsigned int i;
    long n;
    unsigned int *tlv, tlv_size;
    unsigned int tlv_header_size = 0;

    if (mixer_ctl_is_access_tlv_rw(ctl)) {
        tlv_header_size = TLV_HEADER_SIZE;
    }

    tlv_size = num_values + tlv_header_size;

    buf = calloc(1, tlv_size);
    if (buf == NULL) {
        fprintf(stderr, "set_byte_ctl: Failed to alloc mem for bytes %d\n", num_values);
        exit(EXIT_FAILURE);
    }

    tlv = (unsigned int *)buf;
    tlv[0] = 0;
    tlv[1] = num_values;

    for (i = 0; i < num_values; i++) {
        errno = 0;
        n = strtol(values[i], &end, 0);
        if (*end) {
            fprintf(stderr, "%s not an integer\n", values[i]);
            goto fail;
        }
        if (errno) {
            fprintf(stderr, "strtol: %s: %s\n", values[i],
                strerror(errno));
            goto fail;
        }
        if (n < 0 || n > 0xff) {
            fprintf(stderr, "%s should be between [0, 0xff]\n",
                values[i]);
            goto fail;
        }
        /* start filling after the TLV header */
        buf[i + tlv_header_size] = n;
    }

    ret = mixer_ctl_set_array(ctl, buf, tlv_size);
    if (ret < 0) {
        fprintf(stderr, "Failed to set binary control\n");
        goto fail;
    }

    free(buf);
    return;

fail:
    free(buf);
    exit(EXIT_FAILURE);
}

static int tinymix_set_value(struct mixer *mixer, const char *control,
                             char **values, unsigned int num_values)
{
	ALOGI("tinymix_set_value, control: %s; values: %s; num_values %d:", control, *values, num_values);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_ctl_values;
    unsigned int i;

    if (isnumber(control))
        ctl = mixer_get_ctl(mixer, atoi(control));
    else
        ctl = mixer_get_ctl_by_name(mixer, control);

    if (!ctl) {
        fprintf(stderr, "Invalid mixer control: %s\n", control);
        return ENOENT;
    }

    type = mixer_ctl_get_type(ctl);
    num_ctl_values = mixer_ctl_get_num_values(ctl);

    if (type == MIXER_CTL_TYPE_BYTE) {
        tinymix_set_byte_ctl(ctl, values, num_values);
        return ENOENT;
    }

    if (isnumber(values[0])) {
        if (num_values == 1) {
            /* Set all values the same */
            int value = atoi(values[0]);

            for (i = 0; i < num_ctl_values; i++) {
                if (mixer_ctl_set_value(ctl, i, value)) {
                    fprintf(stderr, "Error: invalid value\n");
                    return EINVAL;
                }
            }
        } else {
            /* Set multiple values */
            if (num_values > num_ctl_values) {
                fprintf(stderr,
                        "Error: %d values given, but control only takes %d\n",
                        num_values, num_ctl_values);
                return EINVAL;
            }
            for (i = 0; i < num_values; i++) {
                if (mixer_ctl_set_value(ctl, i, atoi(values[i]))) {
                    fprintf(stderr, "Error: invalid value for index %d\n", i);
                    return EINVAL;
                }
            }
        }
    } else {
        if (type == MIXER_CTL_TYPE_ENUM) {
            if (num_values != 1) {
                fprintf(stderr, "Enclose strings in quotes and try again\n");
                return EINVAL;
            }
            if (mixer_ctl_set_enum_by_string(ctl, values[0])) {
                fprintf(stderr, "Error: invalid enum value\n");
                return EINVAL;
            }
        } else {
            fprintf(stderr, "Error: only enum types can be set with strings\n");
            return EINVAL;
        }
    }

    return 0;
}

static int get_downline_config(void)
{
    int byte7 = -1;
    char buf[100];
    memset(buf, 0, 100);
    int retry_times = 0;
    while(1) {
	property_get(CAR_DOWNLINE_CONFIG, buf, "");
	#if defined(PLATFORM_CHERY_COMMERCIAL)
	if(strlen(buf) >= 13) {
	    if(strcmp(buf, CAR_PART_NUMBER)) {
		byte7 = 1<<7;
	    } else {
		byte7 = 0;
	    }
	    return byte7;
	} else {
	    usleep(1000*1000);
	    retry_times++;
	    if(retry_times > 4) {	//retry 5 times
		ALOGE("%s : get downline setting ERROR, have retry 5 times!", __func__);
		return -1;
	    }
	}
	#elif defined(PLATFORM_CHERY_AUTOMOBILE)
	char data[3];
	if(strlen(buf) > 7) {	//CHERY
	    data[0] = buf[6];
	    data[1] = buf[7];
	    data[2] = 0;
	    byte7 = strtol(data, NULL, 16) << 3;
	    property_get(CAR_DOWNLINE_CONFIG_TWO, buf, "");
	    if(strlen(buf) > 0) {
		data[0] = buf[0];		
		data[1] = 0;
		data[2] = 0;
		byte7 |= (strtol(data, NULL, 8) << 3) & 0x70; 
	    }
	    return byte7;	    
	} else {
	    usleep(1000*1000);
	    retry_times++;
	    if(retry_times > 4) {	//retry 5 times
		ALOGE("%s : get downline setting ERROR, have retry 5 times!", __func__);
		return -1;
	    }
	}
	#elif defined(PLATFORM_GAC_TRUMPCHI)
	char data[3];
	if (strlen(buf) > 16) {	//GAC
	    data[0] = buf[14];
	    data[1] = buf[15];
	    data[2] = 0;
	    byte7 = strtol(data, NULL, 16);
	    return byte7;
	} else {
	    usleep(1000*1000);
	    retry_times++;
	    if(retry_times > 4) {	//retry 5 times
		ALOGE("%s : get downline setting ERROR, have retry 5 times!", __func__);
		return -1;
	    }
	}
	#else
	ALOGE("%s: Do not config platform type, config downline filed!", __func__);
	return -1;
	#endif
    }
}

/* Close an opened dsp device instance */
static int dspdev_close(hw_device_t *device)
{
    free(device);
    return 0;
}

static int dsp_init()
{
    ALOGI("%s;", __func__);
    int external_pa = 0, speaker_num = 0, speaker_type = 0;
    char s_external_pa[4], s_speaker_num[4], s_speaker_type[4];
    int downline_config =  get_downline_config();

    if(downline_config == -1) {
	ALOGE("%s: get downline config failed!", __func__);
    } else {
	external_pa = (downline_config >> 7) & 0x01;
	speaker_num = (downline_config >> 4) & 0x07;
	speaker_type = downline_config & 0x0F;
	ALOGD("%s: get downline config exteral pa %d, speaker num %d, speaker type %d!", __func__, external_pa, speaker_num, speaker_type);
    }
    sprintf(s_external_pa, "%d", external_pa);
    sprintf(s_speaker_num, "%d", speaker_num);
    sprintf(s_speaker_type, "%d", speaker_type);
    dsp_control(tinymix_control_ExtPaConfig, s_external_pa);
    dsp_control(tinymix_control_SpeakerNum, s_speaker_num);
    dsp_control(tinymix_control_SpeakerType, s_speaker_type);
    /*Unmute media channel*/
    dsp_control(tinymix_control_MediaMute, tinymix_control_Mute_Off);

    /*play blank sound to eliminate pop sound in booting*/
    #if defined(PLATFORM_CHERY)
    dsp_control(tinymix_control_NaviVol, "0");
    dsp_control(tinymix_control_BeepVol, "0");
    usleep(1000*500);
    play_sound(blank_sound, sizeof(blank_sound));
    dsp_control(tinymix_control_NaviVol, "120");
    dsp_control(tinymix_control_BeepVol, "36");
    #endif
    return 0;
}

/**
* Play sound by pcm
* @paramer
*  sound_arry: audio data matrix 
*  size: size of audio data matrix
*/
static int play_sound(unsigned char* sound_array, int size)
{
	struct pcm *pcm1, *pcm2;
	int sound_size = size;
	int frame_size, sub_size; 
	char *buffer = (char *)sound_array;
	pcm1 = pcm_open(0, 1, PCM_OUT, &config);
	if (!pcm1 || !pcm_is_ready(pcm1)) {
		ALOGE("%s: pcm1 open failed!", __func__);
		return -1;
	}
	pcm2 = pcm_open(0, 2, PCM_OUT, &config);
	if (!pcm2 || !pcm_is_ready(pcm2)) {
		ALOGE("%s: pcm2 open failed!", __func__);
		return -1;
	}
	frame_size = pcm_frames_to_bytes(pcm1, pcm_get_buffer_size(pcm1));
	sub_size = frame_size;

        for(;sound_size > 0; sound_size -= sub_size) {
		if(sound_size < frame_size)
			frame_size = sound_size;
		if (pcm_write(pcm1, buffer, frame_size)) {
			ALOGE("pcm1_write error!\n");
		}
		if (pcm_write(pcm2, buffer, frame_size)) {
			ALOGE("pcm2_write error!\n");
		}
		buffer += frame_size;
        }
	if(pcm1) {
		pcm_close(pcm1);
		ALOGD("%s: pcm1 close", __func__);
	}	
	if(pcm2) {
		pcm_close(pcm2);
		ALOGD("%s: pcm2 close", __func__);
	}
	return 0;
}

static int dsp_control(const char* control, char* values){
	int ret=0;
	struct mixer *mixer;
	int card = 0;
	mixer = mixer_open(card);
	if (!mixer) {
		ALOGD("Failed to open mixer");
		return ENODEV;
	}
	ret = tinymix_set_value(mixer, control, &values, 1);
	mixer_close(mixer);
	return ret;
}

static int dsp_get_value(const char* control, int* value){
	ALOGD("dsp_get_value : control: %s; value pointer addr: %p;",control, value);
	int ret=0;
    struct mixer *mixer;
	int card = 0;
	mixer = mixer_open(card);
	if (!mixer) {
		ALOGD("Failed to open mixer");
		return ENODEV;
	}
	ret = tinymix_detail_control(mixer, control,value, 1, 1);
	mixer_close(mixer);
	return ret;
}
