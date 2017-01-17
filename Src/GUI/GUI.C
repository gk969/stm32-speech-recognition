/******   GUI.C     ******/

#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "GUI.H"
#include "TFTLCD.h"
#include "delay.h"
#include "USART.h"
#include "touch_panel.h"


#define DEBUG
#ifdef DEBUG
#define DBP(fmt,arg...)  USART1_printf(fmt,##arg)
#define DBPLN(fmt,arg...)  USART1_printf_line(fmt,##arg)
#define DBPH(src, len)  USART1_print_hex(src, len)
#else
#define DBP(fmt,arg...)
#define DBPLN(fmt,arg...)
#define DBPH(src, len)
#endif

GUI_Area Screen={0,0,320,240,GREEN,RED};

GUI_Area Label[]=
{
	{106,19	,99	,28	,GREEN,BLUE},//��ӭʹ��
	{25	,56	,275,28	,GREEN,BLUE},//����������ʶ�����ϵͳ
	{40	,168,99	,28	,BRED ,BLUE},//ģ��ѵ��
	{179,168,99	,28	,BRED ,BLUE},//����ʶ��
	
	{106,18	,99	,28	,GREEN,BLUE},//ģ��ѵ��/����ʶ��
	
	{16	,92	,28	,28	,GRED ,BLUE},//0
	{16	,55	,28	,28	,GRED ,BLUE},//1
	{48	,55	,28	,28	,GRED ,BLUE},//2
	{81	,55	,28	,28	,GRED ,BLUE},//3
	{114,55	,28	,28	,GRED ,BLUE},//4
	{146,55	,28	,28	,GRED ,BLUE},//5
	{179,55	,28	,28	,GRED ,BLUE},//6
	{212,55	,28	,28	,GRED ,BLUE},//7
	{244,55	,28	,28	,GRED ,BLUE},//8
	{277,55	,28	,28	,GRED ,BLUE},//9
	{48	,92	,28	,28	,GRED ,BLUE},//��
	{81	,92	,28	,28	,GRED ,BLUE},//��
	{114,92	,28	,28	,GRED ,BLUE},//��
	{146,92	,28	,28	,GRED ,BLUE},//��
	{179,92	,28	,28	,GRED ,BLUE},//ǰ
	{212,92	,28	,28	,GRED ,BLUE},//��
	{244,92	,28	,28	,GRED ,BLUE},//��
	{277,92	,28	,28	,GRED ,BLUE},//С
	
	{24	,148,74	,28	,BRED ,BLUE},//��ʼ/¼����
	{122,148,28	,28	,GRED ,BLUE},//��־
	{191,148,100,28	,GREEN,BLUE},//������Ч/��Ч
	
	{24 ,194,140,28	,GREEN,BLUE},//��ʶ��/ѵ��%d��
	{232,194,52 ,28	,BRED ,BLUE},//����

	
	{140,100,140,28	,GREEN,BLUE},//�����:�ν�
	
	{85	,55	,140,28	,GRED ,BLUE},//ʶ����:
	{70	,95	,170,28	,GRED ,BLUE},//ƥ�����:
};

u16	touch_left;
u16	touch_top;

void wait_touch(void)
{
	while(1)
	{
		while(!TSC_IN_TOUCH());
        DBPLN("TSC_IN_TOUCH");
		if(touch_panel_read(&touch_left,&touch_top)!=0)
		{
			DBPLN("left=%d, top=%d",touch_left,touch_top);
			while(TSC_IN_TOUCH())touch_panel_read(&touch_left,&touch_top);
			break;
		}
	}
}

u8	touch_area(GUI_Area *area)
{
	if((touch_left>(area->Left))&&(touch_left<(area->Left+area->Width)))
	{
		if((touch_top>(area->Top))&&(touch_top<(area->Top+area->Height)))
		{
			return 1;
		}
	}
	return 0;
}

void GUI_HideArea(GUI_Area *Area)
{
	LCD_Fill(Area->Left,Area->Top,Area->Width,Area->Height,Screen.BackColor);
}

void GUI_ClrArea(GUI_Area *Area)
{
	LCD_Fill(Area->Left,Area->Top,Area->Width,Area->Height,Area->BackColor);
}

void GUI_DispStr(GUI_Area *Area,const u8 *str)
{
	u16 i=0;
	u16 left=0;
	u16 top=0;
	
	while(*(str+i)!=0)
	{
		if(*(str+i)>'~')
		{
			if((left+CN_Width)>Area->Width)
			{
				left=0;
				top+=(CN_Height+1);
				if((top+CN_Height)>Area->Height)
				{
					return;
				}
			}
			LCD_ShowCN(1+left+Area->Left, 2+top+Area->Top, (u8*)(str+i), Area->ForeColor);
			left+=(CN_Width+1);
			i+=2;
		}
		else
		{
			if((left+Char_Width)>Area->Width)
			{
				left=0;
				top+=(Char_Height+1);
				if((top+Char_Height)>Area->Height)
				{
					return;
				}
			}
			LCD_ShowChar(1+left+Area->Left,2+top+Area->Top,*(str+i),Area->ForeColor);
			left+=(Char_Width+1);
			i++;
		}
	}
}

void GUI_printf(GUI_Area *Area,char *fmt, ...) 
{
	char buffer[50];  
	
	va_list arg_ptr; 
	va_start(arg_ptr, fmt);   
	vsnprintf(buffer, 50, fmt, arg_ptr);
	GUI_DispStr(Area,(u8*)buffer);
	va_end(arg_ptr); 
}