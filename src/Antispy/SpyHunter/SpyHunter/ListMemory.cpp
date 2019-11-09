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
#include "ListMemory.h"

CListMemory::CListMemory()
{

}

CListMemory::~CListMemory()
{

}

//
// 枚举进程内存
//
BOOL CListMemory::GetMemorys(ULONG nPid, ULONG pEprocess, vector<MEMORY_INFO> &vectorMemorys)
{
	BOOL bRet = FALSE;

	if (nPid == 0 && pEprocess == 0)
	{
		return bRet;
	}

	vectorMemorys.clear();

	COMMUNICATE_MEMORY cm;
	PALL_MEMORYS pMemorys = NULL;
	ULONG nCnt = 0x1000;

	cm.OpType = enumListProcessMemory;
	cm.op.GetMemory.nPid = nPid;
	cm.op.GetMemory.pEprocess = pEprocess;

	do 
	{
		ULONG nSize = 0;

		if (pMemorys)
		{
			free(pMemorys);
			pMemorys = NULL;
		}

		nSize = sizeof(ALL_MEMORYS) + nCnt * sizeof(MEMORY_INFO);

		pMemorys = (PALL_MEMORYS)malloc(nSize);
		if (!pMemorys)
		{
			break;
		}

		memset(pMemorys, 0, nSize);

		bRet = g_ConnectDriver.CommunicateDriver(&cm, sizeof(COMMUNICATE_MEMORY), pMemorys, nSize, NULL);
		nCnt = pMemorys->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pMemorys->nCnt > 0)
	{
		for (ULONG i = 0; i < pMemorys->nCnt; i++)
		{
			vectorMemorys.push_back(pMemorys->Memorys[i]);
		}
	}

	if (pMemorys)
	{
		free(pMemorys);
		pMemorys = NULL;
	}

	return bRet;
}

//
// 内存块清零
//
BOOL CListMemory::ZeroOfMemory(ULONG nPid, ULONG pEprocess, ULONG nBase, ULONG nSize)
{
	BOOL bRet = FALSE;

	if (nPid == 0 && pEprocess == 0)
	{
		return bRet;
	}

	if (nSize == 0)
	{
		return bRet;
	}

	COMMUNICATE_MEMORY cm;
	cm.OpType = enumZeroMemory;
	cm.op.ZeroMemory.Base = nBase;
	cm.op.ZeroMemory.Size = nSize;
	cm.op.ZeroMemory.nPid = nPid;
	cm.op.ZeroMemory.pEprocess = pEprocess;
	bRet = g_ConnectDriver.CommunicateDriver(&cm, sizeof(COMMUNICATE_MEMORY), NULL, 0, NULL);

	return bRet;
}

//
// 释放内存
//
BOOL CListMemory::FreeMemory(ULONG nPid, ULONG pEprocess, ULONG nBase, ULONG nSize)
{
	BOOL bRet = FALSE;

	if (nPid == 0 && pEprocess == 0)
	{
		return bRet;
	}

	if (nSize == 0)
	{
		return bRet;
	}

	COMMUNICATE_MEMORY cm;
	cm.OpType = enumFreeMemory;
	cm.op.FreeMemory.Base = nBase;
	cm.op.FreeMemory.Size = nSize;
	cm.op.FreeMemory.nPid = nPid;
	cm.op.FreeMemory.pEprocess = pEprocess;
	bRet = g_ConnectDriver.CommunicateDriver(&cm, sizeof(COMMUNICATE_MEMORY), NULL, 0, NULL);

	return bRet;
}