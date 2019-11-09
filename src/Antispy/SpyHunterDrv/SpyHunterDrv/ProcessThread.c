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
#include "ProcessThread.h"
#include "CommonFunction.h"
#include "Process.h"
#include "InitWindows.h"
#include "ldasm.h"

#define PS_CROSS_THREAD_FLAGS_SYSTEM               0x10

//
// 线程是不是已经在队列里面了
//
BOOL IsThreadInList(PETHREAD pThread, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	BOOL bRet = FALSE;
	ULONG nCntTemp = nCnt > pProcessThreads->nCnt ? pProcessThreads->nCnt : nCnt;
	ULONG i = 0;

	if (!pThread || !pProcessThreads)
	{
		return TRUE;
	}

	for (i = 0; i < nCntTemp; i++)
	{
		if (pProcessThreads->Threads[i].pThread == (ULONG)pThread)
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet; 
}

//
// 得到线程的起始地址
//
ULONG GetThreadStartAddress(PETHREAD pThread)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Win32StartAddressOffset = GetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD); 
	ULONG StartAddressOffset = GetGlobalVeriable(enumStartAddressOffset_ETHREAD); 
	ULONG TebOffset = GetGlobalVeriable(enumTebOffset_KTHREAD);
	ULONG SameThreadApcFlags_Offset = GetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD);
	ULONG nStartAddress = 0;
	
	if (!pThread ||
		!MzfMmIsAddressValid ||
		!MzfMmIsAddressValid(pThread))
	{
		return nStartAddress;
	}

	__try
	{
		nStartAddress = *(PULONG)((ULONG)pThread + StartAddressOffset);

		if ( nBuildNumber < 6000 )
		{
			if ( nStartAddress < (ULONG)MmSystemRangeStart)
			{
				ULONG Win32StartAddress = *(PULONG)((ULONG)pThread + Win32StartAddressOffset);
				if ( Win32StartAddress )
				{
					BOOL bRet = nBuildNumber < 2600u ? 
						*((BYTE *)pThread + SameThreadApcFlags_Offset) == 0 : 
					(*(DWORD *)((char *)pThread + SameThreadApcFlags_Offset) & 1) == 0;

					if ( bRet )
					{
						nStartAddress = Win32StartAddress;
					}
				}
			}
		}
		else
		{
			if ( *(DWORD *)((char *)pThread + SameThreadApcFlags_Offset) & 2 )
			{
				nStartAddress = *(DWORD *)((char *)pThread + Win32StartAddressOffset);
			}
			else
			{
				if ( *(DWORD *)((char *)pThread + Win32StartAddressOffset) )
				{
					nStartAddress = *(DWORD *)((char *)pThread + Win32StartAddressOffset);
				}
			}
		}
	}
	__except(1)
	{}

	return nStartAddress;
}

