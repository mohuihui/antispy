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
#include "ProcessMemory.h"
#include "..\\..\\Common\\Common.h"
#include "InitWindows.h"
#include "CommonFunction.h"

NTSTATUS GetMemorys(PEPROCESS pEprocess, PALL_MEMORYS pMemorys, ULONG nCnt)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	HANDLE hProcess = NULL;
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtQueryVirtualMemory MzfNtQueryVirtualMemory = (pfnNtQueryVirtualMemory)GetGlobalVeriable(enumNtQueryVirtualMemory);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);

	if (!ProcessType ||
		!MzfPsGetCurrentThread || 
		!MzfObOpenObjectByPointer ||
		!MzfNtQueryVirtualMemory ||
		!MzfNtClose)
	{
		return status;
	}

	status = MzfObOpenObjectByPointer(pEprocess, 
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
		NULL, 
		GENERIC_ALL, 
		ProcessType, 
		KernelMode, 
		&hProcess
		);

	if (NT_SUCCESS(status))
	{
		ULONG Base = 0;
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		
		while (Base < (ULONG)MM_HIGHEST_USER_ADDRESS)
		{
			MEMORY_BASIC_INFORMATION mbi;
			ULONG ret = 0;
			NTSTATUS statusTemp = MzfNtQueryVirtualMemory(hProcess, 
				(PVOID)Base, 
				MemoryBasicInformation, 
				&mbi, 
				sizeof(MEMORY_BASIC_INFORMATION), 
				&ret);

			if (NT_SUCCESS(statusTemp))
			{
				ULONG nCurCnt = pMemorys->nCnt;
				if (nCnt > nCurCnt)
				{
					pMemorys->Memorys[nCurCnt].Base = Base;
					pMemorys->Memorys[nCurCnt].Size = mbi.RegionSize;
					pMemorys->Memorys[nCurCnt].Protect = mbi.Protect;
					pMemorys->Memorys[nCurCnt].State = mbi.State;
					pMemorys->Memorys[nCurCnt].Type = mbi.Type;
				}

				pMemorys->nCnt++;
				Base += mbi.RegionSize;
			}
			else
			{
				Base += PAGE_SIZE;
			}
		}

		MzfNtClose(hProcess);
		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

//
// 枚举进程内存
//
NTSTATUS ListProcessMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_MEMORY pCm = (PCOMMUNICATE_MEMORY)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0, nCnt = (uOutSize - sizeof(ALL_MEMORYS)) / sizeof(MEMORY_INFO);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	// 参数检测
	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_MEMORY) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_MEMORYS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	nPid = pCm->op.GetMemory.nPid;
	pEprocess = (PEPROCESS)pCm->op.GetMemory.pEprocess;

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
		KdPrint(("ListProcessMemory\n"));
		status = GetMemorys(pEprocess, (PALL_MEMORYS)pOutBuffer, nCnt);
		if (NT_SUCCESS(status))
		{
			if (nCnt >= ((PALL_MEMORYS)pOutBuffer)->nCnt)
			{
				status = STATUS_SUCCESS;
			}
			else
			{
				status = STATUS_BUFFER_TOO_SMALL;
			}
		}
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

//
// 释放内存
//
NTSTATUS PspFreeProcessMemory(PEPROCESS pEprocess, ULONG Base, ULONG Size)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnNtFreeVirtualMemory MzfNtFreeVirtualMemory = (pfnNtFreeVirtualMemory)GetGlobalVeriable(enumNtFreeVirtualMemory);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	HANDLE hProcess = NULL;

	if (!MzfPsGetCurrentThread ||
		!MzfNtClose ||
		!MzfObOpenObjectByPointer ||
		!ProcessType ||
		!MzfNtFreeVirtualMemory ||
		!pEprocess ||
		Size == 0)
	{
		return status;
	}

	status = MzfObOpenObjectByPointer(pEprocess, 
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
		NULL, 
		GENERIC_ALL, 
		ProcessType, 
		KernelMode, 
		&hProcess
		);

	if (NT_SUCCESS(status))
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);

		status = MzfNtFreeVirtualMemory(hProcess, &((PVOID)Base), &Size, MEM_RELEASE);
		KdPrint(("MzfNtFreeVirtualMemory : status : 0x%08X\n", status));

		MzfNtClose(hProcess);
		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

