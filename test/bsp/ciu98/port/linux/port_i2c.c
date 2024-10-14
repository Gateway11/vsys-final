/**@file  port_i2c.c
* @brief  I2C Hardware interface adaptation
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/poll.h>
#include "port_i2c.h"
#include "port_util.h"
#include "port_config.h"
#include "error.h"
#include "util.h"
#include "log.h"




/**************************************************************************
* Global Variable Declaration
***************************************************************************/
I2C_SetDef i2c_comm_handle_se0={0};
i2c_comm_param_t i2c_comm_parm_se0={&i2c_comm_handle_se0, I2C_CHANNEL_I2C0, I2C_SLAVE_ADRR, I2C_SPEED_DEFAULT, I2C_TIMEOUT_DEFAULT,I2C_RETRY_DEFAULT,I2C_PERIPHERAL_SE0};
static uint8_t g_i2c_device_init[MAX_PERIPHERAL_DEVICE]={FALSE};

volatile uint32_t g_i2c_lock=0;
#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
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


/** @defgroup PORT_I2C PORT_I2C
  * @brief hardware portable layer i2c interface driver.
  * @{
  */



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const char   *i2cDev0  = "/dev/i2c-0" ;
static const char   *i2cDev1  = "/dev/i2c-1" ;



/* Private function  -----------------------------------------------*/
/** @defgroup Port_I2c_Private_Functions Port_I2c Private Functions
 * @{
 */



