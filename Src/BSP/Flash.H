#ifndef __FLASH_H
#define __FLASH_H

#include "MFCC.H"

#define FLASH_PAGE_SIZE			2048

#define Flash_Fail		3
#define Flash_Success	0

#define save_mask		12345

#define size_per_ftr	(4*1024)
#define page_per_ftr	(size_per_ftr/FLASH_PAGE_SIZE)
#define ftr_per_comm	4
#define size_per_comm	(ftr_per_comm*size_per_ftr)
#define comm_num		20
#define ftr_total_size	(size_per_comm*comm_num)
#define ftr_end_addr	0x8080000
#define ftr_start_addr	(ftr_end_addr-ftr_total_size)

u8 save_ftr_mdl(v_ftr_tag* ftr, u32 addr);

#endif

