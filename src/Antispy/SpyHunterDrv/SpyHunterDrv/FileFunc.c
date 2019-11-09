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
#include "FileFunc.h"
#include "Peload.h"
#include "IrpOperate.h"
#include "InitWindows.h"
#include "CommonFunction.h"
#include "..\\..\\Common\\Common.h"

NTSTATUS MzfCreateFile( WCHAR *szFilePath, 
					 PHANDLE FileHandle, 
					 ACCESS_MASK DesiredAccess, 
					 ULONG FileAttributes, 
					 ULONG ShareAccess, 
					 ULONG CreateDisposition, 
					 ULONG CreateOptions 
						)
{
	WCHAR SystemRootName[] = {'\\','S','y','s','t','e','m','R','o','o','t','\0'};
	WCHAR FileNodeName[MAX_PATH] = {0};
	UNICODE_STRING FilePath;
	PDEVICE_OBJECT RealDevice = NULL, DeviceObject = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PFILE_OBJECT FileObject = NULL;
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	
	if (!szFilePath || !FileHandle || !MzfObOpenObjectByPointer || !MzfObfDereferenceObject)
	{
		return status;
	}

	KdPrint(("MzfCreateFile-> file: %S\n", szFilePath));

	// 把\SystemRoot\xxx的文件路径形式，改成Windows\xxx的形式
	if (!_wcsnicmp(szFilePath, SystemRootName, wcslen(SystemRootName)))
	{
		ULONG Len = 0;

		if(!GetWindowsRootName(FileNodeName))
		{
			return status;
		}

		Len = wcslen(SystemRootName);
		wcscat(FileNodeName, &szFilePath[Len]);
	}
	else
	{
		// 或者如果文件带完成路径，例如：c:\1\2.exe, 那么转换成\1\2.exe形式
		if (szFilePath[1] != L':' || szFilePath[2] != L'\\')
		{
			return status;
		}

		wcscpy(FileNodeName, &szFilePath[2]);
	}

	if(!GetDeviceObjectFromFileFullName(szFilePath, &RealDevice, &DeviceObject))
	{
		KdPrint(("get device object and real device object faild\n"));
		return status;
	}

	MzfInitUnicodeString(&FilePath, FileNodeName);
	
	status = IrpCreateFileEx(
		&FilePath, 
		DesiredAccess, 
		FileAttributes, 
		ShareAccess, 
		CreateDisposition, 
		CreateOptions, 
		DeviceObject, 
		RealDevice, 
		&FileObject);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("Irp create file failed\n"));
		return status;
	}
	
	KdPrint(("Irp ret success\n"));

	status = MzfObOpenObjectByPointer(
		FileObject,
		0,    // 此处不能设置为KERNEL_HANDLE,否则应用层关不掉
		NULL,
		DesiredAccess | SYNCHRONIZE,
		*IoFileObjectType,
		KernelMode,
		FileHandle);

	KdPrint(("MzfObOpenObjectByPointer status: 0x%08X, %d\n", status, RtlNtStatusToDosError(status)));

	MzfObfDereferenceObject(FileObject);
	return status;
}

NTSTATUS MzfCreateFileEx(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnNtCreateFile MzfNtCreateFile = (pfnNtCreateFile)GetGlobalVeriable(enumNtCreateFile);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	
	KdPrint(("MzfCreateFileEx\n"));

	if (uInSize == sizeof(COMMUNICATE_FILE_OPERATION) &&
		uOutSize == sizeof(HANDLE) &&
		MzfProbeForRead &&
		MzfNtCreateFile &&
		MzfPsGetCurrentThread)
	{
		PCOMMUNICATE_FILE_OPERATION pCom = (PCOMMUNICATE_FILE_OPERATION)pInBuffer;
		WCHAR *szPath = pCom->op.Create.szFilePath;
		ULONG nPathLen = pCom->op.Create.nPathLen;
		
		KdPrint(("in MzfCreateFileEx\n"));

		__try
		{
			PETHREAD pThread = NULL;
			CHAR PreMode = 0;
			HANDLE hFile = NULL;
			OBJECT_ATTRIBUTES oa;
			UNICODE_STRING unName;
			IO_STATUS_BLOCK  IoStatusBlock;

			MzfProbeForRead(szPath, nPathLen, sizeof(WCHAR));
			
			MzfInitUnicodeString(&unName, szPath);
			InitializeObjectAttributes(&oa, &unName, OBJ_CASE_INSENSITIVE, NULL, NULL);

			pThread = MzfPsGetCurrentThread();
			PreMode = ChangePreMode(pThread);
			
			status = MzfNtCreateFile(&hFile, 
				pCom->op.Create.DesiredAccess,
				&oa, 
				&IoStatusBlock,
				NULL,
				pCom->op.Create.FileAttributes,
				pCom->op.Create.ShareAccess,
				pCom->op.Create.CreateDisposition,
				pCom->op.Create.CreateOptions,
				NULL,
				0);
			
			KdPrint(("status: 0x%08X\n", status));

			RecoverPreMode(pThread, PreMode);

			if (NT_SUCCESS(status))
			{
				*(PHANDLE)pOutBuffer = hFile;
			}
		}
		__except(1)
		{

		}
	}

	return status;
}

