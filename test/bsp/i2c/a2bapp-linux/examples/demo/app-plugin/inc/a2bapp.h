/*******************************************************************************
Copyright (c) 2023 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @file: a2bapp.h
* @brief: This file contains Typedefs,configurable macros, structures for running
*          A2B application
* @version: $Revision: 3888 $
* @date: $Date: 2016-01-17 18:46:38 +0530 (Sun, 17 Jan 2016) $
* Developed by: Automotive Software and Systems team, Bangalore, India
*****************************************************************************/

#ifndef __A2BAPP_H__
#define __A2BAPP_H__

/*! \addtogroup Application_Reference
 *  @{
 */

/*============= I N C L U D E S =============*/
#include <stddef.h>
#include "a2b/pal.h"
#include "a2b/conf.h"
#include "a2b/defs.h"
#include "a2b/util.h"
#include "a2b/msg.h"
#include "a2b/msgrtr.h"
#include "a2b/trace.h"
#include "a2b/stack.h"
#include "a2b/seqchart.h"
#include "a2b_bdd_helper.h"
#include "a2b/regdefs.h"
#include "a2b/interrupt.h"
#include "a2b/hwaccess.h"
#include "a2b/system.h"
#include "a2b/diag.h"
#include "a2bplugin-master/plugin.h"
#include "a2bplugin-slave/plugin.h"
#include "timer_priv.h"
#include "stackctx.h"
#include "stdio.h"
#ifdef A2B_ENABLE_AD244xx_SUPPORT
#include "adi_a2b_244x_config.h"
#endif

/*============= D E F I N E S =============*/
#ifndef A2BAPP_LINK_STATICALLY
#define A2BAPP_LINK_STATICALLY
#endif

#define A2B_DISABLE				(0)
#define A2B_ENABLE				(1)

/* Polling interval in ms */
#define A2BAPP_POLL_PERIOD      (1)
/*! Bus Drop Check period in seconds */
#define A2B_BUS_DROP_CHK_PERIOD      	(5u)

#define A2B_APP_TMRTOHANDLE_BECOVF_AFTER_INTERVAL	(1000)	/* In milliseconds */
#define A2B_APP_TMRTOHANDLE_BECOVF_REPEAT_INTERVAL	(1000)	/* In milliseconds */

/* Base memory for stack depends upon the macros in Conf.h & feaure.h. If the macros are changed, profile the stack to update the memory.
 * The base memory can vary across the stack versions.  */
#define A2BAPP_STACK_MAX_BASE_MEMEORY  		(3328) /* In Bytes, includes Trace and Commchan */
#define A2BAPP_STACK_MEM_PER_SLAVE	   		(256)  /* In Bytes, aligning to 256 byte boundary */
#define A2BAPP_STACK_MEMORY_PER_CHAIN  		(A2BAPP_STACK_MAX_BASE_MEMEORY  + A2BAPP_STACK_MEM_PER_SLAVE* A2B_CONF_MAX_NUM_SLAVE_NODES)
#define A2BAPP_STACK_NW_MEMORY         		(A2BAPP_STACK_MEMORY_PER_CHAIN * A2B_CONF_MAX_NUM_MASTER_NODES)

/* Plugin memory requirment */
#define A2BAPP_MAX_MEM_PER_PLUGIN			(64)	/* Size of a2b_PluginApi is 48 bytes , aligning to 64 byte boundary */
#define A2BAPP_PLUGIN_MEMORY_PER_CHAIN		(A2BAPP_MAX_MEM_PER_PLUGIN * (A2B_CONF_MAX_NUM_SLAVE_NODES + 1U))
#define A2BAPP_PLUGIN_NW_MEMORY				(A2BAPP_PLUGIN_MEMORY_PER_CHAIN * A2B_CONF_MAX_NUM_MASTER_NODES)

/* Memory requirement to read single E2PROM block, usually determined by I2C burst size for remote peripheral */
#define A2BAPP_E2PROM_BLOCK_MEMORY			(4096)

#ifdef A2BAPP_ENABLE_RTMBOOT
#define A2BAPP_RTM_NODEADDR (1u)
#endif

#ifdef A2B_PRINT_CONSOLE
#ifndef A2B_PRINT_DEFINED
#define A2B_APP_LOG(...) do{\
						(void)printf(__VA_ARGS__);\
                        }while(0)
#endif
#else
#define A2B_APP_LOG(...)
#endif


#ifdef A2B_PRINT_CONSOLE
#ifndef A2B_PRINT_DEFINED
#define A2B_APP_DBG_LOG(...) do{\
						if(gpApp_Info[0]->bDebug)\
						(void)printf(__VA_ARGS__);\
                        }while(0)
