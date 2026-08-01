/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
*******************************************************************************

   Name       : adi_a2b_spidriver.c
   
   Description: This file is responsible for handling all SPI related functions for BF527.
                 
   Functions  :  adi_a2b_spiOpen()
                 adi_a2b_spiRead()
                 adi_a2b_spiWrite()
                 adi_a2b_spiClose()
                 
                 

   Prepared &
   Reviewed by: Automotive Software and Systems team, 
                IPDC, Analog Devices,  Bangalore, India
                
   @version: $Revision: 3701 $
   @date: $Date: 2015-10-16 11:51:11 +0530 (Fri, 16 Oct 2015) $
               
******************************************************************************/

/*! \addtogroup Target_Dependent Target Dependent
 *  @{
 */

/** @defgroup SPI
 *
 * This driver interface handles PAL SPI for BF527
 *
 */

/*! \addtogroup SPI
 *  @{
 */
 
/*============= I N C L U D E S =============*/
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>

#include "adi_a2b_externs.h"
#include "adi_a2b_spidriver.h"
#include "a2b/error.h"

// a2bstack/a2bstack/inc/a2b/spi.h
// typedef enum
// {
//     A2B_CMD_SPI_LOCAL_REG_WRITE                         = 0x00,
//     A2B_CMD_SPI_LOCAL_REG_READ                          = 0x01,
//     A2B_CMD_SPI_SLAVE_REG_WRITE                         = 0x02,
//     A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_WRITE                = 0x03,
//     A2B_CMD_SPI_STATUS_READ                             = 0x04,
//     A2B_CMD_SPI_BUS_FIFO_READ                           = 0x05,
//     A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE                  = 0x06,
//     A2B_CMD_SPI_REMOTE_I2C_WRITE                        = 0x07,
//     A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST                 = 0x08,
//     A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED       = 0x09,
//     A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED       = 0x99,
//     A2B_CMD_SPI_ABORT                                   = 0x0A,
//     A2B_CMD_SPI_DATA_TUNNEL_FIFO_READ                   = 0x0B,
//     A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE          = 0x0C,
//     A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST   = 0x0D,
//     A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED        = 0x0E,
//     A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED         = 0x0F,
//     A2B_CMD_SPI_SLAVE_REG_READ_REQUEST                  = 0xC0,         /* 0xC0 - 0xDF */
//     A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_READ_REQUEST         = 0xE0,         /* 0xE0 - 0xFF */
// } a2b_SpiCmd;

////////////////////////////////////////////////////////////////////////////////////////////
// 写 Master 的 0x12 = 0x00
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x00\x12\x00'

////////////////////////////////////////////////////////////////////////////////////////////
// 读 Master 的 0x02 开始 3 字节
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x01\x02\x02\x00\x00\x00'

////////////////////////////////////////////////////////////////////////////////////////////
// 写 slave0 的 0x41 = 0x01
// 0x02 = remote register write command
// 0x00 = NODE slave0
// 0x41 = register
// 0x01 = data
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x00\x41\x01'

////////////////////////////////////////////////////////////////////////////////////////////
// 读 slave0 的 0x02 开始 3 字节，分两步
// bit[7:5] = 110b
// bit[4:0] = LEN - 1
// A2B_CMD_SPI_SLAVE_REG_READ_REQUEST(C0) | (LEN - 1)
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\xc2\x00\x02'

// 0x05 = bus FIFO read command
// 后面的 0x00 只是 dummy clock，用来取数据
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x05\x00\x00'

////////////////////////////////////////////////////////////////////////////////////////////
// 写广播
// 0x02 = remote register write command
// 0x80 = NODE slave0(广播node?)
// 0x41 = register
// 0x01 = data
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x80\x41\x01'

////////////////////////////////////////////////////////////////////////////////////////////
// Peri write
// 先设置 slave0 的 A2B_CHIP = 0x1a
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x00\x00\x1a'

// 然后写 remote I2C peripheral
// 0x07 = remote I2C peripheral write
// 0x00 = NODE slave0
// 0x10 = peripheral register address
// 0x55 = data
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x07\x00\x10\x55'

////////////////////////////////////////////////////////////////////////////////////////////
// Peri read
// 先设置 slave0 的 A2B_CHIP = 0x1a
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x00\x00\x1a'

// 设置外设寄存器指针到 0x10
// 文档中 SPI Remote I2C Read Request Transaction 的帧格式是
// 0x07 = remote I2C peripheral write
// 0x00 = slave0
// 0x10 = 发给远端 I2C 外设的第 1 个数据字节
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x07\x00\x10'

// 发起读取 1 字节请求
// 0x08 = remote I2C peripheral read request
// 0x00 = NODE slave0
// 0x00 = LEN-1，表示读 1 字节
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x08\x00\x00'

// 取回数据
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x05\x00'

// 读 2 字节
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x08\x00\x01'
// spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x05\x00\x00'

/*============= D E F I N E S =============*/
#define SPI_DEV_PATH                    "/dev/spidev7.0"
#define SPI_SPEED_HZ                    1000000
#define SPI_BITS_PER_WORD               8
#define SPI_MAX_XFER                    256

