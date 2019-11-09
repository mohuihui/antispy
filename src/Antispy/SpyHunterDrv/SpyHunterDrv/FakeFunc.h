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
#ifndef _FAKE_FUNC_H_
#define _FAKE_FUNC_H_

#include "Struct.h"

NTSTATUS Forbids(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet);
NTSTATUS SetBackupDriverPath(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet);
NTSTATUS SetBackupDriverEvent(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet);

NTSTATUS Fake_NtCreateProcess( OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ParentProcess, IN BOOLEAN InheritObjectTable, IN HANDLE SectionHandle OPTIONAL, IN HANDLE DebugPort OPTIONAL, IN HANDLE ExceptionPort OPTIONAL );
NTSTATUS Fake_NtCreateProcessEx(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ParentProcess, IN ULONG Flags, IN HANDLE SectionHandle OPTIONAL, IN HANDLE DebugPort OPTIONAL, IN HANDLE ExceptionPort OPTIONAL, IN BOOLEAN InJob);
NTSTATUS Fake_NtCreateThread( __out PHANDLE ThreadHandle, __in ACCESS_MASK DesiredAccess, __in_opt POBJECT_ATTRIBUTES ObjectAttributes, __in HANDLE ProcessHandle, __out PCLIENT_ID ClientId, __in PCONTEXT ThreadContext, __in PVOID InitialTeb, __in BOOLEAN CreateSuspended );
NTSTATUS Fake_NtCreateFile ( __out PHANDLE FileHandle, __in ACCESS_MASK DesiredAccess, __in POBJECT_ATTRIBUTES ObjectAttributes, __out PIO_STATUS_BLOCK IoStatusBlock, __in_opt PLARGE_INTEGER AllocationSize, __in ULONG FileAttributes, __in ULONG ShareAccess, __in ULONG CreateDisposition, __in ULONG CreateOptions, __in_bcount_opt(EaLength) PVOID EaBuffer, __in ULONG EaLength );
BOOL Fake_NtUserSwitchDesktop_Win8(HDESK hDesktop, ULONG nUnKnow, ULONG nUnKnow1);
BOOL Fake_NtUserSwitchDesktop_Win7(HDESK hDesktop, ULONG nUnKnow);
BOOL Fake_NtUserSwitchDesktop_xp(HDESK hDesktop);
NTSTATUS Fake_NtCreateKey( OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG TitleIndex, IN PUNICODE_STRING Class OPTIONAL, IN ULONG CreateOptions, OUT PULONG Disposition OPTIONAL );
NTSTATUS Fake_NtRestoreKey( __in HANDLE KeyHandle, __in HANDLE FileHandle, __in ULONG Flags );
NTSTATUS Fake_NtReplaceKey ( __in POBJECT_ATTRIBUTES NewFile, __in HANDLE TargetHandle, __in POBJECT_ATTRIBUTES OldFile );
NTSTATUS Fake_NtLoadKey2( __in POBJECT_ATTRIBUTES TargetKey, __in POBJECT_ATTRIBUTES SourceFile, __in ULONG Flags );
NTSTATUS Fake_NtSetValueKey( IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName, IN ULONG TitleIndex OPTIONAL, IN ULONG Type, IN PVOID Data, IN ULONG DataSize );
NTSTATUS Fake_NtCreateSection( OUT PHANDLE SectionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN PLARGE_INTEGER MaximumSize OPTIONAL, IN ULONG SectionPageProtection, IN ULONG AllocationAttributes, IN HANDLE FileHandle OPTIONAL );
NTSTATUS Fake_NtSetSystemPowerState( IN POWER_ACTION SystemAction, IN SYSTEM_POWER_STATE MinSystemState, IN ULONG Flags);
NTSTATUS Fake_NtInitiatePowerAction( IN POWER_ACTION SystemAction, IN SYSTEM_POWER_STATE MinSystemState, IN ULONG Flags, IN BOOLEAN Asynchronous);
DWORD_PTR Fake_NtUserCallOneParam( DWORD_PTR Param, DWORD Routine);
NTSTATUS Fake_NtShutdownSystem( IN SHUTDOWN_ACTION Action );
BOOL Fake_NtUserLockWorkStation (VOID);
NTSTATUS Fake_NtSetSystemTime( IN PLARGE_INTEGER SystemTime, IN PLARGE_INTEGER NewSystemTime OPTIONAL );
NTSTATUS Fake_NtLoadDriver( IN PUNICODE_STRING DriverServiceName );
DWORD_PTR Fake_NtUserCallNoParam ( DWORD Routine);
NTSTATUS Fake_NtCreateThreadEx ( OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ProcessHandle, IN PVOID StartRoutine, IN PVOID StartContext, IN ULONG CreateThreadFlags, IN ULONG ZeroBits OPTIONAL, IN ULONG StackSize OPTIONAL, IN ULONG MaximumStackSize OPTIONAL, IN PVOID AttributeList );
NTSTATUS Fake_NtCreateUserProcess ( PHANDLE ProcessHandle, PHANDLE ThreadHandle, PVOID Parameter2, PVOID Parameter3, PVOID ProcessSecurityDescriptor, PVOID ThreadSecurityDescriptor, PVOID Parameter6, PVOID Parameter7, PVOID ProcessParameters, PVOID Parameter9, PVOID pProcessUnKnow);

#endif
