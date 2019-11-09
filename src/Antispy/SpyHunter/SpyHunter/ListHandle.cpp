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
#include "ListHandle.h"
#include "Function.h"

CListHandles::CListHandles()
{
	
}

CListHandles::~CListHandles()
{
	
}

//
// Ã¶¾Ù½ø³Ì¾ä±ú
//
BOOL CListHandles::GetHandles(ULONG nPid, ULONG pEprocess, vector<HANDLE_INFO> &vectorHandles)
{
	BOOL bLast = FALSE;

	vectorHandles.clear();

	if (nPid == 0 && pEprocess == 0)
	{
		return bLast;
	}

	COMMUNICATE_HANDLE ch;
	ULONG nCnt = 0x1000;
	PALL_HANDLES pHandles= NULL;
	BOOL bRet = FALSE;

	ch.OpType = enumListProcessHandle;
	ch.nPid = nPid;
	ch.pEprocess = pEprocess;

	do 
	{
		ULONG nSize = 0;

		if (pHandles)
		{
			free(pHandles);
			pHandles = NULL;
		}

		nSize = sizeof(ALL_HANDLES) + nCnt * sizeof(HANDLE_INFO);

		pHandles = (PALL_HANDLES)GetMemory(nSize);
		if (!pHandles)
		{
			break;
		}
		
		bRet = g_ConnectDriver.CommunicateDriver(&ch, sizeof(COMMUNICATE_HANDLE), pHandles, nSize, NULL);

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pHandles)
	{
		for (ULONG i = 0; i < pHandles->nCnt; i++)
		{
			vectorHandles.push_back(pHandles->Handles[i]);
		}

		bLast = TRUE;
	}

	if (pHandles)
	{
		free(pHandles);
		pHandles = NULL;
	}

	return bLast;
}

BOOL CListHandles::CloseHandle(ULONG nPid, ULONG pEprocess, BOOL bForce, ULONG hHandle, ULONG pObject)
{
	COMMUNICATE_HANDLE ch;
	ch.OpType = enumCloseHandle;
	ch.nPid = nPid;
	ch.pEprocess = pEprocess;
	ch.op.Close.bForceKill = bForce;
	ch.op.Close.hHandle = hHandle;
	ch.op.Close.pHandleObject = pObject;
	return g_ConnectDriver.CommunicateDriver(&ch, sizeof(COMMUNICATE_HANDLE), NULL, 0, NULL);
}