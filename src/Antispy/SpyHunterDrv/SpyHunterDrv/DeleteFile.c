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
#include "DeleteFile.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "Peload.h"
#include "libdasm.h"

NTSTATUS CloseAllHandleAboutTheFile(HANDLE hFile)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG ulRet= 0x10000;
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	pfnPsGetCurrentProcessId MzfPsGetCurrentProcessId = (pfnPsGetCurrentProcessId)GetGlobalVeriable(enumPsGetCurrentProcessId);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnNtQueryObject MzfNtQueryObject = (pfnNtQueryObject)GetGlobalVeriable(enumNtQueryObject);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	POBJECT_TYPE pFileObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoFileObjectType);
	PFILE_OBJECT pFile = NULL;
	PEPROCESS pSystemProcess = (PEPROCESS)GetGlobalVeriable(enumSystemProcess);
	
	if (!MzfObfDereferenceObject ||
		!MzfNtClose ||
		!MzfNtQueryObject ||
		!MzfKeUnstackDetachProcess ||
		!MzfKeStackAttachProcess ||
		!pSystemProcess ||
		!MzfPsLookupProcessByProcessId ||
		!pFileObjectType ||
		!MzfObReferenceObjectByHandle ||
		!MzfPsGetCurrentProcessId ||
		!MzfExAllocatePoolWithTag || 
		!MzfNtQuerySystemInformation ||
		!MzfPsGetCurrentThread ||
		!MzfExFreePoolWithTag)
	{
		return status;
	}
	
	if (!NT_SUCCESS(MzfObReferenceObjectByHandle(hFile, 0x80000000, pFileObjectType, KernelMode, &pFile, NULL)))
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	do 
	{
		PVOID pBuffer = MzfExAllocatePoolWithTag(PagedPool, ulRet, MZFTAG);
		if (pBuffer)
		{
			memset(pBuffer, 0, ulRet);
			status = MzfNtQuerySystemInformation(SystemHandleInformation, pBuffer, ulRet, &ulRet);
			if (NT_SUCCESS(status))
			{
				PSYSTEM_HANDLE_INFORMATION pHandles = (PSYSTEM_HANDLE_INFORMATION)pBuffer;
				ULONG i = 0;
				HANDLE hCurrentProcess = MzfPsGetCurrentProcessId();

				for (i = 0; i < pHandles->NumberOfHandles; i++)
				{
					if (hCurrentProcess != (HANDLE)pHandles->Handles[i].UniqueProcessId ||
						(HANDLE)pHandles->Handles[i].HandleValue != hFile)
					{
						if (KeGetObjectType(pHandles->Handles[i].Object) == pFileObjectType &&
							((PFILE_OBJECT)pHandles->Handles[i].Object)->FsContext == pFile->FsContext)
						{
							PSECTION_OBJECT_POINTERS SectionObjectPointer1 = ((PFILE_OBJECT)pHandles->Handles[i].Object)->SectionObjectPointer;
							PSECTION_OBJECT_POINTERS SectionObjectPointer2 = pFile->SectionObjectPointer;

							if (!SectionObjectPointer1 || 
								!MzfMmIsAddressValid(SectionObjectPointer1) ||
								SectionObjectPointer1->SharedCacheMap == SectionObjectPointer2->SharedCacheMap)
							{
								PEPROCESS pEprocess = NULL;
								if (NT_SUCCESS(MzfPsLookupProcessByProcessId((HANDLE)pHandles->Handles[i].UniqueProcessId, &pEprocess)))
								{
									WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
									ULONG hHandle = (ULONG)pHandles->Handles[i].HandleValue;
									KAPC_STATE as;
									OBJECT_DATA_INFORMATION odi;
									DWORD dwRet = 0;

									if (pSystemProcess == pEprocess && (Version == enumWINDOWS_7 || Version == enumWINDOWS_8))
									{
										hHandle |= 0x80000000;
									}
									
									MzfKeStackAttachProcess(pEprocess, &as);

									if (NT_SUCCESS(MzfNtQueryObject((HANDLE)hHandle, ObjectDataInfo, &odi, sizeof(OBJECT_DATA_INFORMATION), &dwRet)))
									{
										if (odi.ProtectFromClose)
										{
											pfnNtSetInformationObject MzfNtSetInformationObject = (pfnNtSetInformationObject)GetGlobalVeriable(enumNtSetInformationObject);
											if (MzfNtSetInformationObject)
											{
												odi.ProtectFromClose = FALSE;
												if (NT_SUCCESS(MzfNtSetInformationObject((HANDLE)hHandle, HandleFlagInformation, &odi, sizeof(OBJECT_DATA_INFORMATION))))
												{
													KdPrint(("force kill handle in MzfNtSetInformationObject\n"));
													MzfNtClose((HANDLE)hHandle);
												}
											}
										}
										else
										{
											MzfNtClose((HANDLE)hHandle);
										}
									}

									MzfKeUnstackDetachProcess(&as);
									MzfObfDereferenceObject(pEprocess);
								}
							}
						}
					}
				}
			}

			MzfExFreePoolWithTag(pBuffer, 0);
			ulRet *= 2;
		}
	} while (status == STATUS_INFO_LENGTH_MISMATCH);
	
	RecoverPreMode(pThread, PreMode);
	MzfObfDereferenceObject(pFile);
	return status;
}