#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
/**
* @brief init irq gpio
* @param [in] 	no 
* @param [out]	no        
* @return GPIO file descriptor	
* @note no
*/
se_error_t port_i2c_gpio_irqwait_init() 
{
	se_error_t ret_code = SE_SUCCESS;
	char path[DIRECTION_MAX];
	char buff[10];
	
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", PORT_I2C_SE0_GPIO_IRQ);
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
* @brief init gpio
* @param [in] handle  device handel
* @return no	
* @note no
*/
void port_i2c_gpio_init(void)
{	
#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
	port_gpio_export(PORT_I2C_SE0_GPIO_IRQ);
	port_gpio_direction(PORT_I2C_SE0_GPIO_IRQ, IN);
	gpio_irq_edge_fd = port_gpio_edge_init(PORT_I2C_SE0_GPIO_IRQ,RISING);
	port_gpio_edge_set(PORT_I2C_SE0_GPIO_IRQ,FALLING);
	port_i2c_gpio_irqwait_init();
#endif
	port_gpio_export(PORT_I2C_SE0_RST_IO);
	usleep(30 * 1000);
	port_gpio_direction(PORT_I2C_SE0_RST_IO, OUT);
}


/**
* @brief Open the I2C device, and set param, etc.
* @param [in] handle   i2c interface handle
* @param [in] channel  i2c interface channel
* @param [in] slave	    i2c Slave device ID address
* @param [in] speed	    i2c communication speed
* @param [in] timeout  i2c timeout
* @param [in] retry	    i2c retry times
* @return status code
* @note no
*/
int port_i2c_set(void *handle, int channel, int slave, int speed, int timeout, int retry)
{
	int fd = 0;
	I2C_SetDef *i2cSetHandle = (I2C_SetDef *)handle;

	if ((fd = open (channel ==1 ? i2cDev0 : i2cDev1, O_RDWR)) < 0)
	{
		printf("Unable to open I2C device: %s\n", strerror (errno)) ;
		return -1;
	}

	i2cSetHandle->aiI2cFds[channel] = fd;

	/*
	* read-write slave device ID address
	*/
	if (ioctl (fd, I2C_SLAVE, slave) < 0)     
	{                                                               
		printf("Can't set i2c slave address: %s\n", strerror (errno))  ;  
		return -1;                                                    
	}  

	i2cSetHandle->aiI2cSlave[channel] = slave;

	
	ioctl(fd,I2C_TIMEOUT,timeout);//timeout
	ioctl(fd,I2C_RETRIES,retry);//retry times

	return fd ;
}


/**
* @brief send multi byte data throuth I2C
* @param [in] handle   interface handle
* @param [in] channel  interface channel
* @param [in] tx_data  start address of the data to be sent
* @param [in] len send data length 
* @return status code	
* @note no
*/
int port_i2c_tx(void *handle, int channel, uint8_t *tx_data, int len)
{
	int fd ;
	int ret = 0;
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;


	I2C_SetDef *i2cSetHandle = (I2C_SetDef *)handle;

	fd = i2cSetHandle->aiI2cFds[channel];

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  =  i2cSetHandle->aiI2cSlave[channel];//slave address
	i2cmsg.flags = 0;    //write
	i2cmsg.len	 = len;
	i2cmsg.buf = tx_data;

	if((ret=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
		//perror("ioctl()");
		//fprintf(stderr,"ioctl returned %d\n",ret);
		return -1;
	}

	return SE_SUCCESS;


}


/**
* @brief receive multi byte data throuth I2C
* @param [in] handle   interface handle
* @param [in] channel  interface channel
* @param [out] rx_data  start address of the data to be received
* @param [in] len recieve data length         
* @return status code
* @note no
*/
int port_i2c_rx(void *handle, int channel, uint8_t *rx_data, int len)
{
	int fd ;
	int ret = 0;
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;


	I2C_SetDef *i2cSetHandle = (I2C_SetDef *)handle;

	fd = i2cSetHandle->aiI2cFds[channel];

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  =  i2cSetHandle->aiI2cSlave[channel];//slave address
	i2cmsg.flags = I2C_M_RD;    //read
	i2cmsg.len	 = len;
	i2cmsg.buf = rx_data;

	if((ret=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
		//perror("ioctl()");
		//fprintf(stderr,"ioctl returned %d\n",ret);
		return -1;
	}

	return SE_SUCCESS;
}




/**
* @brief I2C Interface init
* @param [in] handle    i2c interface handle
* @param [in] channel  i2c channel
* @param [in] slave	    i2c Slave device ID address
* @param [in] speed	    i2c communication speed
* @param [in] timeout i2c timeout
* @param [in] retry      i2c retry times
* @return status code
* @note no
*/
se_error_t port_i2c_init(void *handle, int channel, int slave, int speed, int timeout, int retry)
{
	if(port_i2c_set(handle, channel, slave, speed, timeout, retry)<0)
	{
		return SE_ERR_PARAM_INVALID;
	}
	return SE_SUCCESS;
}


/**
* @brief i2c device handle clear
* @param [in] handle   i2c interface handle
* @param [in] channel i2c channel
* @return status code
* @note no
*/
se_error_t  port_i2c_deinit (void * handle, int channel)
{
	int fd ;
	I2C_SetDef *i2cSetHandle = (I2C_SetDef *)handle;

	fd = i2cSetHandle->aiI2cFds[channel];

	close(fd);
#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
	close( gpio_irq_value_fd);
	close( gpio_irq_edge_fd);
#endif
	return SE_SUCCESS;
}


/**
  * @}
  */




/* Exported functions --------------------------------------------------------*/

/** (@defgroup Port_I2c_Exported_Functions Port_I2c Exported Functions
  * @{
  */


/**
* @brief microsecond delay
* @param [in] us  microsecond
* @return state code		
* @note no
*/
se_error_t port_i2c_periph_delay(uint32_t us)
{
	struct timeval start;
	struct timeval end;

	gettimeofday(&start,NULL); 
	while(1)
	{
		gettimeofday(&end,NULL);
		if((uint32_t)((end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec))>=us)
		{
			break;	
		}	
	}

	return SE_SUCCESS;
}


/**
* @brief start timing
* @param [out]  timer_start  time value of start
* @return state code		
* @note no
*/
se_error_t port_i2c_periph_timer_start(util_timer_t *timer_start) 
{
	struct timeval timeofday;

	gettimeofday(&timeofday,NULL); 
	timer_start->tv_sec = timeofday.tv_sec; 
	timer_start->tv_usec = timeofday.tv_usec; 
	return SE_SUCCESS;			
}


/**
* @brief check timeout
* @param [in]  timer_differ time value of start 
* @return state code	
* @note no
*/
se_error_t port_i2c_periph_timer_differ(util_timer_t *timer_differ) 
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
* -# Initialize the I2C interface
* -# Set RST control IO to high level
* @param [in] periph  device handle
* @return state code	
* @note no
* @see  port_i2c_gpio_init  port_i2c_init
*/
se_error_t port_i2c_periph_init (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph) 
{	  
	se_error_t ret_code = SE_SUCCESS;
	i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		port_i2c_gpio_init();
	
		if(g_i2c_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = port_i2c_init(p_comm_param->i2c_handle,p_comm_param->channel,p_comm_param->slave,p_comm_param->speed,p_comm_param->timeout,p_comm_param->retry);
			if(ret_code != SE_SUCCESS)
			{
				break;
			}
			g_i2c_device_init[p_comm_param->slave_id] = TRUE;
		}

		if(p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
		{
			PORT_I2C_SE0_RST_HIGH();   //High level
		}
	}while(0);

	return ret_code;
}


/**
* @brief Slave communication periph close
* -# Terminate the I2C interface
* -# Set RST control IO to low level
* @param [in] periph  device handle
* @return state code	
* @note no
* @see  port_i2c_deinit
*/
se_error_t port_i2c_periph_deinit (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph) 
{
	se_error_t ret_code = SE_SUCCESS;
	i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if(p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
		{
			if(g_i2c_device_init[I2C_PERIPHERAL_SE0] == TRUE)
			{
				ret_code= port_i2c_deinit(p_comm_param->i2c_handle,p_comm_param->channel);
			}
			PORT_I2C_SE0_RST_LOW();   //Low level	
		}

		g_i2c_device_init[p_comm_param->slave_id] = FALSE;

	}while(0);

    return ret_code;
}


/**
* @brief Send multi-byte data from peripheral periph through I2C interface
* -# Call the port_i2c_tx function of the mcu hal library to send multi-byte data
* @param [in] periph  device handle
* @param [in] inbuf  start address of the data to be sent
* @param [in] inbuf_len send data length 
* @return state code	
* @note no
*/
se_error_t port_i2c_periph_transmit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf, uint32_t  inbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;
	
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

		if(g_i2c_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break;
		}

		if(port_lock(&g_i2c_lock)==LOCK_BUSY)
		{
			ret_code = SE_ERR_BUSY;
			break;
		}	

		ret_code = port_i2c_tx(p_comm_param->i2c_handle, p_comm_param->channel, inbuf, inbuf_len);
		port_unlock(&g_i2c_lock);
		if(ret_code != SE_SUCCESS)
		{
			ret_code = SE_ERR_COMM;
			break;
		}

	}while(0);
	//port_unlock(&g_i2c_lock);
	
	return ret_code;
}


/**
* @brief Receive multi-byte data from peripheral periph via I2C interface
* -# Call port_i2c_rx function of the mcu hal library to receive multi-byte data
* @param [in] periph  device handle
* @param [out] outbuf  start address of the data to be received
* @param [out] outbuf_len recieve data length   
* @return state code	
* @note no
*/
se_error_t port_i2c_periph_receive(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf, uint32_t *outbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

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

		if(g_i2c_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break;
		}

		if(port_lock(&g_i2c_lock)==LOCK_BUSY)
		{
			ret_code = SE_ERR_BUSY;
			break;
		}
				
		ret_code = port_i2c_rx(p_comm_param->i2c_handle, p_comm_param->channel, outbuf, *outbuf_len);
		port_unlock(&g_i2c_lock);
		if(ret_code != SE_SUCCESS)
		{
			ret_code = SE_ERR_COMM;
			break;
		}
		//port_unlock(&g_i2c_lock);

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
se_error_t port_i2c_periph_control(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *inbuf, uint32_t  inbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

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

		if(ctrlcode == PORT_I2C_CTRL_RST)
		{
			if(p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
			{
				PORT_I2C_SE0_RST_LOW();
				port_i2c_periph_delay(PORT_I2C_SE_RST_LOW_DELAY);  //When reset, RST low level duration
				PORT_I2C_SE0_RST_HIGH(); 
				port_i2c_periph_delay(PORT_I2C_SE_RST_HIGH_DELAY);  //After reset, RST high level duration
			}
		}

	}while(0);

	return ret_code;
}


#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
/**
* @brief wait gpio 
* @param [in] periph  device handle
* @param [in] wait_time wait time   
* @param [out]	no        
* @return state code	
* @note no
*/
se_error_t port_i2c_periph_gpio_irqwait_edge(uint32_t wait_time) 
{
	se_error_t ret_code = SE_SUCCESS;
	char buff[10];
    struct pollfd fds[1];

	fds[0].fd = gpio_irq_value_fd;
    fds[0].events = POLLPRI;

    do
    {
		ret_code = poll(fds, 1, wait_time);
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
            ret_code = lseek( gpio_irq_value_fd, 0, SEEK_SET);
            if (ret_code == -1)
            {
				break;
			}
            ret_code = read( gpio_irq_value_fd, buff, 10);
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

I2C_PERIPHERAL_DEFINE_BEGIN(I2C_PERIPHERAL_SE0)
    port_i2c_periph_init,
    port_i2c_periph_deinit,
    port_i2c_periph_delay,
    port_i2c_periph_timer_start,
    port_i2c_periph_timer_differ,
    port_i2c_periph_transmit,
    port_i2c_periph_receive,
    port_i2c_periph_control,
#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
	port_i2c_periph_gpio_irqwait_edge,
	// port_i2c_periph_gpio_irq_edge_set,
	// port_i2c_periph_gpio_irqwait_high,
#endif
    &i2c_comm_parm_se0,
I2C_PERIPHERAL_DEFINE_END()

I2C_PERIPHERAL_REGISTER(I2C_PERIPHERAL_SE0);


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




