#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "SPI.h"
#include "M25P16.h"
#include "FATFS.h"

/*
	以下为M26P16 SPI Flash 操作函数
			M26P16简介：
	The M25P16 is a 16Mbit (2M x 8) Serial Flash
Memory, with advanced write protection mechanisms,
accessed by a high speed SPI-compatible	bus.

	The memory can be programmed 1 to 256 bytes at
a time, using the Page Program instruction.

	The memory is organized as 32 sectors, each 
containing 256 pages. Each page is 256 bytes wide.
Thus, the whole memory can be viewed as consisting
 of 8192 pages, or 2,097,152 bytes.

	The whole memory can be erased using the Bulk
Erase instruction, or a sector at a time, using the
Sector Erase instruction.
			 		储存空间分配表
			项目							地址空间
	GB2312汉字24*24点阵字库	   	0x000000~0x08FFFF 0th~8th 	Sectors
	ASCII字符24*12点阵字库		0x090000~0x09FFFF 9th 	  	Sector
	Unicode to GB2312转换表		0x0A0000~0x0BFFFF 10th~11th	Sectors
	GB2312汉字16*16点阵字库		0x0c0000~0x0fffff 12th~15th Sectors
*/


 /* Private define ------------------------------------------------------------*/
#define PP		   0x02  /* Write to Memory instruction */
#define WRSR       0x01  /* Write Status Register instruction */
#define WREN       0x06  /* Write enable instruction */

#define READ       0x03  /* Read from Memory instruction */
#define RDSR       0x05  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define SE         0xD8  /* Sector Erase instruction */
#define BE         0xC7  /* Bulk Erase instruction */

#define WIP_Flag   0x01  /* Write In Progress (WIP) flag */


/* flash check busy*/
void Flash_Wait_Busy(void)
{

  Select_Flash();	

  SPI1_Exchange_Data(RDSR);

  while(SPI1_Exchange_Data(0xaa) & WIP_Flag); 

  DisSelect_Flash();	   

}

/*
Any address inside the Sector is a valid address 
for the Sector Erase (SE) instruction.
*/
void Flash_Sector_Earse(u32 Addr) 
{	
	if(Addr<0x0C0000)
	{		 
		//USART_SendStr(USART1,"Earse The Protected Area!");
	}
	else

	{		   
		Flash_Wait_Busy();
		
		Select_Flash(); 
		SPI1_Exchange_Data(WREN);	//写使能
		DisSelect_Flash();
		
		Flash_Wait_Busy();
		
		Select_Flash(); 
		SPI1_Exchange_Data(SE);  //块擦除
		SPI1_Exchange_Data((Addr & 0xFF0000) >> 16);
		SPI1_Exchange_Data((Addr & 0xFF00) >> 8);
		SPI1_Exchange_Data(Addr & 0xFF);
		DisSelect_Flash();

	}
}

/*
                Flash_Read
	The first byte addressed can be at any location.
The address is automatically incremented to the
next higher address after each byte of data is shift-
ed out. The whole memory can, therefore, be read
with a single Read Data Bytes (READ) instruction.
When the highest address is reached, the address
counter rolls over to 000000h, allowing the read
sequence to be continued indefinitely.
*/
void Flash_Read(u32 start_address,u8 *buffer,u32 length)
{
	u32 i;

	Flash_Wait_Busy(); 

	Select_Flash();	

    SPI1_Exchange_Data(READ);//	读数据
	SPI1_Exchange_Data((start_address & 0xFF0000) >> 16);
    SPI1_Exchange_Data((start_address & 0xFF00) >> 8);
    SPI1_Exchange_Data(start_address & 0xFF);
	for (i=0;i<length;i++)
	{
		*(buffer+i) = SPI1_Exchange_Data(12);
		//USART_SendData(USART1,SPI1_Exchange_Data(12));
	}

	DisSelect_Flash();
}





/******  写入之前必须擦除 确保要写入的比特位全部为1 否则写入出错******/
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
void Flash_PageProgram(u32 start_address,u8 *buffer,u32 length)
{
	u32 i;

	if((length>256)||(start_address&0x0000FF!=0)||(start_address<0x0C0000))
	{	 
		//USART_SendStr(USART1,"Flash Page Program Error! ");
	}
	else
	{	 
		Flash_Wait_Busy();
		Select_Flash(); 
		SPI1_Exchange_Data(WREN);
		DisSelect_Flash(); 
		
		Flash_Wait_Busy();
		Select_Flash(); 
		SPI1_Exchange_Data(PP);
		SPI1_Exchange_Data((start_address & 0xFF0000) >> 16);
		SPI1_Exchange_Data((start_address & 0x00FF00) >> 8);
		SPI1_Exchange_Data(start_address & 0x0000FF);
		for (i=0;i<length;i++)
		{
			/*
			USART_SendData(USART1,*(buffer+i));
			*/
			SPI1_Exchange_Data(*(buffer+i));
		}
		DisSelect_Flash();
	}
}
/*
void WriteFontfile(void)
{
	FS_Object font_file;
	u8* file_buf;
	u32 i=0;
								
	if(OpenFile(&font_file,"/font/宋体16.dot")==FILE_EXIST)
	{
		for(i=0x0c0000;i<0x100000;i+=0x010000)
		{
			Flash_Sector_Earse(i);
		}
		i=0x0c0000;
	
		while(file_buf=ReadFile(&font_file))
		{
			Flash_PageProgram(i,file_buf,256);
			Flash_PageProgram(i+256,file_buf+256,256);
			i+=512;
		}
	}
}
	 */
