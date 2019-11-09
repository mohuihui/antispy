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
#include "stdafx.h"
#include "Registry.h"
#include "CommonStruct.h"

CRegistry::CRegistry()
{
}

CRegistry::~CRegistry()
{
}

BOOL CRegistry::OpenKey(OUT PHANDLE  KeyHandle,
							IN ACCESS_MASK  DesiredAccess,
							IN PMZF_OBJECT_ATTRIBUTES  ObjectAttributes
							)
{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumOpenKey;
	ck.op.Open.DesiredAccess = DesiredAccess;
	ck.op.Open.ObjectAttributes = ObjectAttributes;
	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), (PVOID)KeyHandle, sizeof(PHANDLE), NULL);
}

NTSTATUS CRegistry::QueryKey(IN HANDLE  KeyHandle,
							 IN KEY_INFORMATION_CLASS  KeyInformationClass,
							 OUT PVOID  KeyInformation,
							 IN ULONG  Length,
							 OUT PULONG  ResultLength
							 )

{
	return 0;
}

BOOL CRegistry::EnumerateValueKey(IN HANDLE  KeyHandle,
								  IN ULONG  Index,
								  IN ULONG  KeyValueInformationClass,
								  OUT PVOID  KeyValueInformation,
								  IN ULONG  Length,
								  OUT PULONG  ResultLength
								  )
{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumEnumValue;
	ck.op.Enum.hKey = KeyHandle;
	ck.op.Enum.Index = Index;
	ck.op.Enum.InformationClass = KeyValueInformationClass;
	ck.op.Enum.Length = Length;

	ENUM_VALUE ev;
	memset(&ev, 0, sizeof(ENUM_VALUE));
	ev.RetLength = ResultLength;
	ev.pValueInfo = KeyValueInformation;

	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), &ev, sizeof(ENUM_VALUE), NULL);
}

BOOL CRegistry::EnumerateKey(IN HANDLE  KeyHandle,
								  IN ULONG  Index,
								  IN ULONG  KeyInformationClass,
								  OUT PVOID  KeyValueInformation,
								  IN ULONG  Length,
								  OUT PULONG  ResultLength
								  )
{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumEnumKey;
	ck.op.Enum.hKey = KeyHandle;
	ck.op.Enum.Index = Index;
	ck.op.Enum.InformationClass = KeyInformationClass;
	ck.op.Enum.Length = Length;

	ENUM_VALUE ev;
	memset(&ev, 0, sizeof(ENUM_VALUE));
	ev.RetLength = ResultLength;
	ev.pValueInfo = KeyValueInformation;

	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), &ev, sizeof(ENUM_VALUE), NULL);
}

BOOL CRegistry::DeleteKey(IN HANDLE KeyHandle)
{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumDeleteKey;
	ck.op.Delete.hKey = KeyHandle;

	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), NULL, 0, NULL);
}

BOOL CRegistry::CreateKey(
			OUT PHANDLE  KeyHandle,
			IN ACCESS_MASK  DesiredAccess,
			IN PMZF_OBJECT_ATTRIBUTES  ObjectAttributes,
			IN ULONG  TitleIndex,
			IN PMZF_UNICODE_STRING  Class  OPTIONAL,
			IN ULONG  CreateOptions,
			OUT PULONG  Disposition  OPTIONAL
			)

{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumCreateKey;
	ck.op.Open.DesiredAccess = DesiredAccess;
	ck.op.Open.ObjectAttributes = ObjectAttributes;

	CREATE_KEY_RETURN ev;
	memset(&ev, 0, sizeof(CREATE_KEY_RETURN));
	ev.pKeyHandle = KeyHandle;
	ev.Disposition = Disposition;

	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), &ev, sizeof(CREATE_KEY_RETURN), NULL);
}

