/**@file  apdu.c
* @brief  apdu interface definition
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include <string.h>
#include "apdu.h"
#include "sm4.h"
#include "sm3.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup CMD
  * @brief Command layer.
  * @{
  */

/** @defgroup APDU APDU
  * @brief apdu command pack , unpack.
  * @{
  */



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup APDU_Exported_Functions APDU Exported Functions
  * @{
  */

/**
* @brief use mast key to encrypt
* @param [in] input  input data
* @param [in] input_len   input data length
* @param [in] mkey->value  device mast key
* @param [in] mkey->value_len  device mast key length
* @param [in] off offset      
* @param [in] in_queue  input deque
* @return rfer error.h
* @note no
* @see   util_queue_rear_push 
*/
se_error_t apdu_mkey_encrypt	(uint8_t *input, uint32_t input_len,sym_key_t * mkey,uint32_t off,double_queue in_queue)
{
	
	uint8_t len_high=0x00;
	uint8_t len_low =0x00;
	uint8_t pading[16]={0}; 
	
	//store the data encrypted into the deque
	len_high=input_len>>8;
	util_queue_rear_push(&len_high,1, in_queue);
	len_low=input_len&0xff;
	util_queue_rear_push(&len_low,1, in_queue);
	
	util_queue_rear_push((uint8_t *)input,input_len, in_queue);
	//whether to add '0x80' at the end of data
	if((in_queue->q_buf_len-off)% 16!=0)
	{   //add '0x80' at the end of data
		pading[0] = 0x80;
		util_queue_rear_push(pading,1, in_queue);
		//whether to add '0x00' at the end of data
		if((in_queue->q_buf_len-off) % 16!=0)
		{
		  memset(pading,0x00,16);
		  util_queue_rear_push(pading,16-(in_queue->q_buf_len-off)%16, in_queue);
		}			
	}
//use the SM4 transport key to encrypt the data
	sm4_crypt_ecb(mkey->val,SM4_ENCRYPT,in_queue->q_buf_len-off, (unsigned char *) &(in_queue->q_buf[in_queue->front_node+off]), (unsigned char *) &(in_queue->q_buf[in_queue->front_node+off])); 

	return SE_SUCCESS;
}

/**
* @brief Encrypt the data by transport key
* @param [in] input  	  data
* @param [in] input_len   data length
* @param [in] off 		the offset of data used to be encrypted in the deque      
* @param [in] in_queue  the address of the deque
* @return refer error.h
* @note no
* @see util_queue_rear_push 
*/
se_error_t apdu_trans_encrypt	(uint8_t *input, uint32_t input_len,uint32_t off,double_queue in_queue)
{
	
	uint8_t len_high=0x00;
	uint8_t len_low =0x00;
	uint8_t pading[16]={0}; 
	
	//store the data encrypted into the deque 
	len_high=input_len>>8;
	util_queue_rear_push(&len_high,1, in_queue);
	len_low=input_len&0xff;
	util_queue_rear_push(&len_low,1, in_queue);
	
	util_queue_rear_push((uint8_t *)input,input_len, in_queue);
	//whether to add '0x80' at the end of data
	if((in_queue->q_buf_len-off)% 16!=0)
	{   //add '0x80' at the end of data
		pading[0] = 0x80;
		util_queue_rear_push(pading,1, in_queue);
		//whether to add '0x00' at the end of data
		if((in_queue->q_buf_len-off) % 16!=0)
		{
		  memset(pading,0x00,16);
		  util_queue_rear_push(pading,16-(in_queue->q_buf_len-off)%16, in_queue);
		}			
	}
	//use the SM4 transport key to encrypt the data
	sm4_crypt_ecb(trans_key,SM4_ENCRYPT,in_queue->q_buf_len-off, (unsigned char *) &(in_queue->q_buf[in_queue->front_node+off]), (unsigned char *) &(in_queue->q_buf[in_queue->front_node+off])); 

	return SE_SUCCESS;
}


/**
* @brief Decrypt the cipher data by transport key     
* @param [in] in_queue  the address of the deque
* @return refer error.h
* @note no
* @see  util_queue_rear_push 
*/
se_error_t apdu_trans_decrypt	(double_queue in_queue)
{
	//use the SM4 transport key to decrypt the cipher data
	sm4_crypt_ecb(trans_key,SM4_DECRYPT,in_queue->q_buf_len, (unsigned char *) &(in_queue->q_buf[in_queue->front_node]), (unsigned char *) &(in_queue->q_buf[in_queue->front_node])); 

	return SE_SUCCESS;
}


/**
* @brief Pin encrypt 
* @param [in] pin->owner     pin owner type
* @param [in] pin->pin_value pin value
* @param [in] pin->pin_len   pin value length:0x06-0x10
* @param [in] in_buf 		 input data
* @param [in] in_buf_len 	 input data length
* @param [in] if_xor 		 whether to XOR
* @param [out] queue_out  	 the address of output deque
* @return refer error.h
* @note no
* @see util_queue_rear_push    sm3  tpdu_execute 
*/
se_error_t apdu_pin_encrypt	(pin_t *pin, const uint8_t *in_buf, uint32_t in_buf_len, double_queue queue_out, bool if_xor)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint8_t sm3_buf[32]={0}; 
	uint8_t pin_buf[32]={0}; 
	uint32_t i = 0;
	uint32_t j = 0;	
	uint32_t out_len = 0;
	double_queue_node queue_in ={0} ;
	util_queue_init(&queue_in);

	//send get_randdmo command 
	//set command header
	tpdu_init_with_id(&command,CMD_GET_RANDOM);
	//set le
	tpdu_set_le(&command, 0x10);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;
	
	//splice the 16 bytes random and pin value
	util_queue_rear_push(pin->pin_value,pin->pin_len, queue_out);
	
	//SM3 digest algorithm
	sm3(queue_out->q_buf+queue_out->front_node, queue_out->q_buf_len, sm3_buf);
    
	if(if_xor)
	{
		//add 2 bytes Ld before the new pin value
		pin_buf[1]=in_buf_len&0xff;
		//put the new pin into pin_buf
		memcpy(pin_buf+2,in_buf,in_buf_len);
		in_buf_len = in_buf_len + 2;	

		//whether to add '0x80' at the end of data
		if((in_buf_len)% 16!=0)
		{   //add '0x80' at the end of data
			pin_buf[in_buf_len] = 0x80;
			in_buf_len = in_buf_len + 1;
			//whether to add '0x00' at the end of data
			if(in_buf_len % 16!=0)
			{
			  if(in_buf_len<16)
			  	in_buf_len = 16;
			  else if(16<in_buf_len)
			  	in_buf_len = 32;
			}			
		}
		
		//XOR the previous 16 bytes of the SM3 digest data and new pin value
		for( j = 0;j < in_buf_len/16; j++)
		{
			for( i = 0; i < 16; i++)
				pin_buf[i+j*16] = sm3_buf[i] ^ pin_buf[i+j*16];
		}

		//store the calculation into deque 
		util_queue_init(queue_out);
	    util_queue_rear_push(pin_buf,in_buf_len, queue_out);
	}
    else
    {
    	//store the calculation into deque 
		util_queue_init(queue_out);
	    util_queue_rear_push(sm3_buf,16, queue_out);
	    
	}
	return SE_SUCCESS;
}



/**
* @brief Symmetric Algorithm padding
* @param [in] padding      padding mode
* @param [in] input        source data
* @param [in] input_len    source data length
* @param [out] output      output data 
* @param [out] output_len  output data length
* @return refer error.h
* @note no
* @see  util_queue_rear_push 
*/
se_error_t apdu_sym_padding	(uint32_t padding, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len)
{
	uint32_t buffer_len = *output_len;
	uint32_t fill = 0;
	uint32_t end = 0;
	double_queue queue_out=(double_queue)input;
	uint8_t temp_buf[16] = {0x00};

	if(padding!=PADDING_NOPADDING&&padding!=PADDING_PKCS7)
	{
		return SE_ERR_PARAM_INVALID;
	}

	if (input == NULL || input_len==0 || output == NULL)
	{
		return SE_ERR_PARAM_INVALID;
	}


	switch (padding) {
	case PADDING_PKCS7:
		fill = buffer_len - input_len;
		end = input_len;
		break;
	case PADDING_NOPADDING:
		fill = 0;
		end = input_len;
	default:
		fill = 0;
		end = input_len;
		break;
	}

	memset(temp_buf, fill, buffer_len - end);
	util_queue_rear_push(temp_buf,buffer_len - input_len, queue_out);
	output = (uint8_t*)queue_out;
	*output_len = buffer_len;


	return SE_SUCCESS;
}



/**
* @brief Symmetric algorithm remove padding data
* @param [in] padding       padding mode
* @param [in] input         source data
* @param [in] input_len     source data length
* @param [out] output       output data
* @param [out] output_len   output data length
* @return refer error.h
* @note no
* @see  util_queue_rear_pop 
*/
se_error_t apdu_sym_unpadding(uint32_t padding, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len)
{
	uint32_t buffer_len = input_len;
	uint32_t offset = 0;
	double_queue queue_in=(double_queue)input;
	double_queue queue_out=(double_queue)input;
	uint32_t off = queue_in->rear_node;

	if (input == NULL || input_len <= 0 || output == NULL || output_len == NULL) {
		return SE_ERR_PARAM_INVALID;
	}

	switch (padding)
	{
		case PADDING_PKCS7:
			if((queue_in->q_buf[off-1] > 0x10)||(queue_in->q_buf[off-1] == 0x00))
			{
				return SE_ERR_PARAM_INVALID;
			}
			for(int i=0;i<queue_in->q_buf[off-1];i++)
			{
				if(queue_in->q_buf[off-1-i] != queue_in->q_buf[off-1])
				{
					return SE_ERR_PARAM_INVALID;
				}
			}
			buffer_len = input_len - queue_in->q_buf[off-1];
			offset = queue_in->q_buf[off-1];
			break;
		case PADDING_NOPADDING:
			buffer_len = input_len;
			offset = 0;
			break;
		default:
			buffer_len = input_len;
			offset = 0;
			break;
	}
    util_queue_rear_pop(offset, queue_out);
	output = (uint8_t*)queue_out;
	*output_len = buffer_len;

	return SE_SUCCESS;
}


/**
* @brief Get the length of the padding data
* @param [in] alg         algorithm type
* @param [in] padding     padding mode
* @param [in] input_len   source data length
* @return padding data length	
* @note no
* @see no
*/
uint32_t apdu_sym_padding_length(uint32_t alg, uint32_t padding, uint32_t input_len)
{
	uint32_t buffer_len = 0;
	uint32_t align_size = 0;
	//set the packet length
	if(alg==ALG_DES128)
		align_size = 8;
	else
		align_size = 16;

	switch (padding)
	{
		case PADDING_NOPADDING:
			buffer_len = input_len;
			break;
		case PADDING_PKCS7:
			if ((input_len == 0) || (input_len%align_size) == 0)
				buffer_len = input_len + 1;
			else
				buffer_len = input_len;
			break;
		default:
			buffer_len = input_len;
			break;
	}

	while ((buffer_len%align_size) != 0)
	{
		buffer_len++;
	}

	return buffer_len;
}



