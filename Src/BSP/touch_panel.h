
#ifndef _TOUCH_PANEL_H
#define _TOUCH_PANEL_H  
//#include "stm32f10x_lib.h"

#include "stm32f10x.h"

#define TSC_INT_PORT GPIOF
#define TSC_INT_PIN  GPIO_Pin_10

#define TSC_NSS_PORT GPIOA
#define TSC_NSS_PIN  GPIO_Pin_4

#define TSC_SPI SPI1
#define TSC_EXCHANGE_DATA(output) SPI_Exchange_Data(TSC_SPI, output)

#define TSC_IN_TOUCH()   (!GPIO_ReadInputDataBit(TSC_INT_PORT, TSC_INT_PIN))

#define SELECT_TSC(isSelect) GPIO_WriteBit(TSC_NSS_PORT, TSC_NSS_PIN, !isSelect)

#define CMD_RDX 0x90  //0B10010000即用差分方式读X坐标
#define CMD_RDY	0xD0  //0B11010000即用差分方式读Y坐标 

u8  touch_panel_read(u16 *X,u16 *Y);

#endif



