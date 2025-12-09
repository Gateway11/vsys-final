/*=============================================================================
 *
 * Project: a2bstack
 *
 * Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
 * This software is subject to the terms and conditions of the license set 
 * forth in the project LICENSE file. Downloading, reproducing, distributing or 
 * otherwise using the software constitutes acceptance of the license. The 
 * software may not be used except as expressly authorized under the license.
 *
 *=============================================================================
 *
 * \file:   hwaccess.c
 * \author: Automotive Software Team, Bangalore
 * \brief:  Implements the stack wrapper functions to access I2C/SPI read/write interfaces.
 *
 *=============================================================================
 */

/*============================================================================*/
/** 
 * \ingroup  a2bstack_hwaccess
 * \defgroup a2bstack_hwaccess_priv          \<Private\>
 * \private 
 *  
 * This defines hardware access API's that are private to the stack.
 */
/*============================================================================*/

/*======================= I N C L U D E S =========================*/

#include "a2b/pal.h"
#include "a2b/util.h"
#include "a2b/trace.h"
#include "a2b/error.h"
#include "a2b/regdefs.h"
#include "a2b/defs.h"
#include "a2b/msgrtr.h"
#include "a2b/stringbuffer.h"
#include "a2b/seqchart.h"
#include "stack_priv.h"
#include "i2c_priv.h"
#include "a2b/spi.h"
#include "stackctx.h"
#include "utilmacros.h"
#include "a2b/timer.h"
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
#include "a2bplugin-slave/adi_a2b_periconfig.h"
#endif
#include "a2b/hwaccess.h"
/*======================= D E F I N E S ===========================*/
#define A2B_DELAY_FOR_NEWSTRUCT_IN_MS	        (1U)	/* Delay which should be applied after writing the CONTROL.NEWSTRUCT bit */
#define A2B_DELAY_FOR_NEWSTRUCT_SCENARIO	(5U)	/* Scenario information used while logging delay during command list generation */
#define A2B_OFFSET_DAT_SPICMDWDTH                (0u) /* Offset for SPI Command width from payload Byte 0 */
#define A2B_OFFSET_DAT_SPICMD                          (1u) /* Offset for SPI Command from payload Byte 0 */
#define A2B_OFFSET_DAT_ADDRINC                       (3u) /* Offset for Address Increment from payload Byte 0 */
/*============= D A T A =============*/
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
static a2b_UInt8 spiDataBuffer[64u];
static a2b_UInt8 ReadDataBuffer[256u];
#endif
/*
** Function Prototype section
*/
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
static a2b_UInt32 a2b_SpiCmdsAndPeriWrite(a2b_StackContext*  ctx, a2b_Int16 nNodeAddr, a2b_SpiCmd eSpiCmd, a2b_UInt16 regAddr, a2b_UInt32 nDataCount, a2b_UInt8* pDataBytes, a2b_UInt32 nMaxTransac);
#endif
/*!****************************************************************************
*  \ingroup         a2bstack_hwaccess
*
*  \b               a2b_regWrite
*
*  Writes bytes to the SPI/I2C device. This is an synchronous call and will block
*  until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    node    The A2B node address.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_regWrite(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16                  nWrite,
	void*                       wBuf)
{
	a2b_HResult 				status = A2B_RESULT_SUCCESS;
	struct a2b_StackContext* 	mCtx;

	/* Need a master plugin context to do I2C/SPI calls */
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
	if ( A2B_NULL == mCtx)
	{
		/* This should *never* happen */
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		/* Check hardware interface */
		if (mCtx->stk->accessInterface == A2B_ACCESS_I2C)
		{
			if (A2B_NODEADDR_MASTER == node)
			{
				status = a2b_i2cMasterWrite(mCtx, nWrite, wBuf);
			}
			else
			{
				status = a2b_i2cSlaveWrite(mCtx, node, nWrite, wBuf);
			}
		}
		else
		{
			if (A2B_NODEADDR_MASTER == node)
			{
				status = a2b_spiMasterWrite(mCtx, nWrite, wBuf);
			}
			else
			{
				status = a2b_spiSlaveWrite(mCtx, node, nWrite, wBuf);
			}
		}
	}

	return status;
}

