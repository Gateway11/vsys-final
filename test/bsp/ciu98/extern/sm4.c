/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd. 
 File name: 		sm4.c
 Author:			liangww 
 Version:			V1.0	
 Date:			    2023-11-29	
 Description:	    wrap the HED sm4 alg
 History:		
******************************************************************************/
#include "sm4.h"
#include <string.h>
#include <stdio.h>
#include "HED_SM4_Soft.h"

int sm4_crypt_ecb(unsigned char *symkey,
				   int mode,
				   int length,
				   unsigned char *input,
                   unsigned char *output)
{
    ALG_Parameters_Soft sm4_ctx = {0};
    sm4_ctx.DataLen = length;
    sm4_ctx.iDataPtr = input;
    sm4_ctx.iKeyPtr = symkey;
    sm4_ctx.IVPtr = NULL;
    sm4_ctx.oDataPtr = output;

    if(mode == SM4_ENCRYPT)
    {
        sm4_ctx.Type = SYM_ECB_EN;
    }
    else if(mode == SM4_DECRYPT)
    {
        sm4_ctx.Type = SYM_ECB_DE;
    }

    if( HED_SM4Block_Soft(&sm4_ctx) != 0xB7C8D9EA)
    {
        return -1;
    }

    return 0;
}


int sm4_crypt_cbc( unsigned char *symkey,
                    int mode,
                    int length,
                    unsigned char iv[16],
                    unsigned char *input,
                    unsigned char *output )
{
    ALG_Parameters_Soft sm4_ctx = {0};
    sm4_ctx.DataLen = length;
    sm4_ctx.iDataPtr = input;
    sm4_ctx.iKeyPtr = symkey;
    sm4_ctx.IVPtr = iv;
    sm4_ctx.oDataPtr = output;
    sm4_ctx.Type = SYM_CBC_EN;

    if(mode == SM4_ENCRYPT)
    {
        sm4_ctx.Type = SYM_CBC_EN;
    }
    else if(mode == SM4_DECRYPT)
    {
        sm4_ctx.Type = SYM_CBC_DE;
    }

    if( HED_SM4Block_Soft(&sm4_ctx) != 0xB7C8D9EA)
    {
        return -1;
    }

    return 0;
}
