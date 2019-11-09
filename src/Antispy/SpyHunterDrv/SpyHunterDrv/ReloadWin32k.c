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
#include "ReloadWin32k.h"
#include "InitWindows.h"
#include "CommonFunction.h"
#include "Peload.h"
#include "ldasm.h"

//*************************************************
// Returns:   返回要寻找的进程对象
// Qualifier: 调用NtQuerySystemInformation的SystemProcessesInformation功能，取得系统中的全部进程，然后比较进程名
// Parameter: IN PWCHAR ProcessName - 要得到进程对象的进程名 - eg. csrss.exe
//*************************************************
PEPROCESS LookupProcessByName(IN PWCHAR ProcessName)
{
	UNICODE_STRING cntdProcessName;
	PEPROCESS pProcess = NULL;
	ULONG uSize = 0x10000;
	ULONG uRet = 0, i = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pBuffer = NULL;	
	PSYSTEM_PROCESS_INFORMATION pSpiNext = NULL;
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;

	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject  = (pfnObfDereferenceObject )GetGlobalVeriable(enumObfDereferenceObject );
	pfnPsGetCurrentThread MzfKeGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (!MzfExAllocatePoolWithTag		|| 
		!MzfNtQuerySystemInformation	||
		!MzfExFreePoolWithTag			||
		!MzfPsLookupProcessByProcessId	||
		!MzfObfDereferenceObject		||
		!MzfKeGetCurrentThread			)
	{
		return NULL;
	}

	RtlInitUnicodeString(&cntdProcessName, ProcessName);
	pThread = MzfKeGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	do 
	{
		pBuffer = MzfExAllocatePoolWithTag(PagedPool, uSize, MZFTAG);
		if (pBuffer)
		{
			status = MzfNtQuerySystemInformation(SystemProcessesInformation, pBuffer, uSize, &uRet);
			if (NT_SUCCESS(status))
			{
				pSpiNext = (PSYSTEM_PROCESS_INFORMATION)pBuffer;

				do
				{
					if (RtlCompareUnicodeString(&pSpiNext->ImageName, &cntdProcessName, TRUE) == 0)
					{
						PEPROCESS pCurrProcess;

						status = MzfPsLookupProcessByProcessId((HANDLE)pSpiNext->ProcessId, &pCurrProcess);
						if (NT_SUCCESS(status))
						{
							pProcess = pCurrProcess;
						}
					}

					pSpiNext = (PSYSTEM_PROCESS_INFORMATION)((ULONG)pSpiNext + pSpiNext->NextEntryOffset);  
				} while (pSpiNext->NextEntryOffset && !pProcess);
			}
			else
			{
				uSize *= 2;
			}
			
			MzfExFreePoolWithTag(pBuffer, 0);
			pBuffer = NULL;
		}
		else
		{
			break;
		}

	} while (status == STATUS_INFO_LENGTH_MISMATCH && ++i < 10);

	RecoverPreMode(pThread, PreMode);
	return pProcess;
}

BOOLEAN IsSafeAddress(
					  __in PVOID Address,
					  __in ULONG Size
					  )
{
	BOOLEAN bResult = FALSE;
	ULONG_PTR ulpAddress;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	if (!MzfMmIsAddressValid)
	{
		return FALSE;
	}

	ulpAddress = (ULONG_PTR) Address;
	// non-NULL ptr
	if ( ulpAddress )
	{
		// no wrapping
		if ( ulpAddress + Size >= ulpAddress )
			bResult = TRUE;
	}

	// valid ptr
	bResult &= MzfMmIsAddressValid( Address ) && MzfMmIsAddressValid( (PCHAR) Address + Size - 1 );

	return bResult;
}

