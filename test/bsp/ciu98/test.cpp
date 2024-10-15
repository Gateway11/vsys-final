#include <stdio.h>
#include <stdint.h>

uint16_t proto_spi_crc16(uint32_t Length, uint8_t *Data)
{
	uint8_t chBlock = 0;
	uint16_t wCrc = 0;

	wCrc = 0xFFFF; // CRC_A : ITU-V.41 , CRC_B : ISO 3309

	do
	{
		chBlock = *Data++;
		chBlock = (chBlock ^ (uint8_t)(wCrc & 0x00FF));
		chBlock = (chBlock ^ (chBlock << 4));
		wCrc = (wCrc >> 8) ^ ((uint16_t)chBlock << 8) ^ ((uint16_t)chBlock << 3) ^ ((uint16_t)chBlock >> 4);
	} while (--Length);

	wCrc = ~wCrc; // ISO 3309

	return wCrc;
}

int main()
{
    /*  Write C code in this online editor and run it. */
	
	uint8_t data[] = {0x03, 0x00, 0x04, 0xd3, 0x80, 0x8d, 0xc4};
	uint16_t edc_value = proto_spi_crc16(5, data);

    // (((edc_value >> 8) & 0xff) != output[len + FRAME_HEAD_LEN - EDC_LEN + 1]
    // (edc_value & 0xff) != output[len + FRAME_HEAD_LEN - EDC_LEN])
    printf("Hello, World! %d, %#x\n", edc_value, edc_value);
	printf("Hello, World! %#x, %#x\n", (edc_value >> 8) & 0xff, edc_value & 0xff);
   
   return 0;
}
