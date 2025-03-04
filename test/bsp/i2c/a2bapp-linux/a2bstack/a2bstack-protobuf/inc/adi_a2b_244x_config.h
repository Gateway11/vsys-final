/*******************************************************************************
Copyright (c) 2023 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @file: adi_a2b_cpeconfig.h
* @brief: This file contains A2B CP Header information
* @version: $Revision$
* @date: $Date$
* BCF Version - 1.0.0
* A2B DLL version- 19.1.0
* A2B Stack DLL version- 19.1.0.0
* SigmaStudio version- 4.02.001.1764
* Developed by: Automotive Software and Systems team, Bangalore, India
* THIS IS A SIGMASTUDIO GENERATED FILE AND SHALL NOT BE MODIFIED OUTSIDE OF SIGMASTUDIO
*****************************************************************************/

/*! \addtogroup Target_Independent
* @{
*/

/*! \addtogroup Bus_Configuration Bus Configuration
* @{
*/

#include "adi_a2b_busconfig.h"

#ifndef _ADI_A2B_CPE_CP_H_
#define _ADI_A2B_CPE_CP_H_

/*! \addtogroup Target_Independent
* @{
*/

/** @defgroup CPE_Structure
*
* This module has structure definitions to describe SigmaStudio exported schematic(graph).
* The exported schematic is interpreted using the structure \ref ADI_A2B_244x_CP_CONFIG_STRUCT.
*
*/

/*! \addtogroup CPE_Structure Graph Structure
* @{
*/

/*============= D E F I N E S =============*/

/*! \struct GENERAL
 General Content Protection
 */
typedef struct
{
    a2b_UInt8 nREG_CTL;		/*CP Global Control Register*/
    a2b_UInt8 nREG_CFG;		/*CP Global Static Configuration Register*/
    a2b_UInt8 nREG_SAMPLE_RATE;		/*CP Audio Sample Rate Register*/
    a2b_UInt8 nREG_IRQ_CLR;		/*CP Interrupt Clear Register*/
    a2b_UInt8 nREG_I2SGCFG;		/*CP I2S Global Configuration Register*/
    a2b_UInt8 nREG_I2SCFG;		/*CP I2S Configuration Register*/
    a2b_UInt8 nREG_PINT_CFG;		/*CP Interrupt Pin Configuration Register*/
    a2b_UInt8 nREG_SLEW_FACTOR;		/*CP Slew factor for bit error handling*/
}GENERAL_REGS;

/*! \struct SOC_DRX0
 TDM Configuration for SoC Rx0 Path
 */
typedef struct
{
    a2b_UInt8 nREG_SOC_DRX0_CHAN0;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN1;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN2;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN3;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN4;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN5;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN6;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN7;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN8;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN9;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN10;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN11;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN12;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN13;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN14;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX0_CHAN15;		/*SOC Interface TX Path 0 TDM Channel Configuration Register*/
}SOC_DRX0_REGS;

/*! \struct SOC_DRX1
 TDM Configuration for SoC Rx1 Path
 */
typedef struct
{
    a2b_UInt8 nREG_SOC_DRX1_CHAN0;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN1;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN2;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN3;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN4;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN5;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN6;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN7;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN8;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN9;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN10;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN11;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN12;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN13;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN14;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DRX1_CHAN15;		/*SOC Interface TX Path 1 TDM Channel Configuration Register*/
}SOC_DRX1_REGS;

/*! \struct SOC_DTX0
 TDM Configuration for SoC Tx0 Path
 */
typedef struct
{
    a2b_UInt8 nREG_SOC_DTX0_CHAN0;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN1;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN2;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN3;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN4;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN5;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN6;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN7;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN8;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN9;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN10;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN11;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN12;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN13;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN14;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX0_CHAN15;		/*SOC Interface RX Path 0 TDM Channel Configuration Register*/
}SOC_DTX0_REGS;

/*! \struct SOC_DTX1
 TDM Configuration for SoC Tx1 Path
 */
typedef struct
{
    a2b_UInt8 nREG_SOC_DTX1_CHAN0;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN1;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN2;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN3;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN4;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN5;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN6;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN7;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN8;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN9;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN10;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN11;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN12;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN13;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN14;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_SOC_DTX1_CHAN15;		/*SOC Interface RX Path 1 TDM Channel Configuration Register*/
}SOC_DTX1_REGS;

/*! \struct A2B_DRX0
 TDM Configuration for A2B Rx0 Path
 */
typedef struct
{
    a2b_UInt8 nREG_A2B_DRX0_CHAN0;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN1;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN2;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN3;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN4;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN5;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN6;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN7;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN8;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN9;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN10;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN11;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN12;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN13;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN14;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX0_CHAN15;		/*A2B Interface TX Path 0 TDM Channel Configuration Register*/
}A2B_DRX0_REGS;

/*! \struct A2B_DRX1
 TDM Configuration for A2B Rx1 Path
 */
typedef struct
{
    a2b_UInt8 nREG_A2B_DRX1_CHAN0;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN1;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN2;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN3;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN4;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN5;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN6;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN7;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN8;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN9;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN10;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN11;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN12;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN13;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN14;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DRX1_CHAN15;		/*A2B Interface TX Path 1 TDM Channel Configuration Register*/
}A2B_DRX1_REGS;

/*! \struct A2B_DTX0
 TDM Configuration for A2B Tx0 Path
 */
