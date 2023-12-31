#include <stdio.h>
#include <android/hardware/automotive/audiocontrol/2.0/IAudioControl.h>

using namespace android;
using namespace android::hardware::automotive::audiocontrol::V2_0;

using android::hardware::audio::V4_0::IPrimaryDevice;

//https://blog.csdn.net/sevenjoin/article/details/107619014
//https://blog.csdn.net/qq_40731414/article/details/126823262
//https://blog.csdn.net/IBMQUSTZJ/article/details/80722590      tinyxml
//out/soong/.intermediates/hardware/interfaces/automotive/audiocontrol/2.0
sp<IAudioControl> audiocontrol = IAudioControl::getService();

sp<IPrimaryDevice> mPrimaryDevice = IPrimaryDevice::getService();

int32_t main(int argc, char** argv) {

    printf("%p, args=%s\n", audiocontrol.get(), argv[1]);

    if(strcmp(argv[1], "geq") == 0) {
        AudioParamConfig config;
        config.command = Audio_Command_Type::GEQ;
        config.equalizer.type = Audio_Geq_Type::EFFECT_TYPE;
        //system/libhidl/base/include/hidl/HidlSupport.h
        config.equalizer.eq_effect_type = hardware::hidl_array<uint8_t, 16>({'P', 'e', 'r', 'c', 'u', 's', 's', 'i', 'o', 'n'});
        config.equalizer.frequency_index = 3;
        config.values = 2;
        config.extra_data[0] = 1;
        config.extra_data[1] = 3;
        config.extra_datalen = 2;

        printf("%s\n", toString(config).c_str());
        audiocontrol->setAudioParamConfig(config);
    }
    mPrimaryDevice->setParameters({}, {{"----key----", "----value----"}});
    return 0;
}
