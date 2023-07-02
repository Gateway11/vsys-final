
//
//  event_hub.h
//  vsys
//
//  Created by 薯条 on 2018/1/30.
//  Copyright © 2018年 薯条. All rights reserved.
//

#ifndef EVENT_HUB_H
#define EVENT_HUB_H

#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "vsys_activation.h"

namespace vsys {

class EventHub{
public:
    EventHub();
    
    ~EventHub();
    
    void add_callback(voice_event_callback callback, void* token);
    
    void send_voice_event(voice_event_t* voice_event);
    
private:
    void thread_loop();
    
private:
    std::list<voice_event_t*> voice_events;
    std::mutex mutex;
    std::thread thread;
    std::condition_variable condition;
    
    void* token;
    voice_event_callback event_callback;
    
    bool thread_exit;
};
    
}

#endif /* EVENT_HUB_H */
