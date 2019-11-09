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
#ifndef _LIST_THREADS_H_
#define _LIST_THREADS_H_

#include "../../Common/Common.h"
#include <vector>
#include <algorithm>
#include "ProcessFunc.h"
#include "Function.h"

using namespace std;

class CListThreads
{
public:
	CListThreads();
	~CListThreads();
	BOOL ListThreads(ULONG nPid, ULONG pEprocess, vector<THREAD_INFO>& vectorThreads);
	BOOL KillThread(ULONG pThread, ULONG nPid = 0, ULONG pEprocess = 0);
	BOOL ResumeThread(ULONG pThread);
	BOOL SuspendThread(ULONG pThread);
	ULONG GetSuspenCount(ULONG pThread);
private:
	CProcessFunc m_ProcessFunc;
};

#endif