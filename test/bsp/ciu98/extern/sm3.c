/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd. 
 File name: 		sm4.c
 Author:			liangww 
 Version:			V1.0	
 Date:			    2023-11-29	
 Description:	    wrap the HED sm3 alg
 History:		
******************************************************************************/
#include "sm3.h"
#include <string.h>
#include <stdio.h>
#include "HED_SM3_Soft.h"

void sm3( unsigned char *input, int ilen,
           unsigned char output[32])
{
    HED_SM3_Calc_Soft(input, ilen, output);
}