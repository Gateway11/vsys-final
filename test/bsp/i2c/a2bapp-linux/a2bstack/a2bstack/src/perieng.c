/*******************************************************************************
Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
 * @file:    perieng.c
 * @brief:   This is the implementation of Tx and Rx state machines for SPI peripherals
 * @version: $Revision: 8951 $
 * @date:    $Date: 2019-04-12 17:55:42 +0530 (Fri, 12 Apr 2019) $
 * Developed by: Automotive Software and Systems team, Bangalore, India
*****************************************************************************/
/*! \addtogroup PERI_Engine PERI Engine
 *  @{
 */

/** @defgroup PERI_Engine    PERI Engine
 *
 * This module handles the Tx and Rx state machines for SPI peripherals
 *
 */

/*! \addtogroup PERI_Engine
 *  @{
 */

/*============= I N C L U D E S =============*/

#include "stack_priv.h"
#include "spi_priv.h"
#include "stackctx.h"
#include "perieng.h"
#include "a2b/hwaccess.h"
#include "a2b/regdefs.h"
#include "a2b/error.h"
#include "a2b/timer.h"
#include "a2b/util.h"
#ifdef _TESSY_INCLUDES_
#include "a2b/spi.h"
#endif /* _TESSY_INCLUDES_ */


/*============= D E F I N E S =============*/

/*============= D A T A =============*/

/*============= L O C A L  P R O T O T Y P E S =============*/


static void 		a2b_spiltAddr		(a2b_UInt8 pDstBuf[] ,a2b_UInt8 nAddrwidth, a2b_UInt32 nAddr);
static void 		a2b_concatAddr		(a2b_UInt8 pDstBuf[] ,a2b_UInt8 nAddrwidth, a2b_UInt32 *pnAddr);
static void 		a2b_rstSpiTxInfo	(a2b_SpiTxInfo *pSpiTxInfo);
static void 		a2b_rstSpiRxInfo	(a2b_SpiRxInfo *pSpiRxInfo);
static void 		a2b_rstSpiFdInfo	(a2b_SpiFdInfo *pSpiFdInfo);
static void			a2b_rstSpiOthInfo	(a2b_SpiInfo *pSpiInfo);

