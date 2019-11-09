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

#include "Function.h"
#include "..\\..\\Common\Common.h"
#include "ConnectDriver.h"
#include "Function.h"
#include <list>
using namespace std;

typedef LONG NTSTATUS;

#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)   
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)   
#define STATUS_NOT_IMPLEMENTED          ((NTSTATUS)0xC0000002L)   
#define STATUS_INVALID_INFO_CLASS       ((NTSTATUS)0xC0000003L)   
#define STATUS_INFO_LENGTH_MISMATCH     ((NTSTATUS)0xC0000004L)   

typedef enum _KEY_INFORMATION_CLASS {
	KeyBasicInformation,
	KeyNodeInformation,
	KeyFullInformation,
	KeyNameInformation,
	KeyCachedInformation,
	KeyFlagsInformation,
	KeyVirtualizationInformation,
	KeyHandleTagsInformation,
	MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
} KEY_INFORMATION_CLASS;

typedef struct _KEY_INFO {
	ULONG   nNameLen;
	CString szName;
} KEY_INFO, *PKEY_INFO;

typedef struct _VALUE_INFO {
	ULONG   Type;
	ULONG   nDataLen;
	ULONG   nNameLen;
	CString szName;
	PVOID pData;
} VALUE_INFO, *PVALUE_INFO;

class CRegistry
{
public:
	CRegistry();
	virtual ~CRegistry();
	BOOL OpenKey(PHANDLE KeyHandle, ACCESS_MASK DesiredAccess, PMZF_OBJECT_ATTRIBUTES ObjectAttributes);
	HANDLE OpenKey(CString szKey);
	NTSTATUS QueryKey(HANDLE KeyHandle, KEY_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation, ULONG Length, PULONG ResultLength);
	BOOL EnumerateValueKey(IN HANDLE KeyHandle, IN ULONG Index, IN ULONG KeyValueInformationClass, OUT PVOID KeyValueInformation, IN ULONG Length, OUT PULONG ResultLength );
	BOOL EnumerateKey(IN HANDLE KeyHandle, IN ULONG Index, IN ULONG KeyInformationClass, OUT PVOID KeyValueInformation, IN ULONG Length, OUT PULONG ResultLength );
	BOOL DeleteKey(IN HANDLE KeyHandle);
	BOOL CreateKey( OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN PMZF_OBJECT_ATTRIBUTES ObjectAttributes, IN ULONG TitleIndex, IN PMZF_UNICODE_STRING Class OPTIONAL, IN ULONG CreateOptions, OUT PULONG Disposition OPTIONAL );
	HANDLE CreateKey( CString szKey, PULONG Disposition = NULL);
	BOOL SetValueKey( IN HANDLE KeyHandle, IN PMZF_UNICODE_STRING ValueName, IN ULONG TitleIndex OPTIONAL, IN ULONG Type, IN PVOID Data, IN ULONG DataSize );
	BOOL DeleteValueKey( IN HANDLE KeyHandle, IN PMZF_UNICODE_STRING ValueName );
	BOOL RenameKey( IN HANDLE KeyHandle, IN PMZF_UNICODE_STRING NewName );
	BOOL CopyRegTree(CString szDes, CString szSrc);
	BOOL CopyRegValues(HANDLE hDes, HANDLE hSrc);
	void DeleteKeyAndSubKeys(CString szKey);
	void EnumKeys(CString szKey, list<KEY_INFO> &keyList);
	BOOL EnableOrDisableKey(CString szKeyRoot, BOOL bEnable);
	void EnumValues(CString szKey, list<VALUE_INFO> &valueList);
	void FreeValueList(list<VALUE_INFO> &valueList);
	void DeleteValue(CString szKeyPath, CString szValue);
	BOOL EnableOrDisableValue(CString szKey, CString szValue, BOOL bEnable);
	BOOL ModifyValue(IN CString szKey, IN CString szValueName, IN ULONG Type, IN PVOID Data, IN ULONG DataSize);
	BOOL ModifyNULLNameValue(IN CString szKey, IN ULONG Type, IN PVOID Data, IN ULONG DataSize);
private:
	CConnectDriver m_Driver;
	CommonFunctions m_Function;
};

#endif