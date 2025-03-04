/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @file: adi_a2b_datatypes.h
* @brief: This file contains typoedefs for basic data types used in A2B framework
* @date: $Date: 2015-07-21 09:58:13 +0530 (Tue, 21 Jul 2015) $
* Developed by: Automotive Software and Systems team, Bangalore, India
*****************************************************************************/

/*! \addtogroup Basic_Types Basic Types
* @{
*/

#ifndef _ADI_A2B_DATA_TYPES_H_
#define _ADI_A2B_DATA_TYPES_H_

/*============= I N C L U D E S =============*/
#include "a2b/ctypes.h"
/*============= D E F I N E S =============*/

/*! Internal memory code sections */
#define A2B_L1_CODE   __attribute__ ((section ("L1_code")))
/*! External memory code sections */
#define A2B_L3_CODE   __attribute__ ((section ("L3_code")))
/*! Internal memory data sections */
#define A2B_L1_DATA   __attribute__ ((section ("L1_data")))
/*! External memory data sections */
#define A2B_L3_DATA   __attribute__ ((section ("L3_data")))
/*! Graph data section  */
#define ADI_A2B_MEM_BCF_DATA   __attribute__ ((section ("L3_data")))
/*! Graph data section  */
#define ADI_A2B_MEM_BCF_CONST_DATA const
/*! Graph data section  */
#define ADI_A2B_MEM_GRAPH_DATA   __attribute__ ((section ("L3_data")))
/*! Peripheral data  section  */
#define ADI_A2B_MEM_PERI_CONFIG_DATA __attribute__ ((section ("sdram0_data")))
/*! Peripheral data  section  */
#define ADI_A2B_MEM_PERI_CONFIG_CONST_DATA const
/*! Peripheral configuration section  */
#define ADI_A2B_MEM_PERI_CONFIG_UNIT __attribute__ ((section ("sdram0_data")))

/*! Peripheral data  section  */
#define ADI_A2B_MEM_CONFIG_DATA __attribute__ ((section ("sdram0_data")))

/* Storage classes */

#define STATIC          static

#define EXTERN          extern

#define VOLATILE        volatile

#define REGISTER        register

#define NULL_PTR        ((void *)0)


/*============= D A T A T Y P E S=============*/

typedef unsigned short uint16;

typedef unsigned int uint32;

typedef unsigned long long uint64;

typedef unsigned char uint8;

typedef short int16;

typedef long int32;

typedef long long int64;

typedef signed char int8;

/* Boolean */
#define TRUE  (1)

#define FALSE (0)

/*======= P U B L I C P R O T O T Y P E S ========*/



#endif /* _ADI_A2B_FRAMEWORK_H_ */

/** 
 @}
*/

/*
*
* EOF: $URL$
*
*/

