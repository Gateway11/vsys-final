#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "adi_a2b_commandlist.h"
#include "a2b-pal-interface.h"
#include "regdefs.h"

#define MAX_ACTIONS 256
#define MAX_CONFIG_DATA MAX_ACTIONS << 6
#define CHECK_RANGE(val, lo, hi) (((val) >= (lo)) && ((val) <= (hi)))

struct a2b_bus;
struct a2b_node {
    struct a2b_bus *sub_bus;
    uint16_t position;
    uint8_t cycle;
};

struct a2b_bus {
    ADI_A2B_DISCOVERY_CONFIG *pA2BConfig;
    ADI_A2B_DISCOVERY_CONFIG fileA2BConfig[MAX_ACTIONS];
    size_t num_actions;

    struct a2b_node *nodes;
    uint8_t num_nodes;
    uint8_t id;
    uint8_t master_fmt;
};

struct a2b_bus bus;
char sub_bus_files[16][128];
uint8_t bus_parents[16];
uint8_t num_files;

uint8_t last_bus_id;
uint8_t last_addr;

uint8_t config_buffer[MAX_CONFIG_DATA];
size_t write_offset = 0;

int32_t deviceHandle;

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
    } else if (sscanf(action, "<include file=%s parent=\"%hhu\"",
                sub_bus_files[num_files], &bus_parents[num_files]) == 2) {
        num_files++;
        return;
    } else {
        config->eOpCode = INVALID;
        return;
    }

    if (config->eOpCode == WRITE || config->eOpCode == DELAY) {
        if (write_offset + config->nDataCount >= MAX_CONFIG_DATA) {
            printf("Warning: Exceeding maximum configuration data limit!\n");
            return;
        }
        // Parse multiple numbers
        char* token = strtok(strchr(action, '>') + 1 /* Find position after '>' */, " ");
        size_t index = 0;
        config->paConfigData = config_buffer + write_offset;
        while (token != NULL && config->nDataCount) {
            config->paConfigData[index++] = (uint8_t)strtoul(token, NULL, 16); // Convert to hexadecimal
            token = strtok(NULL, " ");
        }
        write_offset += index;
        config->nDataCount = index;
    }
}