/**
* @brief Switch working mode command package
* @param [in] type     work mode
* @return refer error.h	
* @note no
* @see  util_queue_init  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute_no_response
*/
se_error_t apdu_switch_mode (work_mode  type)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	uint32_t p2 = 0;
	
	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_ENTER_LOWPOWER);
	//set P1P2
	p2 = 0x00;
	tpdu_set_p1p2 (&command,0x00,p2);

	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;	
	
	
	return SE_SUCCESS;
}


/**
* @brief chenge app config 
* @param [in] in_buf P1 | P2 | LC | DATA
* @return refer error.h	
* @note no
* @see   util_queue_init   tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute_no_response
*/
se_error_t apdu_config_system_info (uint8_t  *in_buf)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	uint32_t p1 = 0;
	uint32_t p2 = 0;
	
		//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_CONFIG_SYSTEM_INFO);
	//set p1 p2
	p1 = in_buf[0];
	p2 = in_buf[1];
	tpdu_set_p1p2 (&command,p1,p2);
	//input data is stored in deque
	 util_queue_rear_push(&in_buf[3],in_buf[2], &queue_in);
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;	

	return SE_SUCCESS;
}


/**
* @brief control
* @param [in] ctrlcode  control mode
* @param [in] in_buf inputdata
* @param [in] in_buf_len   inputdata length
* @param [out] out_buf  output data
* @param [out] out_buf_len  output data length
* @return refer error.h
* @note no
* @see no
*/
se_error_t apdu_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	return SE_SUCCESS;
}


/**
* @brief Device authentication command package 
* @param [in] in_buf  input data
* @param [in] in_buf_len   input data length
* @return refer error.h
* @note no
* @see no
*/
se_error_t apdu_ext_auth(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t ext_kid)
{

	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	//parameters check
	if(in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to apdu_ext_auth input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//deque initialization
	util_queue_init(&queue_in);
	//input data is stored in deque
    util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_EXTTERN_AUTH);
	tpdu_set_p1p2 (&command,0x00,ext_kid);
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;
}


/**
* @brief Write the key or pin command package 
* @param [in] in_buf  master key or key info
* @param [in] in_buf_len   input data length
* @param [in] if_encrypt  whether to encrypt
* @param [in] if_write  whether to write key 
* @param [in] if_update_mkey   whether to update master key
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_cla  tpdu_set_p1p2  tpdu_execute_no_response  util_queue_size
*/
se_error_t apdu_write_key(const uint8_t *in_buf, uint32_t in_buf_len,sym_key_t * mkey, bool if_encrypt, bool if_write, bool if_update_mkey, bool if_update_ekey )
{
  iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	//check parameters
	if(in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to apdu_write_key input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if((if_write==true&&if_update_mkey==true) || (if_write==true&&if_update_ekey==true) || (if_update_mkey==true && if_update_ekey==true)) 
	{  
		LOGE("failed to apdu_write_key bool params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_WRITE_KEY);
	//set P1 P2
	if(if_write==true)
		tpdu_set_p1p2 (&command,0x00,0x00);
	else if(if_update_mkey==true)
		tpdu_set_p1p2 (&command,0x01,0x00);
	else if(if_update_ekey==true)
		tpdu_set_p1p2 (&command,0x01,0x03);
	else
	    tpdu_set_p1p2 (&command,0x01,0x01);
	//whther to transport the data by cipher
	if(if_encrypt)
	{
		if((if_update_mkey == false) && (if_update_ekey == false))
		{
		ret =  apdu_trans_encrypt((uint8_t *)in_buf, in_buf_len,0,&queue_in);
		//set CLA
		tpdu_set_cla (&command,CMD_CLA_CASE4);
		}
		else
		{
			ret =  apdu_mkey_encrypt((uint8_t *)in_buf, in_buf_len,mkey,0,&queue_in);	  
			//set CLA
			tpdu_set_cla (&command,CMD_CLA_CASE4);
		}

	}
    else
	{
	    //input data is stored in deque
	    util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
	    //set CLA
	    tpdu_set_cla (&command,CMD_CLA_CASE3);
    }
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;

}




/**
* @brief Generate/update asymmetric public and private key pair command package
* @param [in] pub_key->alg  	 algorithm type  
* @param [in] pub_key->id  		 public key id
* @param [in] pri_key->id 		 private key id
* @param [out] pub_key->val 	 public key
* @param [out] pub_key->val_len  public key length
* @return refer error.h
* @note no
* @see  util_queue_init  util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute  util_queue_size
*/
se_error_t  apdu_generate_key(pub_key_t *pub_key,pri_key_t *pri_key, sym_key_t * symkey )
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint8_t temp_buf[16] = {0};
	uint8_t pri_offset =8;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
    //set command header
	tpdu_init_with_id(&command,CMD_GENERATE_KEY);
	//convert to internal key type
	temp_buf[KEYUSAGE] = 0x02;//key usage info is application
    if(symkey != NULL)
    {
        temp_buf[KID] =  symkey->id;
		switch(symkey->alg)
		{
			case ALG_AES128:
				temp_buf[KALG] = 0x60;
				break;
			case ALG_DES128:
				temp_buf[KALG] = 0x00;
				break;
			case ALG_SM4:
				temp_buf[KALG] = 0x40;
				break;
		}
		util_queue_rear_push(temp_buf,8, &queue_in);
	}
	else
	{
	if(pub_key->id != pri_key->id)//store the public key and private key into different kid.
	{
        //public kid
        temp_buf[KID] = pub_key->id;
        //private kid
        temp_buf[KID+pri_offset] = pri_key->id;
		//private key usage info is appliaction
    	temp_buf[KEYUSAGE+pri_offset] = 0x02;
        //algorithm type and model length setting
		switch(pub_key->alg)
		{
			case ALG_ECC256_NIST:
				temp_buf[KALG] = 0xA0;
				temp_buf[KMODEL_LEN] = 0x20;
			    temp_buf[KALG+pri_offset] = 0xA1;
				temp_buf[KMODEL_LEN+pri_offset] = 0x20;
				break;
			case ALG_SM2:
				temp_buf[KALG] = 0x90;
				temp_buf[KMODEL_LEN] = 0x20;
			    temp_buf[KALG+pri_offset] = 0x91;
				temp_buf[KMODEL_LEN+pri_offset] = 0x20;
				break;
			case ALG_RSA1024_CRT:
				temp_buf[KALG] = 0x80;
				temp_buf[KMODEL_LEN] = 0x20;
			    temp_buf[KALG+pri_offset] = 0x82;
				temp_buf[KMODEL_LEN+pri_offset] = 0x20;
				break;
			case ALG_RSA2048_CRT:
				temp_buf[KALG] = 0x80;
				temp_buf[KMODEL_LEN] = 0x40;
			    temp_buf[KALG+pri_offset] = 0x82;
				temp_buf[KMODEL_LEN+pri_offset] = 0x40;
				break;
			case ALG_RSA1024_STANDAND:
				temp_buf[KALG] = 0x80;
				temp_buf[KMODEL_LEN] = 0x20;
			    temp_buf[KALG+pri_offset] = 0x81;
				temp_buf[KMODEL_LEN+pri_offset] = 0x20;
				break;
			case ALG_RSA2048_STANDAND:
				temp_buf[KALG] = 0x80;
				temp_buf[KMODEL_LEN] = 0x40;
			    temp_buf[KALG+pri_offset] = 0x81;
				temp_buf[KMODEL_LEN+pri_offset] = 0x40;
				break;
		}
		util_queue_rear_push(temp_buf,16, &queue_in);
	}
	else if(pub_key->id == pri_key->id)//store the public key and private key into the same kid.
	{
		  //kid of public/private key
		  temp_buf[KID] = pub_key->id;
		  switch(pub_key->alg)
		  {
            case ALG_ECC256_NIST:
				temp_buf[KALG] = 0xA2;
				temp_buf[KMODEL_LEN] = 0x20;
				break;
			case ALG_SM2:
				temp_buf[KALG] = 0x92;
				temp_buf[KMODEL_LEN] = 0x20;
				break;
			case ALG_RSA1024_CRT:
				temp_buf[KALG] = 0x84;
				temp_buf[KMODEL_LEN] = 0x20;
				break;
			case ALG_RSA2048_CRT:
				temp_buf[KALG] = 0x84;
				temp_buf[KMODEL_LEN] = 0x40;
				break;
			case ALG_RSA1024_STANDAND:
				temp_buf[KALG] = 0x83;
				temp_buf[KMODEL_LEN] = 0x20;
				break;
			case ALG_RSA2048_STANDAND:
				temp_buf[KALG] = 0x83;
				temp_buf[KMODEL_LEN] = 0x40;
				break;
			}
		   util_queue_rear_push(temp_buf,8, &queue_in);
	}
	}

	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	if(symkey == NULL)
	{
	    if(pub_key->alg==ALG_ECC256_NIST||pub_key->alg==ALG_SM2)
		{
			memcpy(pub_key->val,&queue_out.q_buf[off],64);
			pub_key->val_len = 64;
		}
		else if(pub_key->alg==ALG_RSA1024_CRT||pub_key->alg==ALG_RSA1024_STANDAND)
		{
			memcpy(pub_key->val,&queue_out.q_buf[off],132);
			pub_key->val_len = 132;
		}
		else if(pub_key->alg==ALG_RSA2048_CRT||pub_key->alg==ALG_RSA2048_STANDAND)
		{
			memcpy(pub_key->val,&queue_out.q_buf[off],260);
			pub_key->val_len = 260;
		}
	}
	

	return SE_SUCCESS;
}



/**
* @brief generate shared key command package
* @param [in] shared_key->alg  	algorithm type    
* @param [in] input  			input data
* @param [in] input_len    		input data length
* @param [in] if_return_key 	whether to return the key 
* @param [in] if_return_s  		whether to return s
* @param [out] shared_key  		shared key
* @param [out] sm2_s  			s
* @return refer error.h
* @note no
* @see util_queue_init  tpdu_init_with_id  tpdu_set_p1p2  util_queue_rear_push  tpdu_execute
*/
se_error_t apdu_generate_shared_key (uint8_t *in_buf, uint32_t in_buf_len, unikey_t *shared_key, uint8_t *sm2_s, bool if_return_key, bool if_return_s)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint8_t return_key_flag = 0;
	uint8_t return_s_flag = 0;
	//uint8_t temp_buf[4] = {0};
	uint32_t out_len = 0;


	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);

	//set command header
	tpdu_init_with_id(&command,CMD_GEN_SHARED_KEY);
	
	//key:0x00-export 0x80 do not export
	return_key_flag = (if_return_key==true)?0x10:0x00;
	//s:0x01-export 0x00 not export
	return_s_flag =  (if_return_s==true)?0x20:0x00;
	//set P1P2
	 switch(shared_key->alg)
		  {
            case ALG_ECDH_ECC256:
				p1 =return_key_flag|return_s_flag|0x00;
				break;
			case ALG_ECDH_SM2:
				p1 =return_key_flag|return_s_flag|0x01;
				break;
			case ALG_SM2_SM2:
				p1 =return_key_flag|return_s_flag|0x02;
				break;
			}
	 if(shared_key->alg==ALG_ECDH_ECC256||shared_key->alg==ALG_ECDH_SM2)
	 {
	      p2 =shared_key->id;
	 }
	tpdu_set_p1p2 (&command,p1,p2);

	//input data is stored in deque
	util_queue_rear_push(in_buf,in_buf_len,&queue_in);

	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;

	if(shared_key->alg==ALG_ECDH_ECC256||shared_key->alg==ALG_ECDH_SM2)
	{
		//ECC return 32 bytes key
		if(if_return_key==true)
		{
			memcpy(shared_key->val,&queue_out.q_buf[off],32);
			shared_key->val_len = 32;
		}
	}
	else
	{
		if(if_return_key==true)
		{
			memcpy(shared_key->val,&queue_out.q_buf[off],16);
			shared_key->val_len = 16;
		}
		if(if_return_s==true)
			memcpy(sm2_s,&queue_out.q_buf[off+16],64);		
	}

	return SE_SUCCESS;

}


