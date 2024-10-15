/**@file  proto_spi.c
 * @brief  SPI communication protocol layer driver (implemented according to HED SPI communication protocol specification)
 * @author  liangww
 * @date  2021-04-28
 * @version	V1.0
 * @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 */

/***************************************************************************
 * Include Header Files
 ***************************************************************************/

#include "proto_spi.h"
#include "error.h"
#include "log.h"
uint8_t reset_buf[PROTO_SPI_RERESET_MAX_LEN] = {0};
uint8_t atr_buf[PROTO_SPI_ATR_MAX_LEN + 4] = {0};

/**************************************************************************
 * Variable Declaration
 ***************************************************************************/
const uint8_t cRESETAPDU_HED_SE[ACTIVE_REQ_FRAME_LEN] = {PIB_ACTIVE_FRAME, 0x00, 0x04, PIB_ACTIVE_FRAME_RESET, 0x00, 0x89, 0xC4};
const uint8_t cRATRAPDU_HED_SE[ACTIVE_REQ_FRAME_LEN] = {PIB_ACTIVE_FRAME, 0x00, 0x04, PIB_ACTIVE_FRAME_RATR, 0x00, 0xF3, 0x6B};
const uint8_t cNAK_HED_SE[PROCESS_FRAME_LEN] = {PIB_PROCESS_FRAME, 0x00, 0x03, PIB_PROCESS_FRAME_NAK_CRC_INFO, 0xD4, 0x3A};
const uint8_t cWTX_HED_SE[PROCESS_FRAME_LEN] = {PIB_PROCESS_FRAME, 0x00, 0x03, PIB_PROCESS_FRAME_WTX_INFO, 0xD3, 0x4C};
static spi_param_t g_spi_param[MAX_PERIPHERAL_DEVICE] = {{0, 0, 0, 0, SPI_DEFAULT}, {0, 0, 0, 0, SPI_DEFAULT}, {0, 0, 0, 0, SPI_DEFAULT}, {0, 0, 0, 0, SPI_DEFAULT}};
uint8_t g_bSPIHedOpenSeMode = HED20_SPI_OPEN_SE_RESET_REQ; // Operation mode when opening se
uint8_t g_bSPIHedRstSeMode = HED20_SPI_RESET_SE_RESET_REQ; // Operation mode when resetting se

static peripheral_bus_driver g_proto_spi = {
	PERIPHERAL_SPI,
	{NULL},
	proto_spi_init,
	proto_spi_deinit,
	proto_spi_open,
	proto_spi_close,
	proto_spi_transceive,
	proto_spi_reset,
	proto_spi_control,
	proto_spi_delay,
	NULL};

PERIPHERAL_BUS_DRIVER_REGISTER(PERIPHERAL_SPI, g_proto_spi);

/** @addtogroup SE_Driver
 * @{
 */

/** @addtogroup PROTO
 * @brief link protocol layer.
 * @{
 */

/** @defgroup PROTO_SPI PROTO_SPI
 * @brief hed spi communication driver.
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function  -----------------------------------------------*/
/** @defgroup Proto_Spi_Private_Functions Proto_Spi Private Functions
 * @{
 */

/**
 * @brief Send data through SPI interface
 * @param [in] periph  device handle
 * @param [in] inbuf  start address of input data
 * @param [in] inbuf_len input data length
 * @return status code
 * @note no
 * @see no
 */
se_error_t proto_spi_transmit(peripheral *periph, uint8_t *inbuf, uint32_t inbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;

	HAL_SPI_PERIPHERAL_STRUCT_POINTER pSpiPeriph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if ((inbuf == NULL) || (inbuf_len == 0U))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		pSpiPeriph->chip_select(pSpiPeriph, TRUE);
		ret_code = pSpiPeriph->transmit(pSpiPeriph, inbuf, inbuf_len);
		pSpiPeriph->chip_select(pSpiPeriph, FALSE);

	} while (0);

	return ret_code;
}

/**
 * @brief Receive data through SPI interface
 * @param [in] periph  device handle
 * @param [out] outbuf  start address of the received data
 * @param [out] outbuf_len receive data length
 * @return status code
 */
se_error_t proto_spi_receive(peripheral *periph, uint8_t *outbuf, uint32_t outbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	HAL_SPI_PERIPHERAL_STRUCT_POINTER pSpiPeriph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if ((outbuf == NULL) || (outbuf_len == 0U))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		memset(outbuf, 0xff, outbuf_len);
		pSpiPeriph->chip_select(pSpiPeriph, TRUE);
		ret_code = pSpiPeriph->receive(pSpiPeriph, outbuf, &outbuf_len);
		pSpiPeriph->chip_select(pSpiPeriph, FALSE);

	} while (0);

	return ret_code;
}

/*************************************************
  Function:	  proto_spi_crc16
  Description:  Calculates the CRC value for the specified length of data
  Input:
			CRCType��calculation type of CRC
			Length��calculttion data length
			Data��start address of the calculttion data
  Return:	value of crc
  Others:
*************************************************/
uint16_t proto_spi_crc16(uint32_t CRCType, uint32_t Length, uint8_t *Data)
{
	uint8_t chBlock = 0;
	uint16_t wCrc = 0;

	wCrc = (CRCType == CRC_A) ? 0x6363 : 0xFFFF; // CRC_A : ITU-V.41 , CRC_B : ISO 3309

	do
	{
		chBlock = *Data++;
		chBlock = (chBlock ^ (uint8_t)(wCrc & 0x00FF));
		chBlock = (chBlock ^ (chBlock << 4));
		wCrc = (wCrc >> 8) ^ ((uint16_t)chBlock << 8) ^ ((uint16_t)chBlock << 3) ^ ((uint16_t)chBlock >> 4);
	} while (--Length);

	if (CRCType != CRC_A)
	{
		wCrc = ~wCrc; // ISO 3309
	}

	return wCrc;
}

