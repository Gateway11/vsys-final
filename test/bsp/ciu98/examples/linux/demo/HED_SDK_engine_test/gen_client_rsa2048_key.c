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
#include <openssl/engine.h>
#include <openssl/ossl_typ.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include "se.h"


//openssl name
#define ENGINE_NAME "hed_engine"


void load_hed_engine(ENGINE  *e)
{
	//load and init openssl engine
	ENGINE_load_builtin_engines();
	e = ENGINE_by_id(ENGINE_NAME);
	if(!e)
	{
		printf("loading Engine failed!!\n");
	}
	printf("Engine ID : %s\n",ENGINE_get_id(e));

	if(!ENGINE_init(e))
	{
		printf("Init hed Engine failed!!\n");
	}
	printf("hed engine init Ok\n");

	if(!ENGINE_set_default(e, ENGINE_METHOD_ALL))
	{
		printf("Hed Engine failede!\n");
	}
	printf("hed engine setting Ok\n");
}


int main(int argc, char * argv[])
{
    int ret = SE_SUCCESS;
	RSA *rsaKey = NULL;
	ENGINE  *e = NULL;
	BIGNUM *bne;
	FILE *file;

	load_hed_engine(e);
	/* call the RSA_generate_key_ex */
	bne=BN_new();
	ret=BN_set_word(bne,RSA_F4);
	rsaKey=RSA_new();
	RSA_generate_key_ex(rsaKey, 2048, bne, NULL);
	
	if((file = fopen("./client_public_key.pem","wb")) == NULL)
	{
		printf("\nopen file falied\n");
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}
	ret =  PEM_write_RSA_PUBKEY(file, rsaKey);
	fclose(file);

    return ret;
} 
