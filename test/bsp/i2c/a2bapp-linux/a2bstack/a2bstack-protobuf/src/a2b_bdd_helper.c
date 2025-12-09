/*=============================================================================
 *
 * Project: a2bstack
 *
 * Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
 * This software is proprietary & confidential to Analog Devices, Inc.
 * and its licensors. See LICENSE for complete details.
 *
 *=============================================================================
 *
 * \file:   a2b_bdd_helper.c
 * \brief:  The implementation of BDD Helper functions
 *
 *=============================================================================
 */


 /*======================= I N C L U D E S =========================*/

#define A2B_DUMP_BDD

#ifdef A2B_DUMP_BDD
#include "stddef.h"
#include "stdio.h"
#endif

#include "a2b/ctypes.h"
#include "a2b_bdd_helper.h"
#include "a2b/regdefs.h"
#include "a2b/util.h"
#ifdef A2B_BCF_FROM_FILE_IO
#include "adi_a2b_externs.h"
#endif
/*======================= D E F I N E S ===========================*/
/*! Configure only non default values */
#define A2B_CONFIGURE_ONLY_NON_DEFAULT	(1)
#define ENABLE_AD243x_SUPPORT
/*! Updates the 'has' field */
#define A2B_UPDATE_HAS(x, y, z) x.has_##y = (x.y == z) ? A2B_FALSE : A2B_TRUE
#define A2B_SET_HAS(x, y) x.has_##y =  A2B_TRUE;
#define A2B_CLEAR_HAS(x, y) x.has_##y =  A2B_FALSE;
#define A2B_CONFIG_HAS(x, y, z) x.has_##y =  z;
#define A2B_UPDATE_HAS_ARR(x,y,k,z) x.has_##y = (x.y[k] == z) ? A2B_FALSE : A2B_TRUE;

#define A2B_MAX_SIZE_OF_CUSTOM_ID_TO_READ (50)

/*! \addtogroup Network_Configuration
 *  @{
 */

 /*! \addtogroup Bus_Configuration  Bus Configuration
 * @{
 */
 /*======================= L O C A L  P R O T O T Y P E S  =========*/
#ifdef ADI_SIGMASTUDIO_BCF
static void  adi_a2b_ParseMasterNCD(bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg,
	ADI_A2B_COMMON_CONFIG* pCommon);
static void  adi_a2b_ParseSlaveNCD(bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg);
static void adi_a2b_ParseMasterPinMux34(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg);
static void adi_a2b_ParseMasterPinMux56(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg);
static void  adi_a2b_ParseSlavePinMux012(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg);
static void adi_a2b_ParseSlavePinMux34(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg);
static void  adi_a2b_ParseSlavePinMux56(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg);
#ifdef ENABLE_AD242x_SUPPORT
static void  adi_a2b_ParseMasterNCD_242x(bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg,
	ADI_A2B_COMMON_CONFIG* pCommon);
static void  adi_a2b_ParseSlaveNCD_242x(bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg);
static void  adi_a2b_ParseMasterPinMux12(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg);
static void  adi_a2b_ParseMasterPinMux7(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg);
static void  adi_a2b_ParseSlavePinMux7(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg);

static void adi_a2b_SetforAllReg_241x_master(bdd_Node* pNode);
static void adi_a2b_SetforAllReg_241x_slave(bdd_Node* pNode);
static void adi_a2b_SetforAllReg_242x_master(bdd_Node* pNode);
static void adi_a2b_SetforAllReg_242x_slave(bdd_Node* pNode);
static void adi_a2b_CheckforDefault(bdd_Node* pNode);
static void adi_a2b_CheckforAutoConfig(bdd_Node* pNode, bool bAutoConfig);

#endif

#ifdef ENABLE_AD243x_SUPPORT
static void  adi_a2b_ParseMasterNCD_243x(bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg,
	ADI_A2B_COMMON_CONFIG* pCommon);
static void  adi_a2b_ParseSlaveNCD_243x(bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg);
static void adi_a2b_ParseSioPinCfg(bdd_Node* pNode,
	A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings);
static void adi_a2b_ParseNonSioPinCfg(bdd_Node* pNode,
	A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings);
static void  adi_a2b_ParseMasterPinMux0(bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg);
static void adi_a2b_ParseGPIO7PinCfg(bdd_Node* pNode,
	A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings);
static void adi_a2b_ParseI2CPinCfg(bdd_Node* pNode,
	A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings);
static void adi_a2b_ParseSPIPinCfg(bdd_Node* pNode,
	A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings);
static void adi_a2b_ParseADRPinCfg(bdd_Node* pNode,
	A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings);
static void adi_a2b_ParseVmtr(bdd_Node* pNode,
	A2B_VMTR_SETTINGS* poVmtrSettings);
static void adi_a2b_ParsePwm(bdd_Node* pNode,
	A2B_PWM_SETTINGS* poPwmSettings);
static void adi_a2b_ParseSpiDT(bdd_Node* pNode,
	A2B_SPI_SETTINGS* poSpiSettings);
static void adi_a2b_SetforAllReg_243x_master(bdd_Node* pNode);
static void adi_a2b_SetforAllReg_243x_slave(bdd_Node* pNode);
#endif

#elif defined (A2B_BCF_FROM_SOC_EEPROM)

a2b_HResult a2b_EepromWriteRead(a2b_Handle hnd, a2b_UInt16 addr, a2b_UInt16 nWrite,
	const a2b_Byte* wBuf, a2b_UInt16 nRead,
	a2b_Byte* rBuf);
#elif defined (A2B_BCF_FROM_FILE_IO)

#endif

/*<! Utility functions*/
#ifdef ADI_SIGMASTUDIO_BCF
static void setRegVal(a2b_UInt32 *pnRegVal, a2b_UInt8 nFieldVal, a2b_UInt8 nMask, a2b_UInt8 nPos);
#endif

/**
 @}
*/

/**
 @}
*/
/*======================= D A T A  ================================*/

/*****************************************************************************************/
/*!
@brief      This function parses Bus Configuration Data(BCD) to Peripheral Config Table

@param [in] pFrameWorkHandle    Framework configuration pointer
@param [in] pBusDescription     Pointer to bus configuration data

@return     void

*/
/******************************************************************************************/



/*======================= M A C R O S =============================*/


/*======================= C O D E =================================*/

#ifdef A2B_DUMP_BDD
/*!****************************************************************************
*
*  \b              a2b_bddDecode
*
*  Helper routine to decode an A2B BDD.
*
*  \param          [in]    bdd          BDD to dump
*
*  \pre            None
*
*  \post           None
*
*  \return         True = success, False = Failure
*
******************************************************************************/
void
a2b_bddDump
(
	bdd_Network* bdd
)
{
	size_t  idx, idx2;

#define OPTb( sTitle, val, bHas ) \
        if (bHas) { printf("%s %s\n", sTitle, (val)?"TRUE":"false"); } else { printf("%s NOT SET\n", sTitle); }
#define OPTs( sTitle, val, bHas ) \
        if (bHas) { printf("%s '%s'\n", sTitle, val); } else { printf("%s NOT SET\n", sTitle); }
#define OPT8( sTitle, val, bHas ) \
        if (bHas) { printf("%s %02X\n", sTitle, val); } else { printf("%s NOT SET\n", sTitle); }
#define OPT16( sTitle, val, bHas ) \
        if (bHas) { printf("%s %04X\n", sTitle, val); } else { printf("%s NOT SET\n", sTitle); }
#define OPT32( sTitle, val, bHas ) \
        if (bHas) { printf("%s %08X\n", sTitle, val); } else { printf("%s NOT SET\n", sTitle); }

#define REQsz( sTitle, val ) printf("%s %08lX\n", sTitle, val)
#define REQb( sTitle, val )  printf("%s %s\n", sTitle, (val)?"TRUE":"false")
#define REQs( sTitle, val )  printf("%s '%s'\n", sTitle, val)
#define REQ8( sTitle, val )  printf("%s %02X\n", sTitle, val)
#define REQ16( sTitle, val ) printf("%s %04X\n", sTitle, val)
#define REQ32( sTitle, val ) printf("%s %08X\n", sTitle, val)

	if (bdd == A2B_NULL)
	{
		printf("a2b_bddDump: NULL bdd\n");
		return;
	}

	printf("\na2b_bddDump: START ===========================================\n");

	printf("sampleRate: %08X (%d)\n", bdd->sampleRate, bdd->sampleRate);
	REQ32("masterAddr:", bdd->masterAddr);

	printf("metaData:\n");
	REQ32(">    date:        ", bdd->metaData.date);
	REQ32(">    version:     ", bdd->metaData.version);
	OPTs(">    author:      ", bdd->metaData.author, bdd->metaData.has_author);
	OPTs(">    organization:", bdd->metaData.organization, bdd->metaData.has_organization);
	OPTs(">    company:     ", bdd->metaData.company, bdd->metaData.has_company);

	printf("policy:\n");
	switch (bdd->policy.discoveryMode)
	{
	case 0:  printf(">    discoveryMode: SIMPLE\n"); break;
	case 1:  printf(">    discoveryMode: MODIFIED\n"); break;
	case 2:  printf(">    discoveryMode: OPTIMIZED\n"); break;
	case 3:  printf(">    discoveryMode: ADVANCED\n"); break;
	default: printf(">    discoveryMode: UNKNOWN (%d)\n", bdd->policy.discoveryMode); break;
	}
	switch (bdd->policy.cfgMethod)
	{
	case 0:  printf(">    cfgMethod:     AUTO\n"); break;
	case 1:  printf(">    cfgMethod:     BDD\n"); break;
	case 2:  printf(">    cfgMethod:     HYBRID\n"); break;
	default: printf(">    cfgMethod:     UNKNOWN (%d)\n", bdd->policy.cfgMethod); break;
	}
	switch (bdd->policy.cfgPriority)
	{
	case 0:  printf(">    cfgPriority:   AUTO\n"); break;
	case 1:  printf(">    cfgPriority:   BDD\n"); break;
	default: printf(">    cfgPriority:   UNKNOWN (%d)\n", bdd->policy.cfgPriority); break;
	}
	switch (bdd->policy.cfgErrPolicy)
	{
	case 0:  printf(">    cfgErrPolicy:  FATAL\n"); break;
	case 1:  printf(">    cfgErrPolicy:  ERROR\n"); break;
	case 2:  printf(">    cfgErrPolicy:  WARN\n"); break;
	case 3:  printf(">    cfgErrPolicy:  NONE\n"); break;
	default: printf(">    cfgErrPolicy:  UNKNOWN (%d)\n", bdd->policy.cfgErrPolicy); break;
	}

#if 0
	printf("Streams: (cnt: %d)\n", bdd->streams_count);
	for (idx = 0; idx < bdd->streams_count; idx++)
	{
		REQsz(">----idx:                 ", idx);
		REQs(">    name:                ", bdd->streams[idx].name);
		REQ32(">    sampleRate:          ", bdd->streams[idx].sampleRate);
		REQ32(">    sampleRateMultiplier:", bdd->streams[idx].sampleRateMultiplier);
		REQ32(">    numChans:            ", bdd->streams[idx].numChans);

	} /* streams */
#endif

	printf("Nodes: (cnt: %d)\n", bdd->nodes_count);
	for (idx = 0; idx < bdd->nodes_count; idx++)
	{
		REQsz(">----idx:          ", idx);
		switch (bdd->nodes[idx].nodeType)
		{
		case 0:  printf(">    nodeType: UNKNOWN\n"); break;
		case 1:  printf(">    nodeType: MASTER\n"); break;
		case 2:  printf(">    nodeType: SLAVE\n"); break;
		default: printf(">    nodeType: UNKNOWN (%d)\n", bdd->nodes[idx].nodeType); break;
		}
		printf(">    ctrlRegs:\n");
		OPT32(">        bcdnslots:", bdd->nodes[idx].ctrlRegs.bcdnslots, bdd->nodes[idx].ctrlRegs.has_bcdnslots);
		OPT32(">        ldnslots: ", bdd->nodes[idx].ctrlRegs.ldnslots, bdd->nodes[idx].ctrlRegs.has_ldnslots);
		OPT32(">        lupslots: ", bdd->nodes[idx].ctrlRegs.lupslots, bdd->nodes[idx].ctrlRegs.has_lupslots);
		REQ32(">        dnslots:  ", bdd->nodes[idx].ctrlRegs.dnslots);
		REQ32(">        upslots:  ", bdd->nodes[idx].ctrlRegs.upslots);
		REQ32(">        respcycs: ", bdd->nodes[idx].ctrlRegs.respcycs);
		OPT32(">        slotfmt:  ", bdd->nodes[idx].ctrlRegs.slotfmt, bdd->nodes[idx].ctrlRegs.has_slotfmt);

		if (!bdd->nodes[idx].has_intRegs)
		{
			printf(">    ctrlRegs: NOT SET\n");
		}
		else
		{
			printf(">    intRegs:\n");
			OPT32(">        intmsk0:", bdd->nodes[idx].intRegs.intmsk0, bdd->nodes[idx].intRegs.has_intmsk0);
			OPT32(">        intmsk1:", bdd->nodes[idx].intRegs.intmsk1, bdd->nodes[idx].intRegs.has_intmsk1);
			OPT32(">        intmsk2:", bdd->nodes[idx].intRegs.intmsk2, bdd->nodes[idx].intRegs.has_intmsk2);
			OPT32(">        becctl: ", bdd->nodes[idx].intRegs.becctl, bdd->nodes[idx].intRegs.has_becctl);
		}

		if (!bdd->nodes[idx].has_tuningRegs)
		{
			printf(">    tuningRegs: NOT SET\n");
		}
		else
		{
			printf(">    tuningRegs:\n");
			OPT32(">        vregctl:", bdd->nodes[idx].tuningRegs.vregctl, bdd->nodes[idx].tuningRegs.has_vregctl);
			OPT32(">        txactl: ", bdd->nodes[idx].tuningRegs.txactl, bdd->nodes[idx].tuningRegs.has_txactl);
			OPT32(">        rxactl: ", bdd->nodes[idx].tuningRegs.rxactl, bdd->nodes[idx].tuningRegs.has_rxactl);
			OPT32(">        txbctl: ", bdd->nodes[idx].tuningRegs.txbctl, bdd->nodes[idx].tuningRegs.has_txbctl);
			OPT32(">        rxbctl: ", bdd->nodes[idx].tuningRegs.rxbctl, bdd->nodes[idx].tuningRegs.has_rxbctl);
		}

		printf(">    i2cI2sRegs:\n");
		REQ32(">        i2ccfg:     ", bdd->nodes[idx].i2cI2sRegs.i2ccfg);
		REQ32(">        pllctl:     ", bdd->nodes[idx].i2cI2sRegs.pllctl);
		REQ32(">        i2sgcfg:    ", bdd->nodes[idx].i2cI2sRegs.i2sgcfg);
		REQ32(">        i2scfg:     ", bdd->nodes[idx].i2cI2sRegs.i2scfg);
		OPT32(">        i2srate:    ", bdd->nodes[idx].i2cI2sRegs.i2srate, bdd->nodes[idx].i2cI2sRegs.has_i2srate);
		OPT32(">        i2stxoffset:", bdd->nodes[idx].i2cI2sRegs.i2stxoffset, bdd->nodes[idx].i2cI2sRegs.has_i2stxoffset);
		OPT32(">        i2srxoffset:", bdd->nodes[idx].i2cI2sRegs.i2srxoffset, bdd->nodes[idx].i2cI2sRegs.has_i2srxoffset);
		OPT32(">        syncoffset: ", bdd->nodes[idx].i2cI2sRegs.syncoffset, bdd->nodes[idx].i2cI2sRegs.has_syncoffset);
		REQ32(">        pdmctl:     ", bdd->nodes[idx].i2cI2sRegs.pdmctl);
		REQ32(">        errmgmt:    ", bdd->nodes[idx].i2cI2sRegs.errmgmt);

		printf(">    pinIoRegs:\n");
		OPT32(">        clkcfg: ", bdd->nodes[idx].pinIoRegs.clkcfg, bdd->nodes[idx].pinIoRegs.has_clkcfg);
		REQ32(">        gpiooen:", bdd->nodes[idx].pinIoRegs.gpiooen);
		REQ32(">        gpioien:", bdd->nodes[idx].pinIoRegs.gpioien);
		REQ32(">        pinten: ", bdd->nodes[idx].pinIoRegs.pinten);
		REQ32(">        pintinv:", bdd->nodes[idx].pinIoRegs.pintinv);
		REQ32(">        pincfg: ", bdd->nodes[idx].pinIoRegs.pincfg);

		REQb(">    ignEeprom:      ", bdd->nodes[idx].ignEeprom);
		REQb(">    verifyNodeDescr:", bdd->nodes[idx].verifyNodeDescr);

		printf(">    nodeDescr:\n");
		REQ32(">        vendor: ", bdd->nodes[idx].nodeDescr.vendor);
		REQ32(">        product:", bdd->nodes[idx].nodeDescr.product);
		REQ32(">        version:", bdd->nodes[idx].nodeDescr.version);

		REQ32(">   downstreamBcastCnt:", bdd->nodes[idx].downstreamBcastCnt);
		printf(">   dwnstream: (cnt: %d)\n", bdd->nodes[idx].downstream_count);
		for (idx2 = 0; idx2 < bdd->nodes[idx].downstream_count; idx2++)
		{
			printf(">        downstream[%2ld]: %08X\n", idx2, bdd->nodes[idx].downstream[idx2]);
		} /* downstream */

		REQ32(">   upstreamBcastCnt:", bdd->nodes[idx].upstreamBcastCnt);
		printf(">   ustream: (cnt: %d)\n", bdd->nodes[idx].upstream_count);
		for (idx2 = 0; idx2 < bdd->nodes[idx].upstream_count; idx2++)
		{
			printf(">        upstream[%2ld]: %08X\n", idx2, bdd->nodes[idx].upstream[idx2]);
		} /* upstream */


	} /* nodes */

	printf("a2b_bddDump: END   ===========================================\n\n");
	fflush(stdout);

} /* a2b_bddDump */
#endif



/*!****************************************************************************
*
*  \b              a2b_bddDecode
*
*  Helper routine to decode an A2B BDD.
*
*  \param          [in]    bddData      pointer to binary BDD
*  \param          [in]    bddLen       length of bddData
*  \param          [out]   bddOut       decoded BDD
*
*  \pre            None
*
*  \post           None
*
*  \return         True = success, False = Failure
*
******************************************************************************/
a2b_Bool
a2b_bddDecode
(
	const a2b_Byte* bddData,
	a2b_UInt32      bddLen,
	bdd_Network* bddOut
)
{
	pb_istream_t    stream;

	if ((bddData == A2B_NULL) || (bddLen == 0))
	{
		return A2B_FALSE;
	}

	/* Cast away the constant-ness. The Nanopb API needs to be corrected
	 * since 'bddData' is never modified in a pb_istream_t (and hence
	 * can be treated as const).
	 */
	stream = pb_istream_from_buffer((a2b_Byte*)bddData, bddLen);

	if (!pb_decode(&stream, bdd_Network_fields, bddOut))
	{
		return A2B_FALSE;
	}

#ifdef A2B_DUMP_BDD
	a2b_bddDump(bddOut);
#endif

	return A2B_TRUE;

} /* a2b_bddDecode */


/*!****************************************************************************
*
*  \b              a2b_bddPalInit
*
*  Helper routine to initialize some ECB values within the PAL.
*
*  \param          [in]    ecb
*  \param          [in]    bdd      decoded BDD (e.g. from a2b_bddDecode)
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
void
a2b_bddPalInit
(
	A2B_ECB* ecb,
	const bdd_Network* bdd
)
{
	if (A2B_NULL != ecb)
	{
		if (0 != bdd->nodes_count)
		{
			ecb->baseEcb.masterNodeInfo.vendorId = (a2b_UInt8)
				bdd->nodes[0].nodeDescr.vendor;
			ecb->baseEcb.masterNodeInfo.productId = (a2b_UInt8)
				bdd->nodes[0].nodeDescr.product;
			ecb->baseEcb.masterNodeInfo.version = (a2b_UInt8)
				bdd->nodes[0].nodeDescr.version;
		}

		ecb->baseEcb.i2cMasterAddr = (a2b_UInt16)bdd->masterAddr;
	}

} /* a2b_bddPalInit */

/*! \addtogroup Network_Configuration
 *  @{
 */

 /*! \addtogroup Bus_Configuration  Bus Configuration
 * @{
 */

#ifdef ADI_SIGMASTUDIO_BCF

 /*!****************************************************************************
 *
 *  \b              adi_a2b_ComprBcfParse_bdd
 *
 *  Helper routine to decode an A2B BDD.
 *
 *  \param          [in]    pCmprBusDescription      pointer to compressed BCF
 *  \param          [in]    bdd_Graph       BDD destination array
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         True = success, False = Failure
 *
 ******************************************************************************/
a2b_Bool adi_a2b_ComprBcfParse_bdd(ADI_A2B_COMPR_BCD* pCmprBusDescription,
	bdd_Network* bdd_Graph, a2b_UInt8 nBusIndex)
{
	a2b_Bool bRet;
	/* Resetting the bdd network */
	memset(bdd_Graph, 0, sizeof(bdd_Network));

	/* Try to load the BDD structure */
	/* The Network Configuration is the binary exported by the Network
	* Configuration tool encoded in a Google Protobuf format.
	*/
	bRet = a2b_bddDecode(pCmprBusDescription->apNetworkconfig[nBusIndex]->pgA2bNetwork, pCmprBusDescription->apNetworkconfig[nBusIndex]->gA2bNetworkLen, bdd_Graph);

	return bRet;

}

