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
#include "ListThread.h"

CListThreads::CListThreads()
{

}

CListThreads::~CListThreads()
{

}

//
// 枚举线程
//
BOOL CListThreads::ListThreads(ULONG nPid, ULONG pEprocess, vector<THREAD_INFO>& vectorThreads)
{
	BOOL bRet = FALSE;
	
	if (nPid == 0 && pEprocess == 0)
	{
		return bRet;
	}
	
	vectorThreads.clear();

	COMMUNICATE_THREAD cpt;
	ULONG nCnt = 1000;
	PALL_THREADS pModuleThread = NULL;

	cpt.OpType = enumListProcessThread;
	cpt.nPid = nPid;
	cpt.pEprocess = pEprocess;

	do 
	{
		ULONG nSize = 0;

		if (pModuleThread)
		{
			free(pModuleThread);
			pModuleThread = NULL;
		}

		nSize = sizeof(ALL_THREADS) + nCnt * sizeof(THREAD_INFO);

		pModuleThread = (PALL_THREADS)GetMemory(nSize);
		if (!pModuleThread)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&cpt, sizeof(COMMUNICATE_THREAD), pModuleThread, nSize, NULL);
		nCnt = pModuleThread->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pModuleThread->nCnt > 0)
	{
		for (ULONG i = 0; i < pModuleThread->nCnt; i++)
		{
			vectorThreads.push_back(pModuleThread->Threads[i]);
		}
	}

	if (pModuleThread)
	{
		free(pModuleThread);
		pModuleThread = NULL;
	}

	return bRet;
}

//
// 杀线程
//
BOOL CListThreads::KillThread(ULONG pThread, ULONG nPid, ULONG pEprocess)
{
	if (!pThread)
	{
		return FALSE;
	}

	COMMUNICATE_THREAD ckt;
	ckt.op.Kill.pThread = pThread;
	ckt.OpType = enumKillThread;
	ckt.nPid = nPid;
	ckt.pEprocess = pEprocess;

	return g_ConnectDriver.CommunicateDriver(&ckt, sizeof(COMMUNICATE_THREAD), NULL, 0, NULL);
}

//
// 挂起线程
//
BOOL CListThreads::SuspendThread(ULONG pThread)
{
	if (!pThread)
	{
		return FALSE;
	}

	COMMUNICATE_THREAD cst;
	cst.op.Suspend.pThread = pThread;
	cst.op.Suspend.bSuspend = TRUE;
	cst.OpType = enumResumeOrSuspendThread;
	return g_ConnectDriver.CommunicateDriver(&cst, sizeof(COMMUNICATE_THREAD), NULL, 0, NULL);
}

//
// 恢复线程
//
BOOL CListThreads::ResumeThread(ULONG pThread)
{
	if (!pThread)
	{
		return FALSE;
	}

	COMMUNICATE_THREAD cst;
	cst.op.Suspend.pThread = pThread;
	cst.op.Suspend.bSuspend = FALSE;
	cst.OpType = enumResumeOrSuspendThread;
	return g_ConnectDriver.CommunicateDriver(&cst, sizeof(COMMUNICATE_THREAD), NULL, 0, NULL);
}

//
// 获取进程被挂起计数
//
ULONG CListThreads::GetSuspenCount(ULONG pThread)
{
	ULONG nRet = 0;

	if (!pThread)
	{
		return nRet;
	}

	COMMUNICATE_THREAD cgt;
	cgt.OpType = enumGetThreadSuspendCnt;
	cgt.op.GetSuspendCnt.pThread = pThread;
	if (!g_ConnectDriver.CommunicateDriver(&cgt, sizeof(COMMUNICATE_THREAD), &nRet, sizeof(ULONG), NULL))
	{
		nRet = 0;
	}

	return nRet;
}