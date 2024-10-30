/*******************************************************************************
Copyright (c) 2017 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
*******************************************************************************

   Name       : a2bapp_bf.c

   Description: This file demonstrates usage of I2C commandlist exported from SigmaStudio for system bring up

   Functions  : main()

   Prepared &
   Reviewed by: Automotive Software and Systems team,
                IPDC, Analog Devices,  Bangalore, India

   @version: $Revision: 3626 $
   @date: $Date: 2016-11-027 14:04:13 +0530 $

******************************************************************************/

/*============= I N C L U D E S =============*/

#include "adi_a2b_i2c_commandlist.h"

/*============= D E F I N E S =============*/

#define I2C_DEV_PATH "/dev/i2c-13" // Adjust as necessary
#define I2C_MASTER_ADDR 0x68
#define I2C_SLAVE_ADDR 0x69

#define I2C_TIMEOUT_DEFAULT             700             /*!< timeout: 700ms*/
#define I2C_RETRY_DEFAULT                 3             /*!< retry times: 3 */

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

typedef uint8_t             a2b_UInt8;
typedef uint16_t             a2b_UInt16;
typedef uint32_t            a2b_UInt32;

/*============= D A T A =============*/

int32_t arrayAddrs[2];

/*============= C O D E =============*/

void parseXML(const char *path);
static int32_t adi_a2b_NetworkSetup(void);
static void adi_a2b_Concat_Addr_Data(a2b_UInt8 pDstBuf[], a2b_UInt32 nAddrwidth, a2b_UInt32 nAddr);
int32_t adi_a2b_SystemInit(void);
void  adi_a2b_Delay(a2b_UInt32 nTime);
int32_t adi_a2b_I2COpen(a2b_UInt16 nDeviceAddress);
int32_t adi_a2b_I2CWrite(a2b_UInt16 nDeviceAddress, a2b_UInt16 nWrite, a2b_UInt8* wBuf);
int32_t adi_a2b_I2CWriteRead(a2b_UInt16 nDeviceAddress, a2b_UInt16 nWrite, a2b_UInt8* wBuf, a2b_UInt16 nRead, a2b_UInt8* rBuf);

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

int main(int argc, char *argv[])
{
	uint32_t nResult = 0;
	
	/* Begin adding your custom code here */
    if(argc == 2) {
        parseXML(argv[1]);
    }

	do
	{
		/* PAL call, open I2C driver */
		arrayAddrs[0] = adi_a2b_I2COpen(I2C_MASTER_ADDR);
		arrayAddrs[1] = adi_a2b_I2COpen(I2C_SLAVE_ADDR);

		/* Configure a2b system */
		nResult = adi_a2b_NetworkSetup();
		if (nResult != 0)
		{
			/* discovery failed */
			break;
		}

	}while(0);

	return 0;
}

void parseXML(const char *path)
{

}

/****************************************************************************/
/*!
 @brief          This function does A2B network discovery and the peripheral configuration
 @return          None

 */