static a2b_Bool 	a2b_checkWrRdMode	(a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	a2b_checkSpiFdParams(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	a2b_enablSpiErrorInt(a2b_StackContext *pCtx);
a2b_HResult 	a2b_checkSpiErrorInt(a2b_StackContext *pCtx);
static a2b_HResult  a2b_checkSpiAtomicParams(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	a2b_checkSpiBulkParams(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_Bool 	a2b_checkSpiOnGoingTransaction(a2b_StackContext *pCtx);
static a2b_HResult  a2b_checkSpiParams	(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	a2b_prepForSpiFdRegBasedAccess(a2b_StackContext *pCtx, a2b_UInt8 nTranscSzInBytes, a2b_SpiWrRdParams *pSpiWrRdParams);
static void 		a2b_onRemoteDevConfigSpiToSpiResponseTimeout(struct a2b_Timer *timer, a2b_Handle userData);

static a2b_HResult	spiWrBlock			(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult	spiWrRdBlock		(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	spiWrNonBlock		(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	spiWrRdNonBlock		(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	spiFdBlock			(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	spiFdNonBlock		(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	a2b_spiWrRdBlock	(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	a2b_spiWrRdNonBlock	(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);
static a2b_HResult 	a2b_handlePayloadPendingTx(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr);
static a2b_HResult 	a2b_handlePayloadPendingRx(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr);
static a2b_HResult 	a2b_handlePayloadPendingFd(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr);
static a2b_HResult 	a2b_handleSpiDoneIrpt(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr);
static void  a2b_handleSpiDataTunErr(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr, A2B_SPI_EVENT *pSpiEvent);

/*============= C O D E =============*/

/*****************************************************************************/
/*!
@brief			This function splits the address into individual bytes array.
				This function can handle upto 4 byte addresses.

@param [out]    pDstBuf		Pointer to destination buffer
@param [in]     nAddrwidth	Address width in bytes
@param [in]     nAddr		Address which has to be split

@return			None
*/
/*****************************************************************************/
static void a2b_spiltAddr(a2b_UInt8 pDstBuf[] ,a2b_UInt8 nAddrwidth, a2b_UInt32 nAddr)
{
    /* Store the read values in the place holder */
    switch(nAddrwidth)
    {   /* Byte */
        case 1u:
        	pDstBuf[0U]  =  (a2b_UInt8)nAddr;
        	break;
         /* 16 bit word*/
        case 2u:
        	pDstBuf[0U]  =  (a2b_UInt8)((nAddr & 0xFF00U) >> 8U);
        	pDstBuf[1U]  =  (a2b_UInt8)(nAddr & 0xFFU);
        	break;
		/* 24 bit word */
        case 3u:
        	pDstBuf[0U]  =  (a2b_UInt8)((nAddr & 0xFF0000U) >> 16U);
        	pDstBuf[1U]  =  (a2b_UInt8)((nAddr & 0xFF00U) >> 8U);
        	pDstBuf[2U]  =  (a2b_UInt8)(nAddr & 0xFFU);
        	break;
        /* 32 bit word */
        case 4u:
        	pDstBuf[0U]  =  (a2b_UInt8)((nAddr & 0xFF000000U) >> 24U);
        	pDstBuf[1U]  =  (a2b_UInt8)((nAddr & 0xFF0000U) >> 16U);
        	pDstBuf[2U]  =  (a2b_UInt8)((nAddr & 0xFF00U) >> 8U);
        	pDstBuf[3U]  =  (a2b_UInt8)(nAddr & 0xFFU);
        	break;
        default:
		/* Do Nothing */
        	break;
    }
}

/*****************************************************************************/
/*!
@brief			This function concatenates the bytes addresses into a word address
				This function can handle upto 4 byte addresses.

@param [in]     pDstBuf		Pointer to buffer which holds the address in byte form
@param [in]     nAddrwidth	Address width in bytes
@param [out]    pnAddr		Pointer to address which will have the address in word form

@return			None
*/
/*****************************************************************************/
static void a2b_concatAddr(a2b_UInt8 pDstBuf[] ,a2b_UInt8 nAddrwidth, a2b_UInt32 *pnAddr)
{
    /* Store the read values in the place holder */
    switch(nAddrwidth)
    {   /* Byte */
        case 1u:
        	*pnAddr		 =  pDstBuf[0U];
        	break;
         /* 16 bit word*/
        case 2u:
        	*pnAddr		 =  ( (((a2b_UInt32)pDstBuf[0U] << 8u ) & 0x0000FF00U)  |
        					  (((a2b_UInt32)pDstBuf[1U])        & 0x000000FFU) );
        	break;
		/* 24 bit word */
        case 3u:
        	*pnAddr		 =  ( 	(((a2b_UInt32)pDstBuf[0U] << 16u) & 0x00FF0000U) |
        						(((a2b_UInt32)pDstBuf[1U] << 8u ) & 0x0000FF00U) |
								(((a2b_UInt32)pDstBuf[2U]) 		  & 0x000000FFU) );
        	break;
        /* 32 bit word */
        case 4u:
        	*pnAddr		 =  ( 	(((a2b_UInt32)pDstBuf[0U] << 24u) & 0xFF000000U) |
        						(((a2b_UInt32)pDstBuf[1U] << 16u) & 0x00FF0000U) |
								(((a2b_UInt32)pDstBuf[2U] << 8u)  & 0x0000FF00U) |
								(((a2b_UInt32)pDstBuf[3U])        & 0x000000FFU) );
        	break;
        default:
		/* Do Nothing */
        	break;
    }
}

/*****************************************************************************/
/*!
@brief			This function resets the SPI transmission state params

@param [in]     pSpiTxInfo		Pointer to SPI transmission info structure

@return			None
*/
/*****************************************************************************/
 static void a2b_rstSpiTxInfo(a2b_SpiTxInfo *pSpiTxInfo)
{
	pSpiTxInfo->bfPayloadPendingTx			= A2B_FALSE;
	pSpiTxInfo->nCurrWriteBytes				= 0u;
	pSpiTxInfo->nDataSplitSizeInBytes       = 0u;
	pSpiTxInfo->nNumberOfDataBlocks         = 0u;
	pSpiTxInfo->nNumberOfLeftOutDataInBytes = 0u;
	pSpiTxInfo->nDataBlockIdx				= 0u;
}

/*****************************************************************************/
/*!
@brief			This function resets the SPI reception state params

@param [in]     pSpiRxInfo		Pointer to SPI reception info structure

@return			None
*/
/*****************************************************************************/
static void a2b_rstSpiRxInfo(a2b_SpiRxInfo *pSpiRxInfo)
{
	pSpiRxInfo->bfPayloadPendingRx			= A2B_FALSE;
	pSpiRxInfo->nCurrReadBytes				= 0u;
	pSpiRxInfo->nDataSplitSizeInBytes       = 0u;
	pSpiRxInfo->nNumberOfDataBlocks         = 0u;
	pSpiRxInfo->nNumberOfLeftOutDataInBytes = 0u;
	pSpiRxInfo->nDataBlockIdx				= 0u;
}

/*****************************************************************************/
/*!
@brief			This function resets the SPI full duplex state params

@param [in]     pSpiFdInfo		Pointer to SPI full duplex info structure

@return			None
*/
/*****************************************************************************/
static void a2b_rstSpiFdInfo(a2b_SpiFdInfo *pSpiFdInfo)
{
	pSpiFdInfo->bfPayloadPendingFd			= A2B_FALSE;
	pSpiFdInfo->nCurrFdBytes				= 0u;
	pSpiFdInfo->nDataSplitSizeInBytes       = 0u;
	pSpiFdInfo->nNumberOfDataBlocks         = 0u;
	pSpiFdInfo->nNumberOfLeftOutDataInBytes = 0u;
	pSpiFdInfo->nDataBlockIdx				= 0u;
}

/*****************************************************************************/
/*!
@brief			This function resets the SPI info other params

@param [in]     pSpiInfo		Pointer to SPI full duplex info structure

@return			None
*/
/*****************************************************************************/
static void a2b_rstSpiOthInfo(a2b_SpiInfo *pSpiInfo)
{
	a2b_UInt8	nCnt;

	pSpiInfo->nSpiToSpiPeriConfigReq 	= 0U;
	pSpiInfo->nSpiToSpiPeriConfigResp 	= 0U;
	pSpiInfo->fSplitTransRequired		= A2B_FALSE;

	for(nCnt=0U; nCnt<A2B_CONF_MAX_NUM_SLAVE_NODES ; nCnt++)
	{
		pSpiInfo->fSpiToSpiPeriConfigReq[nCnt] 	= A2B_FALSE;
		pSpiInfo->fSpiToSpiPeriConfigResp[nCnt]	= A2B_FALSE;
	}
}

/*****************************************************************************/
/*!
@brief			This function checks if the SPI transaction is write only or
				write-read mode based on parameters set by user

@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			a2b_Bool type
					A2B_TRUE:	If its write-read mode
					A2B_FALSE:	If its Write only mode
*/
/*****************************************************************************/
static a2b_Bool a2b_checkWrRdMode(a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_Bool bSpiWrRdMode = A2B_TRUE;

	if( (pSpiWrRdParams->nReadBytes  == 0u) && (pSpiWrRdParams->pRBuf == A2B_NULL) &&
		(pSpiWrRdParams->nWriteBytes != 0u) && (pSpiWrRdParams->pWBuf != A2B_NULL) )
	{
		/* SPI Write only mode */
		bSpiWrRdMode = A2B_FALSE;
	}
	else if((pSpiWrRdParams->nReadBytes  != 0u) && (pSpiWrRdParams->pRBuf != A2B_NULL) &&
			(pSpiWrRdParams->nWriteBytes != 0u) && (pSpiWrRdParams->pWBuf != A2B_NULL) )
	{
		/* SPI Write Read mode */
		bSpiWrRdMode = A2B_TRUE;
	}
	else
	{
		/* Do Nothing */
	}

	return (bSpiWrRdMode);
}

/*****************************************************************************/
/*!
@brief			This function checks the validity of SPI parameters in full duplex mode

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_checkSpiFdParams(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_SpiInfo	*pSpiInfo;

	pSpiInfo = &pCtx->stk->oSpiInfo;
	if((pSpiInfo->eSpiMode == A2B_SPI_FD_CMD_BASED) || (pSpiInfo->eSpiMode == A2B_SPI_FD_REG_BASED) || (pSpiInfo->eSpiMode == A2B_SPI_FD_CMD_BASED_EXTENDED) )
	{
		if(pSpiWrRdParams->nWriteBytes != pSpiWrRdParams->nReadBytes)
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
		}
	}
	return (result);
}

/*****************************************************************************/
/*!
@brief			This function enables the SPI interrupts

@param [in]     pCtx		Pointer to stack context

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_enablSpiErrorInt(a2b_StackContext *pCtx)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_UInt8 	wBuf[2U];

	wBuf[0]  = 	A2B_REG_SPIMSK;
	wBuf[1]  = 	A2B_BITM_SPIMSK_FIFOUIEN 	|
				A2B_BITM_SPIMSK_FIFOOIEN 	|
				A2B_BITM_SPIMSK_BADCMDIEN 	|
				A2B_BITM_SPIMSK_SPIDTIEN 	|
				A2B_BITM_SPIMSK_SPII2CIEN	|
				A2B_BITM_SPIMSK_SPIREGIEN	|
				A2B_BITM_SPIMSK_SPIDIEN		;

	result = a2b_regWrite(pCtx, A2B_NODEADDR_MASTER, 2U, wBuf);
	return (result);
}

/*****************************************************************************/
/*!
@brief			This function checks for the SPI interrupts by reading A2B_REG_SPIINT register

@param [in]     pCtx		Pointer to stack context

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS: If A2B_BITM_SPIINT_SPIDONE is read
                - 1: A2B_SPI_FAILED	: If any other SPI interrupts are read
*/
/*****************************************************************************/
a2b_HResult a2b_checkSpiErrorInt(a2b_StackContext *pCtx)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_UInt8	nRegAddr, nRegValue, nSpiIntMsk;

	nSpiIntMsk = A2B_BITM_SPIINT_FIFOUNF  	|
				 A2B_BITM_SPIINT_FIFOOVF   	|
				 A2B_BITM_SPIINT_BADCMD    	|
				 A2B_BITM_SPIINT_SPIDTERR  	|
				 A2B_BITM_SPIINT_SPII2CERR 	|
				 A2B_BITM_SPIINT_SPIREGERR 	;

	nRegAddr  = A2B_REG_SPIINT;

	result = a2b_regWriteRead(pCtx, A2B_NODEADDR_MASTER, 1U, &nRegAddr, 1U, &nRegValue);
	if(A2B_SUCCEEDED(result))
	{
		if((nRegValue & nSpiIntMsk) != 0U)
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_SPITOSPI_PERICONFIG_FAILED);
		}

	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function checks the parameters for atomic transactions

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_checkSpiAtomicParams(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_Bool 	bSpiWrRdMode;
	a2b_SpiInfo	*pSpiInfo;

	pSpiInfo = &pCtx->stk->oSpiInfo;
	bSpiWrRdMode = a2b_checkWrRdMode(pSpiWrRdParams);

	if(pSpiInfo->eSpiMode == A2B_SPI_ATOMIC)
	{
		if((pSpiWrRdParams->nWriteBytes > A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_ADDR_WIDTH_INBYTES)  && (bSpiWrRdMode == A2B_TRUE))
		{
			/* Writebytes should be <= A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_ADDR_WIDTH_INBYTES, during atomic write-read transactions */
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
		}
		else if(pSpiWrRdParams->nAddrWidth > A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_ADDR_WIDTH_INBYTES)
		{
			/* nAddrWidth should be <= A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_ADDR_WIDTH_INBYTES, during atomic write transactions */
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
		}
		else
		{
			/* Return A2B_SPI_SUCCESS, if SPI ATOMIC write-read transaction is selected and number of write bytes is LESS than 4 */
			result = A2B_RESULT_SUCCESS;
		}
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function checks the parameters for bulk transactions

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_checkSpiBulkParams(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_Bool 	bSpiWrRdMode;
	a2b_SpiInfo	*pSpiInfo;

	pSpiInfo = &pCtx->stk->oSpiInfo;
	bSpiWrRdMode = a2b_checkWrRdMode(pSpiWrRdParams);

	if(((pSpiInfo->eSpiMode == A2B_SPI_BULK) ||(pSpiInfo->eSpiMode == A2B_SPI_BULK_EXTENDED))  && (bSpiWrRdMode == A2B_TRUE))
	{
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function checks the if there is an on going SPI transaction

@param [in]     pCtx				Pointer to stack context

@return			a2b_Bool type
                - false:	If there is no SPI transaction
                - true:		If an on going SPI transaction is found
*/
/*****************************************************************************/
static a2b_Bool a2b_checkSpiOnGoingTransaction(a2b_StackContext *pCtx)
{
	a2b_Bool	bSpiOnGoingTransaction = A2B_FALSE;
	a2b_SpiInfo	*pSpiInfo = &pCtx->stk->oSpiInfo;

	if((pSpiInfo->oSpiTxInfo.bfPayloadPendingTx == A2B_TRUE) || (pSpiInfo->oSpiRxInfo.bfPayloadPendingRx == A2B_TRUE) || (pSpiInfo->oSpiFdInfo.bfPayloadPendingFd == A2B_TRUE))
	{
		bSpiOnGoingTransaction = A2B_TRUE;
	}

	return (bSpiOnGoingTransaction);
}

/*****************************************************************************/
/*!
@brief			This function checks the parameters for SPI transactions (Fd, Atomic & Bulk)
				by calling relevant functions

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_checkSpiParams(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;

	result = a2b_checkSpiFdParams(pCtx, pSpiWrRdParams);

	if(A2B_SUCCEEDED(result))
	{
		result = a2b_checkSpiAtomicParams(pCtx, pSpiWrRdParams);
	}

	if(A2B_SUCCEEDED(result))
	{
		result = a2b_checkSpiBulkParams(pCtx, pSpiWrRdParams);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function writes to A2B_REG_SPIFDSIZE & A2B_REG_SPIFDTARG master node registers
				and prepares for full duplex register based SPI transaction

@param [in]     pCtx			 Pointer to stack context
@param [in]     nTranscSzInBytes Number of transaction bytes to be written
@param [in]     pSpiWrRdParams	 Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED

*/
/*****************************************************************************/
static a2b_HResult a2b_prepForSpiFdRegBasedAccess(a2b_StackContext *pCtx, a2b_UInt8 nTranscSzInBytes, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_UInt8 	wBuf[2U];

	wBuf[0] = A2B_REG_SPIFDSIZE;
	wBuf[1] = nTranscSzInBytes;

	result = a2b_regWrite(pCtx, A2B_NODEADDR_MASTER, 2U, wBuf);
	if(A2B_FAILED(result))
	{
		/* Add TRACE */
	}
	else
	{
		wBuf[0] = A2B_REG_SPIFDTARG;
		wBuf[1] = a2b_prepSpiSsNodeByte(pSpiWrRdParams->nNode, 0U, (a2b_UInt16)pSpiWrRdParams->nSlaveSel);

		result = a2b_regWrite(pCtx, A2B_NODEADDR_MASTER, 2U, wBuf);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This is time out function for SPI to SPI transaction

@param [in]     timer			Pointer to timer
@param [in]     userData		Pointer to user data

@return			None
*/
/*****************************************************************************/
static void a2b_onRemoteDevConfigSpiToSpiResponseTimeout(struct a2b_Timer *timer, a2b_Handle userData)
{
	a2b_StackContext 	*pCtx = (a2b_StackContext*)userData;
	a2b_SpiInfo			*pSpiInfo;

	A2B_UNUSED(timer);
	pSpiInfo		= &pCtx->stk->oSpiInfo;

	if(pSpiInfo->oSpiRxInfo.bfPayloadPendingRx == A2B_TRUE)
	{
		pSpiInfo->pfStatCb(pCtx, A2B_SPI_EVENT_RX_TIMEOUT, (a2b_Int8)pSpiInfo->oSpiWrRdParams.nNode);

		/* Reset the SPI transmission state params */
		a2b_rstSpiRxInfo(&pSpiInfo->oSpiRxInfo);
	}
	else if(pSpiInfo->oSpiTxInfo.bfPayloadPendingTx == A2B_TRUE)
	{
		pSpiInfo->pfStatCb(pCtx, A2B_SPI_EVENT_TX_TIMEOUT, (a2b_Int8)pSpiInfo->oSpiWrRdParams.nNode);

		/* Reset the SPI transmission state params */
		a2b_rstSpiTxInfo(&pSpiInfo->oSpiTxInfo);
	}
	else if(pSpiInfo->oSpiFdInfo.bfPayloadPendingFd == A2B_TRUE)
	{
		pSpiInfo->pfStatCb(pCtx, A2B_SPI_EVENT_FD_TIMEOUT, (a2b_Int8)pSpiInfo->oSpiWrRdParams.nNode);

		/* Reset the SPI transmission state params */
		a2b_rstSpiFdInfo(&pSpiInfo->oSpiFdInfo);
	}
	else
	{
		/* Do Nothing */
	}

	a2b_rstSpiOthInfo(pSpiInfo);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral write only transactions (Blocking Mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult spiWrBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_UInt32	nDataSplitSizeInBytes = 0u, nNumberOfDataBlocks = 0u, nNumberOfLeftOutDataInBytes = 0u, nTempAddr, nBaseAddr = 0u, nAddrIncremBytes;
	a2b_UInt32	nDataBlockIdx,nMaxDataSize, nRem, nActualDataSizeInBytes;
	a2b_SpiCmd 	eSpiCmd;
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_SpiInfo	*pSpiInfo;
	a2b_UInt8	nAddrWidth, nAddrOffset, nUsrSpiCmdAndAddrBytes;
	a2b_UInt8 	anTempWbuf[A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES];

	nAddrWidth          		= pSpiWrRdParams->nAddrWidth;
	nAddrOffset         		= pSpiWrRdParams->nAddrOffset;
	nUsrSpiCmdAndAddrBytes		= nAddrWidth + nAddrOffset;

	pSpiInfo 					= &pCtx->stk->oSpiInfo;
	nMaxDataSize 				= (a2b_UInt32)(A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES - (a2b_UInt32)nUsrSpiCmdAndAddrBytes);

    /* For splitting purpose */
    if (pSpiWrRdParams->nAddrIncrement == 0u)
    {
    	pSpiWrRdParams->nAddrIncrement  = 1u;
    }

    nRem 						= nMaxDataSize % pSpiWrRdParams->nAddrIncrement;
    nActualDataSizeInBytes		= (a2b_UInt32)(pSpiWrRdParams->nWriteBytes - (a2b_UInt32)nUsrSpiCmdAndAddrBytes);
    nDataSplitSizeInBytes       = nMaxDataSize - nRem;
    nNumberOfDataBlocks         = nActualDataSizeInBytes / nDataSplitSizeInBytes;
    nNumberOfLeftOutDataInBytes = nActualDataSizeInBytes % nDataSplitSizeInBytes;
    nAddrIncremBytes          	= (nDataSplitSizeInBytes / pSpiWrRdParams->nAddrIncrement);

    /* Get corresponding SPI command */
    eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, A2B_FALSE);

    /* Get base address and concatenate the address into a2b_UInt32 variable */
	a2b_concatAddr(&pSpiWrRdParams->pWBuf[nAddrOffset], nAddrWidth, &nBaseAddr);

    for (nDataBlockIdx = 0u; nDataBlockIdx < nNumberOfDataBlocks; nDataBlockIdx++)
    {
    	/* Increment the address from which next chunk of data to be written */
    	nTempAddr = nBaseAddr + nDataBlockIdx * nAddrIncremBytes;

    	/* Copy the user SPI cmd and the address bytes */
    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], nUsrSpiCmdAndAddrBytes);

    	/* Split updated address into byte array */
    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

    	if(nDataBlockIdx == 0U)
    	{
			/* Copy the data bytes after the user SPI cmd and the address bytes */
			(void)a2b_memcpy(&anTempWbuf[nUsrSpiCmdAndAddrBytes], &pSpiWrRdParams->pWBuf[nUsrSpiCmdAndAddrBytes], nDataSplitSizeInBytes);
    	}
    	else
    	{
			/* Copy the data bytes after the user SPI cmd and the address bytes */
			(void)a2b_memcpy(&anTempWbuf[nUsrSpiCmdAndAddrBytes], &pSpiWrRdParams->pWBuf[(nDataBlockIdx * nDataSplitSizeInBytes) + (nUsrSpiCmdAndAddrBytes)], nDataSplitSizeInBytes);
    	}

    	/* Use normal bulk mode for writing the final block */
    	if((nDataBlockIdx == nNumberOfDataBlocks - 1U) && (nNumberOfLeftOutDataInBytes == 0u)  && (eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED))
    	{
    		eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE;
    	}

    	result = a2b_spiPeriphWrite(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, (nDataSplitSizeInBytes + nUsrSpiCmdAndAddrBytes), &anTempWbuf);

    	if(pSpiInfo->eDTOptimizationMode == A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED)
    	{
    		if(A2B_SUCCEEDED(result))
			{
				/* Check for any SPI error interrupt */
				result = a2b_checkSpiErrorInt(pCtx);
			}
    	}

    	if(A2B_FAILED(result))
    	{
        	break;
		}
    }
    if((A2B_SUCCEEDED(result)) && (nNumberOfLeftOutDataInBytes != 0u))
    {

    	/* Increment the address from which next chunk of data to be written */
    	nTempAddr = nBaseAddr + nNumberOfDataBlocks * nAddrIncremBytes;

    	/* Copy the user SPI cmd and the address bytes */
    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], (a2b_Size)nUsrSpiCmdAndAddrBytes);

    	/* Split updated address into byte array */
    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

    	if(nNumberOfDataBlocks == 0U)
    	{
			/* Copy the data bytes after the user SPI cmd and the address bytes */
			(void)a2b_memcpy(&anTempWbuf[nUsrSpiCmdAndAddrBytes], &pSpiWrRdParams->pWBuf[nUsrSpiCmdAndAddrBytes], (a2b_Size)nNumberOfLeftOutDataInBytes);
    	}
    	else
    	{
			/* Copy the data bytes after the user SPI cmd and the address bytes */
			(void)a2b_memcpy(&anTempWbuf[nUsrSpiCmdAndAddrBytes], &pSpiWrRdParams->pWBuf[(nNumberOfDataBlocks * nDataSplitSizeInBytes) + (nUsrSpiCmdAndAddrBytes)], (a2b_Size)nNumberOfLeftOutDataInBytes);
    	}

    	/* Use normal bulk mode for writing the final block */
    	if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED)
    	{
    		eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE;
    	}
    	result = a2b_spiPeriphWrite(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, (nNumberOfLeftOutDataInBytes + nUsrSpiCmdAndAddrBytes), &anTempWbuf);

    	if(pSpiInfo->eDTOptimizationMode == A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED)
    	{
    		if(A2B_SUCCEEDED(result))
			{
				/* Check for any SPI error interrupt */
				result = a2b_checkSpiErrorInt(pCtx);
			}
    	}
    }

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral write-read transactions (Blocking Mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult spiWrRdBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_UInt32	nDataSplitSizeInBytes = 0u, nNumberOfDataBlocks = 0u, nNumberOfLeftOutDataInBytes = 0u, nTempAddr, nBaseAddr = 0u, nAddrIncremBytes;
	a2b_UInt32	nDataBlockIdx,nMaxDataSize, nRem, nActualDataSizeInBytes;
	a2b_SpiCmd 	eSpiCmd;
	a2b_HResult 	result = A2B_RESULT_SUCCESS;
	a2b_SpiInfo	*pSpiInfo;
	a2b_UInt8	nAddrWidth, nAddrOffset, nUsrSpiCmdAndAddrBytes;
	a2b_UInt8 	anTempWbuf[A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES];

	nAddrWidth          		= pSpiWrRdParams->nAddrWidth;
	nAddrOffset         		= pSpiWrRdParams->nAddrOffset;
	nUsrSpiCmdAndAddrBytes		= nAddrWidth + nAddrOffset;

	pSpiInfo 					= &pCtx->stk->oSpiInfo;
	nMaxDataSize 				= A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES;

    /* For splitting purpose */
    if (pSpiWrRdParams->nAddrIncrement == 0u)
    {
    	pSpiWrRdParams->nAddrIncrement  = 1u;
    }

    nRem 						= nMaxDataSize % pSpiWrRdParams->nAddrIncrement;
    nActualDataSizeInBytes		= pSpiWrRdParams->nReadBytes;
    nDataSplitSizeInBytes       = nMaxDataSize - nRem;
    nNumberOfDataBlocks         = nActualDataSizeInBytes / nDataSplitSizeInBytes;
    nNumberOfLeftOutDataInBytes = nActualDataSizeInBytes % nDataSplitSizeInBytes;
    nAddrIncremBytes          	= (nDataSplitSizeInBytes / pSpiWrRdParams->nAddrIncrement);

    /* Get corresponding SPI command */
    eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, A2B_TRUE);

    /* Get base address and concatenate the address into a2b_UInt32 variable */
	a2b_concatAddr(&pSpiWrRdParams->pWBuf[nAddrOffset], nAddrWidth, &nBaseAddr);

    for (nDataBlockIdx = 0u; nDataBlockIdx < nNumberOfDataBlocks; nDataBlockIdx++)
    {
    	/* Increment the address from which next chunk of data to be written */
    	nTempAddr = nBaseAddr + nDataBlockIdx * nAddrIncremBytes;

    	/* Copy the user SPI cmd and the address bytes */
    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], nUsrSpiCmdAndAddrBytes);

    	/* Split updated address into byte array */
    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

    	result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, pSpiWrRdParams->nWriteBytes, &anTempWbuf,
    									nDataSplitSizeInBytes, &pSpiWrRdParams->pRBuf[nDataBlockIdx * nDataSplitSizeInBytes]);

    	if(pSpiInfo->eDTOptimizationMode == A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED)
    	{
    		if(A2B_SUCCEEDED(result))
			{
				/* Check for any SPI error interrupt */
				result = a2b_checkSpiErrorInt(pCtx);
			}
    	}

    	if(A2B_FAILED(result))
    	{
			break;
		}
    }
	if((A2B_SUCCEEDED(result)) && (nNumberOfLeftOutDataInBytes != 0u))
    {
    	/* Increment the address from which next chunk of data to be written */
    	nTempAddr = nBaseAddr + nNumberOfDataBlocks * nAddrIncremBytes;

    	/* Copy the user SPI cmd and the address bytes */
    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], (a2b_Size)nUsrSpiCmdAndAddrBytes);

    	/* Split updated address into byte array */
    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

    	result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, pSpiWrRdParams->nWriteBytes, &anTempWbuf,
    									nNumberOfLeftOutDataInBytes, &pSpiWrRdParams->pRBuf[nNumberOfDataBlocks * nDataSplitSizeInBytes]);

    	if(pSpiInfo->eDTOptimizationMode == A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED)
    	{
    		if(A2B_SUCCEEDED(result))
			{
				/* Check for any SPI error interrupt */
				result = a2b_checkSpiErrorInt(pCtx);
			}
    	}

    }

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral write only transactions (Non-Blocking Mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult spiWrNonBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult 	result = A2B_RESULT_SUCCESS;
	a2b_SpiInfo		*pSpiInfo;
	a2b_SpiTxInfo	*pSpiTxInfo;
	a2b_SpiCmd 		eSpiCmd;
	a2b_UInt32		nMaxDataSize, nRem, nActualDataSizeInBytes;
	a2b_UInt16 		nWrite;
	a2b_UInt8		nAddrWidth, nAddrOffset, nUsrSpiCmdAndAddrBytes;

	nAddrWidth          		= pSpiWrRdParams->nAddrWidth;
	nAddrOffset         		= pSpiWrRdParams->nAddrOffset;
	nUsrSpiCmdAndAddrBytes		= nAddrWidth + nAddrOffset;

	pSpiInfo					= &pCtx->stk->oSpiInfo;
	pSpiTxInfo					= &pSpiInfo->oSpiTxInfo;
	nMaxDataSize				= (a2b_UInt32)(A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES - (a2b_UInt32)nUsrSpiCmdAndAddrBytes);

    /* For splitting purpose */
    if (pSpiWrRdParams->nAddrIncrement == 0u)
    {
    	pSpiWrRdParams->nAddrIncrement  = 1u;
    }

    nRem 									= nMaxDataSize % pSpiWrRdParams->nAddrIncrement;
    nActualDataSizeInBytes					= (a2b_UInt32)pSpiWrRdParams->nWriteBytes - (a2b_UInt32)nUsrSpiCmdAndAddrBytes;
    pSpiTxInfo->nDataSplitSizeInBytes       = nMaxDataSize - nRem;
    pSpiTxInfo->nNumberOfDataBlocks         = nActualDataSizeInBytes / pSpiTxInfo->nDataSplitSizeInBytes;
    pSpiTxInfo->nNumberOfLeftOutDataInBytes = nActualDataSizeInBytes % pSpiTxInfo->nDataSplitSizeInBytes;
    pSpiTxInfo->nDataBlockIdx				= 0U;

	if(pSpiTxInfo->nNumberOfDataBlocks != 0U)
	{
		nWrite = (a2b_UInt16)pSpiTxInfo->nDataSplitSizeInBytes;
	}
	else
	{
		nWrite = (a2b_UInt16)pSpiTxInfo->nNumberOfLeftOutDataInBytes;
	}

 	/* Get corresponding SPI command */
    eSpiCmd 		= a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, A2B_FALSE);
	if((pSpiTxInfo->nNumberOfDataBlocks == 0U) && (eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED))
	{
		eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE;
	}

	result = a2b_spiPeriphWrite(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, (nWrite + nUsrSpiCmdAndAddrBytes), &pSpiWrRdParams->pWBuf[0U]);
	if(A2B_FAILED(result))
	{
		/* Add TRACE  */
	}
	else
	{
		pSpiTxInfo->nDataBlockIdx++;
		pSpiInfo->oSpiTxInfo.nCurrWriteBytes	+= (nWrite + nUsrSpiCmdAndAddrBytes);
		pSpiInfo->oSpiTxInfo.bfPayloadPendingTx  = A2B_TRUE;
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral write-read transactions (Non-Blocking Mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult spiWrRdNonBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_SpiInfo		*pSpiInfo;
	a2b_SpiRxInfo	*pSpiRxInfo;
	a2b_UInt32		nMaxDataSize, nRem, nActualDataSizeInBytes;
	a2b_SpiCmd 		eSpiCmd;
	a2b_HResult 	result = A2B_RESULT_SUCCESS;
	a2b_UInt16 		nRead;

	pSpiInfo 					= &pCtx->stk->oSpiInfo;
	pSpiRxInfo					= &pSpiInfo->oSpiRxInfo;
	nMaxDataSize 				= A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES;

    /* For splitting purpose */
    if (pSpiWrRdParams->nAddrIncrement == 0u)
    {
    	pSpiWrRdParams->nAddrIncrement  = 1u;
    }

    nRem 									= nMaxDataSize % pSpiWrRdParams->nAddrIncrement;
    nActualDataSizeInBytes					= pSpiWrRdParams->nReadBytes;
    pSpiRxInfo->nDataSplitSizeInBytes       = nMaxDataSize - nRem;
    pSpiRxInfo->nNumberOfDataBlocks         = nActualDataSizeInBytes / pSpiRxInfo->nDataSplitSizeInBytes;
    pSpiRxInfo->nNumberOfLeftOutDataInBytes = nActualDataSizeInBytes % pSpiRxInfo->nDataSplitSizeInBytes;
    pSpiRxInfo->nDataBlockIdx				= 0U;
	if (pSpiRxInfo->nNumberOfDataBlocks != 0U)
	{
		nRead = (a2b_UInt16)pSpiRxInfo->nDataSplitSizeInBytes;
	}
	else
	{
		nRead = (a2b_UInt16)pSpiRxInfo->nNumberOfLeftOutDataInBytes;
	}

    /* Get corresponding SPI command */
    eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, A2B_TRUE);

	result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, pSpiWrRdParams->nWriteBytes, &pSpiWrRdParams->pWBuf[0U], nRead, &pSpiWrRdParams->pRBuf[0U]);
	if(A2B_FAILED(result))
	{
		/* Add TRACE */
	}
    else
    {
		pSpiRxInfo->nDataBlockIdx++;
		pSpiInfo->oSpiRxInfo.nCurrReadBytes		+= nRead;
		pSpiInfo->oSpiRxInfo.bfPayloadPendingRx  = A2B_TRUE;
    }

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral full duplex transactions (Blocking Mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult spiFdBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_UInt32	nDataSplitSizeInBytes = 0u, nNumberOfDataBlocks = 0u, nNumberOfLeftOutDataInBytes = 0u;
	a2b_UInt32	nDataBlockIdx,nActualDataSizeInBytes;
	a2b_SpiCmd 	eSpiCmd;
	a2b_HResult result  = A2B_RESULT_SUCCESS;
	a2b_SpiInfo	*pSpiInfo;

	pSpiInfo 					= &pCtx->stk->oSpiInfo;
    nActualDataSizeInBytes		= pSpiWrRdParams->nWriteBytes;
    nDataSplitSizeInBytes       = A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES;
    nNumberOfDataBlocks         = nActualDataSizeInBytes / nDataSplitSizeInBytes;
    nNumberOfLeftOutDataInBytes = nActualDataSizeInBytes % nDataSplitSizeInBytes;

    /* Get corresponding SPI command */
    eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, A2B_TRUE);

	for (nDataBlockIdx = 0u; nDataBlockIdx < nNumberOfDataBlocks; nDataBlockIdx++)
	{
		if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED)
		{
			result = a2b_prepForSpiFdRegBasedAccess(pCtx, nDataSplitSizeInBytes - 1u, pSpiWrRdParams);
		}
		/* Use normal full duplex mode for writing the final block */
		if((nDataBlockIdx == nNumberOfDataBlocks - 1U) && (nNumberOfLeftOutDataInBytes == 0u)  && (eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED))
		{
			eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED;
		}
		if (A2B_SUCCEEDED(result))
		{
			result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, nDataSplitSizeInBytes, &pSpiWrRdParams->pWBuf[nDataBlockIdx * nDataSplitSizeInBytes],
										nDataSplitSizeInBytes, &pSpiWrRdParams->pRBuf[nDataBlockIdx * nDataSplitSizeInBytes]);

	    	if(pSpiInfo->eDTOptimizationMode == A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED)
	    	{
	    		if(A2B_SUCCEEDED(result))
				{
					/* Check for any SPI error interrupt */
					result = a2b_checkSpiErrorInt(pCtx);
				}
	    	}
		}
		if(A2B_FAILED(result))
		{
			break;
		}
	}

	if((A2B_SUCCEEDED(result)) && (nNumberOfLeftOutDataInBytes != 0u))
	{
		if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED)
		{
			result = a2b_prepForSpiFdRegBasedAccess(pCtx, nNumberOfLeftOutDataInBytes, pSpiWrRdParams);
		}
		/* Use normal full duplex mode for writing the final block */
		else if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED)
    	{
    		eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED;
    	}
		else
		{
			/* Do nothing */
		}

		if(A2B_SUCCEEDED(result))
		{
			result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, nNumberOfLeftOutDataInBytes, &pSpiWrRdParams->pWBuf[nNumberOfDataBlocks * nDataSplitSizeInBytes],
													nNumberOfLeftOutDataInBytes, &pSpiWrRdParams->pRBuf[nNumberOfDataBlocks * nDataSplitSizeInBytes]);
	    	if(pSpiInfo->eDTOptimizationMode == A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED)
	    	{
	    		if(A2B_SUCCEEDED(result))
				{
					/* Check for any SPI error interrupt */
					result = a2b_checkSpiErrorInt(pCtx);
				}
	    	}
		}
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral full duplex transactions (Non-Blocking Mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult spiFdNonBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_SpiInfo		*pSpiInfo;
	a2b_SpiFdInfo	*pSpiFdInfo;
	a2b_UInt32		nActualDataSizeInBytes;
	a2b_SpiCmd 		eSpiCmd;
	a2b_HResult 	result  = A2B_RESULT_SUCCESS;
	a2b_UInt16 		nFdBytes;

	pSpiInfo 					= &pCtx->stk->oSpiInfo;
	pSpiFdInfo					= &pSpiInfo->oSpiFdInfo;

    nActualDataSizeInBytes					= pSpiWrRdParams->nWriteBytes;
    pSpiFdInfo->nDataSplitSizeInBytes       = A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES;
    pSpiFdInfo->nNumberOfDataBlocks         = nActualDataSizeInBytes / pSpiFdInfo->nDataSplitSizeInBytes;
    pSpiFdInfo->nNumberOfLeftOutDataInBytes = nActualDataSizeInBytes % pSpiFdInfo->nDataSplitSizeInBytes;
    pSpiFdInfo->nDataBlockIdx				= 0U;
	if(pSpiFdInfo->nNumberOfDataBlocks != 0U)
	{
		nFdBytes = pSpiFdInfo->nDataSplitSizeInBytes;
		  /* Get corresponding SPI command */
   	 	eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, A2B_TRUE);
		if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED)
		{
			result = a2b_prepForSpiFdRegBasedAccess(pCtx, nFdBytes - 1U, pSpiWrRdParams);
		}
	}
	else
	{
		nFdBytes = pSpiFdInfo->nDataSplitSizeInBytes;
		  /* Get corresponding SPI command */
   	 	eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, A2B_TRUE);
		if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED)
		{
			result = a2b_prepForSpiFdRegBasedAccess(pCtx, nFdBytes, pSpiWrRdParams);
		}
		nFdBytes = pSpiFdInfo->nNumberOfLeftOutDataInBytes;
	}

	if((pSpiFdInfo->nNumberOfDataBlocks == 1U) && (eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED ))
	{
		eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED;
	}

	if (A2B_SUCCEEDED(result))
	{
		result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, (a2b_UInt16)eSpiCmd, 0U /* ChipAddr Unused */, (a2b_UInt16)pSpiWrRdParams->nSlaveSel, nFdBytes, &pSpiWrRdParams->pWBuf[0U], nFdBytes, &pSpiWrRdParams->pRBuf[0U]);
		if(A2B_FAILED(result))
		{
			/* Add TRACE */
		}
		else
		{
			pSpiFdInfo->nDataBlockIdx++;
			pSpiInfo->oSpiFdInfo.nCurrFdBytes		+= nFdBytes;
			pSpiInfo->oSpiFdInfo.bfPayloadPendingFd  = A2B_TRUE;
		}
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral write-only, write-read or
				full duplex modes (Blocking mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_spiWrRdBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult 	result = A2B_RESULT_SUCCESS;
	a2b_Bool 		bSpiWrRdMode;
	A2B_SPI_MODE	eSpiMode;

	/* Checking for SPI write only/write-read mode */
	bSpiWrRdMode = a2b_checkWrRdMode(pSpiWrRdParams);
	if(bSpiWrRdMode == A2B_TRUE)
	{
		eSpiMode = pCtx->stk->oSpiInfo.eSpiMode;
		if((eSpiMode == A2B_SPI_ATOMIC) || (eSpiMode == A2B_SPI_BULK) || (eSpiMode == A2B_SPI_BULK_EXTENDED))
		{
			result = spiWrRdBlock(pCtx, pSpiWrRdParams);
		}
		else if((eSpiMode == A2B_SPI_FD_CMD_BASED) || (eSpiMode == A2B_SPI_FD_REG_BASED) || (eSpiMode == A2B_SPI_FD_CMD_BASED_EXTENDED))
		{
			result = spiFdBlock(pCtx, pSpiWrRdParams);
		}
		else
		{
			/* Do nothing */
		}
	}
	else
	{
		result = spiWrBlock(pCtx, pSpiWrRdParams);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI to SPI peripheral write-only, write-read or
				full duplex modes (Non-Blocking mode)

@param [in]     pCtx				Pointer to stack context
@param [in]     pSpiWrRdParams		Pointer to SPI write read parameter structure

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_spiWrRdNonBlock(a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult 	result = A2B_RESULT_SUCCESS;
	a2b_Bool 		bSpiWrRdMode;
	A2B_SPI_MODE	eSpiMode;

	bSpiWrRdMode = a2b_checkWrRdMode(pSpiWrRdParams);
	eSpiMode = pCtx->stk->oSpiInfo.eSpiMode;
	if(bSpiWrRdMode == A2B_TRUE)
	{
		if((eSpiMode == A2B_SPI_ATOMIC) || (eSpiMode == A2B_SPI_BULK)|| (eSpiMode == A2B_SPI_BULK_EXTENDED))
		{
			result = spiWrRdNonBlock(pCtx, pSpiWrRdParams);
		}
		else if((eSpiMode == A2B_SPI_FD_CMD_BASED) || (eSpiMode == A2B_SPI_FD_REG_BASED) || (eSpiMode == A2B_SPI_FD_CMD_BASED_EXTENDED))
		{
			result = spiFdNonBlock(pCtx, pSpiWrRdParams);
		}
		else
		{
			/* Do nothing */
		}

	}
	else
	{
		result = spiWrNonBlock(pCtx, pSpiWrRdParams);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles if there is a pending payload which has to be transmitted.
				This is used in SPI Non-Blocking write-only transactions

@param [in]     pCtx				Pointer to stack context
@param [in]     nNodeAddr			SPI transaction target node number

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_handlePayloadPendingTx(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr)
{
	a2b_HResult 		result = A2B_RESULT_SUCCESS;
	a2b_SpiCmd 			eSpiCmd;
	a2b_SpiInfo			*pSpiInfo;
	a2b_SpiTxInfo		*pSpiTxInfo;
	a2b_SpiWrRdParams 	*pSpiWrRdParams;
	a2b_Bool 			bSpiWrRdMode = A2B_TRUE;
	a2b_UInt8			nAddrWidth, nAddrOffset, nUsrSpiCmdAndAddrBytes;
	a2b_UInt32			nDataSplitSizeInBytes, nNumberOfDataBlocks, nNumberOfLeftOutDataInBytes, nDataBlockIdx, nTempAddr, nBaseAddr = 0u, nAddrIncremBytes;
	a2b_UInt8 			anTempWbuf[A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES];

	pSpiInfo		= &pCtx->stk->oSpiInfo;
	pSpiTxInfo		= &pSpiInfo->oSpiTxInfo;
	pSpiWrRdParams  = &pSpiInfo->oSpiWrRdParams;

	/* Check write read mode */
	bSpiWrRdMode = a2b_checkWrRdMode(pSpiWrRdParams);

    /* Get corresponding SPI command */
    eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, bSpiWrRdMode);

	if(pSpiTxInfo->nCurrWriteBytes == pSpiWrRdParams->nWriteBytes)
	{
		pSpiInfo->pfStatCb(pCtx, A2B_SPI_EVENT_TX_DONE, nNodeAddr);

		/* Reset the SPI transmission state params */
		a2b_rstSpiTxInfo(pSpiTxInfo);
	}
	else
	{
		nDataSplitSizeInBytes		= pSpiTxInfo->nDataSplitSizeInBytes;
		nNumberOfDataBlocks			= pSpiTxInfo->nNumberOfDataBlocks;
		nNumberOfLeftOutDataInBytes	= pSpiTxInfo->nNumberOfLeftOutDataInBytes;
		nDataBlockIdx				= pSpiTxInfo->nDataBlockIdx;
		nAddrIncremBytes          	= (nDataSplitSizeInBytes / pSpiWrRdParams->nAddrIncrement);

		nAddrWidth          		= pSpiWrRdParams->nAddrWidth;
		nAddrOffset         		= pSpiWrRdParams->nAddrOffset;
		nUsrSpiCmdAndAddrBytes		= nAddrWidth + nAddrOffset;

	    /* Get base address and concatenate the address into a2b_UInt32 variable */
		a2b_concatAddr(&pSpiWrRdParams->pWBuf[nAddrOffset], nAddrWidth, &nBaseAddr);

		if(nDataBlockIdx < nNumberOfDataBlocks)
		{
	    	/* Increment the address from which next chunk of data to be written */
	    	nTempAddr = nBaseAddr + nDataBlockIdx * nAddrIncremBytes;

	    	/* Copy the user SPI cmd and the address bytes */
	    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], (a2b_Size)nUsrSpiCmdAndAddrBytes);

	    	/* Split updated address into byte array */
	    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

			/* Copy the data bytes after the user SPI cmd and the address bytes */
			(void)a2b_memcpy(&anTempWbuf[nUsrSpiCmdAndAddrBytes], &pSpiWrRdParams->pWBuf[(nDataBlockIdx * nDataSplitSizeInBytes) + (nUsrSpiCmdAndAddrBytes)], nDataSplitSizeInBytes);

	    	/* Use normal bulk mode for writing the final block */
	    	if((nDataBlockIdx == nNumberOfDataBlocks - 1U) && (nNumberOfLeftOutDataInBytes == 0u)  && (eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED))
	    	{
	    		eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE;
	    	}

			result = a2b_spiPeriphWrite(pCtx, pSpiWrRdParams->nNode, eSpiCmd, 0U /* ChipAddr Unused */, pSpiWrRdParams->nSlaveSel, (nDataSplitSizeInBytes + nUsrSpiCmdAndAddrBytes), &anTempWbuf);
			if(A2B_FAILED(result))
			{
				return (result);
			}
			else
			{
				pSpiTxInfo->nDataBlockIdx++;
				pSpiTxInfo->nCurrWriteBytes		+= nDataSplitSizeInBytes;
				pSpiTxInfo->bfPayloadPendingTx   = A2B_TRUE;
			}
		}
		else
		{
	    	/* Increment the address from which next chunk of data to be written */
	    	nTempAddr = nBaseAddr + nNumberOfDataBlocks * nAddrIncremBytes;

	    	/* Copy the user SPI cmd and the address bytes */
	    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], (a2b_Size)nUsrSpiCmdAndAddrBytes);

	    	/* Split updated address into byte array */
	    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

			/* Copy the data bytes after the user SPI cmd and the address bytes */
			(void)a2b_memcpy(&anTempWbuf[nUsrSpiCmdAndAddrBytes], &pSpiWrRdParams->pWBuf[(nNumberOfDataBlocks * nDataSplitSizeInBytes) + (nUsrSpiCmdAndAddrBytes)], (a2b_Size)nNumberOfLeftOutDataInBytes);

			/* Use normal bulk mode for writing the final block */
			if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED)
			{
				eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE;
			}

	    	result = a2b_spiPeriphWrite(pCtx, pSpiWrRdParams->nNode, eSpiCmd, 0U /* ChipAddr Unused */, pSpiWrRdParams->nSlaveSel, (nNumberOfLeftOutDataInBytes + nUsrSpiCmdAndAddrBytes), &anTempWbuf);
			if(A2B_FAILED(result))
			{
	        	return (result);
			}
	        else
	        {
	        	pSpiTxInfo->nCurrWriteBytes		+= nNumberOfLeftOutDataInBytes;
	        	pSpiTxInfo->bfPayloadPendingTx   = A2B_TRUE;
	        }
		}
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles if there is a pending payload which has to be received.
				This is used in SPI Non-Blocking write-read transactions

