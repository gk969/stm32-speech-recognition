#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "SDCard.h"
#include "FATFS.H"

#include "M25P16.h"

void Disk_ReadSec(u32 Addr, u32 *DataBuf)
{
	SD_ReadBlock(Addr, DataBuf, BYTES_PERSEC);
}

void Disk_WriteSec(u32 Addr, u32 *DataBuf)
{
	SD_WriteBlock(Addr, DataBuf, BYTES_PERSEC);
}

u16 UnicodeToGb2312(u16 Unicode)
{
	u16 GB2312;
	u8 tem;
	u8 *temp=(u8 *)(&GB2312);
	Flash_Read(0x0A0000+Unicode*2,(u8 *)&GB2312,2);

	/* Big endian to Little endian */
	tem=*(temp+1);
	*(temp+1)=*temp;
	*temp=tem;

	return GB2312;
}


