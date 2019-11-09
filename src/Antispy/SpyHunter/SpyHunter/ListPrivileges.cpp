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
#include "ListPrivileges.h"

CListPrivileges::CListPrivileges()
{

}

CListPrivileges::~CListPrivileges()
{

}

BOOL CListPrivileges::EnumPrivileges(ULONG nPid, 
									 ULONG pEprocess, 
									 vector<LUID_AND_ATTRIBUTES>& vectorPrivileges
									 )
{
	if (nPid == 0 && pEprocess == 0)
	{
		return FALSE;
	}

	vectorPrivileges.clear();

	COMMUNICATE_PRIVILEGES cpp;
	PTOKEN_PRIVILEGES pTp = NULL;
	BOOL bRet = FALSE;
	ULONG nSize = 0, nRetSize = sizeof(TOKEN_PRIVILEGES) + 100 * sizeof(LUID_AND_ATTRIBUTES);
	
	cpp.OpType = enumListProcessPrivileges;
	cpp.op.GetPrivilege.nPid = nPid;
	cpp.op.GetPrivilege.pEprocess = pEprocess;

	do 
	{
		nSize = nRetSize;

		if (pTp)
		{
			free(pTp);
			pTp = NULL;
		}

		pTp = (PTOKEN_PRIVILEGES)GetMemory(nSize);;
		if (!pTp)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&cpp, sizeof(COMMUNICATE_PRIVILEGES), pTp, nSize, NULL);

		nRetSize = *(PULONG)pTp + 0x1000;

	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pTp->PrivilegeCount > 0)
	{
		for (ULONG i = 0; i < pTp->PrivilegeCount; i++)
		{
			vectorPrivileges.push_back(pTp->Privileges[i]);
		}
	}

	if (pTp)
	{
		free(pTp);
		pTp = NULL;
	}

	return bRet;
}

BOOL CListPrivileges::EnablePrivilege(ULONG nPid, ULONG pEprocess, TOKEN_PRIVILEGES *pTp)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return FALSE;
	}

	if (pTp == NULL)
	{
		return FALSE;
	}

	COMMUNICATE_PRIVILEGES cpp;
	cpp.OpType = enumAdjustTokenPrivileges;
	cpp.op.AdjustPrivilege.nPid = nPid;
	cpp.op.AdjustPrivilege.pEprocess = pEprocess;
	cpp.op.AdjustPrivilege.pPrivilegeState = pTp;

	return g_ConnectDriver.CommunicateDriver(&cpp, sizeof(COMMUNICATE_PRIVILEGES), NULL, 0, NULL);
}