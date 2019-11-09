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
#include "ListDriver.h"

CListDrivers::CListDrivers()
{

}

CListDrivers::~CListDrivers()
{
	
}

//
// 修复驱动的路径
//
void CListDrivers::FixDriverPath(PDRIVER_INFO pDriverInfo)
{
	if (!pDriverInfo || wcslen(pDriverInfo->szDriverPath) == 0)
	{
		return;
	}

	WCHAR szWindowsDirectory[MAX_PATH] = {0};
	WCHAR szDriverDirectory[MAX_PATH] = {0};	
	WCHAR szDriver[] = {'\\', 's', 'y', 's', 't', 'e', 'm', '3', '2', '\\', 'd', 'r', 'i', 'v', 'e', 'r', 's', '\\', '\0'};

	GetWindowsDirectory(szWindowsDirectory, MAX_PATH - 1);
	wcscpy(szDriverDirectory, szWindowsDirectory);
	wcscat(szDriverDirectory, szDriver);
	
	WCHAR *szOriginPath = pDriverInfo->szDriverPath;
	WCHAR szPath[MAX_PATH] = {0};
	WCHAR *szTemp = wcschr(szOriginPath, L'\\');
	
	// 没有目录信息，只有一个驱动名字的，直接拼接Driver目录。
	if (!szTemp)
	{
		wcscpy(szPath, szDriverDirectory);
		wcscat(szPath, szOriginPath);
		wcscpy(szOriginPath, szPath);
		szOriginPath[wcslen(szPath)] = L'\0';
	}
	else
	{
		WCHAR szFuck[] = {'\\', '?', '?', '\\', '\0'};
		WCHAR szSystemRoot[] = {'\\', 'S', 'y', 's', 't', 'e', 'm', 'R', 'o', 'o', 't', '\0'};
		WCHAR szWindows[] = {'\\', 'W', 'i', 'n', 'd', 'o', 'w', 's', '\0'};
		WCHAR szWinnt[] = {'\\', 'W', 'i', 'n', 'n', 't', '\0'};
		size_t nOrigin = wcslen(szOriginPath);

		if ( nOrigin >= wcslen(szFuck) && !_wcsnicmp(szOriginPath, szFuck, wcslen(szFuck)) )
		{
			wcscpy(szPath, szOriginPath + wcslen(szFuck));
			wcscpy(szOriginPath, szPath);
			szOriginPath[wcslen(szPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(szSystemRoot) && !_wcsnicmp(szOriginPath, szSystemRoot, wcslen(szSystemRoot)))
		{
			wcscpy(szPath, szWindowsDirectory);
			wcscat(szPath, szOriginPath + wcslen(szSystemRoot));
			wcscpy(szOriginPath, szPath);
			szOriginPath[wcslen(szPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(szWindows) && !_wcsnicmp(szOriginPath, szWindows, wcslen(szWindows)))
		{
			wcscpy(szPath, szWindowsDirectory);
			wcscat(szPath, szOriginPath + wcslen(szWindows));
			wcscpy(szOriginPath, szPath);
			szOriginPath[wcslen(szPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(szWinnt) && !_wcsnicmp(szOriginPath, szWinnt, wcslen(szWinnt)))
		{
			wcscpy(szPath, szWindowsDirectory);
			wcscat(szPath, szOriginPath + wcslen(szWinnt));
			wcscpy(szOriginPath, szPath);
			szOriginPath[wcslen(szPath)] = L'\0';
		}
	}
	
	// 如果是短文件名
	if (wcschr(szOriginPath, '~'))
	{
		WCHAR szLongPath[MAX_PATH] = {0};
		DWORD nRet = GetLongPathName(szOriginPath, szLongPath, MAX_PATH);
		if ( !(nRet >= MAX_PATH || nRet == 0) )
		{
			wcscpy(szOriginPath, szLongPath);
			szOriginPath[wcslen(szLongPath)] = L'\0';
		}
	}
}

//
// 枚举驱动
//
BOOL CListDrivers::ListDrivers(vector<DRIVER_INFO> &vectorDrivers)
{
	BOOL bRet = FALSE;
	OPERATE_TYPE opType = enumListDriver;
	ULONG nCnt = 1000;
	PALL_DRIVERS pDriverInfo = NULL;

	vectorDrivers.clear();

	do 
	{
		ULONG nSize = sizeof(ALL_DRIVERS) + nCnt * sizeof(DRIVER_INFO);

		if (pDriverInfo)
		{
			free(pDriverInfo);
			pDriverInfo = NULL;
		}

		pDriverInfo = (PALL_DRIVERS)GetMemory(nSize);
		if (!pDriverInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pDriverInfo, nSize, NULL);
		
		nCnt = pDriverInfo->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pDriverInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pDriverInfo->nCnt; i++)
		{
			FixDriverPath(&pDriverInfo->Drivers[i]);
			vectorDrivers.push_back(pDriverInfo->Drivers[i]);
		}
	}

	if (pDriverInfo)
	{
		free(pDriverInfo);
		pDriverInfo = NULL;
	}

	return bRet;
}

//
// 卸载驱动
//
BOOL CListDrivers::UnLoadDriver(ULONG DriverObject)
{
	BOOL bRet = FALSE;

	if (DriverObject == 0 || DriverObject <= 0x80000000)
	{
		return bRet;
	}

	COMMUNICATE_DRIVER cd;
	cd.OpType = enumUnloadDriver;
	cd.op.Unload.DriverObject = DriverObject;
	bRet = g_ConnectDriver.CommunicateDriver(&cd, sizeof(COMMUNICATE_DRIVER), NULL, 0, NULL);

	return bRet;
}