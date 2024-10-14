/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		proto_i2c.c
 Author:			zhengwd 
 Version:			V1.0	
 Date:			2021-04-26	
 Description:	        
 History:		

******************************************************************************/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "proto_i2c.h"
#include "error.h"
#include "log.h"


/**************************************************************************
* Variable Declaration
***************************************************************************/
uint8_t   g_bI2cHedPfsmi=0;	 //Max Protocol Frame Size For Master Integer: PFSMI
uint16_t   g_sI2cHedPfsm=0;	 //Max Protocol Frame Size For Master : PFSM = (PFSMI*16), Chained transmission is not supported if PFSMI is 0
uint16_t   g_sI2cHedPfss=0;	 //Max Protocol Frame Size For Master: PFSS = (PFSSI*16), Chained transmission is not supported if PFSSI is 0
uint8_t   g_bI2cHedOpenSeMode = HED20_I2C_OPEN_SE_RESET_REQ;              //Operation mode when opening se
uint8_t   g_bI2cHedRstSeMode = HED20_I2C_RESET_SE_RESET_REQ;                //Operation mode when resetting se

static peripheral_bus_driver g_proto_i2c = {
    PERIPHERAL_I2C,
   {NULL},
    proto_i2c_init,
    proto_i2c_deinit,
    proto_i2c_open,
    proto_i2c_close,
    proto_i2c_transceive,
    proto_i2c_reset,
    proto_i2c_control,
    proto_i2c_delay,
    NULL
};


PERIPHERAL_BUS_DRIVER_REGISTER(PERIPHERAL_I2C, g_proto_i2c);



/*************************************************
  Function:	  proto_i2c_crc16
  Description:  Calculates the CRC value for the specified length of data
  Input:	
            CRCType:calculation type of CRC
            Length:calculttion data length
            Data:start address of the calculttion data
  Return:	value of crc	
  Others:		
*************************************************/
uint16_t proto_i2c_crc16(uint32_t CRCType,uint32_t Length ,uint8_t *Data)
{
	uint8_t chBlock = 0;
	uint16_t wCrc = 0;

	wCrc = (CRCType == CRC_A) ? 0x6363 : 0xFFFF;	// CRC_A : ITU-V.41 , CRC_B : ISO 3309

	do
	{
		chBlock = *Data++;
		chBlock = (chBlock^(uint8_t)(wCrc & 0x00FF));
		chBlock = (chBlock^(chBlock<<4));
		wCrc= (wCrc >> 8)^((uint16_t)chBlock << 8)^((uint16_t)chBlock<<3)^((uint16_t)chBlock>>4);
	} while (--Length);

	if (CRCType != CRC_A)
	{
		wCrc = ~wCrc; // ISO 3309
	}

	return wCrc;
}



/****************************************************************** 
Function:		proto_i2c_send_atr_rs_frame
Description:	Send R frame , S frame or frame of ATR request
		1.organize frame data according to the type of frame
		2.Send frame data throuth transmit function
Input:	  periph   device handle
          	  frame_type  type of frame
		  frame_param parameter of frame
Output: 	  no	
Return:       status code
Others: 	   no 
******************************************************************/
se_error_t proto_i2c_send_atr_rs_frame(peripheral *periph, uint8_t frame_type, uint8_t  frame_param)
{
	se_error_t stErrCode = SE_SUCCESS;
	uint16_t sOff = 0;
	uint16_t sCrc = 0;
	uint8_t abFrameBuf[PROTO_I2C_RS_FRAME_SIZE] = {0};
  util_timer_t timer = {0};
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;	

	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	pI2cPeriph->delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);

	//1.organize PIB LEN data of R frame , S frame or frame of ATR request frame
	abFrameBuf[PROTO_I2C_PIB_OFFSET] = frame_type|(frame_param&0x0F);
	abFrameBuf[PROTO_I2C_LEN_OFFSET] = 0x00;
	abFrameBuf[PROTO_I2C_LEN_OFFSET+1] = 0x00;
	sOff = sOff + PROTO_I2C_PIB_LEN_SIZE;

	//2.calculate CRC value. organize EDC data of R frame , S frame or frame of ATR request frame
	sCrc = proto_i2c_crc16(CRC_B,sOff, abFrameBuf); 	//4 
	abFrameBuf[sOff] = sCrc&0xFF;
	abFrameBuf[sOff+1] = (sCrc>>8)&0xFF;
	sOff += PROTO_I2C_EDC_SIZE;
  
  //set the lock wait timeout time
	timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME; 
	pI2cPeriph->timer_start(&timer);

	do
	{
		if(pI2cPeriph->timer_differ(&timer) != SE_SUCCESS)
		{
			stErrCode = SE_ERR_TIMEOUT;	
			LOGE("Failed:open periph mutex,  ErrCode-%08X.", stErrCode);
			break;
		}

    //3.Send R frame , S frame or frame of ATR request
    //g_bI2cFrameType = frame_type;
	  stErrCode = pI2cPeriph->transmit(pI2cPeriph, abFrameBuf, sOff);
		if(stErrCode == SE_ERR_BUSY)
		{
			continue;
		}

		else if(stErrCode != SE_SUCCESS)
		{
			return stErrCode;	
		}
		else
		{
			break;
		}
	}while(1);

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE
	stErrCode = pI2cPeriph->gpio_irqwait_edge(I2C_RECEVIE_FRAME_WAIT_GPIO_RISING_IRQ_TIME);
	if(stErrCode != SE_SUCCESS)
	{
		LOGE("\npoll gpio rising stat FAILED!!!!!!\n");
		return stErrCode;	
	}		
