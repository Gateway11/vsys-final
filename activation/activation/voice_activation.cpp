//
//  voice_activation.cpp
//  vsys
//
//  Created by 薯条 on 2018/1/17.
//  Copyright © 2018年 薯条. All rights reserved.
//

#define VSERION(tag) (tag ": " __DATE__ "-" __TIME__)

#define LOG_TAG "activation"
#define SAMPLE_RATE_MS 10

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fstream>

#include "debug.h"
#include "vsys_audio.h"
#include "voice_activation.h"
#include "buf_manager.h"
#include "audio_converter.h"

namespace vsys {
    
//std::ofstream output_stream("/Users/daixiang/Desktop/vsys/data/sounds/baomao_M_0020.16000.1.16bit.pcm", std::ios::out | std::ios::binary);
VoiceActivation::VoiceActivation():mic_ids(nullptr), mic_pos(nullptr), mic_delay(nullptr), buff(nullptr),
                                data_nonew(nullptr), bf_input(nullptr), bf_output(nullptr), vad_input(nullptr), vad_output(nullptr),
                                bf_handle(0), vbv_handle(0), vad_handle(0), buff_offset(0), bf_input_offset(0), bf_output_total(0),
                                col_nonew(0), vad_input_offset(0), frame_num(45), last_frame(0), vad_flag(0),
                                need_asr(false), has_vad(false), data_output(false), awaked(false), canceled(false), vad_enable(true){
    
    memset(nnet_path, 0, sizeof(nnet_path));
    memset(nnet_path_ruoqi, 0, sizeof(nnet_path_ruoqi));
    memset(phone_table, 0, sizeof(phone_table));
    memset(sl_info, 0, sizeof(float) * 2);
                                    
    event_hub = std::make_shared<EventHub>();
}
    
VoiceActivation::~VoiceActivation(){
    exit_vbv();
    
    delete[] mic_ids;
    delete[] mic_pos;
    delete[] mic_delay;
    
    release_buffer2((void **)buff);
    release_buffer2((void **)data_nonew);
    
    release_buffer(bf_input);
    release_buffer(bf_output);
    
    release_buffer(vad_input);
    release_buffer(vad_output);
}
    
int32_t VoiceActivation::init(const activation_param_t* param, const char* path, bool vad_enable){
    assert(param);
    
    if(!audio_is_valid_sample_rate(param->sample_rate)){
        VSYS_DEBUGE("Unsupport sample rate %d", param->sample_rate);
        return -1;
    }
    if(!audio_is_valid_format(param->sample_size_bits)){
        VSYS_DEBUGE("Unsupport bits per sample %d", param->sample_size_bits);
        return -1;
    }
    if(set_parameters(param) != 0){
        return -1;
    }
    
    frame_size = sample_rate / 1000 * SAMPLE_RATE_MS;
    this->vad_enable = vad_enable;
    
    VSYS_DEBUGI("init(): sample rate %d, sample bits %d, mic num %d, channel num %d, frame size %d",
                sample_rate, sample_size_bits, num_mics, num_channels, frame_size);
    
    snprintf(nnet_path, sizeof(nnet_path), "%s/workdir_cn/rasr.emb.ini", path);
    std::ifstream nnet_stream(nnet_path, std::ios::in | std::ios::binary);
    if(!nnet_stream){
        memset(nnet_path, 0, sizeof(nnet_path));
        snprintf(nnet_path, sizeof(nnet_path), "%s/workdir_cn/final.svd.mod", path);
        nnet_stream.open(nnet_path, std::ios::in | std::ios::binary);
        if(!nnet_stream){
            VSYS_DEBUGE("%s not exist or permission denied", nnet_path);
            return -1;
        }
        create_vt_manager(MODEL_DNN);
        snprintf(nnet_path_ruoqi, sizeof(nnet_path), "%s/workdir_cn/final.ruoqi.mod", path);
        snprintf(phone_table, sizeof(nnet_path), "%s/workdir_cn/phonetable", path);
    }else{
        create_vt_manager(MODEL_CTC);
    }
    nnet_stream.close();

    __cod = std::make_shared<r2mem_cod>(sample_size_bits);
    
    audio_converter = std::make_shared<AudioConverter>(sample_size_bits, num_mics, num_channels, mic_ids);
    
    buff = malloc_buffer2(num_mics, frame_size);
    
    if(num_mics > 1)
    bf_input = (float *)malloc_buffer(frame_size * num_mics * sizeof(float));
    
    vad_input_total = frame_size;
    vad_input = (float *)malloc_buffer(vad_input_total * sizeof(float));
    
    vad_output_total = frame_size;
    vad_output = (float *)malloc_buffer(vad_output_total * sizeof(float));
    
    return enter_vbv();
}
    
void VoiceActivation::create_vt_manager(AcousticModel model){
    vt_word_manager = std::make_shared<VtWordManager>((void *)this,
          [](void* token, const WordInfo* word_info, const uint32_t& word_num)->int32_t{
              return ((VoiceActivation *)token)->sync_vt_word(word_info, word_num);
          }, model);
}
    
int32_t VoiceActivation::set_parameters(const activation_param_t* param){
    
    sample_rate = param->sample_rate;
    sample_size_bits = param->sample_size_bits;
    num_mics = param->num_mics;
    num_channels = param->num_channels;
    
    mic_ids = new uint32_t[num_mics];
    mic_pos = new float[num_mics * 3];
    mic_delay = new float[num_mics];
    memset(mic_ids, 0, num_mics * sizeof(uint32_t));
    memset(mic_pos, 0, num_mics * 3 * sizeof(float));
    memset(mic_delay, 0, num_mics * sizeof(float));
    
    if(param->mic_params != nullptr){
        for (uint32_t i = 0; i < num_mics; i++) {
            if(param->mic_params[i].id >= num_channels){
                VSYS_DEBUGE("Invalid mic id %d, expected id range [0, %d]", param->mic_params[i].id, num_channels - 1);
                return -1;
            }
            mic_ids[i] = param->mic_params[i].id;
        }
        if(param->mask & MIC_PARAM_POSTION_MASK){
            for (uint32_t i = 0; i < num_mics; i++) {
                mic_pos[i * 3 + 0] = param->mic_params[i].position.x;
                mic_pos[i * 3 + 1] = param->mic_params[i].position.y;
                mic_pos[i * 3 + 2] = param->mic_params[i].position.z;
            }
        }
        if(param->mask & MIC_PARAM_DELAY_MASK){
            for (uint32_t i = 0; i < num_mics; i++) {
                mic_delay[i] = param->mic_params[i].delay;
            }
        }
    }else if(num_mics > 1 || num_channels != 1){
        VSYS_DEBUGE("Invalid parameter : mic num %d, channel num %d", num_mics, num_channels);
        return -1;
    }
    for (uint32_t i = 0; i < num_mics; i++) {
        VSYS_DEBUGD("set_parameters(): %d, [%f, %f, %f], %f",
                    mic_ids[i], mic_pos[i * 3 + 0], mic_pos[i * 3 + 1], mic_pos[i * 3 + 2], mic_delay[i]);
    }
    return 0;
}
    
int32_t VoiceActivation::enter_vbv(){
    if(num_mics > 1){
        if(!(bf_handle = r2ssp_bf_create(mic_pos, num_mics))){
            VSYS_DEBUGE("Failed to create bf inst %d  %p", num_mics, mic_pos);
            return -1;
        }
        if(r2ssp_bf_init(bf_handle, SAMPLE_RATE_MS, sample_rate) != 0){
            //
        }
        if(r2ssp_bf_set_mic_delays(bf_handle, mic_delay, num_mics) != 0){
            //
        }
        set_beam_former_steer(sl_info[0], sl_info[1]);
    }
    if(!(vbv_handle = r2_vbv_create(num_mics, mic_pos, mic_delay, nnet_path, phone_table))){
        VSYS_DEBUGE("Failed to create vbv inst");
        return -1;
    }
    
    WordInfo pWordLst[2];
    pWordLst[0].iWordType = WORD_AWAKE ;
    strcpy(pWordLst[0].pWordContent_UTF8, "若琪");
    strcpy(pWordLst[0].pWordContent_PHONE, "r|l|r_B|l_B|# w o4|o4_E|## q|q_B|# i2|i2_E|##");
    pWordLst[0].fBlockAvgScore = 4.2 ;
    pWordLst[0].fBlockMinScore = 2.7 ;
    pWordLst[0].bLeftSilDet = true ;
    pWordLst[0].bRightSilDet = false ;
    pWordLst[0].bRemoteAsrCheckWithAec = true ;
    pWordLst[0].bRemoteAsrCheckWithNoAec = true ;
    pWordLst[0].bLocalClassifyCheck = true ;
    pWordLst[0].fClassifyShield = -0.3f ;
    strcpy(pWordLst[0].pLocalClassifyNnetPath, nnet_path_ruoqi);
    
    sync_vt_word(pWordLst, 1);

    if(!(vad_handle = VD_NewVad(1))){
        VSYS_DEBUGE("Failed to create vad inst");
        return -1;
    }
    return 0;
}
    
void VoiceActivation::exit_vbv(){
    r2ssp_bf_free(bf_handle);
    r2_vbv_free(vbv_handle);
    VD_DelVad(vad_handle);
}
    
int32_t VoiceActivation::sync_vt_word(const WordInfo* word_info, const uint32_t& word_num){
    for (uint32_t i = 0;  i < word_num; i++) {
        VSYS_DEBUGD("%s", word_info[i].pWordContent_PHONE);
        VSYS_DEBUGD("%s", word_info[i].pWordContent_UTF8);
        VSYS_DEBUGD("%f", word_info[i].fBlockAvgScore);
        VSYS_DEBUGD("%f", word_info[i].fBlockMinScore);
        VSYS_DEBUGD("%d", word_info[i].bLeftSilDet);
        VSYS_DEBUGD("%d", word_info[i].bRightSilDet);
        VSYS_DEBUGD("%d", word_info[i].bRemoteAsrCheckWithAec);
        VSYS_DEBUGD("%d", word_info[i].bRemoteAsrCheckWithNoAec);
        VSYS_DEBUGD("%d", word_info[i].bLocalClassifyCheck);
        VSYS_DEBUGD("%f", word_info[i].fClassifyShield);
        VSYS_DEBUGD("%s", word_info[i].pLocalClassifyNnetPath);
        VSYS_DEBUGD("---------------------------------------------------------------------");
    }
    
    while (locker.test_and_set(std::memory_order_acquire));
    int ret = r2_vbv_setwords(vbv_handle, word_info, word_num);
    locker.clear(std::memory_order_release);
    return ret;
}
    
int32_t VoiceActivation::control(active_action action){
    switch (action) {
        case ACTIVATION_SET_STATE_AWAKE:
            need_asr = true;
            break;
        case ACTIVATION_SET_STATE_SLEEP:
            awaked = false;
            need_asr = false;
            break;
        default:
            return -1;
    }
    return 0;
}

int32_t VoiceActivation::add_vt_word(const vt_word_t* vt_word){
    if(vt_word == nullptr) return -1;
    
    VSYS_DEBUGD("word %s,       phone %s", vt_word->word_utf8, vt_word->phone);
    
    if(strlen(vt_word->word_utf8) <= 0 || strlen(vt_word->phone) <= 0){
        return -1;
    }
    return vt_word_manager->add_vt_word(vt_word);
}

int32_t VoiceActivation::remove_vt_word(const std::string& word){
    if(word.empty()) return -1;
    return vt_word_manager->remove_vt_word(word);
}

int32_t VoiceActivation::get_vt_words(vt_word_t*& vt_words_out){
    return vt_word_manager->get_vt_words(vt_words_out);
}

void VoiceActivation::regist_callback(voice_event_callback callback, void* token){
    event_hub->add_callback(callback, token);
}

int32_t VoiceActivation::process(const uint8_t* input, const size_t& byte_size){
    
    if(byte_size > 0 && input == nullptr) return -1;
    if(byte_size % ((audio_bytes_per_sample(sample_size_bits) * num_channels)) != 0) {
        VSYS_DEBUGE("Unknown input format");
        return -1;
    }

    float** data_mul;
    uint32_t len_mul;

    audio_converter->convert((void *)input, byte_size, data_mul, len_mul);
//    pcm_out.write((char *)(data_mul[0]), len_mul * sizeof(float));
    
    uint32_t total = len_mul + buff_offset;
    uint32_t num_frames = total / frame_size;
    uint32_t offset = buff_offset, offset2 = 0;
    
    int32_t ret = 0;
//    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < num_frames; i++) {
        for (uint32_t j = 0; j < num_mics; j++) {
            memcpy(buff[j] + offset, data_mul[j] + i * frame_size - offset2, (frame_size - offset) * sizeof(float));
        }
//        pcm_out.write((char *)(buff[0]), frame_size * sizeof(float));
        
        ret = processfrm((const float **)buff, frame_size);
        
        offset = 0;
        offset2 = buff_offset;
    }
//    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tp);
//    VSYS_DEBUGI("----------------------------------------                  %lld", elapsed.count());
    buff_offset = total % frame_size;
    uint32_t length = total > frame_size ? buff_offset : len_mul;
    for (uint32_t i = 0; i < num_mics; i++) {
        memcpy(buff[i], data_mul[i] + (len_mul - length), length * sizeof(float));
    }
    return ret;
}
    
