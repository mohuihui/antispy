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
/********************************************************************
	created:	2012/06/05	16:59
	file:		Process.c
	author:		莫灰灰
	blog：		http://blog.csdn.net/hu3167343

	purpose:	进程检测模块
*********************************************************************/

#include "Process.h"
#include "CommonFunction.h"
#include "disasm.h"
#include "HookPort.h"
#include "ProcessModule.h"
#include "InitWindows.h"
#include "InlineHook.h"
#include "ldasm.h"
#include "ProcessHandle.h"
#include "CommonFunction.h"
#include "ProcessThread.h"

////////////////////////////////////////////////////////////////////////////

BOOL GetPathBySectionObject(
							IN PSECTION_OBJECT pSectionObject, 
							OUT WCHAR* szPath
							)
{
	BOOL bGetPath = FALSE;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!MzfMmIsAddressValid)
	{
		return FALSE;
	}

	if (pSectionObject && szPath && KeGetCurrentIrql() <= APC_LEVEL)
	{
		PSEGMENT pSegment = NULL;
		PCONTROL_AREA pControlArea = NULL;
		PFILE_OBJECT pFileObject = NULL;
		
		__try
		{
			if (MzfMmIsAddressValid(pSectionObject))
			{
				pSegment = (PSEGMENT)pSectionObject->Segment;
				if (pSegment && MzfMmIsAddressValid(pSegment))
				{
					pControlArea = pSegment->ControlArea;
					if (pControlArea && MzfMmIsAddressValid(pControlArea))
					{
						pFileObject = pControlArea->FilePointer;

						if ( 
							WinVersion == enumWINDOWS_7				||
							WinVersion == enumWINDOWS_VISTA			||
							WinVersion == enumWINDOWS_VISTA_SP1_SP2	||
							WinVersion == enumWINDOWS_8
							)
						{
							pFileObject = (PFILE_OBJECT)((ULONG)pFileObject & 0xFFFFFFF8);
						}

						bGetPath = GetPathByFileObject(pFileObject, szPath);
						if (!bGetPath)
						{
							KdPrint(("pSectionObject: 0x%08X, pFileObject: 0x%08X\n", pSectionObject, pFileObject));
						}
					}
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			WCHAR szUnknow[] = {'U','n','k','n','o','w','\0'};
			KdPrint(("section get process path into __except\n"));
			wcscpy(szPath, /*L"Unknow"*/szUnknow);
		}
	}

	return bGetPath;
}

////////////////////////////////////////////////////////////////////////////

BOOL GetPathByPeb(
				  PEPROCESS pEprocess, 
				  OUT WCHAR* szPath
				  )
{
	BOOL bGetPath = FALSE;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);

	if (!MzfMmIsAddressValid || 
		!MzfIoGetCurrentProcess || 
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess || 
		!MzfProbeForRead ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag)
	{
		return FALSE;
	}

	if (pEprocess && szPath && KeGetCurrentIrql() < DISPATCH_LEVEL)
	{
		PMY_PEB peb = NULL;
		ULONG PebOffset = GetGlobalVeriable(enumPebOffset_EPROCESS);

		if (!MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + PebOffset)))
		{
			return bGetPath;
		}

		peb = *(PMY_PEB*)((ULONG)pEprocess + PebOffset);

		if ((ULONG)peb > 0 && (ULONG)peb < SYSTEM_ADDRESS_START)
		{
			KAPC_STATE ks;
			PPEB_LDR_DATA pLdr = NULL;
			BOOL bAttach = FALSE;
			PVOID pBuffer = MzfExAllocatePoolWithTag(PagedPool, MAX_PATH * sizeof(WCHAR), MZFTAG);
			
			if (pBuffer)
			{
				memset(pBuffer, 0, MAX_PATH * sizeof(WCHAR));

				if (MzfIoGetCurrentProcess() != pEprocess)
				{
					MzfKeStackAttachProcess(pEprocess, &ks);
					bAttach = TRUE;
				}

				__try
				{
					MzfProbeForRead(peb, sizeof(MY_PEB), 1);

					pLdr = (PPEB_LDR_DATA)(peb->Ldr);
					if ((ULONG)pLdr > 0 && (ULONG)pLdr < SYSTEM_ADDRESS_START)
					{
						PLIST_ENTRY entry = NULL; 
						PLDR_DATA_TABLE_ENTRY pLdrEntry = NULL;

						MzfProbeForRead(pLdr, sizeof(PEB_LDR_DATA), 1);

						entry = pLdr->InLoadOrderModuleList.Flink;
						pLdrEntry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
						if ((ULONG)pLdrEntry > 0 && (ULONG)pLdrEntry < SYSTEM_ADDRESS_START)
						{
							MzfProbeForRead(pLdrEntry, sizeof(LDR_DATA_TABLE_ENTRY), 1);

							if (pLdrEntry->FullDllName.Buffer && pLdrEntry->FullDllName.Length)
							{
								ULONG uLen = CmpAndGetStringLength(&pLdrEntry->FullDllName, MAX_PATH);
								MzfProbeForRead(pLdrEntry->FullDllName.Buffer, uLen * sizeof(WCHAR), sizeof(WCHAR));
								wcsncpy((WCHAR *)pBuffer, pLdrEntry->FullDllName.Buffer, uLen);
								bGetPath = TRUE;
							}
						}
					}
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					WCHAR szUnknow[] = {'U','n','k','n','o','w','\0'};
					wcscpy(szPath, /*L"Unknow"*/szUnknow);
					KdPrint(("peb get process path by peb in __except\n"));
				}

				if (bAttach)
				{
					MzfKeUnstackDetachProcess(&ks);
					bAttach = FALSE;
				}

				if (bGetPath)
				{
					wcscpy(szPath, (WCHAR *)pBuffer);
				}
				
				MzfExFreePoolWithTag(pBuffer, 0);
			}
		}
	}

	return bGetPath;
}

