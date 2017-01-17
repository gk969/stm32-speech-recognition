#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "SPI.h"
#include "spiFlash.h"
#include "FATFS.h"
#include "delay.h"
#include <string.h>

/*
?  512 Equal Sectors with 4K byte each
  - Any Sector can be erased individually
?  32 Equal Blocks with 64K byte each
  - Any Block can be erased individually
?  Program Capability
   - Byte base
  - Page base (256 bytes)
?  Latch-up protected to 100mA from -1V to Vcc +1V

                    储存空间分配表
            项目                            地址空间
    GB2312汉字24*24点阵字库     0x000000~0x08FFFF 0th~8th   Sectors
    ASCII字符24*12点阵字库      0x090000~0x09FFFF 9th       Sector
    Unicode to GB2312转换表     0x0A0000~0x0BFFFF 10th~11th Sectors
    GB2312汉字16*16点阵字库     0x0c0000~0x0fffff 12th~15th Sectors
*/



#define SPI_PORT    SPI2

#define TOTAL_SIZE        (8<<20)
#define PAGE_SIZE 256
#define SECTOR_SIZE (4<<10)
#define BLOCK_SIZE  (64<<10)

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


#define SPI_FLASH_CMD_PP    0x02  /* Write to Memory instruction */
#define SPI_FLASH_CMD_WRSR  0x01  /* Write Status Register instruction */
#define SPI_FLASH_CMD_WREN  0x06  /* Write enable instruction */
#define SPI_FLASH_CMD_WRDI  0x04  /* Write Disable instruction */

#define SPI_FLASH_CMD_READ  0x03  /* Read from Memory instruction */
#define SPI_FLASH_CMD_RDSR  0x05  /* Read Status Register instruction  */
#define SPI_FLASH_CMD_RDID  0x9F  /* Read identification */
#define SPI_FLASH_CMD_SE    0x20  /* Sector Erase instruction */
#define SPI_FLASH_CMD_BE    0x52  /* Block Erase instruction */
#define SPI_FLASH_CMD_CE    0xC7  /* Chip Erase instruction */

#define SPI_FLASH_WIP_FLAG  0x01  /* Write In Progress (WIP) flag */



#include "spiFlashTable.h"
void writeTableData(){
    u32 i;
    for(i=0; i<500; i++){
        delay_ms(10);
    }
    DBPLN("start writeTableData");
    
    {
        u8 testBuf[SECTOR_SIZE];
        u32 page;
        for(page=0; page<sizeof(dataToWrite); page+=PAGE_SIZE){
            u32 flashAddr=DATA_OFFSET+page;
            if(flashAddr%SECTOR_SIZE==0){
                spiFlashSectorEarse(flashAddr);
            }
            
            spiFlashPageProgram(flashAddr, dataToWrite+page, PAGE_SIZE);
            spiFlashRead(flashAddr, testBuf, PAGE_SIZE);
            
            DBPLN("spiFlashPageProgram %08X %s", DATA_OFFSET+page, memcmp(testBuf, dataToWrite+page, PAGE_SIZE)?"Fail":"Success");
        }
    }
}



#define TEST_SIZE   PAGE_SIZE
#define TEST_ADDR   PAGE_SIZE
void spiFlashTest(u8 seed) {
    u8 testBuf[TEST_SIZE];
    
    DBPLN("SPI flash before erase at %d:", TEST_ADDR);
    spiFlashRead(TEST_ADDR, testBuf, TEST_SIZE);
    DBPH(testBuf, TEST_SIZE);
    DBPLN("");

    DBPLN("SPI flash after erase at %d:", TEST_ADDR);
    spiFlashSectorEarse(TEST_ADDR);
    spiFlashRead(TEST_ADDR, testBuf, TEST_SIZE);
    DBPH(testBuf, TEST_SIZE);
    DBPLN("");

    {
        u32 i;

        for(i = 0; i < TEST_SIZE; i++) {
            testBuf[i] = seed++;
        }
    }

    DBPLN("SPI flash after program at %d:", TEST_ADDR);
    spiFlashPageProgram(TEST_ADDR, testBuf, TEST_SIZE);
    spiFlashRead(TEST_ADDR, testBuf, TEST_SIZE);
    DBPH(testBuf, TEST_SIZE);
    DBPLN("");
}


void spiFlashSelect(bool isSelect) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, !isSelect);
}

void spiFlashInit() {
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    spiFlashSelect(false);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI_PORT, &SPI_InitStructure);

    SPI_Cmd(SPI_PORT, ENABLE);
}

