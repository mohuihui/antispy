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
#include "DirectIo.h"
#include "Process.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"

DIRECT_IO_TYPE IsIoplTrue(ULONG pEprocess)
{
	DIRECT_IO_TYPE Type = enumOK;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG IoplOffset = GetGlobalVeriable(enumIoplOffset_KPROCESS);

	if (MzfMmIsAddressValid && IoplOffset && MzfMmIsAddressValid((PVOID)(pEprocess + IoplOffset)))
	{
		BYTE bIopl = *(PBYTE)(pEprocess + IoplOffset);
		if (bIopl != 0)
		{
			Type = enumIOPL;
		}
	}

	return Type;
}

DIRECT_IO_TYPE IsTSSLargee()
{
	DIRECT_IO_TYPE Type = enumOK;
	GDTR gdtr;
	PGDTENTRY gdt = NULL;
	WORD TSSseg = 0;

	__asm {
		cli                         // 屏蔽中断
		sgdt  gdtr                  // 得到 GDT 基地址与段界限
		str   TSSseg                // 得到 TSS 选择子
		movzx esi,TSSseg            // 扩展到 ESI 中以便计算
		add   esi,gdtr.dwBase       // 得到 TSS 在 GDT 中描述符
		mov   gdt,esi
		sti							// 恢复中断
	}

	if ( gdt->dwLimit > DEFAULT_TSS_LIMIT )
	{
		Type = enumTSSLarge;
	}

	return Type;
}

DIRECT_IO_TYPE IsIOPLOffsetChange(ULONG pEprocess)
{
	DIRECT_IO_TYPE Type = enumOK;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG IopmOffsetOffset = GetGlobalVeriable(enumIopmOffsetOffset_KPROCESS);

	if (MzfMmIsAddressValid && IopmOffsetOffset && MzfMmIsAddressValid((PVOID)(pEprocess + IopmOffsetOffset)))
	{
		WORD IopmOffset = *(WORD *)(pEprocess + IopmOffsetOffset);
		if (IopmOffset != DEFAULT_IOPL_OFFSET)
		{
			Type = enumIOPLOffsetChange;
		}
	}

	return Type;
}

BOOL PspIsDirectIo(ULONG pEprocess, DIRECT_IO_TYPE *Type)
{
	BOOL bRet = FALSE;
	ULONG IopmOffsetOffset = GetGlobalVeriable(enumIopmOffsetOffset_KPROCESS);
	pfnObReferenceObjectByPointer MzfObReferenceObjectByPointer = (pfnObReferenceObjectByPointer)GetGlobalVeriable(enumObReferenceObjectByPointer);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);

	if (ProcessType &&
		MzfObReferenceObjectByPointer && 
		MzfObfDereferenceObject && 
		IsRealProcess((PEPROCESS)pEprocess) && 
		Type)
	{
		*Type |= IsTSSLargee();

		if (NT_SUCCESS(MzfObReferenceObjectByPointer((PEPROCESS)pEprocess, 0, ProcessType, KernelMode)))
		{
			*Type |= IsIoplTrue(pEprocess);	
			*Type |= IsIOPLOffsetChange(pEprocess);
			MzfObfDereferenceObject((PEPROCESS)pEprocess);
		}

		if (*Type != enumOK)
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

//
// 获取可以直接IO的进程
//
NTSTATUS GetDirectIoProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDIRECT_IO_PROCESS pDioProcess = (PDIRECT_IO_PROCESS)pOutBuffer;
	ULONG nCnt = 1000;
	PALL_PROCESSES pProcessInfo = NULL;

	if (!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag)
	{
		return status;
	}

	if (!pInBuffer || 
		uInSize != sizeof(OPERATE_TYPE) ||
		!pOutBuffer ||
		uOutSize < sizeof(DIRECT_IO_PROCESS))
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	do 
	{
		ULONG nSize = sizeof(ALL_PROCESSES) + nCnt * sizeof(PROCESS_INFO);

		if (pProcessInfo)
		{
			MzfExFreePoolWithTag(pProcessInfo, 0);
			pProcessInfo = NULL;
		}

		pProcessInfo = (PALL_PROCESSES)MzfExAllocatePoolWithTag(PagedPool, nSize, MZFTAG);
		if (pProcessInfo)
		{
			memset(pProcessInfo, 0, nSize);
			status = PspListProcess(pProcessInfo, nSize);
		}

		nCnt = pProcessInfo->nCnt + 100;

	} while (status == STATUS_BUFFER_TOO_SMALL);

	if (NT_SUCCESS(status) && pProcessInfo->nCnt > 0)
	{
		ULONG i = 0;

		for ( i = 0; i < pProcessInfo->nCnt; i++ )
		{
			DIRECT_IO_TYPE Type = enumOK;

			if (PspIsDirectIo(pProcessInfo->ProcessInfo[i].ulEprocess, &Type))
			{
				if (pDioProcess->nCnt > pDioProcess->nRetCnt)
				{
					pDioProcess->Process[pDioProcess->nRetCnt].Type = Type;
					pDioProcess->Process[pDioProcess->nRetCnt].Pid = pProcessInfo->ProcessInfo[i].ulPid;
					pDioProcess->Process[pDioProcess->nRetCnt].pEprocess = pProcessInfo->ProcessInfo[i].ulEprocess;
					wcscpy(pDioProcess->Process[pDioProcess->nRetCnt].szPath, pProcessInfo->ProcessInfo[i].szPath);
				}

				pDioProcess->nRetCnt++;
			}
		}
	}

	if (pProcessInfo)
	{
		MzfExFreePoolWithTag(pProcessInfo, 0);
		pProcessInfo = NULL;
	}

	return status;
}

