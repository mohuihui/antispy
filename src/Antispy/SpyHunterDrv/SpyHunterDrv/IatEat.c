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
#include "IatEat.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "DispatchHook.h"
#include "Peload.h"

/*************************************************
Returns:   BOOL
Qualifier: 获得kernel的路径
Parameter: WCHAR * szNtoskrnlPath - 返回kernel路径
*************************************************/
BOOL GetKernelPath(OUT WCHAR *szNtoskrnlPath)
{
	ULONG ulSize = 0, i = 0;
	PMODULES pModuleList = NULL;
	CHAR* szKernelName = NULL;
	ANSI_STRING AnsiKernelName;
	UNICODE_STRING UnKernelName;
	BOOL bRet = TRUE;
	WCHAR szSystem32[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','s','y','s','t','e','m','3','2','\\','\0'};
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;

	if (!szNtoskrnlPath ||
		!MzfNtQuerySystemInformation || 
		!MzfExAllocatePoolWithTag || 
		!MzfExFreePoolWithTag ||
		!MzfPsGetCurrentThread ||
		!MzfRtlInitAnsiString) 
	{
		return FALSE;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	if (MzfNtQuerySystemInformation(SystemModuleInformation, NULL, 0, &ulSize) != STATUS_INFO_LENGTH_MISMATCH)
	{
		return FALSE;
	}

	pModuleList = (PMODULES)MzfExAllocatePoolWithTag(PagedPool, ulSize, MZFTAG);
	if (pModuleList)
	{
		memset(pModuleList, 0, ulSize);
		if (!NT_SUCCESS(MzfNtQuerySystemInformation(SystemModuleInformation, pModuleList, ulSize, &ulSize)))
		{
			MzfExFreePoolWithTag(pModuleList, 0);
			RecoverPreMode(pThread, PreMode);
			return FALSE;
		}
	}

	RecoverPreMode(pThread, PreMode);

	szKernelName = pModuleList->smi[0].ModuleNameOffset + pModuleList->smi[0].ImageName;
	MzfRtlInitAnsiString(&AnsiKernelName, szKernelName);
	MzfRtlAnsiStringToUnicodeString(&UnKernelName, &AnsiKernelName, TRUE);

	wcscat(szNtoskrnlPath, szSystem32);
	memcpy(szNtoskrnlPath + wcslen(szSystem32), UnKernelName.Buffer, UnKernelName.Length);

	MzfExFreePoolWithTag(pModuleList, 0);
	MzfRtlFreeUnicodeString(&UnKernelName);

	return TRUE;
}

/*************************************************
Returns:   BOOL
Qualifier: 获得HAL.DLL的路径，因为有些破解内核会修改hal.dll的名字
Parameter: WCHAR * szNtoskrnlPath - 返回kernel路径
*************************************************/
BOOL GetHalPath(OUT WCHAR *szNtoskrnlPath)
{
	ULONG ulSize = 0, i = 0;
	PMODULES pModuleList = NULL;
	CHAR* szKernelName = NULL;
	ANSI_STRING AnsiKernelName;
	UNICODE_STRING UnKernelName;
	BOOL bRet = TRUE;
	WCHAR szSystem32[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','s','y','s','t','e','m','3','2','\\','\0'};
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;

	if (!szNtoskrnlPath ||
		!MzfNtQuerySystemInformation || 
		!MzfExAllocatePoolWithTag || 
		!MzfExFreePoolWithTag ||
		!MzfPsGetCurrentThread ||
		!MzfRtlInitAnsiString) 
	{
		return FALSE;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	if (MzfNtQuerySystemInformation(SystemModuleInformation, NULL, 0, &ulSize) != STATUS_INFO_LENGTH_MISMATCH)
	{
		return FALSE;
	}

	pModuleList = (PMODULES)MzfExAllocatePoolWithTag(PagedPool, ulSize, MZFTAG);
	if (pModuleList)
	{
		memset(pModuleList, 0, ulSize);
		if (!NT_SUCCESS(MzfNtQuerySystemInformation(SystemModuleInformation, pModuleList, ulSize, &ulSize)))
		{
			MzfExFreePoolWithTag(pModuleList, 0);
			RecoverPreMode(pThread, PreMode);
			return FALSE;
		}
	}

	RecoverPreMode(pThread, PreMode);

	// 通常第二个就是hal.dll
	szKernelName = pModuleList->smi[1].ModuleNameOffset + pModuleList->smi[1].ImageName;
	MzfRtlInitAnsiString(&AnsiKernelName, szKernelName);
	MzfRtlAnsiStringToUnicodeString(&UnKernelName, &AnsiKernelName, TRUE);

	wcscat(szNtoskrnlPath, szSystem32);
	memcpy(szNtoskrnlPath + wcslen(szSystem32), UnKernelName.Buffer, UnKernelName.Length);

	MzfExFreePoolWithTag(pModuleList, 0);
	MzfRtlFreeUnicodeString(&UnKernelName);
	
	KdPrint(("hal: %S\n", szNtoskrnlPath));

	return TRUE;
}

/*************************************************
Returns:   BOOL
Qualifier: reload hal.dll with out fix
Parameter: ULONG * pBase - the reloaded hal base
*************************************************/
BOOL ReloadHalWithoutFix(OUT ULONG* pBase)
{
	ULONG HalDllBase = 0, HalDllSize = 0; 
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	WCHAR szHalDll[] = {'H','a','l','.','D','l','l','\0'};
	WCHAR szHalmacpiDll[] = {'h','a','l','m','a','c','p','i','.','d','l','l','\0'};
	WCHAR *szModuleName = NULL;
	BOOL bRet = FALSE;

	if (!pBase)
	{
		return FALSE;
	}

	HalDllBase = (ULONG)LookupKernelModuleByNameW(szHalDll, &HalDllSize);

	if (!HalDllBase || !HalDllSize)
	{
		HalDllBase = (ULONG)LookupKernelModuleByNameW(szHalmacpiDll, &HalDllSize);

		if (HalDllBase && HalDllSize)
		{
			szModuleName = szHalmacpiDll;
		}
	}
	else
	{
		szModuleName = szHalDll;
	}

	if (HalDllBase && HalDllSize && szModuleName)
	{
		ULONG NewBase = 0;
		WCHAR szSystem32[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','\0'};
		WCHAR szPath[MAX_PATH] = {0};

		wcscpy(szPath, szSystem32);
		wcscat(szPath, szModuleName);

		if (PeLoadWithoutFix(szPath, &NewBase))
		{
			bRet = TRUE;
			*pBase = NewBase;
		}
	}

	return bRet;
}

ULONG GetWithoutFixModuleBase(CHAR *ModuleName)
{
	ULONG pReloadBase = 0;
	
	if (ModuleName)
	{
		CHAR szNtkrnlpa[] = {'n','t','k','r','n','l','p','a','.','e','x','e','\0'};
		CHAR szNtoskrnl[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
		CHAR szNtkrnlmp[] = {'n','t','k','r','n','l','m','p','.','e','x','e','\0'};
		CHAR szNtkrpamp[] = {'n','t','k','r','p','a','m','p','.','e','x','e','\0'};
		CHAR szHalmacpi[] = {'h','a','l','m','a','c','p','i','.','d','l','l','\0'};
		CHAR szHal[] = {'h','a','l','.','d','l','l','\0'};
		ULONG NewBase = 0;

		//ntkrnlpa.exe、ntoskrnl.exe、ntkrnlmp.exe、ntkrpamp.exe
		if (!_stricmp(ModuleName, szNtkrnlpa)	||
			!_stricmp(ModuleName, szNtoskrnl)	||
			!_stricmp(ModuleName, szNtkrnlmp)	||
			!_stricmp(ModuleName, szNtkrpamp))
		{	
			WCHAR szKernel[MAX_PATH] = {0};
			if (GetKernelPath(szKernel) && PeLoadWithoutFix(szKernel, &NewBase))
			{
				pReloadBase = NewBase;
			}
		}
		else if (!_stricmp(ModuleName, szHal)	||
			!_stricmp(ModuleName, szHalmacpi)	)
		{
			WCHAR szHal[MAX_PATH] = {0};
			if (GetHalPath(szHal) && PeLoadWithoutFix(szHal, &NewBase))
			{
				pReloadBase = NewBase;
			}
		}

		if (!pReloadBase)
		{
			WCHAR szSystem32[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','\0'};
			WCHAR szPath[MAX_PATH] = {0};
			pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
			pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
			pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);

			if (MzfRtlInitAnsiString && MzfRtlAnsiStringToUnicodeString && MzfRtlFreeUnicodeString)
			{
				ANSI_STRING asModule;
				UNICODE_STRING unModule;
		
				MzfRtlInitAnsiString(&asModule, ModuleName);
				if (NT_SUCCESS(MzfRtlAnsiStringToUnicodeString(&unModule, &asModule, TRUE)))
				{
					wcscpy(szPath, szSystem32);
					wcsncat(szPath, unModule.Buffer, unModule.Length / sizeof(WCHAR));
					
					if (PeLoadWithoutFix(szPath, &NewBase))
					{
						pReloadBase = NewBase;
					}
					else
					{
						WCHAR szDriver[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','D','r','i','v','e','r','s','\\','\0'};
						memset(szPath, 0, MAX_PATH * sizeof(WCHAR));
						wcscpy(szPath, szDriver);
						wcsncat(szPath, unModule.Buffer, unModule.Length / sizeof(WCHAR));
						if (PeLoadWithoutFix(szPath, &NewBase))
						{
							pReloadBase = NewBase;
						}
					}

					MzfRtlFreeUnicodeString(&unModule);
				}
			}
		}
	}

	return pReloadBase;
}

ULONG GetOriginModuleBase(CHAR *ModuleName)
{
	ULONG ModuleBase = 0;

	if (ModuleName)
	{
		CHAR szNtkrnlpa[] = {'n','t','k','r','n','l','p','a','.','e','x','e','\0'};
		CHAR szNtoskrnl[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
		CHAR szNtkrnlmp[] = {'n','t','k','r','n','l','m','p','.','e','x','e','\0'};
		CHAR szNtkrpamp[] = {'n','t','k','r','p','a','m','p','.','e','x','e','\0'};
		CHAR szHalmacpi[] = {'h','a','l','m','a','c','p','i','.','d','l','l','\0'};
		CHAR szHal[] = {'h','a','l','.','d','l','l','\0'};

		//ntkrnlpa.exe、ntoskrnl.exe、ntkrnlmp.exe、ntkrpamp.exe
		if (!_stricmp(ModuleName, szNtkrnlpa)	||
			!_stricmp(ModuleName, szNtoskrnl)	||
			!_stricmp(ModuleName, szNtkrnlmp)	||
			!_stricmp(ModuleName, szNtkrpamp))
		{	
			ModuleBase = GetGlobalVeriable(enumOriginKernelBase);
		}
		else if (!_stricmp(ModuleName,szHal)	||
			!_stricmp(ModuleName, szHalmacpi)	)
		{
			ModuleBase = GetGlobalVeriable(enumOriginHalDllBase);
		}

		if (!ModuleBase)
		{
			ModuleBase = (ULONG)GetKernelModuleBase(ModuleName);
		}
	}

	return ModuleBase;
}

NTSTATUS EnumImportDirectoryHooks(
								ULONG OriginBase, 
								ULONG ReloadBase, 
								ULONG Size, 
								PALL_IAT_EAT_HOOK pIatEatHookInfo, 
								ULONG nCnt
								)
{
	PIMAGE_IMPORT_DESCRIPTOR ImageImportDescriptor = NULL;
	DWORD ImportSize = 0;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);

	if (!OriginBase								|| 
		!ReloadBase								|| 
		!pIatEatHookInfo						|| 
		!MzfMmIsAddressValid					||
		!MzfExFreePoolWithTag					||
		!MzfRtlInitAnsiString					|| 
		!MzfRtlFreeUnicodeString				||
		!MzfRtlImageDirectoryEntryToData		||
		!MzfRtlAnsiStringToUnicodeString		||
		!MzfMmIsAddressValid((PVOID)OriginBase) ||
		!MzfMmIsAddressValid((PVOID)ReloadBase) )
	{
		return STATUS_INVALID_PARAMETER;
	}

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)ReloadBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return STATUS_UNSUCCESSFUL;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)ReloadBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return STATUS_UNSUCCESSFUL;
		}

		ImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)MzfRtlImageDirectoryEntryToData((PVOID)ReloadBase, 
			TRUE, 
			IMAGE_DIRECTORY_ENTRY_IMPORT, 
			&ImportSize);

		if (ImageImportDescriptor == NULL)
		{
			return STATUS_UNSUCCESSFUL;
		}

		KdPrint(("ImageImportDescriptor: 0x%08X, ImportSize: 0x%08X\n", (ULONG)ImageImportDescriptor - ReloadBase, ImportSize));

		while (ImageImportDescriptor->OriginalFirstThunk && ImageImportDescriptor->Name)
		{
			CHAR *ModuleName = (CHAR*)(ReloadBase + ImageImportDescriptor->Name);
			ULONG WithoutFixModuleBase = 0, OriginModuleBase = 0;

			if ( (OriginModuleBase = GetOriginModuleBase(ModuleName)) != 0 &&
				(WithoutFixModuleBase = GetWithoutFixModuleBase(ModuleName)) != 0)
			{
				ULONG i = 0;
				PIMAGE_THUNK_DATA ImageThunkData = (PIMAGE_THUNK_DATA)(ReloadBase + ImageImportDescriptor->OriginalFirstThunk);

				while(ImageThunkData->u1.Ordinal)
				{
					if( !IMAGE_SNAP_BY_ORDINAL32(ImageThunkData->u1.Ordinal) )
					{
						PIMAGE_IMPORT_BY_NAME ImortByName = (PIMAGE_IMPORT_BY_NAME)(ReloadBase + ImageThunkData->u1.AddressOfData);
						ULONG ReloadExportedFunctionAddress = (DWORD)FindExportedRoutineInReloadModule(WithoutFixModuleBase, ImortByName->Name);

						if (ReloadExportedFunctionAddress)
						{
							ULONG IatAddress = OriginBase + ImageImportDescriptor->FirstThunk + i * sizeof(IMAGE_THUNK_DATA);
							if (IatAddress > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)IatAddress))
							{
								ULONG NowFunction = *(PULONG)IatAddress;
								ULONG OriginFunction = ReloadExportedFunctionAddress - WithoutFixModuleBase + OriginModuleBase;
								if ((NowFunction < KernelBase || NowFunction > KernelBase + KernelSize) &&
									NowFunction != OriginFunction )
								{
									ULONG nCurCnt = pIatEatHookInfo->nCnt;
									if (nCnt > nCurCnt)
									{
										ANSI_STRING anFunction;
										UNICODE_STRING unFunction;

										pIatEatHookInfo->hooks[nCurCnt].HookAddress = IatAddress;
										pIatEatHookInfo->hooks[nCurCnt].HookType = enumIat;
										pIatEatHookInfo->hooks[nCurCnt].NowAddress = NowFunction;
										pIatEatHookInfo->hooks[nCurCnt].OriginAddress = OriginFunction;

										MzfRtlInitAnsiString(&anFunction, ImortByName->Name);

										if (NT_SUCCESS(MzfRtlAnsiStringToUnicodeString(&unFunction, &anFunction, TRUE)))
										{
											ULONG len = CmpAndGetStringLength(&unFunction, MAX_FUNCTION_LEN);
											wcsncpy(pIatEatHookInfo->hooks[nCurCnt].szFunction, unFunction.Buffer, len);
											MzfRtlFreeUnicodeString(&unFunction);
										}
										else
										{
											WCHAR szUnknow[] = {'U','n','K','n','o','w','\0'};
											wcscpy(pIatEatHookInfo->hooks[nCurCnt].szFunction, szUnknow);
										}
									}

									KdPrint(("ImageThunkData->u1.Ordinal: %d\n", ImageThunkData->u1.Ordinal));
									KdPrint(("Module: %s Fun: %s, Iat: 0x%08X, Now: 0x%08X, Origin: 0x%08X, Reload: 0x%08X, WFBase: 0x%08X\n", 
										ModuleName, ImortByName->Name, IatAddress, NowFunction, OriginFunction, ReloadExportedFunctionAddress, WithoutFixModuleBase));

									pIatEatHookInfo->nCnt++;
								}
							}
						}
						else
						{
							break;
						}
					}

					i++;
					ImageThunkData++;
				}
				
				MzfExFreePoolWithTag((PVOID)WithoutFixModuleBase, 0);
			}

			ImageImportDescriptor++;
		}
	}
	__except(1)
	{
	}

	return STATUS_SUCCESS;
}

