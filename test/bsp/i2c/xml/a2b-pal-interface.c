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
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "a2b-pal-interface.h"

int32_t adi_a2b_I2COpen(uint16_t nDeviceAddress)
{
    int32_t fd;

#if 0
    fd = open(I2C_DEV_PATH, O_RDWR);
    if (fd < 0) {
        printf("Unable to open I2C device: %s\n", strerror(errno));
        return -1;
    }

    /* Set the I2C slave device address */
    if (ioctl(fd, I2C_SLAVE, nDeviceAddress) < 0) {
        printf("Can't set I2C slave address: %s\n", strerror(errno));
        close(fd); // Close the file descriptor before returning
        return -1;
    }

    /* Set timeout and retry count */
    ioctl(fd, I2C_TIMEOUT, I2C_TIMEOUT_DEFAULT); // Set timeout
    ioctl(fd, I2C_RETRIES, I2C_RETRY_DEFAULT);   // Set retry times
#endif

    return fd;
}

int32_t adi_a2b_I2CWrite(uint16_t nDeviceAddress, uint16_t nWrite, uint8_t* wBuf)
{
    int32_t nResult = 0, fd;

    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg msg;

    fd = nDeviceAddress == I2C_MASTER_ADDR ? arrayAddrs[0] : arrayAddrs[1];

    msg_rdwr.msgs = &msg;
    msg_rdwr.nmsgs = 1;

    msg.addr  = nDeviceAddress;
    msg.flags = 0;
    msg.len   = nWrite;
    msg.buf = wBuf;

#if 0
    nResult = ioctl(fd, I2C_RDWR, &msg_rdwr);
    if (nResult < 0) {
        return -1;
    }
#endif

#ifdef A2B_PRINT_CONSOLE
    for (uint8_t i = 0; i < nWrite - 1; i++) {
        printf(I2C_DEV_PATH" write device(%#x) reg=0x%02x %03d val=0x%02x ("PRINTF_BINARY_PATTERN_INT8") cnt=%d\n",
                nDeviceAddress, wBuf[0] + i, wBuf[0] + i, wBuf[i + 1], PRINTF_BYTE_TO_BINARY_INT8(wBuf[i + 1]), nWrite - 1);
    }
#endif

    return nResult;
}

int32_t adi_a2b_I2CWriteRead(uint16_t nDeviceAddress, uint16_t nWrite, uint8_t* wBuf, uint16_t nRead, uint8_t* rBuf)
{
    int32_t nResult = 0, fd;

    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg msg[2];

    fd = nDeviceAddress == I2C_MASTER_ADDR ? arrayAddrs[0] : arrayAddrs[1];

    msg_rdwr.msgs = msg;
    msg_rdwr.nmsgs = 2;

    msg[0].addr = nDeviceAddress;
    msg[0].flags = 0;
    msg[0].len = nWrite;
    msg[0].buf = wBuf;
    msg[1].addr = nDeviceAddress;
    msg[1].flags = I2C_M_RD;
    msg[1].len = nRead;
    msg[1].buf = rBuf;

#if 0
    nResult = ioctl(fd, I2C_RDWR, &msg_rdwr);
    if (nResult < 0) {
        return -1;
    }
#endif

#ifdef A2B_PRINT_CONSOLE
    for (uint8_t i = 0; i < nRead; i++) {
        printf(I2C_DEV_PATH" read  device(%#x) reg=0x%02x %03d val=0x%02x ("PRINTF_BINARY_PATTERN_INT8") cnt=%d\n",
                nDeviceAddress, wBuf[0] + i, wBuf[0] + i, rBuf[i], PRINTF_BYTE_TO_BINARY_INT8(rBuf[i]), nRead);
    }
#endif

    return nResult;
}
