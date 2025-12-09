/*******************************************************************************
Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
 ******************************************************************************
 * @file:    cpeng.c
 * @brief:   This is an implementation of APIs for post discovery CP handling
 * @version: $Revision: 8951 $
 * @date:    $Date: 2019-04-12 17:55:42 +0530 (Fri, 12 Apr 2019) $
 * Developed by: Automotive Software and Systems team, Bangalore, India
 *****************************************************************************/
/*! \addtogroup CP_Engine	CP Engine
 *  @{
 */

/** @defgroup CP_Engine	CP Engine
 *
 * This is an implementation of APIs for post discovery CP handling
 *
 */

/*! \addtogroup CP_Engine
 *  @{
 */

#include "stack_priv.h"
#include "spi_priv.h"
#include "stackctx.h"
#include "perieng.h"
#include "a2b/hwaccess.h"
#include "a2b/regdefs.h"
#include "a2b/error.h"
#include "a2b/pluginapi.h"
#include "a2b/cpeng.h"

/*============= D E F I N E S =============*/

#ifdef A2B_ENABLE_AD244xx_SUPPORT
/*============= L O C A L  P R O T O T Y P E S =============*/


static a2b_HResult a2b_CPStartAKE(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort);
static a2b_HResult a2b_CPEnc(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort, a2b_Bool bEnable);
static a2b_HResult a2b_CPMute(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort, a2b_Bool Enable);

/*============= C O D E =============*/

/*****************************************************************************/
/*!
@brief			This function takes the input from the application layer and
				calls the APIs based on the command

@param [in]     ctx	        Application context
@param [in]     nNode		Node number: -1 if the node is A2B Master and 0 for Slave 0
@param [in]     eCommand	CP command to be executed(AKE, Enc, Mute)
@param [in]		nPort 		CP Engine Port number

@return			a2b_HResult
 */
/*****************************************************************************/

a2b_HResult a2b_CPEventTrig(struct a2b_StackContext* ctx, a2b_Int16  nNode, ADI_A2B_CP_CMD eCommand, a2b_Byte nPort)
{
	a2b_HResult eRet = 0;

	switch(eCommand){
	case A2B_CP_START_AKE:
		eRet =  a2b_CPStartAKE(ctx, nNode, nPort);
		break;
	case A2B_CP_START_ENCRYPTION:
		eRet =	a2b_CPEnc(ctx, nNode, nPort, 1);		/*Enable encryption*/
		break;
	case A2B_CP_STOP_ENCRYPTION:
		eRet =	a2b_CPEnc(ctx, nNode, nPort, 0);		/*Disable encryption*/
		break;
	case A2B_CP_MUTE:
		eRet =	a2b_CPMute(ctx, nNode, nPort, 1);		/*Enable mute*/
		break;
	case A2B_CP_PORT_UNMUTE:
		eRet =	a2b_CPMute(ctx, nNode, nPort, 0);		/*Disable mute*/
		break;
	default:
		break;
	}

	return (eRet);

}


/*****************************************************************************/
/*!
@brief			This function starts the Authentication based on the port number


@param [in]     ctx	        Application context
@param [in]     nNode		Node number: -1 if the node is A2B Master and 0 for Slave 0
@param [in]		nPort 		CP Engine Port number

@return			a2b_HResult
 */
/*****************************************************************************/


static a2b_HResult a2b_CPStartAKE(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort)
{
	a2b_HResult eRet = 0;
	a2b_UInt16 nRegAddr;
	a2b_Byte wBuf[4];

	nRegAddr = ((A2B_REG_244x_TXPORT_CTL2) + (nPort * A2B_REG_PORT_ADDR_OFFSET));

	wBuf[0] = ((nRegAddr & 0xFF00u) >> 8u);
	wBuf[1] = (nRegAddr  & 0xFFu);
	wBuf[2] = A2B_REG_244x_AKE_START;

	eRet = a2b_CPregWrite(ctx, nNodeAddr, 3, wBuf);
	return eRet;
}

