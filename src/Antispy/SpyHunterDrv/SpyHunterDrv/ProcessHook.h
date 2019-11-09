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
#include "ProcessHook.h"
#include "InitWindows.h"
#include "Process.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"

ULONG EnumProcessIatHook(
						 ULONG OriginBase, 
						 ULONG ReloadBase, 
						 ULONG Size, 
						 PIAT_EAT_HOOK_INFO pIatEatHookInfo, 
						 ULONG nCnt
						 )
{
// 	PIMAGE_IMPORT_DESCRIPTOR ImageImportDescriptor = NULL;
// 	DWORD ImportSize = 0;
// 	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
// 	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
// 	ULONG iCnt = 0;
// 	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
// 	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
// 	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
// 	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
// 	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
// 	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
// 	ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);
// 
// 	if (!OriginBase					|| 
// 		!ReloadBase					|| 
// 		!Size						|| 
// 		!pIatEatHookInfo			|| 
// 		!nCnt						|| 
// 		!MzfMmIsAddressValid		||
// 		!MzfRtlInitAnsiString		|| 
// 		!MzfRtlFreeUnicodeString	||
// 		!MzfRtlAnsiStringToUnicodeString ||
// 		!MzfMmIsAddressValid((PVOID)OriginBase) ||
// 		!MzfMmIsAddressValid((PVOID)ReloadBase) )
// 	{
// 		return 0;
// 	}
// 
// 	ImageDosHeader = (PIMAGE_DOS_HEADER)ReloadBase;
// 	if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
// 	{
// 		return 0;
// 	}
// 
// 	ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)ReloadBase + ImageDosHeader->e_lfanew);
// 	if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
// 	{
// 		return 0;
// 	}
// 
// 	ImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)MzfImageDirectoryEntryToData((PVOID)ReloadBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportSize);
// 	if (ImageImportDescriptor == NULL)
// 	{
// 		return 0;
// 	}
// 
// 	KdPrint(("ImageImportDescriptor: 0x%08X, ImportSize: 0x%08X\n", (ULONG)ImageImportDescriptor - ReloadBase, ImportSize));
// 
// 	while (ImageImportDescriptor->OriginalFirstThunk && ImageImportDescriptor->Name)
// 	{
// 		CHAR *ModuleName = (CHAR*)(ReloadBase + ImageImportDescriptor->Name);
// 		ULONG WithoutFixModuleBase = 0, OriginModuleBase = 0;
// 		BOOL bFree = FALSE;
// 
// 		if ( (OriginModuleBase = GetOriginModuleBase(ModuleName)) != 0 &&
// 			(WithoutFixModuleBase = GetWithoutFixModuleBase(ModuleName, &bFree)) != 0)
// 		{
// 			ULONG i = 0;
// 			PIMAGE_THUNK_DATA ImageThunkData = (PIMAGE_THUNK_DATA)(ReloadBase + ImageImportDescriptor->OriginalFirstThunk);
// 
// 			while(ImageThunkData->u1.Ordinal)
// 			{
// 				if( !IMAGE_SNAP_BY_ORDINAL32(ImageThunkData->u1.Ordinal) )
// 				{
// 					PIMAGE_IMPORT_BY_NAME ImortByName = (PIMAGE_IMPORT_BY_NAME)(ReloadBase + ImageThunkData->u1.AddressOfData);
// 					ULONG ReloadExportedFunctionAddress = (DWORD)FindExportedRoutineInReloadModule(WithoutFixModuleBase, ImortByName->Name);
// 
// 					if (ReloadExportedFunctionAddress)
// 					{
// 						ULONG IatAddress = OriginBase + ImageImportDescriptor->FirstThunk + i * sizeof(IMAGE_THUNK_DATA);
// 						ULONG NowFunction = *(PULONG)(OriginBase + ImageImportDescriptor->FirstThunk + i * sizeof(IMAGE_THUNK_DATA));
// 						ULONG OriginFunction = ReloadExportedFunctionAddress - WithoutFixModuleBase + OriginModuleBase;
// 
// 						if ((NowFunction < KernelBase || NowFunction > KernelBase + KernelSize) &&
// 							NowFunction != OriginFunction && 
// 							IatAddress > SYSTEM_ADDRESS_START &&
// 							MzfMmIsAddressValid((PVOID)IatAddress))
// 						{
// 							if (nCnt > iCnt)
// 							{
// 								ANSI_STRING anFunction;
// 								UNICODE_STRING unFunction;
// 
// 								pIatEatHookInfo[iCnt].HookAddress = IatAddress;
// 								pIatEatHookInfo[iCnt].HookType = enumIat;
// 								pIatEatHookInfo[iCnt].NowAddress = NowFunction;
// 								pIatEatHookInfo[iCnt].OriginAddress = OriginFunction;
// 
// 								MzfRtlInitAnsiString(&anFunction, ImortByName->Name);
// 
// 								if (NT_SUCCESS(MzfRtlAnsiStringToUnicodeString(&unFunction, &anFunction, TRUE)))
// 								{
// 									ULONG len = CmpAndGetStringLength(&unFunction, MAX_FUNCTION_LEN);
// 									wcsncpy(pIatEatHookInfo[iCnt].szFunction, unFunction.Buffer, len);
// 									MzfRtlFreeUnicodeString(&unFunction);
// 								}
// 								else
// 								{
// 									WCHAR szUnknow[] = {'U','n','K','n','o','w','\0'};
// 									wcscpy(pIatEatHookInfo[iCnt].szFunction, szUnknow);
// 								}
// 
// 								iCnt++;
// 							}
// 
// 							KdPrint(("ImageThunkData->u1.Ordinal: %d\n", ImageThunkData->u1.Ordinal));
// 							KdPrint(("Module: %s Fun: %s, Iat: 0x%08X, Now: 0x%08X, Origin: 0x%08X, Reload: 0x%08X, WFBase: 0x%08X\n", 
// 								ModuleName, ImortByName->Name, IatAddress, NowFunction, OriginFunction, ReloadExportedFunctionAddress, WithoutFixModuleBase));
// 						}
// 					}
// 					else
// 					{
// 						break;
// 					}
// 				}
// 
// 				i++;
// 				ImageThunkData++;
// 			}
// 
// 			if (bFree)
// 			{
// 				ExFreePool((PVOID)WithoutFixModuleBase);
// 			}
// 		}
// 
// 		ImageImportDescriptor++;
// 	}

//	return iCnt;
}

NTSTATUS ListProcessHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	return status;
}