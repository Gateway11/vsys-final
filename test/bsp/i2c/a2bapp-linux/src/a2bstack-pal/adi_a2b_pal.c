/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
*******************************************************************************

   Name       : adi_a2b_pal.c
   
   Description: This file is responsible for handling all TWI related functions.        
                 
   Functions  : a2b_I2cInit()
                a2b_I2cOpenFunc()
                a2b_I2cCloseFunc()
                a2b_I2cReadFunc()
                a2b_I2cWriteFunc()
                a2b_I2cWriteReadFunc()
                a2b_I2cShutdownFunc()
               
                 
   Prepared &
   Reviewed by: Automotive Software and Systems team, 
                IPDC, Analog Devices,  Bangalore, India
                
   @version: $Revision: 3626 $
   @date: $Date: 2015-10-05 14:04:13 +0530 (Mon, 05 Oct 2015) $
               
******************************************************************************/
/*! \addtogroup Target_Dependent Target Dependent
 *  @{
 */

/** @defgroup PAL
 *
 * This module handles all the pal CB wrappers for the underlying target
 * platform
 *
 */

/*! \addtogroup PAL
 *  @{
 */


/*============= I N C L U D E S =============*/

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "adi_a2b_framework.h"
#include "adi_a2b_externs.h"
#include "a2bplugin-slave/adi_a2b_periconfig.h"
#include "a2b/stack.h"
#include "a2b/util.h"
#include "a2b/conf.h"
#include "a2b/i2c.h"
#include "a2b/error.h"
#include "a2bplugin-master/plugin.h"
#include "plugin_priv.h"
#include "adi_a2b_spidriver.h"

#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/time.h>

/*============= D E F I N E S =============*/
#define I2C_DEV_PATH                    "/dev/i2c-16"
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

/*============= D A T A =============*/

/* Pointer to Platform ECB */
static a2b_PalEcb* pPalEcb;
A2B_PAL_L3_DATA
static a2b_UInt8 aDataBuffer[ADI_A2B_MAX_PERI_CONFIG_UNIT_SIZE];

/*============= C O D E =============*/
/*
** Function Prototype section
*/
static a2b_UInt32 adi_a2b_AudioHostConfig(a2b_PalEcb*  palEcb, ADI_A2B_PERI_DEVICE_CONFIG* psDeviceConfig);

/*
** Function Definition section
*/

/*!****************************************************************************
*
*  \b              a2b_palInit
*
*  <b> API Details: </b><br>
*  Initializes the Platform Abstraction Layer (PAL) structure [function
*  pointers] with defaults for the given platform. These defaults can be
*  over-ridden as needed if necessary.
*                                                                       <br><br>
*  <b> Linux Implementation Details: </b><br>
*  This sets up all the function pointers for core PAL functionality used
*  by the A2B stack.  If A2B_FEATURE_MEMORY_MANAGER is NOT defined then
*  Linux malloc/free is used instead of the stack memory manager.  This
*  also implements some environment variable support for Linux:             <br>
*  - A2B_CONF_TRACE_CHAN_URL                                                <br>
*  - A2B_CONF_TRACE_LEVEL                                                   <br>
*  - A2B_CONF_I2C_DEVICE_PATH                                               <br>
*  - A2B_CONF_PLUGIN_SEARCH_PATTERN                                         <br>
*                                                                           <br>
*  \param          [in]    pal      Pointer to the Platform Abstraction Layer to
*                                   initialize with defaults.
*
*  \param          [in]    ecb      The environment control block (ecb) for
*                                   this platform.
*
*  \pre            None
*
*  \post           The pal function pointer API will be fully
*                  populated with default values.
*
*  \return         None
*
******************************************************************************/
A2B_PAL_L1_CODE
void
a2b_palInit
    (
    struct a2b_StackPal*    pal,
    A2B_ECB*                ecb
    )
{
    a2b_Char* envValue;

    if ( A2B_NULL != pal )
    {
        a2b_memset(pal, 0, sizeof(*pal));
        a2b_memset(ecb, 0, sizeof(*ecb));

        /* Do necessary base initialization */
        a2b_stackPalInit(pal, ecb);

#ifndef A2B_FEATURE_MEMORY_MANAGER
        /* Use Linux malloc/free */
        pal->memMgrInit      = a2b_pal_memMgrInit;
        pal->memMgrOpen      = a2b_pal_memMgrOpen;
        pal->memMgrMalloc    = a2b_pal_memMgrMalloc;
        pal->memMgrFree      = a2b_pal_memMgrFree;
        pal->memMgrClose     = a2b_pal_memMgrClose;
        pal->memMgrShutdown  = a2b_pal_memMgrShutdown;
#endif

        pal->timerInit       = a2b_pal_TimerInitFunc;
        pal->timerGetSysTime = a2b_pal_TimerGetSysTimeFunc;
        pal->timerShutdown   = a2b_pal_TimerShutdownFunc;

#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
        pal->logInit         = a2b_pal_logInit;
        pal->logOpen         = a2b_pal_logOpen;
        pal->logClose        = a2b_pal_logClose;
        pal->logWrite        = a2b_pal_logWrite;
        pal->logShutdown     = a2b_pal_logShutdown;
#endif

        pal->i2cInit         = a2b_pal_I2cInit;
        pal->i2cOpen         = a2b_pal_I2cOpenFunc;
        pal->i2cClose        = a2b_pal_I2cCloseFunc;
        pal->i2cRead         = a2b_pal_I2cReadFunc;
        pal->i2cWrite        = a2b_pal_I2cWriteFunc;
        pal->i2cWriteRead    = a2b_pal_I2cWriteReadFunc;
        pal->i2cShutdown     = a2b_pal_I2cShutdownFunc;

        pal->spiInit		 = adi_a2b_spiInit;
        pal->spiOpen		 = adi_a2b_spiOpen;
        pal->spiClose		 = adi_a2b_spiClose;
        pal->spiRead		 = adi_a2b_spiRead;
        pal->spiWrite		 = adi_a2b_spiWrite;
        pal->spiWriteRead	 = adi_a2b_spiWriteRead;
        pal->spiFd			 = adi_a2b_spiFd;
		pal->spiShutdown	 = A2B_NULL;

        pal->audioInit       = a2b_pal_AudioInitFunc;
        pal->audioOpen       = a2b_pal_AudioOpenFunc;
        pal->audioClose      = a2b_pal_AudioCloseFunc;
        pal->audioConfig     = a2b_pal_AudioConfigFunc;
        pal->audioShutdown   = a2b_pal_AudioShutdownFunc;

        pal->pluginsLoad     = a2bapp_pluginsLoad;
        pal->pluginsUnload   = a2bapp_pluginsUnload;

        pal->getVersion      = a2b_pal_infoGetVersion;
        pal->getBuild        = a2b_pal_infoGetBuild;
#if defined A2B_BCF_FROM_FILE_IO
        pal->fileRead		 = a2b_pal_FileRead;
#endif
        if ( A2B_NULL != ecb )
        {
            ecb->baseEcb.i2cAddrFmt    = A2B_I2C_ADDR_FMT_7BIT;
            ecb->baseEcb.i2cBusSpeed   = A2B_I2C_BUS_SPEED_100KHZ;
            ecb->baseEcb.i2cMasterAddr = A2B_CONF_DEFAULT_MASTER_NODE_I2C_ADDR;
        }
    }
} /* a2b_palInit */

