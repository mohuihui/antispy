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
#include "ProcessModule.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"

///////////////////////////////////////////////////////////////

BOOL IsModuleInList(ULONG Base, ULONG Size, PALL_MODULES pPmi, ULONG nCnt)
{
	BOOL bIn = FALSE;
	ULONG i = 0;
	ULONG nTempCnt = pPmi->nCnt > nCnt ? nCnt : pPmi->nCnt;

	for (i = 0; i < nTempCnt; i++)
	{
		if (Base == pPmi->Modules[i].Base && Size == pPmi->Modules[i].Size)
		{
			bIn = TRUE;
			break;
		}
	}

	return bIn;
}	

void WalkerModuleList(PLIST_ENTRY pList, ULONG nType, PALL_MODULES pPmi, ULONG nCnt)
{
	PLIST_ENTRY entry = NULL;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	
	if (!MzfProbeForRead || !MzfExFreePoolWithTag || !pList || !pPmi)
	{
		return;
	}

	entry = pList->Flink;

	while ((ULONG)entry > 0 && (ULONG)entry < SYSTEM_ADDRESS_START && entry != pList)
	{
		PLDR_DATA_TABLE_ENTRY pLdrEntry = NULL;
		
		switch (nType)
		{
		case 1:
			pLdrEntry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			break;

		case 2:
			pLdrEntry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			break;

		case 3:
			pLdrEntry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);
			break;
		}
		
		if ((ULONG)pLdrEntry > 0 && (ULONG)pLdrEntry < SYSTEM_ADDRESS_START)
		{
			__try
			{
				MzfProbeForRead(pLdrEntry, sizeof(LDR_DATA_TABLE_ENTRY), 1);

				if (!IsModuleInList((ULONG)pLdrEntry->DllBase, pLdrEntry->SizeOfImage, pPmi, nCnt))
				{
					if (nCnt > pPmi->nCnt)
					{
						ULONG nLen = CmpAndGetStringLength(&pLdrEntry->FullDllName, MAX_PATH);
						
						pPmi->Modules[pPmi->nCnt].Base = (ULONG)pLdrEntry->DllBase;
						pPmi->Modules[pPmi->nCnt].Size = pLdrEntry->SizeOfImage;
						
						MzfProbeForRead(pLdrEntry->FullDllName.Buffer, nLen * sizeof(WCHAR), sizeof(WCHAR));

						wcsncpy(pPmi->Modules[pPmi->nCnt].Path, pLdrEntry->FullDllName.Buffer, nLen);
					}

					pPmi->nCnt++;
				}
			}
			__except(1)
			{
				KdPrint(("WalkerModuleList __except(1)\n"));
			}
		}

		entry = entry->Flink;
	}
}

//
// 枚举进程的peb获取模块列表
//
NTSTATUS EnumDllModuleByPeb( PEPROCESS pEprocess, PALL_MODULES pPmi, ULONG nCnt )
{
	ULONG PebOffset = GetGlobalVeriable(enumPebOffset_EPROCESS);
	BOOL bAttach = FALSE;
	KAPC_STATE ks;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);

	if (!MzfMmIsAddressValid		|| 
		!MzfIoGetCurrentProcess		|| 
		!MzfKeStackAttachProcess	|| 
		!MzfKeUnstackDetachProcess	|| 
		!MzfProbeForRead			||
		!pEprocess || !pPmi			||
		!nCnt						||
		KeGetCurrentIrql() >= DISPATCH_LEVEL)
	{
		return status;
	}

	KdPrint(("ENTER EnumDllModuleByPeb\n"));
	
	if (!MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + PebOffset)))
	{
		return status;
	}

	if (MzfIoGetCurrentProcess() != pEprocess)
	{
		MzfKeStackAttachProcess(pEprocess, &ks);
		bAttach = TRUE;
	}

	__try
	{
		PMY_PEB peb = *(PMY_PEB*)((ULONG)pEprocess + PebOffset);

		if ((ULONG)peb > 0 && (ULONG)peb < SYSTEM_ADDRESS_START)
		{
			PPEB_LDR_DATA pLdr = NULL;
			
			MzfProbeForRead(peb, sizeof(MY_PEB), 1);
			MzfProbeForRead(peb->Ldr, sizeof(PEB_LDR_DATA), 1);

			pLdr = (PPEB_LDR_DATA)(peb->Ldr);

			if ((ULONG)pLdr > 0 && (ULONG)pLdr < SYSTEM_ADDRESS_START)
			{
				WalkerModuleList(&pLdr->InLoadOrderModuleList, 1, pPmi, nCnt);
				WalkerModuleList(&pLdr->InMemoryOrderModuleList, 2, pPmi, nCnt);
				WalkerModuleList(&pLdr->InInitializationOrderModuleList, 3, pPmi, nCnt);

				status = STATUS_SUCCESS;
			}
		}
	}
	__except(1)
	{
		KdPrint(("EnumDllModuleByPeb catch __except\n"));
		status = STATUS_UNSUCCESSFUL;
	}

	if (bAttach)
	{
		MzfKeUnstackDetachProcess(&ks);
		bAttach = FALSE;
	}

	return status;
}

