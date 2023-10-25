#include <iostream>
#include <map>

std::map<std::string, std::string> addr_map {
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
    {"bus0",                                            "bus0"},
    {"bus1",                                            "bus1"},
    {"bus2",                                            "bus2"},
    {"bus3",                                            "bus3"},
    {"bus6",                                            "bus6"}};

int main()
{
    std::cout << "Hello World";

    std::string usage = "AUDIO_USAGE_NOTIFICATION_COMMUNICATION_DELAYED";
    const std::string& addr = addr_map[usage.c_str()];
    if (addr.size()){
        printf("----------%s", addr.c_str());
    }   
    return 0;
}
