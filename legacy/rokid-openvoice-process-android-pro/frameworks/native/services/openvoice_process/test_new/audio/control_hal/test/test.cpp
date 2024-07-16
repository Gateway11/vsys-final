#include <stdio.h>

#include <android/hardware/automotive/audiocontrol/2.0/IAudioControl.h>
#include <android/hardware/audio/4.0/IPrimaryDevice.h>

using namespace android;
using namespace android::hardware;
using namespace android::hardware::automotive::audiocontrol::V2_0;

using android::hardware::audio::common::V6_0::AudioUsage;

sp<IAudioControl> audiocontrol = IAudioControl::getService();

//using ::android::hardware::audio::V4_0::IPrimaryDevice;
//using android::hardware::audio::V4_0::ParameterValue;
//sp<IPrimaryDevice> device = IPrimaryDevice::getService();

int32_t main(int argc, char** argv) {

    if (argc < 2) {
        printf("---------------------------- %s/%s ----------------------------\n", __DATE__, __TIME__);
        printf("adb shell audiocontrol_test geq\n");
        printf("adb shell audiocontrol_test mute 2 bus0 bus1 0\n");
        printf("adb shell audiocontrol_test duck 1 bus0 1 bus3 2 bus0 bus3\n");
        printf("adb shell audiocontrol_test req 1001/1000(safety/emergency) 1/2/3/4\n");
        printf("adb shell audiocontrol_test abn 1001/1000\n");
        return 0;
    }
    printf("%p, args=%s\n", audiocontrol.get(), argv[1]);

    if (strcmp(argv[1], "geq") == 0) {
        AudioParamConfig config;
        config.command = AudioCommandType::GEQ;
        config.equalizer.type = AudioGeqType::EFFECT_TYPE;
        config.equalizer.eqEffectType = "Percussion";
        config.equalizer.frequencyIdx = 3;
        config.values = 2;
        config.extraData[0] = 1;
        config.extraData[1] = 3;
        config.extraDataLen = 2;

        printf("%s\n", toString(config).c_str());
        audiocontrol->setAudioParamConfig(config);
    } else if (strcmp(argv[1], "req") == 0) {
        if (argc < 4) goto param_err;
        audiocontrol->requestAudioFocus(
                      static_cast<hidl_bitfield<AudioUsage>>(atoi(argv[2])),
                      0,
                      static_cast<hidl_bitfield<AudioFocusChange>>(atoi(argv[3])));
    } else if (strcmp(argv[1], "abn") == 0) {
        if (argc < 3) goto param_err;
        audiocontrol->abandonAudioFocus(static_cast<hidl_bitfield<AudioUsage>>(atoi(argv[2])), 0);
    } else if (strcmp(argv[1], "mute") == 0) {
        MutingInfo mutingInfo;
        mutingInfo.zoneId = 0;
        //audiocontrol_test mute 2 0 1 0
        //audiocontrol_test mute 2 0 1 1 1
        //audiocontrol_test mute 0 1 1
        switch(atoi(argv[2])) {
        case 4:
            mutingInfo.deviceAddressesToMute[3] = argv[6];
        case 3:
            mutingInfo.deviceAddressesToMute[2] = argv[5];
        case 2:
            mutingInfo.deviceAddressesToMute[1] = argv[4];
        case 1:
            mutingInfo.deviceAddressesToMute[0] = argv[3];
        }
        switch(atoi(argv[3 + atoi(argv[2])])) {
        case 4:
            mutingInfo.deviceAddressesToUnmute[3] = argv[7 + atoi(argv[2])];
        case 3:
            mutingInfo.deviceAddressesToUnmute[2] = argv[6 + atoi(argv[2])];
        case 2:
            mutingInfo.deviceAddressesToUnmute[1] = argv[5 + atoi(argv[2])];
        case 1:
            mutingInfo.deviceAddressesToUnmute[0] = argv[4 + atoi(argv[2])];
        }
        //mutingInfo.deviceAddressesToMute = hidl_array<hidl_string, 6>({"bus0"});
        hidl_array<MutingInfo, 1> mutingInfos;
        mutingInfos[0] = mutingInfo;
        audiocontrol->onDevicesToMuteChange(mutingInfos);
    } else if (strcmp(argv[1], "duck") == 0) {
        DuckingInfo duckingInfo;
        duckingInfo.zoneId = 0;

        switch(atoi(argv[2])) {
        case 4:
            duckingInfo.deviceAddressesToDuck[3] = argv[6];
        case 3:
            duckingInfo.deviceAddressesToDuck[2] = argv[5];
        case 2:
            duckingInfo.deviceAddressesToDuck[1] = argv[4];
        case 1:
            duckingInfo.deviceAddressesToDuck[0] = argv[3];
        }
        switch(atoi(argv[3 + atoi(argv[2])])) {
        case 4:
            duckingInfo.deviceAddressesToUnduck[3] = argv[7 + atoi(argv[2])];
        case 3:
            duckingInfo.deviceAddressesToUnduck[2] = argv[6 + atoi(argv[2])];
        case 2:
            duckingInfo.deviceAddressesToUnduck[1] = argv[5 + atoi(argv[2])];
        case 1:
            duckingInfo.deviceAddressesToUnduck[0] = argv[4 + atoi(argv[2])];
        }
        //audiocontrol_test mute 0 0 1 1
        //audiocontrol_test mute 0 1 1 1 1
        //audiocontrol_test mute 3 3 2 1 1 1 1 1
        uint8_t focus_bus_index = 4 + atoi(argv[2]) + atoi(argv[3 + atoi(argv[2])]);
        switch(atoi(argv[focus_bus_index])) {
        case 4:
            duckingInfo.usagesHoldingFocus[3] = argv[4 + focus_bus_index];
        case 3:
            duckingInfo.usagesHoldingFocus[2] = argv[3 + focus_bus_index];
        case 2:
            duckingInfo.usagesHoldingFocus[1] = argv[2 + focus_bus_index];
        case 1:
            duckingInfo.usagesHoldingFocus[0] = argv[1 + focus_bus_index];
        }
        hidl_array<DuckingInfo, 1> duckingInfos;
        duckingInfos[0] = duckingInfo;
        audiocontrol->onDevicesToDuckChange(duckingInfos);
    }
    //ParameterValue param = {"----key----", "----value----"};
    //hardware::hidl_vec<ParameterValue> params = {param};
    //device->setParameters({}, params);
    return 0;

param_err:
    printf("param error\n");
    return 0;
}
