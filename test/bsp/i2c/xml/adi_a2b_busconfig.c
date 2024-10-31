#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adi_a2b_commandlist.h"

#define MAX_ACTIONS 100
#define MAX_CONFIG_DATA 256

unsigned char gBuffer[MAX_CONFIG_DATA];
static unsigned int gBufferOffset = 0;


void parseAction(const char* action, ADI_A2B_DISCOVERY_CONFIG* config, unsigned char deviceAddr) {
    char instr[20], protocol[10];
    char addr[10], len[10], spiCmd[10], spiCmdWidth[10], addrWidth[10], dataWidth[10];

    config->nDeviceAddr = deviceAddr;
    config->nDataCount = 0;

    if (sscanf(action, "<action instr=\"%[^\"]\" SpiCmd=\"%[^\"]\" SpiCmdWidth=\"%[^\"]\" addr_width=\"%[^\"]\" data_width=\"%[^\"]\" len=\"%[^\"]\" addr=\"%[^\"]\" i2caddr=\"%hhu\" Protocol=\"%[^\"]\"",
               instr, spiCmd, spiCmdWidth, addrWidth, dataWidth, len, addr, &config->nDeviceAddr, protocol) >= 8) {
        
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
        config->nDataCount = (unsigned short)atoi(len) - 1;
    } else if (strstr(action, "instr=\"delay\"") != NULL) {
        char* dataStart = strstr(action, ">") + 1; // 找到 '>' 后的位置
        char* dataEnd = strstr(dataStart, "</action>");
        if (dataEnd) {
            char dataStr[10];
            size_t length = dataEnd - dataStart;
            strncpy(dataStr, dataStart, length);
            dataStr[length] = '\0'; // Null-terminate

            config->paConfigData = &(gBuffer[gBufferOffset++]);
            config->paConfigData[0] = (unsigned char)atoi(dataStr);
            config->eOpCode = DELAY;
            config->nDataCount = 1;
        }
    } else {
        config->eOpCode = INVALID;
        return;
    }

    if (strcmp(instr, "writeXbytes") == 0) {
        char* dataStart = strstr(action, ">") + 1; // 找到 '>' 后的位置
        char* dataEnd = strchr(dataStart, '\n'); // 使用 '\n' 作为结束标志
        if (dataEnd) {
            char dataStr[50];
            size_t length = dataEnd - dataStart;
            strncpy(dataStr, dataStart, length);
            dataStr[length] = '\0'; // Null-terminate

            // 解析多个数字
            char* token = strtok(dataStr, " ");
            int index = 0;
            while (token != NULL && config->nDataCount) {
                config->paConfigData = gBuffer + gBufferOffset;
                config->paConfigData[index++] = (unsigned char)strtoul(token, NULL, 16); // 转换为十六进制
                token = strtok(NULL, " ");
            }
            gBufferOffset += index;
            config->nDataCount = index; // 更新数据计数
        }
    }
}

void parseXML(const char* xml, ADI_A2B_DISCOVERY_CONFIG* configs, int* count) {
    const char* pageStart = strstr(xml, "<page");
    const char* actionStart;
    *count = 0;

    if (pageStart) {
        actionStart = strstr(pageStart, "<action");
        while (actionStart && *count < MAX_ACTIONS) {
            const char* actionEnd = strstr(actionStart, "\n");
            if (!actionEnd) break;

            char action[256];
            size_t actionLength = actionEnd - actionStart + 1;
            if (actionLength >= sizeof(action)) break; // 确保不溢出

            strncpy(action, actionStart, actionLength);
            action[actionLength] = '\0'; // Null-terminate

            parseAction(action, &configs[*count], 104); // 假定 i2caddr 是 104
            (*count)++;
            actionStart = strstr(actionEnd, "<action");
        }
    }
}

#include <stdlib.h>
char* read_file(const char* filename, size_t* out_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(filesize + 1); // +1 for null terminator
    if (!buffer) {
        fclose(file);
        perror("Failed to allocate memory");
        return NULL;
    }

    size_t read_size = fread(buffer, 1, filesize, file);
    if (read_size != filesize) {
        free(buffer);
        fclose(file);
        perror("Failed to read file");
        return NULL;
    }

    buffer[read_size] = '\0';
    fclose(file);
    
    if (out_size) {
        *out_size = read_size;
    }
    return buffer;
}

int main(int argc, char *argv[])
{
    const char* filename = "adi_a2b_commandlist.xml";
    size_t size;

    if (argc == 2) filename = argv[1];
    
    char* content = read_file(filename, &size);
    if (!content) return 0;

    printf("File content (%zu bytes):\n%s\n", size, content);

    ADI_A2B_DISCOVERY_CONFIG configs[MAX_ACTIONS];
    int count = 0;

    parseXML(content, configs, &count);
    free(content);
    printf("count=%d\n", count);

    // Print the results
    for (int i = 0; i < count; i++) {
        switch(configs[i].eOpCode) {
            case WRITE:
                printf("Action %d: nDeviceAddr=%#x, eOpCode=write, nAddrWidth=%d, nAddr=%03d 0x%02x, nDataCount=%hu, eProtocol=%d, paConfigData=",
                        i, configs[i].nDeviceAddr, configs[i].nAddrWidth, configs[i].nAddr, configs[i].nAddr, configs[i].nDataCount, configs[i].eProtocol);
                break;
            case READ:
                printf("Action %d: nDeviceAddr=%#x, eOpCode=read , nAddrWidth=%d, nAddr=%03d 0x%02x, nDataCount=%hu, eProtocol=%d\n",
                        i, configs[i].nDeviceAddr, configs[i].nAddrWidth, configs[i].nAddr, configs[i].nAddr, configs[i].nDataCount, configs[i].eProtocol);
                continue;
            case DELAY:
                printf("Action %d: delay, nDataCount=%hu, sleep=", i, configs[i].nDataCount);
                break;
        };
        for (int j = 0; j < configs[i].nDataCount; j++)
            printf(configs[i].eOpCode != DELAY ? "0x%02x " : "%02dms ", configs[i].paConfigData[j]);
        printf("\n");
    }

    return 0;
}

   
