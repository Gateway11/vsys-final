/*
 * A2B24XX driver
 *
 * Copyright 2023 Analog Devices Inc.
 *  Author: ADI Automotive Software Team, Bangalore
 *
 * Licensed under the GPL-2.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/string.h>

#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#include "adi_a2b_commandlist.h"
#include "a2b24xx.h"
#include "regdefs.h"

//#define A2B_SETUP_ALSA

#define DEVICE_NAME "a2b_ctrl"   // Device name
#define CLASS_NAME "a2b24xx"    // Device class name
#define COMMAND_SIZE 128        // Buffer size for receiving commands

#define MAX_ACTIONS  256
#define MAX_CONFIG_DATA (MAX_ACTIONS << 6)

/* Define how often to check (and clear) the fault status register (in ms) */
#define A2B24XX_FAULT_CHECK_INTERVAL 6000

struct a2b24xx {
    struct regmap *regmap;
    unsigned int sysclk;
    enum a2b24xx_sysclk_src sysclk_src;
    enum a2b24xx_type type;

    struct snd_pcm_hw_constraint_list constraints;

    struct device *dev;
    void (*switch_mode)(struct device *dev);

    unsigned int max_master_fs;
    uint8_t master_fmt;
    bool master;

    struct work_struct setup_work;
    struct delayed_work fault_check_work;
    struct mutex node_mutex;
    bool fault_check_running;

    uint8_t cycles[16];
    uint8_t slave_pos[16];
    uint8_t max_node_number;

#ifndef A2B_SETUP_ALSA
    dev_t dev_num;              // Device number
    struct cdev cdev;           // cdev structure
    struct class *dev_class;    // Device class
    char command_buffer[COMMAND_SIZE];
#endif

    ADI_A2B_DISCOVERY_CONFIG *pA2BConfig;
    ADI_A2B_DISCOVERY_CONFIG parseA2BConfig[MAX_ACTIONS];
    size_t actionCount;

    uint8_t configBuffer[MAX_CONFIG_DATA];
    size_t bufferOffset;
};

static void adi_a2b_NetworkSetup(struct device* dev);
static int8_t processInterrupt(struct a2b24xx *a2b24xx, bool rediscovry);

static const struct reg_default a2b24xx_reg_defaults[] = {
    { 0x00, 0x50 }
};

/* Example control - no specific functionality */
static const DECLARE_TLV_DB_MINMAX_MUTE(a2b24xx_control, 0, 0);

static int a2b24xx_reset(struct a2b24xx *a2b24xx)
{
    int ret = 0;

    regcache_cache_bypass(a2b24xx->regmap, true);
    /* A2B reset */
    adi_a2b_NetworkSetup(a2b24xx->dev);

    return ret;
}

static int a2b24xx_reset_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
    struct a2b24xx *a2b24xx = snd_soc_component_get_drvdata(component);

    dev_info(component->dev, "A2B reset triggered via SOC_SINGLE_BOOL_EXT\n");
    a2b24xx_reset(a2b24xx);

    return 1;
}

