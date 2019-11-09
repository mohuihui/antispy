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
#include "fsd.h"
#include "Peload.h"
#include "libdasm.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "DispatchHook.h"

NTSTATUS GetFsdDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDISPATCH_HOOK_INFO pFsdHook = (PDISPATCH_HOOK_INFO)pOutBuffer;
	PULONG pReloadNtfsDispatch = (PULONG)GetGlobalVeriable(enumNtfsReloadDispatchs);
	PULONG pOriginNtfsDispatch = (PULONG)GetGlobalVeriable(enumNtfsOriginalDispatchs);
	PDRIVER_OBJECT pNtfsDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumNtfsDriverObject);
	PULONG pReloadFastfatDispatch = (PULONG)GetGlobalVeriable(enumFastfatReloadDispatchs);
	PULONG pOriginFastfatDispatch = (PULONG)GetGlobalVeriable(enumFastfatOriginalDispatchs);
	PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumFastfatDriverObject);
	
	if (pReloadNtfsDispatch && pNtfsDriverObject && pOriginNtfsDispatch)
	{
		GetDispatchHook( pFsdHook, pNtfsDriverObject, pOriginNtfsDispatch, pReloadNtfsDispatch );
	}

	if (pReloadFastfatDispatch && pOriginFastfatDispatch && pFastfatDriverObject)
	{
		pFsdHook = pFsdHook + 28;
		GetDispatchHook( pFsdHook, pFastfatDriverObject, pOriginFastfatDispatch, pReloadFastfatDispatch );
	}

	return status;
}