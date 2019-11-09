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
#include "Registry.h"
#include "CommonFunction.h"
#include "InitWindows.h"

NTSTATUS KernelOpenKey(OUT PHANDLE  KeyHandle,
					   IN ACCESS_MASK  DesiredAccess,
					   IN POBJECT_ATTRIBUTES  ObjectAttributes
					   )

{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	pfnNtOpenKey MzfNtOpenKey = (pfnNtOpenKey)GetGlobalVeriable(enumNtOpenKey);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	
	if (!KeyHandle ||
		!ObjectAttributes ||
		!MzfNtOpenKey ||
		!MzfPsGetCurrentThread)
	{
		return status;
	}
	
	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtOpenKey(KeyHandle, DesiredAccess, ObjectAttributes);

	RecoverPreMode(pThread, PreMode);

	return status;	
}

NTSTATUS KernelQueryKey(IN HANDLE  KeyHandle,
						IN KEY_INFORMATION_CLASS  KeyInformationClass,
						OUT PVOID  KeyInformation,
						IN ULONG  Length,
						OUT PULONG  ResultLength
						)

{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
// 	PETHREAD pThread = NULL;
// 	CHAR PreMode = 0;
// 	pfnNtOpenKey MzfNtOpenKey = (pfnNtOpenKey)GetGlobalVeriable(enumNtQueryKey);
// 	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
// 	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
// 
// 	if (!KeyHandle ||
// 		!ObjectAttributes ||
// 		!MzfNtOpenKey ||
// 		!MzfPsGetCurrentThread ||
// 		!MzfNtClose)
// 	{
// 		return status;
// 	}
// 
// 	pThread = MzfPsGetCurrentThread();
// 	PreMode = ChangePreMode(pThread);
// 
// 	status = MzfNtOpenKey(KeyHandle, DesiredAccess, ObjectAttributes);
// 
// 	RecoverPreMode(pThread, PreMode);

	return status;
}

NTSTATUS KernelEnumerateValueKey(IN HANDLE  KeyHandle,
								 IN ULONG  Index,
								 IN KEY_VALUE_INFORMATION_CLASS  KeyValueInformationClass,
								 OUT PVOID  KeyValueInformation,
								 IN ULONG  Length,
								 OUT PULONG  ResultLength
								 )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtEnumerateValueKey MzfNtEnumerateValueKey = (pfnNtEnumerateValueKey)GetGlobalVeriable(enumNtEnumerateValueKey);
	
	if (!KeyHandle ||
		!MzfPsGetCurrentThread ||
		!MzfNtEnumerateValueKey)
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtEnumerateValueKey(KeyHandle, Index, KeyValueInformationClass, KeyValueInformation, Length, ResultLength);

	if (status == STATUS_BUFFER_TOO_SMALL)
	{
		KdPrint(("STATUS_BUFFER_TOO_SMALL: %d\n", RtlNtStatusToDosError(status)));
	}
	else if (STATUS_NO_MORE_ENTRIES == status)
	{
		KdPrint(("STATUS_NO_MORE_ENTRIES: %d\n", RtlNtStatusToDosError(status)));
	}

	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS KernelEnumerateKey(IN HANDLE  KeyHandle,
							IN ULONG  Index,
							IN KEY_INFORMATION_CLASS  KeyInformationClass,
							OUT PVOID  KeyInformation,
							IN ULONG  Length,
							OUT PULONG  ResultLength
							)
{	
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtEnumerateKey MzfNtEnumerateKey = (pfnNtEnumerateKey)GetGlobalVeriable(enumNtEnumerateKey);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle ||
		!MzfPsGetCurrentThread ||
		!MzfNtEnumerateKey)
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtEnumerateKey(KeyHandle, Index, KeyInformationClass, KeyInformation, Length, ResultLength);

	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS 
KernelDeleteKey(
			IN HANDLE  KeyHandle
			)
{
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtDeleteKey MzfNtDeleteKey = (pfnNtDeleteKey)GetGlobalVeriable(enumNtDeleteKey);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle ||
		!MzfPsGetCurrentThread ||
		!MzfNtDeleteKey)
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtDeleteKey(KeyHandle);

	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS 
KernelCreateKey(
				OUT PHANDLE  KeyHandle,
				IN ACCESS_MASK  DesiredAccess,
				IN POBJECT_ATTRIBUTES  ObjectAttributes,
				IN ULONG  TitleIndex,
				IN PUNICODE_STRING  Class  OPTIONAL,
				IN ULONG  CreateOptions,
				OUT PULONG  Disposition  OPTIONAL
				)
{
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtCreateKey MzfNtCreateKey = (pfnNtCreateKey)GetGlobalVeriable(enumNtCreateKey);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle ||
		!MzfPsGetCurrentThread ||
		!MzfNtCreateKey)
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtCreateKey(KeyHandle, DesiredAccess, ObjectAttributes, TitleIndex, Class, CreateOptions, Disposition);

	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS 
KernelSetValueKey(IN HANDLE  KeyHandle,
				  IN PUNICODE_STRING  ValueName,
				  IN ULONG  TitleIndex  OPTIONAL,
				  IN ULONG  Type,
				  IN PVOID  Data,
				  IN ULONG  DataSize
				  )
{
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtSetValueKey MzfNtSetValueKey = (pfnNtSetValueKey)GetGlobalVeriable(enumNtSetValueKey);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle ||
		!MzfPsGetCurrentThread ||
		!MzfNtSetValueKey)
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtSetValueKey(KeyHandle, ValueName, TitleIndex, Type, Data, DataSize);

	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS 
KernelDeleteValueKey(IN HANDLE  KeyHandle,
				  IN PUNICODE_STRING  ValueName
				  )
{
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtDeleteValueKey MzfNtDeleteValueKey = (pfnNtDeleteValueKey)GetGlobalVeriable(enumNtDeleteValueKey);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle ||
		!MzfPsGetCurrentThread ||
		!MzfNtDeleteValueKey)
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtDeleteValueKey(KeyHandle, ValueName);

	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS KernelRenameKey(IN HANDLE KeyHandle, IN PUNICODE_STRING NewName )
{
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtRenameKey MzfNtRenameKey = (pfnNtRenameKey)GetGlobalVeriable(enumNtRenameKey);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle ||
		!MzfPsGetCurrentThread ||
		!MzfNtRenameKey)
	{
		return status;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtRenameKey(KeyHandle, NewName);

	RecoverPreMode(pThread, PreMode);
	return status;
}