/**
* @brief Delete key command package
* @param [in] id  kid 
* @return refer error.h
* @note no
* @see no
*/
se_error_t apdu_delete_key(uint8_t id)
{

	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;

	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_DEL_KEY);
	//set P1P2
	tpdu_set_p1p2 (&command,0x00,id);
	
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;
}


/**
* @brief Import key command package
* @param [in] ukey->alg 	ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] ukey->id  	application kid(the private kid and symmetric kid are used to decrypt)
* @param [in] inkey->alg 	algorithm type(ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] inkey->id 	import key id:0x00-0xFF 
* @param [in] inkey->type	key type
* @param [in] inkey->val 	key value
* @param [in] inkey->val_len  application key length
* @param [in] if_cipher   	 whether to encrypt the input key 
* @param [in] if_trasns_key  whether to use the transport key to encrypt
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2	 tpdu_execute	util_queue_size
*/
se_error_t  apdu_import_key (unikey_t *ukey, unikey_t *inkey, bool if_cipher, bool if_trasns_key)
{
	se_error_t ret = 0;
	uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	iso_command_apdu_t command = {0};
	uint8_t temp_buf[8] = {0};
	//uint32_t temp_len = 0;
	//uint32_t off = 0;
	uint32_t cipher_alg= 0;
	

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	
	//initialize the p1/p2
	if(if_cipher)
	{
        //if the key imported is encrypted,the bit 5 of p1 is set into '1'.
		p1=p1|0x10;
		//if the transport key is used to encrypt the import key, the bit 6 of p1 is set into '1'.
		if (if_trasns_key)
		{
			p1=p1|0x20;
			//if the algorithm type of transport key is SM4, the bit4~bit1 are '0100'.  
			p1=p1|0x04;
			//the p2 value(0x02) is the kid of transport key.
			p2=0x02;
		}
		else
		{
	        //set p1 according to the application encryption key algorithm type. 
			switch(ukey->alg)
			{
				case ALG_RSA1024_CRT:
					cipher_alg = 0x08;
					break;
				case ALG_RSA2048_CRT:
					cipher_alg = 0x08;
					break;
				case ALG_ECC256_NIST:
					cipher_alg = 0x0A;
					break;
				case ALG_SM2:
					cipher_alg = 0x09;
					break;
				case ALG_DES128:
					cipher_alg = 0x00;
					break;
				case ALG_SM4:
					cipher_alg = 0x04;
					break;
				case ALG_AES128:
					cipher_alg = 0x06;
					break;
				
			}
			p1=p1|cipher_alg;
		    //p2 value is the application kid
		    p2=ukey->id;

		}
	    
	}

	//organize data (key info + key ciphertext or plaintext)
	temp_buf[KEYUSAGE]=0x02;
	temp_buf[KID]=inkey->id;
	switch(inkey->alg)
		{
			case ALG_AES128:
				temp_buf[KALG] = 0x60;
	            temp_buf[KLEN_LOW]=0x10;
				break;
			case ALG_DES128:
				temp_buf[KALG] = 0x00;
	            temp_buf[KLEN_LOW]=0x10;
				break;
			case ALG_SM4:
				temp_buf[KALG] = 0x40;
	            temp_buf[KLEN_LOW]=0x10;
				break;
			case ALG_ECC256_NIST:
                if(inkey->type==PRI)
                {
  				  temp_buf[KALG] = 0xA1;
  				  temp_buf[KMODEL_LEN] = 0x20;
	              temp_buf[KLEN_LOW]=0x20;
				}
				else if(inkey->type==PUB)
				{
                  temp_buf[KALG] = 0xA0;
				  temp_buf[KMODEL_LEN] = 0x20;
	              temp_buf[KLEN_LOW]=0x40;
				}
			    else if(inkey->type==PRI_PUB_PAIR)
			    {
				   temp_buf[KALG] = 0xA2;
				   temp_buf[KMODEL_LEN] = 0x20;
	               temp_buf[KLEN_LOW]=0x60;
				}					    
				break;
			case ALG_ECC_ED25519:
                  temp_buf[KALG] = 0xB0;
				  temp_buf[KMODEL_LEN] = 0x20;
	              temp_buf[KLEN_LOW]=0x20;				    
				break;
			case ALG_SM2:
				if(inkey->type==PRI)
                {
  				  temp_buf[KALG] = 0x91;
  				  temp_buf[KMODEL_LEN] = 0x20;
	              temp_buf[KLEN_LOW]=0x20;
				}
				else if(inkey->type==PUB)
				{
                  temp_buf[KALG] = 0x90;
				  temp_buf[KMODEL_LEN] = 0x20;
	              temp_buf[KLEN_LOW]=0x40;
				}
			    else if(inkey->type==PRI_PUB_PAIR)
			    {
				   temp_buf[KALG] = 0x92;
				   temp_buf[KMODEL_LEN] = 0x20;
	               temp_buf[KLEN_LOW]=0x60;
				}			
				break;
			case ALG_RSA1024_CRT:
				if(inkey->type==PRI)
                {
  				  temp_buf[KALG] = 0x82;
  				  temp_buf[KMODEL_LEN] = 0x20;
				  temp_buf[KLEN_HIGH]=0x01;
	              temp_buf[KLEN_LOW]=0x44;
				}
				else if(inkey->type==PUB)
				{
                  temp_buf[KALG] = 0x80;
				  temp_buf[KMODEL_LEN] = 0x20;
	              temp_buf[KLEN_LOW]=0x84;
				}
			    else if(inkey->type==PRI_PUB_PAIR)
			    {
				   temp_buf[KALG] = 0x84;
				   temp_buf[KMODEL_LEN] = 0x20;
				   temp_buf[KLEN_HIGH]=0x01;
	               temp_buf[KLEN_LOW]=0xC4;
				}			
				break;
			case ALG_RSA2048_CRT:
				if(inkey->type==PRI)
                {
  				  temp_buf[KALG] = 0x82;
  				  temp_buf[KMODEL_LEN] = 0x40;
				  temp_buf[KLEN_HIGH]=0x02;
	              temp_buf[KLEN_LOW]=0x84;
				}
				else if(inkey->type==PUB)
				{
                  temp_buf[KALG] = 0x80;
				  temp_buf[KMODEL_LEN] = 0x40;
				  temp_buf[KLEN_HIGH]=0x01;
	              temp_buf[KLEN_LOW]=0x04;
				}
			    else if(inkey->type==PRI_PUB_PAIR)
			    {
				   temp_buf[KALG] = 0x84;
				   temp_buf[KMODEL_LEN] = 0x40;
				   temp_buf[KLEN_HIGH]=0x03;
	               temp_buf[KLEN_LOW]=0x84;
				}			
				break;
			case ALG_RSA1024_STANDAND:
				if(inkey->type==PRI)
                {
  				  temp_buf[KALG] = 0x81;
  				  temp_buf[KMODEL_LEN] = 0x20;
				  temp_buf[KLEN_HIGH]=0x01;
	              temp_buf[KLEN_LOW]=0x00;
				}
				else if(inkey->type==PUB)
				{
                  temp_buf[KALG] = 0x80;
				  temp_buf[KMODEL_LEN] = 0x20;
	              temp_buf[KLEN_LOW]=0x84;
				}
			    else if(inkey->type==PRI_PUB_PAIR)
			    {
				   temp_buf[KALG] = 0x83;
				   temp_buf[KMODEL_LEN] = 0x20;
				   temp_buf[KLEN_HIGH]=0x01;
	               temp_buf[KLEN_LOW]=0x04;
				}			
				break;
			case ALG_RSA2048_STANDAND:
				if(inkey->type==PRI)
                {
  				  temp_buf[KALG] = 0x81;
  				  temp_buf[KMODEL_LEN] = 0x40;
				  temp_buf[KLEN_HIGH]=0x02;
	              temp_buf[KLEN_LOW]=0x00;
				}
				else if(inkey->type==PUB)
				{
                  temp_buf[KALG] = 0x80;
				  temp_buf[KMODEL_LEN] = 0x40;
				  temp_buf[KLEN_HIGH]=0x01;
	              temp_buf[KLEN_LOW]=0x04;
				}
			    else if(inkey->type==PRI_PUB_PAIR)
			    {
				   temp_buf[KALG] = 0x83;
				   temp_buf[KMODEL_LEN] = 0x40;
				   temp_buf[KLEN_HIGH]=0x02;
	               temp_buf[KLEN_LOW]=0x04;
				}			
				break;
		}
	
	//key info is stored in deque
	util_queue_rear_push((uint8_t *)temp_buf,8, &queue_in);

	//input data is stored in deque
	if(if_trasns_key)
	{
	  ret =  apdu_trans_encrypt((uint8_t *)inkey->val, inkey->val_len,8,&queue_in);//if the data nedds be protected, the data must be encrypted by transport key before storing the data into deque.
	  if(ret!=SE_SUCCESS)
		return ret;	 
	}
    else
    {
	   util_queue_rear_push((uint8_t *)inkey->val,inkey->val_len, &queue_in);
	}
	
	
	//set command header
	tpdu_init_with_id(&command,CMD_IMPORT_KEY);
	if((queue_in.q_buf_len+8)>0xff)
	{
	  command.isoCase = ISO_CASE_3E;
	}
	else
	{
      command.isoCase = ISO_CASE_3S;
	}
	//set P1/P2
	tpdu_set_p1p2 (&command,p1,p2);
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size(&queue_in));
    if(ret!=SE_SUCCESS)
   	 return ret;

	return SE_SUCCESS;

}