/*!****************************************************************************
*
*  \b              a2b_bcfParse_bdd
*
*  Helper routine to Parse the SigmaStudio BCF file to generate the
*  fields of BDD structure
*
*  \param          [in]    pBusDescription 		Ptr to Bus Description Struct
*  \param          [in]    bdd     			 	decoded BDD (e.g. from a2b_bddDecode)
*  \param          [in]    nBusIndex      		Chain/Bus/network index (To identify each one uniquely)
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
void
a2b_bcfParse_bdd
(
	ADI_A2B_BCD* pBusDescription,
	bdd_Network* bdd_Graph,
	a2b_UInt8	  nBusIndex
)
{
	ADI_A2B_MASTER_SLAVE_CONFIG* pMasterSlaveChain;
	ADI_A2B_COMMON_CONFIG* pCommon;
	ADI_A2B_MASTER_NCD* pMstCfg;
	ADI_A2B_SLAVE_NCD* pSlvCfg;
	bdd_Node* pMstrNode;
	bdd_Node* pSlvNode;
	a2b_UInt8 nIndex1;
	a2b_UInt8 nNumMasternode = pBusDescription->nNumMasterNode;
	a2b_UInt8 nMstrNodeIdx = 0u;
	a2b_UInt8 nPLLCTL = 0u;
	a2b_Bool isAd2428ChipOnward = A2B_FALSE;

	/* Index should be lesser than the number of masters */
	if (nBusIndex >= nNumMasternode)
	{
		return;
	}
	/* Resetting the bdd network */
	memset(bdd_Graph, 0, sizeof(bdd_Network));

	/* Get master-slave chain pointer */
	pMasterSlaveChain = pBusDescription->apNetworkconfig[nBusIndex];

	/* Pointer to master configuration */
	pMstCfg = pMasterSlaveChain->pMasterConfig;

	/* Common configuration settings  */
	pCommon = &pMasterSlaveChain->sCommonSetting;

	bdd_Graph->nodes_count = (pb_size_t)((a2b_UInt32)pMasterSlaveChain->nNumSlaveNode + 1U);
	bdd_Graph->masterAddr = (a2b_UInt32)pCommon->nMasterI2CAddr;
	pMstrNode = &(bdd_Graph->nodes[nMstrNodeIdx]);

	nPLLCTL = pMstCfg->sRegSettings.nPLLCTL;
	adi_a2b_ParseMasterNCD(pMstrNode, pMstCfg, pCommon);

	A2B_SET_HAS(bdd_Graph->policy, has_common_SSSettings);

#ifdef ENABLE_AD242x_SUPPORT
	if (A2B_IS_AD242X_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product))
	{
		adi_a2b_ParseMasterNCD_242x(pMstrNode, pMstCfg, pCommon);
		adi_a2b_SetforAllReg_242x_master(pMstrNode);
	}

	if (A2B_IS_AD2428X_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product))
	{
		/* Let us assume that all the parts have common spread settings */
		bdd_Graph->policy.has_common_SSSettings = 1u;
	}
#endif

#ifdef ENABLE_AD243x_SUPPORT
	if ((A2B_IS_AD2430_8_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product)) ||
		(A2B_IS_AD243X_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product)))
	{
		/* Let us assume that all the parts have common spread settings */
		bdd_Graph->policy.has_common_SSSettings = 1u;

		//call ParseMAsterNCD_243x function
		adi_a2b_ParseMasterNCD_243x(pMstrNode, pMstCfg, pCommon);
		adi_a2b_SetforAllReg_243x_master(pMstrNode);
	}
#endif

	if (A2B_IS_AD241X_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product))
	{
		/* Set has field for all the registers */
		adi_a2b_SetforAllReg_241x_master(pMstrNode);
	}
#if A2B_CONFIGURE_ONLY_NON_DEFAULT
	/* Remove if non deafult */
	adi_a2b_CheckforDefault(pMstrNode);
#endif
	/* Loop over number of slaves */
	for (nIndex1 = 0u; nIndex1 < (a2b_UInt8)pMasterSlaveChain->nNumSlaveNode; nIndex1++)
	{
		pSlvCfg = pMasterSlaveChain->apSlaveConfig[nIndex1];
		pSlvNode = &(bdd_Graph->nodes[1u + nIndex1]);


		adi_a2b_ParseSlaveNCD(pSlvNode, pSlvCfg);
#ifdef ENABLE_AD242x_SUPPORT
		if (A2B_IS_AD242X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
		{
			adi_a2b_ParseSlaveNCD_242x(pSlvNode, pSlvCfg);
			adi_a2b_SetforAllReg_242x_slave(pSlvNode);
		}
#endif

#ifdef ENABLE_AD243x_SUPPORT
		if ((A2B_IS_AD243X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product)) ||
			(A2B_IS_AD2430_8_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product)))
		{
			//call ParseSlaveNCD_243x function
			adi_a2b_ParseSlaveNCD_243x(pSlvNode, pSlvCfg);
			adi_a2b_SetforAllReg_243x_slave(pSlvNode);
		}

		if (A2B_IS_AD2437_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
		{
			bdd_Graph->policy.has_eCableType = pBusDescription->sTargetProperties.eCableType;
			bdd_Graph->policy.eCableType = (bdd_A2bCableType)pBusDescription->sTargetProperties.eCableType;
#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
			if ((bdd_Graph->policy.eCableType == bdd_A2bCableType_RJ45) || (bdd_Graph->policy.eCableType == bdd_A2bCableType_XLR))
			{
				pSlvNode->nodeDescr.has_btwoStepDisc = 1u;
				pSlvNode->nodeDescr.btwoStepDisc = pMasterSlaveChain->apSlaveConfig[nIndex1]->sAuthSettings.bTwoStepDisc;
			}
#endif
		}
#endif
		/* Enable has for all */
		if (A2B_IS_AD241X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
		{
			adi_a2b_SetforAllReg_241x_slave(pSlvNode);
			bdd_Graph->policy.has_common_SSSettings = 0u;
		}

		/* Prune in case of default */
#if A2B_CONFIGURE_ONLY_NON_DEFAULT
		adi_a2b_CheckforDefault(pSlvNode);
#endif
		/* Clear registers for auto configuration */
		adi_a2b_CheckforAutoConfig(pSlvNode, pSlvCfg->bEnableAutoConfig);

		/* Update common spread settings */
		if (nPLLCTL != pSlvCfg->sRegSettings.nPLLCTL)
		{
			bdd_Graph->policy.has_common_SSSettings = 0u;
		}

		/* if any of the part is not AD2428x series or higher, then reset common spread */
		isAd2428ChipOnward = (a2b_Bool)(A2B_IS_AD2428X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product) ||
			A2B_IS_AD243X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product) ||
			A2B_IS_AD2430_8_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product));

		if ((isAd2428ChipOnward == A2B_FALSE))
		{
			bdd_Graph->policy.has_common_SSSettings = 0u;
		}

	}

	bdd_Graph->policy.discoveryMode = (bdd_DiscoveryMode)pBusDescription->sTargetProperties.eDiscoveryMode;
	bdd_Graph->policy.cfgPriority = bdd_CONFIG_PRIORITY_BDD;
	/*bdd_Graph->policy.cfgMethod = bdd_CONFIG_METHOD_BDD; AUTO-CONFIG*/
	bdd_Graph->policy.cfgMethod = bdd_CONFIG_METHOD_HYBRID;
	bdd_Graph->policy.cfgErrPolicy = bdd_CONFIG_ERR_POLICY_FATAL;

	bdd_Graph->policy.has_discoveryStartDelay = A2B_TRUE;
	bdd_Graph->policy.discoveryStartDelay = pBusDescription->sTargetProperties.nDiscoveryStartDelay;

	bdd_Graph->policy.has_bOverrideSelfDisc = pBusDescription->sTargetProperties.bOverrideBusSelfDisc;
	bdd_Graph->policy.bOverrideSelfDisc = pBusDescription->sTargetProperties.bOverrideBusSelfDisc;

	bdd_Graph->policy.bCrossTalkFixApply = pBusDescription->sTargetProperties.bCrossTalkFix;
	bdd_Graph->policy.has_bCrossTalkFixApply = pBusDescription->sTargetProperties.bCrossTalkFix;

	bdd_Graph->policy.has_nRediscWaitTime = pBusDescription->sTargetProperties.nRediscWaitTime;
	bdd_Graph->policy.nRediscWaitTime = pBusDescription->sTargetProperties.nRediscWaitTime;

	bdd_Graph->policy.has_bEnablePartialDisc = pBusDescription->sTargetProperties.bEnablePartialDisc;
	bdd_Graph->policy.bEnablePartialDisc = pBusDescription->sTargetProperties.bEnablePartialDisc;

	bdd_Graph->policy.eA2bDeviceInterface = (bdd_A2bDeviceInterface)pCommon->eA2bDeviceInterface;
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterNCD
*
*  Helper routine to Parse the Master config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*  \param          [in]    pMstCfg   Ptr to Common Config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseMasterNCD
(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg,
	ADI_A2B_COMMON_CONFIG* pCommon
)
{

	pMstrNode->verifyNodeDescr = pMstCfg->sAuthSettings.bTransceiverAuth;
	pMstrNode->ignEeprom = A2B_TRUE;
	pMstrNode->nodeType = bdd_NODE_TYPE_MASTER;

	pMstrNode->has_nodeSetting = (bool)A2B_TRUE;
	pMstrNode->nodeSetting.has_bLocalPwrd = (bool)A2B_TRUE;
	pMstrNode->nodeSetting.bLocalPwrd = (bdd_nodePowerMode)pMstCfg->bLocalPower;
	pMstrNode->nodeSetting.has_eHighPwrSwitchCfg = (bool)A2B_TRUE;
	pMstrNode->nodeSetting.eHighPwrSwitchCfg = (bdd_highPwrSwitchCfg)pMstCfg->nHighPwrSwitchCfg;

	/* Assign ID registers */
	pMstrNode->nodeDescr.product = (a2b_UInt32)pMstCfg->sAuthSettings.nProductID;
	pMstrNode->nodeDescr.vendor = (a2b_UInt32)pMstCfg->sAuthSettings.nVendorID;
	pMstrNode->nodeDescr.version = (a2b_UInt32)pMstCfg->sAuthSettings.nVersionID;

	/* Control registers */
	pMstrNode->ctrlRegs.bcdnslots = 0U;
	pMstrNode->ctrlRegs.ldnslots = 0U;
	pMstrNode->ctrlRegs.lupslots = 0U;

	pMstrNode->ctrlRegs.dnslots = (a2b_UInt32)pMstCfg->sConfigCtrlSettings.nPassDwnSlots;
	pMstrNode->ctrlRegs.upslots = (a2b_UInt32)pMstCfg->sConfigCtrlSettings.nPassUpSlots;
	pMstrNode->ctrlRegs.respcycs = (a2b_UInt32)pMstCfg->sConfigCtrlSettings.nRespCycle;
	pMstrNode->ctrlRegs.slotfmt = (a2b_UInt32)((pCommon->nDwnSlotSize | pCommon->nUpSlotSize | \
		(pCommon->bDwnstreamCompression << (a2b_UInt8)A2B_BITP_SLOTFMT_DNFP) | \
		(pCommon->bUpstreamCompression << (a2b_UInt8)A2B_BITP_SLOTFMT_UPFP)));
	pMstrNode->ctrlRegs.datctl = (a2b_UInt32)pMstCfg->sConfigCtrlSettings.nDatctrl;
	pMstrNode->ctrlRegs.swctl  = (a2b_UInt32)pMstCfg->sRegSettings.nSWCTL;

	pMstrNode->upstreamBcastCnt = pMstrNode->downstreamBcastCnt = pMstrNode->ctrlRegs.bcdnslots;
	pMstrNode->upstream_count = (pb_size_t)pMstrNode->ctrlRegs.upslots;
	pMstrNode->downstream_count = (pb_size_t)pMstrNode->ctrlRegs.dnslots;

	/* I2S & PDM registers */
	pMstrNode->i2cI2sRegs.i2ccfg = (a2b_UInt16)(pMstCfg->sConfigCtrlSettings.bI2CEarlyAck << \
		(a2b_UInt8)A2B_BITP_I2CCFG_EACK);
	pMstrNode->i2cI2sRegs.pllctl = (pMstCfg->sConfigCtrlSettings.nPLLTimeBase | pMstCfg->sConfigCtrlSettings.nBCLKRate);
	pMstrNode->i2cI2sRegs.i2sgcfg = (pMstCfg->sI2SSettings.bEarlySync << (a2b_UInt8)A2B_BITP_I2SGCFG_EARLY) | \
		(pMstCfg->sI2SSettings.nTDMMode | pMstCfg->sI2SSettings.nTDMChSize) | \
		(pMstCfg->sI2SSettings.nSyncMode | pMstCfg->sI2SSettings.nSyncPolarity << A2B_BITP_I2SGCFG_INV);

	pMstrNode->i2cI2sRegs.i2scfg = (pMstCfg->sI2SSettings.bRXInterleave << (a2b_UInt8)A2B_BITP_I2SCFG_RX2PINTL) | \
		(pMstCfg->sI2SSettings.bTXInterleave << (a2b_UInt8)A2B_BITP_I2SCFG_TX2PINTL) | \
		(pMstCfg->sI2SSettings.nBclkRxPolarity << (a2b_UInt8)A2B_BITP_I2SCFG_RXBCLKINV) | \
		(pMstCfg->sI2SSettings.nBclkTxPolarity << (a2b_UInt8)A2B_BITP_I2SCFG_TXBCLKINV);

	pMstrNode->i2cI2sRegs.i2srate = 0U;
	pMstrNode->i2cI2sRegs.i2stxoffset = (a2b_UInt16)(pMstCfg->sI2SSettings.nTxOffset | \
		(pMstCfg->sI2SSettings.bTriStateAfterTx << (a2b_UInt8)A2B_BITP_I2STXOFFSET_TSAFTER) | \
		(pMstCfg->sI2SSettings.bTriStateBeforeTx << A2B_BITP_I2STXOFFSET_TSBEFORE));
	pMstrNode->i2cI2sRegs.i2srxoffset = pMstCfg->sI2SSettings.nRxOffset;
	pMstrNode->i2cI2sRegs.has_syncoffset = A2B_TRUE;
	pMstrNode->i2cI2sRegs.syncoffset = 0U;
	pMstrNode->i2cI2sRegs.pdmctl = pMstCfg->sRegSettings.nPDMCTL;
	pMstrNode->i2cI2sRegs.errmgmt = pMstCfg->sRegSettings.nERRMGMT;

	/* INT registers */
	pMstrNode->has_intRegs = A2B_TRUE;
	pMstrNode->intRegs.becctl = pMstCfg->sRegSettings.nBECCTL;
	pMstrNode->ctrlRegs.control = pMstCfg->sRegSettings.nCONTROL;
	pMstrNode->intRegs.intmsk0 = (pMstCfg->sInterruptSettings.bReportHDCNTErr << (a2b_UInt8)A2B_BITP_INTPND0_HDCNTERR) | \
		(pMstCfg->sInterruptSettings.bReportDDErr << (a2b_UInt8)A2B_BITP_INTPND0_DDERR) | \
		(pMstCfg->sInterruptSettings.bReportCRCErr << (a2b_UInt8)A2B_BITP_INTPND0_CRCERR) | \
		(pMstCfg->sInterruptSettings.bReportDataParityErr << (a2b_UInt8)A2B_BITP_INTPND0_DPERR) | \
		(pMstCfg->sInterruptSettings.bReportPwrErr << (a2b_UInt8)A2B_BITP_INTPND0_PWRERR) | \
		(pMstCfg->sInterruptSettings.bReportErrCntOverFlow << (a2b_UInt8)A2B_BITP_INTPND0_BECOVF) | \
		(pMstCfg->sInterruptSettings.bReportSRFMissErr << (a2b_UInt8)A2B_BITP_INTPND0_SRFERR);

	pMstrNode->intRegs.intmsk1 = (pMstCfg->sInterruptSettings.bReportGPIO3 << (a2b_UInt8)A2B_BITP_INTPND1_IO3PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO4 << (a2b_UInt8)A2B_BITP_INTPND1_IO4PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO5 << (a2b_UInt8)A2B_BITP_INTPND1_IO5PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO6 << (a2b_UInt8)A2B_BITP_INTPND1_IO6PND);

	pMstrNode->intRegs.intmsk2 = (pMstCfg->sInterruptSettings.bSlaveIntReq << (a2b_UInt8)A2B_BITP_INTPND2_SLVIRQ) | \
		(pMstCfg->sInterruptSettings.bReportI2CErr << (a2b_UInt8)A2B_BITP_INTPND2_I2CERR) | \
		(pMstCfg->sInterruptSettings.bDiscComplete << (a2b_UInt8)A2B_BITP_INTPND2_DSCDONE) | \
		(pMstCfg->sInterruptSettings.bIntFrameCRCErr << (a2b_UInt8)A2B_BITP_INTPND2_ICRCERR);

	pMstrNode->pinIoRegs.pincfg = (pMstCfg->sGPIOSettings.bHighDriveStrength << (a2b_UInt8)A2B_BITP_PINCFG_DRVSTR);

	/* Parse pin multiplex - 3 and 4 */
	adi_a2b_ParseMasterPinMux34(pMstrNode, pMstCfg);
	/* Parse pin multiplex - 5 and 6 */
	adi_a2b_ParseMasterPinMux56(pMstrNode, pMstCfg);
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseSlaveNCD
*
*  Helper routine to Parse the Slave config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pSlvNode Ptr to Slave Node of the BDD struct
*  \param          [in]    pSlvCfg   Ptr to Slave Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseSlaveNCD
(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg
)
{
	a2b_UInt32 nIndex;
	pSlvNode->ignEeprom = (pSlvCfg->bEnableAutoConfig) ^ 0x1;/*AUTO-CONFIG*/

	pSlvNode->has_nodeSetting = (bool)A2B_TRUE;
	pSlvNode->nodeSetting.has_bLocalPwrd = (bool)A2B_TRUE;
	pSlvNode->nodeSetting.bLocalPwrd = (bdd_nodePowerMode)pSlvCfg->bLocalPower;
	pSlvNode->nodeSetting.has_eHighPwrSwitchCfg = (bool)A2B_TRUE;
	pSlvNode->nodeSetting.eHighPwrSwitchCfg = (bdd_highPwrSwitchCfg)pSlvCfg->nHighPwrSwitchCfg;
	pSlvNode->verifyNodeDescr = pSlvCfg->sAuthSettings.bTransceiverAuth;
	pSlvNode->nodeType = bdd_NODE_TYPE_SLAVE;

	/* Assign ID registers */
	pSlvNode->nodeDescr.product = (a2b_UInt32)pSlvCfg->sAuthSettings.nProductID;
	pSlvNode->nodeDescr.vendor = (a2b_UInt32)pSlvCfg->sAuthSettings.nVendorID;
	pSlvNode->nodeDescr.version = (a2b_UInt32)pSlvCfg->sAuthSettings.nVersionID;

	/* Custom Node Id Settings */
	pSlvNode->nodeDescr.oCustomNodeIdSettings.bCustomNodeIdAuth = pSlvCfg->sCustomNodeAuthSettings.bCustomNodeIdAuth;
	pSlvNode->nodeDescr.oCustomNodeIdSettings.has_bCustomNodeIdAuth = pSlvCfg->sCustomNodeAuthSettings.bCustomNodeIdAuth;

	if (pSlvNode->nodeDescr.oCustomNodeIdSettings.bCustomNodeIdAuth == A2B_ENABLED)
	{
		if (pSlvCfg->sCustomNodeAuthSettings.nReadFrmMemory == A2B_READ_FROM_MEM)
		{
			pSlvNode->nodeDescr.oCustomNodeIdSettings.bReadFrmMemory = A2B_ENABLED;
			pSlvNode->nodeDescr.oCustomNodeIdSettings.has_bReadFrmMemory = A2B_TRUE;
			pSlvNode->nodeDescr.oCustomNodeIdSettings.has_nDeviceAddr = A2B_TRUE;
			pSlvNode->nodeDescr.oCustomNodeIdSettings.has_nReadMemAddrWidth = A2B_TRUE;
			pSlvNode->nodeDescr.oCustomNodeIdSettings.has_nReadMemAddr = A2B_TRUE;
		}
		else
		{
			pSlvNode->nodeDescr.oCustomNodeIdSettings.bReadFrmMemory = A2B_DISABLED;
		}

		if (pSlvCfg->sCustomNodeAuthSettings.nReadFrmGPIO == A2B_READ_FROM_GPIO)
		{
			pSlvNode->nodeDescr.oCustomNodeIdSettings.bReadGpioPins = A2B_ENABLED;
			pSlvNode->nodeDescr.oCustomNodeIdSettings.has_bReadGpioPins = A2B_TRUE;
		}
		else
		{
			pSlvNode->nodeDescr.oCustomNodeIdSettings.bReadGpioPins = A2B_DISABLED;
		}

		if (pSlvCfg->sCustomNodeAuthSettings.nReadFrmCommCh == A2B_READ_FROM_COMM_CH)
		{
			pSlvNode->nodeDescr.oCustomNodeIdSettings.bReadFrmCommCh = A2B_ENABLED;
			pSlvNode->nodeDescr.oCustomNodeIdSettings.has_bReadFrmCommCh = A2B_TRUE;

		}
		else
		{
			pSlvNode->nodeDescr.oCustomNodeIdSettings.bReadFrmCommCh = A2B_DISABLED;
		}


		pSlvNode->nodeDescr.oCustomNodeIdSettings.has_nNodeId = A2B_TRUE;
		pSlvNode->nodeDescr.oCustomNodeIdSettings.has_nNodeIdLength = A2B_TRUE;
		pSlvNode->nodeDescr.oCustomNodeIdSettings.has_nTimeOut = A2B_TRUE;

		pSlvNode->nodeDescr.oCustomNodeIdSettings.nDeviceAddr = pSlvCfg->sCustomNodeAuthSettings.nDeviceAddr;
		(void)a2b_memcpy(&pSlvNode->nodeDescr.oCustomNodeIdSettings.nNodeId[0], &pSlvCfg->sCustomNodeAuthSettings.nNodeId[0], ADI_A2B_MAX_CUST_NODE_ID_LEN);
		pSlvNode->nodeDescr.oCustomNodeIdSettings.nNodeIdLength = pSlvCfg->sCustomNodeAuthSettings.nNodeIdLength;
		pSlvNode->nodeDescr.oCustomNodeIdSettings.nReadMemAddrWidth = pSlvCfg->sCustomNodeAuthSettings.nReadMemAddrWidth;
		pSlvNode->nodeDescr.oCustomNodeIdSettings.nReadMemAddr = pSlvCfg->sCustomNodeAuthSettings.nReadMemAddr;

		if (pSlvNode->nodeDescr.oCustomNodeIdSettings.has_bReadGpioPins)
		{
			pSlvNode->nodeDescr.oCustomNodeIdSettings.aGpio_count = ADI_A2B_MAX_GPIO_PINS;
			//copy GPIO settings
			for (nIndex = 0u; nIndex < ADI_A2B_MAX_GPIO_PINS; nIndex++)
			{
				pSlvNode->nodeDescr.oCustomNodeIdSettings.aGpio[nIndex] = pSlvCfg->sCustomNodeAuthSettings.aGpio[nIndex];
			}
		}

		pSlvNode->nodeDescr.oCustomNodeIdSettings.nTimeOut = pSlvCfg->sCustomNodeAuthSettings.nTimeOut;
		pSlvNode->nodeDescr.oCustomNodeIdSettings.nRetryCnt = pSlvCfg->sCustomNodeAuthSettings.nRetryCnt;
	}

	/* Control registers */
	pSlvNode->ctrlRegs.bcdnslots = pSlvCfg->sConfigCtrlSettings.nBroadCastSlots;
	pSlvNode->ctrlRegs.ldnslots = pSlvCfg->sConfigCtrlSettings.nLocalDwnSlotsConsume;
	pSlvNode->ctrlRegs.lupslots = pSlvCfg->sConfigCtrlSettings.nLocalUpSlotsContribute;

	pSlvNode->ctrlRegs.dnslots = (a2b_UInt32)pSlvCfg->sConfigCtrlSettings.nPassDwnSlots;
	pSlvNode->ctrlRegs.upslots = (a2b_UInt32)pSlvCfg->sConfigCtrlSettings.nPassUpSlots;
	pSlvNode->ctrlRegs.respcycs = (a2b_UInt32)pSlvCfg->sConfigCtrlSettings.nRespCycle;
	pSlvNode->ctrlRegs.control = (a2b_UInt32)pSlvCfg->sRegSettings.nCONTROL;
	pSlvNode->ctrlRegs.slotfmt = 0u;
    pSlvNode->ctrlRegs.swctl   = (a2b_UInt32)pSlvCfg->sRegSettings.nSWCTL;


	/* I2S & PDM registers */
	pSlvNode->i2cI2sRegs.i2ccfg = (a2b_UInt16)(pSlvCfg->sConfigCtrlSettings.nI2CFrequency) | \
		(pSlvCfg->sConfigCtrlSettings.nSuperFrameRate);
#ifdef ENABLE_AD232x_SUPPORT
	pSlvNode->i2cI2sRegs.i2ccfg |= pSlvCfg->sConfigCtrlSettings.bEnI2cFstModePlus << (a2b_UInt8)A2B_BITP_I2CCFG_FMPLUS;
#endif

	pSlvNode->i2cI2sRegs.pllctl = (pSlvCfg->sRegSettings.nPLLCTL);
	pSlvNode->i2cI2sRegs.i2sgcfg = (pSlvCfg->sI2SSettings.bEarlySync << (a2b_UInt8)A2B_BITP_I2SGCFG_EARLY) | \
		(pSlvCfg->sI2SSettings.nTDMMode | pSlvCfg->sI2SSettings.nTDMChSize) | \
		(pSlvCfg->sI2SSettings.nSyncMode | pSlvCfg->sI2SSettings.nSyncPolarity << A2B_BITP_I2SGCFG_INV);

	pSlvNode->i2cI2sRegs.i2scfg = (pSlvCfg->sI2SSettings.bRXInterleave << (a2b_UInt8)A2B_BITP_I2SCFG_RX2PINTL) | \
		(pSlvCfg->sI2SSettings.bTXInterleave << (a2b_UInt8)A2B_BITP_I2SCFG_TX2PINTL) | \
		(pSlvCfg->sI2SSettings.nBclkRxPolarity << (a2b_UInt8)A2B_BITP_I2SCFG_RXBCLKINV) | \
		(pSlvCfg->sI2SSettings.nBclkTxPolarity << (a2b_UInt8)A2B_BITP_I2SCFG_TXBCLKINV);


	pSlvNode->i2cI2sRegs.i2srate = (pSlvCfg->sI2SSettings.sI2SRateConfig.bReduce << (a2b_UInt8)A2B_BITP_I2SRATE_REDUCE) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.nSamplingRate);

	pSlvNode->i2cI2sRegs.i2stxoffset = 0u;
	pSlvNode->i2cI2sRegs.i2srxoffset = 0u;
	pSlvNode->i2cI2sRegs.syncoffset = pSlvCfg->sI2SSettings.nSyncOffset;
	pSlvNode->i2cI2sRegs.pdmctl |= (pSlvCfg->sPDMSettings.bHPFUse << (a2b_UInt8)(A2B_BITP_PDMCTL_HPFEN)) | \
		(pSlvCfg->sPDMSettings.nHPFCutOff) | \
		(pSlvCfg->sPDMSettings.nNumSlotsPDM0) | (pSlvCfg->sPDMSettings.nNumSlotsPDM1);
	pSlvNode->i2cI2sRegs.errmgmt = pSlvCfg->sRegSettings.nERRMGMT;

	/* INT registers */
	pSlvNode->has_intRegs = A2B_TRUE;
	pSlvNode->intRegs.becctl = pSlvCfg->sRegSettings.nBECCTL;
	pSlvNode->intRegs.intmsk0 = (pSlvCfg->sInterruptSettings.bReportHDCNTErr << (a2b_UInt8)A2B_BITP_INTPND0_HDCNTERR) | \
		(pSlvCfg->sInterruptSettings.bReportDDErr << (a2b_UInt8)A2B_BITP_INTPND0_DDERR) | \
		(pSlvCfg->sInterruptSettings.bReportCRCErr << (a2b_UInt8)A2B_BITP_INTPND0_CRCERR) | \
		(pSlvCfg->sInterruptSettings.bReportDataParityErr << (a2b_UInt8)A2B_BITP_INTPND0_DPERR) | \
		(pSlvCfg->sInterruptSettings.bReportPwrErr << (a2b_UInt8)A2B_BITP_INTPND0_PWRERR) | \
		(pSlvCfg->sInterruptSettings.bReportErrCntOverFlow << (a2b_UInt8)A2B_BITP_INTPND0_BECOVF) | \
		(pSlvCfg->sInterruptSettings.bReportSRFMissErr << (a2b_UInt8)A2B_BITP_INTPND0_SRFERR);

	pSlvNode->intRegs.intmsk1 = (pSlvCfg->sInterruptSettings.bReportGPIO0 << (a2b_UInt8)A2B_BITP_INTPND1_IO0PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO1 << (a2b_UInt8)A2B_BITP_INTPND1_IO1PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO2 << (a2b_UInt8)A2B_BITP_INTPND1_IO2PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO3 << (a2b_UInt8)A2B_BITP_INTPND1_IO3PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO4 << (a2b_UInt8)A2B_BITP_INTPND1_IO4PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO5 << (a2b_UInt8)A2B_BITP_INTPND1_IO5PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO6 << (a2b_UInt8)A2B_BITP_INTPND1_IO6PND);

	pSlvNode->intRegs.intmsk2 = 0u;

	pSlvNode->pinIoRegs.clkcfg = (pSlvCfg->sI2SSettings.nCodecClkRate << A2B_BITP_CLKCFG_CCLKRATE);

	/* Parsing Pin Mux */
	adi_a2b_ParseSlavePinMux012(pSlvNode, pSlvCfg);
	adi_a2b_ParseSlavePinMux34(pSlvNode, pSlvCfg);
	adi_a2b_ParseSlavePinMux56(pSlvNode, pSlvCfg);

	pSlvNode->pinIoRegs.pincfg = (pSlvCfg->sGPIOSettings.bHighDriveStrength << (a2b_UInt8)A2B_BITP_PINCFG_DRVSTR);

}