NTSTATUS EnumExportDirectoryHooks(
								ULONG OriginBase, 
								ULONG ReloadBase, 
								ULONG Size, 
								PALL_IAT_EAT_HOOK pIatEatHookInfo, 
								ULONG nCnt
								)
{
	PIMAGE_EXPORT_DIRECTORY ReloadImageExportDirectory = NULL, ImageExportDirectory = NULL;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	ULONG ExportSize = 0, i = 0;
	PULONG pAddressOfFunctions = NULL, pReloadAddressOfNames = NULL, pAddressOfNames = NULL;
	PUSHORT pReloadAddressOfNameOrdinals = NULL, pAddressOfNameOrdinals = NULL;
	PULONG ReloadAddressOfFunctionsRVA = NULL, AddressOfFunctionsRVA = NULL;
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);

	if (!OriginBase								|| 
		!ReloadBase								|| 
		!pIatEatHookInfo						|| 
		!MzfMmIsAddressValid					||
		!MzfRtlInitAnsiString					|| 
		!MzfRtlFreeUnicodeString				||
		!MzfRtlImageDirectoryEntryToData		||
		!MzfRtlAnsiStringToUnicodeString		||
		!MzfMmIsAddressValid((PVOID)OriginBase) ||
		!MzfMmIsAddressValid((PVOID)ReloadBase) )
	{
		return STATUS_INVALID_PARAMETER;
	}

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)OriginBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return STATUS_UNSUCCESSFUL;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)OriginBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return STATUS_UNSUCCESSFUL;
		}

		ImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)MzfRtlImageDirectoryEntryToData((PVOID)OriginBase, 
			TRUE, 
			IMAGE_DIRECTORY_ENTRY_EXPORT, 
			&ExportSize);

		ReloadImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)MzfRtlImageDirectoryEntryToData((PVOID)ReloadBase, 
			TRUE, 
			IMAGE_DIRECTORY_ENTRY_EXPORT, 
			&ExportSize);

		if (ReloadImageExportDirectory == NULL || ImageExportDirectory == NULL)
		{
			return STATUS_UNSUCCESSFUL;
		}

		ReloadAddressOfFunctionsRVA = (PULONG)(ReloadBase + ReloadImageExportDirectory->AddressOfFunctions);
		AddressOfFunctionsRVA = (PULONG)(OriginBase + ImageExportDirectory->AddressOfFunctions);

		pReloadAddressOfNames = (ULONG*)(ReloadBase + ReloadImageExportDirectory->AddressOfNames); 
		pReloadAddressOfNameOrdinals = (USHORT*)(ReloadBase + ReloadImageExportDirectory->AddressOfNameOrdinals); 

		pAddressOfNames = (ULONG*)(OriginBase + ImageExportDirectory->AddressOfNames); 
		pAddressOfNameOrdinals = (USHORT*)(OriginBase + ImageExportDirectory->AddressOfNameOrdinals); 

		if (!pReloadAddressOfNames || 
			!pReloadAddressOfNameOrdinals)
		{
			return STATUS_UNSUCCESSFUL;
		}

		for (i = 0; i < ReloadImageExportDirectory->NumberOfNames; i++)
		{
			PUCHAR pReloadFuncName = (PUCHAR)(ReloadBase + pReloadAddressOfNames[i]);
			PUCHAR pFuncName = (PUCHAR)(OriginBase + pAddressOfNames[i]);
			USHORT nIndex = 0;

			if (strlen(pReloadFuncName) != strlen(pFuncName) ||
				_stricmp( pReloadFuncName, pFuncName ))
			{
				KdPrint(("%s is patched %s\n", pReloadFuncName, pFuncName));
			}

			nIndex = pReloadAddressOfNameOrdinals[i];
			/*nIndex = pAddressOfNameOrdinals[i];*/

			if (ReloadAddressOfFunctionsRVA[nIndex] != AddressOfFunctionsRVA[nIndex])
			{
				ULONG nCurCnt = pIatEatHookInfo->nCnt;
				if (nCnt > nCurCnt)
				{
					ANSI_STRING anFunction;
					UNICODE_STRING unFunction;

					pIatEatHookInfo->hooks[nCurCnt].HookAddress = (ULONG)&AddressOfFunctionsRVA[nIndex];
					pIatEatHookInfo->hooks[nCurCnt].HookType = enumEat;
					pIatEatHookInfo->hooks[nCurCnt].NowAddress = AddressOfFunctionsRVA[nIndex] + OriginBase;
					pIatEatHookInfo->hooks[nCurCnt].OriginAddress = ReloadAddressOfFunctionsRVA[nIndex] + OriginBase;

					MzfRtlInitAnsiString(&anFunction, pReloadFuncName);

					if (NT_SUCCESS(MzfRtlAnsiStringToUnicodeString(&unFunction, &anFunction, TRUE)))
					{
						ULONG len = CmpAndGetStringLength(&unFunction, MAX_FUNCTION_LEN);
						wcsncpy(pIatEatHookInfo->hooks[nCurCnt].szFunction, unFunction.Buffer, len);
						MzfRtlFreeUnicodeString(&unFunction);
					}
					else
					{
						WCHAR szUnknow[] = {'U','n','K','n','o','w','\0'};
						wcscpy(pIatEatHookInfo->hooks[nCurCnt].szFunction, szUnknow);
					}
				}

				KdPrint(("Eat is patched-> nIndex: %d, Address: 0x%08X, Function: %s, Origin: 0x%08X, Now: 0x%08X\n", 
					nIndex,
					&AddressOfFunctionsRVA[nIndex],
					pReloadFuncName, 
					ReloadAddressOfFunctionsRVA[nIndex] + OriginBase,
					AddressOfFunctionsRVA[nIndex] + OriginBase));

				pIatEatHookInfo->nCnt++;
			}
		}
	}
	__except(1)
	{
	}

	return STATUS_SUCCESS;
}

