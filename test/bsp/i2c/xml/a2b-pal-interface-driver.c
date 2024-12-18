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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "a2b-pal-interface.h"

int32_t adi_a2b_I2C_Write(void* handle, uint16_t deviceAddr, uint16_t writeLength, uint8_t* writeBuffer) {
    int32_t result = 0;
    struct i2c_adapter *adap = (struct i2c_adapter*)handle;

    struct i2c_msg msg[1];
    msg[0].addr  = deviceAddr;
    msg[0].flags = 0;
    msg[0].len   = writeLength;
    msg[0].buf   = writeBuffer;

    if ((result = i2c_transfer(adap, msg, ARRAY_SIZE(msg)) < 0) {
        printf(I2C_DEV_PATH " write device(%#x) reg=0x%02X error, ret=%d\n", deviceAddr, writeBuffer[0], result);
        return -1;
    }

#ifdef A2B_PRINT_CONSOLE
    for (uint16_t i = 0; i < writeLength - 1 && deviceAddr != DSP_XXXX; i++) {
        pr_info(I2C_DEV_PATH " write device(%#x) reg=0x%02X %03d, val=0x%02X (" PRINTF_BINARY_PATTERN_INT8 "), cnt=%d\n",
               deviceAddr, writeBuffer[0] + i, writeBuffer[0] + i, writeBuffer[i + 1], PRINTF_BYTE_TO_BINARY_INT8(writeBuffer[i + 1]), writeLength - 1);
    }
#endif

    return 0;
}

int32_t adi_a2b_I2C_WriteRead(void* handle, uint16_t deviceAddr, uint16_t writeLength, uint8_t* writeBuffer, uint16_t readLength, uint8_t* readBuffer) {
    int32_t result = 0;
    struct i2c_adapter *adap = (struct i2c_adapter*)handle;

    struct i2c_msg msg[2];
    msg[0].addr = deviceAddr;
    msg[0].flags = 0;
    msg[0].len = writeLength;
    msg[0].buf = writeBuffer;
    msg[1].addr = deviceAddr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = readLength;
    msg[1].buf = readBuffer;

    if ((result = i2c_transfer(adap, msg, ARRAY_SIZE(msg)) < 0) {
        printf(I2C_DEV_PATH "  read device(%#x) reg=0x%02X error, ret=%d\n", deviceAddr, writeBuffer[0], result);
        return -1;
    }

#ifdef A2B_PRINT_CONSOLE
    for (uint16_t i = 0; i < readLength && writeLength == 1; i++) {
        pr_info(I2C_DEV_PATH "  read device(%#x) reg=0x%02X %03d, val=\033[4m0x%02X\033[0m (" PRINTF_BINARY_PATTERN_INT8 "), cnt=%d\n",
               deviceAddr, writeBuffer[0] + i, writeBuffer[0] + i, readBuffer[i], PRINTF_BYTE_TO_BINARY_INT8(readBuffer[i]), readLength);
    }
#endif

    return 0;
}

char* a2b_pal_File_Read(const char* filename, size_t* outSize) {
    struct file* file;
    mm_segment_t old_fs;
    char* buffer;
    loff_t file_size;
    size_t read_size;

    // Set the memory segment for user access
    old_fs = get_fs();
    set_fs(KERNEL_DS); // Change to kernel address space

    file = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(file)) {
        set_fs(old_fs); // Restore the original memory segment
        printk(KERN_ERR "Failed to open file: %ld\n", PTR_ERR(file));
        return NULL;
    }

    file_size = i_size_read(file->f_inode);
    buffer = kmalloc(file_size + 1, GFP_KERNEL); // +1 for null terminator
    if (!buffer) {
        filp_close(file, NULL);
        set_fs(old_fs); // Restore the original memory segment
        printk(KERN_ERR "Failed to allocate memory\n");
        return NULL;
    }

    read_size = kernel_read(file, buffer, file_size, &file->f_pos);
    if (read_size < 0) {
        kfree(buffer);
        filp_close(file, NULL);
        set_fs(old_fs); // Restore the original memory segment
        printk(KERN_ERR "Failed to read file: %ld\n", read_size);
        return NULL;
    }

    buffer[read_size] = '\0';
    if (outSize) {
        *outSize = read_size;
    }

    filp_close(file, NULL);
    set_fs(old_fs);

    return buffer;
}

void adi_a2b_Delay(uint32_t time_ms) {
    pr_info("sleep 0x%02xms\n", time_ms);
    mdelay(time_ms);
}
