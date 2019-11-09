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
#include "ProcessTimer.h"
#include "ldasm.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"

VOID FindgptmrFirstOrgtmrListHeadWin2kToWin7()
{
	ULONG pNtUserValidateTimerCallback= GetGlobalVeriable(enumNtUserValidateTimerCallback);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	ULONG i = 0, nCodeLen = 0, n0xE8 = 0;
	PCHAR OpCode = NULL;
	ULONG pValidateTimerCallback = 0;
	ULONG gtmrListHead = 0;

	if (!pNtUserValidateTimerCallback || !MzfMmIsAddressValid || !MzfMmIsAddressValid((PVOID)pNtUserValidateTimerCallback))
	{
		return;
	}
	
	for (i = pNtUserValidateTimerCallback; i < pNtUserValidateTimerCallback + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(BYTE *)OpCode == 0xE8)
		{
			n0xE8++;

			if (WinVersion == enumWINDOWS_2K && n0xE8 == 3)
			{
				pValidateTimerCallback = i + *((PULONG)(i + 1)) + 5;
				KdPrint(("win2000 -> pValidateTimerCallback: 0x%08X\n", pValidateTimerCallback));
				break;
			}
			else if (
				(
				WinVersion == enumWINDOWS_XP || 
				WinVersion == enumWINDOWS_2K3 || 
				WinVersion == enumWINDOWS_2K3_SP1_SP2 || 
				WinVersion == enumWINDOWS_VISTA ||
				WinVersion == enumWINDOWS_VISTA_SP1_SP2 ||
				WinVersion == enumWINDOWS_7
				) && 
				n0xE8 == 2)
			{
				pValidateTimerCallback = i + *((PULONG)(i + 1)) + 5;
				KdPrint(("winxp -> pValidateTimerCallback: 0x%08X\n", pValidateTimerCallback));
				break;
			}
		}
	}

	if (!MzfMmIsAddressValid((PVOID)pValidateTimerCallback))
	{
		return;
	}

	for (i = pValidateTimerCallback; i < pValidateTimerCallback + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}
		
		if (nCodeLen == 5 && *(BYTE *)OpCode == 0xA1)
		{
			gtmrListHead =  *((PULONG)(i + 1));
			KdPrint(("gtmrListHead: 0x%08X\n", gtmrListHead));
			break;
		}
	}

	if (gtmrListHead > SYSTEM_ADDRESS_START)
	{
		KdPrint(("_gptmrFirst: 0x%08X\n", gtmrListHead));
		SetGlobalVeriable(enumgptmrFirst, gtmrListHead);
	}
}

VOID FindgtmrListHeadWin8()
{
	ULONG pNtUserValidateTimerCallback= GetGlobalVeriable(enumNtUserValidateTimerCallback);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG i = 0, nCodeLen = 0, n0xA1 = 0;
	PCHAR OpCode = NULL;
	ULONG gtmrListHead = 0;

	if (!pNtUserValidateTimerCallback || !MzfMmIsAddressValid || !MzfMmIsAddressValid((PVOID)pNtUserValidateTimerCallback))
	{
		return;
	}

	for (i = pNtUserValidateTimerCallback; i < pNtUserValidateTimerCallback + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(BYTE *)OpCode == 0xA1)
		{
			n0xA1++;
			if (n0xA1 == 2)
			{
				gtmrListHead =  *((PULONG)(i + 1));
				KdPrint(("gtmrListHead: 0x%08X\n", gtmrListHead));
				break;
			}
		}
	}

	if (gtmrListHead > SYSTEM_ADDRESS_START)
	{
		KdPrint(("_gptmrFirst: 0x%08X\n", gtmrListHead));
		SetGlobalVeriable(enumgptmrFirst, gtmrListHead);
	}
}

VOID FindgptmrFirstOrgtmrListHead()
{
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);

	if (WinVersion == enumWINDOWS_8)
	{
		FindgtmrListHeadWin8();
	}
	else
	{
		FindgptmrFirstOrgtmrListHeadWin2kToWin7();
	}
}