#define A2B24XX_CONTROL(x) \
    SOC_SINGLE_TLV("A2B" #x " Template", 2, 0, 255, 1, a2b24xx_control), \
    SOC_SINGLE_BOOL_EXT("A2B" #x " Reset", 0, a2b24xx_reset_put, NULL),

/* Example control */
static const struct snd_kcontrol_new a2b24xx_snd_controls[] = { A2B24XX_CONTROL(1) };

static void parseAction(struct a2b24xx *a2b24xx, const char *action, ADI_A2B_DISCOVERY_CONFIG *config) {
    char instr[20], protocol[10];

    const char *pos;
    char *endptr;
    int parseCount = 0;  // Initialize the counter for parsed fields
    char buffer[10];  // Temporary buffer to hold the extracted number string

    size_t *bufferOffset = &a2b24xx->bufferOffset;

    // Parse "instr" field
    pos = strstr(action, "instr=\"");
    if (pos) {
        pos += strlen("instr=\"");  // Skip "instr=\""
        endptr = strchr(pos, '\"');
        if (endptr) {
            size_t instrLen = endptr - pos;
            strncpy(instr, pos, instrLen);
            instr[instrLen] = '\0';  // Null-terminate the string
            parseCount++;  // Increment count for parsed field
        }
    }

    // Parse "addr_width" field
    pos = strstr(action, "addr_width=\"");
    if (pos) {
        pos += strlen("addr_width=\"");
        endptr = strchr(pos, '\"');
        if (endptr) {
            // Copy the numeric part to the buffer and null-terminate
            size_t len = endptr - pos;
            strncpy(buffer, pos, len);
            buffer[len] = '\0';

            // Now, use kstrtou8 to convert the number
            if (kstrtou8(buffer, 10, &config->nAddrWidth) == 0) {
                parseCount++;  // Increment count for parsed field
            }
        }
    }

    // Parse "len" field
    pos = strstr(action, "len=\"");
    if (pos) {
        pos += strlen("len=\"");
        endptr = strchr(pos, '\"');
        if (endptr) {
            // Copy the numeric part to the buffer and null-terminate
            size_t len = endptr - pos;
            strncpy(buffer, pos, len);
            buffer[len] = '\0';

            // Now, use kstrtou16 to convert the number
            if (kstrtou16(buffer, 10, &config->nDataCount) == 0) {
                parseCount++;  // Increment count for parsed field
            }
        }
    }

    // Parse "addr" field
    pos = strstr(action, "addr=\"");
    if (pos) {
        pos += strlen("addr=\"");
        endptr = strchr(pos, '\"');
        if (endptr) {
            // Copy the numeric part to the buffer and null-terminate
            size_t len = endptr - pos;
            strncpy(buffer, pos, len);
            buffer[len] = '\0';

            // Now, use kstrtouint to convert the number
            if (kstrtouint(buffer, 10, &config->nAddr) == 0) {
                parseCount++;  // Increment count for parsed field
            }
        }
    }

    // Parse "i2caddr" field
    pos = strstr(action, "i2caddr=\"");
    if (pos) {
        pos += strlen("i2caddr=\"");
        endptr = strchr(pos, '\"');
        if (endptr) {
            // Copy the numeric part to the buffer and null-terminate
            size_t len = endptr - pos;
            strncpy(buffer, pos, len);
            buffer[len] = '\0';

            // Now, use kstrtou8 to convert the number
            if (kstrtou8(buffer, 10, &config->nDeviceAddr) == 0) {
                parseCount++;  // Increment count for parsed field
            }
        }
    }

    // Output total parsed field count
    // pr_info("Total parsed fields: %d\n", parseCount);

    if (parseCount >= 5) {
        if (strcmp(instr, "writeXbytes") == 0) {
            config->eOpCode = A2B24XX_WRITE;
        } else if (strcmp(instr, "read") == 0) {
            config->eOpCode = A2B24XX_READ;
        } else {
            config->eOpCode = A2B24XX_INVALID;
        }
        config->eProtocol = (strcmp(protocol, "SPI") == 0) ? SPI : I2C;
        config->nDataCount -= config->nAddrWidth;
    } else if (strstr(action, "instr=\"delay\"") != NULL) {
        config->eOpCode = A2B24XX_DELAY;
        config->nDataCount = 1;
    } else {
        config->eOpCode = A2B24XX_INVALID;
        return;
    }

    if (config->eOpCode == A2B24XX_WRITE || config->eOpCode == A2B24XX_DELAY) {
        if (*bufferOffset + config->nDataCount > MAX_CONFIG_DATA) {
            pr_warn("Warning: Exceeding maximum configuration data limit!\n");
            return;
        }
        // Parse multiple numbers
        char *dataStr = strchr(action, '>') + 1; /* Find position after '>' */
        char *token = strsep(&dataStr, " ");
        size_t index = 0;
        config->paConfigData = a2b24xx->configBuffer + *bufferOffset;
        while (token != NULL && config->nDataCount) {
            config->paConfigData[index++] = (uint8_t)strtoul(token, NULL, 16); // Convert to hexadecimal
            token = strsep(&dataStr, " ");
        }
        *bufferOffset += index;
        config->nDataCount = index;
    }
}

static void parseXML(struct a2b24xx *a2b24xx, const char *xml) {
    const char *actionStart = strstr(xml, "<action");
    char *action = kmalloc(6000, GFP_KERNEL); // Allocate 6000 bytes of memory for the action buffer
    size_t *actionCount = &a2b24xx->actionCount;

    *actionCount = 0;
    a2b24xx->bufferOffset = 0;

    while (actionStart && *actionCount < MAX_ACTIONS) {
        const char* actionEnd = strchr(actionStart, '\n'); // Use '\n' as end marker
        size_t actionLength = actionEnd - actionStart + 1;

        if (actionLength >= 6000) {
            pr_warn("Warning: Action length exceeds buffer size!\n");
            goto exit;
        }

        strncpy(action, actionStart, actionLength);
        action[actionLength] = '\0'; // Null-terminate

        parseAction(a2b24xx, action, &a2b24xx->parseA2BConfig[*actionCount]);
        (*actionCount)++;
        actionStart = strstr(actionEnd, "<action");
    }
exit:
    kfree(action);
}

static char* a2b_pal_File_Read(const char* filename, size_t* outSize) {
    struct file* file;
    char* buffer = NULL;
    loff_t pos = 0;
    size_t fileSize = 0;
    ssize_t readSize;

    // Open the file (O_RDONLY indicates read-only)
    file = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_err("Failed to open file: %ld\n", PTR_ERR(file));
        return NULL;
    }

    // Get the file size
    fileSize = i_size_read(file->f_path.dentry->d_inode);
    if (fileSize == 0) {
        filp_close(file, NULL);
        return NULL;
    }

    // Allocate buffer to hold the file content
    buffer = kmalloc(fileSize + 1, GFP_KERNEL); // +1 for null terminator
    if (!buffer) {
        filp_close(file, NULL);
        pr_err("Failed to allocate memory\n");
        return NULL;
    }

    // Read the content of the file into the buffer
    readSize = kernel_read(file, buffer, fileSize, &pos);
    if (readSize < 0) {
        kfree(buffer);
        filp_close(file, NULL);
        pr_err("Failed to read file\n");
        return NULL;
    }

    // Null terminate the buffer
    buffer[readSize] = '\0';

    // Close the file
    filp_close(file, NULL);

    // Return the file size
    if (outSize) {
        *outSize = readSize;
    }

    return buffer;
}

/****************************************************************************/
/*!
 @brief          This function calculates reg value based on width and adds
                 it to the data array

 @param [in]     pDstBuf               Pointer to destination array
 @param [in]     nAddrwidth            Data unpacking boundary(1 byte / 2 byte /4 byte )
 @param [in]     nAddr                 Number of words to be copied

 @return         Return code
                 - 0: Success
                 - 1: Failure
 */
/********************************************************************************/
static void adi_a2b_Concat_Addr_Data(unsigned char pDstBuf[], unsigned int nAddrwidth, unsigned int nAddr)
{
    /* Store the read values in the placeholder */
    switch (nAddrwidth) {
        /* Byte */
        case 1u:
            pDstBuf[0u] = (unsigned char)nAddr;
            break;

        /* 16 bit word */
        case 2u:
            pDstBuf[0u] = (unsigned char)(nAddr >> 8u);
            pDstBuf[1u] = (unsigned char)(nAddr & 0xFFu);
            break;

        /* 24 bit word */
        case 3u:
            pDstBuf[0u] = (unsigned char)((nAddr & 0xFF0000u) >> 16u);
            pDstBuf[1u] = (unsigned char)((nAddr & 0xFF00u) >> 8u);
            pDstBuf[2u] = (unsigned char)(nAddr & 0xFFu);
            break;

        /* 32 bit word */
        case 4u:
            pDstBuf[0u] = (unsigned char)(nAddr >> 24u);
            pDstBuf[1u] = (unsigned char)((nAddr & 0xFF0000u) >> 16u);
            pDstBuf[2u] = (unsigned char)((nAddr & 0xFF00u) >> 8u);
            pDstBuf[3u] = (unsigned char)(nAddr & 0xFFu);
            break;

        default:
            break;
    }
}

static int adi_a2b_I2CWrite(struct device* dev, unsigned short devAddr, unsigned short count, char* bytes)
{
    struct i2c_client* client = to_i2c_client(dev);
    client->addr = devAddr;
    return i2c_master_send(client, bytes, count);
}

static int adi_a2b_I2CRead(struct device* dev, uint16_t devAddr, uint16_t writeLength, uint8_t* writeBuffer, uint16_t readLength, uint8_t* readBuffer)
{
    int ret = -1;
    struct i2c_client* client = to_i2c_client(dev);
    client->addr = devAddr;
    unsigned short i = 0;

    struct i2c_msg msg[] = {
        [0] = {
            .addr = client->addr,
            .flags = 0,
            .len = writeLength,
            .buf = writeBuffer,
        },
        [1] = {
            .addr = client->addr,
            .flags = I2C_M_RD,
            .len = readLength,
            .buf = readBuffer,
        },
    };

    ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
    if (ret < 0) {
        pr_err("%s:i2c_transfer failed, reg 0x%02X\n", __func__, writeBuffer[0]);
        return ret;
    }

    pr_info("%s:i2c read device(0x%X) reg 0x%02X, cnt %d, val:", __func__, devAddr, writeBuffer[0], readLength);
    for (i = 0; i < readLength; i++) {
        pr_cont("0x%02X ", readBuffer[i]);
    }
    pr_cont("\n");

    return 0;
}

typedef struct {
    uint8_t type;
    const char *message;
} IntTypeString_t;

const IntTypeString_t intTypeString[] = {
    {A2B_ENUM_INTTYPE_HDCNTERR             ,        "HDCNTERR "},
    {A2B_ENUM_INTTYPE_DDERR                ,        "DDERR "},
    {A2B_ENUM_INTTYPE_CRCERR               ,        "CRCERR "},
    {A2B_ENUM_INTTYPE_DPERR                ,        "DPERR "},
    {A2B_ENUM_INTTYPE_BECOVF               ,        "BECOVF "},
    {A2B_ENUM_INTTYPE_SRFERR               ,        "SRFERR "},
    {A2B_ENUM_INTTYPE_PWRERR_CS_GND        ,        "PWRERR (Cable Shorted to GND) "},
    {A2B_ENUM_INTTYPE_PWRERR_CS_VBAT       ,        "PWRERR (Cable Shorted to VBat) "},
    {A2B_ENUM_INTTYPE_PWRERR_CS            ,        "PWRERR (Cable Shorted Together) "},
    {A2B_ENUM_INTTYPE_PWRERR_CDISC         ,        "PWRERR (Cable Disconnected or Open Circuit) (AD240x/10/2x Slaves Only) "},
    {A2B_ENUM_INTTYPE_PWRERR_CREV          ,        "PWRERR (Cable Reverse Connected) (AD240x/10/2x Slaves Only) "},
    {A2B_ENUM_INTTYPE_PWRERR_CDISC_REV     ,        "PWRERR - Cable is Disconnected (Open Circuit) or Wrong Port or Reverse Connected (AD243x Only) "},
    {A2B_ENUM_INTTYPE_PWRERR_FAULT         ,        "PWRERR (Indeterminate Fault) "},
    //{A2B_ENUM_INTTYPE_IO0PND               ,        "IO0PND - Slave Only "},
    //{A2B_ENUM_INTTYPE_IO1PND               ,        "IO1PND - Slave Only "},
    //{A2B_ENUM_INTTYPE_IO2PND               ,        "IO2PND - Slave Only "},
    //{A2B_ENUM_INTTYPE_IO3PND               ,        "IO3PND "},
    //{A2B_ENUM_INTTYPE_IO4PND               ,        "IO4PND "},
    //{A2B_ENUM_INTTYPE_IO5PND               ,        "IO5PND "},
    //{A2B_ENUM_INTTYPE_IO6PND               ,        "IO6PND "},
    //{A2B_ENUM_INTTYPE_IO7PND               ,        "IO7PND "},
    //{A2B_ENUM_INTTYPE_DSCDONE              ,        "DSCDONE - Master Only "},
    {A2B_ENUM_INTTYPE_I2CERR               ,        "I2CERR - Master Only "},
    {A2B_ENUM_INTTYPE_ICRCERR              ,        "ICRCERR - Master Only "},
    {A2B_ENUM_INTTYPE_PWRERR_NLS_GND       ,        "PWRERR - Non-Localized Short to GND "},
    {A2B_ENUM_INTTYPE_PWRERR_NLS_VBAT      ,        "PWRERR - Non-Localized Short to VBat "},
    {A2B_ENUM_INTTYPE_PWRERR_OTH           ,        "PWRERR - Other Error, Check SWSTAT2/SWSTAT3."},
    //{A2B_ENUM_INTTYPE_SPIDONE              ,        "SPI Done"},
    {A2B_ENUM_INTTYPE_SPI_REMOTE_REG_ERR   ,        "SPI Remote Register Access Error - Master Only"},
    {A2B_ENUM_INTTYPE_SPI_REMOTE_I2C_ERR   ,        "SPI Remote I2C Access Error - Master Only"},
    {A2B_ENUM_INTTYPE_SPI_DATA_TUN_ERR     ,        "SPI Data Tunnel Access Error"},
    {A2B_ENUM_INTTYPE_SPI_BAD_CMD          ,        "SPI Bad Command"},
    {A2B_ENUM_INTTYPE_SPI_FIFO_OVRFLW      ,        "SPI FIFO Overflow"},
    {A2B_ENUM_INTTYPE_SPI_FIFO_UNDERFLW    ,        "SPI FIFO Underflow"},
    {A2B_ENUM_INTTYPE_VMTR                 ,        "VMTR Interrupt"},
    {A2B_ENUM_INTTYPE_IRPT_MSG_ERR         ,        "PWRERR - Interrupt Messaging Error "},
    {A2B_ENUM_INTTYPE_STRTUP_ERR_RTF       ,        "Startup Error - Return to Factory "},
    {A2B_ENUM_INTTYPE_SLAVE_INTTYPE_ERR    ,        "Slave INTTYPE Read Error - Master Only "},
    //{A2B_ENUM_INTTYPE_STANDBY_DONE         ,        "Standby Done - Master Only "},
    //{A2B_ENUM_INTTYPE_MSTR_RUNNING         ,        "MSTR_RUNNING - Master Only "},
};

static bool processSingleNode(struct a2b24xx *a2b24xx, uint8_t inode) {
    struct device *dev = a2b24xx->dev;

    ADI_A2B_DISCOVERY_CONFIG* pOPUnit;
    unsigned char *aDataBuffer = kmalloc(6000, GFP_KERNEL); // Allocate 6000 bytes of memory for the data buffer

    pr_info("Processing fault for node %d: master_fmt=0x%02X, cycle=0x%02X, slave_pos=%d 0x%02X\n",
            inode, a2b24xx->master_fmt, a2b24xx->cycles[inode],
            a2b24xx->slave_pos[inode], a2b24xx->pA2BConfig[a2b24xx->slave_pos[inode]].nAddr);

//1. Open the Slave node0 switch (SWCTL=0) i.e next upstream node and clear interrupt pending bits (INTPEND=0xFF) and wait for 100ms
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, inode - 1});
    adi_a2b_I2CWrite(dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x00});
    mdelay(100);

