
#ifndef _TOUCH_PANEL_H
#define _TOUCH_PANEL_H  
//#include "stm32f10x_lib.h"

#include "stm32f10x.h"


#define Dummy_Byte 0xA5
			 
#define touch		(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6))   //PB6  
#define no_touch	(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6))   //PB6 	

#define Select_TSC()     GPIO_ResetBits(GPIOB, GPIO_Pin_7)

#define DisSelect_TSC()    GPIO_SetBits(GPIOB, GPIO_Pin_7)


#define CMD_RDX 0X90  //0B10010000即用差分方式读X坐标
#define CMD_RDY	0XD0  //0B11010000即用差分方式读Y坐标 


u8  touch_panel_read(u16 *X,u16 *Y);

#endif