//
// 枚举进程模块
//
NTSTATUS ListProcessModule(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, statusLookupProcess = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCESS_MODULE pCpm = (PCOMMUNICATE_PROCESS_MODULE)pInBuffer;
	ULONG nPid = pCpm->nPid;
	PEPROCESS pTempEprocess = NULL, pEprocess = (PEPROCESS)pCpm->pEprocess;
	ULONG nCnt = (uOutSize - sizeof(ALL_MODULES)) / sizeof(MODULE_INFO);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	
	if (!MzfObfDereferenceObject || !MzfPsLookupProcessByProcessId ||
		!MzfExAllocatePoolWithTag || !MzfExFreePoolWithTag
		)
	{
		return STATUS_UNSUCCESSFUL;
	}

	// 参数检查
	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_PROCESS_MODULE) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_MODULES))
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	if (nPid)
	{
		statusLookupProcess = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);

		if (NT_SUCCESS(statusLookupProcess) && !pEprocess)
		{
			pEprocess = pTempEprocess;
		}
	}

	if (!pEprocess)
	{
		return STATUS_UNSUCCESSFUL;
	}

	KdPrint(("enter ListProcessModule\n"));

	if (IsRealProcess(pEprocess))
	{
		PALL_MODULES pAllModules = (PALL_MODULES)MzfExAllocatePoolWithTag(PagedPool, uOutSize, MZFTAG);
		if (pAllModules)
		{
			memset(pAllModules, 0, uOutSize);

			status = EnumDllModuleByPeb(pEprocess, pAllModules, nCnt);

			if (nCnt >= pAllModules->nCnt)
			{
				RtlCopyMemory(pOutBuffer, pAllModules, uOutSize);
				status = STATUS_SUCCESS;
			}
			else
			{
				status = STATUS_BUFFER_TOO_SMALL;
			}

			MzfExFreePoolWithTag(pAllModules, 0);
			pAllModules = NULL;
		}
	}

	if (NT_SUCCESS(statusLookupProcess))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}
	
	return status;
}

//
// 拷贝模块内存
//
NTSTATUS DumpDllModule(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, statusTemp = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCESS_MODULE pCpmd = (PCOMMUNICATE_PROCESS_MODULE)pInBuffer;
	PEPROCESS pEprocess = NULL, pEprocessTemp = NULL;
	ULONG Base = 0, Size = 0, nPid = 0;
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);

	if (!MzfObfDereferenceObject || !MzfPsLookupProcessByProcessId)
	{
		return status;
	}

	// 参数检查
	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_PROCESS_MODULE) ||
		!pOutBuffer ||
		!uOutSize)
	{
		return STATUS_INVALID_PARAMETER;
	}

	pEprocess = (PEPROCESS)pCpmd->pEprocess;
	Base = pCpmd->op.Dump.Base;
	Size = pCpmd->op.Dump.Size;
	nPid = pCpmd->nPid;

	if (Size == 0								||
		Base >= SYSTEM_ADDRESS_START			||
		Size >= SYSTEM_ADDRESS_START			||
		Base + Size >= SYSTEM_ADDRESS_START		)
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	if (nPid)
	{
		statusTemp = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pEprocessTemp);
		if (NT_SUCCESS(statusTemp) && !pEprocess)
		{
			pEprocess = pEprocessTemp;
		}
	}

	if (!pEprocess)
	{
		return status;
	}

	KdPrint(("enter new DumpDllModule\n"));
	
	if (IsRealProcess(pEprocess))
	{
		status = SafeCopyProcessModules(pEprocess, Base, Size, pOutBuffer);
	}
	
	if (NT_SUCCESS(statusTemp))
	{
		MzfObfDereferenceObject(pEprocessTemp);
	}

	return status;
}