//Now, periodically try to rediscover the slave-1 partially
//1. Write SWCTL=0x21 to all upsteam nodes other than the node from which we are discovering the next node( In above example Write Master node SWCTL=0x21)
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x21});
    for (uint8_t i = 0; i < (inode - 1); i++) {
        adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, i});
        adi_a2b_I2CWrite(dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x21});
    }

//2. Write SWCTL=0x01 in the next upsteam node of discovering node(SWCTL=0x01 in Slave0)
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, inode - 1});
    adi_a2b_I2CWrite(dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x01});

//3. Write master node DISCVRY register with expected response cycle of slave node to be discovered to start the discovery process(DISCVRY = response cycle of the slave1)
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_DISCVRY, a2b24xx->cycles[inode]});

//4. Wait for 35msec for slave node to discover. Can use IRQ interrupt to check if slave node discovery interrupt is received.
    mdelay(35);
//5. If rediscovery is unsuccessful  i.e. resulted in timeout or Open wire fault (INTTYPE: 0x0C) raised:
//          O End the discovery
//          O Open the Slave node0 switch
//          O Clear interrupts, if any
//          O Wait for 100msec. And reattempt partial rediscovery: from step - 1
    if (processInterrupt(a2b24xx, false) >= 0) {
        adi_a2b_I2CWrite(dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x00});
        kfree(aDataBuffer);
        return false;
    }

