#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "spiFlash.h"
#include "TFTLCD.h"
#include "delay.h"
#include "GUI.H"


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


u8  LCD_COORD = COORD_240_320;

/*------------------------液晶说明------------------------------
液晶型号位置
大小:320*240
像素:262K色
VCC   3.3V
VCCIO 2.7~3.3V
数据线操作电压:3.3V
颜色排列:以一个十六位的数存放一个点的颜色 RGB:565 即:最高五位为红色
灰阶.中间六位为绿色灰阶.最低五位为蓝色灰阶.
例如:0XF800 纯红色
     0X07E0 纯绿色
     0X001F 纯蓝色
----------------------------------------------------------------*/

u16 LCD_RD_CDM(u16 index) {
    LCD_WR_REG(index);
    return(*(__IO uint16_t *)(Bank1_LCD_D));
}



void FSMC_LCD_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  FSMC_NORSRAMTimingInitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                //PB0 推挽输出 背光
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
    
    //PORTD复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15; // //PORTD复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;          //复用推挽输出
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //PORTE复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // //PORTD复用推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    //  //PORTG12复用推挽输出 A0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_12;  // //PORTD复用推挽输出
    GPIO_Init(GPIOG, &GPIO_InitStructure);


    /*-- FSMC Configuration ------------------------------------------------------*/
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 0x02;
    FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

    /* Enable FSMC Bank1_SRAM Bank */
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

