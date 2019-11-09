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
#include "FileOperation.h"
#include "FileFunc.h"
#include "InitWindows.h"
#include "IrpOperate.h"
#include "CommonFunction.h"
#include "DeleteFile.h"
#include "..\\..\\Common\\Common.h"

NTSTATUS CreateFileDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	BOOL bException = FALSE;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	KdPrint(("CreateFileDispatch\n"));

	if (MzfProbeForRead &&
		MzfProbeForWrite &&
		pOutBuffer && 
		uOutSize == sizeof(HANDLE))
	{
		PCOMMUNICATE_FILE pCreate = (PCOMMUNICATE_FILE)pInBuffer;
		WCHAR *szPath = pCreate->op.Create.szPath;
		ULONG nPathLen = pCreate->op.Create.nPathLen;
		
		__try
		{
			MzfProbeForRead(szPath, nPathLen, sizeof(WCHAR));
			MzfProbeForWrite(pOutBuffer, uOutSize, sizeof(HANDLE));
		}
		__except(1)
		{
			KdPrint(("MzfProbe error\n"));
			bException = TRUE;
		}
		
		if (!bException)
		{
			ACCESS_MASK DesiredAccess = pCreate->op.Create.DesiredAccess;
			ULONG  FileAttributes = pCreate->op.Create.FileAttributes;
			ULONG  ShareAccess = pCreate->op.Create.ShareAccess;
			ULONG  CreateDisposition = pCreate->op.Create.CreateDisposition;
			ULONG  CreateOptions = pCreate->op.Create.CreateOptions;
			
			status = MzfCreateFile( 
				szPath, 
				pOutBuffer, 
				DesiredAccess, 
				FileAttributes, 
				ShareAccess, 
				CreateDisposition, 
				CreateOptions );
		}
	}

	return status;
}

NTSTATUS QueryDirectoryFileDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	BOOL bException = FALSE;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	PCOMMUNICATE_FILE pQueryDirectory = (PCOMMUNICATE_FILE)pInBuffer;
	HANDLE hFile = pQueryDirectory->op.QueryDirectoryFile.hDirectory;
	PVOID  FileInformation = pQueryDirectory->op.QueryDirectoryFile.FileInformation;
	ULONG  Length = pQueryDirectory->op.QueryDirectoryFile.Length;
	FILE_INFORMATION_CLASS  FileInformationClass = pQueryDirectory->op.QueryDirectoryFile.FileInformationClass;
	BOOLEAN  ReturnSingleEntry = pQueryDirectory->op.QueryDirectoryFile.ReturnSingleEntry;
	BOOLEAN  RestartScan = pQueryDirectory->op.QueryDirectoryFile.RestartScan;
	PULONG pWrittenLen = pQueryDirectory->op.QueryDirectoryFile.pWrittenLen;


	KdPrint(("QueryDirectoryFileDispatch\n"));

	if ( MzfProbeForWrite &&
		hFile &&
		FileInformation &&
		Length > 0 &&
		pWrittenLen)
	{
		__try
		{
			MzfProbeForWrite(FileInformation, Length , 1);
			MzfProbeForWrite(pWrittenLen, sizeof(ULONG) , 1);
		}
		__except(1)
		{
			KdPrint(("MzfProbe error\n"));
			bException = TRUE;
		}

		if (!bException)
		{
			status = IrpQueryDirectoryFileEx( 
				hFile, 
				FileInformation, 
				Length, 
				FileInformationClass, 
				ReturnSingleEntry, 
				RestartScan,
				pWrittenLen
				);
		}
	}

	return status;
}