PSECTION_OBJECT Win2kGetSectionObjectByEprocess(PEPROCESS pEprocess)
{
	PSECTION_OBJECT pSection = NULL;
	KAPC_STATE ApcState;
	NTSTATUS status;
	BOOL bAttach = FALSE;
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfIoGetCurrentProcess || !MzfKeStackAttachProcess || !MzfKeUnstackDetachProcess || !MzfObReferenceObjectByHandle ||!MzfObfDereferenceObject)
	{
		return FALSE;
	}

	if (MzfIoGetCurrentProcess() != pEprocess)
	{
		MzfKeStackAttachProcess((PKPROCESS)pEprocess, &ApcState);
		bAttach = TRUE;
	}
	
	status = MzfObReferenceObjectByHandle(*(PHANDLE)((ULONG)pEprocess + 0x1ac), 0, NULL, KernelMode, &pSection, NULL);
	if ( NT_SUCCESS(status) )  
	{
		MzfObfDereferenceObject(pSection);
		KdPrint(("SectionObject = %x\n", pSection));
	}

	if (bAttach)
	{
		MzfKeUnstackDetachProcess( &ApcState );
	}

	return pSection;
}

BOOL  
GetProcessPathByEprocess(
						 IN PEPROCESS pEprocess, 
						 OUT WCHAR* szPath
						 )
{
	BOOL bGetPath = FALSE;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	ULONG SectionObjectOffset = GetGlobalVeriable(enumSectionObjectOffset_EPROCESS);
	ULONG PebOffset = GetGlobalVeriable(enumPebOffset_EPROCESS);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid && pEprocess && szPath)
	{
		PSECTION_OBJECT pSectionObject = NULL;

		if (WinVersion == enumWINDOWS_2K)
		{
			pSectionObject = Win2kGetSectionObjectByEprocess(pEprocess);
		}
		else if (MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + SectionObjectOffset)))
		{
			pSectionObject = *(PSECTION_OBJECT *)((ULONG)pEprocess + SectionObjectOffset);
		}
		
		if (pSectionObject && MzfMmIsAddressValid(pSectionObject))
		{
			bGetPath = GetPathBySectionObject(pSectionObject, szPath);
		}
		
		if (!bGetPath && MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + PebOffset)))
		{
			KdPrint(("get path by GetPathByPeb\n"));
			bGetPath = GetPathByPeb(pEprocess, szPath);
		}

		if (!bGetPath)
		{
			WCHAR szUnknow[] = {'U','n','k','n','o','w','\0'};
			wcscpy(szPath, /*L"Unknow"*/szUnknow);
		}
	}

	return bGetPath;
}

BOOL GetProcessPathByPid(
						 IN ULONG Pid, 
						 OUT WCHAR* szPath
						 )
{
	BOOL bGetPath = FALSE;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	ULONG SectionObjectOffset = GetGlobalVeriable(enumSectionObjectOffset_EPROCESS);
	ULONG PebOffset = GetGlobalVeriable(enumPebOffset_EPROCESS);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (MzfPsLookupProcessByProcessId &&
		MzfObfDereferenceObject &&
		MzfMmIsAddressValid  && 
		szPath)
	{
		PEPROCESS pEprocess = NULL;
		if (NT_SUCCESS(MzfPsLookupProcessByProcessId((HANDLE)Pid, &pEprocess)))
		{
			PSECTION_OBJECT pSectionObject = NULL;

			if (WinVersion == enumWINDOWS_2K)
			{
				pSectionObject = Win2kGetSectionObjectByEprocess(pEprocess);
			}
			else if (MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + SectionObjectOffset)))
			{
				pSectionObject = *(PSECTION_OBJECT *)((ULONG)pEprocess + SectionObjectOffset);
			}

			if (pSectionObject && MzfMmIsAddressValid(pSectionObject))
			{
				bGetPath = GetPathBySectionObject(pSectionObject, szPath);
			}

			if (!bGetPath && MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + PebOffset)))
			{
				KdPrint(("get path by GetPathByPeb\n"));
				bGetPath = GetPathByPeb(pEprocess, szPath);
			}

			if (!bGetPath)
			{
				WCHAR szUnknow[] = {'U','n','k','n','o','w','\0'};
				wcscpy(szPath, /*L"Unknow"*/szUnknow);
			}

			MzfObfDereferenceObject(pEprocess);
		}
	}

	return bGetPath;
}

//////////////////////////////////////////////////////////////////// 

BOOL IsProcessAlreadyInList(PALL_PROCESSES pProcesses, ULONG nCnt, PEPROCESS pEprocess)
{
	BOOL bRet = FALSE;
	ULONG dwNum = 0, i = 0;

	if (!pEprocess || !pProcesses)
	{
		return TRUE;
	}

	dwNum = pProcesses->nCnt;
	dwNum = dwNum > nCnt ? nCnt : dwNum;

	for (i = 0; i < dwNum; i++)
	{
		if (pProcesses->ProcessInfo[i].ulEprocess == (ULONG)pEprocess)
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}

//////////////////////////////////////////////////////////////////// 

VOID AddProcessToList(PALL_PROCESSES pProcesses, ULONG nCnt, PEPROCESS pEprocess)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG dwNum = 0;
	pfnPsGetProcessCreateTimeQuadPart MzfPsGetProcessCreateTimeQuadPart = (pfnPsGetProcessCreateTimeQuadPart)GetGlobalVeriable(enumPsGetProcessCreateTimeQuadPart);

	if (!MzfMmIsAddressValid || !pProcesses || !pEprocess ||
		!MzfPsGetProcessCreateTimeQuadPart ||
		!MzfMmIsAddressValid(pEprocess) || !IsRealProcess(pEprocess))
	{
		return;
	}
	
	dwNum = pProcesses->nCnt;

	if (nCnt > dwNum)
	{
		ULONG InheritedFromUniqueProcessIdOffset = GetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS);
		ULONG pid = *(PULONG)((ULONG)pEprocess + GetGlobalVeriable(enumObjectTableOffset_EPROCESS));

		if (pid && 
			MzfMmIsAddressValid((PVOID)pid) && 
			(pid = *(PULONG)(pid + GetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE))) > 0)
		{
			pProcesses->ProcessInfo[dwNum].ulPid = pid;
		}
		else
		{
			pfnPsGetProcessId MzfPsGetProcessId = (pfnPsGetProcessId)GetGlobalVeriable(enumPsGetProcessId);
			if (MzfPsGetProcessId)
			{	
				pProcesses->ProcessInfo[dwNum].ulPid = (ULONG)MzfPsGetProcessId(pEprocess);
			}
			else
			{
				pProcesses->ProcessInfo[dwNum].ulPid = *(PULONG)((ULONG)pEprocess + GetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS));
			}
		}

		pProcesses->ProcessInfo[dwNum].ulCreateTime = MzfPsGetProcessCreateTimeQuadPart(pEprocess);
		pProcesses->ProcessInfo[dwNum].ulParentPid = *(PULONG)((ULONG)pEprocess + InheritedFromUniqueProcessIdOffset);
		pProcesses->ProcessInfo[dwNum].ulEprocess = (ULONG)pEprocess;
		GetProcessPathByEprocess(pEprocess, pProcesses->ProcessInfo[dwNum].szPath);
	}

	pProcesses->nCnt++;
}

