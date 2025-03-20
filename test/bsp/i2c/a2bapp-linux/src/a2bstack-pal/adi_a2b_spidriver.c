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
#include <unistd.h>
#include <linux/spi/spidev.h>

#include "adi_a2b_datatypes.h"
#include "adi_a2b_externs.h"
#include "adi_a2b_spidriver.h"
#include "a2b/error.h"

/*============= D E F I N E S =============*/
#define SPI_DEV_PATH                    "/dev/spidev7.0"

/*============= D A T A =============*/
static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
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
    int ret;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    if (mode & SPI_TX_QUAD)
        tr.tx_nbits = 4;
    else if (mode & SPI_TX_DUAL)
        tr.tx_nbits = 2;
    if (mode & SPI_RX_QUAD)
        tr.rx_nbits = 4;
    else if (mode & SPI_RX_DUAL)
        tr.rx_nbits = 2;
    if (!(mode & SPI_LOOP)) {
        if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr.rx_buf = 0;
        else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr.tx_buf = 0;
    }

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
        perror("can't send spi message");

    if (verbose && tx)
        hex_dump(tx, len, 32, "TX");

    if (verbose && rx)
        hex_dump(rx, len, 32, "RX");
}

uint32 adi_a2b_spiInit(A2B_ECB* ecb)
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
    static int32_t fd[2];
    int32_t ret;
#if 0
    for (uint8_t i = 0; i < A2B_ARRAY_SIZE(fd); i++) {
        fd[i] = open(SPI_DEV_PATH, O_RDWR);
        if (fd[i] < 0) {
            perror("Failed to open the SPI device " SPI_DEV_PATH);
            //return A2B_NULL;
        }

        /*
         * spi mode
         */
        ret = ioctl(fd[i], SPI_IOC_WR_MODE32, &mode);
        if (ret == -1)
            perror("can't set spi mode");

        ret = ioctl(fd[i], SPI_IOC_RD_MODE32, &mode);
        if (ret == -1)
            perror("can't get spi mode");
    }
#endif
    return fd;
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
uint32 adi_a2b_spiRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( hnd );
	A2B_UNUSED( addr );

    transfer(*(int32_t *)hnd, NULL, rBuf, nRead);
    //read(*(int32_t *)hnd, rBuf, nRead);
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
uint32 adi_a2b_spiWrite(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf)
{
	A2B_UNUSED( hnd );
	A2B_UNUSED( addr );

    transfer(*(int32_t *)hnd, wBuf, NULL, nWrite);
    //write(*(int32_t *)hnd, wBuf, nWrite);
    return A2B_RESULT_SUCCESS;
}

#pragma section("L1_code")
uint32 adi_a2b_spiWriteRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( hnd );
	A2B_UNUSED( addr );

    transfer(*(int32_t *)hnd, wBuf, rBuf, nWrite > nRead ? nWrite : nRead);
    return A2B_RESULT_SUCCESS;
}

#pragma section("L1_code")
uint32 adi_a2b_spiFd(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( hnd );
	A2B_UNUSED( addr );

    transfer(*(int32_t *)hnd, wBuf, rBuf, nWrite > nRead ? nWrite : nRead);
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
uint32 adi_a2b_spiClose(a2b_Handle hnd)
{
    close(*(int32_t *)hnd);
    close(*(((int32_t *)hnd) + 1));
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