#endif

	return stErrCode;
}


/****************************************************************** 
Function:		proto_i2c_send_i_frame
Description:	Send I frame according to the type of frame
		1.organize I frame data according to the type of frame
		2.Send frame data throuth transmit function
Input:	periph   device handle
          	frame_type  type of frame
		inbuf  start address of input data 
		inbuf_len input data length
Output: 	  no	
Return: 	  status code
Others: 	  no 
******************************************************************/
se_error_t proto_i2c_send_i_frame(peripheral *periph, uint8_t frame_type, uint8_t* inbuf, uint16_t inbuf_len,uint32_t retry_i_frame_num)
{
	se_error_t stErrCode = SE_SUCCESS;	
	uint16_t sOff = 0;
	uint16_t sCrc = 0;
	uint8_t abFrameBuf[PROTO_I2C_PIB_LEN_SIZE] = {0};
	double_queue queue_in = (double_queue)inbuf;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;	
	uint8_t* p_input = NULL;
	util_timer_t timer = {0};

	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if((inbuf == NULL) ||(inbuf_len == 0U))
	{
		return SE_ERR_PARAM_INVALID;
	}

	pI2cPeriph->delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);

	if(retry_i_frame_num == 0)//Add PIB LEN just for the first package the i frame
	{
	abFrameBuf[PROTO_I2C_PIB_OFFSET] = frame_type;
	abFrameBuf[PROTO_I2C_LEN_OFFSET]=(inbuf_len>>8)&0xFF;
	abFrameBuf[PROTO_I2C_LEN_OFFSET+1]=inbuf_len&0xFF;
	sOff = sOff + PROTO_I2C_PIB_LEN_SIZE;

	util_queue_front_push(abFrameBuf, sOff, queue_in);

	//2.calculate CRC value. organize EDC data of I frame
	p_input = &queue_in->q_buf[queue_in->front_node];
	sCrc = proto_i2c_crc16(CRC_B, util_queue_size(queue_in), p_input);	//4 
	sOff = util_queue_size(queue_in);	
	util_queue_rear_push((uint8_t *)&sCrc, PROTO_I2C_EDC_SIZE, queue_in);
	sOff = sOff + PROTO_I2C_EDC_SIZE;
	}
	else
	{
		p_input = &queue_in->q_buf[queue_in->front_node];	
		sOff = util_queue_size(queue_in);	
	}

  //set the lock wait timeout time
	timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME; 
	pI2cPeriph->timer_start(&timer);
  do
	{
		if(pI2cPeriph->timer_differ(&timer) != SE_SUCCESS)
		{
			stErrCode = SE_ERR_TIMEOUT;	
			LOGE("Failed:open periph mutex,  ErrCode-%08X.", stErrCode);
			break;
		}

    //3.Send I frame 
    //g_bI2cFrameType = frame_type;
    stErrCode = pI2cPeriph->transmit(pI2cPeriph, p_input, sOff);

		if(stErrCode == SE_ERR_BUSY)
		{
			continue;
		}

		else if(stErrCode != SE_SUCCESS)
		{
			return stErrCode;	
		}
		else
		{
			break;
		}
	}while(1);

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE
	// pI2cPeriph->edge_set(I2C_IRQ_FALLING);
	stErrCode = pI2cPeriph->gpio_irqwait_edge(I2C_RECEVIE_FRAME_WAIT_GPIO_RISING_IRQ_TIME);
	if(stErrCode != SE_SUCCESS)
	{
		LOGE("\npoll gpio rising stat FAILED!!!!!!\n");
		return stErrCode;	
	}		
