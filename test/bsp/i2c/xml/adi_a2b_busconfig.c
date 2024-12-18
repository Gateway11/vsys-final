#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "adi_a2b_commandlist.h"
#include "a2b-pal-interface.h"
#include "regdefs.h"

#define MAX_ACTIONS 256
#define MAX_CONFIG_DATA (MAX_ACTIONS << 6)

ADI_A2B_DISCOVERY_CONFIG *pA2BConfig, parseA2BConfig[MAX_ACTIONS];
static size_t actionCount = 0;
static int32_t deviceHandles[3];

static uint8_t configBuffer[MAX_CONFIG_DATA];
size_t bufferOffset = 0;

void parseAction(const char* action, ADI_A2B_DISCOVERY_CONFIG* config, uint8_t deviceAddr) {
    char instr[20], protocol[10];

    config->nDeviceAddr = deviceAddr;
    config->nDataCount = 0;

    if (sscanf(action, "<action instr=\"%[^\"]\" SpiCmd=\"%u\" SpiCmdWidth=\"%hhu\" addr_width\
                =\"%hhu\" data_width=\"%hhu\" len=\"%hu\" addr=\"%u\" i2caddr=\"%hhu\" AddrIncr=\"%*[^\"\n]\" Protocol=\"%[^\"]\"",
               instr, &config->nSpiCmd, &config->nSpiCmdWidth, 
               &config->nAddrWidth, &config->nDataWidth, &config->nDataCount, &config->nAddr, &config->nDeviceAddr, protocol) >= 9 ||
        sscanf(action, "<action instr=\"%[^\"]\" addr_width=\"%hhu\" data_width=\"%hhu\" len=\"%hu\" addr=\"%u\" i2caddr=\"%hhu\"",
               instr, &config->nAddrWidth, &config->nDataWidth, &config->nDataCount, &config->nAddr, &config->nDeviceAddr) >= 6) {
        if (strcmp(instr, "writeXbytes") == 0) {
            config->eOpCode = WRITE;
        } else if (strcmp(instr, "read") == 0) {
            config->eOpCode = READ;
        } else {
            config->eOpCode = INVALID;
        }
        config->eProtocol = (strcmp(protocol, "SPI") == 0) ? SPI : I2C;
        config->nDataCount -= config->nAddrWidth;
    } else if (strstr(action, "instr=\"delay\"") != NULL) {
        config->eOpCode = DELAY;
        config->nDataCount = 1;
    } else {
        config->eOpCode = INVALID;
        return;
    }

    if (config->eOpCode == WRITE || config->eOpCode == DELAY) {
        if (bufferOffset + config->nDataCount > MAX_CONFIG_DATA) {
            printf("Warning: %s Exceeding maximum configuration data limit!\n", __func__);
            exit(EXIT_FAILURE);
        }
        // Parse multiple numbers
        char* token = strtok(strchr(action, '>') + 1 /* Find position after '>' */, " ");
        size_t index = 0;
        config->paConfigData = configBuffer + bufferOffset;
        while (token != NULL && config->nDataCount) {
            config->paConfigData[index++] = (uint8_t)strtoul(token, NULL, 16); // Convert to hexadecimal
            token = strtok(NULL, " ");
        }
        bufferOffset += index;
        config->nDataCount = index;
    }
}

void parseXML(const char* xml, ADI_A2B_DISCOVERY_CONFIG* configs, size_t* actionCount) {
    const char* actionStart = strstr(xml, "<action");
    *actionCount = 0;

    while (actionStart && (*actionCount)++ < MAX_ACTIONS) {
        const char* actionEnd = strchr(actionStart, '\n'); // Use '\n' as end marker
        size_t actionLength = actionEnd - actionStart + 1;

        char action[actionLength + 1];
        strncpy(action, actionStart, actionLength);
        action[actionLength] = '\0'; // Null-terminate

        parseAction(action, &configs[*actionCount], 104);
        actionStart = strstr(actionEnd, "<action");
    }
}