BOOL CRegistry::SetValueKey(
			IN HANDLE  KeyHandle,
			IN PMZF_UNICODE_STRING  ValueName,
			IN ULONG  TitleIndex  OPTIONAL,
			IN ULONG  Type,
			IN PVOID  Data,
			IN ULONG  DataSize
			)
{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumSetValueKey;
	ck.op.SetValue.hKey = KeyHandle;
	ck.op.SetValue.ValueName = ValueName;
	ck.op.SetValue.Type = Type;
	ck.op.SetValue.Data = Data;
	ck.op.SetValue.DataSize = DataSize;

	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), NULL, 0, NULL);
}

BOOL CRegistry::DeleteValueKey(
							   IN HANDLE  KeyHandle,
							   IN PMZF_UNICODE_STRING  ValueName
							   )
{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumDeleteValueKey;
	ck.op.DeleteValue.hKey = KeyHandle;
	ck.op.DeleteValue.ValueName = ValueName;

	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), NULL, 0, NULL);
}

BOOL CRegistry::RenameKey(
							   IN HANDLE  KeyHandle,
							   IN PMZF_UNICODE_STRING  NewName
							   )
{
	COMMUNICATE_KEY ck;
	memset(&ck, 0, sizeof(COMMUNICATE_KEY));
	ck.OpType = enumRenameKey;
	ck.op.RenameKey.hKey = KeyHandle;
	ck.op.RenameKey.NewName = NewName;

	return m_Driver.CommunicateDriver(&ck, sizeof(COMMUNICATE_KEY), NULL, 0, NULL);
}

BOOL CRegistry::CopyRegValues(HANDLE hDes, HANDLE hSrc)
{
	BOOL bRetxx = FALSE;
	if (!hDes || !hSrc)
	{
		return bRetxx;
	}

	for (ULONG i = 0; ; i++)
	{
		ULONG nRetLen = 0;
		BOOL bRet = EnumerateValueKey(hSrc, i, KeyValueFullInformation, NULL, 0, &nRetLen);
		if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
		{
			break;
		}
		else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
		{
			PKEY_VALUE_FULL_INFORMATION pBuffer = (PKEY_VALUE_FULL_INFORMATION)malloc(nRetLen + 100);
			if (pBuffer)
			{
				memset(pBuffer, 0, nRetLen + 100);
				bRet = EnumerateValueKey(hSrc, i, KeyValueFullInformation, pBuffer, nRetLen + 100, &nRetLen);

				if (bRet)
				{
					if (pBuffer->NameLength > 0)
					{
						WCHAR szValue[10240] = {0};
						wcsncpy_s(szValue, 10240, pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));

						MZF_UNICODE_STRING unValue;
						if (m_Function.InitUnicodeString(&unValue, szValue))
						{
							SetValueKey(hDes, &unValue, i, pBuffer->Type, (PVOID)((PBYTE)pBuffer + pBuffer->DataOffset), pBuffer->DataLength);
							m_Function.FreeUnicodeString(&unValue);
						}
					}
					else
					{
						MZF_UNICODE_STRING unValue;
						if (m_Function.InitUnicodeString(&unValue, NULL))
						{
							SetValueKey(hDes, &unValue, i, pBuffer->Type, (PVOID)((PBYTE)pBuffer + pBuffer->DataOffset), pBuffer->DataLength);
							m_Function.FreeUnicodeString(&unValue);
						}
					}
				}

				free(pBuffer);
			}
		}
	}

	return bRetxx;
}

void CRegistry::DeleteKeyAndSubKeys(CString szKey)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	MZF_UNICODE_STRING unKey;
	BOOL bQuit = FALSE;
	if (m_Function.InitUnicodeString(&unKey, szKey.GetBuffer()))
	{
		szKey.ReleaseBuffer();

		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			ULONG i = 0;
			while (1)
			{
				if (bQuit)
				{
					break;
				}

				ULONG nRetLen = 0;
				BOOL bRet = EnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &nRetLen);
				if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
				{
					if ( !DeleteKey(hKey) )
					{
						DebugLog(L"Delete error");
						bQuit = TRUE;
					}

					break;
				}
				else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
				{
					PKEY_BASIC_INFORMATION pBuffer = (PKEY_BASIC_INFORMATION)malloc(nRetLen + 100);

					if (pBuffer)
					{
						memset(pBuffer, 0, nRetLen + 100);
						bRet = EnumerateKey(hKey, i, KeyBasicInformation, pBuffer, nRetLen + 100, &nRetLen);

						if (bRet && pBuffer->NameLength)
						{
							WCHAR szName[1024] = {0};
							wcsncpy_s(szName, 1024, pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));
							CString szKeyTemp = szKey + L"\\" + szName;
							DeleteKeyAndSubKeys(szKeyTemp);
						}
						else
						{
							break;
						}

						free(pBuffer);
					}
				}
			}

			CloseHandle(hKey);
		}

		m_Function.FreeUnicodeString(&unKey);
	}
}