/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterPinMux34
*
*  Helper routine to Parse the Master Pin Mux 3&4 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParseMasterPinMux34(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg
)
{
	/* Pin multiplex for GPIO 3*/
	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO3PinUsage)
	{
	case A2B_GPIO_3_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO3IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO3Interrupt << A2B_BITP_PINTEN_IO3IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO3IntPolarity << A2B_BITP_PINTINV_IO3INV);
		break;
	case A2B_GPIO_3_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO3OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO3Val << A2B_BITP_GPIODAT_IO3DAT);
		break;
	case A2B_GPIO_3_AS_DTX0:
		pMstrNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_TX0EN);
		break;
		/*case A2B_GPIO_3_DISABLE:
			break;*/
	default:
		/* Do Nothing */
		break;
	}

	/* Pin multiplex for GPIO 4*/
	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO4PinUsage)
	{
	case A2B_GPIO_4_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO4IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO4Interrupt << A2B_BITP_PINTEN_IO4IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO4IntPolarity << A2B_BITP_PINTINV_IO4INV);
		break;
	case A2B_GPIO_4_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO4OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO4Val << A2B_BITP_GPIODAT_IO4DAT);
		break;
	case A2B_GPIO_4_AS_DTX1:
		pMstrNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_TX1EN);
		break;
		/*case A2B_GPIO_4_DISABLE:
		   break;*/
	default:
		/* Do Nothing */
		break;
	}

}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterPinMux56
*
*  Helper routine to Parse the Master Pin Mux 5&6 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParseMasterPinMux56(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg)
{

	/* Pin multiplex for GPIO 5*/
	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO5PinUsage)
	{
	case A2B_GPIO_5_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO5IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO5Interrupt << A2B_BITP_PINTEN_IO5IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO5IntPolarity << A2B_BITP_PINTINV_IO5INV);
		break;
	case A2B_GPIO_5_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO5OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO5Val << A2B_BITP_GPIODAT_IO5DAT);
		break;
	case A2B_GPIO_5_AS_DRX0:
		pMstrNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_RX0EN);
		break;
		/*case A2B_GPIO_5_DISABLE:
			break;*/
	default:
		/* Do Nothing */
		break;

	}

	/* Pin multiplex for GPIO 6*/
	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO6PinUsage)
	{
	case A2B_GPIO_6_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO6IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO6Interrupt << A2B_BITP_PINTEN_IO6IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO6IntPolarity << A2B_BITP_PINTINV_IO6INV);
		break;
	case A2B_GPIO_6_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO6OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO6Val << A2B_BITP_GPIODAT_IO6DAT);
		break;
	case A2B_GPIO_6_AS_DRX1:
		pMstrNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_RX1EN);
		break;
		/*case A2B_GPIO_6_DISABLE:
			break;*/
	default:
		/* Do Nothing */
		break;

	}
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseSlavePinMux012
*
*  Helper routine to Parse the Slave Pin Mux 0,1&2 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pSlvNode Ptr to Slave Node of the BDD struct
*  \param          [in]    pSlvCfg   Ptr to Slave Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseSlavePinMux012(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg)
{

	/* Pin multiplex for GPIO 0*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO0PinUsage)
	{
	case A2B_GPIO_0_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO0IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO0Interrupt << A2B_BITP_PINTEN_IO0IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO0IntPolarity << A2B_BITP_PINTINV_IO0INV);
		break;
	case A2B_GPIO_0_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO0OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO0Val << A2B_BITP_GPIODAT_IO0DAT);
		break;
		/*case A2B_GPIO_0_DISABLE:
			break; */
	default:
		/* Do Nothing */
		break;

	}

	/* Pin multiplex for GPIO 1*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO1PinUsage)
	{
	case A2B_GPIO_1_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO1IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO1Interrupt << A2B_BITP_PINTEN_IO1IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO1IntPolarity << A2B_BITP_PINTINV_IO1INV);
		break;
	case A2B_GPIO_1_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO1OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO1Val << A2B_BITP_GPIODAT_IO1DAT);
		break;
	case A2B_GPIO_1_AS_CLKOUT1:
#ifdef ENABLE_AD242x_SUPPORT
		if (A2B_IS_AD242X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
		{
			pSlvNode->pinIoRegs.clk1cfg |= (1u << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1EN);
		}
#endif

		break;

		/* case A2B_GPIO_1_DISABLE:
			break; */
	default:
		/* Do Nothing */
		break;
	}

	/* Pin multiplex for GPIO 2*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO2PinUsage)
	{
	case A2B_GPIO_2_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO2IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO2Interrupt << A2B_BITP_PINTEN_IO2IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO2IntPolarity << A2B_BITP_PINTINV_IO2INV);
		break;
	case A2B_GPIO_2_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO2OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO2Val << A2B_BITP_GPIODAT_IO2DAT);
		break;
	case A2B_GPIO_2_AS_CLKOUT2:
#if defined(ENABLE_AD242x_SUPPORT) || defined(ENABLE_AD243X_SUPPORT)
		if (A2B_IS_AD242X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
		{
			pSlvNode->pinIoRegs.clk2cfg |= (1u << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2EN);
		}
		else
#endif
		{
			pSlvNode->pinIoRegs.clkcfg |= (1u << (a2b_UInt8)A2B_BITP_CLKCFG_CCLKEN);
		}
		break;
		/* case A2B_GPIO_2_DISABLE:
			break; */
	default:
		/* Do Nothing */
		break;

	}
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseSlavePinMux34
*
*  Helper routine to Parse the Slave Pin Mux 3&4 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pSlvNode Ptr to Slave Node of the BDD struct
*  \param          [in]    pSlvCfg   Ptr to Slave Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParseSlavePinMux34(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg)
{
	/* Pin multiplex for GPIO 3*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO3PinUsage)
	{
	case A2B_GPIO_3_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO3IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO3Interrupt << A2B_BITP_PINTEN_IO3IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO3IntPolarity << A2B_BITP_PINTINV_IO3INV);
		break;
	case A2B_GPIO_3_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO3OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO3Val << A2B_BITP_GPIODAT_IO3DAT);
		break;
	case A2B_GPIO_3_AS_DTX0:
		pSlvNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_TX0EN);
		break;
		/*    case A2B_GPIO_3_DISABLE:
				break; */
	default:
		/* Do Nothing */
		break;

	}

	/* Pin multiplex for GPIO 4*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO4PinUsage)
	{
	case A2B_GPIO_4_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO4IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO4Interrupt << A2B_BITP_PINTEN_IO4IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO4IntPolarity << A2B_BITP_PINTINV_IO4INV);
		break;
	case A2B_GPIO_4_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO4OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO4Val << A2B_BITP_GPIODAT_IO4DAT);
		break;
	case A2B_GPIO_4_AS_DTX1:
		pSlvNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_TX1EN);
		break;
		/* case A2B_GPIO_4_DISABLE:
		   break; */
	default:
		/* Do Nothing */
		break;
	}


}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseSlavePinMux56
*
*  Helper routine to Parse the Slave Pin Mux 5&6 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pSlvNode Ptr to Slave Node of the BDD struct
*  \param          [in]    pSlvCfg   Ptr to Slave Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseSlavePinMux56(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg)
{


	/* Pin multiplex for GPIO 5*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO5PinUsage)
	{
	case A2B_GPIO_5_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO5IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO5Interrupt << A2B_BITP_PINTEN_IO5IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO5IntPolarity << A2B_BITP_PINTINV_IO5INV);
		break;
	case A2B_GPIO_5_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO5OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO5Val << A2B_BITP_GPIODAT_IO5DAT);
		break;
	case A2B_GPIO_5_AS_DRX0:
		pSlvNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_RX0EN);
		break;
	case A2B_GPIO_5_AS_PDM0:
		pSlvNode->i2cI2sRegs.pdmctl |= (1u << A2B_BITP_PDMCTL_PDM0EN);
		break;
		/* case A2B_GPIO_5_DISABLE:
			break; */
	default:
		/* Do Nothing */
		break;

	}

	/* Pin multiplex for GPIO 6*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO6PinUsage)
	{
	case A2B_GPIO_6_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO6IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO6Interrupt << A2B_BITP_PINTEN_IO6IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO6IntPolarity << A2B_BITP_PINTINV_IO6INV);
		break;
	case A2B_GPIO_6_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO6OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO6Val << A2B_BITP_GPIODAT_IO6DAT);
		break;
	case A2B_GPIO_6_AS_DRX1:
		pSlvNode->i2cI2sRegs.i2scfg |= (1u << A2B_BITP_I2SCFG_RX1EN);
		break;
	case A2B_GPIO_6_AS_PDM1:
		pSlvNode->i2cI2sRegs.pdmctl |= (1u << A2B_BITP_PDMCTL_PDM1EN);
		break;
		/* case A2B_GPIO_6_DISABLE:
			break; */
	default:
		/* Do Nothing */
		break;

	}
}

