#include <iostream>
#include <map>
#include <set>
#include <algorithm>

//std::map<std::string, std::string> addr_map {
std::map<std::string, const char*> addr_map {
    {"AUDIO_USAGE_MEDIA",                               "bus0"},
    {"AUDIO_USAGE_VOICE_COMMUNICATION",                 "bus2"},
    {"AUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING",      "bus2"},
    {"AUDIO_USAGE_ALARM",                               "bus6"},
    {"AUDIO_USAGE_NOTIFICATION",                        "bus6"},
    {"AUDIO_USAGE_NOTIFICATION_RINGTONE",               "bus2"},
    {"AUDIO_USAGE_NOTIFICATION_COMMUNICATION_REQUEST",  "bus6"},
    {"AUDIO_USAGE_NOTIFICATION_COMMUNICATION_INSTANT",  "bus6"},
    {"AUDIO_USAGE_NOTIFICATION_COMMUNICATION_DELAYED",  "bus6"},
    {"AUDIO_USAGE_NOTIFICATION_EVENT",                  "bus6"},
    {"AUDIO_USAGE_ASSISTANCE_ACCESSIBILITY",            "bus1"},
    {"AUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE",      "bus3"},
    {"AUDIO_USAGE_ASSISTANCE_SONIFICATION",             "bus6"},
    {"AUDIO_USAGE_GAME",                                "bus0"},
    {"AUDIO_USAGE_ASSISTANT",                           "bus1"},
    /*{"bus0_media_out",                                  "bus0"},
    {"bus1_voice_command_out",                          "bus1"},
    {"bus2_phone_out",                                  "bus2"},
    {"bus3_navi_out",                                   "bus3"},
    {"bus6_notification_out",                           "bus6"},*/
    {"bus0",                                            "bus0"},
    {"bus1",                                            "bus1"},
    {"bus2",                                            "bus2"},
    {"bus3",                                            "bus3"},
    {"bus6",                                            "bus6"}};

auto audio_device_address_is_valid2 = [](uint8_t addr){
    switch(addr) {
    case 0 ... 3:
    case 6:
          return true;
    }
    return false;
};

static inline bool audio_device_address_is_valid(uint8_t addr) {
    switch(addr) {
    case 0 ... 3:
    case 6:
          return true;
    }
    return false;
}

int main()
{
    std::cout << "Hello World" << std::endl;

    std::string usage("AUDIO_USAGE_NOTIFICATION_COMMUNICATION_DELAYED");
    const std::string& addr = addr_map[usage.c_str()];
    if (addr.size()){
        printf("----------%s\n", addr.c_str());
    }
#if 0
    std::map<std::string, const char*> addr_map {
    const char* addr = addr_map[duckingInfo.usagesHoldingFocus[i].c_str()];
    if (addr && !strncmp(addr, "bus", 3)) {
        atoi(addr + 3);
    }
#endif
#if 0
    std::mutex focus_mutex;
    std::thread thread([&] {
            std::lock_guard<decltype(focus_mutex)> lg(focus_mutex);
            audioControl->requestAudioFocus(usage, zoneId, xxxx);
        });
    thread.detach();
#endif
#if 1
    https://stackoverflow.com/questions/75538/hidden-features-of-c
    std::set<std::string> set{"bus0", "bus3", "bus2", "bus0", "bus0"};
    //for(struct { int32_t i; std::set<std::string>::iterator it; } loop = { 0, set.begin() }; loop.i < std::min(set.size(), (size_t)4); loop.i++, loop.it++)
    for(std::pair<int32_t, std::set<std::string>::iterator> loop{0, set.begin()}; loop.first < std::min(set.size(), (size_t)4); loop.first++, loop.second++)
        printf("%s, ", loop.second->c_str());
        //printf("%s, ", loop.it->c_str());

    printf("\n");

    auto it = set.begin();
    for (int i = 0; i < std::min(set.size(), (size_t)4); i++, it++)
        printf("%s, ", it->c_str());
    printf("\n");
    //std::for_each_n(set.begin(), std::min(set.size(), (size_t)4), [](auto& n){
    //    printf("%s, ", n.c_str());
    //});
#endif
    //for(struct { int a; float b; } loop = { 1, 2 }; loop.a != 0; loop.a--) {
    //for(std::pair<int32_t, float> loop = std::make_pair(1, 2.1); loop.first < 5; loop.first++) {
    for(std::pair<int32_t, float> loop{1, 2.1}; loop.first < 5; loop.first++) {
        printf("%d, %f\n", loop.first, loop.second);
    }
    //char* addr = addr_map[duckingInfo.usagesHoldingFocus[i].c_str()].c_str();
    return 0;
}