/*!****************************************************************************
*  \ingroup         a2bstack_hwaccess
*
*  \b               a2b_simpleRegWrite
*
*  Writes bytes to the SPI/I2C device. This is an synchronous call and will block
*  until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    node    The A2B node address.
*
*  \param   [in]    nRegAddr  Register address
*
*  \param   [in]    nRegVal    Register value
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_simpleRegWrite(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16					nRegAddr,
	a2b_UInt8                   nRegVal
)
{
	a2b_HResult 				status = A2B_RESULT_SUCCESS;
	struct a2b_StackContext* 	mCtx;
	a2b_UInt8 wBuf[4];
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
	if ( A2B_NULL == mCtx)
	{
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		if(nRegAddr > 0xFFu)
		{
			wBuf[0] = A2B_REG_MMRPAGE;
			wBuf[1] = 0x01u;
			status = a2b_regWrite(mCtx, node, 2u, wBuf);
			wBuf[0] = nRegAddr & 0xFFu;
			wBuf[1] = nRegVal;
			status = a2b_regWrite(mCtx, node, 2u, wBuf);
			wBuf[0] = A2B_REG_MMRPAGE;
			wBuf[1] = 0x00u;
			status = a2b_regWrite(mCtx, node, 2u, wBuf);
		}
		else
		{
			wBuf[0] = nRegAddr & 0xFFu;
			wBuf[1] = nRegVal;
			status = a2b_regWrite(mCtx, node, 2u, wBuf);
		}
	}
	return status;
}
/*!****************************************************************************
*  \ingroup         a2bstack_hwaccess
*
*  \b               a2b_simpleRegRead
*
*  Writes bytes to the SPI/I2C device. This is an synchronous call and will block
*  until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    node    The A2B node address.
*
*  \param   [in]    nRegAddr  Register address.
*
*  \param   [in]    rVal    Register value.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_simpleRegRead(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16					nRegAddr,
	a2b_UInt8*                  rVal
)
{
	a2b_HResult 				status = A2B_RESULT_SUCCESS;
	struct a2b_StackContext* 	mCtx;
	a2b_UInt8 wBuf[4];
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);

	if ( A2B_NULL == mCtx)
	{
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		if(nRegAddr > 0xFFu)
		{
			wBuf[0] = A2B_REG_MMRPAGE;
			wBuf[1] = 0x01u;
			status = a2b_regWrite(mCtx, node, 2u, wBuf);
			wBuf[0] = nRegAddr & 0xFFu;
			status = a2b_regWriteRead(mCtx, node, 1u, wBuf, 1u, rVal);
			wBuf[0] = A2B_REG_MMRPAGE;
			wBuf[1] = 0x00u;
			status = a2b_regWrite(mCtx, node, 2u, wBuf);
		}
		else
		{
			wBuf[0] = nRegAddr & 0xFFu;
			status = a2b_regWriteRead(mCtx, node, 1u, wBuf, 1u, rVal);
		}
	}
	return status;
}


/*!****************************************************************************
*  \ingroup         a2bstack_hwaccess
*
*  \b               a2b_regBroadcastWrite
*
*  Broadcast writes bytes to the SPI/I2C device. This is an synchronous call and will block
*  until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_regBroadcastWrite(
	struct a2b_StackContext*    ctx,
	a2b_UInt16                  nWrite,
	void*                       wBuf)
{
	a2b_HResult 				status = A2B_RESULT_SUCCESS;
	struct a2b_StackContext* 	mCtx;

	/* Need a master plugin context to do I2C/SPI calls */
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
	if ( A2B_NULL == mCtx)
	{
		/* This should *never* happen */
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		/* Check hardware interface */
		if (mCtx->stk->accessInterface == A2B_ACCESS_I2C)
		{
			status = a2b_i2cSlaveBroadcastWrite(mCtx, nWrite, wBuf);
		}
		else
		{
			status = a2b_spiSlaveBroadcastWrite(mCtx, nWrite, wBuf);
		}
	}

	return status;
}

