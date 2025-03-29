/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @file: adi_a2b_usbdriver.h
* @brief: This file contains declarations required by USB driver.
* @version: $Revision: 1638 $
* @date: $Date: 2014-02-21 12:13:48 +0530 (Fri, 21 Feb 2014) $
* Developed by: Automotive Software and Systems team, Bangalore, India
*****************************************************************************/

/*! \addtogroup SPI
* @{
*/

#ifndef _ADI_A2B_SPIDRIVER_H_
#define _ADI_A2B_SPIDRIVER_H_

/*============= I N C L U D E S =============*/


/*============= D E F I N E S =============*/


/*============= D A T A T Y P E S=============*/
typedef enum
{
	ADI_A2B_SPI_NOP,
	ADI_A2B_SPI_READ,
	ADI_A2B_SPI_WRITE,
	ADI_A2B_SPI_WRITEREAD,
	ADI_A2B_SPI_FD
}ADI_A2B_SPI_OPSTATE;

/*============= P R O T O T Y P E =============*/

a2b_UInt32 adi_a2b_spiInit(A2B_ECB* ecb);
a2b_Handle adi_a2b_spiOpen(A2B_ECB* ecb);
a2b_UInt32 adi_a2b_spiRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nRead, a2b_Byte* rBuf);
a2b_UInt32 adi_a2b_spiWrite(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf);
a2b_UInt32 adi_a2b_spiWriteRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf);
a2b_UInt32 adi_a2b_spiFd(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf);
a2b_UInt32 adi_a2b_spiClose(a2b_Handle hnd);
a2b_UInt32 adi_a2b_spiInterruptDisable(a2b_UInt8 nValue);

/*============= D A T A =============*/


#endif /* _ADI_A2B_SPIDRIVER_H_ */



/** 
 @}
*/


/*
**
** EOF: adi_a2b_types.h
**
*/
