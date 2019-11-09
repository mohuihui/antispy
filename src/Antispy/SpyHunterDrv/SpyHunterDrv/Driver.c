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
#include "Driver.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "libdasm.h"

//
// 枚举LdrDataTableEntry链表
//
VOID EnumDriverByLdrDataTableEntry(PALL_DRIVERS pDriversInfo, ULONG nCnt)
{
	PKLDR_DATA_TABLE_ENTRY entry = NULL, firstentry = NULL;
	ULONG nMax = PAGE_SIZE;
	ULONG i = 0;
	KIRQL OldIrql;

	firstentry = entry = (PKLDR_DATA_TABLE_ENTRY)GetGlobalVeriable(enumNtoskrnl_KLDR_DATA_TABLE_ENTRY);

	if (!firstentry || !pDriversInfo)
	{
		return;
	}

	OldIrql = KeRaiseIrqlToDpcLevel();

	__try
	{
		do
		{
			if ((ULONG)entry->DllBase > SYSTEM_ADDRESS_START && entry->SizeOfImage > 0)
			{
				ULONG nCurCnt = pDriversInfo->nCnt;
				if (nCnt > nCurCnt)
				{
					pDriversInfo->Drivers[nCurCnt].DriverType = enumHaveOrder;
					pDriversInfo->Drivers[nCurCnt].nLodeOrder = i++;
					pDriversInfo->Drivers[nCurCnt].nBase = (ULONG)entry->DllBase;
					pDriversInfo->Drivers[nCurCnt].nSize = entry->SizeOfImage;

					if (IsUnicodeStringValid(&(entry->FullDllName)))
					{
						ULONG nLen = CmpAndGetStringLength(&(entry->FullDllName), MAX_PATH);
						wcsncpy(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->FullDllName.Buffer, nLen);
					}
					else if (IsUnicodeStringValid(&(entry->BaseDllName)))
					{
						ULONG nLen = CmpAndGetStringLength(&(entry->BaseDllName), MAX_PATH);
						wcsncpy(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->BaseDllName.Buffer, nLen);
					}
				}

				pDriversInfo->nCnt++;
			}

			entry = (PKLDR_DATA_TABLE_ENTRY)entry->InLoadOrderLinks.Flink;

		}while(entry && entry != firstentry && nMax--);	
	}
	__except(1)
	{}

	KeLowerIrql(OldIrql);
}

//
// 判断驱动是否已经在链表里面了
//
BOOL IsDriverInList(PALL_DRIVERS pDriversInfo, PDRIVER_OBJECT pDriverObject, ULONG nCnt)
{
	BOOL bIn = TRUE, bFind = FALSE;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	

	if (!pDriversInfo			|| 
		!pDriverObject			|| 
		!MzfMmIsAddressValid	|| 
		!MzfMmIsAddressValid(pDriverObject))
	{
		return TRUE;
	}
	
	__try
	{
		if (IsAddressValid(pDriverObject, sizeof(DRIVER_OBJECT)))
		{
			PKLDR_DATA_TABLE_ENTRY entry = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;

			if (entry &&
				MzfMmIsAddressValid(entry) && 
				MzfMmIsAddressValid(entry->DllBase) &&
				(ULONG)entry->DllBase > SYSTEM_ADDRESS_START)
			{
				ULONG i = 0;
				ULONG nCntTemp = nCnt > pDriversInfo->nCnt ? pDriversInfo->nCnt : nCnt;
				
				for (i = 0; i < nCntTemp; i++)
				{
					if (pDriversInfo->Drivers[i].nBase == (ULONG)entry->DllBase)
					{
						if (pDriversInfo->Drivers[i].nDriverObject == 0)
						{
							pDriversInfo->Drivers[i].nDriverObject = (ULONG)pDriverObject;
						}

						bFind = TRUE;
						break;
					}
				}

				if (!bFind)
				{
					bIn = FALSE; 
				}
			}
		}
	}
	__except(1)
	{
		bIn = TRUE;
	}
	
	return bIn;
}

void InsertDriver(PALL_DRIVERS pDriversInfo, PDRIVER_OBJECT pDriverObject, ULONG nCnt)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!pDriversInfo || !pDriverObject || !MzfMmIsAddressValid || !MzfMmIsAddressValid(pDriverObject))
	{
		return;
	}
	else
	{
		PKLDR_DATA_TABLE_ENTRY entry = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;

		if (entry &&
			MzfMmIsAddressValid(entry) && 
			MzfMmIsAddressValid(entry->DllBase) &&
			(ULONG)entry->DllBase > SYSTEM_ADDRESS_START)
		{
			ULONG nCurCnt = pDriversInfo->nCnt;
			if (nCnt > nCurCnt)
			{
				pDriversInfo->Drivers[nCurCnt].DriverType = enumHide;
				pDriversInfo->Drivers[nCurCnt].nLodeOrder = 0x10000;
				pDriversInfo->Drivers[nCurCnt].nBase = (ULONG)entry->DllBase;
				pDriversInfo->Drivers[nCurCnt].nSize = entry->SizeOfImage;
				pDriversInfo->Drivers[nCurCnt].nDriverObject = (ULONG)pDriverObject;

				if (IsUnicodeStringValid(&(entry->FullDllName)))
				{
					ULONG nLen = CmpAndGetStringLength(&(entry->FullDllName), MAX_PATH);
					wcsncpy(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->FullDllName.Buffer, nLen);
				}
				else if (IsUnicodeStringValid(&(entry->BaseDllName)))
				{
					ULONG nLen = CmpAndGetStringLength(&(entry->BaseDllName), MAX_PATH);
					wcsncpy(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->BaseDllName.Buffer, nLen);
				}
			}

			pDriversInfo->nCnt++;
		}
	}
}

