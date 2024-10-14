/**
  ******************************************************************************
  * @file    HED_SymmetricAlgModeProcess.h
  * @author  Firmware Team
  * @brief   Soft Alg lib Header File
  * @date    2021-04-09
  * @version V1.0
  ******************************************************************************
  * @attention HED_SymmetricAlgModeProcess.h
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
/** @defgroup ModeProcess
  * @brief HED_SymmetricAlgModeProcess.h
  * @{
  */
#ifndef HED_SYMMETRICALGMODEPROCESS__
#define HED_SYMMETRICALGMODEPROCESS__
/******************************************************************************/
#include "typedef_t.h"
/******************************************************************************/
/*
################################################################################
********************* Enumeration and structure Definition *********************
################################################################################
*/
typedef struct
{
    unsigned char *iDataPtr;
    unsigned int DataLen;
    unsigned char *iKeyPtr;
    unsigned char *IVPtr;
    unsigned char *oDataPtr;
    unsigned char Type;
} ALG_Parameters_Soft;

typedef enum
{
    AlgModeECB = (uint8_t)0x00,
    AlgModeCBC = (uint8_t)0x20,
    AlgModeCFB = (uint8_t)0x10,
    AlgModeOFB = (uint8_t)0x30,
    AlgModeCTR = (uint8_t)0x08,
    AlgModePCBC = (uint8_t)0x18,
} ENUM_AlgMode;
/******************************************************************************/
/*
################################################################################
******************************* Macro Definitions ******************************
################################################################################
*/
#define ALG_SUCCESS               0xB7C8D9EA
#define ALG_LEN_ERR               0x932057CE
/******************************************************************************/
/*
################################################################################
************************ Function Prototype Declaration ************************
################################################################################
*/
uint32_t SymmetricAlgModeProcess_Soft(void (* AlgEcbFunc)(ALG_Parameters_Soft *), ALG_Parameters_Soft *pstPara, uint8_t UnitByteLen);
/******************************************************************************/
#endif /* HED_SYMMETRICALGMODEPROCESS__ */