/**
* @brief Export key command package
* @param [in] ukey->alg 	ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] ukey->id  	application kid(the public kid and symmetric kid are used to encrypt)
* @param [in] exkey->alg 	algorithm type(ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] exkey->id   	export key id:0x00-0xFF  
* @param [in] exkey->type 	key type
* @param [in] if_cipher  	whether to encrypt the private/symmetric key exported 
* @param [out] exkey->val   export key value
* @param [out] exkey->val_len  export key length
* @param [in] if_trasns_key whether to use the transport key to encrypt
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute  util_queue_size
*/
se_error_t  apdu_export_key (unikey_t *ukey, unikey_t *exkey, bool if_cipher, bool if_trasns_key)
{
	se_error_t ret = 0;
	uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	iso_command_apdu_t command = {0};
	uint32_t off = 0;
	uint32_t temp_len = 0;
	uint8_t temp_buf[8] = {0};
	uint8_t cipher_alg = 0x00;
	//initialize the p1/p2
	if(if_cipher==true)
	{
        //if the key exported is encrypted,the bit 5 of p1 is set into '1'.
		p1=p1|0x10;
		//if the transport key is used to encrypt the import key, the bit 6 of p1 is set into '1'.
		if (if_trasns_key)
		{
			p1=p1|0x20;
			//if the algorithm type of transport key is SM4, the bit4~bit1 are '0100'.  
			p1=p1|0x04;
			//the p2 value(0x02) is the kid of transport key.
			p2=0x02;
		}
		else
		{
	         //set p1 according to the application encryption key algorithm type. 
		switch(ukey->alg)
			{
				case ALG_RSA1024_CRT:
					cipher_alg = 0x08;
					break;
				case ALG_RSA2048_CRT:
					cipher_alg = 0x08;
					break;
				case ALG_ECC256_NIST:
					cipher_alg = 0x0A;
					break;
				case ALG_SM2:
					cipher_alg = 0x09;
					break;
				case ALG_DES128:
					cipher_alg = 0x00;
					break;
				case ALG_SM4:
					cipher_alg = 0x04;
					break;
				case ALG_AES128:
					cipher_alg = 0x06;
					break;
				
			}
           
			p1=p1|cipher_alg;
		    //p2 value is the application kid
		    p2=ukey->id;

		}
	    
	}
	//organize data (key info + key ciphertext or plaintext)
		temp_buf[0]=exkey->id;
	    switch(exkey->alg)
			{
				case ALG_AES128:
					temp_buf[1] = 0x60;
					break;
				case ALG_DES128:
					temp_buf[1] = 0x00;
					break;
				case ALG_SM4:
					temp_buf[1] = 0x40;
					break;
				case ALG_ECC256_NIST:
					if(exkey->type==PRI)
					{
					  temp_buf[1] = 0xA1;
					}
					else if(exkey->type==PUB)
					{
					  temp_buf[1]  = 0xA0;
					}		
					break;
				case ALG_SM2:
					if(exkey->type==PRI)
					{
					  temp_buf[1] = 0x91;
					}
					else if(exkey->type==PUB)
					{
					  temp_buf[1] = 0x90;
					}		
					break;
				case ALG_RSA1024_CRT:
					if(exkey->type==PRI)
					{
					  temp_buf[1] = 0x82;
					}
					else if(exkey->type==PUB)
					{
					  temp_buf[1] = 0x80;
					}		
					break;
				case ALG_RSA2048_CRT:
					if(exkey->type==PRI)
					{
					  temp_buf[1] = 0x82;
					}
					else if(exkey->type==PUB)
					{
					  temp_buf[1] = 0x80;
					}			
					break;
				case ALG_RSA1024_STANDAND:
					if(exkey->type==PRI)
					{
					  temp_buf[1] = 0x81;
					}
					else if(exkey->type==PUB)
					{
					  temp_buf[1] = 0x80;
					}		
					break;
				case ALG_RSA2048_STANDAND:
					if(exkey->type==PRI)
					{
					  temp_buf[1] = 0x81;
					}
					else if(exkey->type==PUB)
					{
					  temp_buf[1] = 0x80;
					}		
					break;
			}


	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_EXPORT_KEY);
	//set p1 p2
	tpdu_set_p1p2 (&command,p1,p2);
	//input data is stored in deque
	util_queue_rear_push((uint8_t *)temp_buf,2, &queue_in);
	
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
		if(ret!=SE_SUCCESS)
			return ret;
	if(if_trasns_key)
	{
	  ret =  apdu_trans_decrypt(&queue_out);
	  if(ret!=SE_SUCCESS)
		return ret;	 
	  off = queue_out.front_node;
	  exkey->val_len = 0;
	  exkey->val_len = exkey->val_len|queue_out.q_buf[off];
      exkey->val_len = (exkey->val_len<<8)|queue_out.q_buf[off+1];	  
	  memcpy(&exkey->val,&queue_out.q_buf[off+2],exkey->val_len);
	}
    else
    {
       off = queue_out.front_node;
	   memcpy(&exkey->val,&queue_out.q_buf[off],queue_out.q_buf_len);
	   exkey->val_len = queue_out.q_buf_len;   
	}
	
	if(exkey->val==NULL||exkey->val_len<16||exkey->val_len>656)
	{
	
		return SE_ERR_DATA;
	}
	return SE_SUCCESS;
}



/**
* @brief Get the key information command package.
* @param [in] if_app_key    whther to get thr application key information
* @param [out] out_buf   	output data
* @param [out] out_buf_len  output data length
* @return refer error.h
* @note no
* @see  util_queue_init   tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute  util_queue_size
*/
se_error_t  apdu_get_key_info (bool if_app_key, uint8_t *out_buf,uint32_t *out_buf_len)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint32_t Le = 0;
	uint8_t p1 = 0x00;
	
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_GET_KEY_INFO);
	if (if_app_key == true)
	{
	    p1 = 0x01;
	}
	else
	{
		p1 = 0x00;
	}
	tpdu_set_p1p2(&command,p1,0x00);
	//set Le
	tpdu_set_le(&command, Le);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(out_buf,&queue_out.q_buf[off],queue_out.q_buf_len);
    *out_buf_len = queue_out.q_buf_len;
	if(*out_buf_len!=0x200)
	{	
		LOGE("failed to call apdu_get_key_info!\n");
		return SE_ERR_LENGTH;
	}
	return SE_SUCCESS;
}



/**
* @brief Symmetric encryption and decryption command package
* @param [in]  key->alg 				algorithm type  
* @param [in]  key->id   				kid
* @param [in]  sym_param->mode 			encrypt mode
* @param [in]  sym_param->iv  			IV
* @param [in]  sym_param->iv_len  		IV length
* @param [in]  sym_param->padding_type  padding type
* @param [in]  in_buf 					input data
* @param [in]  in_buf_len 				input data length
* @param [in]  if_first_block 			whether a first block
* @param [in]  if_last_block        	whether a last block
* @param [in]  if_enc					whether to encrypt
* @param [out] out_buf 					output data
* @param [out] out_buf_len 				output data length
* @return refer error.h
* @note no
* @see util_queue_init  util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute  util_queue_size
*/
se_error_t apdu_sym_enc_dec (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len, bool if_first_block, bool if_last_block, bool if_enc)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint32_t off = 0;
	uint32_t temp_len =0;
	uint32_t le = LE_MAX_LEN;
	
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	
	
	//set p1/p2
	//whether the input is cascade data
	if(if_last_block==false)
	{
	    p1=p1|0x80;
	}
	
	//set p1 according to the algorithm type.
	switch(key->alg)
	{
		case ALG_DES128:
			p1=p1|0x00;
			break;
		case ALG_SM4:
			p1=p1|0x40;
			break;
		case ALG_AES128:
			p1=p1|0x60;
			break;	
	}
	//set p1 according to the calculation type.
	if(if_enc)
	{
		p1=p1|0x00;
	}
	else
	{
		p1=p1|0x04;
	}
	//set p1 according to the calculation mode.
	switch(sym_param->mode)
	{
		case SYM_MODE_CBC:
			p1=p1|0x01;
			break;
		case SYM_MODE_ECB:
			p1=p1|0x00;
			break;
	}

	//set p2 according to the application kid.
	p2 = key->id;
	
	//check the first packet splicing command header
	if(if_first_block)
	{
		//splicing data header, iv value
		if(sym_param->mode==SYM_MODE_CBC )
			util_queue_rear_push(sym_param->iv,sym_param->iv_len, &queue_in);
	}

	
	util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
	//encryption
	if(if_enc)
	{
		//if the last block,padding as PKCS7
		if(if_last_block)
		{
			if(sym_param->padding_type==PADDING_PKCS7)
			{
				ret = apdu_sym_padding(sym_param->padding_type, (uint8_t*)&queue_in, in_buf_len,(uint8_t*)&queue_in, out_buf_len);		
				if(ret!=SE_SUCCESS)
					return ret;
			}
		}

	}
    //set command header
	tpdu_init_with_id(&command,CMD_CIPHER_DATA);
	//set p1 p2
	tpdu_set_p1p2 (&command,p1,p2);
    if(queue_in.q_buf_len>0xff)
	{
	  command.isoCase = ISO_CASE_3E;
	}
	else
	{
      command.isoCase = ISO_CASE_3S;
	}
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
		return ret;
	
	if(ret == SE_ERR_DATA_REMAINING)
	{	
		off = queue_out.front_node;
		
		memcpy(out_buf,&queue_out.q_buf[off],temp_len);
		*out_buf_len = temp_len;
		
		while((ret>>8&0xff)==0x61)
		{
			
			util_queue_init(&queue_in);
			util_queue_init(&queue_out);
			tpdu_init_with_id(&command,CMD_GET_RESPONSE);
			tpdu_set_le (&command,le);
			ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
			if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
				return ret;
			
			off = queue_out.front_node;
			
			memcpy(out_buf+(*out_buf_len),&queue_out.q_buf[off],temp_len);
			*out_buf_len += temp_len;
		}
	}

	else
	{
	if(!if_enc)
	{
		if(if_last_block)
		{
			if(sym_param->padding_type==PADDING_PKCS7)
			{
				off = queue_out.rear_node;						
				
				ret = apdu_sym_unpadding(sym_param->padding_type, (uint8_t*)&queue_out,queue_out.q_buf_len ,(uint8_t*)&queue_out, &temp_len);
				if(ret!=SE_SUCCESS)
					return ret;
			}

		}

	}

	off = queue_out.front_node;
	//copy back the output data from the deque
	memcpy(out_buf,&queue_out.q_buf[off],temp_len);
	*out_buf_len = temp_len;

	}
	return SE_SUCCESS;
}




