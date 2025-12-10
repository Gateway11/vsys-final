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
 * \file:   regdefs.h
 * \author: Mentor Graphics, Embedded Software Division
 * \brief:  Provides AD2410 register definitions and masks.
 *
 *=============================================================================
 */

/*============================================================================*/
/** 
 * \defgroup a2bstack_regdefs           Register Definitions
 *  
 * Provides AD2410 register definitions and masks.
 *
 * \{ */
/*============================================================================*/

#ifndef A2B_REGDEFS_H_
#define A2B_REGDEFS_H_

/*======================= I N C L U D E S =========================*/

/*======================= D E F I N E S ===========================*/

/*----------------------------------------------------------------------------*/
/** 
 * \defgroup a2bstack_regdefs_offsets       Register Offsets
 *  
 * These are the register offset values.
 *
 * \{ */
/*----------------------------------------------------------------------------*/
#define A2B_REG_USER_SWCTL 					(0x007BU)
/* ============================================================================================================================ */
/** \name   A2B Register Address Definitions */
/* ============================================================================================================================ */
#define A2B_MOD_BASE                        (0x00000000u)             /*!<  A2B Base Offset */
#define A2B_MOD_MASK                        (0x00000FFFu)             /*!<  A2B Offset Mask */
#define A2B_REG_PINCFG                      (0x00000052u)             /*!<  A2B Pin Configuration Register */
/* ============================================================================================================================ */
/** \name   A2B Analog Tuning Register Definitions */
/** \note   Private undocumented internal registers. Not to be modified by the user */
/* ============================================================================================================================ */
#define A2B_REG_VREGCTL                     (0x0000002Du)             /*!<  A2B Voltage Regulator Control Register */
#define A2B_REG_TXACTL                      (0x0000002Eu)             /*!<  A2B LVDSA TX Control Register */
#define A2B_REG_RXACTL                      (0x0000002Fu)             /*!<  A2B LVDSA RX Control Register */
#define A2B_REG_TXBCTL                      (0x00000030u)             /*!<  A2B LVDSB TX Control Register */
#define A2B_REG_RXBCTL                      (0x00000031u)             /*!<  A2B LVDSB RX Control Register */

/* The registers from 0x2E to 0x31 are reused for different purposes in Demeter */
#define A2B_REG_TXCTL                       (0x0000002Eu)             /*!<  A2B LVDS TX Control Register */
#define A2B_REG_RXCTL                       (0x0000002Fu)             /*!<  A2B LVDS RX Control Register */
#define A2B_REG_TERMCTL                     (0x00000030u)             /*!<  A2B LVDS Termination Control Register */
#define A2B_REG_LVDSTM                      (0x00000031u)             /*!<  A2B LVDS Test Mode Register */
/* ============================================================================================================================ */
/** \name   A2B Config, Master Only, Auto-Broadcast, Shadowed Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_SLOTFMT                     (0x00000010u)             /*!<  A2B Slot Format Register (Master Only, Auto-Broadcast) */
#define A2B_REG_DATCTL                      (0x00000011u)             /*!<  A2B Data Control Register (Master Only, Auto-Broadcast) */
/* ============================================================================================================================ */
/** \name   A2B Config, Shadowed Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_DNSLOTS                     (0x0000000Du)             /*!<  A2B Downstream Slots Register */
#define A2B_REG_UPSLOTS                     (0x0000000Eu)             /*!<  A2B Upstream Slots Register */
#define A2B_REG_RESPCYCS                    (0x0000000Fu)             /*!<  A2B Response Cycles Register */
/* ============================================================================================================================ */
/** \name   A2B Config, Shadowed, Slave Only Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_BCDNSLOTS                   (0x0000000Au)             /*!<  A2B Broadcast Downstream Slots Register (Slave Only) */
#define A2B_REG_LDNSLOTS                    (0x0000000Bu)             /*!<  A2B Local Downstream Slots Register (Slave Only) */
#define A2B_REG_LUPSLOTS                    (0x0000000Cu)             /*!<  A2B Local Upstream Slots Register (Slave Only) */
/* ============================================================================================================================ */
/** \name   A2B Configuration Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_SWCTL                       (0x00000009u)             /*!<  A2B Switch Control Register */
/* ============================================================================================================================ */
/** \name   A2B Control Master Only Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_NODEADR                     (0x00000001u)             /*!<  A2B Node Address Register (Master only) */
/* ============================================================================================================================ */
/** \name   A2B Control, Master Only Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_CONTROL                     (0x00000012u)             /*!<  A2B Control Register (Master Only) */
/* ============================================================================================================================ */
/** \name   A2B Start Discovery, Master Only Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_DISCVRY                     (0x00000013u)             /*!<  A2B Discovery Register (Master Only) */
/* ============================================================================================================================ */
/** \name   A2B Status Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_NODE                        (0x00000029u)             /*!<  A2B Node Register */
#define A2B_REG_TRANSTAT                    (0x0000002Au)             /*!<  A2B Transfer Status Register (Master Only) - Undocumented */
#define A2B_REG_DISCSTAT                    (0x0000002Bu)             /*!<  A2B Discovery Status Register (Master Only) */
#define A2B_REG_NSCURCNT                    (0x0000002Cu)             /*!<  A2B New Structure Current Count Register - Undocumented */
/* ============================================================================================================================ */
/** \name   A2B Status Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_SWSTAT                      (0x00000014u)             /*!<  A2B Switch Status Register */
#define A2B_REG_BSDSTAT                     (0x0000005Eu)             /*!<  A2B Bus Self Discovery Status REgister */  
/* ============================================================================================================================ */
/** \name   E-Fuse Register Definitions */
/** \note   Private undocumented internal registers. Not to be modified by the user */
/* ============================================================================================================================ */
#define A2B_REG_EFUSEADDR                   (0x000000F0u)             /*!<  A2B EFuse Address Register */
#define A2B_REG_EFUSERDAT                   (0x000000F1u)             /*!<  A2B EFuse Read Data Register */
#define A2B_REG_EFUSEWDAT                   (0x000000F2u)             /*!<  A2B EFuse Write Data Register */
/* ============================================================================================================================ */
/** \name   I2C Control Slave Only Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_CHIP                        (0x00000000u)             /*!<  A2B I2C Chip Address Register (Slave Only) */
/* ============================================================================================================================ */
/** \name   I2C, I2S, and PDM Control and Configuration Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_I2CCFG                      (0x0000003Fu)             /*!<  A2B I2C Configuration Register */
#define A2B_REG_PLLCTL                      (0x00000040u)             /*!<  A2B PLL Control Register - Undocumented */
#define A2B_REG_I2SGCFG                     (0x00000041u)             /*!<  A2B I2S Global Configuration Register */
#define A2B_REG_I2SCFG                      (0x00000042u)             /*!<  A2B I2S Configuration Register */
#define A2B_REG_I2SRATE                     (0x00000043u)             /*!<  A2B I2S Rate Register (Slave Only) */
#define A2B_REG_I2STXOFFSET                 (0x00000044u)             /*!<  A2B I2S Transmit Data Offset Register */
#define A2B_REG_I2SRXOFFSET                 (0x00000045u)             /*!<  A2B I2S Receive Data Offset Register */
#define A2B_REG_SYNCOFFSET                  (0x00000046u)             /*!<  A2B SYNC Offset Register (Slave Only) */
#define A2B_REG_PDMCTL                      (0x00000047u)             /*!<  A2B PDM Control Register */
#define A2B_REG_ERRMGMT                     (0x00000048u)             /*!<  A2B Error Management Register */
#define A2B_REG_I2STEST                     (0x00000053u)             /*!<  A2B I2S Test Register */
#define A2B_REG_I2SRRATE                    (0x00000056u)             /*!<  A2B I2S Reduced Rate Register (Master only, Auto-Broadcast) - AD242X only */
#define A2B_REG_I2SRRCTL                    (0x00000057u)             /*!<  A2B I2S Reduced Rate Control Register - AD242X only */
#define A2B_REG_I2SRRSOFFS                  (0x00000058u)             /*!<  A2B I2S Reduced Rate SYNC Offset Register (Slave only) - AD242X only */

/* ============================================================================================================================ */
/** \name   ID Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_VENDOR                      (0x00000002u)             /*!<  A2B Vendor ID Register */
#define A2B_REG_PRODUCT                     (0x00000003u)             /*!<  A2B Product ID Register */
#define A2B_REG_VERSION                     (0x00000004u)             /*!<  A2B Version ID Register */
#define A2B_REG_CAPABILITY                  (0x00000005u)             /*!<  A2B Capability ID Register */
/* ============================================================================================================================ */
/** \name   Interrupt and Error Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_INTSTAT                     (0x00000015u)             /*!<  A2B Interrupt Status Register */
#define A2B_REG_INTSRC                      (0x00000016u)             /*!<  A2B Interrupt Source Register (Master Only) */
#define A2B_REG_INTTYPE                     (0x00000017u)             /*!<  A2B Interrupt Type Register (Master Only) */
#define A2B_REG_INTPND0                     (0x00000018u)             /*!<  A2B Interrupt Pending 0 Register */
#define A2B_REG_INTPND1                     (0x00000019u)             /*!<  A2B Interrupt Pending 1 Register */
#define A2B_REG_INTPND2                     (0x0000001Au)             /*!<  A2B Interrupt Pending 2 Register (Master Only) */
#define A2B_REG_INTMSK0                     (0x0000001Bu)             /*!<  A2B Interrupt Mask 0 Register */
#define A2B_REG_INTMSK1                     (0x0000001Cu)             /*!<  A2B Interrupt Mask 1 Register */
#define A2B_REG_INTMSK2                     (0x0000001Du)             /*!<  A2B Interrupt Mask 2 Register (Master Only) */
#define A2B_REG_BECCTL                      (0x0000001Eu)             /*!<  A2B Bit Error Count Control Register */
#define A2B_REG_BECNT                       (0x0000001Fu)             /*!<  A2B Bit Error Count Register */
#define A2B_REG_RAISE                       (0x00000054u)             /*!<  A2B Raise Interrupt Register */
#define A2B_REG_GENERR                      (0x00000055u)             /*!<  A2B Generate Bus Error */
/* ============================================================================================================================ */
/** \name   PRBS Test Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_TESTMODE                    (0x00000020u)             /*!<  A2B Testmode Register */
#define A2B_REG_ERRCNT0                     (0x00000021u)             /*!<  A2B PRBS Error Count Byte 0 Register */
#define A2B_REG_ERRCNT1                     (0x00000022u)             /*!<  A2B PRBS Error Count Byte 1 Register */
#define A2B_REG_ERRCNT2                     (0x00000023u)             /*!<  A2B PRBS Error Count Byte 2 Register */
#define A2B_REG_ERRCNT3                     (0x00000024u)             /*!<  A2B PRBS Error Count Byte 3 Register */
#define A2B_REG_SEED0                       (0x00000025u)             /*!<  A2B PRBS Seed Byte 0 Register - Undocumented */
#define A2B_REG_SEED1                       (0x00000026u)             /*!<  A2B PRBS Seed Byte 1 Register - Undocumented */
#define A2B_REG_SEED2                       (0x00000027u)             /*!<  A2B PRBS Seed Byte 2 Register - Undocumented */
#define A2B_REG_SEED3                       (0x00000028u)             /*!<  A2B PRBS Seed Byte 3 Register - Undocumented */
/* ============================================================================================================================ */
/** \name   Pin IO and Interrupt Register Definitions */
/* ============================================================================================================================ */
#define A2B_REG_CLKCFG                      (0x00000049u)             /*!<  A2B Clock Config Register - Undocumented */
#define A2B_REG_GPIODAT                     (0x0000004Au)             /*!<  A2B GPIO Output Data Register */
#define A2B_REG_GPIODATSET                  (0x0000004Bu)             /*!<  A2B GPIO Output Data Set Register */
#define A2B_REG_GPIODATCLR                  (0x0000004Cu)             /*!<  A2B GPIO Output Data Clear Register */
#define A2B_REG_GPIOOEN                     (0x0000004Du)             /*!<  A2B GPIO Output Enable Register */
#define A2B_REG_GPIOIEN                     (0x0000004Eu)             /*!<  A2B GPIO Input Enable Register */
#define A2B_REG_GPIOIN                      (0x0000004Fu)             /*!<  A2B GPIO Input Value Register */
#define A2B_REG_PINTEN                      (0x00000050u)             /*!<  A2B Pin Interrupt Enable Register */
#define A2B_REG_PINTINV                     (0x00000051u)             /*!<  A2B Pin Interrupt Invert Register */
#define A2B_REG_CLK1CFG                     (0x00000059u)             /*!<  A2B CLKOUT1 Configuration Register - AD242X only */
#define A2B_REG_CLK2CFG                     (0x0000005Au)             /*!<  A2B CLKOUT2 Configuration Register - AD242X only */
/* ============================================================================================================================ */
/** \name   Bus Monitor Register Definitions - AD242X only */
/* ============================================================================================================================ */
#define A2B_REG_BMMCFG                      (0x0000005Bu)             /*!<  A2B Bus Monitor Mode Configuration Register - AD242X only */
/* ============================================================================================================================ */
/** \name   Sustain Register Definitions - AD242X only */
/* ============================================================================================================================ */
#define A2B_REG_SUSCFG                      (0x0000005Cu)             /*!<  A2B Sustain Configuration Register (Slave only) - AD242X only */
/* ============================================================================================================================ */
/** \name   Slave-to-Slave Routing Register Definitions - AD242X only */
/* ============================================================================================================================ */
#define A2B_REG_UPMASK0                     (0x00000060u)            /*!<  A2B Upstream Data RX Mask 0 (Slave only) - AD242X only */
#define A2B_REG_UPMASK1                     (0x00000061u)            /*!<  A2B Upstream Data RX Mask 1 (Slave only) - AD242X only */
#define A2B_REG_UPMASK2                     (0x00000062u)            /*!<  A2B Upstream Data RX Mask 2 (Slave only) - AD242X only */
#define A2B_REG_UPMASK3                     (0x00000063u)            /*!<  A2B Upstream Data RX Mask 3 (Slave only) - AD242X only */
#define A2B_REG_UPOFFSET                    (0x00000064u)            /*!<  A2B Local Upstream Slots Offset Register (Slave only) - AD242X only */
#define A2B_REG_DNMASK0                     (0x00000065u)            /*!<  A2B Downstream Data RX Mask 0 (Slave only) - AD242X only */
#define A2B_REG_DNMASK1                     (0x00000066u)            /*!<  A2B Downstream Data RX Mask 1 (Slave only) - AD242X only */
#define A2B_REG_DNMASK2                     (0x00000067u)            /*!<  A2B Downstream Data RX Mask 2 (Slave only) - AD242X only */
#define A2B_REG_DNMASK3                     (0x00000068u)            /*!<  A2B Downstream Data RX Mask 3 (Slave only) - AD242X only */
#define A2B_REG_DNOFFSET                    (0x00000069u)            /*!<  A2B Local Downstream Slots Offset Register (Slave only) - AD242X only */
/* ============================================================================================================================ */
/** \name   GPIO Over Distance Register Definitions - AD242X only */
/* ============================================================================================================================ */
#define A2B_REG_GPIODEN                     (0x00000080u)            /*!<  A2B GPIO Over Distance Enable - AD242X only */
#define A2B_REG_GPIOD0MSK                   (0x00000081u)            /*!<  A2B GPIO Over Distance Mask 0 Register - AD242X only */
#define A2B_REG_GPIOD1MSK                   (0x00000082u)            /*!<  A2B GPIO Over Distance Mask 1 Register - AD242X only */
#define A2B_REG_GPIOD2MSK                   (0x00000083u)            /*!<  A2B GPIO Over Distance Mask 2 Register - AD242X only */
#define A2B_REG_GPIOD3MSK                   (0x00000084u)            /*!<  A2B GPIO Over Distance Mask 3 Register - AD242X only */
#define A2B_REG_GPIOD4MSK                   (0x00000085u)            /*!<  A2B GPIO Over Distance Mask 4 Register - AD242X only */
#define A2B_REG_GPIOD5MSK                   (0x00000086u)            /*!<  A2B GPIO Over Distance Mask 5 Register - AD242X only */
#define A2B_REG_GPIOD6MSK                   (0x00000087u)            /*!<  A2B GPIO Over Distance Mask 6 Register - AD242X only */
#define A2B_REG_GPIOD7MSK                   (0x00000088u)            /*!<  A2B GPIO Over Distance Mask 7 Register - AD242X only */
#define A2B_REG_GPIODDAT                    (0x00000089u)            /*!<  A2B GPIO Over Distance Data Register - AD242X only */
#define A2B_REG_GPIODINV                    (0x0000008Au)            /*!<  A2B GPIO Over Distance Invert Register - AD242X only */
/* ============================================================================================================================ */
/** \name   Mailbox Register Definitions (Slave only) - AD242X only */
/* ============================================================================================================================ */
#define A2B_REG_MBOX0CTL                    (0x00000090u)            /*!<  A2B Mailbox 0 Control Register (Slave only) - AD242X only */
#define A2B_REG_MBOX0STAT                   (0x00000091u)            /*!<  A2B Mailbox 0 Status Register (Slave only) - AD242X only */
#define A2B_REG_MBOX0B0                     (0x00000092u)            /*!<  A2B Mailbox 0 Byte 0 Register (Slave only) - AD242X only */
#define A2B_REG_MBOX0B1                     (0x00000093u)            /*!<  A2B Mailbox 0 Byte 1 Register (Slave only) - AD242X only */
#define A2B_REG_MBOX0B2                     (0x00000094u)            /*!<  A2B Mailbox 0 Byte 2 Register (Slave only) - AD242X only */
#define A2B_REG_MBOX0B3                     (0x00000095u)            /*!<  A2B Mailbox 0 Byte 3 Register (Slave only) - AD242X only */
#define A2B_REG_MBOX1CTL                    (0x00000096u)            /*!<  A2B Mailbox 1 Control Register (Slave only) - AD242X only */
#define A2B_REG_MBOX1STAT                   (0x00000097u)            /*!<  A2B Mailbox 1 Status Register (Slave only) - AD242X only */
#define A2B_REG_MBOX1B0                     (0x00000098u)            /*!<  A2B Mailbox 1 Byte 0 Register (Slave only) - AD242X only */
#define A2B_REG_MBOX1B1                     (0x00000099u)            /*!<  A2B Mailbox 1 Byte 1 Register (Slave only) - AD242X only */
#define A2B_REG_MBOX1B2                     (0x0000009Au)            /*!<  A2B Mailbox 1 Byte 2 Register (Slave only) - AD242X only */
#define A2B_REG_MBOX1B3                     (0x0000009Bu)            /*!<  A2B Mailbox 1 Byte 3 Register (Slave only) - AD242X only */

/* ============================================================================================================================ */
/** \name   Shadow Register Committed Copy Read Access Register Definitions */
/** \note   Private undocumented internal registers. Not to be modified by the user */
/* ============================================================================================================================ */
#define A2B_REG_RESPCCC                     (0x00000032u)             /*!<  A2B Response Cycles Committed Copy Register */
#define A2B_REG_DCTLCC                      (0x00000033u)             /*!<  A2B Data Control Committed Copy Register */
#define A2B_REG_DNSCC                       (0x00000034u)             /*!<  A2B Downstream Slots Committed Copy Register */
#define A2B_REG_LDNSCC                      (0x00000035u)             /*!<  A2B Local Downstream Slots Committed Copy Register (Slave Only) */
#define A2B_REG_UPSCC                       (0x00000036u)             /*!<  A2B Upstream Slots Committed Copy Register */
#define A2B_REG_LUPSCC                      (0x00000037u)             /*!<  A2B Local Upstream Slots Committed Copy Register (Slave Only) */
#define A2B_REG_BCDNSCC                     (0x00000038u)             /*!<  A2B Broadcast Downstream Slots Committed Copy Register (Slave Only) */
#define A2B_REG_SFMTCC                      (0x00000039u)             /*!<  A2B Slot Format Committed Copy Register */
/* ============================================================================================================================ */
/** \name   Chip test Register Definitions */
/** \note   Private undocumented internal registers. Not to be modified by the user */
/* ============================================================================================================================ */
#define A2B_REG_PTSTMODE                    (0x0000003Au)             /*!<  A2B Private Testmode Register */
#define A2B_REG_STRAPVAL                    (0x0000003Bu)             /*!<  A2B Strap Values Register */


#define A2B_REG_LINTTYPE                     (0x0000003Eu)            /*!<  Local Interrupt Type Slave Only */
/** \} -- needed for last name */

/** \} -- a2bstack_regdefs_offsets */

/*----------------------------------------------------------------------------*/
/** 
 * \defgroup a2bstack_regdefs_masks         Register Masks & Positions
 *  
 * A2B Field BitMasks, Positions & Enumerations
 *
 * \{ */
/*----------------------------------------------------------------------------*/

/* ============================================================================================================================ */
/** \name   A2B Register Pin Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PINCFG_IRQTS                (5u)           /*!< Tristate IRQ - AD242X Only */
#define A2B_BITP_PINCFG_IRQINV               (4u)           /*!< Invert IRQ - AD242X Only */
#define A2B_BITP_PINCFG_I2CDRVSTR            (3u)           /*!< I2C Pins Drive Strength -AD232X only*/
#define A2B_BITP_PINCFG_TXBLP                (2u)           /*!<  LVDS XCVRB Low Power TX Mode */
#define A2B_BITP_PINCFG_TXALP                (1u)           /*!<  LVDS XCVRA Low Power TX Mode */
#define A2B_BITP_PINCFG_DRVSTR               (0u)           /*!<  Digital Pin Drive Strength */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_PINCFG_IRQTS                (0x00000020u)  /*!< AD242X Only */
#define A2B_BITM_PINCFG_IRQINV               (0x00000010u)  /*!< AD242X Only */
#define A2B_BITM_PINCFG_I2CDRVSTR            (0x00000008u)  /*!< AD232X Only */
#define A2B_BITM_PINCFG_TXBLP                (0x00000004u)
#define A2B_BITM_PINCFG_TXALP                (0x00000002u)
#define A2B_BITM_PINCFG_DRVSTR               (0x00000001u)

#define A2B_REG_PINCFG_RESET                 (0x00000001u)  /*!<  Reset Value for PINCFG */
/* ============================================================================================================================ */
/** \name   A2B Analog Tuning Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        VREGCTL                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VREGCTL_VSSEL               (0u)           /*!<  Select Vsense */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        VREGCTL                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_VREGCTL_VSSEL               (0x00000001u)

#define A2B_REG_VREGCTL_RESET                (0x00000000u)  /*!<  Reset Value for VREGCTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TXACTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXACTL_TXAOVREN             (7u)           /*!<  TXA Control Override Enable */
#define A2B_BITP_TXACTL_TXASLEW              (4u)           /*!<  TXA Slew */
#define A2B_BITP_TXACTL_TXAPE                (2u)           /*!<  TXA Emphasis */
#define A2B_BITP_TXACTL_TXALEVEL             (0u)           /*!<  TXA Level */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TXACTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_TXACTL_TXAOVREN             (0x00000080u)
#define A2B_BITM_TXACTL_TXASLEW              (0x00000030u)
#define A2B_BITM_TXACTL_TXAPE                (0x0000000Cu)
#define A2B_BITM_TXACTL_TXALEVEL             (0x00000003u)

#define A2B_REG_TXACTL_RESET                 (0x00000000u)  /*!<  Reset Value for TXACTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        RXACTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXACTL_RXAOVREN             (7u)           /*!<  RXA Control Override Enable */
#define A2B_BITP_RXACTL_RXAEQ                (6u)           /*!<  RXA Equalization Control */
#define A2B_BITP_RXACTL_RXATYPE              (5u)           /*!<  RXA Receiver Type */
#define A2B_BITP_RXACTL_RXASQEN              (4u)           /*!<  RXA Squelch Enable */
#define A2B_BITP_RXACTL_RXASQUELCH           (2u)           /*!<  RXA Squelch Control */
#define A2B_BITP_RXACTL_RXAHYS               (0u)           /*!<  RXA Hysteresis Control */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        RXACTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_RXACTL_RXAOVREN             (0x00000080u)
#define A2B_BITM_RXACTL_RXAEQ                (0x00000040u)
#define A2B_BITM_RXACTL_RXATYPE              (0x00000020u)
#define A2B_BITM_RXACTL_RXASQEN              (0x00000010u)
#define A2B_BITM_RXACTL_RXASQUELCH           (0x0000000Cu)
#define A2B_BITM_RXACTL_RXAHYS               (0x00000003u)

#define A2B_REG_RXACTL_RESET                 (0x00000000u)  /*!<  Reset Value for RXACTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TXBCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXBCTL_TXBOVREN             (7u)           /*!<  TXB Control Override Enable */
#define A2B_BITP_TXBCTL_TXBSLEW              (4u)           /*!<  TXB Slew */
#define A2B_BITP_TXBCTL_TXBPE                (2u)           /*!<  TXB Emphasis */
#define A2B_BITP_TXBCTL_TXBLEVEL             (0u)           /*!<  TXB Level */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TXBCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_TXBCTL_TXBOVREN             (0x00000080u)
#define A2B_BITM_TXBCTL_TXBSLEW              (0x00000030u)
#define A2B_BITM_TXBCTL_TXBPE                (0x0000000Cu)
#define A2B_BITM_TXBCTL_TXBLEVEL             (0x00000003u)

#define A2B_REG_TXBCTL_RESET                 (0x00000000u)  /*!<  Reset Value for TXBCTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        RXBCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXBCTL_RXBOVREN             (7u)           /*!<  RXB Control Override Enable */
#define A2B_BITP_RXBCTL_RXBEQ                (6u)           /*!<  RXB Equalization Control */
#define A2B_BITP_RXBCTL_RXBTYPE              (5u)           /*!<  RXB Receiver Type */
#define A2B_BITP_RXBCTL_RXBSQEN              (4u)           /*!<  RXB Squelch Enable */
#define A2B_BITP_RXBCTL_RXBSQUELCH           (2u)           /*!<  RXB Squelch Control */
#define A2B_BITP_RXBCTL_RXBHYS               (0u)           /*!<  RXB Hysteresis Control */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        RXBCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_RXBCTL_RXBOVREN             (0x00000080u)
#define A2B_BITM_RXBCTL_RXBEQ                (0x00000040u)
#define A2B_BITM_RXBCTL_RXBTYPE              (0x00000020u)
#define A2B_BITM_RXBCTL_RXBSQEN              (0x00000010u)
#define A2B_BITM_RXBCTL_RXBSQUELCH           (0x0000000Cu)
#define A2B_BITM_RXBCTL_RXBHYS               (0x00000003u)

#define A2B_REG_RXBCTL_RESET                 (0x00000000u)  /*!<  Reset Value for RXBCTL */
/* ============================================================================================================================ */
/** \name   A2B Config, Master Only, Auto-Broadcast, Shadowed Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SLOTFMT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SLOTFMT_UPFP                (7u)           /*!<  Upstream Floating Point */
#define A2B_BITP_SLOTFMT_UPFMT               (7u)           /*!<  Upstream Floating Point - AD242X naming */
#define A2B_BITP_SLOTFMT_UPSIZE              (4u)           /*!<  Upstream Slot Size */
#define A2B_BITP_SLOTFMT_DNFP                (3u)           /*!<  Downstream Floating-Point */
#define A2B_BITP_SLOTFMT_DNSIZE              (0u)           /*!<  Downstream Slot Size */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SLOTFMT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_SLOTFMT_UPFP                (0x00000080u)
#define A2B_BITM_SLOTFMT_UPFMT               (0x00000080u)
#define A2B_BITM_SLOTFMT_UPSIZE              (0x00000070u)
#define A2B_BITM_SLOTFMT_DNFP                (0x00000008u)
#define A2B_BITM_SLOTFMT_DNSIZE              (0x00000007u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SLOTFMT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_SLOTFMT_UPFP_EN             (0x00000080u)  /*!<  Enabled */
#define A2B_ENUM_SLOTFMT_UPFP_DIS            (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_SLOTFMT_UPFMT_EN            (0x00000080u)  /*!<  Enabled */
#define A2B_ENUM_SLOTFMT_UPFMT_DIS           (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_SLOTFMT_UPSIZE_32           (0x00000060u)  /*!<  32 Bits */
#define A2B_ENUM_SLOTFMT_UPSIZE_24           (0x00000040u)  /*!<  24 Bits */
#define A2B_ENUM_SLOTFMT_UPSIZE_12           (0x00000010u)  /*!<  12 Bits */
#define A2B_ENUM_SLOTFMT_UPSIZE_20           (0x00000030u)  /*!<  20 Bits */
#define A2B_ENUM_SLOTFMT_UPSIZE_8            (0x00000000u)  /*!<  8 Bits */
#define A2B_ENUM_SLOTFMT_UPSIZE_16           (0x00000020u)  /*!<  16 Bits */
#define A2B_ENUM_SLOTFMT_UPSIZE_28           (0x00000050u)  /*!<  28 Bits */
#define A2B_ENUM_SLOTFMT_DNFP_EN             (0x00000008u)  /*!<  Enabled */
#define A2B_ENUM_SLOTFMT_DNFP_DIS            (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_SLOTFMT_DNSIZE_32           (0x00000006u)  /*!<  32 Bits */
#define A2B_ENUM_SLOTFMT_DNSIZE_24           (0x00000004u)  /*!<  24 Bits */
#define A2B_ENUM_SLOTFMT_DNSIZE_12           (0x00000001u)  /*!<  12 Bits */
#define A2B_ENUM_SLOTFMT_DNSIZE_20           (0x00000003u)  /*!<  20 Bits */
#define A2B_ENUM_SLOTFMT_DNSIZE_8            (0x00000000u)  /*!<  8 Bits */
#define A2B_ENUM_SLOTFMT_DNSIZE_16           (0x00000002u)  /*!<  16 Bits */
#define A2B_ENUM_SLOTFMT_DNSIZE_28           (0x00000005u)  /*!<  28 Bits */

