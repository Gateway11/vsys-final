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
 * \file:   discovery.c
 * \brief:  The implementation of an A2B master plugin discovery process.
 *
 *=============================================================================
 */
/*! \addtogroup Network_Configuration
 *  @{
 */

/** @defgroup Discovery_and_Node_Configuration Discovery and Node Configuration
 *
 * This module discovers the node and configures the nodes based on the BDD configuration
 *
 */

/*! \addtogroup Discovery_and_Node_Configuration
 *  @{
 */

/*======================= I N C L U D E S =========================*/
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
#include "a2b/hwaccess.h"
#include "a2b/timer.h"
#include "a2b/seqchart.h"
#include "a2bplugin-master/plugin.h"
#include "discovery.h"
#include "periphcfg.h"
#include "override.h"
#include "a2b/audio.h"
#include "a2b/gpio.h"
#include "a2b/seqchart.h"
#include "stackctx.h"
#include "stack_priv.h"
#if defined _TESSY_INCLUDES_ || defined A2B_SS_STACK
#include "msg_priv.h"
#include "a2b/msgtypes.h"
#include "timer_priv.h"
#endif	/* _TESSY_INCLUDES_ */

/*======================= D E F I N E S ===========================*/

#define A2B_MASTER_NODEBDDIDX   (0u)

/** Delay (in msec) for a node discovery */
#define A2B_DISCOVERY_DELAY     (50)

/** Delay (in msec) for a node discovery for a medium or high power bus powered node */
#define A2B_DISCOVERY_DELAY_FOR_MEDHGHPWR_BUSPWRD_NODE_INMSEC		(70U)

/** Delay (in msec) for a High power settings */
#define A2B_HIGH_PWR_SWITCH_DELAY									(100U)

/** Delay (in msec) for a High power CFG4 Open Detect */
#define A2B_HIGH_PWR_CFG4_OPEN_DETECT								(250U)

/* Scenario information used while logging delay during command list generation for TIMER_RESET */
#define A2B_TIMER_RESET_DELAY_SCENARIO								(1U)

/* Scenario information used while logging delay during command list generation for TIMER_DSCVRY */
#define A2B_TIMER_DSCVRY_DELAY_SCENARIO								(2U)

/* Scenario information used while logging delay during command list generation. This is specific for Demeter 0.0C silicon. */
#define A2B_0_0C_HIGH_PWR_SWITCH_DELAY_SCENARIO						(3U)

/* Scenario information used while logging delay during command list generation */
#define A2B_HIGH_PWR_SWITCH_DELAY_SCENARIO							(4U)

/** Delay (in msec) to wait after a software reset. This is re-used for partial node discovery as well */
#define A2B_SW_RESET_DELAY											(25U)

#ifdef A2B_FEATURE_SELF_DISCOVERY
/** Self Discovery Poll Delay(in msec) **/
#define A2B_BSDSTAT_POLL_INTERVAL    (15u)
/** Max number of attempts to read ACTIVE flag **/
#define A2B_BSD_MAX_ATTEMPTS         (10u)
#endif

#define A2B_DEINIT_DSCVREY_END  (1u)
#define A2B_DEINIT_START        (2u)

#define A2B_AD243x_VMTR_OVERFLOW (1u)
#define A2B_AD243x_VMTR_INRANGE  (0u)

/** Define if a search for a plugin to manage a node should be done
 * AFTER the node itself has been completely initialized. If undefined
 * then a search for the managing plugin (including opening the plugin)
 * will occur immediately after the slave node has been discovered but
 * BEFORE the node itself has been initialized.
 */
#if 1
#define FIND_NODE_HANDLER_AFTER_NODE_INIT   (1u)
#else
#undef FIND_NODE_HANDLER_AFTER_NODE_INIT
#endif

#ifdef A2B_QAC
#pragma PRQA_NO_SIDE_EFFECTS a2b_isAd242xChip
#pragma PRQA_NO_SIDE_EFFECTS a2b_isAd243xChip
#pragma PRQA_NO_SIDE_EFFECTS a2b_isAd2430_8_Chip
#endif




#define A2B_CHECK_AND_WRITE(GRP, REG, REGADR)	do{																				\
											if ((A2B_SUCCEEDED(status)) && (plugin->bdd->nodes[nodeIdx].GRP.has_##REG))	\
											{																			\
												wBuf[0] = (a2b_UInt8)A2B_REG_##REGADR;													\
												wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].GRP.REG;			    \
												status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);		\
												i2cCount++;																\
											}																			\
										}while(0)

#define A2B_CHECK_AND_WRITE_ARR(GRP, REG, REGADR, OFFSET)	do{																				\
											if ((A2B_SUCCEEDED(status)) && (plugin->bdd->nodes[nodeIdx].GRP.has_##REG))	\
											{																			\
												wBuf[0] = (a2b_UInt8)(A2B_REG_##REGADR) + OFFSET;													\
												wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].GRP.REG[OFFSET];			    \
												status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);		\
												i2cCount++;																\
											}																			\
										}while(0)

/*======================= L O C A L  P R O T O T Y P E S  =========*/

typedef enum
{
    TIMER_DSCVRY,
    TIMER_RESET,
	TIMER_RE_DSCVRY,
	TIMER_HIGH_PWR_CFG4_OPEN_DETECT
} a2b_dscvryTimer;


/*======================= D A T A  ================================*/


/*======================= M A C R O S =============================*/

#define A2B_HAS_EEPROM( plugin, nodeAddr ) \
        ((a2b_UInt32)((plugin)->discovery.hasEeprom) & ((a2b_UInt32)1u << (a2b_UInt32)(nodeAddr)))

#define A2B_HAS_PLUGIN( plugin, nodeAddr ) \
        ((a2b_UInt32)((plugin)->discovery.hasPlugin) & ((a2b_UInt32)1u << (a2b_UInt32)(nodeAddr)))

#define A2B_NEEDS_PLUGIN_INIT( plugin, nodeAddr ) \
        ((a2b_UInt32)((plugin)->discovery.needsPluginInit) & ((a2b_UInt32)1u << (a2b_UInt32)(nodeAddr)))

#define A2B_HAS_SEARCHED_FOR_HANLDER(plugin, nodeAddr ) \
        ((a2b_UInt32)((plugin)->discovery.hasSearchedForHandler) & ((a2b_UInt32)1u << (a2b_UInt32)(nodeAddr)))

/* Maps slave node address to an internal slave array index */
#define A2B_MAP_SLAVE_ADDR_TO_INDEX(a)  ((a2b_UInt16)(a))
/* Maps slave array index to slave node address */
#define A2B_MAP_SLAVE_INDEX_TO_ADDR(i)  ((a2b_Int16)(i))

/*======================= C O D E =================================*/
static a2b_Bool a2b_dscvryVMTRRead(a2b_Plugin* plugin, a2b_Int16  dscNodeAddr);
static void a2b_dscvryVMTREnable(a2b_Plugin* plugin, a2b_Int16 nNode);
static a2b_Int32 	a2b_dscvryNodeComplete(a2b_Plugin* plugin, a2b_Int16 nodeAddr, a2b_Bool bDoEepromCfg, a2b_UInt32* errCode);
static void 		a2b_dscvryNetComplete(a2b_Plugin* plugin);
static a2b_Bool 	a2b_dscvryPreMasterInit(a2b_Plugin* plugin);
static a2b_Bool 	a2b_dscvryPreSlaveInit(a2b_Plugin* plugin);
static a2b_Int32 	a2b_dscvryReset(a2b_Plugin* plugin);
static void 		a2b_dscvryFindNodeHandler(a2b_Plugin* plugin, a2b_UInt16 slaveNodeIdx);
static void 		a2b_dscvryInitTdmSettings(a2b_Plugin* plugin,a2b_Int16 nodeAddr);
static void 		a2b_dscvryDeinitPluginComplete( struct a2b_Msg* msg, a2b_Bool isCancelled);
static void 		a2b_dscvryInitPluginComplete_NoEeprom(struct a2b_Msg* msg, a2b_Bool  isCancelled);
static void 		a2b_dscvryInitPluginComplete_EepromComplete(struct a2b_Msg* msg, a2b_Bool isCancelled);
static void 		a2b_onDiscTimeout(struct a2b_Timer *timer, a2b_Handle userData);
static void 		a2b_onResetTimeout(struct a2b_Timer *timer, a2b_Handle userData);
static a2b_Bool 	a2b_dscvryStartTimer(a2b_Plugin* plugin, a2b_dscvryTimer type, a2b_Int16 nodeAddr);
static a2b_Bool 	a2b_dscvryNodeInterruptInit(a2b_Plugin* plugin, a2b_Int16 nodeBddIdx);
static const a2b_NodeSignature* a2b_getNodeSignature(a2b_Plugin* plugin, a2b_Int16 nodeAddr);
static a2b_Bool 	a2b_SimpleModeChkNodeConfig(a2b_Plugin* plugin);
static a2b_HResult 	adi_a2b_ReConfigSlot(a2b_Plugin* plugin, a2b_Int16 nodeAddr);
static a2b_HResult 	a2b_FinalMasterSetup(a2b_Plugin* plugin, a2b_Int16 nodeAddr);
static a2b_Bool 	adi_a2b_ConfigureNodePeri(a2b_Plugin* plugin, a2b_Int16 dscNodeAddr);
static a2b_Bool		adi_a2b_ConfigNodeOptimizAdvancedMode(a2b_Plugin* plugin, a2b_Int16 dscNodeAddr);
static a2b_UInt32 	a2b_dscvryInitPlugin(a2b_Plugin* plugin, a2b_Int16  nodeAddr, a2b_MsgCallbackFunc completeCallback);
static void 		a2b_dscvryDeinitPlugin(a2b_Plugin* plugin, a2b_UInt32  mode);
static a2b_HResult 	a2b_ConfigSpreadSpectrum(a2b_Plugin* plugin, a2b_Int16 nodeAddr);
static a2b_Bool		a2b_isAd243xChip(a2b_UInt8 vendorId, a2b_UInt8 productId);
static a2b_Bool		a2b_isAd2430_8_Chip(a2b_UInt8 vendorId, a2b_UInt8 productId);
static a2b_Bool		a2b_isAd242xChipOnward(a2b_UInt8 vendorId, a2b_UInt8 productId);
static a2b_Bool		a2b_isCrossTalkFixApply(a2b_UInt8 vendorId, a2b_UInt8 productId);
static a2b_Bool 	a2b_stackSupportedNode(a2b_UInt8 vendorId, a2b_UInt8 productId, a2b_UInt8 version);
static a2b_Bool 	a2b_dscvryCustomAuthFrmMem(a2b_Plugin* plugin, a2b_NodeSignature nodeSig);
static a2b_Bool 	a2b_dscvryCustomAuthFrmGpio(a2b_Plugin* plugin, a2b_NodeSignature nodeSig);
#ifdef A2B_FEATURE_COMM_CH
static a2b_Bool 	a2b_dscvryNodeMailboxInit (a2b_Plugin* plugin, a2b_Int16 nodeBddIdx);
static a2b_Bool 	a2b_dscvryPostAuthViaCommCh(a2b_Plugin* plugin);
static a2b_Bool 	a2b_dscvryStartCommChAuthTimer(a2b_Plugin* plugin, a2b_UInt16 delay);
static void 		a2b_onCommChAuthTimeout(struct a2b_Timer *timer, a2b_Handle userData);
static a2b_Bool 	a2b_dscvryCustomAuthFrmCommCh(a2b_Plugin* plugin, a2b_NodeSignature nodeSig);
#endif	/* A2B_FEATURE_COMM_CH */

#ifdef A2B_FEATURE_SELF_DISCOVERY
static a2b_Bool     a2b_postSelfDscvryInit(a2b_Plugin* plugin);
static void 		a2b_onSelfDiscTimeout(struct a2b_Timer *timer, a2b_Handle userData);
static a2b_Bool 	a2b_selfDscvryStartTimer(a2b_Plugin* plugin);
static a2b_HResult a2b_populateNodeSig(a2b_Plugin* plugin, a2b_Int16 nodeAddr, a2b_NodeSignature* nodeSig);
#endif
static a2b_Bool		a2b_discvryHighPwrInit(a2b_Plugin* plugin, a2b_Int16 dscNodeAddr, a2b_Int16 dscNodeBddIdx);
static a2b_HResult 	a2b_SpiToSpiPeriConfigSendMsg(a2b_Plugin* plugin, a2b_Int16 nodeAddr);
static void			a2b_SpiToSpiPeriConfigDone(struct a2b_Msg* msg, a2b_Handle userData);
static a2b_UInt16	a2b_getDscvryTimeOutInMs(a2b_Plugin* plugin, a2b_Int16 nodeAddr);
static a2b_Int32	a2b_dscvryWriteMstrI2sgcfgInvSeq(a2b_Plugin* plugin);
static void 		a2b_onReDiscTimeout(struct a2b_Timer *timer, a2b_Handle userData);
static void			a2b_onDiscTimerHghPwrCfg4OpenDetect(struct a2b_Timer *timer, a2b_Handle userData);
static a2b_Bool 	sendNodeDscvryNotification(a2b_Plugin *plugin, a2b_UInt32 discCode, a2b_Int16 dscNodeIdx);
static void         a2b_I2CErrorReported(struct a2b_Msg* msg, a2b_Handle userData);
static void         a2b_ReportI2CError(a2b_Plugin *plugin, a2b_Int16 nodeAddr);
/*!****************************************************************************
*
*  \b              a2b_dscvryInitTdmSettings
*
*  Initialize the master plugins TDM Settings for a specific node.
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
static void
a2b_dscvryInitTdmSettings
    (
    a2b_Plugin*         plugin,
    a2b_Int16           nodeAddr
    )
{
    a2b_Int16           nodeBddIdx = nodeAddr+1;
    const bdd_Node      *bddNodeObj;
    a2b_TdmSettings*    tdmSettings;
    a2b_UInt32          reg;

    (void)a2b_memset( &plugin->pluginTdmSettings, 0, sizeof(a2b_TdmSettings) );

    if ( nodeBddIdx > (a2b_Int16)plugin->bdd->nodes_count- (a2b_Int16)1)
    {
        A2B_DSCVRY_ERROR0( plugin->ctx, "a2b_dscvryInitTdmSettings", 
                           "Invalid nodeAddr" );
        return;
    }

    bddNodeObj  = &plugin->bdd->nodes[nodeBddIdx];
    tdmSettings = &plugin->pluginTdmSettings;

    tdmSettings->networkSampleRate =
                                 ((a2b_UInt16)plugin->bdd->sampleRate & (a2b_UInt16)0xFFFF);

    if ( bddNodeObj->i2cI2sRegs.has_i2srate )
    {
        switch ( (bddNodeObj->i2cI2sRegs.i2srate & A2B_BITM_I2SRATE_I2SRATE) )
        {
        case A2B_ENUM_I2SRATE_1X_SFF:
            tdmSettings->sampleRateMultiplier = 1u;
            break;
        case A2B_ENUM_I2SRATE_2X_SFF:
            tdmSettings->sampleRateMultiplier = 2u;
            break;
        case A2B_ENUM_I2SRATE_4X_SFF:
            tdmSettings->sampleRateMultiplier = 4u;
            break;
        /* TODO: We need to handle the Reduced Rate (RRDIV) enumerated
         * rates introduced by the AD242X chips. This likely means we will
         * need to to enumerate the possible values and include the RRDIV
         * values itself.
         */
        case A2B_ENUM_I2SRATE_SFF_RRDIV:
            tdmSettings->sampleRateMultiplier = 3u;
            break;
        case A2B_ENUM_I2SRATE_SFF_DIV_4:
            tdmSettings->sampleRateMultiplier = 2u;
            break;
        case A2B_ENUM_I2SRATE_SFF_DIV_2:
            tdmSettings->sampleRateMultiplier = 1u;
            break;
        default:
            tdmSettings->sampleRateMultiplier = 1u;
            break;

        }
    }

    switch ( bddNodeObj->i2cI2sRegs.i2sgcfg & A2B_BITM_I2SGCFG_TDMMODE )
    {
    case A2B_ENUM_I2SGCFG_TDM2:
        tdmSettings->tdmMode = 2u;
        break;
    case A2B_ENUM_I2SGCFG_TDM4:
        tdmSettings->tdmMode = 4u;
        break;
    case A2B_ENUM_I2SGCFG_TDM8:
        tdmSettings->tdmMode = 8u;
        break;
    case A2B_ENUM_I2SGCFG_TDM16:
        tdmSettings->tdmMode = 16u;
        break;
    case A2B_ENUM_I2SGCFG_TDM32:
        tdmSettings->tdmMode = 32u;
        break;
    default:
        tdmSettings->tdmMode = 2u;
        break;
    }

    reg = bddNodeObj->i2cI2sRegs.i2sgcfg;
    tdmSettings->slotSize    = (reg & A2B_BITM_I2SGCFG_TDMSS ) ? 16u : 32u;
    tdmSettings->halfCycle   = (a2b_Bool)(( reg & A2B_BITM_I2SGCFG_ALT ) ==
                                  A2B_ENUM_I2SGCFG_ALT_EN );
    tdmSettings->prevCycle   = (a2b_Bool)(( reg & A2B_BITM_I2SGCFG_EARLY ) ==
                                  A2B_ENUM_I2SGCFG_EARLY_EN );
    tdmSettings->fallingEdge = (a2b_Bool)(( reg & A2B_BITM_I2SGCFG_INV ) ==
                                  A2B_ENUM_I2SGCFG_INV_EN );

    reg = bddNodeObj->i2cI2sRegs.i2scfg;
    tdmSettings->rx.invertBclk = (a2b_Bool)(( reg & A2B_BITM_I2SCFG_RXBCLKINV ) ==
                                    A2B_ENUM_I2SCFG_RXBCLKINV_EN );
    tdmSettings->rx.interleave = (a2b_Bool)(( reg & A2B_BITM_I2SCFG_RX2PINTL ) ==
                                    A2B_ENUM_I2SCFG_RX2PINTL_EN );
    tdmSettings->rx.pinEnabled = (a2b_UInt8)(( reg & (A2B_BITM_I2SCFG_RX0EN |
                               A2B_BITM_I2SCFG_RX1EN)) >> A2B_BITP_I2SCFG_RX0EN);
    
    if ( bddNodeObj->i2cI2sRegs.has_i2srxoffset )
    {
        switch ( bddNodeObj->i2cI2sRegs.i2srxoffset & 
                 A2B_BITM_I2SRXOFFSET_RXOFFSET )
        {
        case A2B_ENUM_I2SRXOFFSET_00:
            tdmSettings->rx.offset = 0u;
            break;
        case A2B_ENUM_I2SRXOFFSET_62:
            tdmSettings->rx.offset = 62u;
            break;
        case A2B_ENUM_I2SRXOFFSET_63:
            tdmSettings->rx.offset = 63u;
            break;
        default:
            tdmSettings->rx.offset = 0u;
            break;
        }
    }
    
    tdmSettings->tx.invertBclk = (a2b_Bool)(( reg & A2B_BITM_I2SCFG_TXBCLKINV ) ==
                                    A2B_ENUM_I2SCFG_TXBCLKINV_EN );
    tdmSettings->tx.interleave = (a2b_Bool)(( reg & A2B_BITM_I2SCFG_TX2PINTL ) ==
                                    A2B_ENUM_I2SCFG_TX2PINTL_EN );
    tdmSettings->tx.pinEnabled = (a2b_UInt8)((reg & (A2B_BITM_I2SCFG_TX0EN |
                              A2B_BITM_I2SCFG_TX1EN) ) >> A2B_BITP_I2SCFG_TX0EN);

    if ( bddNodeObj->i2cI2sRegs.has_i2stxoffset )
    {
        switch ( bddNodeObj->i2cI2sRegs.i2stxoffset & 
                 A2B_BITM_I2STXOFFSET_TXOFFSET )
        {
        case A2B_ENUM_I2STXOFFSET_TXOFFSET_00:
            tdmSettings->tx.offset = 0u;
            break;
        case A2B_ENUM_I2STXOFFSET_TXOFFSET_01:
            tdmSettings->tx.offset = 1u;
            break;
        case A2B_ENUM_I2STXOFFSET_TXOFFSET_62:
            tdmSettings->tx.offset = 62u;
            break;
        case A2B_ENUM_I2STXOFFSET_TXOFFSET_63:
            tdmSettings->tx.offset = 63u;
            break;
        default:
            tdmSettings->tx.offset = 0u;
            break;
        }

        tdmSettings->tx.triStateBefore = (a2b_Bool)(( bddNodeObj->i2cI2sRegs.i2stxoffset &
                                           A2B_BITM_I2STXOFFSET_TSBEFORE ) ==
                                           A2B_ENUM_I2STXOFFSET_TSBEFORE_EN );
        tdmSettings->tx.triStateAfter  = (a2b_Bool)(( bddNodeObj->i2cI2sRegs.i2stxoffset &
                                           A2B_BITM_I2STXOFFSET_TSAFTER ) ==
                                           A2B_ENUM_I2STXOFFSET_TSAFTER_EN );
    }
} /* a2b_dscvryInitTdmSettings */


/*!****************************************************************************
*
*  \b              a2b_dscvryDeinitPluginComplete
*
*  Callback when A2B_MSGREQ_PLUGIN_PERIPH_DEINIT has completed processing.
*  This can be called from the dscvryEnd or the Start routines, depends on TID.
*
*  \param          [in]    msg          The response to the de-init request.
*
*  \param          [in]    isCancelled  An indication of whether the request
*                                       was cancelled.
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_dscvryDeinitPluginComplete
    (
    struct a2b_Msg* msg,
    a2b_Bool        isCancelled
    )
{
    a2b_Plugin* plugin = (a2b_Plugin*)a2b_msgGetUserData( msg );

    A2B_UNUSED(isCancelled);

    if ( plugin )
    {
        /* One less pending de-initialization response. */
        plugin->discovery.pendingPluginDeinit--;

        if ( msg )
        {
            /* Whether the de-initialization of the node's peripherals failed
             * (or not) for this plugin we'll continue processing but at least
             * trace the result.
             */
            A2B_TRACE3( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
                "%s DeinitPluginComplete(): node=%hd status=0x%lX",
                A2B_MPLUGIN_PLUGIN_NAME,
                &plugin->nodeSig.nodeAddr,
                &((a2b_PluginDeinit*)a2b_msgGetPayload(msg))->resp.status) );
        }

        if ( plugin->discovery.pendingPluginDeinit == 0u )
        {
            if ( A2B_DEINIT_DSCVREY_END == a2b_msgGetTid( msg ) )
            {
                a2b_dscvryEnd( plugin, 
                               plugin->discovery.discoveryCompleteCode );
            }
            else
            {
                (void)a2b_dscvryReset( plugin );

                /* On A2B_EXEC_COMPLETE a2b_dscvryEnd already called and 
                 * when continuing we NOP here as well.
                 */
            }
        }
    }

} /* a2b_dscvryDeinitPluginComplete */


/*!****************************************************************************
*
*  \b              a2b_dscvryInitPluginComplete_NoEeprom
*
*  Callback when A2B_MSGREQ_PLUGIN_PERIPH_INIT has completed processing.
*  This is specific to the case when No EEPROM is detected or supported.
*
*  \param          [in]    msg          The response to the peripheral init
*                                       request.
*
*  \param          [in]    isCancelled  An indication of whether or not
*                                       the request was cancelled.
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_dscvryInitPluginComplete_NoEeprom
    (
    struct a2b_Msg* msg,
    a2b_Bool        isCancelled
    )
{
    a2b_Plugin* plugin = (a2b_Plugin*)a2b_msgGetUserData( msg );
    a2b_UInt32 nodeAddr = a2b_msgGetTid( msg );
    a2b_HResult status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_PLUGIN,
                                        A2B_EC_INTERNAL);
    bdd_DiscoveryMode eDscMode;
    a2b_UInt32 nDscvrdNode = 0u;

     A2B_UNUSED(isCancelled);

	 if(plugin == A2B_NULL)
	 {
		return;
	 }

	 eDscMode = a2b_ovrGetDiscMode(plugin);

    if ( msg )
    {
		if ( A2B_HAS_PLUGIN(plugin, nodeAddr) )
		{
			plugin->discovery.pendingPluginInit--;
		}
	    nDscvrdNode = (a2b_UInt32)plugin->discovery.dscNumNodes-(a2b_UInt32)1u;
		/* Get the result of the plugin peripheral initialization */
        status = ((a2b_PluginInit*)a2b_msgGetPayload(msg))->resp.status;
    }

    /* If the plugin peripheral initialization failed then ... */
    if ( A2B_FAILED(status) )
    {
        a2b_dscvryEnd(plugin, A2B_ERR_CODE(status));
    }
    else if ( bdd_DISCOVERY_MODE_MODIFIED == eDscMode )
    {
        (void)a2b_dscvryPreSlaveInit( plugin );

        /* Now we wait for INTTYPE.DSCDONE on success */
    }
    else if((bdd_DISCOVERY_MODE_OPTIMIZED == eDscMode) ||
    		(bdd_DISCOVERY_MODE_ADVANCED == eDscMode))
    {
    	if(nDscvrdNode != nodeAddr)
    	{
    		if((a2b_Int16)nDscvrdNode == A2B_NODEADDR_MASTER)
			{
				a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_OK );
			}
			else
			{
				(void)adi_a2b_ConfigNodeOptimizAdvancedMode(plugin, (a2b_Int16)nDscvrdNode);
			}

    	}
    	else
    	{
			if(nodeAddr == ((a2b_UInt32)plugin->bdd->nodes_count-(a2b_UInt32)2u))
    		{
    			a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_OK );
    		}
		}
    }
    else
    {
        /* Completing the control statement */
    }
} /* a2b_dscvryInitPluginComplete_NoEeprom */


/*!****************************************************************************
*
*  \b              a2b_dscvryInitPluginComplete_EepromComplete
*
*  Callback when A2B_MSGREQ_PLUGIN_PERIPH_INIT has completed processing.
*  This is specific to the case when an EEPROM is found and has been
*  processed to completion.
*
*  \param          [in]    msg          Response for EEPROM completion.
*
*  \param          [in]    isCancelled  Indication of whether the request
*                                       was cancelled.
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_dscvryInitPluginComplete_EepromComplete
    (
    struct a2b_Msg* msg,
    a2b_Bool        isCancelled
    )
{
    a2b_Plugin* plugin = (a2b_Plugin*)a2b_msgGetUserData( msg );
    a2b_UInt32 nodeAddr = a2b_msgGetTid( msg );
    a2b_UInt32 nDscvrdNode = 0u;
    a2b_HResult status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_PLUGIN,
                                        A2B_EC_INTERNAL);
    bdd_DiscoveryMode eDscMode;

    A2B_UNUSED(isCancelled);

    if(plugin == A2B_NULL)
    {
    	return;
    }

    eDscMode = a2b_ovrGetDiscMode(plugin);

    if ( msg )
    {

        if ( A2B_HAS_PLUGIN(plugin, nodeAddr) )
        {
            plugin->discovery.pendingPluginInit--;
        }
        nDscvrdNode = (a2b_UInt32)plugin->discovery.dscNumNodes-(a2b_UInt32)1u;
        /* Get the result of the plugin peripheral initialization */
        status = ((a2b_PluginInit*)a2b_msgGetPayload(msg))->resp.status;
    }

    /* If the plugin peripheral initialization failed then ... */
    if ( A2B_FAILED(status) )
    {
        a2b_dscvryEnd(plugin, A2B_ERR_CODE(status));
    }
    else if ( bdd_DISCOVERY_MODE_MODIFIED == a2b_ovrGetDiscMode(plugin) )
    {
        (void)a2b_dscvryPreSlaveInit( plugin );

        /* If returned true:
         * Now we wait for INTTYPE.DSCDONE on success 
         * Else:
         * Complete
         */
    }
    else if((bdd_DISCOVERY_MODE_OPTIMIZED == eDscMode) ||
       		(bdd_DISCOVERY_MODE_ADVANCED == eDscMode))
    {
    	if(bdd_DISCOVERY_MODE_ADVANCED == eDscMode)
		{
			/* Since EEPROM configuration done for current node,
			 * re-configure slots up until  this node
			 * and re-initialize data flow again
			 *  */
			(void)adi_a2b_ReConfigSlot(plugin, (a2b_Int16)nodeAddr);
			(void)a2b_FinalMasterSetup(plugin, A2B_NODEADDR_MASTER);
		}

    	/**********************************************************************
    	 * Also for the new node that has been discovered configure node ,
    	 * re-configure slots and re-initialize data flow again
    	 **********************************************************************/
       	if(nDscvrdNode != nodeAddr)
       	{
       		if((a2b_Int16)nDscvrdNode == A2B_NODEADDR_MASTER)
			{
				a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_OK );
			}
			else
			{
				(void)adi_a2b_ConfigNodeOptimizAdvancedMode(plugin, (a2b_Int16)nDscvrdNode);
			}
       	}
       	else
       	{
       		if(nodeAddr == ((a2b_UInt32)plugin->bdd->nodes_count-(a2b_UInt32)2u))
       		{
				a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_OK );
       		}
       	}
    }
    else
    {
        /* This will either process the next node or 
         * complete the network processing 
         */

#if !defined(A2B_FEATURE_WAIT_ON_PERIPHERAL_CFG_DELAY) && \
    defined(A2B_FEATURE_EEPROM_OR_FILE_PROCESSING)
        /* This is to avoid wasted calls to a2b_dscvryNetComplete 
         * that waste cycles and confuse the UML output.
         */
        if ((( plugin->discovery.hasEeprom == 0u ) &&
             ( plugin->discovery.pendingPluginInit == 0u ) ) ||
            (( plugin->discovery.hasEeprom ) &&
             ( a2b_periphCfgUsingSync() ) &&
             ( plugin->discovery.pendingPluginInit == 0u ) ) )
#endif
        {
            a2b_dscvryNetComplete( plugin );
        }
    }

} /* a2b_dscvryInitPluginComplete_EepromComplete */


/*!****************************************************************************
*
*  \b              a2b_dscvryDeinitPlugin
*
*  This is called to initiate A2B_MSGREQ_PLUGIN_PERIPH_DEINIT ALL slave plugins.
*
*  \param          [in]    plugin
*  \param          [in]    mode
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_dscvryDeinitPlugin
    (
    a2b_Plugin*         plugin,
    a2b_UInt32          mode
    )
{
    struct a2b_Msg* msg;
    a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_Int16 nodeAddr;

    for ( nodeAddr = 0; 
          nodeAddr < (a2b_Int16)plugin->discovery.dscNumNodes;
          nodeAddr++ )
    {
        if ( A2B_HAS_PLUGIN(plugin, nodeAddr) )
        {
            msg = a2b_msgAlloc( plugin->ctx,
                                A2B_MSG_REQUEST,
                                A2B_MSGREQ_PLUGIN_PERIPH_DEINIT );
            if ( msg )
            {
                a2b_msgSetUserData( msg, (a2b_Handle)plugin, A2B_NULL );
                a2b_msgSetTid( msg, mode );
                /* Assume failure de-initializing the plugin's peripherals. */
                ((a2b_PluginDeinit*)a2b_msgGetPayload(msg))->resp.status =
                                            A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                            A2B_FAC_PLUGIN,
                                            A2B_EC_INTERNAL);

                status = a2b_msgRtrSendRequest( msg, 
                                         nodeAddr,
                                         &a2b_dscvryDeinitPluginComplete );

                if ( A2B_SUCCEEDED(status) )
                {
                    plugin->discovery.pendingPluginDeinit++;
                }

                /* Job executor now owns the message, 
                 * or free on error 
                 */
                (void)a2b_msgUnref( msg );
            }
        }
    }

} /* a2b_dscvryDeinitPlugin */


/*!****************************************************************************
*
*  \b              a2b_dscvryInitPlugin
*
*  This is called to initiate A2B_MSGREQ_PLUGIN_PERIPH_INIT with a specific
*  slave plugin.
*
*  \param          [in]    plugin
*  \param          [in]    nodeAddr
*  \param          [in]    completeCallback
*
*  \pre            None
*
*  \post           None
*
*  \return         [add here]
*
******************************************************************************/
static a2b_UInt32
a2b_dscvryInitPlugin
    (
    a2b_Plugin*         plugin,
    a2b_Int16           nodeAddr,
    a2b_MsgCallbackFunc completeCallback
    )
{
    struct a2b_Msg* msg;
    a2b_HResult     result;
    a2b_PluginInit* pluginInit;
    struct a2b_MsgNotifier 	*notifyIrptI2CError;
    a2b_UInt32 status = A2B_EC_OK;

    if ( A2B_NEEDS_PLUGIN_INIT( plugin, nodeAddr ) )
    {
        /* clear the bit */
        plugin->discovery.needsPluginInit ^= ((a2b_UInt32)1 << (a2b_UInt32)nodeAddr);
    }
    
    if ( A2B_HAS_PLUGIN(plugin, nodeAddr) )
    {
        A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_TRACE1),
                    "InitPlugin: %hd ", &nodeAddr ) );
    }
    else
    {
        A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_TRACE1),
                    "No Plugin, Direct Init: %hd ", &nodeAddr ) );
    }

    msg = a2b_msgAlloc( plugin->ctx,
                        A2B_MSG_REQUEST,
                        A2B_MSGREQ_PLUGIN_PERIPH_INIT );
    if ( A2B_NULL == msg )
    {
        A2B_DSCVRY_ERROR0( plugin->ctx, "a2b_dscvryInitPlugin", 
                           "Cannot allocate message" );
        return (a2b_UInt32)A2B_EC_RESOURCE_UNAVAIL;
    }

    a2b_msgSetUserData( msg, (a2b_Handle)plugin, A2B_NULL );
    a2b_msgSetTid( msg, (a2b_UInt32)nodeAddr );

    if ( !A2B_HAS_PLUGIN(plugin, (a2b_UInt32)nodeAddr) )
    {
        /* Without a plugin this call would fail, so 
         * we call the requested callback now.  This 
         * means the msg is somewhat incomplete and 
         * MUST be used carefully.
         */
        completeCallback( msg , A2B_FALSE /* not cancelled */);
        (void)a2b_msgUnref( msg );

        return (a2b_UInt32)A2B_EC_OK;
    }

    /* Build the INIT payload */
    a2b_dscvryInitTdmSettings( plugin, nodeAddr );
    pluginInit = (a2b_PluginInit*)a2b_msgGetPayload( msg );
    pluginInit->req.tdmSettings = &plugin->pluginTdmSettings;
    pluginInit->req.pNodePeriDeviceConfig = (const void *)plugin->periphCfg.pkgCfg;
    /* Assume peripheral initialization failure */
    pluginInit->resp.status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                                A2B_FAC_PLUGIN,
                                                A2B_EC_INTERNAL);
    
    result = a2b_msgRtrSendRequest(msg, 
                                   nodeAddr, /* destNodeAddr */
                                   completeCallback);

    /* Job executor now owns the message, or free on error */
    (void)a2b_msgUnref( msg );

    if ( A2B_FAILED(result) )
    {
        A2B_DSCVRY_ERROR1( plugin->ctx, "a2b_dscvryInitPlugin", 
                           "Cannot send message (%ld)", &result );
        return (a2b_UInt32)A2B_EC_IO;
    }

	/* Register for notifications on I2C to I2C peripheral configuration done events */
    notifyIrptI2CError = a2b_msgRtrRegisterNotify(plugin->ctx, A2B_MSGREQ_PERIPH_I2C_ERROR , &a2b_I2CErrorReported, A2B_NULL, A2B_NULL);
	if(notifyIrptI2CError != A2B_NULL)
	{
        /* Do nothing */
	}
	else
	{
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL);
	}
    plugin->discovery.pendingPluginInit++;

    return status;

} /* a2b_dscvryInitPlugin */