VOID WalkerDirectoryObject(PALL_DRIVERS pDriversInfo, PVOID pDirectoryObject, ULONG nCnt)
{
	POBJECT_TYPE DriverObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDriverObjectType);
	POBJECT_TYPE DeviceObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDeviceObjectType);
	POBJECT_TYPE DirectoryObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumDirectoryObjectType);
 	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (DriverObjectType			&&
		DeviceObjectType			&&
		DirectoryObjectType			&&
		MzfMmIsAddressValid			&&
		pDirectoryObject			&&
		pDriversInfo				&&
		MzfMmIsAddressValid(pDirectoryObject)
		)
	{
		ULONG i = 0;
		POBJECT_DIRECTORY pOd = (POBJECT_DIRECTORY)pDirectoryObject;
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
		
		__try
		{
			for (i = 0; i < NUMBER_HASH_BUCKETS; i++)
			{
				POBJECT_DIRECTORY_ENTRY pode = pOd->HashBuckets[i];
				for (; (ULONG)pode > SYSTEM_ADDRESS_START && MzfMmIsAddressValid(pode); pode = pode->ChainLink)
				{
					if (MzfMmIsAddressValid(pode->Object))
					{
						POBJECT_TYPE pType = KeGetObjectType(pode->Object);
						
						//
						// 如果是目录，那么继续递归遍历
						//
						if (pType == DirectoryObjectType)
						{
							WalkerDirectoryObject( pDriversInfo, pode->Object, nCnt );
						}

						//
						// 如果是驱动对象
						//
						else if (pType == DriverObjectType)
						{
							PDEVICE_OBJECT DeviceObject = NULL;

							if (!IsDriverInList( pDriversInfo, (PDRIVER_OBJECT)pode->Object, nCnt ))
							{
								InsertDriver( pDriversInfo, (PDRIVER_OBJECT)pode->Object, nCnt);
							}
							
							//
							// 遍历设备栈
							//
							for (DeviceObject = ((PDRIVER_OBJECT)pode->Object)->DeviceObject; 
								DeviceObject && MzfMmIsAddressValid(DeviceObject);
								DeviceObject = DeviceObject->AttachedDevice)
							{
								if (!IsDriverInList( pDriversInfo, DeviceObject->DriverObject, nCnt ))
								{
									InsertDriver( pDriversInfo, DeviceObject->DriverObject, nCnt);
								}
							}
						}

						//
						// 如果是设备对象
						//
						else if (pType == DeviceObjectType)
						{
							PDEVICE_OBJECT DeviceObject = NULL;

							if (!IsDriverInList(pDriversInfo, ((PDEVICE_OBJECT)pode->Object)->DriverObject, nCnt))
							{
								InsertDriver(pDriversInfo, ((PDEVICE_OBJECT)pode->Object)->DriverObject, nCnt);
							}

							//
							// 遍历设备栈
							//
							for (DeviceObject = ((PDEVICE_OBJECT)pode->Object)->AttachedDevice; 
								DeviceObject && MzfMmIsAddressValid(DeviceObject);
								DeviceObject = DeviceObject->AttachedDevice)
							{
								if (!IsDriverInList( pDriversInfo, DeviceObject->DriverObject, nCnt ))
								{
									InsertDriver( pDriversInfo, DeviceObject->DriverObject, nCnt );
								}
							}
						}
					}
				}
			}
		}
		__except(1)
		{
		}
	
		KeLowerIrql(OldIrql);
	}
}

//
// 遍历对象目录来枚举驱动对象
//
void EnumDriversByWalkerDirectoryObject(PALL_DRIVERS pDriversInfo, ULONG nCnt)
{	
	NTSTATUS ntStatus;
	OBJECT_ATTRIBUTES objectAttributes; 
	UNICODE_STRING unDirectory; 
	HANDLE hDirectory;
	PVOID pDirectoryObject = NULL;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	WCHAR szDirectory[] = {L'\\', L'\0'};
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	pfnNtOpenDirectoryObject MzfNtOpenDirectoryObject = (pfnNtOpenDirectoryObject)GetGlobalVeriable(enumNtOpenDirectoryObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfNtOpenDirectoryObject || 
		!MzfPsGetCurrentThread || 
		!MzfNtClose || 
		!MzfMmIsAddressValid || 
		!MzfObReferenceObjectByHandle || 
		!MzfObfDereferenceObject)
	{
		return;
	}

	MzfInitUnicodeString(&unDirectory, szDirectory);
	InitializeObjectAttributes(&objectAttributes, &unDirectory, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	
	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	ntStatus = MzfNtOpenDirectoryObject(&hDirectory, 0, &objectAttributes);
	if ( NT_SUCCESS(ntStatus) )
	{
		ntStatus = MzfObReferenceObjectByHandle(hDirectory, 0x10000000, 0, 0, &pDirectoryObject, 0);
		if ( NT_SUCCESS(ntStatus) )
		{
			if (!GetGlobalVeriable(enumDirectoryObjectType))
			{
				POBJECT_TYPE DirectoryObjectType = KeGetObjectType(pDirectoryObject);
				if (DirectoryObjectType)
				{
					SetGlobalVeriable(enumDirectoryObjectType, (ULONG)DirectoryObjectType);
				}
			}

			WalkerDirectoryObject(pDriversInfo, pDirectoryObject, nCnt);
			MzfObfDereferenceObject(pDirectoryObject);
		}

		ntStatus = MzfNtClose(hDirectory);
	}
	
	RecoverPreMode(pThread, PreMode);
}

ULONG GetMmPfnDatabase()
{
	ULONG nStart = 0, nEnd = 0, nCodeLen = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	nStart = GetGlobalVeriable(enumMmGetVirtualForPhysical);
	
	if (!nStart || !MzfMmIsAddressValid)
	{
		return 0;
	}
	
	nEnd = nStart + PAGE_SIZE;

	for ( ; MzfMmIsAddressValid((PVOID)nStart) && nStart < nEnd; nStart += nCodeLen )
	{
		INSTRUCTION	Inst;

		memset(&Inst, 0, sizeof(INSTRUCTION));
		get_instruction(&Inst, (PBYTE)nStart, MODE_32);

		if (Inst.length == 0 || Inst.type == INSTRUCTION_TYPE_RET)
		{
			break;
		}

		if (Inst.type == INSTRUCTION_TYPE_MOV && Inst.length == 6)
		{
// 			KdPrint(("op1.type: %d, op2.type: %d, op2.immediate: 0x%08X, displacement: 0x%08X\n", Inst.op1.type,
// 				Inst.op2.type,
// 				Inst.op2.immediate,
// 				Inst.op2.displacement));
// 
// 			break;

			if (Inst.op1.type == OPERAND_TYPE_REGISTER		&& 
				Inst.op2.type == OPERAND_TYPE_MEMORY		&& 
				Inst.op2.displacement > SYSTEM_ADDRESS_START	&&
				MzfMmIsAddressValid((PVOID)Inst.op2.displacement))
			{
				SetGlobalVeriable(enumMmPfnDatabase, Inst.op2.displacement);
				KdPrint(("MmPfnDatabase: 0x%08X\n", Inst.op2.displacement));
				return Inst.op2.displacement;
			}
		}

		nCodeLen = Inst.length;
	}

	return 0;
}

#define PAGE_SHIFT 12L

BOOL IsPteAddressValid(PHYSICAL_ADDRESS PhysicalAddress)
{
	BOOL bRet = FALSE;
	ULONG MmPfnDatabase = GetGlobalVeriable(enumMmPfnDatabase);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MmPfnDatabase)
	{
		MmPfnDatabase = (ULONG)GetMmPfnDatabase();
	}

	if (MmPfnDatabase > SYSTEM_ADDRESS_START && PhysicalAddress.QuadPart > 0 && MzfMmIsAddressValid)
	{
		ULONG nNtBuildNumber = GetGlobalVeriable(enumBuildNumber);

		if ( nNtBuildNumber == 2195 )
		{
			bRet = TRUE;
		}
		else
		{
			/*

			sizeof(MMPFN) == 0x1C

			kd> dt _MMPFN
			nt!_MMPFN
			+0x000 u1               : <unnamed-tag>
			+0x004 u2               : <unnamed-tag>
			+0x008 PteAddress       : Ptr32 _MMPTE
			+0x008 VolatilePteAddress : Ptr32 Void
			+0x008 Lock             : Int4B
			+0x008 PteLong          : Uint4B
			+0x00c u3               : <unnamed-tag>
			+0x010 OriginalPte      : _MMPTE
			+0x010 AweReferenceCount : Int4B
			+0x018 u4               : <unnamed-tag>

			*/
			PFN_NUMBER PageFrameIndex = (PFN_NUMBER)PhysicalAddress.QuadPart >> PAGE_SHIFT;
			ULONG Pfn = 0x1C * PageFrameIndex;
			ULONG pPteAddress = 0;
			
			if ( Pfn > (ULONG)PageFrameIndex )
			{
				if ( nNtBuildNumber >= 6000 )
				{
					pPteAddress = Pfn + 8; // +0x008 PteAddress       : Ptr32 _MMPTE
				}
				else
				{
					pPteAddress = Pfn + 4;
				}

				if ( pPteAddress > (ULONG)PageFrameIndex )
				{
					ULONG PteAddress = MmPfnDatabase + pPteAddress;
					if ( PteAddress > MmPfnDatabase )
					{
						bRet = MzfMmIsAddressValid((PVOID)PteAddress);
					}
				}
			}
		}
	}

	return bRet;
}

