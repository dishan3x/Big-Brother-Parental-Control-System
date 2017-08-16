/**
  ******************************************************************************
  * @file    main.c
  * @author  Jay Herrmann
  * @version V1.0
  * @date    2017-02-25
  * @brief   Default main function.
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"
#include "attributes.h"
#define MAXINT (1<<32)
static uint32_t lastTicks = 0;
static uint32_t USARTCount = 0;
static uint32_t ticks = 0;
static uint32_t delay = 0;
static commBuffer_t comm;
static commBuffer_t transmit_buff;
int state =0;
	json_t *ResponseObj=NULL, *ActionObj=NULL, *MessageObj=NULL, *MessageResultObj=NULL;
	char *ResponseVal=NULL, *ActionVal=NULL, *MessageVal=NULL, *MessageResultVal=NULL;
	char *Result, *IP=NULL;
	char ResponseStr[256], ActionStr[256], MessageStr[256];
	char RecvStr[512];
	int StateMask = 0;
	char *Action, *Topics1, *Topic = NULL;
	json_error_t error, error1;
	int RC = 0;
	int button =0;

//UART6
#define MAX_STRLEN 2048
volatile char received6String[MAX_STRLEN+1]; //Hold received string
volatile uint32_t haveString;
void Configure_PD0(void);
void Configure_PB12(void);
int main(void)
{
	
	/* System init */
    SystemInit();
    /* Configure PD0 as interrupt */
    Configure_PD0();
    /* Configure PB12 as interrupt */
    Configure_PB12();
	
	//EXTI_GenerateSWInterrupt(EXTI_Line12);
	uint16_t IsLEDOn = 0;
	LED_Init();
	haveString = 0;
	InitUSART6();
	lastTicks = 0;
	

	
	/*Setup SysTick */
	ticks = 0;
	SystemCoreClockUpdate();
	
	
	
	
	/*Config so 1 tick = 1ms or 1000tick = 1 second*/
	if( SysTick_Config(SystemCoreClock / 1000)) while(1);
	InitBuffer(&comm);
	InitBuffer(&transmit_buff);
	static int StartUpState = 0;
	
	//Huzzah StartUp Responses
	//Message1
	//{"Response":"StartUp","Message":{"Description":"ECE631 Serial To Wifi Bridge With MQTT suppport Configured Using JSON"}}
	//Message2
	//{"Response":"StartUp","Message":{"Version":"{\"Info\":\"Version 1.0.1 - 2017-02-07\",\"Date\":\"2017-02-07\",\"MajorMinor\":\"1.0.1\"}"}}

	//Wifi Setup
	//{"Action":"WifiSetup","Wifi":{"SSID":"ECE631Lab","Password":"stm32F4."}}
	//{"Response":"WifiSetup","Message":{"Wifi":"{\"Result\":\"Success\",\"IP\":\"192.168.123.135\"}"}}

	//MQTT Setup
	//{"Action":"MQTTSetup","MQTT":{"Host":"192.168.1.5","Port":"1883"}}
	//{"Response":"MQTTSetup","Message":{"MQTT":"{\"Result\":\"Success\"}"}}