@param [in]     pCtx				Pointer to stack context
@param [in]     nNodeAddr			SPI transaction target node number

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_handlePayloadPendingRx(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr)
{
	a2b_HResult 		result = A2B_RESULT_SUCCESS;
	a2b_SpiCmd 			eSpiCmd;
	a2b_SpiInfo			*pSpiInfo;
	a2b_SpiRxInfo		*pSpiRxInfo;
	a2b_SpiWrRdParams 	*pSpiWrRdParams;
	a2b_Bool 			bSpiWrRdMode = A2B_TRUE;
	a2b_UInt8			nAddrWidth, nAddrOffset, nUsrSpiCmdAndAddrBytes;
	a2b_UInt32			nTempAddr, nDataSplitSizeInBytes, nNumberOfDataBlocks, nNumberOfLeftOutDataInBytes, nDataBlockIdx, nBaseAddr = 0u, nAddrIncremBytes;
	a2b_UInt8 			anTempWbuf[A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES];

	pSpiInfo		= &pCtx->stk->oSpiInfo;
	pSpiRxInfo		= &pSpiInfo->oSpiRxInfo;
	pSpiWrRdParams  = &pSpiInfo->oSpiWrRdParams;

	nAddrWidth          		= pSpiWrRdParams->nAddrWidth;
	nAddrOffset         		= pSpiWrRdParams->nAddrOffset;
	nUsrSpiCmdAndAddrBytes		= nAddrWidth + nAddrOffset;

	/* Check write read mode */
	bSpiWrRdMode = a2b_checkWrRdMode(pSpiWrRdParams);

    /* Get corresponding SPI command */
    eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, bSpiWrRdMode);

    /* Get base address and concatenate the address into a2b_UInt32 variable */
	a2b_concatAddr(&pSpiWrRdParams->pWBuf[nAddrOffset], nAddrWidth, &nBaseAddr);

	if(pSpiRxInfo->nCurrReadBytes == pSpiWrRdParams->nReadBytes)
	{
		pSpiInfo->pfStatCb(pCtx, A2B_SPI_EVENT_RX_MSG, nNodeAddr);

		/* Reset the SPI reception state params */
		a2b_rstSpiRxInfo(pSpiRxInfo);
	}
	else
	{
		nDataSplitSizeInBytes		= pSpiRxInfo->nDataSplitSizeInBytes;
		nNumberOfDataBlocks			= pSpiRxInfo->nNumberOfDataBlocks;
		nNumberOfLeftOutDataInBytes	= pSpiRxInfo->nNumberOfLeftOutDataInBytes;
		nDataBlockIdx				= pSpiRxInfo->nDataBlockIdx;
		nAddrIncremBytes          	= (nDataSplitSizeInBytes / pSpiWrRdParams->nAddrIncrement);

		if(nDataBlockIdx < nNumberOfDataBlocks)
		{
	    	/* Increment the address from which next chunk of data to be written */
	    	nTempAddr = nBaseAddr + nDataBlockIdx * nAddrIncremBytes;

	    	/* Copy the user SPI cmd and the address bytes */
	    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], nUsrSpiCmdAndAddrBytes);

	    	/* Split updated address into byte array */
	    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

	    	result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, eSpiCmd, 0U /* ChipAddr Unused */, pSpiWrRdParams->nSlaveSel, pSpiWrRdParams->nWriteBytes, &anTempWbuf,
	    									nDataSplitSizeInBytes, &pSpiWrRdParams->pRBuf[nDataBlockIdx * nDataSplitSizeInBytes]);
			if(A2B_FAILED(result))
			{
		    	return (result);
			}
		    else
		    {
				pSpiRxInfo->nDataBlockIdx++;
				pSpiRxInfo->nCurrReadBytes		+= nDataSplitSizeInBytes;
				pSpiRxInfo->bfPayloadPendingRx  = A2B_TRUE;
		    }
		}
		else
		{
	    	/* Increment the address from which next chunk of data to be written */
	    	nTempAddr = nBaseAddr + nNumberOfDataBlocks * nAddrIncremBytes;

	    	/* Copy the user SPI cmd and the address bytes */
	    	(void)a2b_memcpy(&anTempWbuf[0U], &pSpiWrRdParams->pWBuf[0U], nUsrSpiCmdAndAddrBytes);

	    	/* Split updated address into byte array */
	    	a2b_spiltAddr(&anTempWbuf[nAddrOffset], nAddrWidth, nTempAddr);

	    	result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, eSpiCmd, 0U /* ChipAddr Unused */, pSpiWrRdParams->nSlaveSel, pSpiWrRdParams->nWriteBytes, &anTempWbuf,
	    									nNumberOfLeftOutDataInBytes, &pSpiWrRdParams->pRBuf[nNumberOfDataBlocks * nDataSplitSizeInBytes]);
			if(A2B_FAILED(result))
			{
	        	return (result);
			}
	        else
	        {
	        	pSpiRxInfo->nCurrReadBytes		+= nNumberOfLeftOutDataInBytes;
	        	pSpiRxInfo->bfPayloadPendingRx  = A2B_TRUE;
	        }
		}
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles if there is a pending payload which has to be transmitted and received.
				This is used in SPI Non-Blocking full duplex transactions

