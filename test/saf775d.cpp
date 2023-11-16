#include <stdio.h>
#include <math.h>
#include <stdint.h>

int32_t main(int argc, char** argv)
{   
#define PI 3.1415926
#define FREQUENCY 1060
    printf("%#X\n", (int32_t)(cos(PI * 2 * FREQUENCY / 48000) * 0x800000));
    
    for (int32_t i = 1000; i < 20000; i++) {
        if (((int32_t)(cos(PI * 2 * i / 48000) * 0x800000)) == 0x7EC512) {
            printf("---------------------------- %d\n", i);
        }
    }
    //printf("%#X\n", 0x1000 - (int32_t)(pow(10, 0 - 12 / 20.0) * 2048));
    int32_t n = atoi(argv[1]);
    if (n >= 0) {
        printf("%#X\n", (int32_t)(pow(10, n / 20.0) * 2048));
    } else {
        printf("%#X\n", 0x1000 - (int32_t)(pow(10, 0 - abs(n) / 20.0) * 2048));
    }

    return 0;
}