/**
 * @brief Send a frame of data according to the frame format of the HED SPI communication protocol
 * @param [in] periph  device handle
 * @param [in] param  communication parameter information
 * @param [in] inbuf  start address of input data
 * @param [in] inbuf_len input data length
 * @return  status code
 * @note Protocol time parameters WPT~T3  need to meet SE chip requirements
 * @see  proto_spi_transmit
 */
se_error_t proto_spi_send_frame(peripheral *periph, spi_param_t *param, uint8_t *inbuf, uint32_t inbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;

	uint8_t bData[WAKEUP_DATA_LEN + 1] = {0};
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if ((param == NULL) || (inbuf == NULL) || (inbuf_len < FRAME_HEAD_LEN))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		memset(bData, 0x00, (WAKEUP_DATA_LEN + 1));							 // debug
		ret_code = proto_spi_transmit(periph, bData, (WAKEUP_DATA_LEN + 1)); // send wakeup data
		if (ret_code != SE_SUCCESS)
		{
			break;
		}

		p_spi_periph->delay(SPI_SEND_CS_WAKEUP_TIME); // delay WPT

		ret_code = proto_spi_transmit(periph, inbuf, inbuf_len); // send PIB LEN DATA EDC
		if (ret_code != SE_SUCCESS)
		{
			break;
		}

#ifndef SPI_SUPPORT_GPIO_IRQ_RECEIVE
		// judge if SM2
		if ((inbuf[APDU_CLA_OFFSET] == 0x80) && (inbuf[APDU_INS_OFFSET] == 0x36) && (((inbuf[APDU_P1_OFFSET] & 0x70) >> 4) == 0x01))
		{
			// T3 is seted to  6.6ms
			p_spi_periph->delay(SPI_SEND_DATA_OVER_WAIT_TIME_FOR_SM2_SIGN); // delay T3
		}
		else
		{
			// T3 is seted to 200us
			p_spi_periph->delay(SPI_SEND_DATA_OVER_WAIT_TIME); // delay T3
		}
#endif

#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
		ret_code = p_spi_periph->gpio_irqwait_edge(SPI_RECEVIE_FRAME_WAIT_GPIO_RISING_IRQ_TIME);
		if (ret_code != SE_SUCCESS)
		{
			LOGE("\npoll gpio rising stat timeout!\n");
			return ret_code;
		}
#endif

	} while (0);

	return ret_code;
}

/**
 * @brief According to the frame format of the HED SPI communication protocol, receive the frame header (start field: PIB LEN)
 * @param [in] periph  device handle
 * @param [out] outbuf  start address of the frame header to be received
 * @param [out] outbuf_len  received data length
 * @return status code
 * @note Protocol time parameters T0~T3 need to meet SE chip requirements
 * @see   proto_spi_receive
 */