#ifdef ENABLE_AD242x_SUPPORT
/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterNCD_242x
*
*  Helper routine to Parse the 242x Master config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*  \param          [in]    pMstCfg   Ptr to Common Config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseMasterNCD_242x
(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg,
	ADI_A2B_COMMON_CONFIG* pCommon
)
{
	uint32_t* psGPIODMask[8];
	A2B_GPIOD_PIN_CONFIG asGPIODConfig[8];
	a2b_UInt8 nGPIOIndex;

	/* GPIOD  */
	pMstrNode->has_gpioDist = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod0msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod1msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod2msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod3msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod4msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod5msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod6msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiod7msk = A2B_TRUE;
	pMstrNode->gpioDist.has_gpioden = A2B_TRUE;
	pMstrNode->gpioDist.has_gpiodinv = A2B_TRUE;

	pMstrNode->gpioDist.gpiodinv = (pMstCfg->sGPIODSettings.sGPIOD1Config.bGPIOSignalInv << 1u) | \
		(pMstCfg->sGPIODSettings.sGPIOD2Config.bGPIOSignalInv << 2u) | \
		(pMstCfg->sGPIODSettings.sGPIOD3Config.bGPIOSignalInv << 3u) | \
		(pMstCfg->sGPIODSettings.sGPIOD4Config.bGPIOSignalInv << 4u) | \
		(pMstCfg->sGPIODSettings.sGPIOD5Config.bGPIOSignalInv << 5u) | \
		(pMstCfg->sGPIODSettings.sGPIOD6Config.bGPIOSignalInv << 6u) | \
		(pMstCfg->sGPIODSettings.sGPIOD7Config.bGPIOSignalInv << 7u);

	pMstrNode->gpioDist.gpioden = (pMstCfg->sGPIODSettings.sGPIOD1Config.bGPIODistance << 1u) | \
		(pMstCfg->sGPIODSettings.sGPIOD2Config.bGPIODistance << 2u) | \
		(pMstCfg->sGPIODSettings.sGPIOD3Config.bGPIODistance << 3u) | \
		(pMstCfg->sGPIODSettings.sGPIOD4Config.bGPIODistance << 4u) | \
		(pMstCfg->sGPIODSettings.sGPIOD5Config.bGPIODistance << 5u) | \
		(pMstCfg->sGPIODSettings.sGPIOD6Config.bGPIODistance << 6u) | \
		(pMstCfg->sGPIODSettings.sGPIOD7Config.bGPIODistance << 7u);

	/* GPIOD Mask update */
	psGPIODMask[0] = &pMstrNode->gpioDist.gpiod0msk;
	psGPIODMask[1] = &pMstrNode->gpioDist.gpiod1msk;
	psGPIODMask[2] = &pMstrNode->gpioDist.gpiod2msk;
	psGPIODMask[3] = &pMstrNode->gpioDist.gpiod3msk;
	psGPIODMask[4] = &pMstrNode->gpioDist.gpiod4msk;
	psGPIODMask[5] = &pMstrNode->gpioDist.gpiod5msk;
	psGPIODMask[6] = &pMstrNode->gpioDist.gpiod6msk;
	psGPIODMask[7] = &pMstrNode->gpioDist.gpiod7msk;

	asGPIODConfig[1] = pMstCfg->sGPIODSettings.sGPIOD1Config;
	asGPIODConfig[2] = pMstCfg->sGPIODSettings.sGPIOD2Config;
	asGPIODConfig[3] = pMstCfg->sGPIODSettings.sGPIOD3Config;
	asGPIODConfig[4] = pMstCfg->sGPIODSettings.sGPIOD4Config;
	asGPIODConfig[5] = pMstCfg->sGPIODSettings.sGPIOD5Config;
	asGPIODConfig[6] = pMstCfg->sGPIODSettings.sGPIOD6Config;
	asGPIODConfig[7] = pMstCfg->sGPIODSettings.sGPIOD7Config;

	for (nGPIOIndex = 1u; nGPIOIndex < 8u; nGPIOIndex++)
	{


		*(psGPIODMask[nGPIOIndex]) = (a2b_UInt16)((asGPIODConfig[nGPIOIndex].abBusPortMask[0] << 0u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[1] << 1u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[2] << 2u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[3] << 3u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[4] << 4u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[5] << 5u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[6] << 6u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[7] << 7u));
	}

	/* SWCTL register */
	pMstrNode->ctrlRegs.has_swctl = (pMstCfg->sRegSettings.nSWCTL != 0) ? A2B_TRUE : A2B_FALSE;
	pMstrNode->ctrlRegs.swctl = pMstCfg->sRegSettings.nSWCTL;

	/* I2S & PDM registers */
	pMstrNode->i2cI2sRegs.i2srrate = pCommon->bEnableReduceRate << (a2b_UInt8)(A2B_BITP_I2SRRATE_RBUS) | \
		(pCommon->nSysRateDivFactor << (a2b_UInt8)A2B_BITP_I2SRRATE_RRDIV);

	pMstrNode->i2cI2sRegs.i2srrctl = (pMstCfg->sI2SSettings.sI2SRateConfig.bRRStrobe << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENSTRB) | \
		(pMstCfg->sI2SSettings.sI2SRateConfig.bRRStrobeDirection << (a2b_UInt8)A2B_BITP_I2SRRCTL_STRBDIR) | \
		(pMstCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraBit << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENXBIT) | \
		(pMstCfg->sI2SSettings.sI2SRateConfig.bRRValidBitLSB << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENVLSB) | \
		(pMstCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraCh << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENCHAN);

	/* INT registers */
	pMstrNode->intRegs.intmsk1 = (pMstCfg->sInterruptSettings.bReportGPIO1 << (a2b_UInt8)A2B_BITP_INTPND1_IO1PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO2 << (a2b_UInt8)A2B_BITP_INTPND1_IO2PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO3 << (a2b_UInt8)A2B_BITP_INTPND1_IO3PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO4 << (a2b_UInt8)A2B_BITP_INTPND1_IO4PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO5 << (a2b_UInt8)A2B_BITP_INTPND1_IO5PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO6 << (a2b_UInt8)A2B_BITP_INTPND1_IO6PND) | \
		(pMstCfg->sInterruptSettings.bReportGPIO7 << (a2b_UInt8)A2B_BITP_INTPND1_IO7PND);
	pMstrNode->pinIoRegs.pincfg = (pMstCfg->sGPIOSettings.bHighDriveStrength << (a2b_UInt8)A2B_BITP_PINCFG_DRVSTR) | \
		(pMstCfg->sGPIOSettings.bIRQInv << (a2b_UInt8)A2B_BITP_PINCFG_IRQINV) | \
		(pMstCfg->sGPIOSettings.bIRQTriState << (a2b_UInt8)A2B_BITP_PINCFG_IRQTS);

#ifdef ENABLE_AD232x_SUPPORT
			pMstrNode->pinIoRegs.pincfg |= (pMstCfg->sGPIOSettings.bI2CDriveStrength << (a2b_UInt8)A2B_BITP_PINCFG_I2CDRVSTR);
#endif
	/*ClockCFg1 & CFG2*/
	pMstrNode->pinIoRegs.clk1cfg |= (pMstCfg->sClkOutSettings.bClk1Div << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1DIV) | \
		(pMstCfg->sClkOutSettings.bClk1PreDiv << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1PDIV) | \
		(pMstCfg->sClkOutSettings.bClk1Inv << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1INV);

	pMstrNode->pinIoRegs.clk2cfg |= (pMstCfg->sClkOutSettings.bClk2Div << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2DIV) | \
		(pMstCfg->sClkOutSettings.bClk2PreDiv << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2PDIV) | \
		(pMstCfg->sClkOutSettings.bClk2Inv << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2INV);
	if (A2B_IS_AD242X_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product))
	{/*This check is needed because this register field is present only for Chiron,
		and this function is called to populate demeter registers too*/
		pMstrNode->i2cI2sRegs.i2sgcfg |= (pMstCfg->sI2SSettings.bSerialRxOnDTx1 << (a2b_UInt8)A2B_BITP_I2SGCFG_RXONDTX1);
	}


	pMstrNode->i2cI2sRegs.pdmctl2 = pMstCfg->sRegSettings.nPDMCTL2;

	pMstrNode->i2cI2sRegs.pllctl = pMstCfg->sRegSettings.nPLLCTL;

	if (A2B_IS_AD242X_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product))
	{
		/*This check is needed because these registers are present only for Chiron,
				and this function is called to populate demeter registers too*/

				/* Tuning Registers */
		pMstrNode->has_tuningRegs = A2B_TRUE;
		pMstrNode->tuningRegs.txactl = pMstCfg->sRegSettings.nTXACTL;
		pMstrNode->tuningRegs.txbctl = pMstCfg->sRegSettings.nTXBCTL;
	}

	/* Pin Multiplex  */
	adi_a2b_ParseMasterPinMux12(pMstrNode, pMstCfg);
	if (!(A2B_IS_AD2430_8_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product)))
	{
		adi_a2b_ParseMasterPinMux7(pMstrNode, pMstCfg);
	}
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseSlaveNCD_242x
*
*  Helper routine to Parse the 242x Slave config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pSlvNode Ptr to Slave Node of the BDD struct
*  \param          [in]    pSlvCfg   Ptr to Slave Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseSlaveNCD_242x
(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg
)
{
	uint32_t* psGPIODMask[8];
	A2B_GPIOD_PIN_CONFIG asGPIODConfig[8];
	a2b_UInt8 nGPIOIndex;

	pSlvNode->ctrlRegs.lupslots = pSlvCfg->sConfigCtrlSettings.nLocalUpSlotsContribute;
	pSlvNode->ctrlRegs.dnslots = pSlvCfg->sConfigCtrlSettings.nPassDwnSlots;
	pSlvNode->ctrlRegs.upslots = pSlvCfg->sConfigCtrlSettings.nPassUpSlots;
	pSlvNode->has_slotEnh = A2B_TRUE;

	if (pSlvCfg->sConfigCtrlSettings.bUseDwnslotConsumeMasks == 1u)
	{
		pSlvNode->ctrlRegs.has_ldnslots = A2B_TRUE;
		pSlvNode->ctrlRegs.ldnslots = ((a2b_UInt8)A2B_BITM_LDNSLOTS_DNMASKEN | pSlvCfg->sConfigCtrlSettings.nSlotsforDwnstrmContribute);
		pSlvNode->slotEnh.dnmask0 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[0] << A2B_BITP_DNMASK0_RXDNSLOT00) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[1] << A2B_BITP_DNMASK0_RXDNSLOT01) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[2] << A2B_BITP_DNMASK0_RXDNSLOT02) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[3] << A2B_BITP_DNMASK0_RXDNSLOT03) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[4] << A2B_BITP_DNMASK0_RXDNSLOT04) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[5] << A2B_BITP_DNMASK0_RXDNSLOT05) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[6] << A2B_BITP_DNMASK0_RXDNSLOT06) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[7] << A2B_BITP_DNMASK0_RXDNSLOT07);

		pSlvNode->slotEnh.dnmask1 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[8] << A2B_BITP_DNMASK0_RXDNSLOT00) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[9] << A2B_BITP_DNMASK0_RXDNSLOT01) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[10] << A2B_BITP_DNMASK0_RXDNSLOT02) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[11] << A2B_BITP_DNMASK0_RXDNSLOT03) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[12] << A2B_BITP_DNMASK0_RXDNSLOT04) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[13] << A2B_BITP_DNMASK0_RXDNSLOT05) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[14] << A2B_BITP_DNMASK0_RXDNSLOT06) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[15] << A2B_BITP_DNMASK0_RXDNSLOT07);

		pSlvNode->slotEnh.dnmask2 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[16] << A2B_BITP_DNMASK0_RXDNSLOT00) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[17] << A2B_BITP_DNMASK0_RXDNSLOT01) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[18] << A2B_BITP_DNMASK0_RXDNSLOT02) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[19] << A2B_BITP_DNMASK0_RXDNSLOT03) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[20] << A2B_BITP_DNMASK0_RXDNSLOT04) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[21] << A2B_BITP_DNMASK0_RXDNSLOT05) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[22] << A2B_BITP_DNMASK0_RXDNSLOT06) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[23] << A2B_BITP_DNMASK0_RXDNSLOT07);

		pSlvNode->slotEnh.dnmask3 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[24] << A2B_BITP_DNMASK0_RXDNSLOT00) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[25] << A2B_BITP_DNMASK0_RXDNSLOT01) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[26] << A2B_BITP_DNMASK0_RXDNSLOT02) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[27] << A2B_BITP_DNMASK0_RXDNSLOT03) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[28] << A2B_BITP_DNMASK0_RXDNSLOT04) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[29] << A2B_BITP_DNMASK0_RXDNSLOT05) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[30] << A2B_BITP_DNMASK0_RXDNSLOT06) | \
			((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[31] << A2B_BITP_DNMASK0_RXDNSLOT07);
	}
	else
	{
		pSlvNode->ctrlRegs.ldnslots = pSlvCfg->sConfigCtrlSettings.nLocalDwnSlotsConsume;
		pSlvNode->slotEnh.dnmask0 = 0u;
		pSlvNode->slotEnh.dnmask1 = 0u;
		pSlvNode->slotEnh.dnmask2 = 0u;
		pSlvNode->slotEnh.dnmask3 = 0u;
	}


	pSlvNode->slotEnh.dnoffset = pSlvCfg->sConfigCtrlSettings.nOffsetDwnstrmContribute;
	pSlvNode->slotEnh.upoffset = pSlvCfg->sConfigCtrlSettings.nOffsetUpstrmContribute;

	pSlvNode->slotEnh.upmask0 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[0] << A2B_BITP_UPMASK0_RXUPSLOT00) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[1] << A2B_BITP_UPMASK0_RXUPSLOT01) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[2] << A2B_BITP_UPMASK0_RXUPSLOT02) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[3] << A2B_BITP_UPMASK0_RXUPSLOT03) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[4] << A2B_BITP_UPMASK0_RXUPSLOT04) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[5] << A2B_BITP_UPMASK0_RXUPSLOT05) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[6] << A2B_BITP_UPMASK0_RXUPSLOT06) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[7] << A2B_BITP_UPMASK0_RXUPSLOT07);

	pSlvNode->slotEnh.upmask1 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[8] << A2B_BITP_UPMASK0_RXUPSLOT00) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[9] << A2B_BITP_UPMASK0_RXUPSLOT01) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[10] << A2B_BITP_UPMASK0_RXUPSLOT02) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[11] << A2B_BITP_UPMASK0_RXUPSLOT03) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[12] << A2B_BITP_UPMASK0_RXUPSLOT04) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[13] << A2B_BITP_UPMASK0_RXUPSLOT05) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[14] << A2B_BITP_UPMASK0_RXUPSLOT06) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[15] << A2B_BITP_UPMASK0_RXUPSLOT07);

	pSlvNode->slotEnh.upmask2 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[16] << A2B_BITP_UPMASK0_RXUPSLOT00) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[17] << A2B_BITP_UPMASK0_RXUPSLOT01) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[18] << A2B_BITP_UPMASK0_RXUPSLOT02) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[19] << A2B_BITP_UPMASK0_RXUPSLOT03) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[20] << A2B_BITP_UPMASK0_RXUPSLOT04) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[21] << A2B_BITP_UPMASK0_RXUPSLOT05) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[22] << A2B_BITP_UPMASK0_RXUPSLOT06) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[23] << A2B_BITP_UPMASK0_RXUPSLOT07);

	pSlvNode->slotEnh.upmask3 = ((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[24] << A2B_BITP_UPMASK0_RXUPSLOT00) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[25] << A2B_BITP_UPMASK0_RXUPSLOT01) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[26] << A2B_BITP_UPMASK0_RXUPSLOT02) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[27] << A2B_BITP_UPMASK0_RXUPSLOT03) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[28] << A2B_BITP_UPMASK0_RXUPSLOT04) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[29] << A2B_BITP_UPMASK0_RXUPSLOT05) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[30] << A2B_BITP_UPMASK0_RXUPSLOT06) | \
		((a2b_UInt8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[31] << A2B_BITP_UPMASK0_RXUPSLOT07);

	pSlvNode->i2cI2sRegs.i2srrctl = (pSlvCfg->sI2SSettings.sI2SRateConfig.bRRStrobe << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENSTRB) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.bRRStrobeDirection << (a2b_UInt8)A2B_BITP_I2SRRCTL_STRBDIR) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraBit << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENXBIT) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.bRRValidBitLSB << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENVLSB) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraCh << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENCHAN);


	pSlvNode->i2cI2sRegs.i2srrsoffs = (pSlvCfg->sI2SSettings.sI2SRateConfig.nRROffset << (a2b_UInt8)A2B_BITP_I2SRRSOFFS_RRSOFFSET);

	pSlvNode->i2cI2sRegs.i2srate = (pSlvCfg->sI2SSettings.sI2SRateConfig.bReduce << (a2b_UInt8)A2B_BITP_I2SRATE_REDUCE) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.nSamplingRate) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.nRBCLKRate << (a2b_UInt8)A2B_BITP_I2SRATE_BLCKRATE) | \
		(pSlvCfg->sI2SSettings.sI2SRateConfig.bShareBusSlot << (a2b_UInt8)A2B_BITP_I2SRATE_SHARE);

	pSlvNode->i2cI2sRegs.pdmctl |= (pSlvCfg->sPDMSettings.bHPFUse << (a2b_UInt8)(A2B_BITP_PDMCTL_HPFEN)) | \
		(pSlvCfg->sPDMSettings.nPDMRate << (a2b_UInt8)(A2B_BITP_PDMCTL_PDMRATE) |
			pSlvCfg->sPDMSettings.nNumSlotsPDM0 | pSlvCfg->sPDMSettings.nNumSlotsPDM1);


	pSlvNode->intRegs.intmsk0 = (pSlvCfg->sInterruptSettings.bReportHDCNTErr << (a2b_UInt8)A2B_BITP_INTPND0_HDCNTERR) | \
		(pSlvCfg->sInterruptSettings.bReportDDErr << (a2b_UInt8)A2B_BITP_INTPND0_DDERR) | \
		(pSlvCfg->sInterruptSettings.bReportCRCErr << (a2b_UInt8)A2B_BITP_INTPND0_CRCERR) | \
		(pSlvCfg->sInterruptSettings.bReportDataParityErr << (a2b_UInt8)A2B_BITP_INTPND0_DPERR) | \
		(pSlvCfg->sInterruptSettings.bReportPwrErr << (a2b_UInt8)A2B_BITP_INTPND0_PWRERR) | \
		(pSlvCfg->sInterruptSettings.bReportErrCntOverFlow << (a2b_UInt8)A2B_BITP_INTPND0_BECOVF) | \
		(pSlvCfg->sInterruptSettings.bReportSRFMissErr << (a2b_UInt8)A2B_BITP_INTPND0_SRFERR) | \
		(pSlvCfg->sInterruptSettings.bReportSRFCrcErr << (a2b_UInt8)A2B_BITP_INTPND0_SRFCRCERR);

	pSlvNode->intRegs.intmsk1 = (pSlvCfg->sInterruptSettings.bReportGPIO0 << (a2b_UInt8)A2B_BITP_INTPND1_IO0PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO1 << (a2b_UInt8)A2B_BITP_INTPND1_IO1PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO2 << (a2b_UInt8)A2B_BITP_INTPND1_IO2PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO3 << (a2b_UInt8)A2B_BITP_INTPND1_IO3PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO4 << (a2b_UInt8)A2B_BITP_INTPND1_IO4PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO5 << (a2b_UInt8)A2B_BITP_INTPND1_IO5PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO6 << (a2b_UInt8)A2B_BITP_INTPND1_IO6PND) | \
		(pSlvCfg->sInterruptSettings.bReportGPIO7 << (a2b_UInt8)A2B_BITP_INTPND1_IO7PND);

	pSlvNode->pinIoRegs.pincfg = (pSlvCfg->sGPIOSettings.bHighDriveStrength << (a2b_UInt8)A2B_BITP_PINCFG_DRVSTR) | \
		(pSlvCfg->sGPIOSettings.bIRQInv << (a2b_UInt8)A2B_BITP_PINCFG_IRQINV) | \
		(pSlvCfg->sGPIOSettings.bIRQTriState << (a2b_UInt8)A2B_BITP_PINCFG_IRQTS);
#ifdef ENABLE_AD232x_SUPPORT
	pSlvNode->pinIoRegs.pincfg = (pSlvCfg->sGPIOSettings.bI2CDriveStrength << (a2b_UInt8)A2B_BITP_PINCFG_I2CDRVSTR);
#endif

	if (A2B_IS_AD2430_8_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
	{
		/* Parsing Pin Mux */
		adi_a2b_ParseSlavePinMux7(pSlvNode, pSlvCfg);
	}


	/* GPIOD  */

	pSlvNode->has_gpioDist = A2B_TRUE;
	pSlvNode->gpioDist.gpiodinv = (pSlvCfg->sGPIODSettings.sGPIOD0Config.bGPIOSignalInv << 0u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD1Config.bGPIOSignalInv << 1u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD2Config.bGPIOSignalInv << 2u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD3Config.bGPIOSignalInv << 3u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD4Config.bGPIOSignalInv << 4u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD5Config.bGPIOSignalInv << 5u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD6Config.bGPIOSignalInv << 6u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD7Config.bGPIOSignalInv << 7u);

	pSlvNode->gpioDist.gpioden = (pSlvCfg->sGPIODSettings.sGPIOD0Config.bGPIODistance << 0u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD1Config.bGPIODistance << 1u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD2Config.bGPIODistance << 2u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD3Config.bGPIODistance << 3u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD4Config.bGPIODistance << 4u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD5Config.bGPIODistance << 5u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD6Config.bGPIODistance << 6u) | \
		(pSlvCfg->sGPIODSettings.sGPIOD7Config.bGPIODistance << 7u);

	/* GPIOD Mask update */
	psGPIODMask[0] = &pSlvNode->gpioDist.gpiod0msk;
	psGPIODMask[1] = &pSlvNode->gpioDist.gpiod1msk;
	psGPIODMask[2] = &pSlvNode->gpioDist.gpiod2msk;
	psGPIODMask[3] = &pSlvNode->gpioDist.gpiod3msk;
	psGPIODMask[4] = &pSlvNode->gpioDist.gpiod4msk;
	psGPIODMask[5] = &pSlvNode->gpioDist.gpiod5msk;
	psGPIODMask[6] = &pSlvNode->gpioDist.gpiod6msk;
	psGPIODMask[7] = &pSlvNode->gpioDist.gpiod7msk;

	asGPIODConfig[0] = pSlvCfg->sGPIODSettings.sGPIOD0Config;
	asGPIODConfig[1] = pSlvCfg->sGPIODSettings.sGPIOD1Config;
	asGPIODConfig[2] = pSlvCfg->sGPIODSettings.sGPIOD2Config;
	asGPIODConfig[3] = pSlvCfg->sGPIODSettings.sGPIOD3Config;
	asGPIODConfig[4] = pSlvCfg->sGPIODSettings.sGPIOD4Config;
	asGPIODConfig[5] = pSlvCfg->sGPIODSettings.sGPIOD5Config;
	asGPIODConfig[6] = pSlvCfg->sGPIODSettings.sGPIOD6Config;
	asGPIODConfig[7] = pSlvCfg->sGPIODSettings.sGPIOD7Config;

	for (nGPIOIndex = 0u; nGPIOIndex < 8u; nGPIOIndex++)
	{

		*(psGPIODMask[nGPIOIndex]) = (a2b_UInt16)((asGPIODConfig[nGPIOIndex].abBusPortMask[0] << 0u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[1] << 1u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[2] << 2u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[3] << 3u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[4] << 4u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[5] << 5u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[6] << 6u) | \
			(asGPIODConfig[nGPIOIndex].abBusPortMask[7] << 7u));
	}

	/* SWCTL register */
	pSlvNode->ctrlRegs.has_swctl = (pSlvCfg->sRegSettings.nSWCTL != 0) ? A2B_TRUE : A2B_FALSE;
	pSlvNode->ctrlRegs.swctl = pSlvCfg->sRegSettings.nSWCTL;

	/*ClockCFg1 & CFG2*/
	pSlvNode->pinIoRegs.clk1cfg |= (pSlvCfg->sClkOutSettings.bClk1Div << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1DIV) | \
		(pSlvCfg->sClkOutSettings.bClk1PreDiv << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1PDIV) | \
		(pSlvCfg->sClkOutSettings.bClk1Inv << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1INV);

	pSlvNode->pinIoRegs.clk2cfg |= (pSlvCfg->sClkOutSettings.bClk2Div << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2DIV) | \
		(pSlvCfg->sClkOutSettings.bClk2PreDiv << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2PDIV) | \
		(pSlvCfg->sClkOutSettings.bClk2Inv << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2INV);

	pSlvNode->ctrlRegs.suscfg = pSlvCfg->sRegSettings.nSUSCFG;

	pSlvNode->has_mbox = A2B_TRUE;
	pSlvNode->mbox.mbox0ctl = pSlvCfg->sRegSettings.nMBOX0CTL;
	pSlvNode->mbox.mbox1ctl = pSlvCfg->sRegSettings.nMBOX1CTL;

	if (A2B_IS_AD242X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
	{
		/*This check is needed because this register field is present only for Chiron,
		and this function is called to populate demeter registers too*/
		pSlvNode->i2cI2sRegs.i2sgcfg |= (pSlvCfg->sI2SSettings.bSerialRxOnDTx1 << (a2b_UInt8)A2B_BITP_I2SGCFG_RXONDTX1);
	}


	pSlvNode->i2cI2sRegs.pdmctl2 |= ((pSlvCfg->sPDMSettings.bPDMInvClk << (a2b_UInt8)A2B_BITP_PDMCTL2_PDMINVCLK) |
		(pSlvCfg->sPDMSettings.bPDMAltClk << (a2b_UInt8)A2B_BITP_PDMCTL2_PDMALTCLK) |
		(pSlvCfg->sPDMSettings.bPDM0FallingEdgeFrst << (a2b_UInt8)A2B_BITP_PDMCTL2_PDM0FFRST) |
		(pSlvCfg->sPDMSettings.bPDM1FallingEdgeFrst << (a2b_UInt8)A2B_BITP_PDMCTL2_PDM1FFRST) |
		(pSlvCfg->sPDMSettings.ePDMDestination << (a2b_UInt8)A2B_BITP_PDMCTL2_PDMDEST));

	pSlvNode->i2cI2sRegs.pllctl = pSlvCfg->sRegSettings.nPLLCTL;

	if (A2B_IS_AD242X_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product))
	{
		/*This check is needed because these registers are present only for Chiron,
						and this function is called to populate demeter registers too*/
		pSlvNode->has_tuningRegs = A2B_TRUE;
		pSlvNode->tuningRegs.txactl |= (pSlvCfg->sRegSettings.nTXACTL);
		pSlvNode->tuningRegs.txbctl |= (pSlvCfg->sRegSettings.nTXBCTL);
	}
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterPinMux12
*
*  Helper routine to Parse the Master Pin Mux 1&2 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseMasterPinMux12(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg)
{
	/* Pin multiplex for GPIO 1*/
	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO1PinUsage)
	{
	case A2B_GPIO_1_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO1IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO1Interrupt << A2B_BITP_PINTEN_IO1IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO1IntPolarity << A2B_BITP_PINTINV_IO1INV);
		break;
	case A2B_GPIO_1_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO1OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO1Val << A2B_BITP_GPIODAT_IO1DAT);
		break;
	case A2B_GPIO_1_AS_CLKOUT1:
		pMstrNode->pinIoRegs.clk1cfg |= (1u << (a2b_UInt8)A2B_BITP_CLKOUT1_CLK1EN);
		break;
		/* case A2B_GPIO_1_DISABLE:
			break; */
	default:
		break;

	}

	/* Pin multiplex for GPIO 2*/
	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO2PinUsage)
	{
	case A2B_GPIO_2_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO2IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO2Interrupt << A2B_BITP_PINTEN_IO2IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO2IntPolarity << A2B_BITP_PINTINV_IO2INV);
		break;
	case A2B_GPIO_2_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO2OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO2Val << A2B_BITP_GPIODAT_IO2DAT);
		break;
	case A2B_GPIO_2_AS_CLKOUT2:
		pMstrNode->pinIoRegs.clk2cfg |= (1u << (a2b_UInt8)A2B_BITP_CLKOUT2_CLK2EN);
		break;
		/* case A2B_GPIO_2_DISABLE:
			break; */
	default:
		/* Do Nothing */
		break;

	}
}


