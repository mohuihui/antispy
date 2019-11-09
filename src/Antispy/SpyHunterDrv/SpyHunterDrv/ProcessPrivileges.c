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
#include "ProcessPrivileges.h"
#include "InitWindows.h"
#include "CommonFunction.h"
#include "..\\..\\Common\Common.h"

//
// 枚举进程权限
//
NTSTATUS ListProessPrivileges(PVOID pInBuffer, ULONG nInLen, PVOID pOutBuffer, ULONG nOutLen, ULONG* uRet)
{
	PCOMMUNICATE_PRIVILEGES pCpp = (PCOMMUNICATE_PRIVILEGES)pInBuffer;
	ULONG nPid = 0; 
	PEPROCESS process = NULL, pEprocess = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL, ObReferenceStatus = STATUS_UNSUCCESSFUL;
	HANDLE hProcess = 0, hToken = 0;
	ULONG nRet = 0;

	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	pfnNtOpenProcessToken MzfNtOpenProcessToken = (pfnNtOpenProcessToken)GetGlobalVeriable(enumNtOpenProcessToken);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtQueryInformationToken MzfNtQueryInformationToken = (pfnNtQueryInformationToken)GetGlobalVeriable(enumNtQueryInformationToken);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfObOpenObjectByPointer || 
		!MzfNtClose || 
		!MzfNtOpenProcessToken || 
		!MzfPsGetCurrentThread || 
		!MzfNtQueryInformationToken ||
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject)
	{
		return status;
	}

	// 检查参数
	if (!pInBuffer ||
		nInLen != sizeof(COMMUNICATE_PRIVILEGES) || 
		!pOutBuffer ||
		nOutLen < sizeof(TOKEN_PRIVILEGES))
	{
		return STATUS_INVALID_PARAMETER;
	}

	nPid = pCpp->op.GetPrivilege.nPid;
	process = (PEPROCESS)pCpp->op.GetPrivilege.pEprocess;
	
	if (nPid)
	{
		ObReferenceStatus = MzfObReferenceObjectByHandle((HANDLE)nPid, 0, ProcessType, KernelMode, &pEprocess, NULL);
		if (NT_SUCCESS(ObReferenceStatus) && !process)
		{
			process = pEprocess;
		}
	}
	
	if (IsRealProcess(process))
	{
		status = MzfObOpenObjectByPointer(process, 
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
			NULL, 
			GENERIC_ALL, 
			ProcessType, 
			KernelMode, 
			&hProcess
			);

		if (NT_SUCCESS(status))
		{
			PETHREAD pThread = NULL;
			CHAR PreMode = 0;

			pThread = MzfPsGetCurrentThread();
			PreMode = ChangePreMode(pThread);

			status = MzfNtOpenProcessToken(hProcess, SACL_SECURITY_INFORMATION, &hToken);

			if (NT_SUCCESS(status))
			{
				status = MzfNtQueryInformationToken(hToken, TokenPrivileges, pOutBuffer, nOutLen, &nRet);

				if (NT_SUCCESS(status))
				{
					KdPrint(("MzfNtQueryInformationToken success\n"));
				}
				else if (status == STATUS_BUFFER_TOO_SMALL)
				{
					*(PULONG)pOutBuffer = nRet;
				}
			}

			if (hToken)
			{
				MzfNtClose(hToken);
			}

			if (hProcess)
			{
				MzfNtClose(hProcess);
			}

			RecoverPreMode(pThread, PreMode);
		}
	}
	
	if (NT_SUCCESS(ObReferenceStatus))
	{
		MzfObfDereferenceObject(pEprocess);
	}
	
	return status;
}

//
// 调整进程权限
//
NTSTATUS AdjustProcessTokenPrivileges(PVOID pInBuffer, ULONG nInLen, PVOID pOutBuffer, ULONG nOutLen, ULONG* uRet)
{
	PCOMMUNICATE_PRIVILEGES pCpp = (PCOMMUNICATE_PRIVILEGES)pInBuffer;
	ULONG nPid = 0; 
	PEPROCESS process = NULL, pEprocess = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL, ObReferenceStatus = STATUS_UNSUCCESSFUL;
	HANDLE hProcess = 0, hToken = 0;
	PTOKEN_PRIVILEGES pTp = NULL;
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	pfnNtOpenProcessToken MzfNtOpenProcessToken = (pfnNtOpenProcessToken)GetGlobalVeriable(enumNtOpenProcessToken);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnNtAdjustPrivilegesToken MzfNtAdjustPrivilegesToken = (pfnNtAdjustPrivilegesToken)GetGlobalVeriable(enumNtAdjustPrivilegesToken);

	if  (!MzfObOpenObjectByPointer || 
		!MzfNtClose || 
		!MzfNtOpenProcessToken || 
		!MzfPsGetCurrentThread || 
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject ||
		!MzfNtAdjustPrivilegesToken)
	{
		return status;
	}

	if (!pInBuffer || nInLen != sizeof(COMMUNICATE_PRIVILEGES))
	{
		return STATUS_INVALID_PARAMETER;
	}

	nPid = pCpp->op.AdjustPrivilege.nPid;
	process = (PEPROCESS)pCpp->op.AdjustPrivilege.pEprocess;
	pTp = pCpp->op.AdjustPrivilege.pPrivilegeState;
	
	if (!pTp)
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	if (nPid)
	{
		ObReferenceStatus = MzfObReferenceObjectByHandle((HANDLE)nPid, 0, ProcessType, KernelMode, &pEprocess, NULL);
		if (NT_SUCCESS(ObReferenceStatus) && !process)
		{
			process = pEprocess;
		}
	}
	
	if (IsRealProcess(process))
	{
		status = MzfObOpenObjectByPointer(process, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, ProcessType, KernelMode, &hProcess);

		if (NT_SUCCESS(status))
		{
			PETHREAD pThread = MzfPsGetCurrentThread();
			CHAR PreMode = ChangePreMode(pThread);

			status = MzfNtOpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken);

			if (NT_SUCCESS(status))
			{
				status = MzfNtAdjustPrivilegesToken(hToken, FALSE, pTp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

				if (NT_SUCCESS(status))
				{
					KdPrint(("MzfNtAdjustPrivilegesToken success\n"));
				}
			}

			if (hToken)
			{
				MzfNtClose(hToken);
			}

			if (hProcess)
			{
				MzfNtClose(hProcess);
			}

			RecoverPreMode(pThread, PreMode);
		}
	}

	if (NT_SUCCESS(ObReferenceStatus))
	{
		MzfObfDereferenceObject(pEprocess);
	}

	return status;
}