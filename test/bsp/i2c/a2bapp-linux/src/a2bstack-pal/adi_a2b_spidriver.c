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
//#include <drivers/spi/adi_spi.h>
#include "adi_a2b_datatypes.h"
#include "adi_a2b_framework.h"
#include "adi_a2b_externs.h"
#include "adi_a2b_spidriver.h"
#include "error.h"

/*============= D E F I N E S =============*/

#define SPI_INTERRPUT_SID		(0x15u)

#define SPI_BLOCKING_MODE		(0x01u)


/*============= D A T A =============*/

/*! SPI driver handle */
//static ADI_SPI_HANDLE hDevice = 0;

/*! SPI operation state */
static ADI_A2B_SPI_OPSTATE eOp;

/*! SPI driver memory */
//static uint8_t SPIDriverMemory[ADI_SPI_DMA_MEMORY_SIZE];

#pragma section("L3_data")
static a2b_UInt8 readBuf[512];
#pragma section("L3_data")
static a2b_UInt8 writeBuf[512];

/*============= C O D E =============*/
/*
** Function Prototype section
*/

/*
** Function Definition section
*/

uint32 adi_a2b_spiInit(A2B_ECB* ecb)
{
	A2B_UNUSED( ecb );

	//return A2B_RESULT_SUCCESS;
    return 0;
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

#if 0
	/* Driver API eResult code */
	ADI_SPI_RESULT eResult = ADI_SPI_FAILURE;

	if (adi_spi_Open(0u, SPIDriverMemory, (uint32_t)ADI_SPI_DMA_MEMORY_SIZE, &hDevice) == ADI_SPI_SUCCESS)
	{
		/* device in master of the SPI interface */
		eResult  = adi_spi_SetMaster(hDevice, true);

		/* SPI data transfers are 16 bit */
		eResult |= (uint32_t)adi_spi_SetWordSize(hDevice, ADI_SPI_TRANSFER_8BIT);

		/* IF (Set SPI Transfer initiate to Tx failed) */
		eResult |= (uint32_t)adi_spi_SetTransferInitiateMode (hDevice, ADI_SPI_TX_INITIATE_INTERRUPT) != ADI_SPI_SUCCESS;

		/* SPI slave select in controlled by software not hardware */
		eResult |= (uint32_t)adi_spi_SetHwSlaveSelect(hDevice, false);

		/* SPI slave select is on SPI slave select 1 pin */
		eResult |= (uint32_t)adi_spi_SetSlaveSelect(hDevice, ADI_SPI_SSEL_ENABLE1);

		/* SPI clock is SCLK divided by 1000 + 1 */
		eResult |= (uint32_t)adi_spi_SetClock(hDevice, 9u);

		eResult |= (uint32_t)adi_spi_SetClockPolarity(hDevice, false);

		/* interrupt mode, i.e no dma */
		eResult |= (uint32_t)adi_spi_EnableDmaMode(hDevice, false);
	}
	else
	{
		eResult = ADI_SPI_FAILURE;
	}

	if(eResult != ADI_SPI_SUCCESS)
	{
		hDevice = 0;
	}

	eOp = ADI_A2B_SPI_NOP;

	return ((a2b_Handle)hDevice);
#endif
    return malloc(100); //TODO
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

#if 0
	ADI_SPI_RESULT 				eResult;	/* Driver API eResult code */
	static ADI_SPI_TRANSCEIVER 	oSPIRead;	/* Transceiver configurations */

	oSPIRead.pPrologue 			= NULL;
	oSPIRead.PrologueBytes 		= 0;
	oSPIRead.pTransmitter 		= NULL;
	oSPIRead.TransmitterBytes 	= 0u;
	oSPIRead.pReceiver 			= rBuf;
	oSPIRead.ReceiverBytes 		= nRead;


    /* submit the SPI transceiver's buffers */
	eResult = adi_spi_SubmitBuffer(hDevice, &oSPIRead);

	eOp = ADI_A2B_SPI_READ;

#endif
    //return (uint32)eResult;
    return 0;
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

    return 0;
}

#pragma section("L1_code")
uint32 adi_a2b_spiWriteRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( hnd );
	A2B_UNUSED( addr );

    return 0;
}

#pragma section("L1_code")
uint32 adi_a2b_spiFd(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	A2B_UNUSED( hnd );
	A2B_UNUSED( addr );

    return 0;
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
	A2B_UNUSED( hnd );

    /* driver API eResult code */
	//ADI_SPI_RESULT eResult;

	/* close the SPI driver */
	//eResult = adi_spi_Close(hDevice);

	//eOp = ADI_A2B_SPI_NOP;
                            
    //return (uint32)eResult;
    return 0;
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




