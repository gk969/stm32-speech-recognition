/******* FAT FS_Object System Operating Library ********/
/*******       Only FAT32 Support !!!      ********/
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "FATFS.h"
#include "Interface.h"
#include "USART.H"
#include "FS_Structure.h"

static void CopyRam(const u8 *Source,u8 *Target,u16 Length);
static void Convert_LngNm(u8 *LngNm);
static void Convert_ShortNm(u8 *ShortNm);
static FS_Status CompareName(u8 *Name1,u8 *Name2);

static u32 Get_ClusAddr(u32 Clus);
static void ReadSec(u32 CurClus,u8 CurSec,u32 *Buffer);
static void WriteSec(u32 CurClus,u8 CurSec,u32 *Buffer);

static u32 Read_NextClusNum(u32 CurClus);
static u32	Write_NextClusNum(u32 LastClus);


u32	FS_Buffer[BYTES_PERSEC/sizeof(u32)];

FATFS FS; 

FS_Object	RootDir;	

/*
函数功能：	文件系统初始化。
			初始化结构体FS，获取FAT文件系统参数。
			初始化结构体RootDir，获取根目录参数。
参数	：	void
返回值	：	void
*/
void FATFS_Init(void)
{		
	
	FAT32_BPB *BPB;
	BPB=(FAT32_BPB *)FS_Buffer;

	Disk_ReadSec(BPB_ADDR, FS_Buffer);
	USART_SendArray(USART1,(u8 *)FS_Buffer,512);
	FS.Sec_PerFAT=BPB->BPB_FATSz32;
	
	FS.Sec_PerClus=BPB->BPB_SecPerClus;											   
	USART1_printf("Sec Per Clus:%d\r\n",FS.Sec_PerClus);
	
	FS.Bytes_PerClus=BYTES_PERSEC*FS.Sec_PerClus;

	USART1_printf("Total Sec32:%d\r\n",BPB->BPB_TotalSec32);

	FS.FS_Size=BPB->BPB_TotalSec32*BYTES_PERSEC;
	USART1_printf("FS_Size:%dMB\r\n",FS.FS_Size>>20);
	
	FS.FAT_Addr=BPB->BPB_ReservedSec*BYTES_PERSEC;

	FS.Root_Clus=BPB->BPB_RootClus;

	FS.Data_Addr=FS.FAT_Addr+
				BPB->BPB_FATSz32*BPB->BPB_NumFATs*BYTES_PERSEC;
	
	RootDir.FstClus=FS.Root_Clus;	 
}

/*
函数功能：	文件系统测试
参数	：	void
返回值	：	void
*/
void FS_Test(void)
{
	FS_Object TextFile;
	FS_Object TextDir;
	u8 str[]="star dust!";
	
	CreateNewObject(&RootDir,&TextDir,"test",DIR);
	CreateNewObject(&TextDir,&TextFile,"text.txt",FILE);
	WriteFile(&TextFile,str,sizeof(str)-1);
	
	if(Search_inDir(&RootDir,&TextDir,"picture",DIR)==FILE_EXIST)
	{
		LsDir(&TextDir);
	}
}

/*
函数功能：	从FAT中读取下一簇号
参数	：	CurClus		当前簇号
返回值	：	NextClus	下一簇号
*/
u32 Read_NextClusNum(u32 CurClus)
{	 
	u32 NextClus;
	
	Disk_ReadSec(FS.FAT_Addr+
				 (CurClus/(BYTES_PERSEC/sizeof(u32))*
				 BYTES_PERSEC),
				 (u32 *)FS_Buffer);

	NextClus=*((u32 *)FS_Buffer+
			  (CurClus%(BYTES_PERSEC/sizeof(u32))));

	return NextClus;
}

