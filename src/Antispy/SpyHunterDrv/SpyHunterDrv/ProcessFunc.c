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
#include "processfunc.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "InitWindows.h"

NTSTATUS KernelOpenProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCES pCp = (PCOMMUNICATE_PROCES)pInBuffer;
	pfnNtOpenProcess MzfNtOpenProcess = (pfnNtOpenProcess)GetGlobalVeriable(enumNtOpenProcess);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	
	if (MzfNtOpenProcess && MzfPsGetCurrentThread)
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		
		status = MzfNtOpenProcess(
			(PHANDLE)pOutBuffer, 
			pCp->op.Open.DesiredAccess, 
			(POBJECT_ATTRIBUTES)pCp->op.Open.ObjectAttributes,
			(PCLIENT_ID)pCp->op.Open.ClientId);

		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

NTSTATUS KernelWriteProcessMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCES pCp = (PCOMMUNICATE_PROCES)pInBuffer;
	HANDLE hProcess = pCp->op.WriteMemory.hProcess;
	PVOID pAddress = pCp->op.WriteMemory.lpBaseAddress;
	PVOID pBuffer = pCp->op.WriteMemory.lpBuffer;
	SIZE_T nSize = pCp->op.WriteMemory.nSize;

	pfnNtWriteVirtualMemory MzfNtWriteVirtualMemory = (pfnNtWriteVirtualMemory)GetGlobalVeriable(enumNtWriteVirtualMemory);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);

	KdPrint(("KernelWriteProcessMemory start: pAddress: 0x%08X, pBuffer: 0x%08X, nSize: %d!\n", pAddress, pBuffer, nSize));

	if (!MzfProbeForRead || 
		!MzfNtWriteVirtualMemory || 
		!MzfPsGetCurrentThread)
	{
		return status;
	}

	if (hProcess && 
		pAddress >= (PVOID)0 && 
		pBuffer && 
		nSize)
	{
		KdPrint(("KernelWriteProcessMemory start: pAddress: 0x%08X, pBuffer: 0x%08X, nSize: %d!\n", pAddress, pBuffer, nSize));

		__try
		{
			SIZE_T NumberOfBytesWritten = 0;
			PETHREAD pThread = NULL;
			CHAR PreMode = 0;

			MzfProbeForRead(pBuffer, nSize, 1);
			
			pThread = MzfPsGetCurrentThread();
			Pre\;\.Mode = ChangePreMode(pThread);

			status = MzfNtWriteVirtualMemory(hProcess, pAddress, pBuffer, nSize, &NumberOfBytesWritten);
			KdPrint(("MzfNtWriteVirtualMemory status: 0x%08X\n", status));

			RecoverPreMode(pThread, PreMode);
		}
		__except(1)
		{
			KdPrint(("KernelWriteProcessMemory error!\n"));;
			status = STATUS_UNSUCCESSFUL;
		}
	}

	return status;
}

NTSTATUS KernelReadProcessMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCES pCp = (PCOMMUNICATE_PROCES)pInBuffer;
	HANDLE hProcess = pCp->op.ReadMemory.hProcess;
	PVOID pAddress = pCp->op.ReadMemory.lpBaseAddress;
	PVOID pBuffer = pCp->op.ReadMemory.lpBuffer;
	SIZE_T nSize = pCp->op.ReadMemory.nSize;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnNtReadVirtualMemory MzfNtReadVirtualMemory = (pfnNtWriteVirtualMemory)GetGlobalVeriable(enumNtReadVirtualMemory);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForRead)GetGlobalVeriable(enumProbeForWrite);

	if (!MzfProbeForWrite || 
		!MzfNtReadVirtualMemory || 
		!MzfPsGetCurrentThread ||
		!MzfProbeForRead)
	{
		return status;
	}

	if (hProcess && pAddress >= (PVOID)0 && pBuffer && nSize)
	{
		__try
		{
			SIZE_T NumberOfBytesWritten = 0;
			PETHREAD pThread = NULL;
			CHAR PreMode = 0;

			MzfProbeForWrite(pBuffer, nSize, 1);
		//	KdPrint(("MzfProbeForWrite ok!\n"));
			MzfProbeForRead(pAddress, nSize, 1);
		//	KdPrint(("MzfProbeForRead ok!\n"));

			pThread = MzfPsGetCurrentThread();
			PreMode = ChangePreMode(pThread);
			status = MzfNtReadVirtualMemory(hProcess, pAddress, pBuffer, nSize, &NumberOfBytesWritten);
			RecoverPreMode(pThread, PreMode);
		}
		__except(1)
		{
			KdPrint(("KernelReadProcessMemory probe error!\n"));
			status = STATUS_UNSUCCESSFUL;
		}
	}

	return status;
}

