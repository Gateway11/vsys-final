#include "a2bapp.h"

int main(int argc, char* argv[]) {
    a2b_App_t gApp_Info;
    memset(&gApp_Info, 0, sizeof(gApp_Info));

    uint32_t nResult = 0;
    bool bRunFlag = true;

    gApp_Info.bFrstTimeDisc = A2B_TRUE;
    a2b_setup(&gApp_Info);

    while(1) {
        nResult = a2b_fault_monitor(&gApp_Info);
        if (nResult != 0) {
            bRunFlag = false;
        }

        a2b_stackTick(gApp_Info.ctx);
    }
    return 0;
}
