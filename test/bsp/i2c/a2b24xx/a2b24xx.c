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
//#include <linux/of_irq.h>
//#include <linux/interrupt.h>

#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#include "adi_a2b_commandlist.h"
#include "a2b24xx.h"
#include "regdefs.h"

#ifdef CONFIG_TEGRA_EPL
#include <linux/tegra-epl.h>
#endif

//#define A2B_SETUP_ALSA

#define DEVICE_NAME "a2b_ctrl"  // Device name
#define CLASS_NAME "a2b24xx"    // Device class name
#define COMMAND_SIZE 128        // Buffer size for receiving commands

#define MAX_ACTIONS  256
#define MAX_CONFIG_DATA (MAX_ACTIONS << 6)

#define MAX_SRFMISS_FREQ 2      // Maximum allowed occurrences of SRFMISS
#define MAX_RETRIES 2           // Maximum number of retries
#define A2B24XX_FAULT_CHECK_INTERVAL 5000

/**
 * @brief EPL reporter ID for A2B24XX.
 * This ID is used when reporting errors to FSI via EPL.
 */
#define A2B24XX_EPL_REPORTER_ID 0x8103

/*
 * Recommendation for BECCTL, for normal operation
 *
 * Do not enable reporting of individual bit errors (CRCERR, DPERR, DDERR, HDCNTERR, ICRCERR).
 * - The A2B transceiver automatically takes action on these errors (like repeating last known sample,
 *   ignoring erroneous interrupt, and retrying).
 * - Even if individual bit error reporting is enabled, the Host DSP may not need to do any corrective action.
 *
 * Instead, count the bit errors in the Bit Error Counter (BECNT) and take the required action if excessive bit errors.
 * For example, in most systems, counting CRC errors in the BECNT register could be sufficient.
 * - Mask all Bit Failure Interrupts, except for Bit Error Count Overflow.
 *
 * -------------------------------------------------------------------------------------
 * |            | Master | Slave  | Comment                                            |
 * -------------------------------------------------------------------------------------
 * | INTMSKO    | 0x70   | 0x70   | Interrupt enabled for SRFERR, BECOVF, and PWRERR   |
 * -------------------------------------------------------------------------------------
 * | INTMSK2    | 0x0B   |  --    | Interrupt enabled for SLVIRQ, I2CERR, and DSCDONE  |
 * -------------------------------------------------------------------------------------
 * | BECCTL     | 0xE4   | 0xE4   | Configuration example: Threshold 256, ENCRC        |
 * -------------------------------------------------------------------------------------
 * Same settings should be used for all nodes
 */
//#define ENABLE_BECCTL_CONF

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

    uint8_t SRFMISS;
    uint8_t cycles[16];
    uint16_t slave_pos[16];
    uint8_t max_node_number;
    uint16_t error_code;

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
static int16_t processInterrupt(struct a2b24xx *a2b24xx, bool rediscover);

static const struct reg_default a2b24xx_reg_defaults[] = {
    { 0x00, 0x50 }
};

/* Example control - no specific functionality */
static const DECLARE_TLV_DB_MINMAX_MUTE(a2b24xx_control, 0, 0);