/*
函数功能：	向文件或文件夹写入数据时获取下一个空白簇号
参数	：	LastClus		上一簇号
返回值	：	New_ClusNum		新的待写入簇号
*/
u32	Write_NextClusNum(u32 LastClus)
{
	u32		New_ClusNum;
	u32		FAT_SecAddr;
	u16		i;
	
	New_ClusNum=FS_EOF;
	for(FAT_SecAddr=FS.FAT_Addr;
		FAT_SecAddr<(FAT_SecAddr+FS.Sec_PerFAT*BYTES_PERSEC);
		FAT_SecAddr+=BYTES_PERSEC)
	{
		Disk_ReadSec(FAT_SecAddr,FS_Buffer);
		for(i=0;i<(BYTES_PERSEC/sizeof(u32));i++)
		{
			if(*((u32 *)FS_Buffer+i)==0)
			{
				New_ClusNum=(FAT_SecAddr-FS.FAT_Addr)/sizeof(u32)+i;
				*((u32 *)FS_Buffer+i)=FS_EOF;
				Disk_WriteSec(FAT_SecAddr,FS_Buffer);
				
				Disk_ReadSec(FAT_SecAddr+FS.Sec_PerFAT*BYTES_PERSEC,FS_Buffer);
				*((u32 *)FS_Buffer+i)=FS_EOF;
				Disk_WriteSec(FAT_SecAddr+FS.Sec_PerFAT*BYTES_PERSEC,FS_Buffer);
				
				if(LastClus!=FST_CLUS)
				{
					Disk_ReadSec(FS.FAT_Addr+
								(LastClus/(BYTES_PERSEC/sizeof(u32))*BYTES_PERSEC),
								(u32 *)FS_Buffer);
					*((u32 *)FS_Buffer+(LastClus%(BYTES_PERSEC/sizeof(u32))))=New_ClusNum;
					Disk_WriteSec(FS.FAT_Addr+
								(LastClus/(BYTES_PERSEC/sizeof(u32))*BYTES_PERSEC),
								(u32 *)FS_Buffer);
					
					Disk_ReadSec(FS.FAT_Addr+FS.Sec_PerFAT*BYTES_PERSEC+
								(LastClus/(BYTES_PERSEC/sizeof(u32))*BYTES_PERSEC),
								(u32 *)FS_Buffer);
					*((u32 *)FS_Buffer+(LastClus%(BYTES_PERSEC/sizeof(u32))))=New_ClusNum;
					Disk_WriteSec(FS.FAT_Addr+FS.Sec_PerFAT*BYTES_PERSEC+
								(LastClus/(BYTES_PERSEC/sizeof(u32))*BYTES_PERSEC),
								(u32 *)FS_Buffer);
				}
				return New_ClusNum;
			}
		}
	}
	
	return New_ClusNum;
}

/*
函数功能：	读取一个扇区
参数	：	CurClus	当前簇号
			CurSec	当前簇内扇区号
			Buffer	数据缓冲区
返回值	：	void
*/
void ReadSec(u32 CurClus,u8 CurSec,u32 *Buffer)
{
	u32 SecAddr;
	SecAddr=Get_ClusAddr(CurClus)+
			CurSec*BYTES_PERSEC;
	Disk_ReadSec(SecAddr,Buffer);
}

/*
函数功能：	写一个扇区
参数	：	CurClus	当前簇号
			CurSec	当前簇内扇区号
			Buffer	数据缓冲区
返回值	：	void
*/
void WriteSec(u32 CurClus,u8 CurSec,u32 *Buffer)
{
	u32 SecAddr;
	SecAddr=Get_ClusAddr(CurClus)+
			CurSec*BYTES_PERSEC;
	Disk_WriteSec(SecAddr,Buffer);
}

/*
函数功能：	获取簇号对应的存储器地址
参数	：	Clus	簇号
返回值	：			地址
*/
u32 Get_ClusAddr(u32 Clus) 
{
	return(FS.Data_Addr+(Clus-2)*FS.Bytes_PerClus);
}	