void VoiceActivation::reset_asr(){
    vad_flag = 0;
    awaked = false;
    canceled = false;
    need_asr = false;
    vad_reset();
    bf_reset();
    __cod->reset();

    if(data_output && !canceled) send_event(VOICE_EVENT_VAD_CANCEL);
    if(vad_enable) data_output = false;
}
    
int32_t VoiceActivation::processfrm(const float** input, const size_t& input_size){
    
    float* data;
    uint32_t data_len;

    beam_forming(input, input_size, data, data_len);

    int32_t vbv_result = do_activation((const float **)input, input_size);
    
    bool awake_pre = (vbv_result & R2_VT_WORD_PRE) != 0 && (vbv_result & R2_VT_WORD_CANCEL) == 0;
    if(awake_pre){
        reset_asr();
        
        r2_vbv_getdetwordinfo(vbv_handle, (const WordInfo **)&pWordInfo, (const WordDetInfo **)&pWordDetInfo);
        
        memset(sl_info, 0, sizeof(float) * 2);
        set_beam_former_steer(pWordDetInfo->fWordSlInfo[0], pWordDetInfo->fWordSlInfo[1]);
        int begin_len = pWordDetInfo->iWordPos_Start + 20 * frame_size;
        
        check_buffer(data_nonew, begin_len, col_nonew);
        r2_vbv_getlastaudio(vbv_handle, begin_len, 0, data_nonew);
        if(!(awake_pre && (vbv_result & R2_VT_WORD_DET_NOCMD) != 0 && pWordInfo->iWordType == WORD_SLEEP)){
            beam_forming((const float **)data_nonew, begin_len, data, data_len);
        }
    }
    
    bool awake = awake_pre && pWordInfo->iWordType == WORD_AWAKE;
    bool awake_cmd = awake_pre && (vbv_result & R2_VT_WORD_DET_CMD) != 0 && pWordInfo->iWordType == WORD_AWAKE;
    bool awake_nocmd = awake_pre && (vbv_result & R2_VT_WORD_DET_NOCMD) != 0 && pWordInfo->iWordType == WORD_AWAKE;
    bool sleep_cmd = awake_pre && (vbv_result & R2_VT_WORD_DET_CMD) != 0 && pWordInfo->iWordType == WORD_SLEEP;
    bool sleep_nocmd = awake_pre && (vbv_result & R2_VT_WORD_DET_NOCMD) != 0 && pWordInfo->iWordType == WORD_SLEEP;
    bool hotword_cmd = awake_pre && (vbv_result & R2_VT_WORD_DET_CMD) != 0 && pWordInfo->iWordType == WORD_HOTWORD;
    bool hotword_nocmd = awake_pre && (vbv_result & R2_VT_WORD_DET_NOCMD) != 0 && pWordInfo->iWordType == WORD_HOTWORD;
    
    bool cmd = awake_cmd || sleep_cmd || hotword_cmd;
    bool nocmd = awake_nocmd || sleep_nocmd || hotword_nocmd;
    
    int32_t force_start = awake ? 1 : 0;
    int32_t vad_result = check_voice_activity(data, data_len, force_start, data, data_len);

    if(VAD_ON(vad_result)){
        VSYS_DEBUGI("VAD ON");
        vad_flag = vad_result;

        awaked = false;
        canceled = false;
        data_output = false;
        __cod->reset();
        if(awake){
            awaked = true;
            need_asr = true;
            send_event(VOICE_EVENT_VAD_COMING);
            send_event(VOICE_EVENT_VT_INFO);
        }
    }
    if(nocmd){
        if(sleep_nocmd) send_event(VOICE_EVENT_LOCAL_SLEEP);
        if(awake_nocmd) send_event(VOICE_EVENT_WAKE_NOCMD);
        if(hotword_nocmd) send_event(VOICE_EVENT_HOTWORD);
    
        if(!sleep_nocmd) set_vad_frame_num(vad_enable ? -1 : 2000);
    }
    if(cmd){
        set_vad_frame_num(vad_enable ? -1 : 2000);
        if(awake_cmd) send_event(VOICE_EVENT_LOCAL_WAKE);
    }
    if(VAD_ON(vad_flag)){
        __cod->process(data, data_len);
        if(!data_output){
            if(cmd || nocmd){
                data_output = true;
                if(cmd){
                    VSYS_DEBUGD("Vad start with wake pre");
                    send_event(VOICE_EVENT_VAD_START);
                }else{
                    canceled = true;
                    __cod->pause();
                }
            }else if(need_asr && !awaked){
                data_output = true;
                if(!vad_enable) set_vad_frame_num(2000);
                
                VSYS_DEBUGD("Vad start with !awaked");
                send_event(VOICE_EVENT_VAD_START);
            }
        }
        if(data_output){
            if(!canceled){
                if(vad_enable && __cod->istoolong()){
                    canceled = true;
                    VSYS_DEBUGI("Cancel since asr too long");
                }
                if(!awaked && !need_asr){
                    canceled = true;
                    VSYS_DEBUGI("Cancel since no asr flag");
                }
                if(canceled){
                    if(!vad_enable){
                        vad_flag = 0;
                        data_output = false;
                        set_vad_frame_num(-1);
                    }
                    __cod->pause();
                    send_event(VOICE_EVENT_VAD_CANCEL);
                }
            }
            if(vad_enable && canceled && awaked){
                if(__cod->isneedresume()){
                    canceled = false;
                    __cod->resume();
                    send_event(VOICE_EVENT_VAD_START);
                }
            }
            if(!canceled){
                if(!awake_pre && __cod->getdatalen() > 100)
                    send_event(VOICE_EVENT_VAD_DATA);
                if(VAD_OFF(vad_result) && __cod->getdatalen() > 0)
                    send_event(VOICE_EVENT_VAD_DATA);
            }
        }
    }
    if(VAD_OFF(vad_result)){
        VSYS_DEBUGI("VAD OFF");
        vad_flag = 0;

        if(vad_enable) 
            need_asr = false;

        if(vad_enable && data_output && !canceled){
            send_event(VOICE_EVENT_VAD_END);
        }
        data_output = false;
    }
    return 0;
}
    
