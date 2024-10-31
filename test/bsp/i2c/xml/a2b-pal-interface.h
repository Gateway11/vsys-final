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

#define I2C_DEV_PATH                    "/dev/i2c-13"
#define I2C_MASTER_ADDR                 0x68
#define I2C_SLAVE_ADDR                  0x69

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

int32_t arrayAddrs[2];

int32_t adi_a2b_I2COpen(uint16_t nDeviceAddress);
int32_t adi_a2b_I2CWrite(uint16_t nDeviceAddress, uint16_t nWrite, uint8_t* wBuf);
int32_t adi_a2b_I2CWriteRead(uint16_t nDeviceAddress, uint16_t nWrite, uint8_t* wBuf, uint16_t nRead, uint8_t* rBuf);

#endif /* __A2B_PAL_INTERFACE_H__ */
