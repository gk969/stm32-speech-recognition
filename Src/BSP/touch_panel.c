#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "SPI.h"
#include "touch_panel.h"
#include "TFTLCD.H"

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

void touchPanelInit(){
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOF |RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  TSC_INT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TSC_INT_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  TSC_NSS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TSC_NSS_PORT, &GPIO_InitStructure);
    SELECT_TSC(false);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(TSC_SPI, &SPI_InitStructure);

    SPI_Cmd(TSC_SPI, ENABLE);
}



u8  touch_panel_read(u16 *X, u16 *Y) {
    u16 X_buffer[10], Y_buffer[10];
    u32 X_temp, Y_temp;
    u16 i = 0;

    while(i < 10 && TSC_IN_TOUCH()) {
        SELECT_TSC(true);
        
        TSC_EXCHANGE_DATA(CMD_RDY);
        delay_us(6);
        
        X_temp = TSC_EXCHANGE_DATA(0);
        X_temp = X_temp << 8;
        X_temp |= TSC_EXCHANGE_DATA(0);
        X_temp >>= 3;

        TSC_EXCHANGE_DATA(CMD_RDX);
        delay_us(6);

        Y_temp = TSC_EXCHANGE_DATA(0);
        Y_temp = Y_temp << 8;
        Y_temp |= TSC_EXCHANGE_DATA(0);
        Y_temp >>= 3;

        SELECT_TSC(false);

        if(X_temp > 100 && X_temp < 4000 && Y_temp > 100 && Y_temp < 4000) {
            X_buffer[i] = X_temp;
            Y_buffer[i] = Y_temp;

            i++;

        }
    }

    if(i == 10) {
        for(i = 0; i < 9; i++) {
            if(X_buffer[i] > X_buffer[i + 1]) {
                X_temp = X_buffer[i];
                X_buffer[i] = X_buffer[i + 1];
                X_buffer[i + 1] = X_temp;
            }

            if(Y_buffer[i] > Y_buffer[i + 1]) {
                Y_temp = Y_buffer[i];
                Y_buffer[i] = Y_buffer[i + 1];
                Y_buffer[i + 1] = Y_temp;
            }
        }

        for(i = 8; i > 0; i--) {
            if(X_buffer[i] < X_buffer[i - 1]) {
                X_temp = X_buffer[i];
                X_buffer[i] = X_buffer[i - 1];
                X_buffer[i - 1] = X_temp;
            }

            if(Y_buffer[i] < Y_buffer[i - 1]) {
                Y_temp = Y_buffer[i];
                Y_buffer[i] = Y_buffer[i - 1];
                Y_buffer[i - 1] = Y_temp;
            }
        }

        X_temp = 0;
        Y_temp = 0;

        for(i = 1; i < 9; i++) {
            X_temp += X_buffer[i];
            Y_temp += Y_buffer[i];
        }

        X_temp /= 8;
        Y_temp /= 8;
        
        //DBPLN("x=%d, y=%d", X_temp, Y_temp);
        
        
        *X = 351-Y_temp*32/353;
        *Y = 257 - X_temp*24/355;

        DBPLN("left=%d, top=%d",*X, *Y);

        return 1;
    } else
        return 0;

}