//*************************************************
// Returns:   返回ShadowTable的地址
// Qualifier: 从KeAddSystemServiceTable函数中得到ShadowTable的地址
//*************************************************
PServiceDescriptorTableEntry_t GetAddrOfShadowTable()
{
	PCHAR Ptr = NULL;
	ULONG i;
	PServiceDescriptorTableEntry_t pShadowTableRet = NULL;
	ULONG bReloadNtosSuccess = GetGlobalVeriable(enumReloadNtosKrnlSuccess);
	ULONG Base = bReloadNtosSuccess ? GetGlobalVeriable(enumNewKernelBase) : GetGlobalVeriable(enumOriginKernelBase);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	CHAR szKeAddSystemServiceTable[] = {'K','e','A','d','d','S','y','s','t','e','m','S','e','r','v','i','c','e','T','a','b','l','e','\0'};

	if (!MzfMmIsAddressValid || !Base)
	{
		return NULL;
	}

	Ptr = FindExportedRoutineByName((PVOID)Base, szKeAddSystemServiceTable);
	if (!Ptr)
	{
		return NULL;
	}

	// scan KeAddSystemServiceTable function body for pointers to shadow table
	for( i = 0; i < PAGE_SIZE; i++, Ptr++ )
	{
		if ( MzfMmIsAddressValid( Ptr ) )
		{
			PVOID ProbeShadowTablePtr;

			ProbeShadowTablePtr = (PVOID) *(PULONG) Ptr;
			// preliminary address check / bypass main SDT ptr
			if ( IsSafeAddress( ProbeShadowTablePtr, 2 * sizeof(ServiceDescriptorTableEntry_t) ) &&
				&KeServiceDescriptorTable != ProbeShadowTablePtr )
			{
				// can't do MmGetSystemAddressForMdl safe here, it returnes invalid(unmapped) VA

				// shadow table first item equals to SDT
				if ( 0 == memcmp( ProbeShadowTablePtr, &KeServiceDescriptorTable, sizeof(ServiceDescriptorTableEntry_t) ) )
				{
					// address found
					pShadowTableRet = (PServiceDescriptorTableEntry_t) ProbeShadowTablePtr;
				}
			}
		}

		if ( pShadowTableRet )
			break;
	}

	return pShadowTableRet;
}

/*************************************************
Returns:   返回函数地址
Qualifier: 根据函数名在IAT中查找kernel导出的函数地址
Parameter: PVOID ImageBase - 模块基址
Parameter: CHAR * szProcName - 要查找函数的函数名
*************************************************/
DWORD GetProcFromIAT(PVOID ImageBase, CHAR* szProcName)      
{
	PIMAGE_IMPORT_DESCRIPTOR ImageImportDescriptor = NULL;
	PIMAGE_THUNK_DATA ImageThunkData = NULL, FirstThunk = NULL;
	PIMAGE_IMPORT_BY_NAME ImortByName = NULL;
	DWORD ImportSize = 0;
	PVOID ModuleBase = NULL;
	DWORD FunctionAddress = 0;
	TCHAR szDllName[20];
	PIMAGE_THUNK_DATA pOrgFirstThunk;
	PIMAGE_THUNK_DATA pFirstThunk;
	DWORD index = 0;

	__try
	{
		ImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)MzfImageDirectoryEntryToData(ImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportSize);
		if (ImageImportDescriptor == NULL)
		{
			return 0;
		}

		while(ImageImportDescriptor->Name || ImageImportDescriptor->FirstThunk || ImageImportDescriptor->OriginalFirstThunk)
		{
			CHAR szNtkrnlpa[] = {'n','t','k','r','n','l','p','a','.','e','x','e','\0'};
			CHAR szNtoskrnl[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
			CHAR szNtkrnlmp[] = {'n','t','k','r','n','l','m','p','.','e','x','e','\0'};
			CHAR szNtkrpamp[] = {'n','t','k','r','p','a','m','p','.','e','x','e','\0'};
			CHAR szHalmacpi[] = {'h','a','l','m','a','c','p','i','.','d','l','l','\0'};
			CHAR szHal[] = {'h','a','l','.','d','l','l','\0'};
			CHAR ModuleName[MAX_PATH] = {0};

			strcpy(ModuleName, (CHAR*)((ULONG)ImageBase + ImageImportDescriptor->Name));

			//ntoskrnl.exe(NTKRNLPA.exe、ntkrnlmp.exe、ntkrpamp.exe)：
			if (_stricmp(ModuleName, szNtkrnlpa) == 0 ||
				_stricmp(ModuleName, szNtoskrnl) == 0 ||
				_stricmp(ModuleName, szNtkrnlmp) == 0 ||
				_stricmp(ModuleName, szNtkrpamp) == 0 )
			{
				break;
			}

			ImageImportDescriptor++;
		}

		pOrgFirstThunk = (PIMAGE_THUNK_DATA)(ImageImportDescriptor->OriginalFirstThunk + (DWORD)ImageBase);
		pFirstThunk = (PIMAGE_THUNK_DATA)(ImageImportDescriptor->FirstThunk + (DWORD)ImageBase);

		for(index = 0; &(pOrgFirstThunk[index]); index++)
		{
			if(!(pOrgFirstThunk[index].u1.Ordinal & IMAGE_ORDINAL_FLAG32 ))
			{
				CHAR szFunName[MAX_PATH] = {0};
				PIMAGE_IMPORT_BY_NAME pFunName = (PIMAGE_IMPORT_BY_NAME)(pOrgFirstThunk[index].u1.ForwarderString + (DWORD)ImageBase);  

				strcpy(szFunName, (CHAR*)(pFunName->Name));

				if(_stricmp(szFunName, szProcName) == 0)
				{
					return pFirstThunk[index].u1.Function;
				}
			}
		}
	}
	__except(1)
	{
	}

	return 0;  
}