BOOL GetFileObjectTypeIndex()
{
	BOOL bRet = FALSE;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING RootName;
	WCHAR szSystemRoot[] = {'\\', 'S', 'y', 's', 't', 'e', 'm', 'R', 'o', 'o', 't', '\\','\0'};
	IO_STATUS_BLOCK isb;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	HANDLE hFile = NULL;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnIoCreateFile MzfIoCreateFile = (pfnIoCreateFile)GetGlobalVeriable(enumIoCreateFile);
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnPsGetCurrentProcessId MzfPsGetCurrentProcessId = (pfnPsGetCurrentProcessId)GetGlobalVeriable(enumPsGetCurrentProcessId);

	if (!MzfPsGetCurrentProcessId ||
		!MzfNtQuerySystemInformation ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!MzfPsGetCurrentThread ||
		!MzfNtClose ||
		!MzfIoCreateFile)
	{
		return FALSE;
	}

	MzfInitUnicodeString(&RootName, szSystemRoot);
	InitializeObjectAttributes(&oa, &RootName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = ZwCreateFile(&hFile, 0x80000000, &oa, &isb, 0, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, 0, 0);
// 	status = MzfIoCreateFile(
// 		&hFile,
// 		SYNCHRONIZE,
// 		&oa,
// 		&isb,
// 		NULL,
// 		0,
// 		FILE_SHARE_READ | FILE_SHARE_WRITE,
// 		FILE_OPEN,
// 		FILE_DIRECTORY_FILE,
// 		0,
// 		0,
// 		0,
// 		0,
// 		IO_NO_PARAMETER_CHECKING);

	if (NT_SUCCESS(status))
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		ULONG ulRet = PAGE_SIZE;

		do 
		{
			PVOID pBuffer = MzfExAllocatePoolWithTag(PagedPool, ulRet, MZFTAG);
			if (pBuffer)
			{
				memset(pBuffer, 0, ulRet);
				status = MzfNtQuerySystemInformation(SystemHandleInformation, pBuffer, ulRet, &ulRet);
				if (NT_SUCCESS(status))
				{
					PSYSTEM_HANDLE_INFORMATION pHandles = (PSYSTEM_HANDLE_INFORMATION)pBuffer;
					ULONG i = 0;
					HANDLE hCurrentProcess = MzfPsGetCurrentProcessId();

					for (i = 0; i < pHandles->NumberOfHandles; i++)
					{
						if (hCurrentProcess == (HANDLE)pHandles->Handles[i].UniqueProcessId ||
							(HANDLE)pHandles->Handles[i].HandleValue == hFile)
						{
							bRet = TRUE;
							KdPrint(("File object index: %d\n", pHandles->Handles[i].ObjectTypeIndex));
							SetGlobalVeriable(enumFileTypeIndex, pHandles->Handles[i].ObjectTypeIndex);
							break;
						}
					}
				}

				MzfExFreePoolWithTag(pBuffer, 0);
				pBuffer = NULL;
				ulRet *= 2;
			}
		} while (status == STATUS_INFO_LENGTH_MISMATCH);

		MzfNtClose(hFile);
		RecoverPreMode(pThread, PreMode);
	}

	return bRet;
}

WCHAR* GetFilePathByFileObject(PFILE_OBJECT pFileObject)
{
	POBJECT_NAME_INFORMATION pBuffer = NULL;
	WCHAR *pRet = NULL;
	DWORD dwBytes = 0;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnObQueryNameString MzfObQueryNameString = (pfnObQueryNameString)GetGlobalVeriable(enumObQueryNameString);

	if (!pFileObject || !pFileObject->DeviceObject)
	{
		return pRet;
	}
	
	pBuffer = (POBJECT_NAME_INFORMATION)MzfExAllocatePoolWithTag(PagedPool, 0x1000, MZFTAG);

	if (pBuffer)
	{
		WCHAR szHarddiskVolume[] = {L'\\', L'D', L'e', L'v', L'i', L'c', L'e', L'\\', L'H', L'a', L'r', L'd', L'd', L'i', L's', L'k', L'V', L'o', L'l', L'u', L'm', L'e', '\0'};
		DWORD uRet = 0;
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		// ObQueryNameString : \Device\HarddiskVolume1\Program Files\VMware\VMware Tools\VMwareTray.exe
		memset(pBuffer, 0, 0x1000);
		status = MzfObQueryNameString(pFileObject->DeviceObject, pBuffer, 0x1000, &uRet);

		if (NT_SUCCESS(status) && 
			pBuffer->Name.Length / sizeof(WCHAR) > wcslen(szHarddiskVolume) &&
			!_wcsnicmp(pBuffer->Name.Buffer, szHarddiskVolume, wcslen(szHarddiskVolume)))
		{
			dwBytes = pBuffer->Name.Length + pFileObject->FileName.Length + sizeof(WCHAR);
			
			pRet = (WCHAR*)MzfExAllocatePoolWithTag(NonPagedPool, dwBytes, MZFTAG);
			if (pRet)
			{
				memset(pRet, 0, dwBytes);
				wcsncpy(pRet, pBuffer->Name.Buffer, pBuffer->Name.Length / sizeof(WCHAR));

				if (pFileObject->FileName.Length > 0 && pFileObject->FileName.Buffer)
				{
					wcsncat(pRet, pFileObject->FileName.Buffer, pFileObject->FileName.Length / sizeof(WCHAR));
				}

			//	KdPrint(("file: %S\n", pRet));
			}
		}

		MzfExFreePoolWithTag(pBuffer, 0);
	}

	return pRet;
}

