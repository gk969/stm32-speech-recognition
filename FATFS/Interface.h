#ifndef __interface_h
#define __interface_h

void Disk_ReadSec(u32 Addr, u32 *DataBuf);
void Disk_WriteSec(u32 Addr, u32 *DataBuf);

u16 UnicodeToGb2312(u16 Unicode);

#endif