int32_t VoiceActivation::beam_forming(const float** input, const uint32_t input_size,
                                      float*& output, uint32_t& output_size){
    if(bf_handle){
        uint32_t total = input_size + bf_input_offset;
        uint32_t num_frames = total / frame_size;
        uint32_t offset = bf_input_offset, offset2 = 0;
        
        check_buffer(bf_output, num_frames * frame_size, bf_output_total);
        for (uint32_t i = 0; i < num_frames; i++) {
            for (uint32_t j = 0; j < num_mics; j++) {
                memcpy(bf_input + j * frame_size + offset, input[j] + i * frame_size - offset2, (frame_size - offset) * sizeof(float));
            }
            if(r2ssp_bf_process(bf_handle, bf_input, frame_size * num_mics, num_mics, bf_output + i * frame_size) != 0){
//                return -1;
            }
            offset = 0;
            offset2 = bf_input_offset;
        }
        bf_input_offset = total % frame_size;
        uint32_t length = total > frame_size ? bf_input_offset : input_size;
        for (uint32_t i = 0; i < num_mics; i++) {
            memcpy(bf_input + i * frame_size + offset, input[i] + (input_size - length), length * sizeof(float));
        }
        output_size = frame_size * num_frames;
    }else{
        check_buffer(bf_output, input_size, bf_output_total);
        memcpy(bf_output, input[0], input_size * sizeof(float));
        output_size = input_size;
    }
    output = bf_output;
    return 0;
}
    