//6. If rediscovery is successful (got the slave node discovery interrupt (INTTYPE==0x18),
//          O Update SWCTL=0x01 in all upsteam nodes
//          O After slave register programming, configure master_fmt, DATCTL and NEWSTRCT on master node to start the audio communication.
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x01});
    for (uint8_t i = 0; i < inode; i++) {
        adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, i});
        adi_a2b_I2CWrite(dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x01});
    }
    for (uint32_t i = a2b24xx->slave_pos[inode]; i < a2b24xx->actionCount; i++) {
        pOPUnit = &a2b24xx->pA2BConfig[i];

        adi_a2b_Concat_Addr_Data(&aDataBuffer[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
        (void)memcpy(&aDataBuffer[pOPUnit->nAddrWidth], pOPUnit->paConfigData, pOPUnit->nDataCount);
        adi_a2b_I2CWrite(dev, pOPUnit->nDeviceAddr, (pOPUnit->nAddrWidth + pOPUnit->nDataCount), aDataBuffer);

        if (pOPUnit->nAddr == A2B_REG_INTMSK0) break;

        //For codec
    }
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 4, (uint8_t[]){A2B_REG_SLOTFMT, a2b24xx->master_fmt, 0x03, 0x81});

    kfree(aDataBuffer);
    return true;
}

