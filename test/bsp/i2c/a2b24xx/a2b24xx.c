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

#define A2B_SETUP_ALSA

#ifdef A2B_SETUP_ALSA
#include "adi_a2b_commandlist.h"
#endif
#include "a2b24xx.h"

#define DEVICE_NAME "a2b_ctl"   // Device name
#define CLASS_NAME "a2b24xx"    // Device class name

struct a2b24xx {
    struct regmap *regmap;
    unsigned int sysclk;
    enum a2b24xx_sysclk_src sysclk_src;
    enum a2b24xx_type type;

    struct snd_pcm_hw_constraint_list constraints;

    struct device *dev;
    void (*switch_mode)(struct device *dev);

    unsigned int max_master_fs;
    bool master;

    dev_t dev_num;              // Device number
    struct cdev cdev;           // cdev structure
    struct class *dev_class;    // Device class

#define BUFFER_SIZE 128         // Buffer size for receiving commands
    char command_buffer[BUFFER_SIZE];

#define MAX_ACTIONS 256
#define MAX_CONFIG_DATA (MAX_ACTIONS << 6)
    ADI_A2B_DISCOVERY_CONFIG *pA2BConfig;
    ADI_A2B_DISCOVERY_CONFIG parseA2BConfig[MAX_ACTIONS];
    size_t actionCount;
};

static const struct reg_default a2b24xx_reg_defaults[] = {
    { 0x00, 0x50 }
};

/* Example control - no specific functionality */
static const DECLARE_TLV_DB_MINMAX_MUTE(a2b24xx_control, 0, 0);

#define A2B24XX_CONTROL(x) \
    SOC_SINGLE_TLV("A2B" #x "Template", 2, 0, 255, 1, a2b24xx_control)

/* Example control */
static const struct snd_kcontrol_new a2b24xx_snd_controls[] = { A2B24XX_CONTROL(1) };

//static int a2b24xx_reset(struct a2b24xx *a2b24xx)
//{
//   int ret = 0;
//
//   regcache_cache_bypass(a2b24xx->regmap, true);
//   /* A2B reset */
//
//   return ret;
//}

static uint8_t configBuffer[MAX_CONFIG_DATA];
size_t bufferOffset = 0;

static void parseAction(const char* action, ADI_A2B_DISCOVERY_CONFIG* config, uint8_t deviceAddr) {
    char instr[20], protocol[10];

    const char *pos;
    char *endptr;
    int parseCount = 0;  // Initialize the counter for parsed fields
    char buffer[10];  // Temporary buffer to hold the extracted number string

    config->nDeviceAddr = deviceAddr;
    config->nDataCount = 0;

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

    // Parse "addrWidth" field
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

    // Parse "i2cAddr" field
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
        if (bufferOffset + config->nDataCount > MAX_CONFIG_DATA) {
            pr_warn("Warning: Exceeding maximum configuration data limit!\n");
            return;
        }
        // Parse multiple numbers
        char *dataStr = strchr(action, '>') + 1; /* Find position after '>' */
        char *token = strsep(&dataStr, " ");
        size_t index = 0;
        config->paConfigData = configBuffer + bufferOffset;
        while (token != NULL && config->nDataCount) {
            config->paConfigData[index++] = (uint8_t)strtoul(token, NULL, 16); // Convert to hexadecimal
            token = strsep(&dataStr, " ");
        }
        bufferOffset += index;
        config->nDataCount = index;
    }
}

static void parseXML(const char* xml, ADI_A2B_DISCOVERY_CONFIG* configs, size_t* actionCount) {
    const char* actionStart = strstr(xml, "<action");
    char *action = kmalloc(6000, GFP_KERNEL); // Allocate 6000 bytes of memory for the action buffer
    *actionCount = 0;

    while (actionStart && *actionCount < MAX_ACTIONS) {
        const char* actionEnd = strchr(actionStart, '\n'); // Use '\n' as end marker
        size_t actionLength = actionEnd - actionStart + 1;

        if (actionLength > 6000) {
            pr_warn("Warning: Action length exceeds buffer size!\n");
            return;
        }
        strncpy(action, actionStart, actionLength);
        action[actionLength] = '\0'; // Null-terminate

        parseAction(action, &configs[*actionCount], 104);
        (*actionCount)++;
        actionStart = strstr(actionEnd, "<action");
    }
    kfree(action);
}

