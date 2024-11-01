#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "adi_a2b_commandlist.h"
#include "a2b-pal-interface.h"
#include "regdefs.h"

#define MAX_ACTIONS 128
#define MAX_CONFIG_DATA (MAX_ACTIONS << 1)

ADI_A2B_DISCOVERY_CONFIG *pA2BConfig, parseA2BConfig[MAX_ACTIONS];
static int actionCount = 0;

unsigned char configBuffer[MAX_CONFIG_DATA];
static unsigned int bufferOffset = 0;

int32_t arrayHandles[2];

void parseAction(const char* action, ADI_A2B_DISCOVERY_CONFIG* config, unsigned char deviceAddr) {
    char instr[20], protocol[10];
    char addr[10], length[10], spiCmd[10], spiCmdWidth[10], addrWidth[10], dataWidth[10];

    config->nDeviceAddr = deviceAddr;
    config->nDataCount = 0;

    if (sscanf(action, "<action instr=\"%[^\"]\" SpiCmd=\"%[^\"]\" SpiCmdWidth=\"%[^\"]\" addr_width=\"%[^\"]\" data_width=\"%[^\"]\" len=\"%[^\"]\" addr=\"%[^\"]\" i2caddr=\"%hhu\" Protocol=\"%[^\"]\"",
               instr, spiCmd, spiCmdWidth, addrWidth, dataWidth, length, addr, &config->nDeviceAddr, protocol) >= 8) {
        
        if (strcmp(instr, "writeXbytes") == 0) {
            config->eOpCode = WRITE;
        } else if (strcmp(instr, "read") == 0) {
            config->eOpCode = READ;
        } else {
            config->eOpCode = INVALID;
        }

        config->eProtocol = (strcmp(protocol, "I2C") == 0) ? I2C : SPI;
        config->nAddrWidth = (unsigned char)atoi(addrWidth);
        config->nDataWidth = (unsigned char)atoi(dataWidth);
        config->nAddr = (unsigned int)atoi(addr);
        config->nDataCount = (unsigned short)atoi(length) - 1;
    } else if (strstr(action, "instr=\"delay\"") != NULL) {
#if 0
        char* dataStart = strstr(action, ">") + 1; // Find position after '>'
        char* dataEnd = strstr(dataStart, "</action>");
        if (dataEnd) {
            char dataStr[10];
            size_t length = dataEnd - dataStart;
            strncpy(dataStr, dataStart, length);
            dataStr[length] = '\0'; // Null-terminate

            config->paConfigData = &(configBuffer[bufferOffset++]);
            //config->paConfigData[0] = (unsigned char)atoi(dataStr);
            config->paConfigData[0] = (unsigned char)strtoul(dataStr, NULL, 16);
            config->eOpCode = DELAY;
            config->nDataCount = 1;
        }
#endif
        config->eOpCode = DELAY;
        config->nDataCount = 1;
    } else {
        config->eOpCode = INVALID;
        return;
    }

    if (strcmp(instr, "writeXbytes") == 0 || strcmp(instr, "delay") == 0) {
        if (bufferOffset + config->nDataCount > MAX_CONFIG_DATA) {
            printf("Warning: Exceeding maximum configuration data limit!\n");
            exit(EXIT_FAILURE);
        }
        char* dataStart = strstr(action, ">") + 1; // Find position after '>'
        char* dataEnd = strchr(dataStart, '\n'); // Use '\n' as end marker
        if (dataEnd) {
            //char dataStr[50];
            char dataStr[config->nDataCount + 1];
            size_t length = dataEnd - dataStart;
            strncpy(dataStr, dataStart, length);
            dataStr[length] = '\0'; // Null-terminate

            // Parse multiple numbers
            char* token = strtok(dataStr, " ");
            int index = 0;
            while (token != NULL && config->nDataCount) {
                config->paConfigData = configBuffer + bufferOffset;
                config->paConfigData[index++] = (unsigned char)strtoul(token, NULL, 16); // Convert to hexadecimal
                token = strtok(NULL, " ");
            }
            bufferOffset += index;
            config->nDataCount = index;
        }
    }
}