void parseXML(struct a2b_bus *bus, const char* xml) {
    const char* actionStart = strstr(xml, "<action");

    while (actionStart && bus->num_actions < MAX_ACTIONS) {
        const char* actionEnd = strchr(actionStart, '\n'); // Use '\n' as end marker
        size_t actionLength = actionEnd - actionStart + 1;

        char action[actionLength + 1];
        strncpy(action, actionStart, actionLength);
        action[actionLength] = '\0'; // Null-terminate

        parseAction(action, &bus->fileA2BConfig[bus->num_actions], 104);
        if ((actionStart = strstr(actionEnd, "<action"))) {
            bus->num_actions++;
        } else {
            actionStart = strstr(actionEnd, "<include");
        }
    }
    bus->num_actions++;
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

/* Example handler implementations */
void logPowerError(uint8_t intType, void* context) {
    // Cast context to specific type if needed
    printf("Power error %d occurred: %s\n", intType, (const char*)context);
}

void defaultHandler(uint8_t intType, void* context) {
    printf("Default handler: interrupt %d\n", intType);
}

/* Interrupt type descriptor structure */
typedef struct {
    uint8_t type;              // Interrupt type identifier
    void (*handle)(uint8_t intType, void* context);  // Generic handler callback
    const char* description;       // Description information
} IntTypeInfo_t;

const IntTypeInfo_t intTypeInfo[] = {
    {A2B_ENUM_INTTYPE_HDCNTERR           , logPowerError,   "HDCNTERR "},
    {A2B_ENUM_INTTYPE_DDERR              , defaultHandler,  "DDERR "},
    {A2B_ENUM_INTTYPE_CRCERR             , NULL,            "CRCERR "},
    {A2B_ENUM_INTTYPE_DPERR              , NULL,            "DPERR "},
    {A2B_ENUM_INTTYPE_BECOVF             , NULL,            "BECOVF "},
    {A2B_ENUM_INTTYPE_SRFERR             , NULL,            "SRFERR "},
    {A2B_ENUM_INTTYPE_PWRERR_CS_GND      , NULL,            "PWRERR (Cable Shorted to GND) "},
    {A2B_ENUM_INTTYPE_PWRERR_CS_VBAT     , NULL,            "PWRERR (Cable Shorted to VBat) "},
    {A2B_ENUM_INTTYPE_PWRERR_CS          , NULL,            "PWRERR (Cable Shorted Together) "},
    {A2B_ENUM_INTTYPE_PWRERR_CDISC       , NULL,            "PWRERR (Cable Disconnected or Open Circuit) (AD240x/10/2x Slaves Only) "},
    {A2B_ENUM_INTTYPE_PWRERR_CREV        , NULL,            "PWRERR (Cable Reverse Connected) (AD240x/10/2x Slaves Only) "},
    {A2B_ENUM_INTTYPE_PWRERR_CDISC_REV   , NULL,            "PWRERR - Cable is Disconnected (Open Circuit) or Wrong Port or Reverse Connected (AD243x Only) "},
    {A2B_ENUM_INTTYPE_PWRERR_FAULT       , NULL,            "PWRERR (Indeterminate Fault) "},
    //{A2B_ENUM_INTTYPE_IO0PND             , NULL,            "IO0PND - Slave Only "},
    //{A2B_ENUM_INTTYPE_IO1PND             , NULL,            "IO1PND - Slave Only "},
    //{A2B_ENUM_INTTYPE_IO2PND             , NULL,            "IO2PND - Slave Only "},
    {A2B_ENUM_INTTYPE_IO3PND             , NULL,            "IO3PND "},
    {A2B_ENUM_INTTYPE_IO4PND             , NULL,            "IO4PND "},
    {A2B_ENUM_INTTYPE_IO5PND             , NULL,            "IO5PND "},
    {A2B_ENUM_INTTYPE_IO6PND             , NULL,            "IO6PND "},
    //{A2B_ENUM_INTTYPE_IO7PND             , NULL,            "IO7PND "},
    //{A2B_ENUM_INTTYPE_DSCDONE            , NULL,            "DSCDONE - Master Only "},
    {A2B_ENUM_INTTYPE_I2CERR             , defaultHandler,  "I2CERR - Master Only "},
    {A2B_ENUM_INTTYPE_ICRCERR            , NULL,            "ICRCERR - Master Only "},
    {A2B_ENUM_INTTYPE_PWRERR_NLS_GND     , NULL,            "PWRERR - Non-Localized Short to GND "},
    {A2B_ENUM_INTTYPE_PWRERR_NLS_VBAT    , NULL,            "PWRERR - Non-Localized Short to VBat "},
    {A2B_ENUM_INTTYPE_PWRERR_OTH         , NULL,            "PWRERR - Other Error, Check SWSTAT2/SWSTAT3."},
    //{A2B_ENUM_INTTYPE_SPIDONE            , NULL,            "SPI Done"},
    {A2B_ENUM_INTTYPE_SPI_REMOTE_REG_ERR , NULL,            "SPI Remote Register Access Error - Master Only"},
    {A2B_ENUM_INTTYPE_SPI_REMOTE_I2C_ERR , NULL,            "SPI Remote I2C Access Error - Master Only"},
    {A2B_ENUM_INTTYPE_SPI_DATA_TUN_ERR   , NULL,            "SPI Data Tunnel Access Error"},
    {A2B_ENUM_INTTYPE_SPI_BAD_CMD        , NULL,            "SPI Bad Command"},
    {A2B_ENUM_INTTYPE_SPI_FIFO_OVRFLW    , NULL,            "SPI FIFO Overflow"},
    {A2B_ENUM_INTTYPE_SPI_FIFO_UNDERFLW  , NULL,            "SPI FIFO Underflow"},
    {A2B_ENUM_INTTYPE_VMTR               , NULL,            "VMTR Interrupt"},
    {A2B_ENUM_INTTYPE_IRPT_MSG_ERR       , NULL,            "PWRERR - Interrupt Messaging Error "},
    {A2B_ENUM_INTTYPE_STRTUP_ERR_RTF     , NULL,            "Startup Error - Return to Factory "},
    {A2B_ENUM_INTTYPE_SLAVE_INTTYPE_ERR  , NULL,            "Slave INTTYPE Read Error - Master Only "},
    //{A2B_ENUM_INTTYPE_STANDBY_DONE       , NULL,            "Standby Done - Master Only "},
    //{A2B_ENUM_INTTYPE_MSTR_RUNNING       , NULL,            "MSTR_RUNNING - Master Only "},
};

#define BUS_SELECT(__bus, __parent, __addr)                                                                    \
({                                                                                                             \
    uint8_t __ret = (__addr);                                                                                  \
    if (__bus) {                                                                                               \
        if (last_bus_id != __bus || last_addr != __addr) {                                                     \
            adi_a2b_I2C_Write(&deviceHandle, A2B_BASE_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, __parent});        \
            adi_a2b_I2C_Write(&deviceHandle, A2B_BUS_ADDR, 2, (uint8_t[]){A2B_REG_CHIP, __addr});              \
            adi_a2b_I2C_Write(&deviceHandle, A2B_BASE_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, __parent | 0x20}); \
         }                                                                                                     \
        last_addr = __addr;                                                                                    \
        __ret = A2B_BUS_ADDR;                                                                                  \
    } else if (last_bus_id != __bus) {                                                                         \
        adi_a2b_I2C_Write(&deviceHandle, A2B_BASE_ADDR, 2, (uint8_t[]){A2B_REG_NODEADR, __parent});            \
    }                                                                                                          \
    last_bus_id = __bus;                                                                                       \
    __ret;                                                                                                     \
})