/*****************************************************************************/
/*!
@brief			Function to Enable or disable encryption on the port number specified


@param [in]     ctx	        Application context
@param [in]     nNode		Node number: -1 if the node is A2B Master and 0 for Slave 0
@param [in]		nPort 		CP Engine Port number
@param [in]		bEnable		Enable/Disable encryption (1 - Enable, 0 - Disable)

@return			a2b_HResult
 */
/*****************************************************************************/
static a2b_HResult a2b_CPEnc(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort, a2b_Bool bEnable)
{
	a2b_HResult eRet = 0;
	a2b_UInt16 nRegAddr;
	a2b_Byte wBuf[4];
	a2b_Byte nRegVal;
	if(nPort < A2B_MAX_TXPORTS)
	{
		nRegAddr = ((A2B_REG_244x_TXPORT_CTL1) + (nPort * A2B_REG_PORT_ADDR_OFFSET));
	}

	wBuf[0] = ((nRegAddr & 0xFF00u) >> 8u);
	wBuf[1] = (nRegAddr  & 0xFFu);

	/*Value present in the register is to be read before the Enc_Enable bit is set*/

	eRet = a2b_CPregWriteRead(ctx, nNodeAddr, 2, wBuf, 1, &nRegVal);

	if(bEnable)
	{
		wBuf[2] = nRegVal |  (A2B_REG_244x_ENC_ENABLE);
	}
	else
	{
		wBuf[2] = nRegVal & (~(A2B_REG_244x_ENC_ENABLE))  ;
	}


	eRet = a2b_CPregWrite(ctx, nNodeAddr, 3, wBuf);

	return eRet;
}


/*****************************************************************************/
/*!
@brief			Function to Enable or disable mute for the port number specified


@param [in]     ctx	        Application context
@param [in]     nNode		Node number: -1 if the node is A2B Master and 0 for Slave 0
@param [in]		nPort 		CP Engine Port number
@param [in]		bEnable		Enable/Disable encryption (1 - Enable, 0 - Disable)

@return			a2b_HResult
 */
/*****************************************************************************/

static a2b_HResult a2b_CPMute(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort, a2b_Bool bEnable)
{
	a2b_HResult eRet = 0;
	a2b_UInt16 nRegAddr;
	a2b_Byte wBuf[4];
	a2b_Byte nRegVal;

	if(nPort < A2B_MAX_TXPORTS)
	{
		nRegAddr = ((A2B_REG_244x_TXPORT_CTL1) + (nPort * A2B_REG_PORT_ADDR_OFFSET));
	}
	else
	{
		nRegAddr = ((A2B_REG_244x_RXPORT_CTL1) + ((nPort - A2B_MAX_TXPORTS) * A2B_REG_PORT_ADDR_OFFSET));
	}

	wBuf[0] = ((nRegAddr & 0xFF00u) >> 8u);
	wBuf[1] = (nRegAddr  & 0xFFu);
	/*Value present in the register is read before the mute value is written*/
	eRet = a2b_CPregWriteRead(ctx, nNodeAddr, 2, wBuf, 1, &nRegVal);

	if(bEnable)
		wBuf[2] = nRegVal | A2B_REG_244x_MUTE_ENABLE;
	else
		wBuf[2] = nRegVal & (~(A2B_REG_244x_MUTE_ENABLE));

	eRet = a2b_CPregWrite(ctx, nNodeAddr, 3, wBuf);
	return eRet;
}



/*****************************************************************************/
/*!
@brief			This function gives the authentication, encryption and mute status of the specified port number

@param [in]     ctx				Application context
@param [in] 	nNode			Node number: -1 if the node is A2B Master and 0 for Slave 0
@param [in]		nPort 			CP Port number
@param [in]		pCPPortStat		Pointer to a structure containing status

@return			a2b_HResult
 */
