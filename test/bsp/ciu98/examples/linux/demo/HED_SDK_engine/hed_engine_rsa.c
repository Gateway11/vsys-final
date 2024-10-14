#include <string.h>
#include "hed_engine_rsa.h"
#include "hed_engine.h"
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/engine.h>
#include <openssl/objects.h>
#include <openssl/ossl_typ.h>
#include <openssl/rsa.h>
#include "se.h"
#include "comm.h"
#include "pin.h"
#include "key.h"
#include "crypto.h"
#include "auth.h"
#include <stdio.h>
#include <stdlib.h>                                               
#include <unistd.h>
#include <stdint.h>

/*globel*/
RSA_METHOD *hed_RSA = NULL;

static int hed_RSA_Pub_Decrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
{
    int ret = 0;
    RSA *dup_rsakey = NULL;
    pub_key_t pub_key={0};
    alg_asym_param_t asym_param={0};
    int hedSE_ret = 0;
    int out_buf_len = 0;
    int model_size  = 0;
    unsigned char e_value[3] = {0x01,0x00,0x01};
    unsigned char tmpbuf[10] = {0};
    unsigned char tmp_buf[4096] = {0};
    int tmp_buf_len = 0;

    LOGI("\nhed engine RSA Pub_Decrypt begin **************\n");
    
    dup_rsakey = RSAPublicKey_dup(rsa);
    RSA_set_method(dup_rsakey, RSA_get_default_method());
    ret = RSA_public_decrypt(flen, from, to, dup_rsakey, padding);
// 
//    BN_bn2bin(RSA_get0_e(rsa), tmpbuf);
//    ret = memcmp(tmpbuf,e_value,3);
//    if(ret != 0)
//    {
//      LOGE("the e value is not suppoted\n");
//      return -1;
//    }

//    model_size = RSA_size(rsa);
//    if(model_size == 256)
//    {
//      pub_key.alg = ALG_RSA2048_CRT;
//    }
//    else if(model_size == 128)
//    {
//       pub_key.alg = ALG_RSA1024_CRT;
//    }
//    else
//    {
//      return -1;
//    }
//    pub_key.id = hed_ctx.kid;

//    switch (padding) {
//    case RSA_PKCS1_PADDING: 
//          asym_param.padding_type = PADDING_NOPADDING;
//          hedSE_ret = api_asym_encrypt (&pub_key, &asym_param, from, flen, tmp_buf, &tmp_buf_len);
//          if(hedSE_ret != 0)
//          {
//            break;
//          }
//          if (0 > (out_buf_len = RSA_padding_check_PKCS1_type_1(to,model_size,tmp_buf,tmp_buf_len,model_size)))
//          {
//            printf("\nthe padding mode SSLv23 is err\n");
//            return -1;
//          }
//          break;
//    case RSA_NO_PADDING: 
//          asym_param.padding_type = PADDING_NOPADDING;
//          hedSE_ret = api_asym_encrypt (&pub_key, &asym_param, from, flen, to, &out_buf_len);
//          break;
//    default:
//          printf("\nthe padding mode is not suppor/';.ted\n");
//          return -1;   
//          break;
//    }

//if ( hedSE_ret != SE_SUCCESS)
//{
//  LOGE("failed to exter_auth_test!\n");
//  return -1;
//} 
//else
//{
//  ret = out_buf_len;
//}
    if (ret > 0)
      LOGI("Pub_Decrypt PASS\n");
    else
      LOGI("Pub_Decrypt FAIL\n");

    return ret;
    LOGI("\nhed engine RSA Pub_Decrypt end **************\n");
}