void LCD_Init(void) {
    FSMC_LCD_Init();
    
    
    LCD_WR_CMD(0x00E3, 0x3008); // Set internal timing
    LCD_WR_CMD(0x00E7, 0x0012); // Set internal timing
    LCD_WR_CMD(0x00EF, 0x1231); // Set internal timing
    LCD_WR_CMD(0x0000, 0x0001); // Start Oscillation
    LCD_WR_CMD(0x0001, 0x0100); // set SS and SM bit
    LCD_WR_CMD(0x0002, 0x0700); // set 1 line inversion

    LCD_WR_CMD(0x0003, 0x1030); //1010 set GRAM write direction and BGR=0,262K colors,1 transfers/pixel.
    LCD_WR_CMD(0x0004, 0x0000); // Resize register
    LCD_WR_CMD(0x0008, 0x0202); // set the back porch and front porch
    LCD_WR_CMD(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
    LCD_WR_CMD(0x000A, 0x0000); // FMARK function
    LCD_WR_CMD(0x000C, 0x0000); // RGB interface setting
    LCD_WR_CMD(0x000D, 0x0000); // Frame marker Position
    LCD_WR_CMD(0x000F, 0x0000); // RGB interface polarity
    //Power On sequence
    LCD_WR_CMD(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
    LCD_WR_CMD(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
    LCD_WR_CMD(0x0012, 0x0000); // VREG1OUT voltage
    LCD_WR_CMD(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
    delay_ms(20); // Dis-charge capacitor power voltage
    LCD_WR_CMD(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
    LCD_WR_CMD(0x0011, 0x0227); // R11h=0x0221 at VCI=3.3V, DC1[2:0], DC0[2:0], VC[2:0]
    delay_us(50); // delay_us 50ms
    LCD_WR_CMD(0x0012, 0x001C); // External reference voltage= Vci;
    delay_us(50); // delay_us 50ms
    LCD_WR_CMD(0x0013, 0x1800); // R13=1200 when R12=009D;VDV[4:0] for VCOM amplitude
    LCD_WR_CMD(0x0029, 0x0014); // R29=000C when R12=009D;VCM[5:0] for VCOMH
    LCD_WR_CMD(0x002B, 0x000D); // Frame Rate = 91Hz
    delay_us(50); // delay_us 50ms
    LCD_WR_CMD(0x0020, 0x0000); // GRAM horizontal Address
    LCD_WR_CMD(0x0021, 0x0000); // GRAM Vertical Address
    // ----------- Adjust the Gamma Curve ----------//
    LCD_WR_CMD(0x0030, 0x0007);
    LCD_WR_CMD(0x0031, 0x0302);
    LCD_WR_CMD(0x0032, 0x0105);
    LCD_WR_CMD(0x0035, 0x0206);
    LCD_WR_CMD(0x0036, 0x0808);
    LCD_WR_CMD(0x0037, 0x0206);
    LCD_WR_CMD(0x0038, 0x0504);
    LCD_WR_CMD(0x0039, 0x0007);
    LCD_WR_CMD(0x003C, 0x0105);
    LCD_WR_CMD(0x003D, 0x0808);
    //------------------ Set GRAM area ---------------//
    LCD_WR_CMD(0x0050, 0x0000); // Horizontal GRAM Start Address
    LCD_WR_CMD(0x0051, 0x00EF); // Horizontal GRAM End Address
    LCD_WR_CMD(0x0052, 0x0000); // Vertical GRAM Start Address
    LCD_WR_CMD(0x0053, 0x013F); // Vertical GRAM Start Address
    LCD_WR_CMD(0x0060, 0xA700); // Gate Scan Line
    LCD_WR_CMD(0x0061, 0x0001); // NDL,VLE, REV
    LCD_WR_CMD(0x006A, 0x0000); // set scrolling line
    //-------------- Partial Display Control ---------//
    LCD_WR_CMD(0x0080, 0x0000);
    LCD_WR_CMD(0x0081, 0x0000);
    LCD_WR_CMD(0x0082, 0x0000);
    LCD_WR_CMD(0x0083, 0x0000);
    LCD_WR_CMD(0x0084, 0x0000);
    LCD_WR_CMD(0x0085, 0x0000);
    //-------------- Panel Control -------------------//
    LCD_WR_CMD(0x0090, 0x0010);
    LCD_WR_CMD(0x0092, 0x0000);
    LCD_WR_CMD(0x0093, 0x0003);
    LCD_WR_CMD(0x0095, 0x0110);
    LCD_WR_CMD(0x0097, 0x0000);
    LCD_WR_CMD(0x0098, 0x0000);
    LCD_WR_CMD(0x0007, 0x0133); // 262K color and display ON
    
    
    LCD_Set_Coord(COORD_320_240);
    LCD_Fill(0, 0, 320, 240, GREEN);
}

u16 RGB(u8 red, u8 green, u8 blue) {
    u16 rgb565 = 0;

    rgb565 = (red >> 3) << 11;
    rgb565 += (green >> 2) << 5;
    rgb565 += blue >> 3;

    return rgb565;
}

void LCD_Set_Disp_Window(u16 left, u16 top, u16 width, u16 height) {
    LCD_WR_CMD(0x0050, left); // Horizontal GRAM Start Address
    LCD_WR_CMD(0x0051, left + width - 1); // Horizontal GRAM End Address
    LCD_WR_CMD(0x0052, top); // Vertical GRAM Start Address
    LCD_WR_CMD(0x0053, top + height - 1); // Vertical GRAM Start Address
}

void LCD_Set_Address(u16 left, u16 top) {
    LCD_WR_CMD(0x0020, left); // Horizontal GRAM Start Address
    LCD_WR_CMD(0x0021, top); // Vertical GRAM Start Address
    LCD_WR_REG(0x0022);
}

void LCD_Set_Coord(u8 coord) {
    LCD_COORD = coord;

    if(LCD_COORD == COORD_320_240) {
        Screen.Width = 320;
        Screen.Height = 240;
    } else {
        Screen.Width = 240;
        Screen.Height = 320;
    }
}

void LCD_Fill(u16 left, u16 top, u16 width, u16 height, u16 color) {
    u32 temp, n;

    if((left + width) > Screen.Width) {
        width = Screen.Width - left;
    }

    if((top + height) > Screen.Height) {
        height = Screen.Height - top;
    }

    if(LCD_COORD == COORD_320_240) {
        temp = width;
        width = height;
        height = temp;

        temp = top;
        top = left;
        left = 240 - temp - width;
    }

    LCD_Set_Disp_Window(left, top, width, height);

    LCD_Set_Address(left, top);

    temp = (u32)width * height;

    for(n = 0; n < temp; n++) {
        LCD_WR_Data(color);
    }

    LCD_Set_Disp_Window(0, 0, 240, 320);
}

void LCD_DrawPoint(u16 left, u16 top, u16 color) {
    u16 temp;

    if((left >= Screen.Width) || (top >= Screen.Height)) {
        return;
    }

    if(LCD_COORD == COORD_320_240) {
        temp = top;
        top = left;
        left = 240 - temp;
    }

    LCD_Set_Address(left, top);
    LCD_WR_Data(color);
}

u16 Abs_Value(u16 num1, u16 num2) {
    if(num1 > num2) {
        return (num1 - num2);
    } else {
        return (num2 - num1);
    }
}

void LCD_DrawLine(u16 left1, u16 top1, u16 left2, u16 top2, u16 color) {
    s32 left, top;
    u16 t;

    if((left1 == left2) && (top1 == top2)) {
        LCD_DrawPoint(left1, top1, color);
    }

    else if(Abs_Value(top2, top1) > Abs_Value(left2, left1)) {
        if(top1 > top2) {
            t = left1;
            left1 = left2;
            left2 = t;
            t = top1;
            top1 = top2;
            top2 = t;
        }

        for(top = top1; top <= top2; top++) {
            left = (s32)left1 - ((s32)top1 - top) * ((s32)left1 - (s32)left2) / ((s32)top1 - (s32)top2);
            LCD_DrawPoint(left, top, color);
        }
    }

    else {
        if(left1 > left2) {
            t = left1;
            left1 = left2;
            left2 = t;
            t = top1;
            top1 = top2;
            top2 = t;
        }

        for(left = left1; left <= left2; left++) {
            top = (s32)top1 - ((s32)left1 - left) * ((s32)top1 - (s32)top2) / ((s32)left1 - (s32)left2);
            LCD_DrawPoint(left, top, color);
        }
    }
}

void LCD_ShowChar(u16 left, u16 top, u8 char_data, u16 color) {
    u32 Addr;
    u8 row, rank, bit;
    u8 Font_Tab[36];

    Addr = 36 * (char_data - ' ') + 589824;
    spiFlashRead(Addr, Font_Tab, 36);

    for(rank = 0; rank < 12; rank++) {
        for(row = 0; row < 3; row++) {
            for(bit = 0; bit < 8; bit++) {
                if((*(Font_Tab + row + rank * 3) & (1 << (7 - bit))) != 0) {
                    LCD_DrawPoint(left + rank, top + row * 8 + bit, color);
                }
            }
        }
    }
}

void LCD_ShowCN(u16 left, u16 top, u8 *cn, u16 color) {
    u32 Addr;
    u8 row, rank, bit;
    u8 *fontDot=getGB2312Font24Dot(cn);
    //DBPLN("LCD_ShowCN %02X %02X", cn[0], cn[1]);
    
    for(row = 0; row < 24; row++) {
        for(rank = 0; rank < 3; rank++) {
            for(bit = 0; bit < 8; bit++) {
                if((*(fontDot + row * 3 + rank) & (1 << (7 - bit))) != 0) {
                    LCD_DrawPoint(left + rank * 8 + bit, top + row, color);
                }
            }
        }
    }
}

