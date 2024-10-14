/**@file  auth_test.h
* @brief  auth_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _AUTH_TEST_H_
#define _AUTH_TEST_H_


#include <stdint.h>
#include "auth.h"
#include "soft_alg.h"
//#include "crypto.h"


typedef struct scp_key
{
    uint8_t enc_key[16];//enc key
    uint8_t mac_key[16];//mac key
    uint8_t dek_key[16];//dek key
    uint8_t new_enc[16];//new enc key
    uint8_t new_mac[16];//new mac key
    uint8_t new_dek[16];//new dek key
    uint8_t out_enc[16]; //process enc key
    uint8_t out_mac[16]; //process mac key
    uint8_t out_dek[16]; //process dek key
    uint8_t CMAC[8];    //CMAC
}scp_key;




/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup AUTH_TEST AUTH_TEST
  * @brief auth_test interface api.
  * @{
  */





/* Exported functions --------------------------------------------------------*/
/** @defgroup AUTH_TEST_Exported_Functions AUTH_TEST Exported Functions
  * @{
  */
	

	
se_error_t auth_test(void);





/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */




#endif
