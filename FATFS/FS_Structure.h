
/**** 指定Align值为1byte,防止编译对齐优化 *******/
#pragma pack(1)
typedef struct { 
	vu8 	bJmpBoot[3];    	//ofs:0.典型的如：0xEB,0x3E,0x90。 
	vu8  	bOEMName[8];    	//ofs:3.典型的如： “MSWIN4.1”。 
	vu16	BPB_BytesPerSec;  	//ofs:11.每扇区字节数。
	vu8 	BPB_SecPerClus;   	//ofs:13.每簇扇区数。 
	vu16  	BPB_ReservedSec;  	//ofs:14.保留扇区数，从 DBR到 FAT的扇区数。 
	vu8 	BPB_NumFATs;   		//ofs:16.FAT的个数。 
	vu16  	BPB_RootEntCnt;   	//ofs:17.FAT16 根目录项数。 
	vu16  	BPB_TotalSec;    	//ofs:19.分区总扇区数(<32M时用)。
	vu8 	BPB_Media;    		//ofs:21.分区介质标识,优盘一般用 0xF8。 
	vu16  	BPB_FATSz16;   		//ofs:22.FAT16中每个 FAT占的扇区数。FAT32中为0 
	vu16  	BPB_SecPerTrk;   	//ofs:24.每道扇区数。 
	vu16  	BPB_NumHeads;    	//ofs:26.磁头数。 
	vu32 	BPB_HiddSec;   		//ofs:28.隐藏扇区数，从 MBR到 DBR的扇区数。 
	vu32 	BPB_TotalSec32;  	//ofs:32.分区总扇区数(>=32M时用)。 
	vu32 	BPB_FATSz32;     	//ofs:36.FAT32中每个FAT占的扇区数。FAT16中为0 
	vu16 	BPB_ExtFlags32;    	//ofs:40. 
	vu16 	BPB_FSVer32;    	//ofs:42.FAT32版本号 
	vu32 	BPB_RootClus;    	//ofs:44.FAT32中根目录所在第一簇簇号 
	vu16  	BPB_FSInfo;    		//ofs:48.FAT32保留区中FSInfo所占扇区数  ”。
	vu32 	BPB_BK32;			//ofs:50
	vu8  	FileSysType[8];   	//ofs:54.“FAT32   ”。 
	vu8 	ExecutableCode[448];//ofs:62.引导代码。 
	vu16  	EndingFlag;    		//ofs:510.结束标识:0xAA55。 
}FAT32_BPB;

typedef struct { 
	vu32	FSI_LeadSig;		//ofs:0. 	FSI标志 0x41615252
	vu8		FSI_Reserved1[480];	//ofs:4. 	此域保留 全零
	vu32	FSI_Strucsig;		//ofs:484	FSI标志 0x61417272
	vu32	FSI_Free_Count;		//ofs:488	保存最新的剩余簇数量
								// 			若为0xffffffff,则需重新计算
	vu32	FSI_NxtFree;		//ofs:492	保存下一剩余簇簇号
								//			若为0xffffffff,则需重新计算
	vu8		FSI_Reserved2[12];	//ofs:496	保留
	vu32	FSI_Trailsig;		//ofs:508	FSI结束标志 0xAA550000
}FAT32_FSinfo;
 
typedef struct{ 
	u8	FileName[8]; 	//ofs:0.文件名 OEM字符
	u8  ExtName[3]; 	//ofs:8.扩展名 
	u8 	Attribute; 		//ofs:11.文件属性。典型值：存档(0x20)、卷标(0x08)。 
	u8  NT_Res; 		//ofs:12.保留 
	u8	CtrTimeTeenth;  //ofs:13.创建时间（毫秒） 
	u16 CtrTime;   		//ofs:14.创建时间 
	u16 CtrDate; 		//ofs:16.创建日期 
	u16	LastAccDate;	//ofs:18.最后访问时间
	u16	FstClusHI;		//ofs:20.文件开始簇号高位
	u16	WrtTime;		//ofs:22.最后写时间
	u16	WrtDate;		//ofs:24.最后写日期
	u16	FstClusLO;		//ofs:26.文件开始簇号低位
	u32 FileLength; 	//ofs:28.文件长度 
}DIR_tag;

typedef	struct{
	u8	Ord;		//ofs:0.该长目录项在本组中的序号
	u8	Name1[10];	//ofs:1.长文件名子项第1~5字符 Unicode字符
	u8	Attr;		//ofs:11.属性 必须为Long_Name 0x0F
	u8	Type;		//ofs:12.一般为零
	u8	ChkSum;		//ofs:13.短文件名校验和
	u8	Name2[12];	//ofs:14.长文件名子项第6~11字符 Unicode字符	
	u16	FstClusLO;	//ofs:26.此处无意义	必须为零 
	u8	Name3[4];	//ofs:28.长文件名子项第12~13字符 Unicode字符
}LongDir_Ent;	
#pragma pack()
