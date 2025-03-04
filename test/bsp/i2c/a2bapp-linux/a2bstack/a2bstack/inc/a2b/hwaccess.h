/*=============================================================================
 *
 * Project: a2bstack
 *
 * Copyright (c) 2023 - Analog Devices Inc. All Rights Reserved.
 * This software is subject to the terms and conditions of the license set 
 * forth in the project LICENSE file. Downloading, reproducing, distributing or 
 * otherwise using the software constitutes acceptance of the license. The 
 * software may not be used except as expressly authorized under the license.
 *
 *=============================================================================
 *
 * \file:   hwaccess.h
 * \author: Automotive Software Team, Bangalore
 * \brief:  This defines the stack wrapper functions to access I2C/SPI read/write interfaces.
 *
 *=============================================================================
 */

/*============================================================================*/
/**
 * \defgroup a2bstack_hwaccess       HW Access Module
 *  
 * The types and associated wrapper APIs providing public low-level I2C/SPI access.
 *  
 * The StackContext provides a mechanism for enforcing access control.
 * This means that only a master plugin can access the master, slave, or
 * peripheral I2C/SPI read/write routines. A slave plugin can only read/write to
 * peripherals attached to its node. An "application" context can also
 * read/write to peripherals attached via I2C/SPI to <em>any</em> slave node.
 *  
 * \{ */
/*============================================================================*/

#ifndef A2B_HW_ACCESS_H_
#define A2B_HW_ACCESS_H_

/*======================= I N C L U D E S =========================*/

#include "a2b/macros.h"
#include "a2b/ctypes.h"
#include "stackctx.h"
#include "a2b/spi.h"

/*----------------------------------------------------------------------------*/
/**
 * \defgroup a2bstack_hwaccess          Types/Defs
 *  
 * The various defines and data types used within the I2C/SPI modules.
 *
 * \{ */
/*----------------------------------------------------------------------------*/

/*======================= D E F I N E S ===========================*/

#define A2B_OFFSET_DAT_ADDRWDTH                   (2u) /* Offset for Address width from payload Byte 0 */
#define A2B_OFFSET_DAT_ADDR                       (4u) /* Offset for Address from payload Byte 0 */
#define A2B_MAX_LENINBYTES_REMOTESPITOI2C  (32u) /* Maximum Bytes that can be sent in one transaction for remote SPI to I2C */
#define A2B_MAX_LENINBYTES_REMOTESPITOSPI  (256u) /* Maximum Bytes that can be sent in one transaction for remote SPI to I2C - can be made 256 */
/*======================= D A T A T Y P E S =======================*/

A2B_BEGIN_DECLS

/* Forward declarations */
struct a2b_StackContext;


/** \} -- a2bstack_hwaccess */

/*===========================================================================
 *
 * Define the *synchronous* I2C/SPI API
 *
 * ========================================================================*/

/*----------------------------------------------------------------------------*/
/**
 * \defgroup a2bstack_hwaccess        Master/Slave Node I2C/SPI Synchronous Access
 *  
 * I2C/SPI API to synchronously read/write to/from the A2B master/slave node.
 *
 * \{ */
/*----------------------------------------------------------------------------*/

A2B_DSO_PUBLIC a2b_HResult A2B_CALL a2b_regRead(
                                            struct a2b_StackContext* ctx,
                                            a2b_UInt16 nRead,
                                            void* rBuf);

A2B_DSO_PUBLIC a2b_HResult A2B_CALL a2b_regWrite(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16                  nWrite,
	void*                       wBuf);

A2B_DSO_PUBLIC a2b_HResult a2b_regBroadcastWrite(
	struct a2b_StackContext*    ctx,
	a2b_UInt16                  nWrite,
	void*                       wBuf);


A2B_DSO_PUBLIC a2b_HResult A2B_CALL a2b_regWriteRead(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16                  nWrite,
	void*                       wBuf,
	a2b_UInt16                  nRead,
	void*                       rBuf);

A2B_DSO_PUBLIC a2b_HResult a2b_simpleRegWrite(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16					nRegAddr,
	a2b_UInt8                   nRegVal	);
A2B_DSO_PUBLIC a2b_HResult a2b_simpleRegRead(
	struct a2b_StackContext*    ctx,
	a2b_Int16                   node,
	a2b_UInt16					nRegAddr,
	a2b_UInt8*                  rVal );
A2B_DSO_PUBLIC a2b_HResult a2b_CheckBusy(struct a2b_StackContext* ctx, a2b_Bool *pBusy);

A2B_DSO_PUBLIC void a2b_delayForNewStruct(struct a2b_StackContext* ctx, a2b_Int16 dscNodeAddr);

/** \} -- a2bstack_hwaccess */


/*----------------------------------------------------------------------------*/
/**
 * \defgroup a2bstack_hwaccess        Peripheral Device Synchronous I2C/SPI Access
 *  
 * I2C/SPI API to synchronously read/write to/from the A2B node peripheral.
 *
 * \{ */
/*----------------------------------------------------------------------------*/

A2B_DSO_PUBLIC a2b_HResult A2B_CALL a2b_periphRead(
                                            struct a2b_StackContext* ctx,
                                            a2b_Int16 node,
                                            a2b_UInt16 hw_ifAddr,
                                            a2b_UInt16 nRead,
                                            void* rBuf);

A2B_DSO_PUBLIC a2b_HResult A2B_CALL a2b_periphWrite(
                                            struct a2b_StackContext* ctx,
                                            a2b_Int16 node,
                                            a2b_UInt16 hw_ifAddr,
											a2b_UInt8 datExportVer,
                                            a2b_UInt16 nWrite,
                                            void* wBuf);

A2B_DSO_PUBLIC a2b_HResult A2B_CALL a2b_periphWriteRead(
                                            struct a2b_StackContext* ctx,
                                            a2b_Int16 node,
                                            a2b_UInt16 hw_ifAddr,
                                            a2b_UInt16 nWrite,
                                            void* wBuf,
                                            a2b_UInt16 nRead,
                                            void* rBuf);
/** \} -- a2bstack_hwaccess */

a2b_HResult a2b_SpiCmdsAndPeriWriteRead(a2b_StackContext*  ctx, a2b_Int16 nNodeAddr, a2b_SpiCmd eSpiCmd, a2b_UInt16 regAddr, a2b_UInt32 nDataCount, a2b_UInt8* pDataBytes, a2b_UInt16 nRead, void* rBuf, a2b_UInt32 nMaxTransac);
a2b_HResult a2b_checkSpiErrorInt(a2b_StackContext* pCtx);
A2B_END_DECLS

/*======================= D A T A =================================*/

/** \} -- a2bstack_hwaccess */

#endif /* A2B_HW_ACCESS_H_ */
