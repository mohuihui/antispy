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
#include "string_tool.h"
#include "AnalyzeCmdLine.h"

UCHAR gsToUpperTable[256 + 4] =
{
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
	0, 0, 0, 0
};

BOOL CutLastStringByChar(wchar_t *pwszString, wchar_t *pwszOut, wchar_t wChar)
{
	BOOL bResult = FALSE;
	int nSize = 0;
	int nIndex = 0;

	if ( NULL == pwszString )
		goto _Exit_;

	nSize = (int)wcslen(pwszString);
	if ( 0 == nSize )
		goto _Exit_;

	nIndex = nSize;

	while ( nIndex > 0 && pwszString[nIndex -1] != wChar )
	{
		--nIndex;
	}

	if (pwszOut)
	{
		if ( nIndex < nSize )
			wcscpy_sEx(pwszOut, MAX_PATH, pwszString + nIndex);
	}

	if ( nIndex > 0 )
		pwszString[nIndex - 1] = 0;

	if ( nIndex == 0 )
		pwszString[nIndex] = 0;

	bResult = TRUE;

_Exit_:

	return bResult;
}

INT wcscpy_sEx(WCHAR *wcDest, size_t numElement, const WCHAR *wcSrc)
{
	INT size = 0;

	if (!wcDest || !numElement)
	{
		return -1;
	}

	wcDest[0] = 0;

	if (!wcSrc)
	{
		return -1;
	}

	size = (int)min(wcslen(wcSrc), numElement);
	if (size == (int)numElement)
	{
		size--;
	}

	memcpy(wcDest, wcSrc, size * sizeof(WCHAR));
	wcDest[size] = '\0';

	return 0;
}

ULONG GetStringHashWZ(const WCHAR *wsSrc )
{
	ULONG ulLoopCount = 0;
	ULONG ulResult = 0;

	if ( wsSrc == NULL)
		return 0;

	while( wsSrc[ulLoopCount] )
	{
		ulResult = (ulResult * 31) +  gsToUpperTable[(UCHAR)(wsSrc[ulLoopCount] >> 8)];
		ulResult = (ulResult * 31) +  gsToUpperTable[(UCHAR)(wsSrc[ulLoopCount] & 0x00ff)];
		ulLoopCount++;
	}

	return ulResult;
}

int WStrToLower(
									 /* [In] */ wchar_t *pszSrc,
									 /* [Out] */wchar_t *pszDest,
									 /* [In] */ int nOutSize
									 )
{
	int nRet = TRUE;
	int nStrLen = 0;
	int nIndex = 0;

	if (!pszSrc || !pszDest)
		goto Exit0;

	nStrLen = (int)wcslen(pszSrc);
	if(nStrLen <= 0 || nStrLen > nOutSize)
		goto Exit0;

	for(nIndex = 0; nIndex < nStrLen; nIndex ++)
	{
		if(pszSrc[nIndex] >= L'A' && pszSrc[nIndex] <= L'Z')
		{
			pszDest[nIndex] = pszSrc[nIndex] + L'a' - L'A';
		}
		else
		{
			pszDest[nIndex] = pszSrc[nIndex];
		}
	}

	nRet = TRUE;

Exit0:

	return nRet;
}

int wcstrim(WCHAR * pwszStr)   
{   
	if( pwszStr == NULL || pwszStr[0] == '\0' )
		return true;


	PWCHAR pStart = pwszStr;
	PWCHAR pEnd = pwszStr + wcslen(pwszStr) - 1;
	int i;
	while(pEnd > pStart && 0x20 == *pEnd)
		pEnd--;
	while(pStart <= pEnd && 0x20 == *pStart)
		pStart++;

	if( pwszStr != pStart || pEnd != pwszStr + wcslen(pwszStr) - 1)
	{
		for(i=0;i<=pEnd - pStart; ++i)
		{
			pwszStr[i] = pStart[i];
		}
		pwszStr[i]= '\0';
	}

	return true;
}  

inline
BOOL IsSeparChar( WCHAR c )
{
	return c == ';';
}

LPCWSTR ExtractString( LPCWSTR pszBegin, LPWSTR pszBuffer, size_t nBufSize )
{
	if( pszBegin == NULL || pszBuffer == NULL || nBufSize == 0 )
		return NULL;

	LPCWSTR pszStart = pszBegin;
	LPCWSTR pszEnd   = NULL;


	while( IsSeparChar( *pszStart ) )
		pszStart++;

	if( *pszStart == '\0' )
		return NULL;


	pszEnd = pszStart;
	while( *pszEnd != '\0' && !IsSeparChar( *pszEnd ) )
		pszEnd++;


	size_t nLen = pszEnd - pszStart;
	if( nBufSize <= nLen )
		return NULL;

	wcsncpy_s( pszBuffer, nBufSize, pszStart, nLen );
	pszBuffer[nLen] = '\0';

	return pszEnd;
}