NTSTATUS
SkillSetFileCompletion(
					   IN PDEVICE_OBJECT DeviceObject,
					   IN PIRP Irp,
					   IN PVOID Context
					   )
{
	BOOL bInit = FALSE;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnKeSetEvent MzfKeSetEvent = (pfnKeSetEvent)GetGlobalVeriable(enumKeSetEvent);
	pfnIoFreeIrp MzfIoFreeIrp = (pfnIoFreeIrp)GetGlobalVeriable(enumIoFreeIrp);

	if (MzfKeSetEvent && MzfIoFreeIrp)
	{
		Irp->UserIosb->Status = Irp->IoStatus.Status;
		Irp->UserIosb->Information = Irp->IoStatus.Information;

		MzfKeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);
		MzfIoFreeIrp(Irp);
		status = STATUS_MORE_PROCESSING_REQUIRED;
	}

	return status;
}

// 把文件系统的Dispatch设置成Reload的版本，防止被病毒hook
ULONG RecoverFileSystemDispatchHook(PDEVICE_OBJECT pDeviceObject, ULONG nIndex)
{
	ULONG OriginDispatch = 0;
	KdPrint(("RecoverFileSystemDispatchHook"));
	
	if (pDeviceObject && nIndex <= 27)
	{
		pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
		PULONG pReload = NULL;

		KdPrint(("pDeviceObject->DriverObject: 0x%08X\n", pDeviceObject->DriverObject));

		if ((PDRIVER_OBJECT)GetGlobalVeriable(enumNtfsDriverObject) == pDeviceObject->DriverObject &&
			GetGlobalVeriable(enumNtfsReloadDispatchs) > SYSTEM_ADDRESS_START)
		{
			KdPrint(("Is NtfsDriverObject"));
			pReload = (PULONG)GetGlobalVeriable(enumNtfsReloadDispatchs);
		}
		else if ((PDRIVER_OBJECT)GetGlobalVeriable(enumFastfatDriverObject) == pDeviceObject->DriverObject)
		{
			KdPrint(("Is FastfatDriverObject"));
			pReload = (PULONG)GetGlobalVeriable(enumFastfatReloadDispatchs);
		}

		if (MzfMmIsAddressValid && (ULONG)pReload > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)pReload[nIndex]))
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
			OriginDispatch = InterlockedExchange((PLONG)&(pDeviceObject->DriverObject->MajorFunction[nIndex]), pReload[nIndex]);
	//		OriginDispatch = (ULONG)pDeviceObject->DriverObject->MajorFunction[nIndex];
			KdPrint(("OriginDispatch: 0x%08X\n", OriginDispatch));
	//		pDeviceObject->DriverObject->MajorFunction[nIndex] = (PDRIVER_DISPATCH)pReload[nIndex];
			WPON();
			KeLowerIrql(OldIrql);
		}
	}

	KdPrint(("OK, rETNRN\n"));
	return OriginDispatch;
}

// 使文件系统Dispatch恢复成挂钩状态
void RestoreFileSystemDispatchHook(PDEVICE_OBJECT pDeviceObject, ULONG nIndex, ULONG OriginDispatch)
{
	KdPrint(("RestoreFileSystemDispatchHook"));

	if (pDeviceObject && nIndex <= 27 && OriginDispatch > SYSTEM_ADDRESS_START)
	{
		pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
		PULONG pReload = NULL;

		if ((PDRIVER_OBJECT)GetGlobalVeriable(enumNtfsDriverObject) == pDeviceObject->DriverObject &&
			GetGlobalVeriable(enumNtfsReloadDispatchs) > SYSTEM_ADDRESS_START)
		{
			pReload = (PULONG)GetGlobalVeriable(enumNtfsReloadDispatchs);
		}
		else if ((PDRIVER_OBJECT)GetGlobalVeriable(enumFastfatDriverObject) == pDeviceObject->DriverObject)
		{
			pReload = (PULONG)GetGlobalVeriable(enumFastfatReloadDispatchs);
		}

		if (MzfMmIsAddressValid && 
			pReload[nIndex] > SYSTEM_ADDRESS_START && 
			MzfMmIsAddressValid((PVOID)pReload[nIndex]) &&
			MzfMmIsAddressValid((PVOID)OriginDispatch))
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
		//	pDeviceObject->DriverObject->MajorFunction[nIndex] = (PDRIVER_DISPATCH)OriginDispatch;
			InterlockedExchange((PLONG)&(pDeviceObject->DriverObject->MajorFunction[nIndex]), OriginDispatch);
			WPON();
			KeLowerIrql(OldIrql);
		}
	}
}

