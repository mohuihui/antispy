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
#include "DebugRegister.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"

ULONG g_nCpuCnt = 0;

// VOID WaitForGetDebugRegister(
// 				 __in struct _KDPC  *Dpc,
// 				 __in_opt PVOID  DeferredContext,
// 				 __in_opt PVOID  SystemArgument1,
// 				 __in_opt PVOID  SystemArgument2
// 				 )
// {
// 	ULONG _DR0 = 0, _DR1 = 0, _DR2 = 0, _DR3 = 0, _DR6 = 0, _DR7 = 0;
// 	PDEBUG_REGISTER_INFO pDebugRegisterInfo = (PDEBUG_REGISTER_INFO)DeferredContext;
// 	KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
// 	LONG n = InterlockedCompareExchange( &pDebugRegisterInfo->nCnt, 0, 0 );
// 
// 	__asm{
// 		push eax
// 		mov eax, dr0
// 		mov _DR0, eax
// 		mov eax, dr1
// 		mov _DR1, eax
// 		mov eax, dr2
// 		mov _DR2, eax
// 		mov eax, dr3
// 		mov _DR3, eax
// 		mov eax, dr6
// 		mov _DR6, eax
// 		mov eax, dr7
// 		mov _DR7, eax
// 		pop eax
// 	}
// 	
// 	pDebugRegisterInfo->DebugRegister[n].DR0 = _DR0;
// 	pDebugRegisterInfo->DebugRegister[n].DR1 = _DR1;
// 	pDebugRegisterInfo->DebugRegister[n].DR2 = _DR2;
// 	pDebugRegisterInfo->DebugRegister[n].DR3 = _DR3;
// 	pDebugRegisterInfo->DebugRegister[n].DR6 = _DR6;
// 	pDebugRegisterInfo->DebugRegister[n].DR7 = _DR7;
// 	
// 	InterlockedIncrement( &pDebugRegisterInfo->nCnt );
// 	InterlockedIncrement( &g_nCpuCnt );
// 	KeLowerIrql(OldIrql);
// }
// 

// void GetAllCpuDebugRegister(PDEBUG_REGISTER_INFO pDebugRegisterInfo)
// {
// 	ULONG i = 0;
// 	KAFFINITY CpuAffinity;
// 	ULONG ulCpuCnt = 0;
// 	
// 	if (!pDebugRegisterInfo)
// 	{
// 		return;
// 	}
// 	
// 	KdPrint(("pDebugRegisterInfo: 0x%08X\n", pDebugRegisterInfo));
// 
// 	g_nCpuCnt = 0;
// 	CpuAffinity = KeQueryActiveProcessors();
// 
// 	for (i = 0; i < 32; i++)
// 	{
// 		if ( (CpuAffinity >> i) & 1 )
// 		{
// 			ulCpuCnt++;
// 		}
// 	}
// 
// 	if ( ulCpuCnt == 1 ) 
// 	{
// 		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
// 		ULONG _DR0 = 0, _DR1 = 0, _DR2 = 0, _DR3 = 0, _DR6 = 0, _DR7 = 0;
// 
// 		__asm{
// 			push eax
// 			mov eax, dr0
// 			mov _DR0, eax
// 			mov eax, dr1
// 			mov _DR1, eax
// 			mov eax, dr2
// 			mov _DR2, eax
// 			mov eax, dr3
// 			mov _DR3, eax
// 			mov eax, dr6
// 			mov _DR6, eax
// 			mov eax, dr7
// 			mov _DR7, eax
// 			pop eax
// 		}
// 		
// 		pDebugRegisterInfo->DebugRegister[pDebugRegisterInfo->nCnt].DR0 = _DR0;
// 		pDebugRegisterInfo->DebugRegister[pDebugRegisterInfo->nCnt].DR1 = _DR1;
// 		pDebugRegisterInfo->DebugRegister[pDebugRegisterInfo->nCnt].DR2 = _DR2;
// 		pDebugRegisterInfo->DebugRegister[pDebugRegisterInfo->nCnt].DR3 = _DR3;
// 		pDebugRegisterInfo->DebugRegister[pDebugRegisterInfo->nCnt].DR6 = _DR6;
// 		pDebugRegisterInfo->DebugRegister[pDebugRegisterInfo->nCnt].DR7 = _DR7;
// 		
// 		InterlockedIncrement( &g_nCpuCnt );
// 		InterlockedIncrement(&pDebugRegisterInfo->nCnt);
// 		KeLowerIrql(OldIrql);
// 	}
// 	else  
// 	{
// 		ULONG  ulCurrentCpu = KeGetCurrentProcessorNumber();
// 
// 		CpuAffinity = KeQueryActiveProcessors();
// 
// 		for (i = 0; i < 32; i++) 
// 		{
// 			if ( (CpuAffinity >> i) & 1 )  
// 			{
// 				if (ulCurrentCpu != i)
// 				{
// 					KDPC Dpc;
// 					KeInitializeDpc(&Dpc, WaitForGetDebugRegister, pDebugRegisterInfo);
// 					KeSetTargetProcessorDpc(&Dpc, (CCHAR)i);
// 					KeSetImportanceDpc(&Dpc, HighImportance);
// 					KeInsertQueueDpc(&Dpc, NULL, NULL);
// 				}
// 				else
// 				{
// 					KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
// 					ULONG _DR0 = 0, _DR1 = 0, _DR2 = 0, _DR3 = 0, _DR6 = 0, _DR7 = 0;
// 					LONG n = InterlockedCompareExchange( &pDebugRegisterInfo->nCnt, 0, 0 );
// 
// 					__asm{
// 						push eax
// 						mov eax, dr0
// 						mov _DR0, eax
// 						mov eax, dr1
// 						mov _DR1, eax
// 						mov eax, dr2
// 						mov _DR2, eax
// 						mov eax, dr3
// 						mov _DR3, eax
// 						mov eax, dr6
// 						mov _DR6, eax
// 						mov eax, dr7
// 						mov _DR7, eax
// 						pop eax
// 					}
// 
// 					pDebugRegisterInfo->DebugRegister[n].DR0 = _DR0;
// 					pDebugRegisterInfo->DebugRegister[n].DR1 = _DR1;
// 					pDebugRegisterInfo->DebugRegister[n].DR2 = _DR2;
// 					pDebugRegisterInfo->DebugRegister[n].DR3 = _DR3;
// 					pDebugRegisterInfo->DebugRegister[n].DR6 = _DR6;
// 					pDebugRegisterInfo->DebugRegister[n].DR7 = _DR7;
// 
// 					InterlockedIncrement( &g_nCpuCnt );
// 					InterlockedIncrement(&pDebugRegisterInfo->nCnt);
// 					KeLowerIrql(OldIrql);
// 				}
// 			}
// 		}
// 	}
// 
// 	while (1)
// 	{
// 		if (g_nCpuCnt == ulCpuCnt)
// 		{
// 			break;
// 		}
// 
// 		WaitMicroSecond(100);
// 	}
// }

