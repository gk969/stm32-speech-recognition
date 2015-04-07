#ifndef M25P16_H
#define	M25P16_H

#define Select_Flash()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)

#define DisSelect_Flash()    GPIO_SetBits(GPIOA, GPIO_Pin_4)


void Flash_Wait_Busy(void);
void Flash_Sector_Earse(u32 Addr);
void Flash_Read(u32 start_address,u8 *buffer,u32 length);
void Flash_PageProgram(u32 start_address,u8 *buffer,u32 length);
void WriteFontfile(void);

#endif