/*!****************************************************************************
*
*  \b              a2b_dscvryEnd
*
*  Terminate/End the discovery process, and changed the scheduler
*  execution to A2B_EXEC_COMPLETE.  After this call the Job executor will
*  start processing jobs again for the master plugin.
*
*  \param          [in]    plugin   Master plugin record
*  \param          [in]    errCode  A2B_EC_xxx error code of the
*                                   discovery process
*
*  \pre            ONLY called when in a suspended mode on the job queue
*                  while processing discovery.
*
*  \post           Job executor will start processing jobs again for
*                  this master plugin.
*
*  \return         None
*
******************************************************************************/
void
a2b_dscvryEnd
    (
    a2b_Plugin* plugin,
    a2b_UInt32 errCode
    )
{
    a2b_UInt8  wBuf[4u];
    struct a2b_Msg* msg;
    a2b_HResult status = A2B_RESULT_SUCCESS;
    bdd_DiscoveryMode eDscMode;
	a2b_Bool isAd243x = A2B_FALSE;
	a2b_Bool isAd2430_8 = A2B_FALSE;
	a2b_UInt8 Idx;

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
    a2b_Bool periphComplete = A2B_TRUE;
    a2b_UInt8 idx;
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

    /* Stop the previously running timer */
    a2b_timerStop( plugin->timer );

    eDscMode = a2b_ovrGetDiscMode(plugin);

    if ( (a2b_UInt32)A2B_EC_OK == errCode )
    {
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
        /* See if all peripheral config has completed */
        periphComplete = (a2b_Bool)(plugin->discovery.hasEeprom == 0u);
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */
    }
	/* For partial A2B bus operation */
	else if (	((a2b_UInt32)A2B_EC_DISCOVERY_PWR_FAULT		== errCode)	|| 
				((a2b_UInt32)A2B_EC_BUSY					== errCode)	||
				((a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH		== errCode)	||
				((a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_TIMEOUT	== errCode)	||
				((a2b_UInt32)A2B_EC_MSTR_NOT_RUNNING        == errCode))
	{
		/* Only in SIMPLE mode discovery */
		if ((bdd_DISCOVERY_MODE_SIMPLE == eDscMode) && (plugin->discovery.dscNumNodes > 0u))
		{
			/* Only for non-critical faults */
			if ((plugin->pwrDiag.results.intrType != (a2b_Int32)A2B_ENUM_INTTYPE_PWRERR_CS_GND)		&&
				(plugin->pwrDiag.results.intrType != (a2b_Int32)A2B_ENUM_INTTYPE_PWRERR_CS_VBAT)	&&
				(plugin->pwrDiag.results.intrType != (a2b_Int32)A2B_ENUM_INTTYPE_PWRERR_NLS_GND)	&&
				(plugin->pwrDiag.results.intrType != (a2b_Int32)A2B_ENUM_INTTYPE_PWRERR_NLS_VBAT)	)
			{
				plugin->pwrDiag.results.isNonCriticalFault = A2B_TRUE;
				plugin->discovery.discoveryCompleteCode = errCode;
				a2b_dscvryNetComplete(plugin);

				/* Immediately return from this function as a2b_dscvryEnd is again called from a2b_dscvryNetComplete function */
				return;
			}
			else
			{
				plugin->pwrDiag.results.isNonCriticalFault = A2B_FALSE;
				if (plugin->pwrDiag.upstrSelfPwrNode != A2B_NODEADDR_MASTER)
				{
					plugin->discovery.discoveryCompleteCode = errCode;
					a2b_dscvryNetComplete(plugin);

					/* Immediately return from this function as a2b_dscvryEnd is again called from a2b_dscvryNetComplete function */
					return;
				}

			}
		}
	}
    /* Discovery error */
    /* Check to make sure we have not already done this processing */
    else if ( plugin->discovery.discoveryCompleteCode == (a2b_UInt32)A2B_EC_OK )
    {
        /* Setting these to prevent executing this code again */
        plugin->discovery.discoveryComplete = A2B_TRUE;
        plugin->discovery.discoveryCompleteCode = errCode;

        /* On an error we need to clear the hasEeprom tracking */
        plugin->discovery.hasEeprom = 0u;

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
        /* Stop all potential peripheral processing */
        for ( idx = 0u; idx < (a2b_UInt8)A2B_ARRAY_SIZE(plugin->periph.node); idx++ )
        {
            if ( plugin->periph.node[idx].mboxHnd )
            {
                (void)a2b_stackCtxMailboxFlush( plugin->ctx,
                                          plugin->periph.node[idx].mboxHnd );

                (void)a2b_stackCtxMailboxFree( plugin->ctx,
                                         plugin->periph.node[idx].mboxHnd );
                plugin->periph.node[idx].mboxHnd = A2B_NULL;
            }
        }
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

        /* Send the A2B_MSGREQ_PLUGIN_PERIPH_DEINIT to all 
         * discovered plugins.
         */
        a2b_dscvryDeinitPlugin( plugin, A2B_DEINIT_DSCVREY_END );

        if ( plugin->discovery.pendingPluginDeinit )
        {
            /* Wait until Plugin Deinit has been responded to */
            return;
        }
    }
    else
    {
        /* Completing the control statement */
    }

    /* Track that discovery has completed and what the error code is
     * so if the peripheral config is still running we can 
     */
    plugin->discovery.discoveryComplete = A2B_TRUE;

    /* Once an error, always an error.  We don't want to clear the 
     * final error on subsequent calls.
     */
    if ((a2b_UInt32)A2B_EC_OK == plugin->discovery.discoveryCompleteCode )
    {
        plugin->discovery.discoveryCompleteCode = errCode;
    }

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
    /* Only complete this when the peripherals are done */
    if ( !periphComplete )
    {
        return;
    }
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

    /* Are we waiting for plugin de/initialization? */
    if (( plugin->discovery.pendingPluginInit ) || 
        ( plugin->discovery.pendingPluginDeinit ))
    {
        return;
    }
	/* Power is already disabled in case of power faults */
	if(errCode != (a2b_UInt32)A2B_EC_DISCOVERY_PWR_FAULT)
	{
		/* Disable power on the B side of the node */
		a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.swctl);

		wBuf[0] = A2B_REG_SWCTL;
		wBuf[1] = ((nUserSWCTL & A2B_REG_USER_SWCTL) | (a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV));
		wBuf[1] &= (~(A2B_BITM_SWCTL_ENSW));

		status = a2b_regWrite(plugin->ctx, ((a2b_Int16)plugin->discovery.dscNumNodes - (a2b_Int16)1), 2u, wBuf);
	}

    if (( A2B_FAILED(status) ) && 
        ((a2b_UInt32)A2B_EC_OK == plugin->discovery.discoveryCompleteCode))
    {
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_INTERNAL );
        return;
    }

    A2B_DSCVRY_SEQGROUP1( plugin->ctx,
                          "Discovery End (err: 0x%lX)",
                          &plugin->discovery.discoveryCompleteCode );

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
    /* STOP ALL possible peripheral processing timers */
    for (idx = 0u; idx < A2B_ARRAY_SIZE(plugin->periph.node); idx++)
    {
        a2b_timerStop(plugin->periph.node[idx].timer);
        (void)a2b_timerUnref( plugin->periph.node[idx].timer );
        plugin->periph.node[idx].timer = A2B_NULL;

        if ( A2B_NULL != plugin->periph.node[idx].mboxHnd )
        {
            (void)a2b_stackCtxMailboxFree(plugin->ctx,
                                     plugin->periph.node[idx].mboxHnd );
        }
        plugin->periph.node[idx].mboxHnd = A2B_NULL;
    }
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

    if ( ((bdd_DISCOVERY_MODE_MODIFIED == eDscMode) ||
    		(bdd_DISCOVERY_MODE_OPTIMIZED == eDscMode) ||
			(bdd_DISCOVERY_MODE_ADVANCED == eDscMode)) && (errCode != (a2b_UInt32)A2B_EC_DISCOVERY_FAILURE))
    {
        /* Only after the peripherals have completed should we finalize
         * the master node and enable up/downstream audio transmission.
         */
        a2b_dscvryNetComplete( plugin );
    }

    wBuf[0] = A2B_REG_CONTROL;
    /* The AD242X (only) needs to be told it's a Master node BEFORE
     * the PLL locks on the SYNC pin. Once the PLL is locked, setting
     * the MSTR bit is ignored. We set it anyway so it's clear this is
     * the master node.
     */
    wBuf[1] = A2B_ENUM_CONTROL_END_DISCOVERY;
    if ( (plugin->nodeSig.hasSiliconInfo) &&
        ((a2b_isAd242xChipOnward(plugin->nodeSig.siliconInfo.vendorId,
        plugin->nodeSig.siliconInfo.productId))) )
    {
        wBuf[1] |= (a2b_UInt8)A2B_ENUM_CONTROL_MSTR;
    }

	isAd243x   = A2B_IS_AD243X_CHIP  (plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);
	isAd2430_8 = A2B_IS_AD2430_8_CHIP(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);
	if((isAd243x) || (isAd2430_8))
	{
		wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.control & (A2B_ENUM_CONTROL_I2SMSINV | A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
	}
	else
	{
		wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.control & (A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
	}

    (void)a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf );

    plugin->discovery.inDiscovery = A2B_FALSE;

    msg = a2b_msgRtrGetExecutingMsg( plugin->ctx, A2B_MSG_MAILBOX );
    if (msg)
    {
        a2b_NetDiscovery*   dscResp;
        dscResp = (a2b_NetDiscovery*)a2b_msgGetPayload( msg );

        status = A2B_RESULT_SUCCESS;
        if (plugin->discovery.discoveryCompleteCode)
        {
            /* Add the severity and facility */
            status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, 
                                      plugin->discovery.discoveryCompleteCode);
        }

        dscResp->resp.status   = status;
        dscResp->resp.numNodes = plugin->discovery.dscNumNodes;
		dscResp->resp.oLastNodeInfo = plugin->slaveNodeSig[dscResp->resp.numNodes].siliconInfo;

		/* Copy the Custom Node ID to response */
		if ((status & 0xFFFF) == A2B_EC_CUSTOM_NODE_ID_AUTH)
		{
			for (Idx = 0u; Idx<(plugin->discovery.CustomNodeAuthID[1]);Idx++)
			{
				dscResp->resp.CustomNodeAuthID[Idx] = plugin->discovery.CustomNodeAuthID[Idx];
			}
		}

        A2B_SEQ_CHART2( (plugin->ctx,
                A2B_NODE_ADDR_TO_CHART_PLUGIN_ENTITY(plugin->nodeSig.nodeAddr),
                A2B_SEQ_CHART_ENTITY_APP,
                A2B_SEQ_CHART_COMM_REPLY,
                A2B_SEQ_CHART_LEVEL_DISCOVERY,
                "DiscoveryResp: status: 0x%08lX, numNodes: %ld", 
                &dscResp->resp.status, &dscResp->resp.numNodes ));    
    }

    a2b_msgRtrExecUpdate( plugin->ctx, A2B_MSG_MAILBOX, A2B_EXEC_COMPLETE );

    /*
     * Notify listeners that discovery is done
     */
    msg = a2b_msgAlloc( plugin->ctx,
                        A2B_MSG_NOTIFY,
                        A2B_MSGNOTIFY_DISCOVERY_DONE );
    if ( A2B_NULL == msg )
    {
        A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                    "%s a2b_dscvryEnd(): Failed to allocate DISCOVERY_DONE "
                    "notification msg", A2B_MPLUGIN_PLUGIN_NAME));
    }
    else
    {
        a2b_DiscoveryStatus* discStatus;

        discStatus = (a2b_DiscoveryStatus*)a2b_msgGetPayload(msg);
        discStatus->status = status;
        discStatus->numNodes = plugin->discovery.dscNumNodes;
        if ( A2B_FAILED(a2b_msgRtrNotify(msg)) )
        {
            A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                                "%s a2b_dscvryEnd(): Failed to emit "
                                "DISCOVERY_DONE notification",
                                A2B_MPLUGIN_PLUGIN_NAME));
        }
        /* We always unref the notification message on success or failure of
         * notification delivery
         */
        (void)a2b_msgUnref(msg);

    }

    A2B_DSCVRY_RAWDEBUG0( plugin->ctx, "dscvryEnd",
                          "== Discovery Ended ==" );

    A2B_DSCVRY_SEQEND( plugin->ctx );

} /* a2b_dscvryEnd */


/*!****************************************************************************
*
*  \b              a2b_onDiscTimeout
*
*  Handle the discovery timeout.
*
*  \param          [in]    timer
*  \param          [in]    userData
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_onDiscTimeout
    (
    struct a2b_Timer *timer,
    a2b_Handle userData
    )
{
    a2b_Plugin* plugin = (a2b_Plugin*)userData;
    a2b_HResult ret;
    a2b_UInt8   dscNumNodes = plugin->discovery.dscNumNodes;
    a2b_Bool	bNetConfigFlag = A2B_FALSE;
	a2b_Bool 	isAd243x = A2B_FALSE;
	a2b_Bool 	isAd2437 = A2B_FALSE;
	a2b_Bool isAd2430_8 = A2B_FALSE;
	a2b_Bool isSiliconAnApp = A2B_FALSE;
	bdd_A2bCableType CableType;

	const		bdd_Node *bddNodeObj;
	a2b_Byte	wBuf[2u], rBuf[2u];
	a2b_Int16	dscNodeBddIdx = (a2b_Int16)plugin->discovery.dscNumNodes;
	a2b_Int16	dscNodeAddr = dscNodeBddIdx - 1;

    A2B_UNUSED(timer);

    /* Check the interrupt status one more time in 
     * case of a timing race condition.
     */
    ret = a2b_intrQueryIrq( plugin->ctx );

    if (( A2B_SUCCEEDED(ret) ) && ( dscNumNodes != plugin->discovery.dscNumNodes ))
    {
        /* Discovery Done, no error, node already handled */
        return;
    }

	bddNodeObj = &plugin->bdd->nodes[dscNodeBddIdx];
	isAd243x = A2B_IS_AD243X_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
	isAd2430_8 = A2B_IS_AD2430_8_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
	isAd2437 = A2B_IS_AD2437_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
	CableType = plugin->bdd->policy.eCableType;

	if ((isAd243x) || (isAd2430_8))
	{
		wBuf[0] = A2B_REG_SWSTAT2;
		rBuf[0] = 0;
		ret = a2b_regWriteRead(plugin->ctx, dscNodeAddr, 1u, wBuf, 2u, rBuf);
	}

	/* Check if Silicon anomaly is applicable */
	isSiliconAnApp = ((isAd243x && !isAd2437) || /* High power but not AD2437 */
		             (isAd2437 && (CableType == bdd_A2bCableType_UTP))); /* AD2437 but with UTP cable */

	if ((isSiliconAnApp) && (bddNodeObj->nodeSetting.eHighPwrSwitchCfg == bdd_highPwrSwitchCfg_HPSW_CFG_4) && (A2B_SUCCEEDED(ret)) && (plugin->discovery.bAd243xCfg4OpenDetect == false))
	{
		/* Reading SWCTL to get the SWCTL.MODE */
		wBuf[0] = A2B_REG_SWCTL;
		rBuf[0] = 0U;
		ret = a2b_regWriteRead(plugin->ctx, dscNodeAddr, 1u, wBuf, 1u, rBuf);
		if ( ((rBuf[0] & A2B_BITM_SWCTL_MODE) >> A2B_BITP_SWCTL_MODE) != 0x00U)
		{
			/* The above check is being done to check 
			   - Only for the first discovery timeout we need to apply change 17. 
			   - During second time we would have written SWCTL.MODE == 0x00, If timeout happens during second time then we need to report it as timeout. 			   

			   Because of issue in Silicon, when we have a Wrong Port, we get a timeout again and will start the Change 17 with SWCTL = 0x01. But here it will timeout again and causes a loop. 
			   If this is not done the s/w will end up in an infinite discovery timeout -> discovery timeout loop.
			   */
			/* Disable power on the B side of the node */
			a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.swctl);
			wBuf[0] = A2B_REG_SWCTL;
			wBuf[1] = ((nUserSWCTL & A2B_REG_USER_SWCTL) | (a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV));
			wBuf[1] &= (~(A2B_BITM_SWCTL_ENSW));
			ret = a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);

			(void)a2b_dscvryStartTimer(plugin, TIMER_HIGH_PWR_CFG4_OPEN_DETECT, dscNumNodes);
		}
		else
		{
			A2B_DSCVRY_ERROR0(plugin->ctx, "onDiscTimeout", "DISCOVERY TIMEOUT");

			bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);
			if ((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
			{
				plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_BUSY;
				a2b_dscvryNetComplete(plugin);
			}
			else
			{
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_BUSY);
			}
		}
	}
	else if (A2B_FAILED(ret))
	{
		ret = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_POWER_DIAG_FAILURE);
		A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "a2b_pwrDiagStart: failed reading SWSTAT2 register" "errCode=0x%X", &ret));
	}
	else
	{
		A2B_DSCVRY_ERROR0(plugin->ctx, "onDiscTimeout", "DISCOVERY TIMEOUT");

		bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);
		if ((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
		{
			plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_BUSY;
			a2b_dscvryNetComplete(plugin);
		}
		else
		{
			a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_BUSY);
		}
	}
} /* a2b_onDiscTimeout */

#ifdef A2B_FEATURE_SELF_DISCOVERY
/*!****************************************************************************
*
*  \b              a2b_onSelfDiscTimeout
*
*  Handle the self discovery poll timeout.
*
*  \param          [in]    timer
*  \param          [in]    userData
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_onSelfDiscTimeout
(
	struct a2b_Timer *timer,
	a2b_Handle userData
)
{
	a2b_Plugin* plugin = (a2b_Plugin*)userData;
	a2b_Bool bActive = false;
	a2b_UInt8 wBuf[4];
	a2b_UInt8 rBuf[3];
	a2b_HResult status;
	a2b_UInt8 bActiveBSD;

	A2B_UNUSED(timer);
	/* Silicon anamoly in 0.0 AD243x*/	
	if (plugin->nodeSig.siliconInfo.version == 0)
	{
		/*Check the DISCSTAT flag for active*/
		wBuf[0] = A2B_REG_DISCSTAT;
		status = a2b_regWriteRead(plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 1u, rBuf);
		bActiveBSD = (rBuf[0] & A2B_BITM_DISCSTAT_DSCACT) >> A2B_BITP_DISCSTAT_DSCACT;


	}
	else
	{
		/* Check for BSD STAT */
		wBuf[0] = A2B_REG_BSDSTAT;
		status = a2b_regWriteRead(plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 1u, rBuf);
		bActiveBSD = (rBuf[0] & A2B_BITM_BSDSTAT_BSDACTIVE) >> A2B_BITP_BSDSTAT_BSDACTIVE;

	}


	if (A2B_FAILED(status))
	{
		A2B_SEQ_GENERROR0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
			"Error in Post Self Discovery Init");
		A2B_DSCVRY_SEQEND(plugin->ctx);
		a2b_dscvryEnd(plugin, A2B_EC_INTERNAL);
	}

	if (!bActiveBSD)
	{
		/* Call PostSelfDiscInit*/
		if (!a2b_postSelfDscvryInit(plugin))
		{
			A2B_SEQ_GENERROR0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
				"Error in Post Self Discovery Init");
			A2B_DSCVRY_SEQEND(plugin->ctx);
			a2b_dscvryEnd(plugin, A2B_EC_BUSY);
		}
	}
	else
	{
		plugin->discovery.nBSDReadCount++;
		if (plugin->discovery.nBSDReadCount > A2B_BSD_MAX_ATTEMPTS)
		{
			A2B_SEQ_GENERROR0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
				"Reached max number of attempts for reading discovery active state");
			A2B_DSCVRY_SEQEND(plugin->ctx);
			a2b_dscvryEnd(plugin, A2B_EC_DISCOVERY_FAILURE);
		}
		/*Self Discovery still in progress, call the timer again*/
		else if (!a2b_selfDscvryStartTimer(plugin))
		{
			A2B_SEQ_GENERROR0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
				"Failed to init discovery timer");
			A2B_DSCVRY_SEQEND(plugin->ctx);
			a2b_dscvryEnd(plugin, A2B_EC_INTERNAL);

		}
	}

} /* a2b_onSelfDiscTimeout */
#endif

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
/*!****************************************************************************
*
*  \b              a2b_dscvryPeripheralProcessingComplete
*
*  Handler when peripheral processing is complete.
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
void 
a2b_dscvryPeripheralProcessingComplete
    ( 
    a2b_Plugin* plugin,
    a2b_Int16   nodeAddr
    )
{

    if (plugin)
    {
#ifdef FIND_NODE_HANDLER_AFTER_NODE_INIT
        a2b_dscvryFindNodeHandler(plugin,
                                A2B_MAP_SLAVE_ADDR_TO_INDEX(nodeAddr));
#endif
        (void)a2b_dscvryInitPlugin( plugin, 
                              nodeAddr,
                              &a2b_dscvryInitPluginComplete_EepromComplete );
    }

} /* a2b_dscvryPeripheralProcessingComplete */

#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */


/*!****************************************************************************
*
*  \b              a2b_onResetTimeout
*
*  Handle the reset timeout.
*
*  \param          [in]    timer
*  \param          [in]    userData
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_onResetTimeout
    (
    struct a2b_Timer *timer,
    a2b_Handle userData
    )
{
    a2b_Plugin* plugin = (a2b_Plugin*)userData;
#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
	a2b_Int16   nodeAddr = 0;
#endif
    A2B_UNUSED(timer);

    if (plugin->discovery.inDiscovery)
    {
    	if(plugin->discovery.bIsMstrRunning)
    	{
#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
			/* checking if this is AD2437 Network, since mixed TRX NW is not allowed, only main node is checked */
			if (plugin->bdd->nodes[0].nodeDescr.product == 0x37)
			{
				for (nodeAddr = 0; nodeAddr < ((a2b_Int16)plugin->bdd->nodes_count - 1); nodeAddr++)
				{
					if ((plugin->bdd->nodes[nodeAddr + 1].nodeDescr.has_btwoStepDisc) && (plugin->bdd->nodes[nodeAddr + 1].nodeDescr.btwoStepDisc))
					{
						plugin->discovery.bFirstStepDisc[nodeAddr] = true;
					}
				}
			}
#endif

    		/* Initialize the Master Node */
			if (a2b_dscvryPreMasterInit(plugin))
			{
				/* Discovery has started */
				return;
			}
    	}
    	else
    	{
    		A2B_SEQ_GENERROR0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
    						"Failed to get master running interrupt");
			A2B_DSCVRY_SEQEND(plugin->ctx);
			a2b_dscvryEnd(plugin, A2B_EC_MSTR_NOT_RUNNING);
    	}

    }

    /* Must be an error or in the wrong state, quit the complete command */
    a2b_msgRtrExecUpdate( plugin->ctx, A2B_MSG_MAILBOX, A2B_EXEC_COMPLETE );

} /* a2b_onResetTimeout */

static a2b_UInt16 a2b_getDscvryTimeOutInMs(a2b_Plugin* plugin, a2b_Int16 nodeAddr)
{
	a2b_Bool	isAd243xMedHiPwrChip = A2B_FALSE;
	a2b_HResult status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL);
	a2b_UInt32	delay = A2B_DISCOVERY_DELAY;

	status = a2b_isAd243xMedHiPwrUseBdd(plugin, nodeAddr, &isAd243xMedHiPwrChip);
	/* We check for the current node if it is trying to detect a medium or high power bus powered node
	   So,
			isAd243xMedHiPwrChip: is for the current node
			plugin->bdd->nodes[nodeAddr + 2].nodeSetting.bLocalPwrd: is for the next node
	*/
	if ((A2B_SUCCEEDED(status)) & (plugin->bdd->nodes[nodeAddr + 2].nodeSetting.bLocalPwrd == bdd_nodePowerMode_SLAVE_BUS_POWERED) & (isAd243xMedHiPwrChip == A2B_TRUE))
	{
		delay = A2B_DISCOVERY_DELAY_FOR_MEDHGHPWR_BUSPWRD_NODE_INMSEC;
	}
	else if (A2B_FAILED(status))
	{
		/*Log TRACE and Sequence: Cannot return here need to add trace instead of return for debugging */
		A2B_DSCVRY_ERROR0(plugin->ctx, "a2b_getDscvryTimeOutInMs", "Failed to determine the presence of medium or high power bus powered node");
	}
	else
	{
		delay = A2B_DISCOVERY_DELAY;
	}

	return (delay);
}

/*!****************************************************************************
*
*  \b              a2b_dscvryStartTimer
*
*  Generate/Start the discovery timer.
*
*  \param          [in]    plugin
*  \param          [in]    type
*
*  \pre            None
*
*  \post           None
*
*  \return         [add here]
*
******************************************************************************/
static a2b_Bool
a2b_dscvryStartTimer
    (
    a2b_Plugin*     plugin,
    a2b_dscvryTimer type,
	a2b_Int16		nodeAddr
    )
{
    /* Default is for the discovery timer */
    a2b_UInt32 delay = A2B_DISCOVERY_DELAY;
    a2b_TimerFunc timerFunc = &a2b_onDiscTimeout; 
	int scenario;

    if (TIMER_RESET == type)
    {
        /* Setup for the reset timer */
    	if(plugin->bdd->policy.has_discoveryStartDelay == A2B_TRUE)
    	{
    		delay = (plugin->bdd->policy.discoveryStartDelay > 0u ? plugin->bdd->policy.discoveryStartDelay : A2B_SW_RESET_DELAY);
    	}
        timerFunc = &a2b_onResetTimeout;
		scenario = A2B_TIMER_RESET_DELAY_SCENARIO;
    }
	else if (TIMER_DSCVRY == type)
	{
		delay = a2b_getDscvryTimeOutInMs(plugin, nodeAddr);
		scenario = A2B_TIMER_DSCVRY_DELAY_SCENARIO;
	}
	else if (TIMER_RE_DSCVRY == type)
	{
		timerFunc = &a2b_onReDiscTimeout;
		delay = plugin->bdd->policy.nRediscWaitTime;
		scenario = A2B_HIGH_PWR_SWITCH_DELAY_SCENARIO;
	}
	else if (TIMER_HIGH_PWR_CFG4_OPEN_DETECT == type)
	{
		timerFunc = &a2b_onDiscTimerHghPwrCfg4OpenDetect;
		delay = (plugin->bdd->policy.nRediscWaitTime > 250u? plugin->bdd->policy.nRediscWaitTime: A2B_HIGH_PWR_CFG4_OPEN_DETECT);
		scenario = A2B_HIGH_PWR_SWITCH_DELAY_SCENARIO;
	}
	else
	{
		/* Completing control statement */
	}

#ifdef A2B_SS_STACK
	if (plugin->ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc != A2B_NULL)
	{
		/* In Milli second*/
		plugin->ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc(delay, nodeAddr, scenario);
	}
#else
	A2B_UNUSED(scenario);
#endif /* A2B_SS_STACK */

    /* Stop the previously running timer */
    a2b_timerStop( plugin->timer );

    /* Single shot timer */
    a2b_timerSet( plugin->timer, delay, 0u );
    a2b_timerSetHandler(plugin->timer, timerFunc);
    a2b_timerSetData(plugin->timer, plugin);
    a2b_timerStart( plugin->timer );

    return A2B_TRUE;

} /* a2b_dscvryStartTimer */

#ifdef A2B_FEATURE_SELF_DISCOVERY

/*!****************************************************************************
*
*  \b              a2b_selfDscvryStartTimer
*
*  Generate/Start the self discovery timer.
*
*  \param          [in]    plugin

*
*  \pre            None
*
*  \post           None
*
*  \return         [add here]
*
******************************************************************************/
static a2b_Bool
a2b_selfDscvryStartTimer
(
	a2b_Plugin*     plugin
)
{
	/* Default is for the discovery timer */
	a2b_UInt32 delay = A2B_BSDSTAT_POLL_INTERVAL;
	a2b_TimerFunc timerFunc = &a2b_onSelfDiscTimeout;

	/* Stop the previously running timer */
	a2b_timerStop(plugin->timer);

	/* Single shot timer */
	a2b_timerSet(plugin->timer, delay, 0u);
	a2b_timerSetHandler(plugin->timer, timerFunc);
	a2b_timerSetData(plugin->timer, plugin);
	a2b_timerStart(plugin->timer);

	return A2B_TRUE;

} /* a2b_selfDscvryStartTimer */
#endif /*A2B_FEATURE_SELF_DISCOVERY*/





/*!****************************************************************************
*
*  \b              a2b_dscvryNodeInterruptInit
*
*  Initialize a nodes interrupt registers
*
*  \param          [in]    plugin       plugin specific data
* 
*  \param          [in]    nodeBddIdx   0=master, 1=slave0, etc
*
*  \pre            None
*
*  \post           None
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool
a2b_dscvryNodeInterruptInit
    (
    a2b_Plugin* plugin,
    a2b_Int16   nodeBddIdx
    )
{
    a2b_Int16 nodeAddr = nodeBddIdx-1;
    a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_UInt32 nRetVal;

    if ((nodeBddIdx < 0) || (nodeBddIdx >= (a2b_Int16)plugin->bdd->nodes_count))
    {
        return A2B_FALSE;
    }

    A2B_DSCVRY_SEQGROUP0( plugin->ctx, 
                          "Interrupt Registers" );

    if (plugin->bdd->nodes[nodeBddIdx].has_intRegs)
    {
        a2b_UInt8 wBuf[4];
        a2b_UInt32 mask = 0u;

        if (plugin->bdd->nodes[nodeBddIdx].intRegs.has_intmsk0)
        {
            mask |= (plugin->bdd->nodes[nodeBddIdx].intRegs.intmsk0 <<
                     A2B_INTRMASK0_OFFSET);
        }
        if (plugin->bdd->nodes[nodeBddIdx].intRegs.has_intmsk1)
        {
            nRetVal = a2b_ovrApplyIntrActive(plugin, nodeBddIdx,
                    A2B_REG_INTMSK1) << A2B_INTRMASK1_OFFSET;
            mask |= nRetVal;
        }
        if (plugin->bdd->nodes[nodeBddIdx].intRegs.has_intmsk2)
        {
            nRetVal = a2b_ovrApplyIntrActive(plugin, nodeBddIdx,
                    A2B_REG_INTMSK2) << A2B_INTRMASK2_OFFSET;
            mask |= nRetVal;
        }

        /* The last node in the network should *not* have the power fault
         * interrupts enabled since it would always trigger. Since there is
         * no connection on the "B" side of the transceiver it will always
         * report an open-circuit condition.
         */
        if (nodeBddIdx + 1 >= (a2b_Int16)plugin->bdd->nodes_count)
        {
            mask &= (a2b_UInt32)(~((a2b_UInt32)A2B_BITM_INTPND0_PWRERR << (a2b_UInt32)A2B_INTRMASK0_OFFSET));
        }

        A2B_TRACE2( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_TRACE3),
                    "%s NodeInterruptInit(): setIntrMask(0x%lX)",
                    A2B_MPLUGIN_PLUGIN_NAME, &mask) );

        (void)a2b_intrSetMask( plugin->ctx, nodeAddr, mask );

        if (plugin->bdd->nodes[nodeBddIdx].intRegs.has_becctl)
        {
            wBuf[0] = A2B_REG_BECCTL;
            wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeBddIdx].intRegs.becctl;
            status = a2b_regWrite( plugin->ctx, nodeAddr, 2u, wBuf );
            if ( A2B_FAILED(status) )
            {
                A2B_DSCVRY_SEQEND( plugin->ctx );
                return A2B_FALSE;
            }
        }
    }

    A2B_DSCVRY_SEQEND( plugin->ctx );

    return A2B_TRUE;

} /* a2b_dscvryNodeInterruptInit */