//////////////////////////////////////////////////////////////////// 

VOID ThreadToProcessAndInsertProcess(PETHREAD pThread, PALL_PROCESSES pProcesses, ULONG nCnt)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoThreadToProcess MzfIoThreadToProcess = (pfnIoThreadToProcess)GetGlobalVeriable(enumIoThreadToProcess);
	POBJECT_TYPE ThreadType = (POBJECT_TYPE)GetGlobalVeriable(enumPsThreadType);
	POBJECT_TYPE pObjectType = NULL;

	if (!MzfMmIsAddressValid || !MzfIoThreadToProcess || !ThreadType || !pThread || !MzfMmIsAddressValid((PVOID)pThread))
	{
		return;
	}

	pObjectType = KeGetObjectType((PVOID)pThread);

	if (pObjectType && pObjectType == ThreadType && IsRealThread(pThread))
	{
		PEPROCESS pEprocess1 = NULL, pEprocess2 = NULL;
		ULONG ApcStateOffset = 0, ProcessOffset = 0;
		
		ApcStateOffset = GetGlobalVeriable(enumApcStateOffset_KTHREAD);
		ProcessOffset = GetGlobalVeriable(enumProcessOffset_KAPC_STATE);

		pEprocess1 = MzfIoThreadToProcess(pThread);
		if (!IsProcessAlreadyInList(pProcesses, nCnt, pEprocess1))
		{
			AddProcessToList(pProcesses, nCnt, pEprocess1);
		}

		pEprocess2 = *(PEPROCESS*)((ULONG)pThread + ApcStateOffset + ProcessOffset);
		if (!IsProcessAlreadyInList(pProcesses, nCnt, pEprocess2) && IsRealProcess(pEprocess2))
		{
			AddProcessToList(pProcesses, nCnt, pEprocess2);
		}
	}
}

////////////////////////////////////////////////////////////////////////////

// 从0级表开始遍历
void BrowseTableL0(ULONG TableAddr, PALL_PROCESSES pProcesses, ULONG nCnt)
{
	ULONG Object = 0;
	ULONG ItemCount = 511;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || !TableAddr || !MzfMmIsAddressValid((PVOID)TableAddr))
	{
		return;
	}

	do 
	{
		TableAddr += 8; 

		if (MzfMmIsAddressValid((PVOID)TableAddr))
		{	
			Object = *(PULONG)TableAddr;
			Object &= 0xfffffff8; 

			ThreadToProcessAndInsertProcess((PETHREAD)Object, pProcesses, nCnt);
		}
	
	} while ( --ItemCount > 0 );
}

//////////////////////////////////////////////////////////////////// 

// 从1级表开始遍历
VOID BrowseTableL1(ULONG TableAddr, PALL_PROCESSES pProcesses, ULONG nCnt)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (TableAddr && MzfMmIsAddressValid((PVOID)TableAddr))
	{
		do 
		{
			BrowseTableL0( *(PULONG)TableAddr, pProcesses, nCnt );
			TableAddr += 4;
		} while (MzfMmIsAddressValid((PVOID)TableAddr) && *(PULONG)TableAddr != 0);
	}
}

//////////////////////////////////////////////////////////////////// 

// 从2级表开始遍历
VOID BrowseTableL2(ULONG TableAddr, PALL_PROCESSES pProcesses, ULONG nCnt)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (TableAddr && MzfMmIsAddressValid((PVOID)TableAddr))
	{
		do 
		{
			BrowseTableL1( *(PULONG)TableAddr, pProcesses, nCnt );
			TableAddr += 4;
		} while (MzfMmIsAddressValid((PVOID)TableAddr) && *(PULONG)TableAddr != 0);
	}
}

//////////////////////////////////////////////////////////////////// 

VOID ScanNotWin2KHandleTable(ULONG uHandleTable, PALL_PROCESSES pProcesses, ULONG nCnt)
{
	ULONG HandleTable = 0;
	ULONG TableCode = 0;
	ULONG flag = 0;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || !uHandleTable || !pProcesses || !MzfMmIsAddressValid((PVOID)uHandleTable))
	{
		return;
	}
	
	HandleTable = *(PULONG)uHandleTable;
	if (HandleTable && MzfMmIsAddressValid((PVOID)HandleTable))
	{
		if (WinVersion == enumWINDOWS_8)
		{
			TableCode = *(PULONG)(HandleTable + 8);
		}
		else
		{
			TableCode = *(PULONG)(HandleTable);
		}

		flag = TableCode & 3;
		TableCode &= 0xfffffffc;  

		switch (flag)
		{
		case 0:
			BrowseTableL0(TableCode, pProcesses, nCnt);
			break;

		case 1:
			BrowseTableL1(TableCode, pProcesses, nCnt);
			break;

		case 2:
			BrowseTableL2(TableCode, pProcesses, nCnt);
			break; 

		default:
			KdPrint(("TableCode error\n"));
		} 			
	}
}

//////////////////////////////////////////////////////////////////// 

// VOID ScanWin2KHandleTable(PWIN2K_HANDLE_TABLE HandleTable, PPROCESSES pProcesses)
// {
// 	int i, j, k;
// 	PHANDLE_TABLE_ENTRY Entry;
// 
// 	if (!HandleTable)
// 	{
// 		return;
// 	}
// 
// 	for (i = 0; i < 0x100; i++)
// 	{
// 		if (HandleTable->Table[i])
// 		{
// 			for (j = 0; j < 0x100; j++)
// 			{
// 				if (HandleTable->Table[i][j])
// 				{
// 					for (k = 0; k < 0x100; k++)
// 					{
// 						Entry = &HandleTable->Table[i][j][k];
// 						if (Entry->Object)
// 						{
// 							ThreadToProcessAndInsertProcess((PETHREAD)((ULONG)(Entry->Object) | 0x80000000), pProcesses);
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// }

