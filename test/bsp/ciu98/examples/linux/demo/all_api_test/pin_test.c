/**@file  info_test.h
* @brief  info_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "pin_test.h"

/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup INFO_TEST INFO_TEST
  * @brief info_test interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup INFO_TEST_Exported_Functions INFO_TEST Exported Functions
  * @{
  */


/**
* @brief change pin demo, reload pin demo and verify pin demo
* @param no
* @return refer error.h
* @note no
* @see api_write_pin api_change_pin api_reload_pin api_verify_pin
*/
se_error_t pin_test (void)
{
	se_error_t ret = 0;
	uint32_t pin_buff_len = 0;
  pin_t usr_pin = {0};
	pin_t pin = {0};
	uint8_t user_pin_buff[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	uint8_t admin_pin_buff[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
	uint8_t pin_buff[8] = {0};
	/****************************************write pin test*****************************************/
  //obtain the write authentication
  usr_pin.limit = 0x0F;
	usr_pin.owner = USER_PIN;
	usr_pin.pin_len = 6;
	memcpy(usr_pin.pin_value, user_pin_buff ,6);
	ret =  api_write_pin(&usr_pin, true);
	// if ( ret != SE_SUCCESS)
	// {
	// 	LOGE("failed to api_write_pin!\n");
	// 	return ret;
	// }
	 /****************************************change pin test*****************************************/	
    pin.owner = USER_PIN;
	memcpy (pin.pin_value, user_pin_buff, 6);//original pin value
	pin.pin_len = 6;
	pin_buff[0] =  0x01;
	pin_buff[1] =  0x02;
	pin_buff[2] =  0x03;
	pin_buff[3] =  0x04;
	pin_buff[4] =  0x05;
	pin_buff[5] =  0x06;
	pin_buff_len = 6;
	ret = api_change_pin (&pin, pin_buff, pin_buff_len) ;
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_change_pin!\n");
		return ret;
	}

	
	 /****************************************reload user pin test*****************************************/		
	
	memcpy (pin.pin_value, admin_pin_buff, 16);//admin pin value
	pin.pin_len = 16;
	pin_buff[0] =  0x11;	
	pin_buff[1] =  0x22;
	pin_buff[2] =  0x33;
	pin_buff[3] =  0x44;
	pin_buff[4] =  0x55;
	pin_buff[5] =  0x66;
	pin_buff_len = 6;
	ret = api_reload_pin (&pin, pin_buff, pin_buff_len) ;
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_reload_pin!\n");
		return ret;
	}

	/****************************************verify pin test*****************************************/	   
	 pin.owner = USER_PIN;
	 pin.pin_len = 0x06;
	 memcpy(pin.pin_value, pin_buff,pin.pin_len);
	 ret = api_verify_pin(&pin);//verify pin
	 if(ret!=SE_SUCCESS)
	 { 	  
	   	  LOGE("failed to api_verify_pin\n");
	   	  return ret;
	 }
	 return SE_SUCCESS;

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


