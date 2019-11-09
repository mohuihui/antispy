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
#include "InlineHook.h"
#include "disasm.h"
#include "CommonFunction.h"
#include "ldasm.h"

KSPIN_LOCK g_SpinLock;
ULONG g_nRaisedCpuCnt = 0;


#define STUB_SHELL_SIZE 0x1b

__declspec(naked) void StubShell()
{
	__asm
	{
		push [esp]
		push [esp]
		call _next
_next:
		xchg dword ptr [esp], eax
		lea eax, [eax - 0x20]
		mov dword ptr [esp + 0xC], eax
		pop eax
		_emit 0xE9
		_emit 'f'
		_emit 'u'
		_emit 'c'
		_emit 'k'
	}
}

BOOL InlineHookExportApi(PHOOK_ENVIRONMENT pHookEnv)
{
	DWORD dwHookStubAddress = 0;
	ULONG uCodeSize = 0;
	KIRQL irql;
	BYTE JMPGate[5] = {0xE9, 0x90, 0x90, 0x90, 0x90};

	if (pHookEnv == NULL)
	{
		return FALSE;
	}

	do 
	{
		BYTE bFirst, bSecond;
		PCHAR pCode;

		bFirst = *((PBYTE)pHookEnv->OriginAddress + uCodeSize);
		bSecond = *((PBYTE)pHookEnv->OriginAddress + uCodeSize + 1);

		if (
			bFirst == 0xE8											||
			bFirst == 0xFF && (bSecond == 0x15 || bSecond == 0x25)	||
			bFirst == 0xEA											||
			bFirst == 0xE9											||
			bFirst == 0xEB											||
			bFirst == 0xF && ( bFirst &= 0xF0, bFirst == 0x80u)		||
			(bFirst & 0xF0) == 0x70									||
			(bFirst & 0xFC) == 0xE0
			)
		{
			return FALSE;
		}

		uCodeSize += SizeOfCode((PVOID)((ULONG)pHookEnv->OriginAddress + uCodeSize), &pCode);
	//	uCodeSize += GetOpCodeSize((PBYTE)pHookEnv->OriginAddress + uCodeSize);

	} while (uCodeSize < sizeof(JMPGate));

	if (uCodeSize > 16)
	{
		return FALSE;
	}

	memset((void*)pHookEnv->SaveBytes, 0x90, sizeof(pHookEnv->SaveBytes));
	RtlCopyMemory((PBYTE)pHookEnv->HookStub, (PBYTE)StubShell, STUB_SHELL_SIZE);
	RtlCopyMemory(pHookEnv->SaveBytes, pHookEnv->OriginAddress, uCodeSize);

	pHookEnv->SizeOfReplaceCode = uCodeSize;

	pHookEnv->JmpToApi[0] = 0xE9;
	*(DWORD*)(&pHookEnv->JmpToApi[1]) = (DWORD)pHookEnv->OriginAddress + uCodeSize - ((DWORD)pHookEnv->JmpToApi + 5);

	dwHookStubAddress = (DWORD)pHookEnv->HookStub;

	pHookEnv->JmpToStub[0] = 0xE9;
	*(DWORD*)(&pHookEnv->JmpToStub[1]) = (DWORD)(dwHookStubAddress) - ((DWORD)pHookEnv->JmpToStub + 5);

	*(DWORD*)(&JMPGate[1]) = ((DWORD)pHookEnv->JmpToStub) - ((DWORD)pHookEnv->OriginAddress + sizeof(JMPGate));
	*(DWORD*)(dwHookStubAddress + STUB_SHELL_SIZE - 4) = (DWORD)pHookEnv->HookAddress - (dwHookStubAddress + STUB_SHELL_SIZE);

	irql = KeRaiseIrqlToDpcLevel();
	WPOFF();
	memcpy(pHookEnv->OriginAddress, JMPGate, sizeof(JMPGate));
	WPON();
	KeLowerIrql(irql);

	pHookEnv->bSuccessHook = TRUE;

	return TRUE;
}

