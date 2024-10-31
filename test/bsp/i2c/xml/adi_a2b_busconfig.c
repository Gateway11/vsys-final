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
int count = 0;

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
            config->nDataCount = index;
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
            if (actionLength >= sizeof(action)) break;

            strncpy(action, actionStart, actionLength);
            action[actionLength] = '\0'; // Null-terminate

            parseAction(action, &configs[*count], 104);
            (*count)++;
            actionStart = strstr(actionEnd, "<action");
        }
    }
}


#if 0
#include <linux/fs.h>
#include <linux/slab.h>

#define BUFFER_SIZE 4096

ssize_t read_file(const char *filename, char **buffer) {
    struct file *file;
    mm_segment_t oldfs;
    ssize_t bytes_read;
    char *temp_buffer;

    file = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(file)) {
        return PTR_ERR(file);
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    temp_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!temp_buffer) {
        filp_close(file, NULL);
        set_fs(oldfs);
        return -ENOMEM;
    }

    bytes_read = kernel_read(file, temp_buffer, BUFFER_SIZE - 1, &file->f_pos);
    if (bytes_read < 0) {
        kfree(temp_buffer);
        filp_close(file, NULL);
        set_fs(oldfs);
        return bytes_read;
    }

    temp_buffer[bytes_read] = '\0';
    *buffer = temp_buffer;
    set_fs(oldfs);
    filp_close(file, NULL);

    return bytes_read;
}

#else

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
#endif

void adi_a2b_Concat_Addr_Data(uint8_t pDstBuf[], uint32_t nAddrwidth, uint32_t nAddr)
{
	/* Store the read values in the place holder */
	switch (nAddrwidth)
	{ /* Byte */
		case 1u:
			pDstBuf[0u] = (uint8_t)nAddr;
			break;
			/* 16 bit word*/
		case 2u:

			pDstBuf[0u] = (uint8_t)(nAddr >> 8u);
			pDstBuf[1u] = (uint8_t)(nAddr & 0xFFu);

			break;
			/* 24 bit word */
		case 3u:
			pDstBuf[0u] = (uint8_t)((nAddr & 0xFF0000u) >> 16u);
			pDstBuf[1u] = (uint8_t)((nAddr & 0xFF00u) >> 8u);
			pDstBuf[2u] = (uint8_t)(nAddr & 0xFFu);
			break;

			/* 32 bit word */
		case 4u:
			pDstBuf[0u] = (uint8_t)(nAddr >> 24u);
			pDstBuf[1u] = (uint8_t)((nAddr & 0xFF0000u) >> 16u);
			pDstBuf[2u] = (uint8_t)((nAddr & 0xFF00u) >> 8u);
			pDstBuf[3u] = (uint8_t)(nAddr & 0xFFu);
			break;

		default:
			break;

	}
}

void adi_a2b_Delay(uint32_t nTime)
{
    printf("Sleep %dms\n", nTime);
    usleep(nTime * 1000);
}

int32_t adi_a2b_NetworkSetup()
{
	ADI_A2B_DISCOVERY_CONFIG* pOPUnit;
	uint32_t nIndex, nIndex1;
	int32_t status = 0;
	/* Maximum number of writes */
	static uint8_t aDataBuffer[6000];
	static uint8_t aDataWriteReadBuf[4u];
	uint32_t nDelayVal;

	/* Loop over all the configuration */
	//for (nIndex = 0; nIndex < CONFIG_LEN; nIndex++)
	for (nIndex = 0; nIndex < count; nIndex++)
	{
		//pOPUnit = &gaA2BConfig[nIndex];
		pOPUnit = &configs[nIndex];
		/* Operation code*/
		switch (pOPUnit->eOpCode)
		{
			/* Write */
			case WRITE:
				adi_a2b_Concat_Addr_Data(&aDataBuffer[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
				(void)memcpy(&aDataBuffer[pOPUnit->nAddrWidth], pOPUnit->paConfigData, pOPUnit->nDataCount);
				/* PAL Call, replace with custom implementation  */
				status = adi_a2b_I2CWrite((uint16_t)pOPUnit->nDeviceAddr, (uint16_t)(pOPUnit->nAddrWidth + pOPUnit->nDataCount), &aDataBuffer[0u]);
				break;

				/* Read */
			case READ:
				(void)memset(&aDataBuffer[0u], 0u, pOPUnit->nDataCount);
				adi_a2b_Concat_Addr_Data(&aDataWriteReadBuf[0u], pOPUnit->nAddrWidth, pOPUnit->nAddr);
				/* PAL Call, replace with custom implementation  */
				status = adi_a2b_I2CWriteRead((uint16_t)pOPUnit->nDeviceAddr, (uint16_t)pOPUnit->nAddrWidth, &aDataWriteReadBuf[0u], (uint16_t)pOPUnit->nDataCount, &aDataBuffer[0u]);
				break;

				/* Delay */
			case DELAY:
				nDelayVal = 0u;
				for(nIndex1 = 0u; nIndex1 < pOPUnit->nDataCount; nIndex1++)
				{
					nDelayVal = pOPUnit->paConfigData[nIndex1] | nDelayVal << 8u;
				}
				(void)adi_a2b_Delay(nDelayVal);
				break;

			default:
				break;

		}
		if (status != 0)
		{
			/* I2C read/write failed! No point in continuing! */
			break;
		}
	}

	return status;
}

int main(int argc, char *argv[])
{
    const char* filename = "adi_a2b_commandlist.xml";
    size_t size;

    if (argc == 2) filename = argv[1];
    
    char* content = read_file(filename, &size);
    if (!content) return 0;

    printf("File content (%zu bytes):\n%s\n", size, content);

    //ADI_A2B_DISCOVERY_CONFIG configs[MAX_ACTIONS];
    //int count = 0;

    parseXML(content, configs, &count);
    free(content);
    printf("count=%d\n", count);

    // Print the results
    for (int i = 0; i < count; i++) {
        switch(configs[i].eOpCode) {
            case WRITE:
                printf("Action %02d: nDeviceAddr=%#x, eOpCode=write, nAddrWidth=%d, nAddr=%03d 0x%02x, nDataCount=%hu, eProtocol=%d, paConfigData=",
                        i, configs[i].nDeviceAddr, configs[i].nAddrWidth, configs[i].nAddr, configs[i].nAddr, configs[i].nDataCount, configs[i].eProtocol);
                break;
            case READ:
                printf("Action %02d: nDeviceAddr=%#x, eOpCode=read , nAddrWidth=%d, nAddr=%03d 0x%02x, nDataCount=%hu, eProtocol=%d\n",
                        i, configs[i].nDeviceAddr, configs[i].nAddrWidth, configs[i].nAddr, configs[i].nAddr, configs[i].nDataCount, configs[i].eProtocol);
                continue;
            case DELAY:
                printf("Action %02d: delay, nDataCount=%hu, sleep=", i, configs[i].nDataCount);
                break;
        };
        for (int j = 0; j < configs[i].nDataCount; j++)
            printf(configs[i].eOpCode != DELAY ? "0x%02x " : "%02dms ", configs[i].paConfigData[j]);
        printf("\n");
    }

#if 1
    /* PAL call, open I2C driver */
    arrayAddrs[0] = adi_a2b_I2COpen(I2C_MASTER_ADDR);
    arrayAddrs[1] = adi_a2b_I2COpen(I2C_SLAVE_ADDR);
    
    /* Configure a2b system */
    adi_a2b_NetworkSetup();

    //close(arrayAddrs[0]);
    //close(arrayAddrs[1]);
#endif

    return 0;
}
