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
#include "Disassmbly.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"

NTSTATUS DumpKernelMemory(ULONG pBase, ULONG nSize, PVOID pOutBuffer)
{
	pfnIoAllocateMdl MzfIoAllocateMdl = (pfnIoAllocateMdl)GetGlobalVeriable(enumIoAllocateMdl);
	pfnMmBuildMdlForNonPagedPool MzfMmBuildMdlForNonPagedPool = (pfnMmBuildMdlForNonPagedPool)GetGlobalVeriable(enumMmBuildMdlForNonPagedPool);
	pfnMmMapLockedPagesSpecifyCache MzfMmMapLockedPagesSpecifyCache = (pfnMmMapLockedPagesSpecifyCache)GetGlobalVeriable(enumMmMapLockedPagesSpecifyCache);
	pfnMmUnmapLockedPages MzfMmUnmapLockedPages = (pfnMmUnmapLockedPages)GetGlobalVeriable(enumMmUnmapLockedPages);
	pfnIoFreeMdl MzfIoFreeMdl = (pfnIoFreeMdl)GetGlobalVeriable(enumIoFreeMdl);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG nOutBuffer = (ULONG)pOutBuffer;

	if (!MzfIoAllocateMdl ||
		!MzfMmBuildMdlForNonPagedPool ||
		!MzfMmMapLockedPagesSpecifyCache ||
		!MzfMmUnmapLockedPages ||
		!MzfIoFreeMdl ||
		!MzfProbeForRead ||
		!MzfMmIsAddressValid)
	{
		return status;
	}

	if (pBase > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)pBase) &&
		nSize < SYSTEM_ADDRESS_START &&
		pBase + nSize > SYSTEM_ADDRESS_START)
	{
		__try
		{
			ULONG nRemainSize = PAGE_SIZE - (pBase & 0xFFF);
			ULONG nCopyedSize = 0;
			KdPrint(("Base: 0x%08X, Size: 0x%08X\n", pBase, nSize));

			while ( nCopyedSize < nSize )
			{
				PMDL pMdl = NULL;
				BOOL bCopyOk = FALSE;

				if ( nSize - nCopyedSize < nRemainSize )
				{
					nRemainSize = nSize - nCopyedSize;
				}

				pMdl = MzfIoAllocateMdl( (PVOID)(pBase & 0xFFFFF000), PAGE_SIZE, FALSE, FALSE, NULL);

				if ( pMdl )
				{
					PVOID pMapedAddress = NULL;

					MzfMmBuildMdlForNonPagedPool(pMdl);
					pMapedAddress = MzfMmMapLockedPagesSpecifyCache( pMdl, KernelMode, MmCached, NULL, 0, NormalPagePriority );

					if ( pMapedAddress )
					{
						memcpy((PVOID)nOutBuffer, (PVOID)((ULONG)pMapedAddress + (pBase & 0xFFF)), nRemainSize);
						MzfMmUnmapLockedPages( pMapedAddress, pMdl );
						bCopyOk = TRUE;
					}

					MzfIoFreeMdl(pMdl);
				}

				if ( !bCopyOk )
				{
					memset((PVOID)nOutBuffer, 0, nRemainSize);
				}

				if ( nCopyedSize )
				{
					nRemainSize = PAGE_SIZE;
				}

				nCopyedSize += nRemainSize;
				pBase += nRemainSize;
				nOutBuffer += nRemainSize;
			}

			status = STATUS_SUCCESS;
		}
		__except(1)
		{
			status = STATUS_UNSUCCESSFUL;
		}
	}

	return status;
}

NTSTATUS DumpRing3Memory(ULONG Base, ULONG Size, ULONG Pid, ULONG Process, PVOID pOutBuffer)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PEPROCESS pEprocess = NULL;
	NTSTATUS statusLookupProcess = STATUS_UNSUCCESSFUL;
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);

	if (!MzfObfDereferenceObject || 
		!MzfPsLookupProcessByProcessId ||
		!MzfExAllocatePoolWithTag || 
		!MzfExFreePoolWithTag || 
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess || 
		!MzfProbeForRead || 
		!MzfIoGetCurrentProcess)
	{
		return STATUS_UNSUCCESSFUL;
	}

	KdPrint(("DumpRing3Memory\n"));

	if (!pOutBuffer || 
		Base >= 0x80000000 ||
		Size <= 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (!Process && Pid)
	{
		statusLookupProcess = MzfPsLookupProcessByProcessId((HANDLE)Pid, &pEprocess);
	}
	else if (Process)
	{
		pEprocess = (PEPROCESS)Process;
	}

	KdPrint(("DumpRing3Memory: Base: 0x%08X, Size: 0x%X, Pid: %d, pEprocess: 0x%08X\n", Base, Size, Pid, pEprocess));

	if ( pEprocess && IsRealProcess(pEprocess) )
	{
		BOOL bAttach = FALSE;
		KAPC_STATE ks;
		PVOID pBuffer = NULL;

		pBuffer = MzfExAllocatePoolWithTag(PagedPool, Size, MZFTAG);
		if (!pBuffer)
		{
			if (NT_SUCCESS(statusLookupProcess))
			{
				MzfObfDereferenceObject(pEprocess);
			}

			return STATUS_UNSUCCESSFUL;
		}

		memset(pBuffer, 0, Size);

		__try
		{
			if (pEprocess != MzfIoGetCurrentProcess())
			{
				MzfKeStackAttachProcess(pEprocess, &ks);
				bAttach = TRUE;
			}

			KdPrint(("Start dump: Base: 0x%08X, Size: 0x%08X\n", Base, Size));

			MzfProbeForRead((PVOID)Base, Size, 1);
			memcpy(pBuffer, (PVOID)Base, Size);

			if (bAttach)
			{
				MzfKeUnstackDetachProcess(&ks);
				bAttach = FALSE;
			}

			memcpy(pOutBuffer, pBuffer, Size);

			status = STATUS_SUCCESS;
		}
		__except(1)
		{
			KdPrint(("__except(1)\n"));
			if (bAttach)
			{
				MzfKeUnstackDetachProcess(&ks);
				bAttach = FALSE;
			}

			status = STATUS_UNSUCCESSFUL;
		}

		MzfExFreePoolWithTag(pBuffer, 0);
	}

	if (NT_SUCCESS(statusLookupProcess))
	{
		MzfObfDereferenceObject(pEprocess);
	}

	return status;
}

NTSTATUS Disassmbly(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_DISASSEMBLY pCd = (PCOMMUNICATE_DISASSEMBLY)pInBuffer;
	ULONG Pid = pCd->nPid;
	ULONG pEprocess = pCd->pEprocess;
	ULONG Address = pCd->Base;
	ULONG Size = pCd->Size;

	KdPrint(("Pid: %d, Address: 0x%08X, Size: 0x%08X\n", Pid, Address, Size));

	if (Address >= SYSTEM_ADDRESS_START)
	{
		status = DumpKernelMemory(Address, Size, pOutBuffer);
	}
	else if (Address < SYSTEM_ADDRESS_START && Address > 0 && (Pid || pEprocess))
	{
		status = DumpRing3Memory(Address, Size, Pid, pEprocess, pOutBuffer);
	}

	return status;
}