static int a2b24xx_reset(struct a2b24xx *a2b24xx)
{
    struct i2c_client *client = to_i2c_client(a2b24xx->dev);

    disable_irq(client->irq);
    cancel_delayed_work_sync(&a2b24xx->fault_check_work);
    regcache_cache_bypass(a2b24xx->regmap, true);

    /* A2B reset */
    adi_a2b_NetworkSetup(a2b24xx->dev);

    enable_irq(client->irq);
    schedule_delayed_work(&a2b24xx->fault_check_work,
                    msecs_to_jiffies(A2B24XX_FAULT_CHECK_INTERVAL));
    return 0;
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

static void a2b24xx_epl_report_error(uint32_t error_code)
{
#ifdef CONFIG_TEGRA_EPL
    struct epl_error_report_frame error_report;
    u64 time;

    asm volatile("mrs %0, cntvct_el0" : "=r" (time));

    error_report.error_code = error_code;
    error_report.error_attribute = 0x0;
    error_report.reporter_id = A2B24XX_EPL_REPORTER_ID;
    error_report.timestamp = (u32) time;

    epl_report_error(error_report);
#endif
    return;
}

static void parseAction(struct a2b24xx *a2b24xx, const char *action, ADI_A2B_DISCOVERY_CONFIG *config) {
    char instr[20], protocol[10];
    size_t *bufferOffset = &a2b24xx->bufferOffset;

    if (sscanf(action, "<action instr=\"%s SpiCmd=\"%u\" SpiCmdWidth=\"%hhu\" addr_width\
                 =\"%hhu\" data_width=\"%hhu\" len=\"%hu\" addr=\"%u\" i2caddr=\"%hhu\" AddrIncr=\"%*s\" Protocol=\"%s\"",
                instr, &config->nSpiCmd, &config->nSpiCmdWidth,
                &config->nAddrWidth, &config->nDataWidth, &config->nDataCount, &config->nAddr, &config->nDeviceAddr, protocol) == 9) {

#ifndef ENABLE_BECCTL_CONF
        if (config->nAddr == A2B_REG_BECCTL && config->nAddrWidth == 1) {
            config->eOpCode = A2B24XX_INVALID;
            return;
        }
#endif
        if (strstr(instr, "writeXbytes")) {
            config->eOpCode = A2B24XX_WRITE;
        } else if (strstr(instr, "read")) {
            config->eOpCode = A2B24XX_READ;
        } else {
            config->eOpCode = A2B24XX_INVALID;
        }
        config->eProtocol = strstr(protocol, "SPI") ? SPI : I2C;
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
        pr_warn("%s:i2c read device(0x%X) reg 0x%02X failed\n", __func__, devAddr, writeBuffer[0]);
        return ret;
    }

#if 0
    pr_info("%s:i2c read device(0x%X) reg 0x%02X, cnt %d, val:", __func__, devAddr, writeBuffer[0], readLength);
    for (uint32_t i = 0; i < readLength; i++) {
        pr_cont("0x%02X ", readBuffer[i]);
    }
    pr_cont("\n");
#endif

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
    uint8_t retryCount = 0;

    if (inode == 0 || inode >= a2b24xx->max_node_number) return false;

    pr_info("Processing node %d: master_fmt=0x%02X, cycle=0x%02X, slave_pos=%d 0x%02X\n",
            inode, a2b24xx->master_fmt, a2b24xx->cycles[inode],
            a2b24xx->slave_pos[inode], a2b24xx->pA2BConfig[a2b24xx->slave_pos[inode]].nAddr);

#ifdef ENABLE_BECCTL_CONF
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, 0x80});
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_BECNT, 0x00});
#endif

//https://ez.analog.com/a2b/f/q-a/536836/a2b-hotpluggable-or-how-to-resync-the-bus
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
//retry:
    if (processInterrupt(a2b24xx, false) != A2B_ENUM_INTTYPE_DSCDONE) {
        if (++retryCount < MAX_RETRIES) {
            //mdelay(25);
            //goto retry:
        }
        //adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_CONTROL, 0x82});
        adi_a2b_I2CWrite(dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x00});
        return false;
    }