#define A2B_REG_SLOTFMT_RESET                (0x00000000u)  /*!<  Reset Value for SLOTFMT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DATCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DATCTL_STANDBY              (7u)           /*!<  Enable Standby Mode */
#define A2B_BITP_DATCTL_SCRDIS               (6u)           /*!<  Disable Scrambler */
#define A2B_BITP_DATCTL_ENDSNIFF             (5u)           /*!<  Enable Data Output on Bus Monitor Mode - AD242X only */
#define A2B_BITP_DATCTL_UPS                  (1u)           /*!<  Enable Upstream Slots */
#define A2B_BITP_DATCTL_DNS                  (0u)           /*!<  Enable Downstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DATCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DATCTL_STANDBY              (0x00000080u)
#define A2B_BITM_DATCTL_SCRDIS               (0x00000040u)
#define A2B_BITM_DATCTL_ENDSNIFF             (0x00000020u)
#define A2B_BITM_DATCTL_UPS                  (0x00000002u)
#define A2B_BITM_DATCTL_DNS                  (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DATCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_DATCTL_STANDBY_EN           (0x00000080u)  /*!<  Enabled */
#define A2B_ENUM_DATCTL_STANDBY_DIS          (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_DATCTL_SCRDIS_ON            (0x00000040u)  /*!<  Scrambling Disabled */
#define A2B_ENUM_DATCTL_SCRDIS_OFF           (0x00000000u)  /*!<  Scrambling Enabled */
#define A2B_ENUM_DATCTL_ENDSNIFF_ON          (0x00000020u)  /*!<  Enable Data Output on Attached Bus Monitor Node */
#define A2B_ENUM_DATCTL_ENDSNIFF_OFF         (0x00000000u)  /*!<  Disable Data Output on Attached Bus Monitor Node */
#define A2B_ENUM_DATCTL_UPS_EN               (0x00000002u)  /*!<  Enabled */
#define A2B_ENUM_DATCTL_UPS_DIS              (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_DATCTL_DNS_EN               (0x00000001u)  /*!<  Enabled */
#define A2B_ENUM_DATCTL_DNS_DIS              (0x00000000u)  /*!<  Disabled */

#define A2B_REG_DATCTL_RESET                 (0x00000000u)  /*!<  Reset Value for DATCTL */
/* ============================================================================================================================ */
/** \name   A2B Config, Shadowed Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNSLOTS                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DNSLOTS_DNSLOTS             (0u)           /*!<  Number of Downstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNSLOTS                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DNSLOTS_DNSLOTS             (0x0000003Fu)

#define A2B_REG_DNSLOTS_RESET                (0x00000000u)  /*!<  Reset Value for DNSLOTS */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPSLOTS                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_UPSLOTS_UPSLOTS             (0)           /*!<  Number of Upstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPSLOTS                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_UPSLOTS_UPSLOTS             (0x0000003F)

#define A2B_REG_UPSLOTS_RESET                (0x00000000u)  /*!<  Reset Value for UPSLOTS */
#define A2B_REG_RESPCYCS_RESET               (0x00000040u)  /*!<  Reset Value for RESPCYCS */
/* ============================================================================================================================ */
/** \name   A2B Config, Shadowed, Slave Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BCDNSLOTS                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_BCDNSLOTS_BCDNSLOTS         (0u)           /*!<  Broadcast Downstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BCDNSLOTS                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_BCDNSLOTS_BCDNSLOTS         (0x0000003Fu)

#define A2B_REG_BCDNSLOTS_RESET              (0x00000000u)  /*!<  Reset Value for BCDNSLOTS */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LDNSLOTS                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_LDNSLOTS_LDNSLOTS           (0u)           /*!<  Number of Downstream Slots Targeted at Local Node */
#define A2B_BITP_LDNSLOTS_DNMASKEN           (7u)           /*!<  Downstream Broadcast Mask Enable - AD242X only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LDNSLOTS                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_LDNSLOTS_LDNSLOTS           (0x0000003Fu)
#define A2B_BITM_LDNSLOTS_DNMASKEN           (0x00000080u)  /*!< AD242X only */

#define A2B_REG_LDNSLOTS_RESET               (0x00000000u)  /*!<  Reset Value for LDNSLOTS */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LUPSLOTS                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_LUPSLOTS_LUPSLOTS           (0u)           /*!<  Number of Upstream Slots Generated By Local Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LUPSLOTS                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_LUPSLOTS_LUPSLOTS           (0x0000003Fu)

#define A2B_REG_LUPSLOTS_RESET               (0x00000000u)  /*!<  Reset Value for LUPSLOTS */
/* ============================================================================================================================ */
/** \name   A2B Configuration Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWCTL                                Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SWCTL_NAUTODISC             (7u)           /*!<  No Auto Disconnect - Undocumented */
#define A2B_BITP_SWCTL_DISNXT                (6u)           /*!<  Disable Next - Undocumented */
#define A2B_BITP_SWCTL_MODE                  (4u)           /*!<  External Switch Mode */
#define A2B_BITP_SWCTL_DIAGMODE              (3u)           /*!<  Enable Switch Diagnosis Mode */
#define A2B_BITP_SWCTL_ORT                   (1u)           /*!<  Open Reverse Timer - Undocumented*/
#define A2B_BITP_SWCTL_DET_OV                (1u)           /*!<  For AD243x , Override */
#define A2B_BITP_SWCTL_ENSW                  (0u)           /*!<  Enable Switch */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWCTL                                Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_SWCTL_NAUTODISC             (0x00000080u)
#define A2B_BITM_SWCTL_DISNXT                (0x00000040u)
#define A2B_BITM_SWCTL_MODE                  (0x00000030u)
#define A2B_BITM_SWCTL_DIAGMODE              (0x00000008u)
#define A2B_BITM_SWCTL_ORT                   (0x00000006u)
#define A2B_BITM_SWCTL_CFG_DET_OV            (0x00000002u)
#define A2B_BITM_SWCTL_ENSW                  (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWCTL                                Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_SWCTL_MODE_INT_VSSN_EXT_SWP (0x00000000u)  /*!<  Use internal switch for VSSN, extern for SWP pin */
#define A2B_ENUM_SWCTL_MODE_NO_DWN_PHAN_PWR  (0x00000010u)  /*!<  Dwnstr node not using phantom pwr, not prop term bias */
#define A2B_ENUM_SWCTL_MODE_VOLT_ON_VIN      (0x00000020u)  /*!<  Voltage on the VIN pin */
#define A2B_ENUM_SWCTL_MODE_RESERVED         (0x00000030u)  /*!<  Reserved */
#define A2B_ENUM_SWCTL_DIAGMODE_EN           (0x00000008u)  /*!<  Switch Diagnosis Mode Enabled */
#define A2B_ENUM_SWCTL_DIAGMODE_DIS          (0x00000000u)  /*!<  Switch Diagnosis Mode Disabled */
#define A2B_ENUM_SWCTL_ENSW_EN               (0x00000001u)  /*!<  Switches Enabled */
#define A2B_ENUM_SWCTL_ENSW_DIS              (0x00000000u)  /*!<  Switches Disabled */

#define A2B_REG_SWCTL_RESET                  (0x00000000u)  /*!<  Reset Value for SWCTL */
/* ============================================================================================================================ */
/** \name   A2B Control Master Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODEADR                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_NODEADR_BRCST               (7u)           /*!<  Broadcast */
#define A2B_BITP_NODEADR_PERI                (5u)           /*!<  Enable Peripheral */
#define A2B_BITP_NODEADR_NODE                (0u)           /*!<  Addressed Slave Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODEADR                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_NODEADR_BRCST               (0x00000080u)
#define A2B_BITM_NODEADR_PERI                (0x00000020u)
#define A2B_BITM_NODEADR_NODE                (0x0000000Fu)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODEADR                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_NODEADR_BRCST_EN            (0x00000080u)  /*!<  Next write to slave nodes is handled as broadcast access */
#define A2B_ENUM_NODEADR_BRCST_DIS           (0x00000000u)  /*!<  Normal, directed register access */
#define A2B_ENUM_NODEADR_PERI_EN             (0x00000020u)  /*!<  Remote Peripheral Access enabled */
#define A2B_ENUM_NODEADR_PERI_DIS            (0x00000000u)  /*!<  Remote Peripheral Access disabled */

#define A2B_REG_NODEADR_RESET                (0x00000000u)  /*!<  Reset Value for NODEADR */
/* ============================================================================================================================ */
/** \name   A2B Control, Master Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CONTROL                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CONTROL_MSTR                (7u)           /*!<  Master Node - AD242X only */
#define A2B_BITP_CONTROL_I2SMSINV            (5u)           /*!<  I2S Master/Slave Invert - AD243x only */
#define A2B_BITP_CONTROL_XCVRBINV            (4u)           /*!<  LVDS inverse - AD2428 only */
#define A2B_BITP_CONTROL_SWBYP				 (3u)           /*!<  LVDS inverse - AD2428 only */
#define A2B_BITP_CONTROL_SOFTRST             (2u)           /*!<  Soft Reset of Protocol Engine */
#define A2B_BITP_CONTROL_ENDDSC              (1u)           /*!<  End Discovery Mode */
#define A2B_BITP_CONTROL_NEWSTRCT            (0u)           /*!<  New Structure */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CONTROL                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_CONTROL_MSTR                (0x00000080u)  /*!<  AD242X only */
#define A2B_BITM_CONTROL_I2SMSINV            (0x00000020u)  /*!<  I2S Master/Slave Invert - AD243x only */
#define A2B_BITM_CONTROL_SOFTRST             (0x00000004u)
#define A2B_BITM_CONTROL_XCVRBINV            (0x00000010u)
#define A2B_BITM_CONTROL_SWBYP				 (0x00000008u)
#define A2B_BITM_CONTROL_ENDDSC              (0x00000002u)
#define A2B_BITM_CONTROL_NEWSTRCT            (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CONTROL                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_CONTROL_MSTR                (0x00000080u)  /*!<  Master node - AD242X only */
#define A2B_ENUM_CONTROL_MSTR_NO_ACTION      (0x00000000u)  /*!<  Slave node */
#define A2B_ENUM_CONTROL_I2SMSINV            (0x00000020u)  /*!<  I2S Master/Slave Invert - AD243x only */
#define A2B_ENUM_CONTROL_I2SMSINV_NO_ACTION  (0x00000000u)  /*!<  No action */
#define A2B_ENUM_CONTROL_XCVRBINV            (0x00000010u)  /*!<  LVDS inverse - AD2428 only */
#define A2B_ENUM_CONTROL_XCVRBINV_NO_ACTION  (0x00000000u)  /*!<  No action */
#define A2B_ENUM_CONTROL_SWBYP				 (0x00000008u)  /*!<  SWBYP - AD2428 only */
#define A2B_ENUM_CONTROL_SWBYP_NO_ACTION	 (0x00000000u)  /*!<  No action */
#define A2B_ENUM_CONTROL_RESET_PE            (0x00000004u)  /*!<  Reset Protocol Engine */
#define A2B_ENUM_CONTROL_SOFTRST_NO_ACTION   (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_CONTROL_END_DISCOVERY       (0x00000002u)  /*!<  End Discovery */
#define A2B_ENUM_CONTROL_ENDDSC_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_CONTROL_START_NS            (0x00000001u)  /*!<  Enable New Structure */
#define A2B_ENUM_CONTROL_NEWSTRCT_NO_ACTION  (0x00000000u)  /*!<  No Action */

#define A2B_REG_CONTROL_RESET                (0x00000000u)  /*!<  Reset Value for CONTROL */
/* ============================================================================================================================ */
/** \name   A2B Start Discovery, Master Only Register Field Definitions */
/* ============================================================================================================================ */
#define A2B_REG_DISCVRY_RESET                (0x00000000u)             /*!<      Reset Value for DISCVRY */
/* ============================================================================================================================ */
/** \name   A2B Status Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODE                                 Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_NODE_LAST                   (7u)           /*!<  Last Node */
#define A2B_BITP_NODE_NLAST                  (6u)           /*!<  Next-to-Last Node */
#define A2B_BITP_NODE_DISCVD                 (5u)           /*!<  Node Discovered */
#define A2B_BITP_NODE_NUMBER                 (0u)           /*!<  Number Currently Assigned to Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODE                                 Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_NODE_LAST                   (0x00000080u)
#define A2B_BITM_NODE_NLAST                  (0x00000040u)
#define A2B_BITM_NODE_DISCVD                 (0x00000020u)
#define A2B_BITM_NODE_NUMBER                 (0x0000000Fu)

#define A2B_REG_NODE_RESET_AD241x            (0x00000080u)  /*!<  Reset Value for AD241x NODE */
#define A2B_REG_NODE_RESET_AD242X            (0x00000000u)  /*!<  Reset Value for AD242X NODE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TRANSTAT                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TRANSTAT_ECCERR             (6u)           /*!<  ECC Error Code */
#define A2B_BITP_TRANSTAT_INNS               (3u)           /*!<  In New Structure Mode */
#define A2B_BITP_TRANSTAT_INDISC             (2u)           /*!<  In Discovery Mode */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TRANSTAT                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_TRANSTAT_ECCERR             (0x000000C0u)
#define A2B_BITM_TRANSTAT_INNS               (0x00000008u)
#define A2B_BITM_TRANSTAT_INDISC             (0x00000004u)

#define A2B_REG_TRANSTAT_RESET               (0x00000000u)  /*!<  Reset Value for TRANSTAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DISCSTAT                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DISCSTAT_DSCACT             (7u)           /*!<  Discovery Active */
#define A2B_BITP_DISCSTAT_DNODE              (0u)           /*!<  Discovery Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DISCSTAT                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DISCSTAT_DSCACT             (0x00000080u)
#define A2B_BITM_DISCSTAT_DNODE              (0x0000000Fu)

#define A2B_REG_DISCSTAT_RESET               (0x00000000u)  /*!<  Reset Value for DISCSTAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LINTTYPE                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_LINTTYPE_LINTTYPE           (0u)           /*!<  Local Interrupt Type - AD242X only*/
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LINTTYPE                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_LINTTYPE_LINTTYPE           (0x000000FFu)

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LINTTYPE                           Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_LINTTYPE_MBOX0_FULL         (48u)           /*!<  Mailbox 0 Full */
#define A2B_ENUM_LINTTYPE_MBOX0_EMPTY        (49u)           /*!<  Mailbox 0 Empty */
#define A2B_ENUM_LINTTYPE_MBOX1_FULL         (50u)           /*!<  Mailbox 1 Full */
#define A2B_ENUM_LINTTYPE_MBOX1_EMPTY        (51u)           /*!<  Mailbox 2 Empty */

#define A2B_REG_LINTTYPE_RESET               (0x00000000u)  /*!<  Reset Value for LINTTYPE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NSCURCNT                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_NSCURCNT_CURCNT             (0u)           /*!<  Current Count */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NSCURCNT                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_NSCURCNT_CURCNT             (0x00000007u)

#define A2B_REG_NSCURCNT_RESET               (0x00000000u)  /*!<  Reset Value for NSCURCNT */
/* ============================================================================================================================ */
/** \name   A2B Status Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWSTAT                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SWSTAT_FAULT_NLOC           (7u)           /*!<  Switch Fault Not Localized */
#define A2B_BITP_SWSTAT_FAULT_CODE           (4u)           /*!<  Switch Fault Code */
#define A2B_BITP_SWSTAT_FAULT                (1u)           /*!<  Switch Fault */
#define A2B_BITP_SWSTAT_FIN                  (0u)           /*!<  Switch Activation Complete */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWSTAT                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_SWSTAT_FAULT_NLOC           (0x00000080u)
#define A2B_BITM_SWSTAT_FAULT_CODE           (0x00000070u)
#define A2B_BITM_SWSTAT_FAULT                (0x00000002u)
#define A2B_BITM_SWSTAT_FIN                  (0x00000001u)

#define A2B_REG_SWSTAT_RESET                 (0x00000000u)  /*!<  Reset Value for SWSTAT */
/* ============================================================================================================================ */
/** \name   E-Fuse Register Field Definitions */
/* ============================================================================================================================ */
#define A2B_REG_EFUSEADDR_RESET              (0x00000000u)  /*!<  Reset Value for EFUSEADDR */
#define A2B_REG_EFUSERDAT_RESET              (0x00000000u)  /*!<  Reset Value for EFUSERDAT */
#define A2B_REG_EFUSEWDAT_RESET              (0x00000000u)  /*!<  Reset Value for EFUSEWDAT */
/* ============================================================================================================================ */
/** \name   I2C Control Slave Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CHIP                                 Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CHIP_CHIPADR                (0u)           /*!<  I2C Chip Address */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CHIP                                 Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_CHIP_CHIPADR                (0x0000007Fu)

#define A2B_REG_CHIP_RESET                   (0x00000050u)  /*!<  Reset Value for CHIP */
/* ============================================================================================================================ */
/** \name   I2C, I2S, and PDM Control and Configuration Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2CCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2CCFG_FRAMERATE            (2u)           /*!<  Audio Frame Rate (A2B Slave Only) */
#define A2B_BITP_I2CCFG_EACK                 (1u)           /*!<  Early Acknowledge (A2B Master Only) */
#define A2B_BITP_I2CCFG_DATARATE             (0u)           /*!<  I2C Data Rate (A2B Slave Only) */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2CCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2CCFG_FRAMERATE            (0x00000004u)
#define A2B_BITM_I2CCFG_EACK                 (0x00000002u)
#define A2B_BITM_I2CCFG_DATARATE             (0x00000001u)

#define A2B_REG_I2CCFG_RESET                 (0x00000000u)  /*!<  Reset Value for I2CCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PLLCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PLLCTL_BCLKSEL              (2u)           /*!<  BCLK Bypass to PLL - AD241x only */
#define A2B_BITP_PLLCTL_BCLKDIV              (0u)           /*!<  BCLK PLL Multiplication Select - AD241x only */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PLLCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_PLLCTL_BCLKSEL              (0x00000004u)
#define A2B_BITM_PLLCTL_BCLKDIV              (0x00000003u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PLLCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_PLLCTL_BCLKSEL_EN           (0x00000004u)  /*!<  BCLK Selected */
#define A2B_ENUM_PLLCTL_BCLKSEL_DIS          (0x00000000u)  /*!<  SYNC Selected */
#define A2B_ENUM_PLLCTL_BCLKDIV_4X           (0x00000001u)  /*!<  2x (BCLK=24.576 MHz in TDM16) */
#define A2B_ENUM_PLLCTL_BCLKDIV_8X           (0x00000000u)  /*!<  1x (BCLK=12.288 MHz in TDM8) */
#define A2B_ENUM_PLLCTL_BCLKDIV_2X           (0x00000002u)  /*!<  4x (BCLK=49.152 MHz for TDM32) */

#define A2B_REG_PLLCTL_RESET                 (0x00000000u)  /*!<  Reset Value for PLLCTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SGCFG                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SGCFG_INV                 (7u)           /*!<  Invert Sync */
#define A2B_BITP_I2SGCFG_EARLY               (6u)           /*!<  Early Sync */
#define A2B_BITP_I2SGCFG_ALT                 (5u)           /*!<  Alternating Sync */
#define A2B_BITP_I2SGCFG_TDMSS               (4u)           /*!<  TDM Slot Size */
#define A2B_BITP_I2SGCFG_TDMMODE             (0u)           /*!<  TDM Mode */
#define A2B_BITP_I2SGCFG_RXONDTX1 			 (3u)			/*!<  RX on DTX1 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SGCFG                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2SGCFG_INV                 (0x00000080u)
#define A2B_BITM_I2SGCFG_EARLY               (0x00000040u)
#define A2B_BITM_I2SGCFG_ALT                 (0x00000020u)
#define A2B_BITM_I2SGCFG_TDMSS               (0x00000010u)
#define A2B_BITM_I2SGCFG_TDMMODE             (0x00000007u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SGCFG                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2SGCFG_INV_EN              (0x00000080u)  /*!<  Falling edge of SYNC pin corresponds to the start of an audio frame */
#define A2B_ENUM_I2SGCFG_INV_DIS             (0x00000000u)  /*!<  Rising edge of SYNC pin corresponds to the start of an audio frame */
#define A2B_ENUM_I2SGCFG_EARLY_EN            (0x00000040u)  /*!<  Change SYNC pin in previous cycle */
#define A2B_ENUM_I2SGCFG_EARLY_DIS           (0x00000000u)  /*!<  Change SYNC pin in same cycle */
#define A2B_ENUM_I2SGCFG_ALT_EN              (0x00000020u)  /*!<  Drive SYNC Pin Alternating */
#define A2B_ENUM_I2SGCFG_ALT_DIS             (0x00000000u)  /*!<  Drive SYNC Pin for 1 Cycle */
#define A2B_ENUM_I2SGCFG_16_BIT              (0x00000010u)  /*!<  16-Bit */
#define A2B_ENUM_I2SGCFG_32_BIT              (0x00000000u)  /*!<  32-Bit */
#define A2B_ENUM_I2SGCFG_TDM24               (0x00000006u)  /*!<  TDM24 */
#define A2B_ENUM_I2SGCFG_TDM16               (0x00000004u)  /*!<  TDM16 */
#define A2B_ENUM_I2SGCFG_TDM4                (0x00000001u)  /*!<  TDM4 */
#define A2B_ENUM_I2SGCFG_TDM12               (0x00000003u)  /*!<  TDM12 */
#define A2B_ENUM_I2SGCFG_TDM2                (0x00000000u)  /*!<  TDM2 */
#define A2B_ENUM_I2SGCFG_TDM32               (0x00000007u)  /*!<  TDM32 */
#define A2B_ENUM_I2SGCFG_TDM8                (0x00000002u)  /*!<  TDM8 */
#define A2B_ENUM_I2SGCFG_TDM20               (0x00000005u)  /*!<  TDM20 */

#define A2B_REG_I2SGCFG_RESET                (0x00000000u)  /*!<  Reset Value for I2SGCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SCFG_RXBCLKINV            (7u)           /*!<  RX BCLK Invert */
#define A2B_BITP_I2SCFG_RX2PINTL             (6u)           /*!<  RX 2 Pin Interleave */
#define A2B_BITP_I2SCFG_RX1EN                (5u)           /*!<  I2S RX 1 Enable */
#define A2B_BITP_I2SCFG_RX0EN                (4u)           /*!<  I2S RX 0 Enable */
#define A2B_BITP_I2SCFG_TXBCLKINV            (3u)           /*!<  TX BCLK Invert */
#define A2B_BITP_I2SCFG_TX2PINTL             (2u)           /*!<  TX 2 Pin Interleave */
#define A2B_BITP_I2SCFG_TX1EN                (1u)           /*!<  I2S TX 1 Enable */
#define A2B_BITP_I2SCFG_TX0EN                (0u)           /*!<  I2S TX 0 Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2SCFG_RXBCLKINV            (0x00000080u)
#define A2B_BITM_I2SCFG_RX2PINTL             (0x00000040u)
#define A2B_BITM_I2SCFG_RX1EN                (0x00000020u)
#define A2B_BITM_I2SCFG_RX0EN                (0x00000010u)
#define A2B_BITM_I2SCFG_TXBCLKINV            (0x00000008u)
#define A2B_BITM_I2SCFG_TX2PINTL             (0x00000004u)
#define A2B_BITM_I2SCFG_TX1EN                (0x00000002u)
#define A2B_BITM_I2SCFG_TX0EN                (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SCFG                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2SCFG_RXBCLKINV_EN         (0x00000080u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_RXBCLKINV_DIS        (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SCFG_RX2PINTL_EN          (0x00000040u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_RX2PINTL_DIS         (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SCFG_RX1EN_EN             (0x00000020u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_RX1EN_DIS            (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SCFG_RX0EN_EN             (0x00000010u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_RX0EN_DIS            (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SCFG_TXBCLKINV_EN         (0x00000008u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_TXBCLKINV_DIS        (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SCFG_TX2PINTL_EN          (0x00000004u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_TX2PINTL_DIS         (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SCFG_TX1EN_EN             (0x00000002u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_TX1EN_DIS            (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SCFG_TX0EN_EN             (0x00000001u)  /*!<  Enabled */
#define A2B_ENUM_I2SCFG_TX0EN_DIS            (0x00000000u)  /*!<  Disabled */

#define A2B_REG_I2SCFG_RESET                 (0x00000000u)  /*!<  Reset Value for I2SCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRATE                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SRATE_SHARE               (7u)           /*!<  Share Bus Slots in Reduced Rate Mode - AD242X only */
#define A2B_BITP_I2SRATE_REDUCE              (6u)           /*!<  Reduce and Duplicate */
#define A2B_BITP_I2SRATE_FRAMES              (4u)           /*!<  Superframes Used - AD241x only*/
#define A2B_BITP_I2SRATE_BLCKRATE            (3u)           /*!<  BCLK Frequency - AD242X only*/
#define A2B_BITP_I2SRATE_I2SRATE             (0u)           /*!<  I2S Rate Setting */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRATE                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2SRATE_SHARE               (0x00000080u)  /*!<  AD242X only */
#define A2B_BITM_I2SRATE_REDUCE              (0x00000040u)
#define A2B_BITM_I2SRATE_FRAMES              (0x00000030u)
#define A2B_BITM_I2SRATE_BCLKRATE            (0x00000038u)
#define A2B_BITM_I2SRATE_I2SRATE             (0x00000007u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRATE                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2SRATE_SHARE_EN            (0x00000080u)  /*!<  Enabled - AD242X only */
#define A2B_ENUM_I2SRATE_SHARE_DIS           (0x00000000u)  /*!<  Disabled - AD242X only */
#define A2B_ENUM_I2SRATE_REDUCE_EN           (0x00000040u)  /*!<  Enabled */
#define A2B_ENUM_I2SRATE_REDUCE_DIS          (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SRATE_FRAMES_EN           (0x00000010u)  /*!<  Enabled */
#define A2B_ENUM_I2SRATE_FRAMES_DIS          (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_I2SRATE_4X_SFF              (0x00000006u)  /*!<  SFF x 4 */
#define A2B_ENUM_I2SRATE_2X_SFF              (0x00000005u)  /*!<  SFF x 2 */
#define A2B_ENUM_I2SRATE_SFF_RRDIV           (0x00000003u)  /*!<  SFF / RRDIV */
#define A2B_ENUM_I2SRATE_SFF_DIV_4           (0x00000002u)  /*!<  SFF / 4 */
#define A2B_ENUM_I2SRATE_SFF_DIV_2           (0x00000001u)  /*!<  SFF / 2 */
#define A2B_ENUM_I2SRATE_1X_SFF              (0x00000000u)  /*!<  1x SFF */

#define A2B_REG_I2SRATE_RESET                (0x00000000u)  /*!<  Reset Value for I2SRATE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRATE                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SRRATE_RBUS               (7u)           /*!<  Reduced Data Rate - AD242X only */
#define A2B_BITP_I2SRRATE_RRDIV              (0u)           /*!<  Reduced Rate Divide - AD242X only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRATE                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2SRRATE_RBUS               (0x00000080u)  /*!<  AD242X only */
#define A2B_BITM_I2SRRATE_RRDIV              (0x0000003Fu)  /*!<  AD242X only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRATE                             Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2SRRATE_RBUS_EN            (0x00000080u)  /*!<  Enabled - AD242X only */
#define A2B_ENUM_I2SRRATE_RBUS_DIS           (0x00000000u)  /*!<  Disabled - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_1            (0x00000001u)  /*!< 1:1 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_2            (0x00000002u)  /*!< 2:2 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_4            (0x00000004u)  /*!< 4:4 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_8            (0x00000008u)  /*!< 8:8 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_12           (0x00000012u)  /*!< 12:12 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_16           (0x00000016u)  /*!< 16:16 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_20           (0x00000020u)  /*!< 20:20 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_24           (0x00000024u)  /*!< 24:24 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_28           (0x00000028u)  /*!< 28:28 - AD242X only */
#define A2B_ENUM_I2SRRATE_RRDIV_32           (0x00000032u)  /*!< 32:32 - AD242X only */