#ifdef A2B_SETUP_ALSA
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

    pr_info("%s:i2c read device(0x%X) reg 0x%02X, cnt %d, val:\n", __func__, devAddr, writeBuffer[0], readLength);
    for (i = 0; i < readLength; i++) {
        pr_info("0x%02X\n", readBuffer[i]);
    }

    return 0;
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
#endif

// Function to handle device open
static int a2b24xx_ctl_open(struct inode *inode, struct file *filp)
{
    struct a2b24xx *a2b24xx = container_of(inode->i_cdev, struct a2b24xx, cdev);
    filp->private_data = a2b24xx;

    pr_info("a2b_ctl device opened\n");
    return 0;
}

// Function to handle write operations
static ssize_t a2b24xx_ctl_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct a2b24xx *a2b24xx = file->private_data;
    size_t len = count < BUFFER_SIZE - 1 ? count : BUFFER_SIZE - 1;

    if (copy_from_user(a2b24xx->command_buffer, buf, len)) {
        pr_err("Failed to receive command from user\n");
        return -EFAULT;
    }

    a2b24xx->command_buffer[len] = '\0'; // Null-terminate the string
    pr_info("Received data: %s\n", a2b24xx->command_buffer);

    if (strncmp(a2b24xx->command_buffer, "reinit", 6) == 0) {
        /* Setting up A2B network */
        adi_a2b_NetworkSetup(a2b24xx->dev);
    }

    return len;
}

// File operations structure
static const struct file_operations a2b24xx_ctl_fops = {
    .owner = THIS_MODULE,
    .open = a2b24xx_ctl_open,
    .write = a2b24xx_ctl_write,
};

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
    int ret = 0;

#ifdef A2B_SETUP_ALSA
    // Setting up A2B network
    // adi_a2b_NetworkSetup(codec->dev);
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

    // Allocate a device number dynamically
    ret = alloc_chrdev_region(&a2b24xx->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device number\n");
        return ret;
    }

    // Initialize the cdev structure
    cdev_init(&a2b24xx->cdev, &a2b24xx_ctl_fops);
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

    char *content = a2b_pal_File_Read("/home/nvidia/adi_a2b_commandlist.xml", &size);
    if (content) {
        pr_info("File content (%zu bytes)\n", size);

		// Parse XML configuration
        parseXML(content, a2b24xx->parseA2BConfig, &a2b24xx->actionCount);
        a2b24xx->pA2BConfig = a2b24xx->parseA2BConfig;
        kfree(content);
    } else {
        a2b24xx->pA2BConfig = gaA2BConfig;
        a2b24xx->actionCount = CONFIG_LEN;
    }

    pr_info("Action count: %zu, Buffer used: %zu\n", a2b24xx->actionCount, bufferOffset);

#if 1
    // Print the results
    for (int i = 0; i < a2b24xx->actionCount; i++) {
        ADI_A2B_DISCOVERY_CONFIG* pA2BConfig = a2b24xx->pA2BConfig;
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

#ifdef A2B_SETUP_ALSA
    /* Setting up A2B network */
    adi_a2b_NetworkSetup(dev);
#endif

    return snd_soc_register_component(dev, &a2b24xx_codec_driver, &a2b24xx_dai, 1);
}
EXPORT_SYMBOL_GPL(a2b24xx_probe);

int a2b24xx_remove(struct device *dev)
{
    struct a2b24xx *a2b24xx = dev_get_drvdata(dev);

    device_destroy(a2b24xx->dev_class, a2b24xx->dev_num);  // Destroy the device node
    class_destroy(a2b24xx->dev_class);  // Destroy the device class
    cdev_del(&a2b24xx->cdev);  // Delete the cdev
    unregister_chrdev_region(a2b24xx->dev_num, 1);  // Free the device number

    pr_info("a2b24xx driver exited\n");

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