/*!****************************************************************************
*
*  \b              a2b_dscvryNodeComplete
*
*  Configuration of master/slave node after discovery
*
*  \param          [in]    plugin        plugin specific data
*  \param          [in]    nodeAddr      -1=master, 0=slave0, 1=slave1, etc
*  \param          [in]    bDoEepromCfg   Configure node from EEPROM
*  \param          [in]    errCode        Pointer to the Error code passed
*  										  from this function
**
*  \pre            Should call this routine only for the master node
*
*  \post           On failures the a2b_dscvryEnd() is expected to be called
*                  outside this function.
*
*  \return         status - sucess (0u)
*							failure (0xFFFFFFFFu)
******************************************************************************/
static a2b_Int32
a2b_dscvryNodeComplete
(
	a2b_Plugin* plugin,
	a2b_Int16   nodeAddr,
	a2b_Bool    bDoEepromCfg,
	a2b_UInt32* errCode
)
{
	a2b_UInt8 wBuf[4];
	a2b_Int16 nodeIdx = nodeAddr + 1;
	a2b_HResult status = A2B_RESULT_SUCCESS;
	a2b_UInt32 i2cCount = 0u;
	a2b_Int32 retCode;
	a2b_Bool bGroupLogged = A2B_FALSE;
	a2b_Bool isAd242xOnward = A2B_FALSE;
	a2b_Bool isAd243x = A2B_FALSE;
	a2b_Bool isAd2430_8 = A2B_FALSE;
	a2b_Bool bRetVal;
	a2b_UInt8 nIndex;
	bdd_DiscoveryMode eDiscMode;

	eDiscMode = a2b_ovrGetDiscMode(plugin);
	A2B_DSCVRY_SEQGROUP1(plugin->ctx,
		"NodeComplete for nodeAddr %hd", &nodeAddr);

	*errCode = (a2b_UInt32)A2B_EC_OK;

#ifndef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
	A2B_UNUSED(bDoEepromCfg);
#endif

	if ((nodeAddr < A2B_NODEADDR_MASTER) ||
		(nodeIdx >= (a2b_Int16)plugin->bdd->nodes_count))
	{
		A2B_DSCVRY_SEQEND(plugin->ctx);
		*errCode = (a2b_UInt32)A2B_EC_INVALID_PARAMETER;
		return A2B_EXEC_COMPLETE_FAIL;
	}

	if ( (A2B_NODEADDR_MASTER == nodeAddr) & (plugin->nodeSig.hasSiliconInfo) )
	{
		isAd242xOnward	= a2b_isAd242xChipOnward(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);
		isAd243x		= a2b_isAd243xChip		(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);
		isAd2430_8      = a2b_isAd2430_8_Chip   (plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);

	}
	else if (plugin->slaveNodeSig[nodeAddr].hasSiliconInfo)
	{
		isAd242xOnward	= a2b_isAd242xChipOnward(plugin->slaveNodeSig[nodeAddr].siliconInfo.vendorId, plugin->slaveNodeSig[nodeAddr].siliconInfo.productId);
		isAd243x		= a2b_isAd243xChip		(plugin->slaveNodeSig[nodeAddr].siliconInfo.vendorId, plugin->slaveNodeSig[nodeAddr].siliconInfo.productId);
		isAd2430_8      = a2b_isAd2430_8_Chip   (plugin->slaveNodeSig[nodeAddr].siliconInfo.vendorId, plugin->slaveNodeSig[nodeAddr].siliconInfo.productId);
	}
	else
	{
		/* Completing the control statement */
	}

    /*---------------------------------------------------*/
    /* Some simple setup prior to further initialization */
    /*---------------------------------------------------*/

    /* NOTE: A2B_REG_NODEADR managed by I2C, no need to set it */
    
    if ( (bdd_DISCOVERY_MODE_SIMPLE == eDiscMode) && (A2B_NODEADDR_MASTER == nodeAddr) )
    {
		if (plugin->pwrDiag.results.isNonCriticalFault == A2B_TRUE)
		{
			/*	During partial discovery:
				- Switch is already disabled in line fault section for master. So, proceed for configuring the master node which is discovered. 
				- CONFIGURING THE MASTER NODE ITSELF MAY NOT BE REQUIRED AS THE NON-CRITICAL FAULT OCCURED BETWEEN MASTER AND SLAVE 0. HOWEVER, KEEPING IT FOR NOW!!
			*/				
		}
		else
		{
			a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.swctl);

			wBuf[0] = A2B_REG_SWCTL;
			/* Enable switch during a fresh discovery */
			wBuf[1] = A2B_BITM_SWCTL_ENSW | (a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV);
			wBuf[1] |= (nUserSWCTL & A2B_REG_USER_SWCTL);
			status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf);
			i2cCount++;
		}  
    }

	/*RESPCYCS in case of Self Discovery*/
#ifdef A2B_FEATURE_SELF_DISCOVERY
	if (plugin->discovery.bIsSelfDiscoveredNtwrk)
	{
		/* Set the response cycle timing */
		wBuf[0] = A2B_REG_RESPCYCS;
		wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].ctrlRegs.respcycs;
		status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);
		i2cCount++;
	}
#endif
#ifdef A2B_SS_STACK
	if (nodeAddr == (plugin->discovery.dscNumNodes - 1))
	{
		if (plugin->ctx->stk->ecb->palEcb.oAppEcbPal.PreCustomNodeConfig != A2B_NULL)
		{
			plugin->ctx->stk->ecb->palEcb.oAppEcbPal.PreCustomNodeConfig(nodeAddr);
		}
   }
#endif
    /*-------------------*/
    /* Control Registers */
    /*-------------------*/

    A2B_DSCVRY_SEQGROUP0( plugin->ctx, "Control Registers" );

	A2B_CHECK_AND_WRITE(ctrlRegs, bcdnslots, BCDNSLOTS);
	A2B_CHECK_AND_WRITE(ctrlRegs, ldnslots, LDNSLOTS);
	A2B_CHECK_AND_WRITE(ctrlRegs, lupslots, LUPSLOTS);

    if(((a2b_UInt32)nodeIdx < (a2b_UInt32)(plugin->bdd->nodes_count - (a2b_UInt32)1u)) && (bdd_DISCOVERY_MODE_ADVANCED != eDiscMode))
    {
		A2B_CHECK_AND_WRITE(ctrlRegs, dnslots, DNSLOTS);
		A2B_CHECK_AND_WRITE(ctrlRegs, upslots, UPSLOTS);
    }
   
    A2B_DSCVRY_SEQEND( plugin->ctx );

    /*-------------------*/
    /* I2C/I2S Registers */
    /*-------------------*/
	A2B_DSCVRY_SEQGROUP0(plugin->ctx, "I2C/I2S Registers");

	A2B_CHECK_AND_WRITE(i2cI2sRegs, i2ccfg, I2CCFG);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, syncoffset, SYNCOFFSET);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, i2stxoffset, I2STXOFFSET);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, i2srxoffset, I2SRXOFFSET);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, i2sgcfg, I2SGCFG);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, i2scfg, I2SCFG);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, i2srate, I2SRATE);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, pdmctl, PDMCTL);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, pdmctl2, PDMCTL2);
	A2B_CHECK_AND_WRITE(i2cI2sRegs, errmgmt, ERRMGMT);

    A2B_DSCVRY_SEQEND(plugin->ctx);    

    /*-------------------*/
    /* Pin I/O Registers */
    /*-------------------*/

    bGroupLogged = A2B_FALSE;
    if ( A2B_SUCCEEDED(status) )
    {
        bGroupLogged = A2B_TRUE;
        A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                              "Pin I/O Registers" );
		if (!isAd242xOnward)
		{
			A2B_CHECK_AND_WRITE(pinIoRegs, clkcfg, CLKCFG);
		}

    }


	A2B_CHECK_AND_WRITE(pinIoRegs, gpiodat, GPIODAT);
	A2B_CHECK_AND_WRITE(pinIoRegs, gpiooen, GPIOOEN);
	A2B_CHECK_AND_WRITE(pinIoRegs, gpioien, GPIOIEN);
	A2B_CHECK_AND_WRITE(pinIoRegs, pinten,  PINTEN);
	A2B_CHECK_AND_WRITE(pinIoRegs, pintinv, PINTINV);
	A2B_CHECK_AND_WRITE(pinIoRegs, pincfg,  PINCFG);


    /*
     * Additional AD242X registers
     */

    /*--------------------------------------*/
    /* Clock Config Registers - AD242X only */
    /*--------------------------------------*/
	if (isAd242xOnward)
	{
		A2B_CHECK_AND_WRITE(pinIoRegs, clk1cfg, CLK1CFG);
		A2B_CHECK_AND_WRITE(pinIoRegs, clk2cfg, CLK2CFG);
	}
    if ( bGroupLogged )
    {
        A2B_DSCVRY_SEQEND(plugin->ctx);
    }

    /*------------------------------------------------------*/
    /* Slot Enhancement Registers (AD242x only, slave only) */
    /*------------------------------------------------------*/

    if ( (A2B_SUCCEEDED(status)) &&
        (isAd242xOnward) &&
        (A2B_NODEADDR_MASTER != nodeAddr) &&
        (plugin->bdd->nodes[nodeIdx].has_slotEnh) )
    {
        A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                              "Slot Enhancement Registers" );

		A2B_CHECK_AND_WRITE(slotEnh, upmask0, UPMASK0);
		A2B_CHECK_AND_WRITE(slotEnh, upmask1, UPMASK1);
		A2B_CHECK_AND_WRITE(slotEnh, upmask2, UPMASK2);
		A2B_CHECK_AND_WRITE(slotEnh, upmask3, UPMASK3);
		A2B_CHECK_AND_WRITE(slotEnh, upoffset, UPOFFSET);

		A2B_CHECK_AND_WRITE(slotEnh, dnmask0,  DNMASK0);
		A2B_CHECK_AND_WRITE(slotEnh, dnmask1,  DNMASK1);
		A2B_CHECK_AND_WRITE(slotEnh, dnmask2,  DNMASK2);
		A2B_CHECK_AND_WRITE(slotEnh, dnmask3,  DNMASK3);
		A2B_CHECK_AND_WRITE(slotEnh, dnoffset, DNOFFSET);  

        A2B_DSCVRY_SEQEND(plugin->ctx);
    }


    /*--------------------------------------------*/
    /* GPIO over Distance Registers (AD242x only) */
    /*--------------------------------------------*/
    if ( (A2B_SUCCEEDED(status)) &&
        (isAd242xOnward) &&
        (plugin->bdd->nodes[nodeIdx].has_gpioDist) )
    {
        A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                              "GPIO Over Distance Registers" );

		A2B_CHECK_AND_WRITE(gpioDist, gpiod0msk, GPIOD0MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpiod1msk, GPIOD1MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpiod2msk, GPIOD2MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpiod3msk, GPIOD3MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpiod4msk, GPIOD4MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpiod5msk, GPIOD5MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpiod6msk, GPIOD6MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpiod7msk, GPIOD7MSK);
		A2B_CHECK_AND_WRITE(gpioDist, gpioddat, GPIODDAT);
		A2B_CHECK_AND_WRITE(gpioDist, gpiodinv, GPIODINV);
		A2B_CHECK_AND_WRITE(gpioDist, gpioden, GPIODEN);

        A2B_DSCVRY_SEQEND(plugin->ctx);
    }

	/*Demeter and Plutus Start*/
	if ((isAd243x) || (isAd2430_8))
	{

		if (plugin->bdd->nodes[nodeIdx].has_i2sCrossbarRegs)
		{
			A2B_DSCVRY_SEQGROUP0(plugin->ctx,
				"Crossbar Registers");
			/* Crossbar registers */
			for (nIndex = 0; nIndex < plugin->bdd->nodes[nodeIdx].i2sCrossbarRegs.rxmask_count; nIndex++)
			{
				A2B_CHECK_AND_WRITE_ARR(i2sCrossbarRegs, rxmask, RXMASK0, nIndex);
			}
			for (nIndex = 0; nIndex < plugin->bdd->nodes[nodeIdx].i2sCrossbarRegs.txcrossbar_count; nIndex++)
			{
				A2B_CHECK_AND_WRITE_ARR(i2sCrossbarRegs, txcrossbar, TXXBAR0, nIndex);
			}
			A2B_DSCVRY_SEQEND(plugin->ctx);
		}

		if (isAd243x) /* SPI and Data Tunnel applicable only for Demeter and not Plutus */
		{
			if (plugin->bdd->nodes[nodeIdx].has_spiRegs)
			{
				A2B_DSCVRY_SEQGROUP0(plugin->ctx,
					"SPI Registers");
				/* SPI registers */
				A2B_CHECK_AND_WRITE(spiRegs, spicfg, SPICFG);
				A2B_CHECK_AND_WRITE(spiRegs, spickdiv, SPICKDIV);
				A2B_CHECK_AND_WRITE(spiRegs, spifdsize, SPIFDSIZE);
				A2B_CHECK_AND_WRITE(spiRegs, spifdtarg, SPIFDTARG);
				A2B_CHECK_AND_WRITE(spiRegs, spipicfg, SPIPINCFG);
				A2B_CHECK_AND_WRITE(spiRegs, spiint, SPIINT);
				A2B_CHECK_AND_WRITE(spiRegs, spimsk, SPIMSK);
				A2B_DSCVRY_SEQEND(plugin->ctx);

			}

		if (plugin->bdd->nodes[nodeIdx].has_dataTunnelRegs)
		{
			A2B_DSCVRY_SEQGROUP0(plugin->ctx,
				"Data Tunnel Registers");
			/* Data tunnel registers */
			A2B_CHECK_AND_WRITE(dataTunnelRegs, dtcfg, DTCFG);
			A2B_CHECK_AND_WRITE(dataTunnelRegs, dtslots, DTSLOTS);
			A2B_CHECK_AND_WRITE(dataTunnelRegs, dtndnoffs, DTDNOFFS);
			A2B_CHECK_AND_WRITE(dataTunnelRegs, dtnupoffs, DTUPOFFS);
			A2B_DSCVRY_SEQEND(plugin->ctx);
		}
	    }
		if (plugin->bdd->nodes[nodeIdx].has_pwmRegs)
		{
			A2B_DSCVRY_SEQGROUP0(plugin->ctx,
				"PWM Registers");
			/* WRITE MMR PAGE - 1 Register*/
			wBuf[0] = A2B_REG_MMRPAGE;
			wBuf[1] = 0x01u;
			status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);

			/* PWM registers */
			A2B_CHECK_AND_WRITE(pwmRegs, pwmcfg, PWMCFG);
			A2B_CHECK_AND_WRITE(pwmRegs, pwmfreq, PWMFREQ);
			A2B_CHECK_AND_WRITE(pwmRegs, pwmblink1, PWMBLINK1);
			A2B_CHECK_AND_WRITE(pwmRegs, pwmblink2, PWMBLINK2);
			for (nIndex = 0; nIndex < plugin->bdd->nodes[nodeIdx].pwmRegs.pwmval_count; nIndex++)
			{
				A2B_CHECK_AND_WRITE_ARR(pwmRegs, pwmval, PWM1VALL, nIndex);
			}
			for (nIndex = 0; nIndex < plugin->bdd->nodes[nodeIdx].pwmRegs.pwmoe_count; nIndex++)
			{
				A2B_CHECK_AND_WRITE_ARR(pwmRegs, pwmoe, PWMOEVALL, nIndex);
			}

			/* WRITE MMR PAGE - 0 Register*/
			wBuf[0] = A2B_REG_MMRPAGE;
			wBuf[1] = 0x00u;
		    status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);
			A2B_DSCVRY_SEQEND(plugin->ctx);

		}

		if (isAd243x) /* VMTR applicable only for Demeter and not Plutus */
		{
			if (plugin->bdd->nodes[nodeIdx].has_vmtrRegs)
			{
				A2B_DSCVRY_SEQGROUP0(plugin->ctx,
					"VMTR Registers");
				/* WRITE MMR PAGE - 1 Register*/
				wBuf[0] = A2B_REG_MMRPAGE;
				wBuf[1] = 0x01u;
				status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);

				/* VMTR registers */
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_ven, VMTR_VEN);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_inten, VMTR_INTEN);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_mxstat, VMTR_MXSTAT);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_mnstat, VMTR_MNSTAT);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmax0, VMTR_VMAX0);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmin0, VMTR_VMIN0);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmax1, VMTR_VMAX1);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmin1, VMTR_VMIN1);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmax2, VMTR_VMAX2);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmin2, VMTR_VMIN2);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmax3, VMTR_VMAX3);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmin3, VMTR_VMIN3);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmax4, VMTR_VMAX4);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmin4, VMTR_VMIN4);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmax5, VMTR_VMAX5);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmin5, VMTR_VMIN5);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmax6, VMTR_VMAX6);
				A2B_CHECK_AND_WRITE(vmtrRegs, vmtr_vmin6, VMTR_VMIN6);

				/* WRITE MMR PAGE - 0 Register*/
				wBuf[0] = A2B_REG_MMRPAGE;
				wBuf[1] = 0x00;
				status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);

				A2B_DSCVRY_SEQEND(plugin->ctx);
			}
		}
	}
	
	/*Demeter End*/


#ifdef A2B_FEATURE_COMM_CH
    if (plugin->bdd->nodes[nodeIdx].nodeDescr.oCustomNodeIdSettings.bReadFrmCommCh == A2B_FALSE)
    {
#endif	/* A2B_FEATURE_COMM_CH */
		/*---------------------------------------------*/
		/* Mailbox Registers (AD242x only, slave only) */
		/*---------------------------------------------*/

		if ( (A2B_SUCCEEDED(status)) &&
			(isAd242xOnward) &&
			(plugin->bdd->nodes[nodeIdx].has_mbox) &&
			(A2B_NODEADDR_MASTER != nodeAddr) )
		{
			A2B_DSCVRY_SEQGROUP0( plugin->ctx, "Mailbox Registers" );

			A2B_CHECK_AND_WRITE(mbox, mbox0ctl, MBOX0CTL);
			A2B_CHECK_AND_WRITE(mbox, mbox1ctl, MBOX1CTL);

			A2B_DSCVRY_SEQEND(plugin->ctx);
		}
#ifdef A2B_FEATURE_COMM_CH
	}
#endif	/* A2B_FEATURE_COMM_CH */
	
	if (isAd242xOnward)
	{
		if ((A2B_NODEADDR_MASTER != nodeAddr))
		{
			/* A2B_REG_SUSCFG - AD242X only for slave nodes */
			A2B_CHECK_AND_WRITE(i2cI2sRegs, i2srrsoffs, I2SRRSOFFS);
			A2B_CHECK_AND_WRITE(ctrlRegs, suscfg, SUSCFG);
		}
		A2B_CHECK_AND_WRITE(i2cI2sRegs, i2srrctl, I2SRRCTL);

	}



    /*------------------*/
    /* Tuning Registers */
    /*------------------*/

    if (plugin->bdd->nodes[nodeIdx].has_tuningRegs)
    {
        A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                              "Tuning Registers" );

		A2B_CHECK_AND_WRITE(tuningRegs, vregctl, VREGCTL);
		if ((!isAd243x) && (!isAd2430_8))
		{
			A2B_CHECK_AND_WRITE(tuningRegs, txactl, TXACTL);
			A2B_CHECK_AND_WRITE(tuningRegs, rxactl, RXACTL);
			A2B_CHECK_AND_WRITE(tuningRegs, txbctl, TXBCTL);
			A2B_CHECK_AND_WRITE(tuningRegs, rxbctl, RXBCTL);
		}
		else
		{
			A2B_CHECK_AND_WRITE(tuningRegs, txctl, TXCTL);
			A2B_CHECK_AND_WRITE(tuningRegs, rxctl, RXCTL);
		}
        A2B_DSCVRY_SEQEND(plugin->ctx);
    }

    /*---------------------*/
    /* Interrupt Registers */
    /*---------------------*/

    bRetVal = a2b_dscvryNodeInterruptInit(plugin, nodeIdx);
    if (( A2B_SUCCEEDED(status) ) && (!bRetVal))
    {
        A2B_DSCVRY_ERROR0( plugin->ctx, "nodeComplete",
                           "Failed to set node interrupts" );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        *errCode = (a2b_UInt32)A2B_EC_IO;
        return A2B_EXEC_COMPLETE_FAIL;
    }

    /*---------------------*/
    /* Final Master Setup  */
    /*---------------------*/

    if ( A2B_SUCCEEDED(status) )
    {
        if ((A2B_NODEADDR_MASTER != nodeAddr) && (bdd_DISCOVERY_MODE_SIMPLE == eDiscMode))
        {
		    /* Don't enable switch to last slave */
        	if(nodeAddr != ((a2b_Int16)plugin->bdd->nodes_count - 2))
        	{
				if (plugin->pwrDiag.results.isNonCriticalFault == A2B_TRUE)
				{
					/*	During partial discovery:
						- Switch is already disabled in line fault section for the slave node where we found a non-critical fault. 
						- So, proceed for configuring the master node which is discovered
					*/
				}
				else
				{
					a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[nodeAddr].ctrlRegs.swctl);

					wBuf[0] = A2B_REG_SWCTL;
					/* Enable switch during a fresh discovery */
					wBuf[1] = A2B_BITM_SWCTL_ENSW | (a2b_UInt8)(plugin->slaveNodeSig[nodeAddr].highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV);
					wBuf[1] |= (nUserSWCTL & A2B_REG_USER_SWCTL);
					status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);
					i2cCount++;
				}				
        	}
        }
    }

	/*-----------------------*/
	/* 244x CP Regs Processing */
	/*-----------------------*/
#ifdef A2B_ENABLE_AD244xx_SUPPORT
	if((plugin->p244xCPNetConfigStruct != A2B_NULL) && (nodeAddr != -1))
	{
		a2b_dscvryCPRegConfig(plugin, nodeAddr);
	}
#endif

    /*-----------------------*/
    /* Peripheral Processing */
    /*-----------------------*/
    retCode = A2B_EXEC_COMPLETE;
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
    if (( A2B_SUCCEEDED(status) ) && (bDoEepromCfg) &&
        ( A2B_HAS_EEPROM(plugin, nodeAddr) ) )
    {
        retCode = a2b_periphCfgInitProcessing( plugin, nodeAddr );
        if ( A2B_EXEC_COMPLETE_FAIL == retCode )
        {
            /* On an error we need to clear the hasEeprom tracking */
            plugin->discovery.hasEeprom = 0u;

            A2B_DSCVRY_ERROR0( plugin->ctx, "nodeComplete",
                               "EEPROM Processing Failed" );
            A2B_DSCVRY_SEQEND( plugin->ctx );
            *errCode = (a2b_UInt32)A2B_EC_IO;
            return A2B_EXEC_COMPLETE_FAIL;
        }
    }
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */


    /*-------------------*/
    /* Status Processing */
    /*-------------------*/

    if ( A2B_FAILED(status) )
    {
        A2B_DSCVRY_ERROR1( plugin->ctx, "nodeComplete",
                           "I2C failure at operation: %ld", &i2cCount );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        *errCode = (a2b_UInt32)A2B_EC_INTERNAL;
        return A2B_EXEC_COMPLETE_FAIL;
    }

    A2B_DSCVRY_SEQEND( plugin->ctx );

    return retCode;

} /* a2b_dscvryNodeComplete */

/*!****************************************************************************
*
*  \b              a2b_SpiToSpiPeriConfigSendMsg
*
*  This is called at the last for final master setup. This function is repeatedly
*  called for each node which is AD243x chip assuming there may be an SPI peripheral
*  connected. This function sends a message to slave plugins to configure the SPI peripheral.
*
*  \param          [in]    plugin           plugin specific data
*
*  \param          [in]    nodeAddr          NodeAddress -1=master
*
*  \pre            None
*
*  \post           None
*
*  \return         A2B_TRUE == Message sending succeeded
*                  A2B_FALSE == Message sending failed
*
******************************************************************************/
static a2b_HResult a2b_SpiToSpiPeriConfigSendMsg(a2b_Plugin* plugin, a2b_Int16 nodeAddr)
{
	struct a2b_Msg *msg;
	a2b_HResult result = 0;
	a2b_PluginInit* pluginInit;

    msg = a2b_msgAlloc( plugin->ctx, A2B_MSG_REQUEST, A2B_MSGREQ_PLUGIN_SPI_PERIPH_INIT );
    if ( A2B_NULL == msg )
    {
        A2B_DSCVRY_ERROR0( plugin->ctx, "a2b_SpiToSpiPeriConfigSendMsg", "Cannot allocate message" );
        return (a2b_UInt32)A2B_EC_RESOURCE_UNAVAIL;
    }

    /* Setting master plugin handle as user data in notify message */
    a2b_msgSetUserData( msg, (a2b_Handle)plugin, A2B_NULL );

    /* Sending the master node address */
    a2b_msgSetTid( msg, (a2b_UInt32)nodeAddr );

    if ( !A2B_HAS_PLUGIN(plugin, (a2b_UInt32)nodeAddr) )
    {
        /* Without a plugin this call would fail, so
         * we call the requested callback now.  This
         * means the msg is somewhat incomplete and
         * MUST be used carefully.
         */
	/* not cancelled */
        /* completeCallback( msg , A2B_FALSE ); */
        (void)a2b_msgUnref( msg );

        return (a2b_UInt32)A2B_EC_OK;
    }

    pluginInit 								= (a2b_PluginInit*)a2b_msgGetPayload( msg );
    pluginInit->req.tdmSettings 			= &plugin->pluginTdmSettings;
    pluginInit->req.pNodePeriDeviceConfig 	= (const void *)plugin->periphCfg.pkgCfg;

    /* Assume peripheral initialization failure */
    pluginInit->resp.status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL);

    result = a2b_msgRtrSendRequest(msg, nodeAddr /* destNodeAddr */, A2B_NULL);

    /* Job executor now owns the message, or free on error */
    (void)a2b_msgUnref( msg );

    return (result);
}

/*!****************************************************************************
*
*  \b              a2b_SpiToSpiPeriConfigDone
*
*  This serves as a notification callback function. This function is called when
*  a slave plugin completes the configuration of all the SPI peripherals connected
*  to the node. So, this function is called multiple times for every AD243x slave nodes.
*  Once all the SPI peripherals of every slave node is configured, this function will end
*  the discovery.
*
*  \param          [in]    msg           A2B message instance
*
*  \param          [in]    userData      An opaque pointer to user defined data
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void a2b_SpiToSpiPeriConfigDone(struct a2b_Msg* msg, a2b_Handle userData)
{
	a2b_Plugin	*plugin;
	a2b_Int16   nodeAddr=0;
	a2b_UInt32 	errCode = (a2b_UInt32)A2B_EC_OK;
	a2b_Bool	fSpiToSpiPeriConfigDoneSuccess = false;

	if(msg)
	{
		/* Getting master plugin handle from user data in notify message */
		plugin 		= (a2b_Plugin*)a2b_msgGetUserData(msg);

		/* Getting the current slave node address which emitted this notification */
		nodeAddr 	= (a2b_Int16)a2b_msgGetTid(msg);

		/* Increment the SPI to SPI peripheral configuration responses and the flag */
		a2b_stackIncrSpiToSpiPeriConfigRespFlags(plugin->ctx, nodeAddr);

		if(a2b_stackChkSpiToSpiPeriConfigReqRespCnt(plugin->ctx))
		{
			for(nodeAddr=0; nodeAddr< (a2b_Int16)A2B_CONF_MAX_NUM_SLAVE_NODES ; nodeAddr++)
			{
				if(a2b_stackChkSpiToSpiPeriConfigReqRespFlags(plugin->ctx, nodeAddr))
				{
					fSpiToSpiPeriConfigDoneSuccess = true;
				}
				else
				{
					fSpiToSpiPeriConfigDoneSuccess = false;
					break;
				}
			}

			if(fSpiToSpiPeriConfigDoneSuccess)
			{
				errCode = A2B_EC_OK;
			}
			else
			{
				errCode = A2B_EC_SPITOSPI_PERICONFIG_FAILED;
			}

			a2b_dscvryEnd( plugin, errCode );
		}
	}
	A2B_UNUSED(userData);
}

/*!****************************************************************************
*
*  \b              a2b_ReportI2CError
*
*  Creates an I2C error message for Application
*
*  \param          [in]    plugin           plugin specific data
*
*  \param          [in]    nodeAddr         NodeAddress -1(0xFF)=master
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void a2b_ReportI2CError(a2b_Plugin *plugin, a2b_Int16 nodeAddr)
{
	struct a2b_Msg* notification;
	a2b_I2CError* I2CError;
	a2b_HResult result = A2B_RESULT_SUCCESS;

    /* Allocate a notification message */
    notification = a2b_msgAlloc(plugin->ctx,
                                A2B_MSG_NOTIFY,
								A2B_MSGREQ_PERIPH_I2C_ERROR_APP);

    if ( A2B_NULL == notification )
    {
        A2B_TRACE0((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                "a2b_I2CErrorReported: "
                "failed to allocate notification"));
    }
    else
    {
    	I2CError = (a2b_I2CError*)a2b_msgGetPayload(notification);
    	I2CError->I2CAddr = (nodeAddr & 0xFF00) >> 8u;
		I2CError->nodeAddr = nodeAddr & 0xFF;
        /* Make best effort delivery of notification */
        result = a2b_msgRtrNotify(notification);
        if ( A2B_FAILED(result) )
        {
            A2B_TRACE1((plugin->ctx,
                    (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                    "a2b_I2CErrorReported: "
                    "failed to report I2C Error: 0x%lX",
                    &result));
        }

        /* We no longer need this message */
        (void)a2b_msgUnref(notification);
    }
}

/*!****************************************************************************
*
*  \b              a2b_ReportI2CError
*
*  This serves as a notification callback function. This function is called when
*  I2C error is reported from slave plugin
*
*  \param          [in]    msg           A2B message instance
*
*  \param          [in]    userData      An opaque pointer to user defined data
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void a2b_I2CErrorReported(struct a2b_Msg* msg, a2b_Handle userData)
{
	a2b_Plugin	*plugin;
	a2b_Int16   nodeAddr=0;

	if(msg)
	{
		/* Getting master plugin handle from user data in notify message */
		plugin 		= (a2b_Plugin*)a2b_msgGetUserData(msg);

		/* Getting the current slave node address which emitted this notification */
		nodeAddr 	= (a2b_Int16)a2b_msgGetTid(msg);

		a2b_ReportI2CError(plugin, nodeAddr);
	}
}

/*!****************************************************************************
*
*  \b              a2b_FinalMasterSetup
*
*  Final setup of master node after discovery. This is called at the last for
*  all modes of discovery except Advanced Mode. In advanced mode, this function
*  is repeatedly called after reconfiguration of slots at each node.
*
*  \param          [in]    plugin           plugin specific data
*
*  \param          [in]    nodeAddr          NodeAddress -1=master
*
*  \pre            None
*
*  \post           None
*
*  \return         A2B_TRUE == NetConfiguration is to be done
*                  A2B_FALSE == NetConfiguration is not to be done
*
******************************************************************************/
static a2b_HResult
a2b_FinalMasterSetup(a2b_Plugin* plugin,
		a2b_Int16   nodeAddr)
{
	a2b_HResult 			status = A2B_RESULT_SUCCESS;
    a2b_Bool 				isAd242xMaster = A2B_FALSE, isAd243x = A2B_FALSE;
	a2b_Bool isAd2430_8 = A2B_FALSE;
    a2b_UInt32 				i2cCount = 0u;
    a2b_UInt8 				wBuf[4u];
    a2b_Int16 				nodeIdx = nodeAddr+1;
    a2b_UInt32 	errCode =   (a2b_UInt32)A2B_EC_OK;
    struct a2b_MsgNotifier 	*notifyIrptSpiToSpiPeriConfigDone;

    A2B_UNUSED(errCode);
    
    if ((A2B_NODEADDR_MASTER == nodeAddr) && (plugin != A2B_NULL))
    {

        if ( plugin->nodeSig.hasSiliconInfo )
        {
            isAd242xMaster = a2b_isAd242xChipOnward(
                                            plugin->nodeSig.siliconInfo.vendorId,
                                            plugin->nodeSig.siliconInfo.productId);
        }

        A2B_DSCVRY_SEQGROUP0( plugin->ctx, "Final Master Setup" );
		
		/* Configuration of Spread spectrum - AD2428 onwards */
		(void)a2b_ConfigSpreadSpectrum(plugin, nodeAddr);

		wBuf[0] = A2B_REG_NODEADR;
		wBuf[1] = (a2b_UInt8)0;
		status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
		i2cCount++;

		if (plugin->bdd->nodes[nodeIdx].ctrlRegs.has_slotfmt)
		{
			wBuf[0] = A2B_REG_SLOTFMT;
			wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].ctrlRegs.slotfmt;
			status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
			i2cCount++;
		}

		if ( A2B_SUCCEEDED(status) )
		{
			/* Enable the up and downstream slots */
			wBuf[0] = A2B_REG_DATCTL;

			if (plugin->bdd->nodes[nodeIdx].ctrlRegs.has_datctl) 
			{
				wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].ctrlRegs.datctl;
			}
			else 
			{
				wBuf[1] = A2B_BITM_DATCTL_UPS | A2B_BITM_DATCTL_DNS;
			}
			status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
			i2cCount++;
		}

		/* A2B_REG_I2SRRATE - AD242X only, Master node only */
		if ( (A2B_SUCCEEDED(status)) &&
			(isAd242xMaster) &&
			(plugin->bdd->nodes[nodeIdx].i2cI2sRegs.has_i2srrate ))
		{
			wBuf[0] = A2B_REG_I2SRRATE;
			wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].i2cI2sRegs.i2srrate;
			status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
			i2cCount++;
		}

		if ( A2B_SUCCEEDED(status) )
		{
			/* Push all shadowed/cached registers */
			wBuf[0] = A2B_REG_CONTROL;
			/* The AD242X (only) needs to be told it's a Master node
			 * BEFORE the PLL locks on the SYNC pin. Once the PLL is
			 * locked, setting the MSTR bit is ignored. We set it
			 * anyway so it's clear this is the master node.
			 */
			wBuf[1] = A2B_ENUM_CONTROL_START_NS;
			if ( isAd242xMaster )
			{
				wBuf[1] |= (a2b_UInt8)A2B_ENUM_CONTROL_MSTR;
			}

			isAd243x = A2B_IS_AD243X_CHIP(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);
			isAd2430_8 = A2B_IS_AD2430_8_CHIP(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);
			if ((isAd243x) || (isAd2430_8))
			{
				/* Adding additional bit fields with AD243x onwards */
				wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[nodeIdx].ctrlRegs.control & (A2B_ENUM_CONTROL_I2SMSINV | A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
			}
			else
			{
				/* Adding additional bit fields with AD2428 onwards */
				wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[nodeIdx].ctrlRegs.control & (A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
			}

			status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
			i2cCount++;

			/* Active delay for new struct */
			a2b_delayForNewStruct(plugin->ctx, A2B_NODEADDR_MASTER);
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );

#ifndef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
		if (plugin->ctx->stk->accessInterface == A2B_ACCESS_SPI)
		{
			a2b_stackResetSpiToSpiPeriReqRespFlags(plugin->ctx);

			/* Register for notifications on SPI to SPI peripheral configuration done events */
			notifyIrptSpiToSpiPeriConfigDone = a2b_msgRtrRegisterNotify(plugin->ctx, A2B_MSGNOTIFY_SPITOSPI_PERICONFIG_DONE, &a2b_SpiToSpiPeriConfigDone, A2B_NULL, A2B_NULL);
			if (notifyIrptSpiToSpiPeriConfigDone != A2B_NULL)
			{
				/* We need to configure the SPI peripherals on the discovered nodes */
				for (nodeAddr = ((a2b_Int16)plugin->discovery.dscNumNodes - (a2b_Int16)1); nodeAddr > A2B_NODEADDR_MASTER; nodeAddr--)
				{
					isAd243x = A2B_IS_AD243X_CHIP(plugin->slaveNodeSig[nodeAddr].siliconInfo.vendorId,
						plugin->slaveNodeSig[nodeAddr].siliconInfo.productId);
					if (isAd243x)
					{
						/* Send SPI to SPI peripheral configuration messages to slave plugin */
						status = a2b_SpiToSpiPeriConfigSendMsg(plugin, nodeAddr);

						/* Increment the SPI to SPI peripheral configuration requests and the flag */
						a2b_stackIncrSpiToSpiPeriConfigReqFlags(plugin->ctx, nodeAddr);
					}
				}
			}
			else
			{
				status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL);
			}
		}		
#else
        /* We need to configure the SPI peripherals on the discovered nodes */
        for ( nodeAddr = ((a2b_Int16)plugin->discovery.dscNumNodes-(a2b_Int16)1); nodeAddr > A2B_NODEADDR_MASTER; nodeAddr-- )
        {
        	isAd243x = A2B_IS_AD243X_CHIP(	plugin->slaveNodeSig[nodeAddr].siliconInfo.vendorId,
        									plugin->slaveNodeSig[nodeAddr].siliconInfo.productId);
        	if(isAd243x)
        	{
        		/* Write SPI to SPI peripheral configuration */
        		status = a2b_periphSpiCfgProcessing(plugin, nodeAddr);
        	}
        }

#endif
    }
    return status;

} /* a2b_FinalMasterSetup */


