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
#ifndef _LIST_DRIVERS_H_
#define _LIST_DRIVERS_H_

#include "../../Common/Common.h"
#include <vector>
#include <algorithm>
#include "Function.h"

using namespace std;

class CListDrivers
{
public:
	CListDrivers();
	~CListDrivers();
	BOOL ListDrivers(vector<DRIVER_INFO> &vectorDrivers);
	BOOL UnLoadDriver(ULONG DriverObject);
private:
	void FixDriverPath(PDRIVER_INFO pDriverInfo);
};

#endif