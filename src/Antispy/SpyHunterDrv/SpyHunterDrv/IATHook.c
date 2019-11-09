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
#include "iathook.h"
#include "CommonFunction.h"
#include "InitWindows.h"

pfnKeUserModeCallback oldKeUserModeCallback = NULL;

BOOL IsLoadLibraryCallback(ULONG ApiNumber)
{
	BOOL bRet = FALSE;
	ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);

	if ( 2195 == nBuildNumber )
	{
		bRet = ApiNumber == 64;
	}
	else
	{
		if ( nBuildNumber >= 7600 )
		{
			if ( nBuildNumber >= 9200 )
			{
				bRet = ApiNumber == 67;
			}
			else
			{
				bRet = ApiNumber == 65;
			}
		}
		else
		{
			bRet = ApiNumber == 66;
		}
	}

	return bRet;
}

NTSTATUS 
filter_KeUserModeCallback(
						IN ULONG ApiNumber,
						IN PVOID InputBuffer,
						IN ULONG InputLength,
						OUT ULONG OutputBuffer,
						IN PULONG OutputLength
						)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	if (IsLoadLibraryCallback(ApiNumber))
	{
		KdPrint(("is load library\n"));
		ntStatus = STATUS_ACCESS_DENIED;
	}

	return ntStatus;
}

__declspec(naked) NTSTATUS Fake_KeUserModeCallback(
	IN ULONG ApiNumber,
	IN PVOID InputBuffer,
	IN ULONG InputLength,
	OUT ULONG OutputBuffer,
	IN PULONG OutputLength
	)
{
	__asm
	{
		//jmp oldKeUserModeCallback;
		push ebp;
		mov ebp,esp;
		sub esp,0x100;
		pushad;
		pushfd;
		push OutputLength;
		push OutputBuffer;
		push InputLength;
		push InputBuffer;
		push ApiNumber;
		call filter_KeUserModeCallback;
		cmp eax, 0xC0000022;
		jz _DENIED_RETURN;
		popfd;
		popad;
		MOV ESP, EBP;
		POP EBP;
		jmp oldKeUserModeCallback;
_DENIED_RETURN:
		popfd;
		popad;
		mov eax,0xC0000001;
		MOV ESP, EBP;
		POP EBP;
		ret 0x14;
	}
}

