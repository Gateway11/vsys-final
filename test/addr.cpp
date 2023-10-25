#include <iostream>
#include <map>

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
    //char* addr = addr_map[duckingInfo.usagesHoldingFocus[i].c_str()].c_str();
    return 0;
}