void VoiceActivation::set_beam_former_steer(float azimuth, float elevation, int32_t steer){
    sl_info[0] = azimuth;
    sl_info[1] = elevation;
    if(steer > 0 && bf_handle){
        r2ssp_bf_steer(bf_handle, sl_info[0], sl_info[1], sl_info[0] + 3.1415926f, 0);
    }
}
    
int32_t VoiceActivation::bf_reset(){
    if(num_mics > 1){
        r2ssp_bf_free(bf_handle);
        bf_handle = r2ssp_bf_create(mic_pos, num_mics);
        r2ssp_bf_init(bf_handle, SAMPLE_RATE_MS, sample_rate);
        r2ssp_bf_set_mic_delays(bf_handle, mic_delay, num_mics);
        set_beam_former_steer(sl_info[0], sl_info[1]);
    }
    return 0;
}

int32_t VoiceActivation::check_voice_activity(const float* input, const uint32_t input_size, int32_t force_start,
                                             float*& output, uint32_t& output_size){

    uint32_t num_frames = (input_size + vad_input_offset) / frame_size;
    write_to_buffer(&vad_input, &vad_input_offset, &vad_input_total, input, input_size);
    
    uint32_t i, ret = 0, offset = 0;
    for (i = 0; i < num_frames; i++) {
        if(force_start){
            force_start = 0;
            VD_SetStart(vad_handle, 0);
            set_vad_frame_num(2000);
        }
        VD_InputFloatWave(vad_handle, vad_input + i * frame_size, frame_size, false, false);
        if(!has_vad){
            if(VD_GetVoiceStartFrame(vad_handle) >= 0){
                ret |= VAD_ON_MASK;
                has_vad = true;
            }
        }else{
            if(VD_GetVoiceStopFrame(vad_handle) > 0){
                ret |= VAD_OFF_MASK;
            }
        }
        if(has_vad){
            uint32_t curr_frame = VD_GetVoiceFrameNum(vad_handle);
            if(curr_frame > last_frame){
                float* vad_data = (float *)VD_GetFloatVoice(vad_handle) + last_frame * frame_size;
                uint32_t vad_len = (curr_frame - last_frame) * frame_size;
//                pcm_out.write((char *)vad_data, vad_len * sizeof(float));
                write_to_buffer(&vad_output, &offset, &vad_output_total, vad_data, vad_len);
                last_frame = curr_frame;
            }
            if(VAD_OFF(ret)){
                vad_reset();
                i++;
                break;
            }
        }
    }
    output = vad_output;
    output_size = offset;
    
    vad_input_offset -= i * frame_size;
    memcpy(vad_input, vad_input + i * num_frames, vad_input_offset * sizeof(float));
    return ret;
}
    
