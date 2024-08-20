#include <iostream>
#include <thread>

int main()
{
    std::cout << "Hello World" << std::endl;
    static std::once_flag flag;
    std::call_once(flag, [&]{ std::thread thread([&]{
                int32_t timer = 5;
                while (--timer) {
                    //AHAL_DBG("################## timer = %d", timer);
                    printf("---------------------\n");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                //str_parms_add_int(params, "handle_dpin", AUDIO_DEVICE_IN_AUX_DIGITAL | AUDIO_DEVICE_OUT_USB_HEADSET);
                //AudioExtn::audio_extn_set_parameters(adev_, parms);
                }); thread.detach(); }); 

    int32_t timer2 = 5;
    while (--timer2) {
        printf("--ddddd-------------------\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
