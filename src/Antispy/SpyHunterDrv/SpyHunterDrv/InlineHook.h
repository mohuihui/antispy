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
#ifndef _INLINE_HOOK_H_
#define _INLINE_HOOK_H_

#include <ntifs.h>
#include <windef.h>

typedef struct _HOOK_ENVIRONMENT_
{
	BYTE SaveBytes[16];
	BYTE JmpToApi[5];
	BYTE HookStub[0x30];
	BYTE JmpToStub[5];
	PVOID OriginAddress;
	PVOID HookAddress;
	ULONG SizeOfReplaceCode;
	BOOL bSuccessHook;
}HOOK_ENVIRONMENT,*PHOOK_ENVIRONMENT;

typedef enum _HOOK_TYPE_
{
	enumHookExport,
	enumHookNotExport,
	enumUnHook
}HOOK_TYPE;

BOOL SafeHookOrUnhook(PHOOK_ENVIRONMENT pHookEnv, HOOK_TYPE nHook);

#endif