// 获取文件系统中导入的MmFlushImageSection的地址
void GetFileSystemImpFunctions(FILE_SYSTEM_TYPE type)
{
	CHAR szModuleName[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
	CHAR szMmFlushImageSection[] = {'M','m','F','l','u','s','h','I','m','a','g','e','S','e','c','t','i','o','n','\0'};
	CHAR szIofCompleteRequest[] = {'I','o','f','C','o','m','p','l','e','t','e','R','e','q','u','e','s','t','\0'};
	WCHAR szDriverDir[] = {
		'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','d','r','i','v','e','r','s','\\','\0'		
	};
	
	if (type == enumNtfs)
	{
		ULONG NtfsBase = GetGlobalVeriable(enumNtfsBase);
		ULONG NtfsSize = GetGlobalVeriable(enumNtfsSize);
		
		if (NtfsBase && NtfsSize)
		{
			WCHAR szNtfsPath[MAX_PATH] = {0};
			WCHAR szNtfs[] = {'n','t','f','s','.','s','y','s','\0'};
			ULONG NewBase = 0, NewEnd = 0;
			ULONG NtfsImpMmFlushImageSection = 0;

			wcscpy(szNtfsPath, szDriverDir);
			wcscat(szNtfsPath, szNtfs);
			if (MzfMapFile(szNtfsPath, &NewBase, &NewEnd))
			{
				ULONG Offset = 0;
				if (FindIatFunction((PVOID)NewBase, NtfsSize, szModuleName, szMmFlushImageSection, &Offset) && Offset)
				{
					NtfsImpMmFlushImageSection = NtfsBase + Offset;
					SetGlobalVeriable(enumNtfsImpMmFlushImageSection, NtfsImpMmFlushImageSection);
					KdPrint(("NtfsImpMmFlushImageSection: 0x%08X\n", NtfsImpMmFlushImageSection));
				}
				
				if (FindIatFunction((PVOID)NewBase, NtfsSize, szModuleName, szIofCompleteRequest, &Offset) && Offset)
				{
					ULONG NtfsImpIofCompleteRequest = NtfsBase + Offset;
					SetGlobalVeriable(enumNtfsImpIofCompleteRequest, NtfsImpIofCompleteRequest);
					KdPrint(("NtfsImpIofCompleteRequest: 0x%08X\n", NtfsImpIofCompleteRequest));
				}

				ExFreePoolWithTag((PVOID)NewBase, 0);
			}
		}
	}
	else if (type == enumFastfat)
	{
		ULONG FastfatBase = GetGlobalVeriable(enumFastfatBase);
		ULONG FastfatSize = GetGlobalVeriable(enumFastfatSize);

		if (FastfatBase && FastfatSize)
		{
			WCHAR szFastfatPath[MAX_PATH] = {0};
			WCHAR szFastfat[] = {'f','a','s','t','f','a','t','.','s','y','s','\0'};
			ULONG NewBase = 0, NewEnd = 0;
			ULONG FastfatImpMmFlushImageSection = 0;

			wcscpy(szFastfatPath, szDriverDir);
			wcscat(szFastfatPath, szFastfat);
			if (MzfMapFile(szFastfatPath, &NewBase, &NewEnd))
			{
				ULONG Offset = 0;
				if (FindIatFunction((PVOID)NewBase, FastfatSize, szModuleName, szMmFlushImageSection, &Offset) && Offset)
				{
					FastfatImpMmFlushImageSection = FastfatBase + Offset;
					SetGlobalVeriable(enumFastfatImpMmFlushImageSection, FastfatImpMmFlushImageSection);
					KdPrint(("FastfatImpMmFlushImageSection: 0x%08X\n", FastfatImpMmFlushImageSection));
				}
				
				if (FindIatFunction((PVOID)NewBase, FastfatSize, szModuleName, szIofCompleteRequest, &Offset) && Offset)
				{
					ULONG FastfatImpIofCompleteRequest = FastfatBase + Offset;
					SetGlobalVeriable(enumFastfatImpIofCompleteRequest, FastfatImpIofCompleteRequest);
					KdPrint(("FastfatImpIofCompleteRequest: 0x%08X\n", FastfatImpIofCompleteRequest));
				}

				ExFreePoolWithTag((PVOID)NewBase, 0);
			}
		}
	}
}

BOOLEAN 
FakeMmFlushImageSection(
					IN PSECTION_OBJECT_POINTERS  SectionPointer,
					IN MMFLUSH_TYPE  FlushType
					)
{
	pfnMmFlushImageSection MzfMmFlushImageSection = (pfnMmFlushImageSection)GetGlobalVeriable(enumMmFlushImageSection);
	KdPrint(("SectionPointer: 0x%08X, FlushType: %d\n", SectionPointer, FlushType));

	if (MzfMmFlushImageSection)
	{
		KdPrint(("MzfMmFlushImageSection: 0x%08X\n", MzfMmFlushImageSection));
		return MzfMmFlushImageSection(SectionPointer, FlushType);
	}

	return FALSE;
}

void HookFileSystemMmFlushImageSection()
{
	ULONG FastfatImpMmFlushImageSection = GetGlobalVeriable(enumFastfatImpMmFlushImageSection);
	ULONG NtfsImpMmFlushImageSection = GetGlobalVeriable(enumNtfsImpMmFlushImageSection);

	if (!FastfatImpMmFlushImageSection)
	{
		GetFileSystemImpFunctions(enumFastfat);
		FastfatImpMmFlushImageSection = GetGlobalVeriable(enumFastfatImpMmFlushImageSection);
	}

	if (!NtfsImpMmFlushImageSection)
	{
		GetFileSystemImpFunctions(enumNtfs);
		NtfsImpMmFlushImageSection = GetGlobalVeriable(enumNtfsImpMmFlushImageSection);
	}

	if (FastfatImpMmFlushImageSection)
	{
		LONG OriginFastfatImpMmFlushImageSection = ExchangeAddress((PULONG)FastfatImpMmFlushImageSection, (ULONG)FakeMmFlushImageSection);
		SetGlobalVeriable(enumOriginFastfatImpMmFlushImageSection, (ULONG)OriginFastfatImpMmFlushImageSection);
		KdPrint(("hook FastfatImpMmFlushImageSection\n"));
	}

	if (NtfsImpMmFlushImageSection)
	{
		LONG OriginNtfsImpMmFlushImageSection = ExchangeAddress((PULONG)NtfsImpMmFlushImageSection, (ULONG)FakeMmFlushImageSection);
		SetGlobalVeriable(enumOriginNtfsImpMmFlushImageSection, (ULONG)OriginNtfsImpMmFlushImageSection);
		KdPrint(("hook NtfsImpMmFlushImageSection\n"));
	}
}

void UnHookFileSystemMmFlushImageSection()
{
	ULONG FastfatImpMmFlushImageSection = GetGlobalVeriable(enumFastfatImpMmFlushImageSection);
	ULONG NtfsImpMmFlushImageSection = GetGlobalVeriable(enumNtfsImpMmFlushImageSection);
	ULONG OriginFastfatImpMmFlushImageSection = GetGlobalVeriable(enumOriginFastfatImpMmFlushImageSection);
	ULONG OriginNtfsImpMmFlushImageSection = GetGlobalVeriable(enumOriginNtfsImpMmFlushImageSection);

	if (FastfatImpMmFlushImageSection && OriginFastfatImpMmFlushImageSection)
	{
		ExchangeAddress((PULONG)FastfatImpMmFlushImageSection, OriginFastfatImpMmFlushImageSection);
	}

	if (NtfsImpMmFlushImageSection && OriginNtfsImpMmFlushImageSection)
	{
		ExchangeAddress((PULONG)NtfsImpMmFlushImageSection, OriginNtfsImpMmFlushImageSection);
	}
}

// 把文件系统的导入的IofCompleteRequest函数设置成Reload的版本，防止被病毒hook
void RecoverFileSystemImpIofCompleteRequest(PDEVICE_OBJECT pDeviceObject)
{
	ULONG OriginDispatch = 0;
	ULONG ReloadIofCompleteRequest = GetGlobalVeriable(enumIofCompleteRequest);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	KdPrint(("RecoverFileSystemImpIofCompleteRequest"));

	if (MzfMmIsAddressValid && 
		ReloadIofCompleteRequest && 
		MzfMmIsAddressValid((PVOID)ReloadIofCompleteRequest) && 
		pDeviceObject)
	{
		if ((PDRIVER_OBJECT)GetGlobalVeriable(enumNtfsDriverObject) == pDeviceObject->DriverObject)
		{
			ULONG NtfsImpIofCompleteRequest = GetGlobalVeriable(enumNtfsImpIofCompleteRequest);
			if (!NtfsImpIofCompleteRequest)
			{
				GetFileSystemImpFunctions(enumNtfs);
				NtfsImpIofCompleteRequest = GetGlobalVeriable(enumNtfsImpIofCompleteRequest);
			}

			if (NtfsImpIofCompleteRequest)
			{
				LONG Origin = ExchangeAddress((PULONG)NtfsImpIofCompleteRequest, ReloadIofCompleteRequest);
				SetGlobalVeriable(enumOriginNtfsImpIofCompleteRequest, (ULONG)Origin);
				KdPrint(("hook NtfsImpMmFlushImageSection\n"));
			}
		}
		else if ((PDRIVER_OBJECT)GetGlobalVeriable(enumFastfatDriverObject) == pDeviceObject->DriverObject)
		{
			ULONG FastfatImpIofCompleteRequest = GetGlobalVeriable(enumFastfatImpIofCompleteRequest);
			if (!FastfatImpIofCompleteRequest)
			{
				GetFileSystemImpFunctions(enumFastfat);
				FastfatImpIofCompleteRequest = GetGlobalVeriable(enumFastfatImpIofCompleteRequest);
			}

			if (FastfatImpIofCompleteRequest)
			{
				LONG Origin = ExchangeAddress((PULONG)FastfatImpIofCompleteRequest, ReloadIofCompleteRequest);
				SetGlobalVeriable(enumOriginFastfatImpIofCompleteRequest, (ULONG)Origin);
				KdPrint(("hook FastfatImpIofCompleteRequest\n"));
			}
		}
	}

	KdPrint(("OK, rETNRN\n"));
}

void RestoreFileSystemImpIofCompleteRequest(PDEVICE_OBJECT pDeviceObject)
{
	ULONG OriginDispatch = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("RecoverFileSystemImpIofCompleteRequest"));

	if (MzfMmIsAddressValid && pDeviceObject)
	{
		if ((PDRIVER_OBJECT)GetGlobalVeriable(enumNtfsDriverObject) == pDeviceObject->DriverObject)
		{
			ULONG NtfsImpIofCompleteRequest = GetGlobalVeriable(enumNtfsImpIofCompleteRequest);
			ULONG Origin = GetGlobalVeriable(enumOriginNtfsImpIofCompleteRequest);

			if (NtfsImpIofCompleteRequest && Origin)
			{
				ExchangeAddress((PULONG)NtfsImpIofCompleteRequest, Origin);
				KdPrint(("Restore NtfsImpMmFlushImageSection\n"));
			}
		}
		else if ((PDRIVER_OBJECT)GetGlobalVeriable(enumFastfatDriverObject) == pDeviceObject->DriverObject)
		{
			ULONG FastfatImpIofCompleteRequest = GetGlobalVeriable(enumFastfatImpIofCompleteRequest);
			ULONG Origin = GetGlobalVeriable(enumOriginFastfatImpIofCompleteRequest);

			if (FastfatImpIofCompleteRequest && Origin)
			{
				ExchangeAddress((PULONG)FastfatImpIofCompleteRequest, Origin);
				KdPrint(("Restore FastfatImpIofCompleteRequest\n"));
			}
		}
	}

	KdPrint(("OK, rETNRN\n"));
}

