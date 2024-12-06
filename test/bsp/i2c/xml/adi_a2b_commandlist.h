/*******************************************************************************
Copyright (c) 2024 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @brief: This file contains I2C/SPI command sequence to be followed for 
*         discovery and configuration of A2B nodes for an A2B schematic
* @version: $Revision$
* @date: Friday, December 6, 2024-9:56:32 AM
* I2C/SPI Command File Version - 1.0.0
* A2B DLL version- 19.4.5.0
* A2B Stack DLL version- 19.4.5.0
* SigmaStudio version- 4.07.000.1831
* Developed by: Automotive Software and Systems team, Bangalore, India
* THIS IS A SIGMASTUDIO GENERATED FILE
*****************************************************************************/

/*! \addtogroup ADI_A2B_DISCOVERY_CONFIG ADI_A2B_DISCOVERY_CONFIG 
* @{
*/
#ifndef _ADI_A2B_CMD_LIST_H_ 
#define _ADI_A2B_CMD_LIST_H_ 

/*! \struct ADI_A2B_DISCOVERY_CONFIG 
A2B discovery config unit structure 
*/
typedef struct 
 { 
/*! Device address */
	unsigned char nDeviceAddr;

/*! Operation code */
	unsigned char eOpCode;

/*! Reg Sub address width (in bytes) */
	unsigned char nAddrWidth;

/*! Reg Sub address */
	unsigned int nAddr;

/*! Reg data width (in bytes) */
	unsigned char nDataWidth;

/*! Reg data count (in bytes) */
	unsigned short nDataCount;

/*! Config Data */
	unsigned char* paConfigData;

/*! SPI Command width (in bytes), not used for I2C */
	unsigned char nSpiCmdWidth;

/*! SPI Commands, not used for I2C */
	unsigned int nSpiCmd;

/*! Protocol */
	unsigned char eProtocol;

} ADI_A2B_DISCOVERY_CONFIG;

#define WRITE   ((unsigned char) 0x00u)
#define READ    ((unsigned char) 0x01u)
#define DELAY   ((unsigned char) 0x02u)
#define INVALID ((unsigned char) 0xffu)

#define I2C     ((unsigned char) 0x00u)
#define SPI     ((unsigned char) 0x01u)

#define CONFIG_LEN (50) 


