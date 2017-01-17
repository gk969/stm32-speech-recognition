#ifndef __GUI_H
#define __GUI_H

typedef struct 
{
	u16 Left;		//区域离屏幕左边界的距离 像素数
	u16 Top;		//区域离屏幕上边界的距离 像素数
	u16	Width;		//区域宽度
	u16	Height;		//区域高度
	
	u16 BackColor;	//区域背景色
	u16 ForeColor;	//区域前景色
}GUI_Area;

extern GUI_Area Screen;
extern GUI_Area Label[];
extern u16	touch_left;
extern u16	touch_top;

#define	G_wel		0
#define	G_neme		1

#define	G_prc		2
#define	G_recg		3


#define	G_cap		4

//必须保证顺序 模板存储时用到
#define	G_num0		5
#define	G_num9		14
#define	G_up		15
#define	G_down		16
#define	G_front		17
#define	G_back		18
#define	G_left		19
#define	G_right		20
#define	G_big		21
#define	G_little	22

#define	G_comm_fst	G_num0
#define	G_comm_lst	G_little


#define	G_ctrl		23
#define	G_spk		24
#define	G_stus		25

#define	G_count		26
#define	G_return	27

#define	G_designer	28

#define	G_recg_res	29
#define	G_mtch_dis	30


void wait_touch(void);							//等待屏幕点击
u8	 touch_area(GUI_Area *area);				//判断是否点击指定区域
void GUI_HideArea(GUI_Area *Area);				//隐藏区域 显示屏幕前景色
void GUI_ClrArea(GUI_Area *Area);				//清除区域 显示区域背景色
void GUI_DispStr(GUI_Area *Area,const u8 *str);	//在区域内显示字符串
void GUI_printf(GUI_Area *Area,char *fmt, ...);	//printf函数在区域内的实现

#endif