//6. If rediscovery is successful (got the slave node discovery interrupt (INTTYPE==0x18),
//          O Update SWCTL=0x01 in all upsteam nodes
//          O After slave register programming, configure SLOTFMT, DATCTL and NEWSTRCT on master node to start the audio communication.
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x01});
    for (uint8_t i = 0; i < inode; i++) {
        adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, i});
        adi_a2b_I2CWrite(dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_SWCTL, 0x01});
    }

    ADI_A2B_DISCOVERY_CONFIG* pOPUnit;
    unsigned char *aDataBuffer = kmalloc(6000, GFP_KERNEL); // Allocate 6000 bytes of memory for the data buffer
    unsigned int nDelayVal;

    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, inode});
    for (uint32_t i = a2b24xx->slave_pos[inode]; i < a2b24xx->actionCount; i++) {
        pOPUnit = &a2b24xx->pA2BConfig[i];

        // Simple Advanced Optimized Modified
        if ((pOPUnit->nAddr == A2B_REG_NODEADR && pOPUnit->nDeviceAddr == A2B_MASTER_ADDR
                 && (pOPUnit->paConfigData[0] & A2B_BITM_NODEADR_NODE) != inode)
                || (pOPUnit->nAddr == A2B_REG_DISCVRY && pOPUnit->nDeviceAddr == A2B_MASTER_ADDR)
                || (pOPUnit->nAddr == A2B_REG_SWCTL && pOPUnit->paConfigData[0] & A2B_BITM_SWCTL_MODE))
            break;

        pr_info("iiooooooooiiiiiiiii %s, 0x%02X, %02d\n", __func__, pOPUnit->nAddr, pOPUnit->nAddr);

        switch (pOPUnit->eOpCode) {
            case A2B24XX_WRITE:
                adi_a2b_Concat_Addr_Data(&aDataBuffer[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
                (void)memcpy(&aDataBuffer[pOPUnit->nAddrWidth], pOPUnit->paConfigData, pOPUnit->nDataCount);
                adi_a2b_I2CWrite(dev, pOPUnit->nDeviceAddr, (pOPUnit->nAddrWidth + pOPUnit->nDataCount), aDataBuffer);
                break;
            case A2B24XX_DELAY:
                nDelayVal = 0u;
                for (uint8_t nIndex1 = 0u; nIndex1 < pOPUnit->nDataCount; nIndex1++) {
                    nDelayVal = pOPUnit->paConfigData[nIndex1] | nDelayVal << 8u;
                }
                mdelay(nDelayVal);
                break;
            default:
                break;
        }
    }
    adi_a2b_I2CWrite(dev, A2B_MASTER_ADDR, 4, (uint8_t[]){A2B_REG_SLOTFMT, a2b24xx->master_fmt, 0x03, 0x81});

    kfree(aDataBuffer); // Free memory before returning
    return true;
}

static void processFaultNode(struct a2b24xx *a2b24xx, int8_t inode) {
//    uint8_t dataBuffer[1] = {0}; //A2B_REG_NODE

    if (inode <= 0) {
        /* Setting up A2B network */
        adi_a2b_NetworkSetup(a2b24xx->dev);
    } else {
//        adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, inode - 1});
//        adi_a2b_I2CRead(a2b24xx->dev, A2B_SLAVE_ADDR, 1, (uint8_t[]){A2B_REG_NODE}, 1, dataBuffer);
//        if ((dataBuffer[0] & A2B_BITM_NODE_LAST) || a2b24xx->SRFMISS >= MAX_SRFMISS_FREQ) {
            for (uint8_t i = inode; i < a2b24xx->max_node_number; i++) {
                if (!processSingleNode(a2b24xx, i)) {
                    pr_warn("Node %d processing failed. Stopping further discovery\n", i);
                    break;
                }
                mdelay(10);
                uint8_t dataBuffer[2] = {0}; // A2B_REG_INTSRC, A2B_REG_INTTYPE
                adi_a2b_I2CRead(a2b24xx->dev, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_INTSRC}, 2, dataBuffer);
            }
        }
//    }
}

static void checkFaultNode(struct a2b24xx *a2b24xx, int8_t inode) {
    uint8_t dataBuffer[1] = {0}; // A2B_REG_NODE
    int8_t lastNode = A2B_MASTER_NODE;

    for (uint8_t i = 0; i < a2b24xx->max_node_number; i++) {
        adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, i});
        if (adi_a2b_I2CRead(a2b24xx->dev, A2B_SLAVE_ADDR, 1, (uint8_t[]){A2B_REG_NODE}, 1, dataBuffer) < 0) {
            // If discovery is not completed during system boot, the A2B_NODE.LAST bit for the last node will not be set
            lastNode = i - 1;
            break;
        }
        if (dataBuffer[0] & A2B_BITM_NODE_LAST) {
            lastNode = i; // Set lastNode when the A2B_NODE.LAST bit is found
            break;
        }
    }
    if (inode >= 0 && inode < lastNode /*&& a2b24xx->SRFMISS >= MAX_SRFMISS_FREQ*/) {
        pr_info("###### inode=%d, lastNode=%d, SRFMISS=%d\n", inode, lastNode, a2b24xx->SRFMISS);
        lastNode--;
    }
    if (lastNode < (a2b24xx->max_node_number - 1)) {
        pr_warn("Fault detected: Node %d is the last node\n", lastNode);
        processFaultNode(a2b24xx, lastNode + 1);
    }
}

