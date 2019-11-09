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
#include "HookEngine.h"
#include "ldasm.h"
#include "CommonFunction.h"

NTSTATUS InlineHook( PVOID pDes, PVOID pSrc, size_t nSize )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PMDL pMdl = IoAllocateMdl((PVOID)pDes, nSize, FALSE, FALSE, NULL);
	if ( pMdl )
	{
		if ( (DWORD)pDes < SYSTEM_ADDRESS_START )
		{
			PVOID pNewAddr = NULL;
			MmBuildMdlForNonPagedPool(pMdl);
			pMdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
			pNewAddr = MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, 0, NormalPagePriority);
			if ( pNewAddr )
			{
				memcpy(pNewAddr, pSrc, nSize);
				MmUnmapLockedPages(pNewAddr, pMdl);
				status = STATUS_SUCCESS;
			}
		}
		else
		{
			PVOID pNewAddr = NULL;
			__try
			{
				MmProbeAndLockPages(pMdl, KernelMode, IoWriteAccess);

				pNewAddr = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);

				if ( pNewAddr )
				{
					KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
					memcpy(pNewAddr, pSrc, nSize);
					KfLowerIrql(OldIrql);
					status = STATUS_SUCCESS;
				}

				MmUnlockPages(pMdl);
			}
			__except(1)
			{
				status = STATUS_UNSUCCESSFUL;
			}
		}
		
		IoFreeMdl(pMdl);
	}

	return status;
}

// 堆栈中没有返回地址和call地址的hook方式
NTSTATUS InstallInlineHook(PVOID pOriginAddress, PVOID pFakeAddress, PHOOK_ENVIRONMENT pHookEnv)
{
	BYTE bFirst, bSecond;
	int ReplaceCodeSize = 0;
	KIRQL OldIrql;
	BYTE JMPGate[5] = {0xE9, 0x00, 0x00, 0x00, 0x00};
	unsigned char *pOpcode = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (pFakeAddress == NULL || pOriginAddress == NULL || pHookEnv == NULL)
	{
		return status;
	}

	KdPrint(("StubAddress: 0x%08X, HookProc: 0x%08X\n", pOriginAddress, pFakeAddress));

	//如果原来已经有HOOK,则放弃
	do 
	{
		bFirst = *((PBYTE)pOriginAddress + ReplaceCodeSize);
		bSecond = *((PBYTE)pOriginAddress + ReplaceCodeSize + 1);

		if (bFirst == 0xE8
			||bFirst == 0xFF && (bSecond == 0x15 || bSecond == 0x25)
			||bFirst == 0xEA
			||bFirst == 0xE9
			||bFirst == 0xEB
			||bFirst == 0xF && ( bFirst &= 0xF0, bFirst == 0x80)
			||(bFirst & 0xF0) == 0x70
			||(bFirst & 0xFC) == 0xE0)
		{
			return status;
		}

		ReplaceCodeSize += SizeOfCode((PBYTE)pOriginAddress + ReplaceCodeSize, &pOpcode);
		KdPrint(("ReplaceCodeSize : %d\n", ReplaceCodeSize));

	} while (ReplaceCodeSize < sizeof(JMPGate));

	if (ReplaceCodeSize > 16)
	{
		return status;
	}

	// 保存原始函数的指令
	memset((void*)pHookEnv->savebytes, 0x90, sizeof(pHookEnv->savebytes));
	RtlCopyMemory(pHookEnv->savebytes, pOriginAddress, ReplaceCodeSize);

	pHookEnv->SizeOfReplaceCode = ReplaceCodeSize;

	// 保存原始函数地址
	pHookEnv->OrgApiAddr = pOriginAddress;

	// 填充由stub函数跳转回原始函数+ReplaceCodeSize的地址
	pHookEnv->jmptoapi[0] = 0xE9;
	*(DWORD*)(&pHookEnv->jmptoapi[1]) = (DWORD)pOriginAddress + ReplaceCodeSize - ((DWORD)pHookEnv->jmptoapi + 5);

	// 填充由原始函数跳到hook函数的地址
	*(DWORD*)(&JMPGate[1]) = ((DWORD)pFakeAddress) - ((DWORD)pOriginAddress + sizeof(JMPGate));

	// patch api
// 	OldIrql = KeRaiseIrqlToDpcLevel();
// 	WPOFF();
// 	memcpy(pOriginAddress, JMPGate, sizeof(JMPGate));
// 	WPON();
// 	KeLowerIrql(OldIrql);
	
	status = InlineHook(pOriginAddress, JMPGate, sizeof(JMPGate));

	return status;
}

