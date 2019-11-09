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

#include <Windows.h>

#include <vector>
using namespace std;

typedef struct _MY_WSTRING
{
	WCHAR string[MAX_PATH];
}MY_WSTRING,*PMY_WSTRING;

BOOL CutLastStringByChar(wchar_t *pwszString, wchar_t *pwszOut, wchar_t wChar);

INT wcscpy_sEx(WCHAR *wcDest, size_t numElement, const WCHAR *wcSrc);

ULONG GetStringHashWZ(const WCHAR *wsSrc);

int WStrToLower(
				/* [In] */ wchar_t *pszSrc,
				/* [Out] */wchar_t *pszDest,
				/* [In] */ int nOutSize
				);

int GetFileFullPath(PWCHAR pwszPath, WCHAR *pwszCurDir, vector<MY_WSTRING>&  FullPathList);