static void processFaultNode(struct a2b24xx *a2b24xx, uint8_t inode) {
//    uint8_t dataBuffer[1] = {0}; //A2B_REG_NODE
//
//    if (!inode) {
//        adi_a2b_I2CRead(a2b24xx->dev, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_NODE}, 1, dataBuffer);
//    } else {
//        adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, inode - 1});
//        adi_a2b_I2CRead(a2b24xx->dev, A2B_SLAVE_ADDR, 1, (uint8_t[]){A2B_REG_NODE}, 1, dataBuffer);
//    }
//    if (dataBuffer[0] & A2B_BITM_NODE_LAST) {
        if (!inode) {
            /* Setting up A2B network */
            adi_a2b_NetworkSetup(a2b24xx->dev);
        } else {
            for (uint8_t i = inode; i < a2b24xx->max_node_number; i++) {
                if (!processSingleNode(a2b24xx, i)) {
                    break;
                }
                mdelay(100);
            }
        }
//    }
}

static int8_t processInterrupt(struct a2b24xx *a2b24xx, bool rediscovry) {
    uint8_t dataBuffer[2] = {0}; //A2B_REG_INTSRC, A2B_REG_INTTYPE

    adi_a2b_I2CRead(a2b24xx->dev, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_INTSRC}, 1, dataBuffer);
    if (dataBuffer[0]) {
        adi_a2b_I2CRead(a2b24xx->dev, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_INTTYPE}, 1, dataBuffer + 1);
        if (dataBuffer[0] & A2B_BITM_INTSRC_MSTINT) {
            pr_info("Interrupt Source: Master - ");
        } else if (dataBuffer[0] & A2B_BITM_INTSRC_SLVINT) {
            pr_info("Interrupt Source: Slave%d - ", dataBuffer[0] & A2B_BITM_INTSRC_INODE);
        } else {
            pr_warn("No recognized interrupt source: %d - ", dataBuffer[0]);
        }

        for (uint32_t i = 0; i < ARRAY_SIZE(intTypeString); i++) {
            if (intTypeString[i].type == dataBuffer[1]) {
                pr_cont("Interrupt Type: %s\n", intTypeString[i].message);

                if (a2b24xx->fault_check_running && rediscovry) {
                    mutex_lock(&a2b24xx->node_mutex);
                    processFaultNode(a2b24xx, (dataBuffer[0] & A2B_BITM_INTSRC_INODE));
                    mutex_unlock(&a2b24xx->node_mutex); // Release lock
                }
                return (dataBuffer[0] & A2B_BITM_INTSRC_INODE);
            }
        }
        pr_cont("Interrupt Type: Ignorable interrupt (Code: %d)\n", dataBuffer[1]);
    } else if (rediscovry) {
        mutex_lock(&a2b24xx->node_mutex);
        for (uint8_t i = 0; i < a2b24xx->max_node_number; i++) {
            adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, i});
            adi_a2b_I2CRead(a2b24xx->dev, A2B_SLAVE_ADDR, 1, (uint8_t[]){A2B_REG_NODE}, 1, dataBuffer);
            if ((dataBuffer[0] & A2B_BITM_NODE_LAST) && ((i + 1) != a2b24xx->max_node_number)) {
                processFaultNode(a2b24xx, i + 1);
                break;
            }
        }
        mutex_unlock(&a2b24xx->node_mutex); // Release lock
    }
    return -1;
}

/****************************************************************************/
/*!
 @brief          This function does A2B network discovery
                 and the peripheral configuration
 @return         None
 */