ULONG GetpIofCompleteRequest()
{
	ULONG ReloadIofCompleteRequest = GetGlobalVeriable(enumIofCompleteRequest);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG pIofCompleteRequest = 0;
	KdPrint(("GetpIofCompleteRequest: ReloadIofCompleteRequest: 0x%08X\n", ReloadIofCompleteRequest));

	if (ReloadIofCompleteRequest && MzfMmIsAddressValid)
	{
		ULONG Address = 0, nCodeLen = 0;
		
		for (Address = ReloadIofCompleteRequest; Address < ReloadIofCompleteRequest + 100; Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			if (!MzfMmIsAddressValid((PVOID)Address))
			{
				break;
			}

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.length == 0 || Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_JMP)
			{
				KdPrint(("jmp\n"));
				// ff25 8c1d9cb2  - jmp dword ptr [b29c1d8c] // 绝对跳转
				if(Inst.op1.type == OPERAND_TYPE_MEMORY &&
					Inst.length == 6 &&
					MzfMmIsAddressValid((PVOID)Inst.op1.displacement))
				{
					pIofCompleteRequest = Inst.op1.displacement;
					KdPrint(("pIofCompleteRequest: 0x%08X\n", pIofCompleteRequest));
					SetGlobalVeriable(enumpIofCompleteRequest, pIofCompleteRequest);
					break;
				}
			}

			nCodeLen = Inst.length;
		}
	}

	return pIofCompleteRequest;
}

