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
#include "ProcessHandle.h"
#include "InitWindows.h"
#include "Process.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"

//////////////////////////////////////////////////////////////////// 

VOID GetHandleObjectName(HANDLE handle, ULONG Object, WCHAR* szObjectName)
{
	PVOID pHandleName = NULL;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnNtQueryObject MzfNtQueryObject = (pfnNtQueryObject)GetGlobalVeriable(enumNtQueryObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfExAllocatePoolWithTag ||
		!MzfNtQueryObject ||
		!MzfPsGetCurrentThread ||
		!MzfExFreePoolWithTag ||
		!MzfMmIsAddressValid)
	{
		return;
	}

	pHandleName = MzfExAllocatePoolWithTag(PagedPool, 0x1000, MZFTAG);
	if (pHandleName)
	{
		ULONG uRet= 0;
		PETHREAD pThread = NULL;
		CHAR PreMode = 0;

		memset(pHandleName, 0, 0x1000);

		pThread = MzfPsGetCurrentThread();
		PreMode = ChangePreMode(pThread);

		__try
		{
			if (NT_SUCCESS(MzfNtQueryObject((HANDLE)handle, ObjectNameInfo, pHandleName, 0x1000, &uRet)))
			{
				POBJECT_NAME_INFORMATION pOni = (POBJECT_NAME_INFORMATION)pHandleName;
				if (IsUnicodeStringValid(&pOni->Name))
				{
					ULONG nLen = CmpAndGetStringLength(&pOni->Name, MAX_PATH);
					wcsncpy(szObjectName, pOni->Name.Buffer, nLen);
				}
			}
		}
		__except(1)
		{
			KdPrint(("GetHandleObjectName catch __except\n"));
		}
		
		RecoverPreMode(pThread, PreMode);
		MzfExFreePoolWithTag(pHandleName, 0);
	}
}

VOID GetHandleTypeName(HANDLE handle, ULONG Object, WCHAR* szTypeName)
{
	PVOID pHandleName = NULL;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnNtQueryObject MzfNtQueryObject = (pfnNtQueryObject)GetGlobalVeriable(enumNtQueryObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfExAllocatePoolWithTag ||
		!MzfNtQueryObject ||
		!MzfPsGetCurrentThread ||
		!MzfExFreePoolWithTag ||
		!MzfMmIsAddressValid)
	{
		return;
	}

	pHandleName = MzfExAllocatePoolWithTag(PagedPool, 0x1000, MZFTAG);
	if (pHandleName)
	{
		ULONG uRet= 0;
		PETHREAD pThread = NULL;
		CHAR PreMode = 0;

		memset(pHandleName, 0, 0x1000);

		pThread = MzfPsGetCurrentThread();
		PreMode = ChangePreMode(pThread);

		__try
		{
			if (NT_SUCCESS(MzfNtQueryObject((HANDLE)handle, ObjectTypeInfo, pHandleName, 0x1000, &uRet)))
			{
				POBJECT_TYPE_INFORMATION pOni = (POBJECT_TYPE_INFORMATION)pHandleName;
				if (IsUnicodeStringValid(&pOni->TypeName))
				{
					ULONG nLen = CmpAndGetStringLength(&pOni->TypeName, MAX_OBJECT_NAME);
					wcsncpy(szTypeName, pOni->TypeName.Buffer, nLen);
				}
			}
		}
		__except(1)
		{
			KdPrint(("GetHandleTypeName catch __except\n"));
		}

		RecoverPreMode(pThread, PreMode);
		MzfExFreePoolWithTag(pHandleName, 0);
	}
}

VOID InsertHandleToList(PEPROCESS pEprocess, HANDLE handle, ULONG Object, PALL_HANDLES pHandles)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	PEPROCESS pSystemProcess = (PEPROCESS)GetGlobalVeriable(enumSystemProcess);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
	BOOL bAttach = FALSE;
	KAPC_STATE as;
	PHANDLE_INFO pHi = NULL;
	ULONG hHandle = (ULONG)handle;

	if (!pSystemProcess ||
		!MzfMmIsAddressValid || 
		!MzfExAllocatePoolWithTag || 
		!MzfExFreePoolWithTag ||
		!MzfKeStackAttachProcess ||
		!MzfKeUnstackDetachProcess ||
		!MzfIoGetCurrentProcess)
	{
		return;
	}

	if (Object && 
		MzfMmIsAddressValid((PVOID)Object) && 
		(pHi = (PHANDLE_INFO)MzfExAllocatePoolWithTag(NonPagedPool, sizeof(HANDLE_INFO), MZFTAG)) != NULL)
	{
		memset(pHi, 0, sizeof(HANDLE_INFO));

		if (MzfIoGetCurrentProcess() != pEprocess)
		{
			MzfKeStackAttachProcess(pEprocess, &as);
			bAttach = TRUE;
		}
		
		pHi->Handle = (ULONG)handle;
		pHi->Object = Object;
		if (MzfMmIsAddressValid((PVOID)(Object - 0x18)))
		{
			pHi->ReferenceCount = *(PULONG)(Object - 0x18);
		}
		else
		{
			pHi->ReferenceCount = 0;
		}
		
		if ( Version == enumWINDOWS_7 || Version == enumWINDOWS_8 )
		{
			if (pEprocess == pSystemProcess)
			{
				hHandle |= 0x80000000;	// 内核句柄
			}
		}

		GetHandleObjectName((HANDLE)hHandle, Object, pHi->HandleName);
		GetHandleTypeName((HANDLE)hHandle, Object, pHi->ObjectName);

		if (bAttach)
		{
			MzfKeUnstackDetachProcess(&as);
			bAttach = FALSE;
		}

		memcpy(&pHandles->Handles[pHandles->nCnt], pHi, sizeof(HANDLE_INFO));
		MzfExFreePoolWithTag(pHi, 0);
		pHi = NULL;
	}
}

