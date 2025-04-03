/*******************************************************************************
 Copyright (c) 2023 - Analog Devices Inc. All Rights Reserved.
 This software is proprietary & confidential to Analog Devices, Inc.
 and its licensors.
 *******************************************************************************

 Name       : a2bapp.c

 Description: This file is responsible for handling all the a2b network functions
 like discovery , interrupt and  fault handling.
 Functions  : main()
 a2b_setup()
 a2b_fault_monitor()
 a2bapp_ProcessSuperBcf()
 a2bapp_onInterrupt()
 a2bapp_onDiscoveryComplete()
 a2bapp_onPowerFault()
 a2bapp_pluginsLoad()
 a2bapp_pluginsUnload()


 Prepared &
 Reviewed by: Automotive Software and Systems team,
 IPDC, Analog Devices,  Bangalore, India

 @version: $Revision: 3626 $
 @date: $Date: 2016-11-027 14:04:13 +0530 $

 ******************************************************************************/

/*! \addtogroup Application_Reference Application Reference
 *  @{
 */

/** @defgroup Application_Reference
 *
 * This module has reference implementation for A2B Stack usage
 *
 */

/*! \addtogroup Application_Reference
 *  @{
 */



/*============= I N C L U D E S =============*/


#include "adi_a2b_busconfig.h"
#include "a2bapp_defs.h"
#include "a2bapp.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "a2b/pluginapi.h"
#include "adi_a2b_externs.h"
#include <assert.h>
#include <stdio.h>
#ifdef A2B_ENABLE_AD244xx_SUPPORT
#include "adi_a2b_244x_config.h"
#endif
#ifdef ADI_UART_ENABLE
#include "platform.h"
#endif


/*============= D E F I N E S =============*/

#if defined(A2B_BCF_FROM_SOC_EEPROM) && !defined(A2B_FEATURE_EEPROM_OR_FILE_PROCESSING)
#error "A2B_FEATURE_EEPROM_OR_FILE_PROCESSING should also be enabled to use A2B_BCF_FROM_SOC_EEPROM feature"
#endif

#if defined(A2B_BCF_FROM_FILE_IO) && !defined(A2B_FEATURE_EEPROM_OR_FILE_PROCESSING)
#error "A2B_FEATURE_EEPROM_OR_FILE_PROCESSING should also be enabled to use A2B_BCF_FROM_FILE_IO feature"
#endif

#if defined(A2B_BCF_FROM_SOC_EEPROM) && defined(ADI_SIGMASTUDIO_BCF)
#error "A2B_BCF_FROM_SOC_EEPROM & ADI_SIGMASTUDIO_BCF features are mutually exclusive. Define only one to proceed further"
#endif

#if defined(A2B_BCF_FROM_FILE_IO) && defined(ADI_SIGMASTUDIO_BCF)
#error "A2B_BCF_FROM_FILE_IO & ADI_SIGMASTUDIO_BCF features are mutually exclusive. Define only one to proceed further"
#endif

#if defined(A2B_BCF_FROM_FILE_IO) && defined(A2B_BCF_FROM_SOC_EEPROM)
#error "A2B_BCF_FROM_FILE_IO & A2B_BCF_FROM_SOC_EEPROM features are mutually exclusive. Define only one to proceed further"
#endif

#ifndef ENABLE_INTRRUPT_PROCESS
#if (A2BAPP_POLL_PERIOD > A2B_REMOTEDEVCONFIG_SPITOSPI_TIMEOUT_IN_MS)
#error "A2BAPP_POLL_PERIOD is used for interrupt polling. A2BAPP_POLL_PERIOD should ALWAYS be less than A2B_REMOTEDEVCONFIG_SPITOSPI_TIMEOUT_IN_MS"
#endif
#endif	/* ENABLE_INTRRUPT_PROCESS */

#if defined(A2BAPP_ENABLE_RTMBOOT) && !defined(A2BAPP_NODE_LEVEL_DISC_CALLBACK)
#error "A2BAPP_NODE_LEVEL_DISC_CALLBACK should also be enabled to Boot the RTM"
#endif
/*! Array of pointers to store App context */
a2b_App_t *gpApp_Info[A2B_CONF_MAX_NUM_MASTER_NODES];

/*============= D A T A =============*/

#ifdef ENABLE_SUPERBCF
static a2b_UInt32 nDiscTryCnt = 0;
#endif

#ifdef A2B_ENABLE_AD244xx_SUPPORT
a2b_UInt8 nCPResetNodeCnt = 0u;
a2b_Bool nCPTxPortAKEDone = A2B_FALSE;
a2b_UInt8 nCPRxPortAKEDone = A2B_FALSE;
a2b_NetDiscovery* discReq;
a2b_Net_CP_APIs* pCPApiMsgReq;
#endif
#ifdef A2B_FEATURE_PARTIAL_DISC
static a2b_Int32 nNodeStartPartialDisc = A2B_NODEADDR_MASTER;
#endif
#ifdef A2B_APP_STATIC_MEMORY_FOR_STACK
static a2b_Byte gStackMemBuf[A2BAPP_STACK_NW_MEMORY];
static a2b_Byte gPluginMemBuf[A2BAPP_PLUGIN_NW_MEMORY];
#if defined(A2B_BCF_FROM_FILE_IO) || defined(A2B_BCF_FROM_SOC_EEPROM)
static a2b_Byte gE2PromMemBuf[A2BAPP_E2PROM_BLOCK_MEMORY];
ADI_A2B_NETWORK_CONFIG gTgtProperties;
#endif
#endif

/*============= C O D E =============*/

a2b_Int32  a2b_stop(a2b_App_t *pApp_Info);
a2b_UInt32 a2b_reset(a2b_App_t *pApp_Info);
static a2b_Int32 a2b_init(a2b_App_t *pApp_Info);
static a2b_Int32 a2b_load(a2b_App_t *pApp_Info);
static a2b_Int32 a2b_start(a2b_App_t *pApp_Info);
static a2b_Int32 a2b_discover(a2b_App_t *pApp_Info);
static a2b_Int32 a2b_sendDiscoveryMessage(a2b_App_t *pApp_Info);
static a2b_Int32 a2b_setupPwrDiag(a2b_App_t *pApp_Info);
static void a2b_appCtxReset(a2b_App_t *pApp_Info);

#ifdef ENABLE_SUPERBCF
static a2b_Int32 getCurrentSuperBCFIndex(a2b_App_t *pApp_Info, a2b_Int32 nRetryCount);
static a2b_Int32 a2b_ProcessSuperBcf(a2b_App_t *pApp_Info);
#endif

static void a2bapp_onInterrupt(struct a2b_Msg* msg, a2b_Handle userData);
static void a2bapp_onDiscoveryComplete(struct a2b_Msg* msg, a2b_Bool isCancelled);
static void a2bapp_onPowerFault(struct a2b_Msg *msg, a2b_Handle userData);
#ifdef A2BAPP_NODE_LEVEL_DISC_CALLBACK
static void a2bapp_onNodeDiscovery(struct a2b_Msg* msg, a2b_Handle userData);
#ifdef A2BAPP_ENABLE_RTMBOOT
static a2b_HResult a2bapp_RTMBoot(struct a2b_StackContext* ctx, a2b_Int16 nodeAddr);
#endif /* A2BAPP_ENABLE_RTMBOOT */
#endif /* A2BAPP_NODE_LEVEL_DISC_CALLBACK */
static void a2b_app_handle_becovf(struct a2b_Timer* timer, a2b_Handle userData);
static a2b_HResult a2bapp_HandlePwrFaultAnomaly(a2b_App_t *pApp_Info, a2b_Interrupt* interrupt);
static a2b_HResult a2bapp_VMTRMonitor(struct a2b_StackContext *ctx, a2b_Int16 nodeAddr, a2b_UInt8 *minThres, a2b_UInt8 *maxThres);
static a2b_HResult a2b_AppDetectBusDrop(a2b_App_t *pApp_Info);
static a2b_HResult a2b_AppReadReg(struct a2b_StackContext* ctx, a2b_Int16 nodeAddr, a2b_UInt8 reg, a2b_UInt8* value);
static a2b_HResult a2b_AppWriteReg(struct a2b_StackContext* ctx, a2b_Int16 nodeAddr, a2b_UInt8 reg, a2b_UInt8 value);
static void a2bapp_onI2CError(struct a2b_Msg *msg, a2b_Handle userData);

#ifdef A2B_ENABLE_AD244xx_SUPPORT
static void a2b_onCPInterrupt(struct a2b_Msg* msg, a2b_Handle userData);
static void a2b_CPTxPortInterrupt(a2b_UInt8 aCPPortintrType[], a2b_Int16 nNodeAddr);
static void a2b_CPRxPortInterrupt(a2b_UInt8 aCPPortintrType[], a2b_Int16 nNodeAddr);
#endif

#ifndef ADI_SIGMASTUDIO_BCF
static a2b_Bool a2b_loadBdd ( const a2b_Char* bddPath, bdd_Network* bdd);
#endif

#ifdef __ADSPBF71x__
static void a2bapp_initTdmSettings(A2B_ECB* ecb, const bdd_Network* bdd);
#endif  /* __ADSPBF7xx__ */
#ifdef ENABLE_INTERRUPT_PROCESS
static a2b_HResult a2b_processIntrpt(a2b_App_t *pApp_Info);
#endif

#ifdef ENABLE_INTERRUPT_PROCESS
static void a2b_IntrptCallbk(a2b_App_t *pApp_Info)
{
	pApp_Info->bIntrptLatch = 1u;
}
#endif

#ifdef ENABLE_AD243x_SUPPORT
static void a2b_SpiEventCallbk(a2b_App_t *pApp_Info);

/*!****************************************************************************
 *
 *  \b               a2b_SpiEventCallbk
 *
 *  GPIO callback function for SPI interrupts when SPI interrupt is
 *  configured on the GPIO Pin.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          None
 *
 ******************************************************************************/
static void a2b_SpiEventCallbk(a2b_App_t *pApp_Info)
{
	pApp_Info->bSpiIntrptLatch = 1u;
}
#endif /* ENABLE_AD243x_SUPPORT */
/********************************************************************************/
/*!
 @brief This function processes SPI interrupts on GPIO line

 @param [in] void				Stack Context


 @return	0 on Success
 1 on Failure
 */
/***********************************************************************************/
a2b_HResult a2b_processSpiIntrpt(a2b_App_t *pApp_Info)
{
	a2b_UInt8           intSrc;
	a2b_UInt8           intType;
	a2b_UInt8           regOffset;
	a2b_HResult         ret = A2B_RESULT_SUCCESS;

	pApp_Info->bSpiIntrptLatch = 0u;
	/* Read interrupt source of SPI interrupt */
	regOffset = A2B_REG_INTSRC;
	ret = a2b_regWriteRead(pApp_Info->ctx, A2B_NODEADDR_MASTER, 1u, &regOffset, 1u, &intSrc);

	regOffset = A2B_REG_INTTYPE;
	ret = a2b_regWriteRead(pApp_Info->ctx, A2B_NODEADDR_MASTER, 1u, &regOffset, 1u, &intType);

	a2b_spiPeriInterrupt(pApp_Info->ctx, A2B_NULL, intSrc, intType);

	return ret;
}
/*!****************************************************************************
 *
 *  \b               a2b_init
 *
 *  This function initializes the elements of the application context structure.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 a2b_init(a2b_App_t *pApp_Info)
{
	uint32_t nResult = 0;

	if (pApp_Info != NULL)
	{

		a2b_appCtxReset(pApp_Info);
		A2B_APP_DBG_LOG("Reset context done \n\r");

		/* Initialize the A2B Software Stack and Environment Control Block (ECB) */
		(void)a2b_systemInitialize(A2B_NULL, &pApp_Info->ecb);
		A2B_APP_DBG_LOG("System Init done \n\r");

		/* Initialize the Platform Abstraction Layer (PAL) */
		a2b_palInit(&pApp_Info->pal, &pApp_Info->ecb);
		A2B_APP_DBG_LOG("PAL Init done \n\r");

#ifdef ENABLE_INTERRUPT_PROCESS
		(void)adi_a2b_EnablePinInterrupt(4, (void*)&a2b_IntrptCallbk, (uint32_t)pApp_Info, 0u);
#endif
	}

	return nResult;
}

/*!****************************************************************************
 *
 *  \b               a2b_appCtxReset
 *
 *  This function initializes the elements of the application context structure.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static void a2b_appCtxReset(a2b_App_t *pApp_Info)
{
	uint8_t nIndex;
	/* Input flags */
	a2b_Bool	bDebug;
#ifdef A2B_FEATURE_SEQ_CHART
	a2b_Char* seqFile;
#endif
	a2b_UInt32 	nDiscTryCnt;
	a2b_Bool	bFrstTimeDisc;
#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)
	a2b_UInt8   nNumChains;
#endif

	/* take back up */
	nIndex = pApp_Info->ecb.palEcb.nChainIndex;
#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)
	nNumChains = pApp_Info->nNumChains;
#endif
	bDebug = pApp_Info->bDebug;
#ifdef A2B_FEATURE_SEQ_CHART
	seqFile = pApp_Info->seqFile;
#endif	/* A2B_FEATURE_SEQ_CHART */

	/* Save the value of discovery try count & bFrstTimeDisc */
	nDiscTryCnt = pApp_Info->nDiscTryCnt;
	bFrstTimeDisc = pApp_Info->bFrstTimeDisc;

	/* Clear out the Application's context container */
	(void)memset(pApp_Info, 0, sizeof(a2b_App_t));

	/*Restore the value of discovery try count & bFrstTimeDisc */
	pApp_Info->nDiscTryCnt = nDiscTryCnt;
	pApp_Info->bFrstTimeDisc = bFrstTimeDisc;

	/* Restore inputs */
	pApp_Info->ecb.palEcb.nChainIndex = nIndex;
#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)
	pApp_Info->nNumChains = nNumChains;
#endif
	pApp_Info->bDebug 	=  bDebug;
#ifdef A2B_FEATURE_SEQ_CHART
	pApp_Info->seqFile 	=  seqFile;