/*****************************************************************************/

a2b_HResult a2b_getCPPortStat(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort, a2b_CpPortStat* pCPPortStat)
{
	a2b_Byte nBaseAddr;
	a2b_UInt16 nRegAddr;
	a2b_Byte rRdBuf;
	a2b_HResult eRet = 0;
	a2b_Byte wBuf[4];
	a2b_UInt8 nMuteMask, nAuthMask, nEncMask;

	if(nPort < A2B_MAX_TXPORTS)
	{
		nBaseAddr = A2B_TXPORT_STAT;
	}
	else if( (nPort >= A2B_MAX_TXPORTS) && (nPort < (A2B_MAX_RXPORTS + A2B_MAX_TXPORTS)) )
	{
		nBaseAddr = A2B_RXPORT_STAT;
	}
	else
	{
		return ADI_A2B_CP_FAILED;
	}
	wBuf[0] = nPort;
	wBuf[1] = nBaseAddr;
	eRet = a2b_CPregWriteRead(ctx, nNodeAddr, 2, wBuf, 1, &rRdBuf);
	if(A2B_SUCCEEDED(eRet))
	{
		if(nPort < A2B_MAX_TXPORTS )
		{
			pCPPortStat->authStat = (a2b_Byte)((rRdBuf & A2B_CP_TX_AUTH_STAT_MASK) == A2B_CP_TX_AUTH_STAT_MASK) ;
			pCPPortStat->muteStat = (a2b_Byte)((rRdBuf & A2B_CP_TX_MUTE_STAT_MASK) == A2B_CP_TX_MUTE_STAT_MASK);
			pCPPortStat->encStat  = (a2b_Byte)((rRdBuf & A2B_CP_TX_ENC_STAT_MASK) == A2B_CP_TX_ENC_STAT_MASK);

		}
		else
		{
			pCPPortStat->authStat = (a2b_Byte)((rRdBuf & A2B_CP_RX_AUTH_STAT_MASK) == A2B_CP_RX_AUTH_STAT_MASK) ;
			pCPPortStat->muteStat = (a2b_Byte)((rRdBuf & A2B_CP_RX_MUTE_STAT_MASK) == A2B_CP_RX_MUTE_STAT_MASK) ;
			pCPPortStat->encStat  = (a2b_Byte)((rRdBuf & A2B_CP_RX_ENC_STAT_MASK) == A2B_CP_RX_ENC_STAT_MASK);
		}
	}

	return (eRet);

}

/*****************************************************************************/
/*!
@brief			 API to return the read value of the CP engine's register


@param [in]      ctx     		The stack context associated with the write.
@param [in]		 nNodeAddr     	Node number: -1 if the node is A2B Master and 0 for Slave 0
@param [in]      nWrite  		The number of bytes to write.
@param [in]      wBuf    		A buffer containing the data to write. The buffer is of size 'nWrite' bytes.
@param [in]		 nRdCount		Number of bytes to be read
@param [in]	 	 pRdBuf		Buffer to hold the value that is read
@return			 a2b_HResult

@note			 Return error if the access interface is SPI
 */
