#include <stdio.h>
#include <android/hardware/automotive/audiocontrol/2.0/IAudioControl.h>

using namespace std;
using namespace android;
using ::android::hardware::automotive::audiocontrol::V2_0::IAudioControl;
using ::android::hardware::automotive::audiocontrol::V2_0::AudioParamConfig;
using ::android::hardware::automotive::audiocontrol::V2_0::Audio_Command_Type;
using ::android::hardware::automotive::audiocontrol::V2_0::Audio_Geq_Type;

sp<IAudioControl> audiocontrol = IAudioControl::getService();

int32_t main(int argc, char** argv) {

    printf("%p, args=%s\n", audiocontrol.get(), argv[1]);

    if(strcmp(argv[1], "geq") == 0) {
        AudioParamConfig config;
        config.command = Audio_Command_Type::GEQ;
        config.equalizer.type = Audio_Geq_Type::EFFECT_TYPE;
        config.equalizer.eq_effect_type = hardware::hidl_array<uint8_t, 16>({'P', 'e', 'r', 'c', 'u', 's', 's', 'i', 'o', 'n'});
        config.equalizer.frequency_index = 3;
        config.values = 2;
        config.extra_data[0] = 1;
        config.extra_data[1] = 3;
        config.extra_datalen = 2;

        printf("%s\n", toString(config).c_str());
        audiocontrol->setAudioParamConfig(config);
    }
    return 0;
}