#define A2B_REG_I2SRRATE_RESET               (0x00000000u)  /*!<  Reset Value for I2SRRATE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRCTL                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SRRCTL_ENCHAN             (6u)           /*!<  Enable I2S Reduced Rate Valid Bits in Extra I2S Channel- AD242X only */
#define A2B_BITP_I2SRRCTL_STRBDIR            (5u)           /*!<  I2S Reduced Rate Strobe Direction - AD242X only */
#define A2B_BITP_I2SRRCTL_ENSTRB             (4u)           /*!<  Enable I2S Reduced Rate Strobe - AD242X only */
#define A2B_BITP_I2SRRCTL_ENXBIT             (1u)           /*!<  Enable I2S Reduced Rate Valid Bit in Extra Bit - AD242X only */
#define A2B_BITP_I2SRRCTL_ENVLSB             (0u)           /*!<  Enable I2S Reduced Rate Valid Bit in LSB - AD242X only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRCTL                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2SRRCTL_ENCHAN             (0x00000040u)  /*!<  AD242X only */
#define A2B_BITM_I2SRRCTL_STRBDIR            (0x00000020u)  /*!<  AD242X only */
#define A2B_BITM_I2SRRCTL_ENSTRB             (0x00000010u)  /*!<  AD242X only */
#define A2B_BITM_I2SRRCTL_ENXBIT             (0x00000002u)  /*!<  AD242X only */
#define A2B_BITM_I2SRRCTL_ENVLSB             (0x00000001u)  /*!<  AD242X only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRCTL                             Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2SRRCTL_ENCHAN_EN          (0x00000040u)  /*!<  Enabled - AD242X only */
#define A2B_ENUM_I2SRRCTL_ENCHAN_DIS         (0x00000000u)  /*!<  Disabled - AD242X only */
#define A2B_ENUM_I2SRRCTL_STRBDIR_OUT        (0x00000020u)  /*!<  Out - AD242X only */
#define A2B_ENUM_I2SRRCTL_STRBDIR_IN         (0x00000000u)  /*!<  In - AD242X only */
#define A2B_ENUM_I2SRRCTL_ENSTRB_EN          (0x00000010u)  /*!<  Enabled - AD242X only */
#define A2B_ENUM_I2SRRCTL_ENSTRB_DIS         (0x00000000u)  /*!<  Disabled - AD242X only */
#define A2B_ENUM_I2SRRCTL_ENXBIT_EN          (0x00000002u)  /*!<  Enabled - AD242X only */
#define A2B_ENUM_I2SRRCTL_ENXBIT_DIS         (0x00000000u)  /*!<  Disabled - AD242X only */
#define A2B_ENUM_I2SRRCTL_ENVLSB_EN          (0x00000001u)  /*!<  Enabled - AD242X only */
#define A2B_ENUM_I2SRRCTL_ENVLSB_DIS         (0x00000000u)  /*!<  Disabled - AD242X only */


#define A2B_REG_I2SRRCTL_RESET               (0x00000000u)  /*!<  Reset Value for I2SRRCTL */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRSOFFS                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SRRSOFFS_RRSOFFSET        (0u)           /*!<  I2S Reduced Rate SYNC Offset  - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRSOFFS                           Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2SRRSOFFS_RRSOFFSET        (0x00000003u)  /*!<  AD242X only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRRSOFFS                           Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2SRRSOFFS_OFFSET_0         (0x00000000u)  /*!<  Offset by 0 superframes - AD242X only */
#define A2B_ENUM_I2SRRSOFFS_OFFSET_1         (0x00000001u)  /*!<  Offset by 1 superframes - AD242X only */
#define A2B_ENUM_I2SRRSOFFS_OFFSET_2         (0x00000002u)  /*!<  Offset by 2 superframes - AD242X only */
#define A2B_ENUM_I2SRRSOFFS_OFFSET_3         (0x00000003u)  /*!<  Offset by 3 superframes - AD242X only */

#define A2B_REG_I2SRRSOFFS_RESET             (0x00000000u)  /*!<  Reset Value for I2SRRCTL */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKOUT1                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CLKOUT1_CLK1EN              (7u)           /*!<  CLKOUT1 Enable  - AD242X only*/
#define A2B_BITP_CLKOUT1_CLK1INV             (6u)           /*!<  CLKOUT1 Invert Enable  - AD242X only*/
#define A2B_BITP_CLKOUT1_CLK1PDIV            (5u)           /*!<  CLKOUT1 Pre-Divide Value  - AD242X only*/
#define A2B_BITP_CLKOUT1_CLK1DIV             (0u)           /*!<  CLKOUT1 Divide Value  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKOUT1                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_CLKOUT1_CLK1EN           (0x00000080u)     /*!<  AD242X only*/
#define A2B_BITM_CLKOUT1_CLK1INV          (0x00000040u)     /*!<  AD242X only*/
#define A2B_BITM_CLKOUT1_CLK1PDIV         (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_CLKOUT1_CLK1DIV          (0x0000000Fu)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKOUT1                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_CLKOUT1_CLK1EN_EN        (0x00000080u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT1_CLK1EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT1_CLK1INV_EN       (0x00000040u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT1_CLK1INV_DIS      (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT1_CLK1PDIV_32      (0x00000020u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT1_CLK1PDIV_2       (0x00000000u)  /*!<  AD242X only */


#define A2B_REG_CLKOUT1_RESET             (0x00000000u)  /*!<  Reset Value for CLKOUT1 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKOUT2                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CLKOUT2_CLK2EN              (7u)           /*!<  CLKOUT2 Enable  - AD242X only*/
#define A2B_BITP_CLKOUT2_CLK2INV             (6u)           /*!<  CLKOUT2 Invert Enable  - AD242X only*/
#define A2B_BITP_CLKOUT2_CLK2PDIV            (5u)           /*!<  CLKOUT2 Pre-Divide Value  - AD242X only*/
#define A2B_BITP_CLKOUT2_CLK2DIV             (0u)           /*!<  CLKOUT2 Divide Value  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKOUT2                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_CLKOUT2_CLK2EN           (0x00000080u)     /*!<  AD242X only*/
#define A2B_BITM_CLKOUT2_CLK2INV          (0x00000040u)     /*!<  AD242X only*/
#define A2B_BITM_CLKOUT2_CLK2PDIV         (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_CLKOUT2_CLK1DIV          (0x0000000Fu)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKOUT2                             Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_CLKOUT2_CLK1EN_EN        (0x00000080u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT2_CLK1EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT2_CLK1INV_EN       (0x00000040u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT2_CLK1INV_DIS      (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT2_CLK1PDIV_32      (0x00000020u)  /*!<  AD242X only */
#define A2B_ENUM_CLKOUT2_CLK1PDIV_2       (0x00000000u)  /*!<  AD242X only */


#define A2B_REG_CLKOUT2_RESET             (0x00000000u)  /*!<  Reset Value for CLKOUT2 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BMMCFG                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_BMMCFG_BMMNDSC             (2u)           /*!<  BMM No Discovery Mode Enable  - AD242X only*/
#define A2B_BITP_BMMCFG_BMMRXEN             (1u)           /*!<  BMM LVDS XCVR RX Enable  - AD242X only*/
#define A2B_BITP_BMMCFG_BMMEN               (0u)           /*!<  Bus Monitor Mode Enable  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BMMCFG                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_BMMCFG_BMMNDSC          (0x00000004u)     /*!<  AD242X only*/
#define A2B_BITM_BMMCFG_BMMRXEN          (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_BMMCFG_BMMEN            (0x00000001u)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BMMCFG                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_BMMCFG_BMMNDSC_EN       (0x00000004u)    /*!<  AD242X only */
#define A2B_ENUM_BMMCFG_BMMNDSC_DIS      (0x00000000u)    /*!<  AD242X only */
#define A2B_ENUM_BMMCFG_BMMRXEN_EN       (0x00000002u)    /*!<  AD242X only */
#define A2B_ENUM_BMMCFG_BMMRXEN_DIS      (0x00000000u)    /*!<  AD242X only */
#define A2B_ENUM_BMMCFG_BMMEN_EN         (0x00000001u)    /*!<  AD242X only */
#define A2B_ENUM_BMMCFG_BMMEN_DIS        (0x00000000u)    /*!<  AD242X only */



#define A2B_REG_BMMCFG_RESET             (0x00000000u)  /*!<  Reset Value for BMMCFG */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SUSCFG                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SUSCFG_SUSDIS              (5u)           /*!<  Sustain disable  - AD242X only*/
#define A2B_BITP_SUSCFG_SUSOE               (4u)           /*!<  Sustain GPIO Output Enable  - AD242X only*/
#define A2B_BITP_SUSCFG_SUSSEL              (0u)           /*!<  Sustain GPIO Output Select  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SUSCFG                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_SUSCFG_SUSDIS           (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_SUSCFG_SUSOE            (0x00000010u)     /*!<  AD242X only*/
#define A2B_BITM_SUSCFG_SUSSEL           (0x00000007u)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SUSCFG                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_SUSCFG_SUSDIS_EN        (0x00000020u)    /*!<  AD242X only */
#define A2B_ENUM_SUSCFG_SUSDIS_DIS       (0x00000000u)    /*!<  AD242X only */
#define A2B_ENUM_SUSCFG_SUSOE_EN         (0x00000010u)    /*!<  AD242X only */
#define A2B_ENUM_SUSCFG_SUSOE_DIS        (0x00000000u)    /*!<  AD242X only */

#define A2B_REG_SUSCFG_RESET             (0x00000000u)    /*!<  Reset Value for SUSCFG */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK0                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_UPMASK0_RXUPSLOT07          (7u)       /*!<  Receive Upstream Data Slot 7  - AD242X only, slave only */
#define A2B_BITP_UPMASK0_RXUPSLOT06          (6u)       /*!<  Receive Upstream Data Slot 6  - AD242X only, slave only */
#define A2B_BITP_UPMASK0_RXUPSLOT05          (5u)       /*!<  Receive Upstream Data Slot 5  - AD242X only, slave only */
#define A2B_BITP_UPMASK0_RXUPSLOT04          (4u)       /*!<  Receive Upstream Data Slot 4  - AD242X only, slave only */
#define A2B_BITP_UPMASK0_RXUPSLOT03          (3u)       /*!<  Receive Upstream Data Slot 3  - AD242X only, slave only */
#define A2B_BITP_UPMASK0_RXUPSLOT02          (2u)       /*!<  Receive Upstream Data Slot 2  - AD242X only, slave only */
#define A2B_BITP_UPMASK0_RXUPSLOT01          (1u)       /*!<  Receive Upstream Data Slot 1  - AD242X only, slave only */
#define A2B_BITP_UPMASK0_RXUPSLOT00          (0u)       /*!<  Receive Upstream Data Slot 0  - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK0                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_UPMASK0_RXUPSLOT07      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK0_RXUPSLOT06      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK0_RXUPSLOT05      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK0_RXUPSLOT04      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK0_RXUPSLOT03      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK0_RXUPSLOT02      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK0_RXUPSLOT01      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK0_RXUPSLOT00      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK0                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_UPMASK0_RXUPSLOT07_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT07_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT06_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT06_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT05_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT05_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT04_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT04_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT03_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT03_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT02_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT02_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT01_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT01_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT00_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK0_RXUPSLOT00_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_UPMASK0_RESET            (0x00000000u)    /*!<  Reset Value for UPMASK0 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK1                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_UPMASK1_RXUPSLOT15          (7u)       /*!<  Receive Upstream Data Slot 15  - AD242X only, slave only */
#define A2B_BITP_UPMASK1_RXUPSLOT14          (6u)       /*!<  Receive Upstream Data Slot 14 - AD242X only, slave only */
#define A2B_BITP_UPMASK1_RXUPSLOT13          (5u)       /*!<  Receive Upstream Data Slot 13 - AD242X only, slave only */
#define A2B_BITP_UPMASK1_RXUPSLOT12          (4u)       /*!<  Receive Upstream Data Slot 12 - AD242X only, slave only */
#define A2B_BITP_UPMASK1_RXUPSLOT11          (3u)       /*!<  Receive Upstream Data Slot 11 - AD242X only, slave only */
#define A2B_BITP_UPMASK1_RXUPSLOT10          (2u)       /*!<  Receive Upstream Data Slot 10 - AD242X only, slave only */
#define A2B_BITP_UPMASK1_RXUPSLOT09          (1u)       /*!<  Receive Upstream Data Slot 9 - AD242X only, slave only */
#define A2B_BITP_UPMASK1_RXUPSLOT08          (0u)       /*!<  Receive Upstream Data Slot 8  - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK1                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_UPMASK1_RXUPSLOT15      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK1_RXUPSLOT14      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK1_RXUPSLOT13      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK1_RXUPSLOT12      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK1_RXUPSLOT11      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK1_RXUPSLOT10      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK1_RXUPSLOT09      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK1_RXUPSLOT08      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK1                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_UPMASK1_RXUPSLOT15_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT15_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT14_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT14_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT13_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT13_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT12_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT12_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT11_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT11_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT10_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT10_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT09_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT09_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT08_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK1_RXUPSLOT08_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_UPMASK1_RESET            (0x00000000u)    /*!<  Reset Value for UPMASK1 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK2                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_UPMASK2_RXUPSLOT23          (7u)       /*!<  Receive Upstream Data Slot 23 - AD242X only, slave only */
#define A2B_BITP_UPMASK2_RXUPSLOT22          (6u)       /*!<  Receive Upstream Data Slot 22 - AD242X only, slave only */
#define A2B_BITP_UPMASK2_RXUPSLOT21          (5u)       /*!<  Receive Upstream Data Slot 21 - AD242X only, slave only */
#define A2B_BITP_UPMASK2_RXUPSLOT20          (4u)       /*!<  Receive Upstream Data Slot 20 - AD242X only, slave only */
#define A2B_BITP_UPMASK2_RXUPSLOT19          (3u)       /*!<  Receive Upstream Data Slot 19 - AD242X only, slave only */
#define A2B_BITP_UPMASK2_RXUPSLOT18          (2u)       /*!<  Receive Upstream Data Slot 18 - AD242X only, slave only */
#define A2B_BITP_UPMASK2_RXUPSLOT17          (1u)       /*!<  Receive Upstream Data Slot 17 - AD242X only, slave only */
#define A2B_BITP_UPMASK2_RXUPSLOT16          (0u)       /*!<  Receive Upstream Data Slot 16 - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK2                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_UPMASK2_RXUPSLOT23      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK2_RXUPSLOT22      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK2_RXUPSLOT21      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK2_RXUPSLOT20      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK2_RXUPSLOT19      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK2_RXUPSLOT18      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK2_RXUPSLOT17      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK2_RXUPSLOT16      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK2                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_UPMASK2_RXUPSLOT23_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT23_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT22_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT22_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT21_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT21_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT20_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT20_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT19_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT19_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT18_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT18_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT17_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT17_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT16_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK2_RXUPSLOT16_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_UPMASK2_RESET            (0x00000000u)    /*!<  Reset Value for UPMASK2 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK3                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_UPMASK3_RXUPSLOT31          (7u)       /*!<  Receive Upstream Data Slot 31 - AD242X only, slave only */
#define A2B_BITP_UPMASK3_RXUPSLOT30          (6u)       /*!<  Receive Upstream Data Slot 30 - AD242X only, slave only */
#define A2B_BITP_UPMASK3_RXUPSLOT29          (5u)       /*!<  Receive Upstream Data Slot 29 - AD242X only, slave only */
#define A2B_BITP_UPMASK3_RXUPSLOT28          (4u)       /*!<  Receive Upstream Data Slot 28 - AD242X only, slave only */
#define A2B_BITP_UPMASK3_RXUPSLOT27          (3u)       /*!<  Receive Upstream Data Slot 27 - AD242X only, slave only */
#define A2B_BITP_UPMASK3_RXUPSLOT26          (2u)       /*!<  Receive Upstream Data Slot 26 - AD242X only, slave only */
#define A2B_BITP_UPMASK3_RXUPSLOT25          (1u)       /*!<  Receive Upstream Data Slot 25 - AD242X only, slave only */
#define A2B_BITP_UPMASK3_RXUPSLOT24          (0u)       /*!<  Receive Upstream Data Slot 24 - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK3                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_UPMASK3_RXUPSLOT31      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK3_RXUPSLOT30      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK3_RXUPSLOT29      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK3_RXUPSLOT28      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK3_RXUPSLOT27      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK3_RXUPSLOT26      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK3_RXUPSLOT25      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_UPMASK3_RXUPSLOT24      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPMASK3                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_UPMASK3_RXUPSLOT31_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT31_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT30_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT30_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT29_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT29_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT28_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT28_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT27_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT27_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT26_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT26_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT25_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT25_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT24_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_UPMASK3_RXUPSLOT24_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_UPMASK3_RESET            (0x00000000u)    /*!<  Reset Value for UPMASK3 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPOFFSET                        Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_UPOFFSET_UPOFFSET       (0u)           /*!<  Upstream Slots Offset for Local Node  - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPOFFSET                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_UPOFFSET_UPOFFSET       (0x0000001Fu)  /*!<  AD242X only, slave only */

#define A2B_REG_UPOFFSET_RESET           (0x00000000u)    /*!<  Reset Value for UPMASK3 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK0                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DNMASK0_RXDNSLOT07          (7u)       /*!<  Receive DownStream Data Slot 7  - AD242X only, slave only */
#define A2B_BITP_DNMASK0_RXDNSLOT06          (6u)       /*!<  Receive DownStream Data Slot 6  - AD242X only, slave only */
#define A2B_BITP_DNMASK0_RXDNSLOT05          (5u)       /*!<  Receive DownStream Data Slot 5  - AD242X only, slave only */
#define A2B_BITP_DNMASK0_RXDNSLOT04          (4u)       /*!<  Receive DownStream Data Slot 4  - AD242X only, slave only */
#define A2B_BITP_DNMASK0_RXDNSLOT03          (3u)       /*!<  Receive DownStream Data Slot 3  - AD242X only, slave only */
#define A2B_BITP_DNMASK0_RXDNSLOT02          (2u)       /*!<  Receive DownStream Data Slot 2  - AD242X only, slave only */
#define A2B_BITP_DNMASK0_RXDNSLOT01          (1u)       /*!<  Receive DownStream Data Slot 1  - AD242X only, slave only */
#define A2B_BITP_DNMASK0_RXDNSLOT00          (0u)       /*!<  Receive DownStream Data Slot 0  - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK0                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DNMASK0_RXDNSLOT07      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK0_RXDNSLOT06      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK0_RXDNSLOT05      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK0_RXDNSLOT04      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK0_RXDNSLOT03      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK0_RXDNSLOT02      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK0_RXDNSLOT01      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK0_RXDNSLOT00      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK0                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_DNMASK0_RXDNSLOT07_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT07_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT06_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT06_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT05_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT05_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT04_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT04_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT03_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT03_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT02_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT02_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT01_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT01_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT00_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK0_RXDNSLOT00_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_DNMASK0_RESET            (0x00000000u)    /*!<  Reset Value for DNMASK0 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK1                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DNMASK1_RXDNSLOT15          (7u)       /*!<  Receive DownStream Data Slot 15  - AD242X only, slave only */
#define A2B_BITP_DNMASK1_RXDNSLOT14          (6u)       /*!<  Receive DownStream Data Slot 14 - AD242X only, slave only */
#define A2B_BITP_DNMASK1_RXDNSLOT13          (5u)       /*!<  Receive DownStream Data Slot 13 - AD242X only, slave only */
#define A2B_BITP_DNMASK1_RXDNSLOT12          (4u)       /*!<  Receive DownStream Data Slot 12 - AD242X only, slave only */
#define A2B_BITP_DNMASK1_RXDNSLOT11          (3u)       /*!<  Receive DownStream Data Slot 11 - AD242X only, slave only */
#define A2B_BITP_DNMASK1_RXDNSLOT10          (2u)       /*!<  Receive DownStream Data Slot 10 - AD242X only, slave only */
#define A2B_BITP_DNMASK1_RXDNSLOT09          (1u)       /*!<  Receive DownStream Data Slot 9 - AD242X only, slave only */
#define A2B_BITP_DNMASK1_RXDNSLOT08          (0u)       /*!<  Receive DownStream Data Slot 8  - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK1                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DNMASK1_RXDNSLOT15      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK1_RXDNSLOT14      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK1_RXDNSLOT13      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK1_RXDNSLOT12      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK1_RXDNSLOT11      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK1_RXDNSLOT10      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK1_RXDNSLOT09      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK1_RXDNSLOT08      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK1                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_DNMASK1_RXDNSLOT15_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT15_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT14_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT14_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT13_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT13_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT12_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT12_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT11_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT11_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT10_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT10_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT09_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT09_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT08_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK1_RXDNSLOT08_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_DNMASK1_RESET            (0x00000000u)    /*!<  Reset Value for DNMASK1 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK2                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DNMASK2_RXDNSLOT23          (7u)       /*!<  Receive DownStream Data Slot 23 - AD242X only, slave only */
#define A2B_BITP_DNMASK2_RXDNSLOT22          (6u)       /*!<  Receive DownStream Data Slot 22 - AD242X only, slave only */
#define A2B_BITP_DNMASK2_RXDNSLOT21          (5u)       /*!<  Receive DownStream Data Slot 21 - AD242X only, slave only */
#define A2B_BITP_DNMASK2_RXDNSLOT20          (4u)       /*!<  Receive DownStream Data Slot 20 - AD242X only, slave only */
#define A2B_BITP_DNMASK2_RXDNSLOT19          (3u)       /*!<  Receive DownStream Data Slot 19 - AD242X only, slave only */
#define A2B_BITP_DNMASK2_RXDNSLOT18          (2u)       /*!<  Receive DownStream Data Slot 18 - AD242X only, slave only */
#define A2B_BITP_DNMASK2_RXDNSLOT17          (1u)       /*!<  Receive DownStream Data Slot 17 - AD242X only, slave only */
#define A2B_BITP_DNMASK2_RXDNSLOT16          (0u)       /*!<  Receive DownStream Data Slot 16 - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK2                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DNMASK2_RXDNSLOT23      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK2_RXDNSLOT22      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK2_RXDNSLOT21      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK2_RXDNSLOT20      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK2_RXDNSLOT19      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK2_RXDNSLOT18      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK2_RXDNSLOT17      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK2_RXDNSLOT16      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK2                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_DNMASK2_RXDNSLOT23_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT23_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT22_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT22_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT21_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT21_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT20_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT20_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT19_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT19_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT18_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT18_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT17_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT17_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT16_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK2_RXDNSLOT16_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_DNMASK2_RESET            (0x00000000u)    /*!<  Reset Value for DNMASK2 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK3                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DNMASK3_RXDNSLOT31          (7u)       /*!<  Receive DownStream Data Slot 31 - AD242X only, slave only */
#define A2B_BITP_DNMASK3_RXDNSLOT30          (6u)       /*!<  Receive DownStream Data Slot 30 - AD242X only, slave only */
#define A2B_BITP_DNMASK3_RXDNSLOT29          (5u)       /*!<  Receive DownStream Data Slot 29 - AD242X only, slave only */
#define A2B_BITP_DNMASK3_RXDNSLOT28          (4u)       /*!<  Receive DownStream Data Slot 28 - AD242X only, slave only */
#define A2B_BITP_DNMASK3_RXDNSLOT27          (3u)       /*!<  Receive DownStream Data Slot 27 - AD242X only, slave only */
#define A2B_BITP_DNMASK3_RXDNSLOT26          (2u)       /*!<  Receive DownStream Data Slot 26 - AD242X only, slave only */
#define A2B_BITP_DNMASK3_RXDNSLOT25          (1u)       /*!<  Receive DownStream Data Slot 25 - AD242X only, slave only */
#define A2B_BITP_DNMASK3_RXDNSLOT24          (0u)       /*!<  Receive DownStream Data Slot 24 - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK3                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DNMASK3_RXDNSLOT31      (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK3_RXDNSLOT30      (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK3_RXDNSLOT29      (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK3_RXDNSLOT28      (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK3_RXDNSLOT27      (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK3_RXDNSLOT26      (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK3_RXDNSLOT25      (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_BITM_DNMASK3_RXDNSLOT24      (0x00000001u)  /*!<  AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNMASK3                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_DNMASK3_RXDNSLOT31_EN   (0x00000080u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT31_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT30_EN   (0x00000040u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT30_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT29_EN   (0x00000020u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT29_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT28_EN   (0x00000010u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT28_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT27_EN   (0x00000008u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT27_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT26_EN   (0x00000004u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT26_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT25_EN   (0x00000002u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT25_DIS  (0x00000000u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT24_EN   (0x00000001u)  /*!<  AD242X only, slave only */
#define A2B_ENUM_DNMASK3_RXDNSLOT24_DIS  (0x00000000u)  /*!<  AD242X only, slave only */


#define A2B_REG_DNMASK3_RESET            (0x00000000u)    /*!<  Reset Value for DNMASK3 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNOFFSET                        Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DNOFFSET_DNOFFSET       (0u)           /*!<  DownStream Slots Offset for Local Node  - AD242X only, slave only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNOFFSET                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_DNOFFSET_DNOFFSET       (0x0000001Fu)  /*!<  AD242X only, slave only */

#define A2B_REG_DNOFFSET_RESET           (0x00000000u)    /*!<  Reset Value for DNMASK3 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODEN                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIODEN_IOD7EN              (7u)           /*!<  Sustain disable  - AD242X only*/
#define A2B_BITP_GPIODEN_IOD6EN              (6u)           /*!<  Sustain GPIO Output Enable  - AD242X only*/
#define A2B_BITP_GPIODEN_IOD5EN              (5u)           /*!<  Sustain GPIO Output Select  - AD242X only*/
#define A2B_BITP_GPIODEN_IOD4EN              (4u)           /*!<  Sustain GPIO Output Select  - AD242X only*/
#define A2B_BITP_GPIODEN_IOD3EN              (3u)           /*!<  Sustain GPIO Output Select  - AD242X only*/
#define A2B_BITP_GPIODEN_IOD2EN              (2u)           /*!<  Sustain GPIO Output Select  - AD242X only*/
#define A2B_BITP_GPIODEN_IOD1EN              (1u)           /*!<  Sustain GPIO Output Select  - AD242X only*/
#define A2B_BITP_GPIODEN_IOD0EN              (0u)           /*!<  Sustain GPIO Output Select  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODEN                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIODEN_IOD7EN           (0x00000080u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODEN_IOD6EN           (0x00000040u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODEN_IOD5EN           (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODEN_IOD4EN           (0x00000010u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODEN_IOD3EN           (0x00000008u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODEN_IOD2EN           (0x00000004u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODEN_IOD1EN           (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODEN_IOD0EN           (0x00000001u)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODEN                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_GPIODEN_IOD7EN_EN        (0x00000080u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD7EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD6EN_EN        (0x00000040u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD6EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD5EN_EN        (0x00000020u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD5EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD4EN_EN        (0x00000010u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD4EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD3EN_EN        (0x00000008u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD3EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD2EN_EN        (0x00000004u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD2EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD1EN_EN        (0x00000002u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD1EN_DIS       (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD0EN_EN        (0x00000001u)  /*!<  AD242X only */
#define A2B_ENUM_GPIODEN_IOD0EN_DIS       (0x00000000u)  /*!<  AD242X only */

#define A2B_REG_GPIODEN_RESET             (0x00000000u)  /*!<  Reset Value for GPIODEN */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD0MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD0MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO0 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD0MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD0MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD0MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD0MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD1MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD1MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO1 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD1MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD1MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD1MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD1MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD2MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD2MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO2 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD2MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD2MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD2MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD2MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD3MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD3MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO3 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD3MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD3MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD3MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD3MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD4MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD4MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO4 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD4MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD4MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD4MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD4MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD5MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD5MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO5 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD5MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD5MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD5MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD5MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD6MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD6MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO6 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD6MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD6MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD6MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD6MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD7MSK                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOD7MSK_IOD0MSK           (0u)           /*!<  GPIO Over Distance IO7 Mask  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOD7MSK                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOD7MSK_IOD0MSK        (0x000000FFu)     /*!<  AD242X only*/

#define A2B_REG_GPIOD7MSK_RESET           (0x00000000u)     /*!<  Reset Value for GPIOD7MSK */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODDAT                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIODDAT_IOD7DAT             (7u)           /*!<  Bus GPIO Port Bit 7 Value  - AD242X only*/
#define A2B_BITP_GPIODDAT_IOD6DAT             (6u)           /*!<  Bus GPIO Port Bit 6 Value  - AD242X only*/
#define A2B_BITP_GPIODDAT_IOD5DAT             (5u)           /*!<  Bus GPIO Port Bit 5 Value  - AD242X only*/
#define A2B_BITP_GPIODDAT_IOD4DAT             (4u)           /*!<  Bus GPIO Port Bit 4 Value  - AD242X only*/
#define A2B_BITP_GPIODDAT_IOD3DAT             (3u)           /*!<  Bus GPIO Port Bit 3 Value  - AD242X only*/
#define A2B_BITP_GPIODDAT_IOD2DAT             (2u)           /*!<  Bus GPIO Port Bit 2 Value  - AD242X only*/
#define A2B_BITP_GPIODDAT_IOD1DAT             (1u)           /*!<  Bus GPIO Port Bit 1 Value  - AD242X only*/
#define A2B_BITP_GPIODDAT_IOD0DAT             (0u)           /*!<  Bus GPIO Port Bit 0 Value  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODDAT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIODDAT_IOD7DAT          (0x00000080u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODDAT_IOD6DAT          (0x00000040u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODDAT_IOD5DAT          (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODDAT_IOD4DAT          (0x00000010u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODDAT_IOD3DAT          (0x00000008u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODDAT_IOD2DAT          (0x00000004u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODDAT_IOD1DAT          (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODDAT_IOD0DAT          (0x00000001u)     /*!<  AD242X only*/

