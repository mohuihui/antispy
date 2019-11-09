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
#include "ConnectDriver.h"
#include <winioctl.h>

CConnectDriver::CConnectDriver()
{
	m_hDriver = INVALID_HANDLE_VALUE;
}

CConnectDriver::~CConnectDriver()
{
	if (m_hDriver != INVALID_HANDLE_VALUE && m_hDriver != NULL)
	{
		CloseHandle(m_hDriver);
	}
}

// BOOL CConnectDriver::CommunicateDriver(
// 									   PVOID pInData, 
// 									   DWORD nInSize, 
// 									   PVOID pOutData, 
// 									   DWORD nOutSize, 
// 									   DWORD* pRet
// 									   )
// {
// 	BOOL bRet = FALSE;
// 	DWORD dwRet = 0;
// 
// 	if (!pRet)
// 	{
// 		pRet = &dwRet;
// 	}
// 
// 	m_hDriver  = CreateFile( WIN32_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL );
// 	if (m_hDriver != INVALID_HANDLE_VALUE)
// 	{
// 		if (DeviceIoControl( m_hDriver, IOCTL_CONTROL, pInData, nInSize, pOutData, nOutSize, pRet, NULL ))
// 		{
// 			bRet = TRUE;
// 		}
// 
// 		CloseHandle(m_hDriver);
// 		m_hDriver = INVALID_HANDLE_VALUE;
// 	}
// 
// 	return bRet;
// }

BOOL CConnectDriver::CommunicateDriver(
									   PVOID pInData, 
									   DWORD nInSize, 
									   PVOID pOutData, 
									   DWORD nOutSize,
									   DWORD *pdwRet
									   )
{
	BOOL bRet = FALSE;
	DWORD dwRet = 0;
	
	m_hDriver  = CreateFile( szWin32DriverName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL );
	if (m_hDriver != INVALID_HANDLE_VALUE)
	{
		if (DeviceIoControl( m_hDriver, IOCTL_NEITHER_CONTROL, pInData, nInSize, pOutData, nOutSize, &dwRet, NULL ))
		{
			bRet = TRUE;
		}

		CloseHandle(m_hDriver);
		m_hDriver = INVALID_HANDLE_VALUE;
	}

	return bRet;
}