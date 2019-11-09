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
#include "Idt.h"
#include "CommonFunction.h"
#include "libdasm.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"

typedef struct
{
	unsigned short IDTLimit; //代表IDT的大小, 为7FFH
	unsigned short LowIDTbase;
	unsigned short HiIDTbase;
}IDTINFO;

#pragma pack(1)
typedef struct
{
	unsigned short LowOffset;
	unsigned short selector;
	unsigned char unused_lo;
	unsigned char segment_type:4; 
	unsigned char system_segment_flag:1;
	unsigned char DPL:2;
	unsigned char P:1;
	unsigned short HiOffect;
}IDTENTRY;
#pragma pack()

ULONG GetIdtTableInReloadKernel()
{
	ULONG ReloadIdtTable = 0;
	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG NewKernel = GetGlobalVeriable(enumNewKernelBase);
	ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

	KdPrint(("GetIdtTableInReloadKernel-> NewKernel: 0x%08X, KernelSize: 0x%X\n", NewKernel, KernelSize));

	if (KernelBase && NewKernel && KernelSize && MzfMmIsAddressValid)
	{
		ULONG pDriverEntry = 0;	
	
		if (Version == enumWINDOWS_2K)
		{
			if (GetDriverEntryPoint((PVOID)NewKernel, &pDriverEntry, enumEntryPointHead))
			{
				ULONG Address = 0, nCodeLen = 0, nIdtTableOffset = 0;
				ULONG KiSystemStartup = 0;

				KdPrint(("pDriverEentry: 0x%08X\n", pDriverEntry));

				for (Address = pDriverEntry; Address < pDriverEntry + PAGE_SIZE; Address += nCodeLen)
				{
					INSTRUCTION	Inst;

					if (!MzfMmIsAddressValid((PVOID)Address))
					{
						break;
					}

					memset(&Inst, 0, sizeof(INSTRUCTION));
					get_instruction(&Inst, (PBYTE)Address, MODE_32);

					if (Inst.type == INSTRUCTION_TYPE_CALL)
					{
						KdPrint(("Inst.op1.immediate: 0x%08X\n", Inst.op1.immediate));

						KiSystemStartup = Address + Inst.op1.immediate + Inst.length;
						KdPrint(("KiSystemStartup: 0x%08X\n", KiSystemStartup));

						if (KiSystemStartup > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)KiSystemStartup))
						{
							break;
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

				if (KiSystemStartup > SYSTEM_ADDRESS_START)
				{
					KdPrint(("KiSystemStartup: 0x%08X\n", KiSystemStartup));
					
					for (Address = KiSystemStartup; 
						Address < KiSystemStartup + PAGE_SIZE && MzfMmIsAddressValid((PVOID)Address); 
						Address += nCodeLen)
					{
						INSTRUCTION	Inst;

						memset(&Inst, 0, sizeof(INSTRUCTION));
						get_instruction(&Inst, (PBYTE)Address, MODE_32);

						if (Inst.type == INSTRUCTION_TYPE_MOV)
						{
							if (Inst.op1.type == OPERAND_TYPE_REGISTER && 
								Inst.op2.type == OPERAND_TYPE_IMMEDIATE)
							{
								if (Inst.op2.immediate > SYSTEM_ADDRESS_START
									/*&& MzfMmIsAddressValid((PVOID)Inst.op2.immediate)*/
									)
								{
									if (MzfMmIsAddressValid((PVOID)(Address + Inst.length)))
									{
										INSTRUCTION	InstTemp;
										memset(&InstTemp, 0, sizeof(INSTRUCTION));
										get_instruction(&InstTemp, (PBYTE)(Address + Inst.length), MODE_32);

										if (InstTemp.type == INSTRUCTION_TYPE_MOV && 
											InstTemp.op1.type == OPERAND_TYPE_REGISTER && 
											InstTemp.op2.type == OPERAND_TYPE_IMMEDIATE &&
											InstTemp.op2.immediate == 0x800)
										{
											ReloadIdtTable = Inst.op2.immediate - KernelBase + NewKernel;
											KdPrint(("OriginIdtTable: 0x%08X\n", Inst.op2.immediate));
											KdPrint(("ReloadIdtTable: 0x%08X\n", ReloadIdtTable));
											break;
										}
									}
								}
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
		}
		else
		{
			if (GetDriverEntryPoint((PVOID)NewKernel, &pDriverEntry, enumEntryPointHead))
			{
				ULONG Address = 0, nCodeLen = 0, nIdtTableOffset = 0;

				KdPrint(("pDriverEentry: 0x%08X\n", pDriverEntry));

				for (Address = pDriverEntry; 
					Address < pDriverEntry + PAGE_SIZE && MzfMmIsAddressValid((PVOID)Address); 
					Address += nCodeLen)
				{
					INSTRUCTION	Inst;

					memset(&Inst, 0, sizeof(INSTRUCTION));
					get_instruction(&Inst, (PBYTE)Address, MODE_32);

					if (Inst.type == INSTRUCTION_TYPE_MOV)
					{
						if (Inst.op1.type == OPERAND_TYPE_REGISTER && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE)
						{
							if (Inst.op2.immediate > SYSTEM_ADDRESS_START 
								/*&& MzfMmIsAddressValid((PVOID)Inst.op2.immediate)*/
								)
							{
								if (MzfMmIsAddressValid((PVOID)(Address + Inst.length)))
								{
									INSTRUCTION	InstTemp;
									memset(&InstTemp, 0, sizeof(INSTRUCTION));
									get_instruction(&InstTemp, (PBYTE)(Address + Inst.length), MODE_32);

									if (InstTemp.type == INSTRUCTION_TYPE_MOV && 
										InstTemp.op1.type == OPERAND_TYPE_REGISTER && 
										InstTemp.op2.type == OPERAND_TYPE_IMMEDIATE &&
										InstTemp.op2.immediate == 0x800)
									{
										ReloadIdtTable = Inst.op2.immediate - KernelBase + NewKernel;
										KdPrint(("OriginIdtTable: 0x%08X\n", Inst.op2.immediate));
										KdPrint(("ReloadIdtTable: 0x%08X\n", ReloadIdtTable));
										break;
									}
								}
							}
						}
					}

					if (Inst.type == INSTRUCTION_TYPE_RET || Inst.length == 0)
					{
						break;
					}

					nCodeLen = Inst.length;
				}
			}
		}
	}

	return ReloadIdtTable;
}

void GetOriginIdtTable()
{
	ULONG ReloadIdtTable = GetIdtTableInReloadKernel();
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG NewKernel = GetGlobalVeriable(enumNewKernelBase);

	if (KernelBase &&
		NewKernel &&
		MzfMmIsAddressValid && 
		MzfExAllocatePoolWithTag && 
		MzfExFreePoolWithTag &&
		ReloadIdtTable > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)ReloadIdtTable))
	{
		PULONG pOriginIdtTable = (PULONG)MzfExAllocatePoolWithTag(NonPagedPool, MAX_IDT * sizeof(ULONG), MZFTAG);
		PULONG pReloadIdtTable = (PULONG)MzfExAllocatePoolWithTag(NonPagedPool, MAX_IDT * sizeof(ULONG), MZFTAG);
		ULONG i = 0;

		if (pOriginIdtTable && pReloadIdtTable)
		{	
			memset(pOriginIdtTable, 0, sizeof(ULONG) * MAX_IDT);
			memset(pReloadIdtTable, 0, sizeof(ULONG) * MAX_IDT);

			for (i = 0; i < MAX_IDT; i++)
			{
				pOriginIdtTable[i] = *(PULONG)(ReloadIdtTable + i * 8);

				if (pOriginIdtTable[i])
				{
					pReloadIdtTable[i] = pOriginIdtTable[i] - KernelBase + NewKernel;
				}
				else
				{
					pReloadIdtTable[i] = 0;
				}
				
				KdPrint(("IDT[%d] Orgin: 0x%08X, Reload: 0x%08X\n", i, pOriginIdtTable[i], pReloadIdtTable[i]));
			}

			SetGlobalVeriable(enumOriginIdtTable, (ULONG)pOriginIdtTable);
			SetGlobalVeriable(enumReloadIdtTable, (ULONG)pReloadIdtTable);
		}
	}
}

void GetNowIdt(PIDT_HOOK_INFO pIdtInfo)
{
	IDTINFO idt_info;
	IDTENTRY* idt_entrys = NULL;
	ULONG i = 0;

	if (!pIdtInfo)
	{
		return;
	}
	
	memset(&idt_info, 0, sizeof(IDTINFO));

	__asm sidt idt_info;

	idt_entrys = (IDTENTRY*)MAKELONG(idt_info.LowIDTbase, idt_info.HiIDTbase);

	KdPrint(("IDT Addr: 0x%08X\n", idt_entrys));
	KdPrint(("IDT Limit: %d\n", idt_info.IDTLimit));

	for (i = 0; i < MAX_IDT; i++)
	{
		ULONG addr = 0;
		IDTENTRY* pIdtItem = &idt_entrys[i];

		addr = MAKELONG(pIdtItem->LowOffset, pIdtItem->HiOffect);
		
		if (addr <= SYSTEM_ADDRESS_START)
		{
			addr = 0;
		}

		pIdtInfo[i].nIndex = i;
		pIdtInfo[i].pNowAddress = addr;

		KdPrint(("%d 0x%08X\n", i, addr));
	}
}

NTSTATUS EnumIdtHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIDT_HOOK_INFO pIdtInfo = (PIDT_HOOK_INFO)pOutBuffer;
	PULONG pOriginIdtTable = (PULONG)GetGlobalVeriable(enumOriginIdtTable);

	GetNowIdt(pIdtInfo);

	if (!pOriginIdtTable)
	{
		GetOriginIdtTable();
		pOriginIdtTable = (PULONG)GetGlobalVeriable(enumOriginIdtTable);
	}

	if (pOriginIdtTable)
	{
		ULONG i = 0;
		for (i = 0; i < MAX_IDT; i++)
		{
			pIdtInfo[i].pOriginAddress = pOriginIdtTable[i];
		}
	}

	return status;
}