/*************************************************
Returns:   
Qualifier: 通过搜索win32k.sys的EntryPoint，得到W32pTable的地址
Parameter: PVOID NewWin32kBase - reload win32k base
Parameter: DWORD OldWin32kBase - old win32k base 
Parameter: PDWORD * W32pTable - return W32pTable address
*************************************************/
BOOL GetOriginalW32pTable(IN PVOID NewWin32kBase, IN DWORD OldWin32kBase, OUT PDWORD *W32pTable)
{
	BOOL bRet = FALSE;
	PIMAGE_NT_HEADERS NtHeaders;
	ULONG_PTR EntryPoint;
	DWORD dwEntryPoint;
	DWORD dwCurAddress;
	DWORD Length = 0;
	PUCHAR pOpcode;
	ULONG ulCount = 0;
	PVOID pKeAddSystemServiceTable = NULL;
	CHAR szKeAddSystemServiceTable[] = {'K','e','A','d','d','S','y','s','t','e','m','S','e','r','v','i','c','e','T','a','b','l','e','\0'};
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!NewWin32kBase || !W32pTable || !OldWin32kBase || !MzfMmIsAddressValid)
	{
		return FALSE;
	}

	pKeAddSystemServiceTable = (PVOID)GetProcFromIAT(NewWin32kBase, szKeAddSystemServiceTable);

	if (!pKeAddSystemServiceTable)
	{
		KdPrint(("get KeAddSystemServiceTable error\n"));
		return FALSE;
	}

	KdPrint(("NewKeAddSystemServiceTable: 0x%08X\n", pKeAddSystemServiceTable));

	NtHeaders = RtlImageNtHeader(NewWin32kBase);
	if (NtHeaders)
	{
		EntryPoint = NtHeaders->OptionalHeader.AddressOfEntryPoint;
		EntryPoint += (ULONG_PTR)NewWin32kBase;
		dwEntryPoint = (DWORD)EntryPoint;
		
		/*
		通过call    ds:KeAddSystemServiceTable的定位

		IDA反汇编结果：
		INIT:BF9AFE05 68 80 9B 99 BF             push    offset _W32pServiceTable
		INIT:BF9AFE0A FF 15 58 CD 98 BF          call    ds:__imp__KeAddSystemServiceTable@20 ; KeAddSystemServiceTable(x,x,x,x,x)
		*/
		__try
		{
			for(dwCurAddress = dwEntryPoint; dwCurAddress < dwEntryPoint + PAGE_SIZE; dwCurAddress += Length)
			{
				Length = SizeOfCode((PUCHAR)dwCurAddress, &pOpcode);
				if(*(WORD *)dwCurAddress == 0x15ff )
				{
 					if (ulCount++ >= 5)
					{
						break;
					}

					if (MzfMmIsAddressValid((PVOID)(dwCurAddress + 2))			&&
						*(PULONG)(dwCurAddress + 2) > SYSTEM_ADDRESS_START		&&
						MzfMmIsAddressValid((PVOID)(dwCurAddress - 4))			&&
						*(PULONG)(dwCurAddress - 4) > SYSTEM_ADDRESS_START		&& 
						MzfMmIsAddressValid((PVOID)(*(PDWORD)(dwCurAddress + 2) - (DWORD)OldWin32kBase + (DWORD)NewWin32kBase)) &&
						*(PDWORD)(*(PDWORD)(dwCurAddress + 2) - (DWORD)OldWin32kBase + (DWORD)NewWin32kBase) == (DWORD)pKeAddSystemServiceTable)
					{
						*W32pTable = (PDWORD)(*(PDWORD)(dwCurAddress - 4) - (DWORD)OldWin32kBase + (DWORD)NewWin32kBase);
						bRet = TRUE;
						break;
					}
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	return bRet;
}	

NTSTATUS GetRawShadowAddr(PServiceDescriptorTableEntry_t ShadowSSDTAddr, PVOID BaseOfwin32k_sys, PVOID* pOutShadowServiceTable)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	HANDLE hFile = NULL;
	HANDLE hSection = NULL; 
	IO_STATUS_BLOCK IoStatusBlock; 
	OBJECT_ATTRIBUTES ObjectAttributes; 
	PVOID pSectionObject;
	PVOID BaseAddress = NULL;
	ULONG ulLen = 0;
	ULONG	ulSizeShadowTable;
	PVOID   pRawShadowServiceTable;
	UNICODE_STRING usWin32K;
	WCHAR szWin32k[] = 
		{'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','w','i','n','3','2','k','.','s','y','s','\0'};

	RtlInitUnicodeString(&usWin32K, szWin32k);		
	InitializeObjectAttributes(&ObjectAttributes,
		&usWin32K,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL); 

	ntStatus = ZwOpenFile(&hFile, SYNCHRONIZE | FILE_EXECUTE, &ObjectAttributes, &IoStatusBlock,   
		FILE_SHARE_READ,   
		FILE_SYNCHRONOUS_IO_NONALERT);   

	if (NT_SUCCESS(ntStatus))
	{			
		ObjectAttributes.ObjectName = 0;   
		ntStatus = ZwCreateSection(&hSection, SECTION_MAP_EXECUTE | SECTION_MAP_READ | SECTION_QUERY, &ObjectAttributes, 0,PAGE_EXECUTE, SEC_IMAGE, hFile);  
		if (NT_SUCCESS(ntStatus))
		{
			ntStatus = ObReferenceObjectByHandle(hSection, 
				SECTION_MAP_EXECUTE | SECTION_MAP_READ | SECTION_QUERY, 
				*MmSectionObjectType,
				KernelMode,
				(PVOID *)&pSectionObject,
				NULL
				);

			if (NT_SUCCESS(ntStatus))
			{
				ntStatus = MmMapViewInSystemSpace(pSectionObject, &BaseAddress,&ulLen);

				if (NT_SUCCESS(ntStatus))
				{
					PIMAGE_NT_HEADERS32 pImageNtHeaders;					
					ULONG	ulRVA;
					PIMAGE_SECTION_HEADER pImageSectionHeader;
					LARGE_INTEGER ByteOffset;
					int nSections = 0;

					ntStatus = STATUS_UNSUCCESSFUL;

					KdPrint(("MmMapViewInSystemSpace Success"));
					pImageNtHeaders = (PIMAGE_NT_HEADERS32)((PBYTE)BaseAddress + ((PIMAGE_OPTIONAL_HEADER32)BaseAddress)->SizeOfHeaders);					
					ulRVA = (PBYTE)(ShadowSSDTAddr->ServiceTableBase) - (PBYTE)BaseOfwin32k_sys;
					//KdPrint(("Shadow Table RVA:%x\tBaseAddress:%p",ulRVA,BaseOfwin32k_sys));
					pImageSectionHeader = (PIMAGE_SECTION_HEADER)((PBYTE)pImageNtHeaders + sizeof(IMAGE_NT_HEADERS32));
					if (pImageNtHeaders->FileHeader.NumberOfSections > 0)
					{
						for (nSections = 0; nSections < pImageNtHeaders->FileHeader.NumberOfSections; nSections++)
						{
							if (ulRVA >= pImageSectionHeader->VirtualAddress && ulRVA < pImageSectionHeader->VirtualAddress + pImageSectionHeader->Misc.VirtualSize)
							{
								break;
							}

							pImageSectionHeader++;
						}

						ByteOffset.LowPart = pImageSectionHeader->PointerToRawData + ulRVA - pImageSectionHeader->VirtualAddress;

						ByteOffset.HighPart = 0;
						ulSizeShadowTable = ShadowSSDTAddr->NumberOfServices;
						KdPrint(("Read Start Addr:%x\tRead Size:%x",ByteOffset.LowPart,ulSizeShadowTable));
						if (ulSizeShadowTable)
						{
							pRawShadowServiceTable = ExAllocatePoolWithTag(NonPagedPool, ulSizeShadowTable * 4, MZFTAG);
							if (pRawShadowServiceTable)
							{
								RtlZeroMemory(pRawShadowServiceTable, ulSizeShadowTable * 4);
								ntStatus = ZwReadFile(hFile,NULL,NULL,NULL,&IoStatusBlock,pRawShadowServiceTable,ulSizeShadowTable * 4,&ByteOffset,NULL);
								if (NT_SUCCESS(ntStatus))
								{
									//KdPrint(("BaseOfwin32k_sys:%p\tImageBase:%p",BaseOfwin32k_sys,pImageNtHeaders->OptionalHeader.ImageBase));
									DWORD dwAlignOffset = (PBYTE)BaseOfwin32k_sys - (PBYTE)pImageNtHeaders->OptionalHeader.ImageBase;
									ULONG ulNum = 0;
									for(ulNum = 0; ulNum< ulSizeShadowTable; ulNum++)
									{
										((ULONG *)pRawShadowServiceTable)[ulNum] += dwAlignOffset;
									}	

									ntStatus = STATUS_SUCCESS;
								}

								*pOutShadowServiceTable = pRawShadowServiceTable;
							}
							else
							{
								ntStatus = STATUS_INSUFFICIENT_RESOURCES;
							}
						}
					}
					MmUnmapViewInSystemSpace(BaseAddress);						
				}//MmMapViewInSystemSpace Success
				ObDereferenceObject(pSectionObject);
			}//ObReferenceObjectByHandle Success
			ZwClose(hSection);
		}//ZwCreateSection Success
		ZwClose(hFile);
	}//ZwCreateFile Success		

	return ntStatus;
}

BOOL InitReloadWin32K(PServiceDescriptorTableEntry_t ShadowSSDT, ULONG OldWin32kBase)
{
	ULONG NewWin32kBase = 0;
	PULONG OriginShadowServiceTable = NULL, ReloadShadowServiceTable = NULL;
	PULONG *pServiceTable = NULL;
	PServiceDescriptorTableEntry_t OriginShadowSSDT = NULL, ReloadShadowSSDT = NULL;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);

	if (!ShadowSSDT || 
		!OldWin32kBase || 
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag)
	{
		return FALSE;
	}

	OriginShadowServiceTable = MzfExAllocatePoolWithTag(NonPagedPool, ShadowSSDT->NumberOfServices * sizeof(DWORD), MZFTAG);
	ReloadShadowServiceTable = MzfExAllocatePoolWithTag(NonPagedPool, ShadowSSDT->NumberOfServices * sizeof(DWORD), MZFTAG);
	OriginShadowSSDT = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ServiceDescriptorTableEntry_t), MZFTAG);
	ReloadShadowSSDT = MzfExAllocatePoolWithTag(NonPagedPool, sizeof(ServiceDescriptorTableEntry_t), MZFTAG);

	if (ShadowSSDT->NumberOfServices > 0 && 
		OriginShadowServiceTable && 
		ReloadShadowServiceTable &&
		OriginShadowSSDT &&
		ReloadShadowSSDT)
	{
		/*L"\\SystemRoot\\System32\\win32k.sys"*/
		WCHAR szWin32k[] = 
			{'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','w','i','n','3','2','k','.','s','y','s','\0'};

		KdPrint(("win32k service cnt: %d\n", ShadowSSDT->NumberOfServices));
		
		memset(OriginShadowServiceTable, 0, ShadowSSDT->NumberOfServices * sizeof(DWORD));
		memset(ReloadShadowServiceTable, 0, ShadowSSDT->NumberOfServices * sizeof(DWORD));
		memset(OriginShadowSSDT, 0, sizeof(ServiceDescriptorTableEntry_t));
		memset(ReloadShadowSSDT, 0, sizeof(ServiceDescriptorTableEntry_t));

		if (PeLoad(szWin32k, &NewWin32kBase, OldWin32kBase))
		{
			PDWORD pOriginalKiServiceTable = NULL;
			KdPrint(("Safe Win32k ModuleBase: %08x\r\n", NewWin32kBase));

			SetGlobalVeriable(enumReloadWin32kSuccess, 1);
			SetGlobalVeriable(enumNewWin32kBase, NewWin32kBase);

			if(GetOriginalW32pTable((PVOID)NewWin32kBase, OldWin32kBase, &pOriginalKiServiceTable) && pOriginalKiServiceTable)
			{
				ULONG i = 0, nCnt = 0;

				nCnt = ShadowSSDT->NumberOfServices;
				KdPrint(("service cnt: %d\n", nCnt));

				for (i = 0; i < nCnt; i++)
				{
					OriginShadowServiceTable[i] = (ULONG)pOriginalKiServiceTable[i];
					ReloadShadowServiceTable[i] = (ULONG)pOriginalKiServiceTable[i] - (ULONG)OldWin32kBase + (ULONG)NewWin32kBase; 
				}

				OriginShadowSSDT->NumberOfServices = ShadowSSDT->NumberOfServices;
				OriginShadowSSDT->ParamTableBase = ShadowSSDT->ParamTableBase;
				OriginShadowSSDT->ServiceCounterTableBase = ShadowSSDT->ServiceCounterTableBase;
				OriginShadowSSDT->ServiceTableBase = OriginShadowServiceTable;
				SetGlobalVeriable(enumOriginShadowSSDT, (ULONG)OriginShadowSSDT);
				SetGlobalVeriable(enumOriginShadowKiServiceTable, (ULONG)OriginShadowServiceTable);

				ReloadShadowSSDT->NumberOfServices = ShadowSSDT->NumberOfServices;
				ReloadShadowSSDT->ParamTableBase = ShadowSSDT->ParamTableBase;
				ReloadShadowSSDT->ServiceCounterTableBase = ShadowSSDT->ServiceCounterTableBase;
				ReloadShadowSSDT->ServiceTableBase = ReloadShadowServiceTable;
				SetGlobalVeriable(enumReloadShadowSSDT, (ULONG)ReloadShadowSSDT);
				SetGlobalVeriable(enumReloadShadowKiServiceTable, (ULONG)ReloadShadowServiceTable);

				return TRUE;
			}
		}

		if (NT_SUCCESS(GetRawShadowAddr(ShadowSSDT, (PVOID)OldWin32kBase, (PVOID *)&pServiceTable)) && pServiceTable)
		{
			ULONG i = 0, nCnt = ShadowSSDT->NumberOfServices;

			memset(OriginShadowServiceTable, 0, ShadowSSDT->NumberOfServices * sizeof(DWORD));
			memset(ReloadShadowServiceTable, 0, ShadowSSDT->NumberOfServices * sizeof(DWORD));
			memset(OriginShadowSSDT, 0, sizeof(ServiceDescriptorTableEntry_t));
			memset(ReloadShadowSSDT, 0, sizeof(ServiceDescriptorTableEntry_t));

			if (NewWin32kBase)
			{
				KdPrint(("OldWin32kBase: 0x%08X   NewWin32kBase: 0x%08X\n", OldWin32kBase, NewWin32kBase));

				for (i = 0; i < nCnt; i++)
				{
					OriginShadowServiceTable[i] = (ULONG)pServiceTable[i];
					ReloadShadowServiceTable[i] = (ULONG)OriginShadowServiceTable[i] - (ULONG)OldWin32kBase + (ULONG)NewWin32kBase; 
					KdPrint(("%d reload: 0x%08X, origin: 0x%08X, now: 0x%08X\n", i, ReloadShadowServiceTable[i], OriginShadowServiceTable[i], ShadowSSDT->ServiceTableBase[i]));
				}

				OriginShadowSSDT->NumberOfServices = ShadowSSDT->NumberOfServices;
				OriginShadowSSDT->ParamTableBase = ShadowSSDT->ParamTableBase;
				OriginShadowSSDT->ServiceCounterTableBase = ShadowSSDT->ServiceCounterTableBase;
				OriginShadowSSDT->ServiceTableBase = OriginShadowServiceTable;
				SetGlobalVeriable(enumOriginShadowSSDT, (ULONG)OriginShadowSSDT);
				SetGlobalVeriable(enumOriginShadowKiServiceTable, (ULONG)OriginShadowServiceTable);

				ReloadShadowSSDT->NumberOfServices = ShadowSSDT->NumberOfServices;
				ReloadShadowSSDT->ParamTableBase = ShadowSSDT->ParamTableBase;
				ReloadShadowSSDT->ServiceCounterTableBase = ShadowSSDT->ServiceCounterTableBase;
				ReloadShadowSSDT->ServiceTableBase = ReloadShadowServiceTable;
				SetGlobalVeriable(enumReloadShadowSSDT, (ULONG)ReloadShadowSSDT);
				SetGlobalVeriable(enumReloadShadowKiServiceTable, (ULONG)ReloadShadowServiceTable);
			}
			else
			{
				for (i = 0; i < nCnt; i++)
				{
					OriginShadowServiceTable[i] = (ULONG)pServiceTable[i];
				}

				OriginShadowSSDT->NumberOfServices = ShadowSSDT->NumberOfServices;
				OriginShadowSSDT->ParamTableBase = ShadowSSDT->ParamTableBase;
				OriginShadowSSDT->ServiceCounterTableBase = ShadowSSDT->ServiceCounterTableBase;
				OriginShadowSSDT->ServiceTableBase = OriginShadowServiceTable;
				SetGlobalVeriable(enumOriginShadowSSDT, (ULONG)OriginShadowSSDT);
				SetGlobalVeriable(enumOriginShadowKiServiceTable, (ULONG)OriginShadowServiceTable);
			}
			
			MzfExFreePoolWithTag(pServiceTable, 0);
			return TRUE;
		}
	}

	if (OriginShadowServiceTable)
		MzfExFreePoolWithTag(OriginShadowServiceTable, 0);

	if (ReloadShadowServiceTable)
		MzfExFreePoolWithTag(ReloadShadowServiceTable, 0);

	if (OriginShadowSSDT)
		MzfExFreePoolWithTag(OriginShadowSSDT, 0);

	if (ReloadShadowSSDT)
		MzfExFreePoolWithTag(ReloadShadowSSDT, 0);

	if (NewWin32kBase)
		MzfExFreePoolWithTag((PVOID)NewWin32kBase, 0);

	return FALSE;
}