/*============= D A T A =============*/
static int verbose = 1;

/*============= C O D E =============*/

static void hex_dump(const void *src, size_t length, size_t line_size,
             char *prefix)
{
    int i = 0;
    const unsigned char *address = src;
    const unsigned char *line = address;
    unsigned char c;

    printf("%s | ", prefix);
    while (length-- > 0) {
        printf("%02X ", *address++);
        if (!(++i % line_size) || (length == 0 && i % line_size)) {
            if (length == 0) {
                while (i++ % line_size)
                    printf("__ ");
            }
            printf(" | ");  /* right close */
            while (line < address) {
                c = *line++;
                printf("%c", (c < 33 || c == 255) ? 0x2E : c);
            }
            printf("\n");
            if (length > 0)
                printf("%s | ", prefix);
        }
    }
}

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
#if 1
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .speed_hz = SPI_BITS_PER_WORD,
        .bits_per_word = SPI_BITS_PER_WORD,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("can't send spi message");

    if (verbose && tx)
        hex_dump(tx, len, 32, "TX");

    if (verbose && rx)
        hex_dump(rx, len, 32, "RX");
#endif
}

a2b_UInt32 adi_a2b_spiInit(A2B_ECB* ecb)
{
	A2B_UNUSED( ecb );

	return A2B_RESULT_SUCCESS;
}

/*****************************************************************************/
/*!

@brief This API initializes the SPI module.SPI handlers are initialized, preliminary device control
                    is established, and the SPI is prepared for use.

@param [in] nSpiDeviceNo    Device no

@param [in] pUserArgument   User argument
  
    
@return nReturnValue
        - 1: Failure
        - 0: Success
  
*/   
/*****************************************************************************/
#pragma section("L3_code")
a2b_Handle adi_a2b_spiOpen(A2B_ECB* ecb)
{
    A2B_UNUSED( ecb );

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = SPI_BITS_PER_WORD;
    uint32_t speed = SPI_SPEED_HZ;

#if 0
    static int spi_fd = open(dev, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device " SPI_DEV_PATH);
        return A2B_NULL;
    }

    /*
     * CPOL/CPHA must match A2B_SPICFG.SPI_CPOL/SPI_CPHA.
     * If your board config uses another SPI mode, change SPI_MODE_0 here.
     */
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("SPI_IOC_WR_MODE");
        return A2B_NULL;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("SPI_IOC_WR_BITS_PER_WORD");
        return A2B_NULL;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("SPI_IOC_WR_MAX_SPEED_HZ");
        return A2B_NULL;
    }
#endif
    return &spi_fd;
}

/*****************************************************************************/
/*!

@brief:   This Function initiates SPI read through core transfer/DMA

@param [in]:pReadPtr    Pointer to read buffer

@param [in]:nSize       Size in words
  
    
@return nReturnValue
        - 1: Failure
        - 0: Success
  
*/   
/*****************************************************************************/
#pragma section("L1_code")
a2b_UInt32 adi_a2b_spiRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( addr );

    transfer(*(int *)hnd, NULL, rBuf, nRead);
    return A2B_RESULT_SUCCESS;
}

/*****************************************************************************/
/*!

@brief This function initiates SPI write through DMA/core transfer

@param [in] pWriteptr       Pointer to write buffer

@param [in] nCount          Count in bytes
    
@return nReturnValue
        - 1: Failure
        - 0: Success
  
*/   
/*****************************************************************************/
#pragma section("L1_code")
a2b_UInt32 adi_a2b_spiWrite(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf)
{
	A2B_UNUSED( addr );

    transfer(*(int *)hnd, wBuf, NULL, nWrite);
    return A2B_RESULT_SUCCESS;
}

#pragma section("L1_code")
a2b_UInt32 adi_a2b_spiWriteRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( addr );

#if 0
    uint8_t rx[SPI_MAX_XFER];
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)wBuf,
        .rx_buf = (unsigned long)rx,
        .len = nWrite + nRead,
        .speed_hz = SPI_SPEED_HZ,
        .bits_per_word = SPI_BITS_PER_WORD,
        .cs_change = 0;
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        return -errno;

    memcpy(rBuf, &rx[nWrite], nRead);
#endif
    return A2B_RESULT_SUCCESS;
}

#pragma section("L1_code")
a2b_UInt32 adi_a2b_spiFd(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( addr );

    transfer(*(int *)hnd, wBuf, rBuf, nWrite > nRead ? nWrite : nRead);
    return A2B_RESULT_SUCCESS;
}
/*****************************************************************************/
/*!


@brief: This function closes spi driver.The peripheral shall be reset to default state.

@param [in]:nSPINum    SPI device number
    
@return nReturnValue
        - 1: Failure
        - 0: Success
  
*/   
/*****************************************************************************/
#pragma section("L3_code")
a2b_UInt32 adi_a2b_spiClose(a2b_Handle hnd)
{
    close(*(int *)hnd);
    return A2B_RESULT_SUCCESS;
}


/** 
 @}
*/

/**
 @}
*/

/*
**
** EOF: $URL$
**
*/