while(1){	
		
	if((ticks-lastTicks+UINT32_MAX)%UINT32_MAX >= 1000){
	
lastTicks = ticks;

	if(state ==3&&ticks-delay>=3000)
		{
			state =0;
			LED_Off(0);
		LED_Off(1);
			LED_Off(2);
			LED_Off(3);
					ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","cmnd/Action/POWER1","Message","0");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);
			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));
			button =0;
			USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
		}
		else if (state ==2&&button ==1)
		{
			/*ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","cmnd/Action/POWER1","Message","1");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);
			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));
			button =0;
			USART_ITConfig(USART6, USART_IT_TXE,ENABLE);*/
		}

		if(haveStr(&comm)){
	  char text[MAXCOMMBUFFER];				
	  getStr(&comm, text);
    strcpy(RecvStr,text);
		ResponseObj = json_loads(RecvStr, JSON_DISABLE_EOF_CHECK, &error);
		if (!ResponseObj){
			continue;
		}

		RC = json_unpack_ex(ResponseObj, &error, 0,"{s:s,s:o}", "Response", &ResponseVal, "Message", &MessageResultObj);
		strcpy(ResponseStr,ResponseVal);
		free(ResponseVal);ResponseVal = NULL;
		RC = 0;
		if (RC!=0){
			continue;
		}

		int ResponseTypeBitMask = 
				(
				((int)(strcmp(ResponseStr,"StartUp")==0)<<ResponseTypeBit_StartUp)|
				((int)(strcmp(ResponseStr,"WifiSetup")==0)<<ResponseTypeBit_WifiSetup)|
				((int)(strcmp(ResponseStr,"MQTTSetup")==0)<<ResponseTypeBit_MQTTSetup)|
				((int)(strcmp(ResponseStr,"MQTTSubs")==0)<<ResponseTypeBit_MQTTSubs)|
				((int)(strcmp(ResponseStr,"MQTTPub")==0)<<ResponseTypeBit_MQTTPub)|
				((int)(strcmp(ResponseStr,"Subscription")==0)<<ResponseTypeBit_Subscription)
				);

		switch (ResponseTypeBitMask){
			case 1<<ResponseTypeBit_StartUp:
				//{"Description":"ECE631 Serial To Wifi Bridge With MQTT suppport Configured Using JSON"}
					if (json_unpack_ex(MessageResultObj, &error1, 0,"{s:s}", "Description", &MessageVal)==0){
						//Clear all LEDs and States
						LED_Off(0); //Green LED
						LED_Off(3); //Blue LED
						LED_Off(2); //Red LED
						LED_Off(1); //Orange LED
					  StateMask |=1<<0;
						StartUpState = 1;
					}
					//{"Version":"{\"Info\":\"Version 1.0.1 - 2017-02-07\",\"Date\":\"2017-02-07\",\"MajorMinor\":\"1.0.1\"}
					else if ((json_unpack_ex(MessageResultObj, &error1, 0,"{s:s}", "Version", &MessageVal)==0)&& 
						(StartUpState==1)){
						LED_On(3);//Turn On Blue LED
					  StateMask |=1<<1;
						StartUpState=0;
						//Send WifiSetup
						//{"Action":"WifiSetup","Wifi":{"SSID":"ECE631Lab","Password":"stm32F4."}}
						ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","WifiSetup","Wifi","SSID","ECE631Lab","Password","stm32F4.");
						ActionVal = json_dumps(ActionObj,JSON_COMPACT);
						strcpy(ActionStr, ActionVal);//Send to Huzzah via TxBuffer
						putStr(&transmit_buff, ActionStr, strlen(ActionStr));
						USART_ITConfig(USART6, USART_IT_TXE, ENABLE);
						if (ActionVal) {free(ActionVal);ActionVal=NULL;}
					}
					break;
			case 1<<ResponseTypeBit_WifiSetup:
					//{"Wifi":"{\"Result\":\"Success\",\"IP\":\"192.168.123.135\"}"}
					if (json_unpack_ex(MessageResultObj, &error1, 0,"{s:s}", "Wifi", &MessageVal)==0){
						MessageObj = json_loads(MessageVal, JSON_DECODE_ANY, &error);
						free(MessageVal);MessageVal=NULL;
						if (json_unpack_ex(MessageObj, &error1, 0,"{s:s,s:s}", "Result", &Result,"IP",&IP)==0){
							if (strcmp(Result,"Success")==0){
								LED_On(2);//Light Red LED & maybe save IP
								StateMask |=1<<2;
								//Send MQTTSetup
								//{"Action":"MQTTSetup","MQTT":{"Host":"192.168.1.5","Port":"1883"}}
								ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTSetup","MQTT","Host","192.168.123.124","Port","1883");
								ActionVal = json_dumps(ActionObj,JSON_COMPACT);
								strcpy(ActionStr, ActionVal);//Send to Huzzah via TxBuffer
					    	putStr(&transmit_buff, ActionStr, strlen(ActionStr));
						    USART_ITConfig(USART6, USART_IT_TXE, ENABLE);
								if (ActionVal) {free(ActionVal);ActionVal=NULL;}
							}
							if (Result) {free(Result);Result=NULL;}
							if (IP) {free(IP);IP=NULL;}
						}
					}
					break;
			case 1<<ResponseTypeBit_MQTTSetup:
					//{"MQTT":"{\"Result\":\"Success\"}"}
					if (json_unpack_ex(MessageResultObj, &error1, 0,"{s:s}", "MQTT", &MessageVal)==0){
						MessageObj = json_loads(MessageVal, JSON_DECODE_ANY, &error);
						free(MessageVal);MessageVal=NULL;
						if (json_unpack_ex(MessageObj, &error1, 0,"{s:s}", "Result",&Result)==0){
							if (strcmp(Result,"Success")==0){
								LED_On(1);//Light Red
								StateMask |=1<<3;
								ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:[s]}}","Action","MQTTSubs","MQTT","Topics","listen");
								ActionVal = json_dumps(ActionObj,JSON_COMPACT);
								strcpy(ActionStr,ActionVal);
							  putStr(&transmit_buff,ActionStr,strlen(ActionStr));
							//	char* state4Text= "{\"Action\":\"MQTTSubs\",\"MQTT\":{\"Topics\":[\"ec/acc/gui\",\"ec/debug\"]}}\n";
							//	putStr(&transmit_buff, state4Text,strlen(state4Text));
								USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
							}
							if (Result) {free(Result);Result=NULL;}
						}
					}
					break;
					case 1<<ResponseTypeBit_MQTTSubs:
					if (json_unpack_ex(MessageResultObj, &error1, 0,"{s:s}", "MQTT", &MessageVal)==0)
						{
							MessageObj = json_loads(MessageVal, JSON_DECODE_ANY, &error);
							free(MessageVal);MessageVal=NULL;
							if (json_unpack_ex(MessageObj, &error1, 0,"{s:s,s:[s]}", "Action", &Action,"Topics",&Topics1)==0)
								{
									if(strcmp(Action,"Subscribed")==0)
									{
										LED_On(0);//green
										LED_Off(0);
										LED_Off(1);
										LED_Off(2);
										LED_Off(3);
									
									}
									if (Action) {free(Action);Action=NULL;}
									if (Topics1) {free(Topics1);Topics1=NULL;}

								}
						}
						break;
					case 1<<ResponseTypeBit_Subscription:
					//{"MQTT":"{\"Result\":\"Success\"}"}
					if (json_unpack_ex(MessageResultObj, &error1, 0,"{s:s}", "MQTT", &MessageVal)==0){
						MessageObj = json_loads(MessageVal, JSON_DECODE_ANY, &error);
						free(MessageVal);MessageVal=NULL;
						if (json_unpack_ex(MessageObj, &error1, 0,"{s:s,s:s}", "Topic", &Topic,"Message",&MessageVal)==0){
							if (strcmp(Topic,"listen")==0){
									if(strcmp(MessageVal,"request")==0){
										state =1;//child request
										LED_On(1);
									}
								  else if(strcmp(MessageVal,"Imout")==0)
									{
										if(state == 2)//device on
										{
										ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","cmnd/Action/POWER1","Message","0");
										ActionVal = json_dumps(ActionObj,JSON_COMPACT);
										strcpy(ActionStr,ActionVal);
										putStr(&transmit_buff,ActionStr,strlen(ActionStr));
										LED_Off(0);
										LED_Off(1);
										LED_Off(2);
										LED_Off(3);
										state =0;//device off
										USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
										}
									}
									free(MessageVal);MessageVal=NULL;
								}
							}
							if (Topic) {free(Topic);Topic=NULL;}
						}
					
					break;
			default:
				continue;
				break;
		}
		if (ResponseVal) {free(ResponseVal); ResponseVal=NULL;}
		if (MessageVal) {free(MessageVal); MessageVal=NULL;}
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
	USARTCount++;	
	// check if the USART6 receive interrupt flag was set
	if( USART_GetITStatus(USART6, USART_IT_RXNE) ){

		static uint16_t cnt = 0;
		char t = USART6->DR; // the character from the USART6 data register is saved in t

		/* check if the received character is not the LF character (used to determine end of string)
		 * or the if the maximum string length has been been reached
		 */
		if ( !((t == '\n')||(t == '\r'))){
		  	
      putChar(&comm, t);			
			
		}
		else{ // otherwise reset the character counter and print the received string
			
			putChar(&comm, '\0');
			
		}
		
	}
			
	if(USART_GetITStatus(USART6, USART_IT_TXE))
	{
		  
		  USART6->DR = getChar(&transmit_buff);
		  if(!haveStr(&transmit_buff))
			{
				USART_ITConfig(USART6, USART_IT_TXE, DISABLE);	  
			}
		} 
	
	}