#endif

	return stErrCode;

}



/****************************************************************** 
Function:	 proto_i2c_receive_frame
Description:  According to the frame format of the HED SPI communication protocol, receive frame data
		        1.Receive frame data throuth receive function
		        2.analysis received frame data
Input:	      periph device handle
                      headbuf start address of frame header
Output: 	  rbuf  start address of the received frame data
                  rlen  received data length
Return: 	  status code
Others: 	   no 
******************************************************************/
se_error_t proto_i2c_receive_frame(peripheral *periph, uint8_t *headbuf, uint8_t *rbuf, uint16_t *rlen)
{
	se_error_t stErrCode = SE_SUCCESS;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;	
	util_timer_t timer = {0};
	uint8_t bRetryNum=0;
	uint8_t bReceiveHeadFlag = 0;
	uint16_t sRspLen = 3; 
	uint16_t sCrc;

	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if((headbuf == NULL)||(rbuf == NULL)||(rlen == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}

	//1. set the timeout time of frame waiting when receiving data
	timer.interval = PROTO_I2C_RECEVIE_FRAME_WAIT_TIME; 
	pI2cPeriph->timer_start(&timer);

	do
	{
		//2. receive PIB , LEN
		do
		{
#ifndef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time
#endif

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_IRQ_FIRST_RECEVIE_WAIT_TIME);   //delay time
#endif	
			stErrCode = pI2cPeriph->receive(pI2cPeriph, rbuf, (uint32_t *)&sRspLen);	
			if(stErrCode == SE_SUCCESS)
			{
				break;
			}

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time
#endif	
			if(pI2cPeriph->timer_differ(&timer) != SE_SUCCESS)
			{
				LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);
				return SE_ERR_TIMEOUT;	
			}

		}while(1);

		//3. check whether the PIB is correct
		if((rbuf[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_I_FRAME_NO_LINK)&&
		    (rbuf[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_S_FRAME_WTX)&&
		    (rbuf[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_R_FRAME_ACK)&&
		    (rbuf[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_R_FRAME_NAK)&&
		    (rbuf[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_I_FRAME_LINK)) //PIB error
		{
			stErrCode = SE_ERR_DATA;
			break;
		}

		//4. check whether the LEN is correct. If correct, continue to receive data of (3+LEN+2) length
		sRspLen = (*(rbuf+PROTO_I2C_LEN_OFFSET) <<8) + *(rbuf+PROTO_I2C_LEN_OFFSET+1) +PROTO_I2C_PIB_LEN_EDC_SIZE; 
		if(sRspLen >  PROTO_I2C_FRAME_MAX_SIZE)
		{
			stErrCode = SE_ERR_LEN;	
			break;
		}

		if(bReceiveHeadFlag ==0)
		{
			bReceiveHeadFlag = 1;
			continue;
		}

		//5. check whether the CRC is correct. If error, continue to receive data. If the error exceeds three times, the error code is returned.
		sCrc = proto_i2c_crc16(CRC_B, sRspLen-PROTO_I2C_EDC_SIZE, rbuf); 	//4 
		if((rbuf[sRspLen-PROTO_I2C_EDC_SIZE]!=(sCrc&0xff))||
		    (rbuf[sRspLen-PROTO_I2C_EDC_SIZE+1]!=sCrc>>8))
		{
			if(bRetryNum>=PROTO_I2C_CRC_ERROR_RETRY_MAX_NUM)
			{
				stErrCode = SE_ERR_LRC_CRC;
				break;
			}
			bRetryNum++;

			//stErrCode = proto_i2c_send_atr_rs_frame(periph, PROTO_I2C_R_FRAME_NAK, NULL);
			//if(stErrCode != SE_SUCCESS)
			//{
			//	break;
			//}
			bReceiveHeadFlag =0;
			sRspLen = 3;
			continue;
		} 

		//6.if it is WTX frame, continue to receive data and re time.
		if((rbuf[PROTO_I2C_PIB_OFFSET]&0xC0) == PROTO_I2C_S_FRAME_WTX)
		{
			//stErrCode = proto_i2c_send_atr_rs_frame(periph, PROTO_I2C_S_FRAME_WTX, NULL);
			//if(stErrCode != SE_SUCCESS)
			//{
			//	break;
			//}
	
			bReceiveHeadFlag =0;
			sRspLen = 3;

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE
		
			
			stErrCode = pI2cPeriph->gpio_irqwait_edge(I2C_RECEVIE_FRAME_WAIT_GPIO_RISING_IRQ_TIME);
			if(stErrCode != SE_SUCCESS)
			{
				printf("\nbigin poll rising FAILED!!!!!!\n");
				return stErrCode;	
			}		
#endif	
			timer.interval = PROTO_I2C_RECEVIE_FRAME_WAIT_TIME; 
			pI2cPeriph->timer_start(&timer);
			continue;
		}

		//7. output I frame data
		*rlen = sRspLen;
		memcpy(headbuf, rbuf, PROTO_I2C_FRONT_FRAME_SIZE);

		break;

	}while(1);

	return stErrCode;
}


/*********************************************************************** 
Function:       proto_i2c_reset_request 
Description:   send reset request frame, receive response to reset frame
Input:        periph device handle 
Return:      status code
Others:      no 
************************************************************************/
se_error_t proto_i2c_reset_request(peripheral *periph) 
{
	se_error_t stErrCode = SE_SUCCESS;
	util_timer_t timer = {0};
	uint8_t bNakRetryNum=0;
	uint8_t bNakRevNum=0;
	uint8_t bPfssi = 0;
	uint8_t bReceiveHeadFlag = 0;
	uint16_t sRspLen = 3; 
	uint16_t sCrc;
	uint8_t bResetRsp[PROTO_I2C_RESET_RSP_SIZE] = {0};
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;


	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

i2c_reset_request:
	
	//1. send reset request frame
	stErrCode = proto_i2c_send_atr_rs_frame(periph, PROTO_I2C_S_FRAME_RESET, g_bI2cHedPfsmi);
  if(stErrCode != SE_SUCCESS)
	{
		return stErrCode;
	}

	//set the timeout time of frame waiting when receiving data
	timer.interval = PROTO_I2C_RECEVIE_FRAME_WAIT_TIME; 
	pI2cPeriph->timer_start(&timer);

	do
	{
		//2. receive PIB and LEN of  response data
		do
		{
#ifndef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time
#endif

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_IRQ_FIRST_RECEVIE_WAIT_TIME);   //delay time
#endif	
			
			stErrCode = pI2cPeriph->receive(pI2cPeriph, bResetRsp, (uint32_t *)&sRspLen);	
			if(stErrCode == SE_SUCCESS)
			{
				break;
			}

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time
#endif	
			
			if(pI2cPeriph->timer_differ(&timer) != SE_SUCCESS)
			{
				LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);
				return SE_ERR_TIMEOUT;	
			}

		}while(1);

		//3. check whether the PIB is correct
		if((bResetRsp[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_I_FRAME_NO_LINK)&&
		    (bResetRsp[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_S_FRAME_RESET)&&
		    (bResetRsp[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_R_FRAME_NAK))
		{
			stErrCode = SE_ERR_DATA;
			break;
		}

		//4. continue to receive data of (3+LEN+2) length
		sRspLen = PROTO_I2C_RESET_RSP_SIZE; 
		if(bReceiveHeadFlag ==0)
		{
			bReceiveHeadFlag = 1;
			continue;
		}		

		//5. check whether the CRC is correct
		sCrc = proto_i2c_crc16(CRC_B, sRspLen-PROTO_I2C_EDC_SIZE, bResetRsp); 	//4 
		if((bResetRsp[sRspLen-PROTO_I2C_EDC_SIZE]!=(sCrc&0xff))||
		    (bResetRsp[sRspLen-PROTO_I2C_EDC_SIZE+1]!=sCrc>>8))
		{
			if(bNakRetryNum>=PROTO_I2C_CRC_ERROR_RETRY_MAX_NUM)
			{
				stErrCode = SE_ERR_LRC_CRC;
				break;
			}
			bNakRetryNum++;
			bReceiveHeadFlag =0;
			bNakRevNum = 0;
			sRspLen = 3;
			continue;
		} 

		//6.	the received frame type is NAK frame, and the frame data will be retransmitted. If the error exceeds three times, the error code is returned.
		if(bResetRsp[PROTO_I2C_PIB_OFFSET] == PROTO_I2C_R_FRAME_NAK)
		{
			if(bNakRevNum>=PROTO_I2C_CRC_ERROR_RETRY_MAX_NUM)
			{
				stErrCode = SE_ERR_LRC_CRC;
				break;
			}
			bNakRevNum++;
			goto i2c_reset_request;
		}

		
		//7. check whether the param is correct
		bPfssi = bResetRsp[PROTO_I2C_PIB_OFFSET]&0x0F;
		if((bPfssi == 0x00)||(g_bI2cHedPfsmi == 0x00))
		{
			//If bPfssi or g_bI2cHedPfsmi is 0, set them to 0
			bPfssi = 0x00;
			g_bI2cHedPfsmi = 0x00;
		}
		else
		{
			if(g_bI2cHedPfsmi>=bPfssi)
			{
				g_bI2cHedPfsmi = bPfssi;
			}
		}
	
		g_sI2cHedPfsm =(uint16_t) g_bI2cHedPfsmi<<4;
		g_sI2cHedPfss = g_sI2cHedPfsm;
		break;
	}while(1);

	return stErrCode;
}



/*********************************************************************** 
Function:		proto_i2c_atr_request 
Description:   send atr request frame, receive response to atr request frame
Input:	  periph device handle 
Output:      rbuf  start address of the ATR to be received
		  rlen  received data length
Return: 	 status code
Others: 	 no 
************************************************************************/
se_error_t proto_i2c_atr_request(peripheral *periph, uint8_t *rbuf, uint32_t *rlen) 
{
	se_error_t stErrCode = SE_SUCCESS;
	util_timer_t timer = {0};
	uint8_t bNakRetryNum=0;
	uint8_t bNakRevNum=0;
	uint8_t bReceiveHeadFlag = 0;
	uint16_t sRspLen = 3; 
	uint16_t sCrc;
	uint8_t bAtrRsp[PROTO_I2C_ATR_RSP_SIZE] = {0};	
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;


	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

i2c_atr_request:

	//1. send atr request frame
	stErrCode = proto_i2c_send_atr_rs_frame(periph, PROTO_I2C_I_FRAME_ATR, 0x00);
	if(stErrCode != SE_SUCCESS)
	{
		return stErrCode;
	}	

	//set the timeout time of frame waiting when receiving data
	timer.interval = PROTO_I2C_RECEVIE_FRAME_WAIT_TIME; 
	pI2cPeriph->timer_start(&timer);

	do
	{
		//2. receive PIB and LEN of  response data
		do
		{
	
#ifndef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time
#endif

#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_IRQ_FIRST_RECEVIE_WAIT_TIME);   //delay time
#endif		
			
			stErrCode = pI2cPeriph->receive(pI2cPeriph, bAtrRsp, (uint32_t *)&sRspLen);	
			if(stErrCode == SE_SUCCESS)
			{
				break;
			}
			
#ifdef	I2C_SUPPORT_GPIO_IRQ_RECEIVE			
			pI2cPeriph->delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time
#endif	
			
			if(pI2cPeriph->timer_differ(&timer) != SE_SUCCESS)
			{
				LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);
				return SE_ERR_TIMEOUT;	
			}

		}while(1);

		//3. check whether the PIB is correct
		if((bAtrRsp[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_I_FRAME_NO_LINK)&&
		    (bAtrRsp[PROTO_I2C_PIB_OFFSET] != PROTO_I2C_R_FRAME_NAK))
		{
			stErrCode = SE_ERR_DATA;
			break;
		}

		//4. check whether the CRC is correct. If correct, continue to receive data of (3+LEN+2) length.
		sRspLen = (*(bAtrRsp+PROTO_I2C_LEN_OFFSET) <<8) + *(bAtrRsp+PROTO_I2C_LEN_OFFSET+1) + PROTO_I2C_PIB_LEN_EDC_SIZE; 
		if(sRspLen >  PROTO_I2C_ATR_RSP_SIZE)  //LEN error
		{
			stErrCode = SE_ERR_LEN;
			break;
		}

		if(bReceiveHeadFlag ==0)
		{
			bReceiveHeadFlag = 1;
			continue;
		}		

		//5. check whether the CRC is correct
		sCrc = proto_i2c_crc16(CRC_B, sRspLen-PROTO_I2C_EDC_SIZE, bAtrRsp);	//4 
		if((bAtrRsp[sRspLen-PROTO_I2C_EDC_SIZE]!=(sCrc&0xff))||
			(bAtrRsp[sRspLen-PROTO_I2C_EDC_SIZE+1]!=sCrc>>8))
		{
			if(bNakRetryNum>=PROTO_I2C_CRC_ERROR_RETRY_MAX_NUM)
			{
				stErrCode = SE_ERR_LRC_CRC;
				break;
			}
			bNakRetryNum++;
			bReceiveHeadFlag =0;
			bNakRevNum = 0;
			sRspLen = 3;
			continue;
		} 

		//6.	the received frame type is NAK frame, and the frame data will be retransmitted. If the error exceeds three times, the error code is returned.
		if(bAtrRsp[PROTO_I2C_PIB_OFFSET] == PROTO_I2C_R_FRAME_NAK)
		{
			if(bNakRevNum>=PROTO_I2C_CRC_ERROR_RETRY_MAX_NUM)
			{
				stErrCode = SE_ERR_LRC_CRC;
				break;
			}
			bNakRevNum++;
			goto i2c_atr_request;
		}
		
		//7. check whether the param is correct
		if(bAtrRsp[PROTO_I2C_DATA_OFFSET] != 0x3B) 
		{
			stErrCode = SE_ERR_DATA;;
			break;
		}
		
		*rlen = sRspLen-PROTO_I2C_PIB_LEN_EDC_SIZE;
		memcpy(rbuf, bAtrRsp+PROTO_I2C_DATA_OFFSET, *rlen);
		
		break;

	}while(1);

	return stErrCode;
}


/********************************************************************************* 
Function:       proto_i2c_handle 
Description:    According to the frame format of the HED I2C communication protocol to exchange data with the SE
Input:          periph device handle
		   input  input data
		   input_len input data length
Output:       output output data
		   outbuf_len  output data length
Return:         no
Others:         no
*********************************************************************************/
se_error_t proto_i2c_handle(peripheral *periph, uint8_t* inbuf, uint32_t inbuf_len, uint8_t* outbuf, uint32_t* outbuf_len)
{	
	se_error_t stErrCode = SE_SUCCESS;
	uint8_t bWtxRetryNum=0;
	uint8_t bNakRetryNum=0;
	uint16_t sRecvInfLen=0;
	uint8_t abFrontBuf[PROTO_I2C_FRONT_FRAME_SIZE] = {0};
	double_queue queue_out = (double_queue)outbuf;
	uint8_t* p_output = NULL;
	uint32_t retry_i_frame_num=0;

	p_output = &queue_out->q_buf[queue_out->front_node];
	do
	{
		if((g_sI2cHedPfsm == 0) || (g_sI2cHedPfss == 0))	   //A value of 0 indicates that chained transmissoin is not supported.
		{
			
			//1. send non chained information frame
			stErrCode = proto_i2c_send_i_frame(periph, PROTO_I2C_I_FRAME_NO_LINK, inbuf, (uint32_t)inbuf_len,retry_i_frame_num);
      if(stErrCode != SE_SUCCESS)
      {
      	return stErrCode;
      }	


			//2. receive the frame data
			stErrCode = proto_i2c_receive_frame(periph, abFrontBuf, p_output, &sRecvInfLen);
			if(stErrCode != SE_SUCCESS)
			{
				if(stErrCode == SE_ERR_TIMEOUT)
				{
					if(bWtxRetryNum>=PROTO_I2C_TIMEOUT_RETRY_MAX_NUM)
					{
						stErrCode = proto_i2c_reset_request(periph);
						if(stErrCode == SE_SUCCESS)
						{
							stErrCode = SE_ERR_TIMEOUT;
						}
						else
						{
							stErrCode = SE_ERR_COMM;
						}
						break;
					}
					bWtxRetryNum++;
					retry_i_frame_num = bWtxRetryNum;
					continue;
				}
				break;
			}

			retry_i_frame_num = 0;//reset the times of i frame sended

			//3. the received frame type is NAK frame, and the frame data will be retransmitted. 
			//If the error exceeds three times, the error code is returned.
			if(abFrontBuf[PROTO_I2C_PIB_OFFSET] == PROTO_I2C_R_FRAME_NAK)
			{
				bNakRetryNum++;
				retry_i_frame_num = bNakRetryNum;
				if(bNakRetryNum>=PROTO_I2C_CRC_ERROR_RETRY_MAX_NUM)
				{
					stErrCode = proto_i2c_reset_request(periph);
					if(stErrCode == SE_SUCCESS)
					{
						stErrCode = SE_ERR_LRC_CRC;
					}
					else
					{
						stErrCode = SE_ERR_COMM;
					}
					break;
				}
				continue;
			}

			//4. At this point, the sending and receiving are correct.
			queue_out->q_buf_len = sRecvInfLen;
			queue_out->rear_node =  queue_out->front_node + sRecvInfLen;
			util_queue_front_pop(PROTO_I2C_PIB_LEN_SIZE, queue_out); //remove start domain data: PIB, LEN
			util_queue_rear_pop(PROTO_I2C_EDC_SIZE, queue_out);//remove terminating domain: CRC
			*outbuf_len = util_queue_size(queue_out);

			break;	
		}

		else
		{
			//In case of chain communication, an error is returned.
			stErrCode = SE_ERR_COMM;
		}
	}while(1);

	return stErrCode;
}



/************************************************************************************
Function:       proto_i2c_init 
Description:    HED I2C communication protocol parameter initialization
Input:         periph device handle
Output:        no  
Return:        status code
Others:        no 
**************************************************************************************/
se_error_t proto_i2c_init(peripheral *periph) 
{
	se_error_t stErrCode = SE_SUCCESS;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	stErrCode = pI2cPeriph->init(pI2cPeriph);	
	if(stErrCode != SE_SUCCESS)
	{
		LOGE("Failed:i2c protocol init,  ErrCode-%08X.", stErrCode);
	}
	else
	{
		LOGI("Success!");
	}

	return stErrCode;
}


/************************************************************************************
Function:       proto_i2c_deinit 
Description:    HED I2C communication protocol parameter termination
Input:          periph device handle 
Output:        no  
Return:        status code
Others:        no 
**************************************************************************************/
se_error_t proto_i2c_deinit(peripheral *periph) 
{
	se_error_t stErrCode = SE_SUCCESS;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	stErrCode = pI2cPeriph->deinit(pI2cPeriph);
	if(stErrCode != SE_SUCCESS)
	{
		LOGE("Failed:i2c protocol deinit,  ErrCode-%08X.", stErrCode);
	}
	else
	{
		LOGI("Success!");
	}

	return stErrCode;
}


/*********************************************************************** 
Function:       proto_i2c_open 
Description:   When connecting a slave device, call this function to get the ATR of the slave device
	             1. Through the function list pointer registered by the port layer device
		     2. Send reset command sequence
                     3. Send the ATR request command sequence to get the ATR value of the slave device
Input:        periph device handle  
Output:      rbuf   start address of the ATR to be received
		  rlen  received data length
Return:      status code
Others:      no 
************************************************************************/
se_error_t proto_i2c_open(peripheral *periph, uint8_t *rbuf, uint32_t *rlen) 
{
	se_error_t stErrCode = SE_SUCCESS;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL))||((rbuf != NULL) && (rlen == NULL)))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	do
	{
		
		//stErrCode = pI2cPeriph->control(pI2cPeriph, (uint32_t)PROTO_I2C_CTRL_RST, NULL, (uint32_t)NULL);
		//if(stErrCode != SE_SUCCESS)
		//{
			//LOGE("Failed:protocol rst io control,  ErrCode-%08X.", stErrCode);
			//break;
		//}
        	pI2cPeriph->delay(PROTO_I2C_SE_RST_DELAY);//delay 350ms
		if(g_bI2cHedOpenSeMode == HED20_I2C_OPEN_SE_RESET_REQ)
		{
			stErrCode = proto_i2c_reset_request(periph);
			if(stErrCode != SE_SUCCESS)
			{
				LOGE("Failed:protocol reset request,	ErrCode-%08X.", stErrCode);
				break;
			}
		}

		if((rbuf != NULL)&&(rlen != NULL))
		{
			stErrCode = proto_i2c_atr_request(periph, rbuf, rlen);
			if(stErrCode != SE_SUCCESS)
			{
				LOGE("Failed:protocol atr request,	ErrCode-%08X.", stErrCode);
				break;
			}	
		}

		LOGI("Open Periph Success!");
		break;

	}while(0);

	return stErrCode;
}



/************************************************************************************
Function:       proto_i2c_close
Description:    close the device
Input:         periph device handle
Output:        no  
Return:        status code
Others:        no 
**************************************************************************************/
se_error_t proto_i2c_close(peripheral *periph) 
{
	se_error_t ret_code = SE_SUCCESS;

	return ret_code;
}


/****************************************************************** 
Function:		proto_i2c_transceive 
Description:	Send and receive data through the I2C interface?
Input:	   periph device handle
		   sbuf start address of the input two-way queue
		   slen the input two-way queue data length
Output:       rbuf start address of the output two-way queue
		   rlen  the output two-way queue data length
Return:        status code 
Others:  no 
******************************************************************/
se_error_t proto_i2c_transceive(peripheral *periph, uint8_t *sbuf,  uint32_t  slen, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t stErrCode = SE_SUCCESS;
	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((sbuf == NULL) || (rbuf == NULL) || (slen == 0U) || (rlen == NULL))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	stErrCode = proto_i2c_handle(periph, sbuf, slen, rbuf, rlen);
	if(stErrCode != SE_SUCCESS)
	{
		LOGE("Failed:protocol communication,  ErrCode-%08X.", stErrCode);
	}
	else
	{
		LOGI("Communication Success!");
	}
	return stErrCode;	
}

/*********************************************************************** 
Function:       proto_i2c_reset 
Description:   reset the device and get the ATR of the slave device
                    1. Through the function list pointer registered by the port layer device, call the control function of the port layer spi interface to reset the SE
                    2. Send reset command sequence
                    3. Send the ATR request command sequence to get the ATR value of the slave device
Input:         periph device handle
Output:      rbuf  start address of the ATR to be received
		  rlen  received data length
Return:      status code
Others:      no 
************************************************************************/
se_error_t proto_i2c_reset(peripheral *periph, uint8_t *rbuf, uint32_t *rlen) 
{
	se_error_t stErrCode = SE_SUCCESS;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL))||((rbuf != NULL) && (rlen == NULL)))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	do
	{
		//control the RST pin of SE for reset operation
		stErrCode = pI2cPeriph->control(pI2cPeriph, (uint32_t)PROTO_I2C_CTRL_RST, NULL, (uint32_t)NULL);
		if(stErrCode != SE_SUCCESS)
		{
			LOGE("Failed:protocol rst io control,  ErrCode-%08X.", stErrCode);
			break;
		}

		pI2cPeriph->delay(PROTO_I2C_SE_RST_DELAY);//delay 350ms

		if(g_bI2cHedOpenSeMode == HED20_I2C_RESET_SE_RESET_REQ)
		{
			stErrCode = proto_i2c_reset_request(periph);
			if(stErrCode != SE_SUCCESS)
			{
				LOGE("Failed:protocol reset request,	ErrCode-%08X.", stErrCode);
				break;
			}

		}

		if((rbuf != NULL)&&(rlen != NULL))
		{
			stErrCode = proto_i2c_atr_request(periph, rbuf, rlen);
			if(stErrCode != SE_SUCCESS)
			{
				LOGE("Failed:protocol atr request,	ErrCode-%08X.", stErrCode);
				break;
			}	
		}

		LOGI("Reset Periph Success!");
		break;

	}while(0);

	return stErrCode;
}



