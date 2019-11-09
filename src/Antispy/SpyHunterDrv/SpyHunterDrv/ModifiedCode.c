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
#include "ModifiedCode.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "Peload.h"
#include "libdasm.h"

//
// 反汇编得到一个函数的大致大小
//
ULONG GetSizeOfFunction(ULONG FunctionAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG nCodeLen = 0;

	if (MzfMmIsAddressValid && FunctionAddress && MzfMmIsAddressValid((PVOID)FunctionAddress))
	{
		ULONG Address = 0;

		__try
		{
			for (Address = FunctionAddress; 
				Address < FunctionAddress + PAGE_SIZE && MzfMmIsAddressValid((PVOID)Address); 
				Address += nCodeLen)
			{
				INSTRUCTION	Inst;

				memset(&Inst, 0, sizeof(INSTRUCTION));
				get_instruction(&Inst, (PBYTE)Address, MODE_32);

				if (Inst.length == 0 ||
					Inst.type == INSTRUCTION_TYPE_RET ||
					(Inst.type == INSTRUCTION_TYPE_INT && Inst.opcode == 0xc3))
				{
					break;
				}

				// 到了下一个函数开头的地方
				else if (Address - FunctionAddress > 5 &&
					Inst.type == INSTRUCTION_TYPE_PUSH &&
					Inst.opcode == 0x55 &&
					*(PWORD)(Address + Inst.length) == 0xec8b)
				{
					break;
				}

				nCodeLen = Inst.length;
			}

			nCodeLen = Address - FunctionAddress;
		}
		__except(1)
		{
			nCodeLen = 0;
		}
	}

	return nCodeLen;
}

//
// 判断给定函数地址是否在code段中
//
BOOL IsCodeSection( ULONG ModuleBase, PIMAGE_NT_HEADERS NtHeaders, ULONG Address )
{
	BOOL bRet = FALSE;

	if (!ModuleBase || !NtHeaders || !Address)
	{
		return bRet;
	}

	__try
	{
		PIMAGE_SECTION_HEADER NtSection = IMAGE_FIRST_SECTION( NtHeaders );
		ULONG i = 0;

		for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) 
		{
			if (Address > NtSection->VirtualAddress + ModuleBase && 
				Address < NtSection->VirtualAddress + ModuleBase + NtSection->Misc.VirtualSize)
			{
				CHAR szINIT[] = {'I','N','I','T','\0'};

				// 标志是0x20, 并且不在初始化段中
				if ( ((NtSection->Characteristics/* & IMAGE_SCN_CNT_CODE*/) == IMAGE_SCN_CNT_CODE) &&	
					_stricmp(NtSection->Name, szINIT))

				{
					bRet = TRUE;
				}

				break;
			}
			
			++NtSection;
		}
	}
	__except(1)
	{
		bRet = FALSE;
	}

	return bRet;
}

void AddExportFunctionsPatchsItem(PALL_EXPORT_FUNCTION_PATCHS pInfo, 
								  ULONG nCnt,
								  ULONG OriginFunction,
								  ULONG HookAddress,
								  ULONG pDifAddress,
								  ULONG nDifCnt,
								  ULONG OriginBase,
								  ULONG ReloadBase,
								  CHAR *pReloadFuncName)
{
	ANSI_STRING anFunction;
	UNICODE_STRING unFunction;
	ULONG nCurCnt = pInfo->nCnt;
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);

	if (nCnt > nCurCnt)
	{
		pInfo->ExportPatch[nCurCnt].FunctionAddress = OriginFunction;
		pInfo->ExportPatch[nCurCnt].HookAddress = HookAddress;
		pInfo->ExportPatch[nCurCnt].PatchedAddress = pDifAddress;
		pInfo->ExportPatch[nCurCnt].PatchLen = nDifCnt;

		MemCpy(pInfo->ExportPatch[nCurCnt].NowBytes, (PVOID)pDifAddress, nDifCnt);
		MemCpy(pInfo->ExportPatch[nCurCnt].OriginBytes, (PVOID)(pDifAddress - OriginBase + ReloadBase), nDifCnt);

		MzfRtlInitAnsiString(&anFunction, pReloadFuncName);

		if (NT_SUCCESS(MzfRtlAnsiStringToUnicodeString(&unFunction, &anFunction, TRUE)))
		{
			ULONG len = CmpAndGetStringLength(&unFunction, MAX_FUNCTION_LEN);
			wcsncpy(pInfo->ExportPatch[nCurCnt].szFunction, unFunction.Buffer, len);
			MzfRtlFreeUnicodeString(&unFunction);
		}
		else
		{
			WCHAR szUnknow[] = {'U','n','K','n','o','w','\0'};
			wcscpy(pInfo->ExportPatch[nCurCnt].szFunction, szUnknow);
		}
	}

	pInfo->nCnt++;
}

