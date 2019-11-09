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
// /********************************************************************
// 	created:	2012/06/05	16:40
// 	file:		HookPort.c
// 	author:		莫灰灰
// 	blog：		http://blog.csdn.net/hu3167343
// 
// 	purpose:	HOOK KiFastCallEntry，记录下各个当前调用进程的EPROCESS，主要用来枚举隐藏进程。
// *********************************************************************/
// 
// #include "HookPort.h"
// #include "Struct.h"
// #include "CommonFunction.h"
// #include "disasm.h"
// #include "Process.h"
// #include "InlineHook.h"
// 
// HOOK_ENVIRONMENT g_heKiFastCallEntry;
// 
// pfnNtDisplayString g_pOriginNtDisplayString = NULL;
// PVOID g_pKiFastCallEntryRetAddress = NULL;
// ULONG g_KiFastCallEntry = 0;
// 
// /////////////////////////////////////////////////////////////////////////////
// 
// NTSTATUS
// FakeNtDisplayString(
// 					PUNICODE_STRING DisplayString
// 					)
// 
// /*++
// 
// Routine Description:
// 
//     NtDisplayString的Fake函数，主要用来回溯到上一层函数调用的返回地址。
// 
// Arguments:
// 
//     DisplayString - 待显示的UNICODE字符串
// 
// Return Value:
// 
//     如果是我们自己的调用，返回0xBAD12345。
// 	如果不是我们的调用，那么调用原始函数。
//     
// --*/
// 
// {
// 	KPROCESSOR_MODE preMode = ExGetPreviousMode();
// 	WCHAR *szString = NULL;
// 
// 	if (DisplayString && (DisplayString->Length >= 3 * sizeof(WCHAR)) && preMode == KernelMode)
// 	{
// 		KdPrint(("FakeNtDisplayString: %wZ\n", DisplayString));
// 			
// 		szString = DisplayString->Buffer;
// 		if (szString[0] == L'm' &&
// 			szString[1] == L'Z' &&
// 			szString[2] == L'f')
// 		{
// 			KdPrint(("is NtDisplayString my call\n"));
// 
// 			__asm{
// 				push eax
// 				mov eax, ebp
// 				mov eax, [eax+4]
// 				mov g_pKiFastCallEntryRetAddress, eax 
// 				pop eax
// 			}
// 
// 			return 0xBAD12345;
// 		}
// 	}
// 	
// 	return g_pOriginNtDisplayString(DisplayString);
// }
// 
// /////////////////////////////////////////////////////////////////////////////
// 
// NTSTATUS 
// GetKiFastCallEntryRetAddress()
// 
// /*++
// 
// Routine Description:
// 
//     SSDT Hook NtDisplayString函数，在Fake函数中判断是否是我们的函数，然后堆栈回溯得到上一层函数的返回地址。
// 
// Arguments:
// 
//     None.
// 
// Return Value:
// 
//     STATUS_SUCCESS - 获取上一层函数返回地址成功
// 	STATUS_UNSUCCESSFUL - 获取上一层函数返回地址失败
//     
// --*/
// 
// {
// 	NTSTATUS status = STATUS_UNSUCCESSFUL;
// 	PBYTE pZwDisplayString = (PBYTE)GetFunctionAddressByName(L"ZwDisplayString");
// 	DWORD dwNtDisplayStringId = 0;
// 	WCHAR mzf[] = {L'm', L'Z', L'f'};
// 	UNICODE_STRING unMzf;
// 
// 	RtlInitUnicodeString(&unMzf, mzf);
// 
// 	if (pZwDisplayString)
// 	{
// 		if (*(PBYTE)pZwDisplayString == 0xB8)
// 		{
// 			dwNtDisplayStringId = *(PDWORD)((PBYTE)pZwDisplayString + 1);
// 
// 			if (dwNtDisplayStringId > 0 && dwNtDisplayStringId < 1000)
// 			{
// 				int i = 0;
// 				for (i = 0; i < 3; i++)
// 				{
// 					WPOFF();
// 					g_pOriginNtDisplayString = (pfnNtDisplayString)InterlockedExchange((PULONG)&KeServiceDescriptorTable.ServiceTableBase[dwNtDisplayStringId], (ULONG)FakeNtDisplayString);
// 					WPON();
// 
// 					if (g_pOriginNtDisplayString)
// 					{
// 						status = ZwDisplayString(&unMzf);
// 						if (status == 0xBAD12345 && (ULONG)g_pKiFastCallEntryRetAddress > (ULONG)MmSystemRangeStart)
// 						{
// 							KdPrint(("g_pKiFastCallEntryRetAddress: 0x%08X\n", g_pKiFastCallEntryRetAddress));
// 							status = STATUS_SUCCESS;
// 						}
// 						else
// 						{
// 							status = STATUS_UNSUCCESSFUL;
// 						}
// 					}
// 
// 					WPOFF();
// 					InterlockedExchange((PULONG)&KeServiceDescriptorTable.ServiceTableBase[dwNtDisplayStringId], (ULONG)g_pOriginNtDisplayString);
// 					WPON();
// 
// 					if (NT_SUCCESS(status))
// 					{
// 						break;
// 					}
// 				}
// 			}
// 		}
// 	}
// 
// 	return status;
// }
// 
// /////////////////////////////////////////////////////////////////////////////
// 
// PVOID  
// GetKiFastCallEntry()
// {
// 	PVOID pOriginKiFastCallEntry = NULL;
// 
// 	__asm{
// 		pushad
// 		mov ecx, 0x176
// 		rdmsr
// 		mov pOriginKiFastCallEntry, eax
// 		popad
// 	}
// 
// 	if (pOriginKiFastCallEntry && (ULONG)pOriginKiFastCallEntry > (ULONG)MmSystemRangeStart)
// 	{
// 		return pOriginKiFastCallEntry;
// 	}
// 
// 	return NULL;
// }
// 
// /////////////////////////////////////////////////////////////////////////////
// 
// VOID
// ProcessFilter()
// {
// 	PETHREAD thread = PsGetCurrentThread();
// 	ThreadToProcessAndInsertProcess(thread, enumTempList);
// }
// 
// /////////////////////////////////////////////////////////////////////////////
// 
// __declspec(naked) 
// FakeKiFastCallEntryHead()
// {
// 	__asm{
// 		pushad
// 		pushfd
// 		push fs
// 		push 0x30
// 		pop fs
// 
// 		call ProcessFilter
// 
// 		pop fs
// 		popfd
// 		popad
// 
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 		_emit 0x90
// 	}
// }
// 
// /////////////////////////////////////////////////////////////////////////////
// 
// BOOL 
// InlieHookKiFastCallEntry()
// {
// 	PVOID KiFastCallEntry = GetKiFastCallEntry();
// 	BOOL bRet = FALSE;
// 
// 	if (KiFastCallEntry)
// 	{
// 		memset(&g_heKiFastCallEntry, 0, sizeof(HOOK_ENVIRONMENT));
// 
// 		g_heKiFastCallEntry.HookAddress = FakeKiFastCallEntryHead;
// 		g_heKiFastCallEntry.OriginAddress = KiFastCallEntry ;
// 
// 		bRet = SafeHookOrUnhook(&g_heKiFastCallEntry, enumHookNotExport);
//  	}
// 
// 	return bRet;
// }
// 
// BOOL 
// UnInlieHookKiFastCallEntry()
// {
// 	return SafeHookOrUnhook(&g_heKiFastCallEntry, enumUnHook);;
// }
// 
// /////////////////////////////////////////////////////////////////////////////
// 
// __declspec(naked) 
// FakeKiFastCallEntry()
// {
// 	__asm
// 	{
// 		pushad
// 		pushfd
// 		push fs
// 		push 0x30
// 		pop fs
// 		call ProcessFilter
// 		pop fs
// 		popfd
// 		popad
// 		jmp g_KiFastCallEntry
// 	}
// }
// 
// NTSTATUS HookKiFastCallEntry()
// {
// 	NTSTATUS status = STATUS_UNSUCCESSFUL;
// 
// 	__asm {
// 		pushad
// 		mov ecx, 0x176
// 		rdmsr                          
// 		mov g_KiFastCallEntry, eax
// 		mov eax, FakeKiFastCallEntry     
// 		wrmsr 
// 		popad
// 	}
// 
// 	return status;
// }