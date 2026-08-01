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
#include <string.h>
#include <linux/spi/spidev.h>

#include "adi_a2b_externs.h"
#include "adi_a2b_spidriver.h"
#include "a2b/error.h"

/*
 * A2B SPI command values are defined in a2bstack/a2bstack/inc/a2b/spi.h.
 *
 * Register access:
 *   0x00  A2B_CMD_SPI_LOCAL_REG_WRITE
 *   0x01  A2B_CMD_SPI_LOCAL_REG_READ
 *   0x02  A2B_CMD_SPI_SLAVE_REG_WRITE
 *   0x04  A2B_CMD_SPI_STATUS_READ
 *   0x05  A2B_CMD_SPI_BUS_FIFO_READ
 *   0xC0  A2B_CMD_SPI_SLAVE_REG_READ_REQUEST, 0xC0 - 0xDF
 *
 * Remote I2C peripheral access:
 *   0x07  A2B_CMD_SPI_REMOTE_I2C_WRITE
 *   0x08  A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST
 *
 * Remote SPI peripheral data tunnel:
 *   0x03  A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_WRITE
 *   0x06  A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE
 *   0x09  A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED
 *   0x99  A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED, stack marker only
 *   0x0A  A2B_CMD_SPI_ABORT
 *   0x0B  A2B_CMD_SPI_DATA_TUNNEL_FIFO_READ
 *   0x0C  A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE
 *   0x0D  A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST
 *   0x0E  A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED
 *   0x0F  A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED
 *   0xE0  A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_READ_REQUEST, 0xE0 - 0xFF
 *
 * Remote SPI peripheral usage:
 *   - small register-style read/write: atomic
 *   - large write-only stream:         bulk
 *   - simultaneous MOSI/MISO exchange: full duplex
 *   - more than 256 bytes with CS low: extended bulk/full duplex
 */

/*
 * Master register examples.
 *
 * Write master register 0x12 = 0x00:
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x00\x12\x00'
 *
 * Read 3 bytes from master register 0x02:
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x01\x02\x02\x00\x00\x00'
 */

/*
 * Slave register examples.
 *
 * Write slave0 register 0x41 = 0x01:
 *   0x02  remote register write command
 *   0x00  NODE slave0
 *   0x41  register
 *   0x01  data
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x00\x41\x01'
 *
 * Read 3 bytes from slave0 register 0x02:
 *   step 1: issue read request
 *     cmd bit[7:5] = 110b
 *     cmd bit[4:0] = LEN - 1
 *     cmd          = A2B_CMD_SPI_SLAVE_REG_READ_REQUEST | (LEN - 1)
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\xC2\x00\x02'
 *
 *   step 2: read bus FIFO. The bytes after 0x05 are dummy clocks.
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x05\x00\x00\x00'
 *
 * Broadcast write, register 0x41 = 0x01:
 *   0x02  remote register write command
 *   0x80  broadcast node address
 *   0x41  register
 *   0x01  data
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x80\x41\x01'
 */

/*
 * Remote I2C peripheral examples.
 *
 * Write slave0 I2C peripheral, peripheral register 0x10 = 0x55:
 *   step 1: set slave0 A2B_CHIP = 0x1A
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x00\x00\x1A'
 *
 *   step 2: write remote I2C peripheral
 *     0x07  remote I2C peripheral write command
 *     0x00  NODE slave0
 *     0x10  peripheral register address
 *     0x55  data
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x07\x00\x10\x55'
 *
 * Read 1 byte from slave0 I2C peripheral register 0x10:
 *   step 1: set slave0 A2B_CHIP = 0x1A
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x02\x00\x00\x1A'
 *
 *   step 2: set remote I2C peripheral register pointer to 0x10
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x07\x00\x10'
 *
 *   step 3: issue remote I2C read request, LEN - 1 = 0
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x08\x00\x00'
 *
 *   step 4: read bus FIFO
 *     spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x05\x00'
 *
 * Read 2 bytes:
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x08\x00\x01'
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x05\x00\x00'
 */

