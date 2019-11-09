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
#include "FakeFunc.h"
#include "..\\..\\Common\\Common.h"
#include "InitWindows.h"
#include "HookEngine.h"
#include "CommonFunction.h"
#include "SelfProtectSSDT.h"
#include "IATHook.h"

NTSTATUS
Fake_NtCreateProcess(
					  OUT PHANDLE ProcessHandle,
					  IN ACCESS_MASK DesiredAccess,
					  IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
					  IN HANDLE ParentProcess,
					  IN BOOLEAN InheritObjectTable,
					  IN HANDLE SectionHandle OPTIONAL,
					  IN HANDLE DebugPort OPTIONAL,
					  IN HANDLE ExceptionPort OPTIONAL
					  )
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtCreateProcess NtCreateProcess = (pfnNtCreateProcess)GetGlobalVeriable(enumOriginNtCreateProcess);
		status = NtCreateProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ParentProcess, InheritObjectTable, SectionHandle, DebugPort, ExceptionPort);
	}

	return status;
}

NTSTATUS
Fake_NtCreateProcessEx(OUT PHANDLE ProcessHandle,
						IN ACCESS_MASK DesiredAccess,
						IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
						IN HANDLE ParentProcess,
						IN ULONG Flags,
						IN HANDLE SectionHandle OPTIONAL,
						IN HANDLE DebugPort OPTIONAL,
						IN HANDLE ExceptionPort OPTIONAL,
						IN BOOLEAN InJob)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	KdPrint(("NtCreateProcessEx\n"));

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtCreateProcessEx NtCreateProcessEx = (pfnNtCreateProcessEx)GetGlobalVeriable(enumOriginNtCreateProcessEx);
		status = NtCreateProcessEx(ProcessHandle, DesiredAccess, ObjectAttributes, ParentProcess, Flags, SectionHandle, DebugPort, ExceptionPort, InJob);
	}

	return status;
}

NTSTATUS 
Fake_NtCreateUserProcess (
						   PHANDLE ProcessHandle,
						   PHANDLE ThreadHandle,
						   PVOID Parameter2,
						   PVOID Parameter3,
						   PVOID ProcessSecurityDescriptor,
						   PVOID ThreadSecurityDescriptor,
						   PVOID Parameter6,
						   PVOID Parameter7,
						   PVOID ProcessParameters,
						   PVOID Parameter9,
						   PVOID pProcessUnKnow)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtCreateUserProcess NtCreateUserProcess = (pfnNtCreateUserProcess)GetGlobalVeriable(enumOriginNtCreateUserProcess);
		status = NtCreateUserProcess(ProcessHandle, 
			ThreadHandle, 
			Parameter2, 
			Parameter3, 
			ProcessSecurityDescriptor, 
			ThreadSecurityDescriptor, 
			Parameter6, 
			Parameter7, 
			ProcessParameters,
			Parameter9,
			pProcessUnKnow);
	}

	return status;
}

NTSTATUS
Fake_NtCreateFile (
					__out PHANDLE FileHandle,
					__in ACCESS_MASK DesiredAccess,
					__in POBJECT_ATTRIBUTES ObjectAttributes,
					__out PIO_STATUS_BLOCK IoStatusBlock,
					__in_opt PLARGE_INTEGER AllocationSize,
					__in ULONG FileAttributes,
					__in ULONG ShareAccess,
					__in ULONG CreateDisposition,
					__in ULONG CreateOptions,
					__in_bcount_opt(EaLength) PVOID EaBuffer,
					__in ULONG EaLength
					)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	pfnNtCreateFile NtCreateFile = (pfnNtCreateFile)GetGlobalVeriable(enumOriginNtCreateFile);

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		status = NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
	}
	else
	{
		if ( CreateDisposition && CreateDisposition != FILE_CREATE && CreateDisposition <= FILE_MAXIMUM_DISPOSITION)
		{
			if ( CreateDisposition == FILE_OPEN_IF )
			{
				CreateDisposition = FILE_OPEN;
			}
			else
			{
				if ( CreateDisposition == FILE_OVERWRITE_IF )
				{
					CreateDisposition = FILE_OVERWRITE;
				}
			}

			status = NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
		}
	}
	
	return status;
}