static int16_t processInterrupt(struct a2b24xx *a2b24xx, bool deepCheck) {
    uint8_t dataBuffer[2] = {0}; // A2B_REG_INTSRC, A2B_REG_INTTYPE
    int8_t inode = A2B_MASTER_NODE;

    adi_a2b_I2CRead(a2b24xx->dev, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_INTSRC}, 2, dataBuffer);
    if (dataBuffer[0]) {
        //adi_a2b_I2CRead(a2b24xx->dev, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_INTTYPE}, 1, dataBuffer + 1);
        if (dataBuffer[0] & A2B_BITM_INTSRC_MSTINT) {
            pr_warn("Interrupt Source: Master - ");
        } else if (dataBuffer[0] & A2B_BITM_INTSRC_SLVINT) {
            inode = dataBuffer[0] & A2B_BITM_INTSRC_INODE;
            pr_warn("Interrupt Source: Slave%d - ", dataBuffer[0] & A2B_BITM_INTSRC_INODE);
        } else {
            pr_warn("No recognized interrupt source: %d - ", dataBuffer[0]);
        }

        for (uint32_t i = 0; i < ARRAY_SIZE(intTypeString); i++) {
            if (intTypeString[i].type == dataBuffer[1]) {
                pr_cont("Interrupt Type: %s\n", intTypeString[i].message);

                if (a2b24xx->error_code != *(uint16_t *)dataBuffer) {
                    a2b24xx_epl_report_error(a2b24xx->error_code = *(uint16_t *)dataBuffer);
                }

                a2b24xx->SRFMISS = dataBuffer[1] == A2B_ENUM_INTTYPE_SRFERR ? a2b24xx->SRFMISS + 1 : 0;
                if (deepCheck) {
                    checkFaultNode(a2b24xx, inode);
                }
                return dataBuffer[1];
            }
        }
        pr_cont("Interrupt Type: Ignorable interrupt (Code: %d)\n", dataBuffer[1]);
        return dataBuffer[1];
    } else if (deepCheck) {
        checkFaultNode(a2b24xx, A2B_INVALID_NODE);
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
    int32_t  errorCode = -1;

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
                if (pOpUnit->nAddr == A2B_REG_INTTYPE) {
                    if (errorCode < 0)
                        errorCode = processInterrupt(a2b24xx, false);
                    continue;
                }
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

    return 0;
}

// Function to handle write operations
static ssize_t a2b24xx_ctrl_write(struct file *file,
                        const char __user *buf, size_t count, loff_t *ppos)
{
    struct a2b24xx *a2b24xx = file->private_data;
    int32_t node_addr, mic = -1;
    uint8_t params[4] = {0};
    uint8_t config[] = {0x11, 0x91};

    size_t len = min(count, sizeof(a2b24xx->command_buffer ) - 1);
    if (copy_from_user(a2b24xx->command_buffer, buf, len)) {
        pr_err("Failed to receive command from user\n");
        return -EFAULT;
    }

    a2b24xx->command_buffer[len] = '\0'; // Null-terminate the string
    pr_info("Received data: %s\n", a2b24xx->command_buffer);

    if (strncmp(a2b24xx->command_buffer, "Reset", 5) == 0) {
        a2b24xx_reset(a2b24xx); // Perform reset operation
        return len;
    }

    if (strncmp(a2b24xx->command_buffer, "Fault Check", 11) == 0) {
        cancel_delayed_work_sync(&a2b24xx->fault_check_work); // Cancel fault check
        return len;
    }

    // https://ez.analog.com/a2b/f/q-a/541883/ad2428-loopback-test
    if (sscanf(a2b24xx->command_buffer, "Loopback Slave%d", &node_addr) == 1) {
        cancel_delayed_work_sync(&a2b24xx->fault_check_work); // Cancel fault check

        if (node_addr < a2b24xx->max_node_number) {
            if (node_addr == -1) {
                adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_I2STEST, 0x06});
            } else {
                adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, node_addr});
                adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_I2STEST, 0x06});
            }
        }
        return len;
    }

    if (sscanf(a2b24xx->command_buffer, "RX Slave%hhu %hhu", &params[0], &params[1]) == 2) {
        pr_info("RX Slave(%d) (%d)\n", params[0], params[1]);

        if (params[0] < a2b24xx->max_node_number && params[1] < sizeof(config)) {
            mutex_lock(&a2b24xx->node_mutex);
            adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, params[0]});
            adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_I2SCFG, config[params[1]]});
            adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x00});
            mutex_unlock(&a2b24xx->node_mutex); // Release lock
        }
        return len;
    }

    if (sscanf(a2b24xx->command_buffer, "PDM Slave%d MIC%d", &node_addr, &mic) >= 1) {
        pr_info("PDM Slave(%d) MIC(%d)\n", node_addr, mic);

        if (node_addr < a2b24xx->max_node_number) {
            mutex_lock(&a2b24xx->node_mutex);
            for (uint8_t i = 0; i < a2b24xx->max_node_number; i++) {
                adi_a2b_I2CWrite(a2b24xx->dev, A2B_MASTER_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, i});
                adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_I2SCFG, 0x01});
                if (node_addr < 0) {
                    adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x15});
                } else if (node_addr == i) {
                    switch(mic) {
                        case 0:
                            adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x11});
                            break;
                        case 1:
                            adi_a2b_I2CWrite(a2b24xx->dev, A2B_SLAVE_ADDR, 2, (uint8_t[]){A2B_REG_PDMCTL, 0x14});
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

    return len;
}

