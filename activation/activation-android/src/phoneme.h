//
//  phoneme.h
//  vsys
//
//  Created by 薯条 on 2018/3/1.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef PHONEME_H
#define PHONEME_H

#include <map>
#include <vector>
#include <string>

namespace vsys {
    
class Phoneme{
public:
    Phoneme();
    
    ~Phoneme();
    
    std::string find_phoneme(std::string& head);
    
private:
    void load_phoneme();
    
    bool read_line(char* line);
    
private:
    uint32_t offset;
    
    std::map<std::string, std::string> phoneme_maps;
};
    
}

#endif /* PHONEME_H */