BOOL FixHookFunction(PHOOK_ENVIRONMENT pHookEnv)
{
	ULONG uOpCodeLen = 0;
	BOOL bRet = FALSE;
	ULONG i = 0;

	if (pHookEnv == NULL)
	{
		return FALSE;
	}

	for ( i = 0; i < 0x100; i++)
	{
		if (MmIsAddressValid((PVOID)((ULONG)pHookEnv->HookAddress + uOpCodeLen)))
		{
			ULONG uTemp = 0;
			PCHAR pCode;

			//uTemp = GetOpCodeSize((PBYTE)((ULONG)pHookEnv->HookAddress + uOpCodeLen));
			uTemp = SizeOfCode((PVOID)((ULONG)pHookEnv->HookAddress + uOpCodeLen), &pCode);
			uOpCodeLen += uTemp;

			if (uTemp == 1 && *(PBYTE)((ULONG)pHookEnv->HookAddress + uOpCodeLen) == 0x90)
			{
				BYTE JmpGate[5] = {0xe9, 'f', 'u', 'c', 'k'};
				KIRQL Irql;

				*(PULONG)((ULONG)JmpGate + 1) = (ULONG)pHookEnv->OriginAddress - ((ULONG)pHookEnv->HookAddress + pHookEnv->SizeOfReplaceCode + uOpCodeLen) + (pHookEnv->SizeOfReplaceCode - 5);

				Irql = KeRaiseIrqlToDpcLevel();
				WPOFF();

				RtlCopyMemory((PBYTE)((ULONG)pHookEnv->HookAddress + uOpCodeLen), pHookEnv->SaveBytes, pHookEnv->SizeOfReplaceCode);
				RtlCopyMemory((PBYTE)((ULONG)pHookEnv->HookAddress + uOpCodeLen + pHookEnv->SizeOfReplaceCode), JmpGate, sizeof(JmpGate));

				WPON();
				KeLowerIrql(Irql);

				bRet = TRUE;
				break;
			}
		}
		else
		{
			break;
		}
	}

	return bRet;
}

BOOL InlineHookNotExportApi(PHOOK_ENVIRONMENT pHookEnv)
{
	ULONG ulCodeLen = 0;
	KIRQL Irql;
	BYTE JMPGate[5] = {0xe9, 'f', 'u', 'c', 'k'};
	BOOL bRet = FALSE;

	if (pHookEnv == NULL)
	{
		return FALSE;
	}

	do 
	{
		BYTE bFirst, bSecond;
		PCHAR pCode;

		bFirst = *((PBYTE)pHookEnv->OriginAddress + ulCodeLen);
		bSecond = *((PBYTE)pHookEnv->OriginAddress + ulCodeLen + 1);

		if (
			bFirst == 0xE8											||
			bFirst == 0xFF && (bSecond == 0x15 || bSecond == 0x25)	||
			bFirst == 0xEA											||
			bFirst == 0xE9											||
			bFirst == 0xEB											||
			bFirst == 0xF && ( bFirst &= 0xF0, bFirst == 0x80u)		||
			(bFirst & 0xF0) == 0x70									||
			(bFirst & 0xFC) == 0xE0
			)
		{
			return FALSE;
		}

		ulCodeLen += SizeOfCode((PVOID)((ULONG)pHookEnv->OriginAddress + ulCodeLen), &pCode);
	} while (ulCodeLen < sizeof(JMPGate));

	if (ulCodeLen > 16)
	{
		return FALSE;
	}

	pHookEnv->SizeOfReplaceCode = ulCodeLen;
	memcpy(pHookEnv->SaveBytes, (PBYTE)pHookEnv->OriginAddress, ulCodeLen);

	if (FixHookFunction(pHookEnv))
	{
		*(PULONG)((PBYTE)JMPGate + sizeof(BYTE)) = (ULONG)pHookEnv->HookAddress - ((ULONG)pHookEnv->OriginAddress + 5);

		Irql = KeRaiseIrqlToDpcLevel();
		WPOFF();
		RtlCopyMemory((PBYTE)pHookEnv->OriginAddress, JMPGate, ulCodeLen);
		WPON();
		KeLowerIrql(Irql);

		pHookEnv->bSuccessHook = TRUE;
		bRet = TRUE;
	}

	return bRet;
}

