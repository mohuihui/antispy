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
#include "reloadhaldll.h"
#include "InitWindows.h"
#include "CommonFunction.h"
#include "Peload.h"

BOOL ReloadHalDll()
{
	ULONG HalDllBase = 0, HalDllSize = 0; 
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	WCHAR szHalDll[] = {'H','a','l','.','D','l','l','\0'};
	WCHAR szHalmacpiDll[] = {'h','a','l','m','a','c','p','i','.','d','l','l','\0'};
	WCHAR *szModuleName = NULL;

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

		KdPrint(("HalDllBase: 0x%08X, HalDllSize: 0x%X, szModuleName: %s\n", HalDllBase, HalDllSize, szModuleName));

		SetGlobalVeriable(enumOriginHalDllBase, HalDllBase);
		SetGlobalVeriable(enumHalDllSize, HalDllSize);
		
		wcscpy(szPath, szSystem32);
		wcscat(szPath, szModuleName);

		if (PeLoad(szPath, &NewBase, HalDllBase) && NewBase > SYSTEM_ADDRESS_START)
		{
			ULONG NewBaseWithFix = 0;

			SetGlobalVeriable(enumNewHalDllBase, NewBase);

			if (PeLoadWithoutFix(szPath, &NewBaseWithFix))
			{
				SetGlobalVeriable(enumNewHalDllBaseWithoutFix, NewBase);
			}
			
			KdPrint(("New Hall.dll Base: 0x%08X\n", NewBase));
		}
	}

	return TRUE;
}

BOOL GetHalDllInfo()
{
	ULONG HalDllBase = 0, HalDllSize = 0; 
	WCHAR szHalDll[] = {'H','a','l','.','D','l','l','\0'};
	WCHAR szHalmacpiDll[] = {'h','a','l','m','a','c','p','i','.','d','l','l','\0'};

	HalDllBase = (ULONG)LookupKernelModuleByNameW(szHalDll, &HalDllSize);

	if (!HalDllBase || !HalDllSize)
	{
		HalDllBase = (ULONG)LookupKernelModuleByNameW(szHalmacpiDll, &HalDllSize);
	}

	if (HalDllBase && HalDllSize)
	{
		SetGlobalVeriable(enumOriginHalDllBase, HalDllBase);
		SetGlobalVeriable(enumHalDllSize, HalDllSize);
	}

	return TRUE;
}