// NTSTATUS EnumDebugRegisters(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
// {
// 	NTSTATUS status = STATUS_SUCCESS;
// 	PDEBUG_REGISTER_INFO pDebugRegisterInfo = (PDEBUG_REGISTER_INFO)pOutBuffer;
// 	ULONG i = 0;
// 	PDEBUG_REGISTER_INFO pTemp = ExAllocatePoolWithTag(PagedPool, uOutSize, MZFTAG);
// 
// 	if (pTemp)
// 	{
// 		memset(pTemp, 0, uOutSize);
// 		GetAllCpuDebugRegister(pTemp);
// 		
// 		memcpy(pDebugRegisterInfo, pTemp, uOutSize);
// 
// 		for (i = 0; i < pDebugRegisterInfo->nCnt; i++)
// 		{
// 			KdPrint(("DR0: 0x%08X, DR1: 0x%08X, DR2: 0x%08X, DR3: 0x%08X, DR6: 0x%08X, DR7: 0x%08X",
// 				pDebugRegisterInfo->DebugRegister[i].DR0,
// 				pDebugRegisterInfo->DebugRegister[i].DR1,
// 				pDebugRegisterInfo->DebugRegister[i].DR2,
// 				pDebugRegisterInfo->DebugRegister[i].DR3,
// 				pDebugRegisterInfo->DebugRegister[i].DR6,
// 				pDebugRegisterInfo->DebugRegister[i].DR7));
// 		}
// 
// 		ExFreePool(pTemp);
// 	}
// 	
// 	return status;
// }

void GetDebugRegister(PDEBUG_REGISTER pDebugRegisterInfo)
{
	KIRQL OldIrql;
	ULONG _DR0 = 0, _DR1 = 0, _DR2 = 0, _DR3 = 0, _DR6 = 0, _DR7 = 0;

	if (!pDebugRegisterInfo)
	{
		return;
	}

	OldIrql = KeRaiseIrqlToDpcLevel();
	
	__asm{
		push eax
		mov eax, dr0
		mov _DR0, eax
		mov eax, dr1
		mov _DR1, eax
		mov eax, dr2
		mov _DR2, eax
		mov eax, dr3
		mov _DR3, eax
		mov eax, dr6
		mov _DR6, eax
		mov eax, dr7
		mov _DR7, eax
		pop eax
	}

	pDebugRegisterInfo->DR0 = _DR0;
	pDebugRegisterInfo->DR1 = _DR1;
	pDebugRegisterInfo->DR2 = _DR2;
	pDebugRegisterInfo->DR3 = _DR3;
	pDebugRegisterInfo->DR6 = _DR6;
	pDebugRegisterInfo->DR7 = _DR7;

	KeLowerIrql(OldIrql);
}

