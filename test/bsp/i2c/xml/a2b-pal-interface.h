/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
 * @file:    a2b-pal-interface.h
 * @brief:   Application header file
 * @version: $Revision$
 * @date:    $Date$
 * Developed by: Automotive Software and Systems team, Bengaluru, India
*****************************************************************************/

#ifndef __A2B_PAL_INTERFACE_H__
#define __A2B_PAL_INTERFACE_H__

#define A2B_PRINT_CONSOLE

#define I2C_DEV_PATH                    "/dev/i2c-16"
#define A2B_BASE_ADDR                   0x68
#define A2B_BUS_ADDR                    0x69

#define I2C_TIMEOUT_DEFAULT             700             /*!< timeout: 700ms*/
#define I2C_RETRY_DEFAULT               3               /*!< retry times: 3 */

#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c %c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
      PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
      PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

void adi_a2b_Delay(uint32_t time);
int32_t adi_a2b_I2C_Open(uint16_t deviceAddr);
void adi_a2b_I2C_Close(int32_t handle);
int32_t adi_a2b_I2C_Write(void* handle, uint16_t deviceAddr, uint16_t writeLength, uint8_t* writeBuffer);
int32_t adi_a2b_I2C_WriteRead(void* handle, uint16_t deviceAddr, uint16_t writeLength, uint8_t* writeBuffer, uint16_t readLength, uint8_t* readBuffer);
char* a2b_pal_File_Read(const char* filename, size_t* outSize);

#endif /* __A2B_PAL_INTERFACE_H__ */
