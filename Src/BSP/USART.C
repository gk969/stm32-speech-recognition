#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "USART.h"

//#define Debug

void USART1_configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;            //USART1 TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);          //A端口

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;           //USART1 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //复用开漏输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    /* USART1 configuration ------------------------------------------------------*/
    /* USART and USART2 configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure USART1 */
    USART_Init(USART1, &USART_InitStructure);


    /* Enable USART1 Receive interrupts */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

    /* Enable the USART1 */
    USART_Cmd(USART1, ENABLE);
}

void USART_SendStr(USART_TypeDef *USARTx, uint8_t *Data) {
    u16 i;

    for(i = 0; * (Data + i) != '\0'; i++) {
        USART_SendData(USARTx, *(Data + i));
    }
}

void USART_SendArray(USART_TypeDef *USARTx, uint8_t *Data , u16 len) {
    u16 i;

    for(i = 0; i < len; i++) {
        USART1_printf("%x ", *(Data + i));
    }
}

void UASRT_DMA_TXConfiguration(USART_TypeDef *USARTx, u8 *BufferSRC, u32 BufferSize) {
    DMA_InitTypeDef DMA_InitStructure;

    DMA_ClearFlag(DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4 | DMA1_FLAG_GL4);

    /* DMA1 Channel4 disable */
    DMA_Cmd(DMA1_Channel4, DISABLE);

    /* DMA1 Channel4 Config */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(USARTx->DR));
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BufferSRC;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = BufferSize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    /* DMA1 Channel4 enable */
    DMA_Cmd(DMA1_Channel4, ENABLE);
}

char printfBuf[256];
void USART1_printf(char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(printfBuf, sizeof(printfBuf), fmt, arg_ptr);
    USART_SendStr(USART1, (u8 *)printfBuf);
    va_end(arg_ptr);
}

void USART1_printf_line(char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(printfBuf, sizeof(printfBuf), fmt, arg_ptr);
    va_end(arg_ptr);
    
    USART_SendStr(USART1, (u8 *)printfBuf);
    USART_SendStr(USART1, "\r\n");
}

void USART1_print_hex(void *data, u32 size){
    u8 *buf=(u8*)data;
    
    u32 i;
    for(i=0; i<size; i++){
        USART1_printf("%02X ", buf[i]);
        
        if(i%8==7){
            USART_SendStr(USART1, " ");
        }
        
        if(i%16==15){
            USART_SendStr(USART1, "\r\n");
        }
    }
    
    USART_SendStr(USART1, "\r\n");
}
