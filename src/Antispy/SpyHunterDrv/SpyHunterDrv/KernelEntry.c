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
#include "KernelEntry.h"
#include "CommonFunction.h"
#include "libdasm.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"

BOOL FindOriginKiFastCallEntry()
{
	ULONG TotalCountBytes = 0;
	ULONG_PTR VA;
	ULONG SizeOfBlock;
	ULONG FixupVA;
	USHORT Offset;
	PUSHORT NextOffset = NULL;
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION NextBlock;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	PIMAGE_SECTION_HEADER TextSection = NULL;
	ULONG pTextSectionAddress = 0, nTextSectionSize = 0;
	CHAR szText[] = {'.','t','e','x','t','\0'};
	PVOID NewImageBase = (PVOID)GetGlobalVeriable(enumNewKernelBase);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	KdPrint(("FindOriginKiFastCallEntry\n"));

	if (!NewImageBase || !MzfMmIsAddressValid)
	{
		return FALSE;
	}

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)NewImageBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)NewImageBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		NtHeaders = RtlImageNtHeader( NewImageBase );
		if (NtHeaders == NULL) 
		{
			return FALSE;
		}

		TextSection = GetSecionAddress(NtHeaders, szText);
		if (!TextSection)
		{
			return FALSE;
		}

		pTextSectionAddress = TextSection->VirtualAddress + (ULONG)NewImageBase;
		nTextSectionSize = TextSection->Misc.VirtualSize;
		KdPrint(("pTextSectionAddress: 0x%08X, Size: 0x%X\n", pTextSectionAddress, nTextSectionSize));

		//
		// Locate the relocation section.
		//

		NextBlock = (PIMAGE_BASE_RELOCATION)MzfImageDirectoryEntryToData(
			NewImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &TotalCountBytes);

		//
		// It is possible for a file to have no relocations, but the relocations
		// must not have been stripped.
		//

		if (!NextBlock || !TotalCountBytes) 
		{
			if (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) 
			{
				KdPrint(("Image can't be relocated, no fixup information.\n"));
				return FALSE;
			}
			else 
			{
				return TRUE;
			}
		}

		while (TotalCountBytes)
		{
			SizeOfBlock = NextBlock->SizeOfBlock;
			TotalCountBytes -= SizeOfBlock;
			SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
			SizeOfBlock /= sizeof(USHORT);
			NextOffset = (PUSHORT)((PCHAR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));

			VA = (ULONG_PTR)NewImageBase + NextBlock->VirtualAddress;

			while (SizeOfBlock--) 
			{
				Offset = *NextOffset & (USHORT)0xfff;
				FixupVA = (ULONG)(VA + Offset);

				// .text:00427B18 68 90 97 46 00  push offset _KiFastCallEntry
				if (FixupVA > pTextSectionAddress &&
					FixupVA < pTextSectionAddress + nTextSectionSize &&
					*(BYTE *)(FixupVA - 1) == 0x68)
				{
					ULONG pKiFastCallEntry = *(PULONG)FixupVA;

					if (pKiFastCallEntry > SYSTEM_ADDRESS_START && 
						MzfMmIsAddressValid((PVOID)pKiFastCallEntry) &&
						*(PBYTE)(FixupVA + 4) == 0x68 &&
						*(PULONG)(FixupVA + 5) == 0x176)
					{
						SetGlobalVeriable(enumKiFastCallEntry, pKiFastCallEntry);
						KdPrint(("pKiFastCallEntry: 0x%08X\n", pKiFastCallEntry));
						return TRUE;
					}
				}

				++NextOffset;
				if ( !NextOffset || (ULONG)NextOffset <= SYSTEM_ADDRESS_START || !MzfMmIsAddressValid(NextOffset)) 
				{
					return FALSE;
				}
			}

			NextBlock = (PIMAGE_BASE_RELOCATION)NextOffset;
		}
	}
	__except(1)
	{
		return FALSE;
	}

	return TRUE;
}