NTSTATUS SetShadowHook(VERIABLE_INDEX nVerIndex)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PEPROCESS pCsrssProcess = (PEPROCESS)GetGlobalVeriable(enumCsrssEprocess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	PServiceDescriptorTableEntry_t pNowShadowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowShadowSSDT);
	PServiceDescriptorTableEntry_t pOriginShadowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumOriginShadowSSDT);
	KAPC_STATE as;
	ULONG nOrigin = 0;
	ULONG nIndex = GetGlobalVeriable(nVerIndex); 
	ULONG nFake = GetGlobalVeriable(nVerIndex + 2);
	KIRQL oldIrql;

	if (pCsrssProcess == NULL || 
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess || 
		!pNowShadowSSDT ||
		!pOriginShadowSSDT ||
		!nFake) // hook函数不能为0
	{
		return status;
	}

	// attach 到Csrss进程
	MzfKeStackAttachProcess(pCsrssProcess, &as);
	
	// 保存原始函数
	nOrigin = pOriginShadowSSDT->ServiceTableBase[nIndex];
	SetGlobalVeriable(nVerIndex + 1, nOrigin);
	
	// 提升IRQL
	oldIrql = KeRaiseIrqlToDpcLevel();

	// hook
	WPOFF();
	pNowShadowSSDT->ServiceTableBase[nIndex] = nFake;
	WPON();
	
	KeLowerIrql(oldIrql);

	MzfKeUnstackDetachProcess(&as);

	status = STATUS_SUCCESS;

	return status;
}

NTSTATUS SetSSDTHook(VERIABLE_INDEX nVerIndex)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PServiceDescriptorTableEntry_t pNowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowSSDT);
	PServiceDescriptorTableEntry_t pOriginSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumOriginSSDT);
	KAPC_STATE as;
	ULONG nOrigin = 0;
	ULONG nIndex = GetGlobalVeriable(nVerIndex); 
	ULONG nFake = GetGlobalVeriable(nVerIndex + 2);
	KIRQL oldIrql;

	if (!pNowSSDT ||
		!pOriginSSDT ||
		!nFake) // hook函数不能为0
	{
		return status;
	}

	// 保存原始函数
	nOrigin = pOriginSSDT->ServiceTableBase[nIndex];
	SetGlobalVeriable(nVerIndex + 1, nOrigin);

	// 提升IRQL
	oldIrql = KeRaiseIrqlToDpcLevel();

	// hook
	WPOFF();
	pNowSSDT->ServiceTableBase[nIndex] = nFake;
	WPON();

	KeLowerIrql(oldIrql);

	status = STATUS_SUCCESS;

	return status;
}

NTSTATUS RestoreSSDTHook(VERIABLE_INDEX nVerIndex)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PServiceDescriptorTableEntry_t pNowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowSSDT);
	KAPC_STATE as;
	ULONG nIndex = GetGlobalVeriable(nVerIndex); 
	KIRQL oldIrql;
	ULONG nOrigin = GetGlobalVeriable(nVerIndex + 1);

	if (!pNowSSDT || !nOrigin)
	{
		return status;
	}

	// 提升IRQL
	oldIrql = KeRaiseIrqlToDpcLevel();

	// hook
	WPOFF();
	pNowSSDT->ServiceTableBase[nIndex] = nOrigin;
	WPON();

	KeLowerIrql(oldIrql);

	status = STATUS_SUCCESS;

	return status;
}

NTSTATUS RestoreShadowHook(VERIABLE_INDEX nVerIndex)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PEPROCESS pCsrssProcess = (PEPROCESS)GetGlobalVeriable(enumCsrssEprocess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	PServiceDescriptorTableEntry_t pNowShadowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowShadowSSDT);
	KAPC_STATE as;
	ULONG nOrigin = GetGlobalVeriable(nVerIndex + 1);
	ULONG nIndex = GetGlobalVeriable(nVerIndex); 
	KIRQL oldIrql;

	if (pCsrssProcess == NULL || 
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess || 
		!pNowShadowSSDT ||
		!nOrigin)
	{
		return status;
	}

	// attach 到Csrss进程
	MzfKeStackAttachProcess(pCsrssProcess, &as);

	// 提升IRQL
	oldIrql = KeRaiseIrqlToDpcLevel();

	// hook
	WPOFF();
	pNowShadowSSDT->ServiceTableBase[nIndex] = nOrigin;
	WPON();

	KeLowerIrql(oldIrql);

	MzfKeUnstackDetachProcess(&as);

	status = STATUS_SUCCESS;

	return status;
}