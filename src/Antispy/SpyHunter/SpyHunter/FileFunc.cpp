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
#include "FileFunc.h"
#include "..\\..\\Common\Common.h"

CFileFunc::CFileFunc(void)
{

}

CFileFunc::~CFileFunc(void)
{

}

HANDLE CFileFunc::Create(
			  WCHAR *szPath, 
			  ULONG nPathLen, 
			  ACCESS_MASK DesiredAccess,
			  ULONG  FileAttributes,
			  ULONG  ShareAccess,
			  ULONG  CreateDisposition,
			  ULONG  CreateOptions
			  )
{
	HANDLE hFile = NULL;

	if (szPath && nPathLen <= MAX_PATH * sizeof(WCHAR))
	{
		COMMUNICATE_FILE cf;
		cf.OpType = enumCreateFile;
		cf.op.Create.szPath = szPath;
		cf.op.Create.nPathLen = nPathLen;
		cf.op.Create.DesiredAccess = DesiredAccess;
		cf.op.Create.FileAttributes = FileAttributes;
		cf.op.Create.ShareAccess = ShareAccess;
		cf.op.Create.CreateDisposition = CreateDisposition;
		cf.op.Create.CreateOptions = CreateOptions;

		if (!m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), &hFile, sizeof(HANDLE), NULL))
		{
			hFile = NULL;
		}
	}

	return hFile;
}

HANDLE CFileFunc::CreateEx(
						 WCHAR *szPath, 
						 ULONG nPathLen, 
						 ACCESS_MASK DesiredAccess,
						 ULONG  FileAttributes,
						 ULONG  ShareAccess,
						 ULONG  CreateDisposition,
						 ULONG  CreateOptions
						 )
{
	HANDLE hFile = NULL;

	if (szPath && nPathLen <= MAX_PATH * sizeof(WCHAR))
	{
		COMMUNICATE_FILE_OPERATION cf;
		cf.OpType = enumCreateFileEx;
		cf.op.Create.szFilePath = szPath;
		cf.op.Create.nPathLen = nPathLen;
		cf.op.Create.DesiredAccess = DesiredAccess;
		cf.op.Create.FileAttributes = FileAttributes;
		cf.op.Create.ShareAccess = ShareAccess;
		cf.op.Create.CreateDisposition = CreateDisposition;
		cf.op.Create.CreateOptions = CreateOptions;

		if (!m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE_OPERATION), &hFile, sizeof(HANDLE), NULL))
		{
			hFile = NULL;
		}
	}

	return hFile;
}

BOOL CFileFunc::QueryDirectoryFile(
						__in HANDLE  FileHandle,
						__out PVOID  FileInformation,
						__in ULONG  Length,
						__in ULONG  FileInformationClass,
						__in BOOLEAN  ReturnSingleEntry,
						__in BOOLEAN  RestartScan,
						__out PULONG pWrittenLen
						)
{
	if (!FileHandle || !FileInformation || Length <= 0 || !pWrittenLen)
	{
		return FALSE;
	}

	COMMUNICATE_FILE cf;
	cf.OpType = enumQueryDirectoryFile;
	cf.op.QueryDirectoryFile.hDirectory = FileHandle;
	cf.op.QueryDirectoryFile.FileInformation = FileInformation;
	cf.op.QueryDirectoryFile.Length = Length;
	cf.op.QueryDirectoryFile.FileInformationClass = FileInformationClass;
	cf.op.QueryDirectoryFile.ReturnSingleEntry = ReturnSingleEntry;
	cf.op.QueryDirectoryFile.RestartScan = RestartScan;
	cf.op.QueryDirectoryFile.pWrittenLen = pWrittenLen;

	return m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), NULL, 0, NULL);
}

BOOL CFileFunc::ReadEx(HANDLE hFile, PVOID pBuffer, ULONG nReadBytes, ULONG *pRetBytes)
{
	BOOL bRet = FALSE;
	
	if (hFile == NULL ||
		pBuffer == NULL ||
		nReadBytes == 0)
	{
		return bRet;
	}

	DWORD dwRet = 0;
	COMMUNICATE_FILE_OPERATION cf;
	cf.OpType = enumReadFileEx;
	cf.op.Read.hFile = hFile;
	cf.op.Read.pBuffer = pBuffer;
	cf.op.Read.nReadBytes = nReadBytes;

	if (m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE_OPERATION), &dwRet, sizeof(DWORD), NULL))
	{
		bRet = TRUE;
		if (pRetBytes)
		{
			*pRetBytes = dwRet;
		}
	}

	return bRet;
}

BOOL CFileFunc::WriteEx(HANDLE hFile, PVOID pBuffer, ULONG nWriteBytes, ULONG *pRetBytes)
{
	BOOL bRet = FALSE;

	if (hFile == NULL ||
		pBuffer == NULL ||
		nWriteBytes == 0)
	{
		return bRet;
	}

	DWORD dwRet = 0;
	COMMUNICATE_FILE_OPERATION cf;
	cf.OpType = enumWriteFileEx;
	cf.op.Write.hFile = hFile;
	cf.op.Write.pBuffer = pBuffer;
	cf.op.Write.nWriteBytes = nWriteBytes;

	if (m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE_OPERATION), &dwRet, sizeof(DWORD), NULL))
	{
		bRet = TRUE;
		if (pRetBytes)
		{
			*pRetBytes = dwRet;
		}
	}

	return bRet;
}