void parseXML(const char* xml, ADI_A2B_DISCOVERY_CONFIG* configs, int* actionCount) {
    const char* pageStart = strstr(xml, "<page");
    const char* actionStart;
    *actionCount = 0;

    if (pageStart) {
        actionStart = strstr(pageStart, "<action");
        while (actionStart && *actionCount < MAX_ACTIONS) {
            const char* actionEnd = strstr(actionStart, "\n");
            if (!actionEnd) break;

            char action[256];
            size_t actionLength = actionEnd - actionStart + 1;
            if (actionLength >= sizeof(action)) {
                printf("Warning: Action length exceeds buffer size!\n");
                break;
            }

            strncpy(action, actionStart, actionLength);
            action[actionLength] = '\0'; // Null-terminate

            parseAction(action, &configs[*actionCount], 104);
            (*actionCount)++;
            actionStart = strstr(actionEnd, "<action");
        }
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

void delay(uint32_t time) {
    printf("Sleep 0x%02xms\n", time);
    usleep(time * 1000);
}

int32_t setupNetwork() {
    ADI_A2B_DISCOVERY_CONFIG* pOpUnit;
    uint32_t index, innerIndex;
    int32_t status = 0, handle;

    static uint8_t dataBuffer[4096];
    static uint8_t dataWriteReadBuffer[4u];
    uint32_t delayValue;

    for (index = 0; index < actionCount; index++) {
        pOpUnit = &pA2BConfig[index];
        handle = pOpUnit->nDeviceAddr == A2B_MASTER_ADDR ? arrayHandles[0] : arrayHandles[1];
        /* Operation code */
        switch (pOpUnit->eOpCode) {
            case WRITE:
                concatAddrData(&dataBuffer[0u], pOpUnit->nAddrWidth, pOpUnit->nAddr);
                (void)memcpy(&dataBuffer[pOpUnit->nAddrWidth], pOpUnit->paConfigData, pOpUnit->nDataCount);
                status = adi_a2b_I2C_Write(&handle, (uint16_t)pOpUnit->nDeviceAddr,
                        (uint16_t)(pOpUnit->nAddrWidth + pOpUnit->nDataCount), &dataBuffer[0u]);
                break;

            case READ:
                (void)memset(&dataBuffer[0u], 0u, pOpUnit->nDataCount);
                concatAddrData(&dataWriteReadBuffer[0u], pOpUnit->nAddrWidth, pOpUnit->nAddr);
                status = adi_a2b_I2C_WriteRead(&handle, (uint16_t)pOpUnit->nDeviceAddr,
                        (uint16_t)pOpUnit->nAddrWidth, &dataWriteReadBuffer[0u], (uint16_t)pOpUnit->nDataCount, &dataBuffer[0u]);
                break;

            case DELAY:
                delayValue = 0u;
                for (innerIndex = 0u; innerIndex < pOpUnit->nDataCount; innerIndex++) {
                    delayValue = pOpUnit->paConfigData[innerIndex] | delayValue << 8u;
                }
                delay(delayValue);
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
    printf("Action count=%d\n", actionCount);

#if 0
    // Print the results
    for (int i = 0; i < actionCount; i++) {
        switch (pA2BConfig[i].eOpCode) {
            case WRITE:
                printf("Action %02d: nDeviceAddr=0x%02x, eOpCode=write, nAddrWidth=%d, nAddr=%05d 0x%04x, nDataCount=%hu, paConfigData=",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth, pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount);
                break;
            case READ:
                printf("Action %02d: nDeviceAddr=0x%02x, eOpCode= read, nAddrWidth=%d, nAddr=%05d 0x%04x, nDataCount=%hu\n",
                       i, pA2BConfig[i].nDeviceAddr, pA2BConfig[i].nAddrWidth, pA2BConfig[i].nAddr, pA2BConfig[i].nAddr, pA2BConfig[i].nDataCount);
                continue;
            case DELAY:
                printf("Action %02d: delay, nDataCount=%hu, sleep=", i, pA2BConfig[i].nDataCount);
                break;
        }

        for (int j = 0; j < pA2BConfig[i].nDataCount; j++) {
            printf(pA2BConfig[i].eOpCode != DELAY ? "0x%02x " : "%02dms ", pA2BConfig[i].paConfigData[j]);
        }
        printf("\n");
    }
#endif

    /* PAL call, open I2C driver */
    arrayHandles[0] = adi_a2b_I2C_Open(A2B_MASTER_ADDR);
    arrayHandles[1] = adi_a2b_I2C_Open(A2B_SLAVE_ADDR);
    
    /* Configure A2B system */
    setupNetwork();

    adi_a2b_I2C_Close(arrayHandles[0]);
    adi_a2b_I2C_Close(arrayHandles[1]);

    return 0;
}
