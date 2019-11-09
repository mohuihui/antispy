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
#include "KernelMemory.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"

NTSTATUS ModifyKernelMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (uInSize == sizeof(COMMUNICATE_MODIFY_KERNEL_ADDRESS) &&
		MzfProbeForRead &&
		MzfMmIsAddressValid)
	{
		PCOMMUNICATE_MODIFY_KERNEL_ADDRESS pCmda = (PCOMMUNICATE_MODIFY_KERNEL_ADDRESS)pInBuffer;
		DWORD dwAddress = pCmda->nAddress;
		DWORD dwLen = pCmda->nLen;
		PBYTE pCotent = pCmda->pContent;

		KdPrint(("dwAddress: 0x%08X, dwLen: %d, pCotent: 0x%08X\n", dwAddress, dwLen, pCotent));
		if (dwAddress > SYSTEM_ADDRESS_START && dwLen > 0 && pCotent > 0)
		{
			__try
			{
				MzfProbeForRead(pCotent, dwLen ,1);

// 				if (MzfMmIsAddressValid((PVOID)dwAddress))
// 				{
// 					*(PBYTE)dwAddress = *pCotent;
// 					status = STATUS_SUCCESS;
// 				}

				if (IsAddressValid((PVOID)dwAddress, dwLen))
				{
					KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
					status = STATUS_SUCCESS;

					WPOFF();
					__try
					{
						memcpy((PVOID)dwAddress, pCotent, dwLen);
					}
					__except(1)
					{
						status = STATUS_UNSUCCESSFUL;
					}

					WPON();
					KeLowerIrql(OldIrql);
				}
			}
			__except(1)
			{
				status = STATUS_UNSUCCESSFUL;
			}
		}
	}

	return status;
}