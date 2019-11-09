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
#include "stdafx.h"
#include "ListTimer.h"

CListTimers::CListTimers()
{

}

CListTimers::~CListTimers()
{

}

BOOL CListTimers::EnumTimers(vector<TIMER_INFO>& vectorTimers)
{
	PALL_TIMERS pTimerInfo = NULL;
	OPERATE_TYPE ot = enumListProcessTimer;
	BOOL bRet = FALSE;
	ULONG nCnt = 1000;

	vectorTimers.clear();

	do 
	{
		ULONG nSize = sizeof(ALL_TIMERS) + nCnt * sizeof(TIMER_INFO);

		if (pTimerInfo)
		{
			free(pTimerInfo);
			pTimerInfo = NULL;
		}

		pTimerInfo = (PALL_TIMERS)GetMemory(nSize);
		if (!pTimerInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pTimerInfo, nSize, NULL);

		nCnt = pTimerInfo->nCnt + 100;

	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pTimerInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pTimerInfo->nCnt; i++)
		{
			vectorTimers.push_back(pTimerInfo->timers[i]);
		}
	}

	if (pTimerInfo)
	{
		free(pTimerInfo);
		pTimerInfo = NULL;
	}

	return bRet;
}

BOOL CListTimers::RemoveTimer(ULONG TimerObject)
{
	if (!TimerObject)
	{
		return FALSE;
	}

	COMMUNICATE_TIMER ct;
	ct.op.Remove.TimerObject = TimerObject;
	ct.OpType = enumRemoveTimer;
	return g_ConnectDriver.CommunicateDriver(&ct, sizeof(COMMUNICATE_TIMER), NULL, 0, NULL);
}

BOOL CListTimers::ModifyTimeOut(ULONG TimerObject, ULONG TimeOut)
{
	if (!TimerObject)
	{
		return FALSE;
	}

	COMMUNICATE_TIMER ct;
	ct.op.ModifyTimeOut.TimerObject = TimerObject;
	ct.op.ModifyTimeOut.TimeOut = TimeOut;
	ct.OpType = enumModifyTimeOut;
	return g_ConnectDriver.CommunicateDriver(&ct, sizeof(COMMUNICATE_TIMER), NULL, 0, NULL);
}