@param [in]     pCtx				Pointer to stack context
@param [in]     nNodeAddr			SPI transaction target node number

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_handlePayloadPendingFd(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr)
{
	a2b_HResult 		result = A2B_RESULT_SUCCESS;
	a2b_SpiCmd 			eSpiCmd;
	a2b_SpiInfo			*pSpiInfo;
	a2b_SpiFdInfo		*pSpiFdInfo;
	a2b_SpiWrRdParams 	*pSpiWrRdParams;
	a2b_Bool 			bSpiWrRdMode = A2B_TRUE;
	a2b_UInt32			nDataSplitSizeInBytes, nNumberOfDataBlocks, nNumberOfLeftOutDataInBytes, nDataBlockIdx;

	pSpiInfo		= &pCtx->stk->oSpiInfo;
	pSpiFdInfo		= &pSpiInfo->oSpiFdInfo;
	pSpiWrRdParams  = &pSpiInfo->oSpiWrRdParams;

	/* Check write read mode */
	bSpiWrRdMode = a2b_checkWrRdMode(pSpiWrRdParams);

    /* Get corresponding SPI command */
    eSpiCmd = a2b_getSpiCmd(pCtx, pSpiInfo->eSpiMode, bSpiWrRdMode);

	if(pSpiFdInfo->nCurrFdBytes == pSpiWrRdParams->nReadBytes)
	{
		pSpiInfo->pfStatCb(pCtx, A2B_SPI_EVENT_FD_MSG, nNodeAddr);

		/* Reset the SPI reception state params */
		a2b_rstSpiFdInfo(pSpiFdInfo);
	}
	else
	{
		nDataSplitSizeInBytes		= pSpiFdInfo->nDataSplitSizeInBytes;
		nNumberOfDataBlocks			= pSpiFdInfo->nNumberOfDataBlocks;
		nNumberOfLeftOutDataInBytes	= pSpiFdInfo->nNumberOfLeftOutDataInBytes;
		nDataBlockIdx				= pSpiFdInfo->nDataBlockIdx;

		if(nDataBlockIdx < nNumberOfDataBlocks)
		{
			if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED)
			{
				result = a2b_prepForSpiFdRegBasedAccess(pCtx, nDataSplitSizeInBytes, pSpiWrRdParams);
				if(A2B_FAILED(result))
	    		{
					return (result);
				}
			}

			/* Update the command for last transaction from extended mode to normal mode for final block write */
			if((nDataBlockIdx == nNumberOfDataBlocks - 1U) && (nNumberOfLeftOutDataInBytes == 0u)  && (eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED))
			{
				eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED;
			}

			result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, eSpiCmd, 0U /* ChipAddr Unused */, pSpiWrRdParams->nSlaveSel, nDataSplitSizeInBytes, &pSpiWrRdParams->pWBuf[nDataBlockIdx * nDataSplitSizeInBytes],
											nDataSplitSizeInBytes, &pSpiWrRdParams->pRBuf[nDataBlockIdx * nDataSplitSizeInBytes]);
			if(A2B_FAILED(result))
    		{
				return (result);
			}
		    else
		    {
				pSpiFdInfo->nDataBlockIdx++;
				pSpiFdInfo->nCurrFdBytes		+= nDataSplitSizeInBytes;
				pSpiFdInfo->bfPayloadPendingFd  = A2B_TRUE;
		    }
		}
		else
		{
			if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED)
			{
				result = a2b_prepForSpiFdRegBasedAccess(pCtx, nNumberOfLeftOutDataInBytes, pSpiWrRdParams);
				if(A2B_FAILED(result))
				{
					return (result);
				}
			}
			/* Update the command for last transaction from extended mode to normal mode */
			else if(eSpiCmd == A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED)
			{
				eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED;
			}

			else
			{
				/* Do nothing */
			}

	    	result = a2b_spiPeriphWriteRead(pCtx, pSpiWrRdParams->nNode, eSpiCmd, 0U /* ChipAddr Unused */, pSpiWrRdParams->nSlaveSel, nNumberOfLeftOutDataInBytes, &pSpiWrRdParams->pWBuf[nNumberOfDataBlocks * nDataSplitSizeInBytes],
	    									nNumberOfLeftOutDataInBytes, &pSpiWrRdParams->pRBuf[nNumberOfDataBlocks * nDataSplitSizeInBytes]);
			if(A2B_FAILED(result))
			{
	        	return (result);
			}
	        else
	        {
	        	pSpiFdInfo->nCurrFdBytes		+= nNumberOfLeftOutDataInBytes;
	        	pSpiFdInfo->bfPayloadPendingFd  = A2B_TRUE;
	        }
		}
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI done interrupt case

