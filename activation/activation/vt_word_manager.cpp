//
//  vt_word_manager.cpp
//  vsys
//
//  Created by 薯条 on 2018/1/19.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include <cctype>

#include "debug.h"
#include "vt_word_manager.h"

namespace vsys {
    
VtWordManager::VtWordManager(void* _token,
                             int32_t (*_sync)(void* token, const WordInfo* word_info, const uint32_t& word_num), AcousticModel _model)
    :token(_token), sync(_sync), phoneme(std::make_shared<Phoneme>()), vt_model(_model), vt_words(nullptr){
}
    
VtWordManager::~VtWordManager(){
    delete[] vt_words;
}
    
int32_t VtWordManager::add_vt_word(const vt_word_t* vt_word){
    if(!is_valid_vt_type(vt_word->type)){
        VSYS_DEBUGE("Unknown vt type %d", vt_word->type);
        return -1;
    }
    if(has_vt_word(vt_word->word_utf8)){
        VSYS_DEBUGI("%s already exists", vt_word->word_utf8);
        return -1;
    }
    if(!save_vt_word(vt_word)) return -1;
    return sync_vt_word();
}

int32_t VtWordManager::remove_vt_word(const std::string& word){
    bool find = false;
    {
        std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);

        std::vector<WordInfo>::iterator it =  word_infos.begin();
        while (it != word_infos.end()) {
            if(word.compare(it->pWordContent_UTF8) == 0){
                word_infos.erase(it);
                find = true;
                break;
            }
        }
    }
    if(!find) {
        VSYS_DEBUGE("%s is not exists", word.c_str());
        return -1;
    }
    return sync_vt_word();
}

int32_t VtWordManager::get_vt_words(vt_word_t*& vt_words_out){
    delete[] vt_words;
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
    
    uint32_t word_num = word_infos.size();
    vt_words = new vt_word_t[word_num];
    
    for (uint32_t i = 0; i < word_num; i++) {
        vt_words[i].type = get_vt_type(word_infos[i].iWordType);
        strcpy(vt_words[i].word_utf8, word_infos[i].pWordContent_UTF8);
        strcpy(vt_words[i].phone, word_infos[i].pWordContent_PHONE);
        vt_words[i].block_avg_score = word_infos[i].fBlockAvgScore;
        vt_words[i].block_min_score = word_infos[i].fBlockMinScore;
        vt_words[i].left_sil_det = word_infos[i].bLeftSilDet;
        vt_words[i].right_sil_det = word_infos[i].bRightSilDet;
        vt_words[i].remote_asr_check_with_aec = word_infos[i].bRemoteAsrCheckWithAec;
        vt_words[i].remote_asr_check_without_aec = word_infos[i].bRemoteAsrCheckWithNoAec;
        if(word_infos[i].bLocalClassifyCheck){
            vt_words[i].mask |= VT_WORD_LOCAL_CLASSIFY_CHECK_MASK;
        }
        vt_words[i].classify_shield = word_infos[i].fClassifyShield;
        strcpy(vt_words[i].nnet_path, word_infos[i].pLocalClassifyNnetPath);
    }
    vt_words_out = vt_words;
    return 0;
}
    
bool VtWordManager::is_valid_vt_type(word_type type){
    switch (type) {
        case VSYS_WORD_AWAKE:
        case VSYS_WORD_SLEEP:
        case VSYS_WORD_HOTWORD:
            return true;
    }
    return false;
}
    
bool VtWordManager::has_vt_word(const std::string &word){
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);

    uint32_t word_num = word_infos.size();
    for (uint32_t i = 0; i < word_num; i++) {
        if(word.compare(word_infos[i].pWordContent_UTF8) == 0){
            return true;
        }
    }
    return false;
}
    
bool VtWordManager::vt_word_formation(const vt_word_t* vt_word, WordInfo& word_info){
    std::string phone;
    uint32_t word_size = get_word_size(vt_word->phone, vt_word->mask & VT_WORD_USE_OUTSIDE_PHONE_MASK);
    
    if((vt_word->mask & VT_WORD_USE_OUTSIDE_PHONE_MASK) == 0){
        if(!pinyin2phoneme(vt_word->phone, phone)){
            return false;
        }
    }else{
        phone = vt_word->phone;
    }
    
    float block_avg_score = (vt_word->mask & VT_WORD_BLOCK_AVG_SCORE_MASK) != 0
    ? vt_word->block_avg_score : get_score_param(4.2f, word_size);
    
    float block_min_score = (vt_word->mask & VT_WORD_BLOCK_MIN_SCROE_MASK) != 0
    ? vt_word->block_min_score : get_score_param(2.7f, word_size);
    
    if(block_avg_score < 3.2f) block_avg_score = 3.2f;
    if(block_min_score < 1.7f) block_min_score = 1.7f;
    
    bool left_sil_det = (vt_word->mask & VT_WORD_LEFT_SIL_DET_MASK) != 0
    ? vt_word->left_sil_det : true;
    
    bool right_sil_det = (vt_word->mask & VT_WORD_RIGHT_SIL_DET_MASK) != 0
    ? vt_word->right_sil_det : false;
    
    bool remote_asr_check_with_aec = (vt_word->mask & VT_WORD_REMOTE_CHECK_WITH_AEC_MASK) != 0
    ? vt_word->remote_asr_check_with_aec : true;
    
    bool remote_asr_check_without_aec = (vt_word->mask & VT_WORD_REMOTE_CHECK_WITH_NOAEC_MASK) != 0
    ? vt_word->remote_asr_check_without_aec : true;
    
    bool local_classify_check = (vt_word->mask & VT_WORD_LOCAL_CLASSIFY_CHECK_MASK) != 0 ? true : false;
    
    float classify_shield = (vt_word->mask & VT_WORD_CLASSIFY_SHIELD_MASK) != 0
    ? vt_word->classify_shield : -0.3;
    
    word_info.iWordType = get_vt_type(vt_word->type);
    strcpy(word_info.pWordContent_UTF8, vt_word->word_utf8);
    strcpy(word_info.pWordContent_PHONE, phone.c_str());
    word_info.fBlockAvgScore = block_avg_score;
    word_info.fBlockMinScore = block_min_score;
    word_info.bLeftSilDet = left_sil_det;
    word_info.bRightSilDet = right_sil_det;
    word_info.bRemoteAsrCheckWithAec = remote_asr_check_with_aec;
    word_info.bRemoteAsrCheckWithNoAec = remote_asr_check_without_aec;
    word_info.bLocalClassifyCheck = local_classify_check;
    word_info.fClassifyShield = classify_shield;
    strcpy(word_info.pLocalClassifyNnetPath, vt_word->nnet_path);
    return true;
}