/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterPinMux7
*
*  Helper routine to Parse the Master Pin Mux 7 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
* \NOTE: This function should be called for all AD243x variants other than AD2430/AD2438
******************************************************************************/
static void  adi_a2b_ParseMasterPinMux7(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg)
{

	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO7PinUsage)
	{
	case A2B_GPIO_7_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO7IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO7Interrupt << A2B_BITP_PINTEN_IO7IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO7IntPolarity << A2B_BITP_PINTINV_IO7INV);
		break;
	case A2B_GPIO_7_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO7OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO7Val << A2B_BITP_GPIODAT_IO7DAT);
		break;
		/*case A2B_GPIO_7_DISABLE:
			break; */
	case A2B_GPIO_7_PDMCLK:
		pMstrNode->i2cI2sRegs.pdmctl2 |= (1u << (a2b_UInt8)A2B_BITP_PDMCTL2_PDMALTCLK);
		break;
	case A2B_GPIO_7_RRSTRB:
		pMstrNode->i2cI2sRegs.i2srrctl |= (1u << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENSTRB);
		break;
	default:
		break;

	}
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseSlavePinMux7
*
*  Helper routine to Parse the Slave Pin Mux 7 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pSlvNode Ptr to Slave Node of the BDD struct
*  \param          [in]    pSlvCfg   Ptr to Slave Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
*  \NOTE: This function should be called for all AD243x variants other than AD2430/AD2438
******************************************************************************/
static void  adi_a2b_ParseSlavePinMux7(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg)
{

	/* Pin multiplex for GPIO 7*/
	switch (pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO7PinUsage)
	{
	case A2B_GPIO_7_INPUT:
		pSlvNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO7IEN);
		pSlvNode->pinIoRegs.pinten |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO7Interrupt << A2B_BITP_PINTEN_IO7IE);
		pSlvNode->pinIoRegs.pintinv |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO7IntPolarity << A2B_BITP_PINTINV_IO7INV);
		break;
	case A2B_GPIO_7_OUTPUT:
		pSlvNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO7OEN);
		pSlvNode->pinIoRegs.gpiodat |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO7Val << A2B_BITP_GPIODAT_IO7DAT);
		break;
		/*case A2B_GPIO_7_DISABLE:
			break; */
	case A2B_GPIO_7_PDMCLK:
		pSlvNode->i2cI2sRegs.pdmctl2 |= (1u << (a2b_UInt8)A2B_BITP_PDMCTL2_PDMALTCLK);
		break;
	case A2B_GPIO_7_RRSTRB:
		pSlvNode->i2cI2sRegs.i2srrctl |= (1u << (a2b_UInt8)A2B_BITP_I2SRRCTL_ENSTRB);
	default:
		break;

	}
}
#endif

#ifdef ENABLE_AD243x_SUPPORT
/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterNCD_243x
*
*  Helper routine to Parse the 243x Master config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*  \param          [in]    pMstCfg   Ptr to Common Config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseMasterNCD_243x
(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg,
	ADI_A2B_COMMON_CONFIG* pCommon
)
{
	a2b_UInt32 i;
	a2b_Bool isAd2430_8 = A2B_FALSE;
	uint32_t* psGPIODMask;
	A2B_GPIOD_PIN_CONFIG asGPIODConfig;

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pMstrNode->nodeDescr.vendor, pMstrNode->nodeDescr.product);

	/*Call 242x config to populate all legacy registers*/
	adi_a2b_ParseMasterNCD_242x(pMstrNode, pMstCfg, pCommon);

	/**update regsiters with new fields**/

	/* I2S sync disable field
	 * If in GUI,
	 * 		Disable Sync pin bit is not set	(0), then SYNC is ENABLED
	 * 		Disable Sync pin bit is set 	(1), then SYNC is DISABLED
	 * */
	if (pMstCfg->sI2SSettings.bSync == A2B_ENABLED)
	{
		pMstrNode->i2cI2sRegs.i2sgcfg |= ((a2b_UInt8)0 << (a2b_UInt8)A2B_BITP_I2SGCFG_SYNCDIS);
	}
	else
	{
		pMstrNode->i2cI2sRegs.i2sgcfg |= ((a2b_UInt8)1 << (a2b_UInt8)A2B_BITP_I2SGCFG_SYNCDIS);
	}

	/* AD243x has TXCTL only */
	pMstrNode->tuningRegs.txctl = (pMstCfg->sRegSettings.nTXACTL);
	/*Update GPIO0 configuration*/
	pMstrNode->gpioDist.gpiodinv |= (pMstCfg->sGPIODSettings.sGPIOD0Config.bGPIOSignalInv << 0u);
	pMstrNode->gpioDist.gpioden |= (pMstCfg->sGPIODSettings.sGPIOD0Config.bGPIODistance << 0u);
	psGPIODMask = &pMstrNode->gpioDist.gpiod0msk;
	asGPIODConfig = pMstCfg->sGPIODSettings.sGPIOD0Config;
	*psGPIODMask = (a2b_UInt16)((asGPIODConfig.abBusPortMask[0] << 0u) | \
				(asGPIODConfig.abBusPortMask[1] << 1u) | \
				(asGPIODConfig.abBusPortMask[2] << 2u) | \
				(asGPIODConfig.abBusPortMask[3] << 3u) | \
				(asGPIODConfig.abBusPortMask[4] << 4u) | \
				(asGPIODConfig.abBusPortMask[5] << 5u) | \
				(asGPIODConfig.abBusPortMask[6] << 6u) | \
				(asGPIODConfig.abBusPortMask[7] << 7u));
	/** Populate new registers **/
	if (!isAd2430_8)
	{
		pMstrNode->ctrlRegs.has_swctl2 = (pMstCfg->sRegSettings.nSWCTL2 != 0) ? A2B_TRUE : A2B_FALSE;
		pMstrNode->ctrlRegs.swctl2 = pMstCfg->sRegSettings.nSWCTL2;

		pMstrNode->ctrlRegs.has_swctl3 = (pMstCfg->sRegSettings.nSWCTL3 != 0) ? A2B_TRUE : A2B_FALSE;
		pMstrNode->ctrlRegs.swctl3 = pMstCfg->sRegSettings.nSWCTL3;

		pMstrNode->ctrlRegs.has_swctl5 = (pMstCfg->sRegSettings.nSWCTL5 != 0) ? A2B_TRUE : A2B_FALSE;
		pMstrNode->ctrlRegs.swctl5 = pMstCfg->sRegSettings.nSWCTL5;

		/*SPI and DT*/
		adi_a2b_ParseSpiDT(pMstrNode, &pMstCfg->oSpiSettings);
	}

	/*Tx Xbar*/
	pMstrNode->i2sCrossbarRegs.has_txcrossbar = A2B_TRUE;
	pMstrNode->i2sCrossbarRegs.txcrossbar_count = A2B_TOTAL_TXBAR_REGS;
	for (i = 0u; i < A2B_TOTAL_TXBAR_REGS; i++)
	{
		pMstrNode->i2sCrossbarRegs.txcrossbar[i] = pMstCfg->anTxXbarSettings[i];
	}

	/*RxXbar*/
	pMstrNode->i2sCrossbarRegs.has_rxmask = A2B_TRUE;
	pMstrNode->i2sCrossbarRegs.rxmask_count = A2B_TOTAL_RXMASK_REGS;
	for (i = 0u; i < A2B_TOTAL_RXMASK_REGS; i++)
	{
		pMstrNode->i2sCrossbarRegs.rxmask[i] = pMstCfg->anRxXbarSettings[i];
	}

	/*VMTR settings*/
	if (!isAd2430_8)
	{
		adi_a2b_ParseVmtr(pMstrNode, &pMstCfg->oVmtrSettings);
	}

	/*pwm*/
	adi_a2b_ParsePwm(pMstrNode, &pMstCfg->oPwmSettings);

	/*Pin assign*/
	pMstrNode->pinIoRegs.pincfg |= pMstCfg->oPinAssignSettings.nGpioMode << (a2b_UInt8)A2B_BITP_PINCFG_GPIOSEL;

	adi_a2b_ParseSioPinCfg(pMstrNode, &pMstCfg->oPinAssignSettings);
	adi_a2b_ParseNonSioPinCfg(pMstrNode, &pMstCfg->oPinAssignSettings);

	if (pMstCfg->sI2SSettings.bRXInterleave)
	{
		setRegVal(&pMstrNode->i2cI2sRegs.i2scfg, 7u, A2B_BITM_I2SCFG_RXPINS, A2B_BITP_I2SCFG_RXPINS);//7 is the value of the field if Rx Interlave is enabled for AD243x
	}

	if (pMstCfg->sI2SSettings.bTXInterleave)
	{
		setRegVal(&pMstrNode->i2cI2sRegs.i2scfg, 7u, A2B_BITM_I2SCFG_TXPINS, A2B_BITP_I2SCFG_TXPINS);  //7 is the value of the field if Tx Interlave is enabled for AD243x
	}
	/*I2STEST register*/
	pMstrNode->i2cI2sRegs.i2stest = (a2b_UInt8)(pMstCfg->sRegSettings.nI2STEST & A2B_BITM_I2STEST_EXTLOOPBK);
	adi_a2b_ParseMasterPinMux0(pMstrNode, pMstCfg);
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseMasterPinMux0
*
*  Helper routine to Parse the Master Pin Mux 0 from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pMstrNode Ptr to Master Node of the BDD struct
*  \param          [in]    pMstCfg   Ptr to Master Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseMasterPinMux0(
	bdd_Node* pMstrNode,
	ADI_A2B_MASTER_NCD* pMstCfg)
{
	/* Pin multiplex for GPIO 0*/
	switch (pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO0PinUsage)
	{
	case A2B_GPIO_0_INPUT:
		pMstrNode->pinIoRegs.gpioien |= (1u << (a2b_UInt8)A2B_BITP_GPIOIEN_IO0IEN);
		pMstrNode->pinIoRegs.pinten |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO1Interrupt << A2B_BITP_PINTEN_IO0IE);
		pMstrNode->pinIoRegs.pintinv |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO1IntPolarity << A2B_BITP_PINTINV_IO0INV);
		break;
	case A2B_GPIO_0_OUTPUT:
		pMstrNode->pinIoRegs.gpiooen |= (1u << (a2b_UInt8)A2B_BITP_GPIOOEN_IO0OEN);
		pMstrNode->pinIoRegs.gpiodat |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO1Val << A2B_BITP_GPIODAT_IO0DAT);
		break;
	default:
		break;

	}

}

/*!****************************************************************************
*
*  \b              adi_a2b_ParseSioPinCfg
*
*  Helper routine to Parse the 243x Sio pin config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode                Ptr to  Node of the BDD struct
*  \param          [in]    oPinAssignSettings   Instance of pin config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParseSioPinCfg(bdd_Node* pNode, A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings)
{
	a2b_UInt32 nTxPins = 0u;
	a2b_UInt32 nRxPins = 0u;
	a2b_UInt32 nPDMPins = 0u;
	a2b_Bool isAd2430_8 = A2B_FALSE;


	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product);

	/* SIO 0 - No change for (AD2430/38) */
	switch (pPinAssignSettings->oSio0.eFunc)
	{
	case FUNC_RX0:
		nRxPins++;
		break;
	case FUNC_PDM0:
		nPDMPins++;
		pNode->i2cI2sRegs.pdmctl |= (1u << A2B_BITP_PDMCTL_PDM0EN);
		break;
	default:
		/* do nothing */
		break;
	}

	/* SIO1 - No change for (AD2430/38)*/
	switch (pPinAssignSettings->oSio1.eFunc)
	{
	case FUNC_RX0:
	case FUNC_RX1:
		nRxPins++;
		break;
	case FUNC_PDM1:
		nPDMPins++;
		pNode->i2cI2sRegs.pdmctl |= (1u << A2B_BITP_PDMCTL_PDM1EN);
		break;
	case FUNC_TX3:
		nTxPins++;
		break;
	default:
		/* do nothing */
		break;
	}



	if (isAd2430_8)
	{
		/* SIO2 */
		switch (pPinAssignSettings->oSio2.eFunc)
		{
		case FUNC_RX0:
		case FUNC_RX1:
		case FUNC_RX2:
			nRxPins++;
			break;
		case FUNC_PDM_CLK:
			pNode->i2cI2sRegs.pdmctl2 |= (1u << A2B_BITP_PDMCTL2_PDMALTCLK);
			pNode->i2cI2sRegs.i2srrctl &= (~(A2B_BITM_I2SRRCTL_ENSTRB));
			break;
		case FUNC_TX2:
			nTxPins++;
			break;
		default:
			/* do nothing */
			break;
		}
		/* SIO3 */
		switch (pPinAssignSettings->oSio3.eFunc)
		{
		case FUNC_RX1:
		case FUNC_RX2:
		case FUNC_RX3:
			nRxPins++;
			break;
		case FUNC_TX1:
			nTxPins++;
			break;
		case FUNC_PWM_CH3:
			pNode->pwmRegs.pwmcfg |= A2B_BITM_PWMCFG_PWM3EN;
			break;

		default:
			/* do nothing */
			break;
		}
		/* SIO4 */
		switch (pPinAssignSettings->oSio4.eFunc)
		{
		case FUNC_TX0:
			nTxPins++;
			break;
		case FUNC_PWM_OE:
			pNode->pwmRegs.pwmcfg |= A2B_BITM_PWMCFG_PWMOEEN;
			break;
		default:
			/* do nothing */
			break;
		}
	}
	else
	{
		/* SIO2 */
		switch (pPinAssignSettings->oSio2.eFunc)
		{
		case FUNC_RX0:
		case FUNC_RX1:
		case FUNC_RX2:
			nRxPins++;
			break;
		case FUNC_SS1:
			pNode->spiRegs.has_spipicfg = A2B_TRUE;
			pNode->spiRegs.spipicfg |= (1u << A2B_BITP_SPIPINCFG_SPIMSS1EN);
			break;
		case FUNC_TX2:
			nTxPins++;
			break;
		default:
			/* do nothing */
			break;
		}
		/* SIO3 */
		switch (pPinAssignSettings->oSio3.eFunc)
		{
		case FUNC_RX1:
		case FUNC_RX2:
		case FUNC_RX3:
			nRxPins++;
			break;
		case FUNC_TX1:
			nTxPins++;
			break;
		default:
			/* do nothing */
			break;
		}
		/* SIO4 */
		switch (pPinAssignSettings->oSio4.eFunc)
		{
		case FUNC_TX0:
			nTxPins++;
			break;
		default:
			/* do nothing */
			break;
		}
	}




	/* SIO4 */




	setRegVal(&pNode->i2cI2sRegs.i2scfg, nRxPins, A2B_BITM_I2SCFG_RXPINS, A2B_BITP_I2SCFG_RXPINS);
	setRegVal(&pNode->i2cI2sRegs.i2scfg, nTxPins, A2B_BITM_I2SCFG_TXPINS, A2B_BITP_I2SCFG_TXPINS);


}


/*!****************************************************************************
*
*  \b              adi_a2b_ParseNonSioPinCfg
*
*  Helper routine to Parse the 243x Non Sio pin config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode                Ptr to  Node of the BDD struct
*  \param          [in]    oPinAssignSettings   Instance of pin config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParseNonSioPinCfg(bdd_Node* pNode, A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings)
{
	a2b_Bool isAd2430_8 = A2B_FALSE;

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product);

	if (!isAd2430_8)
	{
		adi_a2b_ParseGPIO7PinCfg(pNode, pPinAssignSettings);
	}

	adi_a2b_ParseI2CPinCfg(pNode, pPinAssignSettings);

	if (!isAd2430_8)
	{
		adi_a2b_ParseSPIPinCfg(pNode, pPinAssignSettings);
	}

	adi_a2b_ParseADRPinCfg(pNode, pPinAssignSettings);

}

/*!****************************************************************************
*
*  \b              adi_a2b_PraseGPIO7PinCfg
*
*  Helper routine to Parse the 243x GPIO7 pin config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode                Ptr to  Node of the BDD struct
*  \param          [in]    pPinAssignSettings   Pointer to of pin config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
* \note: This function shoule be called for 243x variants other than AD2430/AD2438
******************************************************************************/
static void adi_a2b_ParseGPIO7PinCfg(bdd_Node* pNode, A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings)
{
	switch (pPinAssignSettings->oGPIO7.eFunc)
	{
	case FUNC_RR_STRB:
		setRegVal(&pNode->i2cI2sRegs.i2srrctl, 1u, A2B_BITM_I2SRRCTL_ENSTRB, A2B_BITP_I2SRRCTL_ENSTRB);
		setRegVal(&pNode->i2cI2sRegs.pdmctl2, 0u, A2B_BITM_PDMCTL2_PDMALTCLK, A2B_BITP_PDMCTL2_PDMALTCLK);
		break;
	case FUNC_PDM_CLK:
		setRegVal(&pNode->i2cI2sRegs.i2srrctl, 0u, A2B_BITM_I2SRRCTL_ENSTRB, A2B_BITP_I2SRRCTL_ENSTRB);
		setRegVal(&pNode->i2cI2sRegs.pdmctl2, 1u, A2B_BITM_PDMCTL2_PDMALTCLK, A2B_BITP_PDMCTL2_PDMALTCLK);
		break;
	default:
		/* GPIO: This config will be taken care in GPIO config*/
		setRegVal(&pNode->i2cI2sRegs.i2srrctl, 0u, A2B_BITM_I2SRRCTL_ENSTRB, A2B_BITP_I2SRRCTL_ENSTRB);
		setRegVal(&pNode->i2cI2sRegs.pdmctl2, 0u, (a2b_UInt8)A2B_BITM_PDMCTL2_PDMALTCLK, A2B_BITP_PDMCTL2_PDMALTCLK);
		break;
	}

}


/*!****************************************************************************
*
*  \b              adi_a2b_PraseI2CPinCfg
*
*  Helper routine to Parse the 243x I2C pin config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode                Ptr to  Node of the BDD struct
*  \param          [in]    pPinAssignSettings   Pointer to of pin config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParseI2CPinCfg(bdd_Node* pNode, A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings)
{
	/*SCL, SDA - Settings for both of these will remain same.*/
	if (pPinAssignSettings->oSCL.eFunc == FUNC_I2C)
	{
		setRegVal(&pNode->i2cI2sRegs.i2ccfg, 0u, A2B_BITM_I2CCFG_DISABLE, A2B_BITP_I2CCFG_DISABLE);

	}
	else
	{ /*GPIO*/
		setRegVal(&pNode->i2cI2sRegs.i2ccfg, 1u, A2B_BITM_I2CCFG_DISABLE, A2B_BITP_I2CCFG_DISABLE);
	}

}

/*!****************************************************************************
*
*  \b              adi_a2b_PraseSPIPinCfg
*
*  Helper routine to Parse the 243x SPI pin config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode                Ptr to  Node of the BDD struct
*  \param          [in]    pPinAssignSettings   Pointer to of pin config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
*  \note: This function shoule be called for 243x variants other than AD2430/AD2438
******************************************************************************/
static void adi_a2b_ParseSPIPinCfg(bdd_Node* pNode, A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings)
{
	uint32_t nSpiMode = 2u; /* default value */
	/*SPI MOSI, MISO, SCK, ADR1 */
	switch (pPinAssignSettings->oMISO.eFunc)
	{
	case FUNC_SPI_SLAVE:
		nSpiMode = 0u;
		setRegVal(&pNode->spiRegs.spicfg, nSpiMode, A2B_BITM_SPICFG_SPIMODE, A2B_BITP_SPICFG_SPIMODE);
		setRegVal(&pNode->pinIoRegs.clk1cfg, 0u, A2B_BITM_CLKOUT1_CLK1EN, A2B_BITP_CLKOUT1_CLK1EN);
		setRegVal(&pNode->pwmRegs.pwmcfg, 0u, A2B_BITM_PWMCFG_PWMOEEN, A2B_BITP_PWMCFG_PWMOEEN);
		break;
	case FUNC_SPI_MASTER:
		nSpiMode = 1u;
		setRegVal(&pNode->spiRegs.spicfg, nSpiMode, A2B_BITM_SPICFG_SPIMODE, A2B_BITP_SPICFG_SPIMODE);
		/* TODO should the pwm and clk1cfg be set to 0 like earlier? */
		break;
	default:
		setRegVal(&pNode->pwmRegs.pwmcfg, ((pPinAssignSettings->oMISO.eFunc == FUNC_PWM_CH1) ? 1u : 0u),
			A2B_BITM_PWMCFG_PWM1EN, A2B_BITP_PWMCFG_PWM1EN);
		setRegVal(&pNode->pwmRegs.pwmcfg, ((pPinAssignSettings->oMOSI.eFunc == FUNC_PWM_CH2) ? 1u : 0u),
			A2B_BITM_PWMCFG_PWM2EN, A2B_BITP_PWMCFG_PWM2EN);
		setRegVal(&pNode->pwmRegs.pwmcfg, ((pPinAssignSettings->oSCK.eFunc == FUNC_PWM_CH3) ? 1u : 0u),
			A2B_BITM_PWMCFG_PWM3EN, A2B_BITP_PWMCFG_PWM3EN);
		break;
	}

}