@param [in]     pCtx				Pointer to stack context
@param [in]     nNodeAddr			SPI transaction target node number

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static a2b_HResult a2b_handleSpiDoneIrpt(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr)
{
	a2b_HResult			result = A2B_RESULT_SUCCESS;
	a2b_SpiInfo			*pSpiInfo;
	a2b_SpiRxInfo		*pSpiRxInfo;
	a2b_SpiWrRdParams 	*pSpiWrRdParams;
	a2b_UInt32			nDataSplitSizeInBytes, nNumberOfDataBlocks, nNumberOfLeftOutDataInBytes, nDataBlockIdx;

	pSpiInfo 		= &pCtx->stk->oSpiInfo;
	pSpiRxInfo		= &pSpiInfo->oSpiRxInfo;
	pSpiWrRdParams  = &pSpiInfo->oSpiWrRdParams;

    /* Stop the previously running timer */
    a2b_timerStop(pCtx->stk->oSpiInfo.pSpiToSpiPeriConfigTimeoutTimer);

    /* If a split transaction required flag set */
    if(pCtx->stk->oSpiInfo.fSplitTransRequired)
    {
    	pCtx->stk->oSpiInfo.fSplitTransRequired = A2B_FALSE;

		nDataSplitSizeInBytes		= pSpiRxInfo->nDataSplitSizeInBytes;
		nNumberOfDataBlocks			= pSpiRxInfo->nNumberOfDataBlocks;
		nNumberOfLeftOutDataInBytes	= pSpiRxInfo->nNumberOfLeftOutDataInBytes;
		nDataBlockIdx				= pSpiRxInfo->nDataBlockIdx;

		if(nDataBlockIdx < nNumberOfDataBlocks)
		{
			result = a2b_spiDtFifoRead(pCtx, nDataSplitSizeInBytes, &pSpiWrRdParams->pRBuf[(nDataBlockIdx-1) * nDataSplitSizeInBytes]);
		}
		else
		{
			if(nNumberOfLeftOutDataInBytes == 0)
			{
				nNumberOfLeftOutDataInBytes = nDataSplitSizeInBytes;
			}
			result = a2b_spiDtFifoRead(pCtx, nNumberOfLeftOutDataInBytes, &pSpiWrRdParams->pRBuf[(nNumberOfDataBlocks - 1) * nDataSplitSizeInBytes]);
		}
		if(A2B_FAILED(result))
		{
        	return (result);
		}

		result = a2b_handlePayloadPendingRx(pCtx, nNodeAddr);
    }
    else
    {
    	/* Payload pending transmission or reception or full duplex */
    	if(pSpiInfo->oSpiTxInfo.bfPayloadPendingTx)
    	{
    		result = a2b_handlePayloadPendingTx(pCtx, nNodeAddr);
    	}

    	if ((pSpiInfo->oSpiRxInfo.bfPayloadPendingRx) && A2B_SUCCEEDED(result))
    	{
    		result = a2b_handlePayloadPendingRx(pCtx, nNodeAddr);
    	}

    	if((pSpiInfo->oSpiFdInfo.bfPayloadPendingFd) && A2B_SUCCEEDED(result))
    	{
    		result = a2b_handlePayloadPendingFd(pCtx, nNodeAddr);
    	}
    }

    return (result);
}