//
// 插入线程对象
//
VOID InsertThread(PETHREAD pThread, PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	ULONG CidOffset = GetGlobalVeriable(enumCidOffset_ETHREAD);
	ULONG Win32StartAddressOffset = GetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD); 
	ULONG StartAddressOffset = GetGlobalVeriable(enumStartAddressOffset_ETHREAD); 
	ULONG TebOffset = GetGlobalVeriable(enumTebOffset_KTHREAD); 
	ULONG PriorityOffset = GetGlobalVeriable(enumPriorityOffset_KTHREAD); 
	ULONG ContextSwitchesOffset = GetGlobalVeriable(enumContextSwitchesOffset_KTHREAD); 
	ULONG StateOffset = GetGlobalVeriable(enumStateOffset_KTHREAD); 
	PEPROCESS pCurrentProcess = NULL;
	POBJECT_TYPE ThreadType = (POBJECT_TYPE)GetGlobalVeriable(enumPsThreadType);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnObReferenceObjectByPointer MzfObReferenceObjectByPointer = (pfnObReferenceObjectByPointer)GetGlobalVeriable(enumObReferenceObjectByPointer);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfObReferenceObjectByPointer || !MzfMmIsAddressValid || !MzfObfDereferenceObject || !ThreadType ||
		!MzfIoGetCurrentProcess || !MzfKeStackAttachProcess || !MzfKeUnstackDetachProcess)
	{
		return;
	}

	if (pThread && process && MzfMmIsAddressValid((PVOID)pThread) && KeGetObjectType((PVOID)pThread) == ThreadType)
	{ 
		pfnIoThreadToProcess MzfIoThreadToProcess = (pfnIoThreadToProcess)GetGlobalVeriable(enumIoThreadToProcess);
		PEPROCESS pEprocess = NULL;
		
		if (MzfIoThreadToProcess)
		{
			pEprocess = MzfIoThreadToProcess(pThread);
		}
		else
		{
			pEprocess = (PEPROCESS)(*(PULONG)(ULONG)pThread + GetGlobalVeriable(enumThreadsProcessOffset_ETHREAD));
		}

		if (pEprocess == process &&
			!IsThreadInList(pThread, pProcessThreads, nCnt) && 
			NT_SUCCESS(MzfObReferenceObjectByPointer(pThread, 0, NULL, KernelMode)))
		{
			ULONG nCurCnt = pProcessThreads->nCnt;
			if (nCnt > nCurCnt)
			{
				pfnPsGetThreadId MzfPsGetThreadId = (pfnPsGetThreadId)GetGlobalVeriable(enumPsGetThreadId);
				if (MzfPsGetThreadId)
				{
					pProcessThreads->Threads[nCurCnt].Tid = (ULONG)MzfPsGetThreadId(pThread);
				}
				else
				{
					pProcessThreads->Threads[nCurCnt].Tid = *(PULONG)((ULONG)pThread + CidOffset + 4);
				}

				pProcessThreads->Threads[nCurCnt].pThread = (ULONG)pThread;
				pProcessThreads->Threads[nCurCnt].Win32StartAddress = GetThreadStartAddress(pThread);
				pProcessThreads->Threads[nCurCnt].Teb = *(PULONG)((ULONG)pThread + TebOffset);
				pProcessThreads->Threads[nCurCnt].Priority = *(PUCHAR)((ULONG)pThread + PriorityOffset);
				pProcessThreads->Threads[nCurCnt].ContextSwitches = *(PULONG)((ULONG)pThread + ContextSwitchesOffset);
				pProcessThreads->Threads[nCurCnt].State = *(PUCHAR)((ULONG)pThread + StateOffset);
			}

			pProcessThreads->nCnt++;
			MzfObfDereferenceObject(pThread);
		}
	} 
}

////////////////////////////////////////////////////////////////////////////

VOID ProcessThreadListWalker(PLIST_ENTRY pList, PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	ULONG WaitListEntryOffset = GetGlobalVeriable(enumWaitListEntryOffset_KTHREAD);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (pList && MzfMmIsAddressValid(pList) && MzfMmIsAddressValid(pList->Flink))
	{
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
		PLIST_ENTRY pEntry = pList->Flink;
		ULONG nMaxCnt = PAGE_SIZE * 2;

		while (MzfMmIsAddressValid(pEntry) && pEntry != pList && nMaxCnt--)
		{
			ULONG pThread = (ULONG)pEntry - WaitListEntryOffset;

			if ( MzfMmIsAddressValid((PVOID)pThread) )
			{
				InsertThread((PETHREAD)pThread, process, pProcessThreads, nCnt);
			}

			pEntry = pEntry->Flink;
		}

		KeLowerIrql(OldIrql);
	}
}

