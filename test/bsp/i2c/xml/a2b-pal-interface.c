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
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "a2b-pal-interface.h"

int32_t adi_a2b_I2C_Open(uint16_t deviceAddr) {
    int32_t handle = -1;

    handle = open(I2C_DEV_PATH, O_RDWR);
    if (handle < 0) {
        perror("Unable to open I2C device " I2C_DEV_PATH);
        return -1;
    }

    /* Set the I2C slave device address */
    if (ioctl(handle, I2C_SLAVE, deviceAddr) < 0) {
        perror("Can't set I2C slave address");
        close(handle); // Close the file descriptor before returning
        return -1;
    }

    /* Set timeout and retry count */
    ioctl(handle, I2C_TIMEOUT, I2C_TIMEOUT_DEFAULT); // Set timeout
    ioctl(handle, I2C_RETRIES, I2C_RETRY_DEFAULT);   // Set retry times

    return handle;
}

void adi_a2b_I2C_Close(int32_t handle) {
    close(handle);
}

int32_t adi_a2b_I2C_Write(void* handle, uint16_t deviceAddr, uint16_t writeLength, uint8_t* writeBuffer) {
    int32_t result = 0;
    int32_t fd = *(int32_t *)handle;

    struct i2c_rdwr_ioctl_data msgRdwr;
    struct i2c_msg msg;

    msgRdwr.msgs = &msg;
    msgRdwr.nmsgs = 1;

    msg.addr  = deviceAddr;
    msg.flags = 0;
    msg.len   = writeLength;
    msg.buf   = writeBuffer;

#if 0
    if ((result = ioctl(fd, I2C_RDWR, &msgRdwr)) < 0) {
        return -1;
    }
#endif

#ifdef A2B_PRINT_CONSOLE
    for (uint16_t i = 0; i < MIN(writeLength - 1, 5); i++) {
        printf(I2C_DEV_PATH " write device(%#x) reg=0x%02X %03d, val=0x%02X (" PRINTF_BINARY_PATTERN_INT8 "), cnt=%d\n",
               deviceAddr, writeBuffer[0] + i, writeBuffer[0] + i, writeBuffer[i + 1], PRINTF_BYTE_TO_BINARY_INT8(writeBuffer[i + 1]), writeLength - 1);
    }
#endif

    return 0;
}

int32_t adi_a2b_I2C_WriteRead(void* handle, uint16_t deviceAddr, uint16_t writeLength, uint8_t* writeBuffer, uint16_t readLength, uint8_t* readBuffer) {
    int32_t result = 0;
    int32_t fd = *(int32_t *)handle;

    struct i2c_rdwr_ioctl_data msgRdwr;
    struct i2c_msg msg[2];

    msgRdwr.msgs = msg;
    msgRdwr.nmsgs = 2;

    msg[0].addr = deviceAddr;
    msg[0].flags = 0;
    msg[0].len = writeLength;
    msg[0].buf = writeBuffer;
    msg[1].addr = deviceAddr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = readLength;
    msg[1].buf = readBuffer;

#if 0
    if ((result = ioctl(fd, I2C_RDWR, &msgRdwr)) < 0) {
        return -1;
    }
#endif

#ifdef A2B_PRINT_CONSOLE
    for (uint16_t i = 0; i < readLength && writeLength == 1; i++) {
        printf(I2C_DEV_PATH "  read device(%#x) reg=0x%02X %03d, val=\033[4m0x%02X\033[0m (" PRINTF_BINARY_PATTERN_INT8 "), cnt=%d\n",
               deviceAddr, writeBuffer[0] + i, writeBuffer[0] + i, readBuffer[i], PRINTF_BYTE_TO_BINARY_INT8(readBuffer[i]), readLength);
    }
#endif

    return 0;
}

char* a2b_pal_File_Read(const char* filename, size_t* outSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(fileSize + 1); // +1 for null terminator
    size_t readSize = fread(buffer, 1, fileSize, file);
    if (readSize != fileSize) {
        free(buffer);
        fclose(file);
        perror("Failed to read file");
        return NULL;
    }

    buffer[readSize] = '\0';
    fclose(file);

    if (outSize) {
        *outSize = readSize;
    }
    return buffer;
}

void adi_a2b_Delay(uint32_t time) {
    printf("sleep 0x%02xms\n", time);
    usleep(time * 1000);
}
