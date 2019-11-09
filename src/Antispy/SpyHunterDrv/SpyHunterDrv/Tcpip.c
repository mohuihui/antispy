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
#include "Tcpip.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Peload.h"
#include "libdasm.h"
#include "DispatchHook.h"

void IninTcpip()
{
	ULONG Base = GetGlobalVeriable(enumTcpipBase);
	ULONG Size = GetGlobalVeriable(enumTcpipSize);
	WCHAR szDriver[] = {'\\','D','r','i','v','e','r','\\','\0'};
	WCHAR szDriverDir[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'};
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("Tcpip -> Base: 0x%08X, Size: 0x%X\n", Base, Size));
	if (Base && Size && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		WCHAR szFilePath[MAX_PATH] = {0};
		WCHAR szSysName[] = {'t','c','p','i','p','.','s','y','s','\0'};
		WCHAR szDriverName[] = {'t','c','p','i','p','\0'};
		ULONG NewBase = 0;
		WCHAR szDriverPath[MAX_PATH] = {0};
		PDRIVER_OBJECT pDriverObject = NULL;
	
		wcscpy(szDriverPath, szDriver);
		wcscat(szDriverPath, szDriverName);

		if (NT_SUCCESS(GetDriverObjectByName(szDriverPath, &pDriverObject)))
		{
			KdPrint(("Tcpip Driverobject: 0x%08X\n", pDriverObject));
			SetGlobalVeriable(enumTcpipDriverObject, (ULONG)pDriverObject);
		}
		else
		{
			KdPrint(("Get Tcpip driverobject error\n"));
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
				ENTRY_POINT_TYPE EntryType = enumEntryNone;
				WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
				ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

				memset(pOriginDispatchs, 0, sizeof(ULONG) * 28);
				memset(pReloadDispatchs, 0, sizeof(ULONG) * 28);

				if (Version == enumWINDOWS_2K3 || 
					Version == enumWINDOWS_8)
				{
					EntryType = enumEntryPointJmp;
				}
				else
				{
					EntryType = enumEntryPointHead;
				}

				if (!GetDriverEntryPoint((PVOID)NewBase, &pDriverEntry, EntryType))
				{
					KdPrint(("GetDriverEntryPoint error\n"));
					MzfExFreePoolWithTag(pOriginDispatchs, 0);
					MzfExFreePoolWithTag(pReloadDispatchs, 0);
					MzfExFreePoolWithTag((PVOID)NewBase, 0);
					return;
				}
				
				KdPrint(("pDriverEntry: 0x%08X\n", pDriverEntry));

				if (BuildNumber >= 6000)
				{
					ULONG nCallCnt = 0;
					ULONG NlStartup = 0;

					for (Address = pDriverEntry; Address < pDriverEntry + PAGE_SIZE; Address += nCodeLen)
					{
						INSTRUCTION	Inst;

						if (!MzfMmIsAddressValid((PVOID)Address))
						{
							break;
						}

						memset(&Inst, 0, sizeof(INSTRUCTION));
						get_instruction(&Inst, (PBYTE)Address, MODE_32);

						if (Inst.type == INSTRUCTION_TYPE_CALL && Inst.opcode == 0xE8)
						{
							nCallCnt++;
							
							KdPrint(("Inst.op1.immediate: 0x%X\n", Inst.op1.immediate));
							if (nCallCnt == 6 && (Version == enumWINDOWS_VISTA || Version == enumWINDOWS_8))
							{
								NlStartup = Address + Inst.op1.immediate + 5;
							}
							else if (nCallCnt == 8 && Version == enumWINDOWS_VISTA_SP1_SP2)
							{
								NlStartup = Address + Inst.op1.immediate + 5;
							}
							else if (nCallCnt == 10 && Version == enumWINDOWS_7)
							{
								NlStartup = Address + Inst.op1.immediate + 5;
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
					
					KdPrint(("NlStartup: 0x%08X\n", NlStartup));

					if (NlStartup && MzfMmIsAddressValid((PVOID)NlStartup))
					{
						for (Address = NlStartup; Address < NlStartup + PAGE_SIZE; Address += nCodeLen)
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
					}
				}
				else
				{
					ULONG MovTemp = 0;
					BOOL bRepMov = FALSE;

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
							else if (Inst.op1.type == OPERAND_TYPE_MEMORY && 
								Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
								Inst.op1.displacement >= 0x38 &&
								Inst.op1.displacement <= 0xA4)
							{
								ULONG nIndex = (Inst.op1.displacement - 0x38) / sizeof(ULONG);
								pOriginDispatchs[nIndex] = Inst.op2.immediate;
								pReloadDispatchs[nIndex] = Inst.op2.immediate - Base + NewBase;
							}
						}
						else if (Inst.type == INSTRUCTION_TYPE_STOS && !bRepMov)
						{
							if (MovTemp && MmIsAddressValid((PVOID)MovTemp))
							{
								ULONG MovTempReload = MovTemp - Base + NewBase;
								ULONG i = 0;

								KdPrint(("MovTemp: 0x%08X\n", MovTemp));
								KdPrint(("MovTempReload: 0x%08X\n", MovTempReload));

								for (i = 0; i < 28; i++)
								{
									pReloadDispatchs[i] = MovTempReload;
									pOriginDispatchs[i] = MovTemp;
								}

								bRepMov = TRUE;
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
				}
				
				FixOriginDispatch(pOriginDispatchs);
				FixReloadDispatch(pReloadDispatchs);

				SetGlobalVeriable(enumTcpipOriginalDispatchs, (ULONG)pOriginDispatchs);
				SetGlobalVeriable(enumTcpipReloadDispatchs, (ULONG)pReloadDispatchs);
			}

			SetGlobalVeriable(enumReloadTcpipBase, NewBase);
		}
	}
}

NTSTATUS GetTcpipDispatch(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDISPATCH_HOOK_INFO pHookInfo = (PDISPATCH_HOOK_INFO)pOutBuffer;
	PULONG pReloadDispatch = (PULONG)GetGlobalVeriable(enumTcpipReloadDispatchs);
	PULONG pOriginDispatch = (PULONG)GetGlobalVeriable(enumTcpipOriginalDispatchs);
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumTcpipDriverObject);

	if (!pReloadDispatch || 
		!pDriverObject || 
		!pOriginDispatch)
	{
		IninTcpip();
		pReloadDispatch = (PULONG)GetGlobalVeriable(enumTcpipReloadDispatchs);
		pOriginDispatch = (PULONG)GetGlobalVeriable(enumTcpipOriginalDispatchs);
		pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumTcpipDriverObject);
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