VOID EnumProcessThreadByWalkerThreadList(PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	ULONG WaitInListHead = GetGlobalVeriable(enumWaitInListHead);
	ULONG DispatcherReadyListHead = GetGlobalVeriable(enumDispatcherReadyListHead);
	
	if (!WaitInListHead)
	{
		GetWaitListHeadAndDispatcherReadyListHead();
		WaitInListHead = GetGlobalVeriable(enumWaitInListHead);
		DispatcherReadyListHead = GetGlobalVeriable(enumDispatcherReadyListHead);
	}

	if (WaitInListHead)
	{
		KdPrint(("walker KiWaitInListHead: 0x%08X\n", WaitInListHead));
		ProcessThreadListWalker((PLIST_ENTRY)WaitInListHead, process, pProcessThreads, nCnt);
	}

	if (DispatcherReadyListHead)
	{
		WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
		KdPrint(("walker g_DispatcherReadyListHead: 0x%08X\n", DispatcherReadyListHead));
		if (WinVersion == enumWINDOWS_2K)
		{
			ProcessThreadListWalker((PLIST_ENTRY)DispatcherReadyListHead, process, pProcessThreads, nCnt);
		}
		else
		{
			ULONG i = 0;
			for (i = 0; i < 32; i++)
			{
				ProcessThreadListWalker((PLIST_ENTRY)DispatcherReadyListHead, process, pProcessThreads, nCnt);
				DispatcherReadyListHead = (ULONG)DispatcherReadyListHead  + sizeof(LIST_ENTRY);
			}
		}
	}
}

VOID ProcessThreadBrowseTableL0(ULONG TableAddr, PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	ULONG Object = 0;
	ULONG ItemCount = 511;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (TableAddr && MzfMmIsAddressValid((PVOID)TableAddr))
	{	
		do 
		{
			TableAddr += 8; 

			if (MzfMmIsAddressValid((PVOID)TableAddr))
			{	
				Object = *(PULONG)TableAddr;
				Object &= 0xfffffff8; 

				InsertThread((PETHREAD)Object, process, pProcessThreads, nCnt);
			}
		} while ( --ItemCount > 0 );
	}
}

//////////////////////////////////////////////////////////////////// 

VOID ProcessThreadBrowseTableL1(ULONG TableAddr, PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (TableAddr && MzfMmIsAddressValid((PVOID)TableAddr))
	{
		do 
		{
			ProcessThreadBrowseTableL0( *(PULONG)TableAddr, process, pProcessThreads, nCnt );
			TableAddr += 4;
		} while (MzfMmIsAddressValid((PVOID)TableAddr) && *(PULONG)TableAddr != 0);
	}
}

//////////////////////////////////////////////////////////////////// 

VOID ProcessThreadBrowseTableL2(ULONG TableAddr, PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (TableAddr && MzfMmIsAddressValid((PVOID)TableAddr))
	{
		do 
		{
			ProcessThreadBrowseTableL1( *(PULONG)TableAddr, process, pProcessThreads, nCnt );
			TableAddr += 4;
		} while (MzfMmIsAddressValid((PVOID)TableAddr) && *(PULONG)TableAddr != 0);
	}
}

//////////////////////////////////////////////////////////////////// 

VOID ScanNotWin2KHandleTableToFindThread(ULONG uHandleTable, PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	ULONG HandleTable = 0;
	ULONG TableCode = 0;
	ULONG flag = 0;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (uHandleTable && MzfMmIsAddressValid((PVOID)uHandleTable))
	{
		HandleTable = *(PULONG)uHandleTable;
		if (HandleTable && MzfMmIsAddressValid((PVOID)HandleTable))
		{
			if (WinVersion == enumWINDOWS_8)
			{
				TableCode = *(PULONG)(HandleTable + 8);
			}
			else
			{
				TableCode = *(PULONG)(HandleTable);
			}

			flag = TableCode & 3;
			TableCode &= 0xfffffffc;  

			switch (flag)
			{
			case 0:
				ProcessThreadBrowseTableL0(TableCode, process, pProcessThreads, nCnt);
				break;

			case 1:
				ProcessThreadBrowseTableL1(TableCode, process, pProcessThreads, nCnt);
				break;

			case 2:
				ProcessThreadBrowseTableL2(TableCode, process, pProcessThreads, nCnt);
				break; 

			default:
				KdPrint(("TableCode error\n"));
			} 			
		}
	}
}