void CmpKernelEntry(PKERNEL_ENTRY_HOOK_INFO pKeHookInof, ULONG nCnt)
{
	ULONG KiFastCallEntry = GetGlobalVeriable(enumKiFastCallEntry);
	ULONG iCnt = 0;

	if (!KiFastCallEntry)
	{
		FindOriginKiFastCallEntry();
		KiFastCallEntry = GetGlobalVeriable(enumKiFastCallEntry);
	}

	if(KiFastCallEntry && pKeHookInof && nCnt)
	{
		ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
		ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);
		ULONG ReloadBase = GetGlobalVeriable(enumNewKernelBase);
		pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

		if (MzfMmIsAddressValid &&
			KernelBase &&
			KernelSize && 
			ReloadBase &&
			KiFastCallEntry > KernelBase &&
			KiFastCallEntry < KernelBase + KernelSize)
		{
			ULONG ReloadKiFastCallEntry = KiFastCallEntry - KernelBase + ReloadBase;
			
			if (ReloadKiFastCallEntry > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)ReloadKiFastCallEntry))
			{
				ULONG Address = 0, nCodeLen = 0;
				ULONG i = 0, nDifCnt = 0, pDifAddress = 0;

				__try
				{
					for (Address = ReloadKiFastCallEntry; 
						Address < ReloadKiFastCallEntry + PAGE_SIZE && MzfMmIsAddressValid((PVOID)Address); 
						Address += nCodeLen)
					{
						INSTRUCTION	Inst;

						memset(&Inst, 0, sizeof(INSTRUCTION));
						get_instruction(&Inst, (PBYTE)Address, MODE_32);

						if (Inst.type == INSTRUCTION_TYPE_CALL &&
							Inst.op1.type == OPERAND_TYPE_REGISTER)
						{
							Address += Inst.length;
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

					nCodeLen = Address - ReloadKiFastCallEntry;
					if (nCodeLen > PAGE_SIZE)
					{
						nCodeLen = PAGE_SIZE;
					}

					KdPrint(("KiFastCallEntry CodeLen: %d\n", nCodeLen));

					for ( i = 0; i < nCodeLen; i++ )
					{
						if ( *((PBYTE)KiFastCallEntry + i) != *((PBYTE)ReloadKiFastCallEntry + i) )
						{
							if (!nDifCnt)
							{
								pDifAddress = (ULONG)KiFastCallEntry + i;
							}

							nDifCnt++;
						}
						else 
						{
							if (nDifCnt <= 5 && nDifCnt > 0)
							{
								INSTRUCTION	Inst;
								BOOL bHooked = FALSE;
								ULONG HookAddress = 0;

								memset(&Inst, 0, sizeof(INSTRUCTION));
								get_instruction(&Inst, (PBYTE)pDifAddress, MODE_32);

								if (Inst.type == INSTRUCTION_TYPE_CALL ||
									Inst.type == INSTRUCTION_TYPE_JMP ||
									Inst.type == INSTRUCTION_TYPE_PUSH ||
									Inst.type == INSTRUCTION_TYPE_MOV)
								{
									HookAddress = GetInlineHookAddress(pDifAddress, nDifCnt);

									if (HookAddress)
									{
										if ((ULONG)Inst.length > nDifCnt)
										{
											nDifCnt = Inst.length;
										}

										KdPrint(("pDifAddress: 0x%08X, nDifCnt: %d, HookAddress: 0x%08X\n", pDifAddress, nDifCnt, HookAddress));
									}
								}

								if (iCnt < nCnt)
								{
									pKeHookInof[iCnt].PatchLen = nDifCnt;
									pKeHookInof[iCnt].OriginAddress = pDifAddress;
									pKeHookInof[iCnt].HookAddress = HookAddress;

									if (nDifCnt > MAX_PATCHED_LEN)
									{
										nDifCnt = MAX_PATCHED_LEN;
									}

									MemCpy(pKeHookInof[iCnt].OriginBytes, (PVOID)(pDifAddress - KernelBase + ReloadBase), nDifCnt);		
									MemCpy(pKeHookInof[iCnt].NowBytes, (PVOID)pDifAddress, nDifCnt);

									iCnt++;
								}
							}
							else if (nDifCnt > 5)
							{
								ULONG nCodeLen = 0, nNotHookLen = 0;
								INSTRUCTION	Inst;
								ULONG AddressTemp = 0, NotHookAddressStart = 0;
								BOOL bHooked = FALSE;

								AddressTemp = NotHookAddressStart = pDifAddress;

								for ( nCodeLen = 0; nCodeLen < nDifCnt; nCodeLen += Inst.length )
								{	
									memset(&Inst, 0, sizeof(INSTRUCTION));
									get_instruction(&Inst, (PBYTE)AddressTemp, MODE_32);

									if (Inst.type == INSTRUCTION_TYPE_CALL ||
										Inst.type == INSTRUCTION_TYPE_JMP  ||
										Inst.type == INSTRUCTION_TYPE_PUSH ||
										Inst.type == INSTRUCTION_TYPE_MOV)
									{
										ULONG HookAddress = GetInlineHookAddress(AddressTemp, Inst.length );

										if (HookAddress)
										{
											bHooked = TRUE;

											if (nNotHookLen)
											{
												ULONG nLEN = AddressTemp - NotHookAddressStart;
												KdPrint(("pDifAddress: 0x%08X, nLEN: %d\n", NotHookAddressStart, nLEN));	
												if (iCnt < nCnt)
												{
													pKeHookInof[iCnt].PatchLen = nLEN;
													pKeHookInof[iCnt].OriginAddress = NotHookAddressStart;
													pKeHookInof[iCnt].HookAddress = 0;

													if (nLEN > MAX_PATCHED_LEN)
													{
														nLEN = MAX_PATCHED_LEN;
													}

													MemCpy(pKeHookInof[iCnt].OriginBytes, (PVOID)(NotHookAddressStart - KernelBase + ReloadBase), nLEN);		
													MemCpy(pKeHookInof[iCnt].NowBytes, (PVOID)NotHookAddressStart, nLEN);

													iCnt++;
												}
											}

											NotHookAddressStart = AddressTemp + Inst.length;
											nNotHookLen = 0;
											KdPrint(("pDifAddress: 0x%08X, HookAddress: 0x%08X\n", AddressTemp, HookAddress));
											if (iCnt < nCnt)
											{
												pKeHookInof[iCnt].PatchLen = Inst.length;
												pKeHookInof[iCnt].OriginAddress = AddressTemp;
												pKeHookInof[iCnt].HookAddress = HookAddress;

												if (Inst.length > MAX_PATCHED_LEN)
												{
													Inst.length = MAX_PATCHED_LEN;
												}

												MemCpy(pKeHookInof[iCnt].OriginBytes, (PVOID)(AddressTemp - KernelBase + ReloadBase), Inst.length);		
												MemCpy(pKeHookInof[iCnt].NowBytes, (PVOID)AddressTemp, Inst.length);

												iCnt++;
											}
										}
										else
										{
											nNotHookLen += Inst.length;
										}
									}
									else
									{
										nNotHookLen += Inst.length;
									}

									AddressTemp += Inst.length;
								}

								if (!bHooked)
								{
									KdPrint(("pDifAddress: 0x%08X, len: %d\n", pDifAddress, nDifCnt));
									if (iCnt < nCnt)
									{
										pKeHookInof[iCnt].PatchLen = nDifCnt;
										pKeHookInof[iCnt].OriginAddress = pDifAddress;
										pKeHookInof[iCnt].HookAddress = 0;

										if (nDifCnt > MAX_PATCHED_LEN)
										{
											nDifCnt = MAX_PATCHED_LEN;
										}

										MemCpy(pKeHookInof[iCnt].OriginBytes, (PVOID)(pDifAddress - KernelBase + ReloadBase), nDifCnt);		
										MemCpy(pKeHookInof[iCnt].NowBytes, (PVOID)pDifAddress, nDifCnt);

										iCnt++;
									}
								}
							}

							nDifCnt = 0;
						}
					}
				}
				__except(1)
				{

				}
			}
		}
	}
}