NTSTATUS
Fake_NtCreateThread(
					 __out PHANDLE ThreadHandle,
					 __in ACCESS_MASK DesiredAccess,
					 __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
					 __in HANDLE ProcessHandle,
					 __out PCLIENT_ID ClientId,
					 __in PCONTEXT ThreadContext,
					 __in PVOID InitialTeb,
					 __in BOOLEAN CreateSuspended
					 )
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtCreateThread NtCreateThread = (pfnNtCreateThread)GetGlobalVeriable(enumOriginNtCreateThread);
		status = NtCreateThread(ThreadHandle, DesiredAccess, ObjectAttributes, ProcessHandle, ClientId, ThreadContext, InitialTeb, CreateSuspended);
	}

	return status;
}

NTSTATUS 
Fake_NtCreateThreadEx (
						OUT PHANDLE ThreadHandle,
						IN ACCESS_MASK DesiredAccess,
						IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
						IN HANDLE ProcessHandle,
						IN PVOID StartRoutine,
						IN PVOID StartContext,
						IN ULONG CreateThreadFlags,
						IN ULONG ZeroBits OPTIONAL,
						IN ULONG StackSize OPTIONAL,
						IN ULONG MaximumStackSize OPTIONAL,
						IN PVOID AttributeList
						)
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtCreateThreadEx NtCreateThreadEx = (pfnNtCreateThreadEx)GetGlobalVeriable(enumOriginNtCreateThreadEx);
		status = NtCreateThreadEx(ThreadHandle, 
			DesiredAccess, 
			ObjectAttributes, 
			ProcessHandle, 
			StartRoutine, 
			StartContext, 
			CreateThreadFlags, 
			ZeroBits, 
			StackSize, 
			MaximumStackSize, 
			AttributeList);
	}

	return status;
}

BOOL
Fake_NtUserSwitchDesktop_xp(HDESK hDesktop)
{
	BOOL bRet = FALSE;
	return bRet;
}

BOOL
Fake_NtUserSwitchDesktop_Win7(HDESK hDesktop, ULONG nUnKnow)
{
	BOOL bRet = FALSE;
	return bRet;
}

BOOL
Fake_NtUserSwitchDesktop_Win8(HDESK hDesktop, ULONG nUnKnow, ULONG nUnKnow1)
{
	BOOL bRet = FALSE;
	return bRet;
}

#define ProbeForReadSmallStructure(Address, Size, Alignment) {               \
	ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||                       \
	((Alignment) == 4) || ((Alignment) == 8) ||                       \
	((Alignment) == 16));                                             \
	if ((Size == 0) || (Size > 0x10000)) {                                   \
	ASSERT(0);                                                           \
	ProbeForRead(Address, Size, Alignment);                              \
	} else {                                                                 \
	if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {               \
	ExRaiseDatatypeMisalignment();                                   \
	}                                                                    \
	if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {      \
	*(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;              \
	}                                                                    \
	}                                                                        \
}

#define ProbeAndReadUnicodeString(Source)  \
	(((Source) >= (UNICODE_STRING * const)MM_USER_PROBE_ADDRESS) ? \
	(*(volatile UNICODE_STRING * const)MM_USER_PROBE_ADDRESS) : (*(volatile UNICODE_STRING *)(Source)))

#define ProbeAndReadUnicodeStringEx(Dst, Src) *(Dst) = ProbeAndReadUnicodeString(Src)

