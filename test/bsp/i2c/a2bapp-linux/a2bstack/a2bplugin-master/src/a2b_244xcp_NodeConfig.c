/*******************************************************************************
Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @file: a2b_244xcp_NodeConfig.c
* @brief: This file contains functions to configure CP registers of the AD244x
* @version: $Revision$
* @date: $Date$
* BCF Version - 1.0.0
*****************************************************************************/

#include "a2b/pluginapi.h"
#include "a2b/error.h"
#include "a2b/conf.h"
#include "a2b/defs.h"
#include "a2b/util.h"
#include "a2b/msg.h"
#include "a2b/msgrtr.h"
#include "a2b/trace.h"
#include "a2b/stack.h"
#include "a2b/regdefs.h"
#include "a2b/interrupt.h"
#include "a2b/stackctxmailbox.h"
#include "a2b/i2c.h"
#include "a2b/timer.h"
#include "a2b/seqchart.h"
#include "a2bplugin-master/plugin.h"
#include "discovery.h"
#include "periphcfg.h"
#include "override.h"
#include "a2b/audio.h"

#include "a2b/spi.h"
#include "stackctx.h"
#include "stack_priv.h"
#include "a2b/cpeng.h"
#ifdef _TESSY_INCLUDES_
#include "msg_priv.h"
#include "a2b/msgtypes.h"
#include "timer_priv.h"
#include "stackctx.h"
#endif	/* _TESSY_INCLUDES_ */

/*****************************************************************************/
#ifdef A2B_ENABLE_AD244xx_SUPPORT
/*======================= D E F I N E S ===========================*/
#define A2B_CP_SHADOW_REGS_CNT     (30u)

/*****************************************************************************/



/*****************************************************************************/
extern const a2b_UInt16 ga244xCPRegAddrMap[541];

static a2b_HResult ConfigRegVal(a2b_Plugin*  plugin, A2B_CP_I2C_PARAM *pI2cRegParam);
static a2b_HResult a2b_Configure_ShadowReg(a2b_Plugin* plugin, a2b_Int16 nodeAddr);


/*!****************************************************************************
 *
 *  \b              a2b_dscvryCPRegConfig
 *
 *  Configures CP engine.
 *
 *  \param          [in]    plugin
 *  \param          [in]    nodeAddr
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         None
 *
 ******************************************************************************/
A2B_DSO_PUBLIC void
a2b_dscvryCPRegConfig
(
		a2b_Plugin*         plugin,
		a2b_Int16           nodeAddr
)
{
	a2b_UInt16 nNodeId = (a2b_UInt16)(nodeAddr + 1);
	a2b_UInt32 nIdx = 0u;
	a2b_UInt8 nDefaultVal = 0u;
	a2b_UInt8 *pFuncGrpPtr;
	a2b_UInt8  wBuf[4u];
	a2b_UInt8  rBuf[4u];
	a2b_UInt16 i2cAddr = 0x60;
	a2b_UInt16 nRegAddIdx = 1u;
	a2b_HResult status = A2B_RESULT_SUCCESS;
	A2B_CP_I2C_PARAM oI2CRegStruct;

	if(plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId] != A2B_NULL)
	{
		i2cAddr = (a2b_UInt16)plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->nI2cAddr;
		/* GENERAL_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP General Registers" );
		pFuncGrpPtr = (a2b_UInt8 *)&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sReggeneral;
		for(nIdx=1u; nIdx<sizeof(GENERAL_REGS); nIdx++)
		{
			nDefaultVal = 0x0u;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* SOCRX0_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP SOCRX0 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRegsoc_drx0;
		for(nIdx=0u; nIdx<sizeof(SOC_DRX0_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );


		/* SOCRX1_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP SOCRX1 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRegsoc_drx1;
		for(nIdx=0u; nIdx<sizeof(SOC_DRX1_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* SOCTX0_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP SOCTX0 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRegsoc_dtx0;
		for(nIdx=0u; nIdx<sizeof(SOC_DTX0_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* SOCTX1_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP SOCTX1 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRegsoc_dtx1;
		for(nIdx=0u; nIdx<sizeof(SOC_DTX1_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* A2BRX0_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP A2BRX0 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRega2b_drx0;
		for(nIdx=0u; nIdx<sizeof(A2B_DRX0_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* A2BRX1_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP A2BRX1 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRega2b_drx1;
		for(nIdx=0u; nIdx<sizeof(A2B_DRX1_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* A2BTX0_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP A2BTX0 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRega2b_dtx0;
		for(nIdx=0u; nIdx<sizeof(A2B_DTX0_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* A2BTX1_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP A2BTX1 Registers" );
		pFuncGrpPtr = (a2b_UInt8* )&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRega2b_dtx1;
		for(nIdx=0u; nIdx<sizeof(A2B_DTX1_REGS); nIdx++)
		{
			nDefaultVal = 0xFFu;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* TXPORT_CFG_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP TXPORT CFG Registers" );
		pFuncGrpPtr = (a2b_UInt8 *)&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRegtxport[0];
		for(nIdx=0u; nIdx< (sizeof(TXPORT_REGS)*12); nIdx++)
		{
			nDefaultVal = 0x00u;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* RXPORT_CFG_REGS */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP RXPORT CFG Registers" );
		pFuncGrpPtr = (a2b_UInt8 *)&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sRegrxport[0];
		for(nIdx=0u; nIdx<(sizeof(RXPORT_REGS)*12); nIdx++)
		{
			nDefaultVal = 0x00u;
			if(pFuncGrpPtr[nIdx] != nDefaultVal)
			{
				oI2CRegStruct.nNodeAddr = nodeAddr;
				oI2CRegStruct.nI2cAddr = i2cAddr;
				oI2CRegStruct.nRegAddIdx = nRegAddIdx;
				oI2CRegStruct.nDataVal = pFuncGrpPtr[nIdx];
				status = ConfigRegVal(plugin, &oI2CRegStruct);
			}
			nRegAddIdx++;
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

		/* nREG_GLOBAL_CTL */
		A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "CP GLOBAL_CTL Register" );
		pFuncGrpPtr = (a2b_UInt8 *)&plugin->p244xCPNetConfigStruct->apCPConfigStruct[nNodeId]->sReggeneral;
		if(pFuncGrpPtr[0] != nDefaultVal)
		{
			oI2CRegStruct.nNodeAddr = nodeAddr;
			oI2CRegStruct.nI2cAddr = i2cAddr;
			oI2CRegStruct.nRegAddIdx = 0u;
			oI2CRegStruct.nDataVal = pFuncGrpPtr[0u];
			status = ConfigRegVal(plugin, &oI2CRegStruct);
		}
		else
		{
			oI2CRegStruct.nNodeAddr = nodeAddr;
			oI2CRegStruct.nI2cAddr = i2cAddr;
			oI2CRegStruct.nRegAddIdx = 0u;
			oI2CRegStruct.nDataVal = 0u;
			status = ConfigRegVal(plugin, &oI2CRegStruct);
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );
	}
}

