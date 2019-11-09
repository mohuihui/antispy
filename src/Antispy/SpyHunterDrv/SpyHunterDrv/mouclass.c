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
#include "mouclass.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Peload.h"
#include "libdasm.h"
#include "DispatchHook.h"

void IninMouClass()
{
	ULONG MouclassBase = GetGlobalVeriable(enumMouclassBase);
	ULONG MouclassSize = GetGlobalVeriable(enumMouclassSize);
	WCHAR szDriver[] = {'\\','D','r','i','v','e','r','\\','\0'};
	WCHAR szDriverDir[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'};
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("MouclassBase: 0x%08X, MouclassSize: 0x%X\n", MouclassBase, MouclassSize));
	if (MouclassBase && MouclassSize && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		WCHAR szMouclassPath[MAX_PATH] = {0};
		WCHAR szMouclassSys[] = {'m','o','u','c','l','a','s','s','.','s','y','s','\0'};
		WCHAR szMouclass[] = {'M','o','u','c','l','a','s','s','\0'};
		ULONG NewBase = 0;
		WCHAR szDrivrtMouclass[MAX_PATH] = {0};
		PDRIVER_OBJECT pDriverObject = NULL;

		wcscpy(szDrivrtMouclass, szDriver);
		wcscat(szDrivrtMouclass, szMouclass);

		if (NT_SUCCESS(GetDriverObjectByName(szDrivrtMouclass, &pDriverObject)))
		{
			KdPrint(("Mouclass Driverobject: 0x%08X\n", pDriverObject));
			SetGlobalVeriable(enumMouClassDriverObject, (ULONG)pDriverObject);
		}
		else
		{
			KdPrint(("Get Kbdclass driverobject error\n"));
		}

		wcscpy(szMouclassPath, szDriverDir);
		wcscat(szMouclassPath, szMouclassSys);

		if (PeLoad(szMouclassPath, &NewBase, MouclassBase))
		{
			ULONG Offset = 0;
			PULONG pOriginDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);
			PULONG pReloadDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);

			if (pOriginDispatchs && pReloadDispatchs)
			{
				ULONG pDriverEntry = 0;
				ULONG nCodeLen = 0, Address = 0;
				ULONG MovTemp = 0;

				memset(pOriginDispatchs, 0, sizeof(ULONG) * 28);
				memset(pReloadDispatchs, 0, sizeof(ULONG) * 28);

				if (!GetDriverEntryPoint((PVOID)NewBase, &pDriverEntry, enumEntryPointJmp))
				{
					KdPrint(("GetDriverEntryPoint error\n"));
					MzfExFreePoolWithTag(pOriginDispatchs, 0);
					MzfExFreePoolWithTag(pReloadDispatchs, 0);
					MzfExFreePoolWithTag((PVOID)NewBase, 0);
					return;
				}

				for (Address = pDriverEntry; Address < pDriverEntry + PAGE_SIZE; Address += nCodeLen)
				{
					INSTRUCTION	Inst;

					if (!MzfMmIsAddressValid((PVOID)Address))
					{
						break;
					}

					memset(&Inst, 0, sizeof(INSTRUCTION));
					get_instruction(&Inst, (PBYTE)Address, MODE_32);

					if (Inst.type == INSTRUCTION_TYPE_MOV)
					{
						if (Inst.op1.type == OPERAND_TYPE_MEMORY && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
							Inst.op1.displacement >= 0x38 &&
							Inst.op1.displacement <= 0xA4)
						{
							ULONG nIndex = (Inst.op1.displacement - 0x38) / sizeof(ULONG);
							pOriginDispatchs[nIndex] = Inst.op2.immediate;
							pReloadDispatchs[nIndex] = Inst.op2.immediate - MouclassBase + NewBase;
						}
					}

					if (Inst.length == 0 || Inst.type == INSTRUCTION_TYPE_INT)
					{
						break;
					}
// 					else if (Inst.type == INSTRUCTION_TYPE_RET)
// 					{
// 						break;
// 					}

					nCodeLen = Inst.length;
				}

				FixOriginDispatch(pOriginDispatchs);
				FixReloadDispatch(pReloadDispatchs);

				SetGlobalVeriable(enumMouClassOriginalDispatchs, (ULONG)pOriginDispatchs);
				SetGlobalVeriable(enumMouClassReloadDispatchs, (ULONG)pReloadDispatchs);
			}

			SetGlobalVeriable(enumReloadMouClassBase, NewBase);
		}
	}
}

NTSTATUS GetMouClassDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDISPATCH_HOOK_INFO pFsdHook = (PDISPATCH_HOOK_INFO)pOutBuffer;
	PULONG pReloadKbdClassDispatch = (PULONG)GetGlobalVeriable(enumMouClassReloadDispatchs);
	PULONG pOriginKbdClassDispatch = (PULONG)GetGlobalVeriable(enumMouClassOriginalDispatchs);
	PDRIVER_OBJECT pKbdClassDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumMouClassDriverObject);

	if (!pReloadKbdClassDispatch || 
		!pKbdClassDriverObject || 
		!pOriginKbdClassDispatch)
	{
		IninMouClass();
		pReloadKbdClassDispatch = (PULONG)GetGlobalVeriable(enumMouClassReloadDispatchs);
		pOriginKbdClassDispatch = (PULONG)GetGlobalVeriable(enumMouClassOriginalDispatchs);
		pKbdClassDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumMouClassDriverObject);
	}

	KdPrint(("pReload: 0x%08X,  pDriverObject: 0x%08X, pOriginKbdClassDispatch: 0x%08X\n", 
		pReloadKbdClassDispatch,
		pKbdClassDriverObject,
		pOriginKbdClassDispatch));

	if (pReloadKbdClassDispatch && 
		pKbdClassDriverObject && 
		pOriginKbdClassDispatch)
	{
		GetDispatchHook( pFsdHook, pKbdClassDriverObject, pOriginKbdClassDispatch, pReloadKbdClassDispatch );
	}

	return status;
}