VOID RecoveTSSLarge()
{
	GDTR gdtr;
	PGDTENTRY gdt = NULL;
	WORD TSSseg = 0;

	__asm {
		cli                            // 屏蔽中断
		sgdt  gdtr                    // 得到 GDT 基地址与段界限
		str   TSSseg                // 得到 TSS 选择子
		movzx esi,TSSseg            // 扩展到 ESI 中以便计算
		add   esi,gdtr.dwBase       // 得到 TSS 在 GDT 中描述符
		mov   gdt,esi
	}

	gdt->dwLimit = DEFAULT_TSS_LIMIT;
	gdt->dwType = NOT_BUSY;            //必须设置为NOTBUSY状态

	__asm {
		ltr TSSseg                // 将设置好的 TSS 重新装入
		sti                        // 开中断
	}
}

NTSTATUS RecoveIOPL(ULONG pEprocess, ULONG pid)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, tempstatus = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG IoplOffset = GetGlobalVeriable(enumIoplOffset_KPROCESS);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	PEPROCESS pTempProcess = NULL;

	if (!ProcessType || 
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject)
	{
		return status;
	}

	tempstatus = MzfObReferenceObjectByHandle((HANDLE)pid, 0, ProcessType, KernelMode, &pTempProcess, NULL);

	if (IsRealProcess((PEPROCESS)pEprocess) && 
		MzfMmIsAddressValid && 
		IoplOffset && 
		MzfMmIsAddressValid((PVOID)(pEprocess + IoplOffset)))
	{
		*(BOOL *)(pEprocess + IoplOffset) = FALSE;
		status = STATUS_SUCCESS;
	}

	if (NT_SUCCESS(tempstatus))
	{
		MzfObfDereferenceObject(pTempProcess);
	}

	return status;
}

NTSTATUS RecoveIOPLOffset(ULONG pEprocess, ULONG pid)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, tempstatus = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnKe386IoSetAccessProcess MzfKe386IoSetAccessProcess = (pfnKe386IoSetAccessProcess)GetGlobalVeriable(enumKe386IoSetAccessProcess);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	PEPROCESS pTempProcess = NULL;

	if (!MzfMmIsAddressValid ||
		!MzfKe386IoSetAccessProcess ||
		!ProcessType || 
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject)
	{
		return status;
	}

	tempstatus = MzfObReferenceObjectByHandle((HANDLE)pid, 0, ProcessType, KernelMode, &pTempProcess, NULL);

	if (IsRealProcess((PEPROCESS)pEprocess))
	{
		if (MzfKe386IoSetAccessProcess((PEPROCESS)pEprocess, IO_ACCESS_MAP_NONE))
		{
			status = STATUS_SUCCESS;
		}
	}

	if (NT_SUCCESS(tempstatus))
	{
		MzfObfDereferenceObject(pTempProcess);
	}

	return status;
}

NTSTATUS DisableDirectIo(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_DIRECTIO pDioComm = (PCOMMUNICATE_DIRECTIO)pInBuffer;
	ULONG pid = pDioComm->op.Disable.pid;
	ULONG pEprocess = pDioComm->op.Disable.pEprocess;
	DIRECT_IO_TYPE Type = pDioComm->op.Disable.Type;

	if (Type & enumTSSLarge)
	{
		RecoveTSSLarge();
	}

	if (Type & enumIOPL)
	{
		status = RecoveIOPL(pEprocess, pid);
	}

	if (Type & enumIOPLOffsetChange)
	{
		status = RecoveIOPLOffset( pEprocess, pid);
	}

	return status;
}