ULONG KepEnumExportFunctionsPatchs(
									ULONG OriginBase, 
									ULONG ReloadBase, 
									ULONG Size, 
									PALL_EXPORT_FUNCTION_PATCHS pModifyCodeHookInfo, 
									ULONG nCnt
								 )
{
	PIMAGE_EXPORT_DIRECTORY ReloadImageExportDirectory = NULL;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	ULONG ExportSize = 0;
	PULONG pReloadAddressOfNames = NULL;
	PUSHORT pReloadAddressOfNameOrdinals = NULL;
	PULONG ReloadAddressOfFunctionsRVA = 0;
	ULONG i = 0;
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!OriginBase					|| 
		!ReloadBase					|| 
		!pModifyCodeHookInfo		|| 
		!MzfRtlInitAnsiString		|| 
		!MzfRtlFreeUnicodeString	||
		!MzfMmIsAddressValid		||	
		!MzfRtlImageDirectoryEntryToData ||
		!MzfRtlAnsiStringToUnicodeString )
	{
		return 0;
	}

	ImageDosHeader = (PIMAGE_DOS_HEADER)ReloadBase;

	__try
	{
		// 验证PE头
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return 0;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)ReloadBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return 0;
		}
		
		// 得到导出表
		ReloadImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)MzfRtlImageDirectoryEntryToData(
			(PVOID)ReloadBase, 
			TRUE, 
			IMAGE_DIRECTORY_ENTRY_EXPORT, 
			&ExportSize
			);
		if (ReloadImageExportDirectory == NULL)
		{
			return 0;
		}

		ReloadAddressOfFunctionsRVA = (PULONG)(ReloadBase + ReloadImageExportDirectory->AddressOfFunctions);
		pReloadAddressOfNames = (ULONG*)(ReloadBase + ReloadImageExportDirectory->AddressOfNames); 
		pReloadAddressOfNameOrdinals = (USHORT*)(ReloadBase + ReloadImageExportDirectory->AddressOfNameOrdinals); 

		if (!pReloadAddressOfNames || 
			!pReloadAddressOfNameOrdinals)
		{
			return 0;
		}

		for (i = 0; i < ReloadImageExportDirectory->NumberOfNames; i++)
		{
			PUCHAR pReloadFuncName = (PUCHAR)(ReloadBase + pReloadAddressOfNames[i]);
			USHORT nIndex = pReloadAddressOfNameOrdinals[i];
			ULONG ReloadFunction = ReloadAddressOfFunctionsRVA[nIndex] + ReloadBase;
			ULONG OriginFunction = ReloadAddressOfFunctionsRVA[nIndex] + OriginBase;

			if (ReloadAddressOfFunctionsRVA[nIndex] && 
				MzfMmIsAddressValid((PVOID)ReloadFunction) &&
				MzfMmIsAddressValid((PVOID)OriginFunction))
			{
				// 判断是否是code段
				if (IsCodeSection(ReloadBase, ImageNtHeaders, ReloadFunction))
				{
					ULONG FunctionSize = GetSizeOfFunction(ReloadFunction);
					ULONG j = 0, nDifCnt = 0, pDifAddress = 0;

					for (j = 0; j < FunctionSize; j++)
					{
						if ( *((PBYTE)OriginFunction + j) != *((PBYTE)ReloadFunction + j) &&
							*(PULONG)(ReloadFunction + j) != 0 &&
							*(PULONG)(ReloadFunction + sizeof(ULONG) + j) != 0) 
						{
							// 如果是第一个字节,就记录下这个地址
							if (!nDifCnt)
							{
								pDifAddress = (ULONG)OriginFunction + j;
							}

							// 递增不一样的字节数
							nDifCnt++;
						}

						// 如果是相等的
						else 
						{
							// 如果不相等的字节数在5个字节以内
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
										AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
											nCnt, 
											OriginFunction,
											HookAddress,
											pDifAddress,
											nDifCnt,
											OriginBase,
											ReloadBase,
											pReloadFuncName);

										KdPrint(("pReloadFuncName: %s, pDifAddress: 0x%08X, nDifCnt: %d, HookAddress: 0x%08X\n", 
											pReloadFuncName, pDifAddress, nDifCnt, HookAddress));
									}
								}

								// dif小于5字节
								else
								{
									if (nDifCnt < 5)
									{
										memset(&Inst, 0, sizeof(INSTRUCTION));
										get_instruction(&Inst, (PBYTE)pDifAddress - (5 - nDifCnt), MODE_32);

										if (Inst.type == INSTRUCTION_TYPE_CALL ||
											Inst.type == INSTRUCTION_TYPE_JMP ||
											Inst.type == INSTRUCTION_TYPE_PUSH ||
											Inst.type == INSTRUCTION_TYPE_MOV)
										{
											HookAddress = GetInlineHookAddress(pDifAddress - (5 - nDifCnt), nDifCnt);
											if (HookAddress)
											{
												AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
													nCnt, 
													OriginFunction,
													HookAddress,
													pDifAddress,
													nDifCnt,
													OriginBase,
													ReloadBase,
													pReloadFuncName);

												KdPrint(("pReloadFuncName: %s, pDifAddress: 0x%08X, nDifCnt: %d, HookAddress: 0x%08X\n", 
													pReloadFuncName, pDifAddress, nDifCnt, HookAddress));
											}
										}	
									}
								}

								// 没有找到跳转的模块
								if (!HookAddress)
								{
									AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
										nCnt, 
										OriginFunction,
										0,
										pDifAddress,
										nDifCnt,
										OriginBase,
										ReloadBase,
										pReloadFuncName);
									
									KdPrint(("pReloadFuncName: %s, pDifAddress: 0x%08X, len: %d\n", pReloadFuncName, pDifAddress, nDifCnt));
								}
							}

							// dif大于5字节
							else if (nDifCnt > 5)
							{
								ULONG nCodeLen = 0, nNotHookLen = 0;
								INSTRUCTION	Inst;
								ULONG AddressTemp = 0, NotHookAddressStart = 0;
								BOOL bHooked = FALSE;

								AddressTemp = NotHookAddressStart = pDifAddress;
								
								// 分割每一小块
								for ( nCodeLen = 0; nCodeLen < nDifCnt; nCodeLen += Inst.length )
								{
									memset(&Inst, 0, sizeof(INSTRUCTION));
									get_instruction(&Inst, (PBYTE)AddressTemp, MODE_32);

									if ( Inst.length == 0 )
									{
										break;
									}

									if (Inst.type == INSTRUCTION_TYPE_CALL ||
										Inst.type == INSTRUCTION_TYPE_JMP  ||
										Inst.type == INSTRUCTION_TYPE_PUSH ||
										Inst.type == INSTRUCTION_TYPE_MOV)
									{
										ULONG HookAddress = GetInlineHookAddress( AddressTemp, Inst.length );
										if (HookAddress)
										{
											bHooked = TRUE;

											if (nNotHookLen)
											{
												AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
													nCnt, 
													OriginFunction,
													0,
													NotHookAddressStart,
													AddressTemp - NotHookAddressStart,
													OriginBase,
													ReloadBase,
													pReloadFuncName);

												KdPrint(("pReloadFuncName: %s, pDifAddress: 0x%08X, nLEN: %d\n", 
													pReloadFuncName, NotHookAddressStart, AddressTemp - NotHookAddressStart));	
											}

											NotHookAddressStart = AddressTemp + Inst.length;
											nNotHookLen = 0;
	
											AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
												nCnt, 
												OriginFunction,
												HookAddress,
												AddressTemp,
												Inst.length,
												OriginBase,
												ReloadBase,
												pReloadFuncName);

											KdPrint(("pReloadFuncName: %s, pDifAddress: 0x%08X, HookAddress: 0x%08X\n", 
												pReloadFuncName, AddressTemp, HookAddress));
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

								// 最后,如果全部没有跳转的模块,那么就把整块内存加入到列表中
								if (!bHooked)
								{
									AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
										nCnt, 
										OriginFunction,
										0,
										pDifAddress,
										nDifCnt,
										OriginBase,
										ReloadBase,
										pReloadFuncName);
										
									KdPrint(("pReloadFuncName: %s, pDifAddress: 0x%08X, len: %d\n", 
										pReloadFuncName, pDifAddress, nDifCnt));
								}
							}

							nDifCnt = 0;
						}	
					}
				}
			}
		}
	}
	__except(1)
	{
	}

	return 0;
}