/**
* @brief MAC command package
* @param [in]  key->alg 		  ALG_AES128/ALG_DES128/ALG_SM4  
* @param [in]  key->id    		  kid
* @param [in]  sym_param->iv   	  IV
* @param [in]  sym_param->iv_len  IV length
* @param [in]  sym_param->padding_type padding type
* @param [in]  in_buf 			  input data
* @param [in]  in_buf_len 		  input 
* @param [in]  mac 				 MAC
* @param [in]  mac_len 			 MAC length(when verify mac)
* @param [in]  if_first_block    whether a first block
* @param [in]  if_last_block     whether a last block
* @param [in] if_mac 		     whether to calculate the mac
* @param [out] mac 				 MAC
* @param [out] mac_len 			 MAC length(when calculate mac)
* @return error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute	util_queue_size
*/
se_error_t apdu_mac (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *mac, uint32_t *mac_len, bool if_first_block, bool if_last_block, bool if_mac)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint32_t off = 0;
	uint32_t temp_len =0;
	
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_CIPHER_DATA);
	//set p1/p2
	
	//whether the input is cascade data
	if(if_last_block==false)
	{
	    p1=p1|0x80;
	}
	
	//set p1 according to the algorithm type.
	switch(key->alg)
	{
		case ALG_DES128:
			p1=p1|0x00;
			break;
		case ALG_SM4:
			p1=p1|0x40;
			break;
		case ALG_AES128:
			p1=p1|0x60;
			break;	
	}
	//set p1 according to the calculation type.
	if(if_mac)
	{
		p1=p1|0x08;
	}
	else
	{
		p1=p1|0x0C;
	}
	//set p1 according to the MAC padding type.
	switch(sym_param->padding_type)
	{
		case PADDING_NOPADDING:
			p1=p1|0x00;
			break;
		case PADDING_ISO9797_M1:
			p1=p1|0x01;
			break;
		case PADDING_ISO9797_M2:
			p1=p1|0x02;
			break;
	}
	//set p2 according to the application kid.
	p2 = key->id;
	tpdu_set_p1p2 (&command,p1,p2);
	//check the first packet splicing command header
	if(if_first_block)
	{
		//splicing data header, iv value
		if(sym_param->iv_len!=0)
			util_queue_rear_push(sym_param->iv,sym_param->iv_len, &queue_in);
	}

	util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
	if(!if_mac)
	{
		if(if_last_block)
		{

     		//verify MAC, splicing mac to the end
			util_queue_rear_push((uint8_t *)mac,*mac_len, &queue_in);					
		}

	}
	
	if(queue_in.q_buf_len>0xff)
	   {
		 command.isoCase = ISO_CASE_3E;
	   }
	   else
	   {
		 command.isoCase = ISO_CASE_3S;
	   }

	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS)
		return ret;


	//calculate MAC
	if(if_mac)
	{
		if(if_last_block)
		{
			off = queue_out.front_node;
			//copy back the output data from the deque
			memcpy(mac,&queue_out.q_buf[off],temp_len);
			*mac_len = temp_len;						

		}

	}

	return SE_SUCCESS;

}





/**
* @brief Asymmetric encryption command package
* @param [in] key->alg  				ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id 					public kid
* @param [in] asym_param->padding_type  rsa padding mode(PADDING_NOPADDING/PADDING_PKCS1)(only valid for RSA algorithm)
* @param [in] in_buf  					plaintext data  
* @param [in] in_buf_len  				plaintext data length
* @param [in] if_last_block  			whether a last block
* @param [out] out_buf  				ciphertext data
* @param [out] out_buf_len 				ciphertext data length
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute	util_queue_size
*/
se_error_t apdu_asym_enc (pub_key_t *key, const alg_asym_param_t *asym_param,const uint8_t *in_buf,uint32_t in_buf_len, uint8_t * out_buf, uint32_t * out_buf_len, bool if_last_block)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint32_t off = 0;
	uint32_t temp_len =0;
	uint32_t le = LE_MAX_LEN;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_PKI_ENCIPHER);
	//set p1 p2
	//whether the input is cascade data
	if(if_last_block==false)
	{
	    p1=p1|0x80;
	}
	
	//set p1 according to the asymmetric algorithm type.
	switch(key->alg)
	{
		case ALG_RSA1024_CRT:
			p1=p1|0x00;
			break;
		case ALG_RSA2048_CRT:
			p1=p1|0x00;
			break;
		case ALG_ECC256_NIST:
			p1=p1|0x20;
			break;
		case ALG_SM2:
			p1=p1|0x10;
			break;	
	}
	//set p1 according to the rsa padding mode.
	if(key->alg == ALG_RSA1024_CRT || key->alg == ALG_RSA2048_CRT )
	{
	    if(asym_param->padding_type==PADDING_NOPADDING)
		{
			p1=p1|0x00;
		}
		else if(asym_param->padding_type==PADDING_PKCS1)
		{
			p1=p1|0x01;
		}
	}
	
	
	//set p2 according to the application kid.
	p2 = key->id;
	tpdu_set_p1p2 (&command,p1,p2);


	//input data is stored in deque
	util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
    if(queue_in.q_buf_len>0xff)
	{
		command.isoCase = ISO_CASE_3E;
	}
    else
    {
	    command.isoCase = ISO_CASE_3S;
    }
    if(if_last_block)
	{
		command.isoCase = ISO_CASE_4E;
		tpdu_set_le(&command,le);
	}
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
		return ret;

	//whether a last block
	if(if_last_block)
	{
		off = queue_out.front_node;
		//copy back the output data from the deque
		memcpy(out_buf,&queue_out.q_buf[off],temp_len);
		*out_buf_len = temp_len;
		//if 61xx is returned,send the cmd CMD_GET_RESPONSE
		while((ret>>8&0xff)==0x61)
		{
			//send get_response command to obtain the remaining data
			util_queue_init(&queue_in);
			util_queue_init(&queue_out);
			tpdu_init_with_id(&command,CMD_GET_RESPONSE);
			tpdu_set_le (&command,le);
			ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
			if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
				return ret;
			//splice the remaining data
			off = queue_out.front_node;
			//copy back the output data from the deque
			memcpy(out_buf+(*out_buf_len),&queue_out.q_buf[off],temp_len);
			*out_buf_len += temp_len;
		}

	}


	return SE_SUCCESS;
}


/**
* @brief Asymmetric decryption command package
* @param [in] ke->alg 			 		ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id  					public kid
* @param [in] asym_param->padding_type  rsa padding mode(PADDING_NOPADDING/PADDING_PKCS1)(only valid for RSA algorithm)
* @param [in] in_buf  					plaintext data  
* @param [in] in_buf_len  				plaintext data length
* @param [in] if_last_block  			whether a last block
* @param [out] out_buf  				plaintext data  
* @param [out] out_buf_len  			plaintext data length
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute	util_queue_size
*/
se_error_t apdu_asym_dec (pri_key_t *key, const alg_asym_param_t *asym_param,const uint8_t *in_buf,uint32_t in_buf_len, uint8_t * out_buf, uint32_t * out_buf_len, bool if_last_block)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint32_t off = 0;
	uint32_t temp_len =0;
	uint32_t le = LE_MAX_LEN;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_PKI_DECIPHER);
	//set p1 p2
	//whether the input is cascade data
	if(if_last_block==false)
	{
	    p1=p1|0x80;
	}
	
	//set p1 according to the asymmetric algorithm type.
	switch(key->alg)
	{
		case ALG_RSA1024_CRT:
			p1=p1|0x00;
			break;
		case ALG_RSA2048_CRT:
			p1=p1|0x00;
			break;
		case ALG_ECC256_NIST:
			p1=p1|0x20;
			break;
		case ALG_SM2:
			p1=p1|0x10;
			break;	
	}
	//set p1 according to the rsa padding mode.
	if(key->alg == ALG_RSA1024_CRT || key->alg == ALG_RSA2048_CRT )
	{
	    if(asym_param->padding_type==PADDING_NOPADDING)
		{
			p1=p1|0x00;
		}
		else if(asym_param->padding_type==PADDING_PKCS1)
		{
			p1=p1|0x01;
		}
	}

	//set p2 according to the application kid.
	p2 = key->id;
	tpdu_set_p1p2 (&command,p1,p2);


	//input data is stored in deque
	util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
    if(queue_in.q_buf_len>0xff)
	{
		command.isoCase = ISO_CASE_3E;
	}
    else
    {
	    command.isoCase = ISO_CASE_3S;
    }
    if(if_last_block)
	{
		command.isoCase = ISO_CASE_4E;
		tpdu_set_le(&command,le);
	}
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
		return ret;

	//whether a last block
	if(if_last_block)
	{
		off = queue_out.front_node;
		//copy back the output data from the deque
		memcpy(out_buf,&queue_out.q_buf[off],temp_len);
		*out_buf_len = temp_len;
		//if 61xx is returned,send the cmd CMD_GET_RESPONSE
		while((ret>>8&0xff)==0x61)
		{
			//send get_response command to obtain the remaining data
			util_queue_init(&queue_in);
			util_queue_init(&queue_out);
		    tpdu_init_with_id(&command,CMD_GET_RESPONSE);
			tpdu_set_le (&command,le);
			ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
			if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
				return ret;
			//splice the remaining data
			off = queue_out.front_node;
			//copy back the output data from the deque
			memcpy(out_buf+(*out_buf_len),&queue_out.q_buf[off],temp_len);
			*out_buf_len += temp_len;
		}

	}


	return SE_SUCCESS;
}