//
// 枚举内核模块IAT/EAT HOOK
//
NTSTATUS EnumIatEatHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PALL_IAT_EAT_HOOK pIatEatHookInfo = (PALL_IAT_EAT_HOOK)pOutBuffer;
	ULONG nCnt = (uOutSize - sizeof(ALL_IAT_EAT_HOOK)) / sizeof(IAT_EAT_HOOK_INFO);
	PCOMMUNICATEIAT_IAT_EAT_HOOK pIatEatInput = (PCOMMUNICATEIAT_IAT_EAT_HOOK)pInBuffer;
	ULONG Base = 0, Size = 0, NewImageBase = 0;
	WCHAR *szPath = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	// 
	// 参数检查
	// 
	if (!pInBuffer || 
		uInSize != sizeof(COMMUNICATE_IAT_EAT_HOOK) ||
		!pOutBuffer || 
		uOutSize < sizeof(ALL_IAT_EAT_HOOK))
	{
		return STATUS_INVALID_PARAMETER;
	}

	Base = pIatEatInput->op.Get.Base;
	Size = pIatEatInput->op.Get.Size;
	szPath = pIatEatInput->op.Get.szPath;

	if (Size == 0							||
		Size > SYSTEM_ADDRESS_START			||
		Base < SYSTEM_ADDRESS_START			||
		Base + Size < SYSTEM_ADDRESS_START	||
		wcslen(szPath) < 3)
	{
		return STATUS_INVALID_PARAMETER;	
	}

	KdPrint(("szPath: %S\n", szPath));
	if (PeLoadWithoutFix(szPath, &NewImageBase) && NewImageBase > SYSTEM_ADDRESS_START)
	{
		EnumExportDirectoryHooks(
			Base, 
			NewImageBase, 
			Size, 
			pIatEatHookInfo, 
			nCnt
			);

		EnumImportDirectoryHooks(
			Base, 
			NewImageBase, 
			Size, 
			pIatEatHookInfo, 
			nCnt
			);
	}

	if (NewImageBase)
	{
		ExFreePool((PVOID)NewImageBase);
	}

	if (nCnt >= pIatEatHookInfo->nCnt)
	{
		status = STATUS_SUCCESS; 
	}
	else
	{
		status = STATUS_BUFFER_TOO_SMALL;
	}

	return status;
}

NTSTATUS RestoreIatEatHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATEIAT_IAT_EAT_HOOK pIatEatInput = (PCOMMUNICATEIAT_IAT_EAT_HOOK)pInBuffer;
	ULONG HookAddress = pIatEatInput->op.Restore.HookAddress;
	ULONG OriginDispatch = pIatEatInput->op.Restore.OriginDispatch;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("HookAddress: 0x%08X, OriginDispatch: 0x%08X\n", HookAddress, OriginDispatch));

	if (MzfMmIsAddressValid &&
		OriginDispatch != 0 && 
		HookAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)HookAddress))
	{
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
		WPOFF();
	//	*(PULONG)HookAddress = OriginDispatch;
		InterlockedExchange((PLONG)HookAddress, (LONG)OriginDispatch);
		WPON();
		KeLowerIrql(OldIrql);

		status = STATUS_SUCCESS;
	}

	return status;
}