/*****************************************************************************/
/*!
@brief  This API initializes I2C subsystem.

@param [in]:ecb  - PAL ECB structure.
  
    
@return Return code
        -1: Failure
        -0: Success
        
\note I2C and TWI terms are used  interchangeably        
  
*/   
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_UInt32 a2b_pal_I2cInit(A2B_ECB* ecb)
{
    a2b_UInt32 nReturnValue = (a2b_UInt32)0;
    return nReturnValue;
}

/*****************************************************************************/
/*!
@brief  This API Post Initialization of I2C subsystem and
        returns the handle

@param [in]:fmt  - 7-bit or 10-bit address.
@param [in]:speed  - I2C Bus Speed.
@param [in]:ecb  - PAL ECB structure.

@return Return code
			Handle to the I2C module

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_Handle a2b_pal_I2cOpenFunc(a2b_I2cAddrFmt fmt,
        a2b_I2cBusSpeed speed, A2B_ECB* ecb)
{
    static int32_t fd = -1;
    fd = open(I2C_DEV_PATH, O_RDWR);

    if (fd < 0) {
        perror("Failed to open I2C device " I2C_DEV_PATH);
        //return A2B_NULL;
    }

    /* Set timeout and retry count */
    ioctl(fd, I2C_TIMEOUT, I2C_TIMEOUT_DEFAULT); // Set timeout
    ioctl(fd, I2C_RETRIES, I2C_RETRY_DEFAULT);   // Set retry times

    ecb->palEcb.i2chnd = &fd;
    return ecb->palEcb.i2chnd;
}

/*****************************************************************************/
/*!
@brief  This API Reads a bytes of data from an I2C device

@param [in]:hnd  - Handle to the I2C Sub-system.
@param [in]:addr  - Device Address to which I2C communication
                    should happen.
@param [in]:nRead  - Number of bytes to be read.
@param [in]:rBuf  - Pointer to the buffer where read bytes are
                    to be stored.
@return Return code
        -1: Failure
        -0: Success

*/
/*****************************************************************************/
A2B_PAL_L1_CODE
a2b_HResult a2b_pal_I2cReadFunc(a2b_Handle hnd,
        a2b_UInt16 addr, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
    return a2b_pal_I2cWriteReadFunc(hnd, addr, 1, rBuf, nRead - 1, rBuf + 1);
}

