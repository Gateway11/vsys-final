//
//  aec_wrap.cpp
//  vsys
//
//  Created by 薯条 on 18/2/5.
//  Copyright © 2018年 薯条. All rights reserved.
//
#include <fstream>

#include "aec_wrap.h"
#include "buf_manager.h"

namespace vsys {
    
//std::ofstream pcm_out("/Users/daixiang/Desktop/vsys/data/sounds/pcm_out.pcm", std::ios::out | std::ios::binary);
//Aec::Aec(const audio_format_t& audio_format, vsys_uint_t* mic_ids,
//    vsys_uint_t speaker_num, vsys_uint_t* speaker_ids, vsys_float_t scaling)
//    :audio_format(audio_format), speaker_num(speaker_num), scaling(scaling), buff_len(0), temp_len(0){
//
//    _mic_ids = (vsys_uint_t *)vsys_allocate(audio_format.numChannels * sizeof(vsys_uint_t));
//    _speaker_ids = (vsys_uint_t *)vsys_allocate(speaker_num * sizeof(vsys_uint_t));
//    memcpy(_mic_ids, mic_ids, audio_format.numChannels * sizeof(vsys_uint_t));
//    memcpy(_speaker_ids, speaker_ids, speaker_num * sizeof(vsys_uint_t));
//
//    aec_handle = r2ssp_aec_create(0);
//    r2ssp_aec_init(aec_handle, audio_format.sampleRateHz, audio_format.numChannels, speaker_num);
//
//    frame_size = audio_format.sampleRateHz / 1000 * 16;
//    frame_size_o = audio_format.sampleRateHz / 1000 * 10;
//    buff_o_size = frame_size;
//
//    buff = (vsys_float_t *)vsys_allocate((audio_format.numChannels + speaker_num) * frame_size * sizeof(vsys_float_t));
//    temp = vsys_allocate2(audio_format.numChannels, frame_size);
//    buff_o = vsys_allocate2(audio_format.numChannels, buff_o_size);
//}
//
//vsys_int_t Aec::process(const vsys_float_t **data_in, const vsys_size_t len_in,
//                            vsys_float_t **&data_out, vsys_size_t &len_out){
//    len_out = 0;
//    if(len_in <= 0) return -1;
//    vsys_uint_t total = len_in + buff_len;
//    vsys_uint_t num_frames = total / frame_size;
//    vsys_uint_t num_frames2 = (num_frames * frame_size + temp_len) / frame_size_o;
//    vsys_size_t offset = buff_len, offset2 = 0;
//
//    if(num_frames2 * frame_size > frame_size) {
//        buff_o_size = num_frames2 * frame_size;
//        vsys_reallocate2(&buff_o, audio_format.numChannels, 0, buff_o_size);
//    }
//    if(num_frames){
//        for (vsys_uint_t i = 0; i < audio_format.numChannels; i++) {
//            memcpy(buff_o[i], temp[i], temp_len * sizeof(vsys_float_t));
//        }
//    }
//    for (vsys_uint_t i = 0; i < num_frames; i++) {
//        for (vsys_uint_t j = 0; j < audio_format.numChannels; j++) {
//            memcpy(buff + j * frame_size + offset, data_in[_mic_ids[j]] + i * frame_size - offset2,
//                   (frame_size - offset) * sizeof(vsys_float_t));
//        }
//        for (vsys_uint_t j = 0; j < speaker_num; j++) {
//            memcpy(buff + (audio_format.numChannels + j) * frame_size + offset,
//                   data_in[_speaker_ids[j]] + i * frame_size - offset2, (frame_size - offset) * sizeof(vsys_float_t));
//        }
//        for (vsys_uint_t j = 0; j < (audio_format.numChannels + speaker_num) * frame_size; j++) {
//            buff[j] /= scaling;
//        }
//        r2ssp_aec_buffer_farend(aec_handle, buff + audio_format.numChannels * frame_size, speaker_num * frame_size);
//        r2ssp_aec_process(aec_handle, buff, audio_format.numChannels * frame_size, buff, 0);
//        for (vsys_uint_t j = 0; j < audio_format.numChannels * frame_size; j++) {
//            buff[j] *= scaling;
//        }
////        pcm_out.write((char *)(buff + 4 * frame_size), frame_size * 4);
//        for (vsys_uint_t j = 0; j < audio_format.numChannels; j++) {
//            memcpy(buff_o[j] + i * frame_size + temp_len, buff + j * frame_size, frame_size * sizeof(vsys_float_t));
//        }
//        offset = 0;
//        offset2 = buff_len;
//    }
//    data_out = buff_o;
//    buff_len = total % frame_size;
//    vsys_uint_t length = total > frame_size ? total - num_frames * frame_size : buff_len - offset;
//
//    for (vsys_uint_t i = 0; i < audio_format.numChannels; i++) {
//        memcpy(buff + i * frame_size + offset,
//               data_in[_mic_ids[i]] + (len_in - length), length * sizeof(vsys_float_t));
//    }
//    for (vsys_uint_t i = 0; i < speaker_num; i++) {
//        memcpy(buff + (audio_format.numChannels + i) * frame_size + offset,
//               data_in[_speaker_ids[i]] + (len_in - length), length * sizeof(vsys_float_t));
//    }
//    if(num_frames){
//        len_out = num_frames * frame_size + temp_len;
//        temp_len = len_out % frame_size_o;
//        len_out -= temp_len;
//        for (vsys_uint_t i = 0; i < audio_format.numChannels; i++) {
//            memcpy(temp[i], buff_o[i] + len_out, temp_len * sizeof(vsys_float_t));
//        }
//    }
//    return 0;
//}
//
//Aec::~Aec(){
//    r2ssp_aec_free(aec_handle);
//
//    vsys_free(_mic_ids);
//    vsys_free(_speaker_ids);
//    vsys_free(buff);
//
//    vsys_free2((void **)temp);
//    vsys_free2((void **)buff_o);
//}
    
}