#endif	/* A2B_FEATURE_SEQ_CHART */

	/* Store the Pointers */
	gpApp_Info[nIndex] = pApp_Info;

}
#ifdef ENABLE_SUPERBCF
/*!****************************************************************************
 *
 *  \b               getCurrentSuperBCFIndex
 *
 *  This function returns the current BCF index in a super BCF file to be used
 *  during discovery based on the current retry count.
 *
 *  \param           [in]    nRetryCount   discovery iteration count
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 getCurrentSuperBCFIndex(a2b_App_t *pApp_Info, a2b_Int32 nRetryCount)
{
	a2b_Int32 nCurrBCFIndex = 0;

	if( (pApp_Info->nDefaultBCDIndex == 0xFF) || (pApp_Info->nDefaultBCDIndex == pApp_Info->nNumBCD))
	{
		pApp_Info->nDefaultBCDIndex = pApp_Info->nNumBCD - 1u;
	}
	if (nRetryCount >= pApp_Info->nNumBCD)
	{
		nCurrBCFIndex = pApp_Info->nDefaultBCDIndex;
	}
	else
	{
		nCurrBCFIndex = ((nRetryCount + pApp_Info->nDefaultBCDIndex + 1) % pApp_Info->nNumBCD);
	}

	return nCurrBCFIndex;

}
#endif
/*!****************************************************************************
 *
 *  \b               a2b_load
 *
 *  This function loads network configuration into the Stack context
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 a2b_load(a2b_App_t *pApp_Info)
{
	uint32_t nResult = 0, pos;
	int i = 0;
	uint16_t nL5Ptr = 0;
#ifdef ENABLE_SUPERBCF
	a2b_Int32 nSuperBcfIndex;
#endif
	a2b_HResult stat;
#ifdef ADI_A2B_BCF_COMPRESSED
	a2b_Bool bRet;
#endif
	/*
	 * Decode the network configuration and store it into the bdd element of
	 * the Application context.
	 */
#ifdef 	ADI_SIGMASTUDIO_BCF

	A2B_APP_LOG("\n\rUsing SigmaStudio BCF File\n\r");

#ifdef ADI_A2B_BCF_COMPRESSED

#ifndef ENABLE_SUPERBCF
	/* Parse compressed BDD */
	bRet = adi_a2b_ComprBcfParse_bdd(&sCmprBusDescription, &pApp_Info->bdd, pApp_Info->ecb.palEcb.nChainIndex);
	if(!bRet)
	{
		nResult = 1;
		A2B_APP_DBG_LOG("Bdd Decode Error \n\r");
		return nResult;
	}
	/* Parse compressed BCF to store peripheral info */
	adi_a2b_ParsePeriCfgFrComBCF(&sCmprBusDescription, pApp_Info->aPeriNetworkTable, pApp_Info->ecb.palEcb.nChainIndex);
	pApp_Info->pTargetProperties = &sCmprBusDescription.sTargetProperties;

#else
	/* using BCF adi_a2b_busconfig.c */
	pApp_Info->nNumBCD = sCmprSuperBCD.nNumBCD;
	pApp_Info->nDefaultBCDIndex = sCmprSuperBCD.nDefaultBCDIndex;
	nSuperBcfIndex = getCurrentSuperBCFIndex(pApp_Info,nDiscTryCnt);
	/* Parse compressed BDD */
	bRet = adi_a2b_ComprBcfParse_bdd(sCmprSuperBCD.apBusDescription[nSuperBcfIndex], &pApp_Info->bdd, pApp_Info->ecb.palEcb.nChainIndex);
	if(!bRet)
	{
		nResult = 1;
		A2B_APP_DBG_LOG("Bdd Decode Error \n\r");
		return nResult;
	}
	/* Parse compressed BCF to store peripheral info */
	adi_a2b_ParsePeriCfgFrComBCF(sCmprSuperBCD.apBusDescription[nSuperBcfIndex], &pApp_Info->aPeriNetworkTable[0U], pApp_Info->ecb.palEcb.nChainIndex);

	pApp_Info->pTargetProperties = &sCmprSuperBCD.apBusDescription[nSuperBcfIndex]->sTargetProperties;
#endif

#else /* ADI_A2B_BCF_COMPRESSED */

#ifndef ENABLE_SUPERBCF
	/* using BCF adi_a2b_busconfig.c */
	pApp_Info->pBusDescription = &sBusDescription;
	pApp_Info->pTargetProperties = &sBusDescription.sTargetProperties;
#else
	pApp_Info->nNumBCD = sSuperBCD.nNumBCD;
	pApp_Info->nDefaultBCDIndex = sSuperBCD.nDefaultBCDIndex;
	nSuperBcfIndex = getCurrentSuperBCFIndex(pApp_Info,nDiscTryCnt);
	/* using BCF adi_a2b_busconfig.c */
	pApp_Info->pBusDescription = sSuperBCD.apBusDescription[nSuperBcfIndex];
	pApp_Info->pTargetProperties = &pApp_Info->pBusDescription->sTargetProperties;

#endif

	/* Parse BCf and store in BDD */
	a2b_bcfParse_bdd(pApp_Info->pBusDescription, &pApp_Info->bdd, pApp_Info->ecb.palEcb.nChainIndex);

	/* Parse BCF to store peripheral info */
	adi_a2b_ParsePeriCfgTable(pApp_Info->pBusDescription, &pApp_Info->aPeriNetworkTable[0], pApp_Info->ecb.palEcb.nChainIndex);

	A2B_APP_DBG_LOG("BCF parse done \n\r");

#endif

	/* assign the peripheral configuration table (including audio host) */
	pApp_Info->ecb.palEcb.pAudioHostDeviceConfig = &pApp_Info->aPeriNetworkTable[0u];


#elif defined (A2B_BCF_FROM_SOC_EEPROM)

	/* Requesting for a I2C handle so as to access SOC EEPROM device.
	 * Once the data from EEPROM is read we will close this instance of I2C handle as it will be re-opened during a2b_stackAlloc
	 * */
	a2b_pal_I2cInit(&pApp_Info->ecb);

	a2b_pal_I2cOpenFunc(pApp_Info->ecb.baseEcb.i2cAddrFmt, pApp_Info->ecb.baseEcb.i2cBusSpeed, &pApp_Info->ecb);
#ifdef A2B_APP_STATIC_MEMORY_FOR_STACK
	pApp_Info->panDatFileBuff = (a2b_UInt8 *)&gE2PromMemBuf;
	pApp_Info->pTargetProperties = &gTgtProperties;
#else
	/* Assuming the max size of I2C burst read */
	pApp_Info->panDatFileBuff = malloc( A2BAPP_E2PROM_BLOCK_MEMORY );
	pApp_Info->pTargetProperties = malloc(sizeof(ADI_A2B_NETWORK_CONFIG));
#endif
	/* Create A2B Target properties */

	memset(pApp_Info->pTargetProperties, 0, sizeof(ADI_A2B_NETWORK_CONFIG));

	/* Populate BDD from local EEPROM */
	(void)a2b_get_bddFrmE2promOrFileIO(&pApp_Info->ecb, &pApp_Info->bdd, pApp_Info->panDatFileBuff, pApp_Info->anEeepromPeriCfgInfo, pApp_Info->pTargetProperties);
	/* Find the pointer where audio host config info is stored */
	pApp_Info->ecb.palEcb.pEepromAudioHostConfig = &pApp_Info->anEeepromPeriCfgInfo[0];

	/* Closing the I2C handle */
	a2b_pal_I2cCloseFunc(pApp_Info->ecb.palEcb.i2chnd);

#elif  defined (A2B_BCF_FROM_FILE_IO)
	/* BCF file Open */
	a2b_pal_FileOpen(&pApp_Info->ecb, A2B_CONF_BINARY_BCF_FILE_URL);
#ifdef A2B_APP_STATIC_MEMORY_FOR_STACK
	pApp_Info->panDatFileBuff = gE2PromMemBuf;
	pApp_Info->pTargetProperties = &gTgtProperties;
#else
	/* Assuming the max size of file read */
	pApp_Info->panDatFileBuff = malloc( A2BAPP_E2PROM_BLOCK_MEMORY );
	pApp_Info->pTargetProperties = malloc(sizeof(ADI_A2B_NETWORK_CONFIG));
#endif
	/* Create A2B Target properties */
	memset(pApp_Info->pTargetProperties, 0, sizeof(ADI_A2B_NETWORK_CONFIG));

	/* Populate BDD through binary file read */
	(void)a2b_get_bddFrmE2promOrFileIO(&pApp_Info->ecb, &pApp_Info->bdd, pApp_Info->panDatFileBuff, pApp_Info->anEeepromPeriCfgInfo, pApp_Info->pTargetProperties);
	/* Find the pointer where audio host config info is stored */
	pApp_Info->ecb.palEcb.pEepromAudioHostConfig = &pApp_Info->anEeepromPeriCfgInfo[0];
	pos = pApp_Info->pTargetProperties->nL5Pos;
	pApp_Info->pCustomIdInfo = &pApp_Info->anEeepromPeriCfgInfo[pos];

	if(pApp_Info->pTargetProperties->nSubNodes <= A2B_CONF_MAX_NUM_SLAVE_NODES)
	{
		for(i = 0; i < pApp_Info->pTargetProperties->nSubNodes + 1; i++)
		{
			if(i == 0)
			{
				A2B_GET_UINT16_BE(pApp_Info->nCustomIdPtr[i], pApp_Info->anEeepromPeriCfgInfo, pos);
				pApp_Info->nCustomIdPtr[i]++;
			}
			else
			{
				A2B_GET_UINT16_BE(pApp_Info->nCustomIdPtr[i], pApp_Info->anEeepromPeriCfgInfo, pos);
			}
			pos += 2;
		}
	}
	/* For validation purpose, this function will extract info from the pointer pApp_Info->pCustomIdInfo */
	(void)adi_a2b_extractdata(&pApp_Info->ecb, pApp_Info->pCustomIdInfo, pApp_Info->pTargetProperties, pApp_Info->nLenCustomId);

#else
	/* From Third party tools */

#endif

#ifdef __ADSPBF71x__
	/* Clock for ADSP-BF7xx SPORT's is generated by SigmaDSP part. SPORT's should be enabled before starting discovery, so that A2B chip is clocked.
	 * Call to below function copies the TDM settings from BDD to palecb so that SPORT's configuration is done during PAL audio init function.
	 * */
	a2bapp_initTdmSettings(&pApp_Info->ecb, &pApp_Info->bdd);
#endif  /* __ADSPBF7xx__ */
#ifdef A2B_FEATURE_TRACE
	pApp_Info->ecb.baseEcb.traceLvl = A2B_CONF_DEFAULT_TRACE_LVL;
	pApp_Info->ecb.baseEcb.traceUrl = A2B_CONF_DEFAULT_TRACE_CHAN_URL;
#endif

	/*
	 * Initialize vendor, product, and version information in the ECB.
	 */
	a2b_bddPalInit(&pApp_Info->ecb, &pApp_Info->bdd);
	A2B_APP_DBG_LOG("BDD PAL Init done \n\r");

	/*
	 * Allocate a heap for the Stack. This step may be optional if the
	 * stack was configured to utilize an external memory management system
	 * by not defining A2B_FEATURE_MEMORY_MANAGER in features.h
	 */
#ifdef A2B_APP_STATIC_MEMORY_FOR_STACK
	if(pApp_Info->ecb.baseEcb.heapSize > A2BAPP_STACK_MEMORY_PER_CHAIN)
	{
		nResult = 1;
		A2B_APP_DBG_LOG("Insufficient memory \n\r");
		return nResult;
	}
	else
	{
		pApp_Info->ecb.baseEcb.heap = &gStackMemBuf[pApp_Info->ecb.palEcb.nChainIndex * A2BAPP_STACK_MEMORY_PER_CHAIN];
	}
#else

	pApp_Info->ecb.baseEcb.heap = malloc(pApp_Info->ecb.baseEcb.heapSize);
	A2B_APP_DBG_LOG("Allocate Heap done \n\r");
#endif


	/*
	 * Perform the final allocation of the stack based off of the
	 * specifics of this network configuration.
	 */
	pApp_Info->ctx = a2b_stackAlloc(&pApp_Info->pal, &pApp_Info->ecb, &stat);

#ifdef ENABLE_AD243x_SUPPORT
	if(pApp_Info->bdd.nodes[0].has_spiRegs)
	{
		a2b_UInt8 nFlagVal = ((a2b_UInt8)(pApp_Info->bdd.nodes[0].spiRegs.spipicfg) & A2B_BITM_SPIPINCFG_SPIGPIOEN) ? 0x01U: 0x00U;
		a2b_setSpiIntOnGpioStat(pApp_Info->ctx, nFlagVal);
		if(nFlagVal == 0x1U)
		{
#ifdef ENABLE_INTERRUPT_PROCESS
			(void)adi_a2b_EnablePinInterrupt(0u, (void*)&a2b_SpiEventCallbk, (uint32_t)pApp_Info, 1u);
#endif
		}
	}
#endif /* ENABLE_AD243x_SUPPORT */


#ifdef A2B_ENABLE_AD244xx_SUPPORT
	pApp_Info->p244xCPNetConfig = &s244xCPNetConfig;
	for(a2b_Int32 nIdx=0; nIdx< (A2B_CONF_MAX_NUM_SLAVE_NODES + 1u); nIdx++)
	{
		if(pApp_Info->p244xCPNetConfig->apCPConfigStruct[nIdx] != A2B_NULL)
		{
			pApp_Info->ctx->ccb.app.aCPI2CAddr[nIdx] = pApp_Info->p244xCPNetConfig->apCPConfigStruct[nIdx]->nI2cAddr;
			/* Flag to disable/enable interrupt processing for the indexed node,
			 * If the local slave processor handles CP events, this flag needs to be 1
			 */
			pApp_Info->ctx->ccb.app.bDisIntService[nIdx] = 0u;
			pApp_Info->nValidCPNodes++;
		}
	}
