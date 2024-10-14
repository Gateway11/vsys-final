#include <string.h>
#include "hed_engine_ec.h"
#include "hed_engine.h"
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/engine.h>
#include <openssl/objects.h>
#include <openssl/ossl_typ.h>
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

//Globe
const EC_KEY_METHOD *default_ec = NULL;
EC_KEY_METHOD *ec_methods = NULL;

int hed_EC_KEY_generate_key(EC_KEY *eckey)
{
    LOGI("\nhed keygen begin **************\n");
    int ok = 0;
    BN_CTX *ctx = NULL;
    BIGNUM *x = NULL;
    BIGNUM *y = NULL;
    const BIGNUM *order = NULL;
    int ret = 0;
	   pub_key_t pub_key={0};
	   pri_key_t pri_key={0};
  
    if ((ctx = BN_CTX_new()) == NULL)
        goto err;
    x = BN_CTX_get(ctx);
    y = BN_CTX_get(ctx);
    //generate temporary ECC key pair
    pub_key.alg = ALG_ECC256_NIST;
    pub_key.id  = TMPPUB_KID;
    pri_key.id  = TMPPRI_KID;
    LOGI("the tmp pubkey kid is: 0x%02x\n",TMPPUB_KID);
    LOGI("the tmp prikey kid is: 0x%02x\n",TMPPRI_KID);
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
    
    ok = 1;

 err:
    BN_CTX_free(ctx);
    LOGI("hed keygen end **************\n");
    return ok;
}

