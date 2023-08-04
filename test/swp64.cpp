#include <stdio.h>
#include <stdint.h>

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
	
	printf("Hello, World! %#llx, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x\n", 
		   *(uint64_t *)buf, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
}

int main()
{
   	/*  Write C code in this online editor and run it. */
	my_printf(0x12345678, 4, 0x9abcdef0, 4);
	my_printf(0x12, 1, 0x34, 1);
	my_printf(0x12, 1, 0x3456789a, 4);
	my_printf(0x123456789abcdef0, 8, 0x3456789abcdef012, 8);
   return 0;
}
