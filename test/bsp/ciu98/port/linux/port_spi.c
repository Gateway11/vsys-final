/**@file  port_spi.c
* @brief  SPI Hardware interface adaptation
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <asm/ioctl.h>
#include <sys/poll.h> 
#include <linux/spi/spidev.h>
#include "port_spi.h"
#include "port_util.h"
#include "port_config.h"
#include "error.h"
#include "util.h"
#include "log.h"


/**************************************************************************
* Global Variable Declaration
***************************************************************************/
SPI_SetDef spi_comm_handle_se0={0};
spi_comm_param_t spi_comm_parm_se0={&spi_comm_handle_se0, SPI_CHANNEL_SPI0, PORT_SPI_COMM_MODE0, SPI_SPEED, SPI_BPW,SPI_PERIPHERAL_SE0,PORT_SPI_CS_CTRL_MODE_HARD};
static uint8_t g_spi_device_init[MAX_PERIPHERAL_DEVICE]={FALSE};
volatile uint32_t locktag_spi=0;
#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
static uint32_t gpio_irq_value_fd = 0;
static uint32_t gpio_irq_edge_fd = 0;
#endif


/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PORT 
  * @brief hardware  portable layer .
  * @{
  */


/** @defgroup PORT_SPI PORT_SPI
  * @brief hardware portable layer spi interface driver.
  * @{
  */



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const char   *spiDev0  = "/dev/spidev0.0" ;
static const char   *spiDev1  = "/dev/spidev0.1" ;

/* Private function  -----------------------------------------------*/
/** @defgroup Port_Spi Private_Functions Port_Spi Private Functions
 * @{
 */


#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
/**
* @brief init irq gpio
* @param [in] 	no 
* @param [out]	no        
* @return GPIO file descriptor	
* @note no
*/
se_error_t port_spi_gpio_irqwait_init() 
{
	se_error_t ret_code = SE_SUCCESS;
	char path[DIRECTION_MAX];
	char buff[10];
	
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", PORT_SPI_SE0_GPIO_IRQ);
	gpio_irq_value_fd = open(path, O_RDONLY);
	if (gpio_irq_value_fd < 0) 
	{
		//fprintf(stderr, "failed to open gpio value for reading!\n");
		return(-1);
	}
	else
	{
		ret_code = read(gpio_irq_value_fd, buff, 10);
		if (ret_code == -1)
		{
			return(-1);
		} 
		else
		{
			return SE_SUCCESS;
		}   
	}
}
#endif


/**
* @brief Initialize the gpio of the peripheral SE
* @return no	
* @note no
*/
void port_spi_gpio_init(HAL_SPI_PERIPHERAL_STRUCT_POINTER handle)
{
	spi_comm_param_pointer p_comm_param = (spi_comm_param_pointer)(handle->extra);

#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
	port_gpio_export(PORT_SPI_SE0_GPIO_IRQ);
	port_gpio_direction(PORT_SPI_SE0_GPIO_IRQ, IN);
	gpio_irq_edge_fd = port_gpio_edge_init(PORT_SPI_SE0_GPIO_IRQ,RISING);
	port_spi_gpio_irqwait_init();
#endif
	port_gpio_export(PORT_SPI_SE0_RST_IO);
	if(p_comm_param->cs_mode==PORT_SPI_CS_CTRL_MODE_SOFT)
	{
		port_gpio_export(PORT_SPI_SE0_CS_IO);
	}
	usleep(30 * 1000);
	
	port_gpio_direction(PORT_SPI_SE0_RST_IO, OUT);
	if(p_comm_param->cs_mode==PORT_SPI_CS_CTRL_MODE_SOFT)
	{
		port_gpio_direction(PORT_SPI_SE0_CS_IO, OUT);
		port_gpio_write(PORT_SPI_SE0_CS_IO, HIGH);	
	}
}


/**
* @brief Open the SPI device, and set it up, with the mode, etc.
* @param [in] handle  spi interface handle
* @param [in] channel  spi interface channel
* @param [in] mode	  spi communication mode
* @param [in] speed	  spi communication speed
* @param [in] bpw	  spi communication bits
* @return status code
* @note no
*/
int port_spi_set(void *handle, int channel, int mode, int speed, uint8_t bpw)
{
	int fd = 0 ;

	SPI_SetDef *spiSetHandle = (SPI_SetDef *)handle;

	if ((fd = open (channel == 0 ? spiDev0 : spiDev1, O_RDWR)) < 0)
	{
		printf("Unable to open SPI device: %s\n", strerror (errno)) ;
		return -1;
	}
	
	spiSetHandle->aiSpiSpeeds[channel] = speed;
	spiSetHandle->aiSpiFds[channel] = fd;

	/*
	* spi read-write mode
	*  Mode 0£º CPOL=0, CPHA=0
	*  Mode 1£º CPOL=0, CPHA=1
	*  Mode 2£º CPOL=1, CPHA=0
	*  Mode 3£º CPOL=1, CPHA=1
	*/
	if (ioctl (fd, SPI_IOC_WR_MODE, &mode) < 0)                     
	{                                                               
		printf("Can't set spi mode: %s\n", strerror (errno)) ;         
		return -1;                                                    
	}                                                               

	if (ioctl (fd, SPI_IOC_RD_MODE, &mode) < 0)                     
	{                                                               
		printf("Can't get spi mode: %s\n", strerror (errno)) ;        
		return -1;                                                 
	}    

	/*
	* spi read-write bits
	*/
	if (ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &bpw) < 0)          
	{                                                               
		printf("Can't set bits per word: %s\n", strerror (errno))  ;  
		return -1;                                                    
	}                                                              

	if (ioctl (fd, SPI_IOC_RD_BITS_PER_WORD, &bpw) < 0)          
	{                                                               
		printf("Can't get bits per word: %s\n", strerror (errno))  ;  
		return -1;                                                   
	}   

	/*
	* spi read-write speed 
	*********************************************************************************
	*/
	if (ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
	{
		printf("Can't set max speed hz: %s\n", strerror (errno));
		return -1;
	}

	if (ioctl (fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0)
	{
		printf("Can't get max speed hz: %s\n", strerror (errno));
		return -1;
	}


	return fd ;
}


/**
* @brief send multi byte data throuth SPI 
* @param [in] periph  interface handle
* @param [in] channel  interface channel
* @param [in] tx_data  start address of the data to be sent
* @param [in] len send data length     
* @return state code		
* @note no
*/
int port_spi_tx(void *handle, int channel, uint8_t *tx_data, int len)
{
	int fd, count = 0;
	int ret;
	struct spi_ioc_transfer spi ;
	SPI_SetDef *spiSetHandle = (SPI_SetDef *)handle;

	memset (&spi, 0, sizeof (spi)) ;

	fd = spiSetHandle->aiSpiFds[channel];

    printf("{");
    for (int i = 0; i < len; i++) {
        printf("0x%02x ", tx_data[i]);
        count += tx_data[i];
    }
    printf("} ");
    if (count != 0) printf("\n");

	spi.tx_buf        = (unsigned long)tx_data ;
	spi.rx_buf        = (unsigned long)NULL ;
	spi.len           = len ;
	spi.delay_usecs   = spiSetHandle->sSpiDelay;
	spi.speed_hz      = spiSetHandle->aiSpiSpeeds [channel] ;
	spi.bits_per_word = spiSetHandle->bSpiBPW; 

	ret = ioctl (fd, SPI_IOC_MESSAGE(1), &spi) ; 
	if(ret<0)
	{
		return ret;
	}
	return SE_SUCCESS;

}



/**
* @brief receive multi byte data throuth SPI 
* @param [in] periph  interface handle
* @param [in] channel  interface channel
* @param [out] rx_data  start address of the data to be received
* @param [out] len recieve data length           
* @return state code	
* @note no
*/
int port_spi_rx(void *handle, int channel, uint8_t *rx_data, int len)
{
	int fd, i;
	int ret;
	struct spi_ioc_transfer spi ;
	SPI_SetDef *spiSetHandle = (SPI_SetDef *)handle;

	memset (&spi, 0, sizeof (spi)) ;

	fd = spiSetHandle->aiSpiFds[channel];

	spi.tx_buf        = (unsigned long)NULL ;
	spi.rx_buf        = (unsigned long)rx_data ;
	spi.len           = len ;
	spi.delay_usecs   = spiSetHandle->sSpiDelay;
	spi.speed_hz      = spiSetHandle->aiSpiSpeeds [channel] ;
	spi.bits_per_word = spiSetHandle->bSpiBPW; ;

	ret = ioctl (fd, SPI_IOC_MESSAGE(1), &spi) ;
	if(ret<0)
	{
		return ret;
	}

    for (i = 0; i < len; i++) {
        if (rx_data[i] != 0xFF) break;
    }
    if (i != len) {
        printf("[");
        for (int j = 0; j < len; j++) {
            printf("0x%02x ", rx_data[j]);
        }
        printf("] ");
        if (rx_data[0] != PIB_ACTIVE_FRAME && rx_data[0] != PIB_PROCESS_FRAME) printf("\n");
    }
	return SE_SUCCESS;

}




/**
* @brief SPI Interface init
* @param [in] handle   spi interface handle
* @param [in] channel spi channel
* @param [in] mode	   spi communication mode
* @param [in] speed	   spi communication speed
* @param [in] bpw	   spi communication bits
* @return state code	
* @note no
*/
se_error_t port_spi_init(void *handle, int channel, int mode, int speed, uint8_t bpw)
{
	if(port_spi_set(handle, channel, mode, speed, bpw)<0)
	{
		return SE_ERR_PARAM_INVALID;
	}
	return SE_SUCCESS;
}


/**
* @brief SPI device handle clear
* @param [in] handle   spi interface handle
* @param [in] channel spi channel
* @return state code	
* @note no
*/
se_error_t  port_spi_deinit (void * handle, int channel)
{
	int fd ;
	SPI_SetDef *spiSetHandle = (SPI_SetDef *)handle;

	fd = spiSetHandle->aiSpiFds[channel];

	close(fd);
#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
	close(gpio_irq_value_fd);
	close(gpio_irq_edge_fd);
#endif
	return SE_SUCCESS;
}


/**
  * @}
  */





/* Exported functions --------------------------------------------------------*/

/** (@defgroup Port_Spi_Exported_Functions Port_Spi Exported Functions
  * @{
  */


/**
* @brief microsecond delay
* @param [in] us  microsecond
* @return state code		
* @note no
*/
se_error_t port_spi_periph_delay(uint32_t us)
{
//	struct timeval start;
//	struct timeval end;

//	gettimeofday(&start,NULL); 
///	while(1)
//	{
//		gettimeofday(&end,NULL);
//		if((uint32_t)((end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec))>=us)
//		{
//			break;	
//		}	
//	}

	usleep(us);

	return SE_SUCCESS;
}


/**
* @brief start timing
* @param [out]  timer_start   time value of start
* @return state code		
* @note no
*/
se_error_t port_spi_periph_timer_start(util_timer_t *timer_start) 
{
	struct timeval timeofday;

	gettimeofday(&timeofday,NULL); 
	timer_start->tv_sec = timeofday.tv_sec; 
	timer_start->tv_usec = timeofday.tv_usec; 
	return SE_SUCCESS;		
}


/**
* @brief check timeout
* @param [in] timer_differ  time value of start 
* @return state code		
* @note no
*/
se_error_t port_spi_periph_timer_differ(util_timer_t *timer_differ) 
{
	struct timeval end;
	uint32_t time_use_ms;

	gettimeofday(&end,NULL);

	time_use_ms=(uint32_t)((end.tv_sec-timer_differ->tv_sec)*1000000+(end.tv_usec-timer_differ->tv_usec))/1000;
	if(time_use_ms>=timer_differ->interval)
	{
		return SE_ERR_TIMEOUT;		
	}
	return SE_SUCCESS;
}



/**
* @brief Slave communication periph init
* -# Initialize the gpio of the peripheral SE
* -# Initialize the SPI interface
* -# Set RST control IO to high level
* @param [in] periph  device handle
* @return state code	
* @note no
* @see  port_spi_gpio_init  port_spi_init
*/
se_error_t port_spi_periph_init (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph) 
{	  
	se_error_t ret_code = SE_SUCCESS;
	spi_comm_param_pointer p_comm_param = (spi_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		port_spi_gpio_init(periph);
	
		if(g_spi_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = port_spi_init(p_comm_param->spi_handle,p_comm_param->channel,p_comm_param->mode,p_comm_param->speed,p_comm_param->bpw);
			if(ret_code != SE_SUCCESS)
			{
				break;
			}
			g_spi_device_init[p_comm_param->slave_id] = TRUE;
		}

		if(p_comm_param->slave_id == SPI_PERIPHERAL_SE0)
		{
			PORT_SPI_SE0_RST_HIGH();    //High level
		}
	}while(0);

	return ret_code;
}


/**
* @brief Slave communication periph close
* -# Terminate the SPI interface
* -# Set RST control IO to low level
* @param [in] periph  device handle
* @return state code		
* @note no
* @see  port_spi_deinit
*/
se_error_t port_spi_periph_deinit (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_comm_param_pointer p_comm_param = (spi_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if(p_comm_param->slave_id == SPI_PERIPHERAL_SE0)
		{
			if(g_spi_device_init[SPI_PERIPHERAL_SE0] == TRUE)
			{
				ret_code= port_spi_deinit(p_comm_param->spi_handle,p_comm_param->channel);
			}
			PORT_SPI_SE0_RST_LOW();   //Low level		
		}

		g_spi_device_init[p_comm_param->slave_id] = FALSE;

	}while(0);

    return ret_code;
}


/**
* @brief Slave communication periph chip enable or disable
* @param [in] enable  enable: TRUE,  disable: FALSE
* @return state code	
*/
se_error_t port_spi_periph_chip_select (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, bool enable) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_comm_param_pointer p_comm_param = (spi_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if(g_spi_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break;
		}
				
		if(enable == TRUE)
		{
			if(p_comm_param->cs_mode==PORT_SPI_CS_CTRL_MODE_SOFT)
			{
				if(p_comm_param->slave_id == SPI_PERIPHERAL_SE0)
				{
					PORT_SPI_SE0_CS_HIGH();
				}	
			}
		}
		else
		{
			if(p_comm_param->cs_mode==PORT_SPI_CS_CTRL_MODE_SOFT)
			{
				if(p_comm_param->slave_id == SPI_PERIPHERAL_SE0)
				{
					PORT_SPI_SE0_CS_LOW();
				}	
			}
		}
	}while(0);

	return ret_code;
}


/**
* @brief Send multi-byte data from peripheral periph through SPI interface
* -# Call the port_spi_tx function of the mcu hal library to send multi-byte data
* @param [in] periph  device handle
* @param [in] inbuf  start address of the data to be sent
* @param [in] inbuf_len send data length    
* @return state code		
* @note no
*/
se_error_t port_spi_periph_transmit(HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf, uint32_t  inbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_comm_param_pointer p_comm_param = (spi_comm_param_pointer)periph->extra;
	
	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if((inbuf == NULL) || (inbuf_len == 0U))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if(g_spi_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break;
		}
		
		if(port_lock(&locktag_spi) == LOCK_UNBUSY)
		{
			ret_code = port_spi_tx(p_comm_param->spi_handle, p_comm_param->channel, inbuf, inbuf_len);
			port_unlock(&locktag_spi);
			if(ret_code != SE_SUCCESS)
			{
				ret_code = SE_ERR_COMM;
				break;
			}
		}
		else 
		{
             ret_code = SE_ERR_BUSY;
			 break;
		}
	}while(0);
	
	return ret_code;
}