BOOL CRegistry::CopyRegTree(CString szDes, CString szSrc)
{
	BOOL bRetxx = FALSE;
	if (szDes.IsEmpty() || szSrc.IsEmpty())
	{
		return bRetxx;
	}

	HANDLE hDes = CreateKey(szDes);
	HANDLE hSrc = CreateKey(szSrc);

	if (!hDes || !hSrc)
	{
		if (hDes)
		{
			CloseHandle(hDes);
			hDes = NULL;
		}

		if (hSrc)
		{
			CloseHandle(hSrc);
			hSrc = NULL;
		}

		return bRetxx;
	}
			
	for (ULONG i = 0; ; i++)
	{
		ULONG nRetLen = 0;
		BOOL bRet = EnumerateKey(hSrc, i, KeyBasicInformation, NULL, 0, &nRetLen);

		if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
		{
			// 拷贝这个键值下的全部值
			CopyRegValues(hDes, hSrc);

			break;
		}
		else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
		{
			PKEY_BASIC_INFORMATION pBuffer = (PKEY_BASIC_INFORMATION)malloc(nRetLen + 100);
			if (pBuffer)
			{
				memset(pBuffer, 0, nRetLen + 100);
				bRet = EnumerateKey(hSrc, i, KeyBasicInformation, pBuffer, nRetLen + 100, &nRetLen);

				if (bRet && pBuffer->NameLength)
				{
					WCHAR szKeyName[1024] = {0};
					wcsncpy_s(szKeyName, 1024, pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));
					
					// 递归创建键
					szDes += CString(L"\\") + szKeyName;
					szSrc += CString(L"\\") + szKeyName;
					CopyRegTree(szDes, szSrc);
				}

				free(pBuffer);
			}
		}
	}

	if (hDes)
	{
		CloseHandle(hDes);
		hDes = NULL;
	}

	if (hSrc)
	{
		CloseHandle(hSrc);
		hSrc = NULL;
	}

	return bRetxx;
}

void CRegistry::EnumKeys(CString szKey, list<KEY_INFO> &keyList)
{
	keyList.clear();

	HANDLE hKey = OpenKey(szKey);
	if (hKey == NULL)
	{
		return;
	}

	for (ULONG i = 0; ; i++)
	{
		ULONG nRetLen = 0;
		BOOL bRet = EnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &nRetLen);

		if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
		{
			break;
		}
		else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
		{
			PKEY_BASIC_INFORMATION pBuffer = (PKEY_BASIC_INFORMATION)malloc(nRetLen + 100);
			if (pBuffer)
			{
				memset(pBuffer, 0, nRetLen + 100);
				bRet = EnumerateKey(hKey, i, KeyBasicInformation, pBuffer, nRetLen + 100, &nRetLen);

				if (bRet && pBuffer->NameLength > 0)
				{
					ULONG nLen = pBuffer->NameLength + sizeof(WCHAR);
					WCHAR *szKeyName = (WCHAR *)malloc(nLen);
					if (szKeyName)
					{
						memset(szKeyName, 0, nLen);
						wcsncpy_s(szKeyName, nLen / sizeof(WCHAR), pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));
						
						KEY_INFO info;
						info.nNameLen = pBuffer->NameLength;
						info.szName = szKeyName;
						keyList.push_back(info);

						free(szKeyName);
						szKeyName = NULL;
					}
				}

				free(pBuffer);
			}
		}
	}

	CloseHandle(hKey);
}

