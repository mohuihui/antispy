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
#include "Comfunc.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Format the value with the actual data as the same way the REGEDIT does....
//// Inputs: 1. Data type of the key
////		 2. Data to be formatted
////		 3. Length of the data
//// Output: 4. Formatted string
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FormatDataWithDataType(DWORD dwKeyType, LPBYTE pbbinKeyData, DWORD dwKeyDataLength, CString &cstrOutput)	
{
	CString cstrTemp, cstrTemp1 ,cstrTemp2;
	int nIndex = 0;
	switch(dwKeyType)
	{
	case REG_SZ:
		{
			CString cstrTemp3 = (WCHAR*)pbbinKeyData;
			for(int i=0;i<cstrTemp3.GetLength();i++)
			{
				cstrTemp1 = cstrTemp3.Mid(i,1); 
				if(cstrTemp1 == L"\"")
				{
					cstrTemp1 = cstrTemp3.Left(i); 
					cstrTemp2 = cstrTemp3.Right(cstrTemp3.GetLength()-i-1);
					cstrTemp3 = cstrTemp1 +L"\\\"" +cstrTemp2;
					i++;
				}
			}

			cstrTemp.Format(_T("\"%s\"\n"), cstrTemp3);
			for(int i=0;i<cstrTemp.GetLength();i++)
			{
				CString cstrTempXx = cstrTemp.Mid(i,2); 
				cstrTemp1 = cstrTemp.Mid(i,1); 
				if(cstrTempXx != L"\\\"" && cstrTemp1 == L"\\")
				{
					cstrTemp1 = cstrTemp.Left(i); 
					cstrTemp2 = cstrTemp.Right(cstrTemp.GetLength()-i-1);
					cstrTemp = cstrTemp1+L"\\\\"+cstrTemp2;
					i++;
				}
			}

			cstrOutput = cstrTemp;
			break;
		}

	case REG_DWORD: /// same is for REG_DWORD_LITTLE_ENDIAN
		{
			DWORD dwValue;
			memcpy(&dwValue, pbbinKeyData, sizeof DWORD);
			cstrTemp.Format(_T("dword:%08x\n"), dwValue);
			cstrOutput = cstrTemp;
			break;
		}

	case REG_BINARY:
	case REG_MULTI_SZ:
	case REG_EXPAND_SZ:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_LIST:
	case REG_RESOURCE_REQUIREMENTS_LIST:
		{
			if(dwKeyType != REG_BINARY)
				cstrOutput.Format(_T("hex(%d):"), dwKeyType);
			else
				cstrOutput.Format(_T("hex:"));

			for(DWORD dwIndex = 0; dwIndex < dwKeyDataLength; dwIndex++)
			{
				cstrTemp1.Format(_T("%02x"), pbbinKeyData[dwIndex]);
				if(dwIndex != 0 && (dwIndex % 0x15 == 0))
				{
					cstrTemp += _T(",\\\n");
					cstrTemp += cstrTemp1;
				}
				else
				{
					if( cstrTemp.IsEmpty() )
						cstrTemp = cstrTemp1;
					else
						cstrTemp += _T(",") + cstrTemp1;
				}
			}

			cstrTemp += _T("\n");
			cstrOutput += cstrTemp;
			break;
		}

	case REG_NONE:
	case REG_DWORD_BIG_ENDIAN:
	case REG_LINK:
		//// TODO : add code for these types...
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
////// Enumerates through the values of the given key and store the values into the file.
////// Invokes "FormatDataWithDataType" function to format the data
////// 
////// Inputs	:	1. Handle to the key
//////			2. Key name
//////			3. File pointer
//////			4. Full path of the key to store in the file
///////////////////////////////////////////////////////////////////////////////////////////
void EnumerateValues(HKEY hKey, CString cstrKey, FILE *fp, CString cstrFullPath, CString szValue)
{
	static HKEY hLastKey = hKey;
	LONG lResult;
	DWORD dwIndex = 0;
	HKEY hCurKey = hKey;
	DWORD dwKeyType; 
	DWORD dwKeyDataLength, dwKeyNameLen;
	LPBYTE pbbinKeyData = NULL; 
	TCHAR *tcKeyName = NULL;
	TCHAR tcDataType[1024] = _T("");

	lResult = RegOpenKeyEx(hCurKey, cstrKey, 0, KEY_QUERY_VALUE , &hKey);
	if(lResult != ERROR_SUCCESS)
		return;

	DWORD lNoOfValues = 0;
	DWORD lLongestKeyNameLen = 1;
	DWORD lLongestDataLen = 1;

	lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &lNoOfValues, &lLongestKeyNameLen, &lLongestDataLen,
		NULL, NULL);

	if(lResult != ERROR_SUCCESS)
		return;

	fwprintf(fp, L"\n[%s]\n", cstrFullPath);

	hLastKey = hKey;

	lLongestKeyNameLen++;
	lLongestDataLen++;

	tcKeyName =	new TCHAR[lLongestKeyNameLen];
	pbbinKeyData = new BYTE[lLongestDataLen];

	CString cstrFinalData, cstrTemp;

	while(TRUE)
	{
		memset(pbbinKeyData, 0, lLongestDataLen);
		memset(tcKeyName, 0, lLongestKeyNameLen);
		dwKeyType = dwKeyDataLength = dwKeyNameLen = 0;

		dwKeyNameLen = lLongestKeyNameLen;
		dwKeyDataLength = lLongestDataLen;

		lResult = RegEnumValue(hKey, dwIndex, tcKeyName, &dwKeyNameLen, NULL, &dwKeyType, pbbinKeyData, &dwKeyDataLength);
		if(lResult == ERROR_NO_MORE_ITEMS)
			break;

		if ( (!szValue.CompareNoCase(szDefault[g_enumLang]) && dwKeyNameLen == 0) ||
			!szValue.CompareNoCase(tcKeyName) )
		{
			FormatDataWithDataType(dwKeyType, pbbinKeyData, dwKeyDataLength, cstrFinalData);

			//// For (default) key names the tcKeyName is empty and dwKeyNameLen is zero ...in such case we need to 
			//// have assignment like @ = "value"
			CString cstrTest;
			cstrTest = tcKeyName;
			if(cstrTest.IsEmpty())
			{
				cstrTemp.Format(_T("@="));
			}
			else
			{
				cstrTemp.Format(_T("\"%s\"="), tcKeyName);
			}
			cstrTemp += cstrFinalData;

			fwprintf(fp, L"%s", (LPCTSTR)cstrTemp);

			break;
		}

		dwIndex++;
	}

	RegCloseKey(hKey);
	delete tcKeyName;
	delete pbbinKeyData;
}

void ExportReGIStry(
					CString cstrKeyRootName, //注册表根值，如HKEY_CURRENT_USER
					CString cstrKeyName,//注册表子键
					CString cstrFileName,
					CString szValue)//导出的文件名（包括路径）
{
	HKEY hKeyRootName;
	CString cstrFullPathStr(_T(""));
	if(cstrKeyRootName == _T("HKEY_CLASSES_ROOT"))
		hKeyRootName = HKEY_CLASSES_ROOT;
	else if(cstrKeyRootName == _T("HKEY_CURRENT_USER"))
		hKeyRootName = HKEY_CURRENT_USER;
	else if(cstrKeyRootName == _T("HKEY_LOCAL_MACHINE"))
		hKeyRootName = HKEY_LOCAL_MACHINE;
	else if(cstrKeyRootName == _T("HKEY_USERS"))
		hKeyRootName = HKEY_USERS;
	else if(cstrKeyRootName == _T("HKEY_PERFORMANCE_DATA"))
		hKeyRootName = HKEY_PERFORMANCE_DATA;
	else if(cstrKeyRootName == _T("HKEY_CURRENT_CONFIG"))
		hKeyRootName = HKEY_CURRENT_CONFIG;
	else if(cstrKeyRootName == _T("HKEY_DYN_DATA"))
		hKeyRootName = HKEY_DYN_DATA;

	FILE *fp;
	/*fp = */
	errno_t nRet = _wfopen_s(&fp, cstrFileName, L"w+");
	if(nRet != 0 || fp == NULL)
	{
		MessageBox(NULL, _T("Error while creating the file"), _T("Registry export"), MB_OK);
		return;
	}

	if(cstrKeyName.IsEmpty())
	{
		cstrFullPathStr = cstrKeyRootName;
	}
	else
	{
		cstrFullPathStr = cstrKeyRootName + _T("\\") + cstrKeyName;
	}

	//// First print the header ..... this may be different for some version of Windows... do manually change if required
	//Here need to add version check
	DWORD dwVersion = GetVersion();
	// Get build numbers for Windows NT or Win32s
	if (dwVersion < 0x80000000)// Windows NT
	{
		fwprintf(fp, L"%s\n", L"Windows Registry Editor Version 5.00");
	}
	else  // Win32s
	{
		fwprintf(fp, L"%s\n", L"REGEDIT4");
	}

	EnumerateValues(hKeyRootName, cstrKeyName, fp, cstrFullPathStr, szValue);

	fclose(fp);
}