se_error_t proto_spi_receive_frame_head(peripheral *periph, uint8_t *outbuf, uint32_t *outbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;

	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if ((outbuf == NULL) || (outbuf_len == NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		ret_code = proto_spi_receive(periph, outbuf, FRAME_HEAD_LEN); // recieve PIB LEN1 LEN2
		if (ret_code != SE_SUCCESS)
		{
			break;
		}
		*outbuf_len = (*(outbuf + LEN_OFFSET) << 8) + *(outbuf + LEN_OFFSET + 1);

	} while (0);

	return ret_code;
}

/**
 * @brief According to the frame format of the HED SPI communication protocol, receive frame data (information field/termination field)
 * @param [in] periph  device handle
 * @param [in] param  communication parameter information
 * @param [out] outbuf  start address of the received data
 * @param [out] outbuf_len  receive data length
 * @return  status code
 * @note Protocol time parameter T5 need to meet SE chip requirements
 * @see   proto_spi_receive
 */
se_error_t proto_spi_receive_frame_data(peripheral *periph, spi_param_t *param, uint8_t *outbuf, uint32_t outbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;

	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if ((param == NULL) || (outbuf == NULL) || (outbuf_len == 0))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		p_spi_periph->delay(SPI_BEFORE_RECEIVE_DATA_WAIT_TIME); // delay T5

		ret_code = proto_spi_receive(periph, outbuf, outbuf_len);
		if (ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

	} while (0);

	return ret_code;
}

/**
* @brief  According to the frame format of the HED SPI communication protocol,
		  receive the frame to activate the frame data,
		  and start the timing operation of the received data timeout according to the frame waiting time while waiting for the reception
* @param [in] periph      device handle
* @param [in] param       communication parameter information
* @param [out] outbuf     start address of the activation frame to be received
* @param [out] outbuf_len received data length
* @return status code
* @note Protocol time parameter T4 need to meet SE chip requirements
* @see  proto_spi_receive_frame_head
*/
se_error_t proto_spi_receive_active_frame(peripheral *periph, spi_param_t *param, uint8_t *output, uint32_t *output_len)
{
	se_error_t retCode = SE_SUCCESS;
	util_timer_t timer = {0};
	uint16_t edc_value;
	uint32_t len = 0;
	uint16_t send_nak_count = 0;
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	if (periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((param == NULL) || (output == NULL) || (*output_len < FRAME_HEAD_LEN))
	{
		return SE_ERR_PARAM_INVALID;
	}

	do
	{

		timer.interval = SPI_RECEVIE_FRAME_WAIT_TIME;
		p_spi_periph->timer_start(&timer);

		// receive frame head
		do
		{
			p_spi_periph->delay(SPI_RESET_POLL_SLAVE_INTERVAL_TIME); // delay T4
			retCode = proto_spi_receive_frame_head(periph, output, &len);
			if (retCode != SE_SUCCESS)
				return retCode;

			if (output[PIB_OFFSET] == PIB_ACTIVE_FRAME)
			{
				break;
			}

			// check whether the received is a process frame
			if (output[PIB_OFFSET] == PIB_PROCESS_FRAME)
			{
				break;
			}
			if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
			{
				LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);

				return SE_ERR_TIMEOUT;
			}
		} while (1);

		if ((len + FRAME_HEAD_LEN) > *output_len)
		{
			retCode = SE_ERR_LEN;
			break;
		}

		retCode = proto_spi_receive_frame_data(periph, param, output + FRAME_HEAD_LEN, len);
		if (retCode != SE_SUCCESS)
		{
			break;
		}

		printf("%s: PIB + LEN + DATA + EDC[", __func__);
		for (int i = 0; i < (FRAME_HEAD_LEN + len); i++)
			printf("0x%02x ", output[i]);
		printf("]\n");

		// check edc
		edc_value = 0;
		// len = len+FRAME_HEAD_LEN;
		edc_value = proto_spi_crc16(CRC_B, len + FRAME_HEAD_LEN - EDC_LEN, output);
		if (((edc_value >> 8) & 0xff) != output[len + FRAME_HEAD_LEN - EDC_LEN + 1] || (edc_value & 0xff) != output[len + FRAME_HEAD_LEN - EDC_LEN])
		{

			send_nak_count++;
			if (send_nak_count > PROTO_SPI_RETRY_NUM)
			{
				return SE_ERR_LRC_CRC;
			}

			p_spi_periph->delay(SPI_SEND_BGT_TIME);													  // delay BGT
			retCode = proto_spi_send_frame(periph, param, (uint8_t *)cNAK_HED_SE, PROCESS_FRAME_LEN); // send NAK frame
			if (retCode != SE_SUCCESS)
			{
				return retCode;
			}
			continue;
		}

		*output_len = len + FRAME_HEAD_LEN;
		break;

	} while (1);

	return retCode;
}

/**
 * @brief send reset request frame 030004D3xxxxxx , receive response frame030004D3xxxxxxx
 * @param [in] periph  device handle
 * @param [in] param  communication parameter information
 * @return status code
 * @see  proto_spi_send_frame  proto_spi_receive_active_frame
 */
se_error_t proto_spi_reset_frame(peripheral *periph, spi_param_t *param, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
	uint32_t bufsize = 0;
	// uint8_t bApduData[ACTIVE_REQ_FRAME_LEN] = {0};
	uint8_t tmp_buf[ACTIVE_REQ_FRAME_LEN] = {0};
	uint16_t spi_edc = 0;
	uint16_t rec_nak_count = 0;
	uint16_t rec_time_out_count = 0;

	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if (param == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}
		memcpy((uint8_t *)tmp_buf, &cRESETAPDU_HED_SE[PIB_OFFSET], ACTIVE_REQ_FRAME_LEN);

		tmp_buf[4] = param->pfsm / 16;
		spi_edc = proto_spi_crc16(CRC_B, ACTIVE_REQ_FRAME_LEN - 2, tmp_buf);
		tmp_buf[ACTIVE_REQ_FRAME_LEN - 2] = (uint8_t)spi_edc & 0xff;		// fill EDC_HIGH
		tmp_buf[ACTIVE_REQ_FRAME_LEN - 1] = (uint8_t)(spi_edc >> 8) & 0xff; // fill EDC_LOW

		// send reset frame
		ret_code = proto_spi_send_frame(periph, param, tmp_buf, ACTIVE_REQ_FRAME_LEN);
		if (ret_code != SE_SUCCESS)
		{
			break;
		}
		bufsize = *rlen;

		// receive response to reset frame
		ret_code = proto_spi_receive_active_frame(periph, param, rbuf, &bufsize);

		if (ret_code != SE_SUCCESS)
		{
			if (ret_code == SE_ERR_TIMEOUT) // resend when timeout
			{
				rec_time_out_count++;
				if (rec_time_out_count < 2)
				{
					continue;
				}
			}
			break;
		}

		// check whether the received is a NAK frame
		if (rbuf[DATA_OFFSET] == PIB_PROCESS_FRAME_NAK_CRC_INFO)
		{
			// ret_code = SE_ERR_RESET;
			rec_nak_count++;
			if (rec_nak_count > PROTO_SPI_RETRY_NUM) // retransmit 3 times
			{
				LOGE("Failed:communication cannot recover,  ErrCode-%08X.", SE_ERR_COMM);
				ret_code = SE_ERR_COMM;
				return SE_ERR_COMM;
			}
			continue;
		}

		// check whether the received is response to reset frame
		if (rbuf[DATA_OFFSET] != PIB_ACTIVE_FRAME_RESET)
		{
			// ret_code = SE_ERR_RESET;
			ret_code = SE_ERR_DATA;
			break;
		}

		*rlen = bufsize;

		if (bufsize != ACTIVE_REQ_FRAME_LEN) // frame length of reset
		{
			ret_code = SE_ERR_LEN;
			break;
		}
		break;

	} while (1);

	return ret_code;
}