BOOL AppendSlashIfNotExist( LPWSTR pwszDir, size_t nSize )
{
	size_t nLen = ::wcslen( pwszDir );
	if( nLen >= nSize - 1 )
		return FALSE;

	if( pwszDir[nLen - 1] != '\\' )
	{
		pwszDir[nLen] = '\\';
		pwszDir[nLen + 1] = '\0';
	}

	return TRUE;
}

BOOL GetEnvironmentVariablesArrayW( LPCTSTR lpName, std::vector<MY_WSTRING>& vecParams )
{
	WCHAR* pBuffer = NULL;
	DWORD dwSize = ::GetEnvironmentVariableW( lpName, pBuffer, 0 );
	if( dwSize == 0 )
		return FALSE;

	pBuffer = new WCHAR[dwSize];
	if( pBuffer == NULL )
		return FALSE;


	dwSize = ::GetEnvironmentVariableW( lpName, pBuffer, dwSize );
	if( dwSize == 0 )
		return FALSE;



	LPCWSTR pszBegin = pBuffer;
	MY_WSTRING  strVar;

	while( ( pszBegin = ::ExtractString( pszBegin, strVar.string, MAX_PATH ) ) != NULL )
	{
		wcstrim( strVar.string );

		if( !::AppendSlashIfNotExist( strVar.string, MAX_PATH ) )
			continue;

		vecParams.push_back( strVar );
	}


	delete [] pBuffer;

	return TRUE;
}

void GetEntryDir( std::vector<MY_WSTRING>& vecEntry )
{
	//MY_WSTRING  strDir;
	vecEntry.clear();

	//::GetSystemDirectory( strDir.string, MAX_PATH );
	//::AppendSlashIfNotExist( strDir.string, MAX_PATH );
	//vecEntry.push_back( strDir );

	//::GetWindowsDirectory( strDir.string, MAX_PATH );
	//::AppendSlashIfNotExist( strDir.string, MAX_PATH );
	//vecEntry.push_back( strDir );


	::GetEnvironmentVariablesArrayW( L"PATH", vecEntry );
}