BOOL ReloadWin32k()
{
	WCHAR szCsrss[] = {'c','s','r','s','s','.','e','x','e','\0'};
	PEPROCESS pCsrss = LookupProcessByName(szCsrss);
	PServiceDescriptorTableEntry_t pShadowServiceTable = NULL;
	KAPC_STATE as;
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!pCsrss || !MzfKeStackAttachProcess || !MzfKeUnstackDetachProcess || !MzfMmIsAddressValid)
	{
		KdPrint(("get csrss.exe error\n"));
		return FALSE;
	}

	KdPrint(("csrss.exe: 0x%08X\n", pCsrss));
	
	SetGlobalVeriable(enumCsrssEprocess, (ULONG)pCsrss);
	MzfKeStackAttachProcess(pCsrss, &as);

	pShadowServiceTable = GetAddrOfShadowTable();
	KdPrint(("pShadowServiceTable: 0x%08X\n", pShadowServiceTable));

	if (pShadowServiceTable								&& 
		MzfMmIsAddressValid(pShadowServiceTable)		&&
		&pShadowServiceTable[1]							&&
		MzfMmIsAddressValid(&pShadowServiceTable[1])	)
	{
		CHAR szWin32k[] = {'w','i','n','3','2','k','.','s','y','s','\0'};
		ULONG ulWin32kBase = 0, ulWin32kSize = 0; 

		ulWin32kBase = (ULONG)LookupKernelModuleByName(szWin32k, &ulWin32kSize);
		if (ulWin32kBase && ulWin32kSize)
		{
			SetGlobalVeriable(enumOriginWin32kBase, ulWin32kBase);
			SetGlobalVeriable(enumWin32kSize, ulWin32kSize);
			SetGlobalVeriable(enumNowShadowSSDT, (ULONG)&pShadowServiceTable[1]);
			SetGlobalVeriable(enumShadowSsdtFunctionsCnt, pShadowServiceTable[1].NumberOfServices);
			KdPrint(("OldWin32kBase: 0x%08X\n", ulWin32kBase));
			if (InitReloadWin32K(&pShadowServiceTable[1], ulWin32kBase))
			{
				KdPrint(("Reload Win32k success\n"));
			}
		}
 	}

	MzfKeUnstackDetachProcess(&as);

	return TRUE;
}