NTSTATUS MzfReadFileEx(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	pfnNtReadFile MzfNtReadFile = (pfnNtReadFile)GetGlobalVeriable(enumNtReadFile);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (uInSize == sizeof(COMMUNICATE_FILE_OPERATION) &&
		uOutSize == sizeof(DWORD) &&
		MzfProbeForWrite &&
		MzfNtReadFile &&
		MzfPsGetCurrentThread)
	{
		PCOMMUNICATE_FILE_OPERATION pCom = (PCOMMUNICATE_FILE_OPERATION)pInBuffer;
		HANDLE hFile = pCom->op.Read.hFile;
		ULONG nReadBytes = pCom->op.Read.nReadBytes;
		PVOID pBuffer = pCom->op.Read.pBuffer;

		KdPrint(("in MzfNtReadFile\n"));
		
		if (hFile == NULL ||
			nReadBytes == 0 ||
			pBuffer == NULL)
		{
			return status;
		}

		__try
		{
			PETHREAD pThread = NULL;
			CHAR PreMode = 0;
			IO_STATUS_BLOCK  IoStatusBlock;

			MzfProbeForWrite(pBuffer, nReadBytes, 1);

			pThread = MzfPsGetCurrentThread();
			PreMode = ChangePreMode(pThread);

			status = MzfNtReadFile(hFile, 
				NULL,
				NULL, 
				NULL,
				&IoStatusBlock,
				pBuffer,
				nReadBytes,
				NULL,
				NULL);

			KdPrint(("MzfNtReadFile status: 0x%08X\n", status));

			RecoverPreMode(pThread, PreMode);

			if (NT_SUCCESS(status))
			{
				*(DWORD*)pOutBuffer = IoStatusBlock.Information;
			}
		}
		__except(1)
		{

		}
	}

	return status;
}

NTSTATUS MzfWriteFileEx(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnNtWriteFile MzfNtWriteFile = (pfnNtWriteFile)GetGlobalVeriable(enumNtWriteFile);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (uInSize == sizeof(COMMUNICATE_FILE_OPERATION) &&
		uOutSize == sizeof(DWORD) &&
		MzfProbeForRead &&
		MzfNtWriteFile &&
		MzfPsGetCurrentThread)
	{
		PCOMMUNICATE_FILE_OPERATION pCom = (PCOMMUNICATE_FILE_OPERATION)pInBuffer;
		HANDLE hFile = pCom->op.Write.hFile;
		ULONG nWriteBytes = pCom->op.Write.nWriteBytes;
		PVOID pBuffer = pCom->op.Write.pBuffer;

		KdPrint(("in MzfNtWriteFile\n"));

		if (hFile == NULL ||
			nWriteBytes == 0 ||
			pBuffer == NULL)
		{
			return status;
		}

		__try
		{
			PETHREAD pThread = NULL;
			CHAR PreMode = 0;
			IO_STATUS_BLOCK  IoStatusBlock;

			MzfProbeForRead(pBuffer, nWriteBytes, 1);

			pThread = MzfPsGetCurrentThread();
			PreMode = ChangePreMode(pThread);

			status = MzfNtWriteFile(hFile, 
				NULL,
				NULL, 
				NULL,
				&IoStatusBlock,
				pBuffer,
				nWriteBytes,
				NULL,
				NULL);

			KdPrint(("MzfNtWriteFile status: 0x%08X\n", status));

			RecoverPreMode(pThread, PreMode);

			if (NT_SUCCESS(status))
			{
				*(DWORD*)pOutBuffer = IoStatusBlock.Information;
			}
		}
		__except(1)
		{

		}
	}

	return status;
}