static unsigned char gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data0[1] =
{
	0x84u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data1[1] =
{
	0x19u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_INTTYPE_Data2[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_INTMSK0_Data3[1] =
{
	0x10u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_INTMSK2_Data4[1] =
{
	0x0Bu	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_BECCTL_Data5[1] =
{
	0xEFu	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_INTPND2_Data6[1] =
{
	0x01u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_RESPCYCS_Data7[1] =
{
	0x7Du	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data8[1] =
{
	0x81u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data9[1] =
{
	0x01u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_I2SGCFG_Data10[1] =
{
	0x22u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_SWCTL_Data11[1] =
{
	0x01u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_DISCVRY_Data12[1] =
{
	0x7Du	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data13[1] =
{
	0x32u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_INTTYPE_Data14[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_SWCTL_Data15[1] =
{
	0x21u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data16[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_VENDOR_Data0[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_PRODUCT_Data1[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_VERSION_Data2[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data17[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_LDNSLOTS_Data3[1] =
{
	0x80u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_LUPSLOTS_Data4[1] =
{
	0x08u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_I2CCFG_Data5[1] =
{
	0x01u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_I2SGCFG_Data6[1] =
{
	0x22u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_I2SCFG_Data7[1] =
{
	0x11u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_PDMCTL_Data8[1] =
{
	0x18u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_GPIODAT_Data9[1] =
{
	0x10u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_GPIOOEN_Data10[1] =
{
	0x10u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_PINCFG_Data11[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_CLK2CFG_Data12[1] =
{
	0xC1u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_DNMASK0_Data13[1] =
{
	0xFFu	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_MBOX1CTL_Data14[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_INTMSK0_Data15[1] =
{
	0x10u	
};

static unsigned char gaConfig_A2BSlaveNode1WBZ_BECCTL_Data16[1] =
{
	0xEFu	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_I2SCFG_Data18[1] =
{
	0x11u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_PINCFG_Data19[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_BECCTL_Data20[1] =
{
	0xEFu	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_DNSLOTS_Data21[1] =
{
	0x08u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_UPSLOTS_Data22[1] =
{
	0x08u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_SWCTL_Data23[1] =
{
	0x01u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data24[1] =
{
	0x80u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data25[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_SLOTFMT_Data26[1] =
{
	0x44u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_DATCTL_Data27[1] =
{
	0x23u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_I2SRRATE_Data28[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data29[1] =
{
	0x81u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data30[1] =
{
	0x01u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data31[1] =
{
	0x00u	
};

static unsigned char gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data32[1] =
{
	0x82u	
};

ADI_A2B_DISCOVERY_CONFIG gaA2BConfig[CONFIG_LEN] =
{

	/*-- COMMANDS FOR DEVICE - A2B Master Node WD1BZ --*/
	{0x68u,	WRITE,	0x01u,	0x00000012u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data0[0] },	/* CONTROL */
	{0x00u,	DELAY,	0x01u,	0x00000000u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data1[0] },	/* A2B_Delay */
	{0x68u,	READ,	0x01u,	0x00000017u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_INTTYPE_Data2[0] },	/* INTTYPE */
	{0x68u,	WRITE,	0x01u,	0x0000001Bu,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_INTMSK0_Data3[0] },	/* INTMSK0 */
	{0x68u,	WRITE,	0x01u,	0x0000001Du,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_INTMSK2_Data4[0] },	/* INTMSK2 */
	{0x68u,	WRITE,	0x01u,	0x0000001Eu,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_BECCTL_Data5[0] },	/* BECCTL */
	{0x68u,	WRITE,	0x01u,	0x0000001Au,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_INTPND2_Data6[0] },	/* INTPND2 */
	{0x68u,	WRITE,	0x01u,	0x0000000Fu,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_RESPCYCS_Data7[0] },	/* RESPCYCS */
	{0x68u,	WRITE,	0x01u,	0x00000012u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data8[0] },	/* CONTROL */
	{0x00u,	DELAY,	0x01u,	0x00000000u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data9[0] },	/* A2B_Delay */
	{0x68u,	WRITE,	0x01u,	0x00000041u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_I2SGCFG_Data10[0] },	/* I2SGCFG */
	{0x68u,	WRITE,	0x01u,	0x00000009u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_SWCTL_Data11[0] },	/* SWCTL */
	{0x68u,	WRITE,	0x01u,	0x00000013u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_DISCVRY_Data12[0] },	/* DISCVRY */
	{0x00u,	DELAY,	0x01u,	0x00000000u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data13[0] },	/* A2B_Delay */
	{0x68u,	READ,	0x01u,	0x00000017u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_INTTYPE_Data14[0] },	/* INTTYPE */
	{0x68u,	WRITE,	0x01u,	0x00000009u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_SWCTL_Data15[0] },	/* SWCTL */
	{0x68u,	WRITE,	0x01u,	0x00000001u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data16[0] },	/* NODEADR */

	/*-- COMMANDS FOR DEVICE - A2B Slave Node1 WBZ --*/
	{0x69u,	READ,	0x01u,	0x00000002u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_VENDOR_Data0[0] },	/* VENDOR */
	{0x69u,	READ,	0x01u,	0x00000003u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_PRODUCT_Data1[0] },	/* PRODUCT */
	{0x69u,	READ,	0x01u,	0x00000004u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_VERSION_Data2[0] },	/* VERSION */

	/*-- COMMANDS FOR DEVICE - A2B Master Node WD1BZ --*/
	{0x68u,	WRITE,	0x01u,	0x00000001u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data17[0] },	/* NODEADR */

	/*-- COMMANDS FOR DEVICE - A2B Slave Node1 WBZ --*/
	{0x69u,	WRITE,	0x01u,	0x0000000Bu,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_LDNSLOTS_Data3[0] },	/* LDNSLOTS */
	{0x69u,	WRITE,	0x01u,	0x0000000Cu,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_LUPSLOTS_Data4[0] },	/* LUPSLOTS */
	{0x69u,	WRITE,	0x01u,	0x0000003Fu,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_I2CCFG_Data5[0] },	/* I2CCFG */
	{0x69u,	WRITE,	0x01u,	0x00000041u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_I2SGCFG_Data6[0] },	/* I2SGCFG */
	{0x69u,	WRITE,	0x01u,	0x00000042u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_I2SCFG_Data7[0] },	/* I2SCFG */
	{0x69u,	WRITE,	0x01u,	0x00000047u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_PDMCTL_Data8[0] },	/* PDMCTL */
	{0x69u,	WRITE,	0x01u,	0x0000004Au,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_GPIODAT_Data9[0] },	/* GPIODAT */
	{0x69u,	WRITE,	0x01u,	0x0000004Du,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_GPIOOEN_Data10[0] },	/* GPIOOEN */
	{0x69u,	WRITE,	0x01u,	0x00000052u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_PINCFG_Data11[0] },	/* PINCFG */
	{0x69u,	WRITE,	0x01u,	0x0000005Au,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_CLK2CFG_Data12[0] },	/* CLK2CFG */
	{0x69u,	WRITE,	0x01u,	0x00000065u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_DNMASK0_Data13[0] },	/* DNMASK0 */
	{0x69u,	WRITE,	0x01u,	0x00000096u,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_MBOX1CTL_Data14[0] },	/* MBOX1CTL */
	{0x69u,	WRITE,	0x01u,	0x0000001Bu,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_INTMSK0_Data15[0] },	/* INTMSK0 */
	{0x69u,	WRITE,	0x01u,	0x0000001Eu,	0x01u,	0x0001u,	&gaConfig_A2BSlaveNode1WBZ_BECCTL_Data16[0] },	/* BECCTL */

	/*-- COMMANDS FOR DEVICE - A2B Master Node WD1BZ --*/
	{0x68u,	WRITE,	0x01u,	0x00000042u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_I2SCFG_Data18[0] },	/* I2SCFG */
	{0x68u,	WRITE,	0x01u,	0x00000052u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_PINCFG_Data19[0] },	/* PINCFG */
	{0x68u,	WRITE,	0x01u,	0x0000001Eu,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_BECCTL_Data20[0] },	/* BECCTL */
	{0x68u,	WRITE,	0x01u,	0x0000000Du,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_DNSLOTS_Data21[0] },	/* DNSLOTS */
	{0x68u,	WRITE,	0x01u,	0x0000000Eu,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_UPSLOTS_Data22[0] },	/* UPSLOTS */
	{0x68u,	WRITE,	0x01u,	0x00000009u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_SWCTL_Data23[0] },	/* SWCTL */
	{0x68u,	WRITE,	0x01u,	0x00000001u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data24[0] },	/* NODEADR */
	{0x68u,	WRITE,	0x01u,	0x00000001u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data25[0] },	/* NODEADR */
	{0x68u,	WRITE,	0x01u,	0x00000010u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_SLOTFMT_Data26[0] },	/* SLOTFMT */
	{0x68u,	WRITE,	0x01u,	0x00000011u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_DATCTL_Data27[0] },	/* DATCTL */
	{0x68u,	WRITE,	0x01u,	0x00000056u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_I2SRRATE_Data28[0] },	/* I2SRRATE */
	{0x68u,	WRITE,	0x01u,	0x00000012u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data29[0] },	/* CONTROL */
	{0x00u,	DELAY,	0x01u,	0x00000000u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_A2BDelay_Data30[0] },	/* A2B_Delay */
	{0x68u,	WRITE,	0x01u,	0x00000001u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_NODEADR_Data31[0] },	/* NODEADR */
	{0x68u,	WRITE,	0x01u,	0x00000012u,	0x01u,	0x0001u,	&gaConfig_A2BMasterNodeWD1BZ_CONTROL_Data32[0] },	/* CONTROL */
};

#endif /* _ADI_A2B_CMD_LIST_H_ */

