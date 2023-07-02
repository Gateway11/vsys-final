//
//  vsys_activation.h
//  vsys
//
//  Created by 薯条 on 2018/1/17.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef VSYS_ACTIVATION_H
#define VSYS_ACTIVATION_H

#include "vsys_types.h"

typedef uint64_t VsysActivationInst;
typedef void (*voice_event_callback)(voice_event_t* voice_event, void* token);

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * 创建激活算法句柄
 *
 * @param param 激活算法配置参数
 * @param path 资源文件相对路径
 * @param vad_enable 是否开启本地VAD
 *
 * @return 成功:激活算法句柄；失败:0
 */
VsysActivationInst VsysActivation_Create(const activation_param_t* param, const char* path, bool vad_enable);

/**
 * 释放激活算法句柄
 *
 * @param handle 激活算法句柄
 */
void VsysActivation_Free(VsysActivationInst handle);
    
/**
 * 注册激活事件回调
 *
 * @param handle 激活算法句柄
 * @param callback 回调函数指针
 * @param token 附加参数
 */
void VsysActivation_RegisterVoiceEventCallback(VsysActivationInst handle, voice_event_callback callback, void* token);

/**
 * 激活处理函数
 *
 * @param handle 激活算法句柄
 * @param input 音频裸数据，pcm格式
 * @param byte_size 音频流长度
 *
 * @return 成功:0；失败:-1
 */
int32_t VsysActivation_Process(VsysActivationInst handle, const uint8_t* input, const size_t byte_size);
    
/**
 * 激活状态控制接口
 *
 * @param handle 激活算法句柄
 * @param action 控制意图
 *
 * @return 成功:0；失败:-1
 */
int32_t VsysActivation_Control(VsysActivationInst handle, active_action action);
    
/**
 * 激活算法配置
 *
 * @param handle 激活算法句柄
 * @param key 键
 * @param val 值
 *
 * @return 成功:0；失败:-1
 */
//int32_t VsysActivation_Config(VsysActivationInst handle, active_param key, const void* val);
    
/**
 * 添加激活词
 *
 * @param handle 激活算法句柄
 * @param vt_word 激活词信息
 *
 * @return 成功:0；失败:-1
 */
int32_t VsysActivation_AddVtWord(VsysActivationInst handle, const vt_word_t* vt_word);
    
/**
 * 删除激活词
 *
 * @param handle 激活算法句柄
 * @param word 激活词中文字符串，UTF-8编码
 *
 * @return 成功:0；失败:-1
 */
int32_t VsysActivation_RemoveVtWord(VsysActivationInst handle, const char* word);
    
/**
 * 查询激活词
 *
 * @param handle 激活算法句柄
 * @param vt_words_out 传入参数
 *
 * @return 成功:激活词个数；失败:-1
 */
int32_t VsysActivation_GetVtWords(VsysActivationInst handle, vt_word_t** vt_words_out);

#ifdef __cplusplus
}
#endif

#endif /* VSYS_ACTIVATION_H */