void VoiceActivation::set_vad_frame_num(int32_t frame_num){
    VSYS_DEBUGI("---------------------set vad end %d0 ms", (frame_num > 0) ? frame_num : this->frame_num);
    VD_SetVadParam(vad_handle, VD_PARAM_MINSILFRAMENUM, frame_num > 0 ? &frame_num : &this->frame_num);
}
    
void VoiceActivation::vad_reset(){
    last_frame = 0;
    has_vad = false;
    VD_RestartVad(vad_handle);
}
    
void VoiceActivation::check_buffer(float*& buff, uint32_t input_size, uint32_t& src_size){
    if(input_size > src_size){
        src_size = input_size;
        remalloc_buffer(&buff, 0, src_size);
    }
}
    
void VoiceActivation::check_buffer(float**& buff, uint32_t input_size, uint32_t& src_size){
    if(input_size > src_size){
        src_size = input_size;
        remalloc_buffer2(&buff, num_mics, 0, src_size);
    }
}

int32_t VoiceActivation::do_activation(const float** input, const uint32_t& len_in){
    
    while (locker.test_and_set(std::memory_order_acquire));
    int32_t vbv_result = r2_vbv_process(vbv_handle, input, len_in, 0, true);
    locker.clear(std::memory_order_release);
    return vbv_result;
}
    