NTSTATUS KernelSuspendProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCES pCp = (PCOMMUNICATE_PROCES)pInBuffer;
	HANDLE hProcess = pCp->op.SuspendOrResume.hProcess;
	pfnNtSuspendProcess MzfNtSuspendProcess = (pfnNtSuspendProcess)GetGlobalVeriable(enumNtSuspendProcess);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (MzfNtSuspendProcess && MzfPsGetCurrentThread && hProcess)
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);

		status = MzfNtSuspendProcess(hProcess);

		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

NTSTATUS KernelResumeProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCES pCp = (PCOMMUNICATE_PROCES)pInBuffer;
	HANDLE hProcess = pCp->op.SuspendOrResume.hProcess;
	pfnNtResumeProcess MzfNtResumeProcess = (pfnNtResumeProcess)GetGlobalVeriable(enumNtResumeProcess);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (MzfNtResumeProcess && MzfPsGetCurrentThread && hProcess)
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);

		status = MzfNtResumeProcess(hProcess);

		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

NTSTATUS KernelVirtualProtectEx(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCES pCp = (PCOMMUNICATE_PROCES)pInBuffer;
	HANDLE hProcess = pCp->op.VirtualProtect.hProcess;
	PVOID pAddress = pCp->op.VirtualProtect.lpAddress;
	DWORD dwNewProtect = pCp->op.VirtualProtect.flNewProtect;
	SIZE_T nSize = pCp->op.VirtualProtect.dwSize;
	pfnNtProtectVirtualMemory MzfNtProtectVirtualMemory = (pfnNtProtectVirtualMemory)GetGlobalVeriable(enumNtProtectVirtualMemory);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (MzfNtProtectVirtualMemory && MzfPsGetCurrentThread && hProcess && pAddress >= (PVOID)0 && nSize)
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);

		status = MzfNtProtectVirtualMemory(hProcess, &pAddress, &nSize, dwNewProtect, (PULONG)pOutBuffer);
		KdPrint(("MzfNtProtectVirtualMemory status: 0x%08X\n", status));

		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

NTSTATUS KernelQueryVirtualMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCES pCp = (PCOMMUNICATE_PROCES)pInBuffer;
	HANDLE ProcessHandle = pCp->op.QueryVirtualMemory.ProcessHandle;
	IN PVOID BaseAddress = pCp->op.QueryVirtualMemory.BaseAddress;
	IN ULONG MemoryInformationClass = pCp->op.QueryVirtualMemory.MemoryInformationClass;
	OUT PVOID MemoryInformation = pCp->op.QueryVirtualMemory.MemoryInformation;
	IN ULONG MemoryInformationLength = pCp->op.QueryVirtualMemory.MemoryInformationLength;
	pfnNtQueryVirtualMemory MzfNtQueryVirtualMemory = (pfnNtQueryVirtualMemory)GetGlobalVeriable(enumNtQueryVirtualMemory);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForRead)GetGlobalVeriable(enumProbeForWrite);

	if (MzfProbeForWrite && 
		MzfNtQueryVirtualMemory && 
		MzfPsGetCurrentThread && 
		ProcessHandle && 
		BaseAddress >= (PVOID)0 && 
		MemoryInformation && 
		MemoryInformationLength)
	{
		BOOL bError = FALSE;

		__try
		{
			MzfProbeForWrite(MemoryInformation, MemoryInformationLength, 1);
		}
		__except(1)
		{
			bError = TRUE;
			status = STATUS_UNSUCCESSFUL;
		}

		if (!bError)
		{
			PETHREAD pThread = MzfPsGetCurrentThread();
			CHAR PreMode = ChangePreMode(pThread);
			ULONG dwRet = 0;

			status = MzfNtQueryVirtualMemory(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, &dwRet);
			KdPrint(("KernelQueryVirtualMemory status: 0x%08X\n", status));

			RecoverPreMode(pThread, PreMode);

			if (uOutSize == sizeof(ULONG) && pOutBuffer)
			{
				*(PULONG)pOutBuffer = dwRet;
			}
		}
	}

	return status;
}