BOOL IsKeyExits(ACCESS_MASK  DesiredAccess, POBJECT_ATTRIBUTES  ObjectAttributes)
{
	BOOL bRet = FALSE;
	pfnNtOpenKey MzfNtOpenKey = (pfnNtOpenKey)GetGlobalVeriable(enumNtOpenKey);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	
	if (!MzfNtOpenKey ||
		!MzfPsGetCurrentThread ||
		!MzfNtClose)
	{
		return bRet;
	}

	__try
	{
		PUNICODE_STRING SafeObjectName = NULL;
		HANDLE hKey = NULL;
		UNICODE_STRING      CapturedObjectName = {0};
		PETHREAD pThread = NULL;
		CHAR PreMode = 0;
		NTSTATUS status;

		ProbeForReadSmallStructure( ObjectAttributes,
			sizeof(OBJECT_ATTRIBUTES),
			PROBE_ALIGNMENT(OBJECT_ATTRIBUTES) );

		SafeObjectName = ObjectAttributes->ObjectName;

		ProbeAndReadUnicodeStringEx(&CapturedObjectName, SafeObjectName);

		ProbeForRead(
			CapturedObjectName.Buffer,
			CapturedObjectName.Length,
			sizeof(WCHAR)
			);
		
		pThread = MzfPsGetCurrentThread();
		PreMode = ChangePreMode(pThread);

		status = MzfNtOpenKey(&hKey, DesiredAccess, ObjectAttributes);
		if (NT_SUCCESS(status))
		{
			bRet = TRUE;
			MzfNtClose(hKey);
		}

		RecoverPreMode(pThread, PreMode);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return bRet;
	}

	return bRet;
}

NTSTATUS 
Fake_NtCreateKey(
				  OUT PHANDLE  KeyHandle,
				  IN ACCESS_MASK  DesiredAccess,
				  IN POBJECT_ATTRIBUTES  ObjectAttributes,
				  IN ULONG  TitleIndex,
				  IN PUNICODE_STRING  Class  OPTIONAL,
				  IN ULONG  CreateOptions,
				  OUT PULONG  Disposition  OPTIONAL
				  )
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy() || IsKeyExits(DesiredAccess, ObjectAttributes))
	{
		pfnNtCreateKey NtCreateKey = (pfnNtCreateKey)GetGlobalVeriable(enumOriginNtCreateKey);
		status = NtCreateKey(KeyHandle, DesiredAccess, ObjectAttributes, TitleIndex, Class, CreateOptions, Disposition);
	}

	return status;
}

NTSTATUS
Fake_NtLoadKey2(
				 __in POBJECT_ATTRIBUTES   TargetKey,
				 __in POBJECT_ATTRIBUTES   SourceFile,
				 __in ULONG                Flags
				 )
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtLoadKey2 NtLoadKey2 = (pfnNtLoadKey2)GetGlobalVeriable(enumOriginNtLoadKey2);
		status = NtLoadKey2(TargetKey, SourceFile, Flags);
	}

	return status;
}

NTSTATUS
Fake_NtReplaceKey (
					__in POBJECT_ATTRIBUTES NewFile,
					__in HANDLE             TargetHandle,
					__in POBJECT_ATTRIBUTES OldFile
					)
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtReplaceKey NtReplaceKey = (pfnNtReplaceKey)GetGlobalVeriable(enumOriginNtReplaceKey);
		status = NtReplaceKey(NewFile, TargetHandle, OldFile);
	}

	return status;
}

NTSTATUS
Fake_NtRestoreKey(
				   __in HANDLE KeyHandle,
				   __in HANDLE FileHandle,
				   __in ULONG Flags
				   )
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtRestoreKey NtRestoreKey = (pfnNtRestoreKey)GetGlobalVeriable(enumOriginNtRestoreKey);
		status = NtRestoreKey(KeyHandle, FileHandle, Flags);
	}

	return status;
}

