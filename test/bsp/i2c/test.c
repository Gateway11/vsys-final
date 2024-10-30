// SPDX-License-Identifier: GPL-2.0-only
//
// ad2433.c  --  ad2433 ALSA SoC audio component driver
//
// Copyright 2018 Realtek Semiconductor Corp.
// Author: Bard Liao <bardliao@realtek.com>
//

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ad2433.h"

#define I2C_DEV_PATH "/dev/i2c-13" // Adjust as necessary
#define I2C_MASTER_ADDR 0x68
#define I2C_SLAVE_ADDR 0x69

#define I2C_TIMEOUT_DEFAULT             700 	        /*!< timeout: 700ms*/
#define I2C_RETRY_DEFAULT                 3             /*!< retry times: 3 */

int32_t map_addrs[2][1] = {{I2C_MASTER_ADDR, -1}, {I2C_SLAVE_ADDR, -1}};

void parse_xml(const char *path)
{
}

int32_t a2b_i2c_set(uint32_t slave_addr)
{
    int fd;
    
    fd = open(I2C_DEV_PATH, O_RDWR);
    if (fd < 0) {
        printf("Unable to open I2C device: %s\n", strerror(errno));
        return -1; 
    }

    /* Set the I2C slave device address */
    if (ioctl(fd, I2C_SLAVE, slave_addr) < 0) {
        printf("Can't set I2C slave address: %s\n", strerror(errno));
        close(fd); // Close the file descriptor before returning
        return -1; 
    }

    /* Set timeout and retry count */
    ioctl(fd, I2C_TIMEOUT, I2C_TIMEOUT_DEFAULT); // Set timeout
    ioctl(fd, I2C_RETRIES, I2C_RETRY_DEFAULT);   // Set retry times

    return fd;
}

int32_t a2b_i2c_write(int32_t fd, uint8_t addr, uint16_t nwrite, uint8_t *wbuf)
{
    int ret = 0;
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg msg;

    msg_rdwr.msgs = &msg;
    msg_rdwr.nmsgs = 1;

    msg.addr  = addr
    msg.flags = 0;    //write
    msg.len   = nwrite;
    msg.buf = wbuf;

    ret = ioctl(fd, I2C_RDWR, &msg_rdwr);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int32_t a2b_i2c_write_read(int32_t fd, uint8_t *addr,
        uint16_t  nwrite, uint8_t *wbuf, uint16_t nread, uint8_t *rbuf)
{
    int ret;
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg msg[2];

    msg_rdwr.msgs = &msg;
    msg_rdwr.nmsgs = 2;

    msg[0].addr = addr;
    msg[0].buf = (uint8_t*)wbuf;
    msg[0].len = nwrite;
    msg[0].flags = 0;
    msg[1].addr = addr;
    msg[1].buf = rbuf;
    msg[1].len = nread;
    msg[1].flags = I2C_M_RD;

    ret = ioctl(fd, I2C_RDWR, &msg_rdwr);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int32_t main(int argc, const char * argv[])
{
    a2b_i2c_set(I2C_MASTER_ADDR);

    a2b_i2c_set(I2C_SLAVE_ADDR);

    if(argc == 2) {
        parse_xml(argv[1]);
    }

    return 0;
}