static int hed_RSA_Priv_Decrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
{
    int ret = 0;
    RSA *dup_rsakey = NULL;
    pri_key_t pri_key={0};
    alg_asym_param_t asym_param={0};
    int hedSE_ret = 0;
    int out_buf_len = 0;
    int model_size = 0;
    unsigned char tmp_buf[4096] = {0};
    int tmp_buf_len = 0;
    unsigned char e_value[3] = {0x01,0x00,0x01};
    unsigned char tmpbuf[10] = {0};

    LOGI("\nhed engine RSA Priv_Decrypt begin **************\n");
  
    // dup_rsakey = RSAPrivateKey_dup(rsa);
    // RSA_set_method(dup_rsakey, RSA_get_default_method());
    // ret = RSA_private_decrypt(flen, from, to, dup_rsakey, padding);
    
    BN_bn2bin(RSA_get0_e(rsa), tmpbuf);
    ret = memcmp(tmpbuf,e_value,3);
    if(ret != 0)
    {
      LOGE("the e value is not suppoted\n");
      return -1;
    }

    model_size = RSA_size(rsa);
    if(model_size == 256)
    {
      pri_key.alg = ALG_RSA2048_CRT;
    }
    else if(model_size == 128)
    {
       pri_key.alg = ALG_RSA1024_CRT;
    }
    else
    {
      return -1;
    }
    hed_ctx.kid = GEN_RSA_PRI_KID;
    pri_key.id = hed_ctx.kid;
   
    switch (padding) {
    case RSA_PKCS1_PADDING: 
          asym_param.padding_type = PADDING_PKCS1;
          hedSE_ret = api_asym_decrypt (&pri_key, &asym_param, from, flen, to, &out_buf_len);
          break;
    case RSA_NO_PADDING: 
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_decrypt (&pri_key, &asym_param, from, flen, to, &out_buf_len);
          break;
    case RSA_SSLV23_PADDING: 
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_decrypt (&pri_key, &asym_param, from, flen, tmp_buf, &tmp_buf_len);
          if(hedSE_ret != 0)
          {
            break;
          }
          if (0 > (out_buf_len = RSA_padding_check_SSLv23(to,model_size,tmp_buf,tmp_buf_len,model_size)))
          {
            printf("\nthe padding mode SSLv23 is err\n");
            return -1;
          }
          break;
    case RSA_PKCS1_OAEP_PADDING:
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_decrypt (&pri_key, &asym_param, from, flen, tmp_buf, &tmp_buf_len);
          if(hedSE_ret != 0)
          {
            break;
          }
          if (0 >(out_buf_len = RSA_padding_check_PKCS1_OAEP(to,model_size,tmp_buf,tmp_buf_len,model_size, NULL , 0)))
          {
            printf("\nthe padding mode SSLv23 is err\n");
            return -1;
          }
          break;
    default:
          printf("\nthe padding mode is not supported\n");
          return -1;   
          break;
    }
    
    if ( hedSE_ret != SE_SUCCESS)
    {
      LOGE("failed to api_asym_decrypt!\n");
      return -1;
    } 
    else
    {
      ret = out_buf_len;
    }


    if (ret > 0)
      LOGI("Priv_Decrypt PASS\n");
    else
      LOGI("Priv_Decrypt FAIL\n");

    return ret;
    LOGI("\nhed engine RSA Priv_Decrypt end **************\n");
}