typedef struct
{
    a2b_UInt8 nREG_A2B_DTX0_CHAN0;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN1;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN2;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN3;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN4;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN5;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN6;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN7;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN8;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN9;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN10;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN11;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN12;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN13;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN14;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX0_CHAN15;		/*A2B Interface RX Path 0 TDM Channel Configuration Register*/
}A2B_DTX0_REGS;

/*! \struct A2B_DTX1
 TDM Configuration for A2B Tx1 Path
 */
typedef struct
{
    a2b_UInt8 nREG_A2B_DTX1_CHAN0;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN1;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN2;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN3;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN4;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN5;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN6;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN7;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN8;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN9;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN10;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN11;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN12;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN13;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN14;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
    a2b_UInt8 nREG_A2B_DTX1_CHAN15;		/*A2B Interface RX Path 1 TDM Channel Configuration Register*/
}A2B_DTX1_REGS;

/*! \struct TXPORT
 TX Port Configuration for Content Protection Ports
 */
typedef struct
{
    a2b_UInt8 nREG_TX_CFG;		/*Virtual Transmit Port Configuration Register*/
    a2b_UInt8 nREG_TX_CTL1;		/*Virtual Transmit Port Control 1 Register*/
    a2b_UInt8 nREG_TX_CTL2;		/*Virtual Transmit Port Control 2 Register*/
    a2b_UInt8 nREG_TX_EVT_MSK;		/*Virtual Transmit Port Event Mask Register*/
    a2b_UInt8 nREG_TX_EVT_CLR;		/*Virtual Transmit Port Event Clear Register*/
    a2b_UInt8 nREG_TX_DTCP_USR0;		/*User Information 0 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_USR1;		/*User Information 1 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_USR2;		/*User Information 2 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_USR3;		/*User Information 3 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_USR4;		/*User Information 4 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_USR5;		/*User Information 5 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_USR6;		/*User Information 6 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_USR7;		/*User Information 7 Register (DTCP Only)*/
    a2b_UInt8 nREG_TX_DTCP_EMI;		/*Encryption Mode Indicator Register (DTCP Only)*/
}TXPORT_REGS;

/*! \struct RXPORT
 RX Port Configuration for Content Protection Ports
 */
typedef struct
{
    a2b_UInt8 nREG_RX_CFG;		/*Virtual Receive Port Configuration Register*/
    a2b_UInt8 nREG_RX_CTL1;		/*Virtual Receive Port Control 1 Register*/
    a2b_UInt8 nREG_RX_CTL2;		/*Virtual Receive Port Control 2 Register*/
    a2b_UInt8 nREG_RX_UNMUTE;		/*Virtual Receive Port Audio Unmute Time Threshold Register*/
    a2b_UInt8 nREG_RX_DSINFO0;		/*Virtual Receive Port Downstream Info 0 Register*/
    a2b_UInt8 nREG_RX_DSINFO1;		/*Virtual Receive Port Downstream Info 1 Register*/
    a2b_UInt8 nREG_RX_EVT_MSK;		/*Virtual Receive Port Event Mask Register*/
    a2b_UInt8 nREG_RX_EVT_CLR;		/*Virtual Receive Port Event Clear Register*/
    a2b_UInt8 nREG_RX_DTCP_USR0;		/*User Information 0 Register (DTCP Only)*/
    a2b_UInt8 nREG_RX_DTCP_USR1;		/*User Information 1 Register (DTCP Only)*/
    a2b_UInt8 nREG_RX_DTCP_USR2;		/*User Information 2 Register (DTCP Only)*/
    a2b_UInt8 nREG_RX_DTCP_USR3;		/*User Information 3 Register (DTCP Only)*/
    a2b_UInt8 nREG_RX_DTCP_USR4;		/*User Information 4 Register (DTCP Only)*/
    a2b_UInt8 nREG_RX_DTCP_USR5;		/*User Information 5 Register (DTCP Only)*/
    a2b_UInt8 nREG_RX_DTCP_USR6;		/*User Information 6 Register (DTCP Only)*/
    a2b_UInt8 nREG_RX_DTCP_USR7;		/*User Information 7 Register (DTCP Only)*/
}RXPORT_REGS;

/*! \struct ADI_A2B_244x_CP_CONFIG_STRUCT 
 */
typedef struct
{
	a2b_Byte nNodeId;
 	a2b_Byte nI2cAddr;
 	GENERAL_REGS	sReggeneral;
	SOC_DRX0_REGS	sRegsoc_drx0;
	SOC_DRX1_REGS	sRegsoc_drx1;
	SOC_DTX0_REGS	sRegsoc_dtx0;
	SOC_DTX1_REGS	sRegsoc_dtx1;
	A2B_DRX0_REGS	sRega2b_drx0;
	A2B_DRX1_REGS	sRega2b_drx1;
	A2B_DTX0_REGS	sRega2b_dtx0;
	A2B_DTX1_REGS	sRega2b_dtx1;
	TXPORT_REGS	sRegtxport[12];
	RXPORT_REGS	sRegrxport[12];
}ADI_A2B_244x_CP_CONFIG_STRUCT; 

typedef struct 
{ 
	 ADI_A2B_244x_CP_CONFIG_STRUCT *apCPConfigStruct[17];  // +1 for master
}ADI_A2B_244x_CP_NETCONFIG; 
#endif /*_ADI_A2B_244x_CP_H_*/

extern ADI_A2B_244x_CP_NETCONFIG s244xCPNetConfig;