//////////////////////////////////////////////////////////////////// 

// VOID ScanWin2KHandleTableToFindThread(PWIN2K_HANDLE_TABLE HandleTable, PEPROCESS process, PALL_THREADS pProcessThreads)
// {
// 	int i, j, k;
// 	PHANDLE_TABLE_ENTRY Entry;
// 
// 	if (!HandleTable)
// 	{
// 		return;
// 	}
// 
// 	for (i = 0; i < 0x100; i++)
// 	{
// 		if (HandleTable->Table[i])
// 		{
// 			for (j = 0; j < 0x100; j++)
// 			{
// 				if (HandleTable->Table[i][j])
// 				{
// 					for (k = 0; k < 0x100; k++)
// 					{
// 						Entry = &HandleTable->Table[i][j][k];
// 						if (Entry->Object)
// 						{
// 							InsertThread((PETHREAD)((ULONG)(Entry->Object) | 0x80000000), process, pProcessThreads);
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// }

//////////////////////////////////////////////////////////////////// 

VOID EnumProcessThreadByPspCidTable(PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	ULONG PspCidTable = GetGlobalVeriable(enumPspCidTable);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (!PspCidTable)
	{
		GetPspCidTable();
		PspCidTable = GetGlobalVeriable(enumPspCidTable);
	}

	if ( process && MzfMmIsAddressValid(process) &&
		PspCidTable && MzfMmIsAddressValid((PVOID)PspCidTable) )
	{
		WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);

		if (WinVersion == enumWINDOWS_2K)
		{	
		//	ScanWin2KHandleTableToFindThread(*(PWIN2K_HANDLE_TABLE*)PspCidTable, process, pProcessThreads);
		}
		else
		{
			ScanNotWin2KHandleTableToFindThread(PspCidTable, process, pProcessThreads, nCnt);
		}
 	}
}

//
// 遍历两个系统线程列表
//
VOID EnumProcessThreadByList(PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	ULONG ThreadListHeadOffset = GetGlobalVeriable(enumThreadListHeadOffset_KPROCESS);
	ULONG ThreadListEntryOffset = GetGlobalVeriable(enumThreadListEntryOffset_KTHREAD);
	ULONG ThreadListHeadOffset1 = GetGlobalVeriable(enumThreadListHeadOffset_EPROCESS);
	ULONG ThreadListEntryOffset1 = GetGlobalVeriable(enumThreadListEntryOffset_ETHREAD);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (process && MzfMmIsAddressValid(process))
	{
		PLIST_ENTRY pList = (PLIST_ENTRY)((ULONG)process + ThreadListHeadOffset);
		if (pList && MzfMmIsAddressValid(pList) && MzfMmIsAddressValid(pList->Flink))
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			ULONG nMaxCnt = PAGE_SIZE * 2;
			PLIST_ENTRY pEntry = pList->Flink;
			while (MzfMmIsAddressValid(pEntry) && pEntry != pList && nMaxCnt--)
			{
				PETHREAD pThread = (PETHREAD)((ULONG)pEntry - ThreadListEntryOffset);
				InsertThread(pThread, process, pProcessThreads, nCnt);
				pEntry = pEntry->Flink;
			}

			KeLowerIrql(OldIrql);
		}
		
		pList = (PLIST_ENTRY)((ULONG)process + ThreadListHeadOffset1);
		if (pList && MzfMmIsAddressValid(pList) && MzfMmIsAddressValid(pList->Flink))
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			ULONG nMaxCnt = PAGE_SIZE * 2;
			PLIST_ENTRY pEntry = pList->Flink;
			while (MzfMmIsAddressValid(pEntry) && pEntry != pList && nMaxCnt--)
			{
				PETHREAD pThread = (PETHREAD)((ULONG)pEntry - ThreadListEntryOffset1);
				InsertThread(pThread, process, pProcessThreads, nCnt);
				pEntry = pEntry->Flink;
			}

			KeLowerIrql(OldIrql);
		}
	}
}