#endif

	/* No context, means failure */
	if (pApp_Info->ctx == A2B_NULL)
	{
		nResult = 1;
		A2B_APP_DBG_LOG("Failed to allocate Stack. Error code: %d \n\r",stat);
	}
	else
	{
		A2B_APP_DBG_LOG("Allocate Stack done \n\r");
#ifndef ADI_A2B_BCF_COMPRESSED
		/* Set the current interface, SPI or I2C */
		pApp_Info->ecb.palEcb.nDeviceInterface = (a2b_UInt32)pApp_Info->bdd.policy.eA2bDeviceInterface;
#else	/* ADI_A2B_BCF_COMPRESSED */
#ifndef ENABLE_SUPERBCF
		/* Set the current interface, SPI or I2C */
		pApp_Info->ecb.palEcb.nDeviceInterface = (a2b_UInt32)pApp_Info->bdd.policy.eA2bDeviceInterface;
#else
		/* 19.9.0 : For SuperBCF we still continue to get device interface from
		 * the bcf bus description since we mention that we will consider
		 * the first BCF files' network config for all BCFs in our documentation.
		 * Using from BDD will make it take from current bdd, and we can't ensure that it is same as the first Bdd. */
		pApp_Info->ecb.palEcb.nDeviceInterface = (a2b_UInt32)sCmprSuperBCD.apBusDescription[nSuperBcfIndex]->apNetworkconfig[0]->eA2bDeviceInterface;
#endif
#endif	/* ADI_A2B_BCF_COMPRESSED */
		a2b_stackSetAccessInterface(pApp_Info->ctx, (ADI_A2B_ACCESS_INTERFACE)pApp_Info->ecb.palEcb.nDeviceInterface);
		nResult = adi_a2b_spiPeriCreate(pApp_Info->ctx);

#if  defined (A2B_BCF_FROM_FILE_IO)
		a2b_stackSetFileIO(pApp_Info->ctx, pApp_Info->ecb.palEcb.fp);
#endif

	}

	return nResult;
}
/*!****************************************************************************
 *
 *  \b               a2b_start
 *
 *  This function involves instructing the Stack to begin polling for interrupts,
 *  enabling sequence charts and debugging output, and hooking in application
 *  level call-backs.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 a2b_start(a2b_App_t *pApp_Info)
{
	uint32_t nResult = 0;
#ifdef ADI_ENABLE_AD244xx_SUPPROT
	ADI_A2B_244x_CP_NETCONFIG s244xCPNetConfig;
#endif
	/* Enable sequence charts */
#ifdef A2B_FEATURE_SEQ_CHART
	if ( A2B_NULL == pApp_Info->seqFile )
	{
		pApp_Info->seqFile = A2B_CONF_DEFAULT_SEQCHART_CHAN_URL;
	}
	if ( A2B_NULL != pApp_Info->seqFile )
	{
		/* Enable sequence charts (if configured) */
		a2b_seqChartStart(pApp_Info->ctx, pApp_Info->seqFile,
				A2B_SEQ_CHART_LEVEL_I2C | A2B_SEQ_CHART_LEVEL_MSGS,
				A2B_SEQ_CHART_OPT_ALL, "Sequence Chart");
	}
#endif

	/* Register for notifications on interrupts */
	pApp_Info->notifyInterrupt = a2b_msgRtrRegisterNotify(pApp_Info->ctx,
			A2B_MSGNOTIFY_INTERRUPT, a2bapp_onInterrupt, pApp_Info, A2B_NULL);

	/* Register for notifications on power faults */
	pApp_Info->notifyPowerFault = a2b_msgRtrRegisterNotify(pApp_Info->ctx,
			A2B_MSGNOTIFY_POWER_FAULT, a2bapp_onPowerFault, pApp_Info, A2B_NULL);

	pApp_Info->notifyI2CError = a2b_msgRtrRegisterNotify(pApp_Info->ctx,
			A2B_MSGREQ_PERIPH_I2C_ERROR_APP, a2bapp_onI2CError, pApp_Info, A2B_NULL);

#ifdef A2B_ENABLE_AD244xx_SUPPORT
	/*Register notifications for CP interrupts*/
	pApp_Info->notifyCPInterrupt = a2b_msgRtrRegisterNotify(pApp_Info->ctx,
			A2B_MSGNOTIFY_CPINTERRUPT, a2b_onCPInterrupt, pApp_Info, A2B_NULL);

#endif

#ifdef A2BAPP_NODE_LEVEL_DISC_CALLBACK
	/* Register for notifications on node discovery */
	pApp_Info->notifyNodeDiscvry = a2b_msgRtrRegisterNotify(pApp_Info->ctx,
	A2B_MSGNOTIFY_NODE_DISCOVERY, a2bapp_onNodeDiscovery, pApp_Info, A2B_NULL);
#endif
	/* Power diagnostic is set by default. Depending on SigmaStudio
	 * settings we should only allow re-discovery or not from a2bapp_onPowerFault() callback
	 */
	(void)a2b_setupPwrDiag(pApp_Info);

	return nResult;
}
/*!****************************************************************************
 *
 *  \b               a2b_sendDiscoveryMessage
 *
 *  This function sends a message to master plugin instructing it to initiate
 *  A2B network discovery.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 a2b_sendDiscoveryMessage(a2b_App_t *pApp_Info)
{
	a2b_NetDiscovery *discReq;
#ifdef A2B_FEATURE_PARTIAL_DISC
	a2b_NetPostDiscovery* discPartialReq;
#endif
	struct a2b_Msg *msg;
	a2b_HResult result = 0;
#ifndef ENABLE_INTERRUPT_PROCESS
	a2b_UInt32 pollTime;
#endif

#ifdef A2B_FEATURE_PARTIAL_DISC
	if (pApp_Info->bdd.policy.bEnablePartialDisc && (nNodeStartPartialDisc != A2B_NODEADDR_MASTER))
	{
		msg = a2b_msgAlloc(pApp_Info->ctx, A2B_MSG_REQUEST, A2B_MSGREQ_NET_POST_DISCOVERY);
		/* Attach the Boolean flag to the message */
		discPartialReq = (a2b_NetPostDiscovery*)a2b_msgGetPayload(msg);
		discPartialReq->req.bIsPartialDiscEnabled = A2B_TRUE;
		discPartialReq->req.bBusDropDetected = A2B_TRUE;
		discPartialReq->req.nBusDropNodeId = nNodeStartPartialDisc;
		/* Add this context to the message */
		a2b_msgSetUserData(msg, (a2b_Handle)pApp_Info, A2B_NULL);
		(void)a2b_msgRtrSendRequest(msg, A2B_NODEADDR_MASTER, a2bapp_onDiscoveryComplete);
	}
	else
#endif
	{
		/* Create a network discovery request message */
		msg = a2b_msgAlloc(pApp_Info->ctx, A2B_MSG_REQUEST, A2B_MSGREQ_NET_DISCOVERY);

		/* Attach the BDD information to the message */
		discReq = (a2b_NetDiscovery*)a2b_msgGetPayload(msg);
		discReq->req.bdd = &pApp_Info->bdd;

#ifdef A2B_ENABLE_AD244xx_SUPPORT
		discReq->req.pCPRegConfig = (a2b_Byte *)gApp_Info.p244xCPNetConfig;
#endif

#ifdef ADI_SIGMASTUDIO_BCF

		/* Attach additional peripheral initialization data as generated.*/
		discReq->req.periphPkg = (const a2b_Byte *)&pApp_Info->aPeriNetworkTable[0u];
		discReq->req.pkgLen = sizeof(ADI_A2B_NETWORK_PERICONFIG);

		/** Flag which indicates the current is first time discovery or re-discovering the network
		 *   bFrstTimeDisc value
		 *				true: Current execution is first time discovery
		 *				false: Current execution is re-discovery
		 */
		discReq->req.bFrstTimeDisc = pApp_Info->bFrstTimeDisc;

#elif defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)

		/* Attach additional peripheral initialization data as generated.*/
		discReq->req.periphPkg = (const a2b_Byte *)&pApp_Info->anEeepromPeriCfgInfo[2];
		discReq->req.pkgLen = 2 * A2B_CONF_MAX_NUM_SLAVE_NODES;
		discReq->req.bFrstTimeDisc = pApp_Info->bFrstTimeDisc;
#else
		/*
		 * Attach additional peripheral initialization data as generated.
		 * by the Network Configuration Tool. In this case no peripheral
		 * initialization is added.
		 */
		discReq->req.periphPkg = A2B_NULL;
		discReq->req.pkgLen = 0;
		discReq->req.bFrstTimeDisc = pApp_Info->bFrstTimeDisc;
#endif

		/* Add this context to the message */
		a2b_msgSetUserData(msg, (a2b_Handle)pApp_Info, A2B_NULL);
		result = a2b_msgRtrSendRequest(msg, A2B_NODEADDR_MASTER, a2bapp_onDiscoveryComplete);
	}
#ifndef ENABLE_INTERRUPT_PROCESS
	/* Interrupt polling interval in milliseconds */
	pollTime = A2BAPP_POLL_PERIOD;

	/* Instruct the Stack to begin interrupt polling every A2BAPP_POLL_PERIOD */
	(void)a2b_intrStartIrqPoll(pApp_Info->ctx, pollTime);
#endif
	/* The message router adds its own reference to the submitted message. */
	(void)a2b_msgUnref(msg);

	return result;
}


/*!****************************************************************************
 *
 *  \b               a2b_discover
 *
 *  This function sends a discovery initating message and ticks till completion.
 *  When using Super BCF this will iterate through the BCFs to complete discovery.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 a2b_discover(a2b_App_t *pApp_Info)
{
	a2b_HResult result = 0;
	result = a2b_sendDiscoveryMessage(pApp_Info);


	if (result != 0)
	{

		A2B_APP_LOG("Error while sending discovery message \n\r");

		return 1;
	}
	A2B_APP_DBG_LOG("Triggering discovery... \r\n");

	/* Clear any outstanding interrupts */
	pApp_Info->bIntrptLatch = 1u;
	/*
	 * Be sure to transition to the Poll state or call a2b_stackTick() in
	 * a loop here until a2b_onDiscoveryComplete() is called otherwise
	 * discovery will not progress!
	 */
	while (1)
	{
		/* tick keeps all process rolling.. so keep ticking */
		a2b_stackTick(pApp_Info->ctx);

#ifdef ENABLE_INTERRUPT_PROCESS
		(void)a2b_processIntrpt(pApp_Info);
#endif
		if (pApp_Info->discoveryDone)
		{
			if (pApp_Info->discoverySuccessful == true)
			{
#ifdef ENABLE_SUPERBCF
				/* Clear the discovery try count after successful discovery */
				nDiscTryCnt = 0U;
#endif
				/* A2B Network discovery and initialization is complete.*/
				break;
			}
			else
			{
				/* A2B Network discovery failed */
				pApp_Info->discoveryDone = false;

#ifdef ENABLE_SUPERBCF
				/* here, only upon custom node authentication failure, other configurations are applied.
				 * other failures like line fault can also be considered
				 *
				 */
				if (pApp_Info->bCustomAuthFailed == true)
				{
					nDiscTryCnt++;

					if (nDiscTryCnt == (pApp_Info->nNumBCD))
					{
						break;
					}

					pApp_Info->bCustomAuthFailed = false;

					/* Discovery as failed. Network order is different.. try a different network combination from Super BCF file */
					result = a2b_ProcessSuperBcf(pApp_Info);
					if (result != 0)
					{
						return (0);
					}
				}
				else
				{
					break;
				}
#else

				break;
#endif

			}
		}
	}

#ifdef A2B_FEATURE_SEQ_CHART
	if ( A2B_NULL != pApp_Info->seqFile )
	{
		/* Do clean up after discovery is done */
		a2b_seqChartStop(pApp_Info->ctx);
	}
#endif

	return result;
}

#ifdef ENABLE_SUPERBCF
/*!****************************************************************************
 *
 *  \b               a2b_ProcessSuperBcf
 *
 *  This function loads a configuration from Super BCF and intiates a fresh discovery.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 a2b_ProcessSuperBcf(a2b_App_t *pApp_Info)
{
	uint32_t nResult = 0;

	/* stop a2b stack*/
	nResult = a2b_stop(pApp_Info);
	if (nResult != 0)
	{
		assert(nResult == 0);
	}

	/* load new configuration file */
	nResult = a2b_load(pApp_Info);
	if (nResult != 0)
	{
		assert(nResult == 0);
	}

	/* start stack again */
	nResult = a2b_start(pApp_Info);
	if (nResult != 0)
	{
		assert(nResult == 0);
	}

	nResult = a2b_sendDiscoveryMessage(pApp_Info);
	if (nResult != 0)
	{
		assert(nResult == 0);
	}

	return nResult;
}
#endif
/*!****************************************************************************
 *
 *  \b               a2b_setupPwrDiag
 *
 *  This function enables/disables line fault monitoring of the stack based on
 *  settings in the bcf.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
static a2b_Int32 a2b_setupPwrDiag(a2b_App_t *pApp_Info)
{
	uint32_t nResult = 0;
	struct a2b_Msg *msg;
	a2b_Bool *pIsLineDiagDisabled;

	/* Create a Disable Line Diagnostics request message*/
	msg = a2b_msgAlloc(pApp_Info->ctx, A2B_MSG_REQUEST, A2B_MSGREQ_NET_DISBALE_LINEDIAG);

	/* Attach the Boolean flag to the message , By default line diagnostics enabled */
	pIsLineDiagDisabled = (a2b_Bool*)a2b_msgGetPayload(msg);
	*pIsLineDiagDisabled = A2B_FALSE; /* Set the flag to True, in case app wants to disable */

	(void)a2b_msgRtrSendRequest(msg, A2B_NODEADDR_MASTER, A2B_NULL);
	(void)a2b_msgUnref(msg);
	a2b_ActiveDelay(pApp_Info->ctx, 5u);

	return nResult;
}

/*!****************************************************************************
 *
 *  \b               a2b_stop
 *
 *  This function stops stack, un-registering call-backs, turning off interrupt polling,
 *  disabling sequence charts, and freeing resources associated with the application
 *  context.
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
a2b_Int32 a2b_stop(a2b_App_t *pApp_Info)
{
	/* Unregister notifications */
	a2b_msgRtrUnregisterNotify(pApp_Info->notifyInterrupt);
