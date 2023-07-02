//
//  aec_wrap.h
//  vsys
//
//  Created by 薯条 on 18/2/5.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef AEC_WRAP_H
#define AEC_WRAP_H

#include "r2ssp.h"

namespace vsys {

//class Aec{
//public:
//    Aec(const audio_format_t& audio_format, vsys_uint_t* mic_ids,
//                        vsys_uint_t aec_num, vsys_uint_t* mic_id_aec, vsys_float_t scaling);
//
//    vsys_int_t process(const vsys_float_t** data_in, const vsys_size_t len_in,
//                        vsys_float_t**& data_out, vsys_size_t& len_out);
//
//    Aec(const Aec& __aec) = delete;
//
//    Aec& operator= (const Aec& __aec) = delete;
//
//    ~Aec();
//
//private:
//    const audio_format_t& audio_format;
//
//    r2ssp_handle aec_handle;
//    vsys_size_t frame_size;
//    vsys_size_t frame_size_o;
//    vsys_float_t scaling;
//
//    vsys_uint_t* _mic_ids;
//    vsys_uint_t speaker_num;
//    vsys_uint_t* _speaker_ids;
//
//    vsys_float_t* buff;
//    vsys_size_t buff_len;
//
//    vsys_float_t** buff_o;
//    vsys_size_t buff_o_size;
//
//    vsys_float_t** temp;
//    vsys_size_t temp_len;
//};
    
}

#endif /* AEC_WRAP_H */
