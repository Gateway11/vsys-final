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
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include "se.h"

int engine_test_sign_verify_ECC(void); 
int engine_test_ECDH(void); 
int engine_test_encrypt_decrypt_RSA(void);
int engine_test_encrypt_decrypt_RSA_EVP(void);
int engine_test_sign_verify_RSA(void);
int engine_test_sign_verify_RSA_EVP(void);


#define ENGINE_NAME "hed_engine"

int main(int argc, char * argv[])
{
    int ret = SE_SUCCESS;

	ret = engine_test_sign_verify_ECC(); 
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi engine_test_sign_verify_ECC\n");
		 return ret;
	}

	ret = engine_test_ECDH();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi engine_test_ECDH\n");
		 return ret;
	}

    ret = engine_test_encrypt_decrypt_RSA();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi engine_test_encrypt_decrypt_RSA\n");
		 return ret;
	}

    ret = engine_test_encrypt_decrypt_RSA_EVP();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi engine_test_encrypt_decrypt_RSA_EVP\n");
		 return ret;
	}

	ret = engine_test_sign_verify_RSA();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi engine_test_sign_verify_RSA\n");
		 return ret;
	}

	ret = engine_test_sign_verify_RSA_EVP();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi engine_test_sign_verify_ECC\n");
		 return ret;
	}

	LOGI("engine_test end!\n");

    return ret;
} 

void load_hed_engine(ENGINE  *e)
{
	
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

int engine_test_sign_verify_ECC(void)
{
	int ret = SE_SUCCESS;
	unsigned char dgst_buf[32] = {0};
	unsigned char sig_buf[100] = {0};
	ENGINE  *e = NULL;
	int sigbuf_len = 0;
	EC_KEY *ecckey =NULL;
	FILE *file;

	load_hed_engine(e);

	ecckey = EC_KEY_new();
	if(1 != ECDSA_sign(0, dgst_buf, 32, sig_buf, &sigbuf_len, ecckey))//Ê¹ÓÃKID 0x11Ç©Ãû
	{
		EC_KEY_free(ecckey);
		ENGINE_free(e);
		return -1;
	}
	// if((file = fopen("./clientpub.pem","rb")) == NULL)
	// {
	// 	printf("\nopen file falied\n");
	// 	EC_KEY_free(ecckey);
	// 	ENGINE_free(e);
	// 	return -1;
	// }
	// ecckey =  PEM_read_EC_PUBKEY(file, NULL, NULL, NULL);
	// fclose(file);
	// if(1 != ECDSA_verify(0, dgst_buf, 32, sig_buf, sigbuf_len, ecckey))
	// {
	// 	EC_KEY_free(ecckey);
	// 	ENGINE_free(e);
	// 	return -1;
	// }

	EC_KEY_free(ecckey);
	ENGINE_free(e);
	return SE_SUCCESS;
}

int engine_test_ECDH(void)
{
	EC_KEY *ecckey1 =NULL;
	EC_KEY *ecckey2 =NULL;
	int ret = -1;
	ENGINE  *e = NULL;
	unsigned char shared_buf[32] = {0};
	int shared_buf_expect_len = 32;
	int shared_buf_out_len = 0;

	ecckey1 = EC_KEY_new();
	ecckey1 = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	ret = EC_KEY_generate_key(ecckey1);
	if(ret != 1)
	{
		LOGE("EC_KEY_generate_key falied\n");
		EC_KEY_free(ecckey1);
		return -1;
	}
	const EC_POINT *ecckey1_pub_point = EC_KEY_get0_public_key(ecckey1);
	
	load_hed_engine(e);

	ecckey2 = EC_KEY_new();
	ecckey2 = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	ret = EC_KEY_generate_key(ecckey2);
	if(ret != 1)
	{
		LOGE("EC_KEY_generate_key falied\n");
		EC_KEY_free(ecckey2);
		ENGINE_free(e);
		return -1;
	}

	shared_buf_out_len = ECDH_compute_key(shared_buf, shared_buf_expect_len,ecckey1_pub_point, ecckey2, NULL);
	if(shared_buf_out_len < 0)
	{
		LOGE("ECDH_compute_key falied\n");
		EC_KEY_free(ecckey2);
		ENGINE_free(e);
		return -1;
	}

	printf("the ECDH result:\n");
	for(int i=0;i<shared_buf_out_len;i++)
	{
		printf("%02X",shared_buf[i]);
	}
	printf("\n");

	EC_KEY_free(ecckey1);
	EC_KEY_free(ecckey2);
	ENGINE_free(e);
	return 0;
}

int engine_test_encrypt_decrypt_RSA(void)
{
    RSA *rsaKey = NULL;
	int ret = -1;
	ENGINE  *e = NULL;
	BIGNUM *bne;
	unsigned char inData[] = "helloworld";
	int  inlen = sizeof(inData);
	unsigned char plainData[256] = { 0 };
	unsigned char encData[256] = { 0 };
    FILE *file;
    int  inlen_2 = 256; 
    unsigned char inData_2[256] = { 0 };
    unsigned char plainData_2[256] = { 0 };
	unsigned char encData_2[256] = { 0 };

	load_hed_engine(e);
	
	bne=BN_new();
	ret=BN_set_word(bne,RSA_F4);
	rsaKey=RSA_new();
	RSA_generate_key_ex(rsaKey, 2048, bne, NULL);
	
	if((file = fopen("./rsa_public_key.pem","wb")) == NULL)
	{
		printf("\nopen file falied\n");
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}
	ret =  PEM_write_RSA_PUBKEY(file, rsaKey);
	fclose(file);
	
	/* get model length */
	int size = RSA_size(rsaKey);

	/* pub encrypt */
	ret = RSA_public_encrypt(inlen, inData, encData, rsaKey, RSA_SSLV23_PADDING);
	if (ret == -1) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}

	ret = RSA_private_decrypt(size, encData, plainData, rsaKey, RSA_SSLV23_PADDING);
	if (ret == -1) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}

	if (ret != inlen) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}
	if(memcmp(inData, plainData, inlen) != 0) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}

    memcpy( inData_2, encData, 256);
  
    ret = RSA_private_encrypt(inlen_2, inData_2, encData_2, rsaKey, RSA_NO_PADDING);
	if (ret == -1) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}


    ret = RSA_public_decrypt(256, encData_2, plainData_2, rsaKey, RSA_NO_PADDING);
	if (ret == -1) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}

  
	if (ret != inlen_2) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}
	if(memcmp(inData_2, plainData_2, inlen_2) != 0) {
		RSA_free(rsaKey);
		ENGINE_free(e);
		return -1;
	}

	printf("engine_test_encrypt_decrypt_RSA success!!!\n");
	RSA_free(rsaKey);
    ENGINE_free(e);

	return 0; 
}

