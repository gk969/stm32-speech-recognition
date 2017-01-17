#ifndef _LCDLCD_INCLUDED_
#define _LCDLCD_INCLUDED_   	 

#include "stm32f10x.h"      

#define RED	  0XF800
#define GREEN 0X07E0
#define BLUE  0X001F
#define BRED  0XF81F
#define GRED  0XFFE0
#define GBLUE 0X07FF
#define BLACK 0X0000
#define WHITE 0XFFFF


#define Bank1_LCD_D    ((uint32_t)0x6C000800)    //disp Data ADDR
#define Bank1_LCD_C    ((uint32_t)0x6C0007FE)	 //disp Reg ADDR  

#define LCD_WR_REG(index) 			*(__IO uint16_t *)(Bank1_LCD_C)=(index)
#define LCD_WR_CMD(index,val) 		*(__IO uint16_t *)(Bank1_LCD_C)=index;*(__IO uint16_t *)(Bank1_LCD_D)=val
#define LCD_WR_Data(val) 			*(__IO uint16_t *)(Bank1_LCD_D)=(val) 	

#define LCD_AbovetoBelow()			LCD_WR_CMD(0x0003,0x1030)
#define LCD_BelowtoAbove()			LCD_WR_CMD(0x0003,0x1010)
	
#define CN_Width	24
#define CN_Height	24
#define	Char_Width	12
#define	Char_Height	24

#define	COORD_240_320	0
#define	COORD_320_240	1

extern u8 LCD_COORD;

void LCD_Init(void);
void LCD_Rst(void);
void FSMC_LCD_Init(void);
void LCD_Set_Disp_Window(u16 left,u16 top,u16 width,u16 height);
void LCD_Set_Address(u16 left,u16 top);
void LCD_Set_Coord(u8 coord);
void LCD_Fill(u16 left,u16 top,u16 width,u16 height,u16 color);
void LCD_DrawPoint(u16 left,u16 top,u16 color);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);
void LCD_ShowChar(u16 left,u16 top,u8 char_data,u16 color);
void LCD_ShowCN(u16 left,u16 top,u8* cn,u16 color);
u16 RGB(u8 red,u8 green,u8 blue);


#endif