/******************************************************************************/
a2b_HResult a2b_CPregWriteRead(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_UInt16 nWrCount, a2b_Byte* wBuf,  a2b_UInt16 nRdCount, a2b_Byte* pRdBuf )
{

	ADI_A2B_CP_RESULT eRet = ADI_A2B_CP_SUCCESS;
	a2b_UInt16 nI2cAddr = ctx->ccb.app.aCPI2CAddr[(nNodeAddr + 1u)];
	a2b_Byte wSpiBuf[8];
	a2b_UInt16 i = 0;
	a2b_Byte nRemBuf[256];


	(void)memset(nRemBuf, 0u, 256u);

	if (ctx->stk->accessInterface == A2B_ACCESS_I2C)
	{

		eRet = a2b_i2cPeriphWriteRead(ctx, nNodeAddr, nI2cAddr, nWrCount, wBuf, nRdCount, pRdBuf);
	}
	else
	{


		if(nNodeAddr == A2B_NODEADDR_MASTER)
		{
			/*Write before read*/
			wSpiBuf[0] = CP_SPI_READREG;
			(void)a2b_memcpy(&wSpiBuf[1], wBuf, nWrCount);
			wSpiBuf[nWrCount+1] = 0x00;
			eRet = a2b_spiPeriphWriteRead(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST, nI2cAddr, A2B_CP_SLAVE_SELECT, nWrCount + 2, wSpiBuf,  4u, pRdBuf);


			/*Command to do a FIFO read*/
			wSpiBuf[0] = CP_SPI_FIFOREAD;
			wSpiBuf[1] = 0x00;
			wSpiBuf[2] = 0x00;
			wSpiBuf[3] = 0x00;
			eRet = a2b_spiPeriphWriteRead(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST, nI2cAddr, A2B_CP_SLAVE_SELECT, 0x04, wSpiBuf, 4u, pRdBuf);

		}
		else
		{
			(void)a2b_memcpy(&wSpiBuf[i++], wBuf, nWrCount);
			eRet = a2b_spiPeriphWriteRead(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST, nI2cAddr, A2B_SPI_SLV_SEL, nWrCount + (i - 1), wSpiBuf, nRdCount, pRdBuf);
		}
	}
	return eRet;
}

/*****************************************************************************/
/*!
@brief			 API to write the value into CP engine's register


@param [in]      ctx     		The stack context associated with the write.
@param [in]		 nNodeAddr     	Node number: -1 if the node is A2B Master and 0 for Slave 0
@param [in]      nWrite  		The number of bytes to write.
@param [in]      wBuf    		A buffer containing the data to write. The buffer is of size 'nWrite' bytes.
@return			 a2b_HResult

@note			 Return error if the access interface is SPI
 */
/******************************************************************************/