#ifdef A2B_ENABLE_AD244xx_SUPPORT
	a2b_msgRtrUnregisterNotify(pApp_Info->notifyCPInterrupt);
#endif
	a2b_msgRtrUnregisterNotify(pApp_Info->notifyPowerFault);
	a2b_msgRtrUnregisterNotify(pApp_Info->notifyNodeDiscvry);

	A2B_APP_DBG_LOG("Unregister notification done \r\n");

	/* Stop interrupt polling */
	(void)a2b_intrStopIrqPoll(pApp_Info->ctx);
	A2B_APP_DBG_LOG("Stop IRQ done... \r\n");

#ifdef A2B_FEATURE_SEQ_CHART
	if ( A2B_NULL != pApp_Info->seqFile )
	{
		/* Stop the sequence chart */
		a2b_seqChartStop(pApp_Info->ctx);
	}
#endif

	/* Free the stack and heap */
	a2b_stackFree(pApp_Info->ctx);
	A2B_APP_DBG_LOG("Free Stack context done \r\n");

#ifdef A2B_APP_STATIC_MEMORY_FOR_STACK
	(void)memset(gStackMemBuf, 0, A2BAPP_STACK_NW_MEMORY);
#else
	if (pApp_Info->ecb.baseEcb.heap != NULL)
	{
		(void)free(pApp_Info->ecb.baseEcb.heap);
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
		(void)free(pApp_Info->panDatFileBuff);
		(void)free(pApp_Info->pTargetProperties);
#endif
	}
#endif
	A2B_APP_DBG_LOG("Free heap done \r\n");

#if  defined (A2B_BCF_FROM_FILE_IO)
	/* BCF file Close */
	(void)a2b_pal_FileClose(&pApp_Info->ecb);
#endif

	/* Shut down the Stack */
	(void)a2b_systemShutdown(A2B_NULL, &pApp_Info->ecb);

	return (0);
}

/*!****************************************************************************
 *
 *  \b               a2b_reset
 *
 *  This function does A2B network soft reset
 *
 *  \param           [in]    pApp_Info   Pointer to a2b_App_t instance
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
a2b_UInt32 a2b_reset(a2b_App_t *pApp_Info)
{
	a2b_HResult nRet = 0U;

    nRet = a2b_AppWriteReg(pApp_Info->ctx, A2B_NODEADDR_MASTER , A2B_REG_CONTROL, A2B_ENUM_CONTROL_RESET_PE);
    if ( A2B_FAILED(nRet) )
    {
    	nRet = 1U;
    }

    return(nRet);
}

/*!****************************************************************************
 *
 *  \b               a2b_multimasterSetup
 *
 *  Multimaster wrapper for bus set up
 *
 *  \param           pApp_Info		Application Context Info
 *
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          1 on Success
 *					0 on Failure
 ******************************************************************************/
a2b_UInt32 a2b_multimasterSetup(a2b_App_t *pApp_Info)
{
#if (defined (ADI_SIGMASTUDIO_BCF)) && (!defined(ENABLE_SUPERBCF))
	uint8_t nNumMasters;
	uint8_t nIndex;
	uint32_t nResult = 0;

#ifndef ADI_A2B_BCF_COMPRESSED
	nNumMasters = sBusDescription.nNumMasterNode;
#else
	nNumMasters = sCmprBusDescription.nNumMasterNode;
#endif

	for (nIndex = 0u; nIndex < nNumMasters; nIndex++)
	{
		A2B_APP_LOG("\n\r Setting Up Network %d... \n\r",nIndex);
		/* Sub network / chain index */
		pApp_Info[nIndex].ecb.palEcb.nChainIndex = nIndex;
		nResult = a2b_setup(&pApp_Info[nIndex]);
	}

	/* Enable audio only if all the chains are discovered */
	if(nResult == 0)
	{
		for (nIndex = 0u; nIndex < nNumMasters; nIndex++)
		{
#ifdef A2B_ENABLE_AUDIO_FROM_APP
			adi_a2b_EnableAudioHost(nIndex, true);
#endif
		}
	}

	return nResult;

#elif defined(A2B_BCF_FROM_FILE_IO)

	uint8_t nNumChains[1];
	uint8_t nIndex = 0;
	uint32_t nResult = 0;

	a2b_init(&pApp_Info[nIndex]);
	/* BCF file Open */
	a2b_pal_FileOpen(&pApp_Info->ecb, A2B_CONF_BINARY_BCF_FILE_URL);
	nResult = a2b_get_nChainsFrmE2promFileIO(&pApp_Info->ecb, nNumChains);
	pApp_Info[nIndex].nNumChains = nNumChains[0];
	a2b_stop(&pApp_Info[nIndex]);

	for (nIndex = 0u; nIndex < nNumChains[0]; nIndex++)
	{
		A2B_APP_LOG("\n\r Setting Up Network %d... \n\r",nIndex);
		/* Sub network / chain index */
		pApp_Info[nIndex].nNumChains = nNumChains[0];
		pApp_Info[nIndex].ecb.palEcb.nChainIndex = nIndex;
		nResult = a2b_setup(&pApp_Info[nIndex]);
	}

#ifdef A2B_ENABLE_AUDIO_FROM_APP
	for (nIndex = 0u; nIndex < nNumChains[0]; nIndex++)
	{
		adi_a2b_EnableAudioHost(nIndex, true);
	}
#endif

	return nResult;
#else
	A2B_UNUSED(pApp_Info);
	/* Currently not supported */
	return 1;
#endif
}

/*!****************************************************************************
 *
 *  \b               a2b_setup
 *
 *  This function allocates the stack context and initiates discovery on the network
 *  If SuperBCF is enabled it also tries out the BCFs in order for successful discovery.
 *  This function should be called from outside the stack callback context.
 *
 *  \param           pApp_Info		Application Context Info
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 ******************************************************************************/
a2b_UInt32 a2b_setup(a2b_App_t *pApp_Info)
{
	uint32_t nResult = 0;

	do
	{
#ifdef A2B_FEATURE_PARTIAL_DISC
		if ((gApp_Info.bBusDropDetected == A2B_TRUE) && pApp_Info->bdd.policy.bEnablePartialDisc)
		{
			/* Partial re-discovery attempt on fault */
			nResult = a2b_discover(&gApp_Info);
			if (nResult != 0)
			{
				A2B_APP_LOG("\n\rERROR partial discovery \n\r");
				break;
			}
			gApp_Info.bfaultDone = A2B_FALSE;
		}
		else if (gApp_Info.faultNode != gApp_Info.bdd.nodes_count - 1)
		{
			nNodeStartPartialDisc = A2B_NODEADDR_MASTER;
#else
		{
#endif
			nResult = a2b_init(pApp_Info);
			if (nResult != 0)
			{
				A2B_APP_LOG("\n\rERROR INIT \n\r");
				break;
			}

			nResult = a2b_load(pApp_Info);

			if (nResult != 0)
			{
				A2B_APP_LOG("ERROR Load \n\r");
				break;
			}

			if(pApp_Info->ctx != A2B_NULL)
			{
				nResult = a2b_AppWriteReg(pApp_Info->ctx, A2B_NODEADDR_MASTER , A2B_REG_CONTROL, A2B_ENUM_CONTROL_RESET_PE);
			}

			nResult = a2b_start(pApp_Info);

			if (nResult != 0)
			{
				A2B_APP_LOG("ERROR Start \n\r");
				break;
			}

			nResult = a2b_discover(pApp_Info);

			if (nResult != 0)
			{
				A2B_APP_LOG("ERROR discover \n\r");
				break;
			}
		}

	} while (0);

	/* Check Discovery status */
	if (pApp_Info->discoverySuccessful)
	{
		nResult = 0u;
	}
	else
	{
		nResult = 1u;
	}

	return (nResult);
}

/*!****************************************************************************
 *
 *  \b               adi_a2b_extractcustominformation
 *
 *  Helper function used to extract the Custom Node ID for the respective node
 *
 *  \param            pApp_Info		Application Context Info
 *  \param            pBuff 		Memory provided by the user to fetch and store the Custom ID info
 *  \param            nBuffSize  	Size of the buffer allocated
 *  \param            nNodeNum  	Node Number(One based indexing) to fetch the Custom ID info of the respective node number
 *
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return         0 on Success
 *					1 on Failure
 ******************************************************************************/
#if  defined (A2B_BCF_FROM_FILE_IO)
a2b_HResult adi_a2b_extractcustominformation(a2b_App_t *pApp_Info, a2b_UInt8 pBuff[], a2b_UInt8 nBuffSize, a2b_UInt8 nNodeNum)
{
	a2b_HResult status = 0;
	a2b_UInt8 wBuf[2] = { 0, 0 };

	if(nNodeNum > pApp_Info->pTargetProperties->nSubNodes + 1)
	{
		status = 1;
	}
	else if((nBuffSize < pApp_Info->nLenCustomId[nNodeNum - 1]) || (nBuffSize == 0))
	{
		status = 1;
	}
	else
	{
		A2B_PUT_UINT16_BE(pApp_Info->nCustomIdPtr[nNodeNum - 1], wBuf, 0u);

		status = adi_a2b_fileread(&pApp_Info->ecb, 2u, wBuf, pApp_Info->nLenCustomId[nNodeNum - 1], pBuff);
	}
	return status;
}
#endif

/*!****************************************************************************
 *
 *  \b               a2b_fault_monitor
 *
 *  If line diagnostics is enabled this function checks if a line fault occurred
 *  post discovery and initiates re-discovery for the no of times configured in BCF.
 *
 *  \param            pApp_Info		Application Context Info
 *
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return         0 on Success
 *					1 on Failure
 ******************************************************************************/
a2b_UInt32 a2b_multiMasterFault_monitor(a2b_App_t *pApp_Info)
{
#if (defined (ADI_SIGMASTUDIO_BCF)) && (!defined(ENABLE_SUPERBCF))
	uint8_t nNumMasters;
	uint8_t nIndex;
	uint32_t nResult = 0;
#ifndef ADI_A2B_BCF_COMPRESSED
	nNumMasters = sBusDescription.nNumMasterNode;
#else
	nNumMasters = sCmprBusDescription.nNumMasterNode;
#endif

	for (nIndex = 0; nIndex < nNumMasters; nIndex++)
	{
		nResult = a2b_fault_monitor(&pApp_Info[nIndex]);
	}
	return nResult;

#elif defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)

	uint8_t nNumChains[1];
	uint8_t nNumMasters = pApp_Info[0].nNumChains;
	uint8_t nIndex;
	uint32_t nResult = 0;

	for (nIndex = 0u; nIndex < nNumMasters; nIndex++)
	{
		nResult = a2b_fault_monitor(&pApp_Info[nIndex]);
	}
	return nResult;
#else
	A2B_UNUSED(pApp_Info);
	/* Currently not supported */
	return 1;
#endif


}

/*!****************************************************************************
 *
 *  \b               a2b_fault_monitor
 *
 *  If line diagnostics is enabled this function checks if a line fault occurred
 *  post discovery and initiates re-discovery for the no of times configured in BCF.
 *
 *  \param           pApp_Info		Application Context Info
 *
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					1 on Failure
 ******************************************************************************/
a2b_UInt32 a2b_fault_monitor(a2b_App_t *pApp_Info)
{
	a2b_UInt32 nResult = 0;
	a2b_UInt8 nChainIndex;

#ifdef ENABLE_INTERRUPT_PROCESS
	(void)a2b_processIntrpt(pApp_Info);
#endif

	/* ensure the num rediscovery attempt is set to 0 in case auto rediscovery on faults are not enabled */
	if ((pApp_Info->pTargetProperties->bAutoDiscCriticalFault == A2B_DISABLED) && (pApp_Info->pTargetProperties->bAutoRediscOnFault == A2B_DISABLED))
	{
		pApp_Info->pTargetProperties->nAttemptsCriticalFault = 0;
	}

	/* If line diagnostics enabled and non-zero re-attempts configured */
	if ((pApp_Info->pTargetProperties->bLineDiagnostics == 1) && (pApp_Info->pTargetProperties->nAttemptsCriticalFault != 0))
	{
		/* If fault has occurred  */
		if ((pApp_Info->bRetry == A2B_TRUE) && (pApp_Info->bfaultDone == A2B_TRUE))
		{
			pApp_Info->bRetry = A2B_FALSE;
			nChainIndex = pApp_Info->ecb.palEcb.nChainIndex;

#ifdef A2B_ENABLE_AUDIO_FROM_APP
			adi_a2b_EnableAudioHost(nChainIndex, false);
#endif

			/* delay between re-discovery attempt */
			a2b_ActiveDelay(pApp_Info->ctx, pApp_Info->pTargetProperties->nRediscInterval);
#ifdef A2B_FEATURE_PARTIAL_DISC
			if(pApp_Info->bBusDropDetected != false && nNodeStartPartialDisc == -1)
#endif
			{
				/* stop a2b stack */
				nResult = a2b_stop(pApp_Info);
			}


			pApp_Info->nDiscTryCnt++;

			/* Indicate whether the application is attempting to discover the A2B network for the first time or not */
			/* It should be populated as A2B_FALSE here as we are trying to re-discover the network to localize the fault location */
			pApp_Info->bFrstTimeDisc = A2B_FALSE;

			/* Re-discover the network */
			pApp_Info->ecb.palEcb.nChainIndex = nChainIndex;
			nResult = a2b_setup(pApp_Info);

#ifdef A2B_ENABLE_AUDIO_FROM_APP
			if(nResult == 0u)
			{
				/* This will work only when there is no dependency on the other network */
				adi_a2b_EnableAudioHost(nChainIndex, true);
			}
#endif



		}
	}

	return (nResult);
}

/*!****************************************************************************
 *
 *  \b               a2bapp_onInterrupt
 *
 *  The handler for A2B interrupt notifications.
 *
 *  \param           [in]    msg         The A2B interrupt notification message.
 *
 *  \param           [in]    userData    User data associated with the
 *                                       notification registration.
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          None
 *
 ******************************************************************************/
static void a2bapp_onInterrupt(struct a2b_Msg* msg, a2b_Handle userData)
{
	a2b_Interrupt* 	interrupt;
	a2b_App_t 		*pApp_Info  = (a2b_App_t *)userData;

	if (msg)
	{
		interrupt = a2b_msgGetPayload(msg);

		(void)a2bapp_HandlePwrFaultAnomaly(pApp_Info, interrupt);

		if (pApp_Info->bDebug)
		{
			if (interrupt)
			{
				A2B_APP_LOG("\n\rINTERRUPT: intrType=%u nodeAddr=%d\n\r", interrupt->intrType, interrupt->nodeAddr);

				/* Add your code to handle interrupt */
			}
			else
			{
				A2B_APP_LOG("\n\rINTERRUPT: failed to retrieve payload\n\r");
			}
		}
	}
}

/*!****************************************************************************
 *
 *  \b               a2bapp_HandlePwrFaultAnamoly
 *
 *  A specific handler for handling A2B interrupts.
 *
 *  \param           [in]    pApp_Info		Application Context Info
 *
 *  \param           [in]    a2b_Interrupt	Notification payload for A2B_MSGNOTIFY_INTERRUPT notifications.
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          None
 *
 ******************************************************************************/
static a2b_HResult a2bapp_HandlePwrFaultAnomaly(a2b_App_t *pApp_Info, a2b_Interrupt* interrupt)
{
	a2b_UInt8 	nValVmtrMxstat = 0U, nValVmtrMnstat = 0U;
	a2b_HResult nRet = 0U;

	/* VBUS max error along with VMTR interrupt */
	if (interrupt->intrType == A2B_ENUM_INTTYPE_VMTR)
	{
		nRet |= a2bapp_VMTRMonitor(pApp_Info->ctx, A2B_NODEADDR_MASTER, &nValVmtrMnstat, &nValVmtrMxstat);
		if ((nValVmtrMxstat & 0x02U) == 0x02U)
		{
			pApp_Info->bRetry = A2B_TRUE;
		}
	}

	return (nRet);
}

/*!****************************************************************************
 *
 *  \b               a2bapp_VMTRMonitor
 *
 *  API for VMTR monitor to check if the monitored voltages are beyond the min and max thresholds.
 *  This API can be periodically called to determine voltage related errors.
 *  This API assumes that all the VMTR threshold registers are already statically configured.
 *
 *  \param           [in]    pApp_Info		Application Context Info
 *
 *  \param           [in]    nodeAddr       Node address of the node
 *
 *  \param           [out]   minThres		Bit encoded minimum voltage errors
 *  										bit 0: VIN; bit 1: VBUS; bit 2: VIOVDD; bit 3: VTRXVDD;
 *  										bit 4: VDVDD; bit 5: VVBUS - VISENSEP; bit 6: VISENSEN - VVSENSEN;
 *
 *  \param           [out]   maxThres		Bit encoded maximum voltage errors
 *  										bit 0: VIN; bit 1: VBUS; bit 2: VIOVDD; bit 3: VTRXVDD;
 *  										bit 4: VDVDD; bit 5: VVBUS - VISENSEP; bit 6: VISENSEN - VVSENSEN;
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          0 on Success
 *					 1 on Failure
 *
 ******************************************************************************/
static a2b_HResult a2bapp_VMTRMonitor(struct a2b_StackContext *ctx, a2b_Int16 nodeAddr, a2b_UInt8 *minThres, a2b_UInt8 *maxThres)
{
	a2b_HResult nRet = 0U;
	a2b_UInt8   nReg;

	nRet |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_MMRPAGE, 0x01U);
	nReg = (A2B_REG_VMTR_MXSTAT & 0xFF);
	nRet |= a2b_AppReadReg (ctx, nodeAddr, nReg, maxThres);
	nReg = (A2B_REG_VMTR_MNSTAT & 0xFF);
	nRet |= a2b_AppReadReg (ctx, nodeAddr, nReg, minThres);
	nRet |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_MMRPAGE, 0x00U);

	return (nRet);
}