// VOID EnumDriverBySearchMemoryToFindDriverObject(PDRIVERS pDriversInfo)
// {
// 	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
// 	ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);
// 	POBJECT_TYPE DriverObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDriverObjectType);
// 	SYSINF_PAGE_COUNT NumberOfPhysicalPages = 0, HighestPhysicalPageNumber = 0;
// 	LARGE_INTEGER nMaxPhysicalPageSize;
// 	ULONG Address = 0;
// 	pfnMmIsNonPagedSystemAddressValid MzfMmIsNonPagedSystemAddressValid = (pfnMmIsNonPagedSystemAddressValid)GetGlobalVeriable(enumMmIsNonPagedSystemAddressValid);
// 	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
// 	pfnMmGetPhysicalAddress MzfMmGetPhysicalAddress = (pfnMmGetPhysicalAddress)GetGlobalVeriable(enumMmGetPhysicalAddress);
// 	pfnMmGetVirtualForPhysical MzfMmGetVirtualForPhysical = (pfnMmGetVirtualForPhysical)GetGlobalVeriable(enumMmGetVirtualForPhysical);
// 	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
// 	pfnIoAllocateMdl MzfIoAllocateMdl = (pfnIoAllocateMdl)GetGlobalVeriable(enumIoAllocateMdl);
// 	pfnMmBuildMdlForNonPagedPool MzfMmBuildMdlForNonPagedPool = (pfnMmBuildMdlForNonPagedPool)GetGlobalVeriable(enumMmBuildMdlForNonPagedPool);
// 	pfnMmMapLockedPagesSpecifyCache MzfMmMapLockedPagesSpecifyCache = (pfnMmMapLockedPagesSpecifyCache)GetGlobalVeriable(enumMmMapLockedPagesSpecifyCache);
// 	pfnMmUnmapLockedPages MzfMmUnmapLockedPages = (pfnMmUnmapLockedPages)GetGlobalVeriable(enumMmUnmapLockedPages);
// 	pfnIoFreeMdl MzfIoFreeMdl = (pfnIoFreeMdl)GetGlobalVeriable(enumIoFreeMdl);
// 	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
// 
// 	if (KernelBase &&
// 		KernelSize &&
// 		DriverObjectType &&
// 		MzfMmIsNonPagedSystemAddressValid &&
// 		MzfNtQuerySystemInformation &&
// 		MzfMmGetPhysicalAddress &&
// 		MzfMmGetVirtualForPhysical &&
// 		MzfMmIsAddressValid &&
// 		MzfIoAllocateMdl &&
// 		MzfMmBuildMdlForNonPagedPool &&
// 		MzfMmMapLockedPagesSpecifyCache &&
// 		MzfMmMapLockedPagesSpecifyCache &&
// 		MzfMmUnmapLockedPages &&
// 		MzfIoFreeMdl &&
// 		MzfPsGetCurrentThread)
// 	{
// 		PETHREAD pThread = MzfPsGetCurrentThread();
// 		CHAR PreMode = ChangePreMode(pThread);
// 		SYSTEM_BASIC_INFORMATION sbi;
// 		NTSTATUS status;
// 		
// 		nMaxPhysicalPageSize.QuadPart = 0;
// 
//  		KdPrint(("enter EnumDriverBySearchMemory\n"));
// 		memset(&sbi, 0, sizeof(SYSTEM_BASIC_INFORMATION));
// 		status = MzfNtQuerySystemInformation(SystemBasicInformation, &sbi, sizeof(SYSTEM_BASIC_INFORMATION), NULL);
// 		if (NT_SUCCESS(status))
// 		{
// 			ULONG nObjectHeaderSize = GetGlobalVeriable(enumObjectHeaderSize);
// 			ULONG nSizeOfDriverObject = sizeof(DRIVER_OBJECT) + nObjectHeaderSize;
// 
// 			NumberOfPhysicalPages = sbi.NumberOfPhysicalPages;
// 			HighestPhysicalPageNumber = sbi.HighestPhysicalPageNumber;
// 			if (NumberOfPhysicalPages > HighestPhysicalPageNumber)
// 			{
// 				NumberOfPhysicalPages = HighestPhysicalPageNumber;
// 			}
// 
// 			KdPrint(("NumberOfPhysicalPages, %d, LowestPhysicalPageNumber: %d, HighestPhysicalPageNumber: %d, sbi.PageSize: %d\n", sbi.NumberOfPhysicalPages, sbi.LowestPhysicalPageNumber, sbi.HighestPhysicalPageNumber, sbi.PageSize));
// 
// 			nMaxPhysicalPageSize.QuadPart = NumberOfPhysicalPages * sbi.PageSize;
// 			Address = (KernelSize + KernelBase) & 0xFFFFF000;
// 
// 			__try
// 			{
// 				ULONG i = 0;
// 				while ( Address < 0xFFFF0000 )
// 				{
// 					if (MzfMmIsAddressValid((PVOID)Address) &&
// 						MzfMmIsNonPagedSystemAddressValid((PVOID)Address))
// 					{
// 						PHYSICAL_ADDRESS PhysicalAddress = MzfMmGetPhysicalAddress((PVOID)Address);
// 
// 						if ( (PhysicalAddress.HighPart || PhysicalAddress.LowPart) &&
// 							PhysicalAddress.QuadPart < nMaxPhysicalPageSize.QuadPart)
// 						{
// 							if (!IsPteAddressValid(PhysicalAddress))
// 							{
// 								break;
// 							}
// 
// 							if (MzfMmGetVirtualForPhysical(PhysicalAddress) == (PVOID)Address)
// 							{
// 								PMDL pMdl = MzfIoAllocateMdl((PVOID)Address, PAGE_SIZE, FALSE, FALSE, NULL);
// 								if (pMdl)
// 								{
// 									PVOID pMdlAddress = NULL;
// 									ULONG nRemainSize = 0, pDriverObject = 0;
// 									ULONG nPageSize = 0xFE8 - sizeof(DRIVER_OBJECT);
// 									MzfMmBuildMdlForNonPagedPool(pMdl);
// 									pMdlAddress = MzfMmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, 0, NormalPagePriority);
// 									if (pMdlAddress)
// 									{
// 										nRemainSize = 0;
// 										pDriverObject = (ULONG)pMdlAddress + nObjectHeaderSize;
// 
// 										do
// 										{
// 											if (KeGetObjectType((PVOID)pDriverObject) == DriverObjectType && 
// 												IsRealDriverObject((PDRIVER_OBJECT)pDriverObject))
// 											{
// 												if (!IsDriverInList( pDriversInfo, (PDRIVER_OBJECT)pDriverObject ))
// 												{
// 													InsertDriver( pDriversInfo, (PDRIVER_OBJECT)pDriverObject);
// 												}
// 
// // 												KdPrint(("%d   DriverObject: 0x%08X\n", i++, Address + nRemainSize + 0x18));
// 												nRemainSize += nSizeOfDriverObject;
// 												pDriverObject += nSizeOfDriverObject;
// 											}
// 
// 											nRemainSize += 8;
// 											pDriverObject += 8;
// 
// 										}while ( nRemainSize <= nPageSize );
// 
// 										MzfMmUnmapLockedPages(pMdlAddress, pMdl);
// 									}
// 
// 									MzfIoFreeMdl(pMdl);
// 								}
// 							}
// 						}
// 					}
// 
// 					Address += PAGE_SIZE;
// 				}
// 			}
// 			__except(1)
// 			{
// 			}
// 		}
// 
// 		RecoverPreMode(pThread, PreMode);
// 	}
// }