static int hed_RSA_Pub_Encrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
{
    int ret = 0;
    RSA *dup_rsakey = NULL;
    pub_key_t pub_key={0};
    alg_asym_param_t asym_param={0};
    int hedSE_ret = 0;
    int out_buf_len = 0;
    int model_size  = 0;
    unsigned char tmp_buf[4096] = {0};
    unsigned char e_value[3] = {0x01,0x00,0x01};
    unsigned char tmpbuf[10] = {0};
    LOGI("\nhed engine RSA Pub_Encrypt begin **************\n");
    // /*默认的RSA算法*/
    // dup_rsakey = RSAPublicKey_dup(rsa);
    // RSA_set_method(dup_rsakey, RSA_get_default_method());
    // ret = RSA_public_encrypt(flen, from, to, dup_rsakey, padding);
    
    BN_bn2bin(RSA_get0_e(rsa), tmpbuf);
    ret = memcmp(tmpbuf,e_value,3);
    if(ret != 0)
    {
      LOGE("the e value is not suppoted\n");
      return -1;
    }
    model_size = RSA_size(rsa);
    if(model_size == 256)
    {
      pub_key.alg = ALG_RSA2048_CRT;
    }
    else if(model_size == 128)
    {
       pub_key.alg = ALG_RSA1024_CRT;
    }
    else
    {
      return -1;
    }
    hed_ctx.kid = GEN_RSA_PRI_KID;
    pub_key.id = hed_ctx.kid;

   
    switch (padding) {
    case RSA_PKCS1_PADDING: 
          asym_param.padding_type = PADDING_PKCS1;
          hedSE_ret = api_asym_encrypt (&pub_key, &asym_param, from, flen, to, &out_buf_len);
          break;
    case RSA_NO_PADDING: 
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_encrypt (&pub_key, &asym_param, from, flen, to, &out_buf_len);
          break;
    case RSA_SSLV23_PADDING: 
          if(flen > model_size -11)
          {
            LOGE("data is too large for key size\n");
            return -1;
          }
          if(0 > RSA_padding_add_SSLv23(tmp_buf, model_size, from, flen))
          {
            printf("\nthe padding mode SSLv23 is err\n");
            return -1;
          }
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_encrypt (&pub_key, &asym_param, tmp_buf,model_size , to, &out_buf_len);
          break;
    case RSA_PKCS1_OAEP_PADDING:
          if(flen > model_size -41)
          {
            LOGE("data is too large for key size\n");
            return -1;
          }
          if(0 > RSA_padding_add_PKCS1_OAEP(tmp_buf, model_size, from, flen, NULL, 0))
          {
            printf("\nthe padding mode PKCS1_OAEP is err\n");
            return -1;
          }
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_encrypt (&pub_key, &asym_param, tmp_buf, model_size, to, &out_buf_len);
          break;
    default:
          printf("\nthe padding mode is not supported\n");
          return -1;   
          break;
    }

   
    if ( hedSE_ret != SE_SUCCESS)
    {
      LOGE("failed to api_asym_encrypt!\n");
      return -1;
    } 
    else
    {
      ret = out_buf_len;
    }


    if (ret > 0)
      LOGI("Pub_Encrypt PASS\n");
    else
      LOGI("Pub_Encrypt FAIL\n");

    return ret;
    LOGI("\nhed engine RSA Pub_Encrypt end **************\n");
}

static int hed_RSA_Priv_Encrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
{
    int ret = 0;
    RSA *dup_rsakey = NULL;
    pri_key_t pri_key={0};
    alg_asym_param_t asym_param={0};
    int hedSE_ret = 0;
    int out_buf_len = 0;
    int model_size = 0;
    unsigned char e_value[3] = {0x01,0x00,0x01};
    unsigned char tmpbuf[10] = {0};
   unsigned char tmp_buf[4096] = {0};
    LOGI("\nhed engine RSA Priv_Encrypt begin **************\n");  
   
    // dup_rsakey = RSAPrivateKey_dup(rsa);
    // RSA_set_method(dup_rsakey, RSA_get_default_method());
    // ret = RSA_private_encrypt(flen, from, to, dup_rsakey, padding);
 
    BN_bn2bin(RSA_get0_e(rsa), tmpbuf);
    ret = memcmp(tmpbuf,e_value,3);
    if(ret != 0)
    {
      LOGE("the e value is not suppoted\n");
      return -1;
    }

    model_size = RSA_size(rsa);
    if(model_size == 256)
    {
      pri_key.alg = ALG_RSA2048_CRT;
    }
    else if(model_size == 128)
    {
       pri_key.alg = ALG_RSA1024_CRT;
    }
    else
    {
      return -1;
    }
    hed_ctx.kid = GEN_RSA_PRI_KID;
    pri_key.id = hed_ctx.kid;
   
    switch (padding) {
    case RSA_PKCS1_PADDING: 
          if(flen > model_size -11)
          {
            LOGE("data is too large for key size\n");
            return -1;
          }
          if(0 > RSA_padding_add_PKCS1_type_1(tmp_buf, model_size, from, flen))
          {
            printf("\nthe padding mode SSLv23 is err\n");
            return -1;
          }
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_decrypt (&pri_key, &asym_param, tmp_buf, model_size, to, &out_buf_len);
          break;
    case RSA_NO_PADDING: 
          asym_param.padding_type = PADDING_NOPADDING;
          hedSE_ret = api_asym_decrypt (&pri_key, &asym_param, from, flen, to, &out_buf_len);
          break;
    default:
          printf("\nthe padding mode is not supported\n");
          return -1;   
          break;
    }
   
    if ( hedSE_ret != SE_SUCCESS)
    {
      LOGE("failed to exter_auth_test!\n");
      return -1;
    } 
    else
    {
      ret = out_buf_len;
    }

    if (ret > 0)
      LOGI("Priv_Encrypt PASS\n");
    else
      LOGI("Priv_Encrypt FAIL\n");

    return ret;
    LOGI("\nhed engine RSA Priv_Encrypt end **************\n");
}