//////////////////////////////////////////////////////////////////// 

VOID EnumProcessByPspCidTable(PALL_PROCESSES pProcesses, ULONG nCnt)
{
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	ULONG PspCidTable = GetGlobalVeriable(enumPspCidTable);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || !pProcesses)
	{
		return;
	}

	if (!PspCidTable)
	{
		GetPspCidTable();
		PspCidTable = GetGlobalVeriable(enumPspCidTable);
	}

	KdPrint(("Pspcidtable: 0x%08X\n", PspCidTable));

	if ( PspCidTable && MzfMmIsAddressValid((PVOID)PspCidTable) )
	{
		if (WinVersion == enumWINDOWS_2K)
		{	
		//	ScanWin2KHandleTable(*(PWIN2K_HANDLE_TABLE*)PspCidTable, pProcesses);
		}
		else
		{
			ScanNotWin2KHandleTable(PspCidTable, pProcesses, nCnt);
		}
 	}
}

//////////////////////////////////////////////////////////////////// 

VOID AddIdleProcess(PALL_PROCESSES pProcesses, ULONG nCnt)
{
	ULONG uIdleAddr = 0;
	ULONG IdleThreadOffset = GetGlobalVeriable(enumIdleThreadOffset_KPRCB);
	ULONG ApcStateOffset = GetGlobalVeriable(enumApcStateOffset_KTHREAD);
	ULONG ProcessOffset = GetGlobalVeriable(enumProcessOffset_KAPC_STATE);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	pfnPsGetProcessCreateTimeQuadPart MzfPsGetProcessCreateTimeQuadPart = (pfnPsGetProcessCreateTimeQuadPart)GetGlobalVeriable(enumPsGetProcessCreateTimeQuadPart);

	if (!MzfMmIsAddressValid || !pProcesses || !MzfPsGetProcessCreateTimeQuadPart)
	{
		return;
	}

	if (BuildNumber >= 6001)
	{
		ULONG PsInitialSystemProcessAddress = GetGlobalVeriable(enumPsInitialSystemProcessAddress);

		if (PsInitialSystemProcessAddress && MzfMmIsAddressValid((PVOID)(PsInitialSystemProcessAddress + sizeof(ULONG))))
		{
			uIdleAddr = *(PULONG)(PsInitialSystemProcessAddress + sizeof(ULONG));
		}
	}
	else
	{
		__asm{
			push eax
			mov eax, FS:[0x20]					;得到KPRCB的地址
			mov uIdleAddr, eax
			pop eax
		}

		if (uIdleAddr && MzfMmIsAddressValid((PVOID)uIdleAddr))
		{
			uIdleAddr += IdleThreadOffset;
			uIdleAddr = *(PULONG)uIdleAddr;
			uIdleAddr += ApcStateOffset;
			uIdleAddr += ProcessOffset;
			uIdleAddr = *(PULONG)uIdleAddr;
		}
	}

	if (uIdleAddr && MzfMmIsAddressValid(((PVOID)uIdleAddr)))
	{
		if (nCnt > pProcesses->nCnt)
		{
			ULONG nNum = pProcesses->nCnt;
			WCHAR szIdle[] = {'I', 'd', 'l', 'e', '\0'};
			pProcesses->ProcessInfo[nNum].ulPid = 0;
			pProcesses->ProcessInfo[nNum].ulParentPid = 0;
			pProcesses->ProcessInfo[nNum].ulEprocess = uIdleAddr;
			pProcesses->ProcessInfo[nNum].ulCreateTime = MzfPsGetProcessCreateTimeQuadPart((PEPROCESS)uIdleAddr);
			wcscpy(pProcesses->ProcessInfo[nNum].szPath, szIdle);
		}

		pProcesses->nCnt++;
	}
}

////////////////////////////////////////////////////////////////////////////

VOID AddSystemProcess(PALL_PROCESSES pProcesses, ULONG nCnt)
{
	ULONG nNum = pProcesses->nCnt;
	PEPROCESS pSystemProcess = (PEPROCESS)GetGlobalVeriable(enumSystemProcess);
	pfnPsGetProcessCreateTimeQuadPart MzfPsGetProcessCreateTimeQuadPart = (pfnPsGetProcessCreateTimeQuadPart)GetGlobalVeriable(enumPsGetProcessCreateTimeQuadPart);

	if ( nCnt > nNum && pSystemProcess > 0 && MmIsAddressValid(pSystemProcess) && MzfPsGetProcessCreateTimeQuadPart )
	{
		WCHAR szSystem[] = {'S', 'y', 's', 't', 'e', 'm', '\0'};
		ULONG InheritedFromUniqueProcessIdOffset = GetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS);
		ULONG pid = *(PULONG)((ULONG)pSystemProcess + GetGlobalVeriable(enumObjectTableOffset_EPROCESS));
		pid = *(PULONG)(pid + GetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE));

		if (pid)
		{
			pProcesses->ProcessInfo[nNum].ulPid = pid;
		}
		else
		{
			pfnPsGetProcessId MzfPsGetProcessId = (pfnPsGetProcessId)GetGlobalVeriable(enumPsGetProcessId);
			if (MzfPsGetProcessId)
			{	
				pProcesses->ProcessInfo[nNum].ulPid = (ULONG)MzfPsGetProcessId(pSystemProcess);
			}
			else
			{
				pProcesses->ProcessInfo[nNum].ulPid = *(PULONG)((ULONG)pSystemProcess + GetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS));
			}
		}

		pProcesses->ProcessInfo[nNum].ulParentPid = 0;
		pProcesses->ProcessInfo[nNum].ulEprocess = (ULONG)pSystemProcess;
		pProcesses->ProcessInfo[nNum].ulCreateTime = MzfPsGetProcessCreateTimeQuadPart(pSystemProcess);
		wcscpy(pProcesses->ProcessInfo[nNum].szPath, szSystem);
	}
	
	pProcesses->nCnt++;
}

//////////////////////////////////////////////////////////////////// 