#ifdef A2B_ENABLE_AD244xx_SUPPORT
static void a2b_onCPInterrupt(struct a2b_Msg* msg, a2b_Handle userData)
{
	a2b_CPInterrupt* pCPinterrupt;


	if (msg)
	{
		pCPinterrupt = (a2b_CPInterrupt *)a2b_msgGetPayload(msg);

		if (pCPinterrupt)
		{
			switch(pCPinterrupt->intrType)
			{
			case A2B_CP_RESET_EVT:
				nCPResetNodeCnt++;
				ADI_UART_PRINT("Reset Event");
				ADI_UART_PRINT("\n\r");
				break;
			case A2B_CP_PORT_EVT_ST:

				a2b_CPTxPortInterrupt(pCPinterrupt->aCPPortintrType, pCPinterrupt->nodeAddr);
				a2b_CPRxPortInterrupt(pCPinterrupt->aCPPortintrType, pCPinterrupt->nodeAddr);
				break;
			case A2B_CP_SYS_ERR_ST:
				ADI_UART_PRINT("\n\rSystem error event!!\n\r");
				break;
			case A2B_CP_OTP_ERR_ST:
				ADI_UART_PRINT("\n\rOTP error event!!\n");
				ADI_UART_PRINT("\n\rReset the CP engine\n");
				ADI_UART_PRINT("\n\r");
				break;
			default:
				break;
			}
		}
		else
		{

		}
	}
}
static void a2b_CPTxPortInterrupt(a2b_UInt8 aCPPortintrType[], a2b_Int16 nNodeAddr)
{
	a2b_UInt8 nIdx = 0u;
	a2b_UInt8 nBitIdx = 0u;
	a2b_UInt8 nBitVal = 1u;
	for(nIdx=0; nIdx < A2B_MAX_TXPORTS; nIdx++)
	{
		if (aCPPortintrType[nIdx])
		{
			for(nBitIdx=0u; nBitIdx < 4u; nBitIdx++)
			{
				switch((aCPPortintrType[nIdx]) & (nBitVal << nBitIdx))
				{
				case A2B_TXPORT_AUTH_DONE_ST:
					nCPTxPortAKEDone = A2B_TRUE;
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rTxPort - %d on Master Node Authentication event: %s \n\r", nIdx, (nCPTxPortAKEDone & 0x01u)? DONE_GREEN : LOST_RED);
					}
					else
					{
						ADI_UART_PRINT("\n\rTxPort - %d on Slave-%d Authentication event: %s \n\r", nIdx, nNodeAddr, (nCPTxPortAKEDone & 0x01u)? DONE_GREEN : LOST_RED);
					}
					SetConsoleFont(NORMAL_TEXT);
					ADI_UART_PRINT("A2B>");
					break;

				case A2B_TXPORT_CP_ERR_ST:

					SetConsoleFont(RED_TEXT);
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rTxPort - %d on Master node CP Error event\n\r", nIdx, nNodeAddr);
					}
					else
					{
						ADI_UART_PRINT("\n\rTxPort - %d on Slave-%d CP Error event\n\r", nIdx, nNodeAddr);
					}

					SetConsoleFont(NORMAL_TEXT);
					break;

				case A2B_TXPORT_CP_TIMEOUT:
					SetConsoleFont(RED_TEXT);
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rTxPort-%d on Master node CP Timeout event \n\r", nIdx);
					}
					else
					{
						ADI_UART_PRINT("\n\rTxPort-%d on Slave-%d CP Timeout event \n\r", nIdx, nNodeAddr);
					}
					SetConsoleFont(NORMAL_TEXT);
					break;

				case A2B_TXPORT_REAUTH_RCVD_ST:
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rTxPort-%d on Master node Reauth Received event\n\r", nIdx);
					}
					else
					{
						ADI_UART_PRINT("\n\rTxPort-%d on Slave-%d Reauth Received event\n\r", nIdx, nNodeAddr);
					}

					break;
				default:
					break;
				}
			}
		}
	}

}

static void a2b_CPRxPortInterrupt(a2b_UInt8 aCPPortintrType[], a2b_Int16 nNodeAddr)
{
	a2b_UInt8 nIdx = 0u;
	a2b_UInt8 nBitIdx = 0u;
	a2b_UInt8 nBitVal = 1u;
	a2b_UInt8 nRd;

	for(nIdx=A2B_MAX_TXPORTS; nIdx < A2B_MAX_CP_PORTS; nIdx++)
	{
		if (aCPPortintrType[nIdx])
		{
			for(nBitIdx=0u; nBitIdx < 7u; nBitIdx++)
			{
				switch((aCPPortintrType[nIdx]) & (nBitVal << nBitIdx))
				{
				case A2B_RXPORT_AUTH_DONE_ST:
					nCPRxPortAKEDone = A2B_TRUE;
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rRxPort - %d on Master node Auth event: %s \n\r", nIdx, (nCPRxPortAKEDone & 0x01u)? DONE_GREEN : LOST_RED);
					}
					else
					{
						ADI_UART_PRINT("\n\rRxPort - %d on Slave-%d Auth event: %s \n\r", nIdx, nNodeAddr, (nCPRxPortAKEDone & 0x01u)? DONE_GREEN : LOST_RED);
					}
					break;
				case A2B_RXPORT_CP_ERR_ST:
					SetConsoleFont(RED_TEXT);
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rCP error event on Master node\n\r");
					}
					else
					{
						ADI_UART_PRINT("\n\rCP error event on Slave-%d\n\r",nNodeAddr);
					}

					SetConsoleFont(NORMAL_TEXT);
					ADI_UART_PRINT("A2B>");
					break;
				case A2B_RXPORT_CP_TIMEOUT:
					SetConsoleFont(RED_TEXT);
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rRxPort - %d CP timeout event on Master Node\n\r", nIdx);
					}
					else
					{
						ADI_UART_PRINT("\n\rRxPort - %d CP timeout event on Slave-%d\n\r", nIdx, nNodeAddr);
					}

					SetConsoleFont(NORMAL_TEXT);
					ADI_UART_PRINT("A2B>");
					break;
				case A2B_RXPORT_LINK_SYNCERR:

					SetConsoleFont(RED_TEXT);
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rRxPort - %d Link sync err event on Master Node\n\r", nIdx);
					}
					else
					{
						ADI_UART_PRINT("\n\rRxPort - %d Link sync err event on Slave-%d\n\r", nIdx, nNodeAddr);
					}

					SetConsoleFont(NORMAL_TEXT);
					ADI_UART_PRINT("A2B>");
					break;
				case A2B_RXPORT_INIT_RCVD_ST:
					SetConsoleFont(GREEN_TEXT);
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						ADI_UART_PRINT("\n\rRxPort - %d AKE Received event on Master Node\n\r", nIdx);
					}
					else
					{
						ADI_UART_PRINT("\n\rRxPort - %d AKE Received event on Slave-%d\n\r", nIdx, nNodeAddr);
					}
					SetConsoleFont(NORMAL_TEXT);
					ADI_UART_PRINT("A2B>");

					break;
				case A2B_RXPORT_ENC_CHNGE_ST:

					SetConsoleFont(GREEN_TEXT);
					adi_hsapp_ReadByte(nNodeAddr, HSAPP_PORT_REGADDR16(nIdx, 0x0Cu),  &nRd);
					nRd = nRd & 0x04;
					if(nNodeAddr == A2B_NODEADDR_MASTER)
					{
						if (nRd != 0)
							ADI_UART_PRINT("\n\rEncryption enabled for Rx Port - %d on Master node\n\r", nIdx);
						else
							ADI_UART_PRINT("\n\rEncryption disabled for Rx Port - %d on Master Node\n\r", nIdx);

					}
					else
					{
						if (nRd != 0)
							ADI_UART_PRINT("\n\rEncryption enabled for Rx Port - %d on Slave-%d\n\r", nIdx, nNodeAddr);
						else
							ADI_UART_PRINT("\n\rEncryption disabled for Rx Port - %d on Slave-%d\n\r", nIdx, nNodeAddr);

					}
					SetConsoleFont(NORMAL_TEXT);
					ADI_UART_PRINT("A2B>");

					break;
				case A2B_RXPORT_MUTE_CHNGE_ST:

					SetConsoleFont(NORMAL_TEXT);
					adi_hsapp_ReadByte(nNodeAddr, HSAPP_PORT_REGADDR16(nIdx, 0x0Cu), &nRd);
					nRd = nRd & 0x02;
					if (nRd != 0)
					{
						SetConsoleFont(RED_TEXT);
						if(nNodeAddr == A2B_NODEADDR_MASTER)
						{
							ADI_UART_PRINT("\n\rMute enabled for Rx Port - %d on Master Node\n\r", nIdx);
						}
						else
						{
							ADI_UART_PRINT("\n\rMute enabled for Rx Port - %d on Slave-%d\n\r", nIdx, nNodeAddr);
						}

						SetConsoleFont(NORMAL_TEXT);
					}

					else
					{
						SetConsoleFont(GREEN_TEXT);
						if(nNodeAddr == A2B_NODEADDR_MASTER)
						{
							ADI_UART_PRINT("\n\rMute disabled for Rx Port - %d on Master node\n\r", nIdx);
						}
						else
						{
							ADI_UART_PRINT("\n\rMute disabled for Rx Port - %d on %d node\n\r", nIdx, nNodeAddr);
						}
						SetConsoleFont(NORMAL_TEXT);
					}
					break;
				default:
					break;
				}
			}
		}
	}

}

#endif
#ifdef A2BAPP_NODE_LEVEL_DISC_CALLBACK
/*!****************************************************************************
 *
 *  \b               a2bapp_onNodeDiscovery
 *
 *  The handler for A2B node level discovery notifications.
 *
 *  Unlike a2bapp_onDiscoveryComplete, this is an OPTIONAL callback which user may register to get notification upon each node discovery or node authentication failed.
 *
 *  \param           [in]    msg         The A2B node level discovery notification message.
 *
 *  \param           [in]    userData    User data associated with the
 *                                       notification registration.
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          None
 *
 ******************************************************************************/
