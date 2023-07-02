//
//  buffer.h
//  vsys
//
//  Created by 薯条 on 2018/2/7.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <memory>

namespace vsys {
    
template <typename T>
class Buffer{
public:
    Buffer() : size_(0), capacity_(0), data_(nullptr) {}
    
    Buffer(const Buffer& buf){
        
    }
    
    Buffer& operator=(const Buffer& buf){
        return *this;
    }
    
    Buffer(const Buffer&& buf){
        
    }

    T* data() const {
        return data_.get();
    }
    
    void append(const Buffer& buf){
        
    }
    
    bool empty() const {
        return size_ == 0;
    }
    
    size_t size() const {
        return size_;
    }
    
    size_t capacity() const {
        return capacity_;
    }
    
    void clear(){
        size_ = 0;
    }
    
    Buffer& operator=(const T*& buf){
        return *this;
    }
    
    Buffer& operator+=(const Buffer& buf){
        return *this;
    }
    
private:
    bool check() const{
        return false;
    }
    
private:
    size_t size_;
    size_t capacity_;
    std::unique_ptr<T[]> data_;
};
    
}

#endif /* BUFFER_H */