#endif
#else
#define A2B_APP_DBG_LOG(...)
#endif

/*============= D A T A T Y P E S=============*/

typedef struct {

	/* Input flags */
	a2b_Bool	bDebug;
#ifdef A2B_FEATURE_SEQ_CHART
	a2b_Char* seqFile;
#endif

	/* Objects for storing info populated inside a2bapp.c */
	A2B_ECB ecb;											/*!< App envirnment control block  */   
	a2b_StackPal pal;										/*!< PAL layer  */
	struct a2b_StackContext *ctx;							/*!< Stack context  */  
	bdd_Network bdd;										/*!< BDD Info  */
	ADI_A2B_BCD* pBusDescription;							/*!< Pointer to Bus Description File */
	ADI_A2B_NETWORK_CONFIG* pTargetProperties;				 /*!< Pointer to Bus Description File */
	ADI_A2B_NODE_PERICONFIG  aPeriNetworkTable[A2B_CONF_MAX_NUM_SLAVE_NODES + 1]; 	/*!< Table to get peripheral configuration structure */
	a2b_UInt8 anEeepromPeriCfgInfo[2 * (A2B_CONF_MAX_NUM_SLAVE_NODES + 1)];
	struct a2b_MsgNotifier *notifyInterrupt;				/*!< Interrupt Notifier  */
	struct a2b_MsgNotifier *notifyPowerFault;				/*!< Power Fault message notifier */
	struct a2b_MsgNotifier *notifyI2CError;   				/*!< I2C Error notifier */
	struct a2b_MsgNotifier *notifyNodeDiscvry;				/*!< Node Discovery notifier */
	struct a2b_Timer* hTmrToHandleBecovf;					/*!< Timer Handler for Bit-error   */

	/* Processing flags local to a2bapp.c */
	a2b_Bool discoveryDone;									/*!< Discovery Done Status  */
	a2b_Bool bCustomAuthFailed;								/*!< Custom node authentication failure indication */
	a2b_Bool bBusDropDetected;								/*!< Flag to detect the Bus drop */
	a2b_Bool bRetry;										/*!< Retry enabled or disabled */
	a2b_UInt32 nDiscTryCnt;									/*!< Count of no of re-discovery attempts  */
	a2b_Bool bBecovfTimerEnable;							/*!< Enable flag for starting timer for resetting bit error count */
	a2b_UInt32 nBecovfRstCnt;
	a2b_UInt8 nNumBCD;
	a2b_UInt8 nDefaultBCDIndex;
	a2b_UInt8 bIntrptLatch;
	a2b_UInt8 bSpiIntrptLatch;								/*!< Flag to check if there is an SPI interrupt present */


	/* Output flags */
	a2b_UInt8 nodesDiscovered;								/*!< Number of slave nodes discovered  */
	a2b_Bool discoverySuccessful;							/*!< Discovery success status */
	const a2b_Char *faultStatus;							/*!< String indicating line fault */
	a2b_Int8 faultNode;										/*!< Node number at which fault occured */
	a2b_UInt8 faultCode;									/*!< Fault code */
	a2b_Bool bfaultDone;									/*!< Flag specifying whether fault has occured */
    a2b_UInt8* panDatFileBuff; 								/*!< Buffer for Static or Dynamically memory allocation */
	/** Flag which indicates the current is first time discovery or re-discovering the network
	*   bFrstTimeDisc value
	*				true: Current execution is first time discovery
	*				false: Current execution is re-discovery
	*/
	a2b_Bool bFrstTimeDisc;

#ifdef A2B_RUN_BIT_ERROR_TEST
	ADI_A2B_BERT_HANDLER oBertHandler;
	a2b_UInt8 *pBertConfigBuff;
	a2b_Bool bIsBertStart;
#endif

#ifdef A2B_ENABLE_AD244xx_SUPPORT
	ADI_A2B_244x_CP_NETCONFIG *p244xCPNetConfig;
	struct a2b_MsgNotifier *notifyCPInterrupt;
	a2b_UInt8 nValidCPNodes;
#endif

#ifdef A2B_FEATURE_COMM_CH
	a2b_Bool  bTxDoneSuccess;
	a2b_Bool  bTxComplete;

	a2b_UInt32 nTxSeqCnt;
	a2b_UInt32 nRxSeqCnt;
	a2b_Bool   bCommChErrSet;
#endif

#if defined(A2B_BCF_FROM_SOC_EEPROM) || defined(A2B_BCF_FROM_FILE_IO)
	a2b_UInt8 nNumChains;
	a2b_UInt8* pCustomIdInfo;

#endif
	a2b_UInt16 nCustomIdPtr[A2B_CONF_MAX_NUM_SLAVE_NODES + 1];
	a2b_UInt8 nLenCustomId[A2B_CONF_MAX_NUM_SLAVE_NODES + 1];
} a2b_App_t;

