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
#ifndef _CLIST_PROCESS_H_
#define _CLIST_PROCESS_H_

#include "../../Common/Common.h"
#include <vector>
#include <algorithm>
#include "ProcessFunc.h"

using namespace std;

class CListProcess
{
public:
	CListProcess();
	~CListProcess();
	BOOL EnumProcess(vector<PROCESS_INFO> &vectorProcess);
	void SortByProcessCreateTime(vector<PROCESS_INFO> &ProcListEx);
	BOOL KillProcess(ULONG nPid, ULONG pEprocess);
	BOOL SuspendProcess(ULONG nPid, ULONG pEprocess);
	BOOL ResumeProcess(ULONG nPid, ULONG pEprocess);
	BOOL InjectDllByRemoteThread(const TCHAR* ptszDllFile, DWORD dwProcessId);
	CString GetProcessCmdLine(DWORD dwPid);
	CString GetProcessCurrentDirectory(DWORD dwPid);
	CString GetProcessStartTime(DWORD dwPid);
	CString GetPebAddress(DWORD dwPid);
private:
	CProcessFunc m_ProcessFunc;
};

#endif