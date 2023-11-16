#include <stdio.h>
#include <math.h>
#include <stdint.h>

int main()
{   
#define PI 3.1415926
#define FREQUENCY 1060
    printf("%#X\n", (int32_t)(cos(PI * 2 * FREQUENCY / 48000) * 0x800000));
    
    for (int32_t i = 1000; i < 20000; i++) {
        if (((int32_t)(cos(PI * 2 * i / 48000) * 0x800000)) == 0x7EC512) {
            printf("---------------------------- %d\n", i);
        }
    }
    return 0;
}
