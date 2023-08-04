#include <stdio.h>
#include <stdint.h>

#define __SWP32(A) (                            \
    (((uint32_t)(A) & 0xff000000) >> 24)    |   \
    (((uint32_t)(A) & 0x00ff0000) >>  8)    |   \
    (((uint32_t)(A) & 0x0000ff00) <<  8)    |   \
    (((uint32_t)(A) & 0x000000ff) << 24))

#define __SWP32_MSB(A, MSB) ((                  \
    (((uint32_t)(A) & 0xff000000) >> 24)    |   \
    (((uint32_t)(A) & 0x00ff0000) >>  8)    |   \
    (((uint32_t)(A) & 0x0000ff00) <<  8)    |   \
    (((uint32_t)(A) & 0x000000ff) << 24)) >> (8 * (4 - (MSB))))

#define __SWP64_MSB2(A, MSB) ((                             \
    (((uint64_t)__SWP32(A & 0x00000000ffffffff)) << 32) |   \
    __SWP32((A & 0xffffffff00000000) >>  32)) >> ((8 - (MSB)) << 3))

#define __SWP64_MSB(A, MSB) ((                          \
    (((uint64_t)(A) & 0xff00000000000000) >> 56)    |   \
    (((uint64_t)(A) & 0x00ff000000000000) >> 40)    |   \
    (((uint64_t)(A) & 0x0000ff0000000000) >> 24)    |   \
    (((uint64_t)(A) & 0x000000ff00000000) >>  8)    |   \
    (((uint64_t)(A) & 0x00000000ff000000) <<  8)    |   \
    (((uint64_t)(A) & 0x0000000000ff0000) << 24)    |   \
    (((uint64_t)(A) & 0x000000000000ff00) << 40)    |   \
    (((uint64_t)(A) & 0x00000000000000ff) << 56)) >> ((8 - (MSB)) << 3))

void my_printf(uint64_t reg, uint8_t reg_val, uint64_t val, uint8_t val_len) {
	uint8_t buf[16] = {0};
	*((uint64_t *)buf) = __SWP64_MSB(reg, reg_val);
	*((uint64_t *)(buf + reg_val)) = __SWP64_MSB(val, val_len);
	
	printf("Hello, World! %#llx, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x\n", 
		   *(uint64_t *)buf, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
}

int main()
{
   	/*  Write C code in this online editor and run it. */
    uint32_t reg = 0x1234/*5678*/, val = 0x9abcdef0, reglen = 2, vallen = 4;

    uint64_t number = ((uint64_t)reg << (vallen << 3)) | val;
    number = __SWP64_MSB(number, reglen + vallen);

    printf("Hello, World! %#llx, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x, %#.2x\n", number,
	        ((uint8_t *)&number)[0], ((uint8_t *)&number)[1],
	        ((uint8_t *)&number)[2], ((uint8_t *)&number)[3],
	        ((uint8_t *)&number)[4], ((uint8_t *)&number)[5],
	        ((uint8_t *)&number)[6], ((uint8_t *)&number)[7]);

    uint32_t number2;
    ((uint8_t *)&number2)[0] = 0x12;
    ((uint8_t *)&number2)[1] = 0x34;
    ((uint8_t *)&number2)[2] = 0x56;
    ((uint8_t *)&number2)[3] = 0x78;

    printf("Hello, World! %d, %#x, %#x, %#x\n", __LINE__,
            number2, __SWP32_MSB(number2, 4), (uint32_t)__SWP64_MSB(number2, 4));

	my_printf(0x12345678, 4, 0x9abcdef0, 4);
	my_printf(0x12, 1, 0x34, 1);
	my_printf(0x12, 1, 0x3456789a, 4);
	my_printf(0x123456789abcdef0, 8, 0x3456789abcdef012, 8);
   return 0;
}