NTSTATUS EnumKernelEntryHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PKERNEL_ENTRY_HOOK_INFO pKeHookInof = (PKERNEL_ENTRY_HOOK_INFO)pOutBuffer;
	ULONG nCnt = uOutSize / sizeof(KERNEL_ENTRY_HOOK_INFO);
	ULONG KiFastCallEntry = GetGlobalVeriable(enumKiFastCallEntry);
	
	KdPrint(("KiFastCallEntry: 0x%08X\n", KiFastCallEntry));

	if (!KiFastCallEntry)
	{
		FindOriginKiFastCallEntry();
		KiFastCallEntry = GetGlobalVeriable(enumKiFastCallEntry);
	}

	if (KiFastCallEntry)
	{
		ULONG NowKiFastCallEntry = 0;
		
		__asm {
			pushad
			mov ecx, 0x176
			rdmsr                          
			mov NowKiFastCallEntry, eax
			popad
		}

		pKeHookInof[0].PatchLen = 0;
		pKeHookInof[0].OriginAddress = KiFastCallEntry;
		pKeHookInof[0].HookAddress = NowKiFastCallEntry;

		KdPrint(("KiFastCallEntry: 0x%08X, NowKiFastCallEntry: 0x%08X\n", KiFastCallEntry, NowKiFastCallEntry));

		CmpKernelEntry(pKeHookInof + 1, nCnt - 1);
		status = STATUS_SUCCESS;
	}
	
	return status;
}