//
// 枚举导出函数的patchs
//
NTSTATUS EnumExportFunctionsPatchs(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PALL_EXPORT_FUNCTION_PATCHS pModifyCodeHookInfo = (PALL_EXPORT_FUNCTION_PATCHS)pOutBuffer;
	PCOMMUNICATE_EXPORT_FUNCTION_HOOK pModifyCodeHookInput = (PCOMMUNICATE_EXPORT_FUNCTION_HOOK)pInBuffer;
	ULONG nCnt = (uOutSize - sizeof(ALL_EXPORT_FUNCTION_PATCHS)) / sizeof(EXPORT_FUNCTION_PATCH_INFO);
	ULONG Base = 0, Size = 0, NewImageBase = 0;
	WCHAR *szPath = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	
	if (!MzfMmIsAddressValid || !MzfExFreePoolWithTag)
	{
		return status;
	}

	// 参数检查
	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_EXPORT_FUNCTION_HOOK) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_EXPORT_FUNCTION_PATCHS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	Base = pModifyCodeHookInput->op.Get.Base;
	Size = pModifyCodeHookInput->op.Get.Size;
	szPath = pModifyCodeHookInput->op.Get.szPath;
	
	if (Size == 0									||
		Base == 0									||
		Size > SYSTEM_ADDRESS_START					||
		Base < SYSTEM_ADDRESS_START					||
		Base + Size < SYSTEM_ADDRESS_START			||
		!MzfMmIsAddressValid((PVOID)Base)			||
		!MzfMmIsAddressValid((PVOID)(Base+Size-1))	||
		wcslen(szPath) <= 3)
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (PeLoad(szPath, &NewImageBase, Base) && 
		NewImageBase != 0					&&
		NewImageBase > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)NewImageBase))
	{
		KepEnumExportFunctionsPatchs(
			Base, 
			NewImageBase, 
			Size, 
			pModifyCodeHookInfo, 
			nCnt
			);

		if (pModifyCodeHookInfo->nCnt <= nCnt)
		{
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	if (NewImageBase)
	{
		MzfExFreePoolWithTag((PVOID)NewImageBase, 0);
		NewImageBase = 0;
	}

	return status;
}

ULONG PspEnumSsdtInlineHookInfo(PULONG OriginFunction, 
								PULONG ReloadFunction, 
								ULONG SsdtCnt, 
								PALL_EXPORT_FUNCTION_PATCHS pModifyCodeHookInfo, 
								ULONG nCnt
								)
{
	ULONG i = 0;
	PVOID pSsdtName = (PVOID)GetGlobalVeriable(enumSsdtFuncionsNameBuffer);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);

	if (!pSsdtName								||
		!MzfMmIsAddressValid					||
		!MzfRtlInitAnsiString					||
		!MzfRtlAnsiStringToUnicodeString		||
		!MzfRtlFreeUnicodeString				||
		!OriginFunction							||
		!ReloadFunction							||
		!SsdtCnt								||
		!pModifyCodeHookInfo					||
		!MzfMmIsAddressValid(OriginFunction)	||
		!MzfMmIsAddressValid(ReloadFunction) )
	{
		return 0;
	}

	for (i = 0; i < SsdtCnt; i++)
	{	
		__try
		{
			if (MzfMmIsAddressValid((PVOID)(OriginFunction + i * sizeof(ULONG)))	&&
				MzfMmIsAddressValid((PVOID)OriginFunction[i])						&& 
				MzfMmIsAddressValid((PVOID)(ReloadFunction + i * sizeof(ULONG)))	&&
				MzfMmIsAddressValid((PVOID)ReloadFunction[i]))
			{
				ULONG FunctionSize = GetSizeOfFunction(ReloadFunction[i]);
				ULONG j = 0, nDifCnt = 0, pDifAddress = 0;

				for (j = 0; j < FunctionSize; j++)
				{
					if ( *((PBYTE)OriginFunction[i] + j) != *((PBYTE)ReloadFunction[i] + j)  &&
						*(PULONG)(ReloadFunction[i] + j) != 0 &&
						*(PULONG)(ReloadFunction[i] + sizeof(ULONG) + j) != 0 )
					{
						if (!nDifCnt)
						{
							pDifAddress = (ULONG)OriginFunction[i] + j;
							KdPrint(("%d, 0x%08X\n", i, pDifAddress));
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
									AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
										nCnt, 
										OriginFunction[i],
										HookAddress,
										pDifAddress,
										nDifCnt,
										OriginFunction[i],
										ReloadFunction[i],
										(PCHAR)pSsdtName + i * 100);
								}
							}
							else
							{
								if (nDifCnt < 5)
								{
									memset(&Inst, 0, sizeof(INSTRUCTION));
									get_instruction(&Inst, (PBYTE)pDifAddress - (5 - nDifCnt), MODE_32);

									if (Inst.type == INSTRUCTION_TYPE_CALL ||
										Inst.type == INSTRUCTION_TYPE_JMP ||
										Inst.type == INSTRUCTION_TYPE_PUSH ||
										Inst.type == INSTRUCTION_TYPE_MOV)
									{
										HookAddress = GetInlineHookAddress(pDifAddress - (5 - nDifCnt), nDifCnt);
										if (HookAddress)
										{
											AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
												nCnt, 
												OriginFunction[i],
												HookAddress,
												pDifAddress,
												nDifCnt,
												OriginFunction[i],
												ReloadFunction[i],
												(PCHAR)pSsdtName + i * 100);
										}
									}	
								}
							}

							if (!HookAddress)
							{
								AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
									nCnt, 
									OriginFunction[i],
									0,
									pDifAddress,
									nDifCnt,
									OriginFunction[i],
									ReloadFunction[i],
									(PCHAR)pSsdtName + i * 100);

								KdPrint(("%d, OriginFunction: 0x%08X, pDifAddress: 0x%08X, nDifCnt: %d\n", 
									i, OriginFunction[i], pDifAddress, nDifCnt));
							}
						}

						// 如果dif大于5字节
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

								if ( Inst.length == 0 )
								{
									break;
								}

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
											AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
												nCnt, 
												OriginFunction[i],
												0,
												NotHookAddressStart,
												nLEN,
												OriginFunction[i],
												ReloadFunction[i],
												(PCHAR)pSsdtName + i * 100);
										}

										NotHookAddressStart = AddressTemp + Inst.length;
										nNotHookLen = 0;

										AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
											nCnt, 
											OriginFunction[i],
											HookAddress,
											AddressTemp,
											Inst.length,
											OriginFunction[i],
											ReloadFunction[i],
											(PCHAR)pSsdtName + i * 100);
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
								AddExportFunctionsPatchsItem(pModifyCodeHookInfo,
									nCnt, 
									OriginFunction[i],
									0,
									pDifAddress,
									nDifCnt,
									OriginFunction[i],
									ReloadFunction[i],
									(PCHAR)pSsdtName + i * 100);
							}
						}

						nDifCnt = 0;
					}	
				}
			}
		}			
		__except(1)
		{
		}
	}

	return 0;
}

