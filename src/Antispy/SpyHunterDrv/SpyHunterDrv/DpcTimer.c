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
#include "DpcTimer.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "ldasm.h"

ULONG GetKiTimerTableListHeadXp()
{
	ULONG KiTimerTableListHead = 0;
	ULONG pKeUpdateSystemTime = GetGlobalVeriable(enumKeUpdateSystemTime);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("pKeUpdateSystemTime: 0X%08x\n", pKeUpdateSystemTime));

	if (pKeUpdateSystemTime && MzfMmIsAddressValid)
	{
		ULONG nStart = pKeUpdateSystemTime;
		ULONG nEnd = pKeUpdateSystemTime + PAGE_SIZE;
		ULONG nCodeLen = 0;
		BYTE btSign = 0;
		ULONG KiTimerTableListHeadCodeLen = 0;
		ULONG Offset = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		switch (Version)
		{
		case enumWINDOWS_2K:
		case enumWINDOWS_XP:
		case enumWINDOWS_2K3:
			btSign = 0x8d;
			KiTimerTableListHeadCodeLen = 7;
			Offset = 3;
			break;
		}

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

			if (nCodeLen == KiTimerTableListHeadCodeLen && *(PBYTE)nStart == btSign)
			{
				KiTimerTableListHead = *(PULONG)(nStart + Offset);
				if (KiTimerTableListHead && MzfMmIsAddressValid((PVOID)KiTimerTableListHead))
				{
					SetGlobalVeriable(enumKiTimerTableListHead, KiTimerTableListHead);
					break;
				}
			}
		}
	}

	return KiTimerTableListHead;
}

ULONG GetKiTimerTableListHeadWin2k3()
{
	ULONG KiTimerTableListHead = 0;
	ULONG pKeCancelTimer = GetGlobalVeriable(enumKeCancelTimer);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("pKeCancelTimer: 0X%08x\n", pKeCancelTimer));

	if (pKeCancelTimer && MzfMmIsAddressValid)
	{
		ULONG nStart = pKeCancelTimer;
		ULONG nEnd = pKeCancelTimer + PAGE_SIZE;
		ULONG nCodeLen = 0;
		BYTE btSign = 0;
		ULONG KiTimerTableListHeadCodeLen = 0;
		ULONG Offset = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		switch (Version)
		{
		case enumWINDOWS_2K3_SP1_SP2:
		case enumWINDOWS_VISTA:
		case enumWINDOWS_VISTA_SP1_SP2:
			btSign = 0x81;
			KiTimerTableListHeadCodeLen = 6;
			Offset = 2;
			break;
		}

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

			if (nCodeLen == KiTimerTableListHeadCodeLen && *(PBYTE)nStart == btSign)
			{
				KiTimerTableListHead = *(PULONG)(nStart + Offset);
				if (KiTimerTableListHead && MzfMmIsAddressValid((PVOID)KiTimerTableListHead))
				{
					SetGlobalVeriable(enumKiTimerTableListHead, KiTimerTableListHead);
					break;
				}
			}
		}
	}

	return KiTimerTableListHead;
}