VOID EnumProcessBySearchMemory()
{
	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG KernelSize = GetGlobalVeriable(enumKernelModuleSize);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	SYSINF_PAGE_COUNT NumberOfPhysicalPages = 0, HighestPhysicalPageNumber = 0, nMaxPhysicalPageSize;
	ULONG Address = 0;

	pfnMmIsNonPagedSystemAddressValid MzfMmIsNonPagedSystemAddressValid = (pfnMmIsNonPagedSystemAddressValid)GetGlobalVeriable(enumMmIsNonPagedSystemAddressValid);
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnMmGetPhysicalAddress MzfMmGetPhysicalAddress = (pfnMmGetPhysicalAddress)GetGlobalVeriable(enumMmGetPhysicalAddress);
	pfnMmGetVirtualForPhysical MzfMmGetVirtualForPhysical = (pfnMmGetVirtualForPhysical)GetGlobalVeriable(enumMmGetVirtualForPhysical);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoAllocateMdl MzfIoAllocateMdl = (pfnIoAllocateMdl)GetGlobalVeriable(enumIoAllocateMdl);
	pfnMmBuildMdlForNonPagedPool MzfMmBuildMdlForNonPagedPool = (pfnMmBuildMdlForNonPagedPool)GetGlobalVeriable(enumMmBuildMdlForNonPagedPool);
	pfnMmMapLockedPagesSpecifyCache MzfMmMapLockedPagesSpecifyCache = (pfnMmMapLockedPagesSpecifyCache)GetGlobalVeriable(enumMmMapLockedPagesSpecifyCache);
	pfnMmUnmapLockedPages MzfMmUnmapLockedPages = (pfnMmUnmapLockedPages)GetGlobalVeriable(enumMmUnmapLockedPages);
	pfnIoFreeMdl MzfIoFreeMdl = (pfnIoFreeMdl)GetGlobalVeriable(enumIoFreeMdl);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (KernelBase &&
		KernelSize &&
		ProcessType &&
		MzfMmIsNonPagedSystemAddressValid &&
		MzfNtQuerySystemInformation &&
		MzfMmGetPhysicalAddress &&
		MzfMmGetVirtualForPhysical &&
		MzfMmIsAddressValid &&
		MzfIoAllocateMdl &&
		MzfMmBuildMdlForNonPagedPool &&
		MzfMmMapLockedPagesSpecifyCache &&
		MzfMmMapLockedPagesSpecifyCache &&
		MzfMmUnmapLockedPages &&
		MzfIoFreeMdl &&
		MzfPsGetCurrentThread)
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		SYSTEM_BASIC_INFORMATION sbi;
		NTSTATUS status;

		KdPrint(("enter EnumProcessBySearchMemory\n"));
		memset(&sbi, 0, sizeof(SYSTEM_BASIC_INFORMATION));
		status = MzfNtQuerySystemInformation(SystemBasicInformation, &sbi, sizeof(SYSTEM_BASIC_INFORMATION), NULL);
		if (NT_SUCCESS(status))
		{
			NumberOfPhysicalPages = sbi.NumberOfPhysicalPages;
			HighestPhysicalPageNumber = sbi.HighestPhysicalPageNumber;
			if (NumberOfPhysicalPages > HighestPhysicalPageNumber)
			{
				NumberOfPhysicalPages = HighestPhysicalPageNumber;
			}
			KdPrint(("NumberOfPhysicalPages, %d, LowestPhysicalPageNumber: %d, HighestPhysicalPageNumber: %d\n", sbi.NumberOfPhysicalPages, sbi.LowestPhysicalPageNumber, sbi.HighestPhysicalPageNumber));

			nMaxPhysicalPageSize = NumberOfPhysicalPages * PAGE_SIZE;
			Address = (KernelSize + KernelBase) & 0xFFFFF000;

			while ( Address < 0xFFFF0000 )
			{
				if (MzfMmIsAddressValid((PVOID)Address) &&
					MzfMmIsNonPagedSystemAddressValid((PVOID)Address))
				{
					PHYSICAL_ADDRESS PhysicalAddress = MzfMmGetPhysicalAddress((PVOID)Address);
					if ( (PhysicalAddress.HighPart || PhysicalAddress.LowPart) &&
						 PhysicalAddress.QuadPart < nMaxPhysicalPageSize)
					{
						if (MzfMmGetVirtualForPhysical(PhysicalAddress) == (PVOID)Address)
						{
							PMDL pMdl = MzfIoAllocateMdl((PVOID)Address, PAGE_SIZE, FALSE, FALSE, NULL);
							if (pMdl)
							{
								PVOID pMdlAddress = NULL;
								ULONG a1 = 0, a2 = 0, a3 = 0, a4= 0;
								ULONG v9 = (0x258 + 0x18) & 0xFFFFFFF8;
								ULONG v31 = 0xFE8 - 0x258;
								MmBuildMdlForNonPagedPool(pMdl);
								pMdlAddress = MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, 0, NormalPagePriority);
								if (pMdlAddress)
								{
									a1 = 0;
									a2 = (ULONG)pMdlAddress + 0x18;
									a3 = (ULONG)pMdlAddress + 0xC;
									a4 = (ULONG)pMdlAddress + 8;

									do
									{
										if (IsRealProcess((PEPROCESS)a2))
										{
											KdPrint(("process: 0x%08X, image: %s\n", Address + a1 + 0x18, (char*)(Address + a1 + 0x18 + 0x174)));
											a1 += v9;
											a2 += v9;
											a3 += v9;
											a4 += v9;
										}

										a1 += 8;
										a2 += 8;
										a3 += 8;
										a4 += 8;

									}while ( a1 <= v31 );

									MmUnmapLockedPages(pMdlAddress, pMdl);
								}
								
								IoFreeMdl(pMdl);
							}
						}
					}
				}

				Address += PAGE_SIZE;
			}
		}

		RecoverPreMode(pThread, PreMode);
	}
}

//////////////////////////////////////////////////////////////////// 