PIMAGE_SECTION_HEADER GetIopfCompleteRequestSecion(PIMAGE_NT_HEADERS NtHeaders)
{
	ULONG i = 0;
	PIMAGE_SECTION_HEADER NtSection = NULL;
	CHAR szText[] = {'.','t','e','x','t','\0'};
	CHAR szPAGEWMI[] = {'P','A','G','E','W','M','I','\0'};
	CHAR szInit[] = {'I','N','I','T','\0'};
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);

	if (!NtHeaders)
	{
		return NULL;
	}
	
	NtSection = IMAGE_FIRST_SECTION( NtHeaders );
	for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) 
	{
		if ( NtSection->Characteristics & IMAGE_SCN_CNT_CODE ) 
		{
			if (WinVersion == enumWINDOWS_2K && !_strnicmp(NtSection->Name, szInit, strlen(szInit)))
			{
				return NtSection;
			}
			else if ((WinVersion == enumWINDOWS_XP || WinVersion == enumWINDOWS_2K3 || WinVersion == enumWINDOWS_2K3_SP1_SP2) &&
				!_strnicmp(NtSection->Name, szPAGEWMI, strlen(szPAGEWMI)))
			{
				return NtSection;
			}
			else if((WinVersion == enumWINDOWS_VISTA || WinVersion == enumWINDOWS_VISTA_SP1_SP2 || WinVersion == enumWINDOWS_7 || WinVersion == enumWINDOWS_8) &&
				!_strnicmp(NtSection->Name, szText, strlen(szText)))
			{
				return NtSection;
			}
		}

		++NtSection;
	}

	return NULL;
}