/*****************************************************************************/
/*!
@brief  This API writes a bytes of data to an I2C device

@param [in]:hnd  - Handle to the I2C Sub-system.
@param [in]:addr  - Device Address to which I2C communication
                    should happen.
@param [in]:Write  - Number of bytes to be written.
@param [in]:wBuf  - Pointer to the buffer from where bytes are
                    to be written.
@return Return code
        -1: Failure
        -0: Success


*/
/*****************************************************************************/
A2B_PAL_L1_CODE
a2b_HResult a2b_pal_I2cWriteFunc(a2b_Handle hnd,
        a2b_UInt16 addr, a2b_UInt16 nWrite,
        const a2b_Byte* wBuf)
{
    a2b_Int32 nReturnValue = A2B_RESULT_SUCCESS;
    int32_t fd = *(int32_t *)hnd;

    struct i2c_rdwr_ioctl_data msgRdwr;
    struct i2c_msg msg;

    msgRdwr.msgs = &msg;
    msgRdwr.nmsgs = 1;

    msg.addr  = addr;
    msg.flags = 0;
    msg.len   = nWrite;
    msg.buf   = (a2b_Byte*)wBuf;

    if ((nReturnValue = ioctl(fd, I2C_RDWR, &msgRdwr)) < 0) {
        printf(I2C_DEV_PATH " write device(%#x) reg=0x%02X error, 
                cnt=%d, ret=%d\n", addr, wBuf[0], nWrite - 1, nReturnValue);
        return 1;
    }

#ifdef A2B_PRINT_CONSOLE
    for (uint16_t i = 0; i < (nWrite - 1); i++) {
        printf(I2C_DEV_PATH " write device(%#x) 
                reg=0x%02X %03d, val=0x%02X (" PRINTF_BINARY_PATTERN_INT8 "), cnt=%d\n",
               addr, wBuf[0] + i, wBuf[0] + i,
               wBuf[i + 1], PRINTF_BYTE_TO_BINARY_INT8(wBuf[i + 1]), nWrite - 1);
    }
#endif

    return 0;
}

/*****************************************************************************/
/*!
@brief  This API writes a bytes of data to an I2C device

@param [in]:hnd  - Handle to the I2C Sub-system.
@param [in]:addr  - Device Address to which I2C communication
                    should happen.
@param [in]:nWrite  - Number of bytes to be written.
@param [in]:wBuf  - Pointer to the buffer from where bytes are
                    to be written.
@param [in]:nRead  - Number of bytes to be read.
@param [in]:rBuf  - Pointer to the buffer where read bytes are
                    to be stored.
@return Return code
        -1: Failure
        -0: Success

*/
/*****************************************************************************/
A2B_PAL_L1_CODE
a2b_HResult a2b_pal_I2cWriteReadFunc(a2b_Handle hnd,
        a2b_UInt16 addr, a2b_UInt16 nWrite,
        const a2b_Byte* wBuf, a2b_UInt16 nRead,
        a2b_Byte* rBuf)
{
    a2b_Int32 nReturnValue = A2B_RESULT_SUCCESS;
    int32_t fd = *(int32_t *)hnd;

    memset(rBuf, 0, nRead);

    struct i2c_rdwr_ioctl_data msgRdwr;
    struct i2c_msg msg[2];

    msgRdwr.msgs = msg;
    msgRdwr.nmsgs = 2;

    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].len = nWrite;
    msg[0].buf = (a2b_Byte*)wBuf;
    msg[1].addr = addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = nRead;
    msg[1].buf = rBuf;

    if ((nReturnValue = ioctl(fd, I2C_RDWR, &msgRdwr)) < 0) {
        printf(I2C_DEV_PATH "  read device(%#x) reg=0x%02X error, 
                cnt=%d, ret=%d\n", addr, wBuf[0], nRead, nReturnValue);
        return 1;
    }

#ifdef A2B_PRINT_CONSOLE
    for (uint16_t i = 0; i < nRead && nWrite == 1; i++) {
        printf(I2C_DEV_PATH "  read device(%#x) reg=0x%02X %03d, 
                val=\033[4m0x%02X\033[0m (" PRINTF_BINARY_PATTERN_INT8 "), cnt=%d\n",
               addr, wBuf[0] + i,
               wBuf[0] + i, rBuf[i], PRINTF_BYTE_TO_BINARY_INT8(rBuf[i]), nRead);
    }
#endif

    return 0;
}

/*****************************************************************************/
/*!
@brief  This API shuts down I2C subsystem.

@param [in]:ecb  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_UInt32 a2b_pal_I2cShutdownFunc(A2B_ECB* ecb)
{
    a2b_UInt32 nReturnValue = (a2b_UInt32)0;
    int32_t fd = *(int32_t *)ecb->palEcb.i2chnd;

    close(fd);
    ecb->palEcb.i2chnd = NULL;

    return nReturnValue;
}


/*****************************************************************************/
/*!
@brief  This API Post Initialization of I2C subsystem and
        returns the handle

@param [in]:fmt  - 7-bit or 10-bit address.
@param [in]:speed  - I2C Bus Speed.
@param [in]:ecb  - PAL ECB structure.

@return Return code
			Handle to the I2C module

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_I2cCloseFunc(a2b_Handle hnd)
{
    a2b_HResult nReturnValue = A2B_RESULT_SUCCESS;
    return nReturnValue;
}

A2B_PAL_L1_CODE
a2b_HResult a2b_EepromWriteRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite,
        const a2b_Byte* wBuf, a2b_UInt16 nRead,
        a2b_Byte* rBuf)
{
    return a2b_pal_I2cWriteReadFunc(hnd, addr, nWrite, wBuf, nRead, rBuf);
}

#ifdef A2B_BCF_FROM_FILE_IO
/*****************************************************************************/
/*!
@brief  This API reads binary file

@param [in]:hnd  - File pointer.
@param [in]:offset  - Number of bytes ot skip from the start.
@param [in]:nRead  - number of bytes to read.
@param [in]:rBuf  - Buffer to read.

@return Return code
			a2b_HResult : 0 -success, 1 failure

*/
/*****************************************************************************/
a2b_HResult a2b_pal_FileRead(a2b_Handle hnd, a2b_UInt16 offset, a2b_UInt16 nRead,
        a2b_Byte* rBuf)
{
	a2b_HResult nReturnValue = A2B_RESULT_SUCCESS;
    FILE* fp = (FILE*)hnd;
    a2b_UInt16 nActualReadBytes;

    if(fp != NULL)
    {
    	nReturnValue = fseek(fp, offset, 0);

    	nActualReadBytes = fread(rBuf, 1, nRead, fp);
    	if(nRead != nActualReadBytes)
    	{
    		nReturnValue = 1;
    	}
    }
    else
    {
    	nReturnValue = 1u;
    }
	return nReturnValue;

}
/*****************************************************************************/
/*!
@brief  This API opens binary file (read mode)

@param [in]:ecb  - A2B ECB structure pointer.
@param [in]:url  - File Path.

@return Return code
			a2b_HResult : 0 -success, 1 failure

*/
/*****************************************************************************/
a2b_HResult a2b_pal_FileOpen(A2B_ECB* ecb, char* url)
{
	a2b_HResult res = A2B_RESULT_SUCCESS;

	/* File read is a binary file. So "rb" in fopen */
	ecb->palEcb.fp = fopen(url, "rb");
	if(ecb->palEcb.fp != A2B_NULL)
	{
		res = A2B_RESULT_SUCCESS;
	}
	else
	{
		res = 1u;
	}

	return(res);
}
/*****************************************************************************/
/*!
@brief  This API closes binary file (read mode)

@param [in]:ecb  - A2B ECB structure pointer.

@return Return code
			a2b_HResult : 0 -success, 1 failure

*/
/*****************************************************************************/
a2b_HResult a2b_pal_FileClose(A2B_ECB* ecb)
{
	 return(fclose(ecb->palEcb.fp));
}
#endif
/*****************************************************************************/
/*!
@brief  This API initializes Timer subsystem.

@param [in]:ecb  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_TimerInitFunc(A2B_ECB* ecb)
{
	pPalEcb = &ecb->palEcb;
	a2b_HResult nReturnValue = (a2b_UInt32)0;

    return nReturnValue;
}

/*****************************************************************************/
/*!
@brief  This API gives the current system time

@return Return : Current time in millisec

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L1_CODE
a2b_UInt64 a2b_pal_TimerGetSysTimeFunc()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);  // Get the current time

    uint64_t milliseconds = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    //uint32_t milliseconds = tv.tv_usec / 1000;
    //uint32_t milliseconds = clock();
    //printf("Current time in milliseconds: %lld\n", milliseconds);

    return milliseconds;
}

/*****************************************************************************/
/*!
@brief  This API shuts the Timer subsystem.

@param [in]:ecb  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_TimerShutdownFunc(A2B_ECB* ecb)
{
	pPalEcb = &ecb->palEcb;
	a2b_HResult nReturnValue = (a2b_UInt32)0;

    return nReturnValue;
}

/*****************************************************************************/
/*!
@brief  This API Initializes the audio sub-system.

@param [in]:ecb  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_AudioInitFunc(A2B_ECB* ecb)
{
    a2b_UInt8 nReturn = 0xFFu;
    a2b_UInt8 nIndex;
    ADI_A2B_PERI_DEVICE_CONFIG* psDeviceConfig;
    ADI_A2B_NODE_PERICONFIG *psPeriConfig;
    a2b_UInt8 wBuf[2];
    a2b_UInt8 rBuf[8];

    /* A2B feature EEPROM processing */
#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined (A2B_BCF_FROM_FILE_IO)
    a2b_PeripheralNode oPeriphNode, *pPeriphNode;
    a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_UInt16 nOffset = 0u, payloadLen = 0u, payloadDataLen = 0u;
    a2b_UInt8 cfgType = 0u, cfgCrc = 0u,  nCfgBlocks = 0u, regAddr = 0u, crc8 = 0u;

    nReturn 	= 0u;
    pPeriphNode = &oPeriphNode;

    /* To read the number of EEPROM */
    A2B_GET_UINT16_BE(nOffset, ecb->palEcb.pEepromAudioHostConfig , 0);
    nOffset += 3u;
#ifdef A2B_BCF_FROM_SOC_EEPROM
    A2B_PUT_UINT16_BE(nOffset, wBuf,0);
    status  = a2b_pal_I2cWriteReadFunc( ecb->palEcb.i2chnd,A2B_I2C_EEPROM_ADDR,
                                      2u,  wBuf,
                                      1u,  &nCfgBlocks );
#else
    status  = a2b_pal_FileRead( ecb->palEcb.fp,  nOffset,
                                      1u,  &nCfgBlocks );
#endif


    A2B_GET_UINT16_BE(pPeriphNode->addr, ecb->palEcb.pEepromAudioHostConfig, 0);

    /* 4 bytes from marker */
    pPeriphNode->addr 	  += 4u;
    pPeriphNode->cfgIdx     = 0u;
    pPeriphNode->nodeAddr   = -1;
    pPeriphNode->nCfgBlocks = nCfgBlocks;

    for (pPeriphNode->cfgIdx = 0u;
         pPeriphNode->cfgIdx < pPeriphNode->nCfgBlocks;
         pPeriphNode->cfgIdx++)
    {

#ifdef A2B_BCF_FROM_SOC_EEPROM
        /* Read the config block header bytes */
        /* [Two byte internal EEPROM address] */
        wBuf[0] = (a2b_UInt8)(pPeriphNode->addr >> 8u);
        wBuf[1] = (a2b_UInt8)(pPeriphNode->addr & 0xFFu);
        status  = a2b_pal_I2cWriteReadFunc( ecb->palEcb.i2chnd, A2B_I2C_EEPROM_ADDR,
                                          2u,  wBuf,
                                          3u,  &aDataBuffer[0u] );
#else
        status  = a2b_pal_FileRead( ecb->palEcb.fp, pPeriphNode->addr,
                                          3u,  &aDataBuffer[0u] );
#endif
        if(status != 0u)
        {
        	nReturn = 0xFFu;
			return (nReturn);
        }

        pPeriphNode->addr += 3u;
        cfgType = (aDataBuffer[0u] >> 4u);
        cfgCrc  = aDataBuffer[2];

        payloadLen = (a2b_UInt16)((a2b_UInt16)(((a2b_UInt16)aDataBuffer[0u]) << (a2b_UInt16)8u) |
                       ((a2b_UInt16)aDataBuffer[1])) & (a2b_UInt16)0xFFFu;
        payloadDataLen = payloadLen;

        /* Read the payload if needed */
        if ( (a2b_UInt8)A2B_DEALY_OP == cfgType )
        {
            usleep(payloadLen * 1000);
        }
        else
        {
        	/* The cfgCrc is byte[2] which for this message
			 * it equates to the addr/reg
			 */
        	regAddr = cfgCrc;
#ifdef A2B_BCF_FROM_SOC_EEPROM
            /* Read the payload */
            wBuf[0] = (a2b_UInt8)(pPeriphNode->addr >> 8u);
            wBuf[1] = (a2b_UInt8)(pPeriphNode->addr & 0xFFu);
            status  = a2b_pal_I2cWriteReadFunc( ecb->palEcb.i2chnd,
            								  	  A2B_I2C_EEPROM_ADDR,
												  2u, wBuf,
												  payloadLen,
												  &aDataBuffer[0u] );
#else

            status  = a2b_pal_FileRead( ecb->palEcb.fp,
            									  pPeriphNode->addr,
												  payloadLen,
												  &aDataBuffer[0u] );
#endif

            if(status != 0u)
			{
				nReturn = 0xFFu;
				return (nReturn);
			}

	        /* Write to the peripheral */
	        status = a2b_pal_I2cWriteFunc( ecb->palEcb.i2chnd,
	                                     (a2b_UInt16)regAddr,
										 payloadDataLen,
										 &aDataBuffer[0u] );
	        if(status != 0u)
			{
				nReturn = 0xFFu;
				return (nReturn);
			}

	        pPeriphNode->addr += payloadLen;
        }
    }

#else
    psPeriConfig = ecb->palEcb.pAudioHostDeviceConfig;
    nReturn = 0u;
    /* Check with exported peripheral configuration */
    for(nIndex  = 0u; nIndex < (a2b_UInt8)psPeriConfig->nNumConfig; nIndex++)
    {
        psDeviceConfig = &psPeriConfig->aDeviceConfig[nIndex];
        if(psDeviceConfig->bPostDiscCfg == 0)
        {
        	nReturn |= (a2b_UInt8)adi_a2b_AudioHostConfig(&(ecb->palEcb), psDeviceConfig);
        }
    }
#endif
    return(nReturn);

}

/*****************************************************************************/
/*!
@brief  This API Initializes the audio sub-system.

@param [in]:ecb  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_Handle a2b_pal_AudioOpenFunc(void)
{
    /* A2B feature EEPROM processing */
#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined (A2B_BCF_FROM_FILE_IO)
	return (void *) 1;
#else
   return (pPalEcb->pAudioHostDeviceConfig);
#endif
}

/*****************************************************************************/
/*!
@brief  This API configures the audio sub-system based on the Master
        TDM settigns

@param [in]:ecb  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_AudioConfigFunc(a2b_Handle hnd,
                                a2b_TdmSettings* tdmSettings)
{
    a2b_UInt8 nReturn=0;
#ifndef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
    a2b_UInt8 nIndex;
    ADI_A2B_PERI_DEVICE_CONFIG* psDeviceConfig;
    ADI_A2B_NODE_PERICONFIG *psPeriConfig;
	A2B_UNUSED(tdmSettings);

    psPeriConfig = (ADI_A2B_NODE_PERICONFIG*)hnd;
    nReturn = 0u;
    /* Check with exported peripheral configuration */
    for(nIndex  = 0u; nIndex < (a2b_UInt8)psPeriConfig->nNumConfig; nIndex++)
    {
        psDeviceConfig = &psPeriConfig->aDeviceConfig[nIndex];
        if(psDeviceConfig->bPostDiscCfg == 1u)
        {
        	nReturn = (a2b_UInt8)adi_a2b_AudioHostConfig(pPalEcb, psDeviceConfig);
        }
    }
#endif
	return (a2b_HResult)nReturn;
}

