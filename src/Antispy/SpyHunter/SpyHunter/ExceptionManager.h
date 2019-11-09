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

class CExceptionManager
{
public:
	CExceptionManager(void);
	~CExceptionManager(void);

public:
	
	HRESULT SetUnHandleException(DWORD dwDMPType = 0);
	HRESULT DumpExcepFile(IN struct _EXCEPTION_POINTERS *ExceptionInfo);

protected:
	HRESULT GenDumpFileName(OUT TCHAR *szFilePath, IN DWORD dwStrLen);
	DWORD GetCurrentPath(IN HMODULE hModule, OUT TCHAR *szPath, IN DWORD dwSize);
protected:
	HANDLE	m_hSingleEvent;
	DWORD	m_dwDMPType;
};
