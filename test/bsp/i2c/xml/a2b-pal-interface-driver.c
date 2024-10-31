/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
 * @file:    a2b-pal-interface.c
 * @brief:   This file handles PAL implementations.
 * @version: $Revision$
 * @date:    $Date$
 * Developed by: Automotive Software and Systems team, Bengaluru, India
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "a2b-pal-interface.h"

int32_t adi_a2b_I2C_Open(uint16_t deviceAddress) {
    // Nothing to do
    return 0;
}

void adi_a2b_I2C_Close(int32_t handle) {
    // Nothing to do
}

int32_t adi_a2b_I2C_Write(void* handle, uint16_t deviceAddress, uint16_t writeLength, uint8_t* writeBuffer) {
    int32_t result = 0;
    int32_t fd = *(int32_t *)handle;

    struct i2c_msg msg;

    msg.addr  = deviceAddress;
    msg.flags = 0;
    msg.len   = writeLength;
    msg.buf   = writeBuffer;

#if 0
    result = i2c_transfer(ad2428_global_data->client->adapter, msg, ARRAY_SIZE(msg));
    if (result < 0) {
        return -1;
    }
#endif

#ifdef A2B_PRINT_CONSOLE
    for (uint8_t i = 0; i < writeLength - 1; i++) {
        pr_info(I2C_DEV_PATH " write device(%#x) reg=0x%02x %03d val=0x%02x (" PRINTF_BINARY_PATTERN_INT8 ") cnt=%d\n",
               deviceAddress, writeBuffer[0] + i, writeBuffer[0] + i, writeBuffer[i + 1], PRINTF_BYTE_TO_BINARY_INT8(writeBuffer[i + 1]), writeLength - 1);
    }
#endif

    return result;
}

int32_t adi_a2b_I2C_WriteRead(void* handle, uint16_t deviceAddress, uint16_t writeLength, uint8_t* writeBuffer, uint16_t readLength, uint8_t* readBuffer) {
    int32_t result = 0;

    msg[0].addr = deviceAddress;
    msg[0].flags = 0;
    msg[0].len = writeLength;
    msg[0].buf = writeBuffer;

    msg[1].addr = deviceAddress;
    msg[1].flags = I2C_M_RD;
    msg[1].len = readLength;
    msg[1].buf = readBuffer;

#if 0
    result = i2c_transfer(ad2428_global_data->client->adapter, msg, ARRAY_SIZE(msg));
    if (result < 0) {
        return -1;
    }
#endif

#ifdef A2B_PRINT_CONSOLE
    for (uint8_t i = 0; i < readLength; i++) {
        pr_info(I2C_DEV_PATH " read  device(%#x) reg=0x%02x %03d val=0x%02x (" PRINTF_BINARY_PATTERN_INT8 ") cnt=%d\n",
               deviceAddress, writeBuffer[0] + i, writeBuffer[0] + i, readBuffer[i], PRINTF_BYTE_TO_BINARY_INT8(readBuffer[i]), readLength);
    }
#endif

    return result;
}

char* a2b_pal_File_Read(const char* filename, size_t* outSize) {
    return buffer;
}
