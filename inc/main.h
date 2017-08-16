/*----------------------------------------------------------------------------
 * Name:    main.h
 * Purpose: 
 * Note(s):
 *----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/

#ifndef __MAIN_H
#define __MAIN_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "jansson.h"
#include "string.h"
#include "circularBuffer.h"
#include "LED.h"
#include "USART.h"


#define ResponseTypeBit_StartUp 0
#define ResponseTypeBit_WifiSetup 1
#define ResponseTypeBit_MQTTSetup 2
#define ResponseTypeBit_MQTTSubs 3
#define ResponseTypeBit_MQTTPub 4
#define ResponseTypeBit_Subscription 5

#endif
