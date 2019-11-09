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
#ifndef _HOOK_ENGINE_H_
#define _HOOK_ENGINE_H_

#include "Struct.h"
#include "InitWindows.h"

//Inline Hook Api所用结构体
typedef struct _HOOK_ENVIRONMENT
{
	//前两项的位置不能改变！
	unsigned char savebytes[16];
	unsigned char jmptoapi[5];
	unsigned char hookstub[0x30];
	unsigned char jmptostub[5];
	void*	OrgApiAddr;
	unsigned long SizeOfReplaceCode;
	//unsigned long oldpro;
	PMDL	pMDLOrgApiAddr;//Inline Hook函数前0x10个字节所用的MDL
	BOOL	bSuccessHook;
	BOOL	bProbeLock;
}HOOK_ENVIRONMENT, *PHOOK_ENVIRONMENT;

//////////////////////////////////////////////////////////////////////

NTSTATUS InstallInlineHook(PVOID pOriginAddress, PVOID pFakeAddress, PHOOK_ENVIRONMENT pHookEnv);

NTSTATUS SetShadowHook(VERIABLE_INDEX nVerIndex);
NTSTATUS SetSSDTHook(VERIABLE_INDEX nVerIndex);

NTSTATUS RestoreSSDTHook(VERIABLE_INDEX nVerIndex);
NTSTATUS RestoreShadowHook(VERIABLE_INDEX nVerIndex);

//////////////////////////////////////////////////////////////////////

#endif
