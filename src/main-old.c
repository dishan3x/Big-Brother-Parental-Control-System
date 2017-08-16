/**
  ******************************************************************************
  * @file    main.c
  * @author  
  * @version V1.0
  * @date    2017-02-28
  * @brief   Default main function.
  ******************************************************************************
*/

#include "main.h"
#include <string.h>

#define MAXINT (1<<32)
static uint32_t ticks = 0;
static uint32_t lastTicks = 0;

//UART6
#define MAX_STRLEN 2048
volatile char received6String[MAX_STRLEN+1]; //Hold received string
volatile uint32_t haveString;

int main(void)
{
	uint16_t IsLEDOn = 0;
  LED_Init();
	haveString = 0;
	InitUSART6();
	/*Setup SysTick */
	ticks = 0;
	lastTicks = 0;
	SystemCoreClockUpdate();

	/*Config so 1 tick = 1ms or 1000tick = 1 second*/
	if( SysTick_Config(SystemCoreClock / 1000)) while(1);

	char* startText= "{\"Action\":\"Debug\",\"Info\":\"Testing UART6\"}";
	SendCharArrayUSART6(startText,strlen(startText));
	while(1){
		if( (ticks-lastTicks+UINT32_MAX)%UINT32_MAX>=1000) {
			lastTicks = ticks;
			if (IsLEDOn)
				LED_Off(0);
			else
				LED_On(0);
			IsLEDOn ^=1;
			if (haveString){
				SendCharArrayUSART6("{\"ReceivedText\":\"",16);
				SendCharArrayUSART6((char*)received6String,strlen((char*)received6String));
				SendCharArrayUSART6("\"}\n",3);
				haveString = 0;
			}
		}
	}
}

void SysTick_Handler(void) {
	ticks++;
}

/*
 * This is a simple IRQ handler for USART6. A better design would
 * be to have a circular list of strings. Head index pointing to 
 * where chars added. Tail index oldest string to be processed
 */
void USART6_IRQHandler(void) {
	// check if the USART6 receive interrupt flag was set
	if( USART_GetITStatus(USART6, USART_IT_RXNE) ){

		static uint16_t cnt = 0;
		char t = USART6->DR; // the character from the USART6 data register is saved in t

		/* check if the received character is not the LF character (used to determine end of string)
		 * or the if the maximum string length has been been reached
		 */
		if ( !((t == '\n')||(t == '\r')) && (cnt < MAX_STRLEN) ){
			received6String[cnt] = t;
			cnt++;
		}
		else{ // otherwise reset the character counter and print the received string
			received6String[cnt] = 0x0;
			cnt = 0;
			haveString = 1;
		}
	}
}