BOOL GetIopfCompleteRequest()
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
	PVOID NewImageBase = (PVOID)GetGlobalVeriable(enumNewKernelBase);
	PIMAGE_SECTION_HEADER InitSection = NULL;
	ULONG pInitSectionAddress = 0, nInitSectionSize = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);
	ULONG pIofCompleteRequest = GetGlobalVeriable(enumpIofCompleteRequest);
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	
	if (!MzfMmIsAddressValid ||
		!MzfRtlImageDirectoryEntryToData)
	{
		return FALSE;
	}

	if (!pIofCompleteRequest)
	{
		pIofCompleteRequest = GetpIofCompleteRequest();
	}

	KdPrint(("GetIopfCompleteRequest \n"));

	if (!NewImageBase || 
		!MzfMmIsAddressValid || 
		!pIofCompleteRequest)
	{
		return FALSE;
	}

//	KdPrint(("pIofCompleteRequest: 0x%08X\n", pIofCompleteRequest));
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

		InitSection = GetIopfCompleteRequestSecion(NtHeaders);
		if (!InitSection)
		{
			return FALSE;
		}

		pInitSectionAddress = InitSection->VirtualAddress + (ULONG)NewImageBase;
		nInitSectionSize = InitSection->Misc.VirtualSize;
		KdPrint(("IopfCompleteRequestSecion: 0x%08X, Size: 0x%X\n", pInitSectionAddress, nInitSectionSize));

		//
		// Locate the relocation section.
		//

		NextBlock = (PIMAGE_BASE_RELOCATION)MzfRtlImageDirectoryEntryToData(
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

				switch (WinVersion)
				{
				case enumWINDOWS_2K:
					{
						if (MzfMmIsAddressValid((PVOID)(FixupVA - 2)) &&
							*(BYTE *)(FixupVA - 2) == 0xC7 && 
							MzfMmIsAddressValid((PVOID)FixupVA) &&
							*(DWORD*)FixupVA == pIofCompleteRequest && 
							MzfMmIsAddressValid((PVOID)(*(DWORD*)(FixupVA + sizeof(DWORD)))))
						{
							KdPrint(("IopfCompleteRequest: 0x%08X\n", *(DWORD*)(FixupVA + sizeof(DWORD))));
							SetGlobalVeriable(enumIopfCompleteRequest, *(DWORD*)(FixupVA + sizeof(DWORD)));
							return TRUE;
						}
					}
					break;

				case enumWINDOWS_XP:
				case enumWINDOWS_2K3:
				case enumWINDOWS_2K3_SP1_SP2:
					{
						if (MzfMmIsAddressValid((PVOID)(FixupVA - 1)) &&
							*(BYTE *)(FixupVA - 1) == 0xB9 && 
							MzfMmIsAddressValid((PVOID)(FixupVA + 4)) &&
							*(BYTE *)(FixupVA + 4) == 0x87 && 
							MzfMmIsAddressValid((PVOID)FixupVA) &&
							*(DWORD*)FixupVA == pIofCompleteRequest && 
							MzfMmIsAddressValid((PVOID)(FixupVA - 5)) &&
							MzfMmIsAddressValid((PVOID)(*(DWORD*)(FixupVA - 5))))
						{
							KdPrint(("IopfCompleteRequest: 0x%08X\n", *(DWORD*)(FixupVA - 5)));
							SetGlobalVeriable(enumIopfCompleteRequest, *(DWORD*)(FixupVA - 5));
							return TRUE;
						}
					}
					break;

				case enumWINDOWS_7:
				case enumWINDOWS_VISTA:
				case enumWINDOWS_VISTA_SP1_SP2:
				case enumWINDOWS_8:
					{
						if (MzfMmIsAddressValid((PVOID)(FixupVA - 15)) &&
							*(USHORT *)(FixupVA - 15) == 0xC033 &&
							MzfMmIsAddressValid((PVOID)(FixupVA - 1)) &&
							*(BYTE *)(FixupVA - 1) == 0xB9 && 
							MzfMmIsAddressValid((PVOID)(FixupVA + 4)) &&
							*(BYTE *)(FixupVA + 4) == 0x87 && 
							MzfMmIsAddressValid((PVOID)FixupVA) &&
							*(DWORD*)FixupVA == pIofCompleteRequest && 
							MzfMmIsAddressValid((PVOID)(FixupVA - 5)) &&
							MzfMmIsAddressValid((PVOID)(*(DWORD*)(FixupVA - 5))))
						{
							KdPrint(("IopfCompleteRequest: 0x%08X\n", *(DWORD*)(FixupVA - 5)));
							SetGlobalVeriable(enumIopfCompleteRequest, *(DWORD*)(FixupVA - 5));
							return TRUE;
						}
					}
					break;
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

void RecoverIopfCompleteRequest()
{
	ULONG pIofCompleteRequest = GetGlobalVeriable(enumpIofCompleteRequest);
	ULONG IopfCompleteRequest = GetGlobalVeriable(enumIopfCompleteRequest);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!pIofCompleteRequest || !IopfCompleteRequest) 
	{
		GetIopfCompleteRequest();
		pIofCompleteRequest = GetGlobalVeriable(enumpIofCompleteRequest);
		IopfCompleteRequest = GetGlobalVeriable(enumIopfCompleteRequest);
	}

	KdPrint(("Recover-> pIofCompleteRequest: 0x%08X, IopfCompleteRequest: 0x%08X\n", pIofCompleteRequest, IopfCompleteRequest));
	if (MzfMmIsAddressValid && 
		pIofCompleteRequest && 
		IopfCompleteRequest && 
		MzfMmIsAddressValid((PVOID)pIofCompleteRequest) && 
		MzfMmIsAddressValid((PVOID)IopfCompleteRequest) && 
		*(DWORD*)pIofCompleteRequest != IopfCompleteRequest) 
	{
		LONG Origin = ExchangeAddress((PULONG)pIofCompleteRequest, IopfCompleteRequest);
		SetGlobalVeriable(enumOriginIopfCompleteRequest, Origin);
		KdPrint(("hOOK IopfCompleteRequest OK\n"));
	}
}

void RestoreIopfCompleteRequest()
{
	ULONG pIofCompleteRequest = GetGlobalVeriable(enumpIofCompleteRequest);
	ULONG Origin = GetGlobalVeriable(enumOriginIopfCompleteRequest);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid && 
		Origin && 
		pIofCompleteRequest && 
		MzfMmIsAddressValid((PVOID)pIofCompleteRequest) &&
		MzfMmIsAddressValid((PVOID)Origin) &&
		*(DWORD*)pIofCompleteRequest != Origin) 
	{
		ExchangeAddress((PULONG)pIofCompleteRequest, Origin);
		SetGlobalVeriable(enumOriginIopfCompleteRequest, 0);
	}
}

