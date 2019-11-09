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
#include "Acpi.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Peload.h"
#include "libdasm.h"
#include "DispatchHook.h"

void IninAcpi()
{
	ULONG Base = GetGlobalVeriable(enumAcpiBase);
	ULONG Size = GetGlobalVeriable(enumAcpiSize);
	WCHAR szDriver[] = {'\\','D','r','i','v','e','r','\\','\0'};
	WCHAR szDriverDir[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'};
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("Acpi -> Base: 0x%08X, Size: 0x%X\n", Base, Size));
	if (Base && Size && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		WCHAR szFilePath[MAX_PATH] = {0};
		WCHAR szSysName[] = {'a','c','p','i','.','s','y','s','\0'};
		WCHAR szDriverName[] = {'a','c','p','i','\0'};
		ULONG NewBase = 0;
		WCHAR szDriverPath[MAX_PATH] = {0};
		PDRIVER_OBJECT pDriverObject = NULL;

		wcscpy(szDriverPath, szDriver);
		wcscat(szDriverPath, szDriverName);

		if (NT_SUCCESS(GetDriverObjectByName(szDriverPath, &pDriverObject)))
		{
			KdPrint(("Acpi Driverobject: 0x%08X\n", pDriverObject));
			SetGlobalVeriable(enumAcpiDriverObject, (ULONG)pDriverObject);
		}
		else
		{
			KdPrint(("Get Acpi driverobject error\n"));
		}

		wcscpy(szFilePath, szDriverDir);
		wcscat(szFilePath, szSysName);

		if (PeLoad(szFilePath, &NewBase, Base))
		{
			ULONG Offset = 0;
			PULONG pOriginDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);
			PULONG pReloadDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);

			if (pOriginDispatchs && pReloadDispatchs)
			{
				ULONG pDriverEntry = 0;
				ULONG nCodeLen = 0, Address = 0;
				ULONG MovTemp = 0;
				ENTRY_POINT_TYPE EntryType = enumEntryNone;
				WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

				memset(pOriginDispatchs, 0, sizeof(ULONG) * 28);
				memset(pReloadDispatchs, 0, sizeof(ULONG) * 28);

				if (Version == enumWINDOWS_2K)
				{
					EntryType = enumEntryPointHead;
				}
				else 
				{
					EntryType = enumEntryPointJmp;
				}

				if (!GetDriverEntryPoint((PVOID)NewBase, &pDriverEntry, EntryType))
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
						if (Inst.op1.type == OPERAND_TYPE_REGISTER && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
							MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
						{
							MovTemp = Inst.op2.immediate;
						}
					}
					else if (Inst.type == INSTRUCTION_TYPE_STOS)
					{
						if (MovTemp && MzfMmIsAddressValid((PVOID)MovTemp))
						{
							ULONG MovTempReload = MovTemp - Base + NewBase;
							ULONG i = 0;

							for (i = 0; i < 28; i++)
							{
								pReloadDispatchs[i] = MovTempReload;
								pOriginDispatchs[i] = MovTemp;
							}
						}

						break;
					}

					if (Inst.length == 0)
					{
						break;
					}
					else if (Inst.type == INSTRUCTION_TYPE_RET)
					{
						break;
					}

					nCodeLen = Inst.length;
				}

				FixOriginDispatch(pOriginDispatchs);
				FixReloadDispatch(pReloadDispatchs);

				SetGlobalVeriable(enumAcpiOriginalDispatchs, (ULONG)pOriginDispatchs);
				SetGlobalVeriable(enumAcpiReloadDispatchs, (ULONG)pReloadDispatchs);
			}

			SetGlobalVeriable(enumReloadAcpiBase, NewBase);
		}
	}
}

NTSTATUS GetAcpiDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDISPATCH_HOOK_INFO pHookInfo = (PDISPATCH_HOOK_INFO)pOutBuffer;
	PULONG pReloadDispatch = (PULONG)GetGlobalVeriable(enumAcpiReloadDispatchs);
	PULONG pOriginDispatch = (PULONG)GetGlobalVeriable(enumAcpiOriginalDispatchs);
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAcpiDriverObject);

	if (!pReloadDispatch || 
		!pDriverObject || 
		!pOriginDispatch)
	{
		IninAcpi();
		pReloadDispatch = (PULONG)GetGlobalVeriable(enumAcpiReloadDispatchs);
		pOriginDispatch = (PULONG)GetGlobalVeriable(enumAcpiOriginalDispatchs);
		pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAcpiDriverObject);
	}

	KdPrint(("pReload: 0x%08X,  pDriverObject: 0x%08X, pOrigin: 0x%08X\n", 
		pReloadDispatch,
		pDriverObject,
		pOriginDispatch));

	if (pReloadDispatch && 
		pDriverObject && 
		pOriginDispatch)
	{
		GetDispatchHook( pHookInfo, pDriverObject, pOriginDispatch, pReloadDispatch );
	}

	return status;
}