void processInterrupt(struct a2b_bus *bus, uint8_t parent, uint8_t addr) {
    uint8_t dataBuffer[2] = {0}; //A2B_REG_INTSRC, A2B_REG_INTTYPE

    adi_a2b_I2C_WriteRead(&deviceHandle, BUS_SELECT(bus->id, parent, addr), 1, (uint8_t[]){A2B_REG_INTSRC}, 1, dataBuffer);
    if (dataBuffer[0]) {
        adi_a2b_I2C_WriteRead(&deviceHandle, A2B_BASE_ADDR, 1, (uint8_t[]){A2B_REG_INTTYPE}, 1, dataBuffer + 1);
        if (dataBuffer[0] & A2B_BITM_INTSRC_MSTINT) {
            printf("Interrupt Source: Master - ");
        } else if (dataBuffer[0] & A2B_BITM_INTSRC_SLVINT) {
            printf("Interrupt Source: Slave%d - ", dataBuffer[0] & A2B_BITM_INTSRC_INODE);
        } else {
            printf("No recognized interrupt source: %d - ", dataBuffer[0]);
        }
        for (uint32_t i = 0; i < ARRAY_SIZE(intTypeInfo); i++) {
            if (intTypeInfo[i].type == dataBuffer[1]) {
                printf("Interrupt Type: %s\n", intTypeInfo[i].description);
                if (intTypeInfo[i].handle)
                    intTypeInfo[i].handle(dataBuffer[1], NULL);
                return;
            }
        }
        printf("Interrupt Type: Ignorable interrupt (Code: %d)\n", dataBuffer[1]);
    }
}

