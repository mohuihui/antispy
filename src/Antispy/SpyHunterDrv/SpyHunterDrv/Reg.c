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
#include "Reg.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Registry.h"

//#define  MY_REG_SOFTWARE_KEY_NAME  L"\\Registry\\Machine\\SYSTEM\\ControlSet001\\Services"

NTSTATUS PspEnumerateKey(WCHAR *szKey, WCHAR* szOutKeys, ULONG nOutLen)
{
	UNICODE_STRING unKey;
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE hKey;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG i = 0;
	ULONG nTotalLen = 0;
// 	ULONG nOutLenTemp = nOutLen / sizeof(WCHAR);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnNtOpenKey MzfNtOpenKey = (pfnNtOpenKey)GetGlobalVeriable(enumNtOpenKey);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtEnumerateKey MzfNtEnumerateKey = (pfnNtEnumerateKey)GetGlobalVeriable(enumNtEnumerateKey);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	WCHAR szService[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','Y','S','T','E','M','\\','C','o','n','t','r','o','l','S','e','t','0','0','1','\\','S','e','r','v','i','c','e','s','\0'};

	if (!szKey ||
		!szOutKeys ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!MzfNtOpenKey ||
		!MzfPsGetCurrentThread ||
		!MzfNtEnumerateKey ||
		!MzfNtClose)
	{
		return status;
	}

	MzfInitUnicodeString(&unKey, /*MY_REG_SOFTWARE_KEY_NAME*/szService);
	InitializeObjectAttributes(&ObjectAttributes, &unKey, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	
	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtOpenKey(&hKey, GENERIC_ALL, &ObjectAttributes);
	if (!NT_SUCCESS(status))
	{
		RecoverPreMode(pThread, PreMode);
		KdPrint(("ZwOpenKey error\n"));
		return status;
	}

	while(1)
	{
		ULONG ulSize = 0;

		status = MzfNtEnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &ulSize);

		if (STATUS_BUFFER_TOO_SMALL == status)
		{	
			PKEY_BASIC_INFORMATION pkbi = NULL;
			ULONG nSize = ulSize + 0x100;

			pkbi = (PKEY_BASIC_INFORMATION)MzfExAllocatePoolWithTag(PagedPool, nSize, MZFTAG);
			if (!pkbi)
			{
				status = STATUS_UNSUCCESSFUL;
				break;
			}
			
			memset(pkbi, 0, nSize);

			status = MzfNtEnumerateKey(hKey, i, KeyBasicInformation, pkbi, nSize, &ulSize);
			if (NT_SUCCESS(status))
			{
				if (nOutLen - nTotalLen <= pkbi->NameLength)
				{
					MzfExFreePoolWithTag(pkbi, 0);
					status = STATUS_BUFFER_OVERFLOW;
					break;
				}
			
				wcsncpy(szOutKeys + nTotalLen, pkbi->Name, pkbi->NameLength / sizeof(WCHAR));
				nTotalLen += pkbi->NameLength / sizeof(WCHAR);
				szOutKeys[nTotalLen] = L'\0';
				nTotalLen++;
			}

			MzfExFreePoolWithTag(pkbi, 0);
		}
		else if (STATUS_NO_MORE_ENTRIES == status)
		{
			status = STATUS_SUCCESS;
			break;
		}

		i++;
	}
	
	MzfNtClose(hKey);
	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS EnumerateKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_REG pCr = 	(PCOMMUNICATE_REG)pInBuffer;
	WCHAR *szKey = pCr->szKey;
	WCHAR *szOutKeys = (WCHAR *)pOutBuffer;
	
	status = PspEnumerateKey(szKey, szOutKeys, uOutSize);

	if (NT_SUCCESS(status))
	{
		if (dwRet)
		{
			*dwRet = uOutSize;
		}
	}
	
	return status;
}