bool VtWordManager::pinyin2phoneme(const std::string &pinyin, std::string &phone){
    std::string result;
    if(vt_model == AcousticModel::MODEL_DNN){
        uint32_t left = 0, right = 0;
        std::string target;
        bool is_first = true;
        
        uint32_t length = pinyin.length();
        while (right < length) {
            if(!std::isalnum(pinyin[right])){
                VSYS_DEBUGE("contains bad pinyin : %s", pinyin.c_str());
                return false;
            }
            if (std::isdigit(pinyin[right])){
                target.assign(pinyin, left, right - left + 1);
                std::string phone = phoneme->find_phoneme(target);
                if(phone.empty()){
                    VSYS_DEBUGE("cannot find phoneme for %s", target.c_str());
                    return false;
                }
                if(!is_first){
                    result.append(" ");
                }
                result.append(phone);
                is_first = false;
                left = right + 1;
            }
            right++;
        }
    }else if(vt_model == AcousticModel::MODEL_CTC){
        result = pinyin;
        for (uint32_t i = 0; i < pinyin.length(); i++) {
            if(std::isdigit(result[i])){
                result[i] = 32;
            }
        }
    }else{
        VSYS_DEBUGE("unknown model");
    }
    phone.assign(result);
    return true;
}
    
uint32_t VtWordManager::get_word_size(const std::string& phone, bool is_pinyin){
    uint32_t length = phone.length(), word_size = 0;
    for (uint32_t i = 0; i < length; i++) {
        if(!is_pinyin && std::isdigit(phone[i])){
            word_size++;
        }
        if(phone[i] == '#' && phone[i + 1] == '#'){
            word_size++;
        }
    }
    return word_size;
}
    
float VtWordManager::get_score_param(float score, uint32_t word_size){
    int32_t iter = (word_size + 1) / 2 - 1;
    for(int i = 0; i < iter; i++) {
        score -= 0.5f;
    }
    return score;
}

WordType VtWordManager::get_vt_type(word_type type){
    switch (type) {
        case VSYS_WORD_AWAKE:
            return WORD_AWAKE;
        case VSYS_WORD_SLEEP:
            return WORD_SLEEP;
        case VSYS_WORD_HOTWORD:
            return WORD_HOTWORD;
    }
    return WORD_OTHER;
}
    
word_type VtWordManager::get_vt_type(WordType type){
    switch (type) {
        case WORD_AWAKE:
            return VSYS_WORD_AWAKE;
        case WORD_SLEEP:
            return VSYS_WORD_SLEEP;
        case WORD_HOTWORD:
            return VSYS_WORD_HOTWORD;
    }
    return VSYS_WORD_OTHER;
}
    
int32_t VtWordManager::sync_vt_word(){
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
    
    uint32_t word_num = word_infos.size();
    WordInfo* word_info_copy = new WordInfo[word_num];
    
    for (uint32_t i = 0; i < word_num; i++) {
        word_info_copy[i].iWordType = word_infos[i].iWordType;
        strcpy(word_info_copy[i].pWordContent_UTF8, word_infos[i].pWordContent_UTF8);
        strcpy(word_info_copy[i].pWordContent_PHONE, word_infos[i].pWordContent_PHONE);
        word_info_copy[i].fBlockAvgScore = word_infos[i].fBlockAvgScore;
        word_info_copy[i].fBlockMinScore = word_infos[i].fBlockMinScore;
        word_info_copy[i].bLeftSilDet = word_infos[i].bLeftSilDet;
        word_info_copy[i].bRightSilDet = word_infos[i].bRightSilDet;
        word_info_copy[i].bRemoteAsrCheckWithAec = word_infos[i].bRemoteAsrCheckWithAec;
        word_info_copy[i].bRemoteAsrCheckWithNoAec = word_infos[i].bRemoteAsrCheckWithNoAec;
        word_info_copy[i].bLocalClassifyCheck = word_infos[i].bLocalClassifyCheck;
        word_info_copy[i].fClassifyShield = word_infos[i].fClassifyShield;
        strcpy(word_info_copy[i].pLocalClassifyNnetPath, word_infos[i].pLocalClassifyNnetPath);
    }
    int ret = sync(token, word_info_copy, word_num);;
    
    delete[] word_info_copy;
    return ret == 0 ? 0 : -1;
}
    
bool VtWordManager::save_vt_word(const vt_word_t* vt_word){
    WordInfo word_info;
    if(!vt_word_formation(vt_word, word_info)){
        return false;
    }
    std::lock_guard<decltype(vt_mutex)> locker(vt_mutex);
    word_infos.push_back(word_info);
    return true;
}
    
}
