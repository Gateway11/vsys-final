//
//  vsys_activation.cpp
//  vsys
//
//  Created by 薯条 on 2018/1/17.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include <stddef.h>

#include "vsys_activation.h"
#include "voice_activation.h"

VsysActivationInst VsysActivation_Create(const activation_param_t* param, const char* path, bool vad_enable){
    
    vsys::VoiceActivation* activation = new vsys::VoiceActivation;
    int ret = activation->init(param, path, vad_enable);
    if(ret != 0) delete activation;
    return (ret == 0) ? (VsysActivationInst) activation : 0;
}

void VsysActivation_Free(VsysActivationInst handle){
    if(handle) {
        delete ((vsys::VoiceActivation *)handle);
    }
}

void VsysActivation_RegisterVoiceEventCallback(VsysActivationInst handle, voice_event_callback callback, void* token){
    if(handle) {
        ((vsys::VoiceActivation *)handle)->regist_callback(callback, token);
    }
}

int32_t VsysActivation_Process(VsysActivationInst handle, const uint8_t* input, const size_t byte_size){
    int32_t ret = -1;
    if(handle) {
        ret = ((vsys::VoiceActivation *)handle)->process(input, byte_size);
    }
    return ret;
}

int32_t VsysActivation_Control(VsysActivationInst handle, const active_action action){
    int32_t ret = -1;
    if(handle) {
        ret = ((vsys::VoiceActivation *)handle)->control(action);
    }
    return ret;
}

int32_t VsysActivation_AddVtWord(VsysActivationInst handle, const vt_word_t* vt_word){
    int32_t ret = -1;
    if(handle) {
        ret = ((vsys::VoiceActivation *)handle)->add_vt_word(vt_word);
    }
    return ret;
}

int32_t VsysActivation_RemoveVtWord(VsysActivationInst handle, const char* word){
    int32_t ret = -1;
    if(handle) {
        ret = ((vsys::VoiceActivation *)handle)->remove_vt_word(word);
    }
    return ret;
}

int32_t VsysActivation_GetVtWords(VsysActivationInst handle, vt_word_t** vt_words_out){
    int32_t ret = -1;
    if(handle) {
        ret = ((vsys::VoiceActivation *)handle)->get_vt_words(*vt_words_out);
    }
    return ret;
}