BOOL IsDenySetValue(HANDLE  KeyHandle, PUNICODE_STRING  ValueName)
{
	BOOL bRet = FALSE;
	
	// 如果选项是禁止设置值，那么就直接返回
	if (InterlockedCompareExchange(GetGlobalVeriable_Address(enumCanSetValues), TRUE, TRUE))
	{
		return TRUE;
	}

	// 否则判断该值是否存在，存在就允许设置，不存在就拒绝添加新值
	__try
	{
		PUNICODE_STRING SafeObjectName = NULL;
		UNICODE_STRING      CapturedObjectName = {0};
		NTSTATUS status;
		KEY_VALUE_BASIC_INFORMATION info;
		ULONG nRetLen = 0;

		SafeObjectName = ValueName;

		ProbeAndReadUnicodeStringEx(&CapturedObjectName, SafeObjectName);

		ProbeForRead(
			CapturedObjectName.Buffer,
			CapturedObjectName.Length,
			sizeof(WCHAR)
			);

		status = ZwQueryValueKey(KeyHandle, &CapturedObjectName, KeyValueBasicInformation, &info, sizeof(KEY_VALUE_BASIC_INFORMATION), &nRetLen);
		if (status == STATUS_OBJECT_NAME_NOT_FOUND)
		{
			bRet = TRUE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return bRet;
	}

	return bRet;
}

NTSTATUS 
Fake_NtSetValueKey(
					IN HANDLE  KeyHandle,
					IN PUNICODE_STRING  ValueName,
					IN ULONG  TitleIndex  OPTIONAL,
					IN ULONG  Type,
					IN PVOID  Data,
					IN ULONG  DataSize
					)
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	pfnNtSetValueKey NtSetValueKey = (pfnNtSetValueKey)GetGlobalVeriable(enumOriginNtSetValueKey);

	if (mode == KernelMode || IsCurrentProcessAntiSpy() || !IsDenySetValue(KeyHandle, ValueName))
	{
		status = NtSetValueKey(KeyHandle, ValueName, TitleIndex, Type, Data, DataSize);
	}

	return status;
}

NTSTATUS 
Fake_NtCreateSection(
					  OUT PHANDLE  SectionHandle,
					  IN ACCESS_MASK  DesiredAccess,
					  IN POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
					  IN PLARGE_INTEGER  MaximumSize OPTIONAL,
					  IN ULONG  SectionPageProtection,
					  IN ULONG  AllocationAttributes,
					  IN HANDLE  FileHandle OPTIONAL
					  )
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	pfnNtCreateSection NtCreateSection = (pfnNtCreateSection)GetGlobalVeriable(enumOriginNtCreateSection);

	KdPrint(("PageProtection: 0x%X, Attributes: 0x%X\n", SectionPageProtection, AllocationAttributes));

	if (mode == KernelMode || 
		IsCurrentProcessAntiSpy() ||
		!(AllocationAttributes & 0x1000000) || 
		!(SectionPageProtection & 0xF0))
	{
		status = NtCreateSection(SectionHandle, DesiredAccess, ObjectAttributes, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle);
	}

	return status;
}

NTSTATUS
Fake_NtInitiatePowerAction(
							IN POWER_ACTION SystemAction,
							IN SYSTEM_POWER_STATE MinSystemState,
							IN ULONG Flags,
							IN BOOLEAN Asynchronous)
{
	NTSTATUS status = STATUS_PRIVILEGE_NOT_HELD;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	pfnNtInitiatePowerAction NtInitiatePowerAction = (pfnNtInitiatePowerAction)GetGlobalVeriable(enumOriginNtInitiatePowerAction);

	if (mode == KernelMode || 
		IsCurrentProcessAntiSpy())
	{
		status = NtInitiatePowerAction(SystemAction, MinSystemState, Flags, Asynchronous);
	}

	return status;
}

NTSTATUS
Fake_NtSetSystemPowerState(
							IN POWER_ACTION SystemAction,
							IN SYSTEM_POWER_STATE MinSystemState,
							IN ULONG Flags)
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	pfnNtSetSystemPowerState NtSetSystemPowerState = (pfnNtSetSystemPowerState)GetGlobalVeriable(enumOriginNtSetSystemPowerState);

	if (mode == KernelMode || 
		IsCurrentProcessAntiSpy())
	{
		status = NtSetSystemPowerState(SystemAction, MinSystemState, Flags);
	}

	return status;
}