//
// 低版本系统枚举定时器
//
NTSTATUS GetProcessTimersWin2kToVista(PVOID pInput, ULONG gptmrFirst, ULONG nCnt) // gptmrFirst是一个单项链表
{
	PALL_TIMERS pTimerInfo = (PALL_TIMERS)pInput;
	PTIMER_INFO pTimers = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG pTimer = 0;
	pfnPsGetThreadId MzfPsGetThreadId = (pfnPsGetThreadId)GetGlobalVeriable(enumPsGetThreadId);
	ULONG pWin32ThreadOffset, pThreadOffset, pTimerCallbackOffset, uElapseOffset1, uElapseOffset2, nNextOffset;
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG nMax = PAGE_SIZE;
	KIRQL OldIrql;

	if (!MzfMmIsAddressValid || 
		!pInput || 
		!gptmrFirst || 
		!MzfMmIsAddressValid((PVOID)gptmrFirst))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	if (BuildNumber == 6000)
	{
		pWin32ThreadOffset = 0x14;
		pThreadOffset = 0x18;
		pTimerCallbackOffset = 0x2c;
		uElapseOffset1 = 0x24;
		uElapseOffset2 = 0x20;
		nNextOffset = 0xc;
	}
	else
	{
		pWin32ThreadOffset = 0x14;
		pThreadOffset = 0x10;
		pTimerCallbackOffset = 0x28;
		uElapseOffset1 = 0x20;
		uElapseOffset2 = 0x1c;
		nNextOffset = 0x8;
	}

	pTimers = pTimerInfo->timers;

	OldIrql = KeRaiseIrqlToDpcLevel();
	
	for (pTimer = *(PULONG)gptmrFirst; 
		pTimer && MzfMmIsAddressValid((PVOID)pTimer) && nMax--; 
		pTimer = *(PULONG)(pTimer + nNextOffset))
	{
		ULONG pWin32Thread = *(ULONG *)(pTimer + pWin32ThreadOffset);
		ULONG pThread = pWin32Thread == 0 ? *(ULONG *)(pTimer + pThreadOffset) : *(ULONG *)(pWin32Thread + 8);

		if (pThread && MzfMmIsAddressValid((PVOID)pThread))
		{
			pThread = *(PULONG)pThread;
			if (IsRealThread((PETHREAD)pThread))
			{
				ULONG nCurCnt = pTimerInfo->nCnt;

				if (nCnt > nCurCnt)
				{
					ULONG pTimerCallback = *(ULONG *)(pTimer + pTimerCallbackOffset);
					ULONG uElapse = *(ULONG *)(pTimer + uElapseOffset1);

					if (!uElapse)
					{
						uElapse = *(ULONG *)(pTimer + uElapseOffset2);
					}

					pTimers[nCurCnt].TimerObject = pTimer;
					pTimers[nCurCnt].pCallback = pTimerCallback;
					pTimers[nCurCnt].nInterval = uElapse;

					if (MzfPsGetThreadId)
					{
						pTimers[nCurCnt].tid = (ULONG)MzfPsGetThreadId((PETHREAD)pThread);
					}
					else
					{
						pTimers[nCurCnt].tid = *(PULONG)(GetGlobalVeriable(enumCidOffset_ETHREAD) + pThread + sizeof(ULONG));
					}

					pTimers[nCurCnt].pEprocess = *(PULONG)(pThread + GetGlobalVeriable(enumThreadsProcessOffset_ETHREAD));
					KdPrint(("process: 0x%08X\n", pTimers[nCurCnt].pEprocess));
				}

				pTimerInfo->nCnt++;
			}
		}
	}

	KeLowerIrql(OldIrql);

	return STATUS_SUCCESS;
}

