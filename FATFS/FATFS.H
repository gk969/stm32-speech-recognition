
#ifndef __FATFS_H
#define __FATFS_H

typedef struct {
u32		FS_Size;
u32		FAT_Addr;
u32		Data_Addr;
u32		Sec_PerFAT;
u8		Sec_PerClus;
u32		Bytes_PerClus;	 
u32		Root_Clus;	 
}FATFS;

typedef struct{
	u8*	Name;			//文件名
	u8	Attrib;		 	//文件属性
	u16 CurSec;			//当前扇区
	u32 CurClus;		//当前簇
	u32	Size;		 	//文件大小
	u32	FstClus;		//文件起始簇
	u32	DirEntryAddr;	//文件目录项地址
}FS_Object;

typedef	u8 	FS_Status;

#define	FAT_NUM			2
#define	BYTES_PERSEC	512

#define	FILE_NOTEXIST	0X00
#define	FILE_EXIST		0X01
#define NAME_ERROR		0X02
#define	FILE_SUCCESSED	0X03
#define	FILE_FAILED		0X04

#define	FST_CLUS	0x00

#define	EMPTY		0X00
#define	DELETED		0XE5

#define	READ_ONLY	0X01
#define	HIDDEN		0X02
#define	SYSTEM		0X04
#define	VOLUME_ID	0X08
#define	DIR			0X10
#define	ARCHIVE		0X20
#define	LONG_NAME	0X0F
#define	FILE		ARCHIVE

#define	LAST_LONGENTRY	0X40

#define	FSINFO_ADDR	512
#define	BPB_ADDR	0
#define FS_EOF		0X0FFFFFFF
#define BAD_CLUS	0X0FFFFFF7
#define DIRENT_SIZE	0X20

#define SUCCESSED	1
#define	FAILED		0

void FATFS_Init(void); 
void FS_Test(void);

FS_Status 	Search_inDir(FS_Object *CurDir,FS_Object *Target,u8 *Target_Name,u8 Object);	
u8* 		ReadFile(FS_Object* Target_File);
void 		SetFileClustoFst(FS_Object* Target_File);
FS_Status 	OpenFile(FS_Object *Target,u8 *FullName);
void 		LsDir(FS_Object* CurDir);

FS_Status 	CreateNewObject(FS_Object* CurDir,FS_Object *Target,u8 *Target_Name,u8 Object);
void 		WriteFile(FS_Object* Target_File,u8 *dataBuf,u32 dataLength);

#endif