/********************************************************************************/
static void adi_a2b_NetworkSetup(struct device* dev)
{
    struct a2b24xx *a2b24xx = dev_get_drvdata(dev);

    ADI_A2B_DISCOVERY_CONFIG* pOPUnit;
    unsigned int nIndex, nIndex1;
    unsigned char *aDataBuffer = kmalloc(6000, GFP_KERNEL); // Allocate 6000 bytes of memory for the data buffer
    unsigned char aDataWriteReadBuf[4u];
    unsigned int nDelayVal;

    /* Loop over all the configuration */
    for (nIndex = 0; nIndex < a2b24xx->actionCount; nIndex++) {
        pOPUnit = &a2b24xx->pA2BConfig[nIndex];
        /* Operation code */
        switch (pOPUnit->eOpCode) {
            /* Write */
            case A2B24XX_WRITE:
                adi_a2b_Concat_Addr_Data(&aDataBuffer[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
                (void)memcpy(&aDataBuffer[pOPUnit->nAddrWidth], pOPUnit->paConfigData, pOPUnit->nDataCount);
                adi_a2b_I2CWrite(dev, pOPUnit->nDeviceAddr, (pOPUnit->nAddrWidth + pOPUnit->nDataCount), aDataBuffer);
                break;

            /* Read */
            case A2B24XX_READ:
                (void)memset(&aDataBuffer[0u], 0u, pOPUnit->nDataCount);
                adi_a2b_Concat_Addr_Data(&aDataWriteReadBuf[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
                //if (pOPUnit->nAddr == A2B_REG_INTTYPE) {
                //    processInterrupt(a2b24xx, false);
                //    continue;
                //}
                adi_a2b_I2CRead(dev, pOPUnit->nDeviceAddr, pOPUnit->nAddrWidth, aDataWriteReadBuf, pOPUnit->nDataCount, aDataBuffer);
                mdelay(2); // Couple of milliseconds should be OK
                break;

            /* Delay */
            case A2B24XX_DELAY:
                nDelayVal = 0u;
                for (nIndex1 = 0u; nIndex1 < pOPUnit->nDataCount; nIndex1++) {
                    nDelayVal = pOPUnit->paConfigData[nIndex1] | nDelayVal << 8u;
                }
                mdelay(nDelayVal);
                break;

            default:
                break;
        }
    }
    kfree(aDataBuffer);
}

#ifndef A2B_SETUP_ALSA
// Function to handle device open
static int a2b24xx_ctrl_open(struct inode *inode, struct file *filp)
{
    struct a2b24xx *a2b24xx = container_of(inode->i_cdev, struct a2b24xx, cdev);
    filp->private_data = a2b24xx;

    pr_info("A2B24xx device opened\n");
    return 0;
}

// Function to handle write operations
static ssize_t a2b24xx_ctrl_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct a2b24xx *a2b24xx = file->private_data;
    size_t len = count < COMMAND_SIZE - 1 ? count : COMMAND_SIZE - 1;
    int slave_id, mic_id = -1;

    if (copy_from_user(a2b24xx->command_buffer, buf, len)) {
        pr_err("Failed to receive command from user\n");
        return -EFAULT;
    }

    a2b24xx->command_buffer[len] = '\0'; // Null-terminate the string
    pr_info("Received data: %s\n", a2b24xx->command_buffer);

    if (strncmp(a2b24xx->command_buffer, "RESET", 5) == 0) {
        a2b24xx_reset(a2b24xx); // Perform reset operation
        return len;
    }

    if (strncmp(a2b24xx->command_buffer, "FAULT CHECK", 11) == 0) {
        if (a2b24xx->fault_check_running) {
            a2b24xx->fault_check_running = false;
            cancel_delayed_work_sync(&a2b24xx->fault_check_work); // Cancel fault check
        } else {
            schedule_delayed_work(&a2b24xx->fault_check_work,
                        msecs_to_jiffies(A2B24XX_FAULT_CHECK_INTERVAL));
        }
        return len;
    }

    if (sscanf(a2b24xx->command_buffer, "SLAVE%d MIC%d", &slave_id, &mic_id) >= 1) {
        pr_err("Received data: Slave(%d), MIC(%d)\n", slave_id, mic_id);
        mutex_lock(&a2b24xx->node_mutex);
        for (uint8_t i = 0; i < a2b24xx->max_node_number; i++) {
            adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, i});
            adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL2, 0x00});
            if (i == slave_id) {
                switch(mic_id) {
                    case 0:
                        adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x13});
                        break;
                    case 1:
                        adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x1C});
                        adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL2, 0x08});
                        break;
                    default:
                        adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x15});
                        break;
                }
            } else {
                adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x00});
            }
        }
        mutex_unlock(&a2b24xx->node_mutex); // Release lock
    }
    return len;
}

// File operations structure
static const struct file_operations a2b24xx_ctrl_fops = {
    .owner = THIS_MODULE,
    .open = a2b24xx_ctrl_open,
    .write = a2b24xx_ctrl_write,
};
#endif

static void a2b24xx_setup_work(struct work_struct *work)
{
    struct a2b24xx *a2b24xx = container_of(work, struct a2b24xx, setup_work);
    uint8_t node_number = 0;

    /* Setting up A2B network */
    adi_a2b_NetworkSetup(a2b24xx->dev);

    for (int32_t i = (a2b24xx->actionCount - 1); i > 0; i--) {
        if (a2b24xx->pA2BConfig[i].nAddr == A2B_REG_SLOTFMT) {
            a2b24xx->master_fmt = a2b24xx->pA2BConfig[i].paConfigData[0];
            break;
        }
        if (a2b24xx->pA2BConfig[i].nAddr == A2B_REG_NODEADR) {
            a2b24xx->max_node_number = a2b24xx->pA2BConfig[i].paConfigData[0] + 1;
        }
    }
    for (uint32_t i = 0; i < a2b24xx->actionCount; i++) {
        if (a2b24xx->pA2BConfig[i].nAddr == A2B_REG_DISCVRY) {
            a2b24xx->cycles[node_number++] = a2b24xx->pA2BConfig[i].paConfigData[0];
        }
        if (node_number == a2b24xx->max_node_number && a2b24xx->pA2BConfig[i].nAddr == A2B_REG_LDNSLOTS) {
            for (int32_t j = i; j > 0; j--) {
                if (a2b24xx->pA2BConfig[j].nAddr == A2B_REG_NODEADR) {
                    a2b24xx->slave_pos[a2b24xx->pA2BConfig[j].paConfigData[0]] = i;
                    break;
                }
            }
        }
    }
    schedule_delayed_work(&a2b24xx->fault_check_work, msecs_to_jiffies(A2B24XX_FAULT_CHECK_INTERVAL));
}

