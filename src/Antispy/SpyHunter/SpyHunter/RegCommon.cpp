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
#include "RegCommon.h"
#include <Sddl.h>

#define	CLASSES_ROOT		L"\\Registry\\Machine\\SOFTWARE\\Classes"
#define	LOCAL_MACHINE		L"\\Registry\\Machine"
#define USERS				L"\\Registry\\User"
#define CURRENT_CONFIGL     L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current"

// 根据传进来的CLSID值获得相应的描述
CString CRegCommon::GetCLSIDDescription(CString szCLSID)
{
	WCHAR szCLSIDKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','C','L','S','I','D','\\','\0'};
	CString szDescription = L"";

	if (szCLSID.IsEmpty())
	{
		return szDescription;
	}

	CString szKey = szCLSIDKey + szCLSID;
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == 0 && ir->nDataLen > 0)
		{
			szDescription = (WCHAR*)ir->pData;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);

	return szDescription;
}

CString CRegCommon::GetInprocServer32Value(CString szKey)
{
	CString szModule = L"";

	if (szKey.IsEmpty())
	{
		return szModule;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == 0 &&  ir->nDataLen > 0)
		{
			szModule = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			break;
		}
	}
	
	m_Registry.FreeValueList(valueList);

	return szModule;
}

// 根据CLSID获得模块的路径
CString CRegCommon::GetCLSIDModule(CString szKey)
{
	CString szModulePath = L"";
	if (szKey.IsEmpty())
	{
		return szModulePath;
	}

	WCHAR szCLSIDKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','C','L','S','I','D','\\','\0'};
	WCHAR szInprocServer32[] = {'I','n','p','r','o','c','S','e','r','v','e','r','3','2','\0'};
	WCHAR szLocalServer32[] = {'L','o','c','a','l','S','e','r','v','e','r','3','2','\0'};
	CString szKeyTemp = szCLSIDKey + szKey;
	list<KEY_INFO> keyList;

	m_Registry.EnumKeys(szKeyTemp, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (wcslen(szInprocServer32) == ir->nNameLen / sizeof(WCHAR) &&
			!ir->szName.CollateNoCase(szInprocServer32))
		{
			CString szKeyPath = szKeyTemp + CString(L"\\") + szInprocServer32;
			szModulePath = GetInprocServer32Value(szKeyPath);
			break;
		}
		else if (wcslen(szLocalServer32) == ir->nNameLen / sizeof(WCHAR) &&
			!ir->szName.CollateNoCase(szLocalServer32))
		{
			CString szKeyPath = szKeyTemp + CString(L"\\") + szLocalServer32;
			szModulePath = GetInprocServer32Value(szKeyPath);
			break;
		}
	}

	return szModulePath;
}

CString CRegCommon::GetCurrentReg()
{
	HANDLE TokenHandle = NULL;
	UCHAR Buffer[1024] = {0};
	PSID_AND_ATTRIBUTES SidBuffer = NULL;
	ULONG Length = 0;
	CString szRet = L"";
	BOOL Status = FALSE;

	Status = OpenThreadToken(GetCurrentThread(),
		TOKEN_QUERY,
		TRUE,
		&TokenHandle);

	if (!Status)
	{
		Status = OpenProcessToken(GetCurrentProcess(),
			TOKEN_QUERY,
			&TokenHandle);

		if (!Status) 
		{
			return szRet;
		}
	}

	SidBuffer = (PSID_AND_ATTRIBUTES)Buffer;
	Status = GetTokenInformation(TokenHandle,
		TokenUser,
		(PVOID)SidBuffer,
		sizeof(Buffer),
		&Length);

	CloseHandle(TokenHandle);
	if (!Status) 
	{
		return szRet;
	}

	LPTSTR StringSid;
	Status = ConvertSidToStringSid(SidBuffer[0].Sid, &StringSid);
	if (!Status) 
	{
		return szRet;
	}

	WCHAR szUser[] = {'\\','R','E','G','I','S','T','R','Y','\\','U','S','E','R','\\','\0'};
	Length = wcslen(StringSid) * sizeof(WCHAR) + sizeof(szUser);

	WCHAR *szPath = (WCHAR *)malloc(Length);
	if (!szPath)
	{
		LocalFree((HLOCAL)StringSid);
		return szRet;
	}

	memset(szPath, 0, Length);
	wcscpy_s(szPath, Length / sizeof(WCHAR), szUser);
	wcscat_s(szPath, Length / sizeof(WCHAR), StringSid);
	LocalFree((HLOCAL)StringSid);
	szRet = szPath;
	free(szPath);
	return szRet;
}

// 返回CKRU的\Registry\User\xxx的形式
CString CRegCommon::GetCurrentUserKeyPath()
{
	CString szKey = GetCurrentReg();

	if (szKey.IsEmpty())
	{
		WCHAR szDefault[] = {'\\','R','e','g','i','s','t','r','y','\\','U','s','e','r','\\','.','D','e','f','a','u','l','t','\0'};
		szKey = szDefault;
	}

	return szKey;
}

CString CRegCommon::NtKeyPath2DosKeyPath(CString szKey)
{
	CString szRet;

	if (szKey.IsEmpty())
	{
		return szRet;
	}

	WCHAR *szKeyBuf = (WCHAR *)malloc((szKey.GetLength() + 1) * sizeof(WCHAR));
	if (szKeyBuf == NULL)
	{
		return szRet;
	}
	
	memset(szKeyBuf, 0, (szKey.GetLength() + 1) * sizeof(WCHAR));
	wcsncpy_s(szKeyBuf, szKey.GetLength() + 1, szKey.GetBuffer(), szKey.GetLength());
	szKey.ReleaseBuffer();
	
	ULONG nLen = wcslen(szKeyBuf);
	ULONG nRemLen = 0;

	CString szCurrentKey = GetCurrentUserKeyPath();
	CString strRootKey;
	
	if (nLen >= wcslen(CLASSES_ROOT) && 
		!_wcsnicmp(szKeyBuf, CLASSES_ROOT, wcslen(CLASSES_ROOT)))
	{
		strRootKey = L"HKEY_CLASSES_ROOT";
		nRemLen = wcslen(CLASSES_ROOT);
	}
	else if (nLen >= (ULONG)szCurrentKey.GetLength() && 
		!_wcsnicmp(szKeyBuf, szCurrentKey.GetBuffer(), szCurrentKey.GetLength()))
	{
		szCurrentKey.ReleaseBuffer();
		strRootKey = L"HKEY_CURRENT_USER";
		nRemLen = szCurrentKey.GetLength();
	}
	else if (nLen >= wcslen(LOCAL_MACHINE) && 
		!_wcsnicmp(szKeyBuf, LOCAL_MACHINE, wcslen(LOCAL_MACHINE)))
	{
		strRootKey = L"HKEY_LOCAL_MACHINE";
		nRemLen = wcslen(LOCAL_MACHINE);
	}
	else if (nLen >= wcslen(USERS) && 
		!_wcsnicmp(szKeyBuf, USERS, wcslen(USERS)))
	{
		strRootKey = L"HKEY_USERS";
		nRemLen = wcslen(USERS);
	}
	else if (nLen >= wcslen(CURRENT_CONFIGL) && 
		!_wcsnicmp(szKeyBuf, CURRENT_CONFIGL, wcslen(CURRENT_CONFIGL)))
	{
		nRemLen = wcslen(CURRENT_CONFIGL);
		strRootKey = L"HKEY_CURRENT_CONFIG";
	}

	szRet = strRootKey + szKey.Right(szKey.GetLength() - nRemLen);
	
	return szRet;
}