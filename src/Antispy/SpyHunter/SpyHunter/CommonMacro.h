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
#ifdef _TRACK

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>

__inline void DebugLog(const TCHAR* ptzFormat, ...)
{
#pragma warning(disable:4996)
	if(!ptzFormat)
		return;
	va_list vlArgs;
	
	TCHAR tzText[1024];
	TCHAR tzText2[1152]; // 1024 + 128
	va_start(vlArgs, ptzFormat);
	if(_vsctprintf(ptzFormat,vlArgs) < 1024)
	{
#ifdef _UNICODE
	vswprintf_s(tzText, 1024, ptzFormat, vlArgs);
#else
	_vstprintf_s(tzText, 1024, ptzFormat, vlArgs);
#endif
	}else
	{
		_tcscpy(tzText, _T("Input va_list too long, won't show the values!!!!!!!!"));
	}

	_stprintf_s(tzText2, 1152, _T("%s%s\n"), L"AntiSpy: ", tzText);
	OutputDebugString(tzText2);
#ifdef _Print_To_Console
	_tprintf(tzText2);
#endif
	va_end(vlArgs);
#pragma warning(default:4996)
}

#else
#define DebugLog(x,...)
#endif


#ifdef _TRACK
__inline void DebugMessageBox(const TCHAR* ptzFormat, ...)
{
#pragma warning(disable:4996)
	if(!ptzFormat)
		return;
	va_list vlArgs;

	TCHAR tzText[1024];
	TCHAR tzText2[1152]; // 1024 + 128
	va_start(vlArgs, ptzFormat);
	if(_vsctprintf(ptzFormat,vlArgs) < 1024)
	{
#ifdef _UNICODE
		vswprintf_s(tzText, 1024, ptzFormat, vlArgs);
#else
		_vstprintf_s(tzText, 1024, ptzFormat, vlArgs);
#endif
	}else
	{
		_tcscpy(tzText, _T("Input va_list too long, won't show the values!!!!!!!!"));
	}

	_stprintf_s(tzText2, 1152, _T("%s\n"), tzText);
	MessageBox(NULL, tzText2, L"AntiSpy", MB_OK);
#ifdef _Print_To_Console
	_tprintf(tzText2);
#endif
	va_end(vlArgs);
#pragma warning(default:4996)
}

#else
#define DebugMessageBox(x,...)
#endif