VOID EnumProcessByApi(PALL_PROCESSES pProcesses, ULONG nCnt)
{
	ULONG uSize = 0x10000;
	ULONG uRet = 0, i = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pBuffer = NULL;
	PSYSTEM_PROCESS_INFORMATION pSpiNext = NULL;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject  = (pfnObfDereferenceObject )GetGlobalVeriable(enumObfDereferenceObject );
	pfnPsGetCurrentThread MzfKeGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	PEPROCESS pSystemProcess = (PEPROCESS)GetGlobalVeriable(enumSystemProcess);

	if (!pProcesses						||
		!pSystemProcess					||
		!MzfExAllocatePoolWithTag		|| 
		!MzfNtQuerySystemInformation	||
		!MzfExFreePoolWithTag			||
		!MzfPsLookupProcessByProcessId	||
		!MzfObfDereferenceObject		||
		!MzfKeGetCurrentThread)
	{
		return;
	}

	do 
	{
		pBuffer = MzfExAllocatePoolWithTag(PagedPool, uSize, MZFTAG);
		if (pBuffer)
		{
			PETHREAD pThread = MzfKeGetCurrentThread();
			CHAR PreMode = ChangePreMode(pThread);

			status = MzfNtQuerySystemInformation(SystemProcessesInformation, pBuffer, uSize, &uRet);
			if (NT_SUCCESS(status))
			{
				pSpiNext = (PSYSTEM_PROCESS_INFORMATION)pBuffer;

				while (1)
				{
					PEPROCESS pProcess = NULL;

					if (NT_SUCCESS(MzfPsLookupProcessByProcessId((HANDLE)(pSpiNext->ProcessId), &pProcess)))
					{
						if (pProcess == pSystemProcess)
						{
							AddSystemProcess(pProcesses, nCnt);
						}
						else
						{
							AddProcessToList(pProcesses, nCnt, pProcess);
						}
						
						MzfObfDereferenceObject(pProcess);
					}

					if (pSpiNext->NextEntryOffset == 0)
					{
						break;
					}

					pSpiNext = (PSYSTEM_PROCESS_INFORMATION)((ULONG)pSpiNext + pSpiNext->NextEntryOffset);  
				}
			}
			else
			{
				uSize *= 2;
			}

			RecoverPreMode(pThread, PreMode);
			MzfExFreePoolWithTag(pBuffer, 0);
			pBuffer = NULL;
		}
		else
		{
			break;
		}

	} while (status == STATUS_INFO_LENGTH_MISMATCH && ++i < 10);
}

//////////////////////////////////////////////////////////////////// 

NTSTATUS PspListProcess(PALL_PROCESSES pProcesses, ULONG uOutSize)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG nCnt = (uOutSize - sizeof(ALL_PROCESSES)) / sizeof(PROCESS_INFO);

	if (!pProcesses)
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	AddIdleProcess(pProcesses, nCnt);
	EnumProcessByApi(pProcesses, nCnt);
	EnumProcessByPspCidTable(pProcesses, nCnt);

	if (nCnt >= pProcesses->nCnt)
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
// 枚举进程
//
NTSTATUS ListProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PALL_PROCESSES pProcesses = (PALL_PROCESSES)pOutBuffer;
	
	
	// 参数检查
	if (!pInBuffer ||
		uInSize != sizeof(OPERATE_TYPE) ||
		!pOutBuffer ||
		uOutSize < sizeof(ALL_PROCESSES))
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	return PspListProcess(pProcesses, uOutSize);
}

///////////////////////////////////////////////////////////////////////

//
// 调用内核函数NtTerminateProcess结束进程
// 
NTSTATUS KillProcessByNtTerminateProcess(PEPROCESS pEprocess)
{
	pfnNtTerminateProcess MzfNtTerminateProcess = (pfnNtTerminateProcess)GetGlobalVeriable(enumNtTerminateProcess);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	HANDLE hProcess;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	KdPrint(("KillProcessByNtTerminateProcess \n"));

	if (!MzfNtTerminateProcess ||
		!MzfObOpenObjectByPointer ||
		!MzfNtClose ||
		!MzfPsGetCurrentThread ||
		!ProcessType)
	{
		return status;
	}

	status = MzfObOpenObjectByPointer(pEprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, ProcessType, KernelMode, &hProcess);
	if (NT_SUCCESS(status))
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		status = MzfNtTerminateProcess(hProcess, 0);
		MzfNtClose(hProcess);
		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

///////////////////////////////////////////////////////////////////////

//
// 往进程内存中写入0xcc
//
void KillProcessByFillUp0xcc(PEPROCESS pEprocess)
{
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnMmGetPhysicalAddress MzfMmGetPhysicalAddress = (pfnMmGetPhysicalAddress)GetGlobalVeriable(enumMmGetPhysicalAddress);
	pfnMmGetVirtualForPhysical MzfMmGetVirtualForPhysical = (pfnMmGetVirtualForPhysical)GetGlobalVeriable(enumMmGetVirtualForPhysical);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);

	KdPrint(("KillProcessByFillUp0xcc \n"));

	if (MzfNtQuerySystemInformation &&
		MzfMmGetPhysicalAddress &&
		MzfMmGetVirtualForPhysical &&
		MzfPsGetCurrentThread &&
		MzfKeStackAttachProcess && 
		MzfKeUnstackDetachProcess &&
		MzfIoGetCurrentProcess) 
	{
		BOOL bAttach = FALSE;
		KAPC_STATE as;
		PETHREAD pThread = NULL;
		CHAR PreMode = 0;
		SYSTEM_BASIC_INFORMATION sbi;
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		if (MzfIoGetCurrentProcess() != pEprocess)
		{
			MzfKeStackAttachProcess(pEprocess, &as);
			bAttach = TRUE;
		}
		
		pThread = MzfPsGetCurrentThread();
		PreMode = ChangePreMode(pThread);
		
		memset(&sbi, 0, sizeof(SYSTEM_BASIC_INFORMATION));
		status = MzfNtQuerySystemInformation(SystemBasicInformation, &sbi, sizeof(SYSTEM_BASIC_INFORMATION), NULL);
		RecoverPreMode(pThread, PreMode);

		if (NT_SUCCESS(status))
		{
			ULONG Address = 0x1000;
			SYSINF_PAGE_COUNT nMaxPhysicalPageSize = 0;
			SYSINF_PAGE_COUNT NumberOfPhysicalPages = sbi.NumberOfPhysicalPages;
			SYSINF_PAGE_COUNT HighestPhysicalPageNumber = sbi.HighestPhysicalPageNumber;

			if (NumberOfPhysicalPages > HighestPhysicalPageNumber)
			{
				NumberOfPhysicalPages = HighestPhysicalPageNumber;
			}

			nMaxPhysicalPageSize = NumberOfPhysicalPages * sbi.PageSize;

			do 
			{
				PHYSICAL_ADDRESS PhysicalAddress = MzfMmGetPhysicalAddress((PVOID)Address);
				if ( (PhysicalAddress.HighPart || PhysicalAddress.LowPart) &&
					PhysicalAddress.QuadPart < nMaxPhysicalPageSize)
				{
					if (MzfMmGetVirtualForPhysical(PhysicalAddress) == (PVOID)Address)
					{
						__try
						{
							WPOFF();
							memset((PVOID)Address, 0xcc, PAGE_SIZE);
							WPON();
						}
						__except(1)
						{
						}
					}
				}
				
				Address += PAGE_SIZE;

			} while (Address < SYSTEM_ADDRESS_START);
		}

		if (bAttach)
		{
			MzfKeUnstackDetachProcess(&as);
			bAttach = FALSE;
		}
	}
}

