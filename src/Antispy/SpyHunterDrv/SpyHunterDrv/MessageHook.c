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
#include "MessageHook.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "process.h"

typedef enum _HANDLE_TYPE
{
	TYPE_FREE = 0,
	TYPE_WINDOW = 1,
	TYPE_MENU = 2,
	TYPE_CURSOR = 3,
	TYPE_SETWINDOWPOS = 4,
	TYPE_HOOK = 5,
	TYPE_CLIPDATA = 6,
	TYPE_CALLPROC = 7,
	TYPE_ACCELTABLE = 8,
	TYPE_DDEACCESS = 9,
	TYPE_DDECONV = 10,
	TYPE_DDEXACT = 11,
	TYPE_MONITOR = 12,
	TYPE_KBDLAYOUT = 13,
	TYPE_KBDFILE = 14,
	TYPE_WINEVENTHOOK = 15,
	TYPE_TIMER = 16,
	TYPE_INPUTCONTEXT = 17,
	TYPE_CTYPES = 18,
	TYPE_GENERIC = 255
}HANDLE_TYPE;

typedef struct _HANDLEENTRY{
	PVOID  phead;
	ULONG  pOwner;      
	BYTE  bType;        
	BYTE  bFlags;       
	WORD  wUniq;       
}HANDLEENTRY, *PHE;

typedef struct _SERVERINFO{   
	WORD wRIPFlags ;       
	WORD wSRVIFlags ;      
	WORD wRIPPID ;         
	WORD wRIPError ;       
	ULONG cHandleEntries;   
}SERVERINFO,*PSERVERINFO;

typedef struct _SHAREDINFO{
	PSERVERINFO psi;      
	PHE aheList;          
	ULONG pDispInfo;      
	ULONG ulSharedDelta;  
}SHAREDINFO,*PSHAREDINFO; 

typedef struct _HEAD
{
	HANDLE h;
	ULONG cLockObj;
}HEAD;

typedef struct _THROBJHEAD
{
	HEAD headinfo;
	PVOID pti; 
}THROBJHEAD;

typedef  struct _DESKHEAD
{
	PVOID rpdesk; 
	PBYTE pSelf ; 
}DESKHEAD;

typedef struct _THRDESKHEAD
{
	THROBJHEAD ThreadObjHead ;
	DESKHEAD DesktopHead ;
}THRDESKHEAD;

typedef struct _HOOK
{
	THRDESKHEAD tshead;
	struct _HOOK* phkNext;
	HOOK_TYPE iHook;    
	ULONG offPfn;    
	UINT flags;        
	DWORD  ihmod;    
	PVOID ptiHooked;    
	PVOID rpdesk;      
}HOOK, *PHOOK;

typedef struct
{
	UNICODE_STRING SectionFileName;
	WCHAR    NameBuffer[ANYSIZE_ARRAY];
} MEMORY_SECTION_NAME,*PMEMORY_SECTION_NAME;

ULONG FindgSharedInfo(ULONG UserRegisterWowHandlers)
{
	ULONG gSharedInfo = 0;
	ULONG uAddr = 0;

	if (UserRegisterWowHandlers && UserRegisterWowHandlers < SYSTEM_ADDRESS_START)
	{
		for (uAddr = UserRegisterWowHandlers; uAddr <= UserRegisterWowHandlers + 0x250; uAddr++)
		{
			if (0x40c7 == *(WORD*)uAddr && 0xb8 == *(BYTE*)(uAddr + 7))
			{
				gSharedInfo = *(DWORD*)(uAddr + 8);
				KdPrint(("gSharedInfo: 0x%08X\n", gSharedInfo));
				break;
			}
		}
	}
	
	return gSharedInfo;
}

