/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		api_generate_shared_key_test.c
 Author:			  liangww
 Version:			  V1.0
 Date:			    2021-06-25
 Description:	  Main program body
 History:

******************************************************************************/

/***************************************************************************
 * Include Header Files
 ***************************************************************************/

#include "se.h"
#include "comm.h"
#include "pin.h"
#include "soft_alg.h"
#include "string2byte.h"
#include "key.h"
#include "crypto.h"
#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
void helpmenu(void)
{
	port_printf("\nHelp menu: api_generate_shared_key_test <option> ...<option>\n");
	port_printf("option:- \n");
	port_printf("-a <alg>          : alg type: ALG_ECDH_ECC256 : 0x00 \n");
	port_printf("                             ALG_ECDH_SM2    : 0x01 \n");
	port_printf("                             ALG_SM2_SM2     : 0x02 \n");
	port_printf("-k <private kid>  : The private kid just for ALG_ECDH_ECC256/ALG_ECDH_SM2   \n");
	port_printf("-i <input file>   : The input data \n");
	port_printf("-l <input length> : The input data length \n");
	port_printf("-h                : Print this help \n");
}

static uint32_t trans_int(const char *aArg)
{
	uint32_t value;

	if (strncmp(aArg, "0x", 2) == 0)
		sscanf(aArg, "%x", &value);
	else
		sscanf(aArg, "%d", &value);

	return value;
}