/**
* @brief Asymmetric signature command package
* @param [in] key->alg  			ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id   			private kid
* @param [in] asym_param->hash_type ALG_SHA1/ALG_SHA256/ALG_SHA384/ALG_SHA512/ALG_SM3/ALG_NONE
* @param [in] in_buf  				input data 
* @param [in] in_buf_len 			input data length
* @param [in] if_last_block  	    whether a last block
* @param [out] sign_buf     		signature
* @param [out] sign_buf_len 		signature length
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute	util_queue_size
*/
se_error_t apdu_asym_sign (pub_key_t *key, const alg_asym_param_t *asym_param,const uint8_t *in_buf,uint32_t in_buf_len, uint8_t *sign_buf, uint32_t * sign_buf_len , bool if_last_block)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint32_t off = 0;
	uint32_t temp_len =0;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_COMPUTE_SIGNATURE);
	//set p1 p2
	//whether the input is cascade data
	if(if_last_block==false)
	{
	    p1=p1|0x80;
	}
	
	//set p1 according to the asymmetric algorithm type.
	switch(key->alg)
	{
		case ALG_RSA1024_CRT:
			p1=p1|0x00;
			break;
		case ALG_RSA2048_CRT:
			p1=p1|0x00;
			break;
		case ALG_ECC256_NIST:
			p1=p1|0x20;
			break;
		case ALG_SM2:
			p1=p1|0x10;
			break;	
	}
	//set p1 according to the hash type.
	switch(asym_param->hash_type)
	{
		case ALG_NONE:
			p1=p1|0x08;
			break;
		case ALG_SHA1:
			p1=p1|0x00;
			break;
		case ALG_SHA224:
			p1=p1|0x01;
			break;
		case ALG_SHA256:
			p1=p1|0x02;
			break;
		case ALG_SHA384:
			p1=p1|0x03;
			break;
		case ALG_SHA512:
			p1=p1|0x04;
			break;
		case ALG_SM3:
			p1=p1|0x05;
			break;	
	}
	
	//set p2 according to the kid.
	p2 = key->id;
	tpdu_set_p1p2 (&command,p1,p2);

	//input data is stored in deque
	util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
    if(queue_in.q_buf_len>0xff)
	{
		command.isoCase = ISO_CASE_3E;
	}
    else
    {
	    command.isoCase = ISO_CASE_3S;
    }
    

	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
		return ret;
	
	if(if_last_block)
	{
		off = queue_out.front_node;
		//copy back the output data from the deque
		memcpy(sign_buf,&queue_out.q_buf[off],temp_len);
		*sign_buf_len = temp_len; 
	}


	return SE_SUCCESS;
}


/**
* @brief Verify signature command package
* @param [in] key->alg  			ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id  				private kid
* @param [in] asym_param->hash_type ALG_SHA1/ALG_SHA256/ALG_SHA384/ALG_SHA512/ALG_SM3/ALG_NONE 
* @param [in] in_buf  				input data
* @param [in] in_buf_len  			input data length
* @param [in] if_last_block  		whether a last block
* @param [in] sign_buf     			signature
* @param [in] sign_buf_len 			signature length
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute	util_queue_size
*/
se_error_t apdu_asym_verify (pub_key_t *key, const alg_asym_param_t *asym_param,const uint8_t *in_buf,uint32_t in_buf_len, uint8_t *sign_buf, uint32_t sign_buf_len , bool if_first_block, bool if_last_block)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint32_t temp_len =0;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_VERIFY_SIGNATURE);
	//set p1 p2
	//whether the input is cascade data
	if(if_last_block==false)
	{
	    p1=p1|0x80;
	}
	
	//set p1 according to the asymmetric algorithm type.
	switch(key->alg)
	{
		case ALG_RSA1024_CRT:
			p1=p1|0x00;
			break;
		case ALG_RSA2048_CRT:
			p1=p1|0x00;
			break;
		case ALG_ECC256_NIST:
			p1=p1|0x20;
			break;
		case ALG_SM2:
			p1=p1|0x10;
			break;	
	}
	//set p1 according to the hash type.
	switch(asym_param->hash_type)
	{
		case ALG_NONE:
			p1=p1|0x08;
			break;
		case ALG_SHA1:
			p1=p1|0x00;
			break;
		case ALG_SHA224:
			p1=p1|0x01;
			break;
		case ALG_SHA256:
			p1=p1|0x02;
			break;
		case ALG_SHA384:
			p1=p1|0x03;
			break;
		case ALG_SHA512:
			p1=p1|0x04;
			break;
		case ALG_SM3:
			p1=p1|0x05;
			break;	
	}
	
	//set p2 according to the kid.
	p2 = key->id;
	tpdu_set_p1p2 (&command,p1,p2);
	//whether a last block
	if(if_first_block)
	{
		util_queue_rear_push(sign_buf,sign_buf_len, &queue_in);
	}

	//input data is stored in deque
	util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
    if(queue_in.q_buf_len>0xff)
	{
		command.isoCase = ISO_CASE_3E;
	}
    else
    {
	    command.isoCase = ISO_CASE_3S;
    }
    

	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS&&ret!=SE_ERR_DATA_REMAINING)
		return ret;

	return SE_SUCCESS;
}

/**
* @brief SM2 get za command package
* @param [in] uid->val           user ID
* @param [in] uid->val_len       user ID length
* @param [in] pub_key->val       SM2 public key X|Y
* @param [in] pub_key->val_len   SM2 public key length
* @param [out] za	             za
* @return refer error.h
* @note no
* @see  util_queue_init  util_queue_rear_push  tpdu_init_with_id	 tpdu_execute  util_queue_size
*/
se_error_t apdu_sm2_get_za (user_id_t* uid, pub_key_t *pub_key , uint8_t *za )
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint8_t temp_data[1] = {0x00};

	if(uid==NULL||pub_key==NULL||za==NULL)
	{
		LOGE("failed to apdu_sm2_get_za pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(uid->val_len==0||uid->val_len>32||pub_key->val_len!=64)
	{
		LOGE("failed to apdu_sm2_get_za input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//input data is stored in deque
	off = queue_in.front_node;
	temp_data[0] = uid->val_len&0xFF;
	util_queue_rear_push(temp_data,1, &queue_in);
	util_queue_rear_push((uint8_t *)(uid->val),uid->val_len, &queue_in);
	util_queue_rear_push((uint8_t *)(pub_key->val),pub_key->val_len, &queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_SM2_GET_ZA);

	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(za,&queue_out.q_buf[off],queue_out.q_buf_len);
 
	return SE_SUCCESS;
}




/**
* @brief Hash calculation command package
* @param [in]  alg 			algorithm type
* @param [in]  in_buf 		input data
* @param [in]  in_buf_len 	input data length
* @param [in]  if_last_block whether a last block
* @param [out] out_buf	 	output data
* @param [out] out_buf_len	output data length
* @return refer error.h
* @note no
* @see util_queue_init	 util_queue_rear_push  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute	util_queue_size
*/
se_error_t apdu_digest (uint32_t alg, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len, bool if_last_block)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint8_t p1 = 0;
	uint8_t p2 = 0;
	uint32_t off = 0;
	uint32_t temp_len =0;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_DIGEST);
	//set P1P2
	//whether the input is cascade data
	if(if_last_block==false)
	{
	    p1=p1|0x80;
	}
	
	//set p1 according to the algorithm type.
	switch(alg)
	{
		case ALG_SHA1:
			p1=p1|0x00;
			break;
		case ALG_SHA224:
			p1=p1|0x01;
			break;
		case ALG_SHA256:
			p1=p1|0x02;
			break;
		case ALG_SHA384:
			p1=p1|0x03;
			break;
		case ALG_SHA512:
			p1=p1|0x04;
			break;
		case ALG_SM3:
			p1=p1|0x05;
			break;	
	}
	tpdu_set_p1p2 (&command,p1,p2);

	//splicing data
	util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
	 if(queue_in.q_buf_len>0xff)
	{
		command.isoCase = ISO_CASE_3E;
	}
    else
    {
	    command.isoCase = ISO_CASE_3S;
    }
    
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS)
		return ret;
	off = queue_out.front_node;
	//copy back the output data from the deque
	memcpy(out_buf,&queue_out.q_buf[off],temp_len);
	*out_buf_len = temp_len;

	return SE_SUCCESS;
	
}




/**
* @brief Get random number command package
* @param [in] expected_len    expect length 
* @param [out] random         random
* @return refer error.h	
* @note no
* @see  util_queue_init  util_queue_rear_push	tpdu_init_with_id  tpdu_execute_no_response
*/
se_error_t apdu_get_random  (uint32_t expected_len, uint8_t *random)
{
  iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_GET_RANDOM);
	//set le
	tpdu_set_le(&command, expected_len&0xFF);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(random,&queue_out.q_buf[off],queue_out.q_buf_len);

	return SE_SUCCESS;
}



/**
* @brief Get SE information command package
* @param [in] type  				information type
* @param [out] info->CHIP_ID   chip 8-byte unique serial numbe
* @param [out] info->PRODUCT_INFO    product information 8 bytes
* @return refer error.h
* @note no
* @see util_queue_init  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute  util_queue_size
*/
se_error_t  apdu_get_info (info_type type, se_info_t * info)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint32_t Le = 0;
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_GET_INFO);
	
	if(type == COS_VERSION)
	{
		tpdu_set_p1p2(&command,PRODUCT_INFO&0xFF,0x00);
	}
	else
	{
		tpdu_set_p1p2(&command,type&0xFF,0x00);
	}
	
	//set Le
	switch(type)
	{
		case CHIP_ID:
			Le=0x08;
			break;
		case PRODUCT_INFO:
			Le=0x08;
			break;
		case LOADER_VERSION:
			Le=0x02;
			break;
		case LOADER_FEK_INFO:
			Le=0x08;
			break;
		case LOADER_FVK_INFO:
			Le=0x08;
			break;
		case COS_VERSION:
			Le=0x04;
			break;
	}
	tpdu_set_le(&command, Le);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	if(type==CHIP_ID)
		memcpy(info->chip_id,&queue_out.q_buf[off],queue_out.q_buf_len);
	else if(type==PRODUCT_INFO)
		memcpy(info->product_info,&queue_out.q_buf[off],queue_out.q_buf_len);
	else if(type==LOADER_VERSION)
		memcpy(info->loader_version,&queue_out.q_buf[off],queue_out.q_buf_len);
	else if(type==LOADER_FEK_INFO)
		memcpy(info->loader_FEK_info,&queue_out.q_buf[off],queue_out.q_buf_len);
	else if(type==LOADER_FVK_INFO)
		memcpy(info->loader_FVK_info,&queue_out.q_buf[off],queue_out.q_buf_len);
	else if(type==COS_VERSION)
		memcpy(info->cos_version,&queue_out.q_buf[off],queue_out.q_buf_len);
	return SE_SUCCESS;
}

/**
* @brief Get SE IVD
* @param [in] ivd_type  		ivd type : 1.Get preset IVD value; 2.Make SE Calculate the IVD and output
* @param [out] IVD_value        IVD value 
* @return refer error.h
* @note no
* @see util_queue_init  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute  util_queue_size
*/
se_error_t  apdu_get_IVD (IVD_type ivd_type ,uint8_t *IVD_value)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint32_t Le = 0;
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_GET_INFO);
	tpdu_set_p1p2(&command,ivd_type&0xFF,0x00);
	//set Le
	Le=0x04;
	tpdu_set_le(&command, Le);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(IVD_value,&queue_out.q_buf[off],queue_out.q_buf_len);
	return SE_SUCCESS;
}