/*****************************************************************************/
/*!
@brief			This function handles SPI data tunnel error case

@param [in]     pCtx				Pointer to stack context
@param [in]     nNodeAddr			SPI transaction target node number
@param [out]    pSpiEvent			Pointer to SPI event

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
static void a2b_handleSpiDataTunErr(a2b_StackContext* pCtx, a2b_Int8 nNodeAddr, A2B_SPI_EVENT *pSpiEvent)
{
    a2b_UInt8	nRBuf;
    a2b_HResult	result;
	result = a2b_spiStatusRead(pCtx, A2B_SPI_SLV_SEL, 1u, &nRBuf);

	if(A2B_FAILED(result))
	{
		pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_FAILURE, nNodeAddr);
		*pSpiEvent = A2B_SPI_EVENT_FAILURE;
	}
	else
	{
		if(nRBuf & (a2b_UInt8)A2B_BITM_SPISTAT_SPIBUSY)
		{
			pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_DATA_TUNNEL_SPIBUSY, nNodeAddr);
			*pSpiEvent = A2B_SPI_EVENT_DATA_TUNNEL_SPIBUSY;
		}
		else if(nRBuf & (a2b_UInt8)A2B_BITM_SPISTAT_DTACTIVE)
		{
			pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_DATA_TUNNEL_ACTIVE, nNodeAddr);
			*pSpiEvent = A2B_SPI_EVENT_DATA_TUNNEL_ACTIVE;
		}
		else if(nRBuf & (a2b_UInt8)A2B_BITM_SPISTAT_DTINVALID)
		{
			pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_DATA_TUNNEL_INVALID, nNodeAddr);
			*pSpiEvent = A2B_SPI_EVENT_DATA_TUNNEL_INVALID;
		}
		else if(nRBuf & (a2b_UInt8)A2B_BITM_SPISTAT_DTABORT)
		{
			pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_DATA_TUNNEL_ABORT, nNodeAddr);
			*pSpiEvent = A2B_SPI_EVENT_DATA_TUNNEL_ABORT;
		}
		else if(nRBuf & (a2b_UInt8)A2B_BITM_SPISTAT_DTBADPKT)
		{
			pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_DATA_TUNNEL_BADPKT, nNodeAddr);
			*pSpiEvent = A2B_SPI_EVENT_DATA_TUNNEL_BADPKT;
		}
		else
		{
			pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_FAILURE, nNodeAddr);
			*pSpiEvent = A2B_SPI_EVENT_FAILURE;
		}
	}

}

/*****************************************************************************/
/*!
@brief			This function handles SPI interrupt case. It handles SPI done interrupts and other SPI error interrupts

@param [in]     pCtx				Pointer to stack context
@param [in]     pHnd				Void pointer
@param [in]     nIntrSrc			Interrupt source
@param [in]     nIntrType			Interrupt type

@return			A2B_SPI_RET type
                - 0: A2B_SPI_SUCCESS
                - 1: A2B_SPI_FAILED
*/
/*****************************************************************************/
A2B_DSO_LOCAL void a2b_spiPeriInterrupt(struct a2b_StackContext* pCtx, a2b_Handle pHnd, a2b_UInt8 nIntrSrc, a2b_UInt8 nIntrType)
{
	a2b_HResult 	result = A2B_RESULT_SUCCESS ;
    a2b_Int16 		nodeAddr;
    A2B_SPI_EVENT 	eSpiEvent;
    a2b_Bool		bSpiOnGoingTransaction = A2B_FALSE, bfClrStateInfo = A2B_FALSE;
    a2b_SpiInfo		*pSpiInfo;

    A2B_UNUSED(nIntrSrc);
    A2B_UNUSED(pHnd);

    if ( (A2B_NULL != pCtx) && (pCtx->stk->oSpiInfo.pfStatCb))
    {
    	bSpiOnGoingTransaction = a2b_checkSpiOnGoingTransaction(pCtx);
    	if (bSpiOnGoingTransaction)
    	{
    		pSpiInfo = &pCtx->stk->oSpiInfo;
    		nodeAddr = pCtx->stk->oSpiInfo.oSpiWrRdParams.nNode;
			switch (nIntrType)
			{
				case A2B_ENUM_INTTYPE_SPIDONE:
					result = a2b_handleSpiDoneIrpt(pCtx, nodeAddr);
					if(A2B_FAILED(result))
		    		{
		            	pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_FAILURE, nodeAddr);
		    		}
					break;
				case A2B_ENUM_INTTYPE_SPI_REMOTE_REG_ERR:
					pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_REMOTE_REG_ACCESS_ERROR, nodeAddr);
					bfClrStateInfo = A2B_TRUE;
					break;
				case A2B_ENUM_INTTYPE_SPI_REMOTE_I2C_ERR:
					pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_REMOTE_I2C_ACCESS_ERROR, nodeAddr);
					bfClrStateInfo = A2B_TRUE;
					break;
				case A2B_ENUM_INTTYPE_SPI_DATA_TUN_ERR:
					a2b_handleSpiDataTunErr(pCtx, nodeAddr, &eSpiEvent);
					bfClrStateInfo = A2B_TRUE;
					break;
				case A2B_ENUM_INTTYPE_SPI_BAD_CMD:
					pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_BAD_COMMAND, nodeAddr);
					bfClrStateInfo = A2B_TRUE;
					break;
				case A2B_ENUM_INTTYPE_SPI_FIFO_OVRFLW:
					pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_FIFO_OVERFLOW, nodeAddr);
					bfClrStateInfo = A2B_TRUE;
					break;
				case A2B_ENUM_INTTYPE_SPI_FIFO_UNDERFLW:
					pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_FIFO_UNDERFLOW, nodeAddr);
					bfClrStateInfo = A2B_TRUE;
					break;
				default:
					pCtx->stk->oSpiInfo.pfStatCb(pCtx, A2B_SPI_EVENT_FAILURE, nodeAddr);
					bfClrStateInfo = A2B_TRUE;
					break;
			}

			if(bfClrStateInfo == A2B_TRUE)
			{
				/* Clear state information upon failure */
				a2b_rstSpiTxInfo(&pSpiInfo->oSpiTxInfo);
				a2b_rstSpiRxInfo(&pSpiInfo->oSpiRxInfo);
				a2b_rstSpiFdInfo(&pSpiInfo->oSpiFdInfo);
				a2b_rstSpiOthInfo(pSpiInfo);
			}
    	}
    }
}

/*****************************************************************************/
/*!
@brief			Generate/Start the SPI to SPI Peripheral Configuration timeout timer

@param [in]     pCtx				Pointer to stack context

@return			none
*/
/*****************************************************************************/
A2B_DSO_LOCAL void a2b_remoteDevConfigSpiToSpiStartTimer(struct a2b_StackContext *pCtx)
{
    a2b_UInt32 		delay = A2B_REMOTEDEVCONFIG_SPITOSPI_TIMEOUT_IN_MS;
    a2b_TimerFunc 	timerFunc = &a2b_onRemoteDevConfigSpiToSpiResponseTimeout;

    /* Stop the previously running timer */
    a2b_timerStop		(pCtx->stk->oSpiInfo.pSpiToSpiPeriConfigTimeoutTimer);

    /* Single shot timer */
    a2b_timerSet		(pCtx->stk->oSpiInfo.pSpiToSpiPeriConfigTimeoutTimer, delay, 0u);
    a2b_timerSetHandler	(pCtx->stk->oSpiInfo.pSpiToSpiPeriConfigTimeoutTimer, timerFunc);
    a2b_timerSetData	(pCtx->stk->oSpiInfo.pSpiToSpiPeriConfigTimeoutTimer, pCtx);
    a2b_timerStart		(pCtx->stk->oSpiInfo.pSpiToSpiPeriConfigTimeoutTimer);
} /* a2b_mailboxStartTimer */


