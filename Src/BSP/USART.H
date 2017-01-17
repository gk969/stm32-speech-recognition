#ifndef USART_H
#define	USART_H

void USART1_configuration(void);
void USART_SendStr(USART_TypeDef* USARTx, uint8_t *Data);
void USART_SendArray(USART_TypeDef* USARTx, uint8_t *Data ,u16 len);
void UASRT_DMA_TXConfiguration(USART_TypeDef* USARTx,u8 *BufferSRC, u32 BufferSize);
void USART1_printf (char *fmt, ...); 

#endif 