NTSTATUS QueryFileLockInformation(WCHAR *szDosPath, 
								  ULONG nDosPathLen, 
								  BOOL bDirectory, 
								  PLOCKED_FILES pLockFiles, 
								  ULONG nCanStore, 
								  ULONG *pnRetCnt
								  )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSYSTEM_HANDLE_INFORMATION pHandles = NULL;
	ULONG nHandles = 0;
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObReferenceObjectByPointer MzfObReferenceObjectByPointer = (pfnObReferenceObjectByPointer)GetGlobalVeriable(enumObReferenceObjectByPointer);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	ULONG i = 0, j = 0;
	ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);
	POBJECT_TYPE pFileObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoFileObjectType);
	
	if (!szDosPath || !nDosPathLen ||!pLockFiles || !nCanStore || !MzfExFreePoolWithTag || !MzfMmIsAddressValid ||
		!MzfObReferenceObjectByPointer || !MzfObfDereferenceObject)
	{
		return status;
	}

	pHandles = (PSYSTEM_HANDLE_INFORMATION)MyQuerySystemInformation(SystemHandleInformation);
	if (!pHandles)
	{
		return status;
	}

	nHandles = pHandles->NumberOfHandles;
	if (!nHandles)
	{
		MzfExFreePoolWithTag(pHandles, 0);
		pHandles = NULL;
		return status;
	}

 	KdPrint(("szDosPath: %S, nDosPathLen: %d, nCanStore: %d, nHandles: %d\n", szDosPath, nDosPathLen, nCanStore, nHandles));

	for (i = 0; i < nHandles; i++)
	{
		SYSTEM_HANDLE_TABLE_ENTRY_INFO info = pHandles->Handles[i];
		PFILE_OBJECT pFileObject = info.Object;
		
		if (nBuildNumber >= 6000)
		{
			pFileObject = (PFILE_OBJECT)((ULONG)pFileObject & 0xFFFFFFF8);
		}

		if ((ULONG)pFileObject > SYSTEM_ADDRESS_START && 
			MzfMmIsAddressValid(pFileObject) &&
			KeGetObjectType(pFileObject) == pFileObjectType &&
			(ULONG)pFileObject->DeviceObject > SYSTEM_ADDRESS_START &&
			MzfMmIsAddressValid(pFileObject->DeviceObject))
		{
			if (NT_SUCCESS(ObReferenceObjectByPointer(pFileObject, 0, *IoFileObjectType, KernelMode)))
			{
				WCHAR *szPath = GetFilePathByFileObject(pFileObject);
				if (szPath)
				{
					BOOL bOk = FALSE;
					KdPrint(("path: %S\n", szPath));

					// 如果是文件夹,那么只比较文件夹部分就好了
					if (bDirectory)
					{
						if (wcslen(szPath) * sizeof(WCHAR) >= nDosPathLen && // 路径要比文件夹的路径长
							!_wcsnicmp(szDosPath, szPath, nDosPathLen / sizeof(WCHAR)))
						{
							bOk = TRUE;
						}
					}

					// 如果是文件,那么必须匹配全部路径
					else 
					{
						// 如果是文件,那么路径要一样
						if (wcslen(szPath) * sizeof(WCHAR) == nDosPathLen && 
							!_wcsnicmp(szDosPath, szPath, nDosPathLen / sizeof(WCHAR)))
						{
							bOk = TRUE;
						}
					}

					if (bOk)
					{
						if (j < nCanStore)
						{
							DWORD nLen = wcslen(szPath);
							if (nLen >= MAX_PATH * 2)
							{
								nLen = MAX_PATH * 2 - 1;
							}

							KdPrint(("nHandle: 0x%08X, nPid: %d\n", info.HandleValue, info.UniqueProcessId));
							pLockFiles[j].nHandle = info.HandleValue;
							pLockFiles[j].nPid = info.UniqueProcessId;
							wcsncpy(pLockFiles[j].szFilePath, szPath, nLen);
						}

						j++;
					}

					ExFreePoolWithTag(szPath, 0);
				}

				ObDereferenceObject(pFileObject);
			}
		}
	}

	if (pnRetCnt)
	{
		*pnRetCnt = j;
	}

	if (j < nCanStore)
	{
		status = STATUS_SUCCESS;
	}
	else
	{
		status = STATUS_INFO_LENGTH_MISMATCH;
	}

	ExFreePoolWithTag(pHandles, 0);
	pHandles = NULL;
	KdPrint(("end\n"));
	return status;
}

