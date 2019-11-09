/* 
 * Copyright (c) [2010-2019] zhenfei.mzf@gmail.com rights reserved.
 * 
 * AntiSpy is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *     http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
*/
#include "ParseNTFS.h"
#include "..\\..\\Common\Common.h"
#include "ConnectDriver.h"
#include "CommonMacro.h"
#include "ntfs\ParaseDisk.h"

BOOL bInit = FALSE;

BOOL
read_sector ( 
	OUT PVOID buffer,		// 输出的buffer
	IN ULONG nSectorNum,	// 要读取的扇区号
	IN ULONG nSectorCounts	// 要去读的扇区数
	) 
{
	READ_SECTORS_CONTEXT ctx;
	CConnectDriver Driver;

	if ( buffer == NULL || !nSectorCounts ) 
	{
		return FALSE ;
	}
	
	ctx.OpType = enumReadSector;
	ctx.nSectorCounts = nSectorCounts;
	ctx.nSectorNum = nSectorNum;

	return Driver.CommunicateDriver(&ctx, sizeof(READ_SECTORS_CONTEXT), buffer, nSectorCounts * 512, NULL);
}

BOOL
write_sector ( 
	IN PWrite_Sectors_Context pContext
	)
{
// 	int nReturn = -1 ;
// 
// 	if ( NULL == pContext || NULL == pContext->pBuffer ) {
// 		return FALSE ;
// 	}
// 
// 	if ( NULL == g_drv ) {
// 		return FALSE ;
// 	}
// 
// 	nReturn = g_drv->IoControl(
// 		IOCTL_WRITE_SECTORS,
// 		(PVOID)pContext->pBuffer,	// InBuffer
// 		pContext->uSectorSize,		// InBuffer Length
// 		NULL,						// OutBuffer
// 		pContext->uSectorSize		// OutBuffer Length
// 		);
// 	
// 	if ( -1 == nReturn ) 
// 	{
// 		return FALSE ;
// 	}
// 	
	return TRUE ;
}


#define PARTITION_TYPE_NTFS         0x07
#define PARTITION_TYPE_FAT32         0x0B
#define PARTITION_TYPE_FAT32_LBA     0x0C

int g_nNumber = 0 ;
ULONG g_sectors_per_cluster	= 0; 
PLARGE_INTEGER g_partitionA_sector_pos[10] = { NULL } ;

static ULONG g_nStartSector = 0;

