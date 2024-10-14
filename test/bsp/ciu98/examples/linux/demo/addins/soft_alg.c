/**@file  soft_alg.c
* @brief  soft_alg interface declearation	 
* @author liuch
* @date  2020-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "soft_alg.h"

/** @addtogroup SE_APP_TEST
  * @{
  */


/** @addtogroup ADDINS
  * @{
  */


/** @defgroup SOFT_ALG SOFT_ALG
  * @brief soft_alg interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup SOFT_ALG_Exported_Functions SOFT_ALG Exported Functions
  * @{
  */


/**
* @brief sm4�ӽ������㷨
* @param [in] in_buf  ��������
* @param [in] in_buf_len  �������ݳ���
* @param [in] key_buf  ��Կ
* @param [in] key_buf_len   ��Կ����
* @param [in] ecb_cbc_mode  �ӽ�ģʽ
* @param [out] out_buf  �������
* @param [out] out_buf_len  ������ݳ���
* @return ��soft_alg.h
* @note no
* @see ����sm4_crypt_ecb sm4_crypt_cbc
*/
uint32_t ex_sm4_enc_dec(uint8_t* in_buf,uint32_t in_buf_len,uint8_t* key_buf,uint32_t key_buf_len,uint32_t ecb_cbc_mode,uint32_t enc_dec_mode,uint8_t* out_buf,uint32_t*out_buf_len)
{

	//uint32_t usRet = 0x9000;
	uint8_t iv[16]={0x00};
	switch (enc_dec_mode){
		case ENCRYPT://encrypt
			{
				if(ecb_cbc_mode==ECB)
				{
					sm4_crypt_ecb(key_buf,1,in_buf_len,in_buf,out_buf);
					*out_buf_len = in_buf_len;
				}
				else
				{
					memcpy(iv,in_buf,16);
					sm4_crypt_cbc(key_buf,1,in_buf_len-16,iv,in_buf+16,out_buf);
					*out_buf_len = in_buf_len-16;
				}
				break;
			}
		case DECRYPT://decrypt
			{
				if(ecb_cbc_mode==ECB)
				{
					sm4_crypt_ecb( key_buf,0,in_buf_len,in_buf,out_buf);
					*out_buf_len = in_buf_len;
				}
				else
				{
					memcpy(iv,in_buf,16);
					sm4_crypt_cbc( key_buf,0,in_buf_len-16,iv,in_buf+16,out_buf);
					*out_buf_len = in_buf_len-16;
				}
				break;
			}

	}



	return *out_buf_len;
}


/**
* @brief SM4 MAC���㷨
* @param [in] in_buf  ��������
* @param [in] in_buf_len  �������ݳ���
* @param [in] iv  ��������
* @param [in] iv_len  �������ݳ���
* @param [in] key_buf  ��Կ
* @param [in] key_buf_len   ��Կ����
* @param [out] out_buf  �������
* @param [out] out_buf_len  ������ݳ���
* @return ��soft_alg.h
* @note no
* @see ����sm4_setkey_enc sm4_crypt_cbc
*/
uint32_t ex_sm4_mac(uint8_t* in_buf,uint32_t in_buf_len,uint8_t* iv,uint32_t iv_len,uint8_t* key_buf,uint32_t key_buf_len,uint8_t* out_buf,uint32_t*out_buf_len)
{

	uint8_t OutData[BUFFER_MAX_LEN] = {0};

	if ((in_buf == NULL) || (iv == NULL))
		return 0xFFFFFFFF;

	sm4_crypt_cbc( key_buf,1,in_buf_len,iv,in_buf,OutData);
	memcpy(out_buf, OutData+in_buf_len-key_buf_len, key_buf_len);

	*out_buf_len = key_buf_len;

	return *out_buf_len;
}

/**
* @brief SM3���㷨
* @param [in] in_buf  ��������
* @param [in] in_buf_len  �������ݳ���
* @param [out] out_buf  �������
* @return ��soft_alg.h
* @note no
* @see ����sm3
*/
uint32_t ex_SM3_digest( uint8_t *in_buf, uint32_t in_buf_len,uint8_t *output)
{
    if ((in_buf == NULL) ||(in_buf_len==0)|| (output == NULL))
		return 0xFFFFFFFF;
	sm3( in_buf, in_buf_len,output);
	return EXCUTE_SUC;
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
	