//
// 移除PEB链表中的模块
//
VOID WalkerModuleListToRemoveDllModule(PLIST_ENTRY pList, ULONG nType, ULONG Base)
{
	PLIST_ENTRY entry = pList->Flink;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);

	if (!MzfProbeForRead && !MzfExFreePoolWithTag)
	{
		return;
	}

	while ((ULONG)entry > 0 && (ULONG)entry < SYSTEM_ADDRESS_START && entry != pList)
	{
		PLDR_DATA_TABLE_ENTRY pLdrEntry = NULL;

		switch (nType)
		{
		case 1:
			pLdrEntry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			break;

		case 2:
			pLdrEntry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			break;

		case 3:
			pLdrEntry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);
			break;
		}

		if ((ULONG)pLdrEntry > 0 && (ULONG)pLdrEntry < SYSTEM_ADDRESS_START)
		{
			__try
			{
				MzfProbeForRead(pLdrEntry, sizeof(LDR_DATA_TABLE_ENTRY), 1);

				if ((ULONG)pLdrEntry->DllBase == Base &&
					(ULONG)entry->Blink < SYSTEM_ADDRESS_START && 
					(ULONG)entry->Flink < SYSTEM_ADDRESS_START)
				{
					RemoveEntryList(entry);
					break;
				}
			}
			__except(1)
			{
				KdPrint(("WalkerModuleList __except(1)\n"));
			}
		}

		entry = entry->Flink;
	}
}

//
// 移除在PEB中的进程模块项
//
NTSTATUS RemoveDllModuleInPeb( PEPROCESS pEprocess, ULONG Base )
{
	ULONG PebOffset = GetGlobalVeriable(enumPebOffset_EPROCESS);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);

	if (!MzfMmIsAddressValid || 
		!MzfIoGetCurrentProcess || 
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess || 
		!MzfProbeForRead)
	{
		return status;
	}

	if (pEprocess && KeGetCurrentIrql() < DISPATCH_LEVEL)
	{
		BOOL bAttach = FALSE;
		KAPC_STATE ks;

		if (!MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + PebOffset)))
		{
			return status;
		}

		if (MzfIoGetCurrentProcess() != pEprocess)
		{
			MzfKeStackAttachProcess(pEprocess, &ks);
			bAttach = TRUE;
		}

		__try
		{
			PMY_PEB peb = *(PMY_PEB*)((ULONG)pEprocess + PebOffset);
			if ((ULONG)peb > 0 && (ULONG)peb < SYSTEM_ADDRESS_START)
			{
				PPEB_LDR_DATA pLdr = NULL;

				MzfProbeForRead(peb, sizeof(MY_PEB), 1);
				MzfProbeForRead(peb->Ldr, sizeof(PEB_LDR_DATA), 1);

				pLdr = (PPEB_LDR_DATA)(peb->Ldr);
				if ((ULONG)pLdr > 0 && (ULONG)pLdr < SYSTEM_ADDRESS_START)
				{
					WalkerModuleListToRemoveDllModule(&pLdr->InLoadOrderModuleList, 1, Base);
					WalkerModuleListToRemoveDllModule(&pLdr->InMemoryOrderModuleList, 2, Base);
					WalkerModuleListToRemoveDllModule(&pLdr->InInitializationOrderModuleList, 3, Base);
					
					status = STATUS_SUCCESS;
				}
			}
		}
		__except(1)
		{
			KdPrint(("EnumDllModuleByPeb catch __except\n"));
		}

		if (bAttach)
		{
			MzfKeUnstackDetachProcess(&ks);
			bAttach = FALSE;
		}
	}

	return status;
}