//
// 新版本系统枚举timer
//
NTSTATUS GetProcessTimersWin2k8ToWin8(PVOID pInput, ULONG gtmrListHead, ULONG nCnt) // gtmrListHead是一个双项链表
{
	PALL_TIMERS pTimerInfo = (PALL_TIMERS)pInput;
	PTIMER_INFO pTimers = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsGetThreadId MzfPsGetThreadId = (pfnPsGetThreadId)GetGlobalVeriable(enumPsGetThreadId);
	ULONG pWin32ThreadOffset, pThreadOffset, pTimerCallbackOffset, uElapseOffset1, uElapseOffset2, pEntryOffset;
	ULONG pEntry = 0;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	ULONG nMax = PAGE_SIZE;
	KIRQL OldIrql;

	if (!MzfMmIsAddressValid || 
		!pInput || 
		!gtmrListHead	||
		!MzfMmIsAddressValid((PVOID)gtmrListHead))
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (WinVersion == enumWINDOWS_8)
	{
		pEntryOffset = 0x2c;

		pWin32ThreadOffset = 0;
		pThreadOffset = 0xc;
		pTimerCallbackOffset = 0x10;
		uElapseOffset1 = 0x14;
		uElapseOffset2 = 0x20;
	}
	else
	{
		pEntryOffset = 0xc;

		pWin32ThreadOffset = 0x14;
		pThreadOffset = 0x18;
		pTimerCallbackOffset = 0x2c;
		uElapseOffset1 = 0x24;
		uElapseOffset2 = 0x20;
	}
	
	pEntry = *(PULONG)(gtmrListHead);
	if (!pEntry || !MzfMmIsAddressValid((PVOID)pEntry) || pEntry == gtmrListHead)
	{
		return STATUS_UNSUCCESSFUL;
	}

	pTimers = pTimerInfo->timers;

	OldIrql = KeRaiseIrqlToDpcLevel();

	while (pEntry != gtmrListHead && nMax--)
	{
		ULONG pTimer = 0, pWin32Thread = 0, pThread = 0;
		
		pTimer = pEntry - pEntryOffset;
		KdPrint(("ptimer: 0x%08X\n", pTimer));

		pWin32Thread = *(ULONG *)(pTimer + pWin32ThreadOffset);
		if (WinVersion == enumWINDOWS_8)
		{
			pThread = *(ULONG *)(pTimer + pThreadOffset);
		}
		else
		{
			pThread = pWin32Thread == 0 ? *(ULONG *)(pTimer + pThreadOffset) : *(ULONG *)(pWin32Thread + 8);
		}
		
		if (pThread && MzfMmIsAddressValid((PVOID)pThread) && *(PULONG)pThread)
		{
			pThread = *(PULONG)pThread;
			if (IsRealThread((PETHREAD)pThread))
			{
				ULONG nCurCnt = pTimerInfo->nCnt;
				if (nCnt > nCurCnt)
				{
					ULONG pTimerCallback = *(ULONG *)(pTimer + pTimerCallbackOffset);
					ULONG uElapse = *(ULONG *)(pTimer + uElapseOffset1);

					if (!uElapse)
					{
						uElapse = *(ULONG *)(pTimer + uElapseOffset2);
					}

					KdPrint(("uElapse: %d\n", uElapse));
					KdPrint(("pTimerCallback: 0x%08X\n", pTimerCallback));
					
					pTimers[nCurCnt].TimerObject = pTimer;
					pTimers[nCurCnt].pCallback = pTimerCallback;
					pTimers[nCurCnt].nInterval = uElapse;

					if (MzfPsGetThreadId)
					{
						pTimers[nCurCnt].tid = (ULONG)MzfPsGetThreadId((PETHREAD)pThread);
					}
					else
					{
						pTimers[nCurCnt].tid = *(PULONG)(GetGlobalVeriable(enumCidOffset_ETHREAD) + pThread + sizeof(ULONG));
					}

					pTimers[nCurCnt].pEprocess = *(PULONG)(pThread + GetGlobalVeriable(enumThreadsProcessOffset_ETHREAD));
				}

				pTimerInfo->nCnt++;
			}
		}

		pEntry = *(PULONG)(pEntry);
	}

	KeLowerIrql(OldIrql);

	return STATUS_SUCCESS;
}

//
// 枚举进程定时器
//
NTSTATUS ListProcessTimers(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG gptmrFirst = GetGlobalVeriable(enumgptmrFirst);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG nCnt = (uOutSize - sizeof(ALL_TIMERS)) / sizeof(TIMER_INFO);

	if (!MzfMmIsAddressValid)
	{
		return status;
	}

	// 参数检查
	if (!pInBuffer || 
		uInSize != sizeof(OPERATE_TYPE) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_TIMERS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (!gptmrFirst)
	{
		FindgptmrFirstOrgtmrListHead();
		gptmrFirst = GetGlobalVeriable(enumgptmrFirst);
	}

	if (gptmrFirst > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)gptmrFirst))
	{
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
		
		if (BuildNumber <= 6000)
		{
			status = GetProcessTimersWin2kToVista(pOutBuffer, gptmrFirst, nCnt);
		}
		else
		{
			status = GetProcessTimersWin2k8ToWin8(pOutBuffer, gptmrFirst, nCnt);
		}
	}

	if (NT_SUCCESS(status))
	{
		if (nCnt >= ((PALL_TIMERS)pOutBuffer)->nCnt)
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

//
// 早期版本系统卸载定时器
//
NTSTATUS RemoveTimerWin2kToVista(ULONG pRemovedTimer, ULONG gptmrFirst) // gptmrFirst是一个单项链表
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	BOOL bOk = TRUE;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || 
		!gptmrFirst	||
		!pRemovedTimer )
	{
		return status;
	}
	
	KdPrint(("RemoveTimerWin2kToVista-> pRemovedTimer: 0x%08X, gptmrFirst: 0x%08X\n", pRemovedTimer, gptmrFirst));

	if (gptmrFirst > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)gptmrFirst) &&
		pRemovedTimer > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pRemovedTimer))
	{
		ULONG pTimer = 0;
		ULONG nNextOffset = 0;
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
		ULONG nMax = PAGE_SIZE;
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
		
		if (BuildNumber == 6000)
		{
			nNextOffset = 0xc;
		}
		else
		{
			nNextOffset = 0x8;
		}

		for (pTimer = gptmrFirst; 
			*(PDWORD)pTimer > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(*(PDWORD)pTimer) ) && nMax--; 
			pTimer = *(PDWORD)pTimer + nNextOffset)
		{
			if (*(PDWORD)pTimer == pRemovedTimer)
			{
				*(PDWORD)pTimer = *(PDWORD)(*(PDWORD)pTimer + nNextOffset);
				status = STATUS_SUCCESS;
				break;
			}
		}

		KeLowerIrql(OldIrql);
	}

	return status;
}