void concatAddrData(uint8_t destBuffer[], uint32_t addrWidth, uint32_t addr) {
    /* Store the read values in the placeholder */
    switch (addrWidth) {
        case 1u:
            destBuffer[0u] = (uint8_t)addr;
            break;
        case 2u:
            destBuffer[0u] = (uint8_t)(addr >> 8u);
            destBuffer[1u] = (uint8_t)(addr & 0xFFu);
            break;
        case 3u:
            destBuffer[0u] = (uint8_t)((addr & 0xFF0000u) >> 16u);
            destBuffer[1u] = (uint8_t)((addr & 0xFF00u) >> 8u);
            destBuffer[2u] = (uint8_t)(addr & 0xFFu);
            break;
        case 4u:
            destBuffer[0u] = (uint8_t)(addr >> 24u);
            destBuffer[1u] = (uint8_t)((addr & 0xFF0000u) >> 16u);
            destBuffer[2u] = (uint8_t)((addr & 0xFF00u) >> 8u);
            destBuffer[3u] = (uint8_t)(addr & 0xFFu);
            break;
        default:
            break;
    }
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

void processInterrupt() {
    uint8_t dataBuffer[2] = {0}; //A2B_REG_INTSRC, A2B_REG_INTTYPE

    adi_a2b_I2C_WriteRead(deviceHandles, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_INTSRC}, 1, dataBuffer);
    if (dataBuffer[0]) {
        adi_a2b_I2C_WriteRead(deviceHandles, A2B_MASTER_ADDR, 1, (uint8_t[]){A2B_REG_INTTYPE}, 1, dataBuffer + 1);
        if (dataBuffer[0] & A2B_BITM_INTSRC_MSTINT) {
            printf("Interrupt Source: Master - ");
        } else if (dataBuffer[0] & A2B_BITM_INTSRC_SLVINT) {
            printf("Interrupt Source: Slave%d - ", dataBuffer[0] & A2B_BITM_INTSRC_INODE);
        } else {
            printf("No recognized interrupt source (Code: 0x%02X)\n", dataBuffer[0]);
            return;
        }
        for (uint32_t i = 0; i < ARRAY_SIZE(intTypeString); i++) {
            if (intTypeString[i].type == dataBuffer[1]) {
                printf("Interrupt Type: %s\n", intTypeString[i].message);
                exit(EXIT_FAILURE);
            }
        }
        printf("Interrupt Type: Ignorable interrupt (Code: %d)\n", dataBuffer[1]);
    }
}

int32_t setupNetwork() {
    ADI_A2B_DISCOVERY_CONFIG* pOpUnit;
    uint32_t index, innerIndex;
    int32_t status = 0, handle;

    static uint8_t dataBuffer[5192];
    static uint8_t dataWriteReadBuffer[4u];
    uint32_t delayValue;

    for (index = 0; index < actionCount; index++) {
        pOpUnit = &pA2BConfig[index];
        handle = pOpUnit->nDeviceAddr == A2B_MASTER_ADDR ? deviceHandles[0] :
            (pOpUnit->nDeviceAddr == A2B_SLAVE_ADDR ? deviceHandles[1] : deviceHandles[2]);
        /* Operation code */
        switch (pOpUnit->eOpCode) {
            case WRITE:
                if (pOpUnit->eProtocol == SPI) break;
                concatAddrData(&dataBuffer[0u], pOpUnit->nAddrWidth, pOpUnit->nAddr);
                (void)memcpy(&dataBuffer[pOpUnit->nAddrWidth], pOpUnit->paConfigData, pOpUnit->nDataCount);
                status = adi_a2b_I2C_Write(&handle, (uint16_t)pOpUnit->nDeviceAddr,
                        (uint16_t)(pOpUnit->nAddrWidth + pOpUnit->nDataCount), &dataBuffer[0u]);
                break;

            case READ:
                if (pOpUnit->eProtocol == SPI) break;
                (void)memset(&dataBuffer[0u], 0u, pOpUnit->nDataCount);
                concatAddrData(&dataWriteReadBuffer[0u], pOpUnit->nAddrWidth, pOpUnit->nAddr);
                if (pOpUnit->nAddr == A2B_REG_INTTYPE) {
                    processInterrupt();
                    continue;
                }
                status = adi_a2b_I2C_WriteRead(&handle, (uint16_t)pOpUnit->nDeviceAddr,
                        (uint16_t)pOpUnit->nAddrWidth, &dataWriteReadBuffer[0u], (uint16_t)pOpUnit->nDataCount, &dataBuffer[0u]);
                break;

            case DELAY:
                delayValue = 0u;
                for (innerIndex = 0u; innerIndex < pOpUnit->nDataCount; innerIndex++) {
                    delayValue = pOpUnit->paConfigData[innerIndex] | delayValue << 8u;
                }
                adi_a2b_Delay(delayValue);
                break;

            default:
                break;
        }

        if (status != 0) {
            break; // I2C read/write failed! No point in continuing!
        }
    }

    return status;
}