//
// 枚举线程
//
void EnumThreads(PEPROCESS process, PALL_THREADS pProcessThreads, ULONG nCnt)
{
	if (!process || !pProcessThreads)
	{
		return;
	}
	
	EnumProcessThreadByList(process, pProcessThreads, nCnt);
 	EnumProcessThreadByPspCidTable(process, pProcessThreads, nCnt);
 	EnumProcessThreadByWalkerThreadList(process, pProcessThreads, nCnt);
}

//
// 枚举进程线程
//
NTSTATUS ListProcessThread(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_THREAD pPt = (PCOMMUNICATE_THREAD)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0, nCnt = 0;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	//
	// 检测参数
	//
	if (!pInBuffer								||
		!pOutBuffer								||
		uInSize != sizeof(COMMUNICATE_THREAD)	||
		uOutSize < sizeof(ALL_THREADS))
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	nCnt = (uOutSize - sizeof(ALL_THREADS)) / sizeof(THREAD_INFO);
	pEprocess = (PEPROCESS)pPt->pEprocess;
	nPid = pPt->nPid;
	
	if (nPid != 0)
	{
		TempStatus = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);
		if (NT_SUCCESS(TempStatus) && pEprocess == NULL)
		{
			pEprocess = pTempEprocess;
		}
	}
	
	if (IsRealProcess(pEprocess))
	{
		EnumThreads(pEprocess, (PALL_THREADS)pOutBuffer, nCnt);
		if (nCnt >= ((PALL_THREADS)pOutBuffer)->nCnt)
		{
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

VOID EraseSystemThreadFlags(PVOID pThread)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG KernelApcDisableOffset_KTHREAD = GetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD);
	ULONG CrossThreadFlagsOffset_ETHREAD = GetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD);

	if (!pThread || 
		!MzfMmIsAddressValid || 
		!KernelApcDisableOffset_KTHREAD || 
		!CrossThreadFlagsOffset_ETHREAD ||
		!MzfMmIsAddressValid(pThread))
	{
		return;
	}

	if (MzfMmIsAddressValid((PVOID)((ULONG)pThread + CrossThreadFlagsOffset_ETHREAD)))
	{
		// #define PS_CROSS_THREAD_FLAGS_SYSTEM 0x10
		// 去掉系统线程标志位
 //		*(PCHAR)((ULONG)pThread + CrossThreadFlagsOffset_ETHREAD) &= ~PS_CROSS_THREAD_FLAGS_SYSTEM;	
 //		*(PCHAR)((ULONG)pThread + CrossThreadFlagsOffset_ETHREAD) &= 0xFB; 

		(*(PULONG)((ULONG)pThread + CrossThreadFlagsOffset_ETHREAD)) &= 0xFFFFFFEF;	
		(*(PCHAR)((ULONG)pThread + CrossThreadFlagsOffset_ETHREAD)) &= 0xFB;
	} 

	if (MzfMmIsAddressValid((PVOID)((ULONG)pThread + KernelApcDisableOffset_KTHREAD)))
	{
		*(PULONG)((ULONG)pThread + KernelApcDisableOffset_KTHREAD) = 0;	
	}
}

