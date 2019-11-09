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
#include "ListHotkey.h"

CListHotKeys::CListHotKeys()
{

}

CListHotKeys::~CListHotKeys()
{

}

BOOL CListHotKeys::EnumHotKeys(vector<HOTKEY_ITEM> &vectorHotKeys)
{
	ULONG nCnt = 1000;
	PALL_HOTKEYS pHotKeyInfo = NULL;
	OPERATE_TYPE ot = enumListHotKey;
	BOOL bRet = FALSE;

	vectorHotKeys.clear();

	do 
	{
		ULONG nSize = sizeof(ALL_HOTKEYS) + nCnt * sizeof(HOTKEY_ITEM);

		if (pHotKeyInfo)
		{
			free(pHotKeyInfo);
			pHotKeyInfo = NULL;
		}

		pHotKeyInfo = (PALL_HOTKEYS)GetMemory(nSize);
		if (!pHotKeyInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pHotKeyInfo, nSize, NULL);
		
		nCnt = pHotKeyInfo->nCnt + 100;

	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pHotKeyInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pHotKeyInfo->nCnt; i++)
		{
			vectorHotKeys.push_back(pHotKeyInfo->HotKeys[i]);
		}
	}

	if (pHotKeyInfo)
	{
		free(pHotKeyInfo);
		pHotKeyInfo = NULL;
	}

	return bRet;
}

BOOL CListHotKeys::RemoveHotKey(ULONG HotKeyObject)
{
	if (HotKeyObject == 0)
	{
		return FALSE;
	}

	COMMUNICATE_HOTKEY ch;
	ch.op.Remove.HotKeyObject = HotKeyObject;
	ch.OpType = enumRemoveHotKey;
	return g_ConnectDriver.CommunicateDriver(&ch, sizeof(COMMUNICATE_HOTKEY), NULL, 0, NULL);
}