int hed_ECDH(
  unsigned char  **pout,
  size_t                   *poutlen,
  const EC_POINT        *pub_key,
  const EC_KEY         *ecdh
)
{
    LOGI("\nhed engine ECDH begin **************\n");
    BN_CTX *ctx;
    BIGNUM *x_pub = NULL;
    BIGNUM *y_pub = NULL;
    const BIGNUM *priv_key;
    const EC_GROUP *group;
    int ret = 0;
    size_t buflen, len, len_tmp;
    unsigned char *buf = NULL;
    unsigned char x_bin_tmp[64] = {0};
    unsigned char x_pub_bin[64] = {0};
    unsigned char y_pub_bin[64] = {0};
    
    
    unikey_t ukey = {0};
	  unikey_t inkey = {0};
    
    
    uint8_t input[70] = {0};
    uint32_t input_len = 64;
    unikey_t shared_key = {0};
    uint8_t sm2_s[64] = {0};
    bool if_return_key = 1;
    bool if_return_s = 0;
    uint32_t i = 0; 
      
    if ((ctx = BN_CTX_new()) == NULL)
        goto err;
    BN_CTX_start(ctx);
    x_pub = BN_CTX_get(ctx);
    y_pub = BN_CTX_get(ctx);
    group = EC_KEY_get0_group(ecdh);
    
    //get out the public key X/Y
    if (!EC_POINT_get_affine_coordinates_GFp(group, pub_key, x_pub, y_pub, NULL)) {
        ECerr(EC_F_ECDH_SIMPLE_COMPUTE_KEY, EC_R_POINT_ARITHMETIC_FAILURE);
        goto err;
    }
    //translate the public key X/Y into binary
     len_tmp = (size_t)BN_bn2bin(x_pub,x_pub_bin);
     len_tmp = (size_t)BN_bn2bin(y_pub,y_pub_bin);
    memcpy(input,x_pub_bin,32);
    memcpy(input+32,y_pub_bin,32);
    shared_key.alg = ALG_ECDH_ECC256;
    shared_key.id = 0xF1;
     //SE ECDH 
    ret = api_generate_shared_key (input, input_len,&shared_key,sm2_s, if_return_key, if_return_s);
    if ( ret != SE_SUCCESS)
    {
    LOGE("failed to generate_shared_key!\n");
    goto err;
    }
  
  
    buflen = (EC_GROUP_get_degree(group) + 7) / 8;
    len = shared_key.val_len;
    if (len > buflen) {
        ECerr(EC_F_ECDH_SIMPLE_COMPUTE_KEY, ERR_R_INTERNAL_ERROR);
        goto err;
    }
    if ((buf = OPENSSL_malloc(buflen)) == NULL) {
        ECerr(EC_F_ECDH_SIMPLE_COMPUTE_KEY, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    memset(buf, 0, buflen - len);   
    memcpy(buf + buflen - len,shared_key.val,shared_key.val_len);
    *pout = buf;
    *poutlen = buflen;
    buf = NULL;

    ret = 1;

 err:
    
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    OPENSSL_free(buf);
    LOGI("hed engine ECDH end **************\n");
    return ret;
}

static ECDSA_SIG* hed_ECDSA_sign(
  const unsigned char  *dgst,
  int                   dgstlen,
  const BIGNUM         *in_kinv,
  const BIGNUM         *in_r,
  EC_KEY               *eckey
)
{
  LOGI("\nhed engine ECC sign begin **************\n");
  LOGI("the sign kid is: 0x%02x\n",hed_ctx.kid);
  LOGI("the dgst len: %d",dgstlen);
  uint8_t     sig[256];
  uint16_t    sig_len = 256;
  uint8_t     r_buf_der[40] = {0};
  uint8_t     s_buf_der[40] = {0};
  uint8_t     hed_sig[256];
  uint32_t    hed_sig_len = 0;
  uint16_t i;
  pub_key_t key={0};
  alg_asym_param_t asym_param={0};
  se_error_t ret = 0;
  ECDSA_SIG  *ecdsa_sig = NULL;
  

  do {
	    key.alg = ALG_ECC256_NIST;
			key.id = hed_ctx.kid;
			asym_param.hash_type = ALG_NONE;
			
			ret = api_asym_sign (&key, &asym_param, dgst, dgstlen,  hed_sig, &hed_sig_len);//signature interface
			if ( ret != SE_SUCCESS)
			{
				LOGE("failed to api_asym_sign!,%04x\n", ret);
				break;
			}
      
      //add the 'T' 'L' before the r value
      r_buf_der[0] = 0x02;
      r_buf_der[1] = 0x21;
      r_buf_der[2] = 0x00;
      memcpy(r_buf_der+3,hed_sig,0x20);
      
      //add the 'T' 'L' before the s value
      s_buf_der[0] = 0x02;
      s_buf_der[1] = 0x21;
      s_buf_der[2] = 0x00;
			memcpy(s_buf_der+3,hed_sig+0x20,0x20);

      //add the 'r' 's' in the sig	
      memcpy(sig+2,r_buf_der,0x23);	
      memcpy(sig+2+0x23,s_buf_der,0x23);
                              
     //the sig 't''l' 
    sig[0] = 0x30;
    sig[1] = 0x46;
  LOGI("\napi_asym_sign sig:\n");
	for(i = 0;i<0x46+2;i++)
	{
			printf("%02x",sig[i]);
	}
    const unsigned char *p = sig;
    ecdsa_sig = d2i_ECDSA_SIG(NULL, &p, 0x46+2);

  }while(FALSE);
  
  LOGI("\nhed engine ECC sign end **************\n"); 
  //return ret;
  return ecdsa_sig;
  
}


uint16_t hedEngine_init_ec(ENGINE *e)
{
  uint16_t ret = 0;
  int (*orig_sign) (int, const unsigned char *, int, unsigned char *,
                    unsigned int *, const BIGNUM *, const BIGNUM *, EC_KEY *)= NULL;
  int (*orig_verify)(int type,const unsigned char *dgst,int dgst_len,const unsigned char *sigbuf,
		    int sig_len,EC_KEY *eckey) = NULL;
  int (*orig_verify_sig)(const unsigned char *dgst,int dgst_len,const ECDSA_SIG *sig,
		    EC_KEY *eckey) = NULL; 
  int (*orig_ecdh)(unsigned char  **, size_t *, const EC_POINT *, const EC_KEY *)  = NULL;
  int (*orig_keygen)(EC_KEY *key) = NULL;
  
  LOGI("\nhedEngine_init_ec begin**************\n");  

  do {
      default_ec = EC_KEY_OpenSSL();
      if (default_ec == NULL)
        break;
      
      ec_methods = EC_KEY_METHOD_new(default_ec);
     
      if (ec_methods == NULL)
        break;
      
      EC_KEY_METHOD_get_sign(ec_methods, &orig_sign, NULL, NULL);
      EC_KEY_METHOD_set_sign(ec_methods, orig_sign, NULL, hed_ECDSA_sign);
      
      EC_KEY_METHOD_get_compute_key(ec_methods,&orig_ecdh);
      EC_KEY_METHOD_set_compute_key(ec_methods,&hed_ECDH);
      
      EC_KEY_METHOD_get_keygen(ec_methods,&orig_keygen);
      EC_KEY_METHOD_set_keygen(ec_methods,&hed_EC_KEY_generate_key);
      
      EC_KEY_METHOD_get_verify(ec_methods, &orig_verify,&orig_verify_sig);
      EC_KEY_METHOD_set_verify(ec_methods, orig_verify, orig_verify_sig);
        		
      ret = ENGINE_set_EC(e, ec_methods);
        
  }while(FALSE);

  LOGI("\nhedEngine_init_ec end**************\n");
  return ret;
}