VOID WaitForHook(
				 __in struct _KDPC  *Dpc,
				 __in_opt PVOID  DeferredContext,
				 __in_opt PVOID  SystemArgument1,
				 __in_opt PVOID  SystemArgument2
				 )
{
	KIRQL OldIrql;

	OldIrql = KeRaiseIrqlToDpcLevel();
	InterlockedIncrement(&g_nRaisedCpuCnt);
	KeAcquireSpinLockAtDpcLevel(&g_SpinLock);
	KeReleaseSpinLockFromDpcLevel(&g_SpinLock);
	KeLowerIrql(OldIrql);
}

BOOL UnInlineHook(PHOOK_ENVIRONMENT pHookEnv)
{
	BOOL bRet = FALSE;

	if (pHookEnv->bSuccessHook)
	{
		KIRQL Irql = KeRaiseIrqlToDpcLevel();
		WPOFF();		
		RtlCopyMemory(pHookEnv->OriginAddress, pHookEnv->SaveBytes, pHookEnv->SizeOfReplaceCode);
		WPON();
		KeLowerIrql(Irql);
		pHookEnv->bSuccessHook = FALSE;
		bRet = TRUE;
	}

	return bRet;
}

BOOL SafeHookOrUnhook(PHOOK_ENVIRONMENT pHookEnv, HOOK_TYPE nHook)
{
	ULONG i = 0;
	KAFFINITY CpuAffinity;
	ULONG ulCpuCnt = 0;
	BOOL bRet = FALSE;

	if (pHookEnv == NULL)
	{
		return bRet;
	}

	g_nRaisedCpuCnt = 0;

	CpuAffinity = KeQueryActiveProcessors();
	for (i = 0; i < 32; i++)
	{
		if ( (CpuAffinity >> i) & 1 )
		{
			ulCpuCnt++;
		}
	}

	if ( ulCpuCnt == 1 ) 
	{
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

		switch (nHook)
		{
		case enumHookExport:
			bRet = InlineHookExportApi(pHookEnv);
			break;

		case enumHookNotExport:
			bRet = InlineHookNotExportApi(pHookEnv);
			break;

		case enumUnHook:
			bRet = UnInlineHook(pHookEnv);	
			break;
		}
		
		KeLowerIrql(OldIrql);
	}
	else  
	{
		KIRQL OldIrql;
		ULONG  ulCurrentCpu = KeGetCurrentProcessorNumber();
		
		KeInitializeSpinLock(&g_SpinLock);
		KeAcquireSpinLock(&g_SpinLock, &OldIrql);
		CpuAffinity = KeQueryActiveProcessors();
		
		for (i = 0; i < 32; i++) 
		{
			if ( ((CpuAffinity >> i) & 1) && ulCurrentCpu != i ) 
			{
				KDPC Dpc;
				KeInitializeDpc(&Dpc, WaitForHook, NULL);
				KeSetTargetProcessorDpc(&Dpc, (CCHAR)i);
				KeSetImportanceDpc(&Dpc, HighImportance);
				KeInsertQueueDpc(&Dpc, NULL, NULL);
			}
		}

		for (i = 0; i < 16; i ++)
		{
			ULONG ulTmp = 1000000;
			while (ulTmp)
			{
				ulTmp--;
			}

			if ( g_nRaisedCpuCnt == (ulCpuCnt - 1) ) 
			{
				switch (nHook)
				{
				case enumHookExport:
					bRet = InlineHookExportApi(pHookEnv);
					break;

				case enumHookNotExport:
					bRet = InlineHookNotExportApi(pHookEnv);
					break;

				case enumUnHook:
					bRet = UnInlineHook(pHookEnv);	
					break;
				}

				break;
			}
		}

		KeReleaseSpinLock(&g_SpinLock, OldIrql);    
	}

	return bRet;
}