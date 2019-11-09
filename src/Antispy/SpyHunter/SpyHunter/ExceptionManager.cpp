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
#include "ExceptionManager.h"
#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp")

#pragma warning(disable:4996) 

CExceptionManager *gpExcepMg = NULL;

LONG WINAPI MyExcepFunc(__in struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	
	if(gpExcepMg != NULL)
	{
		gpExcepMg->DumpExcepFile(ExceptionInfo);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

CExceptionManager::CExceptionManager(void)
:m_dwDMPType(MiniDumpNormal)
{
	gpExcepMg = this;
}

CExceptionManager::~CExceptionManager(void)
{

}

HRESULT CExceptionManager::SetUnHandleException(DWORD dwDMPType /* = 0 */)
{
	m_dwDMPType = dwDMPType;
	LPTOP_LEVEL_EXCEPTION_FILTER  pLast = SetUnhandledExceptionFilter(MyExcepFunc);
	return S_OK;
}

HRESULT CExceptionManager::DumpExcepFile(IN struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	HRESULT hr = S_FALSE;
	TCHAR szDumpFile[MAX_PATH] = {0};
	GenDumpFileName(szDumpFile, MAX_PATH);
	HANDLE hFile = CreateFile(szDumpFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	MINIDUMP_EXCEPTION_INFORMATION m1;
	m1.ThreadId = GetCurrentThreadId();
	m1.ExceptionPointers = ExceptionInfo;
	m1.ClientPointers = TRUE;

	BOOL bMiniDumpRet = MiniDumpWriteDump(
		GetCurrentProcess(), 
		GetCurrentProcessId(), 
		hFile, 
		(MINIDUMP_TYPE)m_dwDMPType, //MiniDumpNormal
		&m1, 
/*		NULL,*/
		NULL, 
		NULL
		);

	CloseHandle(hFile);

	if(!bMiniDumpRet)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	return hr;
}

DWORD CExceptionManager::GetCurrentPath(IN HMODULE hModule, OUT TCHAR *szPath, IN DWORD dwSize)
{
	if(NULL == szPath || 0 == dwSize)
		return 0;
	TCHAR szTemp[MAX_PATH] = {0};
	DWORD dwGet = GetModuleFileName(hModule, szTemp, MAX_PATH);
	if(0 == dwGet)
		return 0;
	TCHAR szDriver[10] = {0};
	TCHAR szDirectory[MAX_PATH] = {0};

	_tsplitpath(szTemp, szDriver, szDirectory, NULL, NULL);
	ZeroMemory(szTemp, MAX_PATH * sizeof(TCHAR));

	_tmakepath(szTemp, szDriver, szDirectory, NULL, NULL);

	DWORD dwLen = (DWORD)_tcslen(szTemp);
	if(dwLen < dwSize)
	{
		_tcscpy(szPath, szTemp);
		return dwLen;
	}
	else
	{
		_tcsncpy(szPath, szTemp, dwSize - 1);
		szPath[dwSize - 1] = '\0';
		return (dwSize - 1);
	}
}

HRESULT CExceptionManager::GenDumpFileName(OUT TCHAR *szFilePath, IN DWORD dwStrLen)
{
	if(szFilePath != NULL)
	{
		TCHAR szCurPath[MAX_PATH] = {0};
		TCHAR szTimes[MAX_PATH] = {0};
		TCHAR szDate[MAX_PATH] = {0};	
		GetDateFormat(NULL, 0, NULL, _T("yyyyMMdd"), szDate, MAX_PATH);
		GetTimeFormat(NULL, TIME_FORCE24HOURFORMAT, NULL, _T("hh'-'mm'-'ss"), szTimes, MAX_PATH);
		GetCurrentPath(NULL, szCurPath, MAX_PATH);
		
		_stprintf(szFilePath, _T("%s%s_%s.dmp"), szCurPath, szDate, szTimes);

		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
}

#pragma warning(default:4996) 