/*
函数功能：	内存复制
参数	：	Source	数据源地址
			Target	数据目标地址
			Length	数据长度
返回值	：	void
*/
void CopyRam(const u8 *Source,u8 *Target,u16 Length)
{
	while(Length!=0)
	{
		Length--;
		*(Target+Length)=*(Source+Length);
	}
}

/*
函数功能：	内存区域清除（置0）
参数	：	Target	目标地址
			Length	目标长度
返回值	：	void
*/
void ClearRam(u8 *Target,u16 Length)
{
	while(Length!=0)
	{
		Length--;
		*(Target+Length)=0;
	}
}

/*
函数功能：	转换长文件名，Unicode码转为Gb2312码。用于读取文件名。
参数	：	LngNm	长文件名地址
返回值	：	void
*/
void Convert_LngNm(u8 *LngNm)
{
	u8 	i,h;
	u8	LngNm_Buffer[106];
	u16 *LngNm_Unicode;
   	LngNm_Unicode=(u16 *)LngNm;
	h=0;
	for(i=0;i<106;i++)
	{		
		if((i&0x01)==0)
		{
			LngNm_Unicode=(u16 *)(LngNm+i);
			if(*LngNm_Unicode>0x00A0)
			{
				*LngNm_Unicode=UnicodeToGb2312(*LngNm_Unicode);
			}	 
			else if(*LngNm_Unicode==0)
			{	
				break;
			}
		}

		if(*(LngNm+i)!=0)
		{
			*(LngNm_Buffer+h)=*(LngNm+i);
			h++;
		}
	}

	*(LngNm_Buffer+h)=0;
	CopyRam(LngNm_Buffer,LngNm,h+1);
}

/*
函数功能：	转换短文件名，去除短文件名中的空格。用于读取文件名。
参数	：	ShortNm	短文件名地址
返回值	：	void
*/
void Convert_ShortNm(u8 *ShortNm)
{
	u8 i,h;
	u8 ShortNmBuffer[13];
	h=0;

	for(i=0;i<8;i++)
	{
		if(*(ShortNm+i)==0x20)
		{
			break;
		}
		*(ShortNmBuffer+h)=*(ShortNm+i);
		h++;
	}
	if(*(ShortNm+8)!=0x20)
	{
		*(ShortNmBuffer+h)='.';
		h++;
	
		for(i=8;i<11;i++)
		{
			if(*(ShortNm+i)==0x20)
			{
				break;
			}
			*(ShortNmBuffer+h)=*(ShortNm+i);
			h++;
		}
	}

	*(ShortNmBuffer+h)=0;
	CopyRam(ShortNmBuffer,ShortNm,h+1);
}