ULONG GetPspTerminateThreadByPointer()
{
	ULONG PspTerminateThreadByPointer = 0;
	pfnNtTerminateThread MzfNtTerminateThread = (pfnNtTerminateThread)GetGlobalVeriable(enumNtTerminateThread);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	KdPrint(("MzfNtTerminateThread: 0x%08X, MzfMmIsAddressValid : 0x%08X\n", MzfNtTerminateThread, MzfMmIsAddressValid));
	if (MzfNtTerminateThread && MzfMmIsAddressValid)
	{
		ULONG i = 0, n = 0;
		ULONG uCodeSize = 0;
		for (i = (ULONG)MzfNtTerminateThread; i < (ULONG)MzfNtTerminateThread + PAGE_SIZE; i += uCodeSize)
		{
			PCHAR pCode;

			if (!MzfMmIsAddressValid((PVOID)i))
			{
				break;
			}

			uCodeSize = SizeOfCode((PVOID)i, &pCode);
			if (uCodeSize == 5 && *((PBYTE)i) == 0xE8)
			{
				KdPrint(("n: %d   0x%08X\n", n, i));
				n++;
			}

			if (n == 3)
			{
				PspTerminateThreadByPointer = i + *(PULONG)(i + 1) + 5;
				break;
			}
		}
	}

	return PspTerminateThreadByPointer;
}

NTSTATUS TerminateThreadByNtTerminateThread(PETHREAD pThread)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	KdPrint(("TerminateThreadByNtTerminateThread\n"));
	if (pThread)
	{
		pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
		pfnNtTerminateThread MzfNtTerminateThread = (pfnNtTerminateThread)GetGlobalVeriable(enumNtTerminateThread);
		pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
		pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
		POBJECT_TYPE ThreadObject = (POBJECT_TYPE)GetGlobalVeriable(enumPsThreadType);

		if (MzfObOpenObjectByPointer && 
			MzfNtTerminateThread && 
			ThreadObject &&
			MzfNtClose &&
			MzfPsGetCurrentThread)
		{
			HANDLE hThread = NULL;
			status = MzfObOpenObjectByPointer(pThread, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, ThreadObject, KernelMode, &hThread);
			if (NT_SUCCESS(status))
			{
				PETHREAD pTempThread = MzfPsGetCurrentThread();
				CHAR PreMode = ChangePreMode(pTempThread);

				status = MzfNtTerminateThread(hThread, 0);

				MzfNtClose(hThread);
				RecoverPreMode(pTempThread, PreMode);
			}
		}
	}

	return status;
}

//
// 结束线程的APC例程
//
void KernelTerminateThreadRoutine( 
	IN PKAPC Apc, 
	IN OUT PKNORMAL_ROUTINE *NormalRoutine, 
	IN OUT PVOID *NormalContext, 
	IN OUT PVOID *SystemArgument1, 
	IN OUT PVOID *SystemArgument2 
	) 
{ 
	PULONG ThreadFlags;
	BOOL bInit = FALSE;
	ULONG CrossThreadFlagsOffset_ETHREAD = GetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD);
	ULONG ulActiveExWorkerOffset_ETHREAD = GetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnPsTerminateSystemThread MzfPsTerminateSystemThread = (pfnPsTerminateSystemThread)GetGlobalVeriable(enumPsTerminateSystemThread);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	PETHREAD pThread = NULL;
	BOOL bSetSystemFlag = FALSE, bActiveExWorker = FALSE;

	if (!MzfExFreePoolWithTag ||
		!MzfPsGetCurrentThread ||
		!MzfPsTerminateSystemThread ||
		!MzfMmIsAddressValid)
	{
		return;
	}

	if (Apc)
	{
		MzfExFreePoolWithTag(Apc, 0);
	}

	KdPrint(("KernelTerminateThreadRoutine\n"));

	pThread = MzfPsGetCurrentThread();

	if (MzfMmIsAddressValid(((PBYTE)pThread + CrossThreadFlagsOffset_ETHREAD)))
	{
		// 设置系统线程标志
		ThreadFlags = (ULONG *)((ULONG)pThread + CrossThreadFlagsOffset_ETHREAD); 
		*ThreadFlags = (*ThreadFlags) | PS_CROSS_THREAD_FLAGS_SYSTEM; 
		
		bSetSystemFlag = TRUE;
	}

	if (MzfMmIsAddressValid(((PBYTE)pThread + ulActiveExWorkerOffset_ETHREAD)))
	{
		// 要结束工作队列的线程，必须设置标志
		*(PULONG)((ULONG)pThread + ulActiveExWorkerOffset_ETHREAD) = FALSE; 
		bActiveExWorker = TRUE;

		/*
		   wrk -> PspExitThread

		   if (Thread->ActiveExWorker) {
               KeBugCheckEx (ACTIVE_EX_WORKER_THREAD_TERMINATION,
                      (ULONG_PTR)Thread,
                      0,
                      0,
                      0);
           }
		*/
	}

	if (bSetSystemFlag && bActiveExWorker)
	{
		NTSTATUS status = MzfPsTerminateSystemThread(STATUS_SUCCESS);
		KdPrint(("MzfPsTerminateSystemThread error: 0x%08X, DosErrorCode: %d\n", status, RtlNtStatusToDosError(status)));
	}
}

