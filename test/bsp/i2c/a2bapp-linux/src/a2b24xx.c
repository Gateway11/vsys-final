#include "a2bapp.h"

int main(int argc, char* argv[]) {


    a2b_App_t gApp_Info;
    memset(&gApp_Info, 0, sizeof(gApp_Info));

    a2b_setup(&gApp_Info);
    return 0;
}
