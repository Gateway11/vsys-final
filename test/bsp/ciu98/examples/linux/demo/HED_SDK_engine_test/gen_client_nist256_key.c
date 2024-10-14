/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		engine_test.c
 Author:			  liangww 
 Version:			  V1.0	
 Date:			    2022-12-12	
 Description:	  Main program body
 History:		

******************************************************************************/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/ossl_typ.h>
#include <openssl/x509.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include "se.h"
#include "key.h"
#include "comm.h"
#include "pin.h"

int main(int argc, char * argv[])
{
    se_error_t ret = 0;
	FILE *file;
	BIGNUM *x = NULL;
    BIGNUM *y = NULL;
    EC_KEY *eckey = NULL;
    pub_key_t pub_key={0};
    pri_key_t pri_key={0};
    int ok = 0;
    BN_CTX *ctx = NULL;
    uint8_t com_outbuf[300] = {0};
    uint32_t outlen = 0;
    uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
    pin_t pin={0};

    if ((ctx = BN_CTX_new()) == NULL)
    goto err;
    x = BN_CTX_get(ctx);
    y = BN_CTX_get(ctx);
    // eckey = EC_KEY_new();
    eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

	//---- 1. Register SE----
	ret = api_register(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi api_register\n");
		 return ret;
	}
	
	//---- 2. Select SE ----
	ret = api_select(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi api_select\n");
		 return ret;
	}


#ifndef CONNECT_NEED_AUTH
		//---- 3.connect and obtain the ATR ----
		ret = api_connect(com_outbuf, &outlen);
		if(ret!=SE_SUCCESS)
		{		
			 LOGE("failed to api_connect\n");
			 return ret;
		}
#endif

	
#ifdef CONNECT_NEED_AUTH
		uint8_t e_key_bin[16]={0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
		sym_key_t e_key = {0};
		//---- 3. connect and obtain the ATR ----
		e_key.val_len = 0x10;
		e_key.alg = ALG_SM4;
		memcpy(e_key.val, e_key_bin, 0x10); 
		ret = api_connect_auth(&e_key, com_outbuf, &outlen);
		if(ret!=SE_SUCCESS)
		{	   
			LOGE("failed to api_connect_auth\n");
			return ret;
		}
#endif
        pin.owner = ADMIN_PIN;
        pin.pin_len = 0x10;
        memcpy(pin.pin_value, pin_buf,pin.pin_len);
        ret = api_verify_pin(&pin);//verify the admin pin 
        if(ret!=SE_SUCCESS)
        { 	  
            LOGE("failed to api_connect\n");
            return ret;
        }

	if((file = fopen("./clientpub.pem","wb")) == NULL)
	{
		printf("\nopen file falied\n");

		return -1;
	}
	pub_key.alg = ALG_ECC256_NIST;
    pub_key.id  = 0x11;
    pri_key.id  = 0x11;
    ret = api_generate_keypair (&pub_key, &pri_key);
    if ( ret != SE_SUCCESS)
    {
      LOGE("failed to generate_keypair_test\n");
      return ret;
    }
    if(!BN_bin2bn(pub_key.val,32,x))
    {
      goto err;
    }
     if(!BN_bin2bn(pub_key.val+32,32,y))
    {
      goto err;
    }
     if(!EC_KEY_set_public_key_affine_coordinates(eckey, x, y))
    {
      goto err;
    } 
    if(!PEM_write_EC_PUBKEY(file,eckey))
    {
        goto err;
    }

    
    ok = 1;

 err:
    BN_CTX_free(ctx);
    EC_KEY_free(eckey);
    fclose(file);
    if(ok == 1)
        LOGI("hed ECC keygen success\n");
    return ok;
	

    return ret;
} 