// 查询文件锁定情况
NTSTATUS QueryFileLockInformationDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	BOOL bException = FALSE;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_FILE pQueryLock = (PCOMMUNICATE_FILE)pInBuffer;
	WCHAR *szDosPath = pQueryLock->op.QueryFileLock.szPath;
	BOOL bDirectory = pQueryLock->op.QueryFileLock.bDirectory;
	ULONG nLen = pQueryLock->op.QueryFileLock.nPathLen;

	KdPrint(("QueryFileLockInformationDispatch\n"));

	if ( MzfProbeForRead &&
		MzfProbeForWrite &&
		szDosPath &&
		nLen &&
		pOutBuffer && 
		uOutSize > sizeof(LOCKED_FILES))
	{
		__try
		{
			MzfProbeForRead(szDosPath, nLen , 1);
			MzfProbeForWrite(pOutBuffer, uOutSize, 1);
		}
		__except(1)
		{
			KdPrint(("MzfProbe error\n"));
			bException = TRUE;
		}

		if (!bException)
		{
			status = QueryFileLockInformation(
				szDosPath, 
				nLen, 
				bDirectory, 
				((PLOCKED_FILE_INFO)pOutBuffer)->files,
				(uOutSize - sizeof(DWORD)) / sizeof(LOCKED_FILES),
				&((PLOCKED_FILE_INFO)pOutBuffer)->nRetCount);

			KdPrint(("status: 0x%08X, %d\n", status, RtlNtStatusToDosError(status)));
		}
	}
	else
	{
		KdPrint(("fuck error\n"));
	}

	return status;
}

NTSTATUS PspCloseHandleDispatch(ULONG handle, PEPROCESS pEprocess)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnNtQueryObject MzfNtQueryObject = (pfnNtQueryObject)GetGlobalVeriable(enumNtQueryObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	PEPROCESS pSystemProcess = (PEPROCESS)GetGlobalVeriable(enumSystemProcess);
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
	BOOL bAttach = FALSE;
	KAPC_STATE as;
	PHANDLE_INFO pHi = NULL;
	ULONG hHandle = handle;
	PVOID pObject = NULL;
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	OBJECT_DATA_INFORMATION odi;
	DWORD dwRet = 0;

	if (!pSystemProcess ||
		!MzfMmIsAddressValid || 
		!MzfKeStackAttachProcess ||
		!MzfKeUnstackDetachProcess ||
		!MzfIoGetCurrentProcess ||
		!MzfNtClose ||
		!MzfNtQueryObject ||
		!MzfPsGetCurrentThread)
	{
		return status;
	}

	if ( Version == enumWINDOWS_7 || Version == enumWINDOWS_8 )
	{
		if (pEprocess == pSystemProcess)
		{
			hHandle |= 0x80000000;
		}
	}

	if (MzfIoGetCurrentProcess() != pEprocess)
	{
		MzfKeStackAttachProcess(pEprocess, &as);
		bAttach = TRUE;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	if (NT_SUCCESS(MzfNtQueryObject((HANDLE)hHandle, ObjectDataInfo, &odi, sizeof(OBJECT_DATA_INFORMATION), &dwRet)))
	{
		if (odi.ProtectFromClose)
		{
			pfnNtSetInformationObject MzfNtSetInformationObject = (pfnNtSetInformationObject)GetGlobalVeriable(enumNtSetInformationObject);
			if (MzfNtSetInformationObject)
			{
				odi.ProtectFromClose = FALSE;
				status = MzfNtSetInformationObject((HANDLE)hHandle, HandleFlagInformation, &odi, sizeof(OBJECT_DATA_INFORMATION));
				if (NT_SUCCESS(status))
				{
					KdPrint(("force kill handle in MzfNtSetInformationObject\n"));
					status = MzfNtClose((HANDLE)hHandle);
				}
			}
		}
		else
		{
			status = MzfNtClose((HANDLE)hHandle);
		}
	}

	RecoverPreMode(pThread, PreMode);

	if (bAttach)
	{
		MzfKeUnstackDetachProcess(&as);
		bAttach = FALSE;
	}

	return status;
}

NTSTATUS CloseFileHandleDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_FILE pCh = (PCOMMUNICATE_FILE)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = pCh->op.CloseHandle.nPid;
	ULONG nHandle = pCh->op.CloseHandle.nHandle;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}

	pEprocess = (PEPROCESS)pCh->op.CloseHandle.pEprocess;
	TempStatus = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);

	if (pEprocess && IsRealProcess(pEprocess))
	{
		status = PspCloseHandleDispatch(nHandle, pEprocess);
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

NTSTATUS SetFileNormalAttributeDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{	
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_FILE pCh = (PCOMMUNICATE_FILE)pInBuffer;
	HANDLE hFile = pCh->op.SetFileAttribute.hFile;
	ULONG Attribute = pCh->op.SetFileAttribute.Attribute;

	if (hFile)
	{
		status = MzfSetFileAttributes(hFile, Attribute);
	}

	return status;
}