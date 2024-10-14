/**
  ******************************************************************************
  * @file    HED_SymmetricAlgModeProcess.c
  * @author  Firmware Team
  * @brief   Soft Alg lib Source File
  * @date    2021-04-09
  * @version V1.0
  ******************************************************************************
  * @attention HED_SymmetricAlgModeProcess.c
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
  * @brief HED_SymmetricAlgModeProcess.c
  * @{
  */
#include "HED_SymmetricAlgModeProcess.h"

/** @addtogroup HAL_Private_Variables
  * @{
  */
///< variable¡¢macro¡¢struct
/**
  * @}
  */

uint32_t SymmetricAlgModeProcess_Soft(void (* AlgEcbFunc)(ALG_Parameters_Soft *), ALG_Parameters_Soft *pstPara, uint8_t UnitByteLen)
{
    ALG_Parameters_Soft stPara;
    uint32_t i = 0, j = 0, num = 0, retValue = ALG_LEN_ERR;
    uint8_t IVBuf[16], InDataBuf[16], OutDataBuf[16];

    if ((pstPara->DataLen == 0)
            || (pstPara->DataLen % UnitByteLen)
            || (NULL == pstPara->iDataPtr)
            || (NULL == pstPara->iKeyPtr)
            || (NULL == pstPara->oDataPtr)
       )
    {
        return ALG_LEN_ERR;
    }
    stPara = *pstPara;
    num = pstPara->DataLen / UnitByteLen;
    switch (pstPara->Type & 0x38)
    {
        case AlgModeECB:
        {
            for (i = 0; i < num; i++)
            {
                AlgEcbFunc((ALG_Parameters_Soft *)&stPara);
                stPara.iDataPtr += UnitByteLen;
                stPara.oDataPtr += UnitByteLen;
            }
			retValue = ALG_SUCCESS;
            break;
        }
        case AlgModeCBC:
        {
        	if (NULL == pstPara->IVPtr)
        	{
				return ALG_LEN_ERR;
			}
            // Copy IV
            for (j = 0; j < UnitByteLen; j++)
            {
                IVBuf[j] = pstPara->IVPtr[j];
            }
            for (i = 0; i < num; i++)
            {
                if ((0 << 6) == (pstPara->Type & (1 << 6)))
                {
                    // Enc and xor before cale
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        InDataBuf[j] = pstPara->iDataPtr[i * UnitByteLen + j] ^ IVBuf[j];
                    }
                }
                else
                {
                    // Dec
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        InDataBuf[j] = pstPara->iDataPtr[i * UnitByteLen + j];
                    }
                }
                stPara.iDataPtr = (uint8_t *)InDataBuf;
                AlgEcbFunc((ALG_Parameters_Soft *)&stPara);
                if ((1 << 6) == (pstPara->Type & (1 << 6)))
                {
                    // Dec and xor after cale
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        pstPara->oDataPtr[i * UnitByteLen + j] = pstPara->oDataPtr[i * UnitByteLen + j] ^ IVBuf[j];
                    }
                }
                if ((0 << 6) == (pstPara->Type & (1 << 6)))
                {
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        // Enc and chiper is next IV
                        IVBuf[j] = pstPara->oDataPtr[i * UnitByteLen + j];
                    }
                }
                else
                {

                    for (j = 0; j < UnitByteLen; j++)
                    {
                        // Dec and Src data is IV
                        IVBuf[j] = InDataBuf[j];
                    }
                }
                stPara.oDataPtr += UnitByteLen;
            }
			retValue = ALG_SUCCESS;
            break;
        }
        case AlgModeCFB:
        {
        	if (NULL == pstPara->IVPtr)
        	{
				return ALG_LEN_ERR;
			}
            // Copy IV
            for (j = 0; j < UnitByteLen; j++)
            {
                InDataBuf[j] = pstPara->IVPtr[j];
            }
            for (i = 0; i < num; i++)
            {
                // CFB mode is xor after cale
                stPara.iDataPtr = (uint8_t *)InDataBuf;
                stPara.oDataPtr = (uint8_t *)OutDataBuf;
                // CFB mode is all enc mode
                stPara.Type = pstPara->Type & ~(1 << 6);
                AlgEcbFunc((ALG_Parameters_Soft *)&stPara);
                if ((0 << 6) == (pstPara->Type & (1 << 6)))
                {
                    // Enc
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        pstPara->oDataPtr[i * UnitByteLen + j] = OutDataBuf[j] ^ pstPara->iDataPtr[i * UnitByteLen + j];
                        InDataBuf[j] = pstPara->oDataPtr[i * UnitByteLen + j];
                    }
                }
                else
                {
                    // Dec
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        InDataBuf[j] = pstPara->iDataPtr[i * UnitByteLen + j];
                        pstPara->oDataPtr[i * UnitByteLen + j] = OutDataBuf[j] ^ pstPara->iDataPtr[i * UnitByteLen + j];
                    }
                }
                stPara.oDataPtr += UnitByteLen;
            }
			retValue = ALG_SUCCESS;
            break;
        }
        case AlgModeOFB:
        {
        	if (NULL == pstPara->IVPtr)
        	{
				return ALG_LEN_ERR;
			}
            // Copy IV
            for (j = 0; j < UnitByteLen; j++)
            {
                InDataBuf[j] = pstPara->IVPtr[j];
            }
            for (i = 0; i < num; i++)
            {
                // OFB mode is xor after cale
                stPara.iDataPtr = (uint8_t *)InDataBuf;
                stPara.oDataPtr = (uint8_t *)OutDataBuf;
                // CFB mode is all enc mode
                stPara.Type = pstPara->Type & ~(1 << 6);
                AlgEcbFunc((ALG_Parameters_Soft *)&stPara);
                if ((0 << 6) == (pstPara->Type & (1 << 6)))
                {
                    // Enc
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        pstPara->oDataPtr[i * UnitByteLen + j] = OutDataBuf[j] ^ pstPara->iDataPtr[i * UnitByteLen + j];
                        InDataBuf[j] = OutDataBuf[j];
                    }
                }
                else
                {
                    // Dec
                    for (j = 0; j < UnitByteLen; j++)
                    {
                        pstPara->oDataPtr[i * UnitByteLen + j] = OutDataBuf[j] ^ pstPara->iDataPtr[i * UnitByteLen + j];
                        InDataBuf[j] = OutDataBuf[j];
                    }
                }
                stPara.oDataPtr += UnitByteLen;
            }
			retValue = ALG_SUCCESS;
            break;
        }
        case AlgModeCTR:
        {
            break;
        }
        case AlgModePCBC:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    return retValue;
}
/**
  * @}
  */
/**
  * @}
  */

