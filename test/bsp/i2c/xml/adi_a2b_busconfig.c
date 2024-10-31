#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "adi_a2b_commandlist.h"
#include "a2b-pal-interface.h"

#define MAX_ACTIONS 100
#define MAX_CONFIG_DATA 256

ADI_A2B_DISCOVERY_CONFIG configs[MAX_ACTIONS];
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
        char* dataStart = strstr(action, ">") + 1; // Find position after '>'
        char* dataEnd = strstr(dataStart, "</action>");
        if (dataEnd) {
            char dataStr[10];
            size_t length = dataEnd - dataStart;
            strncpy(dataStr, dataStart, length);
            dataStr[length] = '\0'; // Null-terminate

            config->paConfigData = &(configBuffer[bufferOffset++]);
            config->paConfigData[0] = (unsigned char)atoi(dataStr);
            config->eOpCode = DELAY;
            config->nDataCount = 1;
        }
    } else {
        config->eOpCode = INVALID;
        return;
    }

    if (strcmp(instr, "writeXbytes") == 0) {
        char* dataStart = strstr(action, ">") + 1; // Find position after '>'
        char* dataEnd = strchr(dataStart, '\n'); // Use '\n' as end marker
        if (dataEnd) {
            char dataStr[50];
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
            if (actionLength >= sizeof(action)) break;

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
    printf("Sleep %dms\n", time);
    usleep(time * 1000);
}

int32_t setupNetwork() {
    ADI_A2B_DISCOVERY_CONFIG* pOpUnit;
    uint32_t index, innerIndex;
    int32_t status = 0, handle;

    static uint8_t dataBuffer[128];
    static uint8_t dataWriteReadBuffer[4u];
    uint32_t delayValue;

    for (index = 0; index < actionCount; index++) {
        pOpUnit = &configs[index];
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
    if (!content) return 0;

    printf("File content (%zu bytes):\n%s\n", size, content);
    parseXML(content, configs, &actionCount);
    free(content);
    printf("Action count=%d\n", actionCount);

#if 0
    // Print the results
    for (int i = 0; i < actionCount; i++) {
        switch (configs[i].eOpCode) {
            case WRITE:
                printf("Action %02d: nDeviceAddr=%#x, eOpCode=write, nAddrWidth=%d, nAddr=%03d 0x%02x, nDataCount=%hu, eProtocol=%d, paConfigData=",
                       i, configs[i].nDeviceAddr, configs[i].nAddrWidth, configs[i].nAddr, configs[i].nAddr, configs[i].nDataCount, configs[i].eProtocol);
                break;
            case READ:
                printf("Action %02d: nDeviceAddr=%#x, eOpCode=read, nAddrWidth=%d, nAddr=%03d 0x%02x, nDataCount=%hu, eProtocol=%d\n",
                       i, configs[i].nDeviceAddr, configs[i].nAddrWidth, configs[i].nAddr, configs[i].nAddr, configs[i].nDataCount, configs[i].eProtocol);
                continue;
            case DELAY:
                printf("Action %02d: delay, nDataCount=%hu, sleep=", i, configs[i].nDataCount);
                break;
        }

        for (int j = 0; j < configs[i].nDataCount; j++) {
            printf(configs[i].eOpCode != DELAY ? "0x%02x " : "%02dms ", configs[i].paConfigData[j]);
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