// BOOL GetDriverEndAddress(IN PDRIVERS pDriversInfo, 
// 						 IN ULONG pAddress, 
// 						 OUT ULONG *pDriverEndAddress)
// {
// 	if (!pDriversInfo || !pAddress || !pDriverEndAddress)
// 	{
// 		pDriverEndAddress = 0;
// 		return TRUE;
// 	}
// 	else
// 	{
// 		ULONG i = 0;
// 		ULONG nCnt = pDriversInfo->nCnt > pDriversInfo->nRetCnt ? pDriversInfo->nRetCnt : pDriversInfo->nCnt;
// 
// 		for (i = 0; i < nCnt; i++)
// 		{
// 			ULONG Base = pDriversInfo->Drivers[i].nBase;
// 			ULONG End = pDriversInfo->Drivers[i].nBase + pDriversInfo->Drivers[i].nSize;
// 
// 			if (pAddress >= Base && pAddress <= End)
// 			{
// 				*pDriverEndAddress = End & 0xFFFFF000;
// 				return TRUE;
// 			}
// 		}
// 	}
// 
// 	return FALSE;
// }

BOOL GetReloadModulesEndAddress(IN ULONG pAddress, 
								OUT ULONG *pDriverEndAddress)
{
	if ( !pAddress || !pDriverEndAddress )
	{
		return TRUE;
	}
	else
	{
		ULONG ReloadNtoskrnl = GetGlobalVeriable(enumNewKernelBase);
		ULONG NtoskrnlSize = GetGlobalVeriable(enumKernelModuleSize);
		ULONG ReloadWin32k = GetGlobalVeriable(enumNewWin32kBase);
		ULONG Win32kSize = GetGlobalVeriable(enumWin32kSize);
		
		ULONG Base = ReloadNtoskrnl;
		ULONG End = ReloadNtoskrnl + NtoskrnlSize;

		if (pAddress >= Base && pAddress <= End)
		{
			*pDriverEndAddress = End & 0xFFFFF000;
			return TRUE;
		}

		Base = ReloadWin32k;
		End = ReloadWin32k + Win32kSize;
		if (pAddress >= Base && pAddress <= End)
		{
			*pDriverEndAddress = End & 0xFFFFF000;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL IsDriverPe(ULONG pAddress, ULONG *SizeOfImage)
{
	BOOL bDriver = FALSE;
	PIMAGE_DOS_HEADER       PEDosHeader = NULL;
	PIMAGE_FILE_HEADER      PEFileHeader = NULL;
	PIMAGE_OPTIONAL_HEADER  PEOptionalHeader = NULL;
	PIMAGE_NT_HEADERS       PENtHeader = NULL;

	__try
	{
		PEDosHeader      = (PIMAGE_DOS_HEADER) pAddress;

		if (PEDosHeader->e_magic == IMAGE_DOS_SIGNATURE &&
			PEDosHeader->e_lfanew < PAGE_SIZE)
		{
			PENtHeader       = (PIMAGE_NT_HEADERS)      ( (ULONG)pAddress + PEDosHeader->e_lfanew );
			PEFileHeader     = (PIMAGE_FILE_HEADER)     ( (ULONG)pAddress + PEDosHeader->e_lfanew + sizeof(ULONG) );
			PEOptionalHeader = (PIMAGE_OPTIONAL_HEADER) ( (ULONG)pAddress + PEDosHeader->e_lfanew + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) );

			if (PENtHeader->Signature == IMAGE_NT_SIGNATURE &&
				PEOptionalHeader->Subsystem == IMAGE_SUBSYSTEM_NATIVE &&
				PEFileHeader->NumberOfSections > 0)
			{
				if (SizeOfImage)
				{
					// 				KdPrint(("Base: 0x%08X, Size: 0x%08X\n", pAddress, PEOptionalHeader->SizeOfImage));
					*SizeOfImage = PEOptionalHeader->SizeOfImage;
				}

				bDriver = TRUE;
			}
		}
	}
	__except(1)
	{
		bDriver = FALSE;
	}

	return bDriver;
}

// void SearchMemoryToFindDubiousPE(PDRIVERS pDriversInfo)
// {
// 	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
// 	ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);
// 	POBJECT_TYPE DriverObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDriverObjectType);
// 	SYSINF_PAGE_COUNT NumberOfPhysicalPages = 0, HighestPhysicalPageNumber = 0, nMaxPhysicalPageSize;
// 	ULONG Address = 0;
// 	pfnMmIsNonPagedSystemAddressValid MzfMmIsNonPagedSystemAddressValid = (pfnMmIsNonPagedSystemAddressValid)GetGlobalVeriable(enumMmIsNonPagedSystemAddressValid);
// 	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
// 	pfnMmGetPhysicalAddress MzfMmGetPhysicalAddress = (pfnMmGetPhysicalAddress)GetGlobalVeriable(enumMmGetPhysicalAddress);
// 	pfnMmGetVirtualForPhysical MzfMmGetVirtualForPhysical = (pfnMmGetVirtualForPhysical)GetGlobalVeriable(enumMmGetVirtualForPhysical);
// 	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
// 	pfnIoAllocateMdl MzfIoAllocateMdl = (pfnIoAllocateMdl)GetGlobalVeriable(enumIoAllocateMdl);
// 	pfnMmBuildMdlForNonPagedPool MzfMmBuildMdlForNonPagedPool = (pfnMmBuildMdlForNonPagedPool)GetGlobalVeriable(enumMmBuildMdlForNonPagedPool);
// 	pfnMmMapLockedPagesSpecifyCache MzfMmMapLockedPagesSpecifyCache = (pfnMmMapLockedPagesSpecifyCache)GetGlobalVeriable(enumMmMapLockedPagesSpecifyCache);
// 	pfnMmUnmapLockedPages MzfMmUnmapLockedPages = (pfnMmUnmapLockedPages)GetGlobalVeriable(enumMmUnmapLockedPages);
// 	pfnIoFreeMdl MzfIoFreeMdl = (pfnIoFreeMdl)GetGlobalVeriable(enumIoFreeMdl);
// 	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
// 
// 	if (KernelBase &&
// 		KernelSize &&
// 		DriverObjectType &&
// 		MzfMmIsNonPagedSystemAddressValid &&
// 		MzfNtQuerySystemInformation &&
// 		MzfMmGetPhysicalAddress &&
// 		MzfMmGetVirtualForPhysical &&
// 		MzfMmIsAddressValid &&
// 		MzfIoAllocateMdl &&
// 		MzfMmBuildMdlForNonPagedPool &&
// 		MzfMmMapLockedPagesSpecifyCache &&
// 		MzfMmMapLockedPagesSpecifyCache &&
// 		MzfMmUnmapLockedPages &&
// 		MzfIoFreeMdl &&
// 		MzfPsGetCurrentThread)
// 	{
// 		PETHREAD pThread = MzfPsGetCurrentThread();
// 		CHAR PreMode = ChangePreMode(pThread);
// 		SYSTEM_BASIC_INFORMATION sbi;
// 		NTSTATUS status;
// 
// // 		KdPrint(("enter SearchMemoryToFindDubiousPE\n"));
// 		memset(&sbi, 0, sizeof(SYSTEM_BASIC_INFORMATION));
// 		status = MzfNtQuerySystemInformation(SystemBasicInformation, &sbi, sizeof(SYSTEM_BASIC_INFORMATION), NULL);
// 		if (NT_SUCCESS(status))
// 		{
// 			ULONG nObjectHeaderSize = GetGlobalVeriable(enumObjectHeaderSize);
// 			ULONG nSizeOfDriverObject = sizeof(DRIVER_OBJECT) + nObjectHeaderSize;
// 
// 			NumberOfPhysicalPages = sbi.NumberOfPhysicalPages;
// 			HighestPhysicalPageNumber = sbi.HighestPhysicalPageNumber;
// 			if (NumberOfPhysicalPages > HighestPhysicalPageNumber)
// 			{
// 				NumberOfPhysicalPages = HighestPhysicalPageNumber;
// 			}
// 
// //			KdPrint(("NumberOfPhysicalPages, %d, LowestPhysicalPageNumber: %d, HighestPhysicalPageNumber: %d\n", sbi.NumberOfPhysicalPages, sbi.LowestPhysicalPageNumber, sbi.HighestPhysicalPageNumber));
// 
// 			nMaxPhysicalPageSize = NumberOfPhysicalPages * PAGE_SIZE;
// 			Address = (KernelSize + KernelBase) & 0xFFFFF000;
// 
// 			__try
// 			{
// 				ULONG i = 0;
// 				while ( Address < 0xFFFF0000 )
// 				{
// 					if (MzfMmIsAddressValid((PVOID)Address) &&
// 						MzfMmIsNonPagedSystemAddressValid((PVOID)Address))
// 					{
// 						PHYSICAL_ADDRESS PhysicalAddress = MzfMmGetPhysicalAddress((PVOID)Address);
// 
// 						if ( (PhysicalAddress.HighPart || PhysicalAddress.LowPart) &&
// 							PhysicalAddress.QuadPart < nMaxPhysicalPageSize)
// 						{
// 							if (MzfMmGetVirtualForPhysical(PhysicalAddress) == (PVOID)Address)
// 							{
// 								ULONG nEndDriverAddress = 0;
// 								if (GetDriverEndAddress(pDriversInfo, Address, &nEndDriverAddress) ||
// 									GetReloadModulesEndAddress(Address, &nEndDriverAddress))
// 								{
// 									if (nEndDriverAddress == 0 ||
// 										nEndDriverAddress <= Address)
// 									{
// 										Address += PAGE_SIZE;
// 									}
// 									else
// 									{
// 										if ( nEndDriverAddress >= 0xFFFF0000 )
// 										{
// 											nEndDriverAddress = 0xFFFF0000;
// 										}
// 
// 										Address = nEndDriverAddress;
// 									}
// 
// 									continue;
// 								}
// 								else
// 								{
// 									PMDL pMdl = MzfIoAllocateMdl((PVOID)Address, PAGE_SIZE, FALSE, FALSE, NULL);
// 									if (pMdl)
// 									{
// 										PVOID pMdlAddress = NULL;
// 										ULONG nRemainSize = 0, pDriverObject = 0;
// 										ULONG nPageSize = 0xFE8 - sizeof(DRIVER_OBJECT);
// 										MzfMmBuildMdlForNonPagedPool(pMdl);
// 										pMdlAddress = MzfMmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, 0, NormalPagePriority);
// 										if (pMdlAddress)
// 										{
// 											ULONG nSizeOfImage = 0;
// 											if (IsDriverPe((ULONG)pMdlAddress, &nSizeOfImage))
// 											{
// 												if (pDriversInfo->nCnt > pDriversInfo->nRetCnt)
// 												{
// 													pDriversInfo->Drivers[pDriversInfo->nRetCnt].DriverType = enumSuspiciousPe;
// 													pDriversInfo->Drivers[pDriversInfo->nRetCnt].nLodeOrder = 0x10000;
// 													pDriversInfo->Drivers[pDriversInfo->nRetCnt].nBase = Address;
// 													pDriversInfo->Drivers[pDriversInfo->nRetCnt].nSize = nSizeOfImage;
// 													pDriversInfo->Drivers[pDriversInfo->nRetCnt].nDriverObject = 0;
// 													pDriversInfo->Drivers[pDriversInfo->nRetCnt].szDriverPath[0] = L'\0';
// 												}
// 
// 												pDriversInfo->nRetCnt++;
// 											}
// 
// 											MzfMmUnmapLockedPages(pMdlAddress, pMdl);
// 										}
// 
// 										MzfIoFreeMdl(pMdl);
// 									}
// 								}
// 							}
// 						}
// 					}
// 
// 					Address += PAGE_SIZE;
// 				}
// 			}
// 			__except(1)
// 			{
// 			}
// 		}
// 
// 		RecoverPreMode(pThread, PreMode);
// 	}
// }

/* eg. c:\windows */
BOOL GetWindowsDirectory(WCHAR *szRootName)
{
	UNICODE_STRING RootName, ObjectName;
	OBJECT_ATTRIBUTES oa;
	HANDLE hLink = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	BOOL bRet = FALSE;
	HANDLE hFile = NULL;
	WCHAR szSystemRoot[] = {'\\', 'S', 'y', 's', 't', 'e', 'm', 'R', 'o', 'o', 't', '\0'};
	IO_STATUS_BLOCK isb;
	POBJECT_TYPE FileObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoFileObjectType);
	PFILE_OBJECT pFileObject = NULL;
	UNICODE_STRING pDosName;
	WCHAR szWindowsDir[MAX_PATH] = {0};
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnNtOpenSymbolicLinkObject MzfNtOpenSymbolicLinkObject = (pfnNtOpenSymbolicLinkObject)GetGlobalVeriable(enumNtOpenSymbolicLinkObject);
	pfnNtQuerySymbolicLinkObject MzfNtQuerySymbolicLinkObject = (pfnNtQuerySymbolicLinkObject)GetGlobalVeriable(enumNtQuerySymbolicLinkObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnIoCreateFile MzfIoCreateFile = (pfnIoCreateFile)GetGlobalVeriable(enumIoCreateFile);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnIoVolumeDeviceToDosName MzfIoVolumeDeviceToDosName = (pfnIoVolumeDeviceToDosName)GetGlobalVeriable(enumIoVolumeDeviceToDosName);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	
	if (!szRootName ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!MzfNtOpenSymbolicLinkObject ||
		!MzfNtQuerySymbolicLinkObject||
		!MzfPsGetCurrentThread ||
		!MzfNtClose ||
		!MzfIoCreateFile ||
		!MzfObReferenceObjectByHandle ||
		!FileObjectType ||
		!MzfIoVolumeDeviceToDosName ||
		!MzfObfDereferenceObject)
	{
		return FALSE;
	}

	MzfInitUnicodeString(&RootName, szSystemRoot);
	InitializeObjectAttributes(&oa, &RootName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = MzfIoCreateFile(
		&hFile,
		SYNCHRONIZE,
		&oa,
		&isb,
		NULL,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		FILE_DIRECTORY_FILE,
		0,
		0,
		0,
		0,
		IO_NO_PARAMETER_CHECKING);

	if (NT_SUCCESS(status) &&
		NT_SUCCESS(MzfObReferenceObjectByHandle(hFile, 0, FileObjectType, KernelMode, &pFileObject, NULL)) &&
		pFileObject->DeviceObject &&
		NT_SUCCESS(MzfIoVolumeDeviceToDosName(pFileObject->DeviceObject, &pDosName)))
	{
		WCHAR *ObjectNameBuffer = NULL;
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);

		wcsncpy(szWindowsDir, pDosName.Buffer, pDosName.Length);

		ObjectNameBuffer = MzfExAllocatePoolWithTag(PagedPool, MAX_PATH_WCHAR, MZFTAG);
		if (ObjectNameBuffer)
		{
			memset(ObjectNameBuffer, 0, MAX_PATH_WCHAR);

			status = MzfNtOpenSymbolicLinkObject(&hLink, GENERIC_READ, &oa);
			if (NT_SUCCESS(status))
			{
				ObjectName.Buffer = ObjectNameBuffer;
				ObjectName.Length = 0;
				ObjectName.MaximumLength = MAX_PATH;

				status = MzfNtQuerySymbolicLinkObject(hLink, &ObjectName, NULL);

				if (NT_SUCCESS(status))
				{
					ULONG nLen = ObjectName.Length / sizeof(WCHAR);
					ULONG i = 0;

					for ( i = nLen - 1; i > 0; i-- )
					{
						if (ObjectNameBuffer[i] == L'\\')
						{
							wcsncat(szWindowsDir, &ObjectNameBuffer[i], nLen - i);
							bRet = TRUE;
							break;
						}
					}
				}
			}
			
			MzfExFreePoolWithTag(ObjectNameBuffer, 0);
		}

		if (hFile)
		{
			MzfNtClose(hFile);
		}
		
		if (hLink)
		{
			MzfNtClose(hLink);
		}
		
		if (pFileObject)
		{
			MzfObfDereferenceObject(pFileObject);
		}

		MzfExFreePoolWithTag(pDosName.Buffer, 0);
		RecoverPreMode(pThread, PreMode);
	}
	
	if (!bRet)
	{
		pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
		pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
		if (MzfProbeForRead && MzfMmIsAddressValid)
		{
			__try
			{
				WCHAR *SystemRootName = (WCHAR*)0x7FFE0030;
				WCHAR szWindows[] = {'X', ':', '\\', 'W', 'I', 'N', 'D', 'O', 'W', 'S', '\0'};
				
				if (MzfMmIsAddressValid(SystemRootName))
				{
					MzfProbeForRead(SystemRootName, wcslen(szWindows) * sizeof(WCHAR), sizeof(WCHAR));

					if (SystemRootName[1] == L':' && SystemRootName[2] == L'\\') 
					{
						wcsncpy(szWindowsDir, SystemRootName, wcslen(szWindows));
						bRet = TRUE;
					}
				}
			}
			__except(1)
			{
			}
		}
	}

	if (bRet)
	{
		wcscpy(szRootName, szWindowsDir);
	}

	return bRet;
}

/* eg. c:\windows\system32\driver\ */
BOOL GetDriverDirectory(WCHAR *szDir)
{
	BOOL bRet = FALSE;
	WCHAR szWindowsDirectory[MAX_PATH] = {0};

	if (GetWindowsDirectory(szWindowsDirectory))
	{
		WCHAR szDriver[] = {'\\', 's', 'y', 's', 't', 'e', 'm', '3', '2', '\\', 'd', 'r', 'i', 'v', 'e', 'r', 's', '\\', '\0'};
		wcscat(szWindowsDirectory, szDriver);
		wcscpy(szDir, szWindowsDirectory);
		bRet = TRUE;
	}

	return bRet;
}

//
// 枚举驱动信息
//
NTSTATUS EnumDrivers(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PALL_DRIVERS pDriversInfo = (PALL_DRIVERS)pOutBuffer;
	ULONG nCnt = (uOutSize - sizeof(ALL_DRIVERS)) / sizeof(DRIVER_INFO);

	// 检查参数
	if (!pInBuffer ||
		!pOutBuffer ||
		uInSize != sizeof(OPERATE_TYPE) ||
		uOutSize < sizeof(ALL_DRIVERS))
	{
		return STATUS_INVALID_PARAMETER;
	}

 	EnumDriverByLdrDataTableEntry(pDriversInfo, nCnt);
    EnumDriversByWalkerDirectoryObject(pDriversInfo, nCnt);
// 	EnumDriverBySearchMemoryToFindDriverObject(pDriversInfo);

	// 枚举可疑PE
//	SearchMemoryToFindDubiousPE(pDriversInfo);

	if (nCnt >= pDriversInfo->nCnt)
	{
		status = STATUS_SUCCESS;
	}
	else
	{
		status = STATUS_BUFFER_TOO_SMALL;
	}
	
	return status;
}

//
// 拷贝驱动内存
//
NTSTATUS DumpDriverMemory(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	pfnIoAllocateMdl MzfIoAllocateMdl = (pfnIoAllocateMdl)GetGlobalVeriable(enumIoAllocateMdl);
	pfnMmBuildMdlForNonPagedPool MzfMmBuildMdlForNonPagedPool = (pfnMmBuildMdlForNonPagedPool)GetGlobalVeriable(enumMmBuildMdlForNonPagedPool);
	pfnMmMapLockedPagesSpecifyCache MzfMmMapLockedPagesSpecifyCache = (pfnMmMapLockedPagesSpecifyCache)GetGlobalVeriable(enumMmMapLockedPagesSpecifyCache);
	pfnMmUnmapLockedPages MzfMmUnmapLockedPages = (pfnMmUnmapLockedPages)GetGlobalVeriable(enumMmUnmapLockedPages);
	pfnIoFreeMdl MzfIoFreeMdl = (pfnIoFreeMdl)GetGlobalVeriable(enumIoFreeMdl);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_DRIVER pCd = (PCOMMUNICATE_DRIVER)pInBuffer;
	ULONG pBase = 0, nSize = 0;
	ULONG nOutBuffer = (ULONG)pOutBuffer;
	
	if (!MzfIoAllocateMdl ||
		!MzfMmBuildMdlForNonPagedPool ||
		!MzfMmMapLockedPagesSpecifyCache ||
		!MzfMmUnmapLockedPages ||
		!MzfIoFreeMdl ||
		!MzfProbeForRead ||
		!MzfMmIsAddressValid)
	{
		return status;
	}

	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_DRIVER) ||
		!pOutBuffer ||
		uOutSize == 0)
	{
		return STATUS_INVALID_PARAMETER;
	}

	pBase = pCd->op.DumpMemory.pBase;
	nSize = pCd->op.DumpMemory.nSize;

	if (pBase > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)pBase) &&
		nSize < SYSTEM_ADDRESS_START &&
		pBase + nSize > SYSTEM_ADDRESS_START)
	{
		__try
		{
			ULONG nRemainSize = PAGE_SIZE - (pBase & 0xFFF);
			ULONG nCopyedSize = 0;
			KdPrint(("Base: 0x%08X, Size: 0x%08X\n", pBase, nSize));

			while ( nCopyedSize < nSize )
			{
				PMDL pMdl = NULL;
				BOOL bCopyOk = FALSE;

				if ( nSize - nCopyedSize < nRemainSize )
				{
					nRemainSize = nSize - nCopyedSize;
				}

				pMdl = MzfIoAllocateMdl( (PVOID)(pBase & 0xFFFFF000), PAGE_SIZE, FALSE, FALSE, NULL);

				if ( pMdl )
				{
					PVOID pMapedAddress = NULL;

					MzfMmBuildMdlForNonPagedPool(pMdl);
					pMapedAddress = MzfMmMapLockedPagesSpecifyCache( pMdl, KernelMode, MmCached, NULL, 0, NormalPagePriority );

					if ( pMapedAddress )
					{
						memcpy((PVOID)nOutBuffer, (PVOID)((ULONG)pMapedAddress + (pBase & 0xFFF)), nRemainSize);
						MzfMmUnmapLockedPages( pMapedAddress, pMdl );
						bCopyOk = TRUE;
					}

					MzfIoFreeMdl(pMdl);
				}
	
				if ( !bCopyOk )
				{
					memset((PVOID)nOutBuffer, 0, nRemainSize);
				}

				if ( nCopyedSize )
				{
					nRemainSize = PAGE_SIZE;
				}

				nCopyedSize += nRemainSize;
				pBase += nRemainSize;
				nOutBuffer += nRemainSize;
			}

			status = STATUS_SUCCESS;
		}
		__except(1)
		{
		}
	}

	return status;
}