DWORD_PTR
Fake_NtUserCallOneParam(
						 DWORD_PTR Param,
						 DWORD Routine)
{
	DWORD_PTR dwRet = 0;
	pfnNtUserCallOneParam NtUserCallOneParam = (pfnNtUserCallOneParam)GetGlobalVeriable(enumOriginNtUserCallOneParam);

	if (IsCurrentProcessAntiSpy())
	{
		dwRet = NtUserCallOneParam(Param, Routine);
	}
	else
	{
		ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);
		ULONG Type = 0;

		if (nBuildNumber == 2195) //2k
		{
			Type = 0x38;
		}
		else if (nBuildNumber >= 2600)
		{
			Type = 0x34;
		}

		if (Routine != Type)
		{
			dwRet = NtUserCallOneParam(Param, Routine); 
		}
	}

	return dwRet;
}

DWORD_PTR
Fake_NtUserCallNoParam (
						 DWORD Routine)
{
	DWORD_PTR dwRet = 0;
	pfnNtUserCallNoParam NtUserCallNoParam = (pfnNtUserCallNoParam)GetGlobalVeriable(enumOriginNtUserCallOneParam);

	if (IsCurrentProcessAntiSpy())
	{
		dwRet = NtUserCallNoParam(Routine);
	}
	else
	{
		ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);
		ULONG Type = 0;
	
		if ( nBuildNumber > 6001 )
		{
			Type = 17 - (nBuildNumber < 9200);
		}
		else
		{
			Type = 15;
		}

		if (Routine != Type)
		{
			dwRet = NtUserCallNoParam(Routine); 
		}
	}

	return dwRet;
}

NTSTATUS
Fake_NtShutdownSystem(
					   IN SHUTDOWN_ACTION Action
					   )
{
	NTSTATUS status = STATUS_PRIVILEGE_NOT_HELD;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtShutdownSystem NtShutdownSystem = (pfnNtShutdownSystem)GetGlobalVeriable(enumOriginNtShutdownSystem);
		status = NtShutdownSystem(Action);
	}

	return status;
}

NTSTATUS
Fake_NtSetSystemTime(
					  IN PLARGE_INTEGER SystemTime,
					  IN PLARGE_INTEGER NewSystemTime OPTIONAL
					  )
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtSetSystemTime NtSetSystemTime = (pfnNtSetSystemTime)GetGlobalVeriable(enumOriginNtSetSystemTime);
		status = NtSetSystemTime(SystemTime, NewSystemTime);
	}

	return status;
}

BOOL
Fake_NtUserLockWorkStation (VOID)
{
	BOOL bRet = FALSE;
	return bRet;
}

BOOL IsBackupDriver()
{
	BOOL bRet = FALSE;

	// 如果选项是禁止设置值，那么就直接返回
	if (InterlockedCompareExchange(GetGlobalVeriable_Address(enumIsBackupDriver), TRUE, TRUE))
	{
		bRet = TRUE;
	}
	
	return bRet;
}

NTSTATUS NtCopyFile(WCHAR *szPath)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WCHAR *szBackupPath = (WCHAR *)GetGlobalVeriable(enumBackupDriverPath);

	if (!szPath || !szBackupPath)
	{
		return status;
	}

	
	return status;
}