static void a2b24xx_fault_check_work(struct work_struct *work)
{
    struct a2b24xx *a2b24xx = container_of(work, struct a2b24xx, fault_check_work.work);
    a2b24xx->fault_check_running = true;

    processInterrupt(a2b24xx, true);

    /* Schedule the next fault check at the specified interval */
    schedule_delayed_work(&a2b24xx->fault_check_work,
                msecs_to_jiffies(A2B24XX_FAULT_CHECK_INTERVAL));
}

/* Template functions */
static int a2b24xx_hw_params(struct snd_pcm_substream *substream,
                              struct snd_pcm_hw_params *params,
                              struct snd_soc_dai *dai)
{
	//struct snd_soc_component *codec = dai->component;
	//struct a2b24xx *a2b24xx = snd_soc_component_get_drvdata(codec);
	//unsigned int rate = params_rate(params);
    int ret = 0;

    // Add custom functionality

    return ret;
}

static int a2b24xx_set_tdm_slot(struct snd_soc_dai *dai,
                                 unsigned int tx_mask, unsigned int rx_mask,
                                 int slots, int width)
{
    // Add custom functionality

    return 0;
}

static int a2b24xx_mute(struct snd_soc_dai *dai, int mute, int stream)
{
    // struct a2b24xx *a2b24xx = snd_soc_component_get_drvdata(dai->component);

    // Add custom functionality

    return 0;
}

static int a2b24xx_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    int ret = 0;

    // Add custom functionality

    return ret;
}

static int a2b24xx_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    // struct a2b24xx *a2b24xx = snd_soc_component_get_drvdata(dai->component);

    // Add custom functionality

    return 0;
}

static const struct snd_soc_dai_ops a2b24xx_dai_ops = {
    .startup = a2b24xx_startup,
    .hw_params = a2b24xx_hw_params,
    .mute_stream = a2b24xx_mute,
    .set_fmt = a2b24xx_set_dai_fmt,
    .set_tdm_slot = a2b24xx_set_tdm_slot,
};

static struct snd_soc_dai_driver a2b24xx_dai = {
    .name = "a2b24xx-hifi",
    .capture = {
        .stream_name = "Capture",
        .channels_min = 1,
        .channels_max = 32,
        .rates = SNDRV_PCM_RATE_KNOT,
        .formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
        .sig_bits = 24,
    },
    .playback = {
        .stream_name = "Playback",
        .channels_min = 1,
        .channels_max = 32,
        .rates = SNDRV_PCM_RATE_KNOT,
        .formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
        .sig_bits = 24,
    },
    .ops = &a2b24xx_dai_ops,
};

/* Supported rates */
static const unsigned int a2b24xx_rates[] = {
    1500, 2000, 2400, 3000, 8000, 12000, 24000, 48000,
};

/* Set system clock */
static int a2b24xx_set_sysclk(struct snd_soc_component *codec,
                              int clk_id, int source, unsigned int freq, int dir)
{
    // struct a2b24xx *a2b24xx = snd_soc_component_get_drvdata(codec);
    // unsigned int mask = 0;
    // unsigned int clk_src;
    // unsigned int ret = 0;

    // No functionality

    return 0;
}

/* Codec probe */
static int a2b24xx_codec_probe(struct snd_soc_component *codec)
{
    struct a2b24xx *a2b24xx = snd_soc_component_get_drvdata(codec);
    int ret = 0;

    dev_info(a2b24xx->dev, "Probed\n");

#ifdef A2B_SETUP_ALSA
    schedule_work(&a2b24xx->setup_work);
#endif

    return ret;
}

static struct snd_soc_component_driver a2b24xx_codec_driver = {
    .probe = a2b24xx_codec_probe,
    .set_sysclk = a2b24xx_set_sysclk,
    .controls = a2b24xx_snd_controls,
    .num_controls = ARRAY_SIZE(a2b24xx_snd_controls),
};

/* Driver probe */
int a2b24xx_probe(struct device *dev, struct regmap *regmap,
                  enum a2b24xx_type type, void (*switch_mode)(struct device *dev))
{
    struct a2b24xx *a2b24xx;
    int ret;
    size_t size;

    if (IS_ERR(regmap))
        return PTR_ERR(regmap);

    a2b24xx = devm_kzalloc(dev, sizeof(*a2b24xx), GFP_KERNEL);
    if (a2b24xx == NULL)
        return -ENOMEM;

    a2b24xx->dev = dev;
    a2b24xx->type = type;
    a2b24xx->regmap = regmap;
    a2b24xx->switch_mode = switch_mode;
    a2b24xx->max_master_fs = 48000;

    a2b24xx->constraints.list = a2b24xx_rates;
    a2b24xx->constraints.count = ARRAY_SIZE(a2b24xx_rates);