/*
函数功能：	对比文件名，英文大小写不敏感。
参数	：	Name1	文件名1地址
			Name2	文件名2地址
返回值	：	SUCCESSED或FAILED
*/
FS_Status CompareName(u8 *Name1,u8 *Name2)
{
	u8 i;
	for(i=0;i<250;i++)
	{
		if(*(Name1+i)>128||*(Name2+i)>128)
		{
			if(*(Name1+i)!=*(Name2+i))
			{
				return FAILED;
			}
		}
		else
		{
			if(*(Name1+i)>*(Name2+i))
			{
				if((*(Name1+i)-32)!=*(Name2+i))
				{
					return FAILED;
				}
			}
			if(*(Name2+i)>*(Name1+i))
			{
				if((*(Name2+i)-32)!=*(Name1+i))
				{
					return FAILED;
				}
			}
		}
		if(*(Name1+i)==0&&*(Name2+i)==0)
		{
			return SUCCESSED;
		}
	}
	return FAILED;
}
/*
函数功能：	根据文件(夹)名在文件夹中寻找文件(夹)，如果找到则获取文件(夹)参数
参数	：	CurDir		结构体指针，当前文件夹
			Target		结构体指针，目标文件(夹)
			Target_Name	目标文件(夹)名
			Object		FILE或DIR，用于区分目标是文件还是文件夹
返回值	：	FILE_EXIST或FILE_NOTEXIST
*/
FS_Status Search_inDir(FS_Object 	*CurDir,		
					   FS_Object 	*Target,		
					   u8 			*Target_Name,
					   u8			Object)		
{	
	DIR_tag	*DirTag;	 
	LongDir_Ent	*LongDirEnt;
	u32		CurClus;
	u16		CurEntAddr;
   	u8		CurSec,LngNmCnt,DirEntCnt;
	u16		Name_Buffer[53]; //最多容纳4个目录项 共52个Unicode字符
	u8		*Name;

	Name_Buffer[52]=0;
	CurClus=CurDir->FstClus;
	LngNmCnt=4;
	DirEntCnt=4;
	do
	{
		/***** Read and Scan FS_Object Data Aera by Sector ******/
		for(CurSec=0;CurSec<FS.Sec_PerClus;CurSec++)
		{
			ReadSec(CurClus,CurSec,FS_Buffer);
	
			/***** Scan Every DirEntry , 32 Bytes per DirEntry! *****/
			for(CurEntAddr=0;CurEntAddr<BYTES_PERSEC;CurEntAddr+=DIRENT_SIZE)
			{
				DirTag=(DIR_tag*)((u32)FS_Buffer+CurEntAddr);
	
				if(DirTag->FileName[0]==EMPTY)
				{
					return	FILE_NOTEXIST;
				}

				if(DirTag->FileName[0]!=DELETED)
				{
					if(DirTag->Attribute==LONG_NAME)
					{		   
						if(LngNmCnt!=0)
						{	
							LngNmCnt--;
							LongDirEnt=(LongDir_Ent*)DirTag;
							Name=(u8 *)Name_Buffer+LngNmCnt*26;
							CopyRam(LongDirEnt->Name1,Name,10);
							CopyRam(LongDirEnt->Name2,Name+10,12);
							CopyRam(LongDirEnt->Name3,Name+22,4);
						}
					}
					else if(DirTag->Attribute&Object)
					{
						if(LngNmCnt!=4)
						{
							LngNmCnt=4;
							Convert_LngNm(Name);
						}
						else
						{	
							Name=(u8 *)Name_Buffer;
							CopyRam(DirTag->FileName,Name,11);
							Convert_ShortNm(Name);
						}
						
						if(CompareName(Name,Target_Name)==SUCCESSED)
						{
							Target->Name	=Target_Name;
							Target->Attrib	=DirTag->Attribute;
							Target->Size	=DirTag->FileLength;
							Target->FstClus=DirTag->FstClusHI;
							Target->FstClus<<=16;
							Target->FstClus+=DirTag->FstClusLO;
							Target->DirEntryAddr=Get_ClusAddr(CurClus)
													 +CurSec*BYTES_PERSEC
													 +CurEntAddr;
							Target->CurClus=Target->FstClus;
							Target->CurSec=0;
							return FILE_EXIST;
						}
	
					}
					if(LngNmCnt!=4)
					{
						DirEntCnt--;
					}
					if(LngNmCnt!=DirEntCnt)
					{
						LngNmCnt=4;
						DirEntCnt=4;
					}
				}	
			}
		}
		CurClus=Read_NextClusNum(CurClus);
	}
	while(CurClus!=0x0fffffff);
	
	return	FILE_NOTEXIST;				
}

