//
//  voice_activation.h
//  vsys
//
//  Created by 薯条 on 2018/1/17.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef VOICE_ACTIVATION_H
#define VOICE_ACTIVATION_H

#include <memory>
#include <atomic>

#include <r2ssp.h>
#include <NNVadIntf.h>
#include "zvbvapi.h"
#include "zvtapi.h"
#include "r2mem_cod.h"

#include "vsys_activation.h"
#include "vt_word_manager.h"
#include "audio_converter.h"
#include "event_hub.h"

namespace vsys {
    
#define VAD_ON_MASK (0x1 << 0)
#define VAD_OFF_MASK (0x1 << 1)
#define VAD_ON(flag) ((flag & VAD_ON_MASK) != 0)
#define VAD_OFF(flag) ((flag & VAD_OFF_MASK) != 0)

class VoiceActivation{
public:
    VoiceActivation();
    
    ~VoiceActivation();

    int32_t init(const activation_param_t* param, const char* path, bool vad_enable);
    
    void regist_callback(voice_event_callback callback, void* token);
    
    int32_t control(active_action action);
    
    int32_t add_vt_word(const vt_word_t* vt_word);
    
    int32_t remove_vt_word(const std::string& word);
    
    int32_t get_vt_words(vt_word_t*& vt_words_out);
    
    int32_t process(const uint8_t* input, const size_t& byte_size);
    
private:
    int32_t enter_vbv();
    
    void exit_vbv();
    
    void create_vt_manager(AcousticModel model);
    
    int32_t set_parameters(const activation_param_t* param);
    
    int32_t sync_vt_word(const WordInfo* word_info, const uint32_t& word_num);
    
    int32_t processfrm(const float** input, const size_t& input_size);
    
    int32_t beam_forming(const float** input, const uint32_t input_size, float*& output, uint32_t& output_size);
    
    void set_beam_former_steer(float azimuth, float elevation, int32_t steer = 1);
    
    int32_t bf_reset();
    
    int32_t check_voice_activity(const float* input, const uint32_t input_size, int32_t force_start, float*& output, uint32_t& output_size);
    
    void vad_reset();
    
    int32_t do_activation(const float** input, const uint32_t& len_in);
    
    void reset_asr();
    
    void check_buffer(float**& buff, uint32_t input_size, uint32_t& src_size);
    
    void check_buffer(float*& buff, uint32_t input_size, uint32_t& src_size);
    
    void set_vad_frame_num(int32_t frame_num);
    
    void send_event(uint32_t event);
    
    void get_vt_info(char*& vt_word, uint32_t& vt_begin, uint32_t& vt_end, float& vt_energy, uint32_t& vt_length);
    
    float get_sl_info();
    
private:
    char nnet_path[256];
    char nnet_path_ruoqi[256];
    char phone_table[256];
    
    r2ssp_handle bf_handle;
    r2_vbv_htask vbv_handle;
    VD_HANDLE vad_handle;
    
    float sl_info[2];
    
    WordInfo* pWordInfo;
    WordDetInfo* pWordDetInfo;
    
    std::shared_ptr<r2mem_cod> __cod;
    std::shared_ptr<EventHub> event_hub;
    std::shared_ptr<VtWordManager> vt_word_manager;
    std::shared_ptr<AudioConverter> audio_converter;
    
    std::atomic_flag locker = ATOMIC_FLAG_INIT;
    
    uint32_t sample_rate;
    uint32_t sample_size_bits;
    uint32_t num_mics;
    uint32_t num_channels;
    float* mic_pos;
    float* mic_delay;
    
    uint32_t* mic_ids;
    uint32_t frame_size;
    
    float** buff;
    uint32_t buff_offset;
    
    float* bf_input;
    uint32_t bf_input_offset;
    
    float* bf_output;
    uint32_t bf_output_total;
    
    float* vad_input;
    uint32_t vad_input_offset;
    uint32_t vad_input_total;
    
    float* vad_output;
    uint32_t vad_output_total;
    
    float** data_nonew;
    uint32_t col_nonew;
    
    int32_t frame_num;
    uint32_t last_frame;
    uint32_t vad_flag;
    bool has_vad;
    bool vad_enable;
    
    bool need_asr;
    bool awaked;
    bool canceled;
    bool data_output;
};
    
}

#endif /* VOICE_ACTIVATION_H */