/*!****************************************************************************
*
*  \b              adi_a2b_PraseADRPinCfg
*
*  Helper routine to Parse the 243x ADR1 and ADR2 pin config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode                Ptr to  Node of the BDD struct
*  \param          [in]    pPinAssignSettings   Pointer to of pin config of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParseADRPinCfg(bdd_Node* pNode, A2B_PIN_ASSIGN_CONFIG* pPinAssignSettings)
{
	a2b_Bool isAd2430_8 = A2B_FALSE;

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product);

	/* ADR1 */
	if (!isAd2430_8)
	{
	switch (pPinAssignSettings->oADR1.eFunc)
	{
	case FUNC_I2C_CLKOUT1:
	setRegVal(&pNode->pinIoRegs.clk1cfg, 1u, A2B_BITM_CLKOUT1_CLK1EN, A2B_BITP_CLKOUT1_CLK1EN);
	break;

	case FUNC_SS0:
	setRegVal(&pNode->spiRegs.spipicfg, 1u, A2B_BITM_SPIPINCFG_SPIMSS0EN, A2B_BITP_SPIPINCFG_SPIMSS0EN);
	break;

	case FUNC_PWM_OE:
	setRegVal(&pNode->pwmRegs.pwmcfg, 1u, A2B_BITM_PWMCFG_PWMOEEN, A2B_BITP_PWMCFG_PWMOEEN);
	break;

	default:
	// GPIO fallback
	setRegVal(&pNode->pwmRegs.pwmcfg, 0u, A2B_BITM_PWMCFG_PWMOEEN, A2B_BITP_PWMCFG_PWMOEEN);
	break;
	}

	switch (pPinAssignSettings->oADR2.eFunc)
	{
	case FUNC_I2C_CLKOUT2:
	setRegVal(&pNode->pinIoRegs.clk2cfg, 1u, A2B_BITM_CLKOUT2_CLK2EN, A2B_BITP_CLKOUT2_CLK2EN);
	break;

	case FUNC_SS2:
	setRegVal(&pNode->spiRegs.spipicfg, 1u, A2B_BITM_SPIPINCFG_SPIMSS2EN, A2B_BITP_SPIPINCFG_SPIMSS2EN);
	break;

	default:
	// GPIO fallback
	setRegVal(&pNode->spiRegs.spipicfg, 0u, A2B_BITM_SPIPINCFG_SPIMSS2EN, A2B_BITP_SPIPINCFG_SPIMSS2EN);
	break;
	}

	}
	else
	{
	switch (pPinAssignSettings->oADR1.eFunc)
	{
	case FUNC_I2C_CLKOUT1:
	setRegVal(&pNode->pinIoRegs.clk1cfg, 1u, A2B_BITM_CLKOUT1_CLK1EN, A2B_BITP_CLKOUT1_CLK1EN);
	break;

	case FUNC_PWM_CH1:
	setRegVal(&pNode->pwmRegs.pwmcfg, 1u, A2B_BITM_PWMCFG_PWM1EN, A2B_BITP_PWMCFG_PWM1EN);
	break;

	case FUNC_RR_STRB:
	setRegVal(&pNode->i2cI2sRegs.i2srrctl, 1u, A2B_BITM_I2SRRCTL_ENSTRB, A2B_BITP_I2SRRCTL_ENSTRB);
	setRegVal(&pNode->i2cI2sRegs.pdmctl2, 0u, A2B_BITM_PDMCTL2_PDMALTCLK, A2B_BITP_PDMCTL2_PDMALTCLK);
	break;

	default:
	// GPIO fallback
	setRegVal(&pNode->pwmRegs.pwmcfg, 0u, A2B_BITM_PWMCFG_PWM1EN, A2B_BITP_PWMCFG_PWM1EN);
	break;
	}

	switch (pPinAssignSettings->oADR2.eFunc)
	{
	case FUNC_I2C_CLKOUT2:
	setRegVal(&pNode->pinIoRegs.clk2cfg, 1u, A2B_BITM_CLKOUT2_CLK2EN, A2B_BITP_CLKOUT2_CLK2EN);
	break;

	case FUNC_PWM_CH2:
	setRegVal(&pNode->pwmRegs.pwmcfg, 1u, A2B_BITM_PWMCFG_PWM2EN, A2B_BITP_PWMCFG_PWM2EN);
	break;

	default:
	// GPIO fallback
	setRegVal(&pNode->pwmRegs.pwmcfg, 0u, A2B_BITM_PWMCFG_PWM2EN, A2B_BITP_PWMCFG_PWM2EN);
	break;
	}

	}

}
/*!****************************************************************************
*
*  \b              adi_a2b_ParseVmtr
*
*  Helper routine to Parse the 243x VMTR register settings from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode            Ptr to  Node of the BDD struct
*  \param          [in]    poVmtrSettings   Pointer to VMTR settings of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
*  \note: This function shoule be called for 243x variants other than AD2430/AD2438
******************************************************************************/
static void adi_a2b_ParseVmtr(bdd_Node* pNode, A2B_VMTR_SETTINGS* poVmtrSettings)
{

	pNode->vmtrRegs.vmtr_ven = poVmtrSettings->bVEN;

	pNode->vmtrRegs.vmtr_inten = poVmtrSettings->bIntEN;

	pNode->vmtrRegs.vmtr_mxstat = poVmtrSettings->nMxStat;

	pNode->vmtrRegs.vmtr_mnstat = poVmtrSettings->nMinStat;

	pNode->vmtrRegs.vmtr_vmax0 = poVmtrSettings->oVtg0.nVmax;

	pNode->vmtrRegs.vmtr_vmin0 = poVmtrSettings->oVtg0.nVmin;

	pNode->vmtrRegs.vmtr_vmax1 = poVmtrSettings->oVtg1.nVmax;

	pNode->vmtrRegs.vmtr_vmin1 = poVmtrSettings->oVtg1.nVmin;

	pNode->vmtrRegs.vmtr_vmax2 = poVmtrSettings->oVtg2.nVmax;

	pNode->vmtrRegs.vmtr_vmin2 = poVmtrSettings->oVtg2.nVmin;

	pNode->vmtrRegs.vmtr_vmax3 = poVmtrSettings->oVtg3.nVmax;

	pNode->vmtrRegs.vmtr_vmin3 = poVmtrSettings->oVtg3.nVmin;

	pNode->vmtrRegs.vmtr_vmax4 = poVmtrSettings->oVtg4.nVmax;

	pNode->vmtrRegs.vmtr_vmin4 = poVmtrSettings->oVtg4.nVmin;

	pNode->vmtrRegs.vmtr_vmax5 = poVmtrSettings->oVtg5.nVmax;

	pNode->vmtrRegs.vmtr_vmin5 = poVmtrSettings->oVtg5.nVmin;

	pNode->vmtrRegs.vmtr_vmax6 = poVmtrSettings->oVtg6.nVmax;

	pNode->vmtrRegs.vmtr_vmin6 = poVmtrSettings->oVtg6.nVmin;
}

/*!****************************************************************************
*
*  \b              adi_a2b_ParsePwm
*
*  Helper routine to Parse the 243x PWM register settings from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode            Ptr to  Node of the BDD struct
*  \param          [in]    poPwmSettings   Pointer to PWM settings of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_ParsePwm(bdd_Node* pNode, A2B_PWM_SETTINGS* poPwmSettings)
{


	pNode->pwmRegs.pwmcfg = poPwmSettings->nPwmCfg;


	pNode->pwmRegs.pwmfreq = poPwmSettings->nPwmFreq;


	pNode->pwmRegs.pwmblink1 = poPwmSettings->nPwmBlink1;


	pNode->pwmRegs.pwmblink2 = poPwmSettings->nPwmBlink2;


	pNode->pwmRegs.pwmval_count = sizeof(pNode->pwmRegs.pwmval) / sizeof(uint32_t); //magic number or macro or arithmetic expression?
	pNode->pwmRegs.pwmval[0] = (poPwmSettings->nPwm1Val & 0x00ff) >> 0u; //PWM1VALL
	pNode->pwmRegs.pwmval[1] = (poPwmSettings->nPwm1Val & 0xff00) >> 8u; //PWM1VALH

	pNode->pwmRegs.pwmval[2] = (poPwmSettings->nPwm2Val & 0x00ff) >> 0u; //PWM2VALL
	pNode->pwmRegs.pwmval[3] = (poPwmSettings->nPwm2Val & 0xff00) >> 8u; //PWM2VALH

	pNode->pwmRegs.pwmval[4] = (poPwmSettings->nPwm3Val & 0x00ff) >> 0u; //PWM3VALL
	pNode->pwmRegs.pwmval[5] = (poPwmSettings->nPwm3Val & 0xff00) >> 8u; //PWM3VALH


	pNode->pwmRegs.pwmoe_count = sizeof(pNode->pwmRegs.pwmoe) / sizeof(uint32_t); //magic number or macro or arithmetic expression?
	pNode->pwmRegs.pwmoe[0] = (poPwmSettings->nPwmOEVal & 0x00ff) >> 0u; //PWM2VALL
	pNode->pwmRegs.pwmoe[1] = (poPwmSettings->nPwmOEVal & 0xff00) >> 8u; //PWM2VALH
}


/*!****************************************************************************
*
*  \b              adi_a2b_ParseSpiDT
*
*  Helper routine to Parse the 243x SPi and DT register settings from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pNode            Ptr to  Node of the BDD struct
*  \param          [in]    poSpiSettings   Pointer to SPI settings of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
*  \note: This function shoule be called for 243x variants other than AD2430/AD2438
******************************************************************************/
static void adi_a2b_ParseSpiDT(bdd_Node* pNode, A2B_SPI_SETTINGS* poSpiSettings)
{


	pNode->spiRegs.spicfg |= (poSpiSettings->nFDSlaveSel << (a2b_UInt8)A2B_BITP_SPICFG_SPIFDSS) | \
		(poSpiSettings->bFDClkStretchEn << (a2b_UInt8)A2B_BITP_SPICFG_ENFDCS) | \
		(poSpiSettings->nCPOL << (a2b_UInt8)A2B_BITP_SPICFG_SPI_CPOL) | \
		(poSpiSettings->nCPHA << (a2b_UInt8)A2B_BITP_SPICFG_SPI_CPHA) | \
		(poSpiSettings->nSPIMode << (a2b_UInt8)A2B_BITP_SPICFG_SPIMODE) | \
		(((poSpiSettings->eTunnelOwnership == DT_OWNER) ? 1u : 0u) << (a2b_UInt8)A2B_BITP_SPICFG_TNLOWNER);

	pNode->spiRegs.spipicfg |= (poSpiSettings->nMstrSS2En << (a2b_UInt8)A2B_BITP_SPIPINCFG_SPIMSS2EN) | \
		(poSpiSettings->nMstrSS1En << (a2b_UInt8)A2B_BITP_SPIPINCFG_SPIMSS1EN) | \
		(poSpiSettings->nMstrSS0En << (a2b_UInt8)A2B_BITP_SPIPINCFG_SPIMSS0EN) | \
		(poSpiSettings->nGpioSelect << (a2b_UInt8)A2B_BITP_SPIPINCFG_SPIGPIOSEL) | \
		(poSpiSettings->nGpioEnable << (a2b_UInt8)A2B_BITP_SPIPINCFG_SPIGPIOEN);



	pNode->spiRegs.spickdiv = poSpiSettings->nClkDivFactor << (a2b_UInt8)A2B_BITP_SPICKDIV_CKDIV;


	pNode->spiRegs.spifdsize = poSpiSettings->nFDSize << (a2b_UInt8)A2B_BITP_SPIFDSIZE_FDSIZE;



	pNode->spiRegs.spifdtarg = (poSpiSettings->nFDTargetNode << (a2b_UInt8)A2B_BITP_SPIFDTARG_NODE) | \
		(poSpiSettings->nTargetSSel << (a2b_UInt8)A2B_BITP_SPIFDTARG_SSEL);


	pNode->spiRegs.spimsk = (poSpiSettings->oSpiIntSettings.bFifoUnderflow << (a2b_UInt8)A2B_BITP_SPIMSK_FIFOUIEN) | \
		(poSpiSettings->oSpiIntSettings.bFifoOverflow << (a2b_UInt8)A2B_BITP_SPIMSK_FIFOOIEN) | \
		(poSpiSettings->oSpiIntSettings.bBadCommand << (a2b_UInt8)A2B_BITP_SPIMSK_BADCMDIEN) | \
		(poSpiSettings->oSpiIntSettings.bDataTunnel << (a2b_UInt8)A2B_BITP_SPIMSK_SPIDTIEN) | \
		(poSpiSettings->oSpiIntSettings.bSpiRemoteI2cAccess << (a2b_UInt8)A2B_BITP_SPIMSK_SPII2CIEN) | \
		(poSpiSettings->oSpiIntSettings.bSpiRemoteRegAccess << (a2b_UInt8)A2B_BITP_SPIMSK_SPIREGIEN) | \
		(poSpiSettings->oSpiIntSettings.bSpiDone << (a2b_UInt8)A2B_BITP_SPIMSK_SPIDIEN);


	pNode->dataTunnelRegs.dtcfg |= (poSpiSettings->bDTEnable << (a2b_UInt8)A2B_BITP_DTCFG_DTEN);

	/* TODO Setting to 0 may not be needed? */
	switch (poSpiSettings->eTunnelPos)
	{
	case DT_POS_FIRST:

		setRegVal(&pNode->dataTunnelRegs.dtcfg, 0u, A2B_BITM_DTCFG_DTLAST, A2B_BITP_DTCFG_DTLAST);
		setRegVal(&pNode->dataTunnelRegs.dtcfg, 1u, A2B_BITM_DTCFG_DTFRST, A2B_BITP_DTCFG_DTFRST);
		break;

	case DT_POS_LAST:
		setRegVal(&pNode->dataTunnelRegs.dtcfg, 1u, A2B_BITM_DTCFG_DTLAST, A2B_BITP_DTCFG_DTLAST);
		setRegVal(&pNode->dataTunnelRegs.dtcfg, 0u, A2B_BITM_DTCFG_DTFRST, A2B_BITP_DTCFG_DTFRST);
		break;

	case DT_POS_MIDDLE:
	default:
		setRegVal(&pNode->dataTunnelRegs.dtcfg, 0u, A2B_BITM_DTCFG_DTLAST, A2B_BITP_DTCFG_DTLAST);
		setRegVal(&pNode->dataTunnelRegs.dtcfg, 0u, A2B_BITM_DTCFG_DTFRST, A2B_BITP_DTCFG_DTFRST);
		break;
	}


	pNode->dataTunnelRegs.dtslots = (poSpiSettings->nDTUpstrmSlots << (a2b_UInt8)A2B_BITP_DTSLOTS_DTUPSLOTS) | \
		(poSpiSettings->nDTDwnstrmSlots << (a2b_UInt8)A2B_BITP_DTSLOTS_DTDNSLOTS);


	pNode->dataTunnelRegs.dtndnoffs = poSpiSettings->nDTDwnstrmOffset << (a2b_UInt8)A2B_BITP_DTDNOFFS_DTDNOFFS;


	pNode->dataTunnelRegs.dtnupoffs = poSpiSettings->nDTUpstrmOffset << (a2b_UInt8)A2B_BITP_DTUPOFFS_DTUPOFFS;

}




/*!****************************************************************************
*
*  \b              adi_a2b_ParseSlaveNCD_243x
*
*  Helper routine to Parse the 243x Slave config from BCF to BDD
*  fields of BDD structure
*
*  \param          [in]    pSlvNode Ptr to Slave Node of the BDD struct
*  \param          [in]    pSlvCfg   Ptr to Slave Node of the BCF struct
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void  adi_a2b_ParseSlaveNCD_243x
(
	bdd_Node* pSlvNode,
	ADI_A2B_SLAVE_NCD* pSlvCfg
)
{
	a2b_UInt32 i;
	a2b_Bool isAd2430_8 = A2B_FALSE;

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pSlvNode->nodeDescr.vendor, pSlvNode->nodeDescr.product);


	/*Call 242x config to populate all legacy registers*/
	adi_a2b_ParseSlaveNCD_242x(pSlvNode, pSlvCfg);

	/**update regsiters with new fields**/

	/* I2S sync disable field
	 * If in GUI,
	 * 		Disable Sync pin bit is not set	(0), then SYNC is ENABLED
	 * 		Disable Sync pin bit is set 	(1), then SYNC is DISABLED
	 * */
	if (pSlvCfg->sI2SSettings.bSync == A2B_ENABLED)
	{
		pSlvNode->i2cI2sRegs.i2sgcfg |= ((a2b_UInt8)0 << (a2b_UInt8)A2B_BITP_I2SGCFG_SYNCDIS);
	}
	else
	{
		pSlvNode->i2cI2sRegs.i2sgcfg |= ((a2b_UInt8)1 << (a2b_UInt8)A2B_BITP_I2SGCFG_SYNCDIS);
	}

	/* AD243x has TXCTL only */
	pSlvNode->tuningRegs.txctl = (pSlvCfg->sRegSettings.nTXACTL);

	/* PDM HPF Freq */
	switch (pSlvCfg->sPDMSettings.ePDMHpfCorner)
	{
	case HPF_CORNERFREQ_1Hz:
		pSlvNode->i2cI2sRegs.pdmctl2 |= A2B_ENUM_PDMCTL2_HPFCORNER_0;
		break;
	case HPF_CORNERFREQ_60Hz:
		pSlvNode->i2cI2sRegs.pdmctl2 |= A2B_ENUM_PDMCTL2_HPFCORNER_1;
		break;
	case HPF_CORNERFREQ_120Hz:
		pSlvNode->i2cI2sRegs.pdmctl2 |= A2B_ENUM_PDMCTL2_HPFCORNER_2;
		break;
	case HPF_CORNERFREQ_240Hz:
		pSlvNode->i2cI2sRegs.pdmctl2 |= A2B_ENUM_PDMCTL2_HPFCORNER_3;
		break;
	}

	if (!isAd2430_8)
	{
		/** Populate new registers **/

		pSlvNode->ctrlRegs.has_swctl2 = (pSlvCfg->sRegSettings.nSWCTL2 != 0) ? A2B_TRUE : A2B_FALSE;
		pSlvNode->ctrlRegs.swctl2 = pSlvCfg->sRegSettings.nSWCTL2;

		pSlvNode->ctrlRegs.has_swctl3 = (pSlvCfg->sRegSettings.nSWCTL3 != 0) ? A2B_TRUE : A2B_FALSE;
		pSlvNode->ctrlRegs.swctl3 = pSlvCfg->sRegSettings.nSWCTL3;

		pSlvNode->ctrlRegs.has_swctl5 = (pSlvCfg->sRegSettings.nSWCTL5 != 0) ? A2B_TRUE : A2B_FALSE;
		pSlvNode->ctrlRegs.swctl5 = pSlvCfg->sRegSettings.nSWCTL5;

		/*SPI and DT*/
		adi_a2b_ParseSpiDT(pSlvNode, &pSlvCfg->oSpiSettings);
	}
	/*Tx Xbar*/

	pSlvNode->i2sCrossbarRegs.txcrossbar_count = A2B_TOTAL_TXBAR_REGS;
	for (i = 0u; i < A2B_TOTAL_TXBAR_REGS; i++)
	{
		pSlvNode->i2sCrossbarRegs.txcrossbar[i] = pSlvCfg->anTxXbarSettings[i];
	}

	/*RxXbar*/

	pSlvNode->i2sCrossbarRegs.rxmask_count = A2B_TOTAL_RXMASK_REGS;
	for (i = 0u; i < A2B_TOTAL_RXMASK_REGS; i++)
	{
		pSlvNode->i2sCrossbarRegs.rxmask[i] = pSlvCfg->anRxXbarSettings[i];
	}

	if (!isAd2430_8)
	{
		/*VMTR settings*/
		adi_a2b_ParseVmtr(pSlvNode, &pSlvCfg->oVmtrSettings);
	}

	/*pwm*/
	adi_a2b_ParsePwm(pSlvNode, &pSlvCfg->oPwmSettings);

	/*Pin assign*/
	pSlvNode->pinIoRegs.pincfg |= pSlvCfg->oPinAssignSettings.nGpioMode << (a2b_UInt8)A2B_BITP_PINCFG_GPIOSEL;

	adi_a2b_ParseSioPinCfg(pSlvNode, &pSlvCfg->oPinAssignSettings);
	adi_a2b_ParseNonSioPinCfg(pSlvNode, &pSlvCfg->oPinAssignSettings);

	/* I2C registers */
	pSlvNode->i2cI2sRegs.i2ccfg |= pSlvCfg->sConfigCtrlSettings.bEnI2cFstModePlus << (a2b_UInt8)A2B_BITP_I2CCFG_FMPLUS;

	if (pSlvCfg->sI2SSettings.bRXInterleave)
	{
		setRegVal(&pSlvNode->i2cI2sRegs.i2scfg, 7u, A2B_BITM_I2SCFG_RXPINS, A2B_BITP_I2SCFG_RXPINS);//7 is the value of the field if Rx Interlave is enabled for AD243x
	}

	if (pSlvCfg->sI2SSettings.bTXInterleave)
	{
		setRegVal(&pSlvNode->i2cI2sRegs.i2scfg, 7u, A2B_BITM_I2SCFG_TXPINS, A2B_BITP_I2SCFG_TXPINS);//7 is the value of the field if Rx Interlave is enabled for AD243x
	}
	/*I2STEST register*/
	pSlvNode->i2cI2sRegs.i2stest = (a2b_UInt8) (pSlvCfg->sRegSettings.nI2STEST & A2B_BITM_I2STEST_EXTLOOPBK);
}



#endif




