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
#include "classpnp.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Peload.h"
#include "libdasm.h"
#include "DispatchHook.h"

void IninClasspnp()
{
	ULONG ClasspnpBase = GetGlobalVeriable(enumClasspnpBase);
	ULONG ClasspnpSize = GetGlobalVeriable(enumClasspnpSize);
	WCHAR szDriver[] = {'\\','D','r','i','v','e','r','\\','\0'};
	WCHAR szDriverDir[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'};
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("ClasspnpBase: 0x%08X, ClasspnpSize: 0x%X\n", ClasspnpBase, ClasspnpSize));
	if (ClasspnpBase && ClasspnpSize && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		WCHAR szClasspnpPath[MAX_PATH] = {0};
		WCHAR szClasspnpSys[] = {'c','l','a','s','s','p','n','p','.','s','y','s','\0'};
		WCHAR szClasspnp[] = {'D','i','s','k','\0'};
		ULONG NewBase = 0;
		WCHAR szDrivrtClasspnp[MAX_PATH] = {0};
		PDRIVER_OBJECT pDriverObject = NULL;

		wcscpy(szDrivrtClasspnp, szDriver);
		wcscat(szDrivrtClasspnp, szClasspnp);

		if (NT_SUCCESS(GetDriverObjectByName(szDrivrtClasspnp, &pDriverObject)))
		{
			KdPrint(("Classpnp Driverobject: 0x%08X\n", pDriverObject));
			SetGlobalVeriable(enumClasspnpDriverObject, (ULONG)pDriverObject);
		}
		else
		{
			KdPrint(("Get Classpnp driverobject error\n"));
		}

		wcscpy(szClasspnpPath, szDriverDir);
		wcscat(szClasspnpPath, szClasspnpSys);

		if (PeLoad(szClasspnpPath, &NewBase, ClasspnpBase))
		{
			ULONG Offset = 0;
			PULONG pOriginDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);
			PULONG pReloadDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);

			if (pOriginDispatchs && pReloadDispatchs)
			{
				PVOID ClassInitialize = NULL;
				CHAR szClassInitialize[] = {'C','l','a','s','s','I','n','i','t','i','a','l','i','z','e','\0'};
				ULONG nCodeLen = 0, Address = 0;
				ULONG MovTemp = 0;

				memset(pOriginDispatchs, 0, sizeof(ULONG) * 28);
				memset(pReloadDispatchs, 0, sizeof(ULONG) * 28);
				
				ClassInitialize = FindExportedRoutineByName((PVOID)NewBase, szClassInitialize);

				if (!ClassInitialize)
				{
					KdPrint(("FindExportedRoutineByName error\n"));
					MzfExFreePoolWithTag(pOriginDispatchs, 0);
					MzfExFreePoolWithTag(pReloadDispatchs, 0);
					MzfExFreePoolWithTag((PVOID)NewBase, 0);
					return;
				}

				for (Address = (ULONG)ClassInitialize; Address < (ULONG)ClassInitialize + PAGE_SIZE; Address += nCodeLen)
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
							pReloadDispatchs[nIndex] = Inst.op2.immediate - ClasspnpBase + NewBase;
						}
						else if (Inst.op1.type == OPERAND_TYPE_REGISTER && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
							MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
						{
							MovTemp = Inst.op2.immediate;
						}
						else if (Inst.op1.type == OPERAND_TYPE_MEMORY && 
							Inst.op2.type == OPERAND_TYPE_REGISTER && 
							Inst.op1.displacement >= 0x38 &&
							Inst.op1.displacement <= 0xA4 &&
							MovTemp &&
							MzfMmIsAddressValid((PVOID)MovTemp))
						{
							ULONG nIndex = (Inst.op1.displacement - 0x38) / sizeof(ULONG);
							pOriginDispatchs[nIndex] = MovTemp;
							pReloadDispatchs[nIndex] = MovTemp - ClasspnpBase + NewBase;
						}
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

				SetGlobalVeriable(enumClasspnpOriginalDispatchs, (ULONG)pOriginDispatchs);
				SetGlobalVeriable(enumClasspnpReloadDispatchs, (ULONG)pReloadDispatchs);
			}

			SetGlobalVeriable(enumReloadClasspnpBase, NewBase);
		}
	}
}

NTSTATUS GetClasspnpDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDISPATCH_HOOK_INFO pClasspnpHook = (PDISPATCH_HOOK_INFO)pOutBuffer;
	PULONG pReloadClasspnpDispatch = (PULONG)GetGlobalVeriable(enumClasspnpReloadDispatchs);
	PULONG pOriginClasspnpDispatch = (PULONG)GetGlobalVeriable(enumClasspnpOriginalDispatchs);
	PDRIVER_OBJECT pClasspnpDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumClasspnpDriverObject);

	if (!pReloadClasspnpDispatch || 
		!pClasspnpDriverObject || 
		!pOriginClasspnpDispatch)
	{
		IninClasspnp();
		pReloadClasspnpDispatch = (PULONG)GetGlobalVeriable(enumClasspnpReloadDispatchs);
		pOriginClasspnpDispatch = (PULONG)GetGlobalVeriable(enumClasspnpOriginalDispatchs);
		pClasspnpDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumClasspnpDriverObject);
	}

	KdPrint(("pReload: 0x%08X,  pDriverObject: 0x%08X, pOrigin: 0x%08X\n", 
		pReloadClasspnpDispatch,
		pClasspnpDriverObject,
		pOriginClasspnpDispatch));

	if (pReloadClasspnpDispatch && 
		pClasspnpDriverObject && 
		pOriginClasspnpDispatch)
	{
		GetDispatchHook( pClasspnpHook, pClasspnpDriverObject, pOriginClasspnpDispatch, pReloadClasspnpDispatch );
	}

	return status;
}