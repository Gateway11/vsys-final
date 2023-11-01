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
    android.hardware.automotive.audiocontrol.V2_0.MutingInfo[] mutingInfos =
        new android.hardware.automotive.audiocontrol.V2_0.MutingInfo[1];
    mutingInfos[0] = new MutingInfo();

    int muteAddrNum = mutingInfos[0].deviceAddressesToMute.length;
    if (carZonesMutingInfo.get(0).deviceAddressesToMute.length < mutingInfos[0].deviceAddressesToMute.length) {
        muteAddrNum = carZonesMutingInfo.get(0).deviceAddressesToMute.length;
    }
    for (int i = 0; i < muteAddrNum; i++) {
        mutingInfos[0].deviceAddressesToMute[i] = carZonesMutingInfo.get(0).deviceAddressesToMute[i];
    }
    for (int i = muteAddrNum; i < mutingInfos[0].deviceAddressesToMute.length; i++) {
        mutingInfos[0].deviceAddressesToMute[i] = "";
    }

    int unMuteAddrNum = mutingInfos[0].deviceAddressesToUnmute.length;
    if (carZonesMutingInfo.get(0).deviceAddressesToUnmute.length < mutingInfos[0].deviceAddressesToUnmute.length) {
        unMuteAddrNum = carZonesMutingInfo.get(0).deviceAddressesToUnmute.length;
    }
    for (int i = 0; i < unMuteAddrNum; i++) {
        mutingInfos[0].deviceAddressesToUnmute[i] = carZonesMutingInfo.get(0).deviceAddressesToUnmute[i];
    }
    for (int i = unMuteAddrNum; i < mutingInfos[0].deviceAddressesToUnmute.length; i++) {
        mutingInfos[0].deviceAddressesToUnmute[i] = "";
    }
#endif
    //char* addr = addr_map[duckingInfo.usagesHoldingFocus[i].c_str()].c_str();
    return 0;
}