BOOL CRegistry::EnableOrDisableKey(CString szKeyRoot, BOOL bEnable)
{
	BOOL bRet = FALSE;

	if (szKeyRoot.IsEmpty())
	{
		return bRet;
	}

	CString szKeyName = szKeyRoot.Right(szKeyRoot.GetLength() - szKeyRoot.ReverseFind('\\') - 1);
	CString szDisableKey;

	// 如果是disable
	if (!bEnable)
	{
		CString szKeyPath = szKeyRoot.Left(szKeyRoot.ReverseFind('\\'));
		szDisableKey = szKeyPath + CString(L"\\") + szAntiSpyDisableAutoruns;
	}
	else
	{
		szDisableKey = szKeyRoot.Left(szKeyRoot.Find(szAntiSpyDisableAutoruns) - 1);
	}

	HANDLE hKey = CreateKey(szDisableKey);
	if (hKey)
	{
		CloseHandle(hKey);
		hKey = NULL;

		szDisableKey += CString(L"\\") + szKeyName;
		hKey = CreateKey(szDisableKey);
		if (hKey)
		{
			CloseHandle(hKey);
			hKey = NULL;

			CopyRegTree(szDisableKey, szKeyRoot);
			DeleteKeyAndSubKeys(szKeyRoot);
		}
	}

	return bRet;
}

BOOL CRegistry::EnableOrDisableValue(CString szKey, CString szValue, BOOL bEnable)
{
	BOOL bRet = FALSE;

	if (szKey.IsEmpty() || szValue.IsEmpty())
	{
		return bRet;
	}

	CString szDisableKey;
	if (!bEnable)
	{
		szDisableKey = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
	}
	else
	{
		szDisableKey = szKey.Left(szKey.ReverseFind('\\'));
	}

	HANDLE hKey = CreateKey(szDisableKey);
	if (!hKey)
	{
		return bRet;
	}
	
	list<VALUE_INFO> valueList;
	EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && !ir->szName.CompareNoCase(szValue))
		{
			MZF_UNICODE_STRING unValue;
			if (m_Function.InitUnicodeString(&unValue, szValue.GetBuffer()))
			{
				SetValueKey(hKey, &unValue, 0, ir->Type, ir->pData, ir->nDataLen);
				m_Function.FreeUnicodeString(&unValue);
				DeleteValue(szKey, szValue);
			}

			break;
		}
	}

	FreeValueList(valueList);
	CloseHandle(hKey);
	hKey = NULL;
	return bRet;
}

HANDLE CRegistry::CreateKey( CString szKey, OUT PULONG  Disposition  OPTIONAL )
{
	HANDLE hKey = NULL;
	MZF_UNICODE_STRING unKey;

	if (m_Function.InitUnicodeString(&unKey, szKey.GetBuffer()))
	{
		MZF_OBJECT_ATTRIBUTES oa;
		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);
		if (!CreateKey(&hKey, KEY_ALL_ACCESS, &oa, 0, NULL, 0, Disposition))
		{
			hKey = NULL;
		}

		m_Function.FreeUnicodeString(&unKey);
	}

	szKey.ReleaseBuffer();

	return hKey;
}

HANDLE CRegistry::OpenKey(CString szKey)
{
	HANDLE hKey = NULL;

	if (szKey.IsEmpty())
	{
		return hKey;
	}

	MZF_UNICODE_STRING unKey;
	if (m_Function.InitUnicodeString(&unKey, szKey.GetBuffer()))
	{
		MZF_OBJECT_ATTRIBUTES oa;
		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);
		if (!OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			hKey = NULL;
		}
		
		m_Function.FreeUnicodeString(&unKey);
	}
	szKey.ReleaseBuffer();
	return hKey;
}