/*!****************************************************************************
*
*  \b              a2b_ConfigSpreadSpectrum
*
*  Final setup of Spread Spectrum Configuration.
*
*  \param          [in]    plugin           plugin specific data
*
*  \param          [in]    nodeAddr          NodeAddress -1=master
*
*  \pre            None
*
*  \post           None
*
*  \return         A2B_TRUE == NetConfiguration is to be done
*                  A2B_FALSE == NetConfiguration is not to be done
*
******************************************************************************/
static a2b_HResult
a2b_ConfigSpreadSpectrum(a2b_Plugin* plugin,
		a2b_Int16   nodeAddr)
{
	a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_Bool isAd2428Master = A2B_FALSE;
	a2b_Bool isAd2435Master = A2B_FALSE;	
	a2b_Bool isAd2430_8 = A2B_FALSE;
    a2b_UInt8 wBuf[4u], nIndex;
    a2b_Int16 nodeIdx = nodeAddr+1;
	a2b_Int16 slvNodeAddr = 0;
	a2b_Int16 slvNodeIdx = 0;
	a2b_Bool commnSpreadSpectrum = A2B_FALSE;

    if ((A2B_NODEADDR_MASTER == nodeAddr) && (plugin != A2B_NULL))
    {
    	if(plugin->bdd->policy.has_has_common_SSSettings == A2B_TRUE)
    	{
    		commnSpreadSpectrum = (a2b_Bool)plugin->bdd->policy.has_common_SSSettings;
    	}

        if ( plugin->nodeSig.hasSiliconInfo )
        {
            isAd2428Master = A2B_IS_AD2428X_CHIP(
                                            plugin->nodeSig.siliconInfo.vendorId,
                                            plugin->nodeSig.siliconInfo.productId);

			isAd2435Master = A2B_IS_AD243X_CHIP(
				plugin->nodeSig.siliconInfo.vendorId,
				plugin->nodeSig.siliconInfo.productId);

			isAd2430_8 = A2B_IS_AD2430_8_CHIP(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);

        }
		if((isAd2428Master) || (isAd2435Master) || (isAd2430_8))
		{
			/* Check whether all nodes should have common spread spectrum settings */
			if(commnSpreadSpectrum)
			{
				wBuf[0] = A2B_REG_NODEADR;
				wBuf[1] = 0x0u;
				status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf);

				wBuf[0] = A2B_REG_PLLCTL;
				wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].i2cI2sRegs.pllctl;
				status = a2b_regBroadcastWrite(plugin->ctx, 2u, wBuf);
			}
			else
			{
				wBuf[0] = A2B_REG_PLLCTL;
				wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeIdx].i2cI2sRegs.pllctl;

				status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
			}
		}
		if((plugin->discovery.dscNumNodes) && (commnSpreadSpectrum == A2B_FALSE))
		{
			/*If the every node has separate settings*/
			for(nIndex = 0u; nIndex < plugin->discovery.dscNumNodes; nIndex++)
			{
				slvNodeAddr = (a2b_Int16)nIndex;
				slvNodeIdx = slvNodeAddr + 1;
				if(plugin->bdd->nodes[slvNodeIdx].i2cI2sRegs.has_pllctl)
				{
					wBuf[0u] = A2B_REG_PLLCTL;
					wBuf[1u] = (a2b_UInt8)plugin->bdd->nodes[slvNodeIdx].i2cI2sRegs.pllctl;
					status = a2b_regWrite(plugin->ctx, slvNodeAddr , 2u, wBuf );
				}

			}
		}

    
    }
    return status;

} /* a2b_ConfigSpreadSpectrum */


/*!****************************************************************************
*
*  \b              a2b_dscvryNetComplete
*
*  Final cleanup steps of network discovery
*  (i.e. when all nodes have been found.)
* 
*  In the case of Simplified Discovery this method can be called more than once.
*
*  \param          [in]    plugin   plugin specific data
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_dscvryNetComplete
    (
    a2b_Plugin* plugin
    )
{
    a2b_UInt8 wBuf[4];
    a2b_Int16 nodeAddr;
    a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_Bool initMaster = A2B_TRUE;
    a2b_UInt32 errCode = (a2b_UInt32)A2B_EC_OK;
    a2b_Int32 retCode;
    a2b_Bool bDoEepromCfg = A2B_TRUE;
#ifdef A2B_FEATURE_PARTIAL_DISC
	a2b_Int16 nNumNodesConfig;
	a2b_Bool bRetVal;
#endif

#ifdef A2B_FEATURE_SEQ_CHART
    a2b_Bool bSeqGroupShown = A2B_FALSE;
#endif

#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
    a2b_Int16 nTempVar;
#if !defined(A2B_FEATURE_SEQ_CHART)
    A2B_UNUSED(nTempVar);
#endif
#endif

    if ( !plugin->discovery.inDiscovery )
    {
        return;
    }

    if ( bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin) )
    {
        a2b_Bool bContLoop;

#ifndef A2B_FEATURE_WAIT_ON_PERIPHERAL_CFG_DELAY
        /* When disabled using Simple discovery the node init will be
         * completed for all nodes, then all peripheral config is queued
         * and will execute cooperatively.  Whereas when enabled the node
         * init is completed followed by the nodes peripheral processing
         * followed by the next nodes int, and so on.
         */
        bDoEepromCfg = A2B_FALSE;
#endif /* A2B_FEATURE_WAIT_ON_PERIPHERAL_CFG_DELAY */

#ifdef A2B_FEATURE_PARTIAL_DISC
		if (plugin->discovery.inPartialDiscovery)
		{
			nNumNodesConfig = (a2b_Int16)plugin->discovery.dscNumPartialNodes - (a2b_Int16)1;
		}
		else
		{
			nNumNodesConfig = A2B_NODEADDR_MASTER;
		}
		/* Start from the latest to the last working slave before partial discovery */
		for (nodeAddr = ((a2b_Int16)plugin->discovery.simpleNodeCount - (a2b_Int16)1);
			nodeAddr > (a2b_Int16)nNumNodesConfig;
			nodeAddr--)
#else
        /* Start from the latest to the first (per spec) */
        for ( nodeAddr = ((a2b_Int16)plugin->discovery.simpleNodeCount-(a2b_Int16)1);
              nodeAddr > A2B_NODEADDR_MASTER; 
              nodeAddr-- )
#endif
        {
#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
            nTempVar = nodeAddr;
#endif
            bContLoop = A2B_FALSE;

            A2B_DSCVRY_SEQGROUP0_COND( plugin->ctx, bSeqGroupShown,
                                       "NetComplete" );

            retCode = a2b_dscvryNodeComplete( plugin, nodeAddr, 
                                              bDoEepromCfg, &errCode );
            plugin->discovery.simpleNodeCount--;

            if ( A2B_EXEC_COMPLETE_FAIL == retCode )
            {
                A2B_DSCVRY_ERROR1( plugin->ctx, "NetComplete",
                                   "Failed to complete node %hd init", 
                                   &nTempVar );
                a2b_dscvryEnd( plugin, errCode );
            }
            else if ( A2B_EXEC_COMPLETE == retCode )
            {
                /* a2b_dscvryInitPlugin() will also check for an
                 * available plugin, however, if no plugin is 
                 * available for a plugin it will call the callback 
                 * right away.  So to avoid a situation where few
                 * or no plugins are in our setup we could possibly 
                 * blow the stack on some systems.  So to avoid
                 * growing the stack unnecessarily we check here
                 * for a plugin and if not available we will loop
                 * to the next plugin.
                 */
#ifdef A2B_FEATURE_WAIT_ON_PERIPHERAL_CFG_DELAY

#ifdef FIND_NODE_HANDLER_AFTER_NODE_INIT
                a2b_dscvryFindNodeHandler(plugin,
                            A2B_MAP_SLAVE_ADDR_TO_INDEX(nodeAddr));
#endif
                if ( A2B_HAS_PLUGIN(plugin, nodeAddr) )
                {
                    errCode = a2b_dscvryInitPlugin( plugin, nodeAddr,
                                  &a2b_dscvryInitPluginComplete_EepromComplete );
                    if ((a2b_UInt32)A2B_EC_OK != errCode )
                    {
                        a2b_dscvryEnd( plugin, errCode );
                    }
                    /* else, waiting for plugin message to process */
                }
                else
#endif /* A2B_FEATURE_WAIT_ON_PERIPHERAL_CFG_DELAY */
                {
                    /* Process the next plugin using this routine */
                    bContLoop = A2B_TRUE;
                }
            }
            else
            {
                /* Completing the control statement */
            }
            /* else, waiting for peripheral node processing to complete */

            if ( !bContLoop )
            {
                A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );
                return;
            }
        }

#ifndef A2B_FEATURE_WAIT_ON_PERIPHERAL_CFG_DELAY
        for ( nodeAddr = plugin->discovery.dscNumNodes-1;
              nodeAddr > A2B_NODEADDR_MASTER; 
              nodeAddr-- )
        {
            retCode = A2B_EXEC_COMPLETE;

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
            if ( A2B_HAS_EEPROM(plugin, nodeAddr) )
            {
                A2B_DSCVRY_SEQGROUP0_COND( plugin->ctx, bSeqGroupShown,
                                           "NetComplete" );

                retCode = a2b_periphCfgInitProcessing( plugin, nodeAddr );
                if ( A2B_EXEC_COMPLETE_FAIL == retCode )
                {
                    /* On an error we need to clear the hasEeprom tracking */
                    plugin->discovery.hasEeprom = 0;

                    A2B_DSCVRY_ERROR0( plugin->ctx, "nodeComplete",
                                       "EEPROM Processing Failed" );
                    a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_IO );
                    A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );
                    return;
                }
            }
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

            if ( A2B_EXEC_COMPLETE == retCode )
            {
#ifdef FIND_NODE_HANDLER_AFTER_NODE_INIT
                a2b_dscvryFindNodeHandler(plugin,
                                        A2B_MAP_SLAVE_ADDR_TO_INDEX(nodeAddr));
#endif
                if ( A2B_HAS_PLUGIN(plugin, nodeAddr) &&
                     A2B_NEEDS_PLUGIN_INIT(plugin, nodeAddr) )
                {
                    /* This ensures we will only queue the message to
                     * the peripheral mailbox for this node.
                     */
                    errCode = a2b_dscvryInitPlugin( plugin, nodeAddr,
                              a2b_dscvryInitPluginComplete_EepromComplete );
                    if ( A2B_EC_OK != errCode )
                    {
                        a2b_dscvryEnd( plugin, errCode );
                        A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );
                        return;
                    }
                    /* else, waiting for plugin message to process */
                }
            }
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
            else if (( A2B_EXEC_SUSPEND == retCode ) &&
                     (a2b_periphCfgUsingSync()) )
            {
                /* MUST be waiting for a delay to complete 
                 * while synchronous cfg blocks processing
                 * is expected.
                 */
                A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );
                return;
            }
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */
        }

        if (( plugin->discovery.hasEeprom ) ||
            (plugin->discovery.pendingPluginInit ) )
        {
            initMaster = A2B_FALSE;
        }

#endif /* A2B_FEATURE_WAIT_ON_PERIPHERAL_CFG_DELAY */

    }
    else /* if ( bdd_DISCOVERY_MODE_MODIFIED == a2b_ovrGetDiscMode(plugin) )*/
    {
		a2b_UInt8 nUserSWCTL = 0;
        A2B_DSCVRY_SEQGROUP0_COND( plugin->ctx, bSeqGroupShown,
                                   "NetComplete" );

        /* If the nodeAddr we were trying to discover was Slave0 
         * then we skip this final step on the node.
         */
        if (plugin->discovery.dscNumNodes != 0u)
        {
            A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                                  "Adjust Node Power" );


            /* We need to adjust the power setting on the discovered nodes */
            for ( nodeAddr = ((a2b_Int16)plugin->discovery.dscNumNodes-(a2b_Int16)2);
                  nodeAddr > A2B_NODEADDR_MASTER; 
                  nodeAddr-- )
            {
				nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[nodeAddr].ctrlRegs.swctl);

#ifdef A2B_FEATURE_TRACE
                nTempVar = nodeAddr;
#endif
                wBuf[0] = A2B_REG_SWCTL;
                wBuf[1] = A2B_ENUM_SWCTL_ENSW_EN;
				wBuf[1] |= (nUserSWCTL & A2B_REG_USER_SWCTL) | (a2b_UInt8)(plugin->slaveNodeSig[nodeAddr].highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV);
                status = a2b_regWrite( plugin->ctx, nodeAddr, 2u, wBuf );
                if ( A2B_FAILED(status) )
                {
                    A2B_DSCVRY_ERROR1( plugin->ctx, "NetComplete",
                                       "Cannot adjust power on nodeAddr: %hd",
                                       &nTempVar );
                    initMaster = A2B_FALSE;
                    break;
                }
            }

			wBuf[0] = A2B_REG_SWCTL;
    		nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.swctl);

			wBuf[1] = A2B_ENUM_SWCTL_ENSW_EN | (a2b_UInt8)(a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV);
			wBuf[1] |= (nUserSWCTL & A2B_REG_USER_SWCTL);
            status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
            if ( A2B_FAILED(status) )
            {
                A2B_DSCVRY_ERROR1( plugin->ctx, "NetComplete",
                                    "Cannot adjust power on nodeAddr: %hd",
                                    &nTempVar );
                initMaster = A2B_FALSE;
            }

            A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );
        }
    }

    if ( bdd_DISCOVERY_MODE_ADVANCED == a2b_ovrGetDiscMode(plugin) )
    {
    	initMaster = A2B_FALSE;
    }

#ifdef A2B_FEATURE_PARTIAL_DISC
	if (plugin->discovery.inPartialDiscovery == A2B_TRUE)
	{
		/* Skip master node configuration as it is a partial discovery and perform only final master setup */
		initMaster = A2B_FALSE;

		if (plugin->discovery.dscNumNodes + (a2b_UInt8)1 >= (a2b_UInt8)plugin->bdd->nodes_count)
		{
			/* All the dropped nodes are discovered */
			status = a2b_FinalMasterSetup(plugin, A2B_NODEADDR_MASTER);
			if (status != A2B_RESULT_SUCCESS)
			{
				A2B_DSCVRY_ERROR0(plugin->ctx, "NetComplete",
					"Failed to complete master node init");
			}

			if (bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin))
			{
				if (a2b_stackChkSpiToSpiPeriConfigReqRespCnt(plugin->ctx))
				{
					/* If there are no SPI to SPI peripheral configuration messages sent to slave plugin, immediately end the discovery */
					a2b_dscvryEnd(plugin, errCode);
				}

			}
			plugin->discovery.inPartialDiscovery = A2B_FALSE;
		}
		else
		{			
			A2B_DSCVRY_ERROR0(plugin->ctx, "NetComplete",
					"Failed to discover dropped nodes");
			a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_DISCOVERY_FAILURE);
		}

		/*---------------------*/
		/* Interrupt Registers */
		/*---------------------*/
		for (nodeAddr = A2B_MASTER_NODEBDDIDX;
			nodeAddr <= ((a2b_Int16)plugin->discovery.dscNumPartialNodes);
			nodeAddr++)
		{
			bRetVal = a2b_dscvryNodeInterruptInit(plugin, nodeAddr);
			if (!bRetVal)
			{
				A2B_DSCVRY_ERROR0(plugin->ctx, "NetComplete",
					"Failed to set master node interrupts");
				A2B_DSCVRY_SEQEND(plugin->ctx);
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_IO);
				return;
			}
		}

		/* Re-enable power fault diagnostics again after partial discovery */
		plugin->bDisablePwrDiag = A2B_FALSE;
	}
#endif

    if ( initMaster )
    {
        A2B_DSCVRY_SEQGROUP0_COND( plugin->ctx, bSeqGroupShown,
                                   "NetComplete" );

        a2b_dscvryInitTdmSettings( plugin, A2B_NODEADDR_MASTER );
        status = a2b_audioConfig( plugin->ctx, &plugin->pluginTdmSettings );

        if ( A2B_FAILED(status) )
        {
            A2B_DSCVRY_ERROR1( plugin->ctx, "NetComplete",
                               "Cannot config audio for nodeAddr: %hd",
                               &nodeAddr );
            a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_IO );
            A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );
            return;
        }

        /* Do final setup of the master node and enable the up/downstream */
        if (a2b_dscvryNodeComplete( plugin, A2B_NODEADDR_MASTER, 
                                 A2B_TRUE, &errCode ) == A2B_EXEC_COMPLETE_FAIL)
        {
            A2B_DSCVRY_ERROR0( plugin->ctx, "NetComplete",
                               "Failed to complete master node init" );
        }
        else
        {
			status = a2b_FinalMasterSetup(plugin, A2B_NODEADDR_MASTER);
			if(status != A2B_RESULT_SUCCESS)
			{
				A2B_DSCVRY_ERROR0( plugin->ctx, "NetComplete",
								   "Failed to complete master node init" );
			}
        }

        if ( bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin) )
        {
#ifndef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
        	if(a2b_stackChkSpiToSpiPeriConfigReqRespCnt(plugin->ctx))
#endif
        	{
        		/* If there are no SPI to SPI peripheral configuration messages sent to slave plugin, immediately end the discovery */
        		a2b_dscvryEnd( plugin, errCode );
        	}

        }
    }

    A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );

} /* a2b_dscvryNetComplete */


/*!****************************************************************************
*
*  \b              a2b_dscvryPreSlaveInit
*
*  Steps taken before finding the next slave node during discovery
*
*  \param          [in]    plugin       plugin specific data
*
*  \pre            None
*
*  \post           The following registers are altered:
*                  A2B_REG_SWCTL
*                  A2B_REG_DISCVRY
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool
a2b_dscvryPreSlaveInit
    (
    a2b_Plugin* plugin
    )
{
    a2b_UInt8 wBuf[4u], rBuf[4], nHPSW_CFG_DET, nHPSW_MODE;
	a2b_HResult status;
	a2b_UInt32 value;
	struct a2b_StackContext* ctx = plugin->ctx;
	a2b_Bool isAd243x;
	a2b_Bool isAd2430_8 = A2B_FALSE;
	a2b_Bool  isXTalkFixApplicable;
	const bdd_Node      *bddNodeObj;
#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
	a2b_UInt8 levelMask;
	a2b_UInt8 enabledMask;
#endif
	/* Discovered node information */
	a2b_Int16 dscNodeBddIdx = (a2b_Int16)plugin->discovery.dscNumNodes;
	a2b_Int16 dscNodeAddr = dscNodeBddIdx - 1;

	/* Must add one to account for the fact that the nodes_count
	 * also includes the master node.
	 */
	if (plugin->discovery.dscNumNodes + (a2b_UInt8)1 >= (a2b_UInt8)plugin->bdd->nodes_count)
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
			"%s PreSlaveInit(): No more BDD slave nodes",
			A2B_MPLUGIN_PLUGIN_NAME));

		if (bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin))
		{
			/* For simple discovery we need to now complete the network
			 * and ensure the peripherals are complete before we call
			 * a2b_dscvryEnd.  If we have peripherals to work on an early
			 * call to a2b_dscvryEnd it will just exit.
			 */
			a2b_dscvryNetComplete(plugin);
		}
		else if (bdd_DISCOVERY_MODE_MODIFIED == a2b_ovrGetDiscMode(plugin)) /* bdd_DISCOVERY_MODE_MODIFIED */
		{
			/* We have completed the modified discovery.  We will call the
			 * a2b_dscvryEnd and it will check if the peripherals have
			 * finished processing
			 */
			a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_OK);
		}
		else
		{
			/* completing control statement */
		}

		return A2B_FALSE;
	}

	A2B_DSCVRY_SEQGROUP0(plugin->ctx,
		"PreSlaveInit");

	status = a2b_discvryHighPwrInit(plugin, dscNodeAddr, dscNodeBddIdx);

	isAd243x = A2B_IS_AD243X_CHIP(plugin->slaveNodeSig[dscNodeAddr].siliconInfo.vendorId, plugin->slaveNodeSig[dscNodeAddr].siliconInfo.productId);
	isAd2430_8 = A2B_IS_AD2430_8_CHIP(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);

	/* Write to Control register to Set LVDS Polarity, applicable only after AD2428
	*/
	wBuf[0] = A2B_REG_CONTROL;

	if((isAd243x) || (isAd2430_8))
	{
		wBuf[1] = (a2b_UInt8)(plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.control & (A2B_ENUM_CONTROL_I2SMSINV | A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
	}
	else
	{
		wBuf[1] = (a2b_UInt8)(plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.control & (A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
	}

#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
	if (plugin->bdd->nodes[dscNodeBddIdx].nodeDescr.product == 0x37)
	{
		if (plugin->discovery.bFirstStepDisc[dscNodeBddIdx])
		{
			/* Configure GPIO as INPUT pin to read the PG signal */
			(void)a2b_gpioInSetEnabled(plugin->ctx, dscNodeAddr, A2B_ENUM_GPIOIEN_IO7_EN, A2B_BITM_GPIOIEN_IO7IEN);

			/* Read GPIO 7 to make sure that PG signal is available */
			(void)a2b_gpioInGetLevels(plugin->ctx, dscNodeAddr, A2B_BITM_GPIOIEN_IO7IEN, &levelMask, &enabledMask);

			if (levelMask & A2B_BITM_GPIOIEN_IO7IEN)
			{
				/* Power Good signal available*/
				A2B_TRACE1((ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
					"%s a2b_dscvryPreSlaveInit(): Power Good Signal available",
					A2B_MPLUGIN_PLUGIN_NAME));
			}
			else
			{
				A2B_DSCVRY_ERROR1(ctx, "PreSlaveInit",
					"Power Good Signal need detected nodeAddr: %hd",
					&dscNodeAddr);
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_TWOSTEP_DISC_CRIT_FAULT);
				A2B_DSCVRY_SEQEND(plugin->ctx);
				return A2B_FALSE;
			}
			wBuf[1] |= A2B_ENUM_CONTROL_SWBYP;
		}
	}
#endif

	status = a2b_regWrite(ctx, dscNodeAddr, 2u, wBuf);

	if (A2B_FAILED(status))
	{
		A2B_DSCVRY_ERROR1(ctx, "PreSlaveInit",
			"Cannot write to CONTROL Reg nodeAddr: %hd",
			&dscNodeAddr);
		a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
		A2B_DSCVRY_SEQEND(plugin->ctx);
		return A2B_FALSE;
	}

	/* Reset Config Detect Override */
	plugin->slaveNodeSig[dscNodeAddr].highPwrSwitchModeOverride = 0u;

	/* SWCTL2.HPSW_CFG_DET for AD243x or higher */
	if (isAd243x)
	{
		/* Check if user wishes to override cfg detection */
		a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.swctl);
		plugin->slaveNodeSig[dscNodeAddr].highPwrSwitchModeOverride = 0u;

		/* Read the node's High power switch mode */
		wBuf[0] = A2B_REG_SWSTAT2;
		rBuf[0] = 0;
		status = a2b_regWriteRead(plugin->ctx, dscNodeAddr, 1u, wBuf, 1u, rBuf);

		nHPSW_CFG_DET = ((rBuf[0] & A2B_BITM_SWSTAT2_HPSW_CFG_DET) >> A2B_BITP_SWSTAT2_HPSW_CFG_DET);
		nHPSW_MODE = (plugin->bdd->nodes[dscNodeBddIdx].nodeSetting.eHighPwrSwitchCfg & A2B_BITM_SWCTL2_HPSW_CFG);

		if (A2B_SUCCEEDED(status))
		{
			if (nHPSW_CFG_DET != nHPSW_MODE)
			{
				/* Set the HPSW_MODE in SWCTL2 */
				wBuf[0] = A2B_REG_SWCTL2;
				wBuf[1] = (a2b_UInt8)(plugin->bdd->nodes[dscNodeBddIdx].nodeSetting.eHighPwrSwitchCfg & A2B_BITM_SWCTL2_HPSW_CFG);
				status = a2b_regWrite(ctx, dscNodeAddr, 2u, wBuf);
				if (A2B_SUCCEEDED(status))
				{
					plugin->slaveNodeSig[dscNodeAddr].highPwrSwitchModeOverride = 1u;

					/* Set the CFG_DET_OV in SWCTL */
					wBuf[0] = A2B_REG_SWCTL;
					wBuf[1] = (a2b_UInt8)(A2B_BITM_SWCTL_CFG_DET_OV);
					wBuf[1] |= (nUserSWCTL & 0x7Au);

					status = a2b_regWrite(ctx, dscNodeAddr, 2u, wBuf);
					if (A2B_FAILED(status))
					{
						A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY, "Failed to write SWCTL of the slave node");
						A2B_DSCVRY_SEQEND(plugin->ctx);
						return A2B_FALSE;
					}

#ifdef A2B_SS_STACK
					if (plugin->ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc != A2B_NULL)
					{
						/* In Milli second*/
						plugin->ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc(A2B_HIGH_PWR_SWITCH_DELAY, dscNodeAddr, A2B_0_0C_HIGH_PWR_SWITCH_DELAY_SCENARIO);
					}
#endif
					/* Delay for 100ms */
					a2b_ActiveDelay(ctx, A2B_HIGH_PWR_SWITCH_DELAY);
				}
				else
				{
					A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY, "Failed to write SWCTL2 of the slave node");
					A2B_DSCVRY_SEQEND(plugin->ctx);
					return A2B_FALSE;
				}
			}
		}
		else
		{
			A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY, "Failed to read SWSTAT2 of the master node");
			A2B_DSCVRY_SEQEND(plugin->ctx);
			return A2B_FALSE;
		}
	}

   {
	a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[plugin->discovery.dscNumNodes].ctrlRegs.swctl);

    /* We need to enable phantom power to the next node,
     * to do so we write to the just discovered node.
     */
    wBuf[0] = A2B_REG_SWCTL;
	if (plugin->bdd->nodes[dscNodeBddIdx].nodeSetting.eHighPwrSwitchCfg == bdd_highPwrSwitchCfg_HPSW_CFG_4)
	{
		wBuf[1] = (A2B_ENUM_SWCTL_MODE_NO_DWN_PHAN_PWR | A2B_ENUM_SWCTL_ENSW_EN | (a2b_UInt8)(plugin->slaveNodeSig[dscNodeAddr].highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV));
	}
	else
	{
		wBuf[1] = A2B_ENUM_SWCTL_ENSW_EN | (a2b_UInt8)(plugin->slaveNodeSig[dscNodeAddr].highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV);
	}

	wBuf[1] |= (nUserSWCTL & A2B_REG_USER_SWCTL);

	/* Do not write */
	if ((a2b_Bool)plugin->discovery.bAd243xCfg4OpenDetect == false)
	{
		status = a2b_regWrite(ctx, dscNodeAddr, 2u, wBuf);
#ifdef A2B_FEATURE_PARTIAL_DISC
		if ((plugin->discovery.inPartialDiscovery == A2B_TRUE) && isAd243x)
		{
			/* Active delay for AD243x after enabling SWCTL */
			a2b_ActiveDelay(ctx, A2B_HIGH_PWR_SWITCH_DELAY);
		}
#endif
	}

    }
    if ( A2B_FAILED(status) )
    {
        A2B_DSCVRY_ERROR1( ctx, "PreSlaveInit",
                           "Cannot enable phantom power on nodeAddr: %hd",
                           &dscNodeAddr );
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_INTERNAL );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }

    /* Only listen for power fault interrupts if it's not the last node
     * in the network. It's likely the last node will always report an
     * open circuit power fault and therefore that interrupt is not enabled
     * on this node
     */

    /* Set up INTMSK0 so we can see power fault interrupts during discovery */
    value = a2b_intrGetMask( ctx, dscNodeAddr );
    if ( value == A2B_INTRMASK_READERR )
    {
        A2B_DSCVRY_ERROR1( ctx, "PreSlaveInit",
                           "Cannot read INTMSK0-2 on nodeAddr: %hd",
                           &dscNodeAddr );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }
    else
    {
        /* Unmask all interrupts which might be helpful for diagnosing
         * discovery failures.
         */
        value |= (
               (a2b_UInt32)A2B_BITM_INTPND0_PWRERR
#if 0
             | A2B_BITM_INTPND0_SRFERR
             | A2B_BITM_INTPND0_BECOVF
             | A2B_BITM_INTPND0_DPERR
             | A2B_BITM_INTPND0_CRCERR
             | A2B_BITM_INTPND0_DDERR
             | A2B_BITM_INTPND0_HDCNTERR
#endif
             ) << (a2b_UInt32)A2B_INTRMASK0_OFFSET
             ;

        status = a2b_intrSetMask( ctx, dscNodeAddr, value );
        if ( A2B_FAILED(status) )
        {
            A2B_DSCVRY_ERROR1( ctx, "PreSlaveInit",
                               "Cannot write to INTMSK0-2 on nodeAddr: %hd",
                               &dscNodeAddr );

            A2B_DSCVRY_SEQEND( plugin->ctx );
            return A2B_FALSE;
        }
    }

	/* Possible work around : DEMETER-2139 */
	if (a2b_stackGetAccessInterface(plugin->ctx) == A2B_ACCESS_SPI)
	{
		wBuf[0] = A2B_REG_NODEADR;
		wBuf[1] = (a2b_UInt8)dscNodeAddr;
		status = a2b_regWrite(ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
	}

	/* Check whether the discovering node belongs to AD2425 series */
	bddNodeObj = &plugin->bdd->nodes[dscNodeBddIdx];
    isXTalkFixApplicable = a2b_isCrossTalkFixApply(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);

    wBuf[0] = A2B_REG_DISCVRY;
	/* Cross Talk fix is enable, then increase the response cycle */
	if ( (plugin->bdd->policy.bCrossTalkFixApply == 1u) && (isXTalkFixApplicable == A2B_TRUE))
	{
		wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[dscNodeBddIdx + 1].ctrlRegs.respcycs + 2u;
	}
	else
	{
		wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[dscNodeBddIdx + 1].ctrlRegs.respcycs;
	}
    status = a2b_regWrite(ctx,  A2B_NODEADDR_MASTER, 2u, &wBuf );

    if ( A2B_FAILED(status) )
    {
        A2B_DSCVRY_ERROR0( ctx, "PreSlaveInit", "Cannot discover next node" );
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_INTERNAL );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }

	/* Setup/Start the discovery timer */
	if (!a2b_dscvryStartTimer(plugin, TIMER_DSCVRY, dscNodeAddr))
	{
		A2B_DSCVRY_SEQEND(plugin->ctx);
		return A2B_FALSE;
	}

    /* Now we wait for INTTYPE.DSCDONE */

    A2B_DSCVRY_RAWDEBUG0( plugin->ctx, "dscvryPreSlaveInit", 
                          "...Waiting for INTTYPE.DSCDONE..." );
    A2B_DSCVRY_SEQEND( plugin->ctx );

    return A2B_TRUE;

} /* a2b_dscvryPreSlaveInit */


  /*!****************************************************************************
  *
  *  \b              a2b_discvryHighPwrInit
  *
  *  Steps taken to init the master node before discovery for high power node
  *
  *  \param          [in]    plugin       plugin specific data
  *
  *  \pre            None
  *
  *  \post           The following registers are altered:
  *                  A2B_REG_TM1KEY
  *                  A2B_REG_SWCTL2
  *                  A2B_REG_SWCTL3
  *                  A2B_REG_SWCTL4
  *                  A2B_REG_SWCTL5
  *                  A2B_REG_PTSTMODE
  *
  *  \return         FALSE=error
  *                  TRUE=success
  *
  ******************************************************************************/
static a2b_Bool a2b_discvryHighPwrInit(a2b_Plugin* plugin, a2b_Int16 dscNodeAddr, a2b_Int16 dscNodeBddIdx)
{
		a2b_Bool  status = 0;
		a2b_UInt8 wBuf[4u];
		struct a2b_StackContext* ctx = plugin->ctx;

		if (plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.has_swctl2)
		{
			/* SWCTL2 KEY */
			wBuf[0] = A2B_REG_SWCTL2;
			wBuf[1] = (a2b_UInt8)((plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.swctl2) & 0x20U); //exclude HPSW_CFG writing here 
			status = a2b_regWrite(ctx, dscNodeAddr, 2u, wBuf);
		}

		if (plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.has_swctl5)
		{
			/* SWCTL5 KEY */
			wBuf[0] = A2B_REG_SWCTL5;
			wBuf[1] = (a2b_UInt8)(plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.swctl5);
			status = a2b_regWrite(ctx, dscNodeAddr, 2u, wBuf);
		}

		return(status);
}