#define A2B_REG_GPIODDAT_RESET             (0x00000000u)     /*!<  Reset Value for GPIODDAT */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODINV                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIODINV_IOD7INV             (7u)           /*!<  GPIO Over Distance IO7 Invert  - AD242X only*/
#define A2B_BITP_GPIODINV_IOD6INV             (6u)           /*!<  GPIO Over Distance IO6 Invert  - AD242X only*/
#define A2B_BITP_GPIODINV_IOD5INV             (5u)           /*!<  GPIO Over Distance IO5 Invert  - AD242X only*/
#define A2B_BITP_GPIODINV_IOD4INV             (4u)           /*!<  GPIO Over Distance IO4 Invert  - AD242X only*/
#define A2B_BITP_GPIODINV_IOD3INV             (3u)           /*!<  GPIO Over Distance IO3 Invert  - AD242X only*/
#define A2B_BITP_GPIODINV_IOD2INV             (2u)           /*!<  GPIO Over Distance IO2 Invert  - AD242X only*/
#define A2B_BITP_GPIODINV_IOD1INV             (1u)           /*!<  GPIO Over Distance IO1 Invert  - AD242X only*/
#define A2B_BITP_GPIODINV_IOD0INV             (0u)           /*!<  GPIO Over Distance IO0 Invert  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODINV                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIODINV_IOD7INV          (0x00000080u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODINV_IOD6INV          (0x00000040u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODINV_IOD5INV          (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODINV_IOD4INV          (0x00000010u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODINV_IOD3INV          (0x00000008u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODINV_IOD2INV          (0x00000004u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODINV_IOD1INV          (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_GPIODINV_IOD0INV          (0x00000001u)     /*!<  AD242X only*/

#define A2B_REG_GPIODINV_RESET             (0x00000000u)     /*!<  Reset Value for GPIODINV */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0CTL                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX0CTL_MB0LEN              (4u)           /*!<  Mailbox 0 Length  - AD242X only*/
#define A2B_BITP_MBOX0CTL_MB0FIEN             (3u)           /*!<  Mailbox 0 Full Interrupt Enable  - AD242X only*/
#define A2B_BITP_MBOX0CTL_MB0EIEN             (2u)           /*!<  Mailbox 0 Empty Interrupt Enable  - AD242X only*/
#define A2B_BITP_MBOX0CTL_MB0DIR              (1u)           /*!<  Mailbox 0 Direction  - AD242X only*/
#define A2B_BITP_MBOX0CTL_MB0EN               (0u)           /*!<  Mailbox 0 Enable  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0CTL                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX0CTL_MB0LEN           (0x00000030u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX0CTL_MB0FIEN          (0x00000008u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX0CTL_MB0EIEN          (0x00000004u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX0CTL_MB0DIR           (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX0CTL_MB0EN            (0x00000001u)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0CTL                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_MBOX0CTL_MB0LEN_1_BYTE    (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0CTL_MB0LEN_2_BYTE    (0x00000010u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0CTL_MB0LEN_3_BYTE    (0x00000020u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0CTL_MB0LEN_4_BYTE    (0x00000030u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX0CTL_MB0FIEN_DIS      (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0CTL_MB0FIEN_EN       (0x00000008u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX0CTL_MB0EIEN_DIS      (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0CTL_MB0EIEN_EN       (0x00000004u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX0CTL_MB0DIR_TX        (0x00000002u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0CTL_MB0DIR_RX        (0x00000000u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX0CTL_MB0EN_DIS        (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0CTL_MB0EN_EN         (0x00000001u)  /*!<  AD242X only */

#define A2B_REG_MBOX0CTL_RESET             (0x00000000u)  /*!<  Reset Value for MBOX0CTL */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0STAT                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX0STAT_MB0EIRQ             (5u)       /*!<  Mailbox 0 Signaling Empty IRQ  - AD242X only*/
#define A2B_BITP_MBOX0STAT_MB0FIRQ             (4u)       /*!<  Mailbox 0 Signaling Full IRQ  - AD242X only*/
#define A2B_BITP_MBOX0STAT_MB0EMPTY            (1u)       /*!<  Mailbox 0 Empty  - AD242X only*/
#define A2B_BITP_MBOX0STAT_MB0FULL             (0u)       /*!<  Mailbox 0 Full  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0STAT                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX0STAT_MB0EIRQ        (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX0STAT_MB0FIRQ        (0x00000010u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX0STAT_MB0EMPTY       (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX0STAT_MB0FULL        (0x00000001u)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0STAT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_MBOX0STAT_MB0EIRQ_ACT    (0x00000020u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0STAT_MB0EIRQ_INACT  (0x00000000u)  /*!<  AD242X only */


#define A2B_ENUM_MBOX0STAT_MB0FIRQ_ACT    (0x00000010u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0STAT_MB0FIRQ_INACT  (0x00000000u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX0STAT_MB0EMPTY_INACT (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0STAT_MB0EMPTY_ACT   (0x00000002u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX0STAT_MB0FULL_INACT  (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX0STAT_MB0FULL_ACT    (0x00000001u)  /*!<  AD242X only */

#define A2B_REG_MBOX0STAT_RESET           (0x00000002u)  /*!<  Reset Value for MBOX0STAT */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B0                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX0B0_MBOX0              (0u)           /*!<  Mailbox 0 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B0                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX0B0_MBOX0            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX0B0_RESET             (0x00000000u)    /*!<  Reset Value for MBOX0B0 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B1                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX0B1_MBOX0              (0u)           /*!<  Mailbox 1 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B1                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX0B1_MBOX0            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX0B1_RESET             (0x00000000u)    /*!<  Reset Value for MBOX0B1 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B2                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX0B2_MBOX0              (0u)           /*!<  Mailbox 2 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B2                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX0B2_MBOX0            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX0B2_RESET             (0x00000000u)    /*!<  Reset Value for MBOX0B2 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B3                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX0B3_MBOX0              (0u)           /*!<  Mailbox 3 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX0B3                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX0B3_MBOX0            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX0B3_RESET             (0x00000000u)    /*!<  Reset Value for MBOX0B3 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1CTL                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX1CTL_MB1LEN              (4u)           /*!<  Mailbox 1 Length  - AD242X only*/
#define A2B_BITP_MBOX1CTL_MB1FIEN             (3u)           /*!<  Mailbox 1 Full Interrupt Enable  - AD242X only*/
#define A2B_BITP_MBOX1CTL_MB1EIEN             (2u)           /*!<  Mailbox 1 Empty Interrupt Enable  - AD242X only*/
#define A2B_BITP_MBOX1CTL_MB1DIR              (1u)           /*!<  Mailbox 1 Direction  - AD242X only*/
#define A2B_BITP_MBOX1CTL_MB1EN               (0u)           /*!<  Mailbox 1 Enable  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1CTL                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX1CTL_MB1LEN           (0x00000030u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX1CTL_MB1FIEN          (0x00000008u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX1CTL_MB1EIEN          (0x00000004u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX1CTL_MB1DIR           (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX1CTL_MB1EN            (0x00000001u)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1CTL                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_MBOX1CTL_MB1LEN_1_BYTE    (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1CTL_MB1LEN_2_BYTE    (0x00000010u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1CTL_MB1LEN_3_BYTE    (0x00000020u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1CTL_MB1LEN_4_BYTE    (0x00000030u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX1CTL_MB1FIEN_DIS      (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1CTL_MB1FIEN_EN       (0x00000008u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX1CTL_MB1EIEN_DIS      (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1CTL_MB1EIEN_EN       (0x00000004u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX1CTL_MB1DIR_TX        (0x00000002u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1CTL_MB1DIR_RX        (0x00000000u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX1CTL_MB1EN_DIS        (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1CTL_MB1EN_EN         (0x00000001u)  /*!<  AD242X only */

#define A2B_REG_MBOX1CTL_RESET             (0x00000000u)  /*!<  Reset Value for MBOX1CTL */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1STAT                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX1STAT_MB1EIRQ             (5u)       /*!<  Mailbox 1 Signaling Empty IRQ  - AD242X only*/
#define A2B_BITP_MBOX1STAT_MB1FIRQ             (4u)       /*!<  Mailbox 1 Signaling Full IRQ  - AD242X only*/
#define A2B_BITP_MBOX1STAT_MB1EMPTY            (1u)       /*!<  Mailbox 1 Empty  - AD242X only*/
#define A2B_BITP_MBOX1STAT_MB1FULL             (0u)       /*!<  Mailbox 1 Full  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1STAT                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX1STAT_MB1EIRQ        (0x00000020u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX1STAT_MB1FIRQ        (0x00000010u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX1STAT_MB1EMPTY       (0x00000002u)     /*!<  AD242X only*/
#define A2B_BITM_MBOX1STAT_MB1FULL        (0x00000001u)     /*!<  AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1STAT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_MBOX1STAT_MB1EIRQ_ACT    (0x00000020u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1STAT_MB1EIRQ_INACT  (0x00000000u)  /*!<  AD242X only */


#define A2B_ENUM_MBOX1STAT_MB1FIRQ_ACT    (0x00000010u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1STAT_MB1FIRQ_INACT  (0x00000000u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX1STAT_MB1EMPTY_INACT (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1STAT_MB1EMPTY_ACT   (0x00000002u)  /*!<  AD242X only */

#define A2B_ENUM_MBOX1STAT_MB1FULL_INACT  (0x00000000u)  /*!<  AD242X only */
#define A2B_ENUM_MBOX1STAT_MB1FULL_ACT    (0x00000001u)  /*!<  AD242X only */

#define A2B_REG_MBOX1STAT_RESET           (0x00000002u)  /*!<  Reset Value for MBOX1STAT */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B0                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX1B0_MBOX1              (0u)           /*!<  Mailbox 1 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B0                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX1B0_MBOX1            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX1B0_RESET             (0x00000000u)    /*!<  Reset Value for MBOX1B0 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B1                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX1B1_MBOX1              (0u)           /*!<  Mailbox 1 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B1                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX1B1_MBOX1            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX1B1_RESET             (0x00000000u)    /*!<  Reset Value for MBOX1B1 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B2                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX1B2_MBOX1              (0u)           /*!<  Mailbox 2 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B2                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX1B2_MBOX1            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX1B2_RESET             (0x00000000u)    /*!<  Reset Value for MBOX1B2 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B3                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MBOX1B3_MBOX1              (0u)           /*!<  Mailbox 3 Data Byte 1  - AD242X only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        MBOX1B3                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_MBOX1B3_MBOX1            (0x000000FFu)    /*!<  AD242X only*/

#define A2B_REG_MBOX1B3_RESET             (0x00000000u)    /*!<  Reset Value for MBOX1B3 */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STXOFFSET                          Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2STXOFFSET_TSBEFORE        (7u)           /*!<  Three-State before TX Slots */
#define A2B_BITP_I2STXOFFSET_TSAFTER         (6u)           /*!<  Three-State after TX Slots */
#define A2B_BITP_I2STXOFFSET_TXOFFSET        (0u)           /*!<  Serial TX Offset */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STXOFFSET                          Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2STXOFFSET_TSBEFORE        (0x00000080u)
#define A2B_BITM_I2STXOFFSET_TSAFTER         (0x00000040u)
#define A2B_BITM_I2STXOFFSET_TXOFFSET        (0x0000003Fu)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STXOFFSET                          Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2STXOFFSET_TSBEFORE_EN     (0x00000080u)  /*!<  Enable */
#define A2B_ENUM_I2STXOFFSET_TSBEFORE_DIS    (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_I2STXOFFSET_TSAFTER_EN      (0x00000040u)  /*!<  Enable */
#define A2B_ENUM_I2STXOFFSET_TSAFTER_DIS     (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_I2STXOFFSET_TXOFFSET_01     (0x00000001u)  /*!<  TX Offset of 1 TDM Slot */
#define A2B_ENUM_I2STXOFFSET_TXOFFSET_63     (0x0000003Fu)  /*!<  TX Offset of 63 TDM Slots */
#define A2B_ENUM_I2STXOFFSET_TXOFFSET_00     (0x00000000u)  /*!<  No TX Offset */
#define A2B_ENUM_I2STXOFFSET_TXOFFSET_62     (0x0000003Eu)  /*!<  TX Offset of 62 TDM Slots */

#define A2B_REG_I2STXOFFSET_RESET            (0x00000000u)  /*!<  Reset Value for I2STXOFFSET */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRXOFFSET                          Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SRXOFFSET_RXOFFSET        (0u)           /*!<  Serial RX Offset */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRXOFFSET                          Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2SRXOFFSET_RXOFFSET        (0x0000003Fu)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRXOFFSET                          Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_I2SRXOFFSET_00              (0x00000000u)  /*!<  No RX Offset */
#define A2B_ENUM_I2SRXOFFSET_62              (0x0000001Eu)  /*!<  62 TDM Slot Offset */
#define A2B_ENUM_I2SRXOFFSET_63              (0x0000001Fu)  /*!<  63 TDM Slot Offset */

#define A2B_REG_I2SRXOFFSET_RESET            (0x00000000u)  /*!<  Reset Value for I2SRXOFFSET */
#define A2B_REG_SYNCOFFSET_RESET             (0x00000000u)  /*!<  Reset Value for SYNCOFFSET */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PDMCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PDMCTL_PDMRATE              (5u)           /*!<  Highpass Filter Cutoff - AD242X only*/
#define A2B_BITP_PDMCTL_HPFCUTOFF            (5u)           /*!<  Highpass Filter Cutoff - AD241x only*/
#define A2B_BITP_PDMCTL_HPFEN                (4u)           /*!<  Highpass Filter Enable */
#define A2B_BITP_PDMCTL_PDM1SLOTS            (3u)           /*!<  PDM1 Slots */
#define A2B_BITP_PDMCTL_PDM1EN               (2u)           /*!<  PDM1 Enable */
#define A2B_BITP_PDMCTL_PDM0SLOTS            (1u)           /*!<  PDM0 Slots */
#define A2B_BITP_PDMCTL_PDM0EN               (0u)           /*!<  PDM0 Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PDMCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_PDMCTL_PDMRATE              (0x00000060u)  /*!<  AD242X only */
#define A2B_BITM_PDMCTL_HPFCUTOFF            (0x000000E0u)
#define A2B_BITM_PDMCTL_HPFEN                (0x00000010u)
#define A2B_BITM_PDMCTL_PDM1SLOTS            (0x00000008u)
#define A2B_BITM_PDMCTL_PDM1EN               (0x00000004u)
#define A2B_BITM_PDMCTL_PDM0SLOTS            (0x00000002u)
#define A2B_BITM_PDMCTL_PDM0EN               (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PDMCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_PDMCTL_SFR_1X               (0x00000000u)  /*!< PDM samples produced at Superframe rate - AD242X only */
#define A2B_ENUM_PDMCTL_SFR_DIV2             (0x00000020u)  /*!< PDM samples produced at 1/2 Superframe rate - AD242X only */
#define A2B_ENUM_PDMCTL_SFR_DIV4             (0x00000040u)  /*!< PDM samples produced at 1/4 Superframe rate - AD242X only */
#define A2B_ENUM_PDMCTL_0_93                 (0x000000C0u)  /*!<  0.93 Hz */
#define A2B_ENUM_PDMCTL_3_73                 (0x00000080u)  /*!<  3.73 Hz */
#define A2B_ENUM_PDMCTL_29_8                 (0x00000020u)  /*!<  29.8 Hz */
#define A2B_ENUM_PDMCTL_7_46                 (0x00000060u)  /*!<  7.46 Hz */
#define A2B_ENUM_PDMCTL_59_9                 (0x00000000u)  /*!<  59.9 Hz */
#define A2B_ENUM_PDMCTL_14_9                 (0x00000040u)  /*!<  14.9 Hz */
#define A2B_ENUM_PDMCTL_1_86                 (0x000000A0u)  /*!<  1.86 Hz */
#define A2B_ENUM_PDMCTL_HPFEN_EN             (0x00000010u)  /*!<  Enable Filter */
#define A2B_ENUM_PDMCTL_HPFEN_DIS            (0x00000000u)  /*!<  Disable Filter */
#define A2B_ENUM_PDMCTL_PDM1SLOTS_2          (0x00000008u)  /*!<  2 Slots */
#define A2B_ENUM_PDMCTL_PDM1SLOTS_1          (0x00000000u)  /*!<  1 Slot */
#define A2B_ENUM_PDMCTL_PDM1EN_EN            (0x00000004u)  /*!<  Enable PDM reception on the pin */
#define A2B_ENUM_PDMCTL_PDM1EN_DIS           (0x00000000u)  /*!<  Disable PDM reception on the pin. */
#define A2B_ENUM_PDMCTL_PDM0SLOTS_2          (0x00000002u)  /*!<  2 Slots */
#define A2B_ENUM_PDMCTL_PDM0SLOTS_1          (0x00000000u)  /*!<  1 Slot */
#define A2B_ENUM_PDMCTL_PDM0EN_EN            (0x00000001u)  /*!<  Enable PDM reception on the pin */
#define A2B_ENUM_PDMCTL_PDM0EN_DIS           (0x00000000u)  /*!<  Disable PDM reception on the pin */

#define A2B_REG_PDMCTL_RESET                 (0x00000000u)  /*!<  Reset Value for PDMCTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        ERRMGMT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_ERRMGMT_ERRSLOT             (2u)           /*!<  Add Error Indicating Channel to I2S/TDM Output */
#define A2B_BITP_ERRMGMT_ERRSIG              (1u)           /*!<  Show Data Error on Remaining Bits */
#define A2B_BITP_ERRMGMT_ERRLSB              (0u)           /*!<  Show Data Error on LSB */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        ERRMGMT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_ERRMGMT_ERRSLOT             (0x00000004u)
#define A2B_BITM_ERRMGMT_ERRSIG              (0x00000002u)
#define A2B_BITM_ERRMGMT_ERRLSB              (0x00000001u)

#define A2B_REG_ERRMGMT_RESET                (0x00000000u)  /*!<  Reset Value for ERRMGMT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STEST                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2STEST_BUSLOOPBK           (4u)           /*!<  Bus Loopback - AD242X only */
#define A2B_BITP_I2STEST_SELRX1              (3u)           /*!<  Select RX1 Block to Loopback Buffer */
#define A2B_BITP_I2STEST_RX2LOOPBK           (2u)           /*!<  RX Block to Loopback Buffer */
#define A2B_BITP_I2STEST_LOOPBK2TX           (1u)           /*!<  Loopback Data to TX Blocks */
#define A2B_BITP_I2STEST_PATTRN2TX           (0u)           /*!<  Default Bit Pattern to Serial TX Blocks */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STEST                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_I2STEST_EXTLOOPBK           (0x000000C0u)  /*!< AD243X only */
#define A2B_BITM_I2STEST_BUSLOOPBK           (0x00000010u)  /*!< AD242X only */
#define A2B_BITM_I2STEST_SELRX1              (0x00000008u)
#define A2B_BITM_I2STEST_RX2LOOPBK           (0x00000004u)
#define A2B_BITM_I2STEST_LOOPBK2TX           (0x00000002u)
#define A2B_BITM_I2STEST_PATTRN2TX           (0x00000001u)

#define A2B_REG_I2STEST_RESET                (0x00000000u)  /*!<  Reset Value for I2STEST */
/* ============================================================================================================================ */
/** \name   ID Register Field Definitions */
/* ============================================================================================================================ */
#define A2B_REG_VENDOR_RESET                (0x000000ADu)   /*!<  Reset Value for VENDOR */
#define A2B_REG_PRODUCT_RESET_24XX          (0x00000010u)   /*!<  Reset Value for PRODUCT */
#define A2B_REG_VERSION_RESET_24XX          (0x00000010u)   /*!<  Reset Value for VERSION */
#define A2B_REG_PRODUCT_RESET_242X          (0x00000025u)   /*!<  Reset Value for PRODUCT */
#define A2B_REG_VERSION_RESET_242X          (0x00000000u)   /*!<  Reset Value for VERSION */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CAPABILITY                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CAPABILITY_MINFO            (7u)           /*!<  On-Chip Module Info Available */
#define A2B_BITP_CAPABILITY_SPIAVAIL         (1u)           /*!<  Module Info Available over SPI */
#define A2B_BITP_CAPABILITY_I2CAVAIL         (0u)           /*!<  I2C Interface Available */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CAPABILITY                           Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_CAPABILITY_MINFO            (0x00000080u)
#define A2B_BITM_CAPABILITY_SPIAVAIL         (0x00000002u)
#define A2B_BITM_CAPABILITY_I2CAVAIL         (0x00000001u)

