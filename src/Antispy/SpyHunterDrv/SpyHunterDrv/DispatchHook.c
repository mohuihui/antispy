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
#include "dispatchhook.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Peload.h"
#include "libdasm.h"

NTSTATUS RestoreDispatchHooks(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDISPATCH_HOOK pcs = (PDISPATCH_HOOK)pInBuffer;
	ULONG nIndex = pcs->op.Restore.nIndex;
	DISPATCH_TYPE DispatchType = pcs->op.Restore.DispatchType;
	PULONG pOrigin = NULL;
	PULONG pReload = NULL;
	PDRIVER_OBJECT pDriverObject = NULL;

	switch (DispatchType)
	{
	case DispatchNtfs:
		{
			PDRIVER_OBJECT pNtfsDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumNtfsDriverObject);
			if (pNtfsDriverObject)
			{
				pDriverObject = pNtfsDriverObject;
				pReload = (PULONG)GetGlobalVeriable(enumNtfsReloadDispatchs);
				pOrigin = (PULONG)GetGlobalVeriable(enumNtfsOriginalDispatchs);
			}
		}
		break;

	case DispatchFastfat:
		{
			PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumFastfatDriverObject);
			if (pFastfatDriverObject)
			{
				pDriverObject = pFastfatDriverObject;
				pReload = (PULONG)GetGlobalVeriable(enumFastfatReloadDispatchs);
				pOrigin = (PULONG)GetGlobalVeriable(enumFastfatOriginalDispatchs);
			}	
		}
		break;
	
	case DispatchExfat:
		break;
			
	case DispatchAcpi:
		{
			PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAcpiDriverObject);
			if (pFastfatDriverObject)
			{
				pDriverObject = pFastfatDriverObject;
				pReload = (PULONG)GetGlobalVeriable(enumAcpiReloadDispatchs);
				pOrigin = (PULONG)GetGlobalVeriable(enumAcpiOriginalDispatchs);
			}	
		}
		break;
			
	case DispatchAtapi:
		{
			PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAtapiDriverObject);
			if (pFastfatDriverObject)
			{
				if (nIndex == 28)
				{
					if (GetGlobalVeriable(enumOriginAtapiStartIoDispatch))
					{
						KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
						WPOFF();
						pFastfatDriverObject->DriverStartIo = (PDRIVER_STARTIO)GetGlobalVeriable(enumOriginAtapiStartIoDispatch);
						WPON();
						KeLowerIrql(OldIrql);
					}
				}
				else
				{
					pDriverObject = pFastfatDriverObject;
					pReload = (PULONG)GetGlobalVeriable(enumAtapiReloadDispatchs);
					pOrigin = (PULONG)GetGlobalVeriable(enumAtapiOriginalDispatchs);
				}
			}	
		}
		break;

	case DispatchDisk:
		{
			PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumClasspnpDriverObject);
			if (pFastfatDriverObject)
			{
				pDriverObject = pFastfatDriverObject;
				pReload = (PULONG)GetGlobalVeriable(enumClasspnpReloadDispatchs);
				pOrigin = (PULONG)GetGlobalVeriable(enumClasspnpOriginalDispatchs);
			}	
		}
		break;

	case DispatchKbdclass:
		{
			PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumKbdClassDriverObject);
			if (pFastfatDriverObject)
			{
				pDriverObject = pFastfatDriverObject;
				pReload = (PULONG)GetGlobalVeriable(enumKbdClassReloadDispatchs);
				pOrigin = (PULONG)GetGlobalVeriable(enumKbdClassOriginalDispatchs);
			}	
		}
		break;

	case DispatchMouclass:
		{
			PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumMouClassDriverObject);
			if (pFastfatDriverObject)
			{
				pDriverObject = pFastfatDriverObject;
				pReload = (PULONG)GetGlobalVeriable(enumMouClassReloadDispatchs);
				pOrigin = (PULONG)GetGlobalVeriable(enumMouClassOriginalDispatchs);
			}	
		}
		break;

	case DispatchTcpip:
		{
			PDRIVER_OBJECT pFastfatDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumTcpipDriverObject);
			if (pFastfatDriverObject)
			{
				pDriverObject = pFastfatDriverObject;
				pReload = (PULONG)GetGlobalVeriable(enumTcpipReloadDispatchs);
				pOrigin = (PULONG)GetGlobalVeriable(enumTcpipOriginalDispatchs);
			}	
		}
		break;

	default:
		{
			KdPrint(("UnKnow Dispatch Type\n"));
		}
		break;
	}

	KdPrint(("index: %d, pOrigin: 0x%08X, pReload: 0x%08X, Driver: 0x%08X\n", nIndex, pOrigin[nIndex], pReload[nIndex], pDriverObject));

	if (pOrigin && pDriverObject)
	{
		if ((ULONG)pDriverObject->MajorFunction[nIndex] != pOrigin[nIndex])
		{
			ExchangeAddress((PULONG)&pDriverObject->MajorFunction[nIndex], pOrigin[nIndex]);
		}

		if (pReload)
		{
			if (memcmp((PBYTE)pOrigin[nIndex], (PBYTE)pReload[nIndex], 0x10))
			{
				KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
				WPOFF();
				memcpy((PVOID)pOrigin[nIndex], (PVOID)pReload[nIndex], 0x10);
				WPON();
				KeLowerIrql(OldIrql);
			}
		}
	}

	return status;
}

ULONG GetDispatchInlineAddress(ULONG nIndex, PULONG pOriginDispatch, PULONG pReloadDispatch)
{
	ULONG pAddress = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (pOriginDispatch[nIndex] && 
		MzfMmIsAddressValid((PVOID)pOriginDispatch[nIndex]) && 
		pReloadDispatch[nIndex] &&
		MzfMmIsAddressValid((PVOID)pReloadDispatch[nIndex]))
	{
		ULONG pReload = pReloadDispatch[nIndex];
		ULONG pOrigin = pOriginDispatch[nIndex];

		if (memcmp((PBYTE)pReload, (PBYTE)pOrigin, 0x10))
		{
			pAddress = GetInlineHookAddress(pOrigin, 0x10);

			if (pAddress == 0 || 
				pAddress < SYSTEM_ADDRESS_START || 
				!MzfMmIsAddressValid((PVOID)pAddress))
			{
				pAddress = 1;
			}
		}
	}

	if (pAddress)
	{
		KdPrint(("%d 0x%08X\n", nIndex, pAddress));
	}

	return pAddress;
}

void GetDispatchHook(PDISPATCH_HOOK_INFO pHookInfo, PDRIVER_OBJECT pDriverObject, PULONG pOriginDispatch, PULONG pReloadDispatch)
{
	ULONG i = 0;
	for ( i = 0; i < 28; i++ )
	{
		pHookInfo[i].nIndex = i;
		pHookInfo[i].pNowAddress = (ULONG)pDriverObject->MajorFunction[i];
		pHookInfo[i].pOriginAddress = pOriginDispatch[i];
		pHookInfo[i].pInlineHookAddress = GetDispatchInlineAddress(i, pOriginDispatch, pReloadDispatch);
	}
}