static a2b_HResult ConfigRegVal(a2b_Plugin*  plugin, A2B_CP_I2C_PARAM *pI2cRegParam)
{

	a2b_HResult status;
	a2b_UInt8  wBuf[4u];
	a2b_UInt8 wSpiBuf[4u];
	a2b_UInt8 i = 0;
	wBuf[0] = (ga244xCPRegAddrMap[pI2cRegParam->nRegAddIdx] >> 8u);
	wBuf[1] = (ga244xCPRegAddrMap[pI2cRegParam->nRegAddIdx] & 0xFFu);
	wBuf[2] = pI2cRegParam->nDataVal;

	if (a2b_stackGetAccessInterface(plugin->ctx) == A2B_ACCESS_I2C)
	{
		status = a2b_i2cPeriphWrite( plugin->ctx, pI2cRegParam->nNodeAddr, pI2cRegParam->nI2cAddr, 3u, wBuf);
	}
	else
	{
		
#ifdef A2B_SS_STACK
		if(plugin->ctx->stk->ecb->palEcb.oAppEcbPal.cpSpiWrite != A2B_NULL)
			status = plugin->ctx->stk->ecb->palEcb.oAppEcbPal.cpSpiWrite(plugin->ctx->stk->spiHnd, pI2cRegParam->nI2cAddr, 3u, wBuf);
#else
		if(pI2cRegParam->nNodeAddr == A2B_NODEADDR_MASTER)
		{
			wSpiBuf[i++] = 0x00;
			(void)a2b_memcpy(&wSpiBuf[i++], wBuf, 3u);
			status = a2b_spiPeriphWrite(plugin->ctx, pI2cRegParam->nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_WRITE, pI2cRegParam->nI2cAddr, A2B_CP_SLAVE_SELECT, 3u + (i - 1), wSpiBuf);
		}
		else
		{
			(void)a2b_memcpy(&wSpiBuf[i++], wBuf, 3u);
			status = a2b_spiPeriphWrite(plugin->ctx, pI2cRegParam->nNodeAddr, A2B_CMD_SPI_REMOTE_I2C_WRITE, pI2cRegParam->nI2cAddr, A2B_SPI_SLV_SEL, 3u + (i - 1), wSpiBuf);
		}
#endif


	}


	return status;
}

#endif	/* #ifdef A2B_ENABLE_AD244xx_SUPPORT */