/*!****************************************************************************
*
*  \b              a2b_dscvryPreMasterInit
*
*  Steps taken to init the master node before discovery
*
*  \param          [in]    plugin       plugin specific data
*
*  \pre            None
*
*  \post           The following registers are altered:
*                  A2B_REG_INTMSK0/1/2
*                  A2B_REG_INTPND2
*                  A2B_REG_RESPCYCS
*                  A2B_REG_CONTROL
*                  A2B_REG_PLLCTL
*                  A2B_REG_I2SGCFG
*                  A2B_REG_SWCTL
*                  A2B_REG_DISCVRY
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool
a2b_dscvryPreMasterInit
    (
    a2b_Plugin* plugin
    )
{
    a2b_UInt8 wBuf[4];
    a2b_UInt8 rBuf[3];
    a2b_Bool isAd242x = A2B_FALSE;
	a2b_Bool isAd2428 = A2B_FALSE;
	a2b_Bool isAd243x = A2B_FALSE;
	a2b_Bool isAd2430_8 = A2B_FALSE;
    a2b_UInt8 masterBddIdx = A2B_MASTER_NODEBDDIDX;
    struct a2b_StackContext* ctx = plugin->ctx;
    a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_UInt32 i2cCount = 0u;
    a2b_UInt32 errCode;
    a2b_Int32 nNodeAddr;
	a2b_UInt8 nHPSW_CFG_DET;
	a2b_UInt8 nHPSW_MODE;
	a2b_UInt8 nUserSWCTL;

#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
	a2b_UInt8 levelMask;
#endif

#ifdef A2B_FEATURE_SEQ_CHART
    a2b_Bool bSeqGroupShown = A2B_FALSE;
#endif
	const bdd_Node      *bddNodeObj;

	bddNodeObj = &plugin->bdd->nodes[0];

    /* The one node would be the master node only */
    if ( 1u == plugin->bdd->nodes_count )
    {
        A2B_DSCVRY_ERROR0( ctx, "PreMasterInit", "No slave nodes" );
        return A2B_FALSE;
    }

    A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "PreMasterInit" );

    /* Read the master node's VID/PID/Version */
    wBuf[0] = A2B_REG_VENDOR;
    status  = a2b_regWriteRead( plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 3u, rBuf );
    if ( A2B_FAILED(status) )
    {
        A2B_SEQ_GENNOTE0( plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
                          "Failed to read vid/pid/version of the master node" );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }
    plugin->nodeSig.hasSiliconInfo = A2B_TRUE;
    plugin->nodeSig.siliconInfo.vendorId = rBuf[0u];
    plugin->nodeSig.siliconInfo.productId = rBuf[1u];
    plugin->nodeSig.siliconInfo.version = rBuf[2u];
    if(bddNodeObj->verifyNodeDescr)
	{
		if ((bddNodeObj->nodeDescr.product != plugin->nodeSig.siliconInfo.productId)||
				(bddNodeObj->nodeDescr.version != plugin->nodeSig.siliconInfo.version))
		{
			A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
				"master pid violation");
			A2B_DSCVRY_SEQEND(plugin->ctx);
			a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_PERMISSION);
			return A2B_FALSE;
		}
	}
    isAd242x = a2b_isAd242xChipOnward(plugin->nodeSig.siliconInfo.vendorId,
                                  plugin->nodeSig.siliconInfo.productId);

	isAd2428 = A2B_IS_AD2428X_CHIP(plugin->nodeSig.siliconInfo.vendorId,
								  plugin->nodeSig.siliconInfo.productId);

	isAd243x = A2B_IS_AD243X_CHIP(plugin->nodeSig.siliconInfo.vendorId,
								  plugin->nodeSig.siliconInfo.productId);

	isAd2430_8 = A2B_IS_AD2430_8_CHIP(plugin->nodeSig.siliconInfo.vendorId, 
		                          plugin->nodeSig.siliconInfo.productId);

    A2B_TRACE4( (ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
                        "%s Master Node: Silicon vid/pid/ver: "
                        "%02bX/%02bX/%02bX",
                        A2B_MPLUGIN_PLUGIN_NAME,
                        &(plugin->nodeSig.siliconInfo.vendorId),
                        &(plugin->nodeSig.siliconInfo.productId),
                        &(plugin->nodeSig.siliconInfo.version) ));
    A2B_SEQ_GENNOTE3( plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
                      "Master Node Silicon: vid/pid/ver: %02bX/%02bX/%02bX",
                      &(plugin->nodeSig.siliconInfo.vendorId),
                      &(plugin->nodeSig.siliconInfo.productId),
                      &(plugin->nodeSig.siliconInfo.version) );


    /* Initialize Interrupts */
    /*-----------------------*/
    if (!a2b_dscvryNodeInterruptInit(plugin, (a2b_Int16)masterBddIdx))
    {
        A2B_SEQ_GENNOTE0( plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY, 
                          "Failed to init node interrupts" );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }

#ifdef A2B_FEATURE_COMM_CH
    /* Initialize Mailbox registers  */
    /*-----------------------*/
    if (!a2b_dscvryNodeMailboxInit(plugin, (a2b_Int16)masterBddIdx))
    {
        A2B_SEQ_GENNOTE0( plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
                          "Failed to init mailbox registers" );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }
#endif	/* A2B_FEATURE_COMM_CH */

    /* Clear the DSCDONE pending flag prior to starting discovery */
    wBuf[0] = A2B_REG_INTPND2;
    wBuf[1] = A2B_BITM_INTPND2_DSCDONE;
	status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf);
    i2cCount++;

    /* NOTE: A2B_REG_NODEADR managed by I2C, no need to set it */

    if ( A2B_SUCCEEDED(status) )
    {
        /* Set the response cycle timing */
        wBuf[0] = A2B_REG_RESPCYCS;
        wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[masterBddIdx].ctrlRegs.respcycs;
		status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
        i2cCount++;
    }

	if (A2B_SUCCEEDED(status))
	{
		/* Push respcycs to appropriate nodes */
		wBuf[0] = A2B_REG_CONTROL;
		/* The AD242X (only) needs to be told it's a Master node BEFORE
		 * the PLL locks on the SYNC pin. Once the PLL is locked, setting
		 * the MSTR bit is ignored. We set it anyway so it's clear this is
		 * the master node.
		 */
		wBuf[1] = A2B_ENUM_CONTROL_START_NS;
		if (isAd242x)
		{
			wBuf[1] |= (a2b_UInt8)A2B_ENUM_CONTROL_MSTR;
		}
		if (isAd2428)
		{
			wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[masterBddIdx].ctrlRegs.control & (A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
	    }
		if ((isAd243x) || (isAd2430_8))
		{
			wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[masterBddIdx].ctrlRegs.control & (A2B_ENUM_CONTROL_I2SMSINV | A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
		}

#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
		if (plugin->bdd->nodes[masterBddIdx].nodeDescr.product == 0x37)
		{
			if (plugin->discovery.bFirstStepDisc[masterBddIdx])
			{
				/* Get status of Power Good Signal for two step discovery */
				(void)a2b_stackPalGetPGSignal(ctx, &levelMask);
				if (levelMask)

				{
					/* Power Good signal available*/
					A2B_TRACE1((ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
						"%s a2b_dscvryPreMasterInit(): Power Good Signal available",
						A2B_MPLUGIN_PLUGIN_NAME));
				}
				else
				{
					nNodeAddr = A2B_NODEADDR_MASTER;
					A2B_DSCVRY_ERROR1(ctx, "PreMasterInit",
						"Power Good Signal need detected nodeAddr: %hd",
						&nNodeAddr);
					a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_TWOSTEP_DISC_CRIT_FAULT);
					A2B_DSCVRY_SEQEND(plugin->ctx);
					return A2B_FALSE;
				}

				/* Bypass the switch to use 5V for initial discovery */
				wBuf[1] |= A2B_ENUM_CONTROL_SWBYP;
			}
		}
#endif
		status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
        i2cCount++;

		/* Active delay for new struct */
		a2b_delayForNewStruct(plugin->ctx, A2B_NODEADDR_MASTER);
    }

	if (A2B_SUCCEEDED(status))
	{
		status = a2b_discvryHighPwrInit(plugin, A2B_NODEADDR_MASTER, masterBddIdx);
	}

	/* SWCTL2.HPSW_CFG_DET for AD243x or higher */
	if ((A2B_SUCCEEDED(status)) && (isAd243x))
	{
		/* Reset Config Detect Override */
		plugin->nodeSig.highPwrSwitchModeOverride = 0u; 
		nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[masterBddIdx].ctrlRegs.swctl);

		/* Read the master node's High power switch mode */
		wBuf[0] = A2B_REG_SWSTAT2;
		rBuf[0] = 0;
		status = a2b_regWriteRead( plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 1u, rBuf);

		nHPSW_CFG_DET = ((rBuf[0] & A2B_BITM_SWSTAT2_HPSW_CFG_DET) >> A2B_BITP_SWSTAT2_HPSW_CFG_DET);
		nHPSW_MODE = (plugin->bdd->nodes[masterBddIdx].nodeSetting.eHighPwrSwitchCfg & A2B_BITM_SWCTL2_HPSW_CFG);

		if (A2B_SUCCEEDED(status))
		{
			if (nHPSW_CFG_DET != nHPSW_MODE)
			{
				/* Set the HPSW_MODE in SWCTL2 */
				wBuf[0] = A2B_REG_SWCTL2;
				wBuf[1] = (a2b_UInt8)(plugin->bdd->nodes[masterBddIdx].nodeSetting.eHighPwrSwitchCfg & A2B_BITM_SWCTL2_HPSW_CFG);
				status = a2b_regWrite(ctx,  A2B_NODEADDR_MASTER, 2u, wBuf);
				if (A2B_SUCCEEDED(status))
				{
					nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.swctl);
					plugin->nodeSig.highPwrSwitchModeOverride = 1u;

					/* Set the CFG_DET_OV in SWCTL */
					wBuf[0] = A2B_REG_SWCTL;
					wBuf[1] = (a2b_UInt8)(A2B_BITM_SWCTL_CFG_DET_OV);
					wBuf[1] |= (nUserSWCTL & 0x7Au);

					status = a2b_regWrite(ctx,  A2B_NODEADDR_MASTER, 2u, wBuf);
					if (A2B_FAILED(status))
					{
						A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY, "Failed to write SWCTL of the master node");
						A2B_DSCVRY_SEQEND(plugin->ctx);
						return A2B_FALSE;
					}

#ifdef A2B_SS_STACK
					if (plugin->ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc != A2B_NULL)
					{
						/* In Milli second*/
						plugin->ctx->stk->ecb->palEcb.oAppEcbPal.DelayLogFunc(A2B_HIGH_PWR_SWITCH_DELAY, A2B_NODEADDR_MASTER, A2B_0_0C_HIGH_PWR_SWITCH_DELAY_SCENARIO);
					}
#endif
					/* Delay for 100ms */
					a2b_ActiveDelay(ctx, A2B_HIGH_PWR_SWITCH_DELAY);
				}
				else
				{
					A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY, "Failed to write SWCTL2 of the master node");
					A2B_DSCVRY_SEQEND(plugin->ctx);
					return A2B_FALSE;
				}
			}
		}
		else
		{
			A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY, "Failed to read SWSTAT2 of the master node");
			A2B_DSCVRY_SEQEND(plugin->ctx);
			return A2B_FALSE;
		}
	}

    /* PLLCTL only exists on older AD24XX chips (e.g. 2410) */
    if ( (A2B_SUCCEEDED(status)) && (!isAd242x) )
    {
        /* PLL configuration */
        wBuf[0] = A2B_REG_PLLCTL;
        wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[masterBddIdx].i2cI2sRegs.pllctl;
                 status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
        i2cCount++;
    }

    if ( A2B_SUCCEEDED(status) )
    {
        /* Rev1.0 requirement -- effects the PLL */
        wBuf[0] = A2B_REG_I2SGCFG;
        wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[masterBddIdx].i2cI2sRegs.i2sgcfg;
                 status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
        i2cCount++;
    }

    if ( A2B_SUCCEEDED(status) )
    {
    	nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.swctl);
        wBuf[0] = A2B_REG_SWCTL;
		if (plugin->bdd->nodes[masterBddIdx].nodeSetting.eHighPwrSwitchCfg == bdd_highPwrSwitchCfg_HPSW_CFG_4)
		{
			wBuf[1] = (A2B_ENUM_SWCTL_MODE_NO_DWN_PHAN_PWR | A2B_BITM_SWCTL_ENSW | (a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV));
		}
		else
		{
			wBuf[1] = (A2B_BITM_SWCTL_ENSW | (a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV));
		}
		wBuf[1] |= (nUserSWCTL & A2B_REG_USER_SWCTL);
        status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
        i2cCount++;
    }

    if ( A2B_SUCCEEDED(status) )
    {
        /* Start Discovery of Next Node */
        wBuf[0] = A2B_REG_DISCVRY;
        wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[masterBddIdx+(a2b_UInt8)1].ctrlRegs.respcycs;
                 status = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, wBuf );
        i2cCount++;
    }

    if ( A2B_FAILED(status) )
    {
        A2B_DSCVRY_ERROR1( plugin->ctx, "PreMasterInit", 
                           "I2C failure at operation: %ld", &i2cCount );
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_INTERNAL );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }

	/* Setup/Start the discovery timer */
	if (!a2b_dscvryStartTimer(plugin, TIMER_DSCVRY, A2B_NODEADDR_MASTER))
	{
		A2B_SEQ_GENERROR0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
			"Failed to init discovery timer");
		A2B_DSCVRY_SEQEND(plugin->ctx);
		return A2B_FALSE;
	}

    if(bdd_DISCOVERY_MODE_ADVANCED == a2b_ovrGetDiscMode(plugin))
    {
		a2b_dscvryInitTdmSettings( plugin, A2B_NODEADDR_MASTER );
		status = a2b_audioConfig( plugin->ctx, &plugin->pluginTdmSettings );

		if ( A2B_FAILED(status) )
		{
			nNodeAddr = A2B_NODEADDR_MASTER;
			A2B_UNUSED(nNodeAddr);
			A2B_DSCVRY_ERROR1( plugin->ctx, "dscvryPreMasterInit",
							   "Advanced Discovery - Cannot config audio for nodeAddr: %hd",
							   &nNodeAddr );
			a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_IO );
			A2B_DSCVRY_SEQEND_COND( plugin->ctx, bSeqGroupShown );
			return A2B_FALSE;
		}

		/* Do final setup of the master node and enable the up/downstream */
		if (a2b_dscvryNodeComplete( plugin, A2B_NODEADDR_MASTER,
								 A2B_TRUE, &errCode ) == A2B_EXEC_COMPLETE_FAIL)
		{
			A2B_DSCVRY_ERROR0( plugin->ctx, "dscvryPreMasterInit",
							   "Advanced Discovery - Failed to complete master node init" );
		}
    }
    /* Now we wait for INTTYPE.DSCDONE */

    A2B_DSCVRY_RAWDEBUG0( plugin->ctx, "dscvryPreMasterInit", 
                          "...Waiting for INTTYPE.DSCDONE..." );
    A2B_DSCVRY_SEQEND( plugin->ctx );

    return A2B_TRUE;

} /* a2b_dscvryPreMasterInit */

#ifdef A2B_FEATURE_COMM_CH
/*!****************************************************************************
*
*  \b              a2b_dscvryNodeMailboxInit
*
*  Steps taken to init the mailboxes for enabling communication over mailbox
*
*  \param          [in]    plugin       plugin specific data
*
*  \param          [in]    nodeBddIdx   index of the node
*
*  \pre            None
*
*  \post           The following registers are altered:
*                  A2B_REG_MBOX0CTL
*                  A2B_REG_MBOX1CTL
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool a2b_dscvryNodeMailboxInit (a2b_Plugin* plugin, a2b_Int16 nodeBddIdx)
{
    a2b_Int16 nodeAddr = nodeBddIdx-1;
    a2b_HResult status = A2B_RESULT_SUCCESS;

    if ((nodeBddIdx < 0) || (nodeBddIdx >= (a2b_Int16)plugin->bdd->nodes_count))
    {
        return A2B_FALSE;
    }

    A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                          "Mailbox Registers" );

    if (plugin->bdd->nodes[nodeBddIdx].has_mbox)
    {
        a2b_UInt8 wBuf[4];

        if (plugin->bdd->nodes[nodeBddIdx].mbox.has_mbox0ctl)
        {
            /* Program MBOX 0 register */
            wBuf[0] = A2B_REG_MBOX0CTL;
            wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeBddIdx].mbox.mbox0ctl;
			status = a2b_regWrite( plugin->ctx, nodeAddr, 2u, wBuf );

            if ( A2B_FAILED(status) )
            {
                A2B_DSCVRY_SEQEND( plugin->ctx );
                return A2B_FALSE;
            }
        }

        if (plugin->bdd->nodes[nodeBddIdx].mbox.has_mbox1ctl)
        {
            /* Program MBOX 1 register */
            wBuf[0] = A2B_REG_MBOX1CTL;
            wBuf[1] = (a2b_UInt8)plugin->bdd->nodes[nodeBddIdx].mbox.mbox1ctl;
            status = a2b_regWrite( plugin->ctx, nodeAddr, 2u, wBuf );

            if ( A2B_FAILED(status) )
            {
                A2B_DSCVRY_SEQEND( plugin->ctx );
                return A2B_FALSE;
            }
        }
    }

    A2B_DSCVRY_SEQEND( plugin->ctx );

    return A2B_TRUE;

} /* a2b_dscvryNodeMailboxInit */
#endif	/* A2B_FEATURE_COMM_CH */

#ifdef A2B_FEATURE_SELF_DISCOVERY
/*!****************************************************************************
*
*  \b              a2b_postSelfDscvryInit
*
*  Steps taken to init for configurations of a2b node after self discovery
*
*  \param          [in]    plugin       plugin specific data
*
*  \pre            None
*
*  \post
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool
a2b_postSelfDscvryInit
(
	a2b_Plugin* plugin
)
{
	a2b_UInt8 wBuf[4];
	a2b_UInt8 rBuf[3];
	a2b_HResult status = A2B_RESULT_SUCCESS;
	a2b_Int nIndex = 0;
	a2b_UInt nNodeCount;
#ifdef A2B_FEATURE_SEQ_CHART
	a2b_Bool bSeqGroupShown = A2B_FALSE;
#endif
	const bdd_Node      *bddNodeObj;

	bddNodeObj = &plugin->bdd->nodes[0];
	if (plugin->nodeSig.siliconInfo.version == 0)
	{
		/*Read Num of nodes discovered*/
		wBuf[0] = A2B_REG_DISCSTAT;
		status = a2b_regWriteRead(plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 1u, rBuf);
		nNodeCount = rBuf[0] & A2B_BITM_DISCSTAT_DNODE >> A2B_BITP_DISCSTAT_DNODE;

	}
	else
	{
		wBuf[0] = A2B_REG_BSDSTAT;
		status = a2b_regWriteRead(plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 1u, rBuf);
		nNodeCount = rBuf[0] & A2B_BITM_BSDSTAT_BSDNODE >> A2B_BITP_BSDSTAT_BSDNODE;


	}
	if (A2B_FAILED(status))
	{
		A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
			"Failed to read number of discovered slave (for BSD) of the master node");
		A2B_DSCVRY_SEQEND(plugin->ctx);

		/*Log TRACE and Sequence*/
		return A2B_FALSE;
	}

	plugin->discovery.dscNumNodes = nNodeCount + 1U;

	if (nNodeCount > (plugin->bdd->nodes_count - 1u))
	{
		/*Trigger a message to the user*/
		A2B_DSCVRY_WARN0(plugin->ctx, "postSelfDscvryInit", "Mismatch num of nodes discovered and bdd");
		plugin->discovery.dscNumNodes = plugin->bdd->nodes_count - 1; 	/*-1 because dscNumNodes only includes slave count, */
	}
	else if (nNodeCount < plugin->bdd->nodes_count-1u) /*-1 because nNodeCount doesnt include the master node*/
	{
		status = a2b_intrQueryIrq(plugin->ctx);
		if (A2B_FAILED(status))
		{
			A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
				"Failed to read interrupts");
			A2B_DSCVRY_SEQEND(plugin->ctx);

			/*Log TRACE and Sequence*/

			return A2B_FALSE;
		}
	}

	/*Read and clear pending line fault interrupts on the last slave*/

	/** Check for Fault - TBD **/


	/* Set Discovery Status*/
	plugin->discovery.inDiscovery = A2B_TRUE;
	plugin->discovery.discoveryCompleteCode = A2B_EC_OK;
	plugin->discovery.discoveryComplete = A2B_TRUE;
	plugin->discovery.simpleNodeCount = plugin->discovery.dscNumNodes;
	plugin->discovery.bIsSelfDiscoveredNtwrk = A2B_TRUE;


	/*populate master node signature*/
	if (!a2b_populateNodeSig(plugin, A2B_NODEADDR_MASTER, &plugin->nodeSig))
	{
		/*Log TRACE and Sequence*/
		return A2B_FALSE;
	}

	/* Populate slave node signature plugin->nodeSignature*/
	for (nIndex = plugin->discovery.dscNumNodes-1; nIndex >= 0 ; nIndex--)
	{
		if (!a2b_populateNodeSig(plugin, nIndex, &plugin->slaveNodeSig[nIndex]))
		{
			/*Log TRACE and Sequence*/
			return A2B_FALSE;
		}
	}

	/*Call dscvryNetComplete*/
	a2b_dscvryNetComplete(plugin);

	return A2B_TRUE;
}

/*!****************************************************************************
*
*  \b              a2b_populateNodeSig
*
*  Function to populate the node signature information such as vendorId, productId, silicon version by reading the A2B node
*
*  \param          [in]    plugin       Plugin specific data
*  \param          [in]    nodeAddr     -1=master, 0=slave0, 1=slave1, etc
*  \param          [in]    nodeSig      Pointer to signature structure used to uniquely describe a master/slave node.
*
*  \pre            None
*
*  \post
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_HResult a2b_populateNodeSig(a2b_Plugin* plugin, a2b_Int16 nodeAddr, a2b_NodeSignature* nodeSig)
{
	a2b_UInt8 wBuf[4];
	a2b_UInt8 rBuf[3];
	a2b_HResult status = A2B_RESULT_SUCCESS;

	wBuf[0] = A2B_REG_VENDOR;
	
	status = a2b_regWriteRead(plugin->ctx, nodeAddr, 1u, wBuf, 3u, rBuf);
	
	
	if (A2B_FAILED(status))
	{
		A2B_SEQ_GENNOTE0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
			"Failed to read vid/pid/version of the master node");
		A2B_DSCVRY_SEQEND(plugin->ctx);
		return A2B_FALSE;
	}

	A2B_INIT_SIGNATURE(nodeSig, nodeAddr);

	(*nodeSig).hasSiliconInfo = A2B_TRUE;
	(*nodeSig).siliconInfo.vendorId = rBuf[0u];
	(*nodeSig).siliconInfo.productId = rBuf[1u];
	(*nodeSig).siliconInfo.version = rBuf[2u];

	(*nodeSig).hasBddInfo = A2B_TRUE;
	(*nodeSig).bddInfo.vendorId = plugin->bdd->nodes[nodeAddr + 1].nodeDescr.vendor;
	(*nodeSig).bddInfo.productId = plugin->bdd->nodes[nodeAddr + 1].nodeDescr.product;
	(*nodeSig).bddInfo.version = plugin->bdd->nodes[nodeAddr + 1].nodeDescr.version;
	

	return A2B_TRUE;
}
#endif /*A2B_FEATURE_SELF_DISCOVERY */
/*!****************************************************************************
*
*  \b               a2b_getNodeSignature
*
*  Returns the node signature information for the specified node address.
*
*  \param           [in]    plugin      The A2B master plugin instance.
*
*  \param           [in]    nodeAddr    The node address to retrieve its
*                                       signature. Slave node addresses are
*                                       in the range [0, N] while the master
*                                       node is A2B_NODEADDR_MASTER.
*
*  \pre             None
*
*  \post            None
*
*  \return          Returns a pointer to the constant node signature structure.
*                   The caller should **not** attempt to free this structure.
*
******************************************************************************/
static const a2b_NodeSignature*
a2b_getNodeSignature
    (
    a2b_Plugin* plugin,
    a2b_Int16   nodeAddr
    )
{
    const a2b_NodeSignature*    nodeSig;
    a2b_UInt32                  iter = 0u;

    /* Search for the node signature information for this slave node */
    do
    {
        nodeSig = a2b_msgRtrGetHandler(plugin->ctx, &iter);
        if ( (nodeSig != A2B_NULL) &&
            (nodeSig->nodeAddr == nodeAddr) )
        {
            break;
        }
    }
    while ( A2B_NULL != nodeSig );

    return nodeSig;
} /* a2b_getNodeSignature */


/*!****************************************************************************
*
*  \b              a2b_dscvryFindNodeHandler
*
*  If a search hasn't been done to find a handler (plugin) to manage this
*  node then try to find one. If this search has already been done then the
*  function returns immediately.
*
*  \param          [in]    plugin       The master plugin specific data.
*
*  \param          [in]    slaveNodeIdx Zero-based index into a slave node
*                                       array where index 0 is the first
*                                       slave, index 1 is second slave, etc...
*
*  \pre            None
*
*  \post           If a handler (e.g. plugin) has been found for this node
*                  then it is bound to this node.
*
*  \return         None
*
******************************************************************************/
static void
a2b_dscvryFindNodeHandler
    (
    a2b_Plugin* plugin,
    a2b_UInt16  slaveNodeIdx
    )
{
    a2b_HResult                 status;
    const a2b_NodeSignature*    nodeSig = &plugin->slaveNodeSig[slaveNodeIdx];

    /* For this use-case the slave node index is identical to the node
     * address.
     */
    a2b_Int16                   nodeAddr =
                                    A2B_MAP_SLAVE_INDEX_TO_ADDR(slaveNodeIdx);

    /* If we've already searched for a handler to manage this plugin
     * then ...
     */
    if ( A2B_HAS_SEARCHED_FOR_HANLDER(plugin, nodeAddr) )
    {
        /* No need to search again */
        return;
    }

    /* Look for a plugin to manage this node */
    status = a2b_stackFindHandler( plugin->ctx, nodeSig);

    /* Mark it as being searched - whether successful or not */
    plugin->discovery.hasSearchedForHandler |= ((a2b_UInt32)1 << (a2b_UInt32)nodeAddr);

    if ( A2B_FAILED(status) )
    {
        A2B_TRACE2( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_DEBUG),
                    "%s findNodeHandler(): No plugin for node: %hd",
                    A2B_MPLUGIN_PLUGIN_NAME, &nodeAddr ));
    }
    else
    {
        /* Track that a plugin exists and will manage this node */
        plugin->discovery.hasPlugin |= ((a2b_UInt32)1 << (a2b_UInt32)nodeAddr);
        plugin->discovery.needsPluginInit |= ((a2b_UInt32)1 << (a2b_UInt32)nodeAddr);

        /* Get the updated signature with the actual plugin name */
        nodeSig = a2b_getNodeSignature(plugin, nodeAddr);
        A2B_SEQ_GENNOTE2( plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
                          "Plugin '%s' opened to manage node: %hd",
                          nodeSig->pluginName, &nodeAddr);
    }
}

/*!****************************************************************************
*
*  \b              a2b_dscvryVMTRRead
*
*  Function to read VMTR registers.
*
*  \param          [in]    plugin   plugin specific data
*
*  \dscNodeAddr    [in]    Discovered Node Address
*
*  \return         1=error
*                  0=success
*
******************************************************************************/
static a2b_Bool
a2b_dscvryVMTRRead
(
	a2b_Plugin* plugin, 
	a2b_Int16  dscNodeAddr
)
{
	a2b_HResult nRet = A2B_AD243x_VMTR_INRANGE;
	a2b_UInt8 	rBuf[4u];
	a2b_UInt8 	wBuf[4u];
	a2b_HResult status = A2B_RESULT_SUCCESS;
	a2b_UInt8 maxThres, minThres, measVolt;

	wBuf[0] = A2B_REG_MMRPAGE;
	wBuf[1] = 0x01;
	status |= a2b_regWrite(plugin->ctx, (dscNodeAddr), 2u, wBuf);

	wBuf[0] = A2B_REG_VMTR_MXSTAT & 0xFF;
	status |= a2b_regWriteRead(plugin->ctx, (dscNodeAddr), 1u, wBuf, 1u, rBuf);
	maxThres = rBuf[0];

	wBuf[0] = A2B_REG_VMTR_MNSTAT & 0xFF;
	status |= a2b_regWriteRead(plugin->ctx, (dscNodeAddr), 1u, wBuf, 1u, rBuf);
	minThres = rBuf[0];

	wBuf[0] = A2B_REG_VMTR_VLTG1 & 0xFF; // Read the measured voltage
	status |= a2b_regWriteRead(plugin->ctx, (dscNodeAddr), 1u, wBuf, 1u, rBuf);
	measVolt = rBuf[0];

	wBuf[0] = A2B_REG_MMRPAGE;
	wBuf[1] = 0x0;
	status |= a2b_regWrite(plugin->ctx, (dscNodeAddr), 2u, wBuf);

	if (A2B_FAILED(status))
	{
		status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_I2C, A2B_EC_INTERNAL);
		A2B_DSCVRY_ERROR1(plugin->ctx, "a2b_dscvryVMTRRead: ",
			"Failed reading VMTR registers for NodeAddr: %hd", &dscNodeAddr); 
	}
	else
	{
		if (maxThres != 0)
		{
			nRet = A2B_AD243x_VMTR_OVERFLOW;
		}
	}

	return (nRet);
}

/*!****************************************************************************
*
*  \b              a2b_dscvryVMTREnable
*
*  Enable VMTR on AD243x sub nodes
*
*  \param          [in]    plugin   plugin specific data
*
*  \nNode    [in]    Discovered Node Address
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static void
a2b_dscvryVMTREnable
(
	a2b_Plugin* plugin,
	a2b_Int16 nNode
)
{
	a2b_HResult nRet = 0U;
	a2b_UInt8 	rBuf[4u];
	a2b_UInt8 	wBuf[4u];
	a2b_HResult status = A2B_RESULT_SUCCESS;
	
	wBuf[0] = A2B_REG_MMRPAGE;
	wBuf[1] = 0x01;
	status = a2b_regWrite(plugin->ctx, (nNode), 2, wBuf);

	wBuf[0] = A2B_REG_VMTR_VEN & 0xFF;
	wBuf[1] = 0x02; // Enable VBUS
	status |= a2b_regWrite(plugin->ctx, (nNode), 2, wBuf);

	wBuf[0] = A2B_REG_VMTR_VMIN1 & 0xFF;
	wBuf[1] = 0x0; // program Min voltage as 0
	status |= a2b_regWrite(plugin->ctx, (nNode), 2, wBuf);

	wBuf[0] = A2B_REG_VMTR_VMAX1 & 0xFF;
	wBuf[1] = 0x50; // program Max voltage for 10V
	status |= a2b_regWrite(plugin->ctx, (nNode), 2, wBuf);

	wBuf[0] = A2B_REG_MMRPAGE;
	wBuf[1] = 0x0;
	status |= a2b_regWrite(plugin->ctx, (nNode), 2, wBuf);

	if(A2B_FAILED(status))
	{
		A2B_DSCVRY_ERROR1(plugin->ctx, "a2b_dscvryVMTREnable: ",
			"Failed enabling VMTR registers for NodeAddr: %hd", &nNode);
	}
}


/*!****************************************************************************
*
*  \b              a2b_dscvryNodeDiscovered
*
*  Called after a node has been discovered.  This can be used to setup
*  the new node depending on the discovery process being used.
*
*  \param          [in]    plugin   plugin specific data
*
*  \pre            Called on DSCDONE
*
*  \post           The following registers are altered:
*                  A2B_REG_SWCTL 
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
a2b_Bool
a2b_dscvryNodeDiscovered
    (
    a2b_Plugin* plugin
    )
{
    a2b_Bool 	bRet = A2B_TRUE, bNetConfigFlag = A2B_FALSE;
    a2b_UInt8 	wBuf[4u];
    a2b_UInt8 	rBuf[8u];
    a2b_Int16	 idx;
    a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_Int16 	dscNodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes;
    a2b_Int16 	dscNodeIdx = dscNodeAddr+1;
    struct a2b_StackContext* ctx = plugin->ctx;
    bdd_DiscoveryMode 	eDiscMode;
	a2b_NodeSignature   nodeSig;
	const bdd_Node      *bddPrevNodeObj;
	const bdd_Node      *bddNodeObj;
	a2b_Bool 			verifyNodeDescr;
	a2b_Bool 			bContinueDisc;
	a2b_Bool 			isAd243xMainNode = A2B_FALSE;
	a2b_Bool isAd2430_8 = A2B_FALSE;
	a2b_Bool 			isXTalkFixApplicable;
	a2b_UInt8 			mstrBit;
	a2b_Bool	bIsAD2430_8MasterPresent = A2B_FALSE;
	a2b_UInt8 nRet = 0;
	a2b_UInt8 ControlReg = 0u;
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)
    a2b_UInt8 			nCfgBlocks = 0u;
    a2b_UInt16 			nOffset;
#endif	/* defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO) */
#endif	/* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
    a2b_UInt8 IsStdPowerLstNode=0;
    a2b_UInt8 Readbuf[8];
    a2b_UInt8 Result;