u8 spiFlashExchangeData(u8 sendData) {
    return SPI_Exchange_Data(SPI_PORT, sendData);
}


/* flash check busy*/
void spiFlashWaitBusy(void) {
    spiFlashSelect(true);
    spiFlashExchangeData(SPI_FLASH_CMD_RDSR);

    while(spiFlashExchangeData(0) & SPI_FLASH_WIP_FLAG);

    spiFlashSelect(false);
}

/*
    spiFlashRead
    The first byte addressed can be at any location.
The address is automatically incremented to the
next higher address after each byte of data is shift-
ed out. The whole memory can, therefore, be read
with a single Read Data Bytes (SPI_FLASH_CMD_READ) instruction.
When the highest address is reached, the address
counter rolls over to 000000h, allowing the read
sequence to be continued indefinitely.
*/
void spiFlashRead(u32 start_address, void *buffer, u32 length) {
    u32 i;
    u8 *data = (u8 *)buffer;

    spiFlashWaitBusy();

    spiFlashSelect(true);

    spiFlashExchangeData(SPI_FLASH_CMD_READ);// ??????
    spiFlashExchangeData((start_address & 0xFF0000) >> 16);
    spiFlashExchangeData((start_address & 0xFF00) >> 8);
    spiFlashExchangeData(start_address & 0xFF);

    for(i = 0; i < length; i++) {
        data[i] = spiFlashExchangeData(0);
    }

    spiFlashSelect(false);
}


/*
Any address inside the Sector is a valid address
for the Sector Erase (SPI_FLASH_CMD_SE) instruction.
*/
void spiFlashSectorEarse(u32 addr) {
    spiFlashWaitBusy();

    spiFlashSelect(true);
    spiFlashExchangeData(SPI_FLASH_CMD_WREN);   //д???
    spiFlashSelect(false);

    spiFlashWaitBusy();

    spiFlashSelect(true);
    spiFlashExchangeData(SPI_FLASH_CMD_SE);  //????????
    spiFlashExchangeData((addr & 0xFF0000) >> 16);
    spiFlashExchangeData((addr & 0xFF00) >> 8);
    spiFlashExchangeData(addr & 0xFF);
    spiFlashSelect(false);
}


/*
    If the 8 least significant address bits (A7-A0) are
not all zero, all transmitted data that goes beyond the
end of the current page are programmed from the start
address of the same page (from the address whose 8
least significant bits (A7-A0) are all zero).

    If more than 256 bytes are sent to the device, pre-
viously latched data are discarded and the last 256
data bytes are guaranteed to be programmed cor-
rectly within the same page. If less than 256 Data
bytes are sent to device, they are correctly pro-
grammed at the requested addresses without hav-
ing any effects on the other bytes of the same page.
*/
void spiFlashPageProgram(u32 start_address, const void *buffer, u32 length) {
    u32 i;
    u8 *data = (u8 *)buffer;

    spiFlashWaitBusy();

    spiFlashSelect(true);
    spiFlashExchangeData(SPI_FLASH_CMD_WREN);
    spiFlashSelect(false);

    spiFlashWaitBusy();

    spiFlashSelect(true);
    spiFlashExchangeData(SPI_FLASH_CMD_PP);
    spiFlashExchangeData((start_address & 0xFF0000) >> 16);
    spiFlashExchangeData((start_address & 0x00FF00) >> 8);
    spiFlashExchangeData(start_address & 0x0000FF);

    for(i = 0; i < length; i++) {
        spiFlashExchangeData(data[i]);
    }

    spiFlashSelect(false);
}

u8 FONT_24DOT_BUF[24*24/8];
u8 *getGB2312Font24Dot(const u8 *gb2312Code){
    spiFlashRead(sizeof(FONT_24DOT_BUF) * ((gb2312Code[1] - 0xA1) + (gb2312Code[0] - 0xA1) * 94), FONT_24DOT_BUF, sizeof(FONT_24DOT_BUF));
    return FONT_24DOT_BUF;
}

void printGB2312Font24Dot(const u8 *gb2312Code){
    u8 row, rank, bit;
    u8 *fontDot=getGB2312Font24Dot(gb2312Code);
    
    DBPLN("font %02X %02X", gb2312Code[0], gb2312Code[1]);
    
    for(row = 0; row < 24; row++) {
        for(rank = 0; rank < 3; rank++) {
            for(bit = 0; bit < 8; bit++) {
                if((*(fontDot + row * 3 + rank) & (1 << (7 - bit))) != 0) {
                    DBP("*");
                }else{
                    DBP(" ");
                }
            }
        }
        DBPLN("");
    }
    DBPLN("");
}