VOID
parase_partion_depth (
					  PMBR_SECTOR mbrsec,
					  ULONG currentSectors,
					  PVOID pTmp
					  )
{
	int MainPartNumber = 0;
	ULONG startlba = 0;
	UCHAR type = 0;
	PPARTITION_ENTRY partition0 = NULL ;
	PBBR_SECTOR bootsec			= NULL ;
	PLARGE_INTEGER result		= NULL ;
	PVOID buffer				= pTmp ;

	if (!mbrsec || !pTmp)
	{
		return;
	}

	for ( MainPartNumber = 0; MainPartNumber < 4; MainPartNumber++ )
	{
		partition0	= &mbrsec->Partition[MainPartNumber]	;
		startlba	= partition0->StartLBA					;
		type		= partition0->PartitionType				;

		// 磁盘总共有4个主分区, 若为0表示该主分区无内容,结束之.
		if ( 0 == startlba || ( 0 != partition0->active && 0x80 != partition0->active ) ) {
			break ;
		}

		startlba += currentSectors ;

		//
		// 校验当前分区的类型:
		//   07H为NTFS分区
		//   0BH为FAT32分区
		//   05H为EXTEND扩展分区(该分区的起始扇区为 "前面的第一个EXTENDX分区起始扇区" + "该分区的相对偏移")
		//   0FH为EXTENDX扩展分区(该分区的起始扇区为 "该分区的相对偏移" 或者 "前面的第一个EXTENDX分区起始扇区" + "该分区的相对偏移")
		//

		memset( buffer, 0, 512 );

		if ( 0x05 == type ) // 是EXTEND扩展分区
		{
			startlba = /*currentSectors + */mbrsec->Partition[1].StartLBA + g_nStartSector
				/*+ mbrsec->Partition[1].TotalSector */;

			read_sector( buffer, startlba, 1 );
			memcpy( (PVOID)mbrsec, buffer, 512 );
			parase_partion_depth( mbrsec, startlba, buffer );

			continue ;
		} 

		else if ( PARTITION_TYPE_NTFS == type ) // 是NTFS分区 ------- OK -------
		{
			read_sector( buffer, startlba, 1 );

			// 分配内存
			result = (PLARGE_INTEGER)malloc( sizeof(LARGE_INTEGER) );
			result->QuadPart = startlba ;

			// 读取本分区的BPB结构,获得保留扇区的数量
			bootsec = (PBBR_SECTOR) buffer ; 
			g_sectors_per_cluster = bootsec->SectorsPerCluster ;

			result->QuadPart += bootsec->ReservedSectors ;
		}

		else if ( 0x0F == type ) // 是EXTENDX扩展分区
		{
			g_nStartSector = startlba;

			read_sector( buffer, startlba, 1 );
			memcpy( (PVOID)mbrsec, buffer, 512 );
			parase_partion_depth( mbrsec, startlba, buffer );
			
			continue ;
		}

		// 若是fat32格式,需要区别对待
		else if( type == PARTITION_TYPE_FAT32 || type == PARTITION_TYPE_FAT32_LBA ) 
		{
			read_sector( buffer, startlba, 1 );
			result->QuadPart += bootsec->NumberOfFATs * bootsec->SectorsPerFAT32 ;
		} 
		else 
		{
			break ;
		}

		// 将读到得物理扇区存入全局变量
		g_partitionA_sector_pos[g_nNumber] = result ;
		g_nNumber++ ;

		DebugLog( L"分区%d在磁盘上的起始扇区: 0x%08lx \n", g_nNumber, result->QuadPart );

		// 越界了. 退出
		if ( g_nNumber > 10 ) 
		{ 
			break ;
		}
	}
}

// 得到磁盘所有分区的物理位置(相对于物理磁盘的扇区号)  
VOID 
get_all_partition_sector () 
{
	PVOID	buffer = NULL, pTmp = NULL ;
	PMBR_SECTOR		mbrsec	= NULL ;

	pTmp = malloc( 0x1000 );
	if (!pTmp)
	{
		return;
	}
	memset( pTmp, 0, 0x1000 );

	buffer = malloc( 512 );
	if (!buffer)
	{
		return;
	}
	memset( buffer, 0, 512 );

	if (read_sector( buffer, 0, 1 ))
	{
		mbrsec = (PMBR_SECTOR) buffer ;
		parase_partion_depth( mbrsec, 0, pTmp );
	}

	free( buffer );
	free( pTmp );

	return ;
}

BOOL
read_sector_ex ( 
				OUT PVOID buffer,		// 输出的buffer
				IN ULONG nOffset,	// 该文件相对于该分区的扇区号
				IN ULONG nSectorCounts,	// 要去读的扇区数
				IN CHAR nDevice
			 ) 
{
	LARGE_INTEGER realdiskpos;
	READ_SECTORS_CONTEXT ctx;
	CConnectDriver Driver;

	if ( buffer == NULL || !nSectorCounts ) 
	{
		return FALSE ;
	}

	if (!bInit)
	{
		get_all_partition_sector ();
		bInit = TRUE;
	}

	int driveId = toupper(nDevice) - 'C';
	realdiskpos.QuadPart = g_partitionA_sector_pos[driveId]->QuadPart + nOffset ;

	ctx.OpType = enumReadSector;
	ctx.nSectorCounts = nSectorCounts;
	ctx.nSectorNum = realdiskpos.LowPart;

	return Driver.CommunicateDriver(&ctx, sizeof(READ_SECTORS_CONTEXT), buffer, nSectorCounts * 512, NULL);
}