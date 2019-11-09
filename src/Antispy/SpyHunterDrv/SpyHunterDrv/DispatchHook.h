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
#ifndef _DISPATCH_HOOK_H_
#define _DISPATCH_HOOK_H_

#include "Struct.h"
#include "..\\..\\Common\\Common.h"

NTSTATUS RestoreDispatchHooks(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);
ULONG GetDispatchInlineAddress(ULONG nIndex, PULONG pOriginNtfsDispatch, PULONG pReloadNtfsDispatch);
void GetDispatchHook(PDISPATCH_HOOK_INFO pFsdHook, PDRIVER_OBJECT pFsdDriverObject, PULONG pOriginFsdDispatch, PULONG pReloadFsdDispatch);

#endif
