/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
*******************************************************************************

   Name       : adi_a2b_irq.c
   
   Description: This file is responsible for handling A2B Pin/GPIO interrupt related functions.        
                 
   Functions  :  adi_a2b_EnableInterrupt()
                 

   Prepared &
   Reviewed by: Automotive Software and Systems team, 
                IPDC, Analog Devices,  Bangalore, India
                
   @version: $Revision: 3408 $
   @date: $Date: 2015-08-09 15:01:16 +0530 (Sun, 09 Aug 2015) $
               
******************************************************************************/
/*! \addtogroup Target_Dependent Target Dependent
 *  @{
 */

/** @defgroup GPIO
 *
 * This driver handles GPIO interrupts from AD24XX. All the interface functions needs to be re-implemented for different processor.
 *
 */


/*! \addtogroup GPIO
 *  @{
 */

/*============= I N C L U D E S =============*/

#include "adi_a2b_datatypes.h"
#include "adi_a2b_framework.h"
#include "adi_a2b_externs.h"

/*============= D E F I N E S =============*/

#define GPIO_MEMORY_SIZE (ADI_GPIO_CALLBACK_MEM_SIZE)

/*============= D A T A =============*/

/*============= C O D E =============*/
/*
** Function Prototype section
*/
#define ADI_GPIO_PORT uint32_t
static void adi_a2b_PinInterruptHandler(ADI_GPIO_PORT ePinInt, a2b_UInt32 nPins,  void *pCBParam);
typedef void (*pfAppCb)(a2b_UInt32 param);
typedef void (*PIN_INT_HANDLER_PTR)(void);
a2b_UInt32 param;

/*
** Function Definition section
*/
/* callback for receiving GPIO input events */
/*****************************************************************************/

/*!
@brief              This function enables GPIO pin as input pin and configures for  
                    edge triggered interrupt

@param [in]         nGPIONum        Abstracted GPIO number  
@param [in]         pUserCallBack   User parameter
    
@return             Return code
                    - 0: Success
                    - 1: Failure  
*/    
/*****************************************************************************/
#pragma section("L1_code")
a2b_UInt32 adi_a2b_EnablePinInterrupt(a2b_UInt8 nGPIONum , void* pUserCallBack, a2b_UInt32 CallBackParam, a2b_UInt8 bFallingEdgeTrig)
{
#if 0
    a2b_UInt32 gpioMaxCallbacks;
	ADI_GPIO_RESULT eResult;

	a2b_UInt32 GPIONum = ADI_GPIO_PIN_0 << nGPIONum;

	ADI_GPIO_SENSE eGpioSense = (bFallingEdgeTrig == 1u) ? ADI_GPIO_SENSE_FALLING_EDGE: ADI_GPIO_SENSE_RISING_EDGE;
	/* allocate memory for the GPIO service */
	static a2b_UInt8 gpioMemory[GPIO_MEMORY_SIZE];
	param = CallBackParam;

	/* initialize the GPIO service */
	eResult = adi_gpio_Init((void*)gpioMemory, GPIO_MEMORY_SIZE, &gpioMaxCallbacks);

	/* set the GPIO direction */
	eResult = adi_gpio_SetDirection(ADI_GPIO_PORT_H, GPIONum, ADI_GPIO_DIRECTION_INPUT);

	/* set edge sense mode (PORT H is connected to Pin Interrupt 3) */
	eResult = adi_gpio_SetInputEdgeSense(ADI_GPIO_PORT_H, GPIONum, eGpioSense);
	eResult = adi_gpio_EnableInterruptMask(ADI_GPIO_PORT_H,GPIONum, ADI_GPIO_MASK_A, TRUE );
	eResult = adi_gpio_RegisterCallback(ADI_GPIO_PORT_H, GPIONum, (ADI_GPIO_CALLBACK)&adi_a2b_PinInterruptHandler, (void*)pUserCallBack);

    return((a2b_UInt32)eResult);
#endif
    return 0;
}


/*! \addtogroup GPIO_Internal_Functions GPIO Internal Functions
 *  @{
 */
/******************************************************************************/
/*!
@brief      Interrupt service routine(ISR), calls user call back function if enabled
            This function shall be invoked upon GPIO event 

@param [in] ePinInt        GPIO port enum
@param [in] nPins          Pin Number within the port
@param [in] pCBParam       Call back parameter

@return     none
    
*/
/*****************************************************************************/
#pragma section("L1_code")
static void adi_a2b_PinInterruptHandler(ADI_GPIO_PORT ePinInt, a2b_UInt32 nPins,  void *pCBParam)
{
#if 0
	if( pCBParam != A2B_NULL)
	{
		((pfAppCb)(pCBParam))(param);
	}
#endif
}

/**
 @}
*/

/**
 @}
*/
/*
**
** EOF: $URL$
**
*/