/**
* @brief Read SE ID command package
* @param [out] se_id->val      SE ID
* @param [out] se_id->val_len  SE ID length
* @return refer error.h
* @note no
* @see  util_queue_init  tpdu_init_with_id  tpdu_set_le  tpdu_execute  util_queue_size
*/
se_error_t  apdu_get_id (se_id_t *se_id )
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_GET_ID);
	//set Le
	tpdu_set_le(&command, 0x02);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(se_id->val,&queue_out.q_buf[off],queue_out.q_buf_len);
	se_id->val_len=queue_out.q_buf_len;

	return SE_SUCCESS;
}


 /**
* @brief Write SEID
* @param [in] se_id->val  SEID
* @param [in] se_id->val_len  SEID length
* @return refer error.h
* @note no
* @see  util_queue_init  util_queue_rear_push  tpdu_init_with_id  tpdu_execute_no_response
*/
se_error_t  apdu_write_SEID(se_id_t *se_id) 
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
  	uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;
	//deque initialization
	util_queue_init(&queue_in);
	//input data is stored in deque
    util_queue_rear_push(se_id->val,se_id->val_len, &queue_in);
	
	//set command header
	tpdu_init_with_id(&command,CMD_WRITE_SEID);
    //set P1P2
	tpdu_set_p1p2 (&command,p1,p2);
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;

}


/**
* @brief Change and reload the pin
* @param [in] pin->owner 		pin owner type
* @param [in] pin->pin_value 	pin value
* @param [in] pin->pin_len  	pin length 0x06-0x10
* @param [in] if_change_pin 	whether to change pin
* @param [in] in_buf 			input data
* @param [in] in_buf_len 		input data length
* @return refer error.h
* @note no
* @see no
*/
se_error_t apdu_change_reload_pin(pin_t *pin, const uint8_t *in_buf, uint32_t in_buf_len, bool if_change_pin)
{

	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;
	double_queue_node queue_in ={0} ;

	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_CHANGE_RELOAD_PIN);
	//encrypt the new pin value,then store the cipher into deque 
	apdu_pin_encrypt(pin,in_buf,in_buf_len,&queue_in,true);
	//set p1 p2
	if(if_change_pin)
	{
        p1 = 0x01;
		if(pin->owner == ADMIN_PIN)
			p2 = 0x00;
	    if(pin->owner == USER_PIN)
			p2 = 0x01;
	}
	else
	{
		p1 = 0x02;
	}
	tpdu_set_p1p2 (&command,p1,p2);
	
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;
}


/**
* @brief Verify pin 
* @param [in] pin->owner 	 pin owner type
* @param [in] pin->pin_value pin value
* @param [in] pin->pin_len   pin length:0x06-0x10
* @return refer error.h
* @note no
* @see no
*/
se_error_t apdu_verify_pin(pin_t *pin)
{

	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;
	uint8_t sm3_result[32] = {0};
	uint8_t random[16] = {0};
	uint8_t in_buf[16] = {0};
	double_queue_node queue_in ={0} ;

	//Calculate the sm3 digest of PIN
	sm3(pin->pin_value, pin->pin_len, sm3_result);

	//Get random
	ret = apdu_get_random(0x10,random);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to apdu_get_random!\n");
		return ret;
	}

	//Use sm3_result first 16 bytes as SM4 key to encrypt the random
	sm4_crypt_ecb(sm3_result, SM4_ENCRYPT, 0x10, random, in_buf); 

	//deque initialization
	util_queue_init(&queue_in);
	//input data is stored in deque
    util_queue_rear_push((uint8_t *)in_buf,0x10, &queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_VERIFY_PIN);
	//set P1 P2
	if(pin->owner == ADMIN_PIN)
		p2 = 0x00;
    else if(pin->owner == USER_PIN)
		p2 = 0x01;
	
	tpdu_set_p1p2 (&command,p1,p2);
	
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;
}


/**
* @brief Select file command package
* @param [in] fid      file FID
* @return refer error.h	
* @note no
* @see  util_queue_init  util_queue_rear_push  tpdu_init_with_id  tpdu_execute_no_response
*/
se_error_t  apdu_select_file(uint32_t  fid)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	uint8_t in_buf[2] = {0x00};
    uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;
	in_buf[0] = (fid>>8)&0xFF;
	in_buf[1] = (fid)&0xFF;
	//deque initialization
	util_queue_init(&queue_in);
	//input data is stored in deque
    util_queue_rear_push((uint8_t *)in_buf,0x02, &queue_in);
	
	//set command header
	tpdu_init_with_id(&command,CMD_SELECT_FILE);
	//set p1 p2
	tpdu_set_p1p2 (&command,p1,p2);
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;
}



/**
* @brief Write file command package
* @param [in] offset 	  file offset
* @param [in] if_encrypt  whether to encrypt the input data
* @param [in] in_buf      input data
* @param [in] in_buf_len  input data length
* @return refer error.h	
* @note no
* @see util_queue_init  util_queue_rear_push  tpdu_init_with_id  tpdu_execute_no_response
*/
se_error_t  apdu_write_file(uint32_t offset, bool  if_encrypt, const uint8_t *in_buf, uint32_t in_buf_len)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	//uint8_t in_buf[2] = {0x00};
	uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;

	p1 = (offset>>8)&0xff;
	p2 = (offset)&0xff;
	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_WRITE_FILE);
	//set p1 p2
	tpdu_set_p1p2 (&command,p1,p2);
	//if the transporting is protected by cipher, encrypt the input data.
	if(if_encrypt==true)
	{
		ret =  apdu_trans_encrypt((uint8_t *)in_buf, in_buf_len,0,&queue_in);//encrypt the data by tranport key
		//set CLA
		tpdu_set_cla (&command,CMD_CLA_CASE2);

	}
    else
	{
	    //input data is stored in deque
	    util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);
	    //set CLA
	    tpdu_set_cla (&command,CMD_CLA_CASE1);
    }

    if((queue_in.q_buf_len)>0xff)
	{
	  command.isoCase = ISO_CASE_3E;
	}
	else
	{
      command.isoCase = ISO_CASE_3S;
	} 
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
	if(ret!=SE_SUCCESS)
		return ret;

	return SE_SUCCESS;
}




/**
* @brief Read file command package
* @param [in] offset 		file offset
* @param [in] if_encrypt    whether to encrypt the output data
* @param [out] out_buf      output data
* @param [out] out_buf_len  output data length
* @return refer error.h	
* @note no
* @see util_queue_init  util_queue_rear_push  tpdu_init_with_id  tpdu_execute_no_response
*/
se_error_t  apdu_read_file(uint32_t offset, bool  if_encrypt, uint32_t expect_len ,uint8_t *out_buf, uint32_t *out_buf_len)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
	uint8_t p1 = 0x00;
	uint8_t p2 = 0x00;
    uint32_t off = 0;
	uint32_t le = 0;
	uint32_t temp_len = 0;
	
	p1 = (offset>>8)&0xff;
	p2 = (offset)&0xff;
	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_READ_FILE); 
	//if the transporting is protected by cipher, encrypt the output data.
	if(if_encrypt)
	{
		//set CLA
		tpdu_set_cla (&command,CMD_CLA_CASE2);
		
	}
    else
	{
	    //set CLA
	    tpdu_set_cla (&command,CMD_CLA_CASE1);
    }
	//set P1 P2
	tpdu_set_p1p2 (&command,p1,p2);
	//le
	if(if_encrypt)
		le = ((expect_len+1+16)/16)*16;
	else le = expect_len;
	tpdu_set_le(&command, le);
	
	command.isoCase = ISO_CASE_2E;
	
    ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	if(if_encrypt==true)
	{
	  ret =  apdu_trans_decrypt(&queue_out);
	  if(ret!=SE_SUCCESS)
		return ret;  
	  off = queue_out.front_node;
	  memcpy(out_buf,&queue_out.q_buf[off+2],expect_len);
	  *out_buf_len = expect_len;  
	}
	else
	{
	   off = queue_out.front_node;
	   memcpy(out_buf,&queue_out.q_buf[off],temp_len);
	  *out_buf_len = temp_len;    
	}

	
	return SE_SUCCESS;
}


/**
* @brief Get file info command package
* @param [in] if_df 		whether to get DF info
* @param [out] out_buf      output data 
* @param [out] out_buf_len  output data length
* @return refer error.h	
* @note no
* @see util_queue_init  util_queue_rear_push  tpdu_init_with_id  tpdu_execute_no_response
*/
se_error_t  apdu_get_file_info(bool  if_df,uint8_t *out_buf, uint32_t *out_buf_len)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    uint32_t off = 0;
	uint32_t temp_len = 0;
	
	//deque initialization
	util_queue_init(&queue_in);
	//set command header
	tpdu_init_with_id(&command,CMD_GET_FILE_INFO); 	
	//set P1 P2
	if(if_df)
	  tpdu_set_p1p2 (&command,0x00,0x00);
    else
	  tpdu_set_p1p2 (&command,0x01,0x00);
	//le
	tpdu_set_le(&command, 0x00);
	
	
    ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &temp_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(out_buf,&queue_out.q_buf[off],temp_len);
	*out_buf_len = temp_len;    
			
	return SE_SUCCESS;
}