NTSTATUS MzfSetFileAttributes(IN HANDLE FileHandle, ULONG Attribute)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK IoStatusBlock;
	PIRP Irp;
	KEVENT kEvent;
	PIO_STACK_LOCATION IrpSp;
	PDEVICE_OBJECT pBaseDeviceObject = NULL;
	ULONG pOriginDispatch = 0;
	PFILE_OBJECT      FileObject;
	FILE_BASIC_INFORMATION    FileInformation;

	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnKeWaitForSingleObject MzfKeWaitForSingleObject = (pfnKeWaitForSingleObject)GetGlobalVeriable(enumKeWaitForSingleObject);

	if (!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject ||
		!MzfKeWaitForSingleObject)
	{
		return ntStatus;
	}

	ntStatus = MzfObReferenceObjectByHandle(FileHandle,
		DELETE,
		*IoFileObjectType,
		KernelMode,
		&FileObject,
		NULL);

	if (!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}

	if (FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL)
		return STATUS_UNSUCCESSFUL;

	pBaseDeviceObject = FileObject->Vpb->DeviceObject;
	Irp = IoAllocateIrp(pBaseDeviceObject->StackSize, FALSE);
	if(Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

	KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

	IoSetCompletionRoutine(
		Irp,
		SkillSetFileCompletion,
		&kEvent,
		TRUE,
		TRUE,
		TRUE);

	memset(&FileInformation, 0, 0x28);
	FileInformation.FileAttributes = Attribute;

	Irp->AssociatedIrp.SystemBuffer = &FileInformation;
	Irp->UserEvent = &kEvent;
	Irp->UserIosb = &IoStatusBlock;
	Irp->RequestorMode = KernelMode;
	Irp->Flags = IRP_CLOSE_OPERATION|IRP_SYNCHRONOUS_API;
	Irp->Tail.Overlay.Thread = PsGetCurrentThread();
	Irp->Tail.Overlay.OriginalFileObject = FileObject;

	IrpSp = IoGetNextIrpStackLocation(Irp);
	IrpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
	IrpSp->DeviceObject = pBaseDeviceObject;
	IrpSp->FileObject = FileObject;
	IrpSp->Parameters.SetFile.Length = sizeof(FILE_BASIC_INFORMATION);
	IrpSp->Parameters.SetFile.FileInformationClass = FileBasicInformation;
	IrpSp->Parameters.SetFile.FileObject = FileObject;

	pOriginDispatch = RecoverFileSystemDispatchHook(pBaseDeviceObject, IRP_MJ_SET_INFORMATION);
	RecoverFileSystemImpIofCompleteRequest(pBaseDeviceObject);
	RecoverIopfCompleteRequest();

	ntStatus = IoCallDriver(pBaseDeviceObject, Irp);

	RestoreFileSystemDispatchHook(pBaseDeviceObject, IRP_MJ_SET_INFORMATION, pOriginDispatch);
	
	if (ntStatus == STATUS_PENDING)
		KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, NULL);

	RestoreFileSystemImpIofCompleteRequest(pBaseDeviceObject);
	RestoreIopfCompleteRequest();

	ntStatus = IoStatusBlock.Status;
	if(!NT_SUCCESS(ntStatus))
	{
		IoFreeIrp(Irp);
		return ntStatus;
	}

	MzfObfDereferenceObject(FileObject);

	return ntStatus;
}