///////////////////////////////////////////////////////////////////////

//
// 杀死进程
//
NTSTATUS KillProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, tempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCESS pCp = (PCOMMUNICATE_PROCESS)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	
	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return status;
	}
	
	// 参数检查
	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_PROCESS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pEprocess = (PEPROCESS)pCp->op.KillProcess.pEprocess;
	nPid = pCp->op.KillProcess.nPid;

	if (nPid)
	{
		tempStatus = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);
		if (NT_SUCCESS(tempStatus) && !pEprocess)
		{
			pEprocess = pTempEprocess;
		}
	}

	if (!pEprocess)
	{
		return status;
	}

	KdPrint(("Enter KillProcess\n"));

	if ((ULONG)pEprocess > SYSTEM_ADDRESS_START						&& 
		MmIsAddressValid(pEprocess)									&&
		(ULONG)pEprocess != GetGlobalVeriable(enumAntiSpyEprocess)	&&
		IsRealProcess(pEprocess))
	{
		// 首先用0xcc填充
		KillProcessByFillUp0xcc(pEprocess);
		
		// 然后在调用Terminate函数结束之
		status = KillProcessByNtTerminateProcess(pEprocess);
	}

	if (NT_SUCCESS(tempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

///////////////////////////////////////////////////////////////////////

//
// 拷贝进程内存
//
NTSTATUS DumpProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, statusLookupProcess = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCESS pCp = (PCOMMUNICATE_PROCESS)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nBase = 0, nSize = 0, hPid = 0;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfPsLookupProcessByProcessId || !MzfObfDereferenceObject)
	{
		return status;
	}

	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_PROCESS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pEprocess = (PEPROCESS)pCp->op.DumpProcess.pEprocess;
	hPid = pCp->op.DumpProcess.nPid;

	if (hPid)
	{
		statusLookupProcess = MzfPsLookupProcessByProcessId((HANDLE)hPid, &pTempEprocess);
		if (NT_SUCCESS(statusLookupProcess) && !pEprocess)
		{
			pEprocess = pTempEprocess;
		}
	}

	if (!pEprocess)
	{
		return status;
	}

	KdPrint(("new DumpProcess\n"));
	
	if ((ULONG)pEprocess != GetGlobalVeriable(enumAntiSpyEprocess) && IsRealProcess(pEprocess))
	{
		pfnMmIsAddressValid	MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
		if (MzfMmIsAddressValid)
		{
			PSECTION_OBJECT pProcessSection = NULL;
			WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
			if (Version == enumWINDOWS_2K)
			{
				pProcessSection = Win2kGetSectionObjectByEprocess(pEprocess);
			}
			else
			{
				ULONG SectionOffset = GetGlobalVeriable(enumSectionObjectOffset_EPROCESS);
				pProcessSection = *(PSECTION_OBJECT *)(SectionOffset + (ULONG)pEprocess);
			}
			
			KdPrint(("pProcessSection : 0x%08X \n", pProcessSection));
			if (MzfMmIsAddressValid(pProcessSection))
			{
				PVOID pSegment = (PVOID)(pProcessSection->Segment);
				
				if (pSegment && MzfMmIsAddressValid(pSegment))
				{
					WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
					ULONG SizeOfSegment = 0, BasedAddress = 0, i = 0;
					PVOID pTempBuffer = NULL;
					KAPC_STATE as;
						
					if (WinVersion == enumWINDOWS_2K)
					{
						SizeOfSegment = ((PSEGMENT_2000)pSegment)->SizeOfSegment.LowPart;
						BasedAddress = (ULONG)((PSEGMENT_2000)pSegment)->SystemImageBase;
						if ( !BasedAddress )
						{
							BasedAddress = (ULONG)((PSEGMENT_2000)pSegment)->BasedAddress;
						}
					}
					else if (WinVersion == enumWINDOWS_XP || 
						 WinVersion == enumWINDOWS_2K3 ||
						 WinVersion == enumWINDOWS_2K3_SP1_SP2)
					{
						SizeOfSegment = ((PSEGMENT_XP)pSegment)->SizeOfSegment.LowPart;
						BasedAddress = (ULONG)((PSEGMENT_XP)pSegment)->SystemImageBase;
						if ( !BasedAddress )
						{
							BasedAddress = (ULONG)((PSEGMENT_XP)pSegment)->BasedAddress;
						}
					}
					else if (WinVersion == enumWINDOWS_VISTA ||
						WinVersion == enumWINDOWS_VISTA_SP1_SP2 ||
						WinVersion == enumWINDOWS_7)
					{
						SizeOfSegment = ((PSEGMENT_VISTA)pSegment)->SizeOfSegment.LowPart;
						BasedAddress = (ULONG)((PSEGMENT_VISTA)pSegment)->BasedAddress;
						if ( !BasedAddress )
						{
							BasedAddress = (ULONG)((PSEGMENT_VISTA)pSegment)->SystemImageBase;
						}
					}
					else if (WinVersion == enumWINDOWS_8)
					{
						SizeOfSegment = ((PSEGMENT_WIN8)pSegment)->SizeOfSegment.LowPart;
						BasedAddress = (ULONG)((PSEGMENT_WIN8)pSegment)->BasedAddress;
						if ( !BasedAddress )
						{
							BasedAddress = (ULONG)((PSEGMENT_WIN8)pSegment)->SystemImageBase;
						}
					}

					if (SizeOfSegment > uOutSize)
					{
						*(PULONG)pOutBuffer = SizeOfSegment;
						KdPrint(("STATUS_BUFFER_TOO_SMALL: %d\n", RtlNtStatusToDosError(STATUS_BUFFER_TOO_SMALL)));
						return STATUS_BUFFER_TOO_SMALL;
					}

					KdPrint(("dmp process -> BasedAddress: 0x%08X, SizeOfSegment: 0x%08X\n", BasedAddress, SizeOfSegment));

					nBase = BasedAddress;
					nSize = SizeOfSegment;
					
				}
			}
		}
	}

	if (nSize > 0)
	{
		status = SafeCopyProcessModules(pEprocess, nBase, nSize, pOutBuffer);
	}

	if (NT_SUCCESS(statusLookupProcess))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}