/**
 * @brief send RATR command sequence to get ATR (send 030003E2XXXXXX, receive ATR information)
 * @param [in] periph     device handle
 * @param [in] param      communication parameter information
 * @param [out] rbuf      start address of the ATR to be received
 * @param [out] rlen      received data length
 * @return status code
 * @see  proto_spi_send_frame  proto_spi_receive_active_frame
 */
se_error_t proto_spi_ratr_frame(peripheral *periph, spi_param_t *param, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
	uint32_t bufsize = 0;
	uint8_t tmp_buf[ACTIVE_REQ_FRAME_LEN] = {0};
	uint16_t spi_edc = 0;
	uint16_t rec_nak_count = 0;
	uint16_t rec_time_out_count = 0;

	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if ((param == NULL) || (rbuf == NULL) || (rlen == NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		memcpy((uint8_t *)tmp_buf, &cRATRAPDU_HED_SE[PIB_OFFSET], ACTIVE_REQ_FRAME_LEN);

		tmp_buf[4] = param->hbsm / 16;
		spi_edc = proto_spi_crc16(CRC_B, ACTIVE_REQ_FRAME_LEN - 2, tmp_buf);
		tmp_buf[ACTIVE_REQ_FRAME_LEN - 2] = spi_edc & 0xff;		   // fill EDC_HIGH
		tmp_buf[ACTIVE_REQ_FRAME_LEN - 1] = (spi_edc >> 8) & 0xff; // fill EDC_LOW

		ret_code = proto_spi_send_frame(periph, param, tmp_buf, ACTIVE_REQ_FRAME_LEN);
		if (ret_code != SE_SUCCESS)
		{
			break;
		}

		bufsize = *rlen;

		ret_code = proto_spi_receive_active_frame(periph, param, rbuf, &bufsize);
		if (ret_code != SE_SUCCESS)
		{
			// resend one time whien timeout
			if (ret_code == SE_ERR_TIMEOUT)
			{
				rec_time_out_count++;
				if (rec_time_out_count < 2)
				{
					continue;
				}
			}

			break;
		}

		// check whether the received is a NAK frame
		if (rbuf[DATA_OFFSET] == PIB_PROCESS_FRAME_NAK_CRC_INFO)
		{
			// ret_code = SE_ERR_RESET;
			rec_nak_count++;
			if (rec_nak_count > PROTO_SPI_RETRY_NUM) // retransmit 3 times
			{
				ret_code = SE_ERR_LRC_CRC;
				break;
			}
			else
			{
				continue;
			}
		}

		// check whether the received is response to ratr frame
		if (rbuf[DATA_OFFSET] != PIB_ACTIVE_FRAME_RATR_RESPONSE)
		{
			ret_code = SE_ERR_ATR;
			break;
		}

		*rlen = bufsize - FRAME_HEAD_LEN - EDC_LEN;
		break;

	} while (1);

	return ret_code;
}

/**
* @brief According to the frame format of the HED SPI communication protocol to exchange data with the SE
* @param [in] periph  device handle
* @param [in] param  communication parameter information
* @param [in] inbuf  input data
* @param [in] inbuf_len input data length
* @param [out] outbuf  output data
* @param [out] outbuf_len output data length
* return status code
* @note During the interaction, it must be based on the received frame type and EDC check, etc.,
		transmission delay frame, NAK frame and error frame retransmission operation
* @see  proto_spi_send_frame  proto_spi_receive_frame_head  proto_spi_receive_frame_data
*/
se_error_t proto_spi_handle(peripheral *periph, spi_param_t *param, uint8_t *inbuf, uint32_t inbuf_len, uint8_t *outbuf, uint32_t *outbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint16_t edc_value;
	uint32_t rec_len = 0;
	// uint8_t dev_id = 0;
	uint16_t send_nak_count = 0;
	uint16_t rec_nak_count = 0;
	// uint16_t rec_wtx_count = 0;
	uint16_t re_tran_count = 0;
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	if (periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((param == NULL) || (inbuf == NULL) || (inbuf_len > FRAME_LEN_MAX) || (outbuf == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}

	// 1.send frame data
	param->type = SPI_INFO;
	ret_code = proto_spi_send_frame(periph, param, inbuf, inbuf_len);
	if (ret_code != SE_SUCCESS)
		return ret_code;

	// 2.set the timeout time of frame waiting when receiving data
	timer.interval = SPI_RECEVIE_FRAME_WAIT_TIME;
	p_spi_periph->timer_start(&timer);

	do
	{
		// 3.the start field of the received frame data, it will time out if it is not received within the frame waiting time
		do
		{
			ret_code = proto_spi_receive_frame_head(periph, outbuf, &rec_len);
			if (ret_code != SE_SUCCESS)
			{
				return ret_code;
			}

			if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
			{
				re_tran_count++;
				break;
			}

			if ((outbuf[PIB_OFFSET] == PIB_INFORMATION_FRAME) || (outbuf[PIB_OFFSET] == PIB_PROCESS_FRAME))
			{
				re_tran_count = 0;
				break;
			}

			if ((outbuf[PIB_OFFSET] == 0x00) && (outbuf[PIB_OFFSET + 1] == 0x00) && (outbuf[PIB_OFFSET + 2] == 0x00))
			{
				p_spi_periph->delay(SPI_BEFORE_RECEIVE_DATA_WAIT_TIME); // delay T5
			}
			else
			{
				p_spi_periph->delay(SPI_RESET_POLL_SLAVE_INTERVAL_TIME); // delay T4
			}

		} while (1);

		// retransmit the information frame when timeout
		if (re_tran_count == 1)
		{
			p_spi_periph->delay(SPI_SEND_BGT_TIME); // delay BGT
			ret_code = proto_spi_send_frame(periph, param, inbuf, inbuf_len);
			// re timing
			timer.interval = SPI_RECEVIE_FRAME_WAIT_TIME;
			p_spi_periph->timer_start(&timer);
			continue;
		}

		// If it times out again, an error code is returned
		else if (re_tran_count == 2)
		{
			LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);
			return SE_ERR_TIMEOUT;
		}

		if (rec_len > (FRAME_DATA_LEN_MAX + EDC_LEN))
		{
			return SE_ERR_LEN;
		}

		// 4.information field and termination field of received frame data
		ret_code = proto_spi_receive_frame_data(periph, param, outbuf + FRAME_HEAD_LEN, rec_len);
		if (ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		// 5.check the EDC value of the received frame data
		edc_value = 0;
		rec_len = rec_len + FRAME_HEAD_LEN;
		edc_value = proto_spi_crc16(CRC_B, rec_len - EDC_LEN, outbuf);
		if ((((edc_value >> 8) & 0xff) == outbuf[rec_len - EDC_LEN + 1]) && ((edc_value & 0xff) == outbuf[rec_len - EDC_LEN])) // EDC true
		{
			send_nak_count = 0;

			// 6.check the received frame type and process it according to the frame type
			if (((outbuf[PIB_OFFSET] == PIB_PROCESS_FRAME) && (outbuf[DATA_OFFSET] == PIB_PROCESS_FRAME_NAK_CRC_INFO)) || ((outbuf[PIB_OFFSET] == PIB_PROCESS_FRAME) && (outbuf[DATA_OFFSET] == PIB_PROCESS_FRAME_NAK_OTHER_INFO)))
			{
				// the received frame type is NAK frame, and the frame data will be retransmitted.
				// If the error exceeds three times, the error code is returned.
				if (param->type == SPI_INFO)
				{
					rec_nak_count++;
					if (rec_nak_count > PROTO_SPI_RETRY_NUM)
					{
						return SE_ERR_LRC_CRC;
					}
					p_spi_periph->delay(SPI_SEND_BGT_TIME);							  // delay BGT
					ret_code = proto_spi_send_frame(periph, param, inbuf, inbuf_len); // retransmit information frame
				}
				else if (param->type == SPI_WTX)
				{
					p_spi_periph->delay(SPI_SEND_BGT_TIME);													   // delay BGT
					ret_code = proto_spi_send_frame(periph, param, (uint8_t *)cWTX_HED_SE, PROCESS_FRAME_LEN); // retransmit the WTX frame
				}

				if (ret_code != SE_SUCCESS) // retransmiting or sending wtx failed
				{
					return ret_code;
				}
			}

			else if ((outbuf[PIB_OFFSET] == PIB_PROCESS_FRAME) && (outbuf[DATA_OFFSET] == PIB_PROCESS_FRAME_WTX_INFO))
			{
				// the received frame type is a delayed request, and the delayed frame is sent in response
				param->type = SPI_WTX;
				ret_code = proto_spi_send_frame(periph, param, (uint8_t *)cWTX_HED_SE, PROCESS_FRAME_LEN);
				if (ret_code != SE_SUCCESS)
				{
					return ret_code;
				}
				// recalculate timeout
				timer.interval = SPI_RECEVIE_FRAME_WAIT_TIME;
				p_spi_periph->timer_start(&timer);
			}

			else if (outbuf[PIB_OFFSET] == PIB_INFORMATION_FRAME)
			{
				// the received frame type is information frame, exit the frame data receiving and sending interactive processing
				*outbuf_len = rec_len;
				return SE_SUCCESS;
			}
			else
			{
				return SE_ERR_PARAM_INVALID;
			}
		}
		else // EDC error
		{
			send_nak_count++;
			if (send_nak_count > PROTO_SPI_RETRY_NUM)
			{
				return SE_ERR_LRC_CRC;
			}

			p_spi_periph->delay(SPI_SEND_BGT_TIME);													   // delay BGT
			ret_code = proto_spi_send_frame(periph, param, (uint8_t *)cNAK_HED_SE, PROCESS_FRAME_LEN); // send NAK frame to slave device
			if (ret_code != SE_SUCCESS)
			{
				return ret_code;
			}
		}
	} while (1);
}

/**
 * @brief According to the HED SPI protocol format, add frame header and frame end data to the two-way queue
 * -# Add PIB, LEN to the head of the double queue
 * -# Add EDC to the tail of the double queue
 * @param [in] periph    device handle
 * @param [in] inbuf     start address of the two-way queue
 * @param [in] inbuf_len the length of the two-way queue
 * @return status code
 * @note no
 * @see no
 */
se_error_t proto_spi_queue_in(peripheral *periph, uint8_t *inbuf, uint16_t inbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	double_queue queue_in = (double_queue)inbuf;
	uint16_t frame_len = 0;
	uint8_t frame_head[3] = {0};
	uint16_t spi_edc = 0;
	uint8_t tmp_buf[2] = {0};
	do
	{
		if (periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if ((inbuf == NULL) || (inbuf_len == 0U))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		// Add starting domains to the double queue separately(PIB, LEN)
		frame_len = (uint16_t)(inbuf_len + EDC_LEN);
		frame_head[0] = PIB_INFORMATION_FRAME;
		frame_head[1] = (frame_len >> 8) & 0xff; // LEN_High
		frame_head[2] = frame_len & 0xff;		 // LEN_Low
		util_queue_front_push(frame_head, FRAME_HEAD_LEN, queue_in);

		// calculate CRC value. Add termination domains to the double queue separately(EDC)
		spi_edc = proto_spi_crc16(CRC_B, util_queue_size(queue_in), &queue_in->q_buf[queue_in->front_node]); // 4 //calculate CRC
		tmp_buf[0] = spi_edc & 0xff;																		 // fill EDC_HIGH
		tmp_buf[1] = (spi_edc >> 8) & 0xff;																	 // fill EDC_LOW
		util_queue_rear_push(tmp_buf, 2, queue_in);

	} while (0);

	return ret_code;
}

/**
 * @}
 */

/* Exported functions --------------------------------------------------------*/

/** @defgroup Proto_Spi_Exported_Functions Proto_Spi Exported Functions
 * @{
 */

/**
 * @brief HED SPI communication protocol parameter initialization
 * -# FSM/FSS and other parameter initialization
 * -# Call the initialization function init of the port layer spi interface through the function list pointer registered by the port layer device
 * @param [in] periph     device handle
 * @return status code
 * @note no
 * @see  port_spi_periph_timer_start timer_start  port_spi_periph_timer_differ port_spi_periph_init
 */
se_error_t proto_spi_init(peripheral *periph)
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint8_t dev_id = 0;

	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	if (p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	// set the lock wait timeout time
	timer.interval = SPI_COMM_MUTEX_WAIT_TIME;
	p_spi_periph->timer_start(&timer);

	do
	{
		if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:init mutex,  ErrCode-%08X.", ret_code);
			break;
		}

		dev_id = p_spi_periph->periph.id;

		g_spi_param[dev_id].pfsm = PROTO_SPI_PFSM_DEFAULT;
		g_spi_param[dev_id].pfss = PROTO_SPI_PFSS_DEFAULT;
		g_spi_param[dev_id].hbsm = PROTO_SPI_HBSM_DEFAULT;
		g_spi_param[dev_id].hbss = PROTO_SPI_HBSS_DEFAULT;

		ret_code = p_spi_periph->init(p_spi_periph);
		if (ret_code != SE_SUCCESS)
		{
			LOGE("Failed:spi potocol,  ErrCode-%08X.", ret_code);
		}
		else
		{
			LOGI("Success!");
		}
		break;
	} while (1);

	return ret_code;
}

/**
 * @brief HED SPI communication protocol parameter termination
 * -# Restore default values for FSM/FSS and other parameters
 * -# Call the deinit of the port layer spi interface through the function list pointer registered by the port layer device
 * @param [in] periph  device handle
 * @return status code
 * @note no
 * @see  port_spi_periph_timer_start timer_start  port_spi_periph_timer_differ port_spi_periph_deinit
 */
se_error_t proto_spi_deinit(peripheral *periph)
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};

	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	if (p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	// set the waiting timeout time
	timer.interval = SPI_COMM_MUTEX_WAIT_TIME;
	p_spi_periph->timer_start(&timer);

	do
	{
		if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:deinit mutex,  ErrCode-%08X.", ret_code);
			break;
		}

		ret_code = p_spi_periph->deinit(p_spi_periph);
		if (ret_code != SE_SUCCESS)
		{
			LOGE("Failed:spi potocol,  ErrCode-%08X.", ret_code);
		}
		else
		{
			LOGI("Success!");
		}
		break;

	} while (1);

	return ret_code;
}

/**
 * @brief When connecting a slave device, call this function to get the ATR of the slave device
 * -# Through the function list pointer registered by the port layer device
 * -# Send reset command sequence
 * -# Send the RATR command sequence to get the ATR value of the slave device
 * @param [in] periph      device handle
 * @param [out] rbuf       start address of the ATR to be received
 * @param [out] rlen       received data length
 * @return status code
 * @note no
 * @see  port_spi_periph_timer_start port_spi_periph_timer_differ proto_spi_reset_frame  proto_spi_ratr_frame
 */
se_error_t proto_spi_open(peripheral *periph, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint32_t rec_len = 0;
	uint8_t dev_id = 0;
	// uint8_t atr_buf[PROTO_SPI_ATR_MAX_LEN+4] = {0};
	// uint8_t reset_buf[PROTO_SPI_RERESET_MAX_LEN ] = {0};
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;
	uint16_t pfss = 0;
	uint16_t hbss = 0;
	uint16_t pfssi[16] = {0, 16, 32, 64, 128, 256, 272, 384, 512, 1024, 2048, 4096, 8192, 16384, 16384, 16384};
	uint8_t index = 0;
	if (p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL)) || ((rbuf != NULL) && (rlen == NULL)))
	{
		return SE_ERR_PARAM_INVALID;
	}

	// set the lock wait timeout time
	timer.interval = SPI_COMM_MUTEX_WAIT_TIME;
	p_spi_periph->timer_start(&timer);

	do
	{
		if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:open periph mutex,  ErrCode-%08X.", ret_code);
			break;
		}

		// ret_code = p_spi_periph->control(p_spi_periph, PROTO_SPI_CTRL_RST, NULL, NULL);
		// if(ret_code != SE_SUCCESS)
		//	{
		//	LOGE("Failed:protocol rst io control,  ErrCode-%08X.", ret_code);
		// break;
		//}

		p_spi_periph->delay(SPI_PROTO_SE_RST_DELAY); // delay 350ms
		dev_id = p_spi_periph->periph.id;

		if (g_bSPIHedOpenSeMode == HED20_SPI_OPEN_SE_RESET_REQ)
		{
			// send the reset command sequence
			rec_len = PROTO_SPI_RERESET_MAX_LEN;
			ret_code = proto_spi_reset_frame(periph, &g_spi_param[dev_id], reset_buf, &rec_len);
			if (ret_code == SE_ERR_BUSY)
			{
				continue;
			}
			else if (ret_code != SE_SUCCESS)
			{
				LOGE("Failed:protocol reset  frame,  ErrCode-%08X.", ret_code);
				break;
			}

			// pfss = (uint16_t)reset_buf[DATA_OFFSET+1]*16;

			index = reset_buf[DATA_OFFSET + 1];
			if (index > 16)
			{
				index = 16;
			}
			pfss = pfssi[index];
			if (g_spi_param[dev_id].pfsm >= pfss)
			{
				g_spi_param[dev_id].pfsm = pfss;
				g_spi_param[dev_id].pfss = pfss;
			}
			else
			{
				g_spi_param[dev_id].pfss = g_spi_param[dev_id].pfsm;
			}
		}

		if ((rbuf != NULL) && (rlen != NULL))
		{
			p_spi_periph->delay(SPI_SEND_BGT_TIME);

			// send RATR command sequence to get ATR value
			rec_len = PROTO_SPI_ATR_MAX_LEN;
			ret_code = proto_spi_ratr_frame(periph, &g_spi_param[dev_id], atr_buf, &rec_len);
			if (ret_code == SE_ERR_BUSY)
			{
				continue;
			}
			else if (ret_code != SE_SUCCESS)
			{
				LOGE("Failed:protocol ratr  frame,  ErrCode-%08X.", ret_code);
				break;
			}

			memmove(rbuf, atr_buf + DATA_OFFSET, rec_len);

			hbss = (uint16_t)atr_buf[DATA_OFFSET + 2] * 16;
			if (g_spi_param[dev_id].hbsm >= hbss)
			{
				g_spi_param[dev_id].hbsm = hbss;
				g_spi_param[dev_id].hbss = hbss;
			}
			else
			{
				g_spi_param[dev_id].hbss = g_spi_param[dev_id].hbsm;
			}

			// rbuf[2] = 0x11;
			*rlen = rec_len;
		}
		LOGI("Open Periph Success!");
		break;
	} while (1);

	return ret_code;
}

