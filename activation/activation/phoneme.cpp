//
//  phoneme.cpp
//  activation
//
//  Created by 薯条 on 2018/3/1.
//  Copyright © 2018年 薯条. All rights reserved.
//

#include "phones.h"
#include "phoneme.h"

namespace vsys {
    
Phoneme::Phoneme():offset(0){
    load_phoneme();
}

Phoneme::~Phoneme(){
    
}

void Phoneme::load_phoneme(){
    char line[1024];
    
    while(read_line(line)){
        uint32_t left = 0, right = 0;
        bool is_head = false, has_head = false;
        std::string head, content;
        
        while(true){
            switch (line[right]) {
                case ' ':
                    if(!is_head){
                        head.assign(line, right);
                        is_head = true;
                    }else{
                        content.append(line, left, right - left).append(has_head ? "|# " : " ");
                    }
                    has_head = false;
                    left = right + 1;
                    break;
                case '_':
                    has_head = true;
                    content.append(line, left, right - left).append("|");
                    break;
                case '\0':
                    content.append(line, left, right - left).append("|##");
                    goto done;
            }
            right++;
        }
done:
        phoneme_maps.insert(std::make_pair(head, content));
        memset(line, 0, sizeof(line));
    }
}
    
bool Phoneme::read_line(char* line){
    uint32_t i = 0;
    while(phones[offset] != '\0'){
        if(phones[offset] == '\n'){
            offset++;
            return true;
        }
        line[i++] = phones[offset++];
    }
    return false;
}
    
std::string Phoneme::find_phoneme(std::string& head){
    std::string result;
    
    auto it = phoneme_maps.find(head);
    if(it != phoneme_maps.end()){
        result = it->second;
    }
    return result;
}
    
}
