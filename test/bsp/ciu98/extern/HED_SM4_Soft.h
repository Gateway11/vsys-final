/**
  ******************************************************************************
  * @file    HED_SM4_Soft.h
  * @author  Firmware Team
  * @brief   Soft Alg lib Header File
  * @date    2021-04-09
  * @version V1.0
  ******************************************************************************
  * @attention HED_SM4_Soft.h
  *
  * <h2><center>CEC Huada Electronic Design Co.,Ltd</center></h2>
  *
  * The copyright of this manual is reserved by CEC Huada Electronic Design Co., Ltd.
  * Without prior permission, any duplication, printing or publication and distribution
  * of the manual will constitute a violation of copyright towards CEC Huada Electronic
  * Design Co., Ltd. CEC Huada Electronic Design Co., Ltd. reserves the right to bring
  * them to court. \n
  * The right to change the manual without notice to the user is reserved by CEC Huada
  * Electronic Design Co., Ltd. Though we have checked content of this manual, errors
  * are unavoidable. So, we will regularly check the content and make necessary
  * modifications in the next version. We suggest you obtain the latest version of this
  * manual from CEC Huada Electronic Design Co., Ltd. before making final design.
  *
  * @par Modify log
  * <table>
  * <tr><th>Date        <th>Version     <th>Author          <th>Description   </tr>
  * <tr><td>2021/04/08  <td>V1.0        <td>Firmware Team   <td>Create        </tr>
  * <tr><td>2021/04/08  <td>V1.1        <td>Firmware Team   <td>ModifyXXXX    </tr>
  * </table>
  ******************************************************************************
  */
/** @defgroup ALG
  * @brief ALG core code
  * @{
  */
/** @defgroup SM4
  * @brief HED_SM4_Soft.h
  * @{
  */
#ifndef HED_SM4_SOFT_H__
#define HED_SM4_SOFT_H__
/****************************************************************************************/
#include "typedef_t.h"
#include "HED_SymmetricAlgModeProcess.h"
/****************************************************************************************/
#define GET_UINT32(n,b,i) (n) = ((((((uint32_t)(b)[(i) + 0] << 8)\
                                +(uint32_t)(b)[(i) + 1]) << 8)\
                            +(uint32_t)(b)[(i) + 2]) << 8)\
                        +(uint32_t)(b)[(i) + 3]
#define PUT_UINT32(n,b,i)                   \
{                                           \
    (b)[(i)    ] = (uint8_t) ( (n) >> 24 );     \
    (b)[(i) + 1] = (uint8_t) ( (n) >> 16 );     \
    (b)[(i) + 2] = (uint8_t) ( (n) >>  8 );     \
    (b)[(i) + 3] = (uint8_t) ( (n)       );     \
}
/****************************************************************************************/
#define BYTESUB(A) ((((((Sbox[(A) >> 24 & 0xFF] << 8)  \
                    + Sbox[(A) >> 16 & 0xFF]) << 8)  \
                + Sbox[(A) >>  8 & 0xFF]) << 8)  \
            + Sbox[(A) & 0xFF])
/****************************************************************************************/
#define ROTL(x,n) (((x) << (n)) | (((x) & 0xFFFFFFFF) >> (32 - (n))))
/****************************************************************************************/
#define L1(B) ((B) ^ ROTL(B, 2) ^ ROTL(B, 10) ^ ROTL(B, 18) ^ ROTL(B, 24))
#define L2(B) ((B) ^ ROTL(B, 13) ^ ROTL(B, 23))
/****************************************************************************************/
void SM4EcbFunc(ALG_Parameters_Soft *pstPara);
uint32_t HED_SM4Block_Soft(ALG_Parameters_Soft *pstPara);
/****************************************************************************************/
#endif /* HED_SM4_SOFT_H__ */
/**
  * @}
  */
/**
  * @}
  */