/********************************************************************************/
int32_t adi_a2b_NetworkSetup()
{
	ADI_A2B_DISCOVERY_CONFIG* pOPUnit;
	uint32_t nIndex, nIndex1;
	int32_t status = 0;
	/* Maximum number of writes */
	static uint8_t aDataBuffer[6000];
	static uint8_t aDataWriteReadBuf[4u];
	uint32_t nDelayVal;

	/* Loop over all the configuration */
	for (nIndex = 0; nIndex < CONFIG_LEN; nIndex++)
	{
		pOPUnit = &gaA2BConfig[nIndex];
		/* Operation code*/
		switch (pOPUnit->eOpCode)
		{
			/* Write */
			case WRITE:
				adi_a2b_Concat_Addr_Data(&aDataBuffer[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
				(void)memcpy(&aDataBuffer[pOPUnit->nAddrWidth], pOPUnit->paConfigData, pOPUnit->nDataCount);
				/* PAL Call, replace with custom implementation  */
				status = adi_a2b_I2CWrite((uint16)pOPUnit->nDeviceAddr, (uint16)(pOPUnit->nAddrWidth + pOPUnit->nDataCount), &aDataBuffer[0u]);
				break;

				/* Read */
			case READ:
				(void)memset(&aDataBuffer[0u], 0u, pOPUnit->nDataCount);
				adi_a2b_Concat_Addr_Data(&aDataWriteReadBuf[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
				/* PAL Call, replace with custom implementation  */
				status = adi_a2b_I2CWriteRead((uint16)pOPUnit->nDeviceAddr, (uint16)pOPUnit->nAddrWidth, &aDataWriteReadBuf[0u], (uint16)pOPUnit->nDataCount, &aDataBuffer[0u]);
				break;

				/* Delay */
			case DELAY:
				nDelayVal = 0u;
				for(nIndex1 = 0u; nIndex1 < pOPUnit->nDataCount; nIndex1++)
				{
					nDelayVal = pOPUnit->paConfigData[nIndex1] | nDelayVal << 8u;
				}
				(void)adi_a2b_Delay(nDelayVal);
				break;

			default:
				break;

		}
		if (status != 0)
		{
			/* I2C read/write failed! No point in continuing! */
			break;
		}
	}

	return status;
}

/****************************************************************************/
/*!
 @brief          This function calculates reg value based on width and adds
 it to the data array

 @param [in]     pDstBuf               Pointer to destination array
 @param [in]     nAddrwidth            Data unpacking boundary(1 byte / 2 byte /4 byte )
 @param [in]     nAddr            	  Number of words to be copied

 @return          Return code
 - 0: Success
 - 1: Failure
 */
/********************************************************************************/
static void adi_a2b_Concat_Addr_Data(a2b_UInt8 pDstBuf[], a2b_UInt32 nAddrwidth, a2b_UInt32 nAddr)
{
	/* Store the read values in the place holder */
	switch (nAddrwidth)
	{ /* Byte */
		case 1u:
			pDstBuf[0u] = (a2b_UInt8)nAddr;
			break;
			/* 16 bit word*/
		case 2u:

			pDstBuf[0u] = (a2b_UInt8)(nAddr >> 8u);
			pDstBuf[1u] = (a2b_UInt8)(nAddr & 0xFFu);

			break;
			/* 24 bit word */
		case 3u:
			pDstBuf[0u] = (a2b_UInt8)((nAddr & 0xFF0000u) >> 16u);
			pDstBuf[1u] = (a2b_UInt8)((nAddr & 0xFF00u) >> 8u);
			pDstBuf[2u] = (a2b_UInt8)(nAddr & 0xFFu);
			break;

			/* 32 bit word */
		case 4u:
			pDstBuf[0u] = (a2b_UInt8)(nAddr >> 24u);
			pDstBuf[1u] = (a2b_UInt8)((nAddr & 0xFF0000u) >> 16u);
			pDstBuf[2u] = (a2b_UInt8)((nAddr & 0xFF00u) >> 8u);
			pDstBuf[3u] = (a2b_UInt8)(nAddr & 0xFFu);
			break;

		default:
			break;

	}
}

void adi_a2b_Delay(a2b_UInt32 nTime)
{
    usleep(nTime);
}

int32_t adi_a2b_I2COpen(a2b_UInt16 nDeviceAddress)
{
    int32_t fd;

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

    return fd;
}

int32_t adi_a2b_I2CWrite(a2b_UInt16 nDeviceAddress, a2b_UInt16 nWrite, a2b_UInt8* wBuf)
{
    int32_t nResult = 0, fd;

    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg msg;

    fd = nDeviceAddress == I2C_MASTER_ADDR ? arrayAddrs[0] : arrayAddrs[1];

    msg_rdwr.msgs = &msg;
    msg_rdwr.nmsgs = 1;

    msg.addr  = wBuf;
    msg.flags = 0;    //write
    msg.len   = nWrite;
    msg.buf = wBuf + 1;

    ret = ioctl(fd, I2C_RDWR, &msg_rdwr);
    if (ret < 0) {
        return -1;
    }

#ifdef A2B_PRINT_CONSOLE
    for (uint8_t i = 0; i < nWrite; i++) {
        printf(I2C_DEV_PATH" write device(%#x) reg=0x%02x %02d val=0x%02x ("PRINTF_BINARY_PATTERN_INT8") cnt=%d\n",
                addr, wBuf[0] + i, wBuf[0] + i, wBuf[i + 1], PRINTF_BYTE_TO_BINARY_INT8(wBuf[i + 1]), nWrite);
    }
#endif

    return nResult;
}

int32_t adi_a2b_I2CWriteRead(a2b_UInt16 nDeviceAddress, a2b_UInt16 nWrite, a2b_UInt8* wBuf, a2b_UInt16 nRead, a2b_UInt8* rBuf)
{
    int32_t nResult = 0, fd;

    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg msg[2];

    fd = nDeviceAddress == I2C_MASTER_ADDR ? arrayAddrs[0] : arrayAddrs[1];


    msg_rdwr.msgs = &msg;
    msg_rdwr.nmsgs = 2;

    msg[0].addr = wBuf;
    msg[0].buf = wBuf + 1;
    msg[0].len = nWrite - 1;
    msg[0].flags = 0;
    msg[1].addr = wBuf;
    msg[1].buf = rBuf;
    msg[1].len = nRead;
    msg[1].flags = I2C_M_RD;

    nResult = ioctl(fd, I2C_RDWR, &msg_rdwr);
    if (nResult < 0) {
        return -1;
    }

#ifdef A2B_PRINT_CONSOLE
    for (uint8_t i = 0; i < nRead; i++) {
        printf(I2C_DEV_PATH" write device(%#x) reg=0x%02x %02d val=0x%02x ("PRINTF_BINARY_PATTERN_INT8") cnt=%d\n",
                addr, wBuf[0] + i, wBuf[0] + i, rBuf[i], PRINTF_BYTE_TO_BINARY_INT8(rBuf[i]), nRead);
    }
#endif

    return nResult;
}
