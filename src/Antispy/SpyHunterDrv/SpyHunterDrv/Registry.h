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
#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include "Struct.h"

NTSTATUS KernelOpenKey(OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes );
NTSTATUS KernelEnumerateValueKey(IN HANDLE KeyHandle, IN ULONG Index, IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, OUT PVOID KeyValueInformation, IN ULONG Length, OUT PULONG ResultLength );
NTSTATUS KernelEnumerateKey(IN HANDLE KeyHandle, IN ULONG Index, IN KEY_INFORMATION_CLASS KeyInformationClass, OUT PVOID KeyInformation, IN ULONG Length, OUT PULONG ResultLength );
NTSTATUS KernelDeleteKey( IN HANDLE KeyHandle );
NTSTATUS KernelCreateKey( OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG TitleIndex, IN PUNICODE_STRING Class OPTIONAL, IN ULONG CreateOptions, OUT PULONG Disposition OPTIONAL );
NTSTATUS KernelSetValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName, IN ULONG TitleIndex OPTIONAL, IN ULONG Type, IN PVOID Data, IN ULONG DataSize );
NTSTATUS KernelDeleteValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName );
NTSTATUS KernelRenameKey(IN HANDLE KeyHandle, IN PUNICODE_STRING NewName );

#endif