int main(int argc, char *argv[])
{
	uint8_t com_outbuf[300] = {0};
	uint8_t in_buf[500] = {0};
	uint8_t out_buf[500] = {0};
	uint32_t in_buf_len = 0;
	uint32_t out_buf_len = 0;
	uint32_t outlen = 0;
	se_error_t ret = 0;
	uint32_t i = 0;
	uint32_t k_flag = 0;
	uint32_t i_flag = 0;
	uint32_t l_flag = 0;
	uint32_t a_flag = 0;
	uint32_t if_encryto = 0;
	int ch;
	uint8_t trankey_val[16] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};
	sym_key_t trankey = {0};
	uint8_t pin_buf[16] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
	pin_t pin = {0};
	uint8_t random[16] = {0};
	// uint8_t key_buf[16]={0};
	uint8_t pri_id;
	uint8_t pub_id;
	uint8_t kid;
	pub_key_t pub_key = {0};
	pri_key_t pri_key = {0};
	uint8_t alg_type;
	char *input_file = NULL;
	char file_in_buf[400];
	uint32_t len;
	FILE *fp;
	uint8_t input[400] = {0};
	uint32_t input_len = 0;
	unikey_t shared_key = {0};
	uint8_t sm2_s[64] = {0};
	uint8_t fixd_pub_key[64];
	bool if_return_key = true;
	bool if_return_s = true;
	uint32_t s_key_info_len = 0;
	uint8_t input_tmp[128] = {0};
	// MCU initialization
	port_mcu_init();

	LOGI("test begin\n");

	if (argc < 2)
	{
		port_printf("\n");
		port_printf("--------------------------\n");
		helpmenu();
		exit(0);
	}

	while ((ch = getopt(argc, argv, "a:k:l:i:")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a_flag = 1;
			alg_type = trans_int(optarg);
			break;
		case 'k':
			k_flag = 1;
			kid = trans_int(optarg);
			if (kid > 0xFF)
			{
				port_printf("The key id must between 0x00 ~ 0xFF\n");
				exit(0);
			}
			break;

		case 'i':
			i_flag = 2;
			input_file = optarg;
			fp = fopen((const char *)optarg, "rb");
			if (!fp)
			{
				port_printf("can not open the file\n");
				return -1;
			}

			fread(file_in_buf, 1, len * 2, fp);

			fclose(fp);
			break;
		case 'l':
			l_flag = 1;
			len = trans_int(optarg);
			break;
		case 'h':
		default:
			helpmenu();
			exit(0);
			break;
		}
	}

	/****************************************SE Register, SE Select, SE Connect and transport key setting*****************************************/

	//---- 1.  Register SE----
	ret = api_register(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if (ret != SE_SUCCESS)
	{
		LOGE("failed to spi api_register\n");
		return ret;
	}

	//---- 2.  Select SE ----
	ret = api_select(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if (ret != SE_SUCCESS)
	{
		LOGE("failed to spi api_select\n");
		return ret;
	}

#ifndef CONNECT_NEED_AUTH

	ret = api_connect(com_outbuf, &outlen);
	if (ret != SE_SUCCESS)
	{
		LOGE("failed to api_connect\n");
		return ret;
	}
#endif

#ifdef CONNECT_NEED_AUTH
	uint8_t e_key_bin[16] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};
	sym_key_t e_key = {0};

	e_key.val_len = 0x10;
	e_key.alg = ALG_SM4;
	memcpy(e_key.val, e_key_bin, 0x10);
	ret = api_connect_auth(&e_key, com_outbuf, &outlen);
	if (ret != SE_SUCCESS)
	{
		LOGE("failed to api_connect_auth\n");
		return ret;
	}
#endif

	//---- 4. Set transport key ----
	memcpy(trankey.val, trankey_val, 16);
	trankey.val_len = 16;
	trankey.id = 0x02;
	pin.owner = ADMIN_PIN;
	pin.pin_len = 0x10;
	memcpy(pin.pin_value, pin_buf, pin.pin_len);
	ret = api_verify_pin(&pin); // verify the admin pin
	if (ret != SE_SUCCESS)
	{
		LOGE("failed to api_verify_pin\n");
		return ret;
	}

	ret = api_set_transkey(&trankey); // set the transport key

	if (ret != SE_SUCCESS)
	{
		LOGE("failed to api_set_transkey\n");
		return ret;
	}
	/****************************************api_generate_shared_key*****************************************/
	if ((a_flag == 1) && (i_flag == 2) && (l_flag == 1))
	{
		if ((alg_type != ALG_SM2_SM2) && (k_flag == 1)) // ECDH
		{
			shared_key.alg = alg_type;
			shared_key.id = kid;
			// strcpy(s_key_info,"D17ED15EDFFFFBC6DB023E1F4E4B9A45C590A816F8CAE301633940E483791728C85E5C328BC059F7A30496B846B5932E7E3E1D34AB53B3CBFF1D98954EFDF511");
			s_key_info_len = 2 * len;
			StringToByte(file_in_buf, input, s_key_info_len);
			input_len = len;
		}

		else if (alg_type == ALG_SM2_SM2) // SM2 ECDH
		{
			// Generate a temporary public and private key pair
			pub_key.alg = ALG_SM2;
			pub_key.id = 0xF0;
			pri_key.id = 0xF1;
			ret = api_generate_keypair(&pub_key, &pri_key);
			if (ret != SE_SUCCESS)
			{
				LOGE("failed to generate_keypair_test\n");
				return ret;
			}
			// 00:SE is the responder 01:SE is the initiator
			input[0] = 0x01;
			// kid
			input[1] = 0xF7;
			// algorithm type
			input[2] = ALG_SM4;
			// Temporary public and private keys, fixed public and private keys
			memcpy(input + 3, "\xF0\xF1\x0F\x10", 4);
			// Counterparty temporary public key
			s_key_info_len = 2 * len;
			StringToByte(file_in_buf, input_tmp, s_key_info_len);
			memcpy(input + 7, input_tmp, 64);
			// Counterparty fixed public key
			// strcpy(s_key_info,"160E12897DF4EDB61DD812FEB96748FBD3CCF4FFE26AA6F6DB9540AF49C942324A7DAD08BB9A459531694BEB20AA489D6649975E1BFCF8C4741B78B4B223007F");
			// s_key_info_len = 2*len;
			// StringToByte(file_in_buf,input+71,s_key_info_len);
			// input_len = len;
			memcpy(input + 71, input_tmp + 64, 64);
			memcpy(input + 135, "\x01\xAA\x01\xBB", 4);
			input_len = 139;
			shared_key.alg = ALG_SM2_SM2;
		}

		ret = api_generate_shared_key(input, input_len, &shared_key, sm2_s, if_return_key, if_return_s);
		if (ret != SE_SUCCESS)
		{
			LOGE("failed to api_generate_shared_key\n");
			return ret;
		}

		port_printf("\n\n**************************************\n");
		switch (alg_type)
		{
		case ALG_ECDH_ECC256:
			port_printf("the alg is:%s\n", "ALG_ECDH_ECC256");
			port_printf("The ECDH_ECC input public key data length:%d\n", len);
			port_printf("The ECDH_ECC input public key data :%s\n", file_in_buf);
			break;
		case ALG_SM2_SM2:
			port_printf("the alg is:%s\n", "ALG_SM2_SM2");
			port_printf("The SM2_SHARED input temp public key data length:%d\n", 64);
			port_printf("The SM2_SHARED input temp public key data :\n");
			for (i = 0; i < 64; i++)
			{
				port_printf("%02x", input_tmp[i]);
			}
			port_printf("\nThe SM2_SHARED input fixede public key data length:%d\n", 64);
			port_printf("The SM2_SHARED input fixed public key data :\n");
			for (i = 64; i < 128; i++)
			{
				port_printf("%02x", input_tmp[i]);
			}
			break;
		case ALG_ECDH_SM2:
			port_printf("the alg is:%s\n", "ALG_ECDH_SM2");
			port_printf("The ECDH_SM2 input public key data length:%d\n", len);
			port_printf("The ECDH_SM2 input public key data :%s\n", file_in_buf);
			break;
		}

		port_printf("\napi_generate_shared_key:");
		for (i = 0; i < shared_key.val_len; i++)
		{
			port_printf("%02x", shared_key.val[i]);
		}
		if (alg_type == ALG_SM2_SM2)
		{
			port_printf("\nthe sm2_s:");
			for (i = 0; i < 64; i++)
			{
				port_printf("%02x", sm2_s[i]);
			}
		}
		port_printf("\n\n**************************************\n");
	}
	else
	{
		port_printf("\n\n**************************************\n");
		port_printf("\n\nthe input parameter is error!\n");
		port_printf("\n\n**************************************\n");
	}

	//---- 4. close SE ----
	ret = api_disconnect();
	if (ret != SE_SUCCESS)
	{
		LOGE("failed to api_disconnect\n");
		return ret;
	}

	LOGI("test end\n");

	return SE_SUCCESS;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