/*!****************************************************************************
*
*  \b              adi_a2b_ParsePeriCfgFrComBCF
*
*  Helper routine parse peripheral info from compressed BCF
*
*  \param          [in]   pBusDescription      	pointer to bus decription
*  \param          [out]  aPeriDownloadTable    Table tp store peripheral config info
*  \param          [in]   nBusIndex       		Bus/Chain Index
*
*  \pre            None
*
*  \post           None
*
*  \return         True = success, False = Failure
*
******************************************************************************/
void adi_a2b_ParsePeriCfgFrComBCF(ADI_A2B_COMPR_BCD* pBusDescription, ADI_A2B_NODE_PERICONFIG aPeriDownloadTable[], a2b_UInt8 nBusIndex)
{
	ADI_A2B_COMPR_MASTER_SLAVE_CONFIG* pMasterSlaveChain;
	a2b_UInt8 nIndex1, nIndex2;
	a2b_UInt8 nNumConfig;
	ADI_A2B_NODE_PERICONFIG_DATA* pSlvCfg;

	/* Number of peripheral devices connected to target processor */
	aPeriDownloadTable[0u].nNumConfig = 0u;
	for (nIndex2 = 0u; nIndex2 < pBusDescription->sTargetProperties.nNumPeriDevice; nIndex2++)
	{
		/* Include only if the configuration exists */
		if (pBusDescription->sTargetProperties.apPeriConfig[nIndex2] != A2B_NULL)
		{
			/* Number of config */
			nNumConfig = aPeriDownloadTable[0u].nNumConfig;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].bActive = 1u;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nConnectedNodeID = 0xFFu;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nSpiSs = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nSpiSS;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nDeviceAddress = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nI2Caddr;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nNumPeriConfigUnit = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nNumPeriConfigUnit;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].paPeriConfigUnit = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->paPeriConfigUnit;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].ePeriDeviceInterface = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->ePeriDeviceInterface;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].bPostDiscCfg = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->bPostDiscCfg;
			aPeriDownloadTable[0u].nNumConfig++;
		}
		else
		{
			aPeriDownloadTable[0u].aDeviceConfig[nIndex2].bActive = 0u;
		}

	}

	/* Initialize empty fields */
	for (nIndex2 = pBusDescription->sTargetProperties.nNumPeriDevice; nIndex2 < (a2b_UInt8)ADI_A2B_MAX_DEVICES_PER_NODE; nIndex2++)
	{
		aPeriDownloadTable[0u].aDeviceConfig[nIndex2].bActive = 0u;
	}

	pMasterSlaveChain = pBusDescription->apNetworkconfig[nBusIndex];
	for (nIndex1 = 0u; nIndex1 < (a2b_UInt8)pMasterSlaveChain->nNumSlaveNode; nIndex1++)
	{
		pSlvCfg = pMasterSlaveChain->apNodePericonfig[nIndex1];
		aPeriDownloadTable[nIndex1 + 1].nNumConfig = 0u;
		/* Peripheral update */
		for (nIndex2 = 0u; nIndex2 < pSlvCfg->nNumPeriDevice; nIndex2++)
		{
			if (pSlvCfg->apNodePeriDeviceConfig[nIndex2]->paPeriConfigUnit != A2B_NULL)
			{
				/* Number of config */
				nNumConfig = aPeriDownloadTable[nIndex1 + 1].nNumConfig;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nIndex2].bActive = 1u;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nConnectedNodeID = nIndex2;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nSpiSs = pSlvCfg->apNodePeriDeviceConfig[nIndex2]->nSpiSS;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nDeviceAddress = pSlvCfg->apNodePeriDeviceConfig[nIndex2]->nI2Caddr;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nNumPeriConfigUnit = pSlvCfg->apNodePeriDeviceConfig[nIndex2]->nNumPeriConfigUnit;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].paPeriConfigUnit = pSlvCfg->apNodePeriDeviceConfig[nIndex2]->paPeriConfigUnit;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].ePeriDeviceInterface = pSlvCfg->apNodePeriDeviceConfig[nIndex2]->ePeriDeviceInterface;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].eSpiMode = pSlvCfg->apNodePeriDeviceConfig[nIndex2]->eSpiMode;
				aPeriDownloadTable[nIndex1 + 1u].nNumConfig++;
			}
			else
			{
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nIndex2].bActive = 0u;
			}
		}

		for (nIndex2 = pSlvCfg->nNumPeriDevice; nIndex2 < (a2b_UInt8)ADI_A2B_MAX_DEVICES_PER_NODE; nIndex2++)
		{
			aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nIndex2].bActive = 0u;
		}
	}

}


/*****************************************************************************************/
/*!
@brief      This function parses Bus Configuration Data(BCD) to Peripheral Config Table

@param [in] pBusDescription       Pointer to bus configuration data
@param [in] aPeriDownloadTable    Framework configuration pointer
@param [in] nBusIndex			  Chain/Bus/network index

@return     void

*/
/******************************************************************************************/
void adi_a2b_ParsePeriCfgTable(ADI_A2B_BCD* pBusDescription, ADI_A2B_NODE_PERICONFIG aPeriDownloadTable[], a2b_UInt8 nBusIndex)
{
	ADI_A2B_MASTER_SLAVE_CONFIG* pMasterSlaveChain;
	ADI_A2B_SLAVE_NCD* pSlvCfg;
	a2b_UInt8 nIndex1, nIndex2;
	a2b_UInt8 nNumConfig;

	/* Number of peripheral devices connected to target processor */
	aPeriDownloadTable[0u].nNumConfig = 0u;

	for (nIndex2 = 0u; nIndex2 < pBusDescription->sTargetProperties.nNumPeriDevice; nIndex2++)
	{
		/* Include only if the configuration exists */
		if (pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->paPeriConfigUnit != A2B_NULL)
		{
			/* Number of config */
			nNumConfig = aPeriDownloadTable[0u].nNumConfig;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].bActive = 1u;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nConnectedNodeID = 0xFFu;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nSpiSs = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nSpiSS;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nDeviceAddress = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nI2Caddr;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].nNumPeriConfigUnit = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nNumPeriConfigUnit;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].paPeriConfigUnit = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->paPeriConfigUnit;
			aPeriDownloadTable[0u].aDeviceConfig[nNumConfig].ePeriDeviceInterface = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->ePeriDeviceInterface;
			aPeriDownloadTable[0u].nNumConfig++;
		}
		else
		{
			aPeriDownloadTable[0u].aDeviceConfig[nIndex2].bActive = 0u;
		}

	}
	/* Initialize empty fields */
	for (nIndex2 = pBusDescription->sTargetProperties.nNumPeriDevice; nIndex2 < (a2b_UInt8)ADI_A2B_MAX_DEVICES_PER_NODE; nIndex2++)
	{
		aPeriDownloadTable[0u].aDeviceConfig[nIndex2].bActive = 0u;
	}

	pMasterSlaveChain = pBusDescription->apNetworkconfig[nBusIndex];

	for (nIndex1 = 0u; nIndex1 < (a2b_UInt8)pMasterSlaveChain->nNumSlaveNode; nIndex1++)
	{
		pSlvCfg = pMasterSlaveChain->apSlaveConfig[nIndex1];
		aPeriDownloadTable[nIndex1 + 1].nNumConfig = 0u;
		/* Peripheral update */
		for (nIndex2 = 0u; nIndex2 < pSlvCfg->nNumPeriDevice; nIndex2++)
		{
			if (pSlvCfg->apPeriConfig[nIndex2]->paPeriConfigUnit != A2B_NULL)
			{
				/* Number of config */
				nNumConfig = aPeriDownloadTable[nIndex1 + 1].nNumConfig;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nIndex2].bActive = 1u;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nConnectedNodeID = pSlvCfg->nNodeID;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nSpiSs = pSlvCfg->apPeriConfig[nIndex2]->nSpiSS;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nDeviceAddress = pSlvCfg->apPeriConfig[nIndex2]->nI2Caddr;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].nNumPeriConfigUnit = pSlvCfg->apPeriConfig[nIndex2]->nNumPeriConfigUnit;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].paPeriConfigUnit = pSlvCfg->apPeriConfig[nIndex2]->paPeriConfigUnit;
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nNumConfig].ePeriDeviceInterface = pSlvCfg->apPeriConfig[nIndex2]->ePeriDeviceInterface;
				aPeriDownloadTable[nIndex1 + 1u].nNumConfig++;
			}
			else
			{
				aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nIndex2].bActive = 0u;
			}
		}

		for (nIndex2 = pSlvCfg->nNumPeriDevice; nIndex2 < (a2b_UInt8)ADI_A2B_MAX_DEVICES_PER_NODE; nIndex2++)
		{
			aPeriDownloadTable[nIndex1 + 1u].aDeviceConfig[nIndex2].bActive = 0u;
		}
	}

}


/*!****************************************************************************
*
*  \b              adi_a2b_CheckforDefault
*
*  Helper routine to check if a node register is configured for default value and
*  if so sets the HAS status in the bdd_node structure as 'false' so that stack
*  does not configure the register during discovery.
*
*  \param          [in]    pNode Pointer to Node of the BDD structure
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_CheckforDefault(bdd_Node* pNode)
{
	uint32_t i;
	a2b_Bool isAd2430_8 = A2B_FALSE;
	a2b_Bool isAd243x = A2B_FALSE;

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product);
	isAd243x = A2B_IS_AD243X_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product);

	/* Control registers */
	A2B_UPDATE_HAS(pNode->ctrlRegs, bcdnslots, 0);
	A2B_UPDATE_HAS(pNode->ctrlRegs, ldnslots, 0);
	A2B_UPDATE_HAS(pNode->ctrlRegs, lupslots, 0);
	A2B_UPDATE_HAS(pNode->ctrlRegs, dnslots, 0);
	A2B_UPDATE_HAS(pNode->ctrlRegs, upslots, 0);
	A2B_UPDATE_HAS(pNode->ctrlRegs, respcycs, 0x40);
	A2B_UPDATE_HAS(pNode->ctrlRegs, slotfmt, 0);
	A2B_UPDATE_HAS(pNode->ctrlRegs, suscfg, 0);
	A2B_UPDATE_HAS(pNode->ctrlRegs, datctl, 0);

	/* Interupt registers */
	A2B_UPDATE_HAS(pNode->intRegs, intmsk0, 0);
	A2B_UPDATE_HAS(pNode->intRegs, intmsk1, 0);
	A2B_UPDATE_HAS(pNode->intRegs, intmsk2, 0);
	A2B_UPDATE_HAS(pNode->intRegs, becctl, 0);

	/* I2C/i2s Registers */
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2ccfg, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, pllctl, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2sgcfg, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2scfg, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2srate, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2stxoffset, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2srxoffset, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, syncoffset, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, pdmctl, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, errmgmt, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2srrate, 1);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2srrctl, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2srrsoffs, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, pdmctl2, 0);
	A2B_UPDATE_HAS(pNode->i2cI2sRegs, i2stest, 0);

	/* PIn IO */
	A2B_UPDATE_HAS(pNode->pinIoRegs, clkcfg, 0);
	A2B_UPDATE_HAS(pNode->pinIoRegs, gpiooen, 0);
	A2B_UPDATE_HAS(pNode->pinIoRegs, gpioien, 0);
	A2B_UPDATE_HAS(pNode->pinIoRegs, pinten, 0);
	A2B_UPDATE_HAS(pNode->pinIoRegs, pintinv, 0);
	A2B_UPDATE_HAS(pNode->pinIoRegs, pincfg, 0x01);
	A2B_UPDATE_HAS(pNode->pinIoRegs, gpiodat, 0);
	A2B_UPDATE_HAS(pNode->pinIoRegs, clk1cfg, 0);
	A2B_UPDATE_HAS(pNode->pinIoRegs, clk2cfg, 0);

	/* Slot Enahnce */
	A2B_UPDATE_HAS(pNode->slotEnh, upmask0, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, upmask1, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, upmask2, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, upmask3, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, upoffset, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, dnmask0, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, dnmask1, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, dnmask2, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, dnmask3, 0);
	A2B_UPDATE_HAS(pNode->slotEnh, dnoffset, 0);

	/* GPIO over distance */
	A2B_UPDATE_HAS(pNode->gpioDist, gpioden, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod0msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod1msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod2msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod3msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod4msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod5msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod6msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiod7msk, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpioddat, 0);
	A2B_UPDATE_HAS(pNode->gpioDist, gpiodinv, 0);

	/* Mail Box */
	A2B_UPDATE_HAS(pNode->mbox, mbox0ctl, 0);
	A2B_UPDATE_HAS(pNode->mbox, mbox1ctl, 0);

	/* Tuning Registers */
	if ((!isAd2430_8) && (!isAd243x))
	{
		A2B_UPDATE_HAS(pNode->tuningRegs, txactl, 0);
		A2B_UPDATE_HAS(pNode->tuningRegs, txbctl, 0);
	}

	A2B_UPDATE_HAS(pNode->tuningRegs, txctl, 0);

	if (!isAd2430_8)
	{

		/* Demeter */

	/* SPI */
		A2B_UPDATE_HAS(pNode->spiRegs, spicfg, 0);
		A2B_UPDATE_HAS(pNode->spiRegs, spickdiv, 0);
		A2B_UPDATE_HAS(pNode->spiRegs, spifdsize, 0);
		A2B_UPDATE_HAS(pNode->spiRegs, spifdtarg, 0);
		A2B_UPDATE_HAS(pNode->spiRegs, spipicfg, 0);
		A2B_UPDATE_HAS(pNode->spiRegs, spiint, 0);
		A2B_UPDATE_HAS(pNode->spiRegs, spimsk, 0);

		A2B_UPDATE_HAS(pNode->dataTunnelRegs, dtcfg, 0);
		A2B_UPDATE_HAS(pNode->dataTunnelRegs, dtslots, 0);
		A2B_UPDATE_HAS(pNode->dataTunnelRegs, dtndnoffs, 0);
		A2B_UPDATE_HAS(pNode->dataTunnelRegs, dtnupoffs, 0);
	}
	/* Crossbar */
	for (i = 0u; i < pNode->i2sCrossbarRegs.txcrossbar_count; i++)
	{
		A2B_UPDATE_HAS_ARR(pNode->i2sCrossbarRegs, txcrossbar, i, 0);
	}
	for (i = 0u; i < pNode->i2sCrossbarRegs.rxmask_count; i++)
	{
		A2B_UPDATE_HAS_ARR(pNode->i2sCrossbarRegs, rxmask, i, 0);
	}

	/* PWM */
	A2B_UPDATE_HAS(pNode->pwmRegs, pwmcfg, 0);
	A2B_UPDATE_HAS(pNode->pwmRegs, pwmfreq, 0);
	A2B_UPDATE_HAS(pNode->pwmRegs, pwmblink1, 0);
	A2B_UPDATE_HAS(pNode->pwmRegs, pwmblink2, 0);
	A2B_UPDATE_HAS(pNode->pwmRegs, pwmblink2, 0);
	for (i = 0u; i < pNode->pwmRegs.pwmval_count; i++)
	{
		A2B_UPDATE_HAS_ARR(pNode->pwmRegs, pwmval, i, 0);
	}
	for (i = 0u; i < pNode->pwmRegs.pwmoe_count; i++)
	{
		A2B_UPDATE_HAS_ARR(pNode->pwmRegs, pwmoe, i, 0);
	}

	if (!isAd2430_8)
	{
		/* VMTR */
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_ven, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_inten, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_mxstat, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_mnstat, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmax0, 0xFF);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmin0, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmax1, 0xFF);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmin1, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmax2, 0xFF);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmin2, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmax3, 0xFF);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmin3, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmax4, 0xFF);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmin4, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmax5, 0xFF);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmin5, 0);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmax6, 0xFF);
		A2B_UPDATE_HAS(pNode->vmtrRegs, vmtr_vmin6, 0);
	}

}

/*!****************************************************************************
*
*  \b              adi_a2b_CheckforAutoConfig
*
*  Helper routine to clear the HAS status of relevant registers if auto configuration
*  via EEPROM is enabled for a Node.
*
*  \param          [in]    pNode 		Pointer to Node of the BDD structure
*  \param          [in]    bAutoConfig  Auto Configuration Flag
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_CheckforAutoConfig(bdd_Node* pNode, bool bAutoConfig)
{
	if (bAutoConfig == A2B_TRUE)
	{
		/* Control registers */
		A2B_CLEAR_HAS(pNode->ctrlRegs, bcdnslots);
		A2B_CLEAR_HAS(pNode->ctrlRegs, ldnslots);
		A2B_CLEAR_HAS(pNode->ctrlRegs, lupslots);


		/* I2C/i2s Registers */
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, i2ccfg);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, pllctl);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, i2sgcfg);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, i2scfg);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, i2srate);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, i2stxoffset);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, i2srxoffset);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, syncoffset);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, pdmctl);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, errmgmt);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, pdmctl2);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, i2stest);
		/* PIn IO */
		A2B_CLEAR_HAS(pNode->pinIoRegs, gpiodat);
		A2B_CLEAR_HAS(pNode->pinIoRegs, gpiooen);
		A2B_CLEAR_HAS(pNode->pinIoRegs, gpioien);
		A2B_CLEAR_HAS(pNode->pinIoRegs, pinten);
		A2B_CLEAR_HAS(pNode->pinIoRegs, pintinv);
		A2B_CLEAR_HAS(pNode->pinIoRegs, pincfg);

	}


}


/*!****************************************************************************
*
*  \b              adi_a2b_SetforAllReg_243x_master
*
*  Helper routine to consider all the valid registers of AD243x master for configuration
*
*  \param          [in]    pNode Pointer to Master Node of the BDD structure
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_SetforAllReg_243x_master(bdd_Node* pNode)
{
	a2b_Bool isAd2430_8 = A2B_FALSE;

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product);

	adi_a2b_SetforAllReg_242x_master(pNode); /* call the 242x set */

	/* TODO check if this is only for a few product revisions */
	A2B_SET_HAS(pNode->i2cI2sRegs, pllctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl2);

	/* Tuning Registers */
	pNode->has_tuningRegs = A2B_TRUE;
	A2B_SET_HAS(pNode->tuningRegs, txctl);

	if (!isAd2430_8)
	{
		/* SPI */
		pNode->has_spiRegs = A2B_TRUE;
		A2B_SET_HAS(pNode->spiRegs, spicfg);
		A2B_SET_HAS(pNode->spiRegs, spickdiv);
		A2B_SET_HAS(pNode->spiRegs, spifdsize);
		A2B_SET_HAS(pNode->spiRegs, spifdtarg);
		A2B_SET_HAS(pNode->spiRegs, spipicfg);
		A2B_SET_HAS(pNode->spiRegs, spiint);
		A2B_SET_HAS(pNode->spiRegs, spimsk);

		/* Data Tunnel */
		pNode->has_dataTunnelRegs = A2B_TRUE;
		A2B_SET_HAS(pNode->dataTunnelRegs, dtcfg);
		A2B_SET_HAS(pNode->dataTunnelRegs, dtslots);
		A2B_SET_HAS(pNode->dataTunnelRegs, dtndnoffs);
		A2B_SET_HAS(pNode->dataTunnelRegs, dtnupoffs);

	}
	/* PWM */
	pNode->has_pwmRegs = A2B_TRUE;
	A2B_SET_HAS(pNode->pwmRegs, pwmcfg);
	A2B_SET_HAS(pNode->pwmRegs, pwmfreq);
	A2B_SET_HAS(pNode->pwmRegs, pwmblink1);
	A2B_SET_HAS(pNode->pwmRegs, pwmblink2);
	A2B_SET_HAS(pNode->pwmRegs, pwmblink2);
	A2B_SET_HAS(pNode->pwmRegs, pwmval);
	A2B_SET_HAS(pNode->pwmRegs, pwmoe);

	/* Cross bar */
	pNode->has_i2sCrossbarRegs = A2B_TRUE;
	A2B_SET_HAS(pNode->i2sCrossbarRegs, rxmask);
	A2B_SET_HAS(pNode->i2sCrossbarRegs, txcrossbar);

	if (!isAd2430_8)
	{

		/* VMTR */
		pNode->has_vmtrRegs = A2B_TRUE;
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_ven);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_inten);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_mxstat);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_mnstat);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax0);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin0);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax1);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin1);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax2);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin2);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax3);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin3);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax4);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin4);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax5);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin5);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax6);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin6);
	}
}

/*!****************************************************************************
*
*  \b              adi_a2b_SetforAllReg_243x_slave
*
*  Helper routine to consider all the valid registers of AD243x slave for configuration
*
*  \param          [in]    pNode Pointer to Slave Node of the BDD structure
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_SetforAllReg_243x_slave(bdd_Node* pNode)
{
	a2b_Bool isAd2430_8 = A2B_FALSE;

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product);

	adi_a2b_SetforAllReg_242x_slave(pNode);

	/* TODO check if this is only for a few product revisions */
	A2B_SET_HAS(pNode->i2cI2sRegs, pllctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl2);

	/* Tuning Registers */
	pNode->has_tuningRegs = A2B_TRUE;
	A2B_SET_HAS(pNode->tuningRegs, txctl);

	if (!isAd2430_8)
	{
		/* SPI */
		pNode->has_spiRegs = A2B_TRUE;
		A2B_SET_HAS(pNode->spiRegs, spicfg);
		A2B_SET_HAS(pNode->spiRegs, spickdiv);
		A2B_SET_HAS(pNode->spiRegs, spifdsize);
		A2B_SET_HAS(pNode->spiRegs, spifdtarg);
		A2B_SET_HAS(pNode->spiRegs, spipicfg);
		A2B_SET_HAS(pNode->spiRegs, spiint);
		A2B_SET_HAS(pNode->spiRegs, spimsk);

		/* Data Tunnel */
		pNode->has_dataTunnelRegs = A2B_TRUE;
		A2B_SET_HAS(pNode->dataTunnelRegs, dtcfg);
		A2B_SET_HAS(pNode->dataTunnelRegs, dtslots);
		A2B_SET_HAS(pNode->dataTunnelRegs, dtndnoffs);
		A2B_SET_HAS(pNode->dataTunnelRegs, dtnupoffs);
	}
	/* PWM */
	pNode->has_pwmRegs = A2B_TRUE;
	A2B_SET_HAS(pNode->pwmRegs, pwmcfg);
	A2B_SET_HAS(pNode->pwmRegs, pwmfreq);
	A2B_SET_HAS(pNode->pwmRegs, pwmblink1);
	A2B_SET_HAS(pNode->pwmRegs, pwmblink2);
	A2B_SET_HAS(pNode->pwmRegs, pwmblink2);
	A2B_SET_HAS(pNode->pwmRegs, pwmval);
	A2B_SET_HAS(pNode->pwmRegs, pwmoe);

	/* Cross bar */
	pNode->has_i2sCrossbarRegs = A2B_TRUE;
	A2B_SET_HAS(pNode->i2sCrossbarRegs, rxmask);
	A2B_SET_HAS(pNode->i2sCrossbarRegs, txcrossbar);

	if (!isAd2430_8)
	{
		/* VMTR */
		pNode->has_vmtrRegs = A2B_TRUE;
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_ven);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_inten);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_mxstat);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_mnstat);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax0);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin0);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax1);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin1);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax2);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin2);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax3);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin3);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax4);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin4);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax5);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin5);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmax6);
		A2B_SET_HAS(pNode->vmtrRegs, vmtr_vmin6);
	}
}

