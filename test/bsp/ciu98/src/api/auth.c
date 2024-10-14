/**@file  auth.c
* @brief  auth interface definition
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "auth.h"


/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */


/** @defgroup AUTH AUTH
  * @brief auth interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup AUTH_Exported_Functions AUTH Exported Functions
  * @{
  */


/**
* @brief Device authentication application
* @param [in] in_buf      application data
* @param [in] in_buf_len  application data length
* @return refer error.h
* @note no
* @see apdu_ext_auth
*/
se_error_t api_ext_auth (const uint8_t *in_buf, uint32_t in_buf_len)

{
	se_error_t ret = 0;
	//parameters check
	if(in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_ext_auth input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call apdu_ext_auth
	ret = apdu_ext_auth(in_buf,in_buf_len, 0x00);//kid is 0x00 
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_ext_auth!\n");
		return ret;
	}

	return SE_SUCCESS;

}


/**
* @brief external auth
* @param [in] in_buf      auth data
* @param [in] in_buf_len  auth data length
* @return rrfer error.h
* @note no
* @see apdu_ext_auth
*/
se_error_t api_pair_auth (const uint8_t *in_buf, uint32_t in_buf_len)

{
	se_error_t ret = 0;
//parameters check
	if(in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_ext_auth input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call apdu_ext_auth
	ret = apdu_ext_auth(in_buf,in_buf_len,0x01);//the kid is 0x01
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call api_pair_auth!\n");
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
 