void setupNetwork(struct a2b_bus *bus, uint8_t parent) {
    ADI_A2B_DISCOVERY_CONFIG* pOpUnit;
    uint32_t index, innerIndex;

    static uint8_t dataBuffer[5192];
    static uint8_t dataWriteReadBuffer[4u];
    uint32_t delayValue;

    for (index = 0; index < bus->num_actions; index++) {
        pOpUnit = &bus->pA2BConfig[index];
        /* Operation code */
        switch (pOpUnit->eOpCode) {
            case WRITE:
                concatAddrData(&dataBuffer[0u], pOpUnit->nAddrWidth, pOpUnit->nAddr);
                (void)memcpy(&dataBuffer[pOpUnit->nAddrWidth], pOpUnit->paConfigData, pOpUnit->nDataCount);
                adi_a2b_I2C_Write(&deviceHandle, BUS_SELECT(bus->id, parent, (uint16_t)pOpUnit->nDeviceAddr),
                        (uint16_t)(pOpUnit->nAddrWidth + pOpUnit->nDataCount), &dataBuffer[0u]);
                break;

            case READ:
                (void)memset(&dataBuffer[0u], 0u, pOpUnit->nDataCount);
                concatAddrData(&dataWriteReadBuffer[0u], pOpUnit->nAddrWidth, pOpUnit->nAddr);
                if (pOpUnit->nAddr == A2B_REG_INTTYPE) {
                    processInterrupt(bus, parent, pOpUnit->nDeviceAddr);
                    continue;
                }
                adi_a2b_I2C_WriteRead(&deviceHandle, BUS_SELECT(bus->id, parent, (uint16_t)pOpUnit->nDeviceAddr),
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
    }
}

bool loadConfig(struct a2b_bus *bus, const char *filename) {
    size_t size;

    char *content = a2b_pal_File_Read(filename, &size);
    if (!content) return false;

    printf("File content (%zu bytes)\n", size);

    // Parse XML configuration
    parseXML(bus, content);
    bus->pA2BConfig = bus->fileA2BConfig;
    free(content);

    printf("File=%s, action count=%zu, buffer used=%zu\n", filename, bus->num_actions, write_offset);

#if 0
    // Print the results
    const ADI_A2B_DISCOVERY_CONFIG* pA2BConfig = bus->pA2BConfig;
    for (int i = 0; i < bus->num_actions; i++) {
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
    return true;
}

void setup(struct a2b_bus *bus, uint8_t parent) {
    uint8_t node_id = 0;

    /* Configure A2B system */
    setupNetwork(bus, parent);

    for (int32_t i = (bus->num_actions - 1); i >= 0; i--) {
        if (bus->pA2BConfig[i].nAddr == A2B_REG_SLOTFMT) {
            bus->master_fmt = bus->pA2BConfig[i].paConfigData[0];
            break;
        }
        if (bus->pA2BConfig[i].nAddr == A2B_REG_NODEADR) {
            bus->num_nodes = bus->pA2BConfig[i].paConfigData[0];
        }
    }

    bus->nodes = calloc(bus->num_nodes + 1, sizeof(struct a2b_node));
    for (uint32_t i = 0; i < bus->num_actions; i++) {
        if (bus->pA2BConfig[i].nAddr == A2B_REG_DISCVRY && bus->pA2BConfig[i].nDeviceAddr == A2B_BASE_ADDR) {
            bus->nodes[node_id++].cycle = bus->pA2BConfig[i].paConfigData[0];
        }
        if (bus->pA2BConfig[i].nAddr == A2B_REG_LDNSLOTS && bus->pA2BConfig[i].nAddrWidth == 1) {
            for (int32_t j = i; j >= 0; j--) {
                if (bus->pA2BConfig[j].nAddr == A2B_REG_NODEADR && bus->pA2BConfig[j + 1].nAddr != A2B_REG_CHIP &&
                    !(bus->pA2BConfig[j].paConfigData[0] & A2B_BITM_NODEADR_PERI)) {
                    bus->nodes[bus->pA2BConfig[j].paConfigData[0]].position = i;
                    break;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    const char* default_filename = "adi_a2b_commandlist.xml";

    if (!loadConfig(&bus, argc == 2 ? argv[1] : default_filename)) {
        bus.pA2BConfig = gaA2BConfig;
        bus.num_actions = CONFIG_LEN;
    }

    /* PAL call, open I2C driver */
    deviceHandle = adi_a2b_I2C_Open(A2B_BASE_ADDR);
    
    setup(&bus, 0);
    for (uint8_t i = 0; i < num_files; i++) {
        if (CHECK_RANGE(bus_parents[i], 0, bus.num_nodes)) {
            bus.nodes[bus_parents[i]].sub_bus = calloc(1, sizeof(struct a2b_bus));
            bus.nodes[bus_parents[i]].sub_bus->id = i + 1;

            if (loadConfig(bus.nodes[bus_parents[i]].sub_bus, sub_bus_files[i])) {
                setup(bus.nodes[bus_parents[i]].sub_bus, bus_parents[i]);
            } else {
                free(bus.nodes[bus_parents[i]].sub_bus);
                bus.nodes[bus_parents[i]].sub_bus = NULL;
            }
        }
    }

    adi_a2b_I2C_Close(deviceHandle);

    return 0;
}