/*
函数功能：	列出文件夹中的所有项目
参数	：	CurDir	当前文件夹
返回值	：	void
*/
void LsDir(FS_Object* CurDir)	
{	
	DIR_tag		*DirTag;	 
	LongDir_Ent	*LongDirEnt;
	u32 	CurClus;
	u16		CurEntAddr;
   	u8		CurSec,LngNmCnt,DirEntCnt;
	u16		Name_Buffer[53]; //最多容纳4个目录项 共52个Unicode字符
	u8		*Name;

	Name_Buffer[52]=0;
	CurClus=CurDir->FstClus;
	LngNmCnt=4;
	DirEntCnt=4;
	do
	{
		/***** Read and Scan FS_Object Data Aera by Sector ******/
		for(CurSec=0;CurSec<FS.Sec_PerClus;CurSec++)
		{
			ReadSec(CurClus,CurSec,FS_Buffer);
	
			/***** Scan Every DirEntry , 32 Bytes per DirEntry! *****/
			for(CurEntAddr=0;CurEntAddr<BYTES_PERSEC;CurEntAddr+=DIRENT_SIZE)
			{
				DirTag=(DIR_tag*)((u32)FS_Buffer+CurEntAddr);
	
				if(DirTag->FileName[0]==EMPTY)
				{
					return;
				} 
				
				if(DirTag->FileName[0]!=DELETED)
				{
					if(DirTag->Attribute==LONG_NAME)
					{		   
						if(LngNmCnt!=0)
						{	
							LngNmCnt--;
							LongDirEnt=(LongDir_Ent*)DirTag;
							Name=(u8 *)Name_Buffer+LngNmCnt*26;
							CopyRam(LongDirEnt->Name1,Name,10);
							CopyRam(LongDirEnt->Name2,Name+10,12);
							CopyRam(LongDirEnt->Name3,Name+22,4);
						}
					}
					else if(DirTag->Attribute&(DIR|FILE))
					{
						if(DirTag->Attribute&DIR)
						{
							USART_SendStr(USART1,"Dir:");
						}
						else
						{
							USART_SendStr(USART1,"File:");
						}
						
						if(LngNmCnt!=4)
						{			 
							LngNmCnt=4;	  
							Convert_LngNm(Name);
							USART_SendStr(USART1,Name);
							USART1_printf("\r\n");
						}
						else
						{	
							Name=(u8 *)Name_Buffer;
							CopyRam(DirTag->FileName,Name,11);
							Convert_ShortNm(Name);
							USART_SendStr(USART1,Name);
							USART1_printf("\r\n");
						}
					}
				}
				if(LngNmCnt!=4)
				{
					DirEntCnt--;
				}
				if(LngNmCnt!=DirEntCnt)
				{
					LngNmCnt=4;
					DirEntCnt=4;
				}	
			}
		}
		CurClus=Read_NextClusNum(CurClus);
	}
	while(CurClus!=0x0fffffff);

	return;				
}

/*
函数功能：	打开文件(夹)。根据给出的完整路径(如"/English/Essays of Travel.txt")找到文件并获取其参数。
参数	：	Target	结构体指针，目标文件(夹)。
			FullName	完整路径 文件(夹)名
返回值	：	FILE_EXIST或FILE_NOTEXIST
*/
FS_Status OpenFile(FS_Object *Target,u8 *FullName)	  
{
	u8 	i,h;
	u8 	NameBuf[106];
	h=0;
	Target->FstClus=RootDir.FstClus;
	if(*FullName!='/')
	{
		return NAME_ERROR;
	}
	else
	{
		for(i=1;*(FullName+i)!=0;i++)
		{
			if(*(FullName+i)!='/')
			{
				NameBuf[h]=*(FullName+i);
				h++;
			}
			else
			{
				NameBuf[h]=0;
				if(Search_inDir(Target,Target,NameBuf,DIR)==FILE_NOTEXIST)
				{
					return FILE_NOTEXIST;
				}
				h=0;
			}
		}
		if(h==0)
		{
			return FILE_EXIST;
		}
		else
		{
			NameBuf[h]=0;		
			if(Search_inDir(Target,Target,NameBuf,FILE)==FILE_NOTEXIST)
			{
				return FILE_NOTEXIST;
			}
			else
			{
				return FILE_EXIST;
			}
		}
	}
}