NTSTATUS 
Fake_NtLoadDriver(
				   IN PUNICODE_STRING  DriverServiceName
				   )
{
	NTSTATUS status = STATUS_PRIVILEGE_NOT_HELD;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		pfnNtLoadDriver NtLoadDriver = (pfnNtLoadDriver)GetGlobalVeriable(enumOriginNtLoadDriver);
		status = NtLoadDriver(DriverServiceName);
	}

	if (mode == UserMode/* && IsBackupDriver() && GetGlobalVeriable(enumBackupDriverPath)*/)
	{
	//	WCHAR *szBackupPath = (WCHAR *)GetGlobalVeriable(enumBackupDriverPath);
		
		__try
		{
			PUNICODE_STRING SafeObjectName = NULL;
			UNICODE_STRING      CapturedObjectName = {0};
			NTSTATUS ntstatus = STATUS_UNSUCCESSFUL;
			ULONG nRetLen = 0;
			HANDLE KeyHandle = NULL;
			OBJECT_ATTRIBUTES oa;

			SafeObjectName = DriverServiceName;
		
			ProbeAndReadUnicodeStringEx(&CapturedObjectName, SafeObjectName);

			ProbeForRead(
				CapturedObjectName.Buffer,
				CapturedObjectName.Length,
				sizeof(WCHAR)
				);
			
			KdPrint(("ObjectName: %wZ\n", &CapturedObjectName));

			InitializeObjectAttributes(&oa, &CapturedObjectName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
			ntstatus = ZwOpenKey(&KeyHandle, SYNCHRONIZE | GENERIC_READ, &oa);
			if (NT_SUCCESS(ntstatus))
			{
				UNICODE_STRING unImagePath;
				WCHAR szImagePath[] = {'I','m','a','g','e','P','a','t','h','\0'};
				ULONG ulSize = 0;
				
				MzfInitUnicodeString(&unImagePath, szImagePath);
				
				ntstatus = ZwQueryValueKey(KeyHandle, &unImagePath, KeyValuePartialInformation, NULL, 0, &ulSize);
				KdPrint(("ntstatus: 0x%08X", ntstatus));

				if (ntstatus != STATUS_OBJECT_NAME_NOT_FOUND  && ulSize > 0)
				{
					//分配实际查询所需的内存空间
					PKEY_VALUE_PARTIAL_INFORMATION pkvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
					if (pkvpi)
					{
						memset(pkvpi, 0, ulSize);

						//查询键值
						ntstatus = ZwQueryValueKey(KeyHandle, &unImagePath, KeyValuePartialInformation, pkvpi, ulSize, &ulSize);
						if(NT_SUCCESS(ntstatus) && (pkvpi->Type == REG_SZ || pkvpi->Type == REG_EXPAND_SZ))
						{
							KdPrint(("ImagePath: %S\n", (WCHAR*)pkvpi->Data));
						}

						ExFreePool(pkvpi);
					}
					
				}

				//关闭注册表句柄
				ZwClose(KeyHandle);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	return status;
}

NTSTATUS SetBackupDriverPath(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);

	PBACKUP_DRIVER_PATH pBackup = (PBACKUP_DRIVER_PATH)pInBuffer;
	WCHAR *szPath = pBackup->szPath;
	DWORD dwLen = pBackup->dwBytes;

	if (!MzfProbeForRead || uInSize != sizeof(BACKUP_DRIVER_PATH) || 
		!szPath || dwLen <= 0 ||
		!MzfExAllocatePoolWithTag)
	{
		return status;
	}
	
	__try
	{
		WCHAR *szBackupPath = (WCHAR *)GetGlobalVeriable(enumBackupDriverPath);
		if (szBackupPath)
		{
			MzfExFreePoolWithTag(szBackupPath, 0);
			szBackupPath = NULL;
			SetGlobalVeriable(enumBackupDriverPath, 0);
		}

		MzfProbeForRead(szPath, dwLen, sizeof(WCHAR));
		szBackupPath = (WCHAR*)MzfExAllocatePoolWithTag(NonPagedPool, dwLen + sizeof(WCHAR), MZFTAG);
		if (szBackupPath)
		{
			memset(szBackupPath, 0, dwLen + sizeof(WCHAR));
			memcpy(szBackupPath, szPath, dwLen);
			KdPrint(("szBackupPath: %S\n", szBackupPath));

			SetGlobalVeriable(enumBackupDriverPath, (ULONG)szBackupPath);

			status = STATUS_SUCCESS;
		}
	}
	__except(1)
	{
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

NTSTATUS SetBackupDriverEvent(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PBACKUP_DRIVER_EVENT pEvent = (PBACKUP_DRIVER_EVENT)pInBuffer;
	
	if (uInSize == sizeof(BACKUP_DRIVER_EVENT))
	{
		HANDLE hEvent = pEvent->hEvent;
		PKEVENT	pCommEvent = (PKEVENT)GetGlobalVeriable(enumBackupDriverEvent);

		if (pCommEvent)
		{
			ObDereferenceObject(pCommEvent);
			pCommEvent = NULL;
		}

		status = ObReferenceObjectByHandle(
			hEvent, 
			EVENT_MODIFY_STATE, 
			*ExEventObjectType, 
			KernelMode, 
			(PVOID*)&pCommEvent, 
			NULL);						

		if (NT_SUCCESS(status))
		{
			KdPrint(("init event success-> pCommEvent: 0x%08X\n", pCommEvent));
		}
	}

	return status;
}

NTSTATUS Forbids(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_FORBID pCf = (PCOMMUNICATE_FORBID)pInBuffer;
	FORBID_TYPE nFt = eForbid_Unknow;
	BOOL bDeny = FALSE;
	VERIABLE_INDEX nIndex = 0;
	BOOL bShadow = FALSE;

	if (uInSize != sizeof(COMMUNICATE_FORBID))
	{
		return status;
	}

	nFt = pCf->ForbidType;
	bDeny = pCf->bDeny;
	KdPrint(("nFt: %d, bDeny: %d\n", nFt, bDeny));

	switch (nFt)
	{
	// 禁止创建进程
	case eForbid_CreateProcess:
		{
			if (bDeny)
			{
				SetSSDTHook(enumNtCreateProcessExIndex);
				if (GetGlobalVeriable(enumNtCreateUserProcessIndex))
				{
					SetSSDTHook(enumNtCreateUserProcessIndex);
				}
			}
			else
			{	
				RestoreSSDTHook(enumNtCreateProcessExIndex);
				if (GetGlobalVeriable(enumNtCreateUserProcessIndex))
				{
					RestoreSSDTHook(enumNtCreateUserProcessIndex);
				}
			}
		}
		break;
	
	// 禁止创建线程
	case eForbid_CreateThread:
		{
			if (bDeny)
			{
				SetSSDTHook(enumNtCreateThreadIndex);
				if (GetGlobalVeriable(enumNtCreateThreadExIndex))
				{
					SetSSDTHook(enumNtCreateThreadExIndex);
				}
			}
			else
			{	
				RestoreSSDTHook(enumNtCreateThreadIndex);
				if (GetGlobalVeriable(enumNtCreateThreadExIndex))
				{
					RestoreSSDTHook(enumNtCreateThreadExIndex);
				}
			}
		}
		break;

	// 禁止创建文件
	case eForbid_CreateFile:
		nIndex = enumNtCreateFileIndex;
		break;

	// 禁止切换桌面
	case eForbid_SwitchDesktop:
		bShadow = TRUE;
		nIndex = enumNtUserSwitchDesktopIndex;
		break;

	// 禁止创建键值
	case eForbid_CreateKey:
		{
			if (bDeny)
			{
				SetSSDTHook(enumNtCreateKeyIndex);
				SetSSDTHook(enumNtLoadKey2Index);
				SetSSDTHook(enumNtReplaceKeyIndex);
				SetSSDTHook(enumNtRestoreKeyIndex);
				SetSSDTHook(enumNtSetValueKeyIndex);
			}
			else
			{	
				RestoreSSDTHook(enumNtCreateKeyIndex);
				RestoreSSDTHook(enumNtLoadKey2Index);
				RestoreSSDTHook(enumNtReplaceKeyIndex);
				RestoreSSDTHook(enumNtRestoreKeyIndex);
				RestoreSSDTHook(enumNtSetValueKeyIndex);
			}
		}
		break;

	// 禁止设置值
	case eForbid_SetKeyValue:
		{
			bDeny ? InterlockedExchange(GetGlobalVeriable_Address(enumCanSetValues), TRUE) : 
				InterlockedExchange(GetGlobalVeriable_Address(enumCanSetValues), FALSE);
		}
		break;

	// 禁止加载模块
	case eForbid_CreateImage:
		nIndex = enumNtCreateSectionIndex;
		break;

	// 禁止关机、重启、注销
	case eForbid_Shutdown:
		{
			if (bDeny)
			{
				SetSSDTHook(enumNtShutdownSystemIndex);
				SetSSDTHook(enumNtSetSystemPowerStateIndex);
				SetSSDTHook(enumNtInitiatePowerActionIndex);
				SetShadowHook(enumNtUserCallOneParamIndex);
			}
			else
			{	
				RestoreSSDTHook(enumNtShutdownSystemIndex);
				RestoreSSDTHook(enumNtSetSystemPowerStateIndex);
				RestoreSSDTHook(enumNtInitiatePowerActionIndex);
				RestoreShadowHook(enumNtUserCallOneParamIndex);
			}
		}
		break;

	// 禁止修改系统时间
	case eForbid_ModifySystemTime:
		nIndex = enumNtSetSystemTimeIndex;
		break;

	// 禁止锁定计算机
	case eForbid_LockWorkStation:
		nIndex = enumNtUserLockWorkStationIndex;
		bShadow = TRUE;
		break;

	// 备份驱动文件
	case eForbid_BackupDriver:
		// 设置标志
		bDeny ? InterlockedExchange(GetGlobalVeriable_Address(enumIsBackupDriver), TRUE) : 
			InterlockedExchange(GetGlobalVeriable_Address(enumIsBackupDriver), FALSE);

		// hook NtLoadDriver
		nIndex = enumNtLoadDriverIndex;
		break;

	// 禁止加载驱动
	case eForbid_LoadDriver:
		nIndex = enumNtLoadDriverIndex;
		break;

	// 禁止注入模块
	case eForbid_InjectModule:
		{
			if (bDeny)
			{
				SetIATHook();
			}
			else
			{
				RestoreIATHook();
			}
		}
		break;

	case eForbid_ScreenCapture:
		{
			if (bDeny)
			{
				SetShadowHook(enumNtGdiBitBltIndex);
				SetShadowHook(enumNtGdiStretchBltIndex);
				SetShadowHook(enumNtGdiMaskBltIndex);
				SetShadowHook(enumNtGdiPlgBltIndex);
				SetShadowHook(enumNtGdiAlphaBlendIndex);
				SetShadowHook(enumNtGdiTransparentBltIndex);
				SetShadowHook(enumNtUserGetClipboardDataIndex);
			}
			else
			{
				RestoreShadowHook(enumNtGdiBitBltIndex);
				RestoreShadowHook(enumNtGdiStretchBltIndex);
				RestoreShadowHook(enumNtGdiMaskBltIndex);
				RestoreShadowHook(enumNtGdiPlgBltIndex);
				RestoreShadowHook(enumNtGdiAlphaBlendIndex);
				RestoreShadowHook(enumNtGdiTransparentBltIndex);
				RestoreShadowHook(enumNtUserGetClipboardDataIndex);
			}
		}
		break;

	default:
		status = STATUS_UNSUCCESSFUL;
		break;
	}

	if (nIndex != 0)
	{
		if (bShadow)
		{
			// 如果是启用禁止
			if (bDeny)
			{
				SetShadowHook(nIndex);
			}
			else
			{	
				RestoreShadowHook(nIndex);
			}
		}
		else
		{
			// 如果是启用禁止
			if (bDeny)
			{
				SetSSDTHook(nIndex);
			}
			else
			{	
				RestoreSSDTHook(nIndex);
			}
		}
		
		status = STATUS_SUCCESS;
	}
	
	return status;
}