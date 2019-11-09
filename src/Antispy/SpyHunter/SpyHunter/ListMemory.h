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
#ifndef _LIST_MEMORY_H_
#define _LIST_MEMORY_H_

#include "../../Common/Common.h"
#include <vector>

using namespace std;

class CListMemory
{
public:
	CListMemory();
	~CListMemory();
	BOOL GetMemorys(ULONG nPid, ULONG pEprocess, vector<MEMORY_INFO> &vectorMemorys);
	BOOL ZeroOfMemory(ULONG nPid, ULONG pEprocess, ULONG nBase, ULONG nSize);
	BOOL FreeMemory(ULONG nPid, ULONG pEprocess, ULONG nBase, ULONG nSize);
};

#endif