BOOL GetSectionName(ULONG Pid, ULONG offPfn, DWORD ihmod, WCHAR *szModuleName)
{
	BOOL bRet = FALSE;
	pfnNtQueryVirtualMemory MzfNtQueryVirtualMemory = (pfnNtQueryVirtualMemory)GetGlobalVeriable(enumNtQueryVirtualMemory);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObOpenObjectByPointer MzfObOpenObjectByPointer = (pfnObOpenObjectByPointer)GetGlobalVeriable(enumObOpenObjectByPointer);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	ULONG SectionBaseAddressOffset = GetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PEPROCESS pEprocess = NULL;
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);

	if (!SectionBaseAddressOffset ||
		!szModuleName ||
		!ProcessType ||
		!MzfObfDereferenceObject ||
		!MzfNtQueryVirtualMemory ||
		!MzfPsLookupProcessByProcessId ||
		!MzfObOpenObjectByPointer ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!MzfPsGetCurrentThread)
	{
		return bRet;
	}
	
	KdPrint(("GetSectionName->  Pid: %d, offPfn: %x, ihmod: %d\n", Pid, offPfn, ihmod));

	status = MzfPsLookupProcessByProcessId((HANDLE)Pid, &pEprocess);
	if (NT_SUCCESS(status))
	{
		HANDLE hProcess; 
		status = MzfObOpenObjectByPointer(pEprocess, OBJ_KERNEL_HANDLE, 0, GENERIC_ALL, ProcessType, KernelMode, &hProcess);
		if (NT_SUCCESS(status))
		{
			PMEMORY_SECTION_NAME pSectionName = (PMEMORY_SECTION_NAME)MzfExAllocatePoolWithTag(PagedPool, PAGE_SIZE, MZFTAG);

			if (pSectionName)
			{
				ULONG pAddress = offPfn;
				PETHREAD pThread = NULL;
				CHAR PreMode = 0;
				ULONG nRet = 0;

				memset(pSectionName, 0, PAGE_SIZE);

				if (!ihmod)
				{
					pAddress += *(PULONG)((ULONG)pEprocess + SectionBaseAddressOffset);
				}

				KdPrint(("pAddress: 0x%08X\n", pAddress));

				pThread = MzfPsGetCurrentThread();
				PreMode = ChangePreMode(pThread);

				status = MzfNtQueryVirtualMemory(hProcess, (PVOID)pAddress, MemorySectionName, pSectionName, PAGE_SIZE, &nRet);

				if (NT_SUCCESS(status))
				{
					WCHAR szPath[MAX_PATH] = {0};
					ULONG nLen = CmpAndGetStringLength(&pSectionName->SectionFileName, MAX_PATH);
					WCHAR *szModule = NULL;

					wcsncpy(szPath, pSectionName->SectionFileName.Buffer, nLen);
					szModule = wcsrchr(szPath, L'\\');

					if (szModule)
					{
						szModule++;
						nLen = wcslen(szModule) >= MAX_MODULE_NAME_LEN ? MAX_MODULE_NAME_LEN - 1 : wcslen(szModule);
						wcsncpy(szModuleName, szModule, nLen);
					}
					else
					{
						nLen = wcslen(szPath) >= MAX_MODULE_NAME_LEN ? MAX_MODULE_NAME_LEN - 1 : wcslen(szPath);
						wcsncpy(szModuleName, szPath, nLen);
					}

					KdPrint(("szModuleName: %S\n", szModuleName));
// 					ULONG nLen = CmpAndGetStringLength(&pSectionName->SectionFileName, MAX_MODULE_NAME_LEN);
// 					wcsncpy(szModuleName, pSectionName->SectionFileName.Buffer, nLen);
// 					
// 					KdPrint(("Len: %d, MaxLen: %d, Name: %wZ, NameBuffer: %S\n", 
// 						pSectionName->SectionFileName.Length,
// 						pSectionName->SectionFileName.MaximumLength,
// 						&pSectionName->SectionFileName,
// 						pSectionName->NameBuffer));
					
					bRet = TRUE;
				}

				RecoverPreMode(pThread, PreMode);
				MzfExFreePoolWithTag(pSectionName, 0);
			}
		}

		MzfObfDereferenceObject(pEprocess);
	}

	return bRet;
}

NTSTATUS EnumMessageHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_MESSAGE_HOOK pCmh = (PCOMMUNICATE_MESSAGE_HOOK)pInBuffer;
	PMESSAGE_HOOK pMessageHook = (PMESSAGE_HOOK)pOutBuffer;
	ULONG nTotalCnt = pMessageHook->nCnt;
	ULONG UserRegisterWowHandlers = pCmh->op.Get.UserRegisterWowHandlers;
	PSHAREDINFO gSharedInfo = (PSHAREDINFO)FindgSharedInfo(UserRegisterWowHandlers);
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid ||
		!MzfProbeForRead ||
		!nTotalCnt)
	{
		return STATUS_UNSUCCESSFUL;
	}

	KdPrint(("UserRegisterWowHandlers: 0x%08X, gSharedInfo: 0x%08X\n", UserRegisterWowHandlers, gSharedInfo));

	__try
	{
		if (gSharedInfo)
		{
			PSERVERINFO pServerInfo = NULL;
			
			MzfProbeForRead(gSharedInfo, sizeof(SHAREDINFO), 1);
			pServerInfo = gSharedInfo->psi;

			if (pServerInfo)
			{
				ULONG i = 0;

				MzfProbeForRead(pServerInfo, sizeof(SERVERINFO), 1);

				for (i = 0; (ULONG)&gSharedInfo->aheList[i] < MmUserProbeAddress; i++)
				{
					MzfProbeForRead(&gSharedInfo->aheList[i], sizeof(HANDLEENTRY), 1);

					if (TYPE_HOOK == gSharedInfo->aheList[i].bType)
					{
						PHOOK pHook = (PHOOK)gSharedInfo->aheList[i].phead;

						if (MzfMmIsAddressValid(pHook) &&
							MzfMmIsAddressValid((PVOID)((ULONG)pHook + 0x30)))
						{
							PVOID pti = pHook->tshead.ThreadObjHead.pti;
							PETHREAD pThread = *(PETHREAD *)pti;
							
							if (pMessageHook->nRetCnt < nTotalCnt)
							{
								if ((ULONG)pThread > SYSTEM_ADDRESS_START &&
									MzfMmIsAddressValid(pThread))
								{
									ULONG Tid = *(PULONG)(GetGlobalVeriable(enumCidOffset_ETHREAD) + (ULONG)pThread + sizeof(ULONG));
									ULONG Pid = *(PULONG)(GetGlobalVeriable(enumCidOffset_ETHREAD) + (ULONG)pThread);

									KdPrint(("pThread: 0x%08X, TID: %d, pid: %d\n", pThread, Tid, Pid));

									pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].flags = pHook->flags;
									pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].ihmod = pHook->ihmod;
									pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].iHook = pHook->iHook;
									pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].offPfn = pHook->offPfn;
									pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].Handle = pHook->tshead.ThreadObjHead.headinfo.h;
									pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].Pid = Pid;
									pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].Tid = Tid;

									GetSectionName(
										Pid, 
										pHook->offPfn, 
										pHook->ihmod, 
										pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].ModuleName
										);

									GetProcessPathByPid(Pid, pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].szPath);
									KdPrint(("szPath: %S\n", pMessageHook->MessageHookInfo[pMessageHook->nRetCnt].szPath));
								}

								pMessageHook->nRetCnt++;
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

	KdPrint(("nRetCnt: %d, nCnt: %d\n", pMessageHook->nRetCnt, pMessageHook->nCnt));
	return STATUS_SUCCESS;
}