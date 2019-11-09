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
#ifndef _PARSE_NTFS_H_
#define _PARSE_NTFS_H_

#include <stdlib.h>
#include <stdio.h>
#include <wtypes.h>
#include <windows.h>

typedef struct _Write_Sectors_Context_ {
	//	ULONG64 lcn ; 
	ULONG64 fileOffset ;
	int uSectorSize ;
	PVOID pBuffer ;
} Write_Sectors_Context, *PWrite_Sectors_Context ;

#pragma pack(1)
typedef struct _PARTITION_ENTRY
{
	UCHAR active;                
	UCHAR StartHead;              
	UCHAR StartSector;             
	UCHAR StartCylinder;          
	UCHAR PartitionType;          
	UCHAR EndHead;                
	UCHAR EndSector;              
	UCHAR EndCylinder;           
	ULONG StartLBA;              
	ULONG TotalSector;            
} PARTITION_ENTRY, *PPARTITION_ENTRY;

typedef struct _MBR_SECTOR
{
	UCHAR             BootCode[446];
	PARTITION_ENTRY   Partition[4];
	USHORT           Signature;
} MBR_SECTOR, *PMBR_SECTOR;


typedef struct _BBR_SECTOR
{
	USHORT JmpCode;				// 0x000     
	UCHAR   NopCode;			// 0x002     
	UCHAR   OEMName[8];			// 0x003    
	USHORT BytesPerSector;		// 0x00B    
	UCHAR   SectorsPerCluster;  // 0x00D 
	USHORT ReservedSectors;		// 0x00E	
	UCHAR   NumberOfFATs;       // 0x010
	USHORT RootEntries;       
	USHORT NumberOfSectors16;    
	UCHAR   MediaDescriptor;     
	USHORT SectorsPerFAT16;     
	USHORT SectorsPerTrack;     
	USHORT HeadsPerCylinder;     
	ULONG   HiddenSectors;      
	ULONG   NumberOfSectors32;    
	ULONG   SectorsPerFAT32;      
} BBR_SECTOR, *PBBR_SECTOR;
#pragma pack()


#ifdef __cplusplus
extern "C" {
#endif

BOOL read_sector_ex ( OUT PVOID buffer, IN ULONG nOffset, IN ULONG nSectorCounts, IN CHAR nDevice );

#ifdef __cplusplus
}
#endif

#endif