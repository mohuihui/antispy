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
#include "stdafx.h"
#include "ListHooks.h"

CListHooks::CListHooks()
{
}

CListHooks::~CListHooks()
{
}

// 
// 获取ssdt hook
//
BOOL CListHooks::EnumSSDTHooks(vector<SSDT_HOOK_INFO>& vectorHooks)
{
	vectorHooks.clear();

	OPERATE_TYPE opType = enumListSsdtHooks;
	ULONG nCnt = 1000;
	PALL_SSDT_HOOK pSsdtHookInfo = NULL;
	BOOL bRet = FALSE;

	do 
	{
		ULONG nSize = sizeof(ALL_SSDT_HOOK) + nCnt * sizeof(SSDT_HOOK_INFO);

		if (pSsdtHookInfo)
		{
			free(pSsdtHookInfo);
			pSsdtHookInfo = NULL;
		}

		pSsdtHookInfo = (PALL_SSDT_HOOK)GetMemory(nSize);
		if (!pSsdtHookInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pSsdtHookInfo, nSize, NULL);
		nCnt = pSsdtHookInfo->nCnt + 100;

	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pSsdtHookInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pSsdtHookInfo->nCnt; i++)
		{
			vectorHooks.push_back(pSsdtHookInfo->SsdtHook[i]);
		}
	}

	if (pSsdtHookInfo)
	{
		free(pSsdtHookInfo);
		pSsdtHookInfo = NULL;
	}

	return bRet;
}

// 
// 枚举内核层导出函数的patch情况
//
BOOL CListHooks::EnumExportFunctionsPatchs(ULONG Base, 
										   ULONG Size, 
										   WCHAR *szPath, 
										   vector<EXPORT_FUNCTION_PATCH_INFO>& vectorHooks)
{
	vectorHooks.clear();

	if (Base == 0 || Size == 0)
	{
		return FALSE;
	}

	COMMUNICATE_EXPORT_FUNCTION_HOOK cieh;
	
	cieh.OpType = enumExportFunctionsPatchs;
	cieh.op.Get.Base = Base;
	cieh.op.Get.Size = Size;
	wcsncpy_s(cieh.op.Get.szPath, MAX_PATH, szPath, wcslen(szPath));

	ULONG nCnt = 1000;
	PALL_EXPORT_FUNCTION_PATCHS pExportInfo = NULL;
	BOOL bRet = FALSE;

	do 
	{
		ULONG nSize = sizeof(ALL_EXPORT_FUNCTION_PATCHS) + nCnt * sizeof(EXPORT_FUNCTION_PATCH_INFO);

		if (pExportInfo)
		{
			free(pExportInfo);
			pExportInfo = NULL;
		}

		pExportInfo = (PALL_EXPORT_FUNCTION_PATCHS)GetMemory(nSize);
		if (!pExportInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&cieh, sizeof(COMMUNICATE_EXPORT_FUNCTION_HOOK), (PVOID)pExportInfo, nSize, NULL);
		nCnt = pExportInfo->nCnt + 100;

	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pExportInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pExportInfo->nCnt; i++)
		{	
			vectorHooks.push_back(pExportInfo->ExportPatch[i]);
		}	
	}

	if (pExportInfo)
	{
		free(pExportInfo);
		pExportInfo = NULL;
	}

	return bRet;
}

BOOL CListHooks::EnumSSDTInlineHooks(vector<EXPORT_FUNCTION_PATCH_INFO>& vectorHooks)
{
	vectorHooks.clear();

	OPERATE_TYPE OpType = enumSsdtInlineHookInfo;
	ULONG nCnt = 1000;
	PALL_EXPORT_FUNCTION_PATCHS pExportInfo = NULL;
	BOOL bRet = FALSE;

	do 
	{
		ULONG nSize = sizeof(ALL_EXPORT_FUNCTION_PATCHS) + nCnt * sizeof(EXPORT_FUNCTION_PATCH_INFO);

		if (pExportInfo)
		{
			free(pExportInfo);
			pExportInfo = NULL;
		}

		pExportInfo = (PALL_EXPORT_FUNCTION_PATCHS)GetMemory(nSize);
		if (!pExportInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&OpType, sizeof(OPERATE_TYPE), (PVOID)pExportInfo, nSize, NULL);

		nCnt = pExportInfo->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pExportInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pExportInfo->nCnt; i++)
		{
			vectorHooks.push_back(pExportInfo->ExportPatch[i]);
		}
	}

	if (pExportInfo)
	{
		free(pExportInfo);
		pExportInfo = NULL;
	}

	return bRet;
}