BOOL 
GetIATAddress(
		IN PVOID pBase,
		IN ULONG ulModuleSize,
		IN PCHAR ImportDllName,
		IN PCHAR ImportApiName,
		OUT PULONG originalFuncAddr,
		OUT PVOID* originalIATAddr
		)
{
	BOOL bRet = FALSE;
	pfnRtlImageDirectoryEntryToData RtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);

	if (pBase && 
		ulModuleSize &&
		ImportDllName && 
		ImportApiName && 
		originalFuncAddr && 
		originalIATAddr && 
		RtlImageDirectoryEntryToData &&
		KeGetCurrentIrql() ==  PASSIVE_LEVEL)
	{
		__try
		{
			ULONG *OriginalFirstThunk = NULL; 
			ULONG *FirstThunk = NULL; 
			ULONG ulRet = 0;
			PIMAGE_IMPORT_DESCRIPTOR pImportModuleDirectory = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(pBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulRet);
			if (pImportModuleDirectory)
			{
				size_t nDllNameLen = strlen(ImportDllName);
				size_t nAPINameLen = strlen(ImportApiName);
				
				KdPrint(("pImportModuleDirectory: 0x%08X\n", pImportModuleDirectory));

				while (pImportModuleDirectory && pImportModuleDirectory->Name)
				{
					PCHAR strModuleName = (PCHAR)((PBYTE)pBase + pImportModuleDirectory->Name);
					KdPrint(("ModuleName:%s\n", strModuleName));

					if (strlen(strModuleName) == nDllNameLen && 
						!_strnicmp(strModuleName, ImportDllName, nDllNameLen))
					{
						PIMAGE_IMPORT_BY_NAME pImageImportByName = NULL;
						int i = 0;

						OriginalFirstThunk    = (ULONG *)((CHAR *)pBase + pImportModuleDirectory->OriginalFirstThunk);
						FirstThunk            = (ULONG *)((CHAR *)pBase + pImportModuleDirectory->FirstThunk);

						for (i = 0; FirstThunk && FirstThunk[i] && OriginalFirstThunk && OriginalFirstThunk[i]; i++)
						{
							if ( (PVOID)OriginalFirstThunk[i] < (PVOID)pBase )
							{
								pImageImportByName =  (PIMAGE_IMPORT_BY_NAME)(OriginalFirstThunk[i] + (PBYTE)pBase);
							}
							else
							{
								pImageImportByName = (PIMAGE_IMPORT_BY_NAME)OriginalFirstThunk[i];
							}
							
							if (pImageImportByName && (PBYTE)pImageImportByName > (PBYTE)pBase && (PBYTE)pImageImportByName < (PBYTE)((PBYTE)pBase + ulModuleSize))
							{
								if ( !strncmp((PCHAR)&pImageImportByName->Name, ImportApiName, nAPINameLen) )
								{     										
									KdPrint(("Find %s IAT Addr:%p Func Addr:%p", ImportApiName, &FirstThunk[i], FirstThunk[i]));

									if (MmIsAddressValid((PVOID)FirstThunk[i]))
									{
										*originalFuncAddr = (ULONG)FirstThunk[i];
										*originalIATAddr  = &FirstThunk[i];
										bRet = TRUE;
									} 

									break;   
								}                                             
							}                              
						}

						break;
					}

					pImportModuleDirectory++;
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}
	}

	return bRet;
}

PVOID WritePointer(PVOID oldPointer, PVOID newAddr)
{ 
	PVOID oldAddr = NULL;

	if (oldPointer && newAddr)
	{
		if ( MmIsAddressValid(oldPointer) && MmIsAddressValid(newAddr) )
		{	
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
			oldAddr = InterlockedExchangePointer(oldPointer, newAddr);
			WPON();
			KeLowerIrql(OldIrql);
		} 
	}

	return oldAddr;
}

BOOL SetIATHook()
{
	BOOL bRet = FALSE, bIATHook = FALSE;
	ULONG nWin32kBase = GetGlobalVeriable(enumOriginWin32kBase);
	ULONG nWin32Size = GetGlobalVeriable(enumWin32kSize);

	char szNtoskrnl[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
	char szKeUserModeCallback[] = {'K','e','U','s','e','r','M','o','d','e','C','a','l','l','b','a','c','k','\0'};
	PVOID pIATPointer_KeUserModeCallback = NULL;

	bRet = GetIATAddress((PVOID)nWin32kBase, nWin32Size, szNtoskrnl, szKeUserModeCallback, (PULONG)&oldKeUserModeCallback, &pIATPointer_KeUserModeCallback);  
	if (bRet)
	{
		if (WritePointer(pIATPointer_KeUserModeCallback, (PVOID)Fake_KeUserModeCallback))
		{
			KdPrint(("oldKeUserModeCallback: %p\n", oldKeUserModeCallback));
			KdPrint(("g_IATPointer_KeUserModeCallback: %p\n", pIATPointer_KeUserModeCallback));

			SetGlobalVeriable(enumKeUserModeCallback, (ULONG)oldKeUserModeCallback);
			SetGlobalVeriable(enumKeUserModeCallbackIatAddress, (ULONG)pIATPointer_KeUserModeCallback);

			bIATHook = TRUE;
		}
	}

	return bIATHook;  
}

VOID  RestoreIATHook()
{
	PVOID oldKeUserModeCallback = (PVOID)GetGlobalVeriable(enumKeUserModeCallback);
	PVOID IATPointer_KeUserModeCallback = (PVOID)GetGlobalVeriable(enumKeUserModeCallbackIatAddress);

	if (oldKeUserModeCallback && 
		IATPointer_KeUserModeCallback &&
		MmIsAddressValid(oldKeUserModeCallback) &&
		MmIsAddressValid(IATPointer_KeUserModeCallback))
	{
		WritePointer(IATPointer_KeUserModeCallback, (PVOID)oldKeUserModeCallback);
	}
}