/**
* @brief loader initialization command package
* @param [in] image_addr upgrade data address
* @return refer error.h
* @note no
* @see util_queue_init  tpdu_init_with_id  tpdu_set_p1p2  tpdu_execute_no_response  util_queue_size
*/
se_error_t apdu_loader_init(uint8_t* image_addr)
{
	se_error_t ret = SE_SUCCESS;
	iso_command_apdu_t command = {0};
	double_queue_node queue_in ={0} ;
	uint16_t cmd_len = 0;
	uint16_t off = 0;
  	uint8_t mac_buf[4]= {0};
	//check parameters
	if(image_addr==NULL)
	{
		LOGE("failed to apdu_loader_init pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//get command length
	cmd_len = image_addr[off]<<8 | image_addr[off+1];
	off+=2;
	if(cmd_len!=0x0009)
	{
		LOGE("failed to apdu_loader_init input length params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(memcmp(image_addr+off,"\xBF\x41\x00",3))
	{
		LOGE("failed to apdu_loader_init input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	//deque initialization
	util_queue_init(&queue_in);
	
	//set command header
	tpdu_init_with_id(&command,CMD_LOADER_INIT);
	//get out 4 bytes MAC value
	memcpy(mac_buf,image_addr+off+5 ,4); 
	//store the  4 bytes MAC value into deque
	//input data is stored in deque
	util_queue_rear_push(mac_buf,4, &queue_in);
	//set p2
	tpdu_set_p1p2 (&command,0x00,image_addr[off+3]);  
	
	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size(&queue_in));
	
	return ret;
}


/**
* @brief loader download
* @param [in] image_addr upgrade data address
* @return refer error.h
* @note no
* @see util_queue_init  tpdu_init_with_id  util_queue_rear_push  tpdu_execute_no_response  util_queue_size
*/
se_error_t apdu_loader_program(uint8_t* image_addr)
{
	se_error_t ret = SE_SUCCESS;
	iso_command_apdu_t command = {0};
	double_queue_node queue_in ={0} ;
	uint16_t cmd_len = 0;
	uint16_t off = 0;

	//check parameters
	if(image_addr==NULL)
	{
		LOGE("failed to apdu_loader_download pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//get command length
	cmd_len = (uint16_t)(image_addr[off]<<8 | image_addr[off+1]);
	off+=2;
	
	if(cmd_len>0x2007)
	{
		LOGE("failed to apdu_loader_download input length params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	//deque initialization
	util_queue_init(&queue_in);
	
	//set command header
	tpdu_init_with_id(&command,CMD_LOADER_PROGRAM);
	memcpy(&(command.classbyte), image_addr+off, 5);
	//tpdu_set_cla (&command,image_addr[off++]);
	//tpdu_set_p1p2 (&command,image_addr[off],image_addr[off+1]);
	
	off+=5;
	
	if(cmd_len > 0x104)//if the command length is 0x104, the length of Lc is 3 bytes
	{
		command.isoCase = ISO_CASE_3E;
		off+=2;
	}
	else
	{
		command.isoCase = ISO_CASE_3S;
	}
	//input data is stored in deque
	//util_queue_rear_push(image_addr+off,cmd_len-5, &queue_in);
	
	util_queue_rear_push(image_addr+off,cmd_len-off+2, &queue_in);

	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size(&queue_in));

	return ret;
}



/**
* @brief loader check data
* @param [in] image_addr upgrade data address
* @return refer error.h
* @note no
* @see util_queue_init  tpdu_init_with_id  util_queue_rear_push  tpdu_execute_no_response  util_queue_size
*/
se_error_t apdu_loader_checkprogram(uint8_t* image_addr)
{
	se_error_t ret = SE_SUCCESS;
	iso_command_apdu_t command = {0};
	double_queue_node queue_in ={0} ;
	uint16_t cmd_len = 0;
	uint16_t off = 0;

	//check parameters
	if(image_addr==NULL)
	{
		LOGE("failed to apdu_loader_checkdata pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//get command length
	cmd_len = (uint16_t)(image_addr[off]<<8 | image_addr[off+1]);
	off+=2;
	
	if((cmd_len!=0x0065) && (cmd_len!=0x0025))
	{
		LOGE("failed to apdu_loader_checkdata input length params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(cmd_len==0x0065)
	{	
		if(memcmp(image_addr+off,"\xBF\x42\x00\x00\x60",5)) 
		{
			LOGE("failed to apdu_loader_checkdata input params!\n");
			return SE_ERR_PARAM_INVALID;
		}		
	}
	else if(cmd_len==0x0025)
	{
		if(memcmp(image_addr+off,"\xBF\x42\x00\x00\x20",5)) 
		{
			LOGE("failed to apdu_loader_checkdata input params!\n");
			return SE_ERR_PARAM_INVALID;
		}	
	}

	
	//deque initialization
	util_queue_init(&queue_in);
	
	//set command header
	tpdu_init_with_id(&command,CMD_LOADER_CHECKPROGRAM);

	//input data is stored in deque
	util_queue_rear_push(image_addr+off+5,cmd_len-5, &queue_in);

	ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size(&queue_in));

	return ret;
}

/**
* @brief V2X GENERATE KEY DERIVE SEED
* @param [in]  derive_type  derive type:the default value is 0x00
* @param [out] out_buf      derive data information
* @note no
* @see  util_queue_init   tpdu_init_with_id  tpdu_set_le  tpdu_execute  util_queue_size
*/
se_error_t  apdu_v2x_gen_key_derive_seed (uint8_t derive_type,derive_seed_t* out_buf)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_V2X_GENERATE_KEY_DERIVE_SEED);
	//set le
	tpdu_set_le(&command, 0xA0);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(out_buf->SM4_KS,&queue_out.q_buf[off],16);
	memcpy(out_buf->SM4_KE,&queue_out.q_buf[off + 16],16);
	memcpy(out_buf->SM2_A,&queue_out.q_buf[off + 32],64);
	memcpy(out_buf->SM2_P,&queue_out.q_buf[off + 96],64);

	return SE_SUCCESS;
}



/**
* @brief V2X RECONSITUTION KEY
* @param [in]  in_buf      reconstruct data info(private KID-1byte|i-4byte|j-4B|CTij)
* @param [in]  in_buf_len  reconstruct data info length
* @param [out] out_buf     pseudonym certificate
* @param [out] out_buf_len pseudonym certificate length
* @return refer error.h
* @note no
* @see  util_queue_init   tpdu_init_with_id  tpdu_set_le  tpdu_execute  util_queue_size
*/
se_error_t  apdu_v2x_reconsitution_key (uint8_t *in_buf, uint32_t *in_buf_len,uint8_t *out_buf, uint32_t *out_buf_len)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	uint8_t p1 = 0;
	uint8_t p2 = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_V2X_RECONSITUTION_KEY);
	//set le
	tpdu_set_le(&command, 0x00);
	//set the p2 according to the private KID
	memcpy(&p2, in_buf, 1);
	tpdu_set_p1p2 (&command,p1,p2);
	//input data is stored in deque
	util_queue_rear_push(in_buf+1,*in_buf_len-1, &queue_in);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;
  
	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(out_buf,&queue_out.q_buf[off],out_len);
	*out_buf_len = out_len;
	
	return SE_SUCCESS;
}

/**
* @brief V2X GET KEY DERIVE SEED
* @param [out] out_buf    derive data information(can refer derive_seed_t)
* @note no
* @see  util_queue_init   tpdu_init_with_id  tpdu_set_le  tpdu_execute  util_queue_size
*/
se_error_t  apdu_v2x_get_derive_seed (derive_seed_t* out_buf)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_V2X_GET_KEY_DERIVE_SEED);
	//set le
	tpdu_set_le(&command, 0xA0);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;

  //copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(out_buf->SM4_KS,&queue_out.q_buf[off],16);
	memcpy(out_buf->SM4_KE,&queue_out.q_buf[off + 16],16);
	memcpy(out_buf->SM2_A,&queue_out.q_buf[off + 32],64);
	memcpy(out_buf->SM2_P,&queue_out.q_buf[off + 96],64);

	return SE_SUCCESS;
}

/**
* @brief PRIVATE KEY TRANSFORMATION
* @param [in]  input_blod_key_id         the private key id for k stored in SE: fixed key ID : 00-0xEF, can not be exported; temporary key ID: 0xF0-0xFE, can be exported
* @param [in]  output_blod_key_id        the private key id for k' to store into SE: fixed key ID : 00-0xEF, can not be exported; temporary key ID: 0xF0-0xFE, can be exported
* @param [in]  curve_type     curve type (curve type only support SM2 right now)     
* @param [in]  key_multiplier a
* @param [in]  key_addend     b
* @param [out] pubkey->value         the public key corresponding the k'
* @param [out] pubkey->value_len     the public key length corresponding the k'
* @return refer util_error.h
* @note 
* @see apdu_private_key_multiply_add
*/
se_error_t apdu_private_key_multiply_add (uint8_t input_blod_key_id, uint8_t output_blod_key_id, enum ecc_curve curve_type, key_multiplier_t key_multiplier, key_addend_t key_addend,  pub_key_t* pubkey)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	uint8_t data_in[66] = {0}; 
	uint32_t data_in_len = 66;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;
    
	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	//set command header
	tpdu_init_with_id(&command,CMD_PRIVATE_KEY_TRANSFORMATION);
	//set le
	tpdu_set_le(&command, 0x00);
  //set the APDU data region : the private kid to transform(1 byte, between 0x00~0xFF) | the private kid have been transformed (1 byte, between 0x00~0xFF) | multiplier a(32 bytes) | addend b(32 bytes)
	data_in[0] = input_blod_key_id;
	data_in[1] = output_blod_key_id;
	memcpy(&data_in[2], &(key_multiplier.val[0]), key_multiplier.val_len);
	memcpy(&data_in[34], &(key_addend.val[0]), key_addend.val_len);
	//input data is stored in deque
	util_queue_rear_push(data_in,data_in_len, &queue_in);
	ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
	if(ret!=SE_SUCCESS)
		return ret;
  
	//copy back the output data from the deque
	off = queue_out.front_node;
	memcpy(&(pubkey->val[0]),&queue_out.q_buf[off],out_len);
	pubkey->val_len = out_len;
	
	return SE_SUCCESS;
}

/**
* @brief MANAGE EM FLAG CMD package
* @param [in]  pub_kid      ED25519 pubkey KID, just effective for updating
* @param [in]  inbuf   input data,just effective for updating
* @param [in]  inbuf_len   input data length, just effective for updating
* @param [in]  if_updata   whether to update
* @param [out] EM_flag     1 byte EM_flag vlue, just effective for reading
* @return refer error.h
* @note no
* @see util_queue_init   tpdu_init_with_id  tpdu_set_p1p2 tpdu_set_le  tpdu_execute  tpdu_execute_no_response util_queue_size
*/
se_error_t  apdu_manage_EM_flag (uint8_t pub_kid, uint8_t * inbuf, uint32_t  inbuf_len, bool if_updata, uint8_t * EM_flag)
{
	iso_command_apdu_t command = {0};
	se_error_t ret = 0;
	uint32_t out_len = 0;
	uint32_t off = 0;
	uint8_t p1 = 0;
	uint8_t p2 = 0;
	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;

	//deque initialization
	util_queue_init(&queue_in);
	util_queue_init(&queue_out);
	
    //set command header
	tpdu_init_with_id(&command,CMD_MANAGE_EM_FLAG);

	if(if_updata == false)
	{
		//set p1 p2
		tpdu_set_p1p2 (&command,0x01,0x00);
		//set le
		tpdu_set_le(&command, 0x01);
		//set APDU type
		command.isoCase = ISO_CASE_2S;

		ret = tpdu_execute(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in), (uint8_t *)&queue_out, &out_len);
		if(ret!=SE_SUCCESS)
			return ret;

		//copy back the output data from the deque
		off = queue_out.front_node;
		memcpy(EM_flag,&queue_out.q_buf[off],queue_out.q_buf_len);
		
	}
	else if(if_updata == true)
	{
		//set p1 p2
		tpdu_set_p1p2 (&command,0x00,pub_kid);
		//set APDU type
		command.isoCase = ISO_CASE_3S;
		//input data is stored in deque
    	util_queue_rear_push(inbuf,inbuf_len, &queue_in);

		ret = tpdu_execute_no_response(&command, (uint8_t *)&queue_in, util_queue_size (&queue_in));
		if(ret!=SE_SUCCESS)
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


