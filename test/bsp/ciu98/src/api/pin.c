/**@file  pin.c
* @brief  pin interface definition
* @author liangww
* @date  2021-05-11
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/

#include "pin.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */

/** @defgroup PIN PIN
  * @brief pin interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup PIN_Exported_Functions PIN Exported Functions
  * @{
  */


/**
* @brief Write pin
* @param [in] pin->owner		PIN owner type
* @param [in] pin->pin_value[16]  PIN value
* @param [in] pin->pin_len  PIN length:0x06-0x10 
* @param [in] if_encrypt  whether to encrypt the PIN by the transport key
* @return refer error.h
* @note no
* @see apdu_write_key
*/ 
se_error_t  api_write_pin(pin_t *pin, bool if_encrypt)
{
	se_error_t ret = 0;
	uint8_t inbuf[24]={0};
	uint32_t inbuf_len = 0;
	
    //parameters check
	if(pin==NULL)
	{  
		LOGE("failed to api_write_pin input pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(pin->owner!=ADMIN_PIN&&pin->owner!=USER_PIN)
		{  
			LOGE("failed to api_write_pin pin owner params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	if(pin->pin_value==NULL)
		{  
			LOGE("failed to api_write_pin pin_value params!\n");
			return SE_ERR_PARAM_INVALID;
		}
   if(pin->pin_len<6||pin->pin_len>16)
		{  
			LOGE("failed to api_write_pin pin_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}
  inbuf_len = pin->pin_len+8;  
	//key information
	inbuf[0]=0x00;//key usage 
	if(pin->owner==ADMIN_PIN)//kid(admin pin:'0x00';user pin:'0x01')
	{
		inbuf[1]=0x00;
	}
	else if(pin->owner==USER_PIN)
	{
		inbuf[1]=0x01;
	}
  inbuf[2] = pin->limit; //PIN limit
	inbuf[6]=0x00;//length:high byte
	inbuf[7]=pin->pin_len;//length:low byte
	memcpy(inbuf+8,pin->pin_value,pin->pin_len);
	//call apdu_write_key
	ret = apdu_write_key(inbuf,inbuf_len,NULL,if_encrypt,true,false,false);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_write_key!\n");
		return ret;
	}

	return SE_SUCCESS;

}
/**
* @brief Change the admin pin or user pin
* @param [in] pin->owner  	  PIN owner type
* @param [in] pin->pin_value  original PIN value
* @param [in] pin->pin_len  PIN length:0x06-0x10 
* @param [in] in_buf   		new PIN value
* @param [in] in_buf_len  	new PIN length 
* @return refer error.h
* @note no
* @see apdu_change_reload_pin
*/ 
se_error_t api_change_pin (pin_t *pin, const uint8_t *in_buf, uint32_t in_buf_len)
{
	se_error_t ret = 0;
	//parameters check
	if(pin==NULL||in_buf==NULL)
	{  
		LOGE("failed to api_change_pin input pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(pin->owner!=ADMIN_PIN&&pin->owner!=USER_PIN)
	{  
		LOGE("failed to api_change_pin pin owner params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(pin->pin_value==NULL)
		{  
			LOGE("failed to api_change_pin pin_value params!\n");
			return SE_ERR_PARAM_INVALID;
		}
    if(pin->pin_len<6||pin->pin_len>16||in_buf_len<6||in_buf_len>16)
		{  
			LOGE("failed to api_change_pin pin_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}

	//call apdu_change_reload_pin
	ret = apdu_change_reload_pin (pin, in_buf, in_buf_len, true);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_change_reload_pin!\n");
		return ret;
	}

	return SE_SUCCESS;
}

/**
* @brief Reload the pin
* @param [in] pin->pin_value  admin PIN value
* @param [in] pin->pin_len   admin PIN length:0x06-0x10 
* @param [in] in_buf         new user PIN value
* @param [in] in_buf_len     new user PIN length
* @return refer error.h
* @note no
* @see apdu_change_reload_pin
*/
se_error_t api_reload_pin  (pin_t *pin, const uint8_t *in_buf, uint32_t in_buf_len)
{
	se_error_t ret = 0;
	//parameters check
	if(pin==NULL||in_buf==NULL)
	{  
		LOGE("failed to api_reload_pin input pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(pin->pin_value==NULL)
		{  
			LOGE("failed to api_reload_pin pin_value params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	if(pin->pin_len<6||pin->pin_len>16||in_buf_len<6||in_buf_len>16)
		{  
			LOGE("failed to api_reload_pin pin_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	//call apdu_change_reload_pin
	ret = apdu_change_reload_pin (pin, in_buf, in_buf_len, false);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_change_reload_pin!\n");
		return ret;
	}

	return SE_SUCCESS;

}


/**
* @brief Verify PIN 
* @param [in] pin->owner   PIN owner type
* @param [in] pin->pin_value  PIN value
* @param [in] pin->pin_len   PIN length:0x06-0x10 
* @return refer error.h
* @note no
* @see apdu_verify_pin
*/
se_error_t api_verify_pin  (pin_t *pin)
{
	se_error_t ret = 0;
	//parameters check
	if(pin==NULL)
	{  
		LOGE("failed to api_verify_pin input pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(pin->pin_value==NULL)
		{  
			LOGE("failed to api_verify_pin pin_value params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	if(pin->pin_len<6||pin->pin_len>16)
		{  
			LOGE("failed to api_verify_pin pin_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	//call apdu_verify_pin 
	ret = apdu_verify_pin (pin);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_verify_pin!\n");
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

/**
  * @}
  */