//
// 释放内存
//
NTSTATUS FreeProcessMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_MEMORY pCm = (PCOMMUNICATE_MEMORY)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0, nBase = 0, nSize = 0;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}

	// 参数检查
	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_MEMORY))
	{
		return STATUS_INVALID_PARAMETER;
	}

	nPid = pCm->op.FreeMemory.nPid;
	nBase = pCm->op.FreeMemory.Base;
	nSize = pCm->op.FreeMemory.Size;
	pEprocess = (PEPROCESS)pCm->op.FreeMemory.pEprocess;
	
	if (nBase >= SYSTEM_ADDRESS_START ||
		nSize >= SYSTEM_ADDRESS_START ||
		nBase + nSize >= SYSTEM_ADDRESS_START
		)
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
		KdPrint(("FreeProcessMemory-> Base: 0x%08X, Size: 0x%08X\n", nBase, nSize));

		status = PspFreeProcessMemory(
			pEprocess, 
			nBase, 
			nSize
			);
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

//
// 修改内存保护属性
//
NTSTATUS PspModifyMemoryProtection(PEPROCESS pEprocess, ULONG Base, ULONG Size, ULONG ModifyType)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnNtProtectVirtualMemory MzfNtProtectVirtualMemory= (pfnNtProtectVirtualMemory)GetGlobalVeriable(enumNtProtectVirtualMemory);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	HANDLE hProcess = NULL;

	if (!MzfPsGetCurrentThread ||
		!MzfNtClose ||
		!MzfObOpenObjectByPointer ||
		!ProcessType ||
		!MzfNtProtectVirtualMemory ||
		!pEprocess || 
		!Size)
	{
		return status;
	}

	status = MzfObOpenObjectByPointer(pEprocess, 
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
		NULL, 
		GENERIC_ALL, 
		ProcessType, 
		KernelMode, 
		&hProcess);

	if (NT_SUCCESS(status))
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		ULONG OldProtection = 0;

		status = MzfNtProtectVirtualMemory(hProcess, &((PVOID)Base), &Size, ModifyType, &OldProtection);
		KdPrint(("MzfNtProtectVirtualMemory status : 0x%08X, OldProtection: 0x%X\n", status, OldProtection));
		MzfNtClose(hProcess);
		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

//
// 更改内存保护属性
//
NTSTATUS ModifyMemoryProtection(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_MEMORY pCm = (PCOMMUNICATE_MEMORY)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0, nBase = 0, nSize = 0, ModifyType = 0;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	// 参数检查
	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_MEMORY))
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	nBase = pCm->op.ModifyProtection.Base;
	nSize = pCm->op.ModifyProtection.Size;
	ModifyType = pCm->op.ModifyProtection.ModifyType;
	nPid = pCm->op.ModifyProtection.nPid;
	pEprocess = (PEPROCESS)pCm->op.ModifyProtection.pEprocess;
	
	if (nBase >= SYSTEM_ADDRESS_START ||
		nSize >= SYSTEM_ADDRESS_START ||
		nBase + nSize >= SYSTEM_ADDRESS_START
		)
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
		KdPrint(("ModifyMemoryProtection-> Base: 0x%08X, Size: 0x%08X\n", nBase, nSize));

		status = PspModifyMemoryProtection(
			pEprocess, 
			nBase, 
			nSize, 
			ModifyType);
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

