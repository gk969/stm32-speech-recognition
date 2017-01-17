#ifndef __DELAY_H
#define __DELAY_H 			   

#define SYSCLK 72

#define	fac_us (SYSCLK/8)		    
#define	fac_ms ((u16)fac_us*1000)

//³õÊ¼»¯ÑÓ³Ùº¯Êý
void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 Nus);
#endif