    dev_set_drvdata(dev, a2b24xx);

#ifndef A2B_SETUP_ALSA
    // Allocate a device number dynamically
    ret = alloc_chrdev_region(&a2b24xx->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device number\n");
        return ret;
    }

    // Initialize the cdev structure
    cdev_init(&a2b24xx->cdev, &a2b24xx_ctrl_fops);
    ret = cdev_add(&a2b24xx->cdev, a2b24xx->dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(a2b24xx->dev_num, 1);
        pr_err("Failed to add cdev\n");
        return ret;
    }

    // Create the device class
    a2b24xx->dev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(a2b24xx->dev_class)) {
        cdev_del(&a2b24xx->cdev);
        unregister_chrdev_region(a2b24xx->dev_num, 1);
        pr_err("Failed to create device class\n");
        return PTR_ERR(a2b24xx->dev_class);
    }

    // Create the device node
    device_create(a2b24xx->dev_class, NULL, a2b24xx->dev_num, NULL, DEVICE_NAME);
    pr_info("Major number: %d, Minor number: %d\n", MAJOR(a2b24xx->dev_num), MINOR(a2b24xx->dev_num));
#endif

    char *content = a2b_pal_File_Read("/lib/firmware/adi_a2b_commandlist.xml", &size);
    if (content) {
        pr_info("File content (%zu bytes)\n", size);

		// Parse XML configuration
        parseXML(a2b24xx, content);
        a2b24xx->pA2BConfig = a2b24xx->parseA2BConfig;
        kfree(content);
    } else {
        a2b24xx->pA2BConfig = gaA2BConfig;
        a2b24xx->actionCount = CONFIG_LEN;
    }

    pr_info("Action count: %zu, Buffer used: %zu\n", a2b24xx->actionCount, a2b24xx->bufferOffset);

#if 0
    // Print the results
    const ADI_A2B_DISCOVERY_CONFIG* pA2BConfig = a2b24xx->pA2BConfig;
    for (int i = 0; i < a2b24xx->actionCount; i++) {
        switch (pA2BConfig[i].eOpCode) {
            case A2B24XX_WRITE:
                pr_info("Action %03d: nDeviceAddr=0x%02X, eOpCode=write, nAddrWidth=%d, nAddr=%05d 0x%04X, nDataCount=%hu, eProtocol=%s, paConfigData=\n",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth,
                       pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount, pA2BConfig[i].eProtocol == SPI ? "SPI" : "I2C");
                break;
            case A2B24XX_READ:
                pr_info("Action %03d: nDeviceAddr=0x%02X, eOpCode= read, nAddrWidth=%d, nAddr=%05d 0x%04X, nDataCount=%hu, eProtocol=%s\n",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth,
                       pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount, pA2BConfig[i].eProtocol == SPI ? "SPI" : "I2C");
                continue;
            case A2B24XX_DELAY:
                pr_info("Action %03d: delay, nDataCount=%hu, sleep=\n", i, pA2BConfig[i].nDataCount);
                break;
        }

        for (int j = 0; j < pA2BConfig[i].nDataCount; j++) {
            pr_info("0x%02X\n", pA2BConfig[i].paConfigData[j]);
        }
    }
#endif

    a2b24xx->fault_check_running = false;
    mutex_init(&a2b24xx->node_mutex); // Initialize the mutex

    INIT_WORK(&a2b24xx->setup_work, a2b24xx_setup_work);
    INIT_DELAYED_WORK(&a2b24xx->fault_check_work, a2b24xx_fault_check_work);
#ifndef A2B_SETUP_ALSA
    schedule_work(&a2b24xx->setup_work);
#endif

    return snd_soc_register_component(dev, &a2b24xx_codec_driver, &a2b24xx_dai, 1);
}
EXPORT_SYMBOL_GPL(a2b24xx_probe);

int a2b24xx_remove(struct device *dev)
{
    struct a2b24xx *a2b24xx = dev_get_drvdata(dev);

#ifndef A2B_SETUP_ALSA
    device_destroy(a2b24xx->dev_class, a2b24xx->dev_num);  // Destroy the device node
    class_destroy(a2b24xx->dev_class);  // Destroy the device class
    cdev_del(&a2b24xx->cdev);  // Delete the cdev
    unregister_chrdev_region(a2b24xx->dev_num, 1);  // Free the device number
#endif

    cancel_work_sync(&a2b24xx->setup_work);
    cancel_delayed_work_sync(&a2b24xx->fault_check_work);

    pr_info("A2B24xx driver exited\n");
    return 0;
}
EXPORT_SYMBOL_GPL(a2b24xx_remove);

static bool a2b24xx_register_volatile(struct device *dev, unsigned int reg)
{
    return true;
}

const struct regmap_config a2b24xx_regmap_config = {
    .max_register = 255,
    .volatile_reg = a2b24xx_register_volatile,
    .cache_type = REGCACHE_NONE,
    .reg_defaults = a2b24xx_reg_defaults,
    .num_reg_defaults = ARRAY_SIZE(a2b24xx_reg_defaults),
};
EXPORT_SYMBOL_GPL(a2b24xx_regmap_config);

MODULE_DESCRIPTION("ASoC A2B24XX driver");
MODULE_AUTHOR("ADI Automotive Software Team, Bangalore");
MODULE_LICENSE("GPL");