ULONG GetKiTimerTableListHeadWin7()
{
	ULONG KiTimerTableListHead = 0;
	ULONG pKeCancelTimer = GetGlobalVeriable(enumKeCancelTimer);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("pKeCancelTimer: 0X%08x\n", pKeCancelTimer));

	if (pKeCancelTimer && MzfMmIsAddressValid)
	{
		ULONG nStart = pKeCancelTimer;
		ULONG nEnd = pKeCancelTimer + PAGE_SIZE;
		ULONG nCodeLen = 0;
		BYTE btSign = 0;
		ULONG KiTimerTableListHeadCodeLen = 0;
		ULONG Offset = 0;
		ULONG KiCancelTimer = 0;
	
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

			if (nCodeLen == 5 && *(PBYTE)nStart == 0xe8)
			{
				KiCancelTimer = nStart + *(PULONG)(nStart + 1) + 5;
				if (KiCancelTimer && MzfMmIsAddressValid((PVOID)KiCancelTimer))
				{
					KdPrint(("KiCancelTimer: 0x%08X\n", KiCancelTimer));
				//	SetGlobalVeriable(enumKiTimerTableListHead, KiTimerTableListHead);
					break;
				}
			}
		}

		if (KiCancelTimer && MzfMmIsAddressValid((PVOID)KiCancelTimer))
		{
			nStart = KiCancelTimer;
			nEnd = KiCancelTimer + PAGE_SIZE;

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

				if (nCodeLen == 7 && *(PBYTE)nStart == 0x8B)
				{
					KiTimerTableListHead = *(PULONG)(nStart + 3);

					if (KiTimerTableListHead && MzfMmIsAddressValid((PVOID)KiTimerTableListHead))
					{
						ULONG Offset = 0;
						WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

						if (Version == enumWINDOWS_7)
						{
							Offset = 0x19A0;
						}
						else if (Version == enumWINDOWS_8)
						{
							Offset = 0x2260 + 0x40;
						}

						KiTimerTableListHead = *(PULONG)KiTimerTableListHead;
						
						if (KiTimerTableListHead && MzfMmIsAddressValid((PVOID)(KiTimerTableListHead + Offset)))
						{
							KdPrint(("KiTimerTableListHead: 0x%08X\n", KiTimerTableListHead + Offset));
							SetGlobalVeriable(enumKiTimerTableListHead, KiTimerTableListHead + Offset);
						}

						break;
					}
				}
			}
		}
	}

	return KiTimerTableListHead;
}

NTSTATUS WalkerKiTimerTableListHeadXp(ULONG KiTimerTableListHead, PDPC_TIMER_INFO pDpcTimerInfo)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG TIMER_TABLE_SIZE = GetGlobalVeriable(enumTIMER_TABLE_SIZE);
	PLIST_ENTRY pKiTimerTableListHead = (PLIST_ENTRY)KiTimerTableListHead;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	KdPrint(("TIMER_TABLE_SIZE: %d\n", TIMER_TABLE_SIZE));

	if (KiTimerTableListHead &&
		pDpcTimerInfo &&
		TIMER_TABLE_SIZE && 
		MzfMmIsAddressValid &&
		MzfMmIsAddressValid(pKiTimerTableListHead))
	{
		ULONG i = 0, n = 0;
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

		for (i = 0; i < TIMER_TABLE_SIZE; i++)
		{
			PLIST_ENTRY pNextEntry = (&pKiTimerTableListHead[i])->Flink;

			while (MzfMmIsAddressValid(pNextEntry) && &pKiTimerTableListHead[i] != pNextEntry)
			{
				PKTIMER pTimer = CONTAINING_RECORD(pNextEntry, KTIMER, TimerListEntry);

				if (pTimer && 
					MzfMmIsAddressValid(pTimer) &&
					MzfMmIsAddressValid(pTimer->Dpc) &&
					MzfMmIsAddressValid(pTimer->Dpc->DeferredRoutine))
				{
					if (pDpcTimerInfo->nCnt > pDpcTimerInfo->nRetCnt)
					{
						PKDPC pDpc = pTimer->Dpc;
						PVOID TimerDispatch = pDpc->DeferredRoutine;

						KdPrint(("%d Timer: 0x%08X, Period: %d, pDpc: 0x%08X, TimerDispatch: 0x%08X\n", 
							n, pTimer, pTimer->Period, pDpc, TimerDispatch));

						pDpcTimerInfo->DpcTimer[n].Dpc = (ULONG)pDpc;
						pDpcTimerInfo->DpcTimer[n].Period = pTimer->Period;
						pDpcTimerInfo->DpcTimer[n].TimeDispatch = (ULONG)TimerDispatch;
						pDpcTimerInfo->DpcTimer[n].TimerObject = (ULONG)pTimer;

						n++;
					}

					pDpcTimerInfo->nRetCnt++;
				}

				pNextEntry = pNextEntry->Flink;
			}
		}

		KeLowerIrql(OldIrql);
	}

	return status;
}