BOOLEAN
SKillDeleteFile(
				IN HANDLE    FileHandle
				)
{
	NTSTATUS          ntStatus = STATUS_SUCCESS;
	PFILE_OBJECT      fileObject;
	PDEVICE_OBJECT    DeviceObject;
	PIRP              Irp;
	KEVENT            event;
	FILE_DISPOSITION_INFORMATION    FileInformation;
	IO_STATUS_BLOCK ioStatus;
	PIO_STACK_LOCATION irpSp;
	PSECTION_OBJECT_POINTERS pSectionObjectPointer;
	PVOID ulImageSectionObject;
	PVOID ulDataSectionObject;
	PVOID ulSharedCacheMap;

	ntStatus = ObReferenceObjectByHandle(FileHandle,
		DELETE,
		*IoFileObjectType,
		KernelMode,
		&fileObject,
		NULL);

	if (!NT_SUCCESS(ntStatus))
	{
		return FALSE;
	}

	DeviceObject = IoGetRelatedDeviceObject(fileObject);
	Irp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
	if (Irp == NULL)
	{
		ObDereferenceObject(fileObject);
		return FALSE;
	}

	KeInitializeEvent(&event, SynchronizationEvent, FALSE);

	FileInformation.DeleteFile = TRUE;

	Irp->AssociatedIrp.SystemBuffer = &FileInformation;
	Irp->UserEvent = &event;
	Irp->UserIosb = &ioStatus;
	Irp->Tail.Overlay.OriginalFileObject = fileObject;
	Irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
	Irp->RequestorMode = KernelMode;

	irpSp = IoGetNextIrpStackLocation(Irp);
	irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
	irpSp->DeviceObject = DeviceObject;
	irpSp->FileObject = fileObject;
	irpSp->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
	irpSp->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
	irpSp->Parameters.SetFile.FileObject = fileObject;

	IoSetCompletionRoutine(
		Irp,
		SkillSetFileCompletion,
		&event,
		TRUE,
		TRUE,
		TRUE);

	// 再加上下面这三行代码 ，MmFlushImageSection函数通过这个结构来检查是否可以删除文件。
	// 这个和Hook
	ulImageSectionObject = NULL;
	ulDataSectionObject = NULL;
	ulSharedCacheMap = NULL;

	pSectionObjectPointer = NULL;
	pSectionObjectPointer = fileObject->SectionObjectPointer;
	if (pSectionObjectPointer && MmIsAddressValid(pSectionObjectPointer))
	{
		ulImageSectionObject = pSectionObjectPointer->ImageSectionObject; // 备份之~~~
		pSectionObjectPointer->ImageSectionObject = 0; //清零，准备删除

		ulDataSectionObject = pSectionObjectPointer->DataSectionObject;  //备份之
		pSectionObjectPointer->DataSectionObject = 0;       //清零，准备删除

		ulSharedCacheMap = pSectionObjectPointer->SharedCacheMap;
		pSectionObjectPointer->SharedCacheMap = 0;
	}

	//发irp删除
	IoCallDriver(DeviceObject, Irp);

	//等待操作完毕
	KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, NULL);

	//删除文件之后，从备份那里填充回来
	pSectionObjectPointer = NULL;
	pSectionObjectPointer = fileObject->SectionObjectPointer;

	if (MmIsAddressValid(pSectionObjectPointer))
	{
		if (ulImageSectionObject)
			pSectionObjectPointer->ImageSectionObject = ulImageSectionObject; //填充回来，不然蓝屏哦

		if (ulDataSectionObject)
			pSectionObjectPointer->DataSectionObject = ulDataSectionObject;

		if (ulSharedCacheMap)
			pSectionObjectPointer->SharedCacheMap = ulSharedCacheMap;
	}

	ObDereferenceObject(fileObject);
	fileObject = NULL;

	return TRUE;
}

NTSTATUS KernelDeleteFile(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
 	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WCHAR *szFilePath = ((PCOMMUNICATE_FILE)pInBuffer)->op.Delete.szPath;
	HANDLE hFile;
	
	KdPrint(("szFilePath: %S\n", szFilePath));

	// 关闭文件的所有句柄
	if (szFilePath &&
		NT_SUCCESS(KernelOpenFile( szFilePath, &hFile, FILE_READ_ATTRIBUTES, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT )))
	{
		CloseAllHandleAboutTheFile(hFile);
	//	ZwClose(hFile);
		hFile = 0;
	}
	
	status = KernelOpenFile( szFilePath, &hFile, DELETE | FILE_WRITE_ATTRIBUTES, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_DELETE, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT );
	if (NT_SUCCESS(status))
	{
		// 设置文件属性为Normal 
		if (NT_SUCCESS(MzfSetFileAttributes(hFile, FILE_ATTRIBUTE_NORMAL)))
		{
			HookFileSystemMmFlushImageSection();

			 if (SKillDeleteFile(hFile))
			 {
				 status = STATUS_SUCCESS;
			 }

			UnHookFileSystemMmFlushImageSection();
		}

		MzfCloseHandle(hFile);
		hFile = 0;
	}

	return status;
}