void Configure_PD0(void) {
    /* Set variables used */
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    /* Enable clock for GPIOD */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* Set pin as input */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    /* Tell system that you will use PD0 for EXTI_Line0 */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource0);
    
    /* PD0 is connected to EXTI_Line0 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
 
    /* Add IRQ vector to NVIC */
    /* PD0 is connected to EXTI_Line0, which has EXTI0_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
}
 
void Configure_PB12(void) {
    /* Set variables used */
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    /* Enable clock for GPIOB */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    /* Set pin as input */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* Tell system that you will use PB12 for EXTI_Line12 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource12);
    
    /* PB12 is connected to EXTI_Line12 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line12;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
 
    /* Add IRQ vector to NVIC */
    /* PB12 is connected to EXTI_Line12, which has EXTI15_10_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
}
 
/* Set interrupt handlers */
/* Handle PD0 interrupt */
/*agree*/
void EXTI0_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        /* Do your stuff when PD0 is changed */
			if(state ==1)
			{
				state =2;//parent agree
				LED_On(0);//green on
				button =1;
				/*ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","Action","Message","ON");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);
			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));*/
				ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","listen","Message","Yes");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);
			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));
			USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
			}
			else if(state ==0)
			{
				ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","cmnd/Action/POWER1","Message","1");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);

			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));
					USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
				LED_On(0);
				state =4;
			}
       
	
        
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
 
/* Handle PB12 interrupt */
void EXTI15_10_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
        /* Do your stuff when PB12 is changed */
     if(state ==1)
			{
			LED_Off(1);
			state =0;
				ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","listen","Message","No");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);
			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));
			USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
			}
			if(state ==2)
			{
				state =3;
				LED_On(3);
				delay =ticks;
				ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","listen","Message","Timeout");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);
			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));
			USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
			}
			else if(state ==4)
			{
					ActionObj = json_pack_ex(&error,0,"{s:s,s:{s:s,s:s}}","Action","MQTTPub","MQTT","Topic","cmnd/Action/POWER1","Message","0");
				ActionVal = json_dumps(ActionObj,JSON_COMPACT);
			 strcpy(ActionStr,ActionVal);
			putStr(&transmit_buff,ActionStr,strlen(ActionStr));
					USART_ITConfig(USART6, USART_IT_TXE,ENABLE);
				state =0;
				LED_Off(0);
			}
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line12);
    }
}

