/**@file  info.c
* @brief  info interface definition
* @author liangww
* @date  2023-12-12
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/

#include "info.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */

/** @defgroup INFO INFO
  * @brief info interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup INFO_Exported_Functions INFO Exported Functions
  * @{
  */


/**
* @brief Get SE information
* @param [in] type					CHIP_ID/PRODUCT_INFO
* @param [out] info->CHIP_ID 		chip 8-byte unique serial number
* @param [out] info->PRODUCT_INFO   product information 8 bytes
* @return  refer error.h
* @note no
* @see apdu_get_info
*/
se_error_t  api_get_info (info_type type, se_info_t * info)
{
	se_error_t ret = 0;
	//parameters check
	if(info==NULL)
	{  
		LOGE("failed to api_get_info pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(type!=CHIP_ID&&type!=PRODUCT_INFO&&type!=LOADER_VERSION&&type!=LOADER_FEK_INFO&&type!=LOADER_FVK_INFO)
	{  
		LOGE("failed to api_get_info input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//call the apdu_get_info interface
	ret = apdu_get_info (type, info);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_get_info!\n");
		return ret;
	}

	return SE_SUCCESS;

}

/**
* @brief Get SE IVD
* @param [in] ivd_type  		ivd type : 1.Get preset CRC32 of COS; 2.Make SE Calculate the CRC32 of COS and output
* @param [out] IVD_value        IVD value : 4 bytes CRC32 | 4 bytes COS version 
* @return refer error.h
* @note no
* @see apdu_get_IVD
*/
se_error_t  api_get_IVD (IVD_type ivd_type ,uint8_t *IVD_value)
{
	se_error_t ret = 0;
	se_info_t info ={0};
	
	//parameters check
	if(IVD_value==NULL)
	{  
		LOGE("failed to api_get_IVD pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(ivd_type!=PRE_IVD&&ivd_type!=CAL_IVD)
	{  
		LOGE("failed to api_get_IVD input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	//call the apdu_get_IVD interface
	ret = apdu_get_IVD (ivd_type, IVD_value);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_get_IVD!\n");
		return ret;
	}

		
	//call the apdu_get_info interface
	ret = apdu_get_info (COS_VERSION, &info);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_get_info!\n");
		return ret;
	}
	memcpy(IVD_value + 4, info.cos_version, 4);

	return SE_SUCCESS;

}

/**
* @brief Get SE ID
* @param [out] se_id->val  SE ID
* @param [out] se_id->val_len  SE ID length
* @return refer error.h
* @note no
* @see apdu_get_id
*/
se_error_t  api_get_id (se_id_t *se_id ) 
{
	se_error_t ret = 0;
	//parameters check
	if(se_id==NULL)
	{  
		LOGE("failed to apdu_get_id pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call the apdu_get_id interface
	ret = apdu_get_id (se_id ) ;
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_get_id!\n");
		return ret;
	}
	return SE_SUCCESS;
}


/**
* @brief Write SE ID
* @param [in] se_id->val  SEID
* @param [in] se_id->val_len  SEID length
* @return refer error.h
* @note obtain the device authentication before writing the SEID
* @see apdu_write_id
*/
se_error_t  api_write_id (se_id_t *se_id)
{
	se_error_t ret = 0;

	//parameters check
	if((se_id == NULL)||(se_id->val_len  == 0))
	{
		LOGE("failed to api_write_id params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	//call the apdu_write_SEID interface
	ret = apdu_write_SEID (se_id);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_write_SEID!\n");
		return ret;
	}

	return SE_SUCCESS;

}

/**
* @brief  Get the SDK 4-byte integer version number
* @param [out] num integer version number
* @return refer error.h
* @note no
* @see no
*/
se_error_t  api_sdk_version_number(uint32_t *num)
{

	//parameters check
	if(num==NULL)
	{  
		LOGE("failed to api_sdk_version_number pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	*num = SDK_VERSION_NUM;

	return SE_SUCCESS;
}

/**
* @brief Get the SDK string version number
* @param [out] str      string version number
* @return refer error.h
* @note no
* @see no
*/
se_error_t  api_sdk_version_string (char *str)
{

	//parameters check
	if(str==NULL)
	{  
		LOGE("failed to api_sdk_version_string pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	strcpy(str,SDK_VERSION_STRING);

	return SE_SUCCESS;
}
/**
* @brief update EM_flag
* @param [in] pub_kid      ED25519 pubkey KID
* @param [in] sig  ED25519 signature value
* @param [in] sig_len      ED25519 signature value length, 64 bytes
* @param [in] random  random
* @param [in] random_len      random length, 16bytes
* @param [in] EM_DID  2 bytes DID
* @return refer error.h
* @note no
* @see apdu_manage_EM_flag
*/
se_error_t  api_update_EM_flag (uint8_t pub_kid,  uint8_t *sig, uint32_t sig_len, uint8_t *random, uint32_t random_len, uint16_t EM_DID)
{
	se_error_t ret = 0;
	uint8_t inbuf[82] = {0};
	uint32_t inbuf_len = 0;
	bool if_updata = true;
	uint8_t EM_DID_internal[2] = {0};

	//parameters check
	if(pub_kid > 0xff)
	{  
		LOGE("failed to api_update_EM_flag params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if((sig == NULL) || (random == NULL))
	{  
		LOGE("failed to api_update_EM_flag params!\n");
		return SE_ERR_PARAM_INVALID;
	}	

	if((sig_len != 64) || (random_len != 16))
	{  
		LOGE("failed to api_update_EM_flag params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if((EM_DID != 0x2055) && (EM_DID != 0x2053) && (EM_DID != 0x2054) && (EM_DID != 0x2056))
	{
		LOGE("failed to api_update_EM_flag params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//package inbuf:random(16byttes)+EM_DID(2bytes)+ ED25519 signature value(64bytes)
	memcpy(inbuf, random, random_len);
	EM_DID_internal[0] = (EM_DID >> 8) & 0xff;
	EM_DID_internal[1] = EM_DID & 0xff;
	memcpy(inbuf+random_len, EM_DID_internal, 2);
	memcpy(inbuf+random_len+2, sig, sig_len);
	inbuf_len = random_len + 2 + sig_len;
	
	//call apdu_manage_EM_flag
	ret = apdu_manage_EM_flag (pub_kid, inbuf, inbuf_len, if_updata, NULL) ;
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call api_update_EM_flag!\n");
		return ret;
	}
	return SE_SUCCESS;
}

/**
* @brief read EM_flag
* @param [out] EM_flag  1 byte EM_flag
* @return refer error.h
* @note no
* @see apdu_manage_EM_flag
*/
se_error_t  api_read_EM_flag (uint8_t *EM_flag)
{
	se_error_t ret = 0;
	bool if_updata = false;
	
	//parameters check
	if(EM_flag == NULL)
	{  
		LOGE("failed to api_read_EM_flag params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	//call apdu_manage_EM_flag
	ret = apdu_manage_EM_flag (0, NULL, 0, if_updata, EM_flag) ;
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call api_update_EM_flag!\n");
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