//
// 移除LdrpHashTable链表中的进程模块项
//
NTSTATUS RemoveDllModuleInLdrpHashTable( LIST_ENTRY *LdrpHashTable, PEPROCESS pEprocess, ULONG Base )
{
	BOOL bAttach = FALSE;
	KAPC_STATE ks;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);

	if (!LdrpHashTable ||
		!pEprocess ||
		!MzfMmIsAddressValid || 
		!MzfIoGetCurrentProcess || 
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess || 
		!MzfProbeForRead)
	{
		return status;
	}
	
	if (MzfIoGetCurrentProcess() != pEprocess)
	{
		MzfKeStackAttachProcess(pEprocess, &ks);
		bAttach = TRUE;
	}

	__try
	{
		LIST_ENTRY *pListEntry = NULL, *pListHead = NULL;
		DWORD size = sizeof(LIST_ENTRY) * 32;
		BOOL bRemove = FALSE;
		int i = 0;

		// 读Hash表
		MzfProbeForRead((PVOID)LdrpHashTable, size, 1);			

		for(i = 0; !bRemove && i < 32; i++)
		{
			pListHead = LdrpHashTable + i;	//指向某hash节点头
			pListEntry = pListHead->Flink;

			while(pListEntry != pListHead)
			{
				PLDR_DATA_TABLE_ENTRY pLdrEntry = (PLDR_DATA_TABLE_ENTRY)((BYTE *)pListEntry - 0x3c);

				if ((ULONG)pLdrEntry > 0 && (ULONG)pLdrEntry < SYSTEM_ADDRESS_START)
				{
					MzfProbeForRead(pLdrEntry, sizeof(LDR_DATA_TABLE_ENTRY), 1);

					if ((ULONG)(pLdrEntry->DllBase) == Base)
					{
						KdPrint(("find dll \n"));
						RemoveEntryList(pListEntry);
						bRemove = TRUE;
						status = STATUS_SUCCESS;
						break;
					}

					KdPrint(("base: 0x%08X, Size: 0x%X\n", pLdrEntry->DllBase, pLdrEntry->SizeOfImage));
				}

				pListEntry = pListEntry->Flink; // 通过节点的hashlinks来递进
			}
		}
	}
	__except(1)
	{
		KdPrint(("RemoveDllModuleInLdrpHashTable catch __except\n"));
	}

	if (bAttach)
	{
		MzfKeUnstackDetachProcess(&ks);
		bAttach = FALSE;
	}

	return status;
}

//
// 根据传进来的模块基地址，调用NtUnmapViewOfSection卸载掉模块
//
NTSTATUS UnloadDllModule(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, statusLookupProcess = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCESS_MODULE pCpmu = (PCOMMUNICATE_PROCESS_MODULE)pInBuffer;
	PEPROCESS pTempEprocess = NULL, pEprocess = NULL;
	ULONG nPid = 0, nBase = 0, nLdrpHashTable = 0;
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnNtUnmapViewOfSection MzfNtUnmapViewOfSection = (pfnNtUnmapViewOfSection)GetGlobalVeriable(enumNtUnmapViewOfSection);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	
	if (!MzfObfDereferenceObject || 
		!MzfPsLookupProcessByProcessId ||
		!MzfNtUnmapViewOfSection ||
		!MzfPsGetCurrentThread||
		!MzfObOpenObjectByPointer ||
		!MzfNtClose ||
		!ProcessType)
	{
		return status;
	}
	
	// 检查参数
	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_PROCESS_MODULE))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pEprocess = (PEPROCESS)pCpmu->pEprocess;
	nPid = pCpmu->nPid;
	nBase = pCpmu->op.Unload.Base;
	nLdrpHashTable = pCpmu->op.Unload.LdrpHashTable;

	if (nPid)
	{
		statusLookupProcess = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);
		if (NT_SUCCESS(statusLookupProcess) && !pEprocess)
		{
			pEprocess = pTempEprocess;
		}
	}

	if (!pEprocess)
	{
		return status;
	}

	KdPrint(("enter UnloadDllModule\n"));
	
	if (IsRealProcess(pEprocess))
	{
		HANDLE hProcess = NULL;

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

			status = MzfNtUnmapViewOfSection(hProcess, (PVOID)nBase);
			if (NT_SUCCESS(status))
			{
				RemoveDllModuleInPeb( pEprocess, nBase );
				RemoveDllModuleInLdrpHashTable((LIST_ENTRY *)nLdrpHashTable, pEprocess, nBase);
			}

			MzfNtClose(hProcess);
			RecoverPreMode(pThread, PreMode);
		}
	}

	if (NT_SUCCESS(statusLookupProcess))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}