static void a2bapp_onNodeDiscovery(struct a2b_Msg* msg, a2b_Handle userData)
{
#ifdef A2BAPP_ENABLE_RTMBOOT
	a2b_HResult 				nRes;
	a2b_App_t *pApp_Info        =   userData;
#endif
	a2b_Nodedscvry* dscvrdNodeMsg;

#ifdef A2B_PRINT_CONSOLE
	a2b_Int16 nodeAddr;
	bdd_Node *bddnode;
	bdd_Network *bddNetwork;
#endif

#ifndef A2BAPP_ENABLE_RTMBOOT
	A2B_UNUSED(userData);
#endif

	if (msg)
	{
		/* details of the currently discovered node */
		dscvrdNodeMsg = a2b_msgGetPayload(msg);

#ifdef A2B_PRINT_CONSOLE
		nodeAddr = dscvrdNodeMsg->nodeAddr; //this will number of slave node discovered
		bddNetwork = (bdd_Network *)dscvrdNodeMsg->bddNetObj;
		bddnode = (bdd_Node *)(&bddNetwork->nodes[nodeAddr]);
#endif
		if (dscvrdNodeMsg)
		{
#ifdef A2BAPP_ENABLE_RTMBOOT
			if(dscvrdNodeMsg->nodeAddr == (A2BAPP_RTM_NODEADDR + 1U))
			{
				/* Call to boot the RTM Module */
				nRes = a2bapp_RTMBoot(pApp_Info->ctx, A2BAPP_RTM_NODEADDR);
				if (A2B_FAILED(nRes))
				{
					A2B_APP_LOG("NODE DISCOVERY: failed to boot RTM\n\r");
				}
			}
#endif
			A2B_APP_LOG("NODE DISCOVERY: nodeType=%u nodeAddr=%d discoveryCompleteCode=%u\n\r", bddnode->nodeType, nodeAddr, dscvrdNodeMsg->discoveryCompleteCode);

#ifndef ENABLE_SUPERBCF
		    /* CRITICAL: Populate the further action which is required to be taken by stack
		     * Set,
		     * 		bContinueDisc to true if required to proceed with discovery process
		     * 		bContinueDisc to false if required to end the discovery process
		     */
		    dscvrdNodeMsg->bContinueDisc = A2B_TRUE;
#endif	/* ENABLE_SUPERBCF */
		}
		else
		{
			A2B_APP_LOG("NODE DISCOVERY: failed to retrieve payload\n\r");
		}
	}
}

#ifdef A2BAPP_ENABLE_RTMBOOT
/*!****************************************************************************
 *
 *  \b               a2bapp_RTMBoot
 *
 *	API for RTM Boot and this is a custom definition and can be replaced by different code if required.
 *
 *  \param           [in]    pApp_Info		Application Context Info
 *
 *  \param           [in]    nodeAddr       Node address of the node
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          A status code that can be checked with the A2B_SUCCEEDED()
 *                   or A2B_FAILED() macro for success or failure of the
 *                   request.
 *
 ******************************************************************************/
static a2b_HResult a2bapp_RTMBoot(struct a2b_StackContext* ctx, a2b_Int16 nodeAddr)
{
	a2b_HResult nRes;

	nRes = a2b_AppWriteReg(ctx, nodeAddr , A2B_REG_SPICFG, A2B_ENUM_SPICFG_SPIMODE_2);/* 0xB0 = SPICFG : Disabling SPI module by writing SPIMODE = 2 in SPICFG register */
	nRes = a2b_AppWriteReg(ctx, nodeAddr , A2B_REG_GPIOOEN, 0xA0);/* 0x4D = GPIOOEN: 0xA0 = 10100000 : GPIO5 (MISO) & GPIO7 (Tuner's RSTN_T pin) are being configured as output here */
	nRes = a2b_AppWriteReg(ctx, nodeAddr , A2B_REG_GPIODAT, 0x20);/* 0x4A = GPIODAT: 0x20 = 00100000 : GPIO7 is pulled low (Tuner's RSTN_T pin) */
	nRes = a2b_AppWriteReg(ctx, nodeAddr , A2B_REG_GPIODAT, 0xA0);/* 0x4A = GPIODAT: 0xA0 = 10100000 : GPIO7 is pulled high (Tuner's RSTN_T pin) */
	nRes = a2b_AppWriteReg(ctx, nodeAddr , A2B_REG_SPICFG, A2B_ENUM_SPICFG_SPIMODE_1);/* 0xB0 = SPICFG : Data Tunnel Target (SPI Master Mode) by writing SPIMODE = 1 in SPICFG register */
	return nRes;
}		/* A2BAPP_ENABLE_RTMBOOT */
#endif
#endif
/*!****************************************************************************
 *
 *  \b               a2bapp_onDiscoveryComplete
 *
 *  The handler which receives the response to the request to discover the
 *  A2B network. This notification is received once complete discovery of the A2B network is complete or upon a fault condition
 *
 *  \param           [in]    msg         The response message to the network
 *                                       discovery request.
 *
 *  \param           [in]    isCancelled An indication of whether the original
 *                                       request was cancelled before it was
 *                                       completed.
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          None
 *
 ******************************************************************************/
static void a2bapp_onDiscoveryComplete(struct a2b_Msg* msg, a2b_Bool isCancelled)
{
	a2b_NetDiscovery* results;

	if ( A2B_NULL == msg)
	{

		/* This should *never* happen */
		A2B_APP_LOG("Error: no response message for network discovery\n\r");

	}
	else
	{
		a2b_App_t *pApp_Info = a2b_msgGetUserData(msg);
		pApp_Info->discoveryDone = true;

		if (isCancelled)
		{

			A2B_APP_LOG("Discovery request was cancelled.\n\r");
#ifdef ADI_UART_ENABLE
			ADI_UART_PRINT("Discovery request was cancelled.\n\r");
#endif
		}
		else
		{
			results = (a2b_NetDiscovery*)a2b_msgGetPayload(msg);
			if (A2B_SUCCEEDED(results->resp.status))
			{

				A2B_APP_LOG("Discovery succeeded with %d nodes discovered\n\r", results->resp.numNodes);
#ifdef ADI_UART_ENABLE
				SetConsoleFont(GREEN_TEXT);
				ADI_UART_PRINT("\n\rDiscovery succeeded with %d nodes discovered\n\r", results->resp.numNodes);
				SetConsoleFont(NORMAL_TEXT);
#endif

				pApp_Info->discoverySuccessful = true;
				pApp_Info->nodesDiscovered = results->resp.numNodes;
#ifdef A2B_ENABLE_AD244xx_SUPPORT
				pApp_Info->ctx->ccb.app.nNodesDiscovered = results->resp.numNodes; /* Application domain nodes discovered count */
#endif
				/* When line fault monitoring is enabled, Allocate a timer to periodically clear BECNT register to reset the error counter */
				if ((pApp_Info->bBecovfTimerEnable == A2B_FALSE) && (pApp_Info->pTargetProperties->bLineDiagnostics == 1))
				{

					pApp_Info->hTmrToHandleBecovf = a2b_timerAlloc(pApp_Info->ctx, (a2b_TimerFunc)a2b_app_handle_becovf, (a2b_Handle)(pApp_Info));
					pApp_Info->bBecovfTimerEnable = A2B_TRUE;

					a2b_timerSet(pApp_Info->hTmrToHandleBecovf, A2B_APP_TMRTOHANDLE_BECOVF_AFTER_INTERVAL, A2B_APP_TMRTOHANDLE_BECOVF_REPEAT_INTERVAL);
					a2b_timerStart(pApp_Info->hTmrToHandleBecovf);
#ifdef A2B_FEATURE_PARTIAL_DISC
					if (pApp_Info->bdd.policy.bEnablePartialDisc && (results->resp.numNodes != 0))
					{
						/* Partial discovery successful */
						nNodeStartPartialDisc = A2B_NODEADDR_MASTER;
					}
#endif
				}
				/* If power fault was detected earlier clear flags and attempt count */
				pApp_Info->nDiscTryCnt = 0;
				pApp_Info->bfaultDone = A2B_FALSE;

			}
			else if( ((results->resp.status & 0xFF) == A2B_EC_BUSY) && (pApp_Info->bdd.policy.bEnablePartialDisc))
			{
				A2B_APP_LOG("\n\rPartial Discovery attempted and no new node found!\n\r");
				pApp_Info->discoverySuccessful = A2B_FALSE;
			}
			else
			{

				A2B_APP_LOG("\n\rDiscovery failed!\n\r");
#ifdef ADI_UART_ENABLE
				ADI_UART_PRINT("\n\rDiscovery failed!\n\r");
#endif

				pApp_Info->discoverySuccessful = false;
				if (((results->resp.status & 0xFFFF) == A2B_EC_CUSTOM_NODE_ID_AUTH) || (results->resp.status & 0xFFFF) == A2B_EC_CUSTOM_NODE_ID_TIMEOUT)
				{
					/* Supplier id authentication failure */
					pApp_Info->faultNode = results->resp.numNodes;
					pApp_Info->bCustomAuthFailed = true;

				}
				if ((results->resp.status & 0xFFFF) == A2B_EC_PERMISSION)
				{
					/* Basic authentication failure */

					pApp_Info->faultNode = results->resp.numNodes + 1;
					pApp_Info->bCustomAuthFailed = true;
					A2B_APP_LOG("Node Authentication failed\n\r");

				}
				if((results->resp.status & 0xFFFF) == A2B_EC_MSTR_NOT_RUNNING)
				{
					/* No master running interrupt */
					A2B_APP_LOG("Master running interrupt not detected. Possible SYNC issues. \n\r");
				}

				if(pApp_Info->bdd.nodes[0].nodeDescr.product == 0x37)
				{
					if((results->resp.status & 0xFFFF) == A2B_EC_TWOSTEP_DISC_FAILED)
					{
						/* Two Step discovery failed */
						A2B_APP_LOG("Two Step discovery failed at Sub node %d\n\r",results->resp.numNodes);
					}
					if( ((results->resp.status & 0xFFFF) == A2B_EC_TWOSTEP_DISC_24V_DISC) || ((results->resp.status & 0xFFFF) == A2B_EC_BUSY))
					{
						if(results->resp.numNodes == 0)
						{
							/* No Next Node */
							A2B_APP_LOG("No Next Node found at main node\n\r");
						}
						else
						{
							/* No Next Node */
							A2B_APP_LOG("No Next Node found at sub node %d \n\r", (results->resp.numNodes-1));
						}
					}
					if(((results->resp.status & 0xFFFF) == A2B_EC_TWOSTEP_DISC_CRIT_FAULT) || ((results->resp.status & 0xFFFF) == A2B_EC_DISCOVERY_PWR_FAULT))
					{
						if(results->resp.numNodes == 0)
						{
							/* Critical Short */
							A2B_APP_LOG("Critical Short found at main node\n\r");
						}
						else
						{
							/* Critical Short */
							A2B_APP_LOG("Critical Short found at sub node %d \n\r", (results->resp.numNodes-1));
						}
					}
				}


				/* Retry again if we are re-trying post power fault */
				if ((pApp_Info->bfaultDone == A2B_TRUE) && (pApp_Info->nDiscTryCnt < pApp_Info->pTargetProperties->nAttemptsCriticalFault))
				{
					pApp_Info->bRetry = A2B_TRUE;
				}
				/* If maximum attempts reached clear the post discovery retry flag */
				else if ((pApp_Info->bfaultDone == A2B_TRUE) && (pApp_Info->nDiscTryCnt == pApp_Info->pTargetProperties->nAttemptsCriticalFault))
				{
					pApp_Info->nDiscTryCnt = 0;
					pApp_Info->bfaultDone = A2B_FALSE;
				}
		        else
		        {
		        	/* Do Nothing */
		        }
			}
		}
	}
}


/*!****************************************************************************
 *
 *  \b               a2bapp_onI2CError
 *
 *  The handler which receives the notification on I2C errors while discovery
 *
 *  \param           [in]    msg         The A2B interrupt notification message.
 *
 *  \param           [in]    userData    User data associated with the
 *                                       notification registration.
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          None
 *
 */
static void a2bapp_onI2CError(struct a2b_Msg *msg, a2b_Handle userData)
{
	a2b_I2CError *I2CError;
	A2B_UNUSED(userData);

	I2CError = (a2b_I2CError *)a2b_msgGetPayload(msg);

	if(I2CError->nodeAddr == A2B_NODEADDR_MASTER)
	{
		A2B_APP_LOG("\n\rIncorrect I2C address at main node\n\r");
	}
	else
	{
		A2B_APP_LOG("\n\rPeripheral configuration failed with I2C address 0x%x on node %d\n\r", I2CError->I2CAddr, I2CError->nodeAddr);
	}
}
/*!****************************************************************************
 *
 *  \b               a2bapp_onPowerFault
 *
 *  The handler which receives power fault diagnostic notifications.
 *
 *  \param           [in]    msg         The diagnostic notification.
 *
 *  \param           [in]    userData    Not used
 *
 *
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          None
 *
 ******************************************************************************/