/*!****************************************************************************
*  \ingroup         a2bstack_hwaccess
*
*  \b               a2b_regWriteRead
*
*  Writes and then reads bytes from the SPI/I2C device. This is an synchronous call and
*  will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write/read.
*
*  \param   [in]    node    The A2B node address.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rBuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    The read buffer holds the contents of the read on success.
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
 A2B_DSO_PUBLIC a2b_HResult a2b_regWriteRead(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16                  nWrite,
	void*                       wBuf,
	a2b_UInt16                  nRead,
	void*                       rBuf)
{
	a2b_HResult 				status = A2B_RESULT_SUCCESS;
	struct a2b_StackContext* 	mCtx;

	/* Need a master plugin context to do I2C/SPI calls */
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
	if ( A2B_NULL == mCtx)
	{
		/* This should *never* happen */
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		/* Check hardware interface */
		if (mCtx->stk->accessInterface == A2B_ACCESS_I2C)
		{
			if (A2B_NODEADDR_MASTER == node)
			{
				status = a2b_i2cMasterWriteRead(mCtx, nWrite, wBuf, nRead, rBuf);
			}
			else
			{
				status = a2b_i2cSlaveWriteRead(mCtx, node, nWrite, wBuf, nRead, rBuf);
			}
		}
		else
		{
			/* SPI code  */
			if (A2B_NODEADDR_MASTER == node)
			{
				status = a2b_spiMasterWriteRead(mCtx, nWrite, wBuf, nRead, rBuf);
			}
			else
			{
				status = a2b_spiSlaveWriteRead(mCtx, node, nWrite, wBuf, nRead, rBuf);
			}
		}
	}

	return status;
}

 /* Function to check for busy */
 A2B_DSO_PUBLIC a2b_HResult a2b_CheckBusy(	struct a2b_StackContext* ctx, a2b_Bool *pBusy)
 {
	struct a2b_StackContext* 	mCtx;
	a2b_HResult 				status = A2B_RESULT_SUCCESS;

	/* Need a master plugin context to do I2C/SPI calls */
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);

	if ( A2B_NULL == mCtx)
	{
		/* This should *never* happen */
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		/* Check hardware interface */
		if (mCtx->stk->accessInterface == A2B_ACCESS_I2C)
		{
			*pBusy = A2B_FALSE;
		}
		else
		{
			*pBusy = a2b_spiCheckBusyStat(mCtx, A2B_SPI_SLV_SEL, 1u);
		}

	}

	return(status);
 }

 /*!****************************************************************************
 *  \ingroup         a2bstack_hwaccess_priv
 *
 *  \b               a2b_delayForNewStruct
 *
 *  This function shall be used to provide an active delay after writing the CONTROL.NEWSTRUCT bit  
 *
 *  \param   [in]    ctx     The stack context associated with the write/read.
 *
 *  \param   [in]    dscNodeAddr	Node Address
 *
 *  \pre     None
 *
 *  \post    None
 *
 *  \return  None
 *
 ******************************************************************************/
 A2B_DSO_PUBLIC void a2b_delayForNewStruct(struct a2b_StackContext* ctx, a2b_Int16 dscNodeAddr)
 {
	 a2b_ActiveDelay(ctx, A2B_DELAY_FOR_NEWSTRUCT_IN_MS);

#ifdef A2B_SS_STACK
	 if (ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc != A2B_NULL)
	 {
		 /* In Milli second*/
		 ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc(A2B_DELAY_FOR_NEWSTRUCT_IN_MS, dscNodeAddr, A2B_DELAY_FOR_NEWSTRUCT_SCENARIO);
	 }
#else
	 A2B_UNUSED(dscNodeAddr);
#endif
 }

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
 /****************************************************************************/
 /*!
     @brief     This function splits the a single SPI command and generate SPI sub commands.
                     It also writes the sub commands to peripheral

     @param [in]     plugin                       Pointer to A2B Slave Plugin
     @param [in]     nNodeAddr               Node address
     @param [in]     eSpiCmd                   SPI command
     @param [in]     regAddr                    I2C Address or SPI slave select
     @param [in]     nDataCount              Number of bytes to be writted
     @param [in]     pDataBytes               Payload buffer pointer
     @param [in]     nMaxTransac            Maximum number of bytes that can be transferred at a time.

     @return          Return code
                     - 0: Success
                     - 1: Failure
 */
 static a2b_UInt32 a2b_SpiCmdsAndPeriWrite(a2b_StackContext*  ctx, a2b_Int16 nNodeAddr, a2b_SpiCmd eSpiCmd, a2b_UInt16 regAddr, a2b_UInt32 nDataCount, a2b_UInt8* pDataBytes, a2b_UInt32 nMaxTransac)
 {
 	a2b_UInt32	nReturn = 0u, nDataSplitSizeInBytes = 0u, nNumberOfDataBlocks = 0u, nNumberOfLeftOutDataInBytes = 0u;
 	a2b_UInt32	nDataBlockIdx=0, nMaxDataSize, nRem, nActualDataSizeInBytes, nAddressOffset;
 	a2b_HResult status = A2B_RESULT_SUCCESS;
 	a2b_UInt8 nAddrIncrement = 0u;
 	a2b_UInt8 nDataWidth = 1u;
 	a2b_UInt8 nAddrWidth = 0u;
 	a2b_UInt8 nUserSpiCmdWidth = 0u;
 	a2b_UInt8 nPeriSpiCmd = 0u;
 	a2b_UInt32 nAddr = 0;
 	a2b_UInt8 *pBuf;

 	nAddrIncrement = pDataBytes[A2B_OFFSET_DAT_ADDRINC];
 	/* For splitting purpose */
 	if (nAddrIncrement == 0u)
 	{
 		nAddrIncrement = 1u;
 	}
 	nAddrWidth = pDataBytes[A2B_OFFSET_DAT_ADDRWDTH];

 	switch (nAddrWidth)
 	{

 		case 1u:
 			nAddr = (a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR];
 			break;

 		case 2u:
 			nAddr = ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR] << 8u) | pDataBytes[A2B_OFFSET_DAT_ADDR+1u];
 			break;

 		case 3u:
 			nAddr = ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR] << 16u) | ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR+1u] << 8u) | pDataBytes[A2B_OFFSET_DAT_ADDR+2u];
 			break;

 		case 4u:
 			nAddr = ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR] << 24u) |  ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR+1u] << 16u) | ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR+2u] << 8u) | pDataBytes[A2B_OFFSET_DAT_ADDR+3u];
 			break;

 		default:
 			nAddr = (a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR];
 			break;
 	}

 	pBuf = &pDataBytes[A2B_OFFSET_DAT_ADDR + nAddrWidth];

 	if((eSpiCmd == A2B_CMD_SPI_REMOTE_I2C_WRITE) || (eSpiCmd == A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST))
 	{
 		nUserSpiCmdWidth = 0u;
 	}
 	else
 	{
 		nUserSpiCmdWidth = pDataBytes[A2B_OFFSET_DAT_SPICMDWDTH];
 	}

 	nMaxDataSize 			         = nMaxTransac - ( (a2b_UInt32)nAddrWidth + nUserSpiCmdWidth);
    nRem 					         = nMaxDataSize % nAddrIncrement;
    nActualDataSizeInBytes	         = (nDataCount - A2B_OFFSET_DAT_ADDR - nAddrWidth) * nDataWidth;
    nDataSplitSizeInBytes            = nMaxDataSize - nRem;
    nNumberOfDataBlocks              = nActualDataSizeInBytes / nDataSplitSizeInBytes;
    nNumberOfLeftOutDataInBytes      = nActualDataSizeInBytes % nDataSplitSizeInBytes;
    nAddressOffset          	     = (nDataSplitSizeInBytes / nAddrIncrement);

    for (nDataBlockIdx = 0u; nDataBlockIdx < nNumberOfDataBlocks; nDataBlockIdx++)
    {
    	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nUserSpiCmdWidth, nPeriSpiCmd , 0u );
    	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nAddrWidth, (nAddr + (nDataBlockIdx * nAddressOffset)), nUserSpiCmdWidth );
    	(void)a2b_memcpy(&spiDataBuffer[nAddrWidth + nUserSpiCmdWidth], &pBuf[nDataBlockIdx * nDataSplitSizeInBytes], nDataSplitSizeInBytes);
     	status = a2b_spiPeriphWrite(ctx, nNodeAddr, eSpiCmd, regAddr, regAddr, (nDataSplitSizeInBytes + nAddrWidth + nUserSpiCmdWidth), &spiDataBuffer[0u]);
        if(status != A2B_RESULT_SUCCESS)
 		{
        	nReturn = 1u;
 			break;
 		}
    }
    if(nNumberOfLeftOutDataInBytes != 0u)
    {
    	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nUserSpiCmdWidth, nPeriSpiCmd , 0u );
    	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nAddrWidth, (nAddr + (nDataBlockIdx * nAddressOffset)), nUserSpiCmdWidth );
     	(void)a2b_memcpy(&spiDataBuffer[nAddrWidth + nUserSpiCmdWidth], &pBuf[nNumberOfDataBlocks * nDataSplitSizeInBytes], nNumberOfLeftOutDataInBytes);
        status = a2b_spiPeriphWrite(ctx, nNodeAddr, eSpiCmd, regAddr, regAddr, (nNumberOfLeftOutDataInBytes + nAddrWidth + nUserSpiCmdWidth), &spiDataBuffer[0u]);
        if(status != A2B_RESULT_SUCCESS)
 		{
 			nReturn = 1u;
 		}
    }

    return (nReturn);
}


 /****************************************************************************/
  /*!
      @brief     This function splits the a single SPI command and generate SPI sub commands.
                      It also writes the sub commands to peripheral

      @param [in]     plugin                       Pointer to A2B Slave Plugin
      @param [in]     nNodeAddr               Node address
      @param [in]     eSpiCmd                   SPI command
      @param [in]     regAddr                    I2C Address or SPI slave select
      @param [in]     nDataCount              Number of bytes to be written
      @param [in]     pDataBytes               Payload buffer pointer
      @param [in]     nRead              	The number of bytes to read from the peripheral. The 'rBuf' parameter must have enough space to hold this number of bytes.
      @param   [in,out]    rBuf             The buffer in which to write the results of the read. It's assumed the buffer is sized to accept 'nRead' bytes of data.
      @param [in]     nMaxTransac            Maximum number of bytes that can be transferred at a time.

      @return          Return code
                      - 0: Success
                      - 1: Failure
  */
 a2b_HResult a2b_SpiCmdsAndPeriWriteRead(a2b_StackContext*  ctx, a2b_Int16 nNodeAddr, a2b_SpiCmd eSpiCmd, a2b_UInt16 regAddr, a2b_UInt32 nDataCount, a2b_UInt8* pDataBytes, a2b_UInt16 nRead, void* rBuf, a2b_UInt32 nMaxTransac)
  {
  	a2b_UInt32	nReturn = 0u, nDataSplitSizeInBytes = 0u, nNumberOfDataBlocks = 0u, nNumberOfLeftOutDataInBytes = 0u;
  	a2b_UInt32	nDataBlockIdx=0, nMaxDataSize, nRem, nActualDataSizeInBytes, nAddressOffset;
  	a2b_HResult status = A2B_RESULT_SUCCESS;
  	a2b_UInt8 nAddrIncrement = 0u;
  	a2b_UInt8 nDataWidth = 1u;
  	a2b_UInt8 nAddrWidth = 0u;
  	a2b_UInt8 nUserSpiCmdWidth = 0u;
  	a2b_UInt8 nPeriSpiCmd = 0u;
  	a2b_UInt32 nAddr = 0;
  	a2b_UInt8 *pBuf;

 	nAddrIncrement = pDataBytes[A2B_OFFSET_DAT_ADDRINC];
 	/* For splitting purpose */
 	if (nAddrIncrement == 0u)
 	{
 		nAddrIncrement = 1u;
 	}
 	nAddrWidth = pDataBytes[A2B_OFFSET_DAT_ADDRWDTH];

 	switch (nAddrWidth)
 	{

 		case 1u:
 			nAddr = (a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR];
 			break;

 		case 2u:
 			nAddr = ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR] << 8u) | pDataBytes[A2B_OFFSET_DAT_ADDR+1u];
 			break;

 		case 3u:
 			nAddr = ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR] << 16u) | ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR+1u] << 8u) | pDataBytes[A2B_OFFSET_DAT_ADDR+2u];
 			break;

 		case 4u:
 			nAddr = ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR] << 24u) |  ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR+1u] << 16u) | ((a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR+2u] << 8u) | pDataBytes[A2B_OFFSET_DAT_ADDR+3u];
 			break;

 		default:
 			nAddr = (a2b_UInt32)pDataBytes[A2B_OFFSET_DAT_ADDR];
 			break;
 	}

 	pBuf = &pDataBytes[A2B_OFFSET_DAT_ADDR + nAddrWidth];

  	if(eSpiCmd == A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST)
  	{
  		nUserSpiCmdWidth = 0u;
  	}
  	else
  	{
  		nUserSpiCmdWidth = pDataBytes[A2B_OFFSET_DAT_SPICMDWDTH];
  	}

  	nMaxDataSize 			         = nMaxTransac - ( (a2b_UInt32)nAddrWidth + nUserSpiCmdWidth);
     nRem 					         = nMaxDataSize % nAddrIncrement;
     nActualDataSizeInBytes	         = (nDataCount - A2B_OFFSET_DAT_ADDR - nAddrWidth) * nDataWidth;
     nDataSplitSizeInBytes            = nMaxDataSize - nRem;
     nNumberOfDataBlocks              = nActualDataSizeInBytes / nDataSplitSizeInBytes;
     nNumberOfLeftOutDataInBytes      = nActualDataSizeInBytes % nDataSplitSizeInBytes;
     nAddressOffset          	     = (nDataSplitSizeInBytes / nAddrIncrement);

     for (nDataBlockIdx = 0u; nDataBlockIdx < nNumberOfDataBlocks; nDataBlockIdx++)
     {
     	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nUserSpiCmdWidth, nPeriSpiCmd , 0u );
     	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nAddrWidth, (nAddr + (nDataBlockIdx * nAddressOffset)), nUserSpiCmdWidth );
     	(void)a2b_memset(&ReadDataBuffer[0u], (a2b_Int32)0u, (a2b_Size)nDataSplitSizeInBytes);
      	status = a2b_spiPeriphWriteRead(ctx, nNodeAddr, eSpiCmd, regAddr, regAddr, (nAddrWidth + nUserSpiCmdWidth), &spiDataBuffer[0u], nDataSplitSizeInBytes, &ReadDataBuffer[0u]);

      	if(A2B_SUCCEEDED(status))
		{
			/* Check for any SPI error interrupt */
			status = a2b_checkSpiErrorInt(ctx);
		}

      	if(status != A2B_RESULT_SUCCESS)
  		{
         	nReturn = 1u;
  			break;
  		}
     }
     if(nNumberOfLeftOutDataInBytes != 0u)
     {
     	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nUserSpiCmdWidth, nPeriSpiCmd , 0u );
     	adi_a2b_Concat_Addr_Data(&spiDataBuffer[0u], nAddrWidth, (nAddr + (nDataBlockIdx * nAddressOffset)), nUserSpiCmdWidth );
      	(void)a2b_memset(&ReadDataBuffer[0u], (a2b_Int32)0u, (a2b_Size)nNumberOfLeftOutDataInBytes);
      	status = a2b_spiPeriphWriteRead(ctx, nNodeAddr, eSpiCmd, regAddr, regAddr, (nAddrWidth + nUserSpiCmdWidth), &spiDataBuffer[0u], nNumberOfLeftOutDataInBytes, &ReadDataBuffer[0u]);
         if(status != A2B_RESULT_SUCCESS)
  		{
  			nReturn = 1u;
  		}
     }

     return (nReturn);
 }

 /*!****************************************************************************
 *  \ingroup         a2bstack_hwaccess
 *
 *  \b               a2b_periphWrite
 *
 *  Writes bytes to the SPI/I2C peripherals. This is an synchronous call and will block
 *  until the operation is complete.
 *
 *  \param   [in]    ctx     The stack context associated with the write.
 *
 *  \param   [in]    node    The A2B node address.
 *
 *  \param 	 [in]	 hw_ifAddr The peripheral I2C address
 *
 *  \param   [in]    nWrite  The number of bytes to write.
 *
 *  \param   [in]    wBuf    A buffer containing the data to write. The buffer
 *                           is of size 'nWrite' bytes.
 *
 *  \pre     None
 *
 *  \post    None
 *
 *  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
 *           #A2B_FAILED() for success or failure of the request.
 *
 ******************************************************************************/
 A2B_DSO_PUBLIC a2b_HResult a2b_periphWrite(
 	struct a2b_StackContext*    ctx,
 	a2b_Int16                   node,
	a2b_UInt16					hw_ifAddr,
	a2b_UInt8                   datExportVer,
 	a2b_UInt16                  nWrite,
 	void*                       wBuf)
 {
 	a2b_HResult 				status = A2B_RESULT_SUCCESS;
 	struct a2b_StackContext* 	mCtx;
 	A2B_SPI_MODE                eSpiMode;
 	a2b_UInt16					IfAddr;
 	a2b_UInt16                  spiCmd;

 	/* Need a master plugin context to do I2C/SPI calls */
 	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
 	if ( A2B_NULL == mCtx)
 	{
 		/* This should *never* happen */
 		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
 	}
 	else
 	{
 		/* Check hardware interface */
 		if( (a2b_Bool)(node  == A2B_NODEADDR_MASTER))
 		{
 			status = a2b_i2cPeriphWrite( mCtx, node, hw_ifAddr, nWrite, ((a2b_UInt8*)wBuf));
 		}
 		else if(mCtx->stk->accessInterface == A2B_ACCESS_I2C)
 		{
 			if(datExportVer != 0x35u)
			{
				status = a2b_i2cPeriphWrite( mCtx, node, hw_ifAddr, nWrite,(a2b_UInt8*)wBuf );
			}
			else
			{
				/* Buffer address is offset to the the actual Address */
				status = a2b_i2cPeriphWrite( mCtx, node, hw_ifAddr, (nWrite-A2B_OFFSET_DAT_ADDR), ((a2b_UInt8*)wBuf+A2B_OFFSET_DAT_ADDR) );
			}
 		}
 		else
 		{
 			/* Check if this is a SPI to SPI write */
 			if(hw_ifAddr & 0x80u)
 			{
 	 			eSpiMode = (A2B_SPI_MODE)(hw_ifAddr & 0x0Fu);
 	 			IfAddr = (hw_ifAddr >> 4u) & 0x7u;

 	 			spiCmd = (a2b_UInt16)a2b_getSpiCmd(mCtx, eSpiMode, 0u);
 	 			if(datExportVer != 0x35u)
 	 			{
 	 				status = a2b_spiPeriphWrite( mCtx, node, spiCmd, IfAddr, IfAddr, nWrite, wBuf);
 	 			}
 	 			else
 	 			{
 	 				status = a2b_SpiCmdsAndPeriWrite(ctx, node, (a2b_SpiCmd)spiCmd, IfAddr, nWrite, wBuf, A2B_MAX_LENINBYTES_REMOTESPITOSPI);
 	 			}
 			}
 			else
 			{
 	 			if(datExportVer != 0x35)
 	 			{
 	 				status = a2b_spiPeriphWrite( mCtx, node,(a2b_UInt16)A2B_CMD_SPI_REMOTE_I2C_WRITE, hw_ifAddr, A2B_SPI_SLV_SEL, nWrite, wBuf);
 	 			}
 	 			else
 	 			{
 	 				status = a2b_SpiCmdsAndPeriWrite(ctx, node, A2B_CMD_SPI_REMOTE_I2C_WRITE, hw_ifAddr, nWrite, wBuf, A2B_MAX_LENINBYTES_REMOTESPITOI2C);
 	 			}
 			}
 		}
 	}

 	return status;
 }
