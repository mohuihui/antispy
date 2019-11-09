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
#include "stdafx.h"
#include "HookFunc.h"
#include "hooklib/HookAPIX.h"

XLIB::CXHookAPI m_HookLoadLibraryExW;
DWORD dwUser32Base = 0, dwUser32End = 0;
WCHAR szImageres[MAX_PATH] = {0};

HINSTANCE WINAPI FakeLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD	dwFlags)
{
	DWORD RetAddr = 0;
	__asm      
	{      
		push [ebp + 4]      
		pop  [RetAddr]      
	}      

	if ( dwFlags != 2 && RetAddr >= dwUser32Base && RetAddr < dwUser32End )
	{
		if (wcslen(szImageres) != wcslen(lpLibFileName) ||
			_wcsicmp(lpLibFileName, szImageres))
		{
			DebugLog(L"Deny lib: %s", lpLibFileName);
			return NULL;
		}
	}

	//调用源函数
	return (HINSTANCE)(m_HookLoadLibraryExW.CallFunction( 3, lpLibFileName, hFile, dwFlags ));
}


BOOL HookLoadLibraryExW()
{
	WCHAR szUser32[] = {'u','s','e','r','3','2','.','d','l','l','\0'};
	dwUser32Base = (DWORD)GetModuleHandle(szUser32);       
	if (!dwUser32Base) 
	{
		return FALSE;    
	}

	dwUser32End = *(DWORD *)(dwUser32Base + 0x3c);       
	dwUser32End = *(DWORD *)(dwUser32Base + dwUser32End + 0x50);
	dwUser32End = dwUser32Base + dwUser32End;
	DebugLog(L"dwUser32Base: 0x%08X, dwUser32End: 0x%08X", dwUser32Base, dwUser32End);

	WCHAR szimageres[] = {'\\','i','m','a','g','e','r','e','s','.','d','l','l','\0'};
	GetSystemDirectory(szImageres, MAX_PATH);
	wcscat_s(szImageres, szimageres);
	DebugLog(szImageres);
	
	CHAR szKernel32[] = {'k','e','r','n','e','l','3','2','.','d','l','l','\0'};
	CHAR szLoadLibraryExW[] = {'L','o','a','d','L','i','b','r','a','r','y','E','x','W','\0'};

	LPVOID lpAddr = XLIB::CXHookAPI::GetAddressByNameA( szKernel32, szLoadLibraryExW );

	return m_HookLoadLibraryExW.InlineHookFunction( lpAddr, FakeLoadLibraryExW );
}