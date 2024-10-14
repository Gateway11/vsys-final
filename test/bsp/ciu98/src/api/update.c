/**@file  update.c
* @brief  update interface definition
* @author  ade
* @date  2020-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "comm.h"
#include "update.h"
#include "info.h"
#include "info.h"

#define UPDATE_COS 0x00
#define UPDATE_PATCH 0x01

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */

/** @defgroup UPDATE UPDATE
  * @brief update interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup UPDATE_Exported_Functions UPDATE Exported Functions
  * @{
  */



/**
* @brief Program update
* @param [in] image_addr image data address
* @param [in] image_len image data length
* @return refer error.h
* @note no
* @see apdu_get_info  apdu_enter_loader  api_reset  apdu_loader_init  apdu_loader_init  apdu_loader_checkdata
*/
se_error_t api_loader_download(uint8_t *image_addr, uint32_t image_len)
{
	se_error_t ret = SE_SUCCESS;
	uint8_t atr[32] = {0};
	uint32_t atr_len = 0;
	uint32_t download_len = 0;
	uint32_t addr_off = 0;
	uint8_t cos_info_version[6] = {0};
	uint32_t cos_info_version_len = 0;
	uint8_t loader_version[4] = {0};
	uint32_t loader_version_len = 0;
	uint8_t loader_FEK_info[8] = {0};
	uint32_t loader_FEK_info_len = 0;
	uint8_t loader_FVK_info[8] = {0};
	uint32_t loader_FVK_info_len = 0;
	uint8_t tmp_len[2] = {0};  
	uint8_t update_type = 0;
	se_info_t info;
	uint8_t cos_info_version_se[6] = {0};
	uint32_t cos_info_version_len_se = 0;
	uint8_t outbuf[32] = {0};
	uint32_t outlen = 0;
	uint16_t patch_version_num_se = 0;
	uint16_t patch_version_num = 0;
	

	//check parameter
	if(image_addr==NULL||image_len==0)
	{  
		LOGE("failed to api_loader_download input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//reset SE to obtain the ATR for anakysis
	ret = api_reset(atr, &atr_len);
	if(ret != SE_SUCCESS)
	{
		LOGE("failed to api_reset!\n");
		return ret;
	}

	// obtain the COS version info, Loader version, FEK version and FVK version in the image file.
	memcpy(tmp_len, image_addr, 2);
	cos_info_version_len = tmp_len[1];
	memcpy(cos_info_version, image_addr+2, cos_info_version_len);
	addr_off = 2 + cos_info_version_len; 
	if((cos_info_version_len != 4) && (cos_info_version_len != 2))
	{
		LOGE("the cos_info_version_len is wrong!\n");
		return SE_ERR_PARAM_INVALID;
	}

	memcpy(tmp_len, image_addr+addr_off, 2);
	loader_version_len = tmp_len[1];
	memcpy(loader_version, image_addr+addr_off+2, loader_version_len);
	addr_off = addr_off + 2 + loader_version_len;
	if(loader_version_len != 4)
	{
		LOGE("the loader_version_len is wrong!\n");
		return SE_ERR_PARAM_INVALID;
	}

	memcpy(tmp_len, image_addr+addr_off, 2);
	loader_FEK_info_len = tmp_len[1];
	memcpy(loader_FEK_info, image_addr+addr_off+2, loader_FEK_info_len);
	addr_off = addr_off + 2 + loader_FEK_info_len;
	if(loader_FEK_info_len != 8)
	{
		LOGE("the loader_FEK_info_len is wrong!\n");
		return SE_ERR_PARAM_INVALID;
	}

	memcpy(tmp_len, image_addr+addr_off, 2);
	loader_FVK_info_len = tmp_len[1];
	memcpy(loader_FVK_info, image_addr+addr_off+2, loader_FVK_info_len);
	addr_off = addr_off + 2 + loader_FVK_info_len;
	if(loader_FVK_info_len != 8)
	{
		LOGE("the loader_FVK_info_len is wrong!\n");
		return SE_ERR_PARAM_INVALID;
	}	

	if(cos_info_version_len == 4)
	{
		//check the ATR COS valid identification bit 
		if(atr[9]&0x20)
		{
			LOGE("the COS status is abnormal!\n");
			return SE_ERR_PARAM_INVALID;
		}

		//obtain 6 bytes COS info for SE 
		ret =  api_get_info (PRODUCT_INFO, &info);
		if ( ret != SE_SUCCESS)
		{
			LOGE("failed to api_get_info!\n");
			return ret;
		}

		memcpy(cos_info_version_se,info.product_info,4);

		//change the update type to UPDATE_PATCH
		update_type = UPDATE_PATCH;

		//compare the first 4 bytes
  		if(memcmp(cos_info_version_se, cos_info_version, 2)!=0 )
		{
			LOGE("the COS version is wrong!\n");
			return SE_ERR_PARAM_INVALID;
		}

		//check the patch version of image file is whether higher than the version of SE
		patch_version_num = (cos_info_version[2] << 8) + cos_info_version[3];
		patch_version_num_se = (cos_info_version_se[2] << 8) + cos_info_version_se[3];
		if(patch_version_num <= patch_version_num_se)
		{
			LOGE("the image patch version is low!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else if (cos_info_version_len == 2)
	{
		//obtain the 4 bytes COS info
		ret =  api_get_info (PRODUCT_INFO, &info);
		if ( ret != SE_SUCCESS)
		{
			LOGE("failed to api_get_info!\n");
			return ret;
		}
		memcpy(cos_info_version_se,info.product_info,2);

		//change the update type to UPDATE_COS
		update_type = UPDATE_COS;

		//compare the first 1.5 bytes
		if(cos_info_version_se[0] != cos_info_version[0])
		{
			LOGE("the image COS version is wrong!\n");
			return SE_ERR_PARAM_INVALID;
		}
		else if((cos_info_version_se[1] >> 4) != (cos_info_version[1] >> 4))
		{
			LOGE("the image COS version is wrong!\n");
			return SE_ERR_PARAM_INVALID;
		}
	
		//check the COS version of image file is whether higher than the version of SE
		if(cos_info_version[1] <= cos_info_version_se[1])
		{
			LOGE("the image COS version is low!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else
	{
			LOGE("the image file is error!\n");
			return SE_ERR_PARAM_INVALID;
	}

	//call api_get_info to obtain the loader version
	ret =  api_get_info (LOADER_VERSION, &info);
	if(ret != SE_SUCCESS)
	{
		LOGE("failed to api_get_info!\n");
		return ret;
	}

	//compare the loader version
	if(memcmp(info.loader_version, loader_version, loader_version_len)!=0 )
	{
		LOGE("the loader_version is wrong!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//call api_get_info to obtain the FEK info
	ret =  api_get_info (LOADER_FEK_INFO, &info);
	if(ret != SE_SUCCESS)
	{
		LOGE("failed to api_get_info!\n");
		return ret;
	}

	//compare the FEK info
	if(memcmp(info.loader_FEK_info, loader_FEK_info, loader_FEK_info_len)!=0 )
	{
		LOGE("the oader_FEK_info is wrong!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//call api_get_info to obtain the FVK info
	ret =  api_get_info (LOADER_FVK_INFO, &info);
	if(ret != SE_SUCCESS)
	{
		LOGE("failed to api_get_info!\n");
		return ret;
	}

	//compare the FEK info
	if(memcmp(info.loader_FVK_info, loader_FVK_info, loader_FVK_info_len)!=0 )
	{
		LOGE("the loader_FVK_info is wrong!\n");
		return SE_ERR_PARAM_INVALID;
	}	

	//check the data is whether effective
	download_len = (uint16_t)(image_addr[addr_off]<<8 | image_addr[addr_off+1]) + 2 + addr_off;
	if(download_len > image_len)
	{
		LOGE("failed to api_loader_download image length!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//download init
	ret = apdu_loader_init(image_addr + addr_off);
	if(ret != SE_SUCCESS)
	{
		port_printf("failed to apdu_loader_init!\n");
		return ret;
	}
	addr_off = download_len ;

	do
	{
		//check the data is whether effective
		download_len = download_len + (uint16_t)(image_addr[addr_off]<<8 | image_addr[addr_off+1]) + 2;
		if(download_len > image_len)
		{
			LOGE("failed to api_loader_download image length!\n");
			return SE_ERR_PARAM_INVALID;
		}

		//obtain the CHECK PROGRAM command from the image
		if(((uint16_t)(image_addr[addr_off]<<8 | image_addr[addr_off+1])==0x0065 ) || ((uint16_t)(image_addr[addr_off]<<8 | image_addr[addr_off+1])==0x0025 ))
		{
			//send the  CHECK PROGRAM command 
			ret = apdu_loader_checkprogram(image_addr+addr_off);
			if(ret != SE_SUCCESS)
			{
				port_printf("failed to apdu_loader_download!\n");
				return ret;
			}
			
			break;  //download over
		}
		
		//download image data
		ret = apdu_loader_program(image_addr+addr_off);
		if(ret != SE_SUCCESS)
		{
			port_printf("failed to apdu_loader_download!\n");
			return ret;
		}
		addr_off = download_len ;

	}while(1);

    //delay 1 second to ensure the updating succesful
    ret =  api_delay (1000000);//ÑÓ³Ù1Ãë
	if(ret!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_connect\n");
		  return ret;
	 } 	
	 
	//check the updating is whether successful
	//obtain the ATR to judge that the COS is whether effective. 
	ret = api_reset(atr, &atr_len);
	if(ret != SE_SUCCESS)
	{
		LOGE("failed to api_reset!\n");
		return ret;
	}

	//judge the update type is whether UPDATE_PATCH
	if(update_type == UPDATE_PATCH)
	{
		//judge the product status of patch is whether effective
		if(!(atr[9]&0x10))
		{
			LOGE("failed to patch valid!\n");
			return SE_ERR_UPDATE;
		}

		//obtain 6 bytes COS info
		ret =  api_get_info (PRODUCT_INFO, &info);
		if(ret != SE_SUCCESS)
		{
			LOGE("failed to api_get_info!\n");
			return ret;
		}

		memcpy(cos_info_version_se,info.product_info,4);
		cos_info_version_len_se = 4;
	}
	else if (update_type == UPDATE_COS)//judge the update type is whether UPDATE_COS
	{
		//judge the product status of COS is whether effective
		if(atr[9]&0x20)
		{
			LOGE("failed to cos valid!\n");
			return SE_ERR_UPDATE;
		}

		//obtain 4 bytes COS info
		ret =  api_get_info (PRODUCT_INFO, &info);
		if ( ret != SE_SUCCESS)
		{
			LOGE("failed to api_get_info!\n");
			return ret;
		}
		memcpy(cos_info_version_se,info.product_info,2);
		cos_info_version_len_se = 2;
	}
	
	//compare the COS info in the image is whether equal to the COS info in the SE
	if(memcmp(cos_info_version_se, cos_info_version, cos_info_version_len_se)!=0 )
	{
		LOGE("the upadted SE COS version is wrong!\n");
		return SE_ERR_UPDATE;
	}
		
	return ret;
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