#endif
 /*!****************************************************************************
 *  \ingroup         a2bstack_hwaccess
 *
 *  \b               a2b_periphWriteRead
 *
 *  Reads bytes from the SPI/I2C peripherals. This is an synchronous call and will block
 *  until the operation is complete.
 *
 *  \param   [in]    ctx     The stack context associated with the write.
 *
 *  \param   [in]    node    The A2B node address.
 *
 *  \param 	 [in]	 hw_ifAddr The peripheral I2C address
 *
 *  \param   [in]    nWrite  The number of bytes to write.
 *
 *  \param   [in]    wBuf    A buffer containing the data to write. The buffer
 *                           is of size 'nWrite' bytes.
 *
 *	\param   [in]    nRead  The number of bytes to read.
 *
 *  \param   [in]    rBuf    A buffer in which to write the results of the read.
 *
 *  \pre     None
 *
 *  \post    None
 *
 *  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
 *           #A2B_FAILED() for success or failure of the request.
 *
 ******************************************************************************/
 A2B_DSO_PUBLIC a2b_HResult a2b_periphWriteRead(
 	struct a2b_StackContext*    ctx,
 	a2b_Int16                   node,
	a2b_UInt16					hw_ifAddr,
 	a2b_UInt16                  nWrite,
 	void*                       wBuf,
	a2b_UInt16					nRead,
	void*						rBuf)
 {
 	a2b_HResult 				status = A2B_RESULT_SUCCESS;
 	struct a2b_StackContext* 	mCtx;

 	/* Need a master plugin context to do I2C/SPI calls */
 	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
 	if ( A2B_NULL == mCtx)
 	{
 		/* This should *never* happen */
 		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
 	}
 	else
 	{
 		/* If it is access to main node peripherals (only I2C access is supported for main node) or SPI-I2C sub node peripheral access */
 		if( (a2b_Bool)(node  == A2B_NODEADDR_MASTER) ||  (mCtx->stk->accessInterface == A2B_ACCESS_I2C))
 		{
 			status = a2b_i2cPeriphWriteRead( mCtx, node, hw_ifAddr, nWrite, wBuf, nRead, rBuf );
 		}
 		else
 		{
 			status = a2b_spiPeriphWriteRead( mCtx, node, A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST, hw_ifAddr, A2B_SPI_SLV_SEL,  nWrite, wBuf, nRead, rBuf );
 		}
 	}
 	return status;
 }

