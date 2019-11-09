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
#include "ListWnd.h"

CListWnds::CListWnds()
{

}

CListWnds::~CListWnds()
{

}

BOOL CListWnds::EnumWnds(vector<WND_INFO> &vectorWnd)
{
	BOOL bRet = FALSE;

	vectorWnd.clear();
	
	OPERATE_TYPE ot = enumListProcessWnd;
	ULONG nCnt = 1000;
	PALL_WNDS pWndInfo = NULL;

	do 
	{
		ULONG nSize = sizeof(ALL_WNDS) + nCnt * sizeof(WND_INFO);

		if (pWndInfo)
		{
			free(pWndInfo);
			pWndInfo = NULL;
		}

		pWndInfo = (PALL_WNDS)GetMemory(nSize);
		if (!pWndInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pWndInfo, nSize, NULL);

		nCnt = pWndInfo->nCnt + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pWndInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pWndInfo->nCnt; i++)
		{
			vectorWnd.push_back(pWndInfo->WndInfo[i]);
		}
	}

	if (pWndInfo)
	{
		free(pWndInfo);
		pWndInfo = NULL;
	}
	
	return bRet;
}