//
//  audio_tools.hpp
//  audio_tools
//
//  Created by 薯条 on 2023/3/31.
//

#ifndef AUDIO_TOOLS_HPP
#define AUDIO_TOOLS_HPP

#if 1
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>

#define FILE_NAME_SIZE 128
typedef struct output_desc {
    int output;
    char path[FILE_NAME_SIZE];
    void *next;
} output_desc_t;

const char* path = "~/Desktop/audio_tools";
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static output_desc* outputs = NULL;
void easy_write(char* tag, uint32_t session, uint32_t pid, uint32_t device, uint32_t sample_rate,
           uint32_t num_channels, uint32_t bits, char* data, size_t size, bool close) {
    pthread_mutex_lock(&mutex);
    output_desc_t* current_output = NULL;
    char str[FILE_NAME_SIZE];
    memset(str, 0, FILE_NAME_SIZE);
    
    sprintf(str, "%s/%s.session%d.pid%d.%#x_%d_%d_%dbit.pcm",
            path, tag, session, pid, device, sample_rate, num_channels, bits);
    
    current_output = outputs;
    while(current_output != NULL){
        if(!strcmp(current_output->path, str)) {
            break;
        }
        current_output = (output_desc_t *)current_output->next;
    }
    if(current_output == NULL || access(str, F_OK) < 0) {
        if(current_output == NULL) {
            current_output = (output_desc *)malloc(sizeof(output_desc_t));
            memcpy(current_output->path, str, FILE_NAME_SIZE);
            if(outputs == NULL) {
                outputs = current_output;
            } else {
                output_desc_t* last_output = outputs;
                while(last_output->next != NULL){
                    last_output = (output_desc_t *)last_output->next;
                }
                last_output->next = current_output;
            }
            current_output->next = NULL;
        }
        current_output->output = open(str, O_WRONLY|O_CREAT|O_APPEND, S_IWUSR|S_IRGRP|S_IROTH);
    }
    write(current_output->output, data, size);
    pthread_mutex_unlock(&mutex);
}
#endif
#if 0

#include <map>
#include <mutex>
#include <fstream>
//__BEGIN_CDECLS
//__END_CDECLS

#ifdef __cplusplus
extern "C"{
#endif
 // code ......

#ifdef __cplusplus
}
#endif

static std::map<std::string, std::ofstream> outputs;
static std::mutex m_write;

const char* path = "/data/debuglogger/audio_dump";
const char* path2 = "/Users/daixiang/Desktop/audio_tools";

void easy_write(std::string tag, uint32_t session, uint32_t pid, uint32_t device, uint32_t sample_rate,
           uint32_t num_channels, uint32_t bits, char* data, size_t size, bool close) {
    
    std::lock_guard<std::mutex> lg(m_write);
    std::ofstream output_stream;
    char str[128];
    
    sprintf(str, "%s/%s.session%d.pid%d.%#x_%d_%d_%dbit.pcm",
            path2, tag.c_str(), session, pid, device, sample_rate, num_channels, bits);
    
    auto iter = outputs.find(str);
    if(iter == outputs.end() || iter->second.bad()) {
        output_stream = std::ofstream(str, std::ios::out | std::ios::binary);
    } else {
        output_stream = std::move(iter->second);
    }
    printf("%s \t\t     %llu      %p\n", str, output_stream.tellp() / size, data);
    
    output_stream.write((char *)data, size);
    //output_stream.flush();
    
    outputs[str] = std::move(output_stream);
}
#endif

#endif /* AUDIO_TOOLS_HPP */
