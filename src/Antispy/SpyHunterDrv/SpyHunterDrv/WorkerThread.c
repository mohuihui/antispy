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
#include "WorkerThread.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "ldasm.h"

ULONG FindExWorkerQueue()
{
	ULONG pExWorkerQueue = 0;
	ULONG pExQueueWorkItem = GetGlobalVeriable(enumExQueueWorkItem);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid && pExQueueWorkItem)
	{
		ULONG nStart = pExQueueWorkItem;
		ULONG nEnd = pExQueueWorkItem + PAGE_SIZE;
		ULONG nCodeLen = 0;
		BYTE btSign = 0;
		ULONG nExWorkerQueueCodeLen = 0, nExWorkerQueueOffset = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		switch (Version)
		{
		case enumWINDOWS_2K:
			btSign = 0x05;
			nExWorkerQueueCodeLen = 5;
			nExWorkerQueueOffset = 1;
			break;
			
		case enumWINDOWS_XP:
		case enumWINDOWS_2K3:
		case enumWINDOWS_2K3_SP1_SP2:
		case enumWINDOWS_VISTA:
		case enumWINDOWS_VISTA_SP1_SP2:
		case enumWINDOWS_7:
			btSign = 0x81;
			nExWorkerQueueCodeLen = 6;
			nExWorkerQueueOffset = 2;
			break;
		}
		
		KdPrint(("pExQueueWorkItem: 0X%08x\n", pExQueueWorkItem));

		for (; nStart < nEnd; nStart += nCodeLen)
		{
			PCHAR pCode;

			if (!MzfMmIsAddressValid((PVOID)(nStart)))
			{
				break;	
			}

			nCodeLen = SizeOfCode((PVOID)nStart, &pCode);
			if (nCodeLen == 0)
			{
				break;
			}

			if (nCodeLen == nExWorkerQueueCodeLen && *(PBYTE)nStart == btSign)
			{
				pExWorkerQueue = *(PULONG)(nStart + nExWorkerQueueOffset);
				if (pExWorkerQueue && MzfMmIsAddressValid((PVOID)pExWorkerQueue))
				{
					KdPrint(("pExWorkerQueue: 0x%08X\n", pExWorkerQueue));
					SetGlobalVeriable(enumExWorkerQueue, pExWorkerQueue);
					break;
				}
			}
		}
	}

	return pExWorkerQueue;
}

ULONG 
PspGetThreadEntry(
				  PEPROCESS pEprocess, 
				  PETHREAD pThread, 
				  ULONG KernelStack, 
				  ULONG InitialStack , 
				  ULONG KernelBase, 
				  ULONG KernelEnd
				  )
{
	ULONG ThreadEntry = 0;
	KAPC_STATE as;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	ULONG KernelStackMaxSize = 0, KernelStackFirst = 0, KernelStackSecond = 0, ThreadEntryOffset = 0;
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

	if (!MzfMmIsAddressValid ||
		!MzfKeStackAttachProcess ||
		!MzfKeUnstackDetachProcess)
	{
		return 0;
	}

	if (Version == enumWINDOWS_2K)
	{
		KernelStackMaxSize = 48;
		KernelStackFirst = 4;
		KernelStackSecond = 5;
		ThreadEntryOffset = 2;
	}
	else if (Version == enumWINDOWS_XP)
	{
		KernelStackMaxSize = 48;
		KernelStackFirst = 3;
		KernelStackSecond = 4;
		ThreadEntryOffset = 2;
	}
	else if (Version == enumWINDOWS_2K3 || Version == enumWINDOWS_2K3_SP1_SP2)
	{
		KernelStackMaxSize = 48;
		KernelStackFirst = 7;
		KernelStackSecond = 8;
		ThreadEntryOffset = 6;
	}
	else if (Version == enumWINDOWS_VISTA || Version == enumWINDOWS_VISTA_SP1_SP2)
	{
		KernelStackMaxSize = 48;
		KernelStackFirst = 10;
		KernelStackSecond = 11;
		ThreadEntryOffset = 8;
	}
	else if (Version == enumWINDOWS_7)
	{
		KernelStackMaxSize = 80;
		KernelStackFirst = 16;
		KernelStackSecond = 17;
		ThreadEntryOffset = 10;
	}

	MzfKeStackAttachProcess(pEprocess, &as);

	__try
	{
		PVOID pKernelStack = (PVOID)KernelStack;
		ULONG pRecallKernelStack = 0;

		if ( (unsigned int)KernelStack < InitialStack && MzfMmIsAddressValid((PVOID)(KernelStack)))
		{
			do
			{
				pRecallKernelStack = (ULONG)((char *)pKernelStack + sizeof(ULONG));

				if ( (ULONG)((char *)pKernelStack + KernelStackMaxSize) > InitialStack )
				{
					break;
				}

				if ( MzfMmIsAddressValid((PVOID)pKernelStack) &&
					*(ULONG*)pKernelStack == (ULONG)pThread &&
					!*(ULONG *)pRecallKernelStack)
				{
					ULONG nFirst = *((ULONG *)pKernelStack + KernelStackFirst);

					if ( nFirst > KernelStack && nFirst < InitialStack)
					{
						ULONG nSecond = *((ULONG *)pKernelStack + KernelStackSecond);

						if ( nSecond > KernelBase )
						{
							if ( nSecond < KernelEnd )
							{
								ThreadEntry = *((ULONG *)pKernelStack + ThreadEntryOffset);
								KdPrint(("Thread: 0x%08X, 0x%08X\n", pThread, ThreadEntry));
								break;
							}

							pRecallKernelStack = (int)((char *)pKernelStack + sizeof(ULONG));
						}
					}
				}

				pKernelStack = (void *)pRecallKernelStack;

			}while ( pRecallKernelStack < InitialStack );
		}
	}
	__except(1)
	{
	}

	MzfKeUnstackDetachProcess(&as);
	return ThreadEntry;
}