/*****************************************************************************/
/*!
@brief  This API Initializes the audio sub-system.

@param [in]:hnd  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_AudioCloseFunc(a2b_Handle hnd)
{
	A2B_UNUSED(hnd);
	return 0;
}

/*****************************************************************************/
/*!
@brief  This API Initializes the audio sub-system.

@param [in]:ecb  - PAL ECB structure.


@return Return code
        -1: Failure
        -0: Success

\note I2C and TWI terms are used  interchangeably

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_AudioShutdownFunc(A2B_ECB* ecb)
{
	A2B_UNUSED(ecb);
	return 0;
}

/*****************************************************************************/
/*!
@brief  This API starts audio processing (to be called from application directly )

@param [in]:


@return Return code
        -1: Failure
        -0: Success

\note

*/
/*****************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult adi_a2b_EnableAudioHost()
{
	return(A2B_RESULT_SUCCESS);
}
/****************************************************************************/
/*!
    @brief          This function configures devices connected to slave node
                    through remote I2C

    @param [in]     pNode                   Pointer to A2B node
    @param [in]     psDeviceConfig          Pointer to peripheral device configuration structure

    @return          Return code
                    - 0: Success
                    - 1: Failure
*/
/********************************************************************************/
A2B_PAL_L3_CODE
static a2b_UInt32 adi_a2b_AudioHostConfig(a2b_PalEcb*  palEcb, ADI_A2B_PERI_DEVICE_CONFIG* psDeviceConfig)
{
	A2B_PAL_L3_DATA
    static a2b_UInt8 aDataWriteReadBuf[4u];

    a2b_UInt32 nReturn = 0u;
    ADI_A2B_PERI_CONFIG_UNIT* pOPUnit;
    a2b_UInt8 nIndex, nIndex1;
    a2b_UInt32 nNumOpUnits;
    a2b_UInt32 nDelayVal;
    a2b_Int16 nodeAddr;

    nNumOpUnits = psDeviceConfig->nNumPeriConfigUnit;

    for(nIndex= 0u ; nIndex < nNumOpUnits ; nIndex++ )
    {
        pOPUnit = &psDeviceConfig->paPeriConfigUnit[nIndex];
        /* Operation code*/
        switch(pOPUnit->eOpCode)
        {
           /* write */
            case 0u:
                    adi_a2b_Concat_Addr_Data(&aDataBuffer[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr, 0);
            	    memcpy(&aDataBuffer[pOPUnit->nAddrWidth], pOPUnit->paConfigData, pOPUnit->nDataCount);

            	    a2b_pal_I2cWriteFunc(palEcb->i2chnd, (a2b_UInt16)psDeviceConfig->nDeviceAddress,
            	    		(pOPUnit->nAddrWidth + pOPUnit->nDataCount), &aDataBuffer[0u]);
                    break;
            /* read */
            case 1u:
            	    adi_a2b_Concat_Addr_Data(&aDataWriteReadBuf[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr, 0);
            	    a2b_pal_I2cWriteReadFunc(palEcb->i2chnd, (a2b_UInt16)psDeviceConfig->nDeviceAddress,
            	    		pOPUnit->nAddrWidth, &aDataWriteReadBuf[0u],
							pOPUnit->nDataCount, &aDataBuffer[0u]);

                    break;
            /* delay */
            case 2u:nDelayVal = 0u;
			for(nIndex1 = 0u; nIndex1 < pOPUnit->nDataCount; nIndex1++)
			{
				nDelayVal = (pOPUnit->paConfigData[nIndex1] << (8u * nIndex1)) | nDelayVal;
			}
            usleep(nDelayVal * 1000);
            break;


            default: break;

        }

        if(nReturn !=0u)
        {
            break;
        }
    }

    return(nReturn);
}

/****************************************************************************/
/*!
    @brief          This function returns the Version of the A2B Stack Software

    @param [in]     major                   Major Version
    @param [in]     minor                   Minor Version
    @param [in]     release           Release number of Software

    @return          Return code
                    - 0: Success
                    - 1: Failure
*/
/********************************************************************************/
A2B_PAL_L3_CODE
void a2b_pal_infoGetVersion(a2b_UInt32* major,
        a2b_UInt32* minor,
        a2b_UInt32* release)
{
	*major = 0x1u;
	*minor = 0x0u;
	*release = 0x10000000u;
}

/****************************************************************************/
/*!
    @brief          This function returns the Version of the A2B Stack Software

    @param [in]     major                   Major Version
    @param [in]     minor                   Minor Version
    @param [in]     release           Release number of Software

    @return          Return code
                    - 0: Success
                    - 1: Failure
*/
/********************************************************************************/
A2B_PAL_L3_CODE
void a2b_pal_infoGetBuild(a2b_UInt32* buildNum,
        const a2b_Char** const buildDate,
        const a2b_Char** const buildOwner,
        const a2b_Char** const buildSrcRev,
        const a2b_Char** const buildHost)
{
	*buildNum = 0x20000000u;
}

#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
/*!****************************************************************************
*
*  \b              a2b_pal_logInit
*
*  <b> API Details: </b><br>
*  This routine is called to do initialization the log subsystem
*  during the stack allocation process.
*
*
*
*  \param          [in]    ecb      The environment control block (ecb) for
*                                   this platform.
*
*  \pre            None
*
*  \post           None
*
*  \return         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_logInit(A2B_ECB*   ecb)
{
	a2b_UInt8 i = 0u;
	for(i=0; i<(A2B_TOTAL_LOG_CH); i++)
	{
		ecb->palEcb.oLogConfig[i].fd = A2B_INVALID_FD;
		ecb->palEcb.oLogConfig[i].inUse = false;
	}
	return A2B_RESULT_SUCCESS;
}

/*!****************************************************************************
*
*  \b              a2b_pal_logShutdown
*
*  <b> API Details: </b><br>
*  This routine is called to shutdown the log subsystem
*  during the stack destroy process.  This routine is called immediately
*  after the a2b_pal_logClose (assuming the close was successful).
*
*
*  \param          [in]    ecb      The environment control block (ecb) for
*                                   this platform.
*
*  \pre            None
*
*  \post           None
*
*  \return         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_logShutdown(A2B_ECB* ecb)
{
    A2B_UNUSED(ecb);
    return A2B_RESULT_SUCCESS;
} /* pal_logShutdown */


/*!****************************************************************************
*
*  \b              a2b_pal_logOpen
*
*  This routine is called to do post-initialization the log subsystem
*  during the stack allocation process.  This routine is called immediately
*  after the pal_logInit (assuming the init was successful).
*
*
*
*  \param          [in]    ecb      The environment control block (ecb) for
*                                   this platform.
*
*  \pre            None
*
*  \post           None
*
*  \return         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_Handle a2b_pal_logOpen(const a2b_Char* url)
{
	a2b_UInt8 i = 0u;
	a2b_Handle nHandle = 0U;

	if ( A2B_NULL != url )
	{
		for(i=0; i<(A2B_TOTAL_LOG_CH); i++)
		{
			if(!pPalEcb->oLogConfig[i].inUse)
			{
				pPalEcb->oLogConfig[i].fd = (a2b_UInt64)fopen(url, "w");
				nHandle = (a2b_Handle)&pPalEcb->oLogConfig[i];
				pPalEcb->oLogConfig[i].inUse = true;
				break;
			}
		}
	}

	return nHandle;
}

/*!****************************************************************************
*
*  \b              a2b_pal_logClose
*
*  This routine is called to de-initialization the log subsystem
*  during the stack destroy process.
*
*
*
*  \param          [in]    ecb      The environment control block (ecb) for
*                                   this platform.
*
*  \pre            None
*
*  \post           None
*
*  \return         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_logClose(a2b_Handle  hnd)
{
	a2b_UInt8 i = 0u;
	a2b_HResult nResult = 0xFFFFFFFFU;
	A2B_LOG_INFO *pLogInfo;

	if ( A2B_NULL != hnd )
	{
		pLogInfo = (A2B_LOG_INFO *)hnd;

		fclose((FILE *)pLogInfo->fd);
		pLogInfo->fd = A2B_INVALID_FD;
		pLogInfo->inUse = false;
		nResult = A2B_RESULT_SUCCESS;
	}

	return nResult;
}

/*!****************************************************************************
*
*  \b              a2b_pal_logWrite
*
*  <b> API Details: </b><br>
*  This routine writes to a log channel.
*
*
*  \param          [in]    hnd      The handle returned from pal_logOpen
*
*  \param          [in]    msg      NULL terminated string to log
*
*  \pre            None
*
*  \post           None
*
*  \return         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L1_CODE
a2b_HResult a2b_pal_logWrite(
    a2b_Handle      hnd,
    const a2b_Char* msg)
{
	a2b_HResult nResult = 0xFFFFFFFFu;
	a2b_UInt32 msg_len = 0u;
	A2B_LOG_INFO *pLogInfo;
	a2b_Char* newline = "\n";

	if ( (A2B_NULL != hnd) && (A2B_NULL != msg) )
	{
		pLogInfo = (A2B_LOG_INFO *)hnd;
		msg_len = a2b_strlen(msg) * sizeof(a2b_Char);
		fwrite(msg, 1, msg_len, (FILE *)pLogInfo->fd);
		fwrite(newline, 1, 1, (FILE *)pLogInfo->fd);
		nResult = A2B_RESULT_SUCCESS;
		(void)fflush((FILE *)pLogInfo->fd);
	}
	return nResult;
}
#endif

#if !defined(A2B_FEATURE_MEMORY_MANAGER)
 /*!****************************************************************************
*
*  a2b_pal_memMgrInit
*
*  This routine is called to do initialization required by the memory manager
*  service during the stack [private] allocation process.
*
*  return:         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_memMgrInit(A2B_ECB *ecb)
{
    A2B_UNUSED( ecb );
    return A2B_RESULT_SUCCESS;
}


/*!****************************************************************************
*
*  a2b_MemMgrOpenFunc
*
*  This routine opens a memory managed heap located at the specified address
*  and of the specified size. If the A2B stack's heap cannot be opened and
*  managed at the specified location (perhaps because the size is insufficient)
*  then the returned handle will be A2B_NULL. The managed heap will use
*  memory pools to avoid fragmentation within the managed region.
*
*  return:         If non-zero the open was considered sucessfully.  If A2B_NULL
*                  I2C initialization must have failed and the stack
*                  allocation will fail.  This returned handle will be passed
*                  back into the other PAL I2C (pal_memMgrXXX) API calls.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_Handle a2b_MemMgrOpenFunc(a2b_Byte *heap, a2b_UInt32  heapSize)
{
    A2B_UNUSED( heap );
    A2B_UNUSED( heapSize );

    /* MUST be a non-NULL value -- Dummy value */
    return (a2b_Handle)1;
}


/*!****************************************************************************
*
*  a2b_pal_memMgrMalloc
*
*  This routine is called to get/allocate a fixed amount of memory.
*
*  return:         Returns an aligned pointer to memory or A2B_NULL if memory
*                  could not be allocated.
*
******************************************************************************/
A2B_PAL_L1_CODE
void *a2b_pal_memMgrMalloc(a2b_Handle  hnd, a2b_UInt32  size)
{
    A2B_UNUSED( hnd );
    A2B_UNUSED( size );

    return A2B_NULL;
}


/*!****************************************************************************
*
*  a2b_pal_memMgrFree
*
*  This routine is called to free previously allocated memory.
*
*  return:         None
*
******************************************************************************/
A2B_PAL_L1_CODE
void a2b_pal_memMgrFree(a2b_Handle  hnd, void *p)
{
    A2B_UNUSED( hnd );
    A2B_UNUSED( p );
}


/*!****************************************************************************
*
*  a2b_pal_memMgrClose
*
*  This routine is called to de-initialization the memory management subsystem
*  during the stack destroy process.  All resources associated with the
*  heap are freed.
*
*  return:         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_memMgrClose(a2b_Handle  hnd)
{
    A2B_UNUSED( hnd );
    return A2B_RESULT_SUCCESS;
}


/*!****************************************************************************
*
*  a2b_pal_memMgrShutdown
*
*  This routine is called to shutdown the memory manager subsystem
*  during the stack destroy process.  This routine is called immediately
*  after the adi_a2b_pal_memMgrClose (assuming the close was successful).
*
*  return:         A status code that can be checked with the A2B_SUCCEEDED()
*                  or A2B_FAILED() for success or failure.
*
******************************************************************************/
A2B_PAL_L3_CODE
a2b_HResult a2b_pal_memMgrShutdown(A2B_ECB*    ecb)
{
    A2B_UNUSED( ecb );
    return A2B_RESULT_SUCCESS;
}
#endif /* A2B_FEATURE_MEMORY_MANAGER */
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