NTSTATUS WalkerKiTimerTableListHeadWin2k3(ULONG KiTimerTableListHead, PDPC_TIMER_INFO pDpcTimerInfo)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG TIMER_TABLE_SIZE = GetGlobalVeriable(enumTIMER_TABLE_SIZE);
	PLIST_ENTRY pKiTimerTableListHead = (PLIST_ENTRY)KiTimerTableListHead;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("TIMER_TABLE_SIZE: %d\n", TIMER_TABLE_SIZE));

	if (KiTimerTableListHead &&
		pDpcTimerInfo &&
		TIMER_TABLE_SIZE && 
		MzfMmIsAddressValid &&
		MzfMmIsAddressValid(pKiTimerTableListHead))
	{
		ULONG i = 0, n = 0;
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

		for (i = 0; i < TIMER_TABLE_SIZE; i++)
		{
			PLIST_ENTRY pEntey= (PLIST_ENTRY)((ULONG)pKiTimerTableListHead + 0x10 * i);
			PLIST_ENTRY pNextEntry= pEntey->Flink;

			while (MzfMmIsAddressValid(pNextEntry) && pEntey != pNextEntry)
			{
				PKTIMER pTimer = CONTAINING_RECORD(pNextEntry, KTIMER, TimerListEntry);

				if (pTimer && 
					MzfMmIsAddressValid(pTimer) &&
					MzfMmIsAddressValid((PVOID)((ULONG)pTimer + 0x20)))
				{
					PKDPC pDpc = pTimer->Dpc;
					ULONG TimerDispatch = 0;

					if (MzfMmIsAddressValid((PVOID)((ULONG)pDpc + 0xc)))
					{
						TimerDispatch = (ULONG)pDpc->DeferredRoutine;

						if (pDpcTimerInfo->nCnt > pDpcTimerInfo->nRetCnt)
						{
							KdPrint(("%d Timer: 0x%08X, pDpc:  0x%08X, TimerDispatch: 0x%08X\n", 
								n, pTimer, pDpc, TimerDispatch /*pTimer->Period, pDpc, TimerDispatch*/));

							pDpcTimerInfo->DpcTimer[n].Dpc = (ULONG)pDpc;
							pDpcTimerInfo->DpcTimer[n].Period = pTimer->Period;
							pDpcTimerInfo->DpcTimer[n].TimeDispatch = (ULONG)TimerDispatch;
							pDpcTimerInfo->DpcTimer[n].TimerObject = (ULONG)pTimer;

							n++;
						}

						pDpcTimerInfo->nRetCnt++;
					}
				}

				pNextEntry = pNextEntry->Flink;
			}
		}

		KeLowerIrql(OldIrql);
	}

	return status;
}

NTSTATUS WalkerKiTimerTableListHeadWin7(ULONG KiTimerTableListHead, PDPC_TIMER_INFO pDpcTimerInfo)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG TIMER_TABLE_SIZE = GetGlobalVeriable(enumTIMER_TABLE_SIZE);
	PLIST_ENTRY pKiTimerTableListHead = (PLIST_ENTRY)KiTimerTableListHead;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("TIMER_TABLE_SIZE: %d\n", TIMER_TABLE_SIZE));

	if (KiTimerTableListHead &&
		pDpcTimerInfo &&
		TIMER_TABLE_SIZE && 
		MzfMmIsAddressValid &&
		MzfMmIsAddressValid(pKiTimerTableListHead))
	{
		ULONG i = 0, n = 0;
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

		for (i = 0; i < TIMER_TABLE_SIZE; i++)
		{
			PLIST_ENTRY pEntey = (PLIST_ENTRY)((ULONG)pKiTimerTableListHead + 0x18 * i + 4);
			PLIST_ENTRY pNextEntry= pEntey->Flink;

			while (MzfMmIsAddressValid(pNextEntry) && pEntey != pNextEntry)
			{
				PKTIMER pTimer = CONTAINING_RECORD(pNextEntry, KTIMER, TimerListEntry);

				if (pTimer && 
					MzfMmIsAddressValid(pTimer) &&
					MzfMmIsAddressValid((PVOID)((ULONG)pTimer + 0x20)))
				{
					PKDPC pDpc = pTimer->Dpc;
					ULONG TimerDispatch = 0;
					
					if (MzfMmIsAddressValid((PVOID)((ULONG)pDpc + 0xc)))
					{
						TimerDispatch = (ULONG)pDpc->DeferredRoutine;
						if (pDpcTimerInfo->nCnt > pDpcTimerInfo->nRetCnt)
						{
							KdPrint(("%d Timer: 0x%08X, pDpc:  0x%08X, TimerDispatch: 0x%08X\n", 
								n, pTimer, pDpc, TimerDispatch /*pTimer->Period, pDpc, TimerDispatch*/));

							pDpcTimerInfo->DpcTimer[n].Dpc = (ULONG)pDpc;
							pDpcTimerInfo->DpcTimer[n].Period = pTimer->Period;
							pDpcTimerInfo->DpcTimer[n].TimeDispatch = (ULONG)TimerDispatch;
							pDpcTimerInfo->DpcTimer[n].TimerObject = (ULONG)pTimer;

							n++;
						}

						pDpcTimerInfo->nRetCnt++;
					}
				}

				pNextEntry = pNextEntry->Flink;
			}
		}

 		KeLowerIrql(OldIrql);
	}

	return status;
}