#endif

    eDiscMode = a2b_ovrGetDiscMode(plugin);
    if (!plugin->discovery.inDiscovery)
    {
        /* Ignore DSCDONE when we are NOT discovering */
        return A2B_FALSE;
    }

    A2B_DSCVRY_SEQGROUP1( ctx,
                          "NodeDiscovered nodeAddr %hd", &dscNodeAddr);

    /* Stop the previously running timer */
    a2b_timerStop( plugin->timer );

	bddPrevNodeObj = &plugin->bdd->nodes[dscNodeAddr];
	bddNodeObj = &plugin->bdd->nodes[dscNodeIdx];
	/*Check whether upstream node is AD2425 or AD2410 series */
	isXTalkFixApplicable = a2b_isCrossTalkFixApply(bddPrevNodeObj->nodeDescr.vendor, bddPrevNodeObj->nodeDescr.product);
	isAd2430_8 = a2b_isAd2430_8_Chip(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
	isAd243xMainNode = a2b_isAd243xChip(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
	
	if ((plugin->bdd->policy.bCrossTalkFixApply) && (dscNodeIdx != 1) && (isXTalkFixApplicable == A2B_TRUE))
	{
		A2B_DSCVRY_SEQGROUP0(ctx, "A2B CrossTalk Fix");

		if ((plugin->nodeSig.hasSiliconInfo) &&
			((a2b_isAd242xChipOnward(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId))))
		{
			 mstrBit = (a2b_UInt8)A2B_ENUM_CONTROL_MSTR;
		}

		wBuf[0] = A2B_REG_RESPCYCS;
		wBuf[1] = plugin->bdd->nodes[0].ctrlRegs.respcycs + 1u;
		status = a2b_regWrite(ctx, A2B_NODEADDR_MASTER, 2, wBuf);

		for (idx = 1u; idx <= dscNodeIdx; idx++)
		{
			wBuf[0] = A2B_REG_RESPCYCS;
			wBuf[1] = plugin->bdd->nodes[idx].ctrlRegs.respcycs + 1u;
			status = a2b_regWrite(ctx, (idx - 1), 2u, wBuf);
		}

		wBuf[0] = A2B_REG_CONTROL;
		wBuf[1] = A2B_ENUM_CONTROL_START_NS;
		wBuf[1] |= mstrBit;

		if((isAd243xMainNode) || (isAd2430_8))
		{
			wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.control & (A2B_ENUM_CONTROL_I2SMSINV | A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
		}
		else
		{
			wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.control & (A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
		}
		status = a2b_regWrite(ctx, A2B_NODEADDR_MASTER, 2, wBuf);

		/* Active delay for new struct */
		a2b_delayForNewStruct(ctx, A2B_NODEADDR_MASTER);

		wBuf[0] = A2B_REG_RESPCYCS;
		wBuf[1] = plugin->bdd->nodes[0].ctrlRegs.respcycs;
		status = a2b_regWrite(ctx, A2B_NODEADDR_MASTER, 2, wBuf);

		for (idx = 1u; idx <= dscNodeIdx; idx++)
		{
			wBuf[0] = A2B_REG_RESPCYCS;
			wBuf[1] = plugin->bdd->nodes[idx].ctrlRegs.respcycs;
			status = a2b_regWrite(ctx, (idx - 1), 2, wBuf);
		}

		wBuf[0] = A2B_REG_CONTROL;
		wBuf[1] = A2B_ENUM_CONTROL_START_NS;
		wBuf[1] |= mstrBit;

		if ((isAd243xMainNode) || (isAd2430_8))
		{
			wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.control & (A2B_ENUM_CONTROL_I2SMSINV | A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
		}
		else
		{
			wBuf[1] |= (a2b_UInt8)(plugin->bdd->nodes[0].ctrlRegs.control & (A2B_ENUM_CONTROL_XCVRBINV | A2B_ENUM_CONTROL_SWBYP));
		}
		status = a2b_regWrite(ctx, A2B_NODEADDR_MASTER, 2, wBuf);

		/* Active delay for new struct */
		a2b_delayForNewStruct(ctx, A2B_NODEADDR_MASTER);

		A2B_DSCVRY_SEQEND(plugin->ctx);
	}
	

    /* Enable phantom power with external switch mode
     *
     * NOTE: v3 ADI documentation shows the Simple Discovery flow  
     *       sending this only to the Master, which the documentation
     *       verbiage says to send it to the slave.  We will send it to
     *       the slave node. 
     */
    wBuf[0] = A2B_REG_SWCTL;
    wBuf[1] = A2B_BITM_SWCTL_ENSW | A2B_ENUM_SWCTL_MODE_VOLT_ON_VIN;
	status = a2b_regWrite(ctx, (dscNodeAddr - 1), 2u, wBuf);

#ifdef A2B_SS_STACK
	if (plugin->ctx->stk->ecb->palEcb.oAppEcbPal.PreCustomNodeConfig != A2B_NULL)
	{
		plugin->ctx->stk->ecb->palEcb.oAppEcbPal.PreCustomNodeConfig(dscNodeAddr - 1);
	}
#endif

    /* NOTE: A2B_REG_NODEADR managed by I2C, no need to set it */
    A2B_DSCVRY_SEQGROUP0( ctx,
                          "A2B VID/PID/VERSION/CAP" );

    /* Read the new nodes VID/PID/VERSION/CAP */
    wBuf[0] = A2B_REG_VENDOR;
    status  = a2b_regWriteRead( ctx, dscNodeAddr, 1u, wBuf, 4u, rBuf );

    A2B_DSCVRY_SEQEND( plugin->ctx );

    if ( A2B_SUCCEEDED(status) )
    {
#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
        a2b_UInt16          capability = rBuf[3u];
#endif

        A2B_INIT_SIGNATURE( &nodeSig, dscNodeAddr );

        /* Silicon VID/PID/VER */
        nodeSig.hasSiliconInfo         = A2B_TRUE;
        nodeSig.siliconInfo.vendorId   = rBuf[0u];
        nodeSig.siliconInfo.productId  = rBuf[1u];
        nodeSig.siliconInfo.version    = rBuf[2u];

        A2B_TRACE6( (ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
                    "%s nodeDiscovered(): Silicon node/vid/pid/ver/cap: "
                    "%02hX/%02bX/%02bX/%02bX/%02bX",
                    A2B_MPLUGIN_PLUGIN_NAME, &dscNodeAddr,
                    &nodeSig.siliconInfo.vendorId,
                    &nodeSig.siliconInfo.productId,
                    &nodeSig.siliconInfo.version,
                    &capability ));

        A2B_SEQ_GENNOTE4( plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
                          "Silicon vid/pid/ver/cap: %02bX/%02bX/%02bX/%02bX", 
                          &nodeSig.siliconInfo.vendorId,
                          &nodeSig.siliconInfo.productId,
                          &nodeSig.siliconInfo.version,
                          &capability );

        /* Verify the stack supports this node */
        if (!a2b_stackSupportedNode(   nodeSig.siliconInfo.vendorId,
                                       nodeSig.siliconInfo.productId, 
                                       nodeSig.siliconInfo.version ))
        {
            A2B_DSCVRY_ERROR5( ctx, "nodeDiscovered",
                               "Incompatible node %hd (%02bX/%02bX/%02bX/%02bX)", 
                               &dscNodeAddr,
                               &nodeSig.siliconInfo.vendorId, 
                               &nodeSig.siliconInfo.productId, 
                               &nodeSig.siliconInfo.version,
                               &capability );

            bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);
            if((bdd_DISCOVERY_MODE_SIMPLE == eDiscMode) && (bNetConfigFlag))
            {
            	plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_PERMISSION;
            	a2b_dscvryNetComplete(plugin);
            }
            else
            {
				a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_PERMISSION );
            }

            A2B_DSCVRY_SEQEND( plugin->ctx );
            return A2B_FALSE;
        }

		/* Mandatory checking for AD2420 & AD2429 */
		verifyNodeDescr = (a2b_Bool)bddNodeObj->verifyNodeDescr;
        /* Optionally validate the node descriptor info */
        if ((verifyNodeDescr) &&
            (( bddNodeObj->nodeDescr.product !=
                                   nodeSig.siliconInfo.productId ) ||
             ( bddNodeObj->nodeDescr.version !=
                                   nodeSig.siliconInfo.version )) )
        {

			/* Copy the signature information to the plugin */
			plugin->slaveNodeSig[dscNodeAddr] = nodeSig;
            A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered",
                              "Failed Authentication ",
                              &dscNodeAddr );

            bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);
            if((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
            {
            	plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_PERMISSION;
            	a2b_dscvryNetComplete(plugin);
            }
            else
            {
				a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_PERMISSION );
            }
            A2B_DSCVRY_SEQEND( plugin->ctx );
            return A2B_FALSE;
        }

        nodeSig.hasI2cCapability = (a2b_Bool)((rBuf[3u] & A2B_BITM_CAPABILITY_I2CAVAIL) != 0u);

#ifdef A2B_ENABLE_SUPPORT_TWO_STEP_DISCOVERY
		if(bddNodeObj->nodeDescr.product == 0x37)
		{
			if(plugin->discovery.bFirstStepDisc[dscNodeAddr])
			{
				/* A range of EEPROMs is accessed to check if there is an EEPROM available with a valid content for two step discovery */
				for(idx = A2B_FIRST_EEPROM_ADDR; idx <= A2B_LAST_EEPROM_ADDR; idx++)
				{
					/* Read EEPROM to Verify the RJ45 Node
					 *  - Verify valid module information */
					wBuf[0]= 0x00;
					wBuf[1]= A2B_MODINFO_ID_ADDR;
					/* Read first 7 bytes */
					Result = a2b_periphWriteRead(ctx,dscNodeAddr, idx, 0x02, &wBuf[0], 0x07, &Readbuf[A2B_MODINFO_ID_ADDR]);
					if( (Result != 0) ||
						(Readbuf[A2B_MODINFO_ID_ADDR] != A2B_MODINFO_ID_VAL) ||
						(Readbuf[A2B_MODINFO_VID_ADDR] != A2B_MODINFO_VID_VAL) ||
						(Readbuf[A2B_MODINFO_PID_ADDR] != A2B_MODINFO_PID_VAL))
					{
						/* Valid EEPROM content is not found, check another EEPROM */
						continue;
					}
					else
					{
						/* EEPROM with valid content found */
						break;
					}
				}

				/* Couldn't find the valid EEPROM content for two step discovery */
				if(idx > A2B_LAST_EEPROM_ADDR)
				{
					/* Reject the device */
					A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered",
									   "Node %hd TwoStep Discovery : Valid EEPROM not found",
									   &dscNodeAddr );
					a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_TWOSTEP_DISC_FAILED );
					A2B_DSCVRY_SEQEND( plugin->ctx );
					return A2B_FALSE;
				}

				wBuf[1]= A2B_BLOCK_MODINFO_POWERCONFIG_ADDR;
				/* Read 6 bytes */
				Result = a2b_periphWriteRead(ctx,dscNodeAddr, idx, 0x02, &wBuf[0], 0x06, &Readbuf[0u]);

				/* Check the power capability */
				if((Readbuf[0u] >> 4u) != A2B_BLOCK_MODINFO_POWERCONFIG)
				{
					/* Last node can be a standard power node. In this case, discovery with high voltage will not be done */
					if (plugin->discovery.dscNumNodes + (a2b_UInt8)1 >= (a2b_UInt8)plugin->bdd->nodes_count)
					{
						IsStdPowerLstNode = A2B_TRUE;
					}
					else
					{
						/* Reject the device */
						A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered",
										   "Node %hd TwoStep Discovery : EEPROM found but content is not valid",
										   &dscNodeAddr );
						a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_TWOSTEP_DISC_FAILED );
						A2B_DSCVRY_SEQEND( plugin->ctx );
						return A2B_FALSE;
					}
				}

				plugin->discovery.bFirstStepDisc[dscNodeAddr] = 0;

				/* Continue if this is not a standard power last node */
				if(IsStdPowerLstNode != A2B_TRUE)
				{
					/* Disable power on the B side of the node */
					a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[dscNodeAddr-1].ctrlRegs.swctl);
					wBuf[0] = A2B_REG_SWCTL;
					wBuf[1] = ((nUserSWCTL & A2B_REG_USER_SWCTL) | (a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV));
					wBuf[1] &= (~(A2B_BITM_SWCTL_ENSW));
					(void)a2b_regWrite(plugin->ctx, dscNodeAddr-1, 2u, wBuf);

					/* Continue with the second discovery step  */
					if(dscNodeAddr==0)
					{
						(void)a2b_dscvryPreMasterInit(plugin);
					}
					else
					{
						(void)a2b_dscvryPreSlaveInit(plugin);
					}

					return A2B_FALSE;
				}
			}
			else /* check if 24V is disconnected */
			{
				wBuf[0] = A2B_REG_MMRPAGE;
				wBuf[1] = 0x01;
				(void)a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);
				wBuf[0] = A2B_REG_VMTR_VEN & 0xFF;
				wBuf[1] = 0x02;
				(void)a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);
				//status = a2b_regWriteRead(ctx, dscNodeAddr, 2u, wBuf, 1u, rBuf);
				wBuf[0] = A2B_REG_MMRPAGE;
				wBuf[1] = 0x00;
				(void)a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);
				wBuf[0] = A2B_REG_MMRPAGE;
				wBuf[1] = 0x01;
				(void)a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);

				wBuf[0] = A2B_REG_VMTR_VLTG1 & 0xFF;
				status = a2b_regWriteRead(ctx, dscNodeAddr, 1u, wBuf, 1u, rBuf);

				wBuf[0] = A2B_REG_MMRPAGE;
				wBuf[1] = 0x00;
				(void)a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);

				/* Threshole need to be defined */
				if (rBuf[0] >= A2B_24V_DISCONECT_THRESHOLD)
				{
					/* Do nothing */
				}
				else
				{
					a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_TWOSTEP_DISC_24V_DISC);
				}
			}
		}
#endif

		/* Custom Node Identification - Start */
		if(bddNodeObj->nodeDescr.oCustomNodeIdSettings.bCustomNodeIdAuth == A2B_TRUE)
		{
			/* Authenticate by reading from memory */
			if(bddNodeObj->nodeDescr.oCustomNodeIdSettings.bReadFrmMemory == A2B_TRUE)
			{
				bRet = a2b_dscvryCustomAuthFrmMem(plugin, nodeSig);
				if (bRet == A2B_FALSE)
				{
					return (A2B_FALSE);
				}
			}

			/* Authenticate by GPIO pins */
			if(bddNodeObj->nodeDescr.oCustomNodeIdSettings.bReadGpioPins == A2B_TRUE)
			{
				bRet = a2b_dscvryCustomAuthFrmGpio(plugin, nodeSig);
				if (bRet == A2B_FALSE)
				{
					return (A2B_FALSE);
				}
			}

#ifdef A2B_FEATURE_COMM_CH
			/* Authenticate by getting the authentication message via communication channel */
			if(bddNodeObj->nodeDescr.oCustomNodeIdSettings.bReadFrmCommCh == A2B_TRUE)
			{
				bRet = a2b_dscvryCustomAuthFrmCommCh(plugin, nodeSig);
				if (bRet == A2B_FALSE)
				{
					return (A2B_FALSE);
				}
			}
#endif	/* A2B_FEATURE_COMM_CH */
		}
		/* Custom Node Identification - End */

#ifdef A2B_FEATURE_COMM_CH
		/* Authenticate by getting the authentication message via communication channel */
		if(bddNodeObj->nodeDescr.oCustomNodeIdSettings.bReadFrmCommCh == A2B_FALSE)
		{
#endif	/* A2B_FEATURE_COMM_CH */
			/* Used only during simple discovery with sync periph processing */
			plugin->discovery.simpleNodeCount++;
			plugin->discovery.dscNumNodes++;

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
        if ((( nodeSig.hasI2cCapability ) && ( !bddNodeObj->ignEeprom ) &&
             ( ( plugin->overrides[0u] & A2B_MPLUGIN_IGN_EEPROM ) == 0u )) ||
            ( plugin->periphCfg.nodeCfg[dscNodeAddr] ))
        {
#if !defined(A2B_BCF_FROM_SOC_EEPROM) && !defined(A2B_BCF_FROM_FILE_IO)
            A2B_DSCVRY_SEQGROUP0( ctx,
                                  "Look for EEPROM at 0x50" );

            /* Read the EEPROM header             */
            /* [Two byte internal EEPROM address] */
            wBuf[0] = 0u;
            wBuf[1] = 0u;
            status = a2b_periphCfgWriteRead( plugin,
                                             dscNodeAddr,
                                             2u,  wBuf,
                                             8u,  rBuf);

            if (( A2B_SUCCEEDED(status) ) &&
                ( rBuf[0] == A2B_MARKER_EEPROM_CONFIG ))
            {
                a2b_UInt8 crc8 = a2b_crc8(rBuf, 0u, 7u);

                if ( crc8 == rBuf[7] )
                {
                    /* EEPROM VID/PID/VER */
                    nodeSig.hasEepromInfo        = A2B_TRUE;
                    nodeSig.eepromInfo.vendorId  = rBuf[1];
                    nodeSig.eepromInfo.productId = rBuf[2];
                    nodeSig.eepromInfo.version   = rBuf[3];

                    A2B_TRACE5( (ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
                                "%s nodeDiscovered(): EEPROM node/vid/pid/ver: "
                                "%02hX/%02bX/%02bX/%02bX",
                                A2B_MPLUGIN_PLUGIN_NAME, &dscNodeAddr,
                                &nodeSig.eepromInfo.vendorId,
                                &nodeSig.eepromInfo.productId,
                                &nodeSig.eepromInfo.version ));

                    A2B_SEQ_GENNOTE3( plugin->ctx, 
                                      A2B_SEQ_CHART_LEVEL_DISCOVERY,
                                      "EEPROM vid/pid/ver: %02bX/%02bX/%02bX", 
                                      &nodeSig.eepromInfo.vendorId,
                                      &nodeSig.eepromInfo.productId,
                                      &nodeSig.eepromInfo.version );

                    /* When this override is set we will parse the version
                     * (which we did) but we will not indicate that there 
                     * was an EEPRROM so we will avoid peripheral processing
                     */
                    if (( plugin->overrides[0] & A2B_MPLUGIN_EEPROM_VER_ONLY ) == 0u )
                    {
                        /* See if we have an override for this specific node */
                        if (( plugin->overrides[1] & (a2b_UInt32)((a2b_UInt32)1u << dscNodeAddr)) == (a2b_UInt32)0u )
                        {
                            plugin->discovery.hasEeprom |= (a2b_UInt32)((a2b_UInt32)1u << dscNodeAddr);
                        }
                        else
                        {
                            A2B_DSCVRYNOTE_DEBUG1( plugin->ctx, 
                                       "nodeDiscovered",
                                       "Override Set, Ignoring node %hd EEPROM",
                                       &dscNodeAddr );
                        }
                    }

                    /* Optionally validate the node descriptor info */
                    if (( bddNodeObj->verifyNodeDescr ) &&
                        (( bddNodeObj->nodeDescr.vendor != 
                                               nodeSig.eepromInfo.vendorId ) ||
                         ( bddNodeObj->nodeDescr.product != 
                                               nodeSig.eepromInfo.productId ) ||
                         ( bddNodeObj->nodeDescr.version != 
                                               nodeSig.eepromInfo.version )) )
                    {
                        /* clear the bit */
                        plugin->discovery.hasEeprom ^= (a2b_UInt32)((a2b_UInt32)1u << dscNodeAddr);

                        A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered", 
                                          "Node %hd EEPROM failed verification",
                                          &dscNodeAddr );

                        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_PERMISSION );
                        A2B_DSCVRY_SEQEND( plugin->ctx );
                        return A2B_FALSE;
                    }
                }
                else
                {
                    A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered", 
                                       "Node %hd EEPROM header CRC incorrect",
                                       &dscNodeAddr );
                }
            }
            else
            {
                A2B_DSCVRYNOTE_DEBUG1( ctx, "nodeDiscovered", 
                                       "Node %hd EEPROM not found",
                                       &dscNodeAddr );
            }

            A2B_DSCVRY_SEQEND( plugin->ctx );
#else
			/* To read the number of config blocks from the EEPROM connected to host */
			A2B_GET_UINT16_BE(nOffset, plugin->periphCfg.pkgCfg , 2* dscNodeAddr);
			nOffset += 3u;
			A2B_PUT_UINT16_BE(nOffset, wBuf,0);
			status  = a2b_periphCfgWriteRead( plugin,
											  dscNodeAddr,
											  2u,  wBuf,
											  1u,  &nCfgBlocks );

			if(nCfgBlocks != 0U)
			{
            plugin->discovery.hasEeprom |= (a2b_UInt32)((a2b_UInt32)1u << dscNodeAddr);
			}
#endif
        }
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

        /* BDD VID/PID/VER */
        nodeSig.hasBddInfo        = A2B_TRUE;
        nodeSig.bddInfo.vendorId  = (a2b_UInt8)bddNodeObj->nodeDescr.vendor;
        nodeSig.bddInfo.productId = (a2b_UInt8)bddNodeObj->nodeDescr.product;
        nodeSig.bddInfo.version   = (a2b_UInt8)bddNodeObj->nodeDescr.version;

        A2B_TRACE5( (ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
                    "%s nodeDiscovered(): BDD node/vid/pid/ver: "
                    "%02hX/%02bX/%02bX/%02bX",
                    A2B_MPLUGIN_PLUGIN_NAME, &dscNodeAddr,
                    &nodeSig.bddInfo.vendorId,
                    &nodeSig.bddInfo.productId,
                    &nodeSig.bddInfo.version ));

        A2B_SEQ_GENNOTE3( plugin->ctx, 
                          A2B_SEQ_CHART_LEVEL_DISCOVERY,
                          "BDD vid/pid/ver: %02bX/%02bX/%02bX", 
                          &nodeSig.bddInfo.vendorId,
                          &nodeSig.bddInfo.productId,
                          &nodeSig.bddInfo.version );

        /* Copy the signature information to the plugin */
        plugin->slaveNodeSig[dscNodeAddr] = nodeSig;

#ifndef FIND_NODE_HANDLER_AFTER_NODE_INIT
        /* We'll attempt to find and open a plugin that can manage the
         * slave node *before* completing the full initialization of the
         * node.
         */
        a2b_dscvryFindNodeHandler(plugin, dscNodeAddr);
#endif
#ifdef A2B_FEATURE_COMM_CH
		}	/* If bReadFrmCommCh == A2B_FALSE */
#endif	/* A2B_FEATURE_COMM_CH */

    }	/* Successful reading of VID, PID */
    else
    {
        A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered",
                           "Cannot read VID/PID/VER/CAP for node: %hd",
                           &dscNodeAddr );
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_PERMISSION );
        A2B_DSCVRY_SEQEND( plugin->ctx );
        return A2B_FALSE;
    }
    A2B_DSCVRY_SEQEND( plugin->ctx );

	a2b_CheckIfAD2430_8NodeMasterPrsnt(plugin, &bIsAD2430_8MasterPresent);
	if (bIsAD2430_8MasterPresent && (bddNodeObj->nodeDescr.product == 0x33))
	{
		plugin->discovery.bAd2430VmtrFault = false;
		/* Enable Voltage Monitor here */ 
		a2b_dscvryVMTREnable(plugin, dscNodeAddr);
		/* Read Voltage Monitor */
		nRet = a2b_dscvryVMTRRead(plugin, dscNodeAddr);
		if (nRet == A2B_AD243x_VMTR_OVERFLOW)
		{
			plugin->discovery.bAd2430VmtrFault = true;
			/*a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_POWER_DIAG_FAILURE);*/

			return false;

		}
	}

#ifdef A2B_FEATURE_COMM_CH
	if(bddNodeObj->nodeDescr.oCustomNodeIdSettings.bReadFrmCommCh == A2B_FALSE)
	{
#endif	/* A2B_FEATURE_COMM_CH */
		if ( bdd_DISCOVERY_MODE_MODIFIED == eDiscMode )
		{
			bRet = adi_a2b_ConfigureNodePeri(plugin, dscNodeAddr);

		} /* end Modified processing */
		else if((bdd_DISCOVERY_MODE_OPTIMIZED == eDiscMode) ||
				(bdd_DISCOVERY_MODE_ADVANCED == eDiscMode))
		{
			if(plugin->discovery.pendingPluginInit == (a2b_UInt8)0)
			{
				bRet = adi_a2b_ConfigNodeOptimizAdvancedMode(plugin, dscNodeAddr);
			}
		}
		else
		{
			/* send notification to application */
			bContinueDisc = sendNodeDscvryNotification( plugin, plugin->discovery.discoveryCompleteCode, dscNodeIdx);
			if(bContinueDisc == A2B_TRUE)
			{
				bRet = a2b_dscvryPreSlaveInit( plugin );
			}
			else
			{
				a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_CANCELLED );
			}
		}
#ifdef A2B_FEATURE_COMM_CH
	}
#endif	/* A2B_FEATURE_COMM_CH */

    /* Now we wait for INTTYPE.DSCDONE on success */
    
    return bRet;

} /* a2b_dscvryNodeDiscovered */
#ifdef A2B_FEATURE_PARTIAL_DISC
/*!****************************************************************************
*
*  \b              a2b_dscvryPartial
*
* Attempt partial discovery upon line faults
*
*  \param          [in]    plugin   plugin specific data
*
*  \pre            None
*
*  \post           None
*
*  \return         A2B_EXEC_COMPLETE == Execution is now complete
*                  A2B_EXEC_SCHEDULE == Execution is unfinished - schedule again
*                  A2B_EXEC_SUSPEND  == Execution is unfinished - suspend
*                                       scheduling until a later event
*
******************************************************************************/
a2b_Int32
a2b_dscvryPartial
(
	a2b_Plugin* plugin
)
{
	a2b_UInt8 wBuf[2u];
	a2b_HResult status;
	a2b_Int16 nodeAddr;
	a2b_UInt8 nUserSWCTL;

	plugin->pwrDiag.state = A2B_PWR_DIAG_STATE_INIT;
	plugin->discovery.inDiscovery = A2B_TRUE;
	plugin->bDisablePwrDiag = A2B_TRUE;
	plugin->discovery.inPartialDiscovery = A2B_TRUE;
	nodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes - (a2b_Int16)1;

	/* Clear discovery error code of previous attempts */
	plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_OK;

	/* Disable power on the B side of the node */
	nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[nodeAddr].ctrlRegs.swctl);
	wBuf[0] = A2B_REG_SWCTL;
	wBuf[1] = ((nUserSWCTL & A2B_REG_USER_SWCTL) | (a2b_UInt8)(plugin->nodeSig.highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV));
	wBuf[1] &= (~(A2B_BITM_SWCTL_ENSW));
	status = a2b_regWrite(plugin->ctx, nodeAddr, 2u, wBuf);
	
	/* Wait for re-discovery wait time */	
	if (a2b_isAd243xChip(plugin->slaveNodeSig[nodeAddr].siliconInfo.vendorId, plugin->slaveNodeSig[nodeAddr].siliconInfo.productId)||
		a2b_isAd2430_8_Chip(plugin->slaveNodeSig[nodeAddr].siliconInfo.vendorId, plugin->slaveNodeSig[nodeAddr].siliconInfo.productId))
	{
		a2b_ActiveDelay(plugin->ctx, plugin->bdd->policy.nRediscWaitTime); /* For AD243x */
	}
	else
	{
		a2b_ActiveDelay(plugin->ctx, A2B_SW_RESET_DELAY); /* For AD242x */
	}
	return (a2b_dscvryPreSlaveInit(plugin));
}
#endif

/*!****************************************************************************
*
*  \b              a2b_dscvryCustomAuthFrmMem
*
*  Called after a node has been discovered.  This can be used to perform
*  custom node authentication by reading from memory.
*
*  \param          [in]    plugin   plugin specific data
*
*  \param          [in]    nodeSig  node signature data
*
*  \pre            Called on DSCDONE
*
*  \post           None
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool a2b_dscvryCustomAuthFrmMem(a2b_Plugin* plugin, a2b_NodeSignature nodeSig)
{
    a2b_Bool 	bNetConfigFlag = A2B_FALSE, nFlgSupIdMatch = A2B_FALSE;
    a2b_UInt8 	wBuf[4u];
    a2b_HResult status = A2B_RESULT_SUCCESS;
    a2b_UInt16 	nWrite, nRead;
	a2b_UInt8 	nIdx, rBufCustomNodeId[50u];
    a2b_Int16 	dscNodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes;
    a2b_Int16 	dscNodeIdx = dscNodeAddr+1;
	a2b_Bool 	bContinueDisc;
	const bdd_Node					*bddNodeObj;
	a2b_CustomAuth					*pSlvNodeCustomAuth;
	const bdd_CustomNodeIdSettings 	*pCustomNodeIdSettings;
	A2B_UNUSED(nodeSig.nodeAddr);

	bddNodeObj 				= &plugin->bdd->nodes[dscNodeIdx];
	pSlvNodeCustomAuth 		= &plugin->customAuth[dscNodeAddr]; /* Using dscNodeAddr as it only has slave node count */
	pCustomNodeIdSettings 	= &bddNodeObj->nodeDescr.oCustomNodeIdSettings;

	switch(pCustomNodeIdSettings->nReadMemAddrWidth)
	{
	case 1U:
		wBuf[0U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr);
		break;
	case 2U:
		wBuf[0U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr >> 8U);
		wBuf[1U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr  & 0xFFU);
		break;
	case 3U:
		wBuf[0U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr >> 16U);
		wBuf[1U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr >> 8U);
		wBuf[2U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr  & 0xFFU);
		break;
	case 4U:
		wBuf[0U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr >> 24U);
		wBuf[1U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr >> 16U);
		wBuf[2U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr >> 8U);
		wBuf[3U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr  & 0xFFU);
		break;
	default:
		wBuf[0U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr >> 8u);
		wBuf[1U]  =  (a2b_UInt8)(pCustomNodeIdSettings->nReadMemAddr & 0xFFu);
		break;
	}

	nWrite = (a2b_UInt16)pCustomNodeIdSettings->nReadMemAddrWidth;
	nRead  = (a2b_UInt16)pCustomNodeIdSettings->nNodeIdLength;

	/* Read from peripheral Memory */
	plugin->discovery.CustomNodeAuthID[0] = A2B_CUSTOM_AUTH_TYPE_MEM;

	do
	{
		/* Read from peripheral memory */
		status = a2b_periphWriteRead(plugin->ctx, dscNodeAddr, (a2b_UInt16)pCustomNodeIdSettings->nDeviceAddr, nWrite, wBuf, nRead, rBufCustomNodeId);

		if (A2B_SUCCEEDED(status))
		{
			/* Length of ID */
			plugin->discovery.CustomNodeAuthID[1] = nRead+2u; /* payload + header (2 bytes) */
			/* Copy the read ID to pass back to application*/
			(void)a2b_memcpy(&plugin->discovery.CustomNodeAuthID[2], rBufCustomNodeId, nRead);

			for (nIdx = 0u; nIdx < nRead; nIdx++)
			{
				if (rBufCustomNodeId[nIdx] != (a2b_UInt8)(pCustomNodeIdSettings->nNodeId[nIdx]))
				{
					nFlgSupIdMatch = A2B_FALSE;
					break;
				}	
				else
				{
					nFlgSupIdMatch = A2B_TRUE;
				}
			}
		}
		else
		{
			/* If read from memory is failed set the nFlgSupIdMatch as false */
			nFlgSupIdMatch = A2B_FALSE;
			/* Length of ID */
			plugin->discovery.CustomNodeAuthID[1] = 2u; /* Only header */
		}

		/* Decrement the retry count */
		pSlvNodeCustomAuth->nRetryCnt--;

		if(nFlgSupIdMatch == A2B_FALSE)
		{
			/* Wait for 1 msec */
			a2b_ActiveDelay(plugin->ctx, 1u);
		}

	} while ((nFlgSupIdMatch == A2B_FALSE) && (pSlvNodeCustomAuth->nRetryCnt > 0));

	if (A2B_SUCCEEDED(status) && (nFlgSupIdMatch == A2B_FALSE))
	{
		/* Set the discovery complete code */
		plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH;

		/* send notification to application */
		bContinueDisc = sendNodeDscvryNotification( plugin, plugin->discovery.discoveryCompleteCode, dscNodeIdx);
		if(bContinueDisc == A2B_TRUE)
		{
			/* Reset the custom node id flag */
			nFlgSupIdMatch = A2B_TRUE;

			/* Reset the discovery complete code */
			plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_OK;
		}
	}

	if (nFlgSupIdMatch == A2B_FALSE)
	{
		if(A2B_FAILED(status))
		{
			A2B_DSCVRY_ERROR2(plugin->ctx, "a2b_dscvryCustomAuthFrmMem", "Peripheral address %hd of Node %hd is Failed to read", &(pCustomNodeIdSettings->nDeviceAddr), &dscNodeAddr);
		}
		else
		{
			A2B_DSCVRY_ERROR1(plugin->ctx, "a2b_dscvryCustomAuthFrmMem", "Node %hd: Custom Node Id Authentication Failed via read from memory", &dscNodeAddr);
		}

		bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);
		if ((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
		{
			if(A2B_FAILED(status))
			{
				plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_I2C_ERROR;
			}
			else
			{
				plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH;
			}
			a2b_dscvryNetComplete(plugin);
		}
		else
		{
			if(A2B_FAILED(status))
			{
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_I2C_ERROR);
			}
			else
			{
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH);
			}

		}
		A2B_DSCVRY_SEQEND(plugin->ctx);
	}

	return (nFlgSupIdMatch);
}

/*!****************************************************************************
*
*  \b              a2b_dscvryCustomAuthFrmGpio
*
*  Called after a node has been discovered.  This can be used to perform
*  custom node authentication by reading from GPIO.
*
*  \param          [in]    plugin   plugin specific data
*
*  \param          [in]    nodeSig  node signature data
*
*  \pre            Called on DSCDONE
*
*  \post           None
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool a2b_dscvryCustomAuthFrmGpio(a2b_Plugin* plugin, a2b_NodeSignature nodeSig)
{
	a2b_Bool 	bNetConfigFlag = A2B_FALSE, nFlgGpioMatch;
	a2b_UInt8 	wBuf[4u];
	a2b_HResult status = A2B_RESULT_SUCCESS;
	a2b_UInt8 	nIdx, rBufGpio[1], nGpioVal;
    a2b_Int16 	dscNodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes;
    a2b_Int16 	dscNodeIdx = dscNodeAddr+1;
    a2b_Bool 	bContinueDisc;
	const bdd_Node					*bddNodeObj;
	a2b_CustomAuth					*pSlvNodeCustomAuth;
	const bdd_CustomNodeIdSettings 	*pCustomNodeIdSettings;
	A2B_UNUSED(nodeSig.nodeAddr);

	bddNodeObj 				= &plugin->bdd->nodes[dscNodeIdx];
	pSlvNodeCustomAuth 		= &plugin->customAuth[dscNodeAddr]; /* Using dscNodeAddr as it only has slave node count */
	pCustomNodeIdSettings 	= &bddNodeObj->nodeDescr.oCustomNodeIdSettings;

	/* GPIO Input Enable*/
	wBuf[0] = A2B_REG_GPIOIEN;
	wBuf[1] = ~(A2B_REG_GPIOIEN_RESET) & 0xFF;
	status = a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);
	wBuf[0u] = A2B_REG_GPIOIN;

	/* Read from GPIO */
	plugin->discovery.CustomNodeAuthID[0] = A2B_CUSTOM_AUTH_TYPE_GPIO;

	do
	{
		/* Read from GPIO */
		status = a2b_regWriteRead(plugin->ctx, dscNodeAddr, 1u, wBuf, 1u, rBufGpio);
		if (A2B_SUCCEEDED(status))
		{
			/* Size is 1 byte */
			plugin->discovery.CustomNodeAuthID[1] = 3u; /* Header + Payload */
			/* Payload */
			plugin->discovery.CustomNodeAuthID[2] = rBufGpio[0];

			for (nIdx = 0u; nIdx < 8u; nIdx++)
			{
				if (pCustomNodeIdSettings->aGpio[nIdx] != A2B_IGNORE)
				{
					nGpioVal = ((rBufGpio[0] & (1u << nIdx)) >> nIdx);
					if (nGpioVal != pCustomNodeIdSettings->aGpio[nIdx])
					{
						nFlgGpioMatch = A2B_FALSE;
						break;
					}
					else
					{
						nFlgGpioMatch = A2B_TRUE;
					}
				}
				else
				{
					nFlgGpioMatch = A2B_TRUE;
				}
			}
		}
		else
		{
			/* If read from memory is failed set the nFlgGpioMatch as false */
			nFlgGpioMatch = A2B_FALSE;
			/* Size is 0 byte */
			plugin->discovery.CustomNodeAuthID[1] = 2u; /* Only Header */
		}

		/* Decrement the retry count */
		pSlvNodeCustomAuth->nRetryCnt--;

		if(nFlgGpioMatch == A2B_FALSE)
		{
			/* Wait for 1 msec */
			a2b_ActiveDelay(plugin->ctx, 1u);
		}

	} while ((nFlgGpioMatch == A2B_FALSE) && (pSlvNodeCustomAuth->nRetryCnt > 0));

	if (nFlgGpioMatch == A2B_FALSE)
	{
		/* Set the discovery complete code */
		plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH;

		/* send notification to application */
		bContinueDisc = sendNodeDscvryNotification( plugin, plugin->discovery.discoveryCompleteCode, dscNodeIdx);
		if(bContinueDisc == A2B_TRUE)
		{
			/* Reset the custom node id flag */
			nFlgGpioMatch = A2B_TRUE;

			/* Reset the discovery complete code */
			plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_OK;
		}
	}

	if (nFlgGpioMatch == A2B_FALSE)
	{
		A2B_DSCVRY_ERROR1(plugin->ctx, "a2b_dscvryCustomAuthFrmGpio", "Node %hd Custom Node Id Authentication via GPIO ", &dscNodeAddr);

		bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);
		if ((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
		{
			plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH;
			a2b_dscvryNetComplete(plugin);
		}
		else
		{
			a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH);
		}
		A2B_DSCVRY_SEQEND(plugin->ctx);
	}

	return(nFlgGpioMatch);
}

