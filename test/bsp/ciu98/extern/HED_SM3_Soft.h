/*******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name:     hed32_api_sm3.h
 Author:        Yuzhh
 Version:       V0.1
 Date:          2023/11/14 20:39:35
 Description:
 History:
                2023/11/14 initial version
*******************************************************************************/
#ifndef  hed32_api_sm3_INC_
#define  hed32_api_sm3_INC_
#include <stdint.h>
#define SM3_IV_WLEN         8            /*  */
#define SM3_BLK_WLEN        16            /*  */
#define SM3_U32             uint32_t            /*  */
typedef struct
{
    SM3_U32              Iv[SM3_IV_WLEN];
    SM3_U32              total; 
    SM3_U32              buff[SM3_BLK_WLEN];
    SM3_U32              num;
} SM3Info;

unsigned int HED_SM3_Init_Soft(SM3Info *c);
unsigned int HED_SM3_Update_Soft(SM3Info *c, const void *data, unsigned int ilen);
unsigned int HED_SM3_Final_Soft(unsigned char *md, SM3Info *c);
void HED_SM3_Calc_Soft(const void *d, unsigned int ilen, unsigned char *md);
#endif   /* ----- #ifndef hed32_api_sm3_INC_  ----- */