int engine_test_encrypt_decrypt_RSA_EVP(void)
{
	int ret = -1;
	ENGINE  *e = NULL;
	RSA *pRSA = NULL;
	EVP_PKEY *pKey = NULL;
	EVP_PKEY_CTX *pkey_ctx;
	const unsigned char *kptr = NULL;
	unsigned char inData[] = "helloworld";
	int  inlen = sizeof(inData);
	unsigned char plainData[256] = { 0 };
	size_t plainlen = 0;
	unsigned char encData[256] = { 0 };
	size_t enclen = sizeof(encData);

 	load_hed_engine(e);

	/* init EVP */
	pKey = EVP_PKEY_new();
	if (!pKey) {
		ENGINE_free(e);
		return -1;
	}
	/* set EVP alg type*/
	EVP_PKEY_set_type(pKey, EVP_PKEY_RSA);
 

	pkey_ctx = EVP_PKEY_CTX_new(pKey, NULL);
	if (!pkey_ctx) {
		EVP_PKEY_free(pKey);
		ENGINE_free(e);
		return -1;
	}
 
	
	if (EVP_PKEY_keygen_init(pkey_ctx) <= 0){
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}
	if (EVP_PKEY_keygen(pkey_ctx, &pKey) <= 0) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}

	
	ret = EVP_PKEY_encrypt_init(pkey_ctx);
	if (ret != 1){
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}

	ret = EVP_PKEY_encrypt(pkey_ctx, NULL, &enclen, inData, inlen);
	if (ret != 1) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		return -1;
	}

	
	ret = EVP_PKEY_encrypt(pkey_ctx, encData, &enclen, inData, inlen);
	if (ret != 1) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}

	
	ret = EVP_PKEY_decrypt_init(pkey_ctx);
	if (ret != 1) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}

	
	ret = EVP_PKEY_decrypt(pkey_ctx, NULL, &plainlen, encData, enclen);
	if (ret != 1) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		return -1;
	}

	
	ret = EVP_PKEY_decrypt(pkey_ctx, plainData, &plainlen, encData, enclen);
	if (ret != 1) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}

	if (plainlen != inlen) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}
	if (memcmp(inData, plainData, inlen) != 0) {
		EVP_PKEY_free(pKey);
		EVP_PKEY_CTX_free(pkey_ctx);
		ENGINE_free(e);
		return -1;
	}

	EVP_PKEY_CTX_free(pkey_ctx);
	ENGINE_free(e);
	printf("engine_test_encrypt_decrypt_RSA_EVP success!!!\n");
	return 0;
}