/*!****************************************************************************
*
*  \b              sendNodeDscvryNotification
*
*  Trigger's a node level discovery notification if an application is registered
*
*  \param          [in]    plugin   	plugin specific data
*
*  \param          [in]    discCode  	Discovery success/failure indication
*
*  \param          [in]    dscNodeIdx	The A2B node address of the node that was just discovered.
*
*  \pre            Called on DSCDONE
*
*  \post           None
*
*  \return         FALSE = end discovery
*                  TRUE = continue discovery
*
******************************************************************************/
static a2b_Bool sendNodeDscvryNotification(a2b_Plugin *plugin, a2b_UInt32 discCode, a2b_Int16 dscNodeIdx)
{
	a2b_HResult result = A2B_RESULT_SUCCESS;
	struct a2b_Msg* notification;
	struct a2b_Nodedscvry* dscvrdNode;
	a2b_Bool bContinueDisc;

	/* Initializing the bContinueDisc with true if the discovery complete code is zero so as to continue with discovery */
	/* Initializing the bContinueDisc with false if the discovery complete code is non zero so as to end the discovery */
	bContinueDisc = (discCode == A2B_EC_OK) ? A2B_TRUE : A2B_FALSE ;

	/* Allocate a notification message */
	notification = a2b_msgAlloc(plugin->ctx, A2B_MSG_NOTIFY, A2B_MSGNOTIFY_NODE_DISCOVERY);

	if ( A2B_NULL == notification )
	{
		A2B_TRACE0((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "sendNodeDscvryNotification: " "failed to allocate notification"));
	}
	else
	{

			dscvrdNode = (a2b_Nodedscvry*)a2b_msgGetPayload(notification);
			dscvrdNode->bddNetObj 				= (const void*)(plugin->bdd);
			dscvrdNode->nodeAddr 				= dscNodeIdx;
			dscvrdNode->discoveryCompleteCode 	= discCode;
			dscvrdNode->bContinueDisc 			= (discCode == A2B_EC_OK) ? A2B_TRUE : A2B_FALSE ;

			/* Make best effort delivery of notification */
			result = a2b_msgRtrNotify(notification);
			if ( A2B_FAILED(result) )
			{
				A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "sendNodeDscvryNotification: " "failed to emit node discovery status notification: 0x%lX", &result));
			}

			/* We no longer need this message */
			(void)a2b_msgUnref(notification);

			/* Get back the user input */
			bContinueDisc = dscvrdNode->bContinueDisc;
	}

	return (bContinueDisc);
}

#ifdef A2B_FEATURE_COMM_CH
/*!****************************************************************************
*
*  \b              adi_a2b_MstrPluginCommChStatusCb
*
*  Master plugin communication channel status callback function
*
*  \param          [in]    pPlugin         Pointer to master plugin specific data
*  \param          [in]    pMsg            Pointer to message
*  \param		   [in]	   eEventType      Enumeration of the event type
*  \param		   [in]	   nNodeAddr       Node address from which the msg is received
*
*  \pre            None
*
*  \post           None
*
*  \return          Returns a Boolean flag of whether the node was configured
*  					successfully.
******************************************************************************/
a2b_Bool adi_a2b_MstrPluginCommChStatusCb(void* pPlugin, a2b_CommChMsg *pMsg, A2B_COMMCH_EVENT eEventType, a2b_Int8 nNodeAddr)
{
	a2b_UInt32			nIdx;
	a2b_NodeSignature   nodeSig;
	a2b_Bool 			bRet = A2B_FALSE, nFlgSupIdMatch = A2B_TRUE, bNetConfigFlag = A2B_FALSE;
	a2b_Plugin* 		plugin 		= (a2b_Plugin*)pPlugin;
    a2b_Int16 			dscNodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes;
    a2b_Int16 			dscNodeIdx 	= dscNodeAddr+1;
    const bdd_Node		*bddNodeObj = &plugin->bdd->nodes[dscNodeIdx];
    a2b_Bool 			bContinueDisc;
    a2b_CustomAuth		*pSlvNodeCustomAuth;
    A2B_UNUSED(nNodeAddr);

    A2B_INIT_SIGNATURE( &nodeSig, dscNodeAddr );

	switch(eEventType)
	{
	case A2B_COMMCH_EVENT_RX_MSG:
		if(pMsg->nMsgId == A2B_COMMCH_MSG_RSP_SLV_NODE_SIGNATURE)
		{
			/* Stop authentication timeout timer */
			a2b_timerStop( plugin->timer );

			/* Read from Mailbox */
			plugin->discovery.CustomNodeAuthID[0] = A2B_CUSTOM_AUTH_TYPE_MAILBOX;
			/* ID Length */
			plugin->discovery.CustomNodeAuthID[1] = pMsg->nMsgLenInBytes + 2u; /* Header + Payload */
			(void)a2b_memcpy(&plugin->discovery.CustomNodeAuthID[2], pMsg->pMsgPayload, pMsg->nMsgLenInBytes);

			if (pMsg->nMsgLenInBytes == bddNodeObj->nodeDescr.oCustomNodeIdSettings.nNodeIdLength)
			{
				for (nIdx = 0u; nIdx < pMsg->nMsgLenInBytes; nIdx++)
				{
					if (pMsg->pMsgPayload[nIdx] != bddNodeObj->nodeDescr.oCustomNodeIdSettings.nNodeId[nIdx])
					{
						nFlgSupIdMatch = A2B_FALSE;
						break;
					}
				}
			}
			else
			{
				/* ID Length */
				plugin->discovery.CustomNodeAuthID[1] = 2u;
				nFlgSupIdMatch = A2B_FALSE;
			}

			if (nFlgSupIdMatch == A2B_FALSE)
			{
				/* Set the discovery complete code */
				plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH;

				/* send notification to application */
				bContinueDisc = sendNodeDscvryNotification( plugin, plugin->discovery.discoveryCompleteCode, dscNodeIdx);
				if(bContinueDisc == A2B_TRUE)
				{
					/* Reset the custom node id flag */
					nFlgSupIdMatch = A2B_TRUE;

					/* Reset the discovery complete code */
					plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_OK;
				}
			}

			if(nFlgSupIdMatch == A2B_FALSE)	/* End discovery */
			{
				/* Copy the signature information to the plugin */
				plugin->slaveNodeSig[dscNodeAddr] = nodeSig;

	            A2B_DSCVRY_ERROR1( plugin->ctx, "nodeDiscovered", "Node %hd: Custom Node Id Authentication Failed while reading via communication channel", &dscNodeAddr );

	            bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);

	            if((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
	            {
	            	plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH;
	            	a2b_dscvryNetComplete(plugin);
	            }
	            else
	            {
					a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH );
	            }

	            A2B_DSCVRY_SEQEND( plugin->ctx );
	            return (A2B_FALSE);
			}
			else
			{
				/* send notification to application */
				bContinueDisc = sendNodeDscvryNotification( plugin, plugin->discovery.discoveryCompleteCode, dscNodeIdx);
				if(bContinueDisc == A2B_TRUE)
				{
					bRet = a2b_dscvryPostAuthViaCommCh(plugin);
				}
				else
				{
					a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_AUTH );
				}
			}
		}
		break;
	case A2B_COMMCH_EVENT_TX_DONE:

		/* If request message is successfully transmitted start the timeout timer
		 * as per timeout specified in bdd
		 * */
		if(pMsg->nMsgId == A2B_COMMCH_MSG_REQ_SLV_NODE_SIGNATURE)
		{
			bRet = a2b_dscvryStartCommChAuthTimer(plugin, bddNodeObj->nodeDescr.oCustomNodeIdSettings.nTimeOut);
		}
		break;
	case A2B_COMMCH_EVENT_TX_TIMEOUT:
		/* If request message transmission has timed out
		 * then end discovery
		 * */
		pSlvNodeCustomAuth = &plugin->customAuth[dscNodeAddr]; /* Using dscNodeAddr as it only has slave node count */

		if(pMsg->nMsgId == A2B_COMMCH_MSG_REQ_SLV_NODE_SIGNATURE)
		{
			if(pSlvNodeCustomAuth->nRetryCnt > 0)
			{
				/* Decrement the retry count */
				pSlvNodeCustomAuth->nRetryCnt--;

				/* Re-issue the custom node auth message via mailbox  */
				bRet = a2b_dscvryCustomAuthFrmCommCh(plugin, plugin->slaveNodeSig[dscNodeAddr]);
			}
			else
			{
				/* Copy the signature information to the plugin */
				plugin->slaveNodeSig[dscNodeAddr] = nodeSig;

				A2B_DSCVRY_ERROR1( plugin->ctx, "nodeDiscovered", "Node %hd: Custom Node Id Authentication Failed : Transmission via communication channel timed out ", &dscNodeAddr );

				bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);

				if((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
				{
					plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_TIMEOUT;
					a2b_dscvryNetComplete(plugin);
				}
				else
				{
					a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_TIMEOUT );
				}

				A2B_DSCVRY_SEQEND( plugin->ctx );
				return (A2B_FALSE);
			}
		}
		break;
	default :
		bRet = A2B_FALSE;
		break;
	}

	return bRet;
}

/*!****************************************************************************
*
*  \b              a2b_dscvryPostAuthViaCommCh
*
*  Function used to continue the discovery flow after the current discovered node
*  has been authorized via communication channel.This requires a separate flow
*  since authorization by communication channel requires a non-blocking communi-
*  cation between master and slave node spanning multiple ticks.
*
*  \param          [in]    plugin         Pointer to master plugin
*  \pre            None
*
*  \post           None
*
*  \return          Returns a Boolean flag of whether the node was configured
*  					successfully.
******************************************************************************/
static a2b_Bool a2b_dscvryPostAuthViaCommCh(a2b_Plugin* plugin)
{
    a2b_Bool 	bRet = A2B_TRUE;
    a2b_Int16 	dscNodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes;
    a2b_Int16 	dscNodeIdx = dscNodeAddr+1;
    bdd_DiscoveryMode 	eDiscMode;
    a2b_NodeSignature   nodeSig;
    const bdd_Node      *bddNodeObj = &plugin->bdd->nodes[dscNodeIdx];
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
    struct a2b_StackContext* ctx = plugin->ctx;
    a2b_UInt8 	wBuf[4u];
    a2b_UInt8 	rBuf[8u];
    a2b_HResult status = A2B_RESULT_SUCCESS;
#endif

    eDiscMode = a2b_ovrGetDiscMode(plugin);
	/* Used only during simple discovery with sync periph processing */
	plugin->discovery.simpleNodeCount++;
	plugin->discovery.dscNumNodes++;

    /* Copy the signature information from the plugin */
    nodeSig = plugin->slaveNodeSig[dscNodeAddr];

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
	if ((( nodeSig.hasI2cCapability ) && ( !bddNodeObj->ignEeprom ) &&
		 ( ( plugin->overrides[0u] & A2B_MPLUGIN_IGN_EEPROM ) == 0u )) ||
		( plugin->periphCfg.nodeCfg[dscNodeAddr] ))
	{
		A2B_DSCVRY_SEQGROUP0( ctx,
							  "Look for EEPROM at 0x50" );

		/* Read the EEPROM header             */
		/* [Two byte internal EEPROM address] */
		wBuf[0] = 0u;
		wBuf[1] = 0u;
		status = a2b_periphCfgWriteRead( plugin,
										 dscNodeAddr,
										 2u,  wBuf,
										 8u,  rBuf);

		if (( A2B_SUCCEEDED(status) ) &&
			( rBuf[0] == A2B_MARKER_EEPROM_CONFIG ))
		{
			a2b_UInt8 crc8 = a2b_crc8(rBuf, 0u, 7u);

			if ( crc8 == rBuf[7] )
			{
				/* EEPROM VID/PID/VER */
				nodeSig.hasEepromInfo        = A2B_TRUE;
				nodeSig.eepromInfo.vendorId  = rBuf[1];
				nodeSig.eepromInfo.productId = rBuf[2];
				nodeSig.eepromInfo.version   = rBuf[3];

				A2B_TRACE5( (ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
							"%s nodeDiscovered(): EEPROM node/vid/pid/ver: "
							"%02hX/%02bX/%02bX/%02bX",
							A2B_MPLUGIN_PLUGIN_NAME, &dscNodeAddr,
							&nodeSig.eepromInfo.vendorId,
							&nodeSig.eepromInfo.productId,
							&nodeSig.eepromInfo.version ));

				A2B_SEQ_GENNOTE3( plugin->ctx,
								  A2B_SEQ_CHART_LEVEL_DISCOVERY,
								  "EEPROM vid/pid/ver: %02bX/%02bX/%02bX",
								  &nodeSig.eepromInfo.vendorId,
								  &nodeSig.eepromInfo.productId,
								  &nodeSig.eepromInfo.version );

				/* When this override is set we will parse the version
				 * (which we did) but we will not indicate that there
				 * was an EEPRROM so we will avoid peripheral processing
				 */
				if (( plugin->overrides[0] & A2B_MPLUGIN_EEPROM_VER_ONLY ) == 0u )
				{
					/* See if we have an override for this specific node */
					if (( plugin->overrides[1] & (a2b_UInt32)((a2b_UInt32)1u << dscNodeAddr)) == (a2b_UInt32)0u )
					{
						plugin->discovery.hasEeprom |= (a2b_UInt32)((a2b_UInt32)1u << dscNodeAddr);
					}
					else
					{
						A2B_DSCVRYNOTE_DEBUG1( plugin->ctx,
								   "nodeDiscovered",
								   "Override Set, Ignoring node %hd EEPROM",
								   &dscNodeAddr );
					}
				}

				/* Optionally validate the node descriptor info */
				if (( bddNodeObj->verifyNodeDescr ) &&
					(( bddNodeObj->nodeDescr.vendor !=
										   nodeSig.eepromInfo.vendorId ) ||
					 ( bddNodeObj->nodeDescr.product !=
										   nodeSig.eepromInfo.productId ) ||
					 ( bddNodeObj->nodeDescr.version !=
										   nodeSig.eepromInfo.version )) )
				{
					/* clear the bit */
					plugin->discovery.hasEeprom ^= (a2b_UInt32)((a2b_UInt32)1u << dscNodeAddr);

					A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered",
									  "Node %hd EEPROM failed verification",
									  &dscNodeAddr );

					a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_PERMISSION );
					A2B_DSCVRY_SEQEND( plugin->ctx );
					return A2B_FALSE;
				}
			}
			else
			{
				A2B_DSCVRY_ERROR1( ctx, "nodeDiscovered",
								   "Node %hd EEPROM header CRC incorrect",
								   &dscNodeAddr );
			}
		}
		else
		{
			A2B_DSCVRYNOTE_DEBUG1( ctx, "nodeDiscovered",
								   "Node %hd EEPROM not found",
								   &dscNodeAddr );
		}
		A2B_DSCVRY_SEQEND( plugin->ctx );
	}
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

	/* BDD VID/PID/VER */
	nodeSig.hasBddInfo        = A2B_TRUE;
	nodeSig.bddInfo.vendorId  = (a2b_UInt8)bddNodeObj->nodeDescr.vendor;
	nodeSig.bddInfo.productId = (a2b_UInt8)bddNodeObj->nodeDescr.product;
	nodeSig.bddInfo.version   = (a2b_UInt8)bddNodeObj->nodeDescr.version;

	A2B_TRACE5( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_INFO),
				"%s nodeDiscovered(): BDD node/vid/pid/ver: "
				"%02hX/%02bX/%02bX/%02bX",
				A2B_MPLUGIN_PLUGIN_NAME, &dscNodeAddr,
				&nodeSig.bddInfo.vendorId,
				&nodeSig.bddInfo.productId,
				&nodeSig.bddInfo.version ));

	A2B_SEQ_GENNOTE3( plugin->ctx,
					  A2B_SEQ_CHART_LEVEL_DISCOVERY,
					  "BDD vid/pid/ver: %02bX/%02bX/%02bX",
					  &nodeSig.bddInfo.vendorId,
					  &nodeSig.bddInfo.productId,
					  &nodeSig.bddInfo.version );

	/* Copy the signature information to the plugin */
	plugin->slaveNodeSig[dscNodeAddr].bddInfo = nodeSig.bddInfo;

#ifndef FIND_NODE_HANDLER_AFTER_NODE_INIT
	/* We'll attempt to find and open a plugin that can manage the
	 * slave node *before* completing the full initialization of the
	 * node.
	 */
	a2b_dscvryFindNodeHandler(plugin, dscNodeAddr);
#endif

	if ( bdd_DISCOVERY_MODE_MODIFIED == eDiscMode )
	{
		(void)adi_a2b_ConfigureNodePeri(plugin, dscNodeAddr);

	} /* end Modified processing */
	else if((bdd_DISCOVERY_MODE_OPTIMIZED == eDiscMode) ||
			(bdd_DISCOVERY_MODE_ADVANCED == eDiscMode))
	{
		if(plugin->discovery.pendingPluginInit == (a2b_UInt8)0)
		{
			bRet = adi_a2b_ConfigNodeOptimizAdvancedMode(plugin, dscNodeAddr);
		}
	}
	else
	{
		bRet = a2b_dscvryPreSlaveInit( plugin );
	}

	return bRet;
}

/*!****************************************************************************
*
*  \b              a2b_dscvryStartCommChAuthTimer
*
*  Generate/Start the authorization timer for timeout on reception of slave node
*  id response upon initiation of a request from master plugin.
*  NOTE: This timer instance is shared for discovery timeout as well
*  since authorization happens after a node is discovered and since timer started
*  for discovery timeout is stopped at this moment, this sharing is possible
*  \param          [in]    plugin
*  \param          [in]    delay
*
*  \pre            None
*
*  \post           None
*
*  \return         [add here]
*
******************************************************************************/
static a2b_Bool
a2b_dscvryStartCommChAuthTimer
    (
    a2b_Plugin*     plugin,
	a2b_UInt16 delay
    )
{
    a2b_TimerFunc timerFunc = &a2b_onCommChAuthTimeout;

    /* Stop the previously running timer */
    a2b_timerStop( plugin->timer );

    /* Single shot timer */
    a2b_timerSet( plugin->timer, (a2b_UInt32)delay, 0u );
    a2b_timerSetHandler(plugin->timer, timerFunc);
    a2b_timerSetData(plugin->timer, plugin);
    a2b_timerStart( plugin->timer );

    return A2B_TRUE;

} /* a2b_dscvryStartCommChAuthTimer */

/*!****************************************************************************
*
*  \b              a2b_onCommChAuthTimeout
*
*  Handle the communication channel authorization timeout.
*
*  \param          [in]    timer
*  \param          [in]    userData
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void
a2b_onCommChAuthTimeout
    (
    struct a2b_Timer *timer,
    a2b_Handle userData
    )
{
    a2b_Plugin* plugin = (a2b_Plugin*)userData;
    a2b_NodeSignature   nodeSig;
	a2b_Int16 			dscNodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes;
	a2b_Bool bNetConfigFlag = A2B_FALSE;

	A2B_UNUSED(timer);
	A2B_INIT_SIGNATURE( &nodeSig, dscNodeAddr );

	/* Copy the signature information to the plugin */
	plugin->slaveNodeSig[dscNodeAddr] = nodeSig;

	/* End the discovery */
	A2B_DSCVRY_ERROR1( plugin->ctx, "nodeDiscovered", "Node %hd: Custom Node Id Authentication Failed : Timed out on response from slave via communication channel ", &dscNodeAddr );

	bNetConfigFlag = a2b_SimpleModeChkNodeConfig(plugin);

	if((bdd_DISCOVERY_MODE_SIMPLE == a2b_ovrGetDiscMode(plugin)) && (bNetConfigFlag))
	{
		plugin->discovery.discoveryCompleteCode = (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_TIMEOUT;
		a2b_dscvryNetComplete(plugin);
	}
	else
	{
		a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_CUSTOM_NODE_ID_TIMEOUT );
	}

	A2B_DSCVRY_SEQEND( plugin->ctx );
}

/*!****************************************************************************
*
*  \b              a2b_dscvryCustomAuthFrmCommCh
*
*  Called after a node has been discovered.  This can be used to perform
*  custom node authentication by reading from mailbox.
*
*  \param          [in]    plugin   plugin specific data
*
*  \param          [in]    nodeSig  node signature data
*
*  \pre            Called on DSCDONE
*
*  \post           None
*
*  \return         FALSE=error
*                  TRUE=success
*
******************************************************************************/
static a2b_Bool a2b_dscvryCustomAuthFrmCommCh(a2b_Plugin* plugin, a2b_NodeSignature nodeSig)
{
	a2b_Bool 		bRet;
	a2b_CommChMsg	oCommChMsgGetCustNodeId;
	A2B_COMMCH_RET	bCommChRet;
	a2b_UInt16		nCommChInstNo;
    a2b_Int16 		dscNodeAddr = (a2b_Int16)plugin->discovery.dscNumNodes;
    a2b_Int16 		dscNodeIdx = dscNodeAddr+1;

	/* Program the mailbox registers so that we can exchange the authentication request and response messages over mailbox */
	bRet = a2b_dscvryNodeMailboxInit(plugin, dscNodeIdx);
	if(bRet == A2B_FALSE)
	{
		A2B_DSCVRY_ERROR1(plugin->ctx, "nodeDiscovered", "Node %hd authorization via Communication channel ," "Programming of Mailbox registers failed ", &dscNodeAddr );
	}

	/* Copy the signature information to the plugin */
	plugin->slaveNodeSig[dscNodeAddr] = nodeSig;

	/* Prepare and send authentication message */
	oCommChMsgGetCustNodeId.nMsgId			= A2B_COMMCH_MSG_REQ_SLV_NODE_SIGNATURE;
	oCommChMsgGetCustNodeId.nMsgLenInBytes 	= 0u;
	oCommChMsgGetCustNodeId.pMsgPayload		= NULL;
	bRet = a2b_GetCommChInstIdForSlvNode(plugin, dscNodeAddr, &nCommChInstNo);
	if(bRet == A2B_TRUE)
	{
		bCommChRet = adi_a2b_CommChMstrTxMsg(plugin->commCh.commChHnd[nCommChInstNo], &oCommChMsgGetCustNodeId , (a2b_Int8)dscNodeAddr);
		if(bCommChRet == A2B_COMMCH_FAILED)
		{
			A2B_DSCVRY_ERROR1(plugin->ctx, "nodeDiscovered", "Node %hd authorization via Communication channel ," "Transmission of request Node id message failed ", &dscNodeAddr );
		}
	}
	else
	{
		A2B_DSCVRY_ERROR1(plugin->ctx, "nodeDiscovered", "Node %hd authorization via Communication channel ," "Communication Channel Instance for slave node address not available ", &dscNodeAddr );
	}

	return (bRet);
}
#endif	/* A2B_FEATURE_COMM_CH */

/*!****************************************************************************
*
*  \b              adi_a2b_ConfigureNodePeri
*
*  Configure node and peripheral in modified and optimized modes of discovery
*
*  \param          [in]    plugin           plugin specific data
*  \param		   [in]	   dscNodeAddr      The address of the node that has
*      			   							to be configured
*
*  \pre            None
*
*  \post           None
*
*  \return          Returns a Boolean flag of whether the node was configured
*  					successfully.
******************************************************************************/
static a2b_Bool
adi_a2b_ConfigureNodePeri(a2b_Plugin* plugin, a2b_Int16 dscNodeAddr)
{
    a2b_UInt32 	errCode;
    a2b_Bool	bRes;
    a2b_Int32 	retCode = a2b_dscvryNodeComplete( plugin, dscNodeAddr,
                                                A2B_TRUE, &errCode );
    switch (retCode)
    {
        case A2B_EXEC_COMPLETE:
            /* No EEPROM Cfg (or done), so init the plugin now */
#ifdef FIND_NODE_HANDLER_AFTER_NODE_INIT
            a2b_dscvryFindNodeHandler(plugin, (a2b_UInt16)dscNodeAddr);
#endif
            if ( A2B_HAS_PLUGIN( plugin, dscNodeAddr ) )
            {
                if ((a2b_UInt32)A2B_EC_OK != a2b_dscvryInitPlugin( plugin, dscNodeAddr,
                                    &a2b_dscvryInitPluginComplete_NoEeprom ))
                {
                	bRes = A2B_FALSE;
                	break;
                }
                bRes = A2B_TRUE;
                break;
            }
            bRes = A2B_FALSE;
            break;

        case A2B_EXEC_COMPLETE_FAIL:
            a2b_dscvryEnd( plugin, errCode );
            bRes = A2B_FALSE;
            break;

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
        case A2B_EXEC_SCHEDULE:
        case A2B_EXEC_SUSPEND:
            if ( a2b_periphCfgUsingSync() )
            {
                /* Node peripheral processing has not completed,
                 * processing, will resume later--would be a delay
                 * cfg block or async processing.
                 */
            	bRes = A2B_TRUE;
            	break;
            }
            bRes = A2B_FALSE;
            break;
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

        default:
        	bRes = A2B_FALSE;
        	break;
    }

    return (bRes);

} /* adi_a2b_ConfigureNodePeri */

/*!****************************************************************************
*
*  \b              adi_a2b_ConfigNodeOptimizAdvancedMode
*
*  Configure node and peripheral in optimized and advanced modes of discovery
*
*  \param          [in]    plugin           plugin specific data
*  \param          [in]    dscNodeAddr      Destination Node address
*
*  \pre            None
*
*  \post           None
*
*  \return         Returns the status of slave intialization for next node
*  				   discovery
*  				   FALSE=error
*                  TRUE=success
******************************************************************************/
static a2b_Bool
adi_a2b_ConfigNodeOptimizAdvancedMode(a2b_Plugin* plugin, a2b_Int16 dscNodeAddr)
{
    a2b_Bool bRet;

	bRet = a2b_dscvryPreSlaveInit( plugin );
	/* Configure Node registers and start plugin initialization */
	(void)adi_a2b_ConfigureNodePeri(plugin, dscNodeAddr);

	if(bdd_DISCOVERY_MODE_ADVANCED == a2b_ovrGetDiscMode(plugin))
	{
		/******************************************************
		 * Reconfigure slots for all Nodes discovered till
		 * now and enable data flow.Note: If EEPROM is present
		 * A2B registers may yet  be written so delay this
		 * till EEPROM and slave plugin initialization is complete
		 * Note: 'hasEeprom' is assigned when node is discovered
		 *****************************************************/
		if(!(A2B_HAS_EEPROM(plugin, dscNodeAddr)))
		{
			/* Re-configure Down and Up slot values for this node */
			(void)adi_a2b_ReConfigSlot(plugin, dscNodeAddr);
			/* Enable Downstream and Upstream data flow */
			(void)a2b_FinalMasterSetup(plugin, A2B_NODEADDR_MASTER);
		}
	}

	return bRet;
}

/*!****************************************************************************
*
*  \b              a2b_dscvryReset
*
*  Reset discovery (variables, A2B, etc)
* 
*  \param          [in]    plugin   plugin specific data
*
*  \pre            None
*
*  \post           None
*
*  \return         A2B_EXEC_COMPLETE == Execution is now complete
*                  A2B_EXEC_SCHEDULE == Execution is unfinished - schedule again
*                  A2B_EXEC_SUSPEND  == Execution is unfinished - suspend 
*                                       scheduling until a later event
*
******************************************************************************/
static a2b_Int32
a2b_dscvryReset
(
	a2b_Plugin* plugin
)
{
	a2b_UInt8		wBuf[4], nI2sgcfgInvVal;
	a2b_UInt8		rBuf[4];
	a2b_HResult status;
#ifdef A2B_ENABLE_AD244xx_SUPPORT
	a2b_UInt16 i2cAddr;
#endif
	a2b_Bool		isAd243x = A2B_FALSE;
	a2b_Bool        isAd2430_8 = A2B_FALSE;
	a2b_Bool		bTempFrstTimeDisc;
	a2b_Bool		bTempMstrRunning;
#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
	a2b_UInt8 nTempVar;
#endif
#ifdef	A2B_FEATURE_SELF_DISCOVERY
	a2b_UInt8 bActiveBSD;
	a2b_Bool bValidBSD = A2B_FALSE;
#endif
#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
	a2b_Int32   mode = a2b_ovrGetDiscMode(plugin);
#endif


#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING
	(void)a2b_periphCfgPreparse(plugin);
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

	/* Unload any instantiated slave plugins */
	(void)a2b_stackFreeSlaveNodeHandler(plugin->ctx, A2B_NODEADDR_NOTUSED);

	/* Storing the first time discovery flag in tempoprary variable as discovery structure is being reset in next step */
	bTempFrstTimeDisc = plugin->discovery.bFrstTimeDisc;
	bTempMstrRunning = plugin->discovery.bIsMstrRunning;
	/* Some discovery tracking variables need resetting */
	(void)a2b_memset(&plugin->discovery, 0, sizeof(a2b_PluginDiscovery));
	plugin->discovery.inDiscovery = A2B_TRUE;

	/* Restoring the first time discovery flag */
	plugin->discovery.bFrstTimeDisc = bTempFrstTimeDisc;
	plugin->discovery.bIsMstrRunning = bTempMstrRunning;

#ifdef A2B_FEATURE_EEPROM_OR_FILE_PROCESSING

#ifndef A2B_FEATURE_COMM_CH
	if (a2b_stackCtxMailboxCount(plugin->ctx) !=
		A2B_ARRAY_SIZE(plugin->periph.node) + 1u)
#else
	if (a2b_stackCtxMailboxCount(plugin->ctx) !=
		A2B_ARRAY_SIZE(plugin->periph.node) + 2u)
#endif
	{
		a2b_UInt8 idx;

		for (idx = 0u; idx < A2B_ARRAY_SIZE(plugin->periph.node); idx++)
		{
#ifdef A2B_FEATURE_TRACE
			nTempVar = idx;
#endif
			/* Init some static tracking needed for timers, etc */
			plugin->periph.node[idx].nodeAddr = (a2b_Int16)idx;
			plugin->periph.node[idx].plugin = plugin;

			/* Create the mailbox for the master plugin
			 * discovery time EEPROM peripheral config
			 * handling/processing.  This allows use to
			 * do peripheral config in parallel to the
			 * discovery of other nodes.
			 */
			plugin->periph.node[idx].mboxHnd = a2b_stackCtxMailboxAlloc(
				plugin->ctx,
				A2B_JOB_PRIO0);
			if (A2B_NULL == plugin->periph.node[idx].mboxHnd)
			{
				A2B_TRACE1((plugin->ctx,
					(A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
					"dscvryReset: failed to create peripheral mailbox %bd",
					&nTempVar));
			}
		}
	}
#endif /* A2B_FEATURE_EEPROM_OR_FILE_PROCESSING */

#if defined(A2B_FEATURE_SEQ_CHART)
	A2B_SEQ_RAW1(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
		"== Starting Discovery Mode %ld ==",
		&mode);
#endif

#if defined(A2B_FEATURE_TRACE)
	A2B_TRACE2((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_DEBUG),
		"%s dscvryReset(): Starting DiscoveryMode %ld",
		A2B_MPLUGIN_PLUGIN_NAME, &mode));
#endif


	/* Read the master node's VID/PID and version */
	wBuf[0] = A2B_REG_VENDOR;
	status = a2b_regWriteRead(plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 3u, rBuf);
	if (A2B_FAILED(status))
	{
		A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
			"%s dscvryReset(): Cannot read master vid/pid",
			A2B_MPLUGIN_PLUGIN_NAME));

		a2b_ReportI2CError(plugin, A2B_NODEADDR_MASTER);
		a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
		return A2B_EXEC_COMPLETE;
	}
	plugin->nodeSig.hasSiliconInfo = A2B_TRUE;
	plugin->nodeSig.siliconInfo.vendorId = rBuf[0u];
	plugin->nodeSig.siliconInfo.productId = rBuf[1u];
	plugin->nodeSig.siliconInfo.version = rBuf[2u];
	isAd243x	= a2b_isAd243xChip(rBuf[0u] /* vid */, rBuf[1u] /* pid */);
	isAd2430_8 = a2b_isAd2430_8_Chip(rBuf[0u] /* vid */, rBuf[1u] /* pid */);


/*---------------------------------------------*/
/*       Initialize CP Regs for Master */
/*---------------------------------------------*/
#ifdef A2B_ENABLE_AD244xx_SUPPORT
	if (plugin->p244xCPNetConfigStruct != A2B_NULL)
	{
		if (plugin->p244xCPNetConfigStruct->apCPConfigStruct[0] != A2B_NULL)
		{
			i2cAddr = (a2b_UInt16)plugin->p244xCPNetConfigStruct->apCPConfigStruct[0]->nI2cAddr;
		}
		a2b_dscvryCPRegConfig(plugin, A2B_NODEADDR_MASTER);
		(void)a2b_ActiveDelay(plugin->ctx, 50);

	}