/****************************************************************** 
Function:		proto_i2c_control
Description:	Send the RST control pin to the slave device through the I2C interface to reset the control command or other control commands
Input:	   periph device handle
		   ctrlcode command control code
		   sbuf start address of input data
		   slen length of input data
Output:        rbuf start address of output data
		   rlen   length of output data
Return:    status code
Others:    no 
******************************************************************/
se_error_t proto_i2c_control(peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen)
{
	se_error_t stErrCode = SE_SUCCESS;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ( ctrlcode== 0U)
	{
		return  SE_ERR_PARAM_INVALID;
	}

	stErrCode = pI2cPeriph->control(pI2cPeriph, ctrlcode, sbuf, slen);
	if(stErrCode != SE_SUCCESS)
	{
		LOGE("Failed:i2c protocol control,  ErrCode-%08X.", stErrCode);
	}
	else
	{
		LOGI("Success!");
	}
	
	return stErrCode;
}



/*********************************************************************** 
Function:     proto_i2c_delay 
Description:  microsecond delay
Input:        periph device handle
              us     microsecond
Return:      status code
Others:      no 
************************************************************************/
se_error_t proto_i2c_delay(peripheral *periph , uint32_t us) 
{
	se_error_t stErrCode = SE_SUCCESS;
	HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;
	
	if(pI2cPeriph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	pI2cPeriph->delay(us);
	return stErrCode;
}