/**
 * @brief close the device
 * @param [in] periph  device handle
 * @return status code
 * @note  RFU
 */
se_error_t proto_spi_close(peripheral *periph)
{
	se_error_t ret_code = SE_SUCCESS;

	return ret_code;
}

/**
 * @brief Send and receive data through the SPI interface
 * -# Add starting domains to the double queue separately(PIB, LEN)
 * -# Add termination domains to the double queue separately(EDC)
 * -# Call the proto_spi_handle function to send commands and receive command responses
 * @param [in] periph    device handle
 * @param [in] sbuf      start address of the input two-way queue
 * @param [in] slen      the input two-way queue data length
 * @param [out] rbuf     start address of the output two-way queue
 * @param [out] rlen     the output two-way queue data length
 * @return status code
 * @see  port_spi_periph_timer_start  port_spi_periph_timer_differ proto_spi_queue_in  proto_spi_handle
 */
se_error_t proto_spi_transceive(peripheral *periph, uint8_t *sbuf, uint32_t slen, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
	//	se_error_t ret_code_bak = SE_SUCCESS;
	util_timer_t timer = {0};
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;
	double_queue queue_in = (double_queue)sbuf;
	double_queue queue_out = (double_queue)rbuf;
	uint16_t rec_len = 0;
	uint8_t dev_id = 0;
	uint8_t *p_input = NULL;
	uint8_t *p_output = NULL;
	uint8_t reset_buf[PROTO_SPI_RERESET_MAX_LEN] = {0};

	if (p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((sbuf == NULL) || (rbuf == NULL) || (slen == 0U) || (rlen == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}

	// data to be sent and received
	ret_code = proto_spi_queue_in(periph, sbuf, slen);
	if (ret_code != SE_SUCCESS)
	{
		return ret_code;
	}

	p_input = &queue_in->q_buf[queue_in->front_node];
	p_output = &queue_out->q_buf[queue_out->front_node];
	dev_id = p_spi_periph->periph.id;

	// set the waiting timeout time
	timer.interval = SPI_COMM_MUTEX_WAIT_TIME;
	p_spi_periph->timer_start(&timer);

	do
	{
		if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:communication mutex,  ErrCode-%08X.", ret_code);
			break;
		}

		// start sending and receiving of SPI data
		ret_code = proto_spi_handle(periph, &g_spi_param[dev_id], p_input, util_queue_size(queue_in), p_output, (uint32_t *)&rec_len);
		if (ret_code == SE_ERR_BUSY)
		{
			continue;
		}

		else if (ret_code == SE_ERR_LRC_CRC || ret_code == SE_ERR_TIMEOUT)
		{

			// ����reset��������
			p_spi_periph->delay(SPI_SEND_BGT_TIME); // delay BGT
			rec_len = PROTO_SPI_RERESET_MAX_LEN;

			if ((proto_spi_reset_frame(periph, &g_spi_param[dev_id], reset_buf, (uint32_t *)&rec_len)) != SE_SUCCESS)
			{
				LOGE("Failed:communication cannot recover,  ErrCode-%08X.", SE_ERR_COMM);
				return SE_ERR_COMM;
			}
			else
			{
				switch (ret_code)
				{
				case SE_ERR_LRC_CRC:
					LOGE("Failed:check lrc,  ErrCode-%08X.", SE_ERR_LRC_CRC);
					break;
				case SE_ERR_TIMEOUT:
					LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);
					break;
				}
			}
			break;
		}

		else if (ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol communication,  ErrCode-%08X.", ret_code);
			break;
		}

		queue_out->q_buf_len = rec_len;
		queue_out->rear_node = queue_out->front_node + rec_len;
		util_queue_front_pop(FRAME_HEAD_LEN, queue_out); // remove start domain data
		util_queue_rear_pop(EDC_LEN, queue_out);		 // remove terminating domain
		*rlen = util_queue_size(queue_out);
		LOGI("Communication Success!");
		break;
	} while (1);

	return ret_code;
}

