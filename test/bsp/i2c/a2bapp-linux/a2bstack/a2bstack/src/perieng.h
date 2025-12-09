/*******************************************************************************
Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
 * @file:    perieng.h
 * @brief:   This  header file contains structure definitions for Tx and Rx state machines
 * @version: $Revision: 8410 $
 * @date:    $Date: 2018-10-15 16:22:08 +0530 (Mon, 15 Oct 2018) $
 * Developed by: Automotive Software and Systems team, Bangalore, India
*****************************************************************************/
/** \addtogroup PERI_Engine
 *  @{
 */

#ifndef ADI_A2B_PERIENG_H_
#define	ADI_A2B_PERIENG_H_

/*============= D E F I N E S =============*/
#include "a2b/spi.h"

/**************************** Private Structures *******************************/

/*! \struct a2b_SpiTxInfo
    Strcuture holding the SPI Tx information
*/
typedef struct a2b_SpiTxInfo
{
	/*!< Flag indicating the entire payload is not sent */
	a2b_Bool				bfPayloadPendingTx;
	
	/*!< Number of bytes transmitted during a message transmission */
	volatile a2b_UInt16		nCurrWriteBytes;
	
	/*!< Data split size in bytes */
	volatile a2b_UInt32		nDataSplitSizeInBytes;

	/*!< Actual number of data blocks */
	volatile a2b_UInt32		nNumberOfDataBlocks;

	/*!< Actual number of data in bytes which are left out */
	volatile a2b_UInt32		nNumberOfLeftOutDataInBytes;

	/*!< Current data block for which transmission is ongoing */
	volatile a2b_UInt32		nDataBlockIdx;

}a2b_SpiTxInfo;

/*! \struct a2b_SpiRxInfo
    Strcuture holding the SPI Rx information
*/
typedef struct a2b_SpiRxInfo
{
	/*!< Flag indicating the entire payload is not received */
	a2b_Bool				bfPayloadPendingRx;

	/*!< Number of bytes received during a message reception */
	volatile a2b_UInt16		nCurrReadBytes;
	
	/*!< Data split size in bytes */
	volatile a2b_UInt32		nDataSplitSizeInBytes;

	/*!< Actual number of data blocks */
	volatile a2b_UInt32		nNumberOfDataBlocks;

	/*!< Actual number of data in bytes which are left out */
	volatile a2b_UInt32		nNumberOfLeftOutDataInBytes;

	/*!< Current data block for which transmission is ongoing */
	volatile a2b_UInt32		nDataBlockIdx;

}a2b_SpiRxInfo;

/*! \struct a2b_SpiFdInfo
    Strcuture holding the SPI Fd information
*/
typedef struct a2b_SpiFdInfo
{
	/*!< Flag indicating the entire payload is not received */
	a2b_Bool				bfPayloadPendingFd;

	/*!< Number of bytes received during a message reception */
	volatile a2b_UInt16		nCurrFdBytes;

	/*!< Data split size in bytes */
	volatile a2b_UInt32		nDataSplitSizeInBytes;

	/*!< Actual number of data blocks */
	volatile a2b_UInt32		nNumberOfDataBlocks;

	/*!< Actual number of data in bytes which are left out */
	volatile a2b_UInt32		nNumberOfLeftOutDataInBytes;

	/*!< Current data block for which transmission is ongoing */
	volatile a2b_UInt32		nDataBlockIdx;

}a2b_SpiFdInfo;

/*! \struct a2b_SpiInfo
    Strcuture holding the various configured parameters of SPI
*/
typedef struct a2b_SpiInfo
{
	/* The below parameters will be used ONLY during discovery.
	 *
	 * These are required to check if ALL the SPI peripherals of ALL the slave nodes are configured
	 *
	 */

	/*!< Number of SPI to SPI peripheral configuration requests for slave nodes	*/
	a2b_UInt8			nSpiToSpiPeriConfigReq;

	/*!< SPI to SPI peripheral configuration request flag for slave nodes	*/
	a2b_Bool			fSpiToSpiPeriConfigReq[A2B_CONF_MAX_NUM_SLAVE_NODES];

	/*!< Number of SPI to SPI peripheral configuration responses from slave nodes	*/
	a2b_UInt8			nSpiToSpiPeriConfigResp;

	/*!< SPI to SPI peripheral configuration response flag from slave nodes	*/
	a2b_Bool			fSpiToSpiPeriConfigResp[A2B_CONF_MAX_NUM_SLAVE_NODES];


	/* The below parameters will be used ONLY post discovery.
	 *
	 * These are required to keep track of SPI to SPI peripheral transactions
	 *
	 */

	/*!< Pointer to status call back function used for indicating events to application */
	pfStatusCb			pfStatCb;

	/*!< Pointer to callback parameter passed during the callback */
	void*				pCbParam;
	
	/*!< SPI mode of operation */
	A2B_SPI_MODE		eSpiMode;
	
	/*!< API mode of operation */
	A2B_API_BLOCK_MODE	eApiMode;

	/*!< Data Tunnel Optimization flag */
	A2B_DT_OPTIMIZATION_MODE	eDTOptimizationMode;

	/*!< SPI Tx information */
	a2b_SpiTxInfo		oSpiTxInfo;
	
	/*!< SPI Rx information */
	a2b_SpiRxInfo		oSpiRxInfo;

	/*!< SPI Fd information */
	a2b_SpiFdInfo		oSpiFdInfo;

	/*!< Pointer to SPI write read paramters */
	a2b_SpiWrRdParams 	oSpiWrRdParams;

	/*!< Pointer to the timer for timeout on non acknowledgment of SPI to SPI peripheral transmission completion */
	struct a2b_Timer*  	pSpiToSpiPeriConfigTimeoutTimer;

	/*!< A flag indicating if a requested A2B SPI transaction mode requires a multiple A2B SPI transactions */
	a2b_Bool			fSplitTransRequired;

	a2b_Byte		     spiProcolBuf[A2B_MAX_LENINBYTES_SPI_TUNNEL_WRITE];

}a2b_SpiInfo;


/*======================= D A T A T Y P E S =======================*/



/*============= G L O B A L  P R O T O T Y P E S =============*/


#endif /* ADI_A2B_PERIENG_H_ */

/**@}*/