NTSTATUS GetHandles(DWORD dwPid, DWORD pEprocess, PALL_HANDLES pOutHandles, ULONG nCnt)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG ulRet= 0x10000;
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);

	if (!MzfExAllocatePoolWithTag || 
		!MzfNtQuerySystemInformation ||
		!MzfPsGetCurrentThread ||
		!MzfExFreePoolWithTag)
	{
		return status;
	}
	
	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

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
				for (i = 0; i < pHandles->NumberOfHandles; i++)
				{
					if (dwPid == pHandles->Handles[i].UniqueProcessId)
					{
						if (nCnt > pOutHandles->nCnt)
						{
							InsertHandleToList((PEPROCESS)pEprocess, 
								(HANDLE)pHandles->Handles[i].HandleValue, 
								(ULONG)pHandles->Handles[i].Object, 
								pOutHandles);
						}

						pOutHandles->nCnt++;
					}
				}
			}

			MzfExFreePoolWithTag(pBuffer, 0);
			ulRet *= 2;
		}
	} while (status == STATUS_INFO_LENGTH_MISMATCH);

	RecoverPreMode(pThread, PreMode);

	if (NT_SUCCESS(status))
	{
		if (nCnt >= pOutHandles->nCnt)
		{
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	return status;
}

//////////////////////////////////////////////////////////////////// 

//
// 枚举进程句柄
//
NTSTATUS ListProcessHandle(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_HANDLE pCh = (PCOMMUNICATE_HANDLE)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0;
	ULONG nCnt = (uOutSize - sizeof(ALL_HANDLES)) / sizeof(HANDLE_INFO);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_HANDLE) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_HANDLES))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pEprocess = (PEPROCESS)pCh->pEprocess;
	nPid = pCh->nPid;

	if (nPid)
	{
		TempStatus = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);
		if (NT_SUCCESS(TempStatus) && !pEprocess)
		{
			pEprocess = pTempEprocess;
		}
	}
	
	if (!pEprocess)
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (IsRealProcess(pEprocess))
	{
		status = GetHandles(nPid, (DWORD)pEprocess, (PALL_HANDLES)pOutBuffer, nCnt);
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

NTSTATUS PspCloseHandle(ULONG handle, ULONG pHandleObject, ULONG bForceKill, PEPROCESS pEprocess)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
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

	if (!pSystemProcess ||
		!MzfMmIsAddressValid || 
		!MzfKeStackAttachProcess ||
		!MzfKeUnstackDetachProcess ||
		!MzfIoGetCurrentProcess ||
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject ||
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

	status = MzfObReferenceObjectByHandle((HANDLE)hHandle, 0x80000000, NULL, KernelMode, &pObject, NULL);
	if (NT_SUCCESS(status))
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		OBJECT_DATA_INFORMATION odi;
		DWORD dwRet = 0;

		if (pObject == (PVOID)pHandleObject &&
			NT_SUCCESS(MzfNtQueryObject((HANDLE)hHandle, ObjectDataInfo, &odi, sizeof(OBJECT_DATA_INFORMATION), &dwRet)))
		{
			if (bForceKill)
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
			else
			{
				if (!odi.ProtectFromClose)
				{
					status = MzfNtClose((HANDLE)hHandle);
				}
			}
		}

		RecoverPreMode(pThread, PreMode);
		MzfObfDereferenceObject(pObject);
	}

	if (bAttach)
	{
		MzfKeUnstackDetachProcess(&as);
		bAttach = FALSE;
	}

	return status;
}