/*****************************************************************************/
/*!
@brief			This function gets the requested SPI command based on SPI mode
				and write only or write-read transaction

@param [in]     pCtx		Pointer to stack context
@param [in]     eSpiMode	Enumeration of SPI mode
@param [in]     bWrRd		Write only or write-read mode indication

@return			a2b_SpiCmd type

*/
/*****************************************************************************/
A2B_DSO_LOCAL a2b_SpiCmd  a2b_getSpiCmd(struct a2b_StackContext *pCtx, A2B_SPI_MODE eSpiMode, a2b_Bool bWrRd)
{
	a2b_SpiCmd 	eSpiCmd;
	a2b_SpiInfo	*pSpiInfo;

	A2B_UNUSED(eSpiMode);

	pSpiInfo = &pCtx->stk->oSpiInfo;
    switch(pSpiInfo->eSpiMode)
    {
		case A2B_SPI_ATOMIC:
			if(bWrRd == A2B_TRUE)
			{
				eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST;
			}
			else
			{
				eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE;
			}
			break;
		case A2B_SPI_FD_CMD_BASED:
			eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED;
			break;
		case A2B_SPI_FD_REG_BASED:
			eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED;
			break;
		case A2B_SPI_BULK:
			if(bWrRd == A2B_FALSE)
			{
				eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE;
			}
			else
			{
				/* Fall back mechanism */
				eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST;
			}
			break;

		case A2B_SPI_BULK_EXTENDED:
			eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED;
			break;

		case A2B_SPI_FD_CMD_BASED_EXTENDED:
			eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED;
			break;

		default:
			eSpiCmd = A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE;
			break;
    }

    return (eSpiCmd);
}

/*****************************************************************************/
/*!
@brief			This function is used to allocate the SPI to SPI peripheral
				configuration time-out timer. This function shall be called only once

@param [in]     pCtx			Pointer to A2B stack context

*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*/
/*****************************************************************************/
A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriCreate(struct a2b_StackContext *pCtx)
{
	a2b_HResult result = A2B_RESULT_SUCCESS ;
	a2b_SpiInfo	*pSpiInfo;

	if(pCtx != A2B_NULL)
	{
		pSpiInfo 								  = &pCtx->stk->oSpiInfo;
		pSpiInfo->pSpiToSpiPeriConfigTimeoutTimer = a2b_timerAlloc(pCtx, A2B_NULL, pCtx);
	}
	else
	{
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function is used to register the call back function, SPI
				mode of operation etc, for SPI peripheral post discovery event handling

@param [in]     pCtx			Pointer to A2B stack context
@param [in]     pSpiConfig		Pointer to SPI configuration structure

*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*/
/*****************************************************************************/
A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriSetMode(struct a2b_StackContext *pCtx, a2b_SpiConfig *pSpiConfig)
{
	a2b_HResult result = A2B_RESULT_SUCCESS ;
	a2b_SpiInfo	*pSpiInfo;
    a2b_Bool	bSpiOnGoingTransaction = A2B_FALSE;

	if((pCtx != A2B_NULL) && (pSpiConfig != A2B_NULL))
	{
		bSpiOnGoingTransaction = a2b_checkSpiOnGoingTransaction(pCtx);
		if(bSpiOnGoingTransaction == A2B_FALSE)
		{
			if((pSpiConfig->eApiMode == A2B_API_NON_BLOCKING) && (pSpiConfig->pfStatCb == A2B_NULL))
			{
				result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
			}
			else
			{
				pSpiInfo 			= &pCtx->stk->oSpiInfo;
				pSpiInfo->pfStatCb	= pSpiConfig->pfStatCb;
				pSpiInfo->pCbParam	= pSpiConfig->pCbParam;
				pSpiInfo->eSpiMode	= pSpiConfig->eSpiMode;
				pSpiInfo->eApiMode	= pSpiConfig->eApiMode;
				pSpiInfo->eDTOptimizationMode = pSpiConfig->eDTOptimizationMode;

				a2b_rstSpiTxInfo(&pSpiInfo->oSpiTxInfo);
				a2b_rstSpiRxInfo(&pSpiInfo->oSpiRxInfo);
				a2b_rstSpiFdInfo(&pSpiInfo->oSpiFdInfo);
				a2b_rstSpiOthInfo(pSpiInfo);
			}
		}
		else
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_SPITOSPI_ONGOING_TRANSACTION);
		}
	}
	else
	{
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function is used get the current SPI & API mode of operation

@param [in]     pCtx			Pointer to A2B stack context
@param [out]    peSpiMode		Pointer to current SPI mode of operation
@param [out]    peApiMode		Pointer to current API mode of operation

*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*/
/*****************************************************************************/
A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriGetMode(struct a2b_StackContext *pCtx, A2B_SPI_MODE *peSpiMode, A2B_API_BLOCK_MODE *peApiMode)
{
	a2b_HResult result = A2B_RESULT_SUCCESS ;
	a2b_SpiInfo	*pSpiInfo;

	if(pCtx != A2B_NULL)
	{
		pSpiInfo 	= &pCtx->stk->oSpiInfo;

		*peSpiMode 	= pSpiInfo->eSpiMode;
		*peApiMode 	= pSpiInfo->eApiMode;
	}
	else
	{
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
	}

	return (result);
}

/*****************************************************************************/
/*!
@brief			This function is used write/write-read payload to/from
				peripherals connected to A2B nodes using SPI interface.

@param [in]     pCtx			Pointer to A2B stack context
@param [in]     pSpiWrRdParams	Pointer to SPI write read params structure

*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*/
/*****************************************************************************/
A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriWrRd(struct a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams)
{
	a2b_HResult result = A2B_RESULT_SUCCESS ;
	a2b_SpiInfo			*pSpiInfo;
	a2b_SpiWrRdParams	*pSpiInfoSpiWrRdParams;
	a2b_Bool			bSpiOnGoingTransaction = A2B_FALSE;
	
	if((pCtx != A2B_NULL) && (pSpiWrRdParams != A2B_NULL))
	{
		bSpiOnGoingTransaction = a2b_checkSpiOnGoingTransaction(pCtx);
		if (bSpiOnGoingTransaction == A2B_FALSE)
		{
			result = a2b_checkSpiParams(pCtx, pSpiWrRdParams);
			if (A2B_SUCCEEDED(result))
			{
				pSpiInfo 				 				= &pCtx->stk->oSpiInfo;
#if 0 /* SPI Error Interrupts Enable is now part of discovery sequence */
				/* Enable SPI error interrupts */
				if(pSpiInfo->eDTOptimizationMode == A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED)
				{
					result = a2b_enablSpiErrorInt(pCtx);
				}
#endif
				pSpiInfoSpiWrRdParams = &pSpiInfo->oSpiWrRdParams;

				pSpiInfoSpiWrRdParams->nNode = pSpiWrRdParams->nNode;
				pSpiInfoSpiWrRdParams->nSlaveSel = pSpiWrRdParams->nSlaveSel;
				pSpiInfoSpiWrRdParams->nWriteBytes = pSpiWrRdParams->nWriteBytes;
				pSpiInfoSpiWrRdParams->pWBuf = pSpiWrRdParams->pWBuf;
				pSpiInfoSpiWrRdParams->nReadBytes = pSpiWrRdParams->nReadBytes;
				pSpiInfoSpiWrRdParams->pRBuf = pSpiWrRdParams->pRBuf;
				pSpiInfoSpiWrRdParams->nAddrIncrement = pSpiWrRdParams->nAddrIncrement;
				pSpiInfoSpiWrRdParams->nAddrWidth = pSpiWrRdParams->nAddrWidth;
				pSpiInfoSpiWrRdParams->nAddrOffset = pSpiWrRdParams->nAddrOffset;

				if (pSpiInfo->eApiMode == A2B_API_BLOCKING)
				{
					result = a2b_spiWrRdBlock(pCtx, pSpiInfoSpiWrRdParams);
				}
				else  /* if(pSpiInfo->eApiMode == A2B_API_NON_BLOCKING)*/
				{
					result = a2b_spiWrRdNonBlock(pCtx, pSpiInfoSpiWrRdParams);
				
				}
			}
		}
		else
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_SPITOSPI_ONGOING_TRANSACTION);
		}
	}
	else
	{
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
	}

	return (result);
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