ULONG GetThreadEntry(PETHREAD pThread)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoThreadToProcess MzfIoThreadToProcess = (pfnIoThreadToProcess)GetGlobalVeriable(enumIoThreadToProcess);
	ULONG KernelStackOffset = GetGlobalVeriable(enumKernelStackOffset_KTHREAD);
	ULONG InitialStackOffset = GetGlobalVeriable(enumInitialStackOffset_KTHREAD);
	ULONG nRet = 0;
	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);
	
	if (IsRealThread(pThread))
	{
		if (InitialStackOffset &&
			KernelStackOffset &&
			MzfMmIsAddressValid && 
			pThread &&
			MzfIoThreadToProcess &&
			MzfMmIsAddressValid(pThread) &&
			MzfMmIsAddressValid((PVOID)((ULONG)pThread + KernelStackOffset)) &&
			MzfMmIsAddressValid((PVOID)((ULONG)pThread + InitialStackOffset)))
		{
			ULONG KernelStack = *(PULONG)((ULONG)pThread + KernelStackOffset);
			ULONG InitialStack = *(PULONG)((ULONG)pThread + InitialStackOffset);
			PEPROCESS pEprocess = MzfIoThreadToProcess(pThread);
			WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

			nRet = PspGetThreadEntry(pEprocess, pThread, KernelStack, InitialStack, KernelBase, KernelBase + KernelSize);
		}
	}
	
	if (nRet < SYSTEM_ADDRESS_START)
	{
		nRet = 0;
	}

	return nRet;
}

NTSTATUS EnumWorkerThread(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG pExWorkerQueue = GetGlobalVeriable(enumExWorkerQueue);
	PWORKER_THREAD_INFO pWorkerThreadInfo = (PWORKER_THREAD_INFO)pOutBuffer;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG QueueListEntryOffset = GetGlobalVeriable(enumQueueListEntryOffset_KTHREAD);

	if (!pExWorkerQueue)
	{
		FindExWorkerQueue();
		pExWorkerQueue = GetGlobalVeriable(enumExWorkerQueue);
	}

	if (pExWorkerQueue && MzfMmIsAddressValid((PVOID)pExWorkerQueue) && QueueListEntryOffset)
	{
		PEX_WORK_QUEUE ExWorkerQueue = (PEX_WORK_QUEUE)pExWorkerQueue;
		ULONG i = 0;

		for (i = CriticalWorkQueue; i < MaximumWorkQueue; i++)
		{
			PLIST_ENTRY Entry = NULL;
			
			for (Entry = (PLIST_ENTRY) ExWorkerQueue[i].WorkerQueue.ThreadListHead.Flink;
				Entry && (Entry != (PLIST_ENTRY) &ExWorkerQueue[i].WorkerQueue.ThreadListHead);
				Entry = Entry->Flink) 
			{
				if (pWorkerThreadInfo->nCnt > pWorkerThreadInfo->nRetCnt)
				{
					KdPrint(("thread: 0x%08X\n", (ULONG)Entry - QueueListEntryOffset));
					pWorkerThreadInfo->Thread[pWorkerThreadInfo->nRetCnt].ThreadObject = (ULONG)Entry - QueueListEntryOffset;
					pWorkerThreadInfo->Thread[pWorkerThreadInfo->nRetCnt].Dispatch = 
						GetThreadEntry((PETHREAD)pWorkerThreadInfo->Thread[pWorkerThreadInfo->nRetCnt].ThreadObject);;
					pWorkerThreadInfo->Thread[pWorkerThreadInfo->nRetCnt].Type = i;
				}

				pWorkerThreadInfo->nRetCnt++;
			}
		}
		
		status = STATUS_SUCCESS;
	}

	return status;
}