/*!****************************************************************************
*
*  \b              adi_a2b_SetforAllReg_242x_master
*
*  Helper routine to consider all the valid registers of AD242x master for configuration
*
*  \param          [in]    pNode Pointer to Master Node of the BDD structure
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_SetforAllReg_242x_master(bdd_Node* pNode)
{
	/* Control registers */
	A2B_SET_HAS(pNode->ctrlRegs, dnslots);
	A2B_SET_HAS(pNode->ctrlRegs, upslots);
	A2B_SET_HAS(pNode->ctrlRegs, respcycs);
	A2B_SET_HAS(pNode->ctrlRegs, slotfmt);
	A2B_SET_HAS(pNode->ctrlRegs, suscfg);
	A2B_SET_HAS(pNode->ctrlRegs, datctl);

	/* Interupt registers */
	A2B_SET_HAS(pNode->intRegs, intmsk0);
	A2B_SET_HAS(pNode->intRegs, intmsk1);
	A2B_SET_HAS(pNode->intRegs, intmsk2);
	A2B_SET_HAS(pNode->intRegs, becctl);

	/* I2C/i2s Registers */
	A2B_SET_HAS(pNode->i2cI2sRegs, i2ccfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2sgcfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2scfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srate);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2stxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, syncoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, errmgmt);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srrctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srrsoffs);


	/* PIn IO */
	A2B_SET_HAS(pNode->pinIoRegs, gpiooen);
	A2B_SET_HAS(pNode->pinIoRegs, gpioien);
	A2B_SET_HAS(pNode->pinIoRegs, pinten);
	A2B_SET_HAS(pNode->pinIoRegs, pintinv);
	A2B_SET_HAS(pNode->pinIoRegs, pincfg);
	A2B_SET_HAS(pNode->pinIoRegs, gpiodat);
	A2B_SET_HAS(pNode->pinIoRegs, clk1cfg);
	A2B_SET_HAS(pNode->pinIoRegs, clk2cfg);


	/* GPIO over distance */
	A2B_SET_HAS(pNode->gpioDist, gpioden);
	A2B_SET_HAS(pNode->gpioDist, gpiod0msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod1msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod2msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod3msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod4msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod5msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod6msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod7msk);
	A2B_SET_HAS(pNode->gpioDist, gpioddat);
	A2B_SET_HAS(pNode->gpioDist, gpiodinv);

	/* Mail Box */
	A2B_SET_HAS(pNode->mbox, mbox0ctl);
	A2B_SET_HAS(pNode->mbox, mbox1ctl);

	/* Set only for AD2428 series */
	if (A2B_IS_AD2428X_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product))
	{
		A2B_SET_HAS(pNode->i2cI2sRegs, pllctl);
		A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl2);
		/* Tuning Registers */
		A2B_SET_HAS(pNode->tuningRegs, txactl);
		A2B_SET_HAS(pNode->tuningRegs, txbctl);
		A2B_SET_HAS(pNode->ctrlRegs, control);
	}
	else
	{
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, pllctl);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, pdmctl2);
		/* Tuning Registers */
		A2B_CLEAR_HAS(pNode->tuningRegs, txactl);
		A2B_CLEAR_HAS(pNode->tuningRegs, txbctl);
		A2B_CLEAR_HAS(pNode->ctrlRegs, control);

	}

}

/*!****************************************************************************
*
*  \b              adi_a2b_SetforAllReg_242x_slave
*
*  Helper routine to consider all the valid registers of AD241x slave for configuration
*
*  \param          [in]    pNode Pointer to Slave Node of the BDD structure
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_SetforAllReg_242x_slave(bdd_Node* pNode)
{
	/* Control registers */
	A2B_SET_HAS(pNode->ctrlRegs, bcdnslots);
	A2B_SET_HAS(pNode->ctrlRegs, ldnslots);
	A2B_SET_HAS(pNode->ctrlRegs, lupslots);
	A2B_SET_HAS(pNode->ctrlRegs, dnslots);
	A2B_SET_HAS(pNode->ctrlRegs, upslots);
	A2B_SET_HAS(pNode->ctrlRegs, respcycs);
	A2B_SET_HAS(pNode->ctrlRegs, slotfmt);
	A2B_SET_HAS(pNode->ctrlRegs, suscfg);

	/* Interupt registers */
	A2B_SET_HAS(pNode->intRegs, intmsk0);
	A2B_SET_HAS(pNode->intRegs, intmsk1);
	A2B_SET_HAS(pNode->intRegs, intmsk2);
	A2B_SET_HAS(pNode->intRegs, becctl);

	/* I2C/i2s Registers */
	A2B_SET_HAS(pNode->i2cI2sRegs, i2ccfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2sgcfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2scfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srate);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2stxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, syncoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, errmgmt);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srrate);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srrsoffs);


	/* PIn IO */
	A2B_SET_HAS(pNode->pinIoRegs, gpiooen);
	A2B_SET_HAS(pNode->pinIoRegs, gpioien);
	A2B_SET_HAS(pNode->pinIoRegs, pinten);
	A2B_SET_HAS(pNode->pinIoRegs, pintinv);
	A2B_SET_HAS(pNode->pinIoRegs, pincfg);
	A2B_SET_HAS(pNode->pinIoRegs, gpiodat);
	A2B_SET_HAS(pNode->pinIoRegs, clk1cfg);
	A2B_SET_HAS(pNode->pinIoRegs, clk2cfg);

	/* Slot Enahnce */
	A2B_SET_HAS(pNode->slotEnh, upmask0);
	A2B_SET_HAS(pNode->slotEnh, upmask1);
	A2B_SET_HAS(pNode->slotEnh, upmask2);
	A2B_SET_HAS(pNode->slotEnh, upmask3);
	A2B_SET_HAS(pNode->slotEnh, upoffset);
	A2B_SET_HAS(pNode->slotEnh, dnmask0);
	A2B_SET_HAS(pNode->slotEnh, dnmask1);
	A2B_SET_HAS(pNode->slotEnh, dnmask2);
	A2B_SET_HAS(pNode->slotEnh, dnmask3);
	A2B_SET_HAS(pNode->slotEnh, dnoffset);

	/* GPIO over distance */
	A2B_SET_HAS(pNode->gpioDist, gpioden);
	A2B_SET_HAS(pNode->gpioDist, gpiod0msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod1msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod2msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod3msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod4msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod5msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod6msk);
	A2B_SET_HAS(pNode->gpioDist, gpiod7msk);
	A2B_SET_HAS(pNode->gpioDist, gpioddat);
	A2B_SET_HAS(pNode->gpioDist, gpiodinv);

	/* Mail Box */
	A2B_SET_HAS(pNode->mbox, mbox0ctl);
	A2B_SET_HAS(pNode->mbox, mbox1ctl);

	/* Set only for AD2428 series */
	if (A2B_IS_AD2428X_CHIP(pNode->nodeDescr.vendor, pNode->nodeDescr.product))
	{
		A2B_SET_HAS(pNode->i2cI2sRegs, pllctl);
		A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl2);

		/* Tuning Registers */
		A2B_SET_HAS(pNode->tuningRegs, txactl);
		A2B_SET_HAS(pNode->tuningRegs, txbctl);

		A2B_SET_HAS(pNode->ctrlRegs, control);
	}
	else
	{
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, pllctl);
		A2B_CLEAR_HAS(pNode->i2cI2sRegs, pdmctl2);

		/* Tuning Registers */
		A2B_CLEAR_HAS(pNode->tuningRegs, txactl);
		A2B_CLEAR_HAS(pNode->tuningRegs, txbctl);
		A2B_CLEAR_HAS(pNode->ctrlRegs, control);
	}

}



/*!****************************************************************************
*
*  \b              adi_a2b_SetforAllReg_241x_slave
*
*  Helper routine to consider all the valid registers of AD241x slave for configuration
*
*  \param          [in]    pNode Pointer to Slave Node of the BDD structure
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_SetforAllReg_241x_slave(bdd_Node* pNode)
{

	/* Control registers */
	A2B_SET_HAS(pNode->ctrlRegs, bcdnslots);
	A2B_SET_HAS(pNode->ctrlRegs, ldnslots);
	A2B_SET_HAS(pNode->ctrlRegs, lupslots);
	A2B_SET_HAS(pNode->ctrlRegs, dnslots);
	A2B_SET_HAS(pNode->ctrlRegs, upslots);
	A2B_SET_HAS(pNode->ctrlRegs, respcycs);
	A2B_SET_HAS(pNode->ctrlRegs, slotfmt);

	/* Interupt registers */
	A2B_SET_HAS(pNode->intRegs, intmsk0);
	A2B_SET_HAS(pNode->intRegs, intmsk1);
	A2B_SET_HAS(pNode->intRegs, intmsk2);
	A2B_SET_HAS(pNode->intRegs, becctl);

	/* I2C/i2s Registers */
	A2B_SET_HAS(pNode->i2cI2sRegs, i2ccfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, pllctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2sgcfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2scfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srate);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2stxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, syncoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, errmgmt);

	/* PIn IO */
	A2B_SET_HAS(pNode->pinIoRegs, clkcfg);
	A2B_SET_HAS(pNode->pinIoRegs, gpiooen);
	A2B_SET_HAS(pNode->pinIoRegs, gpioien);
	A2B_SET_HAS(pNode->pinIoRegs, pinten);
	A2B_SET_HAS(pNode->pinIoRegs, pintinv);
	A2B_SET_HAS(pNode->pinIoRegs, pincfg);
	A2B_SET_HAS(pNode->pinIoRegs, gpiodat);


}


/*!****************************************************************************
*
*  \b              adi_a2b_SetforAllReg_241x_master
*
*  Helper routine to consider all the valid registers of AD241x master for configuration
*
*  \param          [in]    pNode Pointer to Master Node of the BDD structure
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void adi_a2b_SetforAllReg_241x_master(bdd_Node* pNode)
{

	/* Control registers */
	A2B_SET_HAS(pNode->ctrlRegs, dnslots);
	A2B_SET_HAS(pNode->ctrlRegs, upslots);
	A2B_SET_HAS(pNode->ctrlRegs, respcycs);
	A2B_SET_HAS(pNode->ctrlRegs, slotfmt);

	/* Interupt registers */
	A2B_SET_HAS(pNode->intRegs, intmsk0);
	A2B_SET_HAS(pNode->intRegs, intmsk1);
	A2B_SET_HAS(pNode->intRegs, intmsk2);
	A2B_SET_HAS(pNode->intRegs, becctl);

	/* I2C/i2s Registers */
	A2B_SET_HAS(pNode->i2cI2sRegs, i2ccfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, pllctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2sgcfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2scfg);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srate);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2stxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, i2srxoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, syncoffset);
	A2B_SET_HAS(pNode->i2cI2sRegs, pdmctl);
	A2B_SET_HAS(pNode->i2cI2sRegs, errmgmt);

	/* PIN IO */
	A2B_SET_HAS(pNode->pinIoRegs, clkcfg);
	A2B_SET_HAS(pNode->pinIoRegs, gpiooen);
	A2B_SET_HAS(pNode->pinIoRegs, gpioien);
	A2B_SET_HAS(pNode->pinIoRegs, pinten);
	A2B_SET_HAS(pNode->pinIoRegs, pintinv);
	A2B_SET_HAS(pNode->pinIoRegs, pincfg);
	A2B_SET_HAS(pNode->pinIoRegs, gpiodat);

}



/*!****************************************************************************
*
*  \b              setRegVal
*
*  Helper routine to set value of field in register
*
*  \param          [in]    pnRegVal  pointer to the register value
*  \param          [in]    nFieldVal New value of the field
*  \param          [in]    nMask     Mask for the register field to be updated
*  \param          [in]    nPos      Position of the field to be updated
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void setRegVal(a2b_UInt32* pnRegVal, a2b_UInt8 nFieldVal, a2b_UInt8 nMask, a2b_UInt8 nPos)
{
	a2b_UInt8 nNewVal = *pnRegVal;

	nNewVal = nNewVal & ~nMask;
	nNewVal |= nFieldVal << nPos;

	*pnRegVal = nNewVal;
}
#endif
#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)

#define  A2B_LVL0_EEPROM_BYTES		(A2B_EEPROM_ADDR_OFFSET + 17u)
#define  A2B_LVL0_NUM_CHIAN			(A2B_EEPROM_ADDR_OFFSET + 6u)
#define  A2B_LVL0_LVL1_PTR			(A2B_EEPROM_ADDR_OFFSET + 8u)
#define  A2B_LVL1_NUM_SLAVE			(A2B_EEPROM_ADDR_OFFSET + 6u)

/*!****************************************************************************
 *
 *  \b              a2b_getData
 *
 *  Local function to read either E2PROM or File
 *
 *  \param          [in]    ecb      Ptr to Bus Description Struct
 *  \param          [in]    bdd      decoded BDD (e.g. from a2b_bddDecode)
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         None
 *
 ******************************************************************************/
static a2b_HResult a2b_getData(A2B_ECB* ecb, a2b_UInt16 nWrite,
	const a2b_Byte* wBuf, a2b_UInt16 nRead,
	a2b_Byte* rBuf)
{
	a2b_UInt8 status = 0;
#ifndef A2B_BCF_FROM_SOC_EEPROM
	a2b_UInt16 nOffset;
#endif

#ifdef A2B_BCF_FROM_SOC_EEPROM
	status = a2b_EepromWriteRead(ecb->palEcb.i2chnd, A2B_I2C_EEPROM_ADDR, 2u, wBuf, nRead, rBuf);
#else
	A2B_GET_UINT16_BE(nOffset, wBuf, 0u);
	status = a2b_pal_FileRead(ecb->palEcb.fp, nOffset, nRead, rBuf);
#endif
	return status;
}

/*!****************************************************************************
 *
 *  \b              adi_a2b_fileread
 *
 *  Helper routine to perform file read based on the parameters provided
 *
 *  \param          [in]    ecb      Ptr to Bus Description Struct
 *  \param          [in]    nWrite   Size of the write Buf
 *  \param          [in]    wBuf     Pointer to the write buf storing the pointer to read from.
 *  \param          [in]    nRead    Number of bytes to read.
 *  \param          [in]    rBuf     Pointer to the memory provided by the user to store the data read
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         0 on Success
 *					1 on Failure
 *
 ******************************************************************************/
a2b_HResult adi_a2b_fileread(A2B_ECB* ecb, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead,a2b_Byte* rBuf)
{
	a2b_HResult status = 0;
	status = a2b_getData(ecb, nWrite, wBuf, nRead, rBuf);

	return status;
}


/*!****************************************************************************
 *
 *  \b              a2b_get_bddFrmE2promOrFileIO
 *
 *  Helper routine to Parse the SigmaStudio BCF file to generate the
 *  fields of BDD structure
 *
 *  \param          [in]    ecb      Ptr to Bus Description Struct
 *  \param          [in]    bdd      decoded BDD (e.g. from a2b_bddDecode)
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         None
 *
 ******************************************************************************/
a2b_HResult a2b_get_bddFrmE2promOrFileIO(A2B_ECB* ecb, bdd_Network* bdd_Graph, a2b_UInt8* pBuff, a2b_UInt8 pPeriBuf[], ADI_A2B_NETWORK_CONFIG* pTgtProp)
{
	a2b_UInt8 wBuf[2] = { 0, 0 };
	a2b_UInt8 status = 0;
	a2b_UInt8 nL5TotalSize = 0;
	a2b_UInt8 nNumSlaves;
	a2b_UInt8 nNumChain;
	a2b_UInt16 nBDDLength;
	a2b_UInt8 crc8;
	a2b_UInt16 nChRdIndx, nNodeIdx, nL5Idx;
	a2b_UInt16 nPeriConfigLen;
	a2b_UInt8 nChainIndex = ecb->palEcb.nChainIndex;
	a2b_UInt8 pCustomIdData[A2B_MAX_SIZE_OF_CUSTOM_ID_TO_READ];

	/* Read Level 0  */
	A2B_PUT_UINT16_BE(A2B_EEPROM_ADDR_OFFSET, wBuf, 0u);
	status = a2b_getData(ecb, 2u, wBuf, A2B_LVL0_EEPROM_BYTES, pBuff);
	crc8 = a2b_crc8(pBuff, 0u, A2B_LVL0_EEPROM_BYTES - 1);

	if (pBuff[A2B_LVL0_EEPROM_BYTES - 1] != crc8)
	{
		/* CRC Fail */
		status = 1U;
		return status;
	}
	a2b_UInt8 nSchemaVer = pBuff[1];//Noting the schema version
	nNumChain = pBuff[A2B_LVL0_NUM_CHIAN];

	if (nNumChain > 4)
	{
		/* Not supported */
		status = 1U;
		return status;
	}
	/* Read LEVEL 1*/
	A2B_GET_UINT16_BE(nChRdIndx, pBuff, (A2B_LVL0_LVL1_PTR + (nChainIndex * 2u)));//nChRdIndx has pointer to Chain n
	A2B_PUT_UINT16_BE(nChRdIndx, wBuf, 0u);//wBuf has value pointed to the pointer of chain n

	/* Check CRC */
	status = a2b_getData(ecb, 2u, wBuf, A2B_LVL1_NUM_SLAVE + 1u, pBuff);
	/* Get Number of slaves  */
	nNumSlaves = pBuff[A2B_LVL1_NUM_SLAVE];
	pTgtProp->nSubNodes = nNumSlaves;

	/* Update Target properties */
	pTgtProp->bLineDiagnostics = pBuff[2] & 0x01;
	pTgtProp->bAutoDiscCriticalFault = (pBuff[2] & 0x02u) >> 1u;
	pTgtProp->bAutoRediscOnFault = (pBuff[2] & 0x04u) >> 2u;
	pTgtProp->nAttemptsCriticalFault = pBuff[3];
	pTgtProp->nRediscInterval = ((a2b_UInt16)pBuff[5] << 8u) | pBuff[4];

	nChRdIndx += (A2B_LVL1_NUM_SLAVE + 1u);//storing uint16 value offset , currently pointing to Host peripheral config pointer
	A2B_PUT_UINT16_BE(nChRdIndx, wBuf, 0u);

	/* Total number bytes for peri info */
	nPeriConfigLen = (nNumSlaves + 1u) * 2u;

	/* Read and store Configuration Pointers  */
	status = a2b_getData(ecb, 2u, wBuf, nPeriConfigLen, pPeriBuf);
	nChRdIndx += nPeriConfigLen;
	A2B_PUT_UINT16_BE(nChRdIndx, wBuf, 0u);

	/* Read BDD Length */
	status = a2b_getData(ecb, 2u, wBuf, 2u, pBuff);
	nChRdIndx += 2u;
	A2B_PUT_UINT16_BE(nChRdIndx, wBuf, 0u);

	/* Get BDD length */
	A2B_GET_UINT16_BE(nBDDLength, pBuff, 0u);

	/* Read BDD  */
	status = a2b_getData(ecb, 2u, wBuf, nBDDLength, pBuff);

	/* Decode BDD */
	a2b_bddDecode(pBuff, nBDDLength, bdd_Graph);
	if (nSchemaVer >= 0x03)
	{
		nChRdIndx += nBDDLength + 1;
		A2B_PUT_UINT16_BE(nChRdIndx, wBuf, 0u);

		status = a2b_getData(ecb, 2u, wBuf, ((2 * (nNumSlaves + 1u)) + 1), &pPeriBuf[nPeriConfigLen]);
		nL5TotalSize = pPeriBuf[nPeriConfigLen + (2 * (nNumSlaves + 1u))];
		pTgtProp->nL5Pos = nPeriConfigLen;
		pTgtProp->nL5TotalBytes = nL5TotalSize;
	}
	A2B_GET_UINT16_BE(nL5Idx, pPeriBuf, nPeriConfigLen);
	A2B_PUT_UINT16_BE(nL5Idx, wBuf, 0u);

	status = a2b_getData(ecb, 2u, wBuf, nL5TotalSize, pCustomIdData);
	/* Explicitly ensure node level eeprom is not ignored */
	for (nNodeIdx = 0u; nNodeIdx < nNumSlaves + 1; nNodeIdx++)
	{
		bdd_Graph->nodes[nNodeIdx].ignEeprom = 0;
	}

	return status;

}

/*!****************************************************************************
 *
 *  \b              adi_a2b_extractdata
 *
 *  Helper function to fill the lengths of the Custom ID info per node
 *
 *  \param          [in]    ecb      		Ptr to Bus Description Struct
 *  \param          [in]    pCustomIdInfo   Ptr to Store the Custom ID Info
 *  \param          [in]    pTgtProp   		Ptr to Network configuration & monitoring guidance structure
 *  \param          [in]    pBuff   		Ptr to App Handle Length Custom ID memory, used to store the lengths of Custom ID for every node
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         0 on Success
 *					1 on Failure
 *
 ******************************************************************************/
a2b_HResult adi_a2b_extractdata(A2B_ECB* ecb, a2b_UInt8* pCustomIdInfo, ADI_A2B_NETWORK_CONFIG* pTgtProp, a2b_UInt8 pBuff[])
{
	a2b_UInt8 wBuf[2] = { 0, 0 };
	a2b_UInt16 nL5Idx;
	a2b_UInt8 status = 0;
	a2b_UInt8 nNumOfDevices = 0;
	a2b_UInt8 nIdx = 0;
	a2b_UInt8 nL5Bytes = pTgtProp->nL5TotalBytes;
	a2b_UInt8 pCustomIdData[A2B_MAX_SIZE_OF_CUSTOM_ID_TO_READ];
	A2B_GET_UINT16_BE(nL5Idx, pCustomIdInfo, 0);
	A2B_PUT_UINT16_BE(nL5Idx, wBuf, 0u);

	nNumOfDevices = pTgtProp->nSubNodes + 1;
	status = a2b_getData(ecb, 2u, wBuf, nL5Bytes, pCustomIdData);
	for(int i = 0; i < pTgtProp->nL5TotalBytes; i++)
	{
		//checking for number of devices not equal to 0
		if(nNumOfDevices != 0)
		{
			//checking whether the length of Custom ID for the respective node is 0
			if(pCustomIdData[i] != 35)
			{
				pBuff[nIdx] = pCustomIdData[i];
				i += pCustomIdData[i];//incrementing i by length of Custom ID of that respective node
			}
			//if data at index matches 35 i.e."#", length will be returned 0.
			else
			{
				pBuff[nIdx] = 0;
			}
			nNumOfDevices--;
			nIdx++;
		}
	}

	return status;

}


/*!****************************************************************************
 *
 *  \b              a2b_get_nChainsFrmE2promFileIO
 *
 *  Helper routine to Parse the SigmaStudio BCF file to get the
 *  number of chains in the network
 *
 *  \param          [in]    ecb      		Ptr to Bus Description Struct
 *  \param          [in]    pNumChains      Ptr to Store NumChains in network
 *
 *  \pre            None
 *
 *  \post           None
 *
 *  \return         None
 *
 ******************************************************************************/
a2b_HResult a2b_get_nChainsFrmE2promFileIO(A2B_ECB* ecb, a2b_UInt8* pNumChains)
{
	a2b_UInt8 wBuf[2] = { 0, 0 };
	a2b_UInt8 status = 0;
	a2b_UInt8 nReadBuf[A2B_LVL0_EEPROM_BYTES];

	/* Read Level 0  */
	A2B_PUT_UINT16_BE(A2B_EEPROM_ADDR_OFFSET, wBuf, 0u);
	status = a2b_getData(ecb, 2u, wBuf, A2B_LVL0_EEPROM_BYTES, nReadBuf); //TODO
	pNumChains[0] = nReadBuf[A2B_LVL0_NUM_CHIAN];

	return status;
}
#endif


/**
 @}
*/

/**
 @}
*/