/**
 * @brief reset the device and get the ATR of the slave device
 * -# Through the function list pointer registered by the port layer device, call the control function of the port layer spi interface to reset the SE
 * -# Send reset command sequence
 * -# Send the RATR command sequence to get the ATR value of the slave device
 * @param [in] periph    device handle
 * @param [out] rbuf     start address of the ATR to be received
 * @param [out] rlen     ATR length
 * @return status code
 * @note no
 * @see port_spi_periph_timer_start  port_spi_periph_timer_differ port_spi_periph_control  proto_spi_reset_frame  proto_spi_ratr_frame
 */
se_error_t proto_spi_reset(peripheral *periph, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint32_t rec_len = 0;
	uint8_t dev_id = 0;
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	if (p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL)) || ((rbuf != NULL) && (rlen == NULL)))
	{
		return SE_ERR_PARAM_INVALID;
	}

	// set the waiting timeout time
	timer.interval = SPI_COMM_MUTEX_WAIT_TIME;
	p_spi_periph->timer_start(&timer);

	do
	{
		if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:open periph mutex,  ErrCode-%08X.", ret_code);
			break;
		}

		// control the RST pin of SE for reset operation
		ret_code = p_spi_periph->control(p_spi_periph, PROTO_SPI_CTRL_RST, NULL, NULL);
		if (ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol rst io control,  ErrCode-%08X.", ret_code);
			break;
		}

		p_spi_periph->delay(SPI_PROTO_SE_RST_DELAY); // delay 350ms

		dev_id = p_spi_periph->periph.id;

		if (g_bSPIHedRstSeMode == HED20_SPI_RESET_SE_RESET_REQ)
		{
			// send the reset command sequence
			rec_len = PROTO_SPI_RERESET_MAX_LEN;
			ret_code = proto_spi_reset_frame(periph, &g_spi_param[dev_id], reset_buf, &rec_len);
			if (ret_code == SE_ERR_BUSY)
			{
				continue;
			}
			else if (ret_code != SE_SUCCESS)
			{
				LOGE("Failed:protocol reset  frame,  ErrCode-%08X.", ret_code);
				break;
			}
		}

		if ((rbuf != NULL) && (rlen != NULL))
		{
			p_spi_periph->delay(SPI_SEND_BGT_TIME);
			// send RATR command sequence to get ATR value
			rec_len = PROTO_SPI_ATR_MAX_LEN;
			ret_code = proto_spi_ratr_frame(periph, &g_spi_param[dev_id], atr_buf, &rec_len);
			if (ret_code == SE_ERR_BUSY)
			{
				continue;
			}
			else if (ret_code != SE_SUCCESS)
			{
				LOGE("Failed:protocol ratr  frame,  ErrCode-%08X.", ret_code);
				break;
			}

			memmove(rbuf, atr_buf + DATA_OFFSET, rec_len);

			// rbuf[2] = 0x11;
			*rlen = rec_len;
		}
		LOGI("Open Periph Success!");
		break;
	} while (1);

	return ret_code;
}

