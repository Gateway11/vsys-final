/*=============================================================================
 *
 * Project: a2bstack
 *
 * Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
 * This software is subject to the terms and conditions of the license set 
 * forth in the project LICENSE file. Downloading, reproducing, distributing or 
 * otherwise using the software constitutes acceptance of the license. The 
 * software may not be used except as expressly authorized under the license.
 *
 *=============================================================================
 *
 * \file:   defs.h
 * \author: Mentor Graphics, Embedded Software Division
 * \brief:  Miscellaneous system-wide definitions and types.
 *
 *=============================================================================
 */

/*============================================================================*/
/** 
 * \defgroup a2bstack_defs              Global Stack Definitions
 *  
 * Miscellaneous system-wide definitions and types.
 *
 * \{ */
/*============================================================================*/

#ifndef A2B_DEFS_H_
#define A2B_DEFS_H_

/*======================= I N C L U D E S =========================*/

#include "a2b/macros.h"
#include "a2b/ctypes.h"
#include "a2b/conf.h"

/*======================= D E F I N E S ===========================*/

/*----------------------------------------------------------------------------*/
/**
 * \name    Message Actions 
 *  
 * Actions associated with executing messages 
 *  
 * \{ */
/*----------------------------------------------------------------------------*/

/** Execution is now complete */
#define A2B_EXEC_COMPLETE           (1)

/** Execution is unfinished - schedule again */
#define A2B_EXEC_SCHEDULE           (2)

/** Execution is unfinished - suspend scheduling until a later event */
#define A2B_EXEC_SUSPEND            (3)

/** \} */

/** Peripheral address for an EEPROM */
#define A2B_I2C_EEPROM_ADDR         (0x50u)

/** Special marked to ID EEPROM as A2B configuration */
#define A2B_MARKER_EEPROM_CONFIG    (0xABu)

/** This is default mailbox for typical message handling
 *  for a stack contexts mailbox(es).
 */
#define A2B_MSG_MAILBOX             (A2B_NULL)

/** A node address that will never be used for the master or slave nodes */
#define A2B_NODEADDR_NOTUSED        (-2)

/** Used for some interfaces to indicate ALL nodes. */
#define A2B_NODEADDR_ALL            (A2B_NODEADDR_NOTUSED)

/** The node address for the A2B master node */
#define A2B_NODEADDR_MASTER         (-1)


/** Maximum number fo Ports for CP */
#define A2B_MAX_CP_PORTS         (24)



/*======================= D A T A T Y P E S =======================*/

A2B_BEGIN_DECLS

/** Common callback signature */
typedef void (* a2b_CallbackFunc)(a2b_Handle);

/**
 * Information about a registered master/slave node
 */
typedef struct a2b_NodeInfo
{
    /** Vendor Identifier */
    a2b_UInt8 vendorId;

    /** Product Identifier */
    a2b_UInt8 productId;

    /** Module version */
    a2b_UInt8 version;

} a2b_NodeInfo;

///** Enum for A2B node power source */
//typedef enum
//{
//	A2B_BUS_POWERD = 0u,	/*!< The current node is bus powered node */
//	A2B_LOCAL_POWERED		/*!< The current node is locally powered node */
//} a2b_PowerSource;
//
///**
//* A2B board/system level information which requires to be stored 
//*/
//typedef struct a2b_SystemInfo
//{
//	/** Identifies whether the current A2B node is bus powered or local powered 
//	*	true : current A2B node is bus powered
//	*   false: current A2B node is local powered
//	*/
//	a2b_PowerSource ePowerSource;
//
//} a2b_SystemInfo;

/** Priority values associated with jobs */
typedef enum
{
    A2B_JOB_PRIO0 = 0u, /*!< Highest priority */
    A2B_JOB_PRIO1,
    A2B_JOB_PRIO2,
    A2B_JOB_PRIO3,
    A2B_JOB_PRIO4      /*!< Lowest priority */

} a2b_JobPriority;

/** This is a signature structure used to uniquely describe
 *  a master/slave node.  This signature tracks versions
 *  from silicon, EEPROM, and BDD--among other information.
 */
