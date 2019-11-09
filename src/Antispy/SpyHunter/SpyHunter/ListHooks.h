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
#ifndef _LIST_HOOKS_H_H
#define _LIST_HOOKS_H_H

#include "../../Common/Common.h"
#include <vector>
#include "Function.h"

using namespace std;

class CListHooks
{
public:
	CListHooks();
	~CListHooks();
	BOOL EnumSSDTHooks(vector<SSDT_HOOK_INFO>& vectorHooks);
	BOOL EnumExportFunctionsPatchs(ULONG Base, ULONG Size, WCHAR *szPath, vector<EXPORT_FUNCTION_PATCH_INFO>& vectorHooks);
	BOOL EnumSSDTInlineHooks(vector<EXPORT_FUNCTION_PATCH_INFO>& vectorHooks);
	BOOL EnumShadowInlineHooks(vector<SHADOW_INLINE_INFO>& vectorHooks);
	BOOL EnumModuleCodeSectionPatchs(ULONG Base, ULONG Size, WCHAR *szPath, vector<MODULE_PATCH_INFO>& vectorHooks);
	BOOL EnumIatEatHooks(ULONG nBase, ULONG nSize, WCHAR *szPath, vector<IAT_EAT_HOOK_INFO> &vectorHooks);
};

#endif