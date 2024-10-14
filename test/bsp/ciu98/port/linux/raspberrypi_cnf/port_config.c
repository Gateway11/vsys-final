/**@file  port_config.c
* @brief  system initialization etc.
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "port_config.h"

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PORT 
  * @brief hardware  portable layer .
  * @{
  */


/** @defgroup PORT_CONFIG PORT_CONFIG
  * @brief hardware portable layer common interface driver.
  * @{
  */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function  -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

/** @defgroup Port_config_Exported_Functions Port_config Exported Functions
  * @{
  */

/**
* @brief Notify the system that the GPIO pin is exported
* @param [in] pin  GPIO number
* @return 0 :success ;  -1: fail
* @note no
*/
int port_gpio_export(int pin)
{
	char buffer[BUFFER_MAX];
	int len;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) 
	{
		//fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}  

	len = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, len);

	close(fd);
	return(0);
}

/**
* @brief Notify the system that the GPIO pin is unexported
* @param [in] pin  GPIO number
* @return 0 :success ;  -1: fail
* @note no
*/
int port_gpio_unexport(int pin)
{
	char buffer[BUFFER_MAX];
	int len;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd < 0) 
	{
		//fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}

	len = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, len);

	close(fd);
	return(0);
}

/**
* @brief Configure GPIO input/output direction
* @param [in] pin  GPIO number
* @param [in] dir  OUT:output, IN:input
* @return 0 :success ;  -1: fail
* @note no
*/
int port_gpio_direction(int pin, int dir)
{
	static const char dir_str[]  = "in\0out";
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0) 
	{
		//fprintf(stderr, "failed to open gpio direction for writing!\n");
		return(-1);
	}

	if (write(fd, &dir_str[dir == IN ? 0 : 3], dir == IN ? 2 : 3) < 0) 
	{
		//fprintf(stderr, "failed to set direction!\n");
		return(-1);
	}

	close(fd);
	return(0);
}


/**
* @brief GPIO output high or low level
* @param [in] pin  GPIO number
* @param [in] value  HIGH: High level, LOW:Low level
* @return 0 :success ;  -1: fail
* @note no
*/
int port_gpio_write(int pin, int value)
{
	static const char s_values_str[] = "01";
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0) 
	{
		//fprintf(stderr, "failed to open gpio value for writing!\n");
		return(-1);
	}

	if (write(fd, &s_values_str[value == LOW ? 0 : 1], 1) < 0)
	{
		//fprintf(stderr, "failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);
}


/**
* @brief Get GPIO level 
* @param [in] pin  GPIO number
* @return HIGH: High level, LOW:Low level;-1: fail
* @note no
*/
int port_gpio_read(int pin)
{
	char path[DIRECTION_MAX];
	char value_str[3];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (fd < 0) 
	{
		//fprintf(stderr, "failed to open gpio value for reading!\n");
		return(-1);
	}

	if (read(fd, value_str, 3) < 0) 
	{
		//fprintf(stderr, "failed to read value!\n");
		return(-1);
	}

	close(fd);
	return(atoi(value_str));
}


/**
* @brief get gpio irq trig mode
* @param [in] pin  pin num
* @param [in] edge_mode  edge trig mode
* @return gpio edge fp
* @note no
*/
int port_gpio_edge_init(int pin, int edge_mode)
{
	static const char dir_str[]  = "rising\0falling\0both\0none";
	char path[DIRECTION_MAX];
	int fd;

	// if(edge_mode == 0xFF)
	// 	return(0);
	
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/edge", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0) 
	{
		//fprintf(stderr, "failed to open gpio direction for writing!\n");
		return(-1);
	}

	switch(edge_mode)
		{
			case RISING:
				if (write(fd, &dir_str[0], 6) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
			case FALLING:
				if (write(fd, &dir_str[7], 7) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
			case BOTH:
				if (write(fd, &dir_str[15], 4) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
			case NONE:
				if (write(fd, &dir_str[20], 4) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
		}

	// close(fd);

	return(fd);
}

/**
* @brief set GPIO IRQ trig mode
* @param [in] pin  pin num
* @param [in] edge_mode  edge mode
* @return no
* @note no
*/
int port_gpio_edge_set(int gpio_edge_fd, int edge_mode)
{
	static const char dir_str[]  = "rising\0falling\0both\0none";
	// char path[DIRECTION_MAX];
	// int fd;

	// if(edge_mode == 0xFF)
	// 	return(0);
	
	// snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/edge", pin);
	// fd = open(path, O_WRONLY);
	// if (fd < 0) 
	// {
	// 	//fprintf(stderr, "failed to open gpio direction for writing!\n");
	// 	return(-1);
	// }

	switch(edge_mode)
		{
			case RISING:
				if (write(gpio_edge_fd, &dir_str[0], 6) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
			case FALLING:
				if (write(gpio_edge_fd, &dir_str[7], 7) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
			case BOTH:
				if (write(gpio_edge_fd, &dir_str[15], 4) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
			case NONE:
				if (write(gpio_edge_fd, &dir_str[20], 4) < 0) 
				{
					//fprintf(stderr, "failed to set edge!\n");
					return(-1);
				}
				break;
		}

	// close(fd);
	return(0);
}


/**
* @brief gpio init
* @param no
* @return no
* @note no
*/
void port_gpio_init(void)
{	
	port_gpio_export(PORT_POWER_CTRL_IO);
	usleep(30 * 1000);
	port_gpio_direction(PORT_POWER_CTRL_IO, OUT);

	PORT_POWER_CTRL_ON();
}



/**
* @brief mcu initialization
* @param no
* @return no
* @note no
*/
void port_mcu_init(void)
{
	port_gpio_init();
}

/**
* @brief mcu clear
* @param no
* @return no
* @note no
*/
void port_mcu_deinit(void)
{

}



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


