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
#ifndef _COMMON_FUNCTION_H
#define _COMMON_FUNCTION_H

#include <ntifs.h>
#include <windef.h>
#include "InitWindows.h"
#include "Struct.h"

#define		INVALID_PAGE	0
#define		VALID_PAGE		1
#define		PDEINVALID		2
#define		PTEINVALID		3

#define Kmalloc(_s)	ExAllocatePool(NonPagedPool, _s)
#define Kfree(_p)	if( _p ) { ExFreePool(_p); }

typedef enum _ENTRY_POINT_TYPE
{
	enumEntryNone,
	enumEntryPointHead,
	enumEntryPointJmp,
}ENTRY_POINT_TYPE;

PVOID GetFunctionAddressByName(WCHAR *szFunction);
ULONG IsPageValid(ULONG uAddr);
BOOL ValidateUnicodeString(PUNICODE_STRING usStr);
POBJECT_TYPE KeGetObjectType();
BOOL IsPe(PVOID FileBuffer);
BOOL GetPathByFileObject(PFILE_OBJECT pFileObject, WCHAR* szPath);
BOOL IsRealProcess(PEPROCESS pEprocess);
BOOL IsProcessDie(PEPROCESS pEprocess);
PVOID FindExportedRoutineByName ( IN PVOID DllBase, IN CHAR* RoutineName );

__inline WPOFF()
{
	__asm 
	{ 
		cli 
		mov    eax, cr0 
		and    eax, not 0x10000 
		mov    cr0, eax 
	} 
}

__inline WPON()
{
	__asm 
	{ 
		mov    eax, cr0 
		or    eax, 0x10000 
		mov    cr0, eax 
		sti 
	} 
}

__inline CHAR ChangePreMode(PETHREAD pThread)
{
	ULONG PreModeOffset = GetGlobalVeriable(enumPreviousModeOffset_KTHREAD);
	CHAR PreMode = *(PCHAR)((ULONG)pThread + PreModeOffset);
	*(PCHAR)((ULONG)pThread + PreModeOffset) = KernelMode;
	return PreMode;
}

__inline VOID RecoverPreMode(PETHREAD pThread, CHAR PreMode)
{
	ULONG PreModeOffset = GetGlobalVeriable(enumPreviousModeOffset_KTHREAD);
	*(PCHAR)((ULONG)pThread + PreModeOffset) = PreMode;
}	

PVOID MzfGetSystemRoutineAddress ( CHAR* SystemRoutineName );
VOID GetWaitListHeadAndDispatcherReadyListHead();
VOID GetPspCidTable();
VOID MzfInitUnicodeString(IN OUT PUNICODE_STRING DestinationString, IN PCWSTR SourceString);
PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader( PVOID Base );
PVOID MzfImageDirectoryEntryToData ( IN PVOID Base, IN BOOLEAN MappedAsImage, IN USHORT DirectoryEntry, OUT PULONG Size );
BOOL IsUnicodeStringValid(PUNICODE_STRING unString);
ULONG CmpAndGetStringLength(PUNICODE_STRING unString, ULONG nLen);
BOOL IsRealThread(PETHREAD pThread);
BOOL IsRealDriverObject(PDRIVER_OBJECT DriverObject);
HANDLE MapFileAsSection(PUNICODE_STRING FileName, PVOID *ModuleBase);
NTSTATUS MzfCloseHandle(HANDLE hHandle);
ULONG GetInlineHookAddress(ULONG pAddress, ULONG nLen);
LONG ExchangeAddress(PULONG pTargetAddress, ULONG Value);
void SofeCopyMemory(ULONG pDesAddress, PVOID pSrcAddress, ULONG nLen);
BOOL FindIatFunction( IN PVOID pBase, IN ULONG ulModuleSize, IN PCHAR ImportDllName, IN PCHAR ImportApiName, OUT PULONG ImportFunOffset);
NTSTATUS GetDriverObjectByName(WCHAR *szDriverName, PDRIVER_OBJECT *pDriverObject);
BOOL GetDriverEntryPoint(PVOID ImageBase, DWORD *pOutDriverEntry, ENTRY_POINT_TYPE EntryType);
void GetIopInvalidDeviceRequest();
void FixReloadDispatch(PULONG pReloadDispatchs);
void FixOriginDispatch(PULONG pOriginDispatchs);
NTSTATUS GetInlineAddress(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);
PIMAGE_SECTION_HEADER GetSecionAddress(PIMAGE_NT_HEADERS NtHeaders, CHAR *szSectionName);
PVOID LookupKernelModuleByNameW(WCHAR* szModuleName, DWORD* nModuleSize);
PVOID LookupKernelModuleByName(IN CHAR* szModuleName, OUT DWORD* nModuleSize);
PVOID MyQuerySystemInformation(SYSTEM_INFORMATION_CLASS nClass);
void MemCpy(PVOID pDes, PVOID pSrc, ULONG nLen);
BOOL IsAddressValid(PVOID pAddress, ULONG nLen);
NTSTATUS SafeCopyMemory_R0_to_R3(PVOID SrcAddr, PVOID DstAddr, ULONG Size);
NTSTATUS SafeCopyMemory_R3_to_R0(ULONG SrcAddr, ULONG DstAddr, ULONG Size);
NTSTATUS SafeCopyProcessModules(PEPROCESS pEprocess, ULONG nBase, ULONG nSize, PVOID pOutBuffer);
#endif