typedef struct a2b_NodeSignature
{
    /** A2B_TRUE is siliconInfo is was read successfully,
     *  A2B_FALSE if siliconInfo should be ignored.
     */
    a2b_Bool                 hasSiliconInfo;

    /** Read from the A2B node silicon */
    struct a2b_NodeInfo      siliconInfo;

    /** A2B_TRUE is eepromInfo is was read successfully,
     *  A2B_FALSE if eepromInfo should be ignored.
     *  Always A2B_FALSE for master.
     */
    a2b_Bool                 hasEepromInfo;

    /** Read from an EEPROM attached to the slave node */
    struct a2b_NodeInfo      eepromInfo;

    /** A2B_TRUE is bddInfo is was parsed successfully,
     *  A2B_FALSE if bddInfo should be ignored.
     */
    a2b_Bool                 hasBddInfo;

    /** Node descriptor info from the BDD */
    struct a2b_NodeInfo      bddInfo;

    /** A2B_TRUE if A2B indicates I2C bus is available */
    a2b_Bool                 hasI2cCapability;

    /** Identifier for the node being handled by the plugin.  This
     *  reflects the nodeAddr used in the A2B network.  The master
     *  nodeAddr will be set to #A2B_NODEADDR_MASTER.
     */
    a2b_Int16               nodeAddr;


	/* Indicates whether switch mode is supposed to be overden during discovery flow */
	a2b_UInt8			    highPwrSwitchModeOverride;

    /** This returns the name of the instantiated
     * plugin (e.g. a2b_msgRtrGetHandler). When this structure is used as an
     * input parameter this field is ignored.
     */
    a2b_Char                pluginName[A2B_CONF_DEFAULT_PLUGIN_NAME_LEN];

	///** Captures the A2B system level information of the current node */
	//a2b_SystemInfo			systemInfo;
} a2b_NodeSignature;


/*======================= P U B L I C  P R O T O T Y P E S ========*/


A2B_END_DECLS

/*======================= D A T A =================================*/


/*======================= M A C R O S =============================*/

/** Helper to range check nodeAddr */
#define A2B_VALID_NODEADDR(nodeAddr) (((a2b_Bool)((a2b_Int32)nodeAddr >= A2B_NODEADDR_MASTER)) && \
									((a2b_Bool) ((a2b_Int32)nodeAddr < (a2b_Int32)A2B_CONF_MAX_NUM_SLAVE_NODES)))

/** Macro used to initialize the #a2b_NodeSignature */
#ifndef _TESSY_NO_DOWHILE_MACROS_
#define A2B_INIT_SIGNATURE( pSig, addr ) \
    do { \
    	(void)a2b_memset( (pSig), 0, sizeof(a2b_NodeSignature) ); \
        (pSig)->nodeAddr = addr; \
    } while ( 0 )
#else   /* _TESSY_NO_DOWHILE_MACROS_ */
#define A2B_INIT_SIGNATURE( pSig, addr ) \
    { \
    	(void)a2b_memset( (pSig), 0, sizeof(a2b_NodeSignature) ); \
        (pSig)->nodeAddr = addr; \
    }
#endif  /* _TESSY_NO_DOWHILE_MACROS_ */
/** Macro used to initialize the #a2b_NodeSignature silicon version */
#define A2B_INIT_SIGNATURE_SILICON( pSig, pSiliconInfo ) \
    do { \
    (pSig)->hasSiliconInfo = A2B_TRUE; \
    (pSig)->siliconInfo.vendorId = (pSiliconInfo)->vendorId; \
    (pSig)->siliconInfo.productId = (pSiliconInfo)->productId; \
    (pSig)->siliconInfo.version = (pSiliconInfo)->version; \
    } while ( 0 )

/** Macro used to initialize the #a2b_NodeSignature EEPROM version */
#define A2B_INIT_SIGNATURE_EEPROM( pSig, pEepromInfo ) \
    do { \
    (pSig)->hasEepromInfo = A2B_TRUE; \
    (pSig)->eepromInfo.vendorId = (pEepromInfo)->vendorId; \
    (pSig)->eepromInfo.productId = (pEepromInfo)->productId; \
    (pSig)->eepromInfo.version = (pEepromInfo)->version; \
    } while ( 0 )

/** Macro used to initialize the #a2b_NodeSignature BDD version */
#ifndef _TESSY_NO_DOWHILE_MACROS_
#define A2B_INIT_SIGNATURE_BDD( pSig, pBddInfo ) \
    do { \
    (pSig)->hasBddInfo = A2B_TRUE; \
    (pSig)->bddInfo.vendorId = (pBddInfo)->vendorId; \
    (pSig)->bddInfo.productId = (pBddInfo)->productId; \
    (pSig)->bddInfo.version = (pBddInfo)->version; \
    } while ( 0 )
