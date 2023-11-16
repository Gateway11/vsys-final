#include <stdio.h>
#include <math.h>
#include <stdint.h>

int32_t main(int argc, char** argv)
{   
#define PI 3.1415926
#define FREQUENCY 1060
#if 0
    printf("%#X\n", (int32_t)(cos(PI * 2 * FREQUENCY / 48000) * 0x800000));
    printf("%#X\n", 0x1000 - (int32_t)(pow(10, 0 - 12 / 20.0) * 2048));
    
    for (int32_t i = 1000; i < 20000; i++) {
        if (((int32_t)(cos(PI * 2 * i / 48000) * 0x800000)) == 0x7EC512) {
            printf("---------------------------- %d\n", i);
        }
    }
#endif
    int32_t n = atoi(argv[1]);
    if (n >= 0) {
        printf("cos(2πf/fs) * 0x800000  %#X\n", (int32_t)(cos(PI * 2 * n / 48000) * 0x800000));
    } else {
        printf("10^(vol / 20)           %#X\n", 0x1000 - (int32_t)(pow(10, 0 - abs(n) / 20.0) * 2048));
    }

    return 0;
}