/*
函数功能：	顺序读取整个文件
参数	：	Target_File				目标文件	
返回值	：	u8*
*/
u8* ReadFile(FS_Object* Target_File)
{
	if(Target_File->CurClus==FS_EOF)
	{
		return (void*)0;
	}
	
	if(Target_File->CurSec==FS.Sec_PerClus)
	{
		Target_File->CurSec=0;
		Target_File->CurClus=Read_NextClusNum(Target_File->CurClus);
		if(Target_File->CurClus==FS_EOF)
		{
			return (void*)0;
		}
	}
	ReadSec(Target_File->CurClus,Target_File->CurSec,FS_Buffer);
	Target_File->CurSec++;

	return (u8*)FS_Buffer;
}

void SetFileClustoFst(FS_Object* Target_File)
{
	Target_File->CurClus=Target_File->FstClus;
	Target_File->CurSec=0;
}

/*
函数功能：	向文件中写入数据，从文件末尾追加
参数	：	Target_File	目标文件
			dataBuf		数据源
			dataLength	数据长度
返回值	：	void
*/
void WriteFile(FS_Object* Target_File,u8 *dataBuf,u32 dataLength)
{
	DIR_tag	*DirTag;
	u8 	CurSec;
	u32 CurClus=Target_File->FstClus;
	u32	LstClus;
	u32	CurSec_Addr;
	u32	OfsInClus;
	u16	OfsInSec;
	u32	i=0;
	
	do
	{
		LstClus=CurClus;
		CurClus=Read_NextClusNum(CurClus);
	}
	while(CurClus!=FS_EOF);
	
	CurClus=LstClus;
	
	OfsInClus=Target_File->Size%FS.Bytes_PerClus;
	CurSec=OfsInClus/BYTES_PERSEC;
	OfsInSec=OfsInClus%BYTES_PERSEC;
	
	if(dataLength<(BYTES_PERSEC-OfsInSec))
	{
		CurSec_Addr=Get_ClusAddr(CurClus)+CurSec*BYTES_PERSEC;
		Disk_ReadSec(CurSec_Addr,FS_Buffer);
		
		for(;i<dataLength;i++,OfsInSec++)
		{
			*(u8*)((u32)FS_Buffer+OfsInSec)=*(dataBuf+i);
		}
		
		Disk_WriteSec(CurSec_Addr,FS_Buffer);
	}
	else
	{
		do
		{
			if(CurSec==FS.Sec_PerClus)
			{
				CurClus=Write_NextClusNum(CurClus);
				if(CurClus==FS_EOF)
				{
					break;
				}
				CurSec=0;
			}
			CurSec_Addr=Get_ClusAddr(CurClus)+CurSec*BYTES_PERSEC;
			Disk_ReadSec(CurSec_Addr,FS_Buffer);
			
			for(;(OfsInSec<BYTES_PERSEC)&&(i<dataLength);OfsInSec++,i++)
			{
				*(u8*)((u32)FS_Buffer+OfsInSec)=*(dataBuf+i);
			}
			Disk_WriteSec(CurSec_Addr,FS_Buffer);
			
			OfsInSec=0;
			CurSec++;
		}
		while(i<dataLength);
	}
	
	Target_File->Size+=i;
	CurSec_Addr=Target_File->DirEntryAddr-(Target_File->DirEntryAddr%BYTES_PERSEC);
	Disk_ReadSec(CurSec_Addr,FS_Buffer);
	DirTag=(DIR_tag *)((u32)FS_Buffer+(Target_File->DirEntryAddr-CurSec_Addr));
	DirTag->FileLength=Target_File->Size;
	Disk_WriteSec(CurSec_Addr,FS_Buffer);
}

