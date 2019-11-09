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
/*
 * types.h - Misc type definitions not related to on-disk structure.  
 *           Originated from the Linux-NTFS project.
 */

#ifndef _NTFS_TYPES_H
#define _NTFS_TYPES_H

#include <windows.h>


typedef unsigned char  u8;			/* Unsigned types of an exact size */
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned __int64 u64;

typedef signed char  s8;			/* Signed types of an exact size */
typedef signed short s16;
typedef signed int s32;
typedef signed __int64 s64;

typedef u16 le16;
typedef u32 le32;
typedef u64 le64;

/*
 * Declare sle{16,32,64} to be unsigned because we do not want sign extension
 * on BE architectures.
 */
typedef u16 sle16;
typedef u32 sle32;
typedef u64 sle64;

typedef u16 ntfschar;			/* 2-byte Unicode character type. */
#define UCHAR_T_SIZE_BITS 1

/*
 * Clusters are signed 64-bit values on NTFS volumes.  We define two types, LCN
 * and VCN, to allow for type checking and better code readability.
 */
typedef s64 VCN;
typedef sle64 leVCN;
typedef s64 LCN;
typedef sle64 leLCN;

/*
 * The NTFS journal $LogFile uses log sequence numbers which are signed 64-bit
 * values.  We define our own type LSN, to allow for type checking and better
 * code readability.
 */
typedef s64 LSN;
typedef sle64 leLSN;

/*
 * Cygwin has a collision between our BOOL and <windef.h>'s
 * As long as this file will be included after <windows.h> were fine.
 */



/**
 * enum IGNORE_CASE_BOOL -
 */
typedef enum {
	CASE_SENSITIVE = 0,
	IGNORE_CASE = 1,
} IGNORE_CASE_BOOL;

#define STATUS_OK				(0)
#define STATUS_ERROR				(-1)
#define STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT	(-2)
#define STATUS_KEEP_SEARCHING			(-3)
#define STATUS_NOT_FOUND			(-4)



typedef struct {
	BOOL bUse_SCSI ;
	BOOL bDriverLoaded ;
} SCSI_DRIVER, *PSCSI_DRIVER ;

SCSI_DRIVER g_scsi_driver ;

#endif /* defined _NTFS_TYPES_H */