/**
 * @brief Send the RST control pin to the slave device through the SPI interface to reset the control command or other control commands
 * @param [in] periph      device handle
 * @param [in] ctrlcode    command control code
 * @param [in] sbuf        start address of input data
 * @param [in] slen        length of input data
 * @param [out] rbuf       start address of output data
 * @param [out] rlen       length of output data
 * @return status code
 * @see   port_spi_periph_timer_start  port_spi_periph_timer_differ  port_spi_periph_control
 */
se_error_t proto_spi_control(peripheral *periph, uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	if (p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (ctrlcode == 0U)
	{
		return SE_ERR_PARAM_INVALID;
	}

	timer.interval = SPI_COMM_MUTEX_WAIT_TIME;
	p_spi_periph->timer_start(&timer);

	do
	{
		if (p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;
			LOGE("Failed:control periph mutex,  ErrCode-%08X.", ret_code);
			break;
		}
		ret_code = p_spi_periph->control(p_spi_periph, ctrlcode, sbuf, &slen);
		if (ret_code != SE_SUCCESS)
		{
			LOGE("Failed:spi potocol,  ErrCode-%08X.", ret_code);
		}
		else
		{
			LOGI("Success!");
		}
		break;
	} while (1);

	return ret_code;
}

/**
 * @brief microsecond delay
 * @param [in] periph  device handle
 * @param [in] us  microsecond
 * @return status code
 * @note no
 * @see  port_spi_periph_control delay
 */
se_error_t proto_spi_delay(peripheral *periph, uint32_t us)
{
	se_error_t ret_code = SE_SUCCESS;
	HAL_SPI_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_PERIPHERAL_STRUCT_POINTER)periph;

	if (p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	p_spi_periph->delay(us);
	return ret_code;
}

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