int engine_test_sign_verify_RSA(void)
{
	int ret,j;
	ENGINE  *e_hed = NULL;
	RSA *r;
	int i,bits=2048,signlen,datalen,alg,nid;
	unsigned long e=RSA_F4;
	BIGNUM *bne;
	unsigned char data[100],signret[400];

	load_hed_engine(e_hed);

	bne=BN_new();
	ret=BN_set_word(bne,e);
	r=RSA_new();
	ret=RSA_generate_key_ex(r,bits,bne,NULL);
	if(ret!=1)
	{
		printf("RSA_generate_key_ex err!\n");
		RSA_free(r);
		ENGINE_free(e_hed);
		return -1;
	}

	for(i=0;i<100;i++)
		memset(&data[i],i+1,1);
		datalen=32;
		nid=NID_sha256;
	
	ret=RSA_sign(nid,data,datalen,signret,&signlen,r);
	for(j = 0; j < signlen; j++)
	{
		if(j%16==0)
			printf("\n%08xH: ",i);
		printf("%02x ", signret[j]);	
	}
	if(ret!=1)
	{
		printf("RSA_sign err!\n");
		RSA_free(r);
		ENGINE_free(e_hed);
		return -1;
	}

	ret=RSA_verify(nid,data,datalen,signret,signlen,r);
	if(ret!=1)
	{
		printf("RSA_verify err!\n");
		RSA_free(r);
		ENGINE_free(e_hed);
		return -1;
	}

	RSA_free(r);
	ENGINE_free(e_hed);
	printf("engine_test_sign_verify_RSA test ok!\n");
	return 0;

}

int engine_test_sign_verify_RSA_EVP(void)
{
	unsigned char sign_value[1024];	
	ENGINE  *e = NULL;
	BIGNUM *bne = BN_new();
	int sign_len;			
	EVP_MD_CTX *mdctx = NULL;		
	char mess1[] = "xxh";		
	RSA *rsa=NULL;			
	EVP_PKEY *evpKey=NULL;	
	int i;
	
	load_hed_engine(e);

	OpenSSL_add_all_algorithms();

	printf("generate RSA key...");
	BN_set_word(bne,65537);
	rsa=RSA_new();
	RSA_generate_key_ex(rsa, 1024,bne,NULL);
	if(rsa == NULL)
	{
		printf("gen rsa err\n");
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}
	printf(" success.\n");
	evpKey = EVP_PKEY_new();
	if(evpKey == NULL)
	{
		printf("EVP_PKEY_new err\n");
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}
	if(EVP_PKEY_set1_RSA(evpKey,rsa) != 1)	
	{
		printf("EVP_PKEY_set1_RSA err\n");
		RSA_free(rsa);
		ENGINE_free(e);
		EVP_PKEY_free(evpKey);
		return 0;
	}
	
	mdctx = EVP_MD_CTX_new();
	if(!EVP_SignInit_ex(mdctx, EVP_sha1(), NULL))
	{
		printf("err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}
	if(!EVP_SignUpdate(mdctx, mess1, strlen(mess1)))
	{
		printf("err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}
	if(!EVP_SignFinal(mdctx,sign_value,&sign_len,evpKey))	
	{
		printf("err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}
	printf("massage\"%s\"sig is: \n",mess1);
	for(i = 0; i < sign_len; i++)
	{
		if(i%16==0)
			printf("\n%08xH: ",i);
		printf("%02x ", sign_value[i]);	
	}
	printf("\n");	
	EVP_MD_CTX_free(mdctx);
	
	printf("\nverifing...\n");

	mdctx = EVP_MD_CTX_new();
	if(!EVP_VerifyInit_ex(mdctx, EVP_sha1(), NULL))
	{
		printf("EVP_VerifyInit_ex err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}
	if(!EVP_VerifyUpdate(mdctx, mess1, strlen(mess1)))
	{
		printf("err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}	
	if(!EVP_VerifyFinal(mdctx,sign_value,sign_len,evpKey))
	{
		printf("verify err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		ENGINE_free(e);
		return 0;
	}
	else
	{
		printf("the engine_test_sign_verify_RSA_EVP pass.\n");
	}

	EVP_PKEY_free(evpKey);
	RSA_free(rsa);
	EVP_MD_CTX_free(mdctx);
	ENGINE_free(e);
	return 0;

}