int DivPath(PWCHAR pwszPath, WCHAR *pwszCurDir, vector<MY_WSTRING>& FullPathList)
{
	PWCHAR p = NULL;
	MY_WSTRING FullPath = {0}, StrTemp = {0};
	WCHAR wszComma[MAX_PATH] = {0};
	wchar_t wszCmdLine[MAX_PATH] = {0};

	wcstrim(pwszPath);

	/*	wchar_t wszFileName[2][MAX_PATH] = {
	L"Program Files",
	L"Documents and Settings"
	};*/

	wchar_t pwszLowPath[MAX_PATH] = {0};

	int nLen = (int)wcslen(pwszPath);
	if(nLen > MAX_PATH)
		goto Exit1;

	wcscpy_s(wszCmdLine, MAX_PATH, pwszPath);

	WStrToLower(pwszPath, pwszLowPath, MAX_PATH);

	/*if(pwszPath[1] == L':'&& (wcsstr(pwszPath, wszFileName[0]) || wcsstr(pwszPath, wszFileName[1])))
	{
	p = wcsstr(pwszPath, L".exe");
	if(p != NULL)
	{
	*(p + 4) = L'\0';
	}
	}*/

    // 解决管道，导致 firefox 和 thinkpad pc-doctor 冲突的问题。
    if (_wcsnicmp(pwszPath, L"\\\\.\\pipe\\", wcslen(L"\\\\.\\pipe\\")) == 0 || 
        _wcsnicmp(pwszPath, L"\"\\\\.\\pipe\\", wcslen(L"\"\\\\.\\pipe\\")) == 0)
    {
        goto Exit1;
    }

	if((0xFFFFFFFF != GetFileAttributesW(pwszPath))&& ((GetFileAttributesW(pwszPath) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
	{
		wcscpy_s(FullPath.string, MAX_PATH, pwszPath);
		FullPathList.push_back(FullPath);
		goto Exit1;
	}
	if( nLen >= 3)
	{
		if(L'\"'== pwszPath[0] )
		{
			p = wcsstr(pwszPath+1,L"\"");
			if(p != NULL)
			{
				if(wcslen(p) >= 3)
				{
					DivPath(p+1,pwszCurDir,FullPathList);                    
				}
				p[0] = L'\0';
				pwszPath[0] = L' ';
				wcstrim(pwszPath);
			}
		}
		else
		{//check C:\Program\SSS.EXE SSS
			p = wcsstr(pwszPath,L" ");
			if(p != NULL)
			{
				if(wcslen(p) >= 3)
				{
					DivPath(p+1,pwszCurDir,FullPathList);                    
				}
				p[0] = L'\0';
			}
		}
	}
	//get path finished

	//Check Comma
	p = wcsstr(pwszPath,L",");
	if(p != NULL)
	{
		wcscpy_s(wszComma, MAX_PATH, pwszPath);
		p =wszComma + ( p - pwszPath);

		if(wcslen(p) >= 3)
		{
			DivPath(p+1,pwszCurDir,FullPathList);                    
		}

		p[0] = L'\0';
		DivPath(wszComma,pwszCurDir,FullPathList);
	}

	//Filter
	if(L'/' == pwszPath[0])
	{
		p = wcsstr(pwszPath,L".");
		if(p == NULL)
		{
			p = wcsstr(pwszPath,L"\\");
			if(p == NULL)
				goto Exit1;
		}
	}

	//To Full Path
	p = wcsstr(pwszPath,L"\\");
	if(p == NULL)
	{
		if(pwszCurDir)
		{
			DWORD dwStrlen = wcslen(pwszCurDir);
			DWORD dwLen2 = wcslen(pwszPath);
			if(dwStrlen + dwLen2 < MAX_PATH - 1)
			{
				memset(&StrTemp, 0, sizeof(MY_WSTRING));
				wcsncpy_s(StrTemp.string, MAX_PATH, pwszCurDir, dwStrlen);

				if(pwszCurDir[dwStrlen - 1] != L'\\')
				{
					wcsncat_s(StrTemp.string, MAX_PATH, L"\\", _TRUNCATE);
				}

				wcsncat_s(StrTemp.string, MAX_PATH, pwszPath, _TRUNCATE);

				if((0xFFFFFFFF != GetFileAttributesW(StrTemp.string))  && ((GetFileAttributesW(StrTemp.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
				{
					FullPathList.push_back(StrTemp);
					goto Exit1;
				}
			}
		}

		if(wcsstr(pwszPath, L".fon"))
		{
			memset(&StrTemp, 0, sizeof(MY_WSTRING));
			::GetWindowsDirectory(StrTemp.string,MAX_PATH);
			wcsncat_s(StrTemp.string,MAX_PATH, L"\\fonts\\", _TRUNCATE);
			wcsncat_s(StrTemp.string,MAX_PATH, pwszPath, _TRUNCATE);
			if((0xFFFFFFFF != GetFileAttributesW(StrTemp.string))  && ((GetFileAttributesW(StrTemp.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
			{
				FullPathList.push_back(StrTemp);
				goto Exit1;
			}
		}



		memset(&FullPath, 0, sizeof(MY_WSTRING));
		::GetSystemDirectory(FullPath.string,MAX_PATH);
		wcsncat_s(FullPath.string, MAX_PATH, L"\\", _TRUNCATE);
		wcsncat_s(FullPath.string, MAX_PATH, pwszPath, _TRUNCATE);
		if((0xFFFFFFFF != GetFileAttributesW(FullPath.string))  && ((GetFileAttributesW(FullPath.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
		{
			FullPathList.push_back(FullPath);
			goto Exit1;
		}
		else
		{
			memset(&StrTemp, 0, sizeof(MY_WSTRING));
			::GetWindowsDirectory(StrTemp.string,MAX_PATH);
			wcsncat_s(StrTemp.string, MAX_PATH, L"\\", _TRUNCATE);
			wcsncat_s(StrTemp.string, MAX_PATH, pwszPath,_TRUNCATE);
			if((0xFFFFFFFF != GetFileAttributesW(StrTemp.string))  && ((GetFileAttributesW(StrTemp.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
			{
				FullPathList.push_back(StrTemp);
				goto Exit1;
			}
		}


		// 环境变量   add by lichenglin .
		{
			std::vector<MY_WSTRING> vecEntry;
			::GetEntryDir( vecEntry );


			for( size_t i = 0; i < vecEntry.size(); i++ )
			{
				wcsncat_s( vecEntry[i].string, MAX_PATH, pwszPath, _TRUNCATE);

				if((0xFFFFFFFF != GetFileAttributesW(vecEntry[i].string))  && 
					((GetFileAttributesW(vecEntry[i].string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
				{
					FullPathList.push_back(vecEntry[i]);
					goto Exit1;
				}
			}
		}
		//   end.

	}
	else
	{
		if(_wcsnicmp(pwszPath, L"system32\\", 9) == 0)
		{
			p = wcsstr(pwszPath, L"\\");
			memset(&FullPath, 0, sizeof(MY_WSTRING));
			::GetSystemDirectory(FullPath.string,MAX_PATH);

			// fixed by minzhenfei
			if ( FullPath.string[wcslen(FullPath.string)-1] != L'\\' && p[0] != L'\\' )
			{
				wcsncat_s(FullPath.string, MAX_PATH, L"\\", _TRUNCATE);
			}
		
			wcsncat_s(FullPath.string, MAX_PATH, p, _TRUNCATE);
		}
		else
			wcscpy_s(FullPath.string, MAX_PATH, pwszPath);
	}
	if((0xFFFFFFFF != GetFileAttributesW(FullPath.string))  && ((GetFileAttributesW(FullPath.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
	{
		FullPathList.push_back(FullPath);
		goto Exit1;
	}
	else
	{//Extend
		memset(&StrTemp, 0, sizeof(MY_WSTRING));
		memcpy(&StrTemp, &FullPath, sizeof(MY_WSTRING));

		wcsncat_s(StrTemp.string, MAX_PATH, L".exe", _TRUNCATE);
		if((0xFFFFFFFF != GetFileAttributesW(StrTemp.string))  && ((GetFileAttributesW(StrTemp.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
		{
			FullPathList.push_back(StrTemp);
			goto Exit1;
		}
		else
		{
			memset(&StrTemp, 0, sizeof(MY_WSTRING));
			memcpy(&StrTemp, &FullPath, sizeof(MY_WSTRING));
			wcsncat_s(StrTemp.string, MAX_PATH, L".dll", _TRUNCATE);
			if((0xFFFFFFFF != GetFileAttributesW(StrTemp.string))  && ((GetFileAttributesW(StrTemp.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
			{
				FullPathList.push_back(StrTemp);
				goto Exit1;
			}
		}
	}

Exit1:
	p = NULL;
	return true;
}

int GetFileFullPath(PWCHAR pwszPath, WCHAR *pwszCurDir, vector<MY_WSTRING>&  FullPathList)
{
	int nRet = FALSE;
	PWCHAR p;
	MY_WSTRING StrTemp;
	int nStrLen = 0;

	wchar_t *wszCmdLine = NULL;;
	wchar_t *pwszFullPath = NULL;
	wchar_t *wszTemp = NULL;

	wszCmdLine = (WCHAR*)malloc(MAX_PATH*sizeof(WCHAR));
	pwszFullPath = (WCHAR*)malloc(MAX_PATH*sizeof(WCHAR));
	wszTemp = (WCHAR*)malloc(MAX_PATH*sizeof(WCHAR));

	FullPathList.clear();
	ExpandEnvironmentStrings(pwszPath,pwszFullPath,MAX_PATH);

	nStrLen = (int)wcslen(pwszFullPath);

	if(nStrLen >= MAX_PATH)
		goto Exit0;

	wcscpy_s(wszCmdLine, MAX_PATH, pwszFullPath);
	//cut commit ; REM
	p = wcsstr(pwszFullPath,L";");
	if(p != NULL)
		p[0] = '\0';
	/*	p = wcsstr(pwszFullPath,L"REM");
	if(p != NULL)
	{
	p[0] = '\0';
	wcscpy(wszCmdLine, L"\0");
	}*/
	//if(wcsnicmp(pwszFullPath, L"Res://", wcslen(L"Res://")) == 0)

	LPCWSTR pwszHead = wcsstr( pwszFullPath, L"://" );
	if( pwszHead != NULL )
	{
		wcscpy_s(pwszFullPath, MAX_PATH, pwszHead + 3 );
		CutLastStringByChar(pwszFullPath, wszTemp, L'/');
	}

	wcstrim(pwszFullPath);
	if(wcslen(pwszFullPath) <= 2 || wcslen(pwszFullPath) >= MAX_PATH)
	{
		pwszFullPath[0]='\0';
		return TRUE;
	}
	DivPath(pwszFullPath, pwszCurDir, FullPathList);

	if(FullPathList.size() == 0)
	{
		memset(&StrTemp, 0, sizeof(MY_WSTRING));
		if(GetFirstPath(wszCmdLine, StrTemp.string, MAX_PATH))
		{
			if((0xFFFFFFFF != GetFileAttributesW(StrTemp.string))  && ((GetFileAttributesW(StrTemp.string) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
			{
				FullPathList.push_back(StrTemp);
				goto Exit1;
			}
		}
	}

Exit1:
	nRet = TRUE;
Exit0:

	if( wszCmdLine != NULL )
	{
		free( wszCmdLine );
		wszCmdLine = NULL;
	}

	if( pwszFullPath != NULL )
	{
		free( pwszFullPath );
		pwszFullPath = NULL;
	}


	if( wszTemp != NULL )
	{
		free( wszTemp );
		wszTemp = NULL;
	}

	return nRet;
}