#else    /*_TESSY_NO_DOWHILE_MACROS_*/
#define A2B_INIT_SIGNATURE_BDD( pSig, pBddInfo ) \
    { \
    (pSig)->hasBddInfo = A2B_TRUE; \
    (pSig)->bddInfo.vendorId = (pBddInfo)->vendorId; \
    (pSig)->bddInfo.productId = (pBddInfo)->productId; \
    (pSig)->bddInfo.version = (pBddInfo)->version; \
    }
#endif   /* _TESSY_NO_DOWHILE_MACROS_ */


/** This macro detects whether the A2B chip is an older AD241X
 * series chips.
 */
#define A2B_IS_AD241X_CHIP(vid, pid) \
            ((a2b_Bool)(((pid == 0x01u) || (pid == 0x02u) || \
             (pid == 0x03u) || (pid == 0x10u)) && (vid == 0xADu)))

/** This macro detects whether the A2B chip is a newer AD24XX
 * series chip.
 */
#define A2B_IS_AD242X_CHIP(vid, pid) \
            ((a2b_Bool)((vid == 0xADu) && ((pid == 0x21u) || (pid == 0x22u) || \
										   (pid == 0x23u) || (pid == 0x25u) || \
										   (pid == 0x26u) || (pid == 0x27u) || \
										   (pid == 0x28u) || (pid == 0x29u) || (pid == 0x20u)  )))

/** This macro detects whether the A2B chip is a  AD2425 series(AD2425, AD2422, AD2421, 23)
* series chip.
*/
#define A2B_IS_AD2425X_CHIP(vid, pid) \
	((a2b_Bool)((vid == 0xADu) && ((pid == 0x25u) || \
	(pid == 0x21u)||(pid == 0x22u) || (pid == 0x23u))))


 /** This macro detects whether the A2B chip is a newer AD243X
 * series chip.
 */
#define A2B_IS_AD243X_CHIP(vid, pid) \
            ((a2b_Bool)((vid == 0xADu) && ((pid == 0x31u) || (pid == 0x32u) || \
										   (pid == 0x33u) || (pid == 0x35u) || (pid == 0x37u) )))

 /** This macro detects whether the A2B chip is a AD2430/38
* series chip. 
*/
#define A2B_IS_AD2430_8_CHIP(vid, pid) \
           ((a2b_Bool)((vid == 0xADu) && ((pid == 0x30u) || (pid == 0x38u)  \
										  )))

/** This macro detects whether the A2B chip is a AD2437
* series chip.
*/
#define A2B_IS_AD2437_CHIP(vid, pid) \
          ((a2b_Bool)((vid == 0xADu) && (pid == 0x37u)  \
										  ))

/** This macro detects whether the A2B chip is a newer AD2428x (AD2428, AD2426, AD2427)
* series chip.
*/
#define A2B_IS_AD2428X_CHIP(vid, pid) \
	((a2b_Bool)((vid == 0xADu) && ((pid == 0x26u) || (pid == 0x27u) || \
	(pid == 0x28u)||(pid == 0x29u) || (pid == 0x20u))))

#define A2B_IS_AD232X_CHIP(vid, pid, ver) \
    ((a2b_Bool)((vid == 0XAD) && ((pid == 0x27) || (pid == 0x28)) && (ver == 0x80)))

/** This defines the logic used when a node is discovered
 *  to verify that the discovered node can be supported
 *  by the stack. (vid=vendorId, pid=productId, ver=version)
 */
#define A2B_STACK_SUPPORTED_NODE(vid, pid, ver) \
            ( \
            /* Older AD241X chips with versions 0x10 & 0x21 */ \
              (a2b_Bool)( A2B_IS_AD241X_CHIP(vid, pid) 	|| 	\
            		  	  A2B_IS_AD242X_CHIP(vid, pid)	|| 	\
						  A2B_IS_AD243X_CHIP(vid, pid)  ||	\
                          A2B_IS_AD232X_CHIP(vid, pid, ver) || \
                          A2B_IS_AD2430_8_CHIP(vid, pid)) \
            )
#endif /* A2B_DEFS_H_ */