a2b_HResult a2b_CPregWrite(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_UInt16 nWrCount, a2b_Byte* wBuf)
{

	ADI_A2B_CP_RESULT eRet = ADI_A2B_CP_SUCCESS;
	a2b_Byte wSpiBuf[256];
	a2b_UInt16 i = 0;
	a2b_UInt16 nI2cAddr = ctx->ccb.app.aCPI2CAddr[(nNodeAddr + 1u)];
	a2b_UInt8 nReadBuf[4];
	a2b_Byte nRemByte;
	a2b_UInt16 nWrSize = nWrCount - 2u;
	a2b_Byte nRemBuf[256];
	a2b_UInt8 aRdBuf[4];
	a2b_Byte bulkCfg = 0;
	a2b_UInt8 nExit = SPI_DMA_EXIT_32_BYTES;	/*Number of remaining bytes required to exit the bulkmode*/

	(void)memset(nRemBuf, 0u, 256u);

	if (ctx->stk->accessInterface == A2B_ACCESS_I2C)
	{
		eRet = a2b_i2cPeriphWrite(ctx, nNodeAddr, nI2cAddr, nWrCount, wBuf);
	}
	else
	{
		if(nNodeAddr == A2B_NODEADDR_MASTER)	/*Write to slave nodes' CPE is through remote SPI to I2C writes*/
		{

			if ((wBuf[0] == CP_SHADOW_REGISTER_SPACE))
			{
				if(nWrSize < 32)
				{
					bulkCfg = (a2b_Byte)SPI_DMA_32_BYTES;
					GET_REM_BYTES(nRemByte, 32, nWrCount);
					nExit = SPI_DMA_EXIT_32_BYTES;		/*Number of remaining bytes required to exit the bulk mode*/

				}
				else if(nWrSize >= 32 && nWrSize < 64)
				{
					bulkCfg = (a2b_Byte)SPI_DMA_64_BYTES;
					GET_REM_BYTES(nRemByte, 64, nWrCount);
					nExit = SPI_DMA_EXIT_64_BYTES;	    /*Number of remaining bytes required to exit the bulk mode*/
				}
				else if(nWrSize >= 64 && nWrSize < 128)
				{
					bulkCfg = (a2b_Byte)SPI_DMA_128_BYTES;
					GET_REM_BYTES(nRemByte, 128, nWrCount);
					nExit = SPI_DMA_EXIT_128_BYTES;	/*Number of remaining bytes required to exit the bulk mode*/
				}
				else if(nWrSize >= 128 && nWrSize < 256)
				{
					bulkCfg = (a2b_Byte)SPI_DMA_256_BYTES;
					GET_REM_BYTES(nRemByte, 256, nWrCount);
					nExit = SPI_DMA_EXIT_256_BYTES;	/*Number of remaining bytes required to exit the bulk mode*/
				}
				else
				{
					return (eRet = ADI_A2B_CP_FAILED);
				}


				wSpiBuf[0] = CP_SPI_ENTERBULK;
				wSpiBuf[1] = bulkCfg;
				wSpiBuf[2] = 0x00;
				wSpiBuf[3] = 0x00;
				eRet = a2b_spiPeriphWrite(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_WRITE, nI2cAddr, A2B_CP_SLAVE_SELECT, 4u, wSpiBuf );



				/*Write data into the shadow register space*/
				wSpiBuf[0] = CP_SPI_BULKWRITE;
				(void)a2b_memcpy(&wSpiBuf[1], wBuf, 2u);
				wSpiBuf[3] = (nWrCount - 2u);
				(void)a2b_memcpy(&wSpiBuf[4], &wBuf[2], (nWrCount - 2u)); /*Copy values to be written*/
				(void)a2b_memcpy(&wSpiBuf[(nWrCount + 2)], &nRemBuf[0], nRemByte);
				eRet = (ADI_A2B_CP_RESULT)a2b_spiPeriphWrite(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_WRITE, nI2cAddr, A2B_CP_SLAVE_SELECT, (nWrCount + nRemByte + 2u), wSpiBuf);




				/*Exit Bulk mode*/
				wSpiBuf[0] = CP_SPI_EXITBULK;
				(void)a2b_memcpy(&wSpiBuf[1], &nRemBuf[0], nExit);
				eRet = a2b_spiPeriphWrite(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_WRITE, nI2cAddr, A2B_CP_SLAVE_SELECT, (nExit + 1), wSpiBuf);


				if(wBuf[1] != CP_REG_SRM)
				{
					/*Reset the shadow register offset before writing into a register*/
					wSpiBuf[0] = CP_SPI_RESETOFFSET;
					(void)memcpy(&wSpiBuf[1u], nRemBuf, nExit );
					eRet = a2b_spiPeriphWriteRead(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST, nI2cAddr, A2B_CP_SLAVE_SELECT, (nExit + 1), wSpiBuf, 4u, &aRdBuf[0]);

				}
				else
				{
					/*If the register is SRM, do not reset the offset*/
				}


			}
			else
			{
				wSpiBuf[0] = CP_SPI_WRITEREG;
				(void)a2b_memcpy(&wSpiBuf[1u], wBuf, nWrCount);
				eRet = a2b_spiPeriphWrite(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_WRITE, nI2cAddr, A2B_CP_SLAVE_SELECT, nWrCount + 1u, wSpiBuf);
			}
		}
		else
		{
			(void)a2b_memcpy(&wSpiBuf[0], wBuf, nWrCount);
			eRet = a2b_spiPeriphWrite(ctx, nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_WRITE, nI2cAddr, A2B_SPI_SLV_SEL, nWrCount, wSpiBuf);
		}

	}
	return eRet;
}

#endif		/*A2B_ENABLE_AD244xx_SUPPORT*/

/**
 * @}
 */



/*
 *
 * EOF: $URL$
 *
 */




