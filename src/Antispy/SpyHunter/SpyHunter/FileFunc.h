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
#ifndef _FILE_FUNCTIONS_H
#define _FILE_FUNCTIONS_H

#include "ConnectDriver.h"

class CFileFunc
{
public:
	CFileFunc(void);
	~CFileFunc(void);

public:
	HANDLE Create(
		WCHAR *szPath, 
		ULONG nPathLen, 
		ACCESS_MASK DesiredAccess,
		ULONG  FileAttributes,
		ULONG  ShareAccess,
		ULONG  CreateDisposition,
		ULONG  CreateOptions
		);
	
	BOOL QueryDirectoryFile(
		__in HANDLE  FileHandle,
		__out PVOID  FileInformation,
		__in ULONG  Length,
		__in ULONG  FileInformationClass,
		__in BOOLEAN  ReturnSingleEntry,
		__in BOOLEAN  RestartScan,
		__out PULONG pWrittenLen
	);

	HANDLE CreateEx( WCHAR *szPath, ULONG nPathLen, ACCESS_MASK DesiredAccess, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions );
	BOOL ReadEx(HANDLE hFile, PVOID pBuffer, ULONG nReadBytes, ULONG *pRetBytes = NULL);
	BOOL WriteEx(HANDLE hFile, PVOID pBuffer, ULONG nWriteBytes, ULONG *pRetBytes = NULL);
private:
	CConnectDriver m_Driver;
};

#endif