static void a2bapp_onPowerFault(struct a2b_Msg *msg, a2b_Handle userData)
{
	a2b_PowerFault 	*fault;
	a2b_App_t 		*pAppInfo = (a2b_App_t*)userData;
#ifdef A2B_FEATURE_PARTIAL_DISC
	int i = 0;
#endif
	A2B_UNUSED(userData);

	if (pAppInfo->discoveryDone && (pAppInfo->pTargetProperties->bLineDiagnostics == 1))
	{
		A2B_APP_LOG("\n\r Post Discovery Line fault: ");

	}
	else if(!pAppInfo->discoveryDone)
	{

		A2B_APP_LOG("\n\r Line fault During Discovery: ");

	}
	else
	{
	/* Do Nothing */
	}

	if (msg)
	{
		fault = (a2b_PowerFault *)a2b_msgGetPayload(msg);
		if (fault)
		{
			if (A2B_SUCCEEDED(fault->status))
			{
				switch (fault->intrType)
				{
				case A2B_ENUM_INTTYPE_PWRERR_CS_GND:
					pAppInfo->faultStatus = "Cable Shorted to GND";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_CS_GND;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_CS_VBAT:
					pAppInfo->faultStatus = "Cable Shorted to VBat";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_CS_VBAT;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_CS:
					pAppInfo->faultStatus = "Cable Shorted Together";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_CS;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_CDISC:
					pAppInfo->faultStatus = "Cable Disconnected or Open Circuit";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_CDISC;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_CREV:
					pAppInfo->faultStatus = "Cable Reverse Connected or Wrong Port";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_CREV;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_CDISC_REV:
					pAppInfo->faultStatus = "Cable Open, Reverse Connected or Wrong Port";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_CREV;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_FAULT:
					pAppInfo->faultStatus = "Indeterminate Fault";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_CREV;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_NLS_GND:
					pAppInfo->faultStatus = "Non-Localized Short to GND";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_NLS_GND;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_PWRERR_NLS_VBAT:
					pAppInfo->faultStatus = "Non-Localized Short to VBat";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_PWRERR_NLS_VBAT;
					/* Add your code to handle fault */

					break;
				case A2B_ENUM_INTTYPE_STRTUP_ERR_RTF:
					pAppInfo->faultStatus = "Startup Error - Return to Factory";
					pAppInfo->faultCode = A2B_ENUM_INTTYPE_STRTUP_ERR_RTF;
					/* Add your code to handle fault */

					break;
				default:
					pAppInfo->faultStatus = "Unknown";
					pAppInfo->faultCode = 0xFF;
					/* Add your code to handle fault */

					break;
				}

				/* Set flags to indicate fault post discovery */
				pAppInfo->bfaultDone = A2B_TRUE;
				pAppInfo->bRetry = A2B_TRUE;

				pAppInfo->faultNode = fault->faultNode;
				if (!pAppInfo->discoveryDone || (pAppInfo->pTargetProperties->bLineDiagnostics == 1))
				{
					if (fault->faultNode < 0)
					{
						if(fault->faultNode == A2B_NODEADDR_NOTUSED)
						{
							A2B_APP_LOG("Fault will not be localized because of AD243x High / Medium power bus powered nodes or AD2430 / AD2438 Main in the network: ");
						}
						else
						{
							A2B_APP_LOG("Fault detected on Master node: ");
						}
					}
					else
					{
						A2B_APP_LOG("Fault detected on Slave node %d: ", fault->faultNode);
					}

					A2B_APP_LOG("%s\n\r", pAppInfo->faultStatus);
				}

			}
			else
			{
				if (!pAppInfo->discoveryDone || (pAppInfo->pTargetProperties->bLineDiagnostics == 1))
				{
					A2B_APP_LOG("\n\r Power diagnostic failure ");
				}
			}
		}
	}

	if (pAppInfo->discoveryDone)
	{
		/* Set flags to indicate fault post discovery */
		/* Possibly a node has dropped off the network.. try finding it out */
		(void)a2b_AppDetectBusDrop(pAppInfo);

		/***********************************************************/
		/* Add your custom post discovery fault and link code here */
		/***********************************************************/
#ifdef A2B_FEATURE_PARTIAL_DISC
		if ((pAppInfo->bBusDropDetected == true) && (pAppInfo->bdd.policy.bEnablePartialDisc))
		{
			for ( i = A2B_NODEADDR_MASTER; i < nNodeStartPartialDisc; i++)
			{
				/* Disable all interrupts except DSC_DONE */
				(void)a2b_AppWriteReg(pAppInfo->ctx, i, A2B_REG_INTMSK0, 0x00u);
				(void)a2b_AppWriteReg(pAppInfo->ctx, i, A2B_REG_INTMSK1, 0x00u);
				(void)a2b_AppWriteReg(pAppInfo->ctx, i, A2B_REG_INTMSK2, 0x01u);
			}

			/* Extra Stack tick to ensure all peripheral programmed */
			do
			{
				a2b_stackTick(pAppInfo->ctx);
				i++;
			} while (i < 10 * 10);
		}
#endif // A2B_FEATURE_PARTIAL_DISC
	}
}

#ifdef A2BAPP_LINK_STATICALLY
/*!****************************************************************************
 *
 *  \b               a2bapp_pluginsLoad
 *
 *  If the application is linked statically to its plugins then this
 *  function overrides the default PAL version of this function which loads
 *  plugins dynamically. Instead this function initializes the plugins that
 *  are statically linked to the application.
 *
 *  \param           [in,out]    plugins     A pointer to an array plugins that
 *                                           this function will allocate and
 *                                           return.
 *
 *  \param           [in,out]    numPlugins  This will be assigned the number
 *                                           of plugins that have been
 *                                           loaded and initialized.
 *
 *  \param           [in]        ecb         The environment control block
 *                                           for the A2B stack.
 *
 *  \pre             None
 *
 *  \post            None
 *
 *  \return          A status code that can be checked with the A2B_SUCCEEDED()
 *                   or A2B_FAILED() macro for success or failure of the
 *                   request.
 *
 ******************************************************************************/
a2b_HResult a2bapp_pluginsLoad(struct a2b_PluginApi** plugins, a2b_UInt16* numPlugins, A2B_ECB* ecb)
{
	struct a2b_PluginApi *appPlugins;
	a2b_Int32 i = 0;

	A2B_UNUSED(ecb);
#ifdef A2B_APP_STATIC_MEMORY_FOR_STACK
	if(gpApp_Info[ecb->palEcb.nChainIndex]->bdd.nodes_count * sizeof(**plugins) > A2BAPP_PLUGIN_MEMORY_PER_CHAIN)
	{
		A2B_APP_LOG("Not sufficient memory : failed to load plugins \r\n");
		return 1;
	}
	appPlugins = (a2b_PluginApi *)&gPluginMemBuf[ecb->palEcb.nChainIndex * A2BAPP_PLUGIN_MEMORY_PER_CHAIN];
#else
	appPlugins = calloc(gpApp_Info[ecb->palEcb.nChainIndex]->bdd.nodes_count, sizeof(**plugins));
#endif
	(void)A2B_MASTER_PLUGIN_INIT(&appPlugins[0]);
	A2B_APP_DBG_LOG("Master plugin load done \r\n");

	for (i = 1; i < (gpApp_Info[ecb->palEcb.nChainIndex]->bdd.nodes_count); i++)
	{
		(void)A2B_SLAVE_PLUGIN_INIT(&appPlugins[i]);
	}

	A2B_APP_DBG_LOG("Slave plugins load done \r\n");

	*plugins = appPlugins;
	*numPlugins = gpApp_Info[ecb->palEcb.nChainIndex]->bdd.nodes_count;

	return 0u;
}

/*!****************************************************************************
 *
 *  \b               a2bapp_pluginsUnload
 *
 *  If the application is linked statically to its plugins then this
 *  function overrides the default PAL version of this function which unloads
 *  plugins dynamically. Instead this function deallocates the plugins that
 *  are statically linked to the application.
 *
 *  \param           [in]        plugins     A pointer to an array plugins that
 *                                           this function will deallocate.
 *
 *  \param           [in]        numPlugins  The number of plugins in the array.
 *
 *  \param           [in]        ecb         The environment control block
 *                                           for the A2B stack.
 *
 *  \pre             None
 *
 *  \post            The plugins are unloaded and destroyed after this call.
 *
 *  \return          A status code that can be checked with the A2B_SUCCEEDED()
 *                   or A2B_FAILED() macro for success or failure of the
 *                   request.
 *
 ******************************************************************************/
a2b_HResult a2bapp_pluginsUnload(struct a2b_PluginApi* plugins, a2b_UInt16 numPlugins, A2B_ECB* ecb)
{
	A2B_UNUSED(numPlugins);
	A2B_UNUSED(ecb);
#ifdef A2B_APP_STATIC_MEMORY_FOR_STACK
	(void)memset(gPluginMemBuf, 0, A2BAPP_PLUGIN_NW_MEMORY);
#else
	(void)free(plugins);
#endif
	return 0u;
}
#endif

/*!****************************************************************************
 *
 *  \b               a2b_app_handle_becovf
 *
 *  The routine to periodically reset BECNT and BECOVF registers
 *
 *  \param           [in]        timer		Timer pointer
 *
 *  \param           [in]        userData	User data populated in timer callback function.
 *
 *  \post            None
 *
 *  \return          None
 *
 ******************************************************************************/
static void a2b_app_handle_becovf(struct a2b_Timer* timer, a2b_Handle userData)
{
	a2b_App_t* pApp_Info;
	pApp_Info = (a2b_App_t*)(timer->userData);
	A2B_UNUSED(userData);

	if ((pApp_Info->pTargetProperties->bLineDiagnostics) && (pApp_Info->bfaultDone == false))
	{
		/* Reset the BECNT register for every call back of the timer */
		if (a2b_diagWriteReg(pApp_Info->ctx, A2B_NODEADDR_NOTUSED, A2B_REG_BECNT, A2B_REG_BECNT_RESET) != 0)
		{
			/* reset now to force a bus drop check */
			pApp_Info->nBecovfRstCnt = A2B_BUS_DROP_CHK_PERIOD;
		}
		else
		{
			pApp_Info->nBecovfRstCnt++;
		}

		/* Check for bus drop periodically */
		if ((pApp_Info->nBecovfRstCnt % A2B_BUS_DROP_CHK_PERIOD == 0) && (pApp_Info->bBusDropDetected == false))
		{
			pApp_Info->nBecovfRstCnt = 0;
			(void)a2b_AppDetectBusDrop(pApp_Info);
		}
	}
}

/********************************************************************************/
/*!
* \b a2b_AppDetectBusDrop
*
*  This function reads the Vendor Id register of all A2B nodes discovered
* and declares a bus drop at a particular node where the read value is not the expected.
*
* @param [in] pApp_Info				Application Context Info
*
* @return	0 on Success
*            1 on Failure
* */
/***********************************************************************************/
static a2b_HResult a2b_AppDetectBusDrop(a2b_App_t *pApp_Info)
{
	a2b_UInt32 i;
	a2b_UInt8 nVal;
	a2b_HResult nRet = 0;
#ifdef A2B_FEATURE_PARTIAL_DISC
	pApp_Info->faultNode = pApp_Info->bdd.nodes_count - 1;
#endif
	/* Let us detect bus drop fault */
	for (i = 0u; i < pApp_Info->bdd.nodes_count; i++)
	{
		nVal = 0u;
		nRet = a2b_AppReadReg(pApp_Info->ctx, (i - 1), A2B_REG_VENDOR, &nVal);
		if (nVal != 0xADu)
		{
			pApp_Info->faultNode = (i - 2);
			pApp_Info->bBusDropDetected = true;
			pApp_Info->bfaultDone = A2B_TRUE;
			pApp_Info->bRetry = A2B_TRUE;
#ifdef A2B_FEATURE_PARTIAL_DISC
			nNodeStartPartialDisc = pApp_Info->faultNode + 1;
#endif
			if(pApp_Info->pTargetProperties->bLineDiagnostics == 1)
			{
				A2B_APP_LOG("\n\rBus Drop Detected @ Node: %d", i-2);
			}
#ifdef A2B_PRINT_CONSOLE
			(void)fflush(stdout);
#endif
			break;
		}
	}
#ifdef A2B_FEATURE_PARTIAL_DISC
	/* Node drop recovery */
	if (i == gApp_Info.bdd.nodes_count)
	{
		pApp_Info->bBusDropDetected = false;
	}
#endif
	return (nRet);
}

/********************************************************************************/
/*!
 \b a2b_AppReadReg

 This function  reads a register value of a particular A2B node

 @param [in] ctx				Stack Context
 @param [in] nodeAddr		Application context Information
 @param [in] reg				A2B register address
 @param [out] value			Value

 @return	0 on Success
 1 on Failure
 */
/***********************************************************************************/
static a2b_HResult a2b_AppReadReg(struct a2b_StackContext* ctx, a2b_Int16 nodeAddr, a2b_UInt8 reg, a2b_UInt8* value)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_Byte wBuf[1];
	struct a2b_StackContext* mCtx;

	/* Need a master plugin context to do I2C calls */
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
	if ( A2B_NULL == mCtx)
	{
		/* This should *never* happen */
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		wBuf[0] = reg;

		result = a2b_regWriteRead(mCtx, nodeAddr, 1u, &wBuf[0u], 1u, value);
	}

	return (result);
}

/********************************************************************************/
/*!
 \b a2b_AppWriteReg

 This function writes a register value to a particular A2B node

 @param [in] ctx			Stack Context
 @param [in] nodeAddr		Application context Information
 @param [in] reg			A2B register address
 @param [in] value			Value

 @return	0 on Success
			1 on Failure
 */
/***********************************************************************************/
static a2b_HResult a2b_AppWriteReg(struct a2b_StackContext* ctx, a2b_Int16 nodeAddr, a2b_UInt8 reg, a2b_UInt8 value)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_Byte wBuf[2];
	struct a2b_StackContext* mCtx;

	/* Need a master plugin context to do I2C calls */
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
	if ( A2B_NULL == mCtx)
	{
		/* This should *never* happen */
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INTERNAL);
	}
	else
	{
		wBuf[0] = reg;
		wBuf[1] = value;

		result = a2b_regWrite(mCtx, nodeAddr, 2u, wBuf);
	}

	return (result);
}

/********************************************************************************/
/*!
 \b a2b_AppPWMSetup

 This function configures the PWM registers for the PWM functionality.

 @param [in] ctx     		Stack Context
 @param [in] nodeAddr		Address of the slave node for which PWM setting is intended for.
 @param [in] chnlPWM		PWM channel
 @param [in] freqPWM		PWM frequency
 @param [in] blinkPWM		PWM blink rate
 @param [in] nDutyCycle		PWM duty cycle

 @return	0 on Success
            1 on Failure
 */