NTSTATUS EnumDpcTimer(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG KiTimerTableListHead = GetGlobalVeriable(enumKiTimerTableListHead);
	PDPC_TIMER_INFO pDpcTimerInfo = (PDPC_TIMER_INFO)pOutBuffer;
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

	if (!KiTimerTableListHead)
	{
		switch (Version)
		{
			case enumWINDOWS_2K:
			case enumWINDOWS_XP:
			case enumWINDOWS_2K3:
				GetKiTimerTableListHeadXp();
				break;

			case enumWINDOWS_2K3_SP1_SP2:
			case enumWINDOWS_VISTA:
			case enumWINDOWS_VISTA_SP1_SP2:
				GetKiTimerTableListHeadWin2k3();
				break;

			case enumWINDOWS_7:
			case enumWINDOWS_8:
				GetKiTimerTableListHeadWin7();
				break;
		}

		KiTimerTableListHead = GetGlobalVeriable(enumKiTimerTableListHead);
	}

	KdPrint(("KiTimerTableListHead: 0x%08X\n", KiTimerTableListHead));

	if (KiTimerTableListHead)
	{
		switch (Version)
		{
		case enumWINDOWS_2K:
		case enumWINDOWS_XP:
		case enumWINDOWS_2K3:
			status = WalkerKiTimerTableListHeadXp(KiTimerTableListHead, pDpcTimerInfo);
			break;

		case enumWINDOWS_2K3_SP1_SP2:
		case enumWINDOWS_VISTA:
		case enumWINDOWS_VISTA_SP1_SP2:
			status = WalkerKiTimerTableListHeadWin2k3(KiTimerTableListHead, pDpcTimerInfo);
			break;

		case enumWINDOWS_7:
		case enumWINDOWS_8:
			status = WalkerKiTimerTableListHeadWin7(KiTimerTableListHead, pDpcTimerInfo);
			break;
		}
	}
	
	if (pDpcTimerInfo->nCnt >= pDpcTimerInfo->nRetCnt)
	{
		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS RemoveDpcTimer(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG KiTimerTableListHead = GetGlobalVeriable(enumKiTimerTableListHead);
	PCOMMUNICATE_DPCTIMER pRemoveDpcTimer = (PCOMMUNICATE_DPCTIMER)pInBuffer;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnKeCancelTimer MzfKeCancelTimer = (pfnKeCancelTimer)GetGlobalVeriable(enumKeCancelTimer);

	if (pRemoveDpcTimer->pTimer &&
		MzfMmIsAddressValid &&
		MzfKeCancelTimer &&
		MzfMmIsAddressValid((PVOID)pRemoveDpcTimer->pTimer))
	{
		KdPrint(("RemoveDpcTimer: 0x%08X\n", pRemoveDpcTimer->pTimer));

		if (MzfKeCancelTimer((PKTIMER)pRemoveDpcTimer->pTimer))
		{
			status = STATUS_SUCCESS;
		}
	}

	return status;
}