//
// 通过插APC的方法结束线程
//
BOOL KillThreadByQueueApc(PETHREAD Thread)
{
	PKAPC Apc = NULL; 
	NTSTATUS status = STATUS_UNSUCCESSFUL; 
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnKeInitializeApc MzfKeInitializeApc = (pfnKeInitializeApc)GetGlobalVeriable(enumKeInitializeApc);
	pfnKeInsertQueueApc MzfKeInsertQueueApc = (pfnKeInsertQueueApc)GetGlobalVeriable(enumKeInsertQueueApc);

	if (!MzfMmIsAddressValid ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!MzfKeInitializeApc ||
		!MzfKeInsertQueueApc ||
		!Thread				||
		!MzfMmIsAddressValid(Thread))
	{
		return status;
	}

	KdPrint(("KillThreadByQueueApc\n"));

	Apc = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(KAPC), MZFTAG); 
	if (!Apc)
	{
		return status; 
	}

	MzfKeInitializeApc(Apc, 
		Thread, 
		OriginalApcEnvironment, 
		KernelTerminateThreadRoutine, 
		NULL, 
		NULL, 
		KernelMode, 
		NULL);

	if (MzfKeInsertQueueApc(Apc, NULL, NULL, 2))
	{
		status = STATUS_SUCCESS;
	}
	
	return status; 
}

//
// 结束线程
//
NTSTATUS TerminateThread(PETHREAD pThread)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	
	status = TerminateThreadByNtTerminateThread(pThread);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("NtTerminateThread error: 0x%08X, DosErrorCode: %d\n", status, RtlNtStatusToDosError(status)));
		status = KillThreadByQueueApc(pThread);
	}

	return status;
}

//
// 杀线程
//
NTSTATUS KillThread(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_THREAD pCkt = (PCOMMUNICATE_THREAD)pInBuffer;
	PETHREAD pThread = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!MzfMmIsAddressValid)
	{
		return status;
	}

	// 检查参数
	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_THREAD))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pThread = (PETHREAD)pCkt->op.Kill.pThread;
	if (pThread && MzfMmIsAddressValid(pThread) && IsRealThread(pThread))
	{
		status = TerminateThread(pThread);
	}

	return status;
}

