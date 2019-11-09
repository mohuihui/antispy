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
#include "Atapi.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Peload.h"
#include "libdasm.h"
#include "DispatchHook.h"

void IninAtapi()
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Base = 0, Size = 0;
	WCHAR szDriver[] = {'\\','D','r','i','v','e','r','\\','\0'};
	WCHAR szDriverDir[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'};
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (BuildNumber >= 6000)
	{
		Base = GetGlobalVeriable(enumAtaPortBase);
		Size = GetGlobalVeriable(enumAtaPortSize);
	}
	else
	{
		Base = GetGlobalVeriable(enumAtapiBase);
		Size = GetGlobalVeriable(enumAtapiSize);
	}

	KdPrint(("Atapi -> Base: 0x%08X, Size: 0x%X\n", Base, Size));
	if (Base && Size && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		WCHAR szFilePath[MAX_PATH] = {0};
		WCHAR szSysNameXp[] = {'a','t','a','p','i','.','s','y','s','\0'};
		WCHAR szSysNameVista[] = {'a','t','a','p','o','r','t','.','s','y','s','\0'};
		WCHAR *szSysName = NULL;
		WCHAR szDriverName[] = {'a','t','a','p','i','\0'};
		ULONG NewBase = 0;
		WCHAR szDriverPath[MAX_PATH] = {0};
		PDRIVER_OBJECT pDriverObject = NULL;
		
		wcscpy(szDriverPath, szDriver);
		wcscat(szDriverPath, szDriverName);

		if (NT_SUCCESS(GetDriverObjectByName(szDriverPath, &pDriverObject)))
		{
			KdPrint(("Atapi Driverobject: 0x%08X\n", pDriverObject));
			SetGlobalVeriable(enumAtapiDriverObject, (ULONG)pDriverObject);
		}
		else
		{
			KdPrint(("Get Atapi driverobject error\n"));
		}
		
		if (BuildNumber >= 6000)
		{
			szSysName = szSysNameVista;
		}
		else
		{
			szSysName = szSysNameXp;
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
			
				memset(pOriginDispatchs, 0, sizeof(ULONG) * 28);
				memset(pReloadDispatchs, 0, sizeof(ULONG) * 28);
				
				if (BuildNumber >= 6000)
				{
					PVOID AtaPortInitialize = NULL;
					CHAR szAtaPortInitialize[] = {'A','t','a','P','o','r','t','I','n','i','t','i','a','l','i','z','e','\0'};
					INSTRUCTION	Inst;
					ULONG nTempAddress = 0, nOpCodeLen = 0;
					
					AtaPortInitialize = FindExportedRoutineByName((PVOID)NewBase, szAtaPortInitialize);
					if (!AtaPortInitialize)
					{
						KdPrint(("find AtaPortInitialize error\n"));
						MzfExFreePoolWithTag(pOriginDispatchs, 0);
						MzfExFreePoolWithTag(pReloadDispatchs, 0);
						MzfExFreePoolWithTag((PVOID)NewBase, 0);
						return;
					}
					
					KdPrint(("AtaPortInitialize: 0x%08X\n", AtaPortInitialize));

					for (nTempAddress = (ULONG)AtaPortInitialize; nTempAddress < (ULONG)AtaPortInitialize + PAGE_SIZE; nTempAddress += nOpCodeLen)
					{
						if (!MzfMmIsAddressValid((PVOID)nTempAddress))
						{
							break;
						}

						memset(&Inst, 0, sizeof(INSTRUCTION));
						get_instruction(&Inst, (PBYTE)nTempAddress, MODE_32);

						if (Inst.type == INSTRUCTION_TYPE_CALL && Inst.opcode == 0xe8)
						{
							pDriverEntry = Inst.op1.immediate + nTempAddress + 5;
							KdPrint(("Inst.op1.immediate: 0x%X\n", Inst.op1.immediate));
							KdPrint(("pDriverEntry: 0x%08X\n", pDriverEntry));
							break;
						}

						if ( Inst.type == INSTRUCTION_TYPE_RET || Inst.length == 0 )
						{
							break;
						}

						nOpCodeLen = Inst.length;
					}
				}
				else
				{
					ENTRY_POINT_TYPE EntryType = enumEntryNone;
					WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

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
						KdPrint(("FindExportedRoutineByName error\n"));
						MzfExFreePoolWithTag(pOriginDispatchs, 0);
						MzfExFreePoolWithTag(pReloadDispatchs, 0);
						MzfExFreePoolWithTag((PVOID)NewBase, 0);
						return;
					}
				}
				
				KdPrint(("pDriverEntry: 0x%08X\n", pDriverEntry));

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
							pReloadDispatchs[nIndex] = Inst.op2.immediate - Base + NewBase;
						}
						else if (Inst.op1.type == OPERAND_TYPE_REGISTER && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
							MzfMmIsAddressValid((PVOID)Inst.op2.immediate) &&
							Inst.opcode < 0xBF)
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
							pReloadDispatchs[nIndex] = MovTemp - Base + NewBase;
						}
						else if (Inst.op1.type == OPERAND_TYPE_MEMORY && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
							Inst.op1.displacement == 0x30)
						{
							SetGlobalVeriable(enumOriginAtapiStartIoDispatch, Inst.op2.immediate);
							SetGlobalVeriable(enumReloadAtapiStartIoDispatch, Inst.op2.immediate - Base + NewBase);
						}
					}

					if (Inst.type == INSTRUCTION_TYPE_RET || Inst.length == 0)
					{
						break;
					}

					nCodeLen = Inst.length;
				}

				FixOriginDispatch(pOriginDispatchs);
				FixReloadDispatch(pReloadDispatchs);

				SetGlobalVeriable(enumAtapiOriginalDispatchs, (ULONG)pOriginDispatchs);
				SetGlobalVeriable(enumAtapiReloadDispatchs, (ULONG)pReloadDispatchs);
			}

			SetGlobalVeriable(enumReloadAtapiBase, NewBase);
		}
	}
}

NTSTATUS GetAtapiDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDISPATCH_HOOK_INFO pHookInfo = (PDISPATCH_HOOK_INFO)pOutBuffer;
	PULONG pReloadDispatch = (PULONG)GetGlobalVeriable(enumAtapiReloadDispatchs);
	PULONG pOriginDispatch = (PULONG)GetGlobalVeriable(enumAtapiOriginalDispatchs);
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAtapiDriverObject);

	if (!pReloadDispatch || 
		!pDriverObject || 
		!pOriginDispatch)
	{
		IninAtapi();
		pReloadDispatch = (PULONG)GetGlobalVeriable(enumAtapiReloadDispatchs);
		pOriginDispatch = (PULONG)GetGlobalVeriable(enumAtapiOriginalDispatchs);
		pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAtapiDriverObject);
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

	if (GetGlobalVeriable(enumOriginAtapiStartIoDispatch))
	{
		pHookInfo[28].nIndex = 28;
		pHookInfo[28].pNowAddress = (ULONG)pDriverObject->DriverStartIo;
		pHookInfo[28].pOriginAddress = GetGlobalVeriable(enumOriginAtapiStartIoDispatch);
	}

	return status;
}