VOID HaveDriverUnloadThread(IN PVOID lpParam)
{
	pfnPsTerminateSystemThread MzfPsTerminateSystemThread = (pfnPsTerminateSystemThread)GetGlobalVeriable(enumPsTerminateSystemThread);
	pfnObMakeTemporaryObject MzfObMakeTemporaryObject = (pfnObMakeTemporaryObject)GetGlobalVeriable(enumObMakeTemporaryObject);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)lpParam;

	KdPrint(("HaveDriverUnloadThread\n"));

	if (pDriverObject)
	{
		PDRIVER_UNLOAD DriverUnload = pDriverObject->DriverUnload;

		if (DriverUnload)
		{
			DriverUnload(pDriverObject);
			
			KdPrint(("DriverUnload return\n"));

			pDriverObject->FastIoDispatch = NULL;
			memset(pDriverObject->MajorFunction, 0, sizeof(pDriverObject->MajorFunction));
			pDriverObject->DriverUnload = NULL;

			MzfObMakeTemporaryObject(pDriverObject);
			MzfObfDereferenceObject(pDriverObject);
		}
	}
	
	MzfPsTerminateSystemThread(STATUS_SUCCESS);
}

VOID NotHaveDriverUnloadThread(IN PVOID lpParam)
{
	pfnPsTerminateSystemThread MzfPsTerminateSystemThread = (pfnPsTerminateSystemThread)GetGlobalVeriable(enumPsTerminateSystemThread);
	pfnObMakeTemporaryObject MzfObMakeTemporaryObject = (pfnObMakeTemporaryObject)GetGlobalVeriable(enumObMakeTemporaryObject);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoDeleteDevice MzfIoDeleteDevice =(pfnIoDeleteDevice)GetGlobalVeriable(enumIoDeleteDevice);
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)lpParam;
	PDEVICE_OBJECT DeviceObject = NULL;

	if (pDriverObject)
	{
		KdPrint(("NotHaveDriverUnloadThread\n"));
		pDriverObject->FastIoDispatch = NULL;
		memset(pDriverObject->MajorFunction, 0, sizeof(pDriverObject->MajorFunction));
		pDriverObject->DriverUnload = NULL;

		DeviceObject = pDriverObject->DeviceObject;

		while ( DeviceObject && MzfMmIsAddressValid(DeviceObject) )
		{
			MzfIoDeleteDevice(DeviceObject);
			DeviceObject = DeviceObject->NextDevice;
		}

		MzfObMakeTemporaryObject(pDriverObject);
		MzfObfDereferenceObject(pDriverObject);
	}
	
	MzfPsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS PspUnloadDriver(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnObMakeTemporaryObject MzfObMakeTemporaryObject = (pfnObMakeTemporaryObject)GetGlobalVeriable(enumObMakeTemporaryObject);
	pfnPsCreateSystemThread MzfPsCreateSystemThread = (pfnPsCreateSystemThread)GetGlobalVeriable(enumPsCreateSystemThread);
	pfnKeWaitForSingleObject MzfKeWaitForSingleObject = (pfnKeWaitForSingleObject)GetGlobalVeriable(enumKeWaitForSingleObject);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);

	if (!pDriverObject ||
		!MzfMmIsAddressValid ||
		!MzfObfDereferenceObject ||
		!MzfObMakeTemporaryObject ||
		!MzfPsCreateSystemThread ||
		!MzfKeWaitForSingleObject ||
		!MzfNtClose ||
		!MzfPsGetCurrentThread ||
		!MzfObReferenceObjectByHandle ||
		!GetGlobalVeriable(enumPsTerminateSystemThread))
	{
		return status;
	}

	if (MzfMmIsAddressValid(pDriverObject) &&
		MzfMmIsAddressValid((PVOID)((ULONG)pDriverObject + sizeof(DRIVER_OBJECT) - 1)))
	{
		BOOL bDriverUnload = FALSE;
		HANDLE hSystemThread = NULL;

		if (pDriverObject->DriverUnload &&
			(ULONG)pDriverObject->DriverUnload > SYSTEM_ADDRESS_START &&
			MzfMmIsAddressValid(pDriverObject->DriverUnload))
		{
			bDriverUnload = TRUE;
		}
		
		if (bDriverUnload)
		{
			status = MzfPsCreateSystemThread(&hSystemThread, 0, NULL, NULL, NULL, HaveDriverUnloadThread, pDriverObject);
		}
		else
		{
			status = MzfPsCreateSystemThread(&hSystemThread, 0, NULL, NULL, NULL, NotHaveDriverUnloadThread, pDriverObject);
		}
		
		if (NT_SUCCESS(status))
		{
			PETHREAD pThread = NULL, pCurrentThread = NULL;
			CHAR PreMode = 0;

			status = MzfObReferenceObjectByHandle(hSystemThread, 0, NULL, KernelMode, &pThread, NULL);
			if (NT_SUCCESS(status))
			{
				LARGE_INTEGER timeout;
				timeout.QuadPart = -10 * 1000 * 1000 * 3;
				status = MzfKeWaitForSingleObject(pThread, Executive, KernelMode, TRUE, &timeout); // 等待3秒
				MzfObfDereferenceObject(pThread);
			}

			pCurrentThread = MzfPsGetCurrentThread();
			PreMode = ChangePreMode(pCurrentThread);
			MzfNtClose(hSystemThread);
			RecoverPreMode(pCurrentThread, PreMode);
		}
	}

	return status;
}

//
// 卸载驱动模块
//
NTSTATUS UnloadDriver(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_DRIVER pCd = (PCOMMUNICATE_DRIVER)pInBuffer;
	PDRIVER_OBJECT pDriverObject = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDRIVER_OBJECT pSpyHunter = (PDRIVER_OBJECT)GetGlobalVeriable(enumSpyHunterDriverObject);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return status;
	}

	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_DRIVER))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pDriverObject = (PDRIVER_OBJECT)pCd->op.Unload.DriverObject;

	KdPrint(("unload driver 0x%08X\n", pDriverObject));

	if ((ULONG)pDriverObject > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid(pDriverObject) &&
		pSpyHunter != pDriverObject && 
		IsRealDriverObject(pDriverObject) )
	{
		status = PspUnloadDriver(pDriverObject);
	}

	return status;
}