static int  hed_RSA_generate_key(RSA *rsa, int bits, BIGNUM *e, BN_GENCB *cb)
{
  uint16_t ret = 0;
  RSA *int_rsakey = NULL;
  pub_key_t pub_key={0};
  pri_key_t pri_key={0};
  uint8_t e_value[5] = {0x01,0x00,0x01};//e值仅支持
  uint32_t e_value_len = 3;
  uint8_t n_value[3000] = {0};
  uint32_t n_value_len = 0;
  uint8_t d_value[3000] = {0};
  uint32_t d_value_len = 0;
  unsigned char tmpbuf[10] = {0};
  unsigned char testuf[300] = {0};
  LOGI("\nhed Engine rsa genkey begin**************\n");  

  BN_bn2bin(e, tmpbuf);
  ret = memcmp(tmpbuf,e_value,3);
  if(ret != 0)
  {
    LOGE("the e value is not suppoted\n");
    return -1;
  }
 
  if(bits == 1024)
  {
    hed_ctx.alg = ALG_RSA1024_CRT;
  }
  else if(bits == 2048)
  {
    hed_ctx.alg = ALG_RSA2048_CRT;
  }
  else
  {
    printf("\nthe model length is not supported!\n");
    return -1;
  }

  
	pub_key.alg = hed_ctx.alg;
  hed_ctx.kid = GEN_RSA_PRI_KID;
	pub_key.id  = hed_ctx.kid;
	pri_key.id  = hed_ctx.kid;
	ret = api_generate_keypair (&pub_key, &pri_key);
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to generate_keypair_test\n");
		return -1;
	}
	for(int i = 0;i<pub_key.val_len;i++)
	{
		printf("%02x",pub_key.val[i]);
	}
	printf("\n");

  
  memcpy(n_value+1, &(pub_key.val[4]), bits/8);
  n_value_len = bits/8 +1;
  RSA_set0_key(rsa, BN_bin2bn(n_value, n_value_len, NULL), e, NULL);
  BN_bn2bin(RSA_get0_n(rsa),testuf);
  LOGI("\nhed Engine rsa genkey end**************\n"); 
  return 1;

}

uint16_t hedEngine_init_rsa(ENGINE *e)
{
  uint16_t ret = 0;

  hed_RSA = RSA_meth_new("hedse_rsa", 0);

  if (hed_RSA == NULL) {
    return 0;
  }
  LOGI("\nhedEngine_init_rsa begin**************\n");  

  do {
      
    RSA_meth_set_pub_enc(hed_RSA, &hed_RSA_Pub_Encrypt);
    RSA_meth_set_pub_dec(hed_RSA, &hed_RSA_Pub_Decrypt);
    RSA_meth_set_priv_enc(hed_RSA, &hed_RSA_Priv_Encrypt);
    RSA_meth_set_priv_dec(hed_RSA, &hed_RSA_Priv_Decrypt);
    RSA_meth_set_keygen(hed_RSA, &hed_RSA_generate_key);
      		
    ret = ENGINE_set_RSA(e, hed_RSA);
        
  }while(FALSE);

  LOGI("\nhedEngine_init_rsa end**************\n");
  return ret;
}
