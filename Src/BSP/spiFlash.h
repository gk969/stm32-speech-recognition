#ifndef SPI_FLASH_H
#define	SPI_FLASH_H


void spiFlashRead(u32 start_address,void *buffer, u32 length);
void spiFlashSectorEarse(u32 Addr);
void spiFlashPageProgram(u32 start_address, const void *buffer, u32 length);
void spiFlashTest(u8 seed);

u8 *getGB2312Font24Dot(const u8 *gb2312Code);
void printGB2312Font24Dot(const u8 *gb2312Code);
#endif