#if 1
int main(int argc, char* argv[]) {
    const char* filename = "adi_a2b_commandlist.xml";
    size_t size;

    if (argc == 2) filename = argv[1];

    char* content = a2b_pal_File_Read(filename, &size);
    if (content) {
        printf("File content (%zu bytes):\n%s\n", size, content);
        parseXML(content, parseA2BConfig, &actionCount);
        pA2BConfig = parseA2BConfig;
        free(content);
    } else {
        pA2BConfig = gaA2BConfig;
        actionCount = CONFIG_LEN;
    }
    printf("Action count=%zu, bufferOffset=%zu\n", actionCount, bufferOffset);

#if 0
    // Print the results
    for (int i = 0; i < actionCount; i++) {
        switch (pA2BConfig[i].eOpCode) {
            case WRITE:
                printf("Action %03d: nDeviceAddr=0x%02X, eOpCode=write, nAddrWidth=%d, nAddr=%05d 0x%04X, nDataCount=%hu, eProtocol=%s, paConfigData=",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth,
                       pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount, pA2BConfig[i].eProtocol == SPI ? "SPI" : "I2C");
                break;
            case READ:
                printf("Action %03d: nDeviceAddr=0x%02X, eOpCode= read, nAddrWidth=%d, nAddr=%05d 0x%04X, nDataCount=%hu, eProtocol=%s\n",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth,
                       pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount, pA2BConfig[i].eProtocol == SPI ? "SPI" : "I2C");
                continue;
            case DELAY:
                printf("Action %03d: delay, nDataCount=%hu, sleep=", i, pA2BConfig[i].nDataCount);
                break;
        }

        for (int j = 0; j < pA2BConfig[i].nDataCount; j++) {
            printf(pA2BConfig[i].eOpCode != DELAY ? "0x%02X " : "0x%02Xms ", pA2BConfig[i].paConfigData[j]);
        }
        printf("\n");
    }
#endif

    /* PAL call, open I2C driver */
    deviceHandles[0] = adi_a2b_I2C_Open(A2B_MASTER_ADDR);
    deviceHandles[1] = adi_a2b_I2C_Open(A2B_SLAVE_ADDR);
#if DSP_DEVICE_ADDR
    deviceHandles[2] = adi_a2b_I2C_Open(DSP_DEVICE_ADDR);
#endif
    
    /* Configure A2B system */
    setupNetwork();

    adi_a2b_I2C_Close(deviceHandles[0]);
    adi_a2b_I2C_Close(deviceHandles[1]);
#if DSP_DEVICE_ADDR
    adi_a2b_I2C_Close(deviceHandles[2]);
#endif

    return 0;
}

#else
/* Define how often to check (and clear) the fault status register (in ms) */
#define A2B_BUS_CONFIG_FAULT_CHECK_INTERVAL 2000

struct ad2433_data {
    struct device *dev;
    struct regmap *regmap;
    struct i2c_client *client;
    struct delayed_work fault_check_work;
};

static void ad2433_fault_check_work(struct work_struct *work)
{
    struct ad2433_data *ad2433 = container_of(work, struct ad2433_data,
                            fault_check_work.work);
    struct device *dev = ad2433->dev;

    processInterrupt();
    /* Schedule the next fault check at the specified interval */
    schedule_delayed_work(&ad2433->fault_check_work,
                  msecs_to_jiffies(A2B_BUS_CONFIG_FAULT_CHECK_INTERVAL));
}

static const struct regmap_config ad2433_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
};

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id ad2433_of_ids[] = {
    { .compatible = "lenovo,ad2433", },
    { },
};
MODULE_DEVICE_TABLE(of, ad2433_of_ids);
#endif

static int ad2433_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct device *dev = &client->dev;
    struct ad2433_data *ad2433;
    int ret;
    const char* filename = "adi_a2b_commandlist.xml";
    size_t size;


    ad2433 = devm_kzalloc(dev, sizeof(*ad2433), GFP_KERNEL);
    if (!ad2433)
        return -ENOMEM;
    dev_set_drvdata(dev, ad2433);

    ad2433->dev = dev;
    ad2433->client = client;

    ad2433->regmap = devm_regmap_init_i2c(client, &ad2433_regmap_config);
    if (IS_ERR(ad2433->regmap)) {
        ret = PTR_ERR(ad2433->regmap);
        dev_err(dev, "unable to allocate register map: %d\n", ret);
        return ret;
    }

    char* content = a2b_pal_File_Read(filename, &size);
    if (content) {
        dev_info(dev, "File content (%zu bytes):\n%s\n", size, content);
        parseXML(content, parseA2BConfig, &actionCount);
        pA2BConfig = parseA2BConfig;
        free(content);
    } else {
        pA2BConfig = gaA2BConfig;
        actionCount = CONFIG_LEN;
    }
    dev_info(dev, "Action count=%d\n", actionCount);

    /* Configure A2B system */
    setupNetwork();

    INIT_DELAYED_WORK(&ad2433->fault_check_work, ad2433_fault_check_work);
    schedule_delayed_work(&ad2433->fault_check_work, msecs_to_jiffies(A2B_BUS_CONFIG_FAULT_CHECK_INTERVAL));

    return 0;
}

static int ad2433_i2c_remove(struct i2c_client *client)
{
    struct device *dev = &client->dev;
    struct ad2433_data *ad2433 = dev_get_drvdata(dev);

    cancel_delayed_work_sync(&ad2433->fault_check_work);

    return 0;
}

static const struct i2c_device_id ad2433_i2c_ids[] = {
    { "ad2433", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ad2433_i2c_ids);

static struct i2c_driver ad2433_i2c_driver = {
    .driver = {
        .name = "ad2433",
        .of_match_table = of_match_ptr(ad2433_of_ids),
    },
    .probe = ad2433_i2c_probe,
    .remove = ad2433_i2c_remove,
    .id_table = ad2433_i2c_ids,
};
module_i2c_driver(ad2433_i2c_driver);

MODULE_DESCRIPTION("ASoC AD2433 driver");
MODULE_AUTHOR("Johnny Hsu <johnnyhsu@realtek.com>");
MODULE_LICENSE("GPL v2");
#endif