void VoiceActivation::send_event(uint32_t event){
    char* data = nullptr;
    uint32_t length = 0, vt_begin = 0, vt_end = 0;
    float sl = 0.0f, vt_energy = 0.0f;
    
    switch (event) {
        case VOICE_EVENT_VAD_COMING:
            sl = get_sl_info();
            break;
        case VOICE_EVENT_VT_INFO:
            get_vt_info(data, vt_begin, vt_end, vt_energy, length);
            break;
        case VOICE_EVENT_VAD_DATA:
            __cod->getdata2(data, *(int *)&length);
            break;
    }
    char* temp = new char[sizeof(voice_event_t) + length];
    memset(temp, 0, sizeof(voice_event_t) + length);
    voice_event_t* voice_event = (voice_event_t *)temp;
    
    voice_event->event = event;
    voice_event->energy = VD_GetLastFrameEnergy(vad_handle);
    voice_event->threshold_energy = VD_GetThresholdEnergy(vad_handle);
    voice_event->sound_location = sl;
    
    voice_event->vt_info.begin = vt_begin;
    voice_event->vt_info.end = vt_end;
    voice_event->vt_info.energy = vt_energy;
    
    if(length){
        char* buf = temp + sizeof(voice_event_t);
        voice_event->data = buf;
        memcpy(buf, data, length);
    }
    voice_event->length = length;
    
    if(event == VOICE_EVENT_VAD_DATA){
//        pcm_out.write((char *)voice_event->data, length);
    }
    event_hub->send_voice_event(voice_event);
}
    
void VoiceActivation::get_vt_info(char*& vt_word, uint32_t& vt_begin, uint32_t& vt_end, float& vt_energy, uint32_t& vt_length){
    if(pWordInfo != nullptr && pWordDetInfo != nullptr){
        vt_word = pWordInfo->pWordContent_UTF8;
        vt_length = strlen(pWordInfo->pWordContent_UTF8);
        vt_begin = 20 * frame_size;
        vt_end = vt_begin + pWordDetInfo->iWordPos_Start - pWordDetInfo->iWordPos_End;
        vt_energy = pWordDetInfo->fWordEnergy;
    }
    VSYS_DEBUGI("%s     %d      %d      %f", vt_word, vt_begin, vt_end, vt_energy);
}
    
float VoiceActivation::get_sl_info(){
    int32_t azimuth = (sl_info[0] - 3.1415936f) * 180 / 3.1415936f + 0.1f;
    while (azimuth < 0) azimuth += 360;
    while (azimuth >= 360) azimuth -= 360;
//    VSYS_DEBUGI("-----------------------------------%f", azimuth);
    return azimuth;
}
    
}