extern a2b_App_t gApp_Info;
extern a2b_App_t *gpApp_Info[A2B_CONF_MAX_NUM_MASTER_NODES];

/************** PWM related defines for AD243x ***************/
/*----------------------------------------------------------------------------*/
/*  Supported PWM channels                                                    */
/*----------------------------------------------------------------------------*/
typedef enum
{
	A2B_PWM_CHANNEL_1	= 0x00, /* PWM channel 1 */
	A2B_PWM_CHANNEL_2	= 0x01, /* PWM channel 2 */
	A2B_PWM_CHANNEL_3	= 0x02,  /* PWM channel 3 */
	A2B_PWM_CHANNEL_OE 	= 0x03 /* Output Enable */
} a2b_PwmChnl;

/*----------------------------------------------------------------------------*/
/* Supported PWM frequencies                                                  */
/*----------------------------------------------------------------------------*/
typedef enum
{
	A2B_PWM_FREQ_192KHZ	 = 0x00, /* 192 kHz */
	A2B_PWM_FREQ_96KHZ	 = 0x01, /*  96 kHz */
	A2B_PWM_FREQ_48KHZ	 = 0x02, /*  48 kHz */
	A2B_PWM_FREQ_24KHZ	 = 0x03, /*  24 kHz */
	A2B_PWM_FREQ_12KHZ	 = 0x04, /*  12 kHz */
	A2B_PWM_FREQ_6KHZ	 = 0x05, /*   6 kHz */
	A2B_PWM_FREQ_3KHZ	 = 0x06, /*   3 kHz */
	A2B_PWM_FREQ_1500HZ	 = 0x07, /* 1.5 kHz */
	A2B_PWM_FREQ_750HZ	 = 0x08, /* 750 Hz */
	A2B_PWM_FREQ_375HZ	 = 0x09, /* 375 Hz */
	A2B_PWM_FREQ_187HZ	 = 0x0A, /* 187.5 Hz */
	A2B_PWM_FREQ_HOPPING = 0x0B  /* Frequency hopping between 187.5 Hz to 3 kHz */
} a2b_PwmFreq;

/*----------------------------------------------------------------------------*/
/* Supported PWM blink periods                                                */
/*----------------------------------------------------------------------------*/
typedef enum
{
	A2B_PWM_NO_BLINK	 = 0x00, /* No Blink */
	A2B_PWM_BLINK_P25	 = 0x01, /* 0.25s Blink */
	A2B_PWM_BLINK_P5	 = 0x02, /* 0.5s Blink */
	A2B_PWM_BLINK_P75	 = 0x03, /* 0.75s Blink */
	A2B_PWM_BLINK_1 	 = 0x04, /* 1s Blink */
} a2b_PwmBlink;

/*======= P U B L I C P R O T O T Y P E S ========*/

a2b_UInt32 a2b_setup(a2b_App_t *pApp_Info);
a2b_UInt32 a2b_fault_monitor(a2b_App_t *pApp_Info);
a2b_HResult adi_a2b_extractcustominformation(a2b_App_t *pApp_Info, a2b_UInt8 pBuff[], a2b_UInt8 nBuffSize, a2b_UInt8 nNodeNum);
a2b_UInt32 a2b_multimasterSetup(a2b_App_t *pApp_Info);
a2b_UInt32 a2b_multiMasterFault_monitor(a2b_App_t *pApp_Info);
a2b_HResult a2b_processSpiIntrpt(a2b_App_t *pApp_Info);
a2b_HResult a2b_AppPWMSetup(struct a2b_StackContext *ctx, a2b_Int16 nodeAddr, a2b_PwmChnl  chnlPWM,	a2b_PwmFreq freqPWM,
							a2b_PwmBlink blinkPWM, a2b_UInt16  nDutyCycle);
a2b_HResult a2b_AppPWMControl(struct a2b_StackContext *ctx, a2b_Int16 nodeAddr, a2b_PwmChnl  chnlPWM, a2b_PwmBlink blinkPWM, a2b_UInt16  nDutyCycle);
#endif /* __A2BAPP_H__ */

/**
 @}
*/