//
// 获得SSDT函数Inline hook信息
//
NTSTATUS EnumSsdtInlineHookInfo(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PALL_EXPORT_FUNCTION_PATCHS pModifyCodeHookInfo = (PALL_EXPORT_FUNCTION_PATCHS)pOutBuffer;
	ULONG nCnt = (uOutSize - sizeof(ALL_EXPORT_FUNCTION_PATCHS)) / sizeof(EXPORT_FUNCTION_PATCH_INFO);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PULONG pOriginFunction = (PULONG)GetGlobalVeriable(enumOriginKiServiceTable);
	PULONG pReloadFunction = (PULONG)GetGlobalVeriable(enumReloadKiServiceTable);
	ULONG SsdtCnt = GetGlobalVeriable(enumSsdtFunctionsCnt);
	
	if (!pInBuffer ||
		uInSize != sizeof(OPERATE_TYPE) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_EXPORT_FUNCTION_PATCHS))
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	if ((ULONG)pReloadFunction > SYSTEM_ADDRESS_START && 
		(ULONG)pOriginFunction > SYSTEM_ADDRESS_START &&
		SsdtCnt > 0)
	{
		KdPrint(("EnumSsdtInlineHookInfo\n"));
		PspEnumSsdtInlineHookInfo(pOriginFunction, pReloadFunction, SsdtCnt, pModifyCodeHookInfo, nCnt);

		if (pModifyCodeHookInfo->nCnt <= nCnt)
		{
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	return status;
}

void AddShadowInlineHookItem(PALL_SHADOW_INLINE pModifyCodeHookInfo, 
							 ULONG nCnt,
							 ULONG nIndex,
							 ULONG OriginFunction,
							 ULONG HookAddress,
							 ULONG pDifAddress,
							 ULONG nDifCnt,
							 ULONG ReloadFunction)
{
	ULONG nCurCnt = pModifyCodeHookInfo->nCnt;

	if (nCnt > nCurCnt)
	{
		pModifyCodeHookInfo->ShadowHooks[nCurCnt].nIndex = nIndex;
		pModifyCodeHookInfo->ShadowHooks[nCurCnt].FunctionAddress = OriginFunction;
		pModifyCodeHookInfo->ShadowHooks[nCurCnt].HookAddress = HookAddress;
		pModifyCodeHookInfo->ShadowHooks[nCurCnt].PatchedAddress = pDifAddress;
		pModifyCodeHookInfo->ShadowHooks[nCurCnt].PatchLen = nDifCnt;

		MemCpy(pModifyCodeHookInfo->ShadowHooks[nCurCnt].NowBytes, (PVOID)pDifAddress, nDifCnt);
		MemCpy(pModifyCodeHookInfo->ShadowHooks[nCurCnt].OriginBytes, (PVOID)(pDifAddress - OriginFunction + ReloadFunction), nDifCnt);
	}
	
	pModifyCodeHookInfo->nCnt++;
}

ULONG PspEnumShadowSsdtInlineHookInfo(PULONG OriginFunction, 
									  PULONG ReloadFunction, 
									  ULONG SsdtCnt, 
									  PALL_SHADOW_INLINE pModifyCodeHookInfo, 
									  ULONG nCnt
									  )
{
	ULONG i = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid ||
		!OriginFunction ||
		!ReloadFunction ||
		!SsdtCnt ||
		!pModifyCodeHookInfo ||
		!MzfMmIsAddressValid(OriginFunction) ||
		!MzfMmIsAddressValid(ReloadFunction) )
	{
		return 0;
	}

	for (i = 0; i < SsdtCnt; i++)
	{
		__try
		{
			if (MzfMmIsAddressValid((PVOID)(OriginFunction + i * sizeof(ULONG)))	&&
				MzfMmIsAddressValid((PVOID)OriginFunction[i])						&& 
				MzfMmIsAddressValid((PVOID)(ReloadFunction + i * sizeof(ULONG)))	&&
				MzfMmIsAddressValid((PVOID)ReloadFunction[i]) )
			{
				ULONG FunctionSize = GetSizeOfFunction(ReloadFunction[i]);
				ULONG j = 0, nDifCnt = 0, pDifAddress = 0;

				for (j = 0; j < FunctionSize; j++)
				{
					if ( *((PBYTE)OriginFunction[i] + j) != *((PBYTE)ReloadFunction[i] + j)  &&
						*(PULONG)(ReloadFunction[i] + j) != 0 &&
						*(PULONG)(ReloadFunction[i] + sizeof(ULONG) + j) != 0 )
					{
						if (!nDifCnt)
						{
							pDifAddress = (ULONG)OriginFunction[i] + j;
							KdPrint(("%d, 0x%08X\n", i, pDifAddress));
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
									AddShadowInlineHookItem(pModifyCodeHookInfo,
										nCnt, 
										i,
										OriginFunction[i],
										HookAddress,
										pDifAddress,
										nDifCnt,
										ReloadFunction[i]);
								}
							}
							else
							{
								if (nDifCnt < 5)
								{
									memset(&Inst, 0, sizeof(INSTRUCTION));
									get_instruction(&Inst, (PBYTE)pDifAddress - (5 - nDifCnt), MODE_32);

									if (Inst.type == INSTRUCTION_TYPE_CALL ||
										Inst.type == INSTRUCTION_TYPE_JMP ||
										Inst.type == INSTRUCTION_TYPE_PUSH ||
										Inst.type == INSTRUCTION_TYPE_MOV)
									{
										HookAddress = GetInlineHookAddress(pDifAddress - (5 - nDifCnt), nDifCnt);
										if (HookAddress)
										{
											AddShadowInlineHookItem(pModifyCodeHookInfo,
												nCnt, 
												i,
												OriginFunction[i],
												HookAddress,
												pDifAddress,
												nDifCnt,
												ReloadFunction[i]);
										}
									}	
								}
							}

							if (!HookAddress)
							{
								AddShadowInlineHookItem(pModifyCodeHookInfo,
									nCnt, 
									i,
									OriginFunction[i],
									0,
									pDifAddress,
									nDifCnt,
									ReloadFunction[i]);
							}
						}

						// dif 大于5
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
									
								if ( Inst.length == 0 )
								{
									break;
								}

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
											AddShadowInlineHookItem(pModifyCodeHookInfo,
												nCnt, 
												i,
												OriginFunction[i],
												0,
												NotHookAddressStart,
												nLEN,
												ReloadFunction[i]);
										}

										NotHookAddressStart = AddressTemp + Inst.length;
										nNotHookLen = 0;

										AddShadowInlineHookItem(pModifyCodeHookInfo,
											nCnt, 
											i,
											OriginFunction[i],
											HookAddress,
											AddressTemp,
											Inst.length,
											ReloadFunction[i]);
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
								AddShadowInlineHookItem(pModifyCodeHookInfo,
									nCnt, 
									i,
									OriginFunction[i],
									0,
									pDifAddress,
									nDifCnt,
									ReloadFunction[i]);
							}
						}

						nDifCnt = 0;
					}	
				}

			}
		}
		__except(1)
		{
		}
	}

	return 0;
}

//
// 枚举Shadow表的inline hook
//
NTSTATUS EnumShadowSsdtInlineHookInfo(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PALL_SHADOW_INLINE pModifyCodeHookInfo = (PALL_SHADOW_INLINE)pOutBuffer;
	ULONG nCnt = (uOutSize - sizeof(ALL_SHADOW_INLINE)) / sizeof(SHADOW_INLINE_INFO);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PULONG pOriginFunction = (PULONG)GetGlobalVeriable(enumOriginShadowKiServiceTable);
	PULONG pReloadFunction = (PULONG)GetGlobalVeriable(enumReloadShadowKiServiceTable);
	ULONG SsdtCnt = GetGlobalVeriable(enumShadowSsdtFunctionsCnt);
	
	if (!pInBuffer ||
		uInSize != sizeof(OPERATE_TYPE) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_SHADOW_INLINE))
	{
		return STATUS_INVALID_PARAMETER;
	}

	if ((ULONG)pReloadFunction > SYSTEM_ADDRESS_START && 
		(ULONG)pOriginFunction > SYSTEM_ADDRESS_START &&
		SsdtCnt > 0)
	{
		KdPrint(("EnumShadowSsdtInlineHookInfo\n"));

		PspEnumShadowSsdtInlineHookInfo(pOriginFunction, pReloadFunction, SsdtCnt, pModifyCodeHookInfo, nCnt);

		if (pModifyCodeHookInfo->nCnt <= nCnt)
		{
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	return status;
}

void AddModulePatchItem(PALL_OF_MODULE_PATCHS pModifyCodeHookInfo, 
						ULONG nCnt,
						ULONG HookAddress,
						ULONG pDifAddress,
						ULONG nDifCnt,
						ULONG OriginBase,
						ULONG ReloadBase)
{
	ULONG nCurCnt = pModifyCodeHookInfo->nCnt;
	
	if (nCnt > nCurCnt)
	{
		pModifyCodeHookInfo->ModulePatchs[nCurCnt].HookAddress = HookAddress;
		pModifyCodeHookInfo->ModulePatchs[nCurCnt].PatchedAddress = pDifAddress;
		pModifyCodeHookInfo->ModulePatchs[nCurCnt].PatchLen = nDifCnt;

		MemCpy(pModifyCodeHookInfo->ModulePatchs[nCurCnt].NowBytes, (PVOID)pDifAddress, nDifCnt);
		MemCpy(pModifyCodeHookInfo->ModulePatchs[nCurCnt].OriginBytes, (PVOID)(pDifAddress - OriginBase + ReloadBase), nDifCnt);
	}

	pModifyCodeHookInfo->nCnt++;
}

ULONG KepEnumModulePatchs(ULONG OriginBase, 
						ULONG ReloadBase, 
						ULONG Size, 
						PALL_OF_MODULE_PATCHS pModifyCodeHookInfo, 
						ULONG nCnt)
{
	PIMAGE_EXPORT_DIRECTORY ReloadImageExportDirectory = NULL;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	ULONG ExportSize = 0;
	PULONG pReloadAddressOfNames = NULL;
	PUSHORT pReloadAddressOfNameOrdinals = NULL;
	PULONG ReloadAddressOfFunctionsRVA = 0;
	ULONG i = 0;
	PIMAGE_SECTION_HEADER NtSection = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);
	CHAR szINIT[] = {'I','N','I','T','\0'};

	if (!MzfMmIsAddressValid		||
		!MzfRtlImageDirectoryEntryToData ||
		!OriginBase					|| 
		!ReloadBase					|| 
		!pModifyCodeHookInfo		
		)
	{
		return 0;
	}

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)ReloadBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return 0;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)ReloadBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return 0;
		}

		ReloadImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)MzfRtlImageDirectoryEntryToData((PVOID)ReloadBase, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize);
		if (ReloadImageExportDirectory == NULL)
		{
			return 0;
		}

		NtSection = IMAGE_FIRST_SECTION( ImageNtHeaders );
		if (NtSection == NULL)
		{
			return 0;
		}

		for (i = 0; i < ImageNtHeaders->FileHeader.NumberOfSections && i <= 5; i++) 
		{
			if ( ((NtSection->Characteristics & IMAGE_SCN_CNT_CODE) == IMAGE_SCN_CNT_CODE) && // 标志是0x20
				_strnicmp(NtSection->Name, szINIT, strlen(szINIT)) )  // 不在初始化段中
			{
				ULONG ReloadAddress = NtSection->VirtualAddress + ReloadBase;
				ULONG OriginAddress = NtSection->VirtualAddress + OriginBase;
				ULONG Size = NtSection->Misc.VirtualSize;

				if (Size > 0 &&
					ReloadAddress > SYSTEM_ADDRESS_START &&
					OriginAddress > SYSTEM_ADDRESS_START &&
					MzfMmIsAddressValid((PVOID)ReloadAddress) &&
					MzfMmIsAddressValid((PVOID)OriginAddress))
				{
					ULONG n = 0, nDifCnt = 0, pDifAddress = 0;;

					for (n = 0;
						n < Size && MzfMmIsAddressValid((PVOID)(ReloadAddress + n)) && MzfMmIsAddressValid((PVOID)(OriginAddress + n)); 
						n++)
					{
						if ( *((PBYTE)ReloadAddress + n) != *((PBYTE)OriginAddress + n) &&
							*(PULONG)(ReloadAddress + n) != 0 &&
							*(PULONG)(ReloadAddress + sizeof(ULONG) + n) != 0) 
						{
							if (!nDifCnt)
							{
								pDifAddress = (ULONG)OriginAddress + n;
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
										AddModulePatchItem(pModifyCodeHookInfo, 
											nCnt,
											HookAddress, 
											pDifAddress,
											nDifCnt,
											OriginBase, 
											ReloadBase
											);
									}
								}
								else
								{
									if (nDifCnt < 5)
									{
										memset(&Inst, 0, sizeof(INSTRUCTION));
										get_instruction(&Inst, (PBYTE)pDifAddress - (5 - nDifCnt), MODE_32);

										if (Inst.type == INSTRUCTION_TYPE_CALL ||
											Inst.type == INSTRUCTION_TYPE_JMP ||
											Inst.type == INSTRUCTION_TYPE_PUSH ||
											Inst.type == INSTRUCTION_TYPE_MOV)
										{
											HookAddress = GetInlineHookAddress(pDifAddress - (5 - nDifCnt), nDifCnt);
											if (HookAddress)
											{
												AddModulePatchItem(pModifyCodeHookInfo, 
													nCnt,
													HookAddress, 
													pDifAddress,
													nDifCnt,
													OriginBase, 
													ReloadBase
													);
											}
										}	
									}
								}

								if (!HookAddress)
								{
									AddModulePatchItem(pModifyCodeHookInfo, 
										nCnt,
										0, 
										pDifAddress,
										nDifCnt,
										OriginBase, 
										ReloadBase
										);
								}
							}

							// 如果大于5字节
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

									if ( Inst.length == 0 )
									{
										break;
									}

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
												AddModulePatchItem(pModifyCodeHookInfo, 
													nCnt,
													0, 
													NotHookAddressStart,
													nLEN,
													OriginBase, 
													ReloadBase
													);
											}

											NotHookAddressStart = AddressTemp + Inst.length;
											nNotHookLen = 0;

											AddModulePatchItem(pModifyCodeHookInfo, 
												nCnt,
												HookAddress, 
												AddressTemp,
												Inst.length,
												OriginBase, 
												ReloadBase
												);
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
									AddModulePatchItem(pModifyCodeHookInfo, 
										nCnt,
										0, 
										pDifAddress,
										nDifCnt,
										OriginBase, 
										ReloadBase
										);
								}
							}

							nDifCnt = 0;
						}	
					}	
				}
			}

			++NtSection;
		}
	}
	__except(1)
	{
	}

	return 0;
}

//
// 枚举整个模块的代码段patchs
//
NTSTATUS EnumModulePatchs(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PALL_OF_MODULE_PATCHS pModifyCodeHookInfo = (PALL_OF_MODULE_PATCHS)pOutBuffer;
	ULONG nCnt = (uOutSize - sizeof(ALL_OF_MODULE_PATCHS)) / sizeof(MODULE_PATCH_INFO);
	PCOMMUNICATE_EXPORT_FUNCTION_HOOK pModifyCodeHookInput = (PCOMMUNICATE_EXPORT_FUNCTION_HOOK)pInBuffer;
	ULONG Base = 0, Size = 0, NewImageBase = 0;
	WCHAR *szPath = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	
	if (!MzfMmIsAddressValid ||
		!MzfExFreePoolWithTag)
	{
		return status;
	}
	
	if (!pInBuffer || 
		uInSize != sizeof(COMMUNICATE_EXPORT_FUNCTION_HOOK) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_OF_MODULE_PATCHS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	Base = pModifyCodeHookInput->op.Get.Base;
	Size = pModifyCodeHookInput->op.Get.Size;
	szPath = pModifyCodeHookInput->op.Get.szPath;
	
	if (Size == 0									||
		Base == 0									||
		Size > SYSTEM_ADDRESS_START					||
		Base < SYSTEM_ADDRESS_START					||
		Base + Size < SYSTEM_ADDRESS_START			||
		!MzfMmIsAddressValid((PVOID)Base)			||
		!MzfMmIsAddressValid((PVOID)(Base+Size-1))	||
		wcslen(szPath) <= 3)
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (PeLoad(szPath, &NewImageBase, Base) && 
		NewImageBase > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)NewImageBase))
	{
		KepEnumModulePatchs(
			Base, 
			NewImageBase, 
			Size, 
			pModifyCodeHookInfo, 
			nCnt
			);
	}

	if (pModifyCodeHookInfo->nCnt <= nCnt)
	{
		status = STATUS_SUCCESS;
	}
	else
	{	
		status = STATUS_BUFFER_TOO_SMALL;
	}

	if (NewImageBase)
	{
		MzfExFreePoolWithTag((PVOID)NewImageBase, 0);
		NewImageBase = 0;
	}

	return status;
}

NTSTATUS RestoreModifiedCode(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_MODIFY_CODE pModifyCodeInfo = (PCOMMUNICATE_MODIFY_CODE)pInBuffer;
	ULONG PatchAddress = pModifyCodeInfo->op.Restore.ModifyCodeInfo.PatchedAddress;
	ULONG nLen = pModifyCodeInfo->op.Restore.ModifyCodeInfo.PatchLen;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid &&
		PatchAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)PatchAddress) &&
		nLen > 0 &&
		nLen <= MAX_PATCHED_LEN)
	{
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
		WPOFF();
		memcpy((PVOID)PatchAddress, pModifyCodeInfo->op.Restore.ModifyCodeInfo.OriginBytes, nLen);
		WPON();
		KeLowerIrql(OldIrql);
		status = STATUS_SUCCESS;
	}

	return status;
}