// File operations structure
static const struct file_operations a2b24xx_ctrl_fops = {
    .owner = THIS_MODULE,
    .open = a2b24xx_ctrl_open,
    .write = a2b24xx_ctrl_write,
};
#endif

static irqreturn_t a2b24xx_irq_handler(int irq, void *dev_id)
{
    struct a2b24xx *a2b24xx = dev_id;

    pr_info("%s: interrupt handled. %d", __func__, irq);
    if (mutex_trylock(&a2b24xx->node_mutex)) {
        processInterrupt(a2b24xx, false);
        mutex_unlock(&a2b24xx->node_mutex);
    }
    return IRQ_HANDLED;
}

static void a2b24xx_setup_work(struct work_struct *work)
{
    struct a2b24xx *a2b24xx = container_of(work, struct a2b24xx, setup_work);
    struct i2c_client *client = to_i2c_client(a2b24xx->dev);
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
        if (a2b24xx->pA2BConfig[i].nAddr == A2B_REG_DISCVRY && node_number < sizeof(a2b24xx->cycles)) {
            a2b24xx->cycles[node_number++] = a2b24xx->pA2BConfig[i].paConfigData[0];
        }
        if (a2b24xx->pA2BConfig[i].nAddr == A2B_REG_LDNSLOTS && a2b24xx->pA2BConfig[i].nAddrWidth == 1) {
            for (int32_t j = i; j > 0; j--) {
                if (a2b24xx->pA2BConfig[j].nAddr == A2B_REG_NODEADR
                        && a2b24xx->pA2BConfig[j + 1].nAddr != A2B_REG_CHIP
                        && !(a2b24xx->pA2BConfig[j].paConfigData[0] & A2B_BITM_NODEADR_PERI)) {
                    a2b24xx->slave_pos[a2b24xx->pA2BConfig[j].paConfigData[0]] = i;
                    break;
                }
            }
        }
    }

    int status = request_irq(client->irq, a2b24xx_irq_handler, IRQF_TRIGGER_FALLING, __func__, a2b24xx);
    if (status)
        pr_warn("Failed to request IRQ: %d, ret:%d\n", client->irq, status);

    schedule_delayed_work(&a2b24xx->fault_check_work, msecs_to_jiffies(A2B24XX_FAULT_CHECK_INTERVAL));
}

static void a2b24xx_fault_check_work(struct work_struct *work)
{
    struct a2b24xx *a2b24xx = container_of(work, struct a2b24xx, fault_check_work.work);

    mutex_lock(&a2b24xx->node_mutex);
    processInterrupt(a2b24xx, true);
    mutex_unlock(&a2b24xx->node_mutex); // Release lock

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
                pr_info("Action %02d: nDeviceAddr=0x%02X, eOpCode=write, nAddrWidth=%d, nAddr=%05d 0x%04X, nDataCount=%hu, eProtocol=%s, paConfigData=",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth,
                       pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount, pA2BConfig[i].eProtocol == SPI ? "SPI" : "I2C");
                break;
            case A2B24XX_READ:
                pr_info("Action %02d: nDeviceAddr=0x%02X, eOpCode= read, nAddrWidth=%d, nAddr=%05d 0x%04X, nDataCount=%hu, eProtocol=%s\n",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth,
                       pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount, pA2BConfig[i].eProtocol == SPI ? "SPI" : "I2C");
                continue;
            case A2B24XX_DELAY:
                pr_info("Action %02d: delay, nDataCount=%hu, sleep=", i, pA2BConfig[i].nDataCount);
                break;
        }

        for (int j = 0; j < pA2BConfig[i].nDataCount; j++) {
            pr_cont("0x%02X ", pA2BConfig[i].paConfigData[j]);
        }
        pr_cont("\n");
    }
#endif

    a2b24xx->SRFMISS = 0;
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
    struct i2c_client *client = to_i2c_client(a2b24xx->dev);

#ifndef A2B_SETUP_ALSA
    device_destroy(a2b24xx->dev_class, a2b24xx->dev_num);  // Destroy the device node
    class_destroy(a2b24xx->dev_class);  // Destroy the device class
    cdev_del(&a2b24xx->cdev);  // Delete the cdev
    unregister_chrdev_region(a2b24xx->dev_num, 1);  // Free the device number
#endif

    free_irq(client->irq, client);
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