/*
函数功能：	创建文件(夹)时向目录项中写入短文件(夹)名
参数	：	Source	文件(夹)名地址
			Target	目录项中文件名地址
返回值	：	void
*/
void Write_ShortNm(u8 *Source,u8 *Target)
{
	u8 i=0,h=0;
	
	for(i=0;i<8;i++)
	{
		
		if((*(Source+h)!='.')&&(*(Source+h)!=0))
		{
			*(Target+i)=*(Source+h);
			h++;
		}
		else 
		{
			*(Target+i)=' ';
		}
		
		if((*(Target+i)>='a')&&(*(Target+i)<='z'))
		{
			*(Target+i)-=0x20;
		}
		
	}
	if(*(Source+h)=='.')
	{
		h++;
		for(i=8;i<11;i++)
		{
			if(*(Source+h)!=0)
			{
				*(Target+i)=*(Source+h);
				h++;
			}
			else 
			{
				*(Target+i)=' ';
			}
			
			if((*(Target+i)>='a')&&(*(Target+i)<='z'))
			{
				*(Target+i)-=0x20;
			}
		}
	}
	else
	{
		for(i=8;i<11;i++)
		{
			*(Target+i)=' ';
		}
	}
}

/*
函数功能：	在文件夹中创建新文件(夹)，并获取新创建文件(夹)参数
参数	：	CurDir		当前文件夹
			Target		目标文件(夹)，新文件(夹)创建完成后，参数存于此结构体
			Target_Name	目标文件(夹)名，新文件(夹)以此命名
			Object		FILE或DIR，用于区分将要创建的是文件还是文件夹
返回值	：	NAME_ERROR	文件(夹)名错误，此文件(夹)已存在。
			FILE_FAILED	文件(夹)创建失败，空间不足
			FILE_SUCCESSED	文件(夹)创建成功
*/
FS_Status CreateNewObject(FS_Object* CurDir,FS_Object *Target,u8 *Target_Name,u8 Object)
{
	DIR_tag	*DirTag;
	u32		LastClus;
	u32 	CurClus; 
	u32		NewObject_FstClus;
   	u8		CurSec;
	u16		CurEntAddr;
	u32		TotalClus;
	
	if(Search_inDir(CurDir,Target,Target_Name,Object)==FILE_NOTEXIST)
	{
		TotalClus=(FS.FS_Size-FS.Data_Addr)/FS.Bytes_PerClus;
		CurClus=CurDir->FstClus;
		do
		{
			/***** Read and Scan FS_Object Data Aera by Sector ******/
			for(CurSec=0;CurSec<FS.Sec_PerClus;CurSec++)
			{
				ReadSec(CurClus,CurSec,FS_Buffer);
		
				/***** Scan Every DirEntry , 32 Bytes per DirEntry! *****/
				for(CurEntAddr=0;CurEntAddr<BYTES_PERSEC;CurEntAddr+=DIRENT_SIZE)
				{
					DirTag=(DIR_tag*)((u32)FS_Buffer+CurEntAddr);
					
					if((DirTag->FileName[0]==EMPTY)||(DirTag->FileName[0]==DELETED))
					{
						NewObject_FstClus=Write_NextClusNum(FST_CLUS);
						ReadSec(CurClus,CurSec,FS_Buffer);
						Write_ShortNm(Target_Name,DirTag->FileName);
						DirTag->Attribute=Object;
						DirTag->FstClusHI=(u16)(NewObject_FstClus>>16);
						DirTag->FstClusLO=(u16)NewObject_FstClus;
						DirTag->FileLength=0x00;
						WriteSec(CurClus,CurSec,FS_Buffer);
						
						Target->Name=Target_Name;
						Target->Attrib=Object;
						Target->Size=0;
						Target->FstClus=NewObject_FstClus;
						Target->DirEntryAddr=Get_ClusAddr(CurClus)+CurSec*BYTES_PERSEC+CurEntAddr;
						Target->CurClus=Target->FstClus;
						Target->CurSec=0;
						return FILE_SUCCESSED;
					}
				}
			}
			LastClus=CurClus;
			CurClus=Read_NextClusNum(CurClus);
			
			if(CurClus==FS_EOF)
			{
				CurClus=Write_NextClusNum(LastClus);
				USART1_printf("NextClusNum:0x%x",CurClus);
				if(CurClus==FS_EOF)
				{
					return FILE_FAILED;
				}
			}
		}
		while(CurClus<TotalClus);

		return FILE_FAILED;
	}
	else
	{
		return NAME_ERROR;
	}
}

