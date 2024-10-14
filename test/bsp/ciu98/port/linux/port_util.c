/**@file  port_util.c
* @brief  printf, power-on control, etc.
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "port_util.h"
#include "port_config.h"



//_weak void port_printf(const char *format, ...)
/**
* @brief printf output log
* @param [in] format  printf output farmat
* @return no	
* @note no
*/
void port_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}


/**
* @brief SE power on
* @param no
* @return no	
* @note no
*/
void port_power_on(void)
{
	PORT_POWER_CTRL_ON();
}


/**
* @brief SE power off
* @param no
* @return no	
* @note no
*/
void port_power_off(void)
{
	PORT_POWER_CTRL_OFF();
}


/**
* @brief resource protection lock
* @param no
* @return no	
* @note no
*/
int port_lock(volatile int *lock)
{
//int status = BUSY;
if((*lock) == 0)
{
(*lock)++;
  if((*lock)==1)
   {
   return LOCK_UNBUSY;
   }
}
return LOCK_BUSY;
}


/**
* @brief resource protection unlock
* @param no
* @return no	
* @note no
*/

int port_unlock(volatile int *lock)
{
  
(*lock) =0 ;

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