/***********************************************************************************/
a2b_HResult a2b_AppPWMSetup(struct a2b_StackContext *ctx,
							a2b_Int16 nodeAddr,
		                    a2b_PwmChnl  chnlPWM,
							a2b_PwmFreq freqPWM,
							a2b_PwmBlink blinkPWM,
							a2b_UInt16  nDutyCycle)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_UInt8 nReg, nVal, bitPos;

	/* Ensure that SPI is disabled (PWM pins are shared with SPI) */
	result |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_SPICFG, A2B_ENUM_SPICFG_SPIMODE_2);

	/* Disable the PWM channel if already running to apply new configuration */
	nReg = (A2B_REG_PWMCFG & 0xFFu) ; /* Register address changed to 8-bit so as to use the a2b_AppReadReg */
	result |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_MMRPAGE, 0x01U);
	result |= a2b_AppReadReg (ctx, nodeAddr, nReg, &nVal);
	bitPos = 1u << (a2b_UInt8)chnlPWM;
	nVal  &= (a2b_UInt8)~(bitPos);
	result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, nVal);


	/* Set random frequency hopping */
	if (freqPWM != A2B_PWM_FREQ_HOPPING)
	{
		nReg = (A2B_REG_PWMFREQ & 0xFF) ;
		result |= a2b_AppReadReg (ctx, nodeAddr, nReg, &nVal);

		switch (chnlPWM)
		{
			case A2B_PWM_CHANNEL_1:
			case A2B_PWM_CHANNEL_2:
			case A2B_PWM_CHANNEL_3:
				nVal &= ~A2B_BITM_PWMFREQ_PWMPFREQ;
				nVal |= ((a2b_UInt8)freqPWM & 0x0Fu) << A2B_BITP_PWMFREQ_PWMPFREQ; /* Set PWMFREQ.PWMPFREQ */
				result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, nVal);
				break;

			case A2B_PWM_CHANNEL_OE:
				nVal &= ~A2B_BITM_PWMFREQ_PWMOFREQ;
				nVal |= ((a2b_UInt8)freqPWM & 0x0Fu) << A2B_BITP_PWMFREQ_PWMOFREQ; /* Set PWMFREQ.PWMOFREQ */
				result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, nVal);
				break;

			default:
				/* Unknown channel */
				result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INVALID_PARAMETER);
				break;
		}
	}
	else
	{
		nReg = (A2B_REG_PWMCFG & 0xFF) ;
		result |= a2b_AppReadReg (ctx, nodeAddr, nReg, &nVal);

		switch (chnlPWM)
		{
			case A2B_PWM_CHANNEL_1:
			case A2B_PWM_CHANNEL_2:
			case A2B_PWM_CHANNEL_3:
				/* Set PWMCFG.PWMPRAND */
				result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, (nVal | A2B_BITM_PWMCFG_PWMPRAND)); /* Bit 4 is PWMPRAND */
				break;

			case A2B_PWM_CHANNEL_OE:
				/* Set PWMCFG.PWMORAND */
				result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, (nVal | A2B_BITM_PWMCFG_PWMORAND)); /* Bit 5 is PWMORAND */
				break;

			default:
				/* Unknown channel */
				result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INVALID_PARAMETER);
				break;
		}
	}

	result |= a2b_AppPWMControl(ctx, nodeAddr, chnlPWM, blinkPWM, nDutyCycle);

	/* Enable the PWM channel to apply new configuration */
	nReg = (A2B_REG_PWMCFG & 0xFFu) ; /* Register address changed to 8-bit so as to use the a2b_AppReadReg */
	result |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_MMRPAGE, 0x01U);
	result |= a2b_AppReadReg (ctx, nodeAddr, nReg, &nVal);
	bitPos = 1u << (a2b_UInt8)chnlPWM;
	nVal  |= (a2b_UInt8)bitPos;
	result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, nVal);
	
	result |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_MMRPAGE, 0x00U);

	return (result);
}

/********************************************************************************/
/*!
 \b a2b_AppPWMControl

 This function controls the PWM blink and duty cycle registers and can be called
 while PWM channel is running

 @param [in] ctx			Stack Context
 @param [in] nodeAddr		Address of the slave node for which PWM setting is intended for.
 @param [in] chnlPWM		PWM channel
 @param [in] freqPWM		PWM frequency
 @param [in] blinkPWM		PWM blink rate
 @param [in] nDutyCycle		PWM duty cycle

 @return	0 on Success
            1 on Failure
 */
/***********************************************************************************/
a2b_HResult a2b_AppPWMControl(struct a2b_StackContext *ctx,
							 a2b_Int16 nodeAddr,
		                     a2b_PwmChnl  chnlPWM,
							 a2b_PwmBlink blinkPWM,
							 a2b_UInt16  nDutyCycle)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	a2b_UInt8 nReg, nVal, nBlink, bitPos;
	a2b_Byte wBuf[3u];
	struct a2b_StackContext* mCtx;

	result |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_MMRPAGE, 0x01U);
	nBlink = (a2b_UInt8)blinkPWM & A2B_BITM_PWMBLINK1_PWM1BLINK; /* 3-bit blink value */

	switch (chnlPWM)
	{
		case A2B_PWM_CHANNEL_1:
		case A2B_PWM_CHANNEL_2:
			nReg = (A2B_REG_PWMBLINK1 & 0xFF) ;
			result |= a2b_AppReadReg (ctx, nodeAddr, nReg, &nVal);
			bitPos = ((a2b_UInt8)chnlPWM - (a2b_UInt8)A2B_PWM_CHANNEL_1) * A2B_BITP_PWMBLINK1_PWM2BLINK;
			nVal &= ~(A2B_BITM_PWMBLINK1_PWM1BLINK << bitPos);
			nVal |= (nBlink << bitPos); /* Set PWMBLINK1 */
			result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, nVal);
			break;

		case A2B_PWM_CHANNEL_3:
		case A2B_PWM_CHANNEL_OE:
			nReg = (A2B_REG_PWMBLINK2 & 0xFF) ;
			result |= a2b_AppReadReg (ctx, nodeAddr, nReg, &nVal);
			bitPos = ((a2b_UInt8)chnlPWM - (a2b_UInt8)A2B_PWM_CHANNEL_3) * A2B_BITP_PWMBLINK2_PWMOEBLINK;
			nVal &= ~(A2B_BITM_PWMBLINK2_PWM3BLINK << bitPos);
			nVal |= (nBlink << bitPos); /* Set PWMBLINK2 */
			result |= a2b_AppWriteReg(ctx, nodeAddr, nReg, nVal);
			break;

		default:
			/* Unknown channel */
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INVALID_PARAMETER);
			break;
	}

	nReg = (A2B_REG_PWM1VALL & 0xFFu) + ((a2b_UInt8)chnlPWM - (a2b_UInt8)A2B_PWM_CHANNEL_1) * 2u;
	/* Need a master plugin context to do I2C calls */
	mCtx = (a2b_StackContext*)a2b_stackContextFind(ctx, A2B_NODEADDR_MASTER);
	if ( A2B_NULL == mCtx)
	{
		/* This should *never* happen */
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_DIAG, A2B_EC_INVALID_PARAMETER);
	}
	else
	{
		wBuf[0] = nReg;
		wBuf[1] = (a2b_UInt8)(nDutyCycle & 0xFFu);
		wBuf[2] = (a2b_UInt8)((nDutyCycle & 0xFF00u) >> 8u);
		result = a2b_regWrite(mCtx, nodeAddr, 3u, wBuf);      /* Update duty cycle */
	}
	
	result |= a2b_AppWriteReg(ctx, nodeAddr, A2B_REG_MMRPAGE, 0x00U);

	return (result);
}

#ifdef ENABLE_INTERRUPT_PROCESS
/********************************************************************************/
/*!
 @brief This function process a2b interrupts

 @param [in] void				Stack Context


 @return	0 on Success
 1 on Failure
 */
/***********************************************************************************/
static a2b_HResult a2b_processIntrpt(a2b_App_t *pApp_Info)
{
	a2b_HResult nResult = 0;
	if(pApp_Info->bIntrptLatch == 1u)
	{
		pApp_Info->bIntrptLatch = 0u;

		nResult = a2b_intrQueryIrq(pApp_Info->ctx);
#ifdef A2B_ENABLE_AD244xx_SUPPORT
#ifdef A2B_CP_COMBINED_IRQ_PROCESS
		(void)a2b_globalCPintrQueryIrq(pApp_Info->ctx);
#endif
#endif

	}
	return nResult;

}
#endif

#ifdef A2B_THIRD_PARTY
/********************************************************************************/
/*!
 @brief 		This function  attempts to load a BDD stored as a binary Protobuf encoded file
 from 'bddPath' and decodes it into the BDD structure. If no BDD path
 is provided then the built-in BDD is decoded.

 @param [in]   bddPath 	The (optional) path to a Protobuf formatted binary BDD file.
 If the default internal BDD should be used then this should be set to A2B_NULL.
 @param [out]  bdd		Pointer to the BDD structure that will be initialized with the decoded
 Protobuf formatted binary BDD.

 @return		A2B_TRUE on Success
 A2B_FALSE on failure
 */
/***********************************************************************************/
static a2b_Bool a2b_loadBdd ( const a2b_Char* bddPath, bdd_Network* bdd)
{
	a2b_Bool bIsError = A2B_FALSE;
	a2b_Byte* a2bNetwork = (a2b_Byte*)gA2bNetwork;
	a2b_UInt32 a2bNetworkLen = gA2bNetworkLen;
	a2b_UInt32 nRead;
	FILE* fp = A2B_NULL;

	if ( A2B_NULL != bddPath )
	{
		if ( !bIsError )
		{

			fp = fopen(bddPath, "r");
			if ( A2B_NULL == fp )
			{
				bIsError = A2B_TRUE;
			}
		}
		if ( !bIsError )
		{
			fseek(fp, 0L, SEEK_END);
			a2bNetworkLen = ftell(fp);
			a2bNetwork = malloc(a2bNetworkLen);
			if ( a2bNetwork == A2B_NULL )
			{
				bIsError = A2B_TRUE;
			}
			fseek(fp, 0L, SEEK_SET);
		}

		if ( !bIsError )
		{
			nRead = fread(a2bNetwork, 1, a2bNetworkLen, fp);
			if ( a2bNetworkLen != nRead )
			{
				bIsError = A2B_TRUE;
			}
		}
	}

	/* If no error so far then try to decode the BDD binary */
	if ( !bIsError )
	{
		if ( !a2b_bddDecode(a2bNetwork, a2bNetworkLen, bdd) )
		{
			bIsError = A2B_TRUE;
		}
	}

	/*
	 * Clean up resources that may have been allocated
	 */
	if ( A2B_NULL != fp )
	{
		fclose(fp);
	}

	/* We we allocated the buffer from the heap then ... */
	if ( a2bNetwork != (a2b_Byte*)gA2bNetwork )
	{
		free(a2bNetwork);
	}

	return !bIsError;
}
#endif

#ifdef __ADSPBF71x__
/*!****************************************************************************
 *
 *  \b              a2bapp_initTdmSettings
 *
 *  Initialize the TDM Settings from bdd.
 *
 *  \param          [in]    ecb	pointer to ECB structure
 *  \param          [in]    bdd	pointer to BDD network
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         None
 *
 ******************************************************************************/
static void a2bapp_initTdmSettings(A2B_ECB* ecb, const bdd_Network* bdd)
{
	a2b_TdmSettings *pTdmSettings;
	const bdd_Node *pBddNodeObj;
	a2b_UInt32 nReg;

	memset(&ecb->palEcb.oTdmSettings, 0u, sizeof(a2b_TdmSettings));

	pTdmSettings = &ecb->palEcb.oTdmSettings;
	pBddNodeObj = &bdd->nodes[0u];

	/* Only few TDM settings are being populated below as not all are being used to configure the SOC and A2B SPORT's */
	switch (pBddNodeObj->i2cI2sRegs.i2sgcfg & A2B_BITM_I2SGCFG_TDMMODE)
	{
		case A2B_ENUM_I2SGCFG_TDM2:
		pTdmSettings->tdmMode = 2u;
		break;
		case A2B_ENUM_I2SGCFG_TDM4:
		pTdmSettings->tdmMode = 4u;
		break;
		case A2B_ENUM_I2SGCFG_TDM8:
		pTdmSettings->tdmMode = 8u;
		break;
		case A2B_ENUM_I2SGCFG_TDM16:
		pTdmSettings->tdmMode = 16u;
		break;
		case A2B_ENUM_I2SGCFG_TDM32:
		pTdmSettings->tdmMode = 32u;
		break;
		default:
		pTdmSettings->tdmMode = 2u;
		break;
	}

	nReg = pBddNodeObj->i2cI2sRegs.i2sgcfg;
	pTdmSettings->slotSize = (nReg & A2B_BITM_I2SGCFG_TDMSS ) ? 16u : 32u;
	pTdmSettings->halfCycle = (a2b_Bool)(( nReg & A2B_BITM_I2SGCFG_ALT ) == A2B_ENUM_I2SGCFG_ALT_EN );
	pTdmSettings->prevCycle = (a2b_Bool)(( nReg & A2B_BITM_I2SGCFG_EARLY ) == A2B_ENUM_I2SGCFG_EARLY_EN );
	pTdmSettings->fallingEdge = (a2b_Bool)(( nReg & A2B_BITM_I2SGCFG_INV ) == A2B_ENUM_I2SGCFG_INV_EN );

	nReg = pBddNodeObj->i2cI2sRegs.i2scfg;
	pTdmSettings->rx.invertBclk = (a2b_Bool)(( nReg & A2B_BITM_I2SCFG_RXBCLKINV ) == A2B_ENUM_I2SCFG_RXBCLKINV_EN );
	pTdmSettings->rx.interleave = (a2b_Bool)(( nReg & A2B_BITM_I2SCFG_RX2PINTL ) == A2B_ENUM_I2SCFG_RX2PINTL_EN );
	pTdmSettings->rx.pinEnabled = (a2b_UInt8)((nReg & (A2B_BITM_I2SCFG_RX1EN | A2B_BITM_I2SCFG_RX0EN)) >> A2B_BITP_I2SCFG_RX0EN);

	pTdmSettings->tx.invertBclk = (a2b_Bool)(( nReg & A2B_BITM_I2SCFG_TXBCLKINV ) == A2B_ENUM_I2SCFG_TXBCLKINV_EN );
	pTdmSettings->tx.interleave = (a2b_Bool)(( nReg & A2B_BITM_I2SCFG_TX2PINTL ) == A2B_ENUM_I2SCFG_TX2PINTL_EN );
	pTdmSettings->tx.pinEnabled = (a2b_UInt8)((nReg & (A2B_BITM_I2SCFG_TX1EN | A2B_BITM_I2SCFG_TX0EN)) >> A2B_BITP_I2SCFG_TX0EN);
}
#endif  /* __ADSPBF7xx__ */

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

