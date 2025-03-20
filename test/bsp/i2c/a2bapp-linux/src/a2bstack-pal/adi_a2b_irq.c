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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <pthread.h>
#include "adi_a2b_externs.h"

/*============= D E F I N E S =============*/
typedef struct {
    uint8_t nGPIONum;
    void* pUserCallBack;
    uint8_t bFallingEdgeTrig;
} ThreadArgs;

/*============= D A T A =============*/

/*============= C O D E =============*/
/*
** Function Prototype section
*/
static void adi_a2b_PinInterruptHandler(uint8_t ePinInt, a2b_UInt32 nPins,  void *pCBParam);
typedef void (*pfAppCb)(a2b_UInt32 param);
a2b_UInt32 param;

void port_gpio_control(const char *path, const char *value) {
    int32_t fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return;
    }
    write(fd, value, strlen(value));
    close(fd);
}

void* thread_loop(void *arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    char str[64], value;

    snprintf(str, sizeof(str), "%d", args->nGPIONum);
    port_gpio_control("/sys/class/gpio/export", str);

    snprintf(str, sizeof(str), "/sys/class/gpio/%d/direction", args->nGPIONum);
    port_gpio_control(str, "in");

    snprintf(str, sizeof(str), "/sys/class/gpio/%d/edge", args->nGPIONum);
    port_gpio_control(str, args->bFallingEdgeTrig ? "falling" : "rising");

    snprintf(str, sizeof(str), "/sys/class/gpio/%d/value", args->nGPIONum);
    int32_t fd = open(str, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return NULL;
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLPRI;

    snprintf(str, sizeof(str), "%d", args->bFallingEdgeTrig);
    while (1) {
        lseek(fd, 0, SEEK_SET);
        read(fd, &value, 1);
    
        printf("Waiting for GPIO event...\n");
        poll(&pfd, 1, -1);
    
        lseek(fd, 0, SEEK_SET);
        read(fd, &value, 1);
    
        if (value == str[0]) {
            adi_a2b_PinInterruptHandler(args->nGPIONum, args->nGPIONum, args->pUserCallBack);
        }
    }
    
    close(fd);
    free(args);
    return NULL;
}

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
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
    args->nGPIONum = nGPIONum;
    args->pUserCallBack = pUserCallBack;
    args->bFallingEdgeTrig = bFallingEdgeTrig;

    param = CallBackParam;

    pthread_t thread;
    pthread_create(&thread, NULL, thread_loop, (void*)args);
    pthread_detach(thread);

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
static void adi_a2b_PinInterruptHandler(uint8_t ePinInt, a2b_UInt32 nPins,  void *pCBParam)
{
	if( pCBParam != A2B_NULL)
	{
		((pfAppCb)(pCBParam))(param);
	}

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


