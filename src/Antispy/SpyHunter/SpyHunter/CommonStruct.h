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
#ifndef _COMMON_STRUCT_H
#define _COMMON_STRUCT_H

/*#include <winternl.h>*/

// typedef struct _LDR_DATA_TABLE_ENTRY
// {     
// 	LIST_ENTRY     InLoadOrderLinks;
// 	LIST_ENTRY     InMemoryOrderLinks;
// 	LIST_ENTRY     InInitializationOrderLinks;
// 	PVOID          DllBase;
// 	PVOID          EntryPoint;
// 	ULONG          SizeOfImage;
// 	UNICODE_STRING FullDllName;
// 	UNICODE_STRING BaseDllName;
// 	ULONG          Flags;
// 	USHORT         LoadCount;
// 	USHORT         TlsIndex;
// 	union
// 	{
// 		LIST_ENTRY HashLinks;
// 		struct
// 		{
// 			PVOID SectionPointer;
// 			ULONG CheckSum;
// 		};
// 	};
// 	union
// 	{
// 		ULONG TimeDateStamp;
// 		PVOID LoadedImports;
// 	};
// 	PVOID          EntryPointActivationContext;
// 	PVOID          PatchInformation;
// } LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
// 
// 
// #define OBJ_INHERIT             0x00000002L
// #define OBJ_PERMANENT           0x00000010L
// #define OBJ_EXCLUSIVE           0x00000020L
// #define OBJ_CASE_INSENSITIVE    0x00000040L
// #define OBJ_OPENIF              0x00000080L
// #define OBJ_OPENLINK            0x00000100L
// #define OBJ_KERNEL_HANDLE       0x00000200L
// #define OBJ_FORCE_ACCESS_CHECK  0x00000400L
// #define OBJ_VALID_ATTRIBUTES    0x000007F2L
// 
// #define InitializeObjectAttributes( p, n, a, r, s ) {		\
// 	(p)->Length = sizeof( MZF_OBJECT_ATTRIBUTES );      \
// 	(p)->RootDirectory = r;                             \
// 	(p)->Attributes = a;                                \
// 	(p)->ObjectName = n;                                \
// 	(p)->SecurityDescriptor = s;                        \
// 	(p)->SecurityQualityOfService = NULL;               \
// }
// 
// typedef enum _KEY_VALUE_INFORMATION_CLASS {
// 	KeyValueBasicInformation,
// 	KeyValueFullInformation,
// 	KeyValuePartialInformation,
// 	KeyValueFullInformationAlign64,
// 	KeyValuePartialInformationAlign64,
// 	MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
// } KEY_VALUE_INFORMATION_CLASS;
// 
// typedef struct _KEY_VALUE_FULL_INFORMATION {
// 	ULONG   TitleIndex;
// 	ULONG   Type;
// 	ULONG   DataOffset;
// 	ULONG   DataLength;
// 	ULONG   NameLength;
// 	WCHAR   Name[1];            // Variable size
// 	//          Data[1];            // Variable size data not declared
// } KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;
// 
// typedef struct _KEY_BASIC_INFORMATION {
// 	LARGE_INTEGER LastWriteTime;
// 	ULONG   TitleIndex;
// 	ULONG   NameLength;
// 	WCHAR   Name[1];            // Variable length string
// } KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

#endif