//
// 清零内存
//
BOOL ZeroOfMemory(PVOID pDesAddress, ULONG nLength, KPROCESSOR_MODE AccessMode)
{
	ULONG nLast = 0;
	PMDL pMdl = NULL;
	PVOID pMappedAddress = NULL; 
	KIRQL OldIrql; 
	BOOL bRet = FALSE;
	pfnIoAllocateMdl MzfIoAllocateMdl = (pfnIoAllocateMdl)GetGlobalVeriable(enumIoAllocateMdl);
	pfnMmBuildMdlForNonPagedPool MzfMmBuildMdlForNonPagedPool = (pfnMmBuildMdlForNonPagedPool)GetGlobalVeriable(enumMmBuildMdlForNonPagedPool);
	pfnMmUnmapLockedPages MzfMmUnmapLockedPages = (pfnMmUnmapLockedPages)GetGlobalVeriable(enumMmUnmapLockedPages);
	pfnIoFreeMdl MzfIoFreeMdl = (pfnIoFreeMdl)GetGlobalVeriable(enumIoFreeMdl);
	pfnMmMapLockedPagesSpecifyCache MzfMmMapLockedPagesSpecifyCache = (pfnMmMapLockedPagesSpecifyCache)GetGlobalVeriable(enumMmMapLockedPagesSpecifyCache);

	if (!MzfIoAllocateMdl ||
		!MzfMmBuildMdlForNonPagedPool ||
		!MzfMmUnmapLockedPages ||
		!MzfIoFreeMdl ||
		!MzfMmMapLockedPagesSpecifyCache)
	{
		return FALSE;
	}

	nLast = (ULONG)pDesAddress & 0xFFF;
	pMdl = MzfIoAllocateMdl((PVOID)((ULONG)pDesAddress & 0xFFFFF000), nLast + nLength + 0xFFF, FALSE, FALSE, NULL);
	if ( pMdl )
	{
		MzfMmBuildMdlForNonPagedPool(pMdl);
		pMdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

		__try
		{
			pMappedAddress = MzfMmMapLockedPagesSpecifyCache(pMdl, AccessMode, MmCached, NULL, 0, NormalPagePriority);
			if ( pMappedAddress )
			{
				OldIrql = KeRaiseIrqlToDpcLevel();
				memset((char *)pMappedAddress + nLast, 0, nLength);
				KeLowerIrql(OldIrql);
				MzfMmUnmapLockedPages(pMappedAddress, pMdl);
				bRet = TRUE;
			}
		}
		__except(1)
		{
			KdPrint(("MmMapLockedPagesSpecifyCache catch error\n"));
			bRet = FALSE;
		}
		
		MzfIoFreeMdl(pMdl);
	}

	return bRet;
}

NTSTATUS PspZeroProcessMemory(PEPROCESS pEprocess, ULONG Base, ULONG Size)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);

	BOOL bAttach = FALSE;
	KAPC_STATE as;

	if (!MzfIoGetCurrentProcess ||
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess ||
		!MzfProbeForWrite ||
		!pEprocess ||
		Size == 0) 
	{
		return status;
	}

	if (MzfIoGetCurrentProcess() != pEprocess)
	{
		MzfKeStackAttachProcess(pEprocess, &as);
		bAttach = TRUE;
	}
	
	__try
	{
		MzfProbeForWrite((PVOID)Base, Size, 1);
	}
	__except(1)
	{
	}

	if (ZeroOfMemory((PVOID)Base, Size, UserMode))
	{
		status = STATUS_SUCCESS;
	}

	if (bAttach)
	{
		MzfKeUnstackDetachProcess(&as);
		bAttach = FALSE;
	}

	return status;
}

//
// 清零内存
//
NTSTATUS ZeroProcessMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_MEMORY pCm = (PCOMMUNICATE_MEMORY)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0, nBase = 0, nSize = 0;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_MEMORY))
	{
		return STATUS_INVALID_PARAMETER;
	}

	nPid = pCm->op.ZeroMemory.nPid;
	nBase = pCm->op.ZeroMemory.Base;
	nSize = pCm->op.ZeroMemory.Size;
	pEprocess = (PEPROCESS)pCm->op.ZeroMemory.pEprocess;

	if (nBase >= SYSTEM_ADDRESS_START ||
		nSize >= SYSTEM_ADDRESS_START ||
		nBase + nSize >= SYSTEM_ADDRESS_START
		)
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
		KdPrint(("ZeroProcessMemory\n"));
		status = PspZeroProcessMemory(
			pEprocess, 
			nBase, 
			nSize);
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}