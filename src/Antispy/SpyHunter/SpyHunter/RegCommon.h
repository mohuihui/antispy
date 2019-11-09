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
#ifndef _REG_COMMON_H
#define _REG_COMMON_H
#include "Registry.h"
#include "Function.h"
#include <list>
using namespace std;

class CRegCommon
{
public:
	CString GetCLSIDDescription(CString szCLSID);
	CString GetInprocServer32Value(CString szKey);
	CString GetCLSIDModule(CString szKey);
	CString GetCurrentUserKeyPath();
	CString GetCurrentReg();
	CString NtKeyPath2DosKeyPath(CString szKey);
private:
	CRegistry m_Registry;
	CommonFunctions m_Function;
};

#endif