NTSTATUS EnumDebugRegisters(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDEBUG_REGISTER pDebugRegisterInfo = (PDEBUG_REGISTER)pOutBuffer;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	
	if (MzfExAllocatePoolWithTag && MzfExFreePoolWithTag)
	{
		PDEBUG_REGISTER pTemp = MzfExAllocatePoolWithTag(PagedPool, uOutSize, MZFTAG);
		if (pTemp)
		{
			memset(pTemp, 0, uOutSize);
			GetDebugRegister(pTemp);
			memcpy(pDebugRegisterInfo, pTemp, uOutSize);
			MzfExFreePoolWithTag(pTemp, 0);
			pTemp = NULL;

			status = STATUS_SUCCESS;
		}
	}

	return status;
}

VOID WaitMicroSecond(LONG MicroSeconds)
{
	LARGE_INTEGER waitTime;
	waitTime.QuadPart = -10 * MicroSeconds;
	KeDelayExecutionThread(Executive, KernelMode, &waitTime);
}

VOID DpcProc(
			 __in struct _KDPC  *Dpc,
			 __in_opt PVOID  DeferredContext,
			 __in_opt PVOID  SystemArgument1,
			 __in_opt PVOID  SystemArgument2
			 )
{
	KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
	DRX_REG eDRX = *(DRX_REG *)DeferredContext;
	DWORD dwDR7 = 0;

	__asm{
		push eax
		mov eax, dr7
		mov dwDR7, eax
		pop eax
	}

	if (dwDR7 > 0)
	{
		switch (eDRX)
		{
		case enumDR0:
			{
				dwDR7 &= 0xFFF0FFFC;
				__asm
				{
					push eax
					mov eax, 0
					mov dr0, eax
					pop eax
				}
			}
			break;

		case enumDR1:
			{
				dwDR7 &= 0xFF0FFFF3;
				__asm
				{
					push eax
					mov eax, 0
					mov dr1, eax
					pop eax
				}
			}
			break;

		case enumDR2:
			{
				dwDR7 &= 0xF0FFFFCF;
				__asm
				{
					push eax
					mov eax, 0
					mov dr2, eax
					pop eax
				}
			}
			break;

		case enumDR3:
			{
				dwDR7 &= 0x0FFFFF3F;
				__asm
				{
					push eax
					mov eax, 0
					mov dr3, eax
					pop eax
				}
			}
			break;

		case enumDRAll:
			{
				dwDR7 &= 0x0000FF00;
				__asm
				{
					push eax
					mov eax, 0
					mov dr0, eax
					mov dr1, eax
					mov dr2, eax
					mov dr3, eax
					pop eax
				}
			}
			break;
		}
	}

	__asm{
		push eax
		mov eax, dwDR7
		mov dr7, eax
		pop eax
	}

	InterlockedIncrement( &g_nCpuCnt );
	KeLowerIrql(OldIrql);
}

void SetDpcToRecoverRegisters(DRX_REG eDRX)
{
	ULONG i = 0;
	KAFFINITY CpuAffinity = KeQueryActiveProcessors();

	for (i = 0; i < 32; i++) 
	{
		if ( (CpuAffinity >> i) & 1 )  
		{
			KDPC Dpc;
			KeInitializeDpc(&Dpc, DpcProc, (PVOID)&eDRX);
			KeSetTargetProcessorDpc(&Dpc, (CCHAR)i);
			KeSetImportanceDpc(&Dpc, HighImportance);
			KeInsertQueueDpc(&Dpc, NULL, NULL);
		}
	}

	while (1)
	{
		if (KeNumberProcessors == InterlockedCompareExchange( &g_nCpuCnt, 0, KeNumberProcessors ) )
		{
			break;
		}

		WaitMicroSecond(100);
	}
}

// »Ö¸´µ÷ÊÔ¼Ä´æÆ÷
NTSTATUS RecoverRegisters(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	RECOVER_DEBUG_REGISTER rer = *(RECOVER_DEBUG_REGISTER *)pInBuffer;
	DRX_REG eDRX = rer.RegType;

	KdPrint(("RecoverRegisters start\n"));
	g_nCpuCnt = 0;
	SetDpcToRecoverRegisters(eDRX);
	KdPrint(("RecoverRegisters ok\n"));
	return status;
}