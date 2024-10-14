
#ifndef _HED_ENGINE_H_
#define _HED_ENGINE_H_

#include <stdio.h>
#include <openssl/engine.h>

// SETTINGS
#define KEY_CONTEXT_MAX_LEN  (100)

//Macro define
/// Definition for false
#ifndef FALSE
#define FALSE               (0U)
#endif

/// Definition for true
#ifndef TRUE
#define TRUE                (1U)
#endif

#define HED_ENGINE_SUCCESS	1
#define HED_ENGINE_FAIL		0
 

#define EVP_SUCCESS ( 1)
#define EVP_FAIL    (-1)

#define PUBKEYFILE_SIZE 1024
#define PUBKEY_SIZE 1024

//set the temporary ECC key pair kid for ECDHE
#define TMPPUB_KID    0xF0
#define TMPPRI_KID    0xF1

#define INIT_KID 0x11
typedef struct hed_ctx_str
{
  char      key[KEY_CONTEXT_MAX_LEN];
  uint16_t	kid;
  uint8_t  alg;
  char			pubkeyfilename[PUBKEYFILE_SIZE];
  uint8_t   pubkey[PUBKEY_SIZE];
  uint16_t  pubkeylen; 
} hed_ctx_t;

//extern
extern hed_ctx_t hed_ctx;


//hed enine init
uint16_t hedEngine_init_ec(ENGINE *e);
uint16_t hedEngine_init_rsa(ENGINE *e);
uint16_t hedEngine_init_sym(ENGINE *e);
#endif // _HED_ENGINE_H_