NTSTATUS PspEnumerateKeyValues(WCHAR *szKey, PVOID szOutValues, ULONG nOutLen)
{
	UNICODE_STRING unKey;
	OBJECT_ATTRIBUTES  ObjectAttributes;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	HANDLE hKey;
	ULONG i = 0;
	ULONG nTotalLen = 0;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnNtOpenKey MzfNtOpenKey = (pfnNtOpenKey)GetGlobalVeriable(enumNtOpenKey);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtEnumerateValueKey MzfNtEnumerateValueKey = (pfnNtEnumerateValueKey)GetGlobalVeriable(enumNtEnumerateValueKey);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;

	if (!szKey ||
		!szOutValues ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!MzfNtOpenKey ||
		!MzfPsGetCurrentThread ||
		!MzfNtEnumerateValueKey ||
		!MzfNtClose)
	{
		return status;
	}

//	KdPrint(("szKey: %S\n", szKey));

	RtlInitUnicodeString(&unKey, szKey);
	InitializeObjectAttributes(&ObjectAttributes, &unKey, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtOpenKey(&hKey, GENERIC_ALL, &ObjectAttributes);
	if (!NT_SUCCESS(status))
	{
		RecoverPreMode(pThread, PreMode);
		KdPrint(("ZwOpenKey error\n"));
		return status;
	}

	while (1)
	{
		ULONG ulSize = 0;

		status = MzfNtEnumerateValueKey(hKey, i, KeyValueFullInformation, NULL, 0, &ulSize);

		if (status == STATUS_BUFFER_TOO_SMALL)
		{
			ULONG nSize = ulSize + 0x100;
			PKEY_VALUE_FULL_INFORMATION pKvfi = (PKEY_VALUE_FULL_INFORMATION)MzfExAllocatePoolWithTag(PagedPool, nSize, MZFTAG);

			if (!pKvfi)
			{
				status = STATUS_UNSUCCESSFUL;
				break;
			}
			
			memset(pKvfi, 0, nSize);

			status = MzfNtEnumerateValueKey(hKey, i, KeyValueFullInformation, pKvfi, nSize, &ulSize);
			if (NT_SUCCESS(status))
			{
				if (nOutLen - nTotalLen <= ulSize)
				{
					MzfExFreePoolWithTag(pKvfi, 0);
					status = STATUS_BUFFER_OVERFLOW;
					break;
				}
				
// 				KdPrint(("ulSize: %d, DataOffset: %d, DataLength: %d, NameLength: %d\n", 
// 					ulSize, pKvfi->DataOffset, pKvfi->DataLength, pKvfi->NameLength));

// 				if (pKvfi->Type == REG_SZ)
// 				{
// 					KdPrint(("Value: %S, Data: %S\n", pKvfi->Name, pKvfi->Name + pKvfi->NameLength / sizeof(WCHAR) + 1));
// 				}
				
				*(PULONG)((ULONG)szOutValues + nTotalLen) = ulSize;
				memcpy((PVOID)((ULONG)szOutValues + nTotalLen + sizeof(ULONG)), pKvfi, ulSize);
				nTotalLen += sizeof(ULONG);
				nTotalLen += ulSize;
			}

			MzfExFreePoolWithTag(pKvfi, 0);
		}
		else if (STATUS_NO_MORE_ENTRIES == status)
		{
			status = STATUS_SUCCESS;
			break;
		}

		i++;
	}

	MzfNtClose(hKey);
	RecoverPreMode(pThread, PreMode);
	return status;
}

NTSTATUS EnumerateKeyValues(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_REG pCr = 	(PCOMMUNICATE_REG)pInBuffer;
	WCHAR *szKey = pCr->szKey;

	status = PspEnumerateKeyValues(szKey, pOutBuffer, uOutSize);

	if (NT_SUCCESS(status))
	{
		if (dwRet)
		{
			*dwRet = uOutSize;
		}
	}

	return status;
}

NTSTATUS RegOpenKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	ACCESS_MASK Mask = pCk->op.Open.DesiredAccess;
	POBJECT_ATTRIBUTES pOa = (POBJECT_ATTRIBUTES)pCk->op.Open.ObjectAttributes;
	PHANDLE pHandle = (PHANDLE)pOutBuffer;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	__try
	{
		ProbeForRead(pOa, sizeof(OBJECT_ATTRIBUTES), 1);
		status = KernelOpenKey(pHandle, Mask, pOa);
	}
	__except(1)
	{
		status = STATUS_UNSUCCESSFUL;
	}	

	return status;
}

NTSTATUS RegEnumerateValueKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	HANDLE KeyHandle = pCk->op.Enum.hKey;
	ULONG Index = pCk->op.Enum.Index;
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass = pCk->op.Enum.InformationClass;
	ULONG Length = pCk->op.Enum.Length;
	PENUM_VALUE pEv = (PENUM_VALUE)pOutBuffer;

	return KernelEnumerateValueKey(KeyHandle, Index, KeyValueInformationClass, pEv->pValueInfo, Length, pEv->RetLength);
}

NTSTATUS RegEnumerateKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	HANDLE KeyHandle = pCk->op.Enum.hKey;
	ULONG Index = pCk->op.Enum.Index;
	ULONG KeyValueInformationClass = pCk->op.Enum.InformationClass;
	ULONG Length = pCk->op.Enum.Length;
	PENUM_VALUE pEv = (PENUM_VALUE)pOutBuffer;

	return KernelEnumerateKey(KeyHandle, Index, KeyValueInformationClass, pEv->pValueInfo, Length, pEv->RetLength);
}

NTSTATUS RegDeleteKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	HANDLE KeyHandle = pCk->op.Delete.hKey;

	return KernelDeleteKey(KeyHandle);
}

NTSTATUS RegCreateKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	PCREATE_KEY_RETURN pCkr = (PCREATE_KEY_RETURN)pOutBuffer;
	PHANDLE KeyHandle = pCkr->pKeyHandle;
	PULONG pDisposition = pCkr->Disposition;
	ACCESS_MASK DesiredAccess = pCk->op.Open.DesiredAccess;
	POBJECT_ATTRIBUTES ObjectAttributes = (POBJECT_ATTRIBUTES)pCk->op.Open.ObjectAttributes;

	return KernelCreateKey(KeyHandle, DesiredAccess, ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, pDisposition);
}

NTSTATUS RegSetValueKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	HANDLE KeyHandle = pCk->op.SetValue.hKey;
	PUNICODE_STRING pValueName = (PUNICODE_STRING)(pCk->op.SetValue.ValueName);
	ULONG Type = pCk->op.SetValue.Type;
	PVOID Data = pCk->op.SetValue.Data;
	ULONG DataSize = pCk->op.SetValue.DataSize;

	KdPrint(("RegSetValueKey\n"));
	return KernelSetValueKey(KeyHandle, pValueName, 0, Type, Data, DataSize);
}

NTSTATUS RegDeleteValueKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	HANDLE KeyHandle = pCk->op.DeleteValue.hKey;
	PUNICODE_STRING pValueName = (PUNICODE_STRING)(pCk->op.DeleteValue.ValueName);

	KdPrint(("RegDeleteValueKey\n"));
	return KernelDeleteValueKey(KeyHandle, pValueName);
}

NTSTATUS RegRenameKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_KEY pCk = (PCOMMUNICATE_KEY)pInBuffer;
	HANDLE KeyHandle = pCk->op.RenameKey.hKey;
	PUNICODE_STRING pNewName = (PUNICODE_STRING)(pCk->op.RenameKey.NewName);

	KdPrint(("RegRenameKey\n"));
	return KernelRenameKey(KeyHandle, pNewName);
}