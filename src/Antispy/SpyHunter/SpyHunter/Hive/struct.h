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
#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////

#ifndef NTSTATUS
typedef LONG NTSTATUS, *PNTSTATUS;
#endif

#if !defined(STATUS_SUCCESS)
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif /* !STATUS_SUCCESS */

#define NT_SUCCESS(x) ((x)>=0)

// #define dprintf if (DBG) printf

#if DBG
#define dprintf printf
#else
#define dprintf 
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;


//////////////////////////////////////////////////////////////////////////


typedef struct _IO_STATUS_BLOCK {
  union {
    NTSTATUS Status;
    PVOID Pointer;
  } DUMMYUNIONNAME;
  
  ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

//
// File Information Classes for NtQueryInformationFile
//
typedef enum _FILE_INFORMATION_CLASS
{
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,
    FileBothDirectoryInformation,
    FileBasicInformation,
    FileStandardInformation,
    FileInternalInformation,
    FileEaInformation,
    FileAccessInformation,
    FileNameInformation,
    FileRenameInformation,
    FileLinkInformation,
    FileNamesInformation,
    FileDispositionInformation,
    FilePositionInformation,
    FileFullEaInformation,
    FileModeInformation,
    FileAlignmentInformation,
    FileAllInformation,
    FileAllocationInformation,
    FileEndOfFileInformation,
    FileAlternateNameInformation,
    FileStreamInformation,
    FilePipeInformation,
    FilePipeLocalInformation,
    FilePipeRemoteInformation,
    FileMailslotQueryInformation,
    FileMailslotSetInformation,
    FileCompressionInformation,
    FileObjectIdInformation,
    FileCompletionInformation,
    FileMoveClusterInformation,
    FileQuotaInformation,
    FileReparsePointInformation,
    FileNetworkOpenInformation,
    FileAttributeTagInformation,
    FileTrackingInformation,
    FileIdBothDirectoryInformation,
    FileIdFullDirectoryInformation,
    FileValidDataLengthInformation,
    FileShortNameInformation,
    FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;


typedef enum _OBJECT_INFORMATION_CLASS
{
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectTypesInformation,
    ObjectHandleFlagInformation,
    ObjectSessionInformation,
    MaxObjectInfoClass
} OBJECT_INFORMATION_CLASS;

typedef struct _FILE_NAME_INFORMATION
{
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;


typedef struct _SYSTEM_HANDLE_INFORMATION
{
    ULONG            ProcessId;
    UCHAR            ObjectTypeNumber;
    UCHAR            Flags;
    USHORT            Handle;
    PVOID            Object;
    ACCESS_MASK        GrantedAccess;
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION ;

//////////////////////////////////////////////////////////////////////////