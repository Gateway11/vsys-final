/**@file  fs.c
* @brief  fs interface definition
* @author liangww
* @date  2021-05-12
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/

#include "fs.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */

/** @defgroup FS FS
  * @brief fs interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup FS_Exported_Functions FS Exported Functions
  * @{
  */


/**
* @brief Select file
* @param [in] fid	    file FID
* @return refer error.h
* @note no
* @see  apdu_select_file
*/
se_error_t  api_select_file(uint32_t fid)
{
	se_error_t ret = 0;
	//call the apdu_select_file interface
	ret = apdu_select_file(fid);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_select_file!\n");
		return ret;
	}

	return SE_SUCCESS;

}
/**
* @brief Write file
* @param [in] offset  		offset information
* @param [in] in_buf  		input data
* @param [in] in_buf_len  	input data length
* @param [in] if_encrypt  	if encrypt the input data by transport key.
* @return refer error.h
* @note no
* @see apdu_write_file
*/
se_error_t api_write_file (uint32_t offset, const uint8_t *in_buf, uint32_t in_buf_len, bool if_encrypt)
{
	se_error_t ret = 0;
	//parameters check
	if(in_buf==NULL)
	{  
		LOGE("failed to api_write_file input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(in_buf_len>(DBLOCK+16))
	{  
		LOGE("failed to api_write_file input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call the apdu_write_file interface
	ret = apdu_write_file (offset, if_encrypt, in_buf, in_buf_len);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_write_file!\n");
		return ret;
	}

	return SE_SUCCESS;
}

/**
* @brief Read file
* @param [in] offset   		Offset information
* @param [in] expect_len  	expect length
* @param [in] if_encrypt  	if encrypt the input data by transport key.
* @param [out] out_buf  	output data
* @param [out] out_buf_len  output data length
* @return refer error.h
* @note no
* @see apdu_read_file
*/
se_error_t api_read_file  (uint32_t offset, uint32_t expect_len , bool if_encrypt,uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;
	//parameters check
	if(expect_len>(LE_MAX_LEN-16))
	{  
		LOGE("failed to api_read_file input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	//call the apdu_read_file interface
	ret = apdu_read_file  (offset, if_encrypt, expect_len, out_buf, out_buf_len);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_read_file!\n");
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