/*
 * Remote SPI peripheral examples.
 *
 * NODE/SS byte for data tunnel:
 *   bit[7:6]  SSEL, remote SPI peripheral chip select
 *   bit[5]    M/S, 0 = slave node, 1 = master node
 *   bit[3:0]  NODEID, valid when M/S = 0
 *
 * Bulk write 2 bytes to slave0 + SPISSEL0:
 *   0x06  SPI data tunnel bulk write command
 *   0x00  NODE/SS, slave0 + SPISSEL0
 *   0x01  LEN - 1, two bytes follow
 *   0x10  first byte sent to remote SPI peripheral
 *   0x55  second byte sent to remote SPI peripheral
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x06\x00\x01\x10\x55'
 *
 * Atomic read, read 1 byte from slave0 + SPISSEL0:
 *   0x0D  SPI data tunnel atomic large read request
 *   0x00  NODE/SS, slave0 + SPISSEL0
 *   0x00  LEN - 1, read 1 byte
 *   0x90  command/register byte sent to remote SPI peripheral, for example 0x80 | 0x10
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x0D\x00\x00\x90'
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x0B\x00'
 *
 * Atomic read, read 2 bytes:
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x0D\x00\x01\x90'
 *   sleep 0.02
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x0B\x00\x00'
 *
 * Command-based full duplex, write 2 bytes and read 2 bytes:
 *   0x09  full duplex command-based
 *   0x00  NODE/SS, slave0 + SPISSEL0
 *   0x01  READ_LEN - 1, read 2 bytes
 *   0xAA  first byte sent to remote SPI peripheral
 *   0x55  second byte sent to remote SPI peripheral
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x09\x00\x01\xAA\x55'
 *
 * Register-based full duplex, 0x99:
 *   0x99 is a stack-internal marker. Do not send 0x99 with spidev_test.
 *   It uses a dedicated A2B SPI slave select such as SIO2/ADR2, after
 *   configuring A2B_SPIFDSIZE, A2B_SPIFDTARG and A2B_SPICFG.SPIFDSS.
 *   After configuration, the SPI frame contains only payload bytes:
 *     TX_DATA... <-> RX_DATA...
 *
 * Extended full duplex, more than 256 bytes while keeping remote CS low:
 *   middle packet: 0x0E, NODE/SS, LEN - 1, DATA...
 *   final packet:  0x09, NODE/SS, LEN - 1, DATA...
 *   # 第 1 段，继续保持远端 CS
 *      spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x0E\x00\xFF ...256字节数据...'
 *
 *   # 第 2 段，继续保持远端 CS
 *      spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x0E\x00\xFF ...256字节数据...'
 *
 *   # 最后一段，用普通 full duplex 结束，释放远端 CS
 *      spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x09\x00\x0F ...16字节数据...'
 *
 * Extended bulk write, more than 256 bytes while keeping remote CS low:
 *   middle packet: 0x0F, NODE/SS, LEN - 1, DATA...
 *   final packet:  0x06, NODE/SS, LEN - 1, DATA...
 *
 * Abort data tunnel transaction:
 *   spidev_test -D /dev/spidev7.0 -s 1000000 -b 8 -v -p $'\x0A'
 */


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

static a2b_HResult transfer(int fd, uint8_t const *tx, uint8_t *rx, size_t len)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .speed_hz = SPI_SPEED_HZ,
        .bits_per_word = SPI_BITS_PER_WORD,
    };
    int ret;

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("can't send spi message");

    if (verbose && tx)
        hex_dump(tx, len, 32, "TX");

    if (verbose && rx)
        hex_dump(rx, len, 32, "RX");

    return A2B_RESULT_SUCCESS;
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

    static int spi_fd = open(SPI_DEV_PATH, O_RDWR);
#if 0
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
        close(spi_fd);
        return A2B_NULL;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("SPI_IOC_WR_BITS_PER_WORD");
        close(spi_fd);
        return A2B_NULL;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("SPI_IOC_WR_MAX_SPEED_HZ");
        close(spi_fd);
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
    return transfer(*(int *)hnd, NULL, rBuf, nRead);
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
    return transfer(*(int *)hnd, wBuf, NULL, nWrite);
}

#pragma section("L1_code")
a2b_UInt32 adi_a2b_spiWriteRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( addr );

    uint8_t tx[SPI_MAX_XFER];
    uint8_t rx[SPI_MAX_XFER];

    memset(tx, 0, sizeof(tx));
    memset(rx, 0, sizeof(rx));
    memcpy(tx, wBuf, nWrite);
    transfer(*(int *)hnd, tx, rx, nWrite + nRead);
    
    memcpy(rBuf, &rx[nWrite], nRead);
    return A2B_RESULT_SUCCESS;
}

#pragma section("L1_code")
a2b_UInt32 adi_a2b_spiFd(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( addr );

    uint8_t tx[SPI_MAX_XFER];
    uint8_t rx[SPI_MAX_XFER];

    /*
     * Full-duplex SPI exchanges one byte on MISO for each byte sent on MOSI.
     * For command-based A2B full-duplex, the stack passes protocol header +
     * payload in wBuf and expects the same clock window in rBuf. For
     * register-based mode, wBuf/rBuf are the direct payload buffers.
     * Use local RX storage because rBuf is only guaranteed to hold nRead bytes.
     */
    memset(tx, 0, sizeof(tx));
    memset(rx, 0, sizeof(rx));
    memcpy(tx, wBuf, nWrite);
    transfer(*(int *)hnd, tx, rx, nWrite > nRead ? nWrite : nRead);

    if (nRead != 0u)
        memcpy(rBuf, rx, nRead);

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