//
// 新版系统卸载定时器
//
NTSTATUS RemoveTimerWin2k8ToWin8(ULONG pRemovedTimer, ULONG gtmrListHead)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || !gtmrListHead || !pRemovedTimer)
	{
		return status;
	}

	if (gtmrListHead > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)gtmrListHead) &&
		pRemovedTimer > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pRemovedTimer))
	{
		ULONG pEntryOffset = 0;
		ULONG pEntry = 0;
		WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
		KIRQL OldIrql;
		ULONG nMax = PAGE_SIZE;

		if (WinVersion == enumWINDOWS_8)
		{
			pEntryOffset = 0x2c;
		}
		else
		{
			pEntryOffset = 0xc;
		}

		pEntry = *(PULONG)(gtmrListHead);
		if (!pEntry || !MzfMmIsAddressValid((PVOID)pEntry) || pEntry == gtmrListHead)
		{
			return status;
		}

		OldIrql = KeRaiseIrqlToDpcLevel();

		while ( pEntry != gtmrListHead && nMax-- )
		{
			ULONG pTimer = pEntry - pEntryOffset;

			if (pTimer && MzfMmIsAddressValid((PVOID)pTimer) && pTimer == pRemovedTimer)
			{
				RemoveEntryList((PLIST_ENTRY)pEntry);
				status = STATUS_SUCCESS;
				break;
			}

			pEntry = *(PULONG)(pEntry);
		}

		KeLowerIrql(OldIrql);
	}

	return status;
}

//
// 移除定时器
//
NTSTATUS RemoveTimer(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG gptmrFirst = GetGlobalVeriable(enumgptmrFirst);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	PCOMMUNICATE_TIMER pCt = (PCOMMUNICATE_TIMER)pInBuffer;
	ULONG pTimerObject = 0;

	if (!MzfMmIsAddressValid)
	{
		return status;
	}

	if (!pInBuffer || 
		uInSize != sizeof(COMMUNICATE_TIMER))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pTimerObject = pCt->op.Remove.TimerObject;
	if (!pTimerObject || !MzfMmIsAddressValid((PVOID)pTimerObject))
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (!gptmrFirst)
	{
		FindgptmrFirstOrgtmrListHead();
		gptmrFirst = GetGlobalVeriable(enumgptmrFirst);
	}

	KdPrint(("RemoveTimerWin2kToVista"));
	if (gptmrFirst > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)gptmrFirst))
	{
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
		
		if (BuildNumber <= 6000)
		{
			status = RemoveTimerWin2kToVista(pTimerObject, gptmrFirst);
		}
		else
		{
			status = RemoveTimerWin2k8ToWin8(pTimerObject, gptmrFirst);
		}
	}

	return status;
}