//
// 杀句柄
//
NTSTATUS KillHandle(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_HANDLE pCh = (PCOMMUNICATE_HANDLE)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0, handle = 0, pHandleObject = 0, bForceKill = 0;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_HANDLE))
	{
		return STATUS_INVALID_PARAMETER;
	}

	nPid = pCh->nPid;
	pEprocess = (PEPROCESS)pCh->pEprocess;
	handle = pCh->op.Close.hHandle;
	pHandleObject = pCh->op.Close.pHandleObject;
	bForceKill = pCh->op.Close.bForceKill;
	
	if (!handle || !pHandleObject)
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (nPid)
	{
		TempStatus = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);
		if (NT_SUCCESS(TempStatus) && !pEprocess)
		{
			pEprocess = pTempEprocess;
		}
	}
	
	if (IsRealProcess(pEprocess))
	{
		status = PspCloseHandle(handle, pHandleObject, bForceKill, pEprocess);
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

NTSTATUS GetKeyObjectType()
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES objectAttributes; 
	UNICODE_STRING unObjectTypes; 
	HANDLE handle;
	PVOID Object = NULL;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	WCHAR szObjectTypes[] = {L'\\', L'O', L'b', L'j', L'e', L'c', L't', L'T', L'y', L'p', L'e', L's', L'\0'};
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	pfnNtOpenDirectoryObject MzfNtOpenDirectoryObject = (pfnNtOpenDirectoryObject)GetGlobalVeriable(enumNtOpenDirectoryObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	WCHAR szKey[] = {'K','e','y','\0'};

	if (!MzfNtOpenDirectoryObject || 
		!MzfPsGetCurrentThread || 
		!MzfNtClose || 
		!MzfMmIsAddressValid || 
		!MzfObReferenceObjectByHandle || 
		!MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}

	MzfInitUnicodeString(&unObjectTypes, szObjectTypes);
	InitializeObjectAttributes(&objectAttributes, &unObjectTypes, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	ntStatus = MzfNtOpenDirectoryObject(&handle, 0, &objectAttributes);
	if ( NT_SUCCESS(ntStatus) )
	{
		ntStatus = MzfObReferenceObjectByHandle(handle, 0, 0, 0, &Object, 0);
		if ( NT_SUCCESS(ntStatus) )
		{
			int i = 0;
			POBJECT_DIRECTORY pOd = NULL;

			MzfObfDereferenceObject(Object);
			pOd = (POBJECT_DIRECTORY)Object;

			__try
			{
				for (i = 0; i < NUMBER_HASH_BUCKETS; i++)
				{
					POBJECT_DIRECTORY_ENTRY pode = pOd->HashBuckets[i];
					for (; pode; pode = pode->ChainLink)
					{
						ULONG nChars = 0;
						WCHAR *szTypeName = NULL;

						if (MzfMmIsAddressValid(pode->Object))
						{
							switch (WinVersion)
							{
							case enumWINDOWS_2K:
							case enumWINDOWS_XP:
							case enumWINDOWS_2K3:
							case enumWINDOWS_2K3_SP1_SP2:
							case enumWINDOWS_VISTA:
								{
									POBJECT_TYPE_XP pOt = (POBJECT_TYPE_XP)pode->Object;
									KdPrint(("Name: %wZ, Object: 0x%08X\n", &pOt->Name, pode->Object));
									nChars = pOt->Name.Length / sizeof(WCHAR);
									szTypeName = pOt->Name.Buffer;
								}
								break;

							case enumWINDOWS_VISTA_SP1_SP2:
							case enumWINDOWS_7:
							case enumWINDOWS_8:
								{
									POBJECT_TYPE_2008ABOVE pOt = (POBJECT_TYPE_2008ABOVE)pode->Object;
									KdPrint(("Name: %wZ, Object: 0x%08X\n", &pOt->Name, pode->Object));
									nChars = pOt->Name.Length / sizeof(WCHAR);
									szTypeName = pOt->Name.Buffer;
								}
								break;
							}

							if (nChars == wcslen(szKey) &&
								!_wcsnicmp(szKey, szTypeName, nChars))
							{
								SetGlobalVeriable(enumKeyType, (ULONG)pode->Object);
							}
						}
					}
				}
			}
			__except(1)
			{
				KdPrint(("GetAllObjectTypes catch __except\n"));
			}
		}

		ntStatus = MzfNtClose(handle);
	}

	RecoverPreMode(pThread, PreMode);

	return ntStatus;
}

NTSTATUS QueryLockedRegistryHandle(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSYSTEM_HANDLE_INFORMATION pHandles = NULL;
	ULONG nHandles = 0;
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObReferenceObjectByPointer MzfObReferenceObjectByPointer = (pfnObReferenceObjectByPointer)GetGlobalVeriable(enumObReferenceObjectByPointer);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	ULONG i = 0, j = 0, nCanStore = 0;
	ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);
	POBJECT_TYPE pKeyObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumKeyType);
	PEPROCESS pSystemProcess = (PEPROCESS)GetGlobalVeriable(enumSystemProcess);
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
	PCOMMUNICATE_REG_LOCK pQueryLock = (PCOMMUNICATE_REG_LOCK)pInBuffer;
	WCHAR *szDosPath = pQueryLock->szRegPath;
	ULONG nLen = pQueryLock->nPathLen;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	PLOCKED_REG_INFO pOutLockRegInfo = (PLOCKED_REG_INFO)pOutBuffer;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);

	if (pKeyObjectType == NULL)
	{
		GetKeyObjectType();
		pKeyObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumKeyType);
	}

	nCanStore = (uOutSize - sizeof(DWORD)) / sizeof(LOCKED_REG_INFO);

	if (!szDosPath || !nLen ||!uOutSize || !MzfExFreePoolWithTag || !MzfMmIsAddressValid ||
		!MzfObReferenceObjectByPointer || !MzfObfDereferenceObject || !MzfProbeForRead || !MzfProbeForWrite ||
		pKeyObjectType == NULL || !nCanStore || !MzfPsLookupProcessByProcessId ||
		!MzfKeStackAttachProcess || !MzfKeUnstackDetachProcess || !MzfIoGetCurrentProcess)
	{
		return status;
	}

	__try
	{
		MzfProbeForRead(szDosPath, nLen , 1);
		MzfProbeForWrite(pOutBuffer, uOutSize, 1);
	}
	__except(1)
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

	KdPrint(("szDosPath: %S, nHandles: %d\n", szDosPath, nHandles));

	for (i = 0; i < nHandles; i++)
	{
		SYSTEM_HANDLE_TABLE_ENTRY_INFO info = pHandles->Handles[i];
		PFILE_OBJECT pKeyObject = info.Object;

		if (nBuildNumber >= 6000)
		{
			pKeyObject = (PFILE_OBJECT)((ULONG)pKeyObject & 0xFFFFFFF8);
		}

		if ((ULONG)pKeyObject > SYSTEM_ADDRESS_START && 
			MzfMmIsAddressValid(pKeyObject) &&
			KeGetObjectType(pKeyObject) == pKeyObjectType &&
			(ULONG)pKeyObject->DeviceObject > SYSTEM_ADDRESS_START &&
			MzfMmIsAddressValid(pKeyObject->DeviceObject))
		{
			if (NT_SUCCESS(MzfObReferenceObjectByPointer(pKeyObject, 0, NULL, KernelMode)))
			{
				PEPROCESS pEprocess = NULL;

				if (NT_SUCCESS(MzfPsLookupProcessByProcessId((HANDLE)info.UniqueProcessId, &pEprocess)))
				{
					ULONG hHandle = (ULONG)info.HandleValue;
					PVOID pObjectName = NULL;

					if ( Version == enumWINDOWS_7 || Version == enumWINDOWS_8 )
					{
						if (pEprocess == pSystemProcess)
						{
							hHandle |= 0x80000000;
						}
					}

					pObjectName = ExAllocatePoolWithTag(PagedPool, MAX_PATH * sizeof(WCHAR), MZFTAG);
					if (pObjectName)
					{
						KAPC_STATE as;
						BOOL bAttach = FALSE;

						memset(pObjectName, 0, MAX_PATH * sizeof(WCHAR));

						if (MzfIoGetCurrentProcess() != pEprocess)
						{
							MzfKeStackAttachProcess(pEprocess, &as);
							bAttach = TRUE;
						}

						GetHandleObjectName((HANDLE)hHandle, 0, (WCHAR*)pObjectName);
						
						if (bAttach)
						{
							MzfKeUnstackDetachProcess(&as);
						}

						if (wcslen(pObjectName) * sizeof(WCHAR) >= nLen && // 路径要比文件夹的路径长
							!_wcsnicmp(szDosPath, pObjectName, nLen / sizeof(WCHAR)))
						{
							if (j < nCanStore)
							{
								KdPrint(("nHandle: 0x%08X, name: %S, nPid: %d\n", info.HandleValue, (WCHAR*)pObjectName, info.UniqueProcessId));

								pOutLockRegInfo->regs[j].nHandle = info.HandleValue;
								pOutLockRegInfo->regs[j].nPid = info.UniqueProcessId;
								wcsncpy(pOutLockRegInfo->regs[j].szRegPath, pObjectName, wcslen(pObjectName));
							}

							j++;
						}

						ExFreePoolWithTag(pObjectName, 0);
					}

					ObDereferenceObject(pKeyObject);
				}
			}	
		}
	}

	pOutLockRegInfo->nRetCount = j;
	KdPrint(("Total: %d\n", j));

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

	return status;
}