BOOL CListHooks::EnumShadowInlineHooks(vector<SHADOW_INLINE_INFO>& vectorHooks)
{
	vectorHooks.clear();

	OPERATE_TYPE OpType = enumShadowSsdtInlineHookInfo;
	ULONG nCnt = 1000;
	PALL_SHADOW_INLINE pShadowHookInfo = NULL;
	BOOL bRet = FALSE;

	do 
	{
		ULONG nSize = sizeof(ALL_SHADOW_INLINE) + nCnt * sizeof(SHADOW_INLINE_INFO);

		if (pShadowHookInfo)
		{
			free(pShadowHookInfo);
			pShadowHookInfo = NULL;
		}

		pShadowHookInfo = (PALL_SHADOW_INLINE)GetMemory(nSize);
		if (!pShadowHookInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&OpType, sizeof(OPERATE_TYPE), (PVOID)pShadowHookInfo, nSize, NULL);

		nCnt = pShadowHookInfo->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pShadowHookInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pShadowHookInfo->nCnt; i++)
		{
			vectorHooks.push_back(pShadowHookInfo->ShadowHooks[i]);
		}
	}

	if (pShadowHookInfo)
	{
		free(pShadowHookInfo);
		pShadowHookInfo = NULL;
	}

	return bRet;
}

BOOL CListHooks::EnumModuleCodeSectionPatchs(ULONG Base, 
										   ULONG Size, 
										   WCHAR *szPath, 
										   vector<MODULE_PATCH_INFO>& vectorHooks)
{
	vectorHooks.clear();

	if (Base == 0 || Size == 0)
	{
		return FALSE;
	}

	COMMUNICATE_EXPORT_FUNCTION_HOOK cieh;
	memset(&cieh, 0, sizeof(COMMUNICATE_EXPORT_FUNCTION_HOOK));
	cieh.OpType = enumAllOfMoudleInlineHookInfo;
	cieh.op.Get.Base = Base;
	cieh.op.Get.Size = Size;
	wcsncpy_s(cieh.op.Get.szPath, MAX_PATH, szPath, wcslen(szPath));

	ULONG nCnt = 1000;
	PALL_OF_MODULE_PATCHS pAllOfModuleHookInfo = NULL;
	BOOL bRet = FALSE;

	do 
	{
		ULONG nSize = sizeof(ALL_OF_MODULE_PATCHS) + nCnt * sizeof(MODULE_PATCH_INFO);

		if (pAllOfModuleHookInfo)
		{
			free(pAllOfModuleHookInfo);
			pAllOfModuleHookInfo = NULL;
		}

		pAllOfModuleHookInfo = (PALL_OF_MODULE_PATCHS)GetMemory(nSize);
		if (!pAllOfModuleHookInfo)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&cieh, sizeof(COMMUNICATE_EXPORT_FUNCTION_HOOK), (PVOID)pAllOfModuleHookInfo, nSize, NULL);

		nCnt = pAllOfModuleHookInfo->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pAllOfModuleHookInfo->nCnt > 0)
	{
		for (ULONG i = 0; i < pAllOfModuleHookInfo->nCnt; i++)
		{
			vectorHooks.push_back(pAllOfModuleHookInfo->ModulePatchs[i]);
		}
	}

	if (pAllOfModuleHookInfo)
	{
		free(pAllOfModuleHookInfo);
		pAllOfModuleHookInfo = NULL;
	}

	return bRet;
}

//
// 枚举模块的IAT\EAT HOOK
//
BOOL CListHooks::EnumIatEatHooks(ULONG nBase,
								 ULONG nSize,
								 WCHAR *szPath,
								 vector<IAT_EAT_HOOK_INFO> &vectorHooks)
{
	vectorHooks.clear();

	if (nBase == 0 ||
		nSize == 0 ||
		szPath == NULL)
	{
		return FALSE;
	}

	ULONG nCnt = 1000;
	PALL_IAT_EAT_HOOK pHooks = NULL;
	BOOL bRet = FALSE;
	
	COMMUNICATE_IAT_EAT_HOOK cieh;
	memset(&cieh, 0, sizeof(COMMUNICATE_IAT_EAT_HOOK));
	cieh.OpType = enumIatEatHook;
	cieh.op.Get.Base = nBase;
	cieh.op.Get.Size = nSize;
	wcsncpy_s(cieh.op.Get.szPath, MAX_PATH, szPath, wcslen(szPath));
	
	do 
	{
		ULONG nSize = sizeof(ALL_IAT_EAT_HOOK) + nCnt * sizeof(IAT_EAT_HOOK_INFO);

		if (pHooks)
		{
			free(pHooks);
			pHooks = NULL;
		}

		pHooks = (PALL_IAT_EAT_HOOK)GetMemory(nSize);
		if (!pHooks)
		{
			break;
		}
		
		bRet = g_ConnectDriver.CommunicateDriver(&cieh, sizeof(COMMUNICATE_IAT_EAT_HOOK), pHooks, nSize, NULL);

		nCnt = pHooks->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pHooks->nCnt > 0)
	{
		for (ULONG i = 0; i < pHooks->nCnt; i++)
		{
			vectorHooks.push_back(pHooks->hooks[i]);
		}
	}

	if (pHooks)
	{
		free(pHooks);
		pHooks = NULL;
	}

	return bRet;
}