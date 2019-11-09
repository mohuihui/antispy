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
#include "ShadowSSDT.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "libdasm.h"

ULONG GetShadowSsdtFunctionInlineAddress(ULONG nIndex)
{
	ULONG pAddress = 0;
	PULONG pNewShadowSsdt = (PULONG)GetGlobalVeriable(enumReloadShadowKiServiceTable);
	PULONG pNowShadowSsdt = (PULONG)GetGlobalVeriable(enumOriginShadowKiServiceTable);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (pNewShadowSsdt && pNowShadowSsdt && MzfMmIsAddressValid && MzfMmIsAddressValid(pNewShadowSsdt) && MzfMmIsAddressValid(pNowShadowSsdt)) 
	{
		if (MzfMmIsAddressValid((PVOID)pNewShadowSsdt[nIndex]) && MzfMmIsAddressValid((PVOID)pNowShadowSsdt[nIndex]))
		{
			ULONG pOrigin = pNewShadowSsdt[nIndex];
			ULONG pNow = pNowShadowSsdt[nIndex];

			if (memcmp((PBYTE)pOrigin, (PBYTE)pNow, 0x10))
			{
				pAddress = GetInlineHookAddress(pNow, 0x10);

				if (pAddress == 0 || 
					pAddress < SYSTEM_ADDRESS_START || 
					!MzfMmIsAddressValid((PVOID)pAddress))
				{
					pAddress = 1;
				}
			}
		}
	}

	return pAddress;
}

NTSTATUS PspEnumShadowSsdtHookInfo(PSHADOW_SSDT_HOOK psh)
{
	PULONG pOriginShadowSsdt = (PULONG)GetGlobalVeriable(enumOriginShadowKiServiceTable);
	PServiceDescriptorTableEntry_t pShadowSsdt = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowShadowSSDT);
 	ULONG nCnt = GetGlobalVeriable(enumShadowSsdtFunctionsCnt);
 	NTSTATUS status = STATUS_UNSUCCESSFUL;

	KdPrint(("ShadowSsdt nCnt: %d\n", nCnt));

	if ( psh && pOriginShadowSsdt && pShadowSsdt && nCnt )
	{
		ULONG i = 0;
		for (i = 0; i < nCnt; i++)
		{
 			ULONG pOrigin = pOriginShadowSsdt[i];
			ULONG pNow = pShadowSsdt->ServiceTableBase[i];

			if (psh->nCnt > psh->nRetCnt)
			{
				psh->ShadowSsdtHook[psh->nRetCnt].nIndex = i;
				psh->ShadowSsdtHook[psh->nRetCnt].pNowAddress = pNow;
				psh->ShadowSsdtHook[psh->nRetCnt].pOriginAddress = pOrigin;
				psh->ShadowSsdtHook[psh->nRetCnt].pInlineHookAddress = GetShadowSsdtFunctionInlineAddress(i);
			}

			psh->nRetCnt++;
 		}

		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS EnumShadowSsdtHookInfo(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSHADOW_SSDT_HOOK psh = (PSHADOW_SSDT_HOOK)pOutBuffer;

	if (NT_SUCCESS(PspEnumShadowSsdtHookInfo(psh)))
	{
		if (psh->nRetCnt && psh->nCnt > psh->nRetCnt)
		{
			status = STATUS_SUCCESS;
		}
	}

	return status;
}