void CRegistry::EnumValues(CString szKey, list<VALUE_INFO> &valueList)
{
	valueList.clear();

	if (szKey.IsEmpty())
	{
		return;
	}

	HANDLE hKey = OpenKey(szKey);
	if (hKey == NULL)
	{
		return;
	}

	for (ULONG i = 0; ; i++)
	{
		ULONG nRetLen = 0;
		BOOL bRet = EnumerateValueKey(hKey, i, KeyValueFullInformation, NULL, 0, &nRetLen);
		if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
		{
			break;
		}
		else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
		{
			PKEY_VALUE_FULL_INFORMATION pBuffer = (PKEY_VALUE_FULL_INFORMATION)malloc(nRetLen + 100);
			if (pBuffer)
			{
				memset(pBuffer, 0, nRetLen + 100);
				bRet = EnumerateValueKey(hKey, i, KeyValueFullInformation, pBuffer, nRetLen + 100, &nRetLen);

				if (bRet)
				{
					VALUE_INFO info;

					CString szName = L"";
					if (pBuffer->NameLength)
					{
						ULONG nLen = pBuffer->NameLength + sizeof(WCHAR);
						WCHAR *szNameTemp = (WCHAR *)malloc(nLen);
						if (szNameTemp)
						{
							memset(szNameTemp, 0, nLen);
							wcsncpy_s(szNameTemp, nLen / sizeof(WCHAR), pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));
							szName = szNameTemp;

							free(szNameTemp);
						}
					}
					
					info.nNameLen = pBuffer->NameLength;
					info.szName = szName;
					info.Type = pBuffer->Type;
					info.nDataLen = pBuffer->DataLength;
					info.pData = NULL;

					if (pBuffer->DataLength)
					{
						ULONG nLen = pBuffer->DataLength + sizeof(WCHAR) * 2;
						info.pData = malloc(nLen);
						if (info.pData)
						{
							memset(info.pData, 0, nLen);
							memcpy(info.pData, (PVOID)((PBYTE)pBuffer + pBuffer->DataOffset), pBuffer->DataLength);
						}
					}

					valueList.push_back(info);
				}

				free(pBuffer);
			}
		}
	}

	CloseHandle(hKey);
}

void CRegistry::FreeValueList(list<VALUE_INFO> &valueList)
{
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->pData)
		{
			free(ir->pData);
			ir->pData = NULL;
		}
	}
}

void CRegistry::DeleteValue(CString szKeyPath, CString szValue)
{
	if (szKeyPath.IsEmpty() || szValue.IsEmpty())
	{
		return;
	}

	HANDLE hKey = OpenKey(szKeyPath);
	if (hKey)
	{
		MZF_UNICODE_STRING unValue;

		if (m_Function.InitUnicodeString(&unValue, szValue.GetBuffer()))
		{
			DeleteValueKey(hKey, &unValue);
			m_Function.FreeUnicodeString(&unValue);
		}

		szValue.ReleaseBuffer();

		CloseHandle(hKey);
	}
}

BOOL CRegistry::ModifyValue(IN CString szKey,
							IN CString szValueName, 
						   IN ULONG  Type,
						   IN PVOID  Data,
						   IN ULONG  DataSize)
{
	BOOL bRet = FALSE;

	if (szValueName.IsEmpty())
	{
		return bRet;
	}

	HANDLE hKey = OpenKey(szKey);
	if (hKey)
	{
		MZF_UNICODE_STRING unValue;

		if (m_Function.InitUnicodeString(&unValue, szValueName.GetBuffer()))
		{
			bRet = SetValueKey(hKey, &unValue, 0, Type, Data, DataSize);
			m_Function.FreeUnicodeString(&unValue);
		}

		szValueName.ReleaseBuffer();

		CloseHandle(hKey);
	}

	return bRet;
}

BOOL CRegistry::ModifyNULLNameValue(IN CString szKey,
							IN ULONG  Type,
							IN PVOID  Data,
							IN ULONG  DataSize)
{
	BOOL bRet = FALSE;

	HANDLE hKey = OpenKey(szKey);
	if (hKey)
	{
		MZF_UNICODE_STRING unValue;
		if (m_Function.InitUnicodeString(&unValue, NULL))
		{
			bRet = SetValueKey(hKey, &unValue, 0, Type, Data, DataSize);
			m_Function.FreeUnicodeString(&unValue);
		}

		CloseHandle(hKey);
	}

	return bRet;
}