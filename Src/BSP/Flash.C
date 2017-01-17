/*	适用于STM32F103VE	
	全部falsh 512KB 256页 每页2KB
	
	每个语音特征模板占用4KB 采用冗余模板 每个语音指令4个特征模板
	初步设计设定20个语音指令 共占用320KB
	
	flash最后320KB用于存储语音特征模板
	编译器需设置 以免存储区被代码占用
	烧写程序时也不能擦除存储区 选擦除需要的页
*/
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "Flash.H"
#include "USART.h"
#include "MFCC.H"

u8 save_ftr_mdl(v_ftr_tag* ftr, u32 addr)
{
	u32 i;
	u32 ftr_size;
	
	if(((addr%FLASH_PAGE_SIZE)!=0)||(addr<ftr_start_addr)||(addr>(ftr_end_addr-size_per_ftr)))
	{
		USART1_printf("flash addr error");
		return Flash_Fail;
	}
	ftr_size=2*mfcc_num*ftr->frm_num;
	
	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	for(i=0;i<page_per_ftr;i++)
	{
		if(FLASH_ErasePage(addr+FLASH_PAGE_SIZE*i)!=FLASH_COMPLETE)
		{
			USART1_printf("flash Erase Error! ");
			return Flash_Fail;
		}
	}
	
	//保存数据有效标记
	if(FLASH_ProgramHalfWord(addr, save_mask)!=FLASH_COMPLETE)
	{
		USART1_printf("flash Program Error! ");
		return Flash_Fail;
	}
	addr+=2;
	//保存特征模板帧长度
	if(FLASH_ProgramHalfWord(addr,*(u16 *)(&(ftr->frm_num)))!=FLASH_COMPLETE)
	{
		USART1_printf("flash Program Error! ");
		return Flash_Fail;
	}
	addr+=2;
	//保存特征模板MFCC数据
	for(i=0;i<ftr_size;i+=2)
	{
		if(FLASH_ProgramHalfWord(addr+i,*(u16 *)((u32)(ftr->mfcc_dat)+i))!=FLASH_COMPLETE)
		{
			USART1_printf("flash Program Error! ");
			return Flash_Fail;
		}
	}
	
	FLASH_LockBank1();
	return Flash_Success;
}