//
// 挂起或恢复线程
//
NTSTATUS PspSuspendOrResumeThread(PETHREAD pThread, BOOL bSuspend)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnNtSuspendThread MzfNtSuspendThread = (pfnNtSuspendThread)GetGlobalVeriable(enumNtSuspendThread);
	pfnNtResumeThread MzfNtResumeThread = (pfnNtResumeThread)GetGlobalVeriable(enumNtResumeThread);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	POBJECT_TYPE ThreadObject = (POBJECT_TYPE)GetGlobalVeriable(enumPsThreadType);
	HANDLE hThread;
	PETHREAD pCurrentThread = NULL;
	CHAR PreMode = 0;

	if (!MzfNtSuspendThread ||
		!MzfNtResumeThread ||
		!MzfPsGetCurrentThread ||
		!MzfObOpenObjectByPointer ||
		!MzfNtClose ||
		!ThreadObject ||
		!pThread)
	{
		return status;
	}

	//
	// 如果是当前线程就直接返回了
	//
	pCurrentThread = MzfPsGetCurrentThread();
	if (pCurrentThread == pThread)
	{
		status = STATUS_SUCCESS;
	}
	else
	{
		status = MzfObOpenObjectByPointer(pThread, 
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
			NULL, 
			GENERIC_ALL, 
			ThreadObject, 
			KernelMode, 
			&hThread
			);

		if (NT_SUCCESS(status))
		{
			ULONG PreviousSuspendCount = 0;
			PreMode = ChangePreMode(pCurrentThread);
			if (bSuspend)
			{
				status = MzfNtSuspendThread(hThread, &PreviousSuspendCount);
			}
			else
			{
				status = MzfNtResumeThread(hThread, &PreviousSuspendCount);
			}
			
			MzfNtClose(hThread);
			RecoverPreMode(pCurrentThread, PreMode);
		}
	}

	return status;
}

//
// 挂起或恢复线程
//
NTSTATUS SuspendOrResumeThread(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_THREAD pCst = (PCOMMUNICATE_THREAD)pInBuffer;
	PETHREAD pThread = NULL;
	BOOL bSuspend = FALSE;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!MzfMmIsAddressValid)
	{
		return status;
	}

	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_THREAD))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pThread = (PETHREAD)pCst->op.Suspend.pThread;
	bSuspend = pCst->op.Suspend.bSuspend;

	if (pThread && MzfMmIsAddressValid(pThread) && IsRealThread(pThread))
	{
		ULONG SuspendCountOffset = GetGlobalVeriable(enumSuspendCountOffset_KTHREAD);
		if (MzfMmIsAddressValid((PCHAR)((ULONG)pThread + SuspendCountOffset)))
		{
			CHAR nSusCnt = *(PCHAR)((ULONG)pThread + SuspendCountOffset);

			// 如果是要挂起线程，那么挂起计数必须为0
			// 如果是要恢复线程，那么挂起计数必须>0	
			if ( (bSuspend && nSusCnt == 0) ||
				 (!bSuspend && nSusCnt > 0) )
			{
				status = PspSuspendOrResumeThread(pThread, bSuspend);
			}
		}
	}

	return status;
}

//
// 获取线程的挂起计数
//
NTSTATUS PspGetThreadSuspendCount(PETHREAD pThread, ULONG *nRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG SuspendCountOffset = GetGlobalVeriable(enumSuspendCountOffset_KTHREAD);

	if (!pThread ||
		!MzfMmIsAddressValid ||
		!MzfMmIsAddressValid(pThread) ||
		!MzfMmIsAddressValid((PVOID)((ULONG)pThread + SuspendCountOffset)) ||
		!nRet)
	{
		return status;
	}

	*nRet = *(PCHAR)((ULONG)pThread + SuspendCountOffset);
	status = STATUS_SUCCESS;
	return status;
}

//
// 获取线程的挂起计数
//
NTSTATUS GetThreadSuspendCount(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_THREAD pCgt = (PCOMMUNICATE_THREAD)pInBuffer;
	PETHREAD pThread = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!MzfMmIsAddressValid)
	{
		return status;
	}
	
	// 参数检查
	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_THREAD) ||
		!pOutBuffer ||
		uOutSize != sizeof(ULONG))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pThread = (PETHREAD)pCgt->op.GetSuspendCnt.pThread;

	if (pThread && MzfMmIsAddressValid(pThread) && IsRealThread(pThread))
	{
		status = PspGetThreadSuspendCount(pThread, pOutBuffer);
	}

	return status;
}