///////////////////////////////////////////////////////////////////////

//
// 用来判断进程的挂起和恢复计数
//
NTSTATUS ProcessMenu(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCESS pCp = (PCOMMUNICATE_PROCESS)pInBuffer;
	PEPROCESS pEprocess = NULL;
	ULONG hPid = 0;
	COMMUNICATE_THREAD Cpt;
	PALL_THREADS pPt = NULL;
	ULONG nThreadCnt = 1000;
	PRESUME_OR_SUSPEND pRos = (PRESUME_OR_SUSPEND)pOutBuffer;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	PEPROCESS pSpyHunterProess = (PEPROCESS)GetGlobalVeriable(enumAntiSpyEprocess);

	if (!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag)
	{
		return status;
	}
	
	KdPrint(("ProcessMenu\n"));

	// 参数检查
	if (!pInBuffer || 
		uInSize != sizeof(COMMUNICATE_PROCESS) ||
		!pOutBuffer ||
		uOutSize != sizeof(RESUME_OR_SUSPEND))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pEprocess = (PEPROCESS)pCp->op.GetProcessSuspendStatus.pEprocess;
	hPid = pCp->op.GetProcessSuspendStatus.nPid;
	
	// 如果是AS进程,就直接返回了
	if (pSpyHunterProess == pEprocess)
	{
		pRos->bResume = 0;
		pRos->bSuspend = 0;
		return status;
	}

	// 如果是死进程,就直接返回
	if (!IsRealProcess(pEprocess))
	{
		return status;
	}

	Cpt.nPid = hPid;
	Cpt.pEprocess = (ULONG)pEprocess;
	
	// 枚举全部线程
	do 
	{
		ULONG nSize = 0;

		if (pPt)
		{
			MzfExFreePoolWithTag(pPt, 0);
			pPt = NULL;
		}

		nSize = sizeof(ALL_THREADS) + nThreadCnt * sizeof(THREAD_INFO);
		pPt = (PALL_THREADS)MzfExAllocatePoolWithTag(NonPagedPool, nSize, MZFTAG);
		if (pPt)
		{
			memset(pPt, 0, nSize);
			status = ListProcessThread(&Cpt, sizeof(COMMUNICATE_THREAD), pPt, nSize, NULL);
		}

		nThreadCnt = pPt->nCnt + 100;

	} while (!NT_SUCCESS(status) && status == STATUS_BUFFER_TOO_SMALL);

	// 判断每个线程的挂起计数
	if (NT_SUCCESS(status))
	{
		ULONG i = 0;
		for (i = 0; i < pPt->nCnt; i++)
		{
			ULONG nRet = 0;
			PspGetThreadSuspendCount((PETHREAD)pPt->Threads[i].pThread, &nRet);

			if (nRet)
			{
				pRos->bResume = 1;
			}
			else
			{
				pRos->bSuspend = 1;
			}
		}
	}

	if (pPt)
	{
		MzfExFreePoolWithTag(pPt, 0);
		pPt = NULL;
	}
	
	return status;
}

///////////////////////////////////////////////////////////////////////

// 
// 调用先关NT函数实现挂起或者恢复进程
//
NTSTATUS WinXpAndLaterSuspendOrResumeProcess(PEPROCESS pEprocess, BOOL bSuspend)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtResumeProcess MzfNtResumeProcess = (pfnNtResumeProcess)GetGlobalVeriable(enumNtResumeProcess);
	pfnNtSuspendProcess MzfNtSuspendProcess = (pfnNtSuspendProcess)GetGlobalVeriable(enumNtSuspendProcess);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	HANDLE hProcess;
	
	if (!MzfNtClose					||
		!MzfObOpenObjectByPointer	||
		!MzfPsGetCurrentThread		||
		!ProcessType				||
		!MzfNtResumeProcess			||
		!MzfNtSuspendProcess) 
	{
		return status;
	}

	status = MzfObOpenObjectByPointer(pEprocess, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, GENERIC_ALL, ProcessType, KernelMode, &hProcess);
	if (NT_SUCCESS(status))
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);

		if (bSuspend)
		{
			status = MzfNtSuspendProcess(hProcess);
		}
		else
		{
			status = MzfNtResumeProcess(hProcess);
		}

		MzfNtClose(hProcess);
		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

///////////////////////////////////////////////////////////////////////

//
// 恢复或者挂起进程
//
NTSTATUS SuspendOrResumeProcess(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL, TempStatus = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_PROCESS pCp = (PCOMMUNICATE_PROCESS)pInBuffer;
	PEPROCESS pEprocess = NULL, pTempEprocess = NULL;
	ULONG nPid = 0;
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	
	KdPrint(("SuspendOrResumeProcess\n"));
	
	if (!MzfPsLookupProcessByProcessId		||
		!MzfObfDereferenceObject		
		)
	{
		return status;
	}
	
	// 参数检查
	if (!pInBuffer || uInSize != sizeof(COMMUNICATE_PROCESS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pEprocess = (PEPROCESS)pCp->op.SuspendOrResumeProcess.pEprocess;
	nPid = pCp->op.SuspendOrResumeProcess.nPid;

	// 如果有PID
	if (nPid)
	{
		TempStatus = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pTempEprocess);

		if (NT_SUCCESS(TempStatus) && !pEprocess)
		{
			pEprocess = pTempEprocess;
		}
	}
	
	if (!pEprocess) 
	{
		return status;
	}
	
	if (IsRealProcess(pEprocess))
	{
		if (Version == enumWINDOWS_2K)
		{
			
		}
		else
		{
			status = WinXpAndLaterSuspendOrResumeProcess(pEprocess, pCp->op.SuspendOrResumeProcess.bSuspend);
		}
	}
	
	if (NT_SUCCESS(TempStatus))
	{
		MzfObfDereferenceObject(pTempEprocess);
	}

	return status;
}