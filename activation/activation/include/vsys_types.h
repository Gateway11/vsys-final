//
//  vsys_types.h
//  vsys
//
//  Created by 薯条 on 2018/1/18.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef VSYS_TYPES_H
#define VSYS_TYPES_H

#include <inttypes.h>
#include <cstddef>

enum {
    AUDIO_FORMAT_ENCODING_PCM_16BIT         = 0x1,
    AUDIO_FORMAT_ENCODING_PCM_24BIT         = 0x2,
    AUDIO_FORMAT_ENCODING_PCM_32BIT         = 0x3,
    AUDIO_FORMAT_ENCODING_PCM_FLOAT         = 0x4,
};

enum {
    AUDIO_SAMPLT_RATE_16K           = 16000,
//    AUDIO_SAMPLT_RATE_48K         = 48000,
//    AUDIO_SAMPLT_RATE_96K         = 96000,
};

enum active_param_mask{
    MIC_PARAM_POSTION_MASK          = 1 << 0,
    MIC_PARAM_DELAY_MASK            = 1 << 1,
};

typedef struct {
    float x, y, z;
}position_t;

typedef struct {
    position_t position;                // 麦克风三维坐标
    uint32_t id;                        // 麦克风通道id
    float delay;                        // 麦克风延时
}mic_param_t;

typedef struct {
    uint32_t sample_rate;               // 采样率
    uint32_t sample_size_bits;          // 比特率
    uint32_t num_mics;                  // 麦克风数量
    uint32_t num_channels;              // 通道数量
    mic_param_t* mic_params;            // 麦克风精确配置
    uint32_t mask; 
}activation_param_t;

typedef struct {
    size_t begin;                       //
    size_t end;                         // 
    float energy;                       // 激活词能量
} vt_info_t;

typedef struct{
    vt_info_t vt_info;                  // 激活信息
    
    uint32_t event;                     // 激活事件
    size_t length;                      // 当event为VOICE_EVENT_VT_INFO时，length为激活词长度
                                        // 当event为VOICE_EVENT_VAD_DATA时，length为vad长度
    float energy;                       // 
    float threshold_energy;             // 背景噪声预值
    float sound_location;               // 激活角度
    
    void* data;
}voice_event_t;

enum {
    VOICE_EVENT_LOCAL_WAKE = 100,
    VOICE_EVENT_LOCAL_SLEEP,
    VOICE_EVENT_VT_INFO,
    VOICE_EVENT_HOTWORD,
    VOICE_EVENT_WAKE_NOCMD,
    VOICE_EVENT_VAD_COMING,
    VOICE_EVENT_VAD_START,
    VOICE_EVENT_VAD_DATA,
    VOICE_EVENT_VAD_END,
    VOICE_EVENT_VAD_CANCEL,
};

enum active_action{
    ACTIVATION_SET_STATE_AWAKE = 200,   // 设置当前为激活状态
    ACTIVATION_SET_STATE_SLEEP,         // 设置当前为休眠状态
};

//enum active_param{
//
//};

enum word_type{
    VSYS_WORD_AWAKE = 1 ,               // 激活词
    VSYS_WORD_SLEEP ,                   // 休眠词
    VSYS_WORD_HOTWORD ,                 // 热词
    VSYS_WORD_OTHER,                    // 保留
};

enum vt_word_mask{
    VT_WORD_USE_OUTSIDE_PHONE_MASK          = 1 << 0,
    VT_WORD_BLOCK_AVG_SCORE_MASK            = 1 << 1,
    VT_WORD_BLOCK_MIN_SCROE_MASK            = 1 << 2,
    VT_WORD_LEFT_SIL_DET_MASK               = 1 << 3,
    VT_WORD_RIGHT_SIL_DET_MASK              = 1 << 4,
    VT_WORD_REMOTE_CHECK_WITH_AEC_MASK      = 1 << 5,
    VT_WORD_REMOTE_CHECK_WITH_NOAEC_MASK    = 1 << 6,
    VT_WORD_LOCAL_CLASSIFY_CHECK_MASK       = 1 << 7,
    VT_WORD_CLASSIFY_SHIELD_MASK            = 1 << 8,
};

typedef struct{
    char phone[256];                    // 激活词内容，phone串 
    char nnet_path[256];                // 本地二次确认模型绝对路径
    char word_utf8[128];                // 激活词中文字符串，UTF-8编码
    uint64_t mask;                      //
    float block_avg_score;              // 所有phone声学平均得分门限(建议大于等于3.2，小于等于4.2)
    float block_min_score;              // 单个phone声学最小得分门限(建议大于等于1.7，小于等于2.7)
    float classify_shield;              // 本地二次确认门限，只在本地有模型，并开启本地二次确认情况下有效(建议为-0.3)
    bool left_sil_det;                  // 开启左静音检测
    bool right_sil_det;                 // 开启右静音检测
    bool remote_asr_check_with_aec;     // AEC条件下远端二次确认
    bool remote_asr_check_without_aec;  // 非AEC条件下远端二次确认
    word_type type;                     // 激活词类型，见{word_type}定义
}vt_word_t;

#endif /* VSYS_TYPES_H */