/**
* @brief Receive multi-byte data from peripheral periph via SPI interface
* -# Call port_spi_rx function of the mcu hal library to receive multi-byte data
* @param [in] periph  device handle
* @param [out] outbuf  start address of the data to be received
* @param [out] outbuf_len recieve data length        
* @return state code		
* @note no
*/
se_error_t port_spi_periph_receive(HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf, uint32_t *outbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_comm_param_pointer p_comm_param = (spi_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if((outbuf == NULL) || (outbuf_len == NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if(g_spi_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break;
		}
		
		if(port_lock(&locktag_spi) == LOCK_UNBUSY)
		{
			ret_code = port_spi_rx(p_comm_param->spi_handle, p_comm_param->channel, outbuf, *outbuf_len);
			port_unlock(&locktag_spi);

			if(ret_code != SE_SUCCESS)
			{
				ret_code = SE_ERR_COMM;
				break;
			}
	    }
		else 
		{
			 ret_code = SE_ERR_BUSY;
			 break;
		}

	}while(0);
	
	return ret_code;
}


/**
* @brief According to the control code and input data, reset the peripheral periph control operation
* @param [in] periph  device handle
* @param [in] ctrlcode  control code
* @param [in] inbuf   start address for sending control data
* @param [in] inbuf_len control data length           
* @return state code		
* @note no
*/
se_error_t port_spi_periph_control(HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *inbuf, uint32_t  *inbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_comm_param_pointer p_comm_param = (spi_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if(ctrlcode == 0U)
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if(ctrlcode == PORT_SPI_CTRL_RST)
		{
			if(p_comm_param->slave_id == SPI_PERIPHERAL_SE0)
			{
				PORT_SPI_SE0_RST_LOW();
				port_spi_periph_delay(PORT_SPI_SE_RST_LOW_DELAY);  //When reset, RST low level duration
				PORT_SPI_SE0_RST_HIGH(); 
				port_spi_periph_delay(PORT_SPI_SE_RST_HIGH_DELAY);   //After reset, RST high level duration
			}
		}

	}while(0);

	return ret_code;
}

#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE

/**
* @brief wait for gpio signel
* @param [in] wait_time wait time 
* @param [out]	no        
* @return state code
* @note no
*/
se_error_t port_spi_periph_gpio_irqwait_edge(uint32_t wait_time) 
{
	se_error_t ret_code = SE_SUCCESS;
	char buff[10];
    struct pollfd fds[1];

	fds[0].fd = gpio_irq_value_fd;
    fds[0].events = POLLPRI;
    do
    {
		ret_code = poll(fds, 1, wait_time);
		// ret_code = poll(fds, 1, -1);
        if (ret_code == 0)
		{
			ret_code = SE_ERR_TIMEOUT;
			break;	
		}
        else if(ret_code == -1)
		{
			break; 
		}

        if (fds[0].revents & POLLPRI)
        {
            ret_code = lseek(gpio_irq_value_fd, 0, SEEK_SET);
            if (ret_code == -1)
            {
				break;
			}
            ret_code = read(gpio_irq_value_fd, buff, 10);
            if (ret_code == -1)
            {
				break;
			}

			ret_code = SE_SUCCESS; 
			// LOGE("\nit is irq!!!!!!\n");
        }

    }while(0);
	return ret_code;
}


#endif

SPI_PERIPHERAL_DEFINE_BEGIN(SPI_PERIPHERAL_SE0)
    port_spi_periph_init,
    port_spi_periph_deinit,
    port_spi_periph_delay,
    port_spi_periph_timer_start,
    port_spi_periph_timer_differ,
    port_spi_periph_chip_select,
    port_spi_periph_transmit,
    port_spi_periph_receive,
    port_spi_periph_control,
#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
	port_spi_periph_gpio_irqwait_edge,
	// port_spi_periph_gpio_irq_edge_set,
	// port_spi_periph_gpio_irqwait_high,
#endif
    &spi_comm_parm_se0,
SPI_PERIPHERAL_DEFINE_END()

SPI_PERIPHERAL_REGISTER(SPI_PERIPHERAL_SE0);

/**
  * @}
  */

/**
  * @}
  */



/**
  * @}
  */

/**
  * @}
  */