#define A2B_REG_CAPABILITY_RESET             (0x00000001u)  /*!<  Reset Value for CAPABILITY */
/* ============================================================================================================================ */
/** \name   Interrupt and Error Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSTAT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_INTSTAT_IRQ                 (0u)           /*!<  Interrupt Currently Asserted */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSTAT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_INTSTAT_IRQ                 (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSTAT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_INTSTAT_IRQ_HIGH            (0x00000001u)  /*!<  Interrupt request */
#define A2B_ENUM_INTSTAT_IRQ_LOW             (0x00000000u)  /*!<  No Interrupt request */

#define A2B_REG_INTSTAT_RESET                (0x00000000u)  /*!<  Reset Value for INTSTAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSRC                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_INTSRC_MSTINT               (7u)           /*!<  Master Interrupt */
#define A2B_BITP_INTSRC_SLVINT               (6u)           /*!<  Slave Interrupt */
#define A2B_BITP_INTSRC_INODE                (0u)           /*!<  ID for SLVINT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSRC                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_INTSRC_MSTINT               (0x00000080u)
#define A2B_BITM_INTSRC_SLVINT               (0x00000040u)
#define A2B_BITM_INTSRC_INODE                (0x0000000Fu)

#define A2B_REG_INTSRC_RESET                 (0x00000000u)  /*!<  Reset Value for INTSRC */
#define A2B_REG_INTTYPE_RESET                (0x00000000u)  /*!<  Reset Value for INTTYPE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTTYPE                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_INTTYPE_HDCNTERR             (0u)          /*!< HDCNTERR */
#define A2B_ENUM_INTTYPE_DDERR                (1u)          /*!< DDERR */
#define A2B_ENUM_INTTYPE_CRCERR               (2u)          /*!< CRCERR */
#define A2B_ENUM_INTTYPE_DPERR                (3u)          /*!< DPERR */
#define A2B_ENUM_INTTYPE_BECOVF               (4u)          /*!< BECOVF */
#define A2B_ENUM_INTTYPE_SRFERR               (5u)          /*!< SRFERR */
#define A2B_ENUM_INTTYPE_PWRERR_CS_GND        (9u)          /*!< PWRERR (Cable Shorted to GND) */
#define A2B_ENUM_INTTYPE_PWRERR_CS_VBAT       (10u)         /*!< PWRERR (Cable Shorted to VBat) */
#define A2B_ENUM_INTTYPE_PWRERR_CS            (11u)         /*!< PWRERR (Cable Shorted Together) */
#define A2B_ENUM_INTTYPE_PWRERR_CDISC         (12u)         /*!< PWRERR (Cable Disconnected or Open Circuit) (AD240x/10/2x Slaves Only) */
#define A2B_ENUM_INTTYPE_PWRERR_CREV          (13u)         /*!< PWRERR (Cable Reverse Connected) (AD240x/10/2x Slaves Only) */
#define A2B_ENUM_INTTYPE_PWRERR_CDISC_REV     (14u)         /*!< PWRERR - Cable is Disconnected (Open Circuit) or Wrong Port or Reverse Connected (AD243x Only) */
#define A2B_ENUM_INTTYPE_PWRERR_FAULT         (15u)         /*!< PWRERR (Indeterminate Fault) */
#define A2B_ENUM_INTTYPE_IO0PND               (16u)         /*!< IO0PND - Slave Only */
#define A2B_ENUM_INTTYPE_IO1PND               (17u)         /*!< IO1PND - Slave Only */
#define A2B_ENUM_INTTYPE_IO2PND               (18u)         /*!< IO2PND - Slave Only */
#define A2B_ENUM_INTTYPE_IO3PND               (19u)         /*!< IO3PND */
#define A2B_ENUM_INTTYPE_IO4PND               (20u)         /*!< IO4PND */
#define A2B_ENUM_INTTYPE_IO5PND               (21u)         /*!< IO5PND */
#define A2B_ENUM_INTTYPE_IO6PND               (22u)         /*!< IO6PND */
#define A2B_ENUM_INTTYPE_IO7PND               (23u)         /*!< IO7PND */
#define A2B_ENUM_INTTYPE_DSCDONE              (24u)         /*!< DSCDONE - Master Only */
#define A2B_ENUM_INTTYPE_I2CERR               (25u)         /*!< I2CERR - Master Only */
#define A2B_ENUM_INTTYPE_ICRCERR              (26u)         /*!< ICRCERR - Master Only */
#define A2B_ENUM_INTTYPE_PWRERR_NLS_GND       (41u)         /*!< PWRERR - Non-Localized Short to GND */
#define A2B_ENUM_INTTYPE_PWRERR_NLS_VBAT      (42u)         /*!< PWRERR - Non-Localized Short to VBat */
#define A2B_ENUM_INTTYPE_PWRERR_OTH           (47u)         /*!< PWRERR - Other Error, Check SWSTAT2/SWSTAT3.*/
#define A2B_ENUM_INTTYPE_MBOX0_FULL			  (48u)			/*!< Mailbox 0 full */
#define A2B_ENUM_INTTYPE_MBOX0_EMPTY		  (49u)			/*!< Mailbox 0 empty */
#define A2B_ENUM_INTTYPE_MBOX1_FULL			  (50u)			/*!< Mailbox 1 full */
#define A2B_ENUM_INTTYPE_MBOX1_EMPTY		  (51u)			/*!< Mailbox 1 empty */
#define A2B_ENUM_INTTYPE_SPIDONE              (64u)         /*!< SPI Done*/
#define A2B_ENUM_INTTYPE_SPI_REMOTE_REG_ERR   (65u)         /*!< SPI Remote Register Access Error - Master Only*/
#define A2B_ENUM_INTTYPE_SPI_REMOTE_I2C_ERR   (66u)         /*!< SPI Remote I2C Access Error - Master Only*/
#define A2B_ENUM_INTTYPE_SPI_DATA_TUN_ERR     (67u)         /*!< SPI Data Tunnel Access Error*/
#define A2B_ENUM_INTTYPE_SPI_BAD_CMD          (68u)         /*!< SPI Bad Command*/
#define A2B_ENUM_INTTYPE_SPI_FIFO_OVRFLW      (69u)         /*!< SPI FIFO Overflow*/
#define A2B_ENUM_INTTYPE_SPI_FIFO_UNDERFLW    (70u)         /*!< SPI FIFO Underflow*/
#define A2B_ENUM_INTTYPE_VMTR                 (80u)         /*!< VMTR Interrupt*/
#define A2B_ENUM_INTTYPE_IRPT_MSG_ERR		  (128u)		/*!< PWRERR - Interrupt Messaging Error */
#define A2B_ENUM_INTTYPE_STRTUP_ERR_RTF		  (252u)		/*!< Startup Error - Return to Factory */
#define A2B_ENUM_INTTYPE_SLAVE_INTTYPE_ERR    (253u)        /*!< Slave INTTYPE Read Error - Master Only */
#define A2B_ENUM_INTTYPE_STANDBY_DONE         (254u)        /*!< Standby Done - Master Only */
#define A2B_ENUM_INTTYPE_MSTR_RUNNING         (255u)        /*!< MSTR_RUNNING - Master Only */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND0                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_INTPND0_SRFCRCERR           (7u)           /*!<  SRF CRC Error - AD242X slave only */
#define A2B_BITP_INTPND0_SRFERR              (6u)           /*!<  Missed SRF */
#define A2B_BITP_INTPND0_BECOVF              (5u)           /*!<  Bit Error Count error */
#define A2B_BITP_INTPND0_PWRERR              (4u)           /*!<  Switch reporting error on downstream power */
#define A2B_BITP_INTPND0_DPERR               (3u)           /*!<  Data Parity Error */
#define A2B_BITP_INTPND0_CRCERR              (2u)           /*!<  CRC Error */
#define A2B_BITP_INTPND0_DDERR               (1u)           /*!<  Data Decoding Error */
#define A2B_BITP_INTPND0_HDCNTERR            (0u)           /*!<  Header Count Error */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND0                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_INTPND0_SRFCRCERR           (0x00000080u)
#define A2B_BITM_INTPND0_SRFERR              (0x00000040u)
#define A2B_BITM_INTPND0_BECOVF              (0x00000020u)
#define A2B_BITM_INTPND0_PWRERR              (0x00000010u)
#define A2B_BITM_INTPND0_DPERR               (0x00000008u)
#define A2B_BITM_INTPND0_CRCERR              (0x00000004u)
#define A2B_BITM_INTPND0_DDERR               (0x00000002u)
#define A2B_BITM_INTPND0_HDCNTERR            (0x00000001u)

#define A2B_REG_INTPND0_RESET                (0x00000000u)  /*!<  Reset Value for INTPND0 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND1                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_INTPND1_IO7PND              (7u)           /*!<  IO7 Int Pending - AD242X only */
#define A2B_BITP_INTPND1_IO6PND              (6u)           /*!<  IO6 Int Pending */
#define A2B_BITP_INTPND1_IO5PND              (5u)           /*!<  IO5 Int Pending */
#define A2B_BITP_INTPND1_IO4PND              (4u)           /*!<  IO4 Int Pending */
#define A2B_BITP_INTPND1_IO3PND              (3u)           /*!<  IO3 Int Pending */
#define A2B_BITP_INTPND1_IO2PND              (2u)           /*!<  IO2 Int Pending */
#define A2B_BITP_INTPND1_IO1PND              (1u)           /*!<  IO1 Int Pending */
#define A2B_BITP_INTPND1_IO0PND              (0u)           /*!<  IO0 Int Pending (Slave Only) */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND1                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_INTPND1_IO7PND              (0x00000080u)
#define A2B_BITM_INTPND1_IO6PND              (0x00000040u)
#define A2B_BITM_INTPND1_IO5PND              (0x00000020u)
#define A2B_BITM_INTPND1_IO4PND              (0x00000010u)
#define A2B_BITM_INTPND1_IO3PND              (0x00000008u)
#define A2B_BITM_INTPND1_IO2PND              (0x00000004u)
#define A2B_BITM_INTPND1_IO1PND              (0x00000002u)
#define A2B_BITM_INTPND1_IO0PND              (0x00000001u)

#define A2B_REG_INTPND1_RESET                (0x00000000u)   /*!<  Reset Value for INTPND1 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND2                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_INTPND2_SLVIRQ              (3u)           /*!<  Slave Interrupt Received Master Only */
#define A2B_BITP_INTPND2_ICRCERR             (2u)           /*!<  Int Frame CRC Error Master Only */
#define A2B_BITP_INTPND2_I2CERR              (1u)           /*!<  I2C Transaction Error Master Only */
#define A2B_BITP_INTPND2_DSCDONE             (0u)           /*!<  Node Discovered Master Only */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND2                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_INTPND2_SLVIRQ              (0x00000008u)
#define A2B_BITM_INTPND2_ICRCERR             (0x00000004u)
#define A2B_BITM_INTPND2_I2CERR              (0x00000002u)
#define A2B_BITM_INTPND2_DSCDONE             (0x00000001u)

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTMSK2                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_INTMSK2_SLVIRQ              (0x00000008u)
#define A2B_BITM_INTMSK2_ICRCERR             (0x00000004u)
#define A2B_BITM_INTMSK2_I2CERR              (0x00000002u)
#define A2B_BITM_INTMSK2_DSCDONE             (0x00000001u)

#define A2B_BITM_INTMSK1_IO6PND              (0x00000040u)
#define A2B_BITM_INTMSK1_IO5PND              (0x00000020u)
#define A2B_BITM_INTMSK1_IO4PND              (0x00000010u)
#define A2B_BITM_INTMSK1_IO3PND              (0x00000008u)
#define A2B_BITM_INTMSK1_IO2PND              (0x00000004u)
#define A2B_BITM_INTMSK1_IO1PND              (0x00000002u)
#define A2B_BITM_INTMSK1_IO0PND              (0x00000001u)

#define A2B_REG_INTPND2_RESET                (0x00000000u)   /*!<  Reset Value for INTPND2 */
#define A2B_REG_INTMSK0_RESET                (0x00000000u)   /*!<  Reset Value for INTMSK0 */
#define A2B_REG_INTMSK1_RESET                (0x00000000u)   /*!<  Reset Value for INTMSK1 */
#define A2B_REG_INTMSK2_RESET                (0x00000000u)   /*!<  Reset Value for INTMSK2 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BECCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_BECCTL_THRESHLD             (5u)           /*!<  Threshold to Generate an Interrupt */
#define A2B_BITP_BECCTL_ENICRC               (4u)           /*!<  Enable ICRCERR count */
#define A2B_BITP_BECCTL_ENDP                 (3u)           /*!<  Enable DPERR count */
#define A2B_BITP_BECCTL_ENCRC                (2u)           /*!<  Enable CRCERR count */
#define A2B_BITP_BECCTL_ENDD                 (1u)           /*!<  Enable DDERR count */
#define A2B_BITP_BECCTL_ENHDCNT              (0u)           /*!<  Enable HDCNTERR count */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BECCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_BECCTL_THRESHLD             (0x000000E0u)
#define A2B_BITM_BECCTL_ENICRC               (0x00000010u)
#define A2B_BITM_BECCTL_ENDP                 (0x00000008u)
#define A2B_BITM_BECCTL_ENCRC                (0x00000004u)
#define A2B_BITM_BECCTL_ENDD                 (0x00000002u)
#define A2B_BITM_BECCTL_ENHDCNT              (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BECCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_BECCTL_THRESHLD_128         (0x000000C0u)  /*!<  Interrupt After 128 Errors */
#define A2B_ENUM_BECCTL_THRESHLD_32          (0x00000080u)  /*!<  Interrupt After 32 Errors */
#define A2B_ENUM_BECCTL_THRESHLD_4           (0x00000020u)  /*!<  Interrupt After 4 Errors */
#define A2B_ENUM_BECCTL_THRESHLD_16          (0x00000060u)  /*!<  Interrupt After 16 Errors */
#define A2B_ENUM_BECCTL_THRESHLD_2           (0x00000000u)  /*!<  Interrupt After 2 Errors */
#define A2B_ENUM_BECCTL_THRESHLD_256         (0x000000E0u)  /*!<  Interrupt After 256 Errors */
#define A2B_ENUM_BECCTL_THRESHLD_8           (0x00000040u)  /*!<  Interrupt After 8 Errors */
#define A2B_ENUM_BECCTL_THRESHLD_64          (0x000000A0u)  /*!<  Interrupt After 64 Errors */
#define A2B_ENUM_BECCTL_ENICRC_EN            (0x00000010u)  /*!<  Enable Bit Error Counting */
#define A2B_ENUM_BECCTL_ENICRC_DIS           (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_BECCTL_ENDP_EN              (0x00000008u)  /*!<  Parity Error */
#define A2B_ENUM_BECCTL_ENDP_DIS             (0x00000000u)  /*!<  No Parity error */
#define A2B_ENUM_BECCTL_ENCRC_EN             (0x00000004u)  /*!<  CRC Error */
#define A2B_ENUM_BECCTL_ENCRC_DIS            (0x00000000u)  /*!<  No CRC Error */
#define A2B_ENUM_BECCTL_ENDD_EN              (0x00000002u)  /*!<  Enabled */
#define A2B_ENUM_BECCTL_ENDD_DIS             (0x00000000u)  /*!<  Disabled */
#define A2B_ENUM_BECCTL_ENHDCNT_EN           (0x00000001u)  /*!<  Enabled */
#define A2B_ENUM_BECCTL_ENHDCNT_DIS          (0x00000000u)  /*!<  Disabled */

#define A2B_REG_BECCTL_RESET                 (0x00000000u)  /*!<  Reset Value for BECCTL */
#define A2B_REG_BECNT_RESET                  (0x00000000u)  /*!<  Reset Value for BECNT */
#define A2B_REG_RAISE_RESET                  (0x00000000u)  /*!<  Reset Value for RAISE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GENERR                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GENERR_GENICRCERR           (4u)           /*!<  Generate Int Frame CRC Error (Slave Only) */
#define A2B_BITP_GENERR_GENDPERR             (3u)           /*!<  Generate Data Parity Error */
#define A2B_BITP_GENERR_GENCRCERR            (2u)           /*!<  Generate CRC Error */
#define A2B_BITP_GENERR_GENDDERR             (1u)           /*!<  Generate Data Decoding Error */
#define A2B_BITP_GENERR_GENHCERR             (0u)           /*!<  Generate Header Count Error */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GENERR                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GENERR_GENICRCERR           (0x00000010u)
#define A2B_BITM_GENERR_GENDPERR             (0x00000008u)
#define A2B_BITM_GENERR_GENCRCERR            (0x00000004u)
#define A2B_BITM_GENERR_GENDDERR             (0x00000002u)
#define A2B_BITM_GENERR_GENHCERR             (0x00000001u)

#define A2B_REG_GENERR_RESET                 (0x00000000u)  /*!<  Reset Value for GENERR */
/* ============================================================================================================================ */
/** \name   PRBS Test Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TESTMODE                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TESTMODE_RXDPTH             (4u)           /*!<  RX Fifo Depth */
#define A2B_BITP_TESTMODE_PRBSN2N            (2u)           /*!<  PRBS N2N Mode */
#define A2B_BITP_TESTMODE_PRBSDN             (1u)           /*!<  PRBS Data Downstream */
#define A2B_BITP_TESTMODE_PRBSUP             (0u)           /*!<  PRBS Data Upstream */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TESTMODE                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_TESTMODE_RXDPTH             (0x00000070u)
#define A2B_BITM_TESTMODE_PRBSN2N            (0x00000004u)
#define A2B_BITM_TESTMODE_PRBSDN             (0x00000002u)
#define A2B_BITM_TESTMODE_PRBSUP             (0x00000001u)

#define A2B_REG_TESTMODE_RESET               (0x00000000u)  /*!<  Reset Value for TESTMODE */
#define A2B_REG_ERRCNT0_RESET                (0x00000000u)  /*!<  Reset Value for ERRCNT0 */
#define A2B_REG_ERRCNT1_RESET                (0x00000000u)  /*!<  Reset Value for ERRCNT1 */
#define A2B_REG_ERRCNT2_RESET                (0x00000000u)  /*!<  Reset Value for ERRCNT2 */
#define A2B_REG_ERRCNT3_RESET                (0x00000000u)  /*!<  Reset Value for ERRCNT3 */
#define A2B_REG_SEED0_RESET                  (0x00000079u)  /*!<  Reset Value for SEED0 */
#define A2B_REG_SEED1_RESET                  (0x00000000u)  /*!<  Reset Value for SEED1 */
#define A2B_REG_SEED2_RESET                  (0x00000000u)  /*!<  Reset Value for SEED2 */
#define A2B_REG_SEED3_RESET                  (0x00000000u)  /*!<  Reset Value for SEED3 */
/* ============================================================================================================================ */
/** \name   Pin IO and Interrupt Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CLKCFG_CCLKREXT             (4u)           /*!<  Output Clock Rate Extension (Master Only) */
#define A2B_BITP_CLKCFG_CCLKRATE             (1u)           /*!<  Output Clock Rate */
#define A2B_BITP_CLKCFG_CCLKEN               (0u)           /*!<  Enable Output Clock */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_CLKCFG_CCLKREXT             (0x00000070u)
#define A2B_BITM_CLKCFG_CCLKRATE             (0x00000002u)
#define A2B_BITM_CLKCFG_CCLKEN               (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKCFG                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_CLKCFG_CLKOUT_DIV512        (0x00000060u)  /*!<  CLKOUT = pllclk/512 */
#define A2B_ENUM_CLKCFG_CLKOUT_DIV128        (0x00000040u)  /*!<  CLKOUT = pllclk/128 */
#define A2B_ENUM_CLKCFG_CLKOUT_DIV16         (0x00000010u)  /*!<  CLKOUT = pllclk/16 */
#define A2B_ENUM_CLKCFG_CLKOUT_DIV64         (0x00000030u)  /*!<  CLKOUT = pllclk/64 */
#define A2B_ENUM_CLKCFG_USE_CCLKRATE         (0x00000000u)  /*!<  Use CCLKRATE */
#define A2B_ENUM_CLKCFG_CLKOUT_DIV1024       (0x00000070u)  /*!<  CLKOUT = pllclk/1024 */
#define A2B_ENUM_CLKCFG_CLKOUT_DIV32         (0x00000020u)  /*!<  CLKOUT = pllclk/32 */
#define A2B_ENUM_CLKCFG_CLKOUT_DIV256        (0x00000050u)  /*!<  CLKOUT = pllclk/256 */

#define A2B_REG_CLKCFG_RESET                 (0x00000000u)             /*      Reset Value for CLKCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODAT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIODAT_IO7DAT              (7u)           /*!<  IO7 Output Data - AD242X only */
#define A2B_BITP_GPIODAT_IO6DAT              (6u)           /*!<  IO6 Output Data */
#define A2B_BITP_GPIODAT_IO5DAT              (5u)           /*!<  IO5 Output Data */
#define A2B_BITP_GPIODAT_IO4DAT              (4u)           /*!<  IO4 Output Data */
#define A2B_BITP_GPIODAT_IO3DAT              (3u)           /*!<  IO3 Output Data */
#define A2B_BITP_GPIODAT_IO2DAT              (2u)           /*!<  IO2 Output Data */
#define A2B_BITP_GPIODAT_IO1DAT              (1u)           /*!<  IO1 Output Data */
#define A2B_BITP_GPIODAT_IO0DAT              (0u)           /*!<  IO0 Output Data */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODAT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIODAT_IO7DAT              (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_GPIODAT_IO6DAT              (0x00000040u)
#define A2B_BITM_GPIODAT_IO5DAT              (0x00000020u)
#define A2B_BITM_GPIODAT_IO4DAT              (0x00000010u)
#define A2B_BITM_GPIODAT_IO3DAT              (0x00000008u)
#define A2B_BITM_GPIODAT_IO2DAT              (0x00000004u)
#define A2B_BITM_GPIODAT_IO1DAT              (0x00000002u)
#define A2B_BITM_GPIODAT_IO0DAT              (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODAT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_GPIODAT_IO7_HIGH            (0x00000080u)  /*!<  Output High - AD242X only */
#define A2B_ENUM_GPIODAT_IO7_LOW             (0x00000000u)  /*!<  Output Low - AD242X only */
#define A2B_ENUM_GPIODAT_IO6_HIGH            (0x00000040u)  /*!<  Output High */
#define A2B_ENUM_GPIODAT_IO6_LOW             (0x00000000u)  /*!<  Output Low */
#define A2B_ENUM_GPIODAT_IO5_HIGH            (0x00000020u)  /*!<  Output High */
#define A2B_ENUM_GPIODAT_IO5_LOW             (0x00000000u)  /*!<  Output Low */
#define A2B_ENUM_GPIODAT_IO4_HIGH            (0x00000010u)  /*!<  Output High */
#define A2B_ENUM_GPIODAT_IO4_LOW             (0x00000000u)  /*!<  Output Low */
#define A2B_ENUM_GPIODAT_IO3_HIGH            (0x00000008u)  /*!<  Output High */
#define A2B_ENUM_GPIODAT_IO3_LOW             (0x00000000u)  /*!<  Output Low */
#define A2B_ENUM_GPIODAT_IO2_HIGH            (0x00000004u)  /*!<  Output High */
#define A2B_ENUM_GPIODAT_IO2_LOW             (0x00000000u)  /*!<  Output Low */
#define A2B_ENUM_GPIODAT_IO1_HIGH            (0x00000002u)  /*!<  Output High */
#define A2B_ENUM_GPIODAT_IO1_LOW             (0x00000000u)  /*!<  Output Low */
#define A2B_ENUM_GPIODAT_IO0_HIGH            (0x00000001u)  /*!<  Output High */
#define A2B_ENUM_GPIODAT_IO0_LOW             (0x00000000u)  /*!<  Output Low */

#define A2B_REG_GPIODAT_RESET                (0x00000000u)  /*!<  Reset Value for GPIODAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATSET                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIODATSET_IO7DSET          (7u)           /*!<  Write 1 to set IO7DAT - AD242X only */
#define A2B_BITP_GPIODATSET_IO6DSET          (6u)           /*!<  Write 1 to set IO6DAT */
#define A2B_BITP_GPIODATSET_IO5DSET          (5u)           /*!<  Write 1 to set IO5DAT */
#define A2B_BITP_GPIODATSET_IO4DSET          (4u)           /*!<  Write 1 to set IO4DAT */
#define A2B_BITP_GPIODATSET_IO3DSET          (3u)           /*!<  Write 1 to set IO3DAT */
#define A2B_BITP_GPIODATSET_IO2DSET          (2u)           /*!<  Write 1 to set IO2DAT */
#define A2B_BITP_GPIODATSET_IO1DSET          (1u)           /*!<  Write 1 to set IO1DAT */
#define A2B_BITP_GPIODATSET_IO0DSET          (0u)           /*!<  Write 1 to set IO0DAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATSET                           Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIODATSET_IO7DSET          (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_GPIODATSET_IO6DSET          (0x00000040u)
#define A2B_BITM_GPIODATSET_IO5DSET          (0x00000020u)
#define A2B_BITM_GPIODATSET_IO4DSET          (0x00000010u)
#define A2B_BITM_GPIODATSET_IO3DSET          (0x00000008u)
#define A2B_BITM_GPIODATSET_IO2DSET          (0x00000004u)
#define A2B_BITM_GPIODATSET_IO1DSET          (0x00000002u)
#define A2B_BITM_GPIODATSET_IO0DSET          (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATSET                           Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_GPIODATSET_IO7_SET          (0x00000080u)  /*!<  Set Bit - AD242X only */
#define A2B_ENUM_GPIODATSET_IO7_NO_ACTION    (0x00000000u)  /*!<  No Action - AD242X only */
#define A2B_ENUM_GPIODATSET_IO6_SET          (0x00000040u)  /*!<  Set Bit */
#define A2B_ENUM_GPIODATSET_IO6_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATSET_IO5_SET          (0x00000020u)  /*!<  Set Bit */
#define A2B_ENUM_GPIODATSET_IO5_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATSET_IO4_SET          (0x00000010u)  /*!<  Set Bit */
#define A2B_ENUM_GPIODATSET_IO4_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATSET_IO3_SET          (0x00000008u)  /*!<  Set Bit */
#define A2B_ENUM_GPIODATSET_IO3_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATSET_IO2_SET          (0x00000004u)  /*!<  Set Bit */
#define A2B_ENUM_GPIODATSET_IO2_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATSET_IO1_SET          (0x00000002u)  /*!<  Set Bit */
#define A2B_ENUM_GPIODATSET_IO1_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATSET_IO0_SET          (0x00000001u)  /*!<  Set Bit */
#define A2B_ENUM_GPIODATSET_IO0_NO_ACTION    (0x00000000u)  /*!<  No Action */

#define A2B_REG_GPIODATSET_RESET             (0x00000000u)  /*!<  Reset Value for GPIODATSET */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATCLR                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIODATCLR_IO7DCLR          (7u)           /*!<  Write 1 to clear IO7DAT - AD242X only */
#define A2B_BITP_GPIODATCLR_IO6DCLR          (6u)           /*!<  Write 1 to clear IO6DAT */
#define A2B_BITP_GPIODATCLR_IO5DCLR          (5u)           /*!<  Write 1 to clear IO5DAT */
#define A2B_BITP_GPIODATCLR_IO4DCLR          (4u)           /*!<  Write 1 to clear IO4DAT */
#define A2B_BITP_GPIODATCLR_IO3DCLR          (3u)           /*!<  Write 1 to clear IO3DAT */
#define A2B_BITP_GPIODATCLR_IO2DCLR          (2u)           /*!<  Write 1 to clear IO2DAT */
#define A2B_BITP_GPIODATCLR_IO1DCLR          (1u)           /*!<  Write 1 to clear IO1DAT */
#define A2B_BITP_GPIODATCLR_IO0DCLR          (0u)           /*!<  Write 1 to clear IO0DAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATCLR                           Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIODATCLR_IO7DCLR          (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_GPIODATCLR_IO6DCLR          (0x00000040u)
#define A2B_BITM_GPIODATCLR_IO5DCLR          (0x00000020u)
#define A2B_BITM_GPIODATCLR_IO4DCLR          (0x00000010u)
#define A2B_BITM_GPIODATCLR_IO3DCLR          (0x00000008u)
#define A2B_BITM_GPIODATCLR_IO2DCLR          (0x00000004u)
#define A2B_BITM_GPIODATCLR_IO1DCLR          (0x00000002u)
#define A2B_BITM_GPIODATCLR_IO0DCLR          (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATCLR                           Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_GPIODATCLR_IO7_CLEAR        (0x00000080u)  /*!<  Clear Bit - AD242X only */
#define A2B_ENUM_GPIODATCLR_IO7_NO_ACTION    (0x00000000u)  /*!<  No Action - AD242X only */
#define A2B_ENUM_GPIODATCLR_IO6_CLEAR        (0x00000040u)  /*!<  Clear Bit */
#define A2B_ENUM_GPIODATCLR_IO6_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATCLR_IO5_CLEAR        (0x00000020u)  /*!<  Clear Bit */
#define A2B_ENUM_GPIODATCLR_IO5_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATCLR_IO4_CLEAR        (0x00000010u)  /*!<  Clear Bit */
#define A2B_ENUM_GPIODATCLR_IO4_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATCLR_IO3_CLEAR        (0x00000008u)  /*!<  Clear Bit */
#define A2B_ENUM_GPIODATCLR_IO3_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATCLR_IO2_CLEAR        (0x00000004u)  /*!<  Clear Bit */
#define A2B_ENUM_GPIODATCLR_IO2_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATCLR_IO1_CLEAR        (0x00000002u)  /*!<  Clear Bit */
#define A2B_ENUM_GPIODATCLR_IO1_NO_ACTION    (0x00000000u)  /*!<  No Action */
#define A2B_ENUM_GPIODATCLR_IO0_CLEAR        (0x00000001u)  /*!<  Clear Bit */
#define A2B_ENUM_GPIODATCLR_IO0_NO_ACTION    (0x00000000u)  /*!<  No Action */

#define A2B_REG_GPIODATCLR_RESET             (0x00000000u)  /*!<  Reset Value for GPIODATCLR */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOOEN                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOOEN_IO7OEN              (7u)           /*!<  IO7 Output Enable - AD242X only */
#define A2B_BITP_GPIOOEN_IO6OEN              (6u)           /*!<  IO6 Output Enable */
#define A2B_BITP_GPIOOEN_IO5OEN              (5u)           /*!<  IO5 Output Enable */
#define A2B_BITP_GPIOOEN_IO4OEN              (4u)           /*!<  IO4 Output Enable */
#define A2B_BITP_GPIOOEN_IO3OEN              (3u)           /*!<  IO3 Output Enable */
#define A2B_BITP_GPIOOEN_IO2OEN              (2u)           /*!<  IO2 Output Enable */
#define A2B_BITP_GPIOOEN_IO1OEN              (1u)           /*!<  IO1 Output Enable */
#define A2B_BITP_GPIOOEN_IO0OEN              (0u)           /*!<  IO0 Output Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOOEN                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOOEN_IO7OEN              (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_GPIOOEN_IO6OEN              (0x00000040u)
#define A2B_BITM_GPIOOEN_IO5OEN              (0x00000020u)
#define A2B_BITM_GPIOOEN_IO4OEN              (0x00000010u)
#define A2B_BITM_GPIOOEN_IO3OEN              (0x00000008u)
#define A2B_BITM_GPIOOEN_IO2OEN              (0x00000004u)
#define A2B_BITM_GPIOOEN_IO1OEN              (0x00000002u)
#define A2B_BITM_GPIOOEN_IO0OEN              (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOOEN                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_GPIOOEN_IO7_EN              (0x00000080u)  /*!<  Enable - AD242X only */
#define A2B_ENUM_GPIOOEN_IO7_DIS             (0x00000000u)  /*!<  Disable - AD242X only */
#define A2B_ENUM_GPIOOEN_IO6_EN              (0x00000040u)  /*!<  Enable */
#define A2B_ENUM_GPIOOEN_IO6_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOOEN_IO5_EN              (0x00000020u)  /*!<  Enable */
#define A2B_ENUM_GPIOOEN_IO5_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOOEN_IO4_EN              (0x00000010u)  /*!<  Enable */
#define A2B_ENUM_GPIOOEN_IO4_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOOEN_IO3_EN              (0x00000008u)  /*!<  Enable */
#define A2B_ENUM_GPIOOEN_IO3_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOOEN_IO2_EN              (0x00000004u)  /*!<  Enable */
#define A2B_ENUM_GPIOOEN_IO2_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOOEN_IO1_EN              (0x00000002u)  /*!<  Enable */
#define A2B_ENUM_GPIOOEN_IO1_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOOEN_IO0_EN              (0x00000001u)  /*!<  Enable */
#define A2B_ENUM_GPIOOEN_IO0_DIS             (0x00000000u)  /*!<  Disable */

#define A2B_REG_GPIOOEN_RESET                (0x00000000u)  /*!<  Reset Value for GPIOOEN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIEN                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOIEN_IO7IEN              (7u)           /*!<  IO7 Input Enable - AD242X only */
#define A2B_BITP_GPIOIEN_IO6IEN              (6u)           /*!<  IO6 Input Enable */
#define A2B_BITP_GPIOIEN_IO5IEN              (5u)           /*!<  IO5 Input Enable */
#define A2B_BITP_GPIOIEN_IO4IEN              (4u)           /*!<  IO4 Input Enable */
#define A2B_BITP_GPIOIEN_IO3IEN              (3u)           /*!<  IO3 Input Enable */
#define A2B_BITP_GPIOIEN_IO2IEN              (2u)           /*!<  IO2 Input Enable */
#define A2B_BITP_GPIOIEN_IO1IEN              (1u)           /*!<  IO1 Input Enable */
#define A2B_BITP_GPIOIEN_IO0IEN              (0u)           /*!<  IO0 Input Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIEN                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOIEN_IO7IEN              (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_GPIOIEN_IO6IEN              (0x00000040u)
#define A2B_BITM_GPIOIEN_IO5IEN              (0x00000020u)
#define A2B_BITM_GPIOIEN_IO4IEN              (0x00000010u)
#define A2B_BITM_GPIOIEN_IO3IEN              (0x00000008u)
#define A2B_BITM_GPIOIEN_IO2IEN              (0x00000004u)
#define A2B_BITM_GPIOIEN_IO1IEN              (0x00000002u)
#define A2B_BITM_GPIOIEN_IO0IEN              (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIEN                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_GPIOIEN_IO7_EN              (0x00000080u)  /*!<  Enable - AD242X only */
#define A2B_ENUM_GPIOIEN_IO7_DIS             (0x00000000u)  /*!<  Disable - AD242X only */
#define A2B_ENUM_GPIOIEN_IO6_EN              (0x00000040u)  /*!<  Enable */
#define A2B_ENUM_GPIOIEN_IO6_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOIEN_IO5_EN              (0x00000020u)  /*!<  Enable */
#define A2B_ENUM_GPIOIEN_IO5_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOIEN_IO4_EN              (0x00000010u)  /*!<  Enable */
#define A2B_ENUM_GPIOIEN_IO4_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOIEN_IO3_EN              (0x00000008u)  /*!<  Enable */
#define A2B_ENUM_GPIOIEN_IO3_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOIEN_IO2_EN              (0x00000004u)  /*!<  Enable */
#define A2B_ENUM_GPIOIEN_IO2_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOIEN_IO1_EN              (0x00000002u)  /*!<  Enable */
#define A2B_ENUM_GPIOIEN_IO1_DIS             (0x00000000u)  /*!<  Disable */
#define A2B_ENUM_GPIOIEN_IO0_EN              (0x00000001u)  /*!<  Enable */
#define A2B_ENUM_GPIOIEN_IO0_DIS             (0x00000000u)  /*!<  Disable */

#define A2B_REG_GPIOIEN_RESET                (0x00000000u)  /*!<  Reset Value for GPIOIEN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIN                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_GPIOIN_IO7IN                (7u)           /*!<  IO7 Input Value - AD242X only */
#define A2B_BITP_GPIOIN_IO6IN                (6u)           /*!<  IO6 Input Value */
#define A2B_BITP_GPIOIN_IO5IN                (5u)           /*!<  IO5 Input Value */
#define A2B_BITP_GPIOIN_IO4IN                (4u)           /*!<  IO4 Input Value */
#define A2B_BITP_GPIOIN_IO3IN                (3u)           /*!<  IO3 Input Value */
#define A2B_BITP_GPIOIN_IO2IN                (2u)           /*!<  IO2 Input Value */
#define A2B_BITP_GPIOIN_IO1IN                (1u)           /*!<  IO1 Input Value */
#define A2B_BITP_GPIOIN_IO0IN                (0u)           /*!<  IO0 Input Value */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIN                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_GPIOIN_IO7IN                (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_GPIOIN_IO6IN                (0x00000040u)
#define A2B_BITM_GPIOIN_IO5IN                (0x00000020u)
#define A2B_BITM_GPIOIN_IO4IN                (0x00000010u)
#define A2B_BITM_GPIOIN_IO3IN                (0x00000008u)
#define A2B_BITM_GPIOIN_IO2IN                (0x00000004u)
#define A2B_BITM_GPIOIN_IO1IN                (0x00000002u)
#define A2B_BITM_GPIOIN_IO0IN                (0x00000001u)

#define A2B_REG_GPIOIN_RESET                 (0x00000000u)  /*!<  Reset Value for GPIOIN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTEN                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PINTEN_IO7IE                (7u)           /*!<  IO7 Interrupt Request Capability Enable - AD242X only */
#define A2B_BITP_PINTEN_IO6IE                (6u)           /*!<  DRX1/IO6 Interrupt Request Capability Enable */
#define A2B_BITP_PINTEN_IO5IE                (5u)           /*!<  DRX0/IO5 Interrupt Request Capability Enable */
#define A2B_BITP_PINTEN_IO4IE                (4u)           /*!<  DTX1/IO4 Interrupt Request Capability Enable */
#define A2B_BITP_PINTEN_IO3IE                (3u)           /*!<  DTX0/IO3 Interrupt Request Capability Enable */
#define A2B_BITP_PINTEN_IO2IE                (2u)           /*!<  ADR2/IO2 Interrupt Request Capability Enable */
#define A2B_BITP_PINTEN_IO1IE                (1u)           /*!<  ADR1/IO1 Interrupt Request Capability Enable */
#define A2B_BITP_PINTEN_IO0IE                (0u)           /*!<  IRQ/IO0 Interrupt Request Capability Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTEN                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_PINTEN_IO7IE                (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_PINTEN_IO6IE                (0x00000040u)
#define A2B_BITM_PINTEN_IO5IE                (0x00000020u)
#define A2B_BITM_PINTEN_IO4IE                (0x00000010u)
#define A2B_BITM_PINTEN_IO3IE                (0x00000008u)
#define A2B_BITM_PINTEN_IO2IE                (0x00000004u)
#define A2B_BITM_PINTEN_IO1IE                (0x00000002u)
#define A2B_BITM_PINTEN_IO0IE                (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTEN                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_PINTEN_IO7_EN               (0x00000080u)  /*!<  Enable Interrupt Request Capability - AD242X only */
#define A2B_ENUM_PINTEN_IO7_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability - AD242X only */
#define A2B_ENUM_PINTEN_IO6_EN               (0x00000040u)  /*!<  Enable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO6_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO5_EN               (0x00000020u)  /*!<  Enable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO5_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO4_EN               (0x00000010u)  /*!<  Enable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO4_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO3_EN               (0x00000008u)  /*!<  Enable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO3_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO2_EN               (0x00000004u)  /*!<  Enable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO2_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO1_EN               (0x00000002u)  /*!<  Enable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO1_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO0_EN               (0x00000001u)  /*!<  Enable Interrupt Request Capability */
#define A2B_ENUM_PINTEN_IO0_DIS              (0x00000000u)  /*!<  Disable Interrupt Request Capability */

#define A2B_REG_PINTEN_RESET                 (0x00000000u)  /*!<  Reset Value for PINTEN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTINV                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PINTINV_IO7INV              (7u)           /*!<  Invert IO7 - AD242X only */
#define A2B_BITP_PINTINV_IO6INV              (6u)           /*!<  Invert IO6 */
#define A2B_BITP_PINTINV_IO5INV              (5u)           /*!<  Invert IO5 */
#define A2B_BITP_PINTINV_IO4INV              (4u)           /*!<  Invert IO4 */
#define A2B_BITP_PINTINV_IO3INV              (3u)           /*!<  Invert IO3 */
#define A2B_BITP_PINTINV_IO2INV              (2u)           /*!<  Invert IO2 */
#define A2B_BITP_PINTINV_IO1INV              (1u)           /*!<  Invert IO1 */
#define A2B_BITP_PINTINV_IO0INV              (0u)           /*!<  Invert IO0 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTINV                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_PINTINV_IO7INV              (0x00000080u)  /*!< AD242X only */
#define A2B_BITM_PINTINV_IO6INV              (0x00000040u)
#define A2B_BITM_PINTINV_IO5INV              (0x00000020u)
#define A2B_BITM_PINTINV_IO4INV              (0x00000010u)
#define A2B_BITM_PINTINV_IO3INV              (0x00000008u)
#define A2B_BITM_PINTINV_IO2INV              (0x00000004u)
#define A2B_BITM_PINTINV_IO1INV              (0x00000002u)
#define A2B_BITM_PINTINV_IO0INV              (0x00000001u)
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTINV                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_ENUM_PINTINV_IO7_EN              (0x00000080u)  /*!<  Active Low (Falling Edge) - AD242X only */
#define A2B_ENUM_PINTINV_IO7_DIS             (0x00000000u)  /*!<  Disable Inverter - AD242X only */
#define A2B_ENUM_PINTINV_IO6_EN              (0x00000040u)  /*!<  Active Low (Falling Edge) */
#define A2B_ENUM_PINTINV_IO6_DIS             (0x00000000u)  /*!<  Disable Inverter */
#define A2B_ENUM_PINTINV_IO5_EN              (0x00000020u)  /*!<  Active Low (Falling Edge) */
#define A2B_ENUM_PINTINV_IO5_DIS             (0x00000000u)  /*!<  Active High (Rising Edge) */
#define A2B_ENUM_PINTINV_IO4_EN              (0x00000010u)  /*!<  Active Low (Falling Edge) */
#define A2B_ENUM_PINTINV_IO4_DIS             (0x00000000u)  /*!<  Active High (Rising Edge) */
#define A2B_ENUM_PINTINV_IO3_EN              (0x00000008u)  /*!<  Active Low (Falling Edge) */
#define A2B_ENUM_PINTINV_IO3_DIS             (0x00000000u)  /*!<  Active High (Rising Edge) */
#define A2B_ENUM_PINTINV_IO2_EN              (0x00000004u)  /*!<  Active Low (Falling Edge) */
#define A2B_ENUM_PINTINV_IO2_DIS             (0x00000000u)  /*!<  Active High (Rising Edge) */
#define A2B_ENUM_PINTINV_IO1_EN              (0x00000002u)  /*!<  Active Low (Falling Edge) */
#define A2B_ENUM_PINTINV_IO1_DIS             (0x00000000u)  /*!<  Active High (Rising Edge) */
#define A2B_ENUM_PINTINV_IO0_EN              (0x00000001u)  /*!<  Active Low (Falling Edge) */
#define A2B_ENUM_PINTINV_IO0_DIS             (0x00000000u)  /*!<  Active High (Rising Edge) */

#define A2B_REG_PINTINV_RESET                (0x00000000u)  /*!<  Reset Value for PINTINV */
/* ============================================================================================================================ */
/** \name   Shadow Register Committed Copy Read Access Register Field Definitions */
/* ============================================================================================================================ */
#define A2B_REG_RESPCCC_RESET                (0x00000040u)  /*!<  Reset Value for RESPCCC */
#define A2B_REG_DCTLCC_RESET                 (0x00000000u)  /*!<  Reset Value for DCTLCC */
#define A2B_REG_DNSCC_RESET                  (0x00000000u)  /*!<  Reset Value for DNSCC */
#define A2B_REG_LDNSCC_RESET                 (0x00000000u)  /*!<  Reset Value for LDNSCC */
#define A2B_REG_UPSCC_RESET                  (0x00000000u)  /*!<  Reset Value for UPSCC */
#define A2B_REG_LUPSCC_RESET                 (0x00000000u)  /*!<  Reset Value for LUPSCC */
#define A2B_REG_BCDNSCC_RESET                (0x00000000u)  /*!<  Reset Value for BCDNSCC */
#define A2B_REG_SFMTCC_RESET                 (0x00000000u)  /*!<  Reset Value for SFMTCC */
/* ============================================================================================================================ */
/** \name   Chip test Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PTSTMODE                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PTSTMODE_SWTHRSHLD          (5u)           /*!<  Switch Threshold Signal */
#define A2B_BITP_PTSTMODE_IGNSWFIN           (4u)           /*!<  Ignore Switch Finished Signal */
#define A2B_BITP_PTSTMODE_DISECC             (3u)           /*!<  Disable ECC in eFuse */
#define A2B_BITP_PTSTMODE_LDTRIMVAL          (2u)           /*!<  Load Trim Value into eFuse */
#define A2B_BITP_PTSTMODE_CRCDIS             (1u)           /*!<  CRC Disable */
#define A2B_BITP_PTSTMODE_VREGDIS            (0u)           /*!<  Voltage Regulator Disable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PTSTMODE                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITM_PTSTMODE_SWTHRSHLD          (0x00000020u)
#define A2B_BITM_PTSTMODE_IGNSWFIN           (0x00000010u)
#define A2B_BITM_PTSTMODE_DISECC             (0x00000008u)
#define A2B_BITM_PTSTMODE_LDTRIMVAL          (0x00000004u)
#define A2B_BITM_PTSTMODE_CRCDIS             (0x00000002u)
#define A2B_BITM_PTSTMODE_VREGDIS            (0x00000001u)

#define A2B_REG_PTSTMODE_RESET               (0x00000000u)  /*!<  Reset Value for PTSTMODE */
#define A2B_REG_STRAPVAL_RESET               (0x00000000u)  /*!<  Reset Value for STRAPVAL */
/* ============================================================================================================================ */
/*    Trigger Master Definitions */
/* ============================================================================================================================ */

/* ============================================================================================================================ */
/*    Trigger Slave Definitions */
/* ============================================================================================================================ */

/** \} -- needed for last name */
/** \} -- a2bstack_regdefs_masks */

#ifdef A2B_ENABLE_AD244xx_SUPPORT
/* ============================================================================================================================ */
/** \name   AD244x REG OFFSETS */
/** \note   244x CP INTERRUPT STAT REGS */
/* ============================================================================================================================ */

#define A2B_REG_244x_CP_VENDOR_ID	(0x8003u)
#define A2B_REG_244x_CP_PRODUCT_ID	(0x8004u)
#define A2B_REG_244x_CP_IRQ_CLR		(0x800Fu)
#define A2B_REG_244x_CP_IRQ_ST		(0x8010u)
#define A2B_REG_244x_CPIRQ_PORT1	(0x8011u)
#define A2B_REG_244x_CPIRQ_PORT2	(0x8012u)
#define A2B_REG_244x_CPIRQ_PORT3	(0x8013u)
#define A2B_REG_244x_CP_SHADOWREGS	(0x8100u)
#define A2B_REG_244x_TXPORT_EVT_ST	(0x0020u)
#define A2B_REG_244x_TXPORT_EVT_CLR	(0x0022u)
#define A2B_REG_244x_RXPORT_EVT_ST	(0x0C26u)
#define A2B_REG_244x_RXPORT_EVT_CLR	(0x0C28u)

#define A2B_REG_244x_TXPORT_CTL1	(0x0002u)
#define A2B_REG_244x_TXPORT_CTL2	(0x0003u)
#define A2B_REG_244x_RXPORT_CTL1	(0x0C04u)
#define A2B_REG_244x_RXPORT_CTL2	(0x0C05u)
#define A2B_REG_244x_TXPORT_STAT	(0x0008)
#define A2B_REG_244x_RXPORT_STAT	(0x0C0C)
#define A2B_REG_244x_CP_AUTODISC	(0x8021u)

#define A2B_REG_244x_AKE_START		(0x01u)
#define A2B_REG_244x_ENC_ENABLE		(0x01u)
#define A2B_REG_244x_ENC_DISABLE	(0x00u)
#define A2B_REG_244x_MUTE_ENABLE	(0x02u)
#define A2B_REG_244x_MUTE_DISABLE	(0x00u)
#define A2B_REG_CPIRQ_PORT_EVT_ST	(0x02u)
#define A2B_REG_PORT_ADDR_OFFSET	(0x100u)

#define A2B_MAX_TXPORTS				(12u)
#define A2B_MAX_RXPORTS				(12u)
#define A2B_RXCLR_EVT_STATUS		(0x7Fu)
#define A2B_TXCLR_EVT_STATUS		(0x0Fu)
#define A2B_CP_EVT_CLR				(0x01u)

/* CP IRQ EVENT STATUS */
#define A2B_CP_RESET_EVT			(0x1u)
#define A2B_CP_PORT_EVT_ST			(0x2u)
#define A2B_CP_SYS_ERR_ST			(0x4u)
#define A2B_CP_OTP_ERR_ST			(0x8u)

/* TXPORT IRQ EVENT STATUS */
#define A2B_TXPORT_AUTH_DONE_ST		(0x1u)
#define A2B_TXPORT_CP_ERR_ST		(0x2u)
#define A2B_TXPORT_CP_TIMEOUT		(0x4u)
#define A2B_TXPORT_REAUTH_RCVD_ST	(0x8u)

/* RXPORT IRQ EVENT STATUS */
#define A2B_RXPORT_AUTH_DONE_ST		(0x1u)
#define A2B_RXPORT_CP_ERR_ST		(0x2u)
#define A2B_RXPORT_CP_TIMEOUT		(0x4u)
#define A2B_RXPORT_LINK_SYNCERR		(0x8u)
#define A2B_RXPORT_INIT_RCVD_ST		(0x10u)
#define A2B_RXPORT_ENC_CHNGE_ST		(0x20u)
#define A2B_RXPORT_MUTE_CHNGE_ST	(0x40u)

/*Base address for Port status*/
#define A2B_TXPORT_STAT				(0x08)
#define A2B_RXPORT_STAT				(0x0C)

/* Port status masks */
#define A2B_CP_TX_AUTH_STAT_MASK	(0x01u)			/*!< Mask to check if the authentication status bit is enabled */
#define A2B_CP_TX_MUTE_STAT_MASK	(0x02u)			/*!< Mask to check if the mute status bit is enabled */
#define A2B_CP_TX_ENC_STAT_MASK		(0x04u)			/*!< Mask to check if the encryption status bit is enabled */

/*Mask for Rx ports*/
#define A2B_CP_RX_AUTH_STAT_MASK    (0x01u)			/*!< Mask to check if the authentication status bit is enabled */
#define A2B_CP_RX_MUTE_STAT_MASK    (0x02u)			/*!< Mask to check if the mute status bit is enabled */
#define A2B_CP_RX_ENC_STAT_MASK	    (0x04u)			/*!< Mask to check if the encryption status bit is enabled */

#endif
/* ============================================================================================================================ */
/** \name   CHIRON Register Field Definitions */
/* ============================================================================================================================ */

#define A2B_REG_PDMCTL2_RESET                (0x00000000u)            /*  Reset Value for PDMCTL2  */
#define A2B_REG_PDMCTL2                      (0x0000005Du)            /*  A2B PDM Control 2 Register */
#define A2B_REG_CHIPID0_RESET                (0x00000000u)            /*  Reset Value for CHIPID0  */
#define A2B_REG_CHIPID0                      (0x0000006Au)            /*  A2B Chip ID Register 0 */
#define A2B_REG_CHIPID1_RESET                (0x00000000u)            /*  Reset Value for CHIPID1  */
#define A2B_REG_CHIPID1                      (0x0000006Bu)            /*  A2B Chip ID Register 1 */
#define A2B_REG_CHIPID2_RESET                (0x00000000u)            /*  Reset Value for CHIPID2  */
#define A2B_REG_CHIPID2                      (0x0000006Cu)            /*  A2B Chip ID Register 2 */
#define A2B_REG_CHIPID3_RESET                (0x00000000u)            /*  Reset Value for CHIPID3  */
#define A2B_REG_CHIPID3                      (0x0000006Du)            /*  A2B Chip ID Register 3 */
#define A2B_REG_CHIPID4_RESET                (0x00000000u)            /*  Reset Value for CHIPID4  */
#define A2B_REG_CHIPID4                      (0x0000006Eu)            /*  A2B Chip ID Register 4 */
#define A2B_REG_CHIPID5_RESET                (0x00000000u)            /*  Reset Value for CHIPID5  */
#define A2B_REG_CHIPID5                      (0x0000006Fu)            /*  A2B Chip ID Register 5 */

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_PDMCTL2                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PDMCTL2_PDMINVCLK            5u            /*  PDM Inverted Version of Alternate Clock */
#define A2B_BITP_PDMCTL2_PDMALTCLK            4u            /*  PDM Alternate Clock */
#define A2B_BITP_PDMCTL2_PDM1FFRST            3u            /*  PDM1 Falling Edge First Enable */
#define A2B_BITP_PDMCTL2_PDM0FFRST            2u            /*  PDM0 Falling Edge First Enable */
#define A2B_BITP_PDMCTL2_PDMDEST              0u            /*  PDM Destination */
#define A2B_BITM_PDMCTL2_PDMINVCLK           (_ADI_MSK_3(0x00000020,0x00000020U, uint8_t   ))    /*  PDM Inverted Version of Alternate Clock */
#define A2B_BITM_PDMCTL2_PDMALTCLK           (0x00000010u)                                       /*  PDM Alternate Clock */
#define A2B_BITM_PDMCTL2_PDM1FFRST           (_ADI_MSK_3(0x00000008,0x00000008U, uint8_t   ))    /*  PDM1 Falling Edge First Enable */
#define A2B_BITM_PDMCTL2_PDM0FFRST           (_ADI_MSK_3(0x00000004,0x00000004U, uint8_t   ))    /*  PDM0 Falling Edge First Enable */
#define A2B_BITM_PDMCTL2_PDMDEST             (_ADI_MSK_3(0x00000003,0x00000003U, uint8_t   ))    /*  PDM Destination */

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_CHIPID0                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CHIPID0_CHIPID               0            /*  Contains 48-bit Unique Chip ID Value */
#define A2B_BITM_CHIPID0_CHIPID              (_ADI_MSK_3(0x000000FF,0x000000FFU, uint8_t   ))    /*  Contains 48-bit Unique Chip ID Value */

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_CHIPID1                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CHIPID1_CHIPID               0            /*  Contains 48-bit Unique Chip ID Value */
#define A2B_BITM_CHIPID1_CHIPID              (_ADI_MSK_3(0x000000FF,0x000000FFU, uint8_t   ))    /*  Contains 48-bit Unique Chip ID Value */

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_CHIPID2                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CHIPID2_CHIPID               0            /*  Contains 48-bit Unique Chip ID Value */
#define A2B_BITM_CHIPID2_CHIPID              (_ADI_MSK_3(0x000000FF,0x000000FFU, uint8_t   ))    /*  Contains 48-bit Unique Chip ID Value */

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_CHIPID3                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CHIPID3_CHIPID               0            /*  Contains 48-bit Unique Chip ID Value */
#define A2B_BITM_CHIPID3_CHIPID              (_ADI_MSK_3(0x000000FF,0x000000FFU, uint8_t   ))    /*  Contains 48-bit Unique Chip ID Value */

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_CHIPID4                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CHIPID4_CHIPID               0            /*  Contains 48-bit Unique Chip ID Value */
#define A2B_BITM_CHIPID4_CHIPID              (_ADI_MSK_3(0x000000FF,0x000000FFU, uint8_t   ))    /*  Contains 48-bit Unique Chip ID Value */

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_CHIPID5                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_CHIPID5_CHIPID               0            /*  Contains 48-bit Unique Chip ID Value */
#define A2B_BITM_CHIPID5_CHIPID              (_ADI_MSK_3(0x000000FF,0x000000FFU, uint8_t   ))    /*  Contains 48-bit Unique Chip ID Value */


/* ============================================================================================================================ */
/** \name   DEMETER Register Field Definitions */
/* ============================================================================================================================ */


#define A2B_REG_DTCFG_RESET                  0x00000000u           /*      Reset Value for DTCFG  */
#define A2B_REG_DTCFG                        0x0000007Cu           /*  A2B Data Tunnel Configuration Register */
#define A2B_REG_DTSLOTS_RESET                0x00000000u           /*      Reset Value for DTSLOTS  */
#define A2B_REG_DTSLOTS                      0x0000007Du           /*  A2B Data Tunnel Slots Register */
#define A2B_REG_DTDNOFFS_RESET               0x00000000u           /*      Reset Value for DTDNOFFS  */
#define A2B_REG_DTDNOFFS                     0x0000007Eu           /*  A2B Data Tunnel Downstream Offset Register */
#define A2B_REG_DTUPOFFS_RESET               0x00000000u           /*      Reset Value for DTUPOFFS  */
#define A2B_REG_DTUPOFFS                     0x0000007Fu           /*  A2B Data Tunnel Upstream Offset Register */
#define A2B_REG_DTCFGCC_RESET                0x00000000u           /*      Reset Value for DTCFGCC  */
#define A2B_REG_DTCFGCC                      0x0000008Cu           /*  A2B Data Tunnel Configuration Register Committed Copy */
#define A2B_REG_DTSLTSCC_RESET               0x00000000u           /*      Reset Value for DTSLTSCC  */
#define A2B_REG_DTSLTSCC                     0x0000008Du           /*  A2B Data Tunnel Slots Register Committed Copy */
#define A2B_REG_DTDNOFCC_RESET               0x00000000u           /*      Reset Value for DTDNOFCC  */
#define A2B_REG_DTDNOFCC                     0x0000008Eu           /*  A2B Data Tunnel Downstream Offset Register Committed Copy */
#define A2B_REG_DTUPOFCC_RESET               0x00000000u           /*      Reset Value for DTUPOFCC  */
#define A2B_REG_DTUPOFCC                     0x0000008Fu           /*  A2B Data Tunnel Upstream Offset Register Committed Copy */
#define A2B_REG_SWCTL2_RESET                 0x00000000u           /*      Reset Value for SWCTL2  */
#define A2B_REG_SWCTL2                       0x000000A0u           /*  A2B Switch Control Register 2 */
#define A2B_REG_SWCTL3_RESET                 0x00000000u           /*      Reset Value for SWCTL3  */
#define A2B_REG_SWCTL3                       0x000000A1u           /*  A2B Switch Control Register 3 */
#define A2B_REG_SWCTL4_RESET                 0x00000000u           /*      Reset Value for SWCTL4  */
#define A2B_REG_SWCTL4                       0x000000A2u           /*  A2B Switch Control Register 4 */
#define A2B_REG_SWCTL5_RESET                 0x00000000u           /*      Reset Value for SWCTL5  */
#define A2B_REG_SWCTL5                       0x000000A3u           /*  A2B Switch Control Register 5 */
#define A2B_REG_SWCTL6_RESET                 0x00000000u           /*      Reset Value for SWCTL6  */
#define A2B_REG_SWCTL6                       0x000000A4u           /*  A2B Switch Control Register 6 */
#define A2B_REG_SPIDTLCMD_RESET              0x00000000u           /*      Reset Value for SPIDTLCMD  */
#define A2B_REG_SPIDTLCMD                    0x000000AFu           /*  A2B SPI Data Tunnel Last Command Register */
#define A2B_REG_SPICFG_RESET                 0x00000000u           /*      Reset Value for SPICFG  */
#define A2B_REG_SPICFG                       0x000000B0u           /*  A2B SPI Configuration Register */
#define A2B_REG_SPISTAT_RESET                0x00000000u           /*      Reset Value for SPISTAT  */
#define A2B_REG_SPISTAT                      0x000000B1u           /*  A2B SPI Status Register */
#define A2B_REG_SPICKDIV_RESET               0x00000000u           /*      Reset Value for SPICKDIV  */
#define A2B_REG_SPICKDIV                     0x000000B2u           /*  A2B SPI Clock Divide Register */
#define A2B_REG_SPIFDSIZE_RESET              0x00000000u           /*      Reset Value for SPIFDSIZE  */
#define A2B_REG_SPIFDSIZE                    0x000000B3u           /*  A2B SPI Full Duplex Size Register */
#define A2B_REG_SPIFDTARG_RESET              0x00000000u           /*      Reset Value for SPIFDTARG  */
#define A2B_REG_SPIFDTARG                    0x000000B4u           /*  A2B SPI Full Duplex Target Register */
#define A2B_REG_SPIPINCFG_RESET              0x00000000u           /*      Reset Value for SPIPINCFG  */
#define A2B_REG_SPIPINCFG                    0x000000B5u           /*  A2B SPI Pin Configuration Register */
#define A2B_REG_SPIINT_RESET                 0x00000000u           /*      Reset Value for SPIINT  */
#define A2B_REG_SPIINT                       0x000000B6u           /*  A2B SPI Interrupt Register */
#define A2B_REG_SPIMSK_RESET                 0x00000000u           /*      Reset Value for SPIMSK  */
#define A2B_REG_SPIMSK                       0x000000B7u           /*  A2B SPI Interrupt Mask Register */
#define A2B_REG_RXMASK0_RESET                0x000000FFu           /*      Reset Value for RXMASK0  */
#define A2B_REG_RXMASK0                      0x000000B8u           /*  A2B I2S/TDM RX Mask 0 Register */
#define A2B_REG_RXMASK1_RESET                0x000000FFu           /*      Reset Value for RXMASK1  */
#define A2B_REG_RXMASK1                      0x000000B9u           /*  A2B I2S/TDM RX Mask 1 Register */
#define A2B_REG_RXMASK2_RESET                0x000000FFu           /*      Reset Value for RXMASK2  */
#define A2B_REG_RXMASK2                      0x000000BAu           /*  A2B I2S/TDM RX Mask 2 Register */
#define A2B_REG_RXMASK3_RESET                0x000000FFu           /*      Reset Value for RXMASK3  */
#define A2B_REG_RXMASK3                      0x000000BBu           /*  A2B I2S/TDM RX Mask 3 Register */
#define A2B_REG_RXMASK4_RESET                0x000000FFu           /*      Reset Value for RXMASK4  */
#define A2B_REG_RXMASK4                      0x000000BCu           /*  A2B I2S/TDM RX Mask 4 Register */
#define A2B_REG_RXMASK5_RESET                0x000000FFu           /*      Reset Value for RXMASK5  */
#define A2B_REG_RXMASK5                      0x000000BDu           /*  A2B I2S/TDM RX Mask 5 Register */
#define A2B_REG_RXMASK6_RESET                0x000000FFu           /*      Reset Value for RXMASK6  */
#define A2B_REG_RXMASK6                      0x000000BEu           /*  A2B I2S/TDM RX Mask 6 Register */
#define A2B_REG_RXMASK7_RESET                0x000000FFu           /*      Reset Value for RXMASK7  */
#define A2B_REG_RXMASK7                      0x000000BFu           /*  A2B I2S/TDM RX Mask 7 Register */
#define A2B_REG_TXXBAR0_RESET                0x00000000u           /*      Reset Value for TXXBAR0  */
#define A2B_REG_TXXBAR0                      0x000000C0u           /*  A2B Serial TX Crossbar Register 0 */
#define A2B_REG_TXXBAR1_RESET                0x00000001u           /*      Reset Value for TXXBAR1  */
#define A2B_REG_TXXBAR1                      0x000000C1u           /*  A2B Serial TX Crossbar Register 1 */
#define A2B_REG_TXXBAR2_RESET                0x00000002u           /*      Reset Value for TXXBAR2  */
#define A2B_REG_TXXBAR2                      0x000000C2u           /*  A2B Serial TX Crossbar Register 2 */
#define A2B_REG_TXXBAR3_RESET                0x00000003u           /*      Reset Value for TXXBAR3  */
#define A2B_REG_TXXBAR3                      0x000000C3u           /*  A2B Serial TX Crossbar Register 3 */
#define A2B_REG_TXXBAR4_RESET                0x00000004u           /*      Reset Value for TXXBAR4  */
#define A2B_REG_TXXBAR4                      0x000000C4u           /*  A2B Serial TX Crossbar Register 4 */
#define A2B_REG_TXXBAR5_RESET                0x00000005u           /*      Reset Value for TXXBAR5  */
#define A2B_REG_TXXBAR5                      0x000000C5u           /*  A2B Serial TX Crossbar Register 5 */
#define A2B_REG_TXXBAR6_RESET                0x00000006u           /*      Reset Value for TXXBAR6  */
#define A2B_REG_TXXBAR6                      0x000000C6u           /*  A2B Serial TX Crossbar Register 6 */
#define A2B_REG_TXXBAR7_RESET                0x00000007u           /*      Reset Value for TXXBAR7  */
#define A2B_REG_TXXBAR7                      0x000000C7u           /*  A2B Serial TX Crossbar Register 7 */
#define A2B_REG_TXXBAR8_RESET                0x00000008u           /*      Reset Value for TXXBAR8  */
#define A2B_REG_TXXBAR8                      0x000000C8u           /*  A2B Serial TX Crossbar Register 8 */
#define A2B_REG_TXXBAR9_RESET                0x00000009u           /*      Reset Value for TXXBAR9  */
#define A2B_REG_TXXBAR9                      0x000000C9u           /*  A2B Serial TX Crossbar Register 9 */
#define A2B_REG_TXXBAR10_RESET               0x0000000Au           /*      Reset Value for TXXBAR10  */
#define A2B_REG_TXXBAR10                     0x000000CAu           /*  A2B Serial TX Crossbar Register 10 */
#define A2B_REG_TXXBAR11_RESET               0x0000000Bu           /*      Reset Value for TXXBAR11  */
#define A2B_REG_TXXBAR11                     0x000000CBu           /*  A2B Serial TX Crossbar Register 11 */
#define A2B_REG_TXXBAR12_RESET               0x0000000Cu           /*      Reset Value for TXXBAR12  */
#define A2B_REG_TXXBAR12                     0x000000CCu           /*  A2B Serial TX Crossbar Register 12 */
#define A2B_REG_TXXBAR13_RESET               0x0000000Du           /*      Reset Value for TXXBAR13  */
#define A2B_REG_TXXBAR13                     0x000000CDu           /*  A2B Serial TX Crossbar Register 13 */
#define A2B_REG_TXXBAR14_RESET               0x0000000Eu           /*      Reset Value for TXXBAR14  */
#define A2B_REG_TXXBAR14                     0x000000CEu           /*  A2B Serial TX Crossbar Register 14 */
#define A2B_REG_TXXBAR15_RESET               0x0000000Fu           /*      Reset Value for TXXBAR15  */
#define A2B_REG_TXXBAR15                     0x000000CFu           /*  A2B Serial TX Crossbar Register 15 */
#define A2B_REG_TXXBAR16_RESET               0x00000010u           /*      Reset Value for TXXBAR16  */
#define A2B_REG_TXXBAR16                     0x000000D0u           /*  A2B Serial TX Crossbar Register 16 */
#define A2B_REG_TXXBAR17_RESET               0x00000011u           /*      Reset Value for TXXBAR17  */
#define A2B_REG_TXXBAR17                     0x000000D1u           /*  A2B Serial TX Crossbar Register 17 */
#define A2B_REG_TXXBAR18_RESET               0x00000012u           /*      Reset Value for TXXBAR18  */
#define A2B_REG_TXXBAR18                     0x000000D2u           /*  A2B Serial TX Crossbar Register 18 */
#define A2B_REG_TXXBAR19_RESET               0x00000013u           /*      Reset Value for TXXBAR19  */
#define A2B_REG_TXXBAR19                     0x000000D3u           /*  A2B Serial TX Crossbar Register 19 */
#define A2B_REG_TXXBAR20_RESET               0x00000014u           /*      Reset Value for TXXBAR20  */
#define A2B_REG_TXXBAR20                     0x000000D4u           /*  A2B Serial TX Crossbar Register 20 */
#define A2B_REG_TXXBAR21_RESET               0x00000015u           /*      Reset Value for TXXBAR21  */
#define A2B_REG_TXXBAR21                     0x000000D5u           /*  A2B Serial TX Crossbar Register 21 */
#define A2B_REG_TXXBAR22_RESET               0x00000016u           /*      Reset Value for TXXBAR22  */
#define A2B_REG_TXXBAR22                     0x000000D6u           /*  A2B Serial TX Crossbar Register 22 */
#define A2B_REG_TXXBAR23_RESET               0x00000017u           /*      Reset Value for TXXBAR23  */
#define A2B_REG_TXXBAR23                     0x000000D7u           /*  A2B Serial TX Crossbar Register 23 */
#define A2B_REG_TXXBAR24_RESET               0x00000018u           /*      Reset Value for TXXBAR24  */
#define A2B_REG_TXXBAR24                     0x000000D8u           /*  A2B Serial TX Crossbar Register 24 */
#define A2B_REG_TXXBAR25_RESET               0x00000019u           /*      Reset Value for TXXBAR25  */
#define A2B_REG_TXXBAR25                     0x000000D9u           /*  A2B Serial TX Crossbar Register 25 */
#define A2B_REG_TXXBAR26_RESET               0x0000001Au           /*      Reset Value for TXXBAR26  */
#define A2B_REG_TXXBAR26                     0x000000DAu           /*  A2B Serial TX Crossbar Register 26 */
#define A2B_REG_TXXBAR27_RESET               0x0000001Bu           /*      Reset Value for TXXBAR27  */
#define A2B_REG_TXXBAR27                     0x000000DBu           /*  A2B Serial TX Crossbar Register 27 */
#define A2B_REG_TXXBAR28_RESET               0x0000001Cu           /*      Reset Value for TXXBAR28  */
#define A2B_REG_TXXBAR28                     0x000000DCu           /*  A2B Serial TX Crossbar Register 28 */
#define A2B_REG_TXXBAR29_RESET               0x0000001Du           /*      Reset Value for TXXBAR29  */
#define A2B_REG_TXXBAR29                     0x000000DDu           /*  A2B Serial TX Crossbar Register 29 */
#define A2B_REG_TXXBAR30_RESET               0x0000001Eu           /*      Reset Value for TXXBAR30  */
#define A2B_REG_TXXBAR30                     0x000000DEu           /*  A2B Serial TX Crossbar Register 30 */
#define A2B_REG_TXXBAR31_RESET               0x0000001Fu           /*      Reset Value for TXXBAR31  */
#define A2B_REG_TXXBAR31                     0x000000DFu           /*  A2B Serial TX Crossbar Register 31 */
#define A2B_REG_MMRPAGE_RESET                0x00000000u           /*      Reset Value for MMRPAGE  */
#define A2B_REG_MMRPAGE                      0x000000E0u           /*  A2B MMR Page Register */
#define A2B_REG_TM1KEY_RESET                 0x00000000u           /*      Reset Value for TM1KEY  */
#define A2B_REG_TM1KEY                       0x000000F3u           /*  A2B Testmode 1 Key Register */
#define A2B_REG_TM3KEY_RESET                 0x00000000u           /*      Reset Value for TM3KEY  */
#define A2B_REG_TM3KEY                       0x000000F4u           /*  A2B Testmode 3 Key Register */
#define A2B_REG_VMTR_VEN_RESET               0x00000000u           /*      Reset Value for VMTR_VEN  */
#define A2B_REG_VMTR_VEN                     0x00000100u           /*  A2B Enable Voltage Measurement */
#define A2B_REG_VMTR_INTEN_RESET             0x00000000u           /*      Reset Value for VMTR_INTEN  */
#define A2B_REG_VMTR_INTEN                   0x00000101u           /*  A2B Min / Max Error Interrupt Enable */
#define A2B_REG_VMTR_MXSTAT_RESET            0x00000000u           /*      Reset Value for VMTR_MXSTAT  */
#define A2B_REG_VMTR_MXSTAT                  0x00000102u           /*  A2B VMAX Check Result */
#define A2B_REG_VMTR_MNSTAT_RESET            0x00000000u           /*      Reset Value for VMTR_MNSTAT  */
#define A2B_REG_VMTR_MNSTAT                  0x00000103u           /*  A2B VMIN Check Result */
#define A2B_REG_VMTR_CHP1L_RESET             0x00000000u           /*      Reset Value for VMTR_CHP1L  */
#define A2B_REG_VMTR_CHP1L                   0x00000108u           /*  A2B CHOP 1 DOUT[7:0] */
#define A2B_REG_VMTR_CHP1H_RESET             0x00000000u           /*      Reset Value for VMTR_CHP1H  */
#define A2B_REG_VMTR_CHP1H                   0x00000109u           /*  A2B CHOP 1 DOUT[9:8] */
#define A2B_REG_VMTR_CHP0L_RESET             0x00000000u           /*      Reset Value for VMTR_CHP0L  */
#define A2B_REG_VMTR_CHP0L                   0x0000010Au           /*  A2B CHOP 0 DOUT[7:0] */
#define A2B_REG_VMTR_CHP0H_RESET             0x00000000u           /*      Reset Value for VMTR_CHP0H  */
#define A2B_REG_VMTR_CHP0H                   0x0000010Bu           /*  A2B CHOP 0 DOUT[9:8] */
#define A2B_REG_VMTR_ENG_RESET               0x00000000u           /*      Reset Value for VMTR_ENG  */
#define A2B_REG_VMTR_ENG                     0x0000010Fu           /*  A2B Engineering Register */
#define A2B_REG_VMTR_VLTG0_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG0  */
#define A2B_REG_VMTR_VLTG0                   0x00000120u           /*  A2B Measured Voltage 0 */
#define A2B_REG_VMTR_VMAX0_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX0  */
#define A2B_REG_VMTR_VMAX0                   0x00000121u           /*  A2B VMAX Registers 0 */
#define A2B_REG_VMTR_VMIN0_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN0  */
#define A2B_REG_VMTR_VMIN0                   0x00000122u           /*  A2B VMIN Register 0 */
#define A2B_REG_VMTR_VLTG1_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG1  */
#define A2B_REG_VMTR_VLTG1                   0x00000123u           /*  A2B Measured Voltage 1 */
#define A2B_REG_VMTR_VMAX1_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX1  */
#define A2B_REG_VMTR_VMAX1                   0x00000124u           /*  A2B VMAX Registers 1 */
#define A2B_REG_VMTR_VMIN1_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN1  */
#define A2B_REG_VMTR_VMIN1                   0x00000125u           /*  A2B VMIN Register 1 */
#define A2B_REG_VMTR_VLTG2_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG2  */
#define A2B_REG_VMTR_VLTG2                   0x00000126u           /*  A2B Measured Voltage 2 */
#define A2B_REG_VMTR_VMAX2_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX2  */
#define A2B_REG_VMTR_VMAX2                   0x00000127u           /*  A2B VMAX Registers 2 */
#define A2B_REG_VMTR_VMIN2_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN2  */
#define A2B_REG_VMTR_VMIN2                   0x00000128u           /*  A2B VMIN Register 2 */
#define A2B_REG_VMTR_VLTG3_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG3  */
#define A2B_REG_VMTR_VLTG3                   0x00000129u           /*  A2B Measured Voltage 3 */
#define A2B_REG_VMTR_VMAX3_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX3  */
#define A2B_REG_VMTR_VMAX3                   0x0000012Au           /*  A2B VMAX Registers 3 */
#define A2B_REG_VMTR_VMIN3_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN3  */
#define A2B_REG_VMTR_VMIN3                   0x0000012Bu           /*  A2B VMIN Register 3 */
#define A2B_REG_VMTR_VLTG4_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG4  */
#define A2B_REG_VMTR_VLTG4                   0x0000012Cu           /*  A2B Measured Voltage 4 */
#define A2B_REG_VMTR_VMAX4_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX4  */
#define A2B_REG_VMTR_VMAX4                   0x0000012Du           /*  A2B VMAX Registers 4 */
#define A2B_REG_VMTR_VMIN4_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN4  */
#define A2B_REG_VMTR_VMIN4                   0x0000012Eu           /*  A2B VMIN Register 4 */
#define A2B_REG_VMTR_VLTG5_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG5  */
#define A2B_REG_VMTR_VLTG5                   0x0000012Fu           /*  A2B Measured Voltage 5 */
#define A2B_REG_VMTR_VMAX5_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX5  */
#define A2B_REG_VMTR_VMAX5                   0x00000130u           /*  A2B VMAX Registers 5 */
#define A2B_REG_VMTR_VMIN5_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN5  */
#define A2B_REG_VMTR_VMIN5                   0x00000131u           /*  A2B VMIN Register 5 */
#define A2B_REG_VMTR_VLTG6_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG6  */
#define A2B_REG_VMTR_VLTG6                   0x00000132u           /*  A2B Measured Voltage 6 */
#define A2B_REG_VMTR_VMAX6_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX6  */
#define A2B_REG_VMTR_VMAX6                   0x00000133u           /*  A2B VMAX Registers 6 */
#define A2B_REG_VMTR_VMIN6_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN6  */
#define A2B_REG_VMTR_VMIN6                   0x00000134u           /*  A2B VMIN Register 6 */
#define A2B_REG_VMTR_VLTG7_RESET             0x00000000u           /*      Reset Value for VMTR_VLTG7  */
#define A2B_REG_VMTR_VLTG7                   0x00000135u           /*  A2B Measured Voltage 7 */
#define A2B_REG_VMTR_VMAX7_RESET             0x000000FFu           /*      Reset Value for VMTR_VMAX7  */
#define A2B_REG_VMTR_VMAX7                   0x00000136u           /*  A2B VMAX Register 7 */
#define A2B_REG_VMTR_VMIN7_RESET             0x00000000u           /*      Reset Value for VMTR_VMIN7  */
#define A2B_REG_VMTR_VMIN7                   0x00000137u           /*  A2B VMIN Register 7 */
#define A2B_REG_PWMCFG_RESET                 0x00000000u           /*      Reset Value for PWMCFG  */
#define A2B_REG_PWMCFG                       0x00000140u           /*  A2B PWM Configuration Register */
#define A2B_REG_PWMFREQ_RESET                0x00000000u           /*      Reset Value for PWMFREQ  */
#define A2B_REG_PWMFREQ                      0x00000141u           /*  A2B PWM Frequency Register */
#define A2B_REG_PWMBLINK1_RESET              0x00000000u           /*      Reset Value for PWMBLINK1  */
#define A2B_REG_PWMBLINK1                    0x00000142u           /*  A2B PWM Blink Register 1 */
#define A2B_REG_PWMBLINK2_RESET              0x00000000u           /*      Reset Value for PWMBLINK2  */
#define A2B_REG_PWMBLINK2                    0x00000143u           /*  A2B PWM Blink Register 2 */
#define A2B_REG_PWM1VALL_RESET               0x00000000u           /*      Reset Value for PWM1VALL  */
#define A2B_REG_PWM1VALL                     0x00000148u           /*  A2B PWM1 Value Low Bits Register */
#define A2B_REG_PWM1VALH_RESET               0x00000000u           /*      Reset Value for PWM1VALH  */
#define A2B_REG_PWM1VALH                     0x00000149u           /*  A2B PWM1 Value High Bits Register */
#define A2B_REG_PWM2VALL_RESET               0x00000000u           /*      Reset Value for PWM2VALL  */
#define A2B_REG_PWM2VALL                     0x0000014Au           /*  A2B PWM2 Value Low Bits Register */
#define A2B_REG_PWM2VALH_RESET               0x00000000u           /*      Reset Value for PWM2VALH  */
#define A2B_REG_PWM2VALH                     0x0000014Bu           /*  A2B PWM2 Value High Bits Register */
#define A2B_REG_PWM3VALL_RESET               0x00000000u           /*      Reset Value for PWM3VALL  */
#define A2B_REG_PWM3VALL                     0x0000014Cu           /*  A2B PWM3 Value Low Bits Register */
#define A2B_REG_PWM3VALH_RESET               0x00000000u           /*      Reset Value for PWM3VALH  */
#define A2B_REG_PWM3VALH                     0x0000014Du           /*  A2B PWM3 Value High Bits Register */
#define A2B_REG_PWMOEVALL_RESET              0x00000000u           /*      Reset Value for PWMOEVALL  */
#define A2B_REG_PWMOEVALL                    0x0000014Eu           /*  A2B PWM OE Value Low Bits Register */
#define A2B_REG_PWMOEVALH_RESET              0x00000000u           /*      Reset Value for PWMOEVALH  */
#define A2B_REG_PWMOEVALH                    0x0000014Fu           /*  A2B PWM OE Value High Bits Register */
#define A2B_REG_MMRPAGE1_RESET               0x00000000u           /*      Reset Value for MMRPAGE1  */
#define A2B_REG_MMRPAGE1                     0x000001E0u           /*  A2B MMR Page Register */

#define A2B_REG_SWSTAT2                      0x000000A5u
#define A2B_REG_SWSTAT3                      0x000000A6u
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SGCFG                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SGCFG_SYNCDIS 			 (3u)			/*!<  SYNC disable */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2SCFG_RXPINS               (4u)            /*  I2S/TDM RX Pins -  AD243x only */
#define A2B_BITP_I2SCFG_TXPINS               (0u)            /*  I2S/TDM RX Pins -  AD243x only */
#define A2B_BITM_I2SCFG_RXPINS               (0x00000070u)            /*  I2S/TDM RX Pins -  AD243x only*/
#define A2B_BITM_I2SCFG_TXPINS               (0x00000007u)            /*  I2S/TDM RX Pins -  AD243x only*/

/* -------------------------------------------------------------------------------------------------------------------------
          A2B_PDMCTL2                          Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PDMCTL2_HPFCORNER            (6u)            /*  PDM HPF corner freq - AD243x only */
#define A2B_BITM_PDMCTL2_HPFCORNER            (0x000000C0u)   /*  PDM HPF corner freq - AD243x only */

#define A2B_ENUM_PDMCTL2_HPFCORNER_0          (0x00000000U);  /*  HPFCORNER: HPF Corner Frequency 1 Hz (Default) */
#define A2B_ENUM_PDMCTL2_HPFCORNER_1          (0x00000040U);  /*  HPFCORNER: HPF Corner Frequency 60 Hz  */
#define A2B_ENUM_PDMCTL2_HPFCORNER_2          (0x00000080U);  /*  HPFCORNER: HPF Corner Frequency 120 Hz  */
#define A2B_ENUM_PDMCTL2_HPFCORNER_3          (0x000000C0U);  /*  HPFCORNER: HPF Corner Frequency 240 Hz  */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINCFG                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PINCFG_GPIOSEL              (6u);            /*  GPIO Mode Select -  AD243x only*/

/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2CCFG                               Pos/mask               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_I2CCFG_DISABLE               (7u)           	/*  I2C disable -  AD243x only */
#define A2B_BITM_I2CCFG_DISABLE               (0x80u)           /*  I2C disable -  AD243x only */

#define A2B_BITP_I2CCFG_FMPLUS				  (3u);            	/*  I2C Fast Mode Plus (A2B Slave Only) */
#define A2B_BITM_I2CCFG_FMPLUS    			  (0x00000008u);    /*  I2C Fast Mode Plus (A2B Slave Only) */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTCFG                            Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTCFG_DTLAST                 2u            /*  Data Tunnel Last Node */
#define A2B_BITP_DTCFG_DTFRST                 1u            /*  Data Tunnel First Node */
#define A2B_BITP_DTCFG_DTEN                   0u            /*  Enable Data Tunnel */
#define A2B_BITM_DTCFG_DTLAST                0x00000004u    /*  Data Tunnel Last Node */
#define A2B_BITM_DTCFG_DTFRST                0x00000002u    /*  Data Tunnel First Node */
#define A2B_BITM_DTCFG_DTEN                  0x00000001u    /*  Enable Data Tunnel */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTSLOTS                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTSLOTS_DTUPSLOTS            4u            /*  Data Tunnel Upstream Slots */
#define A2B_BITP_DTSLOTS_DTDNSLOTS            0u            /*  Data Tunnel Downstream Slots */
#define A2B_BITM_DTSLOTS_DTUPSLOTS           0x000000F0u    /*  Data Tunnel Upstream Slots */
#define A2B_BITM_DTSLOTS_DTDNSLOTS           0x0000000Fu    /*  Data Tunnel Downstream Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_0         0x00000000u            /*  DTUPSLOTS: 0 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_1         0x00000010u            /*  DTUPSLOTS: 1 Upstream Data Slot */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_2         0x00000020u            /*  DTUPSLOTS: 2 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_3         0x00000030u            /*  DTUPSLOTS: 3 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_4         0x00000040u            /*  DTUPSLOTS: 4 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_5         0x00000050u            /*  DTUPSLOTS: 5 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_6         0x00000060u            /*  DTUPSLOTS: 6 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_7         0x00000070u            /*  DTUPSLOTS: 7 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_8         0x00000080u            /*  DTUPSLOTS: 8 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_9         0x00000090u            /*  DTUPSLOTS: 9 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_10        0x000000A0u            /*  DTUPSLOTS: 10 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_11        0x000000B0u            /*  DTUPSLOTS: 11 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTUPSLOTS_12        0x000000C0u            /*  DTUPSLOTS: 12 Upstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_0         0x00000000u            /*  DTDNSLOTS: 0 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_1         0x00000001u            /*  DTDNSLOTS: 1 Downstream Data Slot */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_2         0x00000002u            /*  DTDNSLOTS: 2 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_3         0x00000003u            /*  DTDNSLOTS: 3 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_4         0x00000004u            /*  DTDNSLOTS: 4 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_5         0x00000005u            /*  DTDNSLOTS: 5 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_6         0x00000006u            /*  DTDNSLOTS: 6 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_7         0x00000007u            /*  DTDNSLOTS: 7 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_8         0x00000008u            /*  DTDNSLOTS: 8 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_9         0x00000009u            /*  DTDNSLOTS: 9 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_10        0x0000000Au            /*  DTDNSLOTS: 10 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_11        0x0000000Bu            /*  DTDNSLOTS: 11 Downstream Data Slots */
#define A2B_ENUM_DTSLOTS_DTDNSLOTS_12        0x0000000Cu            /*  DTDNSLOTS: 12 Downstream Data Slots */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTDNOFFS                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTDNOFFS_DTDNOFFS            0u            /*  Data Tunnel Downstream Offset */
#define A2B_BITM_DTDNOFFS_DTDNOFFS           0x0000001Fu    /*  Data Tunnel Downstream Offset */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTUPOFFS                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTUPOFFS_DTUPOFFS            0u            /*  Data Tunnel Upstream Offset */
#define A2B_BITM_DTUPOFFS_DTUPOFFS           0x0000001Fu    /*  Data Tunnel Upstream Offset */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTCFGCC                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTCFGCC_DTCFGCC              0u            /*  Data Tunnel Config Committed Copy */
#define A2B_BITM_DTCFGCC_DTCFGCC             0x000000FFu    /*  Data Tunnel Config Committed Copy */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTSLTSCC                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTSLTSCC_DTSLTSCC            0u            /*  DTSLTSCC Value */
#define A2B_BITM_DTSLTSCC_DTSLTSCC           0x000000FFu    /*  DTSLTSCC Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTDNOFCC                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTDNOFCC_DTDNOFCC            0u            /*  DTDNOFCC Value */
#define A2B_BITM_DTDNOFCC_DTDNOFCC           0x000000FFu    /*  DTDNOFCC Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_DTUPOFCC                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_DTUPOFCC_DTUPOFCC            0u            /*  DTUPOFCC Value */
#define A2B_BITM_DTUPOFCC_DTUPOFCC           0x000000FFu    /*  DTUPOFCC Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SWCTL5                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SWCTL5_SWCTL5_BIT7           7u            /*  SWCTL5 Bit 7 */
#define A2B_BITP_SWCTL5_LGATE_ILIM_INTEN      6u            /*  Enable LGate Current Limit Detection Interrupt */
#define A2B_BITP_SWCTL5_HGATE_ILIM_INTEN      5u            /*  Enable HGate Current Limit Detection Interrupt */
#define A2B_BITP_SWCTL5_CURR_MIS_INTEN        4u            /*  Enable Current Mismatch Detection Interrupt */
#define A2B_BITP_SWCTL5_SWCTL5_BIT3           3u            /*  SWCTL5 Bit 3 */
#define A2B_BITP_SWCTL5_LGATE_ILIM_DIS        2u            /*  Disable LGate Current Limit Detection */
#define A2B_BITP_SWCTL5_HGATE_ILIM_DIS        1u            /*  Disable HGate Current Limit Detection */
#define A2B_BITP_SWCTL5_CURR_MIS_DIS          0u            /*  Disable Current Mismatch Detection */
#define A2B_BITM_SWCTL5_SWCTL5_BIT7          0x00000080u    /*  SWCTL5 Bit 7 */
#define A2B_BITM_SWCTL5_LGATE_ILIM_INTEN     0x00000040u    /*  Enable LGate Current Limit Detection Interrupt */
#define A2B_BITM_SWCTL5_HGATE_ILIM_INTEN     0x00000020u    /*  Enable HGate Current Limit Detection Interrupt */
#define A2B_BITM_SWCTL5_CURR_MIS_INTEN       0x00000010u    /*  Enable Current Mismatch Detection Interrupt */
#define A2B_BITM_SWCTL5_SWCTL5_BIT3          0x00000008u    /*  SWCTL5 Bit 3 */
#define A2B_BITM_SWCTL5_LGATE_ILIM_DIS       0x00000004u    /*  Disable LGate Current Limit Detection */
#define A2B_BITM_SWCTL5_HGATE_ILIM_DIS       0x00000002u    /*  Disable HGate Current Limit Detection */
#define A2B_BITM_SWCTL5_CURR_MIS_DIS         0x00000001u    /*  Disable Current Mismatch Detection */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SWCTL6                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SWCTL6_SWCTL6_BITS_2_7       2u            /*  SWCTL6 Bits 2 to 7 */
#define A2B_BITP_SWCTL6_SWN_VOLT_DBG          1u            /*  Enable SWN_Volt Debug */
#define A2B_BITP_SWCTL6_SW_EN_SW_FIN          0u            /*  Enable Fast Progression to SW_FIN */
#define A2B_BITM_SWCTL6_SWCTL6_BITS_2_7      0x000000FCu    /*  SWCTL6 Bits 2 to 7 */
#define A2B_BITM_SWCTL6_SWN_VOLT_DBG         0x00000002u    /*  Enable SWN_Volt Debug */
#define A2B_BITM_SWCTL6_SW_EN_SW_FIN         0x00000001u    /*  Enable Fast Progression to SW_FIN */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPIDTLCMD                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPIDTLCMD_LASTCMD            0u            /*  SPI Data Tunnel Last Command */
#define A2B_BITM_SPIDTLCMD_LASTCMD           0x000000FFu    /*  SPI Data Tunnel Last Command */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPICFG                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPICFG_SPIFDSS               6u            /*  SPI Full Duplex Slave Select */
#define A2B_BITP_SPICFG_ENFDCS                5u            /*  Enable Full Duplex Clock Stretching */
#define A2B_BITP_SPICFG_SPI_CPOL              4u            /*  SPI Clock Polarity */
#define A2B_BITP_SPICFG_SPI_CPHA              3u            /*  SPI Clock Phase */
#define A2B_BITP_SPICFG_TNLOWNER              2u            /*  Current Node is Data Tunnel Owner */
#define A2B_BITP_SPICFG_SPIMODE               0u            /*  SPI Mode */
#define A2B_BITM_SPICFG_SPIFDSS              0x000000C0u    /*  SPI Full Duplex Slave Select */
#define A2B_BITM_SPICFG_ENFDCS               0x00000020u    /*  Enable Full Duplex Clock Stretching */
#define A2B_BITM_SPICFG_SPI_CPOL             0x00000010u    /*  SPI Clock Polarity */
#define A2B_BITM_SPICFG_SPI_CPHA             0x00000008u    /*  SPI Clock Phase */
#define A2B_BITM_SPICFG_TNLOWNER             0x00000004u    /*  Current Node is Data Tunnel Owner */
#define A2B_BITM_SPICFG_SPIMODE              0x00000003u    /*  SPI Mode */
#define A2B_ENUM_SPICFG_SPIMODE_0            0x00000000u            /*  SPIMODE: SPI Slave Mode */
#define A2B_ENUM_SPICFG_SPIMODE_1            0x00000001u            /*  SPIMODE: Data Tunnel Target (SPI Master Mode) */
#define A2B_ENUM_SPICFG_SPIMODE_2            0x00000002u            /*  SPIMODE: SPI Module Disabled */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPISTAT                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPISTAT_DTBADPKT             7u            /*  Data Tunnel Bad Packet Detected */
#define A2B_BITP_SPISTAT_DTABORT              6u            /*  Data Tunnel Transaction Aborted */
#define A2B_BITP_SPISTAT_DTINVALID            5u            /*  Data Tunnel Invalid Configuration */
#define A2B_BITP_SPISTAT_DTACTIVE             1u            /*  Data Tunnel Transaction Active */
#define A2B_BITP_SPISTAT_SPIBUSY              0u            /*  SPI Peripheral Busy */
#define A2B_BITM_SPISTAT_DTBADPKT            0x00000080u    /*  Data Tunnel Bad Packet Detected */
#define A2B_BITM_SPISTAT_DTABORT             0x00000040u    /*  Data Tunnel Transaction Aborted */
#define A2B_BITM_SPISTAT_DTINVALID           0x00000020u    /*  Data Tunnel Invalid Configuration */
#define A2B_BITM_SPISTAT_DTACTIVE            0x00000002u    /*  Data Tunnel Transaction Active */
#define A2B_BITM_SPISTAT_SPIBUSY             0x00000001u    /*  SPI Peripheral Busy */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPICKDIV                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPICKDIV_CKDIV               0u            /*  SPI Clock Divide Value */
#define A2B_BITM_SPICKDIV_CKDIV              0x0000003Fu    /*  SPI Clock Divide Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPIFDSIZE                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPIFDSIZE_FDSIZE             0u            /*  SPI Full Duplex Transaction Size */
#define A2B_BITM_SPIFDSIZE_FDSIZE            0x000000FFu    /*  SPI Full Duplex Transaction Size */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPIFDTARG                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPIFDTARG_SSEL               6u            /*  SPI Full Duplex Target Slave Select */
#define A2B_BITP_SPIFDTARG_MNS                5u            /*  SPI Full Duplex Master/Not Slave Target */
#define A2B_BITP_SPIFDTARG_NODE               0u            /*  SPI Full Duplex Target Node */
#define A2B_BITM_SPIFDTARG_SSEL              0x000000C0u    /*  SPI Full Duplex Target Slave Select */
#define A2B_BITM_SPIFDTARG_MNS               0x00000020u    /*  SPI Full Duplex Master/Not Slave Target */
#define A2B_BITM_SPIFDTARG_NODE              0x0000000Fu    /*  SPI Full Duplex Target Node */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPIPINCFG                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPIPINCFG_SPIMSS2EN          6u            /*  SPI Master Slave Select 2 Enable */
#define A2B_BITP_SPIPINCFG_SPIMSS1EN          5u            /*  SPI Master Slave Select 1 Enable */
#define A2B_BITP_SPIPINCFG_SPIMSS0EN          4u            /*  SPI Master Slave Select 0 Enable */
#define A2B_BITP_SPIPINCFG_SPIGPIOEN          3u            /*  SPI GPIO Enable */
#define A2B_BITP_SPIPINCFG_SPIGPIOSEL         0u            /*  SPI GPIO Select */
#define A2B_BITM_SPIPINCFG_SPIMSS2EN         0x00000040u    /*  SPI Master Slave Select 2 Enable */
#define A2B_BITM_SPIPINCFG_SPIMSS1EN         0x00000020u    /*  SPI Master Slave Select 1 Enable */
#define A2B_BITM_SPIPINCFG_SPIMSS0EN         0x00000010u    /*  SPI Master Slave Select 0 Enable */
#define A2B_BITM_SPIPINCFG_SPIGPIOEN         0x00000008u    /*  SPI GPIO Enable */
#define A2B_BITM_SPIPINCFG_SPIGPIOSEL        0x00000007u    /*  SPI GPIO Select */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPIINT                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPIINT_FIFOUNF               6u            /*  SPI FIFO Underflow Error */
#define A2B_BITP_SPIINT_FIFOOVF               5u            /*  SPI FIFO Overflow Error */
#define A2B_BITP_SPIINT_BADCMD                4u            /*  SPI Bad Command Detected */
#define A2B_BITP_SPIINT_SPIDTERR              3u            /*  SPI Data Tunnel Error */
#define A2B_BITP_SPIINT_SPII2CERR             2u            /*  SPI Remote I2C Access Error (Master Only) */
#define A2B_BITP_SPIINT_SPIREGERR             1u            /*  SPI Remote Register Access Error (Master Only) */
#define A2B_BITP_SPIINT_SPIDONE               0u            /*  SPI Done Interrupt */
#define A2B_BITM_SPIINT_FIFOUNF              0x00000040u    /*  SPI FIFO Underflow Error */
#define A2B_BITM_SPIINT_FIFOOVF              0x00000020u    /*  SPI FIFO Overflow Error */
#define A2B_BITM_SPIINT_BADCMD               0x00000010u    /*  SPI Bad Command Detected */
#define A2B_BITM_SPIINT_SPIDTERR             0x00000008u    /*  SPI Data Tunnel Error */
#define A2B_BITM_SPIINT_SPII2CERR            0x00000004u    /*  SPI Remote I2C Access Error (Master Only) */
#define A2B_BITM_SPIINT_SPIREGERR            0x00000002u    /*  SPI Remote Register Access Error (Master Only) */
#define A2B_BITM_SPIINT_SPIDONE              0x00000001u    /*  SPI Done Interrupt */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SPIMSK                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SPIMSK_FIFOUIEN              6u            /*  SPI FIFO Underflow Interrupt Enable */
#define A2B_BITP_SPIMSK_FIFOOIEN              5u            /*  SPI FIFO Overflow Interrupt Enable */
#define A2B_BITP_SPIMSK_BADCMDIEN             4u            /*  SPI Bad Command Interrupt Enable */
#define A2B_BITP_SPIMSK_SPIDTIEN              3u            /*  SPI Data Tunnel Interrupt Enable */
#define A2B_BITP_SPIMSK_SPII2CIEN             2u            /*  SPI I2C Interrupt Enable */
#define A2B_BITP_SPIMSK_SPIREGIEN             1u            /*  SPI Register Interrupt Enable */
#define A2B_BITP_SPIMSK_SPIDIEN               0u            /*  SPI Done Interrupt Enable */
#define A2B_BITM_SPIMSK_FIFOUIEN             0x00000040u    /*  SPI FIFO Underflow Interrupt Enable */
#define A2B_BITM_SPIMSK_FIFOOIEN             0x00000020u    /*  SPI FIFO Overflow Interrupt Enable */
#define A2B_BITM_SPIMSK_BADCMDIEN            0x00000010u    /*  SPI Bad Command Interrupt Enable */
#define A2B_BITM_SPIMSK_SPIDTIEN             0x00000008u    /*  SPI Data Tunnel Interrupt Enable */
#define A2B_BITM_SPIMSK_SPII2CIEN            0x00000004u    /*  SPI I2C Interrupt Enable */
#define A2B_BITM_SPIMSK_SPIREGIEN            0x00000002u    /*  SPI Register Interrupt Enable */
#define A2B_BITM_SPIMSK_SPIDIEN              0x00000001u    /*  SPI Done Interrupt Enable */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK0                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK0_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK0_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK1                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK1_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK1_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK2                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK2_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK2_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK3                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK3_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK3_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK4                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK4_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK4_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK5                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK5_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK5_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK6                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK6_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK6_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_RXMASK7                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_RXMASK7_RXMASK               0u            /*  I2S/TDM RX Mask */
#define A2B_BITM_RXMASK7_RXMASK              0x000000FFu    /*  I2S/TDM RX Mask */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR0                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR0_LOC0                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 0 */
#define A2B_BITM_TXXBAR0_LOC0                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 0 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR1                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR1_LOC1                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 1 */
#define A2B_BITM_TXXBAR1_LOC1                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 1 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR2                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR2_LOC2                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 2 */
#define A2B_BITM_TXXBAR2_LOC2                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 2 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR3                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR3_LOC3                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 3 */
#define A2B_BITM_TXXBAR3_LOC3                0x0000001F    /*  Frame Buffer Location for I2S/TDM TX Channel 3 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR4                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR4_LOC4                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 4 */
#define A2B_BITM_TXXBAR4_LOC4                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 4 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR5                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR5_LOC5                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 5 */
#define A2B_BITM_TXXBAR5_LOC5                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 5 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR6                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR6_LOC6                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 6 */
#define A2B_BITM_TXXBAR6_LOC6                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 6 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR7                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR7_LOC7                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 7 */
#define A2B_BITM_TXXBAR7_LOC7                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 7 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR8                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR8_LOC8                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 8 */
#define A2B_BITM_TXXBAR8_LOC8                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 8 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR9                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR9_LOC9                 0u            /*  Frame Buffer Location for I2S/TDM TX Channel 9 */
#define A2B_BITM_TXXBAR9_LOC9                0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 9 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR10                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR10_LOC10               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 10 */
#define A2B_BITM_TXXBAR10_LOC10              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 10 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR11                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR11_LOC11               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 11 */
#define A2B_BITM_TXXBAR11_LOC11              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 11 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR12                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR12_LOC12               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 12 */
#define A2B_BITM_TXXBAR12_LOC12              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 12 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR13                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR13_LOC13               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 13 */
#define A2B_BITM_TXXBAR13_LOC13              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 13 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR14                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR14_LOC14               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 14 */
#define A2B_BITM_TXXBAR14_LOC14              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 14 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR15                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR15_LOC15               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 15 */
#define A2B_BITM_TXXBAR15_LOC15              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 15 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR16                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR16_LOC16               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 16 */
#define A2B_BITM_TXXBAR16_LOC16              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 16 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR17                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR17_LOC17               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 17 */
#define A2B_BITM_TXXBAR17_LOC17              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 17 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR18                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR18_LOC18               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 18 */
#define A2B_BITM_TXXBAR18_LOC18              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 18 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR19                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR19_LOC19               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 19 */
#define A2B_BITM_TXXBAR19_LOC19              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 19 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR20                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR20_LOC20               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 20 */
#define A2B_BITM_TXXBAR20_LOC20              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 20 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR21                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR21_LOC21               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 21 */
#define A2B_BITM_TXXBAR21_LOC21              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 21 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR22                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR22_LOC22               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 22 */
#define A2B_BITM_TXXBAR22_LOC22              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 22 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR23                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR23_LOC23               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 23 */
#define A2B_BITM_TXXBAR23_LOC23              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 23 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR24                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR24_LOC24               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 24 */
#define A2B_BITM_TXXBAR24_LOC24              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 24 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR25                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR25_LOC25               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 25 */
#define A2B_BITM_TXXBAR25_LOC25              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 25 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR26                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR26_LOC26               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 26 */
#define A2B_BITM_TXXBAR26_LOC26              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 26 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR27                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR27_LOC27               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 27 */
#define A2B_BITM_TXXBAR27_LOC27              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 27 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR28                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR28_LOC28               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 28 */
#define A2B_BITM_TXXBAR28_LOC28              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 28 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR29                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR29_LOC29               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 29 */
#define A2B_BITM_TXXBAR29_LOC29              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 29 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR30                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR30_LOC30               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 30 */
#define A2B_BITM_TXXBAR30_LOC30              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 30 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TXXBAR31                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TXXBAR31_LOC31               0u            /*  Frame Buffer Location for I2S/TDM TX Channel 31 */
#define A2B_BITM_TXXBAR31_LOC31              0x0000001Fu    /*  Frame Buffer Location for I2S/TDM TX Channel 31 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_MMRPAGE                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MMRPAGE_PAGE                 0u            /*  Page for MMR Accesses */
#define A2B_BITM_MMRPAGE_PAGE                0x000000FFu    /*  Page for MMR Accesses */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TM1KEY                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TM1KEY_TM1KEY                0u            /*  Testmode 1 Key Bit Field */
#define A2B_BITM_TM1KEY_TM1KEY               0x000000FFu    /*  Testmode 1 Key Bit Field */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_TM3KEY                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_TM3KEY_TM3KEY                0u            /*  Testmode 3 Key Bit Field */
#define A2B_BITM_TM3KEY_TM3KEY               0x000000FFu    /*  Testmode 3 Key Bit Field */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VEN                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VEN_VLTG7               7u            /*  Voltage Input 7 Enable */
#define A2B_BITP_VMTR_VEN_VLTG60              0u            /*  Voltage Inputs[6:0] Enable */
#define A2B_BITM_VMTR_VEN_VLTG7              0x00000080u    /*  Voltage Input 7 Enable */
#define A2B_BITM_VMTR_VEN_VLTG60             0x0000007Fu    /*  Voltage Inputs[6:0] Enable */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_INTEN                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_INTEN_VLTG7             7u            /*  Enable Interrupt for MIN or MAX errors 7 */
#define A2B_BITP_VMTR_INTEN_VLTG60            0u            /*  Enable Interrupt for MIN or MAX errors 0 to 6 */
#define A2B_BITM_VMTR_INTEN_VLTG7            0x00000080u    /*  Enable Interrupt for MIN or MAX errors 7 */
#define A2B_BITM_VMTR_INTEN_VLTG60           0x0000007Fu    /*  Enable Interrupt for MIN or MAX errors 0 to 6 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_MXSTAT                      Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_MXSTAT_MXERR7           7u            /*  VMAX Error */
#define A2B_BITP_VMTR_MXSTAT_MXERR60          0u            /*  VMAX Error */
#define A2B_BITM_VMTR_MXSTAT_MXERR7          0x00000080u    /*  VMAX Error */
#define A2B_BITM_VMTR_MXSTAT_MXERR60         0x0000007Fu    /*  VMAX Error */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_MNSTAT                      Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_MNSTAT_MNERR7           7u            /*  VMIN Error */
#define A2B_BITP_VMTR_MNSTAT_MNERR60          0u            /*  VMIN Error */
#define A2B_BITM_VMTR_MNSTAT_MNERR7          0x00000080u    /*  VMIN Error */
#define A2B_BITM_VMTR_MNSTAT_MNERR60         0x0000007Fu    /*  VMIN Error */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_CHP1L                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_CHP1L_DOUT70            0u            /*  ADC_DOUT[7:0] for VMTR_CHOP = 1 */
#define A2B_BITM_VMTR_CHP1L_DOUT70           0x000000FFu    /*  ADC_DOUT[7:0] for VMTR_CHOP = 1 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_CHP1H                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_CHP1H_DOUT98            0u            /*  ADC_DOUT[9:8] for VMTR_CHOP = 1 */
#define A2B_BITM_VMTR_CHP1H_DOUT98           0x00000003u    /*  ADC_DOUT[9:8] for VMTR_CHOP = 1 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_CHP0L                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_CHP0L_DOUT70            0u            /*  ADC_DOUT[7:0] for VMTR_CHOP = 0 */
#define A2B_BITM_VMTR_CHP0L_DOUT70           0x000000FFu    /*  ADC_DOUT[7:0] for VMTR_CHOP = 0 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_CHP0H                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_CHP0H_DOUT98            0u            /*  ADC_DOUT[9:8] for VMTR_CHOP = 0 */
#define A2B_BITM_VMTR_CHP0H_DOUT98           0x00000003u    /*  ADC_DOUT[9:8] for VMTR_CHOP = 0 */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_ENG                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_ENG_SEPN                6u            /*  Select Vinn - Vinp Inputs */
#define A2B_BITP_VMTR_ENG_EXTCM               5u            /*  External VCM Select */
#define A2B_BITP_VMTR_ENG_EXTREF              4u            /*  External VREF Select */
#define A2B_BITP_VMTR_ENG_BUFBYP              3u            /*  Buffer Bypass Select */
#define A2B_BITP_VMTR_ENG_STRTTST             2u            /*  Start Engineering Test */
#define A2B_BITP_VMTR_ENG_TSELST              0u            /*  Select Setup Time */
#define A2B_BITM_VMTR_ENG_SEPN               0x000000C0u    /*  Select Vinn - Vinp Inputs */
#define A2B_BITM_VMTR_ENG_EXTCM              0x00000020u    /*  External VCM Select */
#define A2B_BITM_VMTR_ENG_EXTREF             0x00000010u    /*  External VREF Select */
#define A2B_BITM_VMTR_ENG_BUFBYP             0x00000008u    /*  Buffer Bypass Select */
#define A2B_BITM_VMTR_ENG_STRTTST            0x00000004u    /*  Start Engineering Test */
#define A2B_BITM_VMTR_ENG_TSELST             0x00000003u    /*  Select Setup Time */
#define A2B_ENUM_VMTR_ENG_SEP0SEN0           0x00000000u            /*  SEPN: Select vinp, GND Inputs */
#define A2B_ENUM_VMTR_ENG_SEP0SEN1           0x00000040u            /*  SEPN: Select vinp, vnn Inputs */
#define A2B_ENUM_VMTR_ENG_SEP1SEN0           0x00000080u            /*  SEPN: Reserved */
#define A2B_ENUM_VMTR_ENG_SEP1SEN1           0x000000C0u            /*  SEPN: Select GND, vinn Inputs */
#define A2B_ENUM_VMTR_ENG_TSELST08           0x00000000u            /*  tSELST: tSELST = 0.8 us */
#define A2B_ENUM_VMTR_ENG_TSELST10           0x00000001u            /*  tSELST: tSELST = 1.0 us */
#define A2B_ENUM_VMTR_ENG_TSELST12           0x00000002u            /*  tSELST: tSELST = 1.2 us */
#define A2B_ENUM_VMTR_ENG_TSELST14           0x00000003u            /*  tSELST: tSELST = 1.4 us */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG0                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG0_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG0_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX0                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX0_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX0_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN0                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN0_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN0_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG1                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG1_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG1_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX1                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX1_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX1_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN1                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN1_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN1_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG2                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG2_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG2_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX2                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX2_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX2_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN2                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN2_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN2_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG3                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG3_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG3_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX3                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX3_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX3_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN3                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN3_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN3_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG4                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG4_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG4_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX4                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX4_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX4_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN4                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN4_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN4_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG5                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG5_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG5_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX5                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX5_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX5_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN5                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN5_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN5_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG6                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG6_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG6_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX6                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX6_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX6_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN6                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN6_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN6_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VLTG7                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VLTG7_VLTG              0u            /*  Measured Voltage */
#define A2B_BITM_VMTR_VLTG7_VLTG             0x000000FFu    /*  Measured Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMAX7                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMAX7_VMAX              0u            /*  User Defined MAX Voltage */
#define A2B_BITM_VMTR_VMAX7_VMAX             0x000000FFu    /*  User Defined MAX Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_VMTR_VMIN7                       Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_VMTR_VMIN7_VMIN              0u            /*  User Defined MIN Voltage */
#define A2B_BITM_VMTR_VMIN7_VMIN             0x000000FFu    /*  User Defined MIN Voltage */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWMCFG                           Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWMCFG_PWMORAND              5u            /*  PWM OE Pin Random Frequency Enable */
#define A2B_BITP_PWMCFG_PWMPRAND              4u            /*  PWMx Pin Random Frequency Enable */
#define A2B_BITP_PWMCFG_PWMOEEN               3u            /*  PWM OE Pin Enable */
#define A2B_BITP_PWMCFG_PWM3EN                2u            /*  PWM3 Pin Enable */
#define A2B_BITP_PWMCFG_PWM2EN                1u            /*  PWM2 Pin Enable */
#define A2B_BITP_PWMCFG_PWM1EN                0u            /*  PWM1 Pin Enable */
#define A2B_BITM_PWMCFG_PWMORAND             0x00000020u    /*  PWM OE Pin Random Frequency Enable */
#define A2B_BITM_PWMCFG_PWMPRAND             0x00000010u    /*  PWMx Pin Random Frequency Enable */
#define A2B_BITM_PWMCFG_PWMOEEN              0x00000008u    /*  PWM OE Pin Enable */
#define A2B_BITM_PWMCFG_PWM3EN               0x00000004u    /*  PWM3 Pin Enable */
#define A2B_BITM_PWMCFG_PWM2EN               0x00000002u    /*  PWM2 Pin Enable */
#define A2B_BITM_PWMCFG_PWM1EN               0x00000001u    /*  PWM1 Pin Enable */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWMFREQ                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWMFREQ_PWMOFREQ             4u            /*  PWM OE Pin Frequency */
#define A2B_BITP_PWMFREQ_PWMPFREQ             0u            /*  PWMx Pin Frequency */
#define A2B_BITM_PWMFREQ_PWMOFREQ            0x000000F0u    /*  PWM OE Pin Frequency */
#define A2B_BITM_PWMFREQ_PWMPFREQ            0x0000000Fu    /*  PWMx Pin Frequency */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_0          0x00000000u            /*  PWMOFREQ: 192 kHz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_1          0x00000010u            /*  PWMOFREQ: 96 kHz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_2          0x00000020u            /*  PWMOFREQ: 48 kHz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_3          0x00000030u            /*  PWMOFREQ: 24 kHz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_4          0x00000040u            /*  PWMOFREQ: 12 kHz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_5          0x00000050u            /*  PWMOFREQ: 6 kHz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_6          0x00000060u            /*  PWMOFREQ: 3 kHz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_7          0x00000070u            /*  PWMOFREQ: 1500 Hz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_8          0x00000080u            /*  PWMOFREQ: 750 Hz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_9          0x00000090u            /*  PWMOFREQ: 375 Hz */
#define A2B_ENUM_PWMFREQ_PWMOFREQ_A          0x000000A0u            /*  PWMOFREQ: 187.5 Hz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_0          0x00000000u            /*  PWMPFREQ: 192 kHz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_1          0x00000001u            /*  PWMPFREQ: 96 kHz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_2          0x00000002u            /*  PWMPFREQ: 48 kHz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_3          0x00000003u            /*  PWMPFREQ: 24 kHz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_4          0x00000004u            /*  PWMPFREQ: 12 kHz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_5          0x00000005u            /*  PWMPFREQ: 6 kHz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_6          0x00000006u            /*  PWMPFREQ: 3 kHz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_7          0x00000007u            /*  PWMPFREQ: 1500 Hz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_8          0x00000008u            /*  PWMPFREQ: 750 Hz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_9          0x00000009u            /*  PWMPFREQ: 375 Hz */
#define A2B_ENUM_PWMFREQ_PWMPFREQ_A          0x0000000Au            /*  PWMPFREQ: 187.5 Hz */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWMBLINK1                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWMBLINK1_PWM2BLINK          4u            /*  PWM2 Blink Rate */
#define A2B_BITP_PWMBLINK1_PWM1BLINK          0u            /*  PWM1 Blink Rate */
#define A2B_BITM_PWMBLINK1_PWM2BLINK         0x00000070u    /*  PWM2 Blink Rate */
#define A2B_BITM_PWMBLINK1_PWM1BLINK         0x00000007u    /*  PWM1 Blink Rate */
#define A2B_ENUM_PWMBLINK1_PWM2BLINK_0       0x00000000u            /*  PWM2BLINK: No Blink */
#define A2B_ENUM_PWMBLINK1_PWM2BLINK_1       0x00000010u            /*  PWM2BLINK: 1/4 Second Blink */
#define A2B_ENUM_PWMBLINK1_PWM2BLINK_2       0x00000020u            /*  PWM2BLINK: 1/2 Second Blink */
#define A2B_ENUM_PWMBLINK1_PWM2BLINK_3       0x00000030u            /*  PWM2BLINK: 3/4 Second Blink */
#define A2B_ENUM_PWMBLINK1_PWM2BLINK_4       0x00000040u            /*  PWM2BLINK: 1 Second Blink */
#define A2B_ENUM_PWMBLINK1_PWM1BLINK_0       0x00000000u            /*  PWM1BLINK: No Blink */
#define A2B_ENUM_PWMBLINK1_PWM1BLINK_1       0x00000001u            /*  PWM1BLINK: 1/4 Second Blink */
#define A2B_ENUM_PWMBLINK1_PWM1BLINK_2       0x00000002u            /*  PWM1BLINK: 1/2 Second Blink */
#define A2B_ENUM_PWMBLINK1_PWM1BLINK_3       0x00000003u            /*  PWM1BLINK: 3/4 Second Blink */
#define A2B_ENUM_PWMBLINK1_PWM1BLINK_4       0x00000004u            /*  PWM1BLINK: 1 Second Blink */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWMBLINK2                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWMBLINK2_PWMOEBLINK         4u            /*  PWM OE Blink Rate */
#define A2B_BITP_PWMBLINK2_PWM3BLINK          0u            /*  PWM3 Blink Rate */
#define A2B_BITM_PWMBLINK2_PWMOEBLINK        0x00000070u    /*  PWM OE Blink Rate */
#define A2B_BITM_PWMBLINK2_PWM3BLINK         0x00000007u    /*  PWM3 Blink Rate */
#define A2B_ENUM_PWMBLINK2_PWMOEBLINK_0      0x00000000u            /*  PWMOEBLINK: No Blink */
#define A2B_ENUM_PWMBLINK2_PWMOEBLINK_1      0x00000010u            /*  PWMOEBLINK: 1/4 Second Blink */
#define A2B_ENUM_PWMBLINK2_PWMOEBLINK_2      0x00000020u            /*  PWMOEBLINK: 1/2 Second Blink */
#define A2B_ENUM_PWMBLINK2_PWMOEBLINK_3      0x00000030u            /*  PWMOEBLINK: 3/4 Second Blink */
#define A2B_ENUM_PWMBLINK2_PWMOEBLINK_4      0x00000040u            /*  PWMOEBLINK: 1 Second Blink */
#define A2B_ENUM_PWMBLINK2_PWM3BLINK_0       0x00000000u            /*  PWM3BLINK: No Blink */
#define A2B_ENUM_PWMBLINK2_PWM3BLINK_1       0x00000001u            /*  PWM3BLINK: 1/4 Second Blink */
#define A2B_ENUM_PWMBLINK2_PWM3BLINK_2       0x00000002u            /*  PWM3BLINK: 1/2 Second Blink */
#define A2B_ENUM_PWMBLINK2_PWM3BLINK_3       0x00000003u            /*  PWM3BLINK: 3/4 Second Blink */
#define A2B_ENUM_PWMBLINK2_PWM3BLINK_4       0x00000004u            /*  PWM3BLINK: 1 Second Blink */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWM1VALL                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWM1VALL_PWM1VAL             0u            /*  PWM1 Value */
#define A2B_BITM_PWM1VALL_PWM1VAL            0x000000FFu    /*  PWM1 Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWM1VALH                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWM1VALH_PWM1VAL             0u            /*  PWM1 Value */
#define A2B_BITM_PWM1VALH_PWM1VAL            0x000000FFu    /*  PWM1 Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWM2VALL                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWM2VALL_PWM2VAL             0u            /*  PWM2 Value */
#define A2B_BITM_PWM2VALL_PWM2VAL            0x000000FFu    /*  PWM2 Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWM2VALH                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWM2VALH_PWM2VAL             0u            /*  PWM2 Value */
#define A2B_BITM_PWM2VALH_PWM2VAL            0x000000FFu    /*  PWM2 Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWM3VALL                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWM3VALL_PWM3VAL             0u            /*  PWM3 Value */
#define A2B_BITM_PWM3VALL_PWM3VAL            0x000000FFu    /*  PWM3 Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWM3VALH                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWM3VALH_PWM3VAL             0u            /*  PWM3 Value */
#define A2B_BITM_PWM3VALH_PWM3VAL            0x000000FFu    /*  PWM3 Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWMOEVALL                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWMOEVALL_PWMOEVAL           0u            /*  PWM OE Value */
#define A2B_BITM_PWMOEVALL_PWMOEVAL          0x000000FFu    /*  PWM OE Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_PWMOEVALH                        Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_PWMOEVALH_PWMOEVAL           0u            /*  PWM OE Value */
#define A2B_BITM_PWMOEVALH_PWMOEVAL          0x000000FFu    /*  PWM OE Value */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_MMRPAGE1                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_MMRPAGE1_PAGE                0u            /*  Page for MMR Accesses */
#define A2B_BITM_MMRPAGE1_PAGE               0x000000FFu    /*  Page for MMR Accesses */

/* -------------------------------------------------------------------------------------------------------------------------
A2B_BSDSTAT                         Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_BSDSTAT_BSDMODE             7u             /*  BSD Mode Bit. */
#define A2B_BITM_BSDSTAT_BSDMODE             0x00000080u    /*  BSD Mode Bit Mask */

#define A2B_BITP_BSDSTAT_BSDACTIVE           6u             /*  BSD Active Bit. */
#define A2B_BITM_BSDSTAT_BSDACTIVE           0x00000040u    /*  BSD Active Bit Mask */

#define A2B_BITP_BSDSTAT_BSDPLLLCK           5u             /*  BSD PLL Locked Bit. */
#define A2B_BITM_BSDSTAT_BSDPLLLCK           0x00000020u    /*  BSD PLL Locked Bit Mask */

#define A2B_BITP_BSDSTAT_BSDNODE             0u
#define A2B_BITM_BSDSTAT_BSDNODE             0x0000000Fu    /*  BSD Nodes Discovered Mask */

#define A2B_BITP_SWCTL2_HPSW_CFG			 0x00000000u	/*  High Power Switch config position */
#define A2B_BITM_SWCTL2_HPSW_CFG			 0x00000007u    /*  High Power Switch config mask */




/* -------------------------------------------------------------------------------------------------------------------------
For all SPI over distance communication types, the command must be followed by node/SS byte. The Data Tunnel
Owner SPI Slave Select/Node Byte shows the slave select/node byte.

Data Tunnel Owner SPI Slave Select/Node Byte   Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */

#define A2B_BITP_SPISSNB_NODEID				0U
#define A2B_BITP_SPISSNB_MS					5U
#define A2B_BITP_SPISSNB_SSEL				6U

#define A2B_BITM_SPISSNB_NODEID				0x0FU
#define A2B_BITM_SPISSNB_MS					0x20U
#define A2B_BITM_SPISSNB_SSEL				0xC0U

/* -------------------------------------------------------------------------------------------------------------------------
A2B_SWSTAT2                          Pos/Masks         Description
------------------------------------------------------------------------------------------------------------------------- */
#define A2B_BITP_SWSTAT2_LVI_MODE             7u            /*  Voltage Regulator Status Bit */
#define A2B_BITP_SWSTAT2_NO_VBUS              6u            /*  Using VIN as Primary Sense Point */
#define A2B_BITP_SWSTAT2_HPSW_CFG_DET         3u            /*  High Power Switch Config Detect Status Bits */
#define A2B_BITP_SWSTAT2_HS_ILIM              1u            /*  Current Limit on High-Side FET Reached */
#define A2B_BITP_SWSTAT2_LS_ILIM              0u            /*  Current Limit on Low-Side FET Reached */
#define A2B_BITM_SWSTAT2_LVI_MODE            0x00000080u    /*  Voltage Regulator Status Bit */
#define A2B_BITM_SWSTAT2_NO_VBUS             0x00000040u    /*  Using VIN as Primary Sense Point */
#define A2B_BITM_SWSTAT2_HPSW_CFG_DET        0x00000038u    /*  High Power Switch Config Detect Status Bits */
#define A2B_BITM_SWSTAT2_HS_ILIM             0x00000002u    /*  Current Limit on High-Side FET Reached */
#define A2B_BITM_SWSTAT2_LS_ILIM             0x00000001u    /*  Current Limit on Low-Side FET Reached */
/*======================= D A T A T Y P E S =======================*/

//A2B_BEGIN_DECLS

/*======================= P U B L I C  P R O T O T Y P E S ========*/


//A2B_END_DECLS

/*======================= D A T A =================================*/

/** \} -- a2bstack_regdefs */

#endif /* A2B_REGDEFS_H_ */