#endif
		
	/* Self Bus discovery is applicable only for AD243x onwards*/
#ifdef A2B_FEATURE_SELF_DISCOVERY


	if (a2b_isAd243xChip(rBuf[0u] /* vid */, rBuf[1u] /* pid */))
	{
#ifdef A2B_ENABLE_AD244xx_SUPPORT
		/* Only for Silicon rev 0.0*/
		if ((plugin->p244xCPNetConfigStruct != A2B_NULL) && (plugin->p244xCPNetConfigStruct->apCPConfigStruct[0] != A2B_NULL))
		{
			wBuf[0] = ((A2B_REG_244x_CP_AUTODISC & 0xFF00u) >> 8u);
			wBuf[1] = (A2B_REG_244x_CP_AUTODISC  & 0xFFu);
			status = a2b_i2cPeriphWriteRead( plugin->ctx, A2B_NODEADDR_MASTER, i2cAddr, 2u, wBuf, 1u, rBuf);
			/*Value of BSD status register*/
			bValidBSD = (rBuf[0] > 0);
		}
		else
#endif
		{

			/* Check for BSD STAT */
			wBuf[0] = A2B_REG_BSDSTAT;
			status = a2b_regWriteRead(plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 1u, rBuf);
			if (A2B_FAILED(status))
			{
				A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
					"%s dscvryReset(): Cannot read master BSDSTAT",
					A2B_MPLUGIN_PLUGIN_NAME));
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}
			bValidBSD = (rBuf[0] & A2B_BITM_BSDSTAT_BSDMODE) >> A2B_BITP_BSDSTAT_BSDMODE;
		}

		if (bValidBSD)
		{
			/*Check if self dsicovery is ignored*/
			if (!plugin->bdd->policy.bOverrideSelfDisc)
			{
				/*Override discovery mode to simple in case of self discovery*/
				plugin->overrides[0] = plugin->overrides[0] | A2B_MPLUGIN_FORCE_SIMPLE;
				if (plugin->nodeSig.siliconInfo.version == 0) /* Only for Silicon rev 0.0*/

				{
					/* Check for DISC STAT */
					wBuf[0] = A2B_REG_DISCSTAT;
					status = a2b_regWriteRead(plugin->ctx, A2B_NODEADDR_MASTER, 1u, wBuf, 1u, rBuf);
					if (A2B_FAILED(status))
					{
						A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
							"%s dscvryReset(): Cannot read master DISCSTAT",
							A2B_MPLUGIN_PLUGIN_NAME));
						a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
						return A2B_EXEC_COMPLETE;
					}
					bActiveBSD = (rBuf[0] & A2B_BITM_DISCSTAT_DSCACT) >> A2B_BITP_DISCSTAT_DSCACT;
				}
				else
				{

					bActiveBSD = rBuf[0] & A2B_BITM_BSDSTAT_BSDACTIVE >> A2B_BITP_BSDSTAT_BSDACTIVE;
				}

				if (!bActiveBSD)
				{
					/* Call PostSelfDiscInit*/
					if (!a2b_postSelfDscvryInit(plugin))
					{
						A2B_SEQ_GENERROR0(plugin->ctx, A2B_SEQ_CHART_LEVEL_DISCOVERY,
							"Error in Post Self Discovery Init");
						A2B_DSCVRY_SEQEND(plugin->ctx);

						A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
							"%s dscvryReset(): Error in Post Self Discovery Init",
							A2B_MPLUGIN_PLUGIN_NAME));
						a2b_dscvryEnd(plugin, A2B_EC_INTERNAL);
						return A2B_EXEC_COMPLETE;
					}
				}
				else
				{
					//increment the counter
					plugin->discovery.nBSDReadCount++;
					/*Trigger a timer to poll BSD STAT for Active*/
					if (!a2b_selfDscvryStartTimer(plugin))
					{
						A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
							"%s dscvryReset(): Error in Self dsicovery Start timer",
							A2B_MPLUGIN_PLUGIN_NAME));
						a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
						return A2B_EXEC_COMPLETE;

					}

				}
				return A2B_EXEC_SUSPEND;
			}

		}
		else
		{

		}

	}
#endif /* A2B_FEATURE_SELF_DISCOVERY */

	/* Do a software reset on the A2B master node */
	wBuf[0] = A2B_REG_CONTROL;
	/* The AD242X (only) needs to be told it's a Master node BEFORE
	 * the PLL locks on the SYNC pin. Once the PLL is locked, setting
	 * the MSTR bit is ignored. We set it anyway so it's clear this is
	 * the master node.
	 */
	wBuf[1] = A2B_ENUM_CONTROL_RESET_PE;

	if ((!isAd243x) && (!isAd2430_8))
	{
		wBuf[1] |= (a2b_UInt8)A2B_ENUM_CONTROL_MSTR;
		status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
		if (A2B_FAILED(status))
		{
			A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot reset master", A2B_MPLUGIN_PLUGIN_NAME));
			a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
			return A2B_EXEC_COMPLETE;
		}
	}

	if ((isAd243x) || (isAd2430_8))
	{
		/* TBD: Moving the below logic to a separate function requires proper handling of COMPLETE and SUSPEND return values */
		nI2sgcfgInvVal = plugin->bdd->nodes[A2B_MASTER_NODEBDDIDX].i2cI2sRegs.i2sgcfg & (a2b_UInt8)A2B_BITM_I2SGCFG_INV;

		if      ((  nI2sgcfgInvVal & (a2b_UInt8)A2B_BITM_I2SGCFG_INV)  && (plugin->discovery.bFrstTimeDisc == A2B_TRUE))
		{
			/* Perform soft reset */
			wBuf[0] = A2B_REG_CONTROL;
			wBuf[1] = A2B_ENUM_CONTROL_RESET_PE;
			status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
			if (A2B_FAILED(status))
			{
				A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot reset master", A2B_MPLUGIN_PLUGIN_NAME));
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}

			status = a2b_dscvryWriteMstrI2sgcfgInvSeq(plugin);
			if (A2B_FAILED(status))
			{
				A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot write I2SGCFG.INV bit sequence for master", A2B_MPLUGIN_PLUGIN_NAME));
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}
		}
		else if ((  nI2sgcfgInvVal & (a2b_UInt8)A2B_BITM_I2SGCFG_INV)  && (plugin->discovery.bFrstTimeDisc == A2B_FALSE))
		{
			/* Perform soft reset */
			wBuf[0] = A2B_REG_CONTROL;
			wBuf[1] = A2B_ENUM_CONTROL_RESET_PE;
			status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
			if (A2B_FAILED(status))
			{
				A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot reset master", A2B_MPLUGIN_PLUGIN_NAME));
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}

			/* Wait here */
			if (!a2b_dscvryStartTimer(plugin, TIMER_RE_DSCVRY, A2B_NODEADDR_MASTER))
			{
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}
			else
			{
				return A2B_EXEC_SUSPEND;
			}
		}
		else if ((!(nI2sgcfgInvVal & (a2b_UInt8)A2B_BITM_I2SGCFG_INV)) && (plugin->discovery.bFrstTimeDisc == A2B_TRUE))
		{
			/* Apply Soft reset and set MSTR bit */
			wBuf[0] = A2B_REG_CONTROL;
			wBuf[1] = A2B_ENUM_CONTROL_RESET_PE | A2B_ENUM_CONTROL_MSTR;
			status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
			if (A2B_FAILED(status))
			{
				A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot reset master", A2B_MPLUGIN_PLUGIN_NAME));
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}
		}
		else if ((!(nI2sgcfgInvVal & (a2b_UInt8)A2B_BITM_I2SGCFG_INV)) && (plugin->discovery.bFrstTimeDisc == A2B_FALSE))
		{ 
			/* Perform soft reset */
			wBuf[0] = A2B_REG_CONTROL;
			wBuf[1] = A2B_ENUM_CONTROL_RESET_PE;
			status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
			if (A2B_FAILED(status))
			{
				A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot reset master", A2B_MPLUGIN_PLUGIN_NAME));
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}

			/* Wait here */
			if (!a2b_dscvryStartTimer(plugin, TIMER_RE_DSCVRY, A2B_NODEADDR_MASTER))
			{
				a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
				return A2B_EXEC_COMPLETE;
			}
			else
			{
				return A2B_EXEC_SUSPEND;
			}
		}
		else
		{
			/* Completing the control statement */
		}
	}

	if (!a2b_dscvryStartTimer(plugin, TIMER_RESET, A2B_NODEADDR_MASTER))
	{
		a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
		return A2B_EXEC_COMPLETE;
	}

	A2B_DSCVRY_RAWDEBUG0(plugin->ctx, "dscvryReset", "...Reset Delay...");

	return A2B_EXEC_SUSPEND;

} /* a2b_dscvryReset */

  /*!****************************************************************************
  *
  *  \b              a2b_dscvryWriteMstrI2sgcfgInvSeq
  *
  *  This functions is used to write the I2SGCFG.INV bit special sequence.
  *
  *  \param          [in]    plugin   plugin specific data
  *
  *  \pre            None
  *
  *  \post           None
  *
  *  \return         A2B_EXEC_COMPLETE == Execution is now complete
  *                  A2B_EXEC_SCHEDULE == Execution is unfinished - schedule again
  *                  A2B_EXEC_SUSPEND  == Execution is unfinished - suspend
  *                                       scheduling until a later event
  *
  ******************************************************************************/
static a2b_Int32 a2b_dscvryWriteMstrI2sgcfgInvSeq(a2b_Plugin* plugin)
{
	a2b_UInt8		wBuf[4], nI2sgcfgInvVal;
	a2b_HResult		status = A2B_FALSE;

	nI2sgcfgInvVal = plugin->bdd->nodes[A2B_MASTER_NODEBDDIDX].i2cI2sRegs.i2sgcfg & (a2b_UInt8)A2B_BITM_I2SGCFG_INV;

	/* if I2SGCFG.INV bit is set */
	if (nI2sgcfgInvVal & (a2b_UInt8)A2B_BITM_I2SGCFG_INV)
	{
		/* Configure I2SGCFG.INV bit here */
		wBuf[0] = A2B_REG_I2SGCFG;
		wBuf[1] = (nI2sgcfgInvVal & (a2b_UInt8)A2B_BITM_I2SGCFG_INV);
		status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
		if (A2B_FAILED(status))
		{
			A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot write I2SGCFG.INV bit for master", A2B_MPLUGIN_PLUGIN_NAME));
			a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
			return A2B_EXEC_COMPLETE;
		}
	}

	/* Set CONTROL.MSTR bit */
	wBuf[0] = A2B_REG_CONTROL;
	wBuf[1] = (a2b_UInt8)A2B_ENUM_CONTROL_MSTR;
	status = a2b_regWrite(plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf);
	if (A2B_FAILED(status))
	{
		A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s dscvryReset(): Cannot set as master", A2B_MPLUGIN_PLUGIN_NAME));
		a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
		return A2B_EXEC_COMPLETE;
	}

	return (status);
}

/*!****************************************************************************
*
*  \b              a2b_onReDiscTimeout
*
*  Handle the TIMER_RE_DSCVRY timeout.
*
*  \param          [in]    timer
*  \param          [in]    userData
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void a2b_onReDiscTimeout(struct a2b_Timer *timer, a2b_Handle userData)
{
	a2b_HResult	status = A2B_FALSE;
	a2b_Plugin* plugin = (a2b_Plugin*)userData;

	A2B_UNUSED(timer);
	status = a2b_dscvryWriteMstrI2sgcfgInvSeq(plugin);
	if (A2B_FAILED(status))
	{
		A2B_TRACE1((plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR), "%s a2b_onDiscTimerHghPwr(): Cannot write I2SGCFG.INV bit sequence for master", A2B_MPLUGIN_PLUGIN_NAME));
		a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
	}

	if (!a2b_dscvryStartTimer(plugin, TIMER_RESET, A2B_NODEADDR_MASTER))
	{
		a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
	}
}

/*!****************************************************************************
*
*  \b              a2b_onDiscTimerHghPwrCfg4OpenDetect
*
*  Handle the TIMER_HIGH_PWR_CFG4_OPEN_DETECT timeout.
*
*  \param          [in]    timer
*  \param          [in]    userData
*
*  \pre            None
*
*  \post           None
*
*  \return         None
*
******************************************************************************/
static void a2b_onDiscTimerHghPwrCfg4OpenDetect(struct a2b_Timer *timer, a2b_Handle userData)
{

	a2b_UInt8		wBuf[2];
	a2b_HResult		status = A2B_FALSE;
	a2b_Plugin		*plugin = (a2b_Plugin*)userData;

	/* Discovered node information */
	a2b_Int16		dscNodeBddIdx = (a2b_Int16)plugin->discovery.dscNumNodes;
	a2b_Int16		dscNodeAddr = dscNodeBddIdx - 1;
	a2b_UInt8 nUserSWCTL = (a2b_UInt8)(plugin->bdd->nodes[dscNodeBddIdx].ctrlRegs.swctl);

	A2B_UNUSED(timer);
	plugin->discovery.bAd243xCfg4OpenDetect = true;

	wBuf[0] = A2B_REG_SWCTL;
	wBuf[1] = A2B_ENUM_SWCTL_ENSW_EN | (a2b_UInt8)(plugin->slaveNodeSig[dscNodeAddr].highPwrSwitchModeOverride << A2B_BITP_SWCTL_DET_OV);
	wBuf[1] |= (nUserSWCTL & A2B_REG_USER_SWCTL);

	status = a2b_regWrite(plugin->ctx, dscNodeAddr, 2u, wBuf);
	if (A2B_SUCCEEDED(status))
	{
		status = a2b_dscvryPreSlaveInit(plugin);
	}
	else
	{
		A2B_DSCVRY_ERROR1(plugin->ctx, "a2b_onDiscTimerHghPwrCfg4OpenDetect","Cannot enable phantom power on nodeAddr: %hd", &dscNodeAddr);
		a2b_dscvryEnd(plugin, (a2b_UInt32)A2B_EC_INTERNAL);
		A2B_DSCVRY_SEQEND(plugin->ctx);
	}
}
/*!****************************************************************************
*
*  \b              a2b_dscvryStart
*
*  Start the discovery process
* 
*  \param          [in]    plugin           plugin specific data
* 
*  \param          [in]    deinitFirst      deinit slave nodes prior
*                                           to discovery
*
*  \pre            None
*
*  \post           None
*
*  \return         A2B_EXEC_COMPLETE == Execution is now complete
*                  A2B_EXEC_SCHEDULE == Execution is unfinished - schedule again
*                  A2B_EXEC_SUSPEND  == Execution is unfinished - suspend 
*                                       scheduling until a later event
*
******************************************************************************/
a2b_Int32
a2b_dscvryStart
    (
    a2b_Plugin* plugin,
    a2b_Bool    deinitFirst
    )
{
	bdd_DiscoveryMode eDiscMode;
    if ( a2b_ovrGetDiscCfgMethod(plugin) == bdd_CONFIG_METHOD_AUTO )
    {
        /* Currently not supported at this time */
        A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                    "%s dscvryStart(): AUTO Config not supported", 
                     A2B_MPLUGIN_PLUGIN_NAME) );
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_INVALID_PARAMETER );
        return A2B_EXEC_COMPLETE;
    }

    eDiscMode = a2b_ovrGetDiscMode(plugin);
    if ( (bdd_DISCOVERY_MODE_SIMPLE  != eDiscMode) &&
        (bdd_DISCOVERY_MODE_MODIFIED != eDiscMode) &&
		(bdd_DISCOVERY_MODE_OPTIMIZED  != eDiscMode) &&
		(bdd_DISCOVERY_MODE_ADVANCED  != eDiscMode))
    {
        A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                    "%s dscvryStart(): unsupported discovery mode", 
                     A2B_MPLUGIN_PLUGIN_NAME ));
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_INVALID_PARAMETER );
        return A2B_EXEC_COMPLETE;
    }

    if ( plugin->discovery.inDiscovery )
    {
        A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                    "%s dscvryStart(): already in the process of discovery", 
                     A2B_MPLUGIN_PLUGIN_NAME ));
        a2b_dscvryEnd( plugin, (a2b_UInt32)A2B_EC_INVALID_STATE );
        return A2B_EXEC_COMPLETE;
    }

    if ( plugin->bdd->nodes_count == 1u )
    {
        A2B_TRACE1( (plugin->ctx, (A2B_TRC_DOM_PLUGIN | A2B_TRC_LVL_ERROR),
                    "%s dscvryStart(): No slave nodes",
                    A2B_MPLUGIN_PLUGIN_NAME ));
        return A2B_EXEC_COMPLETE;
    }

    if ( deinitFirst )
    {
        a2b_dscvryDeinitPlugin( plugin, A2B_DEINIT_START );
        if ( plugin->discovery.pendingPluginDeinit )
        {
            /* Pending deinit messages so wait to reset and start */
            return A2B_EXEC_SUSPEND;
        }
    }

    return a2b_dscvryReset( plugin );

} /* a2b_dscvryStart */

/*!****************************************************************************
*
*  \b              a2b_SimpleModeChkNodeConfig
*
*  Check if previously discovered nodes are to be configured
*
*  \param          [in]    plugin           plugin specific data
*
*  \pre            None
*
*  \post           None
*
*  \return         A2B_TRUE == NetConfiguration is to be done
*                  A2B_FALSE == NetConfiguration is not to be done
*                  
******************************************************************************/
static a2b_Bool
a2b_SimpleModeChkNodeConfig(a2b_Plugin* plugin)
{
	a2b_Bool bIsConfigReqd = A2B_FALSE;
	if((plugin->discovery.dscNumNodes !=0u) &&
			((plugin->discovery.dscNumNodes != (plugin->bdd->nodes_count)-1u)))
		{
			bIsConfigReqd = A2B_TRUE;
		}
    return bIsConfigReqd;

} /* a2b_SimpleModeChkNodeConfig */

/*!****************************************************************************
*
*  \b              adi_a2b_ReConfigSlot
*
*  This function is responsible for (re)configuring  pass down-slots and
*  pass up-slots
*
*  \param          [in]    plugin           plugin specific data
*  				   [in]	   nodeAddr			Node address of the current node
*  				   							for which the slots has to be
*  				   							reconfigured.
*
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
static a2b_HResult
adi_a2b_ReConfigSlot(a2b_Plugin* plugin,
		       a2b_Int16   nodeAddr)
{

    a2b_UInt8 wBuf[4];
    a2b_UInt8 nDnslots = 0u;
    a2b_UInt8 nUpslots = 0u;
    a2b_UInt8 nMaxBCDSlots = 0u;
    a2b_UInt8 nIndex = 0u;
    a2b_UInt16 nNodeIdx = (a2b_UInt16)((a2b_UInt32)nodeAddr+1u);
    a2b_HResult status;

    A2B_DSCVRY_SEQGROUP0( plugin->ctx,
                              "Reconfig Slots Registers" );


	nUpslots = (a2b_UInt8)plugin->bdd->nodes[nNodeIdx].ctrlRegs.lupslots;
	nDnslots = (a2b_UInt8)plugin->bdd->nodes[nNodeIdx].ctrlRegs.ldnslots;

	nNodeIdx--;
	for(nIndex=0u; nIndex< plugin->discovery.dscNumNodes-1u; nIndex++)
	{
		wBuf[0u] = A2B_REG_DNSLOTS;
		wBuf[1u] = nDnslots;
		status = a2b_regWrite(plugin->ctx, ((a2b_Int16)nNodeIdx - (a2b_Int16)1u), 2u, wBuf );

		wBuf[0u] = A2B_REG_UPSLOTS;
		wBuf[1u] = nUpslots;
		status = a2b_regWrite(plugin->ctx, ((a2b_Int16)nNodeIdx - (a2b_Int16)1u), 2u, wBuf );

		nUpslots += (a2b_UInt8)plugin->bdd->nodes[nNodeIdx].ctrlRegs.lupslots;
		nDnslots += (a2b_UInt8)plugin->bdd->nodes[nNodeIdx].ctrlRegs.ldnslots;

		if((plugin->bdd->nodes[nNodeIdx].ctrlRegs.has_bcdnslots) &&
				(nMaxBCDSlots < plugin->bdd->nodes[nNodeIdx].ctrlRegs.bcdnslots))
		{
		   nMaxBCDSlots = (a2b_UInt8)plugin->bdd->nodes[nNodeIdx].ctrlRegs.bcdnslots;
		}

		nNodeIdx--;
	}

	wBuf[0u] = A2B_REG_DNSLOTS;
	wBuf[1u] = nDnslots + nMaxBCDSlots;
	status  = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf );

	wBuf[0u] = A2B_REG_UPSLOTS;
	wBuf[1u] = nUpslots;
	status  = a2b_regWrite( plugin->ctx, A2B_NODEADDR_MASTER, 2u, &wBuf );

	A2B_DSCVRY_SEQEND(plugin->ctx);
	return(status);
} /* adi_a2b_ReConfigSlot */


/*!****************************************************************************
*
*  \b              a2b_isAd242xChipOnward
*
*  This function detects whether the A2B chip is a after AD242X
*  series chip.
*
*  \param          [in]    vendorId         Vendor Identifier
*  				   [in]	   productId		Product Identifier
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/

static a2b_Bool a2b_isAd242xChipOnward(a2b_UInt8 vendorId, a2b_UInt8 productId)
{
	return (A2B_IS_AD242X_CHIP(vendorId, productId) || A2B_IS_AD243X_CHIP(vendorId, productId) || A2B_IS_AD2430_8_CHIP(vendorId, productId));
}

/*!****************************************************************************
*
*  \b              a2b_isCrossTalkFixApply
*
*  This function detects whether the A2B chip requires Cross talk fix or not
*
*  \param          [in]    vendorId         Vendor Identifier
*  \param		   [in]	   productId		Product Identifier
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
static a2b_Bool a2b_isCrossTalkFixApply(a2b_UInt8 vendorId, a2b_UInt8 productId)
{
	return (A2B_IS_AD2425X_CHIP(vendorId, productId) || A2B_IS_AD241X_CHIP(vendorId, productId));
}


/*!****************************************************************************
*
*  \b              a2b_isAd243xChip
*
*  This function detects whether the A2B chip is a newer AD243X
*  series chip.
*
*  \param          [in]    vendorId         Vendor Identifier
*  				   [in]	   productId		Product Identifier
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/

static a2b_Bool a2b_isAd243xChip(a2b_UInt8 vendorId, a2b_UInt8 productId)
{
	return (A2B_IS_AD243X_CHIP(vendorId, productId));
}

/*!****************************************************************************
*
*  \b              a2b_isAd2430_8_Chip
*
*  This function detects whether the A2B chip is a newer AD2430 or AD2438
*  series chip.
*
*  \param          [in]    vendorId         Vendor Identifier
*  				   [in]	   productId		Product Identifier
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/

static a2b_Bool a2b_isAd2430_8_Chip(a2b_UInt8 vendorId, a2b_UInt8 productId)
{
	return (A2B_IS_AD2430_8_CHIP(vendorId, productId));
}

/*!****************************************************************************
*
*  \b              a2b_isAd243xMedHiPwrUseSwctl2
*
*  This function detects whether the A2B chip is a newer AD243X
*  series chip which has medium or high power capabilities using SWCTL2.HPSW_CFG field.
*
*  \param          [in]		plugin	Pointer to Plugin
*  				   [in]		node	Node number
*  				   [in/out] pbRet	A2B_TRUE: if we detect a medium or high power node
*									A2B_FALSE: if we do not detect a medium or high power node
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
a2b_HResult a2b_isAd243xMedHiPwrUseSwctl2(a2b_Plugin* plugin, a2b_Int16 node, a2b_Bool *pbRet)
{	
	a2b_HResult status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL);
	a2b_Bool 	isAd243x = A2B_FALSE;

	if (node == A2B_NODEADDR_MASTER)
	{
		isAd243x = A2B_IS_AD243X_CHIP(plugin->nodeSig.siliconInfo.vendorId, plugin->nodeSig.siliconInfo.productId);

		/* Populate node signature is not used here as it adds reading of REG_A2B0_VENDOR command before reading of REG_A2B0_INTTYPE register. 
		This happens when we write REG_A2B0_DISCVRY register 
		It is also ensured that the node is populated with its signature when we arrive here. 
		If its not populated we can directly use the vendorId & productId which is part of the BDD structure instead of nodeSignature structure
		*/
	}
	else
	{
		isAd243x = A2B_IS_AD243X_CHIP(plugin->slaveNodeSig[node].siliconInfo.vendorId, plugin->slaveNodeSig[node].siliconInfo.productId);

		/* Populate node signature is not used here as it adds reading of REG_A2B0_VENDOR command before reading of REG_A2B0_INTTYPE register.
		This happens when we write REG_A2B0_DISCVRY register
		It is also ensured that the node is populated with its signature when we arrive here.
		If its not populated we can directly use the vendorId & productId which is part of the BDD structure instead of nodeSignature structure
		*/
	}

	if (isAd243x)
	{
		/*	Accessing eHighPwrSwitchCfg to determine the expected HPSW setting from user */
		if (plugin->bdd->nodes[node + 1].nodeSetting.eHighPwrSwitchCfg != bdd_highPwrSwitchCfg_HPSW_CFG_0)
		{
			*pbRet = A2B_TRUE;
			status = A2B_RESULT_SUCCESS;
		}
		else
		{
			*pbRet = A2B_FALSE;
			status = A2B_RESULT_SUCCESS;
		}
	}
	else
	{
		*pbRet = A2B_FALSE;
		status = A2B_RESULT_SUCCESS;
	}

	return (status);
}

/*!****************************************************************************
*
*  \b              a2b_isAd243xMedHiPwrUseBdd
*
*  This function detects whether the A2B chip is a newer AD243X
*  series chip which has medium or high power capabilities using SWCTL2.HPSW_CFG field.
*
*  \param          [in]		plugin  Pointer to Plugin
*  				   [in]		node	Node number
*  				   [in/out]	pbRet   A2B_TRUE: if we detect a medium or high power node
*									A2B_FALSE: if we do not detect a medium or high power node
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
a2b_HResult a2b_isAd243xMedHiPwrUseBdd(a2b_Plugin* plugin, a2b_Int16 node, a2b_Bool *pbRet)
{
	a2b_HResult status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL);
	a2b_Bool 	isAd243x = A2B_FALSE;
	const		bdd_Node *bddNodeObj;

	bddNodeObj = &plugin->bdd->nodes[node + 1];

	isAd243x = A2B_IS_AD243X_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
	if (isAd243x)
	{
		/*	Accessing expected HPSW_CFG set by the user */
		if (plugin->bdd->nodes[node + 1].nodeSetting.eHighPwrSwitchCfg != bdd_highPwrSwitchCfg_HPSW_CFG_0)
		{
			*pbRet = A2B_TRUE;
			status = A2B_RESULT_SUCCESS;
		}
		else
		{
			*pbRet = A2B_FALSE;
			status = A2B_RESULT_SUCCESS;
		}
	}
	else
	{
		*pbRet = A2B_FALSE;
		status = A2B_RESULT_SUCCESS;
	}

	return (status);
}

/*!****************************************************************************
*
*  \b              a2b_stackSupportedNode
*
*  This function detects whether the stack supportes the A2B chip.
*
*  \param          [in]    vendorId         Vendor Identifier
*  				   [in]	   productId		Product Identifier
*  				   [in]	   version			Version Number
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
static a2b_Bool a2b_stackSupportedNode(a2b_UInt8 vendorId, a2b_UInt8 productId, a2b_UInt8 version)
{
	A2B_UNUSED(version);
	return (A2B_STACK_SUPPORTED_NODE(vendorId, productId, version));
}

/*!****************************************************************************
*
*  \b              a2b_CheckIfMedOrHighPwrBusPwrdNodePresentInNetwrk
*
*  This function detects if there is a medium or high power bus power nodes present in the network
*
*  \param          [in]		plugin   Pointer to Plugin
*  				   [in/out]	pbIsMedOrHighPwrBusPwrdNodePresent	
*  									A2B_TRUE: if we detect a medium or high power node
*									A2B_FALSE: if we do not detect a medium or high power node
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
a2b_HResult a2b_CheckIfMedOrHighPwrBusPwrdNodePresentInNetwrk(a2b_Plugin* plugin, a2b_Bool *pbIsMedOrHighPwrBusPwrdNodePresent)
{
	a2b_UInt8  nIndex;
	a2b_Bool	isAd243xMedHiPwrChip = A2B_FALSE;
	a2b_HResult status = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL);

	status = a2b_isAd243xMedHiPwrUseBdd(plugin, A2B_NODEADDR_MASTER, &isAd243xMedHiPwrChip);
	/* Master is always local powered */
	if ((A2B_SUCCEEDED(status)) && (isAd243xMedHiPwrChip == A2B_TRUE))
	{
		if (plugin->bdd->nodes[1].nodeSetting.bLocalPwrd == bdd_nodePowerMode_SLAVE_BUS_POWERED)
		{
			*pbIsMedOrHighPwrBusPwrdNodePresent = A2B_TRUE;

			/* Immediately return as we found master trying to discover a medium or high power bus powered node */
			return (A2B_RESULT_SUCCESS);
		}
	}
	else if (A2B_FAILED(status))
	{
		return (A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL));
	}
	else
	{
		*pbIsMedOrHighPwrBusPwrdNodePresent = A2B_FALSE;
		status = A2B_RESULT_SUCCESS;
	}

	for (nIndex = 0U; nIndex < plugin->discovery.dscNumNodes; nIndex++)
	{
		status = a2b_isAd243xMedHiPwrUseBdd(plugin, nIndex, &isAd243xMedHiPwrChip);
		if ((A2B_SUCCEEDED(status)) && (plugin->bdd->nodes[nIndex + 1U].nodeSetting.bLocalPwrd == bdd_nodePowerMode_SLAVE_BUS_POWERED) && (isAd243xMedHiPwrChip == A2B_TRUE))
		{
			*pbIsMedOrHighPwrBusPwrdNodePresent = A2B_TRUE;

			/* Immediately return as we found a current node trying to discover a next medium or high power bus powered node */
			return (A2B_RESULT_SUCCESS);
		}
		else if (A2B_FAILED(status))
		{
			return (A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_PLUGIN, A2B_EC_INTERNAL));
		}
		else
		{
			*pbIsMedOrHighPwrBusPwrdNodePresent = A2B_FALSE;
			status = A2B_RESULT_SUCCESS;
		}
	}

	return (status);
}

/*!****************************************************************************
*
*  \b              a2b_CheckIfAD243xNodePresentInNetwrk
*
*  This function detects if there is a AD243x node present in the network based on information in BDD object
*
*  \param          [in]		plugin   Pointer to Plugin
*  				   [in/out]	pbIsAD243xNodeNodePresent
*  									A2B_TRUE: if we detect a AD243x node
*									A2B_FALSE: if we do not detect a AD243x node
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
void a2b_CheckIfAD243xNodePresentInNetwrk(a2b_Plugin* plugin, a2b_Bool *pbIsAD243xNodeNodePresent)
{	
	a2b_Bool 	isAd243x = A2B_FALSE;
	a2b_UInt8	nbddNodeIdx;
	const		bdd_Node *bddNodeObj;

	*pbIsAD243xNodeNodePresent = A2B_FALSE;

	for (nbddNodeIdx = 0 ; nbddNodeIdx < plugin->bdd->nodes_count ; nbddNodeIdx++)
	{
		bddNodeObj = &plugin->bdd->nodes[nbddNodeIdx];

		isAd243x = A2B_IS_AD243X_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
		if (isAd243x)
		{
			*pbIsAD243xNodeNodePresent = A2B_TRUE;
			break;
		}
	}
}

/*!****************************************************************************
*
*  \b              a2b_CheckIfAD2430_8NodePresentInNetwrk
*
*  This function checks if Plutus is present in network or not
*
*  \param          [in]		plugin   Pointer to Plugin
*  				   [in/out]	pbIsAD2430_8NodeNodePresent
*  									A2B_TRUE: if we detect a AD2430/8 node
*									A2B_FALSE: if we do not detect a AD2430/8 node
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
void a2b_CheckIfAD2430_8NodePresentInNetwrk(a2b_Plugin* plugin, a2b_Bool* pbIsAD2430_8NodeNodePresent)
{
	a2b_Bool 	isAd2430_38_master = A2B_FALSE, isAd2430_38_lastnode = A2B_FALSE;
	const		bdd_Node* bddNodeObj;
	

	*pbIsAD2430_8NodeNodePresent = A2B_FALSE;
	
	bddNodeObj = &plugin->bdd->nodes[0];
	isAd2430_38_master = A2B_IS_AD2430_8_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
	bddNodeObj = &plugin->bdd->nodes[plugin->bdd->nodes_count - 1u];
	isAd2430_38_lastnode = A2B_IS_AD2430_8_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);

	if( (isAd2430_38_master) || (isAd2430_38_lastnode))
	{
		*pbIsAD2430_8NodeNodePresent = A2B_TRUE;
	}
}

/*!****************************************************************************
*
*  \b              a2b_CheckIfAD2430_8NodeMasterPrsnt
*
*  This function checks if upstream LPS is AD2430 / 38 master or not
*
*  \param          [in]		plugin   Pointer to Plugin
*  				   [in/out]	pbIsAD2430_8NodeNodePresent
*  									A2B_TRUE: if we detect a AD2430/8 master node
*									A2B_FALSE: if we do not detect a AD2430/8 node
*  \pre            None
*
*  \post           None
*
*
******************************************************************************/
void a2b_CheckIfAD2430_8NodeMasterPrsnt(a2b_Plugin* plugin, a2b_Bool* pbIsAD2430_8NodeNodePresent)
{
	a2b_Bool 	isAd2430_8 = A2B_FALSE;
	a2b_UInt8	nbddNodeIdx;
	const		bdd_Node* bddNodeObj;


	*pbIsAD2430_8NodeNodePresent = A2B_FALSE;
	
	nbddNodeIdx = (plugin->pwrDiag.upstrSelfPwrNode + 1);
	bddNodeObj = &plugin->bdd->nodes[nbddNodeIdx];

	if (bddNodeObj->nodeType == bdd_NODE_TYPE_MASTER)
	{
		isAd2430_8 = A2B_IS_AD2430_8_CHIP(bddNodeObj->nodeDescr.vendor, bddNodeObj->nodeDescr.product);
		if (isAd2430_8)
		{
			*pbIsAD2430_8NodeNodePresent = A2B_TRUE;
		}
	}
}
/**
 @}
*/


/**
 @}
*/