NTSTATUS ModifyTimeOutValueWin2kToVista(ULONG pModifyTimer, ULONG TimeOutValue, ULONG gptmrFirst) // gptmrFirst是一个单项链表
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || !gptmrFirst || !pModifyTimer)
	{
		return status;
	}

	KdPrint(("ModifyTimeOutValueWin2kToVista-> pModifyTimer: 0x%08X, gptmrFirst: 0x%08X\n", pModifyTimer, gptmrFirst));

	if (gptmrFirst > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)gptmrFirst) &&
		pModifyTimer > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pModifyTimer))
	{
		ULONG pTimer = 0;
		ULONG uElapseOffset1, uElapseOffset2, nNextOffset;
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
		KIRQL OldIrql;
		ULONG nMax = PAGE_SIZE;

		if (BuildNumber == 6000)
		{
			uElapseOffset1 = 0x24;
			uElapseOffset2 = 0x20;
			nNextOffset = 0xc;
		}
		else
		{
			uElapseOffset1 = 0x20;
			uElapseOffset2 = 0x1c;
			nNextOffset = 0x8;
		}

		OldIrql = KeRaiseIrqlToDpcLevel();

		for (pTimer = *(PULONG)gptmrFirst; 
			pTimer && MzfMmIsAddressValid((PVOID)pTimer) && nMax--; 
			pTimer = *(PULONG)(pTimer + nNextOffset))
		{
			if (pTimer == pModifyTimer)
			{
				KdPrint(("find pModifyTimer\n"));
				if (MzfMmIsAddressValid((PVOID)(pTimer + uElapseOffset1)))
				{
					*(ULONG *)(pTimer + uElapseOffset1) = TimeOutValue;
				}

				if (MzfMmIsAddressValid((PVOID)(pTimer + uElapseOffset2)))
				{
					*(ULONG *)(pTimer + uElapseOffset2) = TimeOutValue;
				}

				status = STATUS_SUCCESS;
				break;
			}
		}

		KeLowerIrql(OldIrql);
	}

	return status;
}

NTSTATUS ModifyTimeOutValueWin2k8ToWin8(ULONG pModifyTimer, ULONG TimeOutValue, ULONG gtmrListHead) // gptmrFirst是一个单项链表
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || !gtmrListHead || !pModifyTimer)
	{
		return status;
	}

	if ( gtmrListHead > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)gtmrListHead) )
	{
		ULONG pWin32ThreadOffset, pThreadOffset, pTimerCallbackOffset, uElapseOffset1, uElapseOffset2, pEntryOffset;
		ULONG pEntry = 0;
		WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
		KIRQL OldIrql;
		ULONG nMax = PAGE_SIZE;

		if (WinVersion == enumWINDOWS_8)
		{
			pEntryOffset = 0x2c;
			uElapseOffset1 = 0x14;
			uElapseOffset2 = 0x20;
		}
		else
		{
			pEntryOffset = 0xc;
			uElapseOffset1 = 0x24;
			uElapseOffset2 = 0x20;
		}

		pEntry = *(PULONG)(gtmrListHead);
		if (!pEntry || !MzfMmIsAddressValid((PVOID)pEntry) || pEntry == gtmrListHead)
		{
			return status;
		}

		OldIrql = KeRaiseIrqlToDpcLevel();

		while (pEntry != gtmrListHead && nMax--)
		{
			ULONG pTimer = pEntry - pEntryOffset;

			if (pTimer == pModifyTimer)
			{
				if (MzfMmIsAddressValid((PVOID)(pTimer + uElapseOffset1)))
				{
					*(ULONG *)(pTimer + uElapseOffset1) = TimeOutValue;
				}

				if (MzfMmIsAddressValid((PVOID)(pTimer + uElapseOffset2)))
				{
					*(ULONG *)(pTimer + uElapseOffset2) = TimeOutValue;
				}

				status = STATUS_SUCCESS;
				break;
			}
			
			pEntry = *(PULONG)(pEntry);
		}

		KeLowerIrql(OldIrql);
	}

	return status;
}

//
// 修改定时器时间
//
NTSTATUS ModifyTimeOutValue(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG gptmrFirst = GetGlobalVeriable(enumgptmrFirst);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	PCOMMUNICATE_TIMER pCt = (PCOMMUNICATE_TIMER)pInBuffer;
	ULONG pTimerObject = 0, nTimeOut = 0;

	if (!MzfMmIsAddressValid)
	{
		return status;
	}

	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_TIMER))
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	pTimerObject = pCt->op.ModifyTimeOut.TimerObject;
	nTimeOut = pCt->op.ModifyTimeOut.TimeOut;

	if (!pTimerObject || !MzfMmIsAddressValid((PVOID)pTimerObject))
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (!gptmrFirst)
	{
		FindgptmrFirstOrgtmrListHead();
		gptmrFirst = GetGlobalVeriable(enumgptmrFirst);
	}

	KdPrint(("ModifyTimeOutValue"));
	if (gptmrFirst > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)gptmrFirst))
	{
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

		if (BuildNumber <= 6000)
		{
			status = ModifyTimeOutValueWin2kToVista(pTimerObject, nTimeOut,  gptmrFirst);
		}
		else
		{
			status = ModifyTimeOutValueWin2k8ToWin8(pTimerObject, nTimeOut,  gptmrFirst);
		}
	}

	return status;
}