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
#include "IoTimer.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "ldasm.h"

ULONG GetIopTimerQueueHead()
{
	ULONG IopTimerQueueHead = 0;
	ULONG pIoInitializeTimer = GetGlobalVeriable(enumIoInitializeTimer);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (pIoInitializeTimer && MzfMmIsAddressValid)
	{
		ULONG nStart = pIoInitializeTimer;
		ULONG nEnd = pIoInitializeTimer + PAGE_SIZE;
		ULONG nCodeLen = 0;
		BYTE btSign = 0xb9;

		KdPrint(("pIoInitializeTimer: 0X%08x\n", pIoInitializeTimer));

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

			if (nCodeLen == 5 && *(PBYTE)nStart == btSign)
			{
				IopTimerQueueHead = *(PULONG)(nStart + 1);
				if (IopTimerQueueHead && MzfMmIsAddressValid((PVOID)IopTimerQueueHead))
				{
					KdPrint(("IopTimerQueueHead: 0x%08X\n", IopTimerQueueHead));
					SetGlobalVeriable(enumIopTimerQueueHead, IopTimerQueueHead);
					break;
				}
			}
		}
	}

	return IopTimerQueueHead;
}

NTSTATUS EnumIoTimer(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PLIST_ENTRY IopTimerQueueHead = (PLIST_ENTRY)GetGlobalVeriable(enumIopTimerQueueHead);
	PIO_TIMER_INFO pIoTimerInfo = (PIO_TIMER_INFO)pOutBuffer;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KIRQL OldIrql;

	if (!MzfMmIsAddressValid)
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (!IopTimerQueueHead)
	{
		GetIopTimerQueueHead();
		IopTimerQueueHead = (PLIST_ENTRY)GetGlobalVeriable(enumIopTimerQueueHead);
	}

	KdPrint(("IopTimerQueueHead: 0x%08X\n", IopTimerQueueHead));

	OldIrql = KeRaiseIrqlToDpcLevel();

	if (IopTimerQueueHead && MzfMmIsAddressValid((PVOID)IopTimerQueueHead))
	{
		PLIST_ENTRY timerEntry = IopTimerQueueHead->Flink;
		
		while (MzfMmIsAddressValid(timerEntry) && timerEntry != IopTimerQueueHead)
		{
			PIO_TIMER timer = CONTAINING_RECORD( timerEntry, IO_TIMER, TimerList );
			
			if (timer && MzfMmIsAddressValid(timer))
			{
				KdPrint(("TimerFlag: %d, timer: 0x%08X, TimerRoutine: 0x%08X\n", 
					timer->TimerFlag, timer, timer->TimerRoutine));

				if (pIoTimerInfo->nCnt > pIoTimerInfo->nRetCnt)
				{
					pIoTimerInfo->IoTimer[pIoTimerInfo->nRetCnt].TimerObject = (ULONG)timer;
					pIoTimerInfo->IoTimer[pIoTimerInfo->nRetCnt].TimerEntry = (ULONG)timerEntry;
					pIoTimerInfo->IoTimer[pIoTimerInfo->nRetCnt].DeviceObject = (ULONG)timer->DeviceObject;
					pIoTimerInfo->IoTimer[pIoTimerInfo->nRetCnt].TimeDispatch = (ULONG)timer->TimerRoutine;
					pIoTimerInfo->IoTimer[pIoTimerInfo->nRetCnt].Status = (ULONG)timer->TimerFlag;
				}

				pIoTimerInfo->nRetCnt++;
			}

			timerEntry = timerEntry->Flink;
		}
	}
	
	KeLowerIrql(OldIrql);

	return status;
}

NTSTATUS StartOrStopIoTimer(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_IO_TIMER pIoTimerInfo = (PCOMMUNICATE_IO_TIMER)pInBuffer;
	PDEVICE_OBJECT pDeviceObject = (PDEVICE_OBJECT)pIoTimerInfo->op.StopOrStartTimer.pDeviceObject;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!MzfMmIsAddressValid ||
		!pDeviceObject ||
		!MzfMmIsAddressValid(pDeviceObject))
	{
		return status;
	}

	if (pIoTimerInfo->op.StopOrStartTimer.bStart)
	{
		pfnIoStartTimer MzfIoStartTimer = (pfnIoStartTimer)GetGlobalVeriable(enumIoStartTimer);
		if (MzfIoStartTimer)
		{
			MzfIoStartTimer(pDeviceObject);
			status = STATUS_SUCCESS;
		}
	}
	else 
	{
		pfnIoStopTimer MzfIoStopTimer = (pfnIoStopTimer)GetGlobalVeriable(enumIoStopTimer);
		MzfIoStopTimer(pDeviceObject);
		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS RemoveIoTimer(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PLIST_ENTRY IopTimerQueueHead = (PLIST_ENTRY)GetGlobalVeriable(enumIopTimerQueueHead);
	PCOMMUNICATE_IO_TIMER pIoTimerInfo = (PCOMMUNICATE_IO_TIMER)pInBuffer;
	PLIST_ENTRY pTimerEntry = (PLIST_ENTRY)pIoTimerInfo->op.Remove.TimerEntry;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	KIRQL OldIrql;

	if (!MzfMmIsAddressValid ||
		!MzfExFreePoolWithTag ||
		!pTimerEntry ||
		!MzfMmIsAddressValid(pTimerEntry))
	{
		return status;
	}

	if (!IopTimerQueueHead)
	{
		GetIopTimerQueueHead();
		IopTimerQueueHead = (PLIST_ENTRY)GetGlobalVeriable(enumIopTimerQueueHead);
	}

	KdPrint(("IopTimerQueueHead: 0x%08X\n", IopTimerQueueHead));

	OldIrql = KeRaiseIrqlToDpcLevel();

	if (IopTimerQueueHead && MzfMmIsAddressValid((PVOID)IopTimerQueueHead))
	{
		PLIST_ENTRY timerEntry = IopTimerQueueHead->Flink;

		while (MzfMmIsAddressValid(timerEntry) && timerEntry != IopTimerQueueHead)
		{
			if (timerEntry == pTimerEntry)
			{
				PIO_TIMER timer = CONTAINING_RECORD( timerEntry, IO_TIMER, TimerList );
				if (timer && MzfMmIsAddressValid(timer))
				{
					RemoveEntryList(timerEntry);
					MzfExFreePoolWithTag(timer, 0);
					status = STATUS_SUCCESS;
				}

				break;
			}
			
			timerEntry = timerEntry->Flink;
		}
	}

	KeLowerIrql(OldIrql);

	return status;
}