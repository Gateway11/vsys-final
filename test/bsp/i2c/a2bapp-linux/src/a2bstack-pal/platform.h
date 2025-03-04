
#include <stdio.h>                         	/*!< System Stdio library */
#include <stdarg.h>
#include <string.h>
#include <cdefbf527.h>
#include <services/pwr/adi_pwr.h>           /*!< System power header file */
#include <drivers/uart/adi_uart.h>  		/*!< System UART driver file */
#include "adi_initialize.h"                	/*!< ADI initialization header */
#include "adi_a2b_datatypes.h"
#include "perieng.h"

#ifndef _PLATFORM_H_
#define _PLATFORM_H_


/******************* U S E R  C O N F I G U R A T I O N S *********************/
#ifndef ADI_UART_ENABLE
#define 										ADI_UART_ENABLE
#endif
#define ADI_UART_CMD_SIZE             			(300u)			/*!< Max size of command string */
#define ADI_UART_CMD_HIS_SIZE             		(5u)			/*!< Max size of command history */


#define UART_BAUD_RATE							(115200u)
#define UART_NUM								(0u)

#define UART_MEMORY_SIZE      					((ADI_UART_BIDIR_DMA_MEMORY_SIZE>>2u)*4u + 4u)
#define UART_BUFFER_SIZE     					(512u)

#define SERIAL_TX_FIFO_SIZE     				(32u*1024u)
#define SERIAL_RX_FIFO_SIZE     				(128u)
#define ADI_PLT_UART_RX_MSG_QUEUE_SIZE			(ADI_UART_CMD_SIZE * 2)	/*!< UART RX message queue */
#define ADI_PLT_UART_TX_MSG_QUEUE_SIZE			(260u)						/*!< UART TX message queue */
#define ADI_PLT_UART_TX_MSG_SIZE_MAX			(300u)						/*!< UART TX message size in bytes */

#ifdef ADI_UART_ENABLE
#define ADI_UART_PRINT    		uart_printf
#else
#define ADI_UART_PRINT    		dummy_printf
#endif
#define _NORMALTEXT_                   "\e[0m"
#define _BOLDTEXT_                     "\e[1m"
#define _ITALICTEXT_                   "\e[3m"
#define _UNDERLINETEXT_                "\e[4m"
#define _BLINKTEXT_                    "\e[5m"
#define _REDTEXT_					   "\e[31m"
#define _GREENTEXT_					   "\e[32m"
#define _YELLOWTEXT_                   "\e[33m"
#define _BLUETEXT_                     "\e[34m"
#define _MAGENTATEXT_                  "\e[35m"
#define _CYANTEXT_                     "\e[36m"
#define _WHITETEXT_                    "\e[37m"
#define _BLACKTEXT_                    "\e[30m"
#define _IREDTEXT_                     "\e[91m"
#define _IGREENTEXT_                   "\e[92m"
#define _IYELLOWTEXT_                  "\e[93m"
#define _IBLUETEXT_                    "\e[94m"
#define _IMAGENTATEXT_                 "\e[95m"
#define _ICYANTEXT_                    "\e[96m"
#define _IWHITETEXT_                   "\e[97m"
#define _IBLACKTEXT_                   "\e[90m"
#define _CLEARSCREEN_                  "\e[2J"
#define _CURSORHOME_                   "\e[H"

/*============= D A T A  T Y P E S =============*/

typedef struct
{
	uint8_t aMsg[ADI_PLT_UART_TX_MSG_SIZE_MAX];			/*!< UART Tx message */
	uint32_t nLen;										/*!< UART Tx payload length */
}ADI_UART_TX_MSG;

typedef struct
{
	ADI_UART_TX_MSG oMsg[ADI_PLT_UART_TX_MSG_QUEUE_SIZE];		/*!< UART Tx message */
    uint16_t Head;												/*!< Head value of the queue */
    uint16_t Tail;												/*!< Tail value of the queue */
    uint16_t Size;												/*!< Size of the queue */
    uint16_t Max;
}ADI_UART_TX_MSG_FIFO;

typedef struct
{
	uint8_t aData[ADI_PLT_UART_RX_MSG_QUEUE_SIZE];                /*!< UART Rx message */
    uint16_t Head;                                                /*!< Head value of the queue */
    uint16_t Tail;                                                /*!< Tail value of the queue */
    uint16_t Size;                                                /*!< Size of the queue */
    uint16_t Max;
}ADI_UART_RX_MSG_FIFO;



typedef enum
{
	NORMAL_TEXT,
	BOLD_TEXT,
	UNDERLINE_TEXT,
	BLINK_TEXT,
	RED_TEXT,
	GREEN_TEXT,
	YELLOW_TEXT,
	BLUE_TEXT,
	MAGENTA_TEXT,
	CYAN_TEXT,
	WHITE_TEXT,
	BLACK_TEXT,

}CONSOLE_FONT;


/*=====================Function Prototypes==============================*/
a2b_UInt32 uart_printf(const a2b_Char *data, ...);
bool adi_platform_UartRxDequeue(uint8_t *pData);
ADI_UART_HANDLE adi_UartInit(void);
#ifdef ADI_ENABLE_RTM_SUPPORT
void a2b_spiPeriWrRdCbNb(void* pCbParam, A2B_SPI_EVENT eEventType, a2b_Int8 nNodeAddr);
#endif

void SetConsoleFont(CONSOLE_FONT eFont);

#endif
