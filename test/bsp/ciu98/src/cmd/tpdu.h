/**@file  tpdu.h
* @brief  tpdu interface declearation	 
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _TPDU_H_
#define _TPDU_H_

#include "util.h"
#include "error.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup CMD
  * @brief Command layer.
  * @{
  */

/** @defgroup TPDU TPDU
  * @brief tpdu command pack , unpack, execute.
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup TPDU_Exported_Types TPDU Exported Types
  * @{
  */



/**
  * @brief  APDU protocol type
  */
enum 
{
	ISO_CASE_1  = 0x01,  ///<CLA INS P1 P2 00
	ISO_CASE_2S = 0x02,  ///<CLA INS P1 P2 Le
	ISO_CASE_3S = 0x03,  ///<CLA INS P1 P2 Lc Data
	ISO_CASE_4S = 0x04,   ///<CLA INS P1 P2 Lc Data Le
	ISO_CASE_2E = 0x12,  ///<CLA INS P1 P2 Le(Le:3 bytes)
	ISO_CASE_3E = 0x13,  ///<CLA INS P1 P2 Lc Data(Lc:3 bytes))
	ISO_CASE_4E = 0x14,  ///<CLA INS P1 P2 Lc Data Le(Lc:3 bytes) Le: 2bytes))

};

/**
  * @brief  APDU command parameters
  */
typedef struct
{
	uint8_t  isoCase;     ///<protocol type
	uint8_t  classbyte;   ///<CLA
	uint8_t  instruction; ///<INS
	uint8_t  p1;          ///<P1
	uint8_t  p2;          ///<P2
	uint32_t lc;          ///<LC
	uint32_t le;          ///<LE
} iso_command_apdu_t;

/**
  * @brief  CLA 
  */
enum
{ 
	CMD_CLA_CASE1 = 0x00,    
	CMD_CLA_CASE2 = 0x04,
	CMD_CLA_CASE3 = 0x80,
	CMD_CLA_CASE4 = 0x84,
};


/**
  * @brief  commandID
  */
enum 
{
	CMD_ENTER_LOWPOWER=0x00,  ///<enter low power mode
	CMD_EXTTERN_AUTH, ///<exttern auth 
	CMD_WRITE_KEY,///<write key
    CMD_GENERATE_KEY,///<generate key 
	CMD_GEN_SHARED_KEY,///<generate shared key
	CMD_DEL_KEY,///<delete key
    CMD_EXPORT_KEY,///<export key
	CMD_IMPORT_KEY,///<import key
	CMD_GET_KEY_INFO,///<write SE ID
	CMD_CHANGE_RELOAD_PIN ,///<change/reload pin
	CMD_VERIFY_PIN,///<verify pin
	CMD_CIPHER_DATA,///<symmetric calculation 
	CMD_PKI_ENCIPHER,///<asymmetric encrypting calculation
	CMD_PKI_DECIPHER,///<asymmetric derypting calculation
	CMD_COMPUTE_SIGNATURE, ///<signature calculation
	CMD_VERIFY_SIGNATURE, ///<verify signature
    CMD_SM2_GET_ZA,///<get sm2 ZA
	CMD_DIGEST,///<digest calculation
    CMD_GET_RANDOM,///<get random
	CMD_SELECT_FILE,///<select file
    CMD_WRITE_FILE,///<write file
	CMD_READ_FILE,///<read file
	CMD_WRITE_SEID,///<write SE ID 
	CMD_GET_INFO,///<get SE info
	CMD_GET_ID,///<get SE ID
	CMD_GET_RESPONSE,///<get response
	CMD_GET_FILE_INFO,///<get file info
	CMD_LOADER_INIT,     ///<loader init
	CMD_LOADER_PROGRAM,     ///<loader download
	CMD_LOADER_CHECKPROGRAM,    ///<loader check image
	CMD_V2X_GENERATE_KEY_DERIVE_SEED,    ///<generate key derive seed
	CMD_V2X_RECONSITUTION_KEY,    ///<reconstruct private key
	CMD_V2X_GET_KEY_DERIVE_SEED, ///<read the key seed generated
	CMD_PRIVATE_KEY_TRANSFORMATION, ///<private key transform according to : y = ax + b
	CMD_CONFIG_SYSTEM_INFO, ///<chnge se app para config
	CMD_MANAGE_EM_FLAG,  ///<update or read EM_flag
};

/**
  * @}
  */



/* Exported functions --------------------------------------------------------*/
/** @defgroup TPDU_Exported_Functions TPDU Exported Functions
  * @{
  */


iso_command_apdu_t* tpdu_init(iso_command_apdu_t *command, int32_t isoCase, int32_t cla, int32_t ins,int32_t p1, int32_t p2,int32_t lc, int32_t le);


iso_command_apdu_t* tpdu_init_with_id	(iso_command_apdu_t *command, uint8_t commandID);


iso_command_apdu_t* tpdu_set_cla  (iso_command_apdu_t *command, uint32_t cla);


iso_command_apdu_t* tpdu_set_p1p2 (iso_command_apdu_t *command, uint8_t p1, uint8_t p2);


iso_command_apdu_t* tpdu_set_le(iso_command_apdu_t *command, uint32_t le);


se_error_t tpdu_pack(iso_command_apdu_t *command, uint8_t *output, uint32_t *output_len);


se_error_t tpdu_unpack(uint8_t *output, uint32_t *output_len);


se_error_t tpdu_send(uint8_t *input, uint32_t input_len,uint8_t *output, uint32_t *output_len);


se_error_t tpdu_execute	(iso_command_apdu_t *command, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len);


se_error_t tpdu_execute_no_response	(iso_command_apdu_t *command, uint8_t *input, uint32_t input_len);


se_error_t tpdu_change_error_code(uint16_t status_word);

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif
