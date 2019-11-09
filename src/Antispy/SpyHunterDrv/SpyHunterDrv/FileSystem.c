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
#include "FileSystem.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Peload.h"
#include "libdasm.h"

void IninNtfs()
{
	CHAR szModuleName[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
	CHAR szFunctionName[] = {'M','m','F','l','u','s','h','I','m','a','g','e','S','e','c','t','i','o','n','\0'};
	WCHAR szFileSystem[] = {'\\','F','i','l','e','S','y','s','t','e','m','\\','\0'};
	WCHAR szDriverDir[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'};
	ULONG NtfsBase = GetGlobalVeriable(enumNtfsBase);
	ULONG NtfsSize = GetGlobalVeriable(enumNtfsSize);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (NtfsBase && NtfsSize && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		WCHAR szNtfsPath[MAX_PATH] = {0};
		WCHAR szNtfsSys[] = {'n','t','f','s','.','s','y','s','\0'};
		WCHAR szNtfs[] = {'n','t','f','s','\0'};
		ULONG NewBase = 0;
		ULONG NtfsImpMmFlushImageSection = 0;
		WCHAR szFileSystemNtfs[MAX_PATH] = {0};
		PDRIVER_OBJECT pDriverObject = NULL;
		
		wcscpy(szFileSystemNtfs, szFileSystem);
		wcscat(szFileSystemNtfs, szNtfs);

		if (NT_SUCCESS(GetDriverObjectByName(szFileSystemNtfs, &pDriverObject)))
		{
			KdPrint(("Ntfs driverobject: 0x%08X\n", pDriverObject));
			SetGlobalVeriable(enumNtfsDriverObject, (ULONG)pDriverObject);
		}
		else
		{
			KdPrint(("Get Ntfs driverobject error\n"));
		}

		wcscpy(szNtfsPath, szDriverDir);
		wcscat(szNtfsPath, szNtfsSys);

		if (PeLoad(szNtfsPath, &NewBase, NtfsBase))
		{
			ULONG Offset = 0;
			PULONG pOriginDispatchs = NULL, pReloadDispatchs = NULL;

			if (FindIatFunction((PVOID)NewBase, NtfsSize, szModuleName, szFunctionName, &Offset) && Offset)
			{
				NtfsImpMmFlushImageSection = NtfsBase + Offset;
				SetGlobalVeriable(enumNtfsImpMmFlushImageSection, NtfsImpMmFlushImageSection);
				KdPrint(("NtfsImpMmFlushImageSection: 0x%08X\n", NtfsImpMmFlushImageSection));
			}

			pOriginDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);
			pReloadDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);
			if (pOriginDispatchs && pReloadDispatchs)
			{
				ULONG pDriverEntry = 0;
				ULONG nCodeLen = 0, Address = 0;
				ULONG MovTemp = 0;
				ENTRY_POINT_TYPE EntryType = enumEntryNone;
				WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

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

				for (Address = pDriverEntry; Address < pDriverEntry + 1000; Address += nCodeLen)
				{
					INSTRUCTION	Inst;

					if (!MzfMmIsAddressValid((PVOID)Address))
					{
						break;
					}

					memset(&Inst, 0, sizeof(INSTRUCTION));
					get_instruction(&Inst, (PBYTE)Address, MODE_32);

					if (Inst.length == 0)
					{
						break;
					}
					else if (Inst.type == INSTRUCTION_TYPE_RET)
					{
						break;
					}

					if (Inst.type == INSTRUCTION_TYPE_MOV)
					{
						if (Inst.op1.type == OPERAND_TYPE_MEMORY && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
							Inst.op1.displacement >= 0x38 &&
							Inst.op1.displacement <= 0xA4)
						{
							ULONG nIndex = (Inst.op1.displacement - 0x38) / sizeof(ULONG);
							pOriginDispatchs[nIndex] = Inst.op2.immediate;
							pReloadDispatchs[nIndex] = Inst.op2.immediate - NtfsBase + NewBase;
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
							pReloadDispatchs[nIndex] = MovTemp - NtfsBase + NewBase;
						}
					}

					nCodeLen = Inst.length;
				}

				FixOriginDispatch(pOriginDispatchs);
				FixReloadDispatch(pReloadDispatchs);

				SetGlobalVeriable(enumNtfsOriginalDispatchs, (ULONG)pOriginDispatchs);
				SetGlobalVeriable(enumNtfsReloadDispatchs, (ULONG)pReloadDispatchs);
			}

			SetGlobalVeriable(enumReloadNtfsBase, NewBase);
		}
	}
}

void IninFastfat()
{
	CHAR szModuleName[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
	CHAR szFunctionName[] = {'M','m','F','l','u','s','h','I','m','a','g','e','S','e','c','t','i','o','n','\0'};
	WCHAR szFileSystem[] = {'\\','F','i','l','e','S','y','s','t','e','m','\\','\0'};
	WCHAR szDriverDir[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'};
	ULONG FastfatBase = GetGlobalVeriable(enumFastfatBase);
	ULONG FastfatSize = GetGlobalVeriable(enumFastfatSize);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (FastfatBase && FastfatSize && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		WCHAR szFastfatPath[MAX_PATH] = {0};
		WCHAR szFastfatSys[] = {'f','a','s','t','f','a','t','.','s','y','s','\0'};
		WCHAR szFastfat[] = {'f','a','s','t','f','a','t','\0'};
		ULONG NewBase = 0;
		ULONG FastfatImpMmFlushImageSection = 0;
		WCHAR szFileSystemFastfat[MAX_PATH] = {0};
		PDRIVER_OBJECT pDriverObject = NULL;

		wcscpy(szFileSystemFastfat, szFileSystem);
		wcscat(szFileSystemFastfat, szFastfat);

		if (NT_SUCCESS(GetDriverObjectByName(szFileSystemFastfat, &pDriverObject)))
		{
			KdPrint(("Fastfat driverobject: 0x%08X\n", pDriverObject));
			SetGlobalVeriable(enumFastfatDriverObject, (ULONG)pDriverObject);
		}
		else
		{
			KdPrint(("Get Fastfat driverobject error\n"));
		}

		wcscpy(szFastfatPath, szDriverDir);
		wcscat(szFastfatPath, szFastfatSys);
		if (PeLoad(szFastfatPath, &NewBase, FastfatBase))
		{
			ULONG Offset = 0;
			PULONG pOriginDispatchs = NULL, pReloadDispatchs = NULL;

			if (FindIatFunction((PVOID)NewBase, FastfatSize, szModuleName, szFunctionName, &Offset) && Offset)
			{
				FastfatImpMmFlushImageSection = FastfatBase + Offset;
				SetGlobalVeriable(enumFastfatImpMmFlushImageSection, FastfatImpMmFlushImageSection);
				KdPrint(("FastfatImpMmFlushImageSection: 0x%08X\n", FastfatImpMmFlushImageSection));
			}

			pOriginDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);
			pReloadDispatchs = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG) * 28, MZFTAG);
			if (pOriginDispatchs && pReloadDispatchs)
			{
				ULONG pDriverEntry = 0;
				ULONG nCodeLen = 0, Address = 0;
				ULONG MovTemp = 0;

				memset(pOriginDispatchs, 0, sizeof(ULONG) * 28);
				memset(pReloadDispatchs, 0, sizeof(ULONG) * 28);

				if (!GetDriverEntryPoint((PVOID)NewBase, &pDriverEntry, enumEntryPointHead))
				{
					KdPrint(("GetDriverEntryPoint error\n"));
					MzfExFreePoolWithTag(pOriginDispatchs, 0);
					MzfExFreePoolWithTag(pReloadDispatchs, 0);
					MzfExFreePoolWithTag((PVOID)NewBase, 0);
					return;
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

					if (Inst.length == 0)
					{
						break;
					}
					else if (Inst.type == INSTRUCTION_TYPE_RET)
					{
						break;
					}

					if (Inst.type == INSTRUCTION_TYPE_MOV)
					{
						if (Inst.op1.type == OPERAND_TYPE_MEMORY && 
							Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
							Inst.op1.displacement >= 0x38 &&
							Inst.op1.displacement <= 0xA4)
						{
							ULONG nIndex = (Inst.op1.displacement - 0x38) / sizeof(ULONG);
							pOriginDispatchs[nIndex] = Inst.op2.immediate;
							pReloadDispatchs[nIndex] = Inst.op2.immediate - FastfatBase + NewBase;
						}
					}
				
					nCodeLen = Inst.length;
				}

				FixOriginDispatch(pOriginDispatchs);
				FixReloadDispatch(pReloadDispatchs);

				SetGlobalVeriable(enumFastfatOriginalDispatchs, (ULONG)pOriginDispatchs);
				SetGlobalVeriable(enumFastfatReloadDispatchs, (ULONG)pReloadDispatchs);
			}

			SetGlobalVeriable(enumReloadFastfatBase, NewBase);
		}
	}
}

void InitFileSystem(FILE_SYSTEM_TYPE type)
{
	if (type == enumNtfs)
	{
		IninNtfs();
	}
	else if (type == enumFastfat)
	{
		IninFastfat();
	}
}