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
#include "Function.h"
#define STRSAFE_NO_DEPRECATE 
#include <strsafe.h>
#include <odbcinst.h>
#include <afxdb.h>
#include <shlwapi.h>
#include "RegistryDlg.h"
#include "AssemblyDlg.h"
#include "MemoryHexEditDlg.h"
#include "ServiceDlg.h"
#include "FileDlg.h"
#include "RegStringTool.h"

CommonFunctions::CommonFunctions()
{
}

CommonFunctions::~CommonFunctions()
{
}

BOOL CommonFunctions::IsMicrosoftApp(CString szCompany)
{
	BOOL bRet = FALSE;

	if (szCompany.IsEmpty())
	{
		return bRet;
	}

	if (!szCompany.CompareNoCase(L"Microsoft Corporation")					||
		!szCompany.CompareNoCase(L"Microsoft Corp.")						||
		!szCompany.CompareNoCase(L"Microsoft Corp., Veritas Software")		||
		!szCompany.CompareNoCase(L"Microsoft Corp., Veritas Software.")
		)
	{
		bRet = TRUE;
	}
	
	return bRet;
}

BOOL CommonFunctions::IsMicrosoftAppByPath(CString szPath)
{
	CString szCompany = GetFileCompanyName(szPath);
	return IsMicrosoftApp(szCompany);
}

CString CommonFunctions::TrimPath(WCHAR * szPath)
{
	CString strPath;

// 	if (wcschr(szPath, L'~'))
// 	{
// 		WCHAR szLongPath[MAX_PATH] = {0};
// 		GetLongPathName(szPath, szLongPath, MAX_PATH);
// 		wcsncpy_s(szPath, MAX_PATH, szLongPath, wcslen(szLongPath) + 1);
// 	}

	if (wcslen(szPath) > wcslen(L"\\SystemRoot\\") && 
		!_wcsnicmp(szPath, L"\\SystemRoot\\", wcslen(L"\\SystemRoot\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, szPath + wcslen(L"\\SystemRoot\\"));
	}
	else if (wcslen(szPath) > wcslen(L"system32\\") && 
		!_wcsnicmp(szPath, L"system32\\", wcslen(L"system32\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, szPath/* + wcslen(L"system32\\")*/);
	}
	else if (wcslen(szPath) > wcslen(L"\\??\\") &&
		!_wcsnicmp(szPath, L"\\??\\", wcslen(L"\\??\\")))
	{
		strPath = szPath + wcslen(L"\\??\\");
	}
	else if (wcslen(szPath) > wcslen(L"%ProgramFiles%") &&
		!_wcsnicmp(szPath, L"%ProgramFiles%", wcslen(L"%ProgramFiles%")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
		{
			CString szTemp = szSystemDir;
			szTemp = szTemp.Left(szTemp.Find('\\'));
			szTemp += L"\\Program Files";
			szTemp += szPath + wcslen(L"%ProgramFiles%"); 
			strPath = szTemp;
		}
	}
// 	else if (wcschr(szPath, '\\') == NULL)
// 	{
// 		WCHAR szSystemDir[MAX_PATH] = {0};
// 		if (GetSystemDirectory(szSystemDir, MAX_PATH) != 0)
// 		{
// 			CString szTemp = szSystemDir;
// 			szTemp += L"\\";
// 			szTemp += szPath;
// 			strPath = szTemp;
// 		}
// 	}
	else
	{
		strPath = szPath;
	}

	strPath = GetLongPath(strPath);

	return strPath;
}

int CommonFunctions::GetSelectItem(CSortListCtrl *pList)
{
	int nRet = -1;
	
	if (pList)
	{
		POSITION pos = pList->GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			nRet = pList->GetNextSelectedItem(pos);
		}
	}

	return nRet;
}

CString CommonFunctions::GetFileCompanyName(CString szPath)
{
	CString strCompanyName;

	if (szPath.IsEmpty())
	{
		return NULL;
	}

	if (!szPath.CompareNoCase(L"Idle") || !szPath.CompareNoCase(L"System"))
	{
		return NULL;
	}

	if (!PathFileExists(szPath))
	{
		strCompanyName = szFileNotFound[g_enumLang];
		return strCompanyName;
	}
	
	struct LANGANDCODEPAGE {
			WORD wLanguage;
			WORD wCodePage;
		} *lpTranslate;

	LPWSTR lpstrFilename = (LPWSTR)(LPCWSTR)szPath;
	DWORD dwHandle = 0;
	DWORD dwVerInfoSize = GetFileVersionInfoSize(lpstrFilename, &dwHandle);

	if (dwVerInfoSize)
	{
		LPVOID lpbuffer = LocalAlloc(LMEM_ZEROINIT, dwVerInfoSize);

		if (lpbuffer)
		{
			if (GetFileVersionInfo(lpstrFilename, dwHandle, dwVerInfoSize, lpbuffer))
			{
				UINT cbTranslate = 0;

				if ( VerQueryValue( lpbuffer, L"\\VarFileInfo\\Translation", (LPVOID*) &lpTranslate, &cbTranslate))
				{                
					LPCWSTR lpwszBlock = 0;          
					UINT    cbSizeBuf  = 0;
					WCHAR   szSubBlock[MAX_PATH] = {0};

					if ((cbTranslate/sizeof(struct LANGANDCODEPAGE)) > 0)   
					{
						StringCchPrintf( szSubBlock, sizeof(szSubBlock)/sizeof(WCHAR), 
							L"\\StringFileInfo\\%04x%04x\\CompanyName", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage); 
					}

					if ( VerQueryValue(lpbuffer, szSubBlock, (LPVOID*)&lpwszBlock, &cbSizeBuf) )
					{
						WCHAR lpoutCompanyName[MAX_PATH] = {0};

						StringCchCopy(lpoutCompanyName, MAX_PATH/sizeof(WCHAR), (LPCWSTR)lpwszBlock);
						strCompanyName = lpoutCompanyName;
					}
				}
			}

			LocalFree(lpbuffer);
		}
	}

	return strCompanyName;
}

CString CommonFunctions::GetFileDescription(CString szPath)
{
	CString strDescription; 

	if (!szPath.IsEmpty() && PathFileExists(szPath))
	{
		LPWSTR lpstrFilename = (LPWSTR)(LPCWSTR)szPath;
		DWORD dwSize = GetFileVersionInfoSize(lpstrFilename, NULL); 

		if (dwSize)
		{
			LPVOID pBlock = malloc(dwSize); 
			if (pBlock)
			{
				if (GetFileVersionInfo(lpstrFilename, 0, dwSize, pBlock))
				{
					char* pVerValue = NULL; 
					UINT nSize = 0; 
					if (VerQueryValue(pBlock, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&pVerValue, &nSize))
					{
						CString strSubBlock, strTranslation;
						strDescription.Format(L"000%x", *((unsigned short int *)pVerValue)); 
						strTranslation = strDescription.Right(4); 
						strDescription.Format(L"000%x", *((unsigned short int *)&pVerValue[2])); 
						strTranslation += strDescription.Right(4); 

						strSubBlock.Format(L"\\StringFileInfo\\%s\\FileDescription", strTranslation); 
						if (VerQueryValue(pBlock, strSubBlock.GetBufferSetLength(256), (LPVOID*)&pVerValue, &nSize))
						{
							strSubBlock.ReleaseBuffer(); 
							strDescription.Format(L"%s", pVerValue); 
						}
					}
				}

				free(pBlock);
			}
		}
	}

	return strDescription;
}

BOOL CommonFunctions::SetStringToClipboard(CString szImageName)
{
	if (szImageName.IsEmpty())
	{
		return TRUE;
	}

	BOOL bRet = FALSE;
	
	if( OpenClipboard(NULL) )
	{
		HGLOBAL clipbuffer = 0;
		WCHAR* buffer = NULL;

		EmptyClipboard();
		clipbuffer = LocalAlloc(GMEM_ZEROINIT, (szImageName.GetLength() + 1) * sizeof(WCHAR));
		if (clipbuffer)
		{
			buffer = (WCHAR*)GlobalLock(clipbuffer);
			if (buffer)
			{
				wcsncpy_s(buffer, szImageName.GetLength() + 1, szImageName.GetBuffer(), szImageName.GetLength());
				szImageName.ReleaseBuffer();
				SetClipboardData(CF_UNICODETEXT, clipbuffer);
				GlobalUnlock(clipbuffer);

				bRet = TRUE;
			}
		}

		CloseClipboard();
	}

	return bRet;
}

void CommonFunctions::OnCheckAttribute(CString szPath)
{
	if (!szPath.IsEmpty() && PathFileExists(szPath))
	{
		SHELLEXECUTEINFO execInfo ;
		memset( &execInfo, 0, sizeof( execInfo ) ) ;
		execInfo.fMask = SEE_MASK_INVOKEIDLIST ;   
		execInfo.cbSize = sizeof( execInfo ) ;   
		execInfo.hwnd = NULL ;   
		execInfo.lpVerb = _T( "properties" ) ;   
		execInfo.lpFile = szPath ;   
		execInfo.lpParameters = NULL ;   
		execInfo.lpDirectory = NULL ;   
		execInfo.nShow = SW_SHOWNORMAL ;   
		execInfo.hProcess = NULL ;   
		execInfo.lpIDList = 0 ;   
		execInfo.hInstApp = 0 ;   
		ShellExecuteEx( &execInfo ) ;
	}
	else
	{
		MessageBox(NULL, szCannotFindFile[g_enumLang], NULL, MB_OK | MB_ICONWARNING);
	}
}

void CommonFunctions::LocationExplorer(CString szPath)
{
	if (!szPath.IsEmpty() && PathFileExists(szPath))
	{
		CString strCmd;
		strCmd.Format(L"/select,%s", szPath);
		ShellExecuteW(NULL, L"open", L"explorer.exe", strCmd, NULL, SW_SHOW);
	}
	else
	{
		MessageBox(NULL, szCannotFindFile[g_enumLang], NULL, MB_OK | MB_ICONWARNING);
	}
}

void CommonFunctions::BaiduSearch(CString szImageName)
{
	if (!szImageName.IsEmpty())
	{
		/*L"http://www.baidu.com/s?wd=%s"*/
		WCHAR szFormat[] = {'h','t','t','p',':','/','/','w','w','w','.','b','a','i','d','u','.','c','o','m','/','s','?','w','d','=','%','s','\0'};
		WCHAR szOpen[] = {'o','p','e','n','\0'};
		CString strCmd;
		strCmd.Format(szFormat, szImageName);
		ShellExecuteW(NULL, szOpen, strCmd, NULL, NULL, SW_SHOW);
	}
}

void CommonFunctions::GoogleSearch(CString szImageName)
{
	if (!szImageName.IsEmpty())
	{
		/*L"http://www.google.com/search?q=%s"*/
		WCHAR szFormat[] = {'h','t','t','p',':','/','/','w','w','w','.','g','o','o','g','l','e','.','c','o','m','/','s','e','a','r','c','h','?','q','=','%','s','\0'};
		WCHAR szOpen[] = {'o','p','e','n','\0'};
		CString strCmd;
		strCmd.Format(szFormat, szImageName);
		ShellExecuteW(NULL, szOpen, strCmd, NULL, NULL, SW_SHOW);
	}
}

void CommonFunctions::SearchOnline(CString szImageName)
{
	if (!szImageName.IsEmpty())
	{
		if (g_enumLang == enumEnglish)
		{
			GoogleSearch(szImageName);
		}
		else
		{
			BaiduSearch(szImageName);
		}
	}
}

CString CommonFunctions::GetExcelDriver()
{
	WCHAR szBuf[2001];
	WORD cbBufMax = 2000;
	WORD cbBufOut;
	WCHAR *pszBuf = szBuf;
	CString sDriver;

	if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut))
		return NULL;

	do
	{
		if (wcsstr(pszBuf, L"Excel") != 0)
		{
			sDriver = CString(pszBuf);
			break;
		}

		pszBuf = wcschr(pszBuf, L'\0') + 1;
	}
	while (pszBuf[1] != L'\0');

	return sDriver;
}

BOOL CommonFunctions::GetDefaultXlsFileName(CString& sExcelFile)
{
	CString timeStr;
	CTime day = CTime::GetCurrentTime();
	int filenameday, filenamemonth, filenameyear, filehour, filemin, filesec;

	filenameday = day.GetDay();
	filenamemonth = day.GetMonth();
	filenameyear = day.GetYear();
	filehour = day.GetHour();
	filemin = day.GetMinute();
	filesec = day.GetSecond();
	timeStr.Format(L"%04d%02d%02d%02d%02d%02d", filenameyear, filenamemonth, filenameday, filehour, filemin, filesec);

	sExcelFile = timeStr + L".xls";

	CFileDialog fileDlg(
		FALSE, 
		0, 
		sExcelFile, 
		0, 
		L"Excel Files (*.xls)|*.xls|All Files (*.*)|*.*||",
		0
		);

	if (IDOK == fileDlg.DoModal())
	{
		sExcelFile = fileDlg.GetFileName();

		CString szXls = sExcelFile.Right((int)wcslen(L".xls"));

		if (szXls.CompareNoCase(L".xls") != 0)
		{
			sExcelFile += L".xls";
		}

		if ( !PathFileExists(sExcelFile) )
		{
			return TRUE;
		}
		else if	(PathFileExists(sExcelFile) && MessageBox(NULL, L"文件已经存在，是否覆盖？", L"导出", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			if(!DeleteFile(sExcelFile))
			{
				MessageBox(NULL, L"覆盖文件失败！", L"导出", MB_OK | MB_ICONERROR);
				return FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

void CommonFunctions::ExportListToExcel(CSortListCtrl* pList, CString strTitle, CString szStatus)
{
	BOOL bOk = FALSE;

	if (pList->GetItemCount () > 0) 
	{ 
		CDatabase database;
		CString sDriver;
		CString sExcelFile; 
		CString sSql;
		CString tableName = strTitle;

		sDriver = GetExcelDriver();
		if (sDriver.IsEmpty())
		{
			MessageBox(NULL, L"没有安装Excel!\n请先安装Excel软件才能使用导出功能!", L"导出", MB_OK | MB_ICONERROR);
			return;
		}

		if (!GetDefaultXlsFileName(sExcelFile))
			return;
		
		sSql.Format(L"DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%s\";DBQ=%s", sDriver, sExcelFile, sExcelFile);

		if( database.OpenEx(sSql, CDatabase::noOdbcDialog) )
		{
			int i;
			LVCOLUMN columnData;
			CString columnName;
			int columnNum = 0;
			CString strH;
			CString strV;

			sSql = L"";
			strH = L"";
			columnData.mask = LVCF_TEXT;
			columnData.cchTextMax = 100;
			columnData.pszText = columnName.GetBuffer(100);

			for(i = 0; pList->GetColumn(i, &columnData); i++)
			{
				if (i != 0)
				{
					sSql = sSql + L", " ;
					strH = strH + L", " ;
				}

				sSql = sSql + L" " + L"[" + columnData.pszText + L"]" + L" TEXT";
				strH = strH + L" " + L"[" + columnData.pszText + L"]" + L" ";
			}

			columnName.ReleaseBuffer();
			columnNum = i;

			sSql = L"CREATE TABLE " + tableName + L" ( " + sSql +   L" ) ";
			database.ExecuteSQL(sSql);

			for (int nItemIndex = 0; nItemIndex < pList->GetItemCount(); nItemIndex++)
			{
				strV = L"";
				for( i = 0; i < columnNum; i++)
				{
					if (i != 0)
					{
						strV = strV + L", " ;
					}

					strV = strV + L" '" + pList->GetItemText(nItemIndex, i) + L"' ";
				}

				sSql = L"INSERT INTO "+ tableName + L" ("+ strH + L")" + L" VALUES("+ strV + L")";
				database.ExecuteSQL(sSql);
			}

			strV = L"";
			for( i = 0; i < columnNum; i++)
			{
				if (i != 0)
				{
					strV = strV + L", " ;
				}

				strV = strV + L" '" + L" " + L"' ";
			}

			sSql = L"INSERT INTO "+ tableName + L" ("+ strH + L")" + L" VALUES("+ strV + L")";
			database.ExecuteSQL(sSql);

			strV = L"";
			strV = strV + L" '" + szStatus + L"' ";
			for( i = 1; i < columnNum; i++)
			{
				strV = strV + L", " ;
				strV = strV + L" '" + L" " + L"' ";
			}

			sSql = L"INSERT INTO "+ tableName + L" ("+ strH + L")" + L" VALUES("+ strV + L")";
			database.ExecuteSQL(sSql);

			bOk = TRUE;
		}      

		database.Close();

		if ( bOk && PathFileExists(sExcelFile) )
		{
			ShellExecuteW(NULL, L"open", sExcelFile, NULL, NULL, SW_SHOW);
		}
		else
		{
			MessageBox(NULL, L"导出到Excel文件失败!", L"导出", MB_OK | MB_ICONERROR);
		}
	}
}

BOOL CommonFunctions::GetDefaultTxtFileName(CString& sExcelFile)
{
	CString timeStr;
	CTime day = CTime::GetCurrentTime();
	int filenameday, filenamemonth, filenameyear, filehour, filemin, filesec;

	filenameday = day.GetDay();
	filenamemonth = day.GetMonth();
	filenameyear = day.GetYear();
	filehour = day.GetHour();
	filemin = day.GetMinute();
	filesec = day.GetSecond();
	timeStr.Format(L"%04d%02d%02d%02d%02d%02d", filenameyear, filenamemonth, filenameday, filehour, filemin, filesec);

	sExcelFile = timeStr + L".txt";

	CFileDialog fileDlg(
		FALSE, 
		0, 
		sExcelFile, 
		0, 
		L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*||",
		0
		);

	if (IDOK == fileDlg.DoModal())
	{
		sExcelFile = fileDlg.GetFileName();

		CString szXls = sExcelFile.Right((int)wcslen(L".txt"));

		if (szXls.CompareNoCase(L".txt") != 0)
		{
			sExcelFile += L".txt";
		}

		if ( !PathFileExists(sExcelFile) )
		{
			return TRUE;
		}
		else if	(PathFileExists(sExcelFile) && MessageBox(NULL, L"文件已经存在，是否覆盖？", L"导出", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			if(!DeleteFile(sExcelFile))
			{
				MessageBox(NULL, L"覆盖文件失败！", L"导出", MB_OK | MB_ICONERROR);
				return FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

void CommonFunctions::ExportListToTxt(CSortListCtrl* pList, CString szStatus)
{
	if (pList->GetItemCount () > 0) 
	{ 
		CString sExcelFile; 

		if (!GetDefaultTxtFileName(sExcelFile))
			return;

		CFile file;

		TRY
		{
			if (file.Open(sExcelFile, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL))
			{
				int i = 0;
				LVCOLUMN columnData;
				CString columnName;
				int columnNum = 0;
				CString strCloumn;
				CHAR szColumnA[0x4000] = {0};
				WCHAR szColumnW[0x4000] = {0};
				CHAR szStatusA[0x4000] = {0};
				WCHAR szStatusW[0x4000] = {0};

				int nLenTemp = szStatus.GetLength();
				wcsncpy_s(szStatusW, 0x4000, szStatus.GetBuffer(), nLenTemp);
				szStatus.ReleaseBuffer();

				WideCharToMultiByte(CP_ACP, 0, szStatusW, -1, szStatusA, 0x4000, NULL, NULL);
				file.Write(szStatusA, (UINT)strlen(szStatusA));  
				file.Write("\r\n\r\n", (UINT)strlen("\r\n\r\n"));  

				columnData.mask = LVCF_TEXT;
				columnData.cchTextMax = 100;
				columnData.pszText = columnName.GetBuffer(100);

				for(i = 0; pList->GetColumn(i, &columnData); i++)
				{
					strCloumn = strCloumn + columnData.pszText + L"\t\t";
				}

				strCloumn += "\r\n";
				nLenTemp = strCloumn.GetLength();
				wcsncpy_s(szColumnW, 0x4000, strCloumn.GetBuffer(), nLenTemp);
				strCloumn.ReleaseBuffer();
				WideCharToMultiByte(CP_ACP, 0, szColumnW, -1, szColumnA, 0x4000, NULL, NULL);
				file.Write(szColumnA, (UINT)strlen(szColumnA));  

				columnName.ReleaseBuffer();
				columnNum = i;

				for (int nItemIndex = 0; nItemIndex < pList->GetItemCount(); nItemIndex++)
				{
					CHAR szColumnA[0x4000] = {0};
					WCHAR szColumnW[0x4000] = {0};
					CString strItem;

					for( i = 0; i < columnNum; i++)
					{
						strItem = strItem + pList->GetItemText(nItemIndex, i) + L"\t\t";
					}

					strItem += "\r\n";
					nLenTemp = strItem.GetLength();
					wcsncpy_s(szColumnW, 0x4000, strItem.GetBuffer(), nLenTemp);
					strItem.ReleaseBuffer();
					WideCharToMultiByte(CP_ACP, 0, szColumnW, -1, szColumnA, 0x4000, NULL, NULL);
					file.Write(szColumnA, (UINT)strlen(szColumnA));  
				}

				file.Close();
			}
		}
		CATCH_ALL( e )
		{
			file.Abort();   // close file safely and quietly
		//	THROW_LAST();
		}
		END_CATCH_ALL

		if ( PathFileExists(sExcelFile) )
		{
			ShellExecuteW(NULL, L"open", sExcelFile, NULL, NULL, SW_SHOW);
		}
		else
		{
			MessageBox(NULL, L"导出到文本文件失败!", L"导出", MB_OK | MB_ICONERROR);
		}
	}
}

CString CommonFunctions::GetLongPath(CString szPath)
{
	CString strPath;

	if (szPath.Find(L'~') != -1)
	{
		WCHAR szLongPath[MAX_PATH] = {0};
		DWORD nRet = GetLongPathName(szPath, szLongPath, MAX_PATH);
		if (nRet >= MAX_PATH || nRet == 0)
		{
			strPath = szPath;
		}
		else
		{
			strPath = szLongPath;
		}
	}
	else
	{
		strPath = szPath;
	}

	return strPath;
}

CString CommonFunctions::RegParsePath(WCHAR *szData, ULONG dwType)
{
	CString szFilePath;

	if (szData != NULL && wcslen((PWCHAR)szData) > 0)
	{
		set<wstring> AppSet;

		if ( dwType == REG_SZ || dwType == REG_EXPAND_SZ )
		{
			ParseRegistryPathFromType( (PWCHAR)szData, NULL, dwType, AppSet );
		}
		else if ( dwType == REG_MULTI_SZ )
		{
			ParseRegistryPathFromType( NULL,(PWCHAR)szData, dwType, AppSet );
		}

		for( set<wstring>::iterator item=AppSet.begin(); item!=AppSet.end(); item++ )
		{
			if ( wcslen(item->c_str()) > wcslen(SHORT_FILE_NAME) && !PathIsDirectory(item->c_str()) && PathFileExists(item->c_str()))
			{
				szFilePath = item->c_str(); // GetLongPath(item->c_str());
				break;
			}
		}

		if (szFilePath.IsEmpty() && AppSet.size() == 1)
		{
			set<wstring>::iterator item = AppSet.begin();
			szFilePath = item->c_str();
		}

		if (szFilePath.IsEmpty())
		{
			szFilePath = RegParsePath(szData);
		}
	}

	return szFilePath;
}

CString CommonFunctions::RegParsePath(CString szPath)
{
	if (!szPath.IsEmpty())
	{
		CString szTemp = szPath;
		szTemp.MakeUpper();

		int nExe = szTemp.Find(L"RUNDLL32.EXE ");
		if ( nExe != -1 )
		{
			szPath = szPath.Right(szPath.GetLength() - nExe - wcslen(L"RUNDLL32.EXE "));
		}

		if (szPath.Find('/') != -1)
		{
			szPath = szPath.Left(szPath.Find('/') - 1);
		}

		if (szPath.Find(L" -") != -1)
		{
			szPath = szPath.Left(szPath.Find(L" -"));
		}

		if (szPath.Find(',') != -1)
		{
			szPath = szPath.Left(szPath.Find(','));
		}

		szPath.TrimLeft(' ');
		szPath.TrimRight(' ');

		szPath.TrimLeft('"');
		szPath.TrimRight('"');

		WCHAR* szRootTemp = szPath.GetBuffer();
		if (!_wcsnicmp(szRootTemp, L"%SystemRoot%", wcslen(L"%SystemRoot%")))
		{
			WCHAR szSystemDir[MAX_PATH] = {0};
			if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
			{
				CString szTemp = szSystemDir;
				szTemp += (szRootTemp + wcslen(L"%SystemRoot%")); 
				szPath = szTemp;
			}
		}
		else if (!_wcsnicmp(szRootTemp, L"%WinDir%", wcslen(L"%WinDir%")))
		{
			WCHAR szSystemDir[MAX_PATH] = {0};
			if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
			{
				CString szTemp = szSystemDir;
				szTemp += (szRootTemp + wcslen(L"%WinDir%")); 
				szPath = szTemp;
			}
		}
		else if (!_wcsnicmp(szPath.GetBuffer(), L"%ProgramFiles%", wcslen(L"%ProgramFiles%")))
		{
			WCHAR szSystemDir[MAX_PATH] = {0};
			if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
			{
				CString szTemp = szSystemDir;
				szTemp = szTemp.Left(szTemp.Find('\\'));
				szTemp += L"\\Program Files";
				szTemp += (szPath.GetBuffer() + wcslen(L"%ProgramFiles%")); 
				szPath = szTemp;
			}
		}
		// 		else if (szPath.Find('\\') == -1)
		// 		{
		// 			WCHAR szSystemDir[MAX_PATH] = {0};
		// 			if (GetSystemDirectory(szSystemDir, MAX_PATH) != 0)
		// 			{
		// 				CString szTemp = szSystemDir;
		// 				szTemp += L"\\";
		// 				szTemp += szPath;
		// 				szPath = szTemp;
		// 			}
		// 		}

		//	szPath.ReleaseBuffer();

		int nTemp = szPath.ReverseFind('"');
		if (nTemp > 0)
		{
			szPath = szPath.Left(nTemp);
		}

		if (szPath.Find('\\') == -1)
		{
			BOOL bFind = FALSE;
			WCHAR szSystemDir[MAX_PATH] = {0};
			if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
			{
				CString szTemp = szSystemDir;
				szTemp += L"\\" + szPath;
				if (PathFileExists(szTemp))
				{
					szPath = szTemp;
					bFind = TRUE;
				}
			}

			memset(szSystemDir, 0, MAX_PATH * sizeof(WCHAR));

			if (!bFind && GetSystemDirectory(szSystemDir, MAX_PATH))
			{
				CString szTemp = szSystemDir;
				szTemp += L"\\" + szPath;
				if (PathFileExists(szTemp))
				{
					szPath = szTemp;
					bFind = TRUE;
				}
			}
		}
	}

	szPath = GetLongPath(szPath);
	return szPath;
}

VOID CommonFunctions::FindReg(CString szKey, CString szValue)
{
	CJumpToRegistry::JumpToRegistry(szKey, szValue);
}

ULONG CommonFunctions::HexStringToLong(CString szHex)
{
	ULONG nRet = 0;

	if (!szHex.IsEmpty())
	{
		swscanf_s(szHex.GetBuffer(_0X_LEN), L"%x", &nRet);
		szHex.ReleaseBuffer(_0X_LEN);
	}

	return nRet;
}

ULONG CommonFunctions::GetInlineAddress(ULONG pAddress)
{
	ULONG nRet = 0;

	if (pAddress)
	{
		COMMUNICATE_DISPATCH cs;
		cs.OpType = enumGetInlineAddress;
		cs.op.GetInlineAddress.pAddress = pAddress;
		m_Driver.CommunicateDriver(&cs, sizeof(COMMUNICATE_DISPATCH), &nRet, sizeof(ULONG), NULL);
	}

	return nRet;
}

void CommonFunctions::SignVerify(CString szPath)
{
	if (!szPath.IsEmpty() && PathFileExists(szPath))
	{
		CSignVerifyDlg SignVerifyDlg;
		SignVerifyDlg.m_bSingle = TRUE;
		SignVerifyDlg.m_path = szPath;
		SignVerifyDlg.DoModal();
	}
	else
	{
		MessageBox(NULL, szCannotFindFile[g_enumLang], NULL, MB_OK | MB_ICONWARNING);
	}
}

BOOL CommonFunctions::InitUnicodeString(MZF_UNICODE_STRING *unString, WCHAR *szString)
{
	BOOL bRet = FALSE;
	if (!szString)
	{
		unString->Buffer = NULL;
		unString->Length = 0;
		unString->MaximumLength = 2;
		bRet = TRUE;
	}
	else
	{
		ULONG nLen = wcslen(szString);
		if (unString && nLen > 0)
		{
			PWCHAR pBuffer = (PWCHAR)malloc((nLen + 1) * sizeof(WCHAR));
			if (pBuffer)
			{
				memset(pBuffer, 0, (nLen + 1) * sizeof(WCHAR));
				wcscpy_s(pBuffer, nLen + 1, szString);
				unString->Buffer = pBuffer;
				unString->Length = (USHORT)(nLen * sizeof(WCHAR));
				unString->MaximumLength = (USHORT)((nLen + 1) * sizeof(WCHAR));
				bRet = TRUE;
			}
		}
	}

	return bRet;
}

void CommonFunctions::FreeUnicodeString(MZF_UNICODE_STRING *unString)
{
	if (unString && unString->Buffer && unString->Length > 0)
	{
		unString->Length = 0;
		unString->MaximumLength = 0;
		free(unString->Buffer);
		unString->Buffer = NULL;
	}
}

void CommonFunctions::JmpToMyRegistry(CString szKey, CString szData)
{
	if (!szKey.IsEmpty() && g_pRegistryDlg && g_pTab)
	{
		NMHDR nm; 
		nm.hwndFrom = g_pTab->m_hWnd; 
		nm.code = TCN_SELCHANGE;
		g_pTab->SetCurSel(4); 
		g_pTab->SendMessage(WM_NOTIFY, IDC_TAB_MAIN, (LPARAM)&nm);

		((CRegistryDlg*)g_pRegistryDlg)->JmpToReg(szKey, szData);
	}
}

BOOL CommonFunctions::KernelDeleteFile(CString szPath)
{
	BOOL bOK = FALSE;

	if (szPath.IsEmpty() || !PathFileExists(szPath))
	{
		return bOK;
	}

	WCHAR szFilePath[MAX_PATH+1] = {0};
	wcsncpy_s(szFilePath, MAX_PATH, szPath.GetBuffer(), szPath.GetLength());
	szPath.ReleaseBuffer();

	COMMUNICATE_FILE cf;
	cf.OpType = enumDeleteFile;
	cf.op.Delete.szPath = szFilePath;
	bOK = m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), NULL, 0, NULL);

	return bOK;
}

void CommonFunctions::Disassemble(DWORD dwAddress, DWORD dwSize, DWORD dwPid, DWORD dwEprocess)
{
	CAssemblyDlg AssmDlg;
	AssmDlg.m_dwPid = dwPid;
	AssmDlg.m_dwEprocess = dwEprocess;
	AssmDlg.m_dwAddress = dwAddress;
	AssmDlg.m_dwSize = dwSize;
	AssmDlg.DoModal();
}

CString CommonFunctions::DisassembleString(DWORD dwAddress, DWORD dwSize, DWORD dwPid, DWORD dwEprocess)
{
	CAssemblyDlg AssmDlg;
	AssmDlg.m_dwPid = dwPid;
	AssmDlg.m_dwEprocess = dwEprocess;
	AssmDlg.m_dwAddress = dwAddress;
	AssmDlg.m_dwSize = dwSize;
	return AssmDlg.DisasmMemory(dwAddress, dwSize, dwPid, dwEprocess);
}

//WCHAR转TCHAR
// CString CommonFunctions::ws2ts(LPWSTR szSrc, int cbMultiChar=-1)
// {
// 	mystring strDst;
// 	if ( szSrc==NULL || cbMultiChar==0 ){
// 		return strDst;
// 	}
// #ifdef _UNICODE
// 	strDst.assign(szSrc);
// #else
// 	TCHAR*pBuff=NULL;
// 	int nLen=WideCharToMultiByte(CP_ACP,NULL,szSrc,cbMultiChar,NULL,0,NULL,FALSE);
// 	if ( nLen>0 ){
// 		pBuff=new TCHAR[nLen+1];
// 		if ( pBuff ){
// 			WideCharToMultiByte(CP_ACP,NULL,szSrc,cbMultiChar,pBuff,nLen,NULL,FALSE);
// 			pBuff[nLen]=0;
// 			strDst.assign(pBuff);
// 			delete[] pBuff;
// 		}
// 	}
// #endif
// 
// 	return strDst;
// }


//多字节字符串转WCHAR字符串
CString CommonFunctions::ms2ws(LPCSTR szSrc, int cbMultiChar)
{
	CString strDst;
	if ( szSrc==NULL || cbMultiChar==0 ){
		return strDst;
	}

	WCHAR*pBuff=NULL;
	int nLen=MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,NULL,0);
	if ( nLen>0 ){
		pBuff=new WCHAR[nLen+1];
		memset(pBuff, 0, (nLen+1)*sizeof(WCHAR));
		if ( pBuff ){
			MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,pBuff,nLen);
			pBuff[nLen]=0;
			strDst = pBuff;
			delete[] pBuff;
		}
	}

	return strDst;
}

void CommonFunctions::ExportAutoRunValue(CString szKey, CString szPath, CString szValue)
{
	if (!szKey.IsEmpty() && g_pRegistryDlg && g_pTab)
	{
		((CRegistryDlg*)g_pRegistryDlg)->ExportValue(szKey, szValue, szPath);
	}
}

void CommonFunctions::HexEdit(DWORD dwAddress, DWORD dwSize, DWORD dwPid, DWORD dwEprocess)
{
	CMemoryHexEditDlg EditDlg;
	EditDlg.m_dwPid = dwPid;
	EditDlg.m_dwEprocess = dwEprocess;
	EditDlg.m_dwAddress = dwAddress;
	EditDlg.m_dwSize = dwSize;
	EditDlg.DoModal();
}

void CommonFunctions::JmpToService(CString szService)
{
	if (!szService.IsEmpty() && g_pServiceDlg && g_pTab)
	{
		NMHDR nm; 
		nm.hwndFrom = g_pTab->m_hWnd; 
		nm.code = TCN_SELCHANGE;
		g_pTab->SetCurSel(6); 
		g_pTab->SendMessage(WM_NOTIFY, IDC_TAB_MAIN, (LPARAM)&nm);

		((CServiceDlg*)g_pServiceDlg)->JmpToService(szService);
	}
}

void CommonFunctions::JmpToFile(CString szFilePath)
{
	if (!PathFileExists(szFilePath))
	{
		CString szMsgBox; 
		szMsgBox = szRegistryIsNoExsit[g_enumLang];
		szMsgBox += L" \'" + szFilePath + L"\'"; 
		::MessageBox(NULL, szMsgBox, L"AntiSpy", MB_OK | MB_ICONINFORMATION);
		return;
	}

	CString szPath = szFilePath.Left(szFilePath.ReverseFind('\\'));
	CString szFile = szFilePath.Right(szFilePath.GetLength() - szFilePath.ReverseFind('\\') - 1);

	if (!szPath.IsEmpty() && g_pFileDlg && g_pTab)
	{
		NMHDR nm; 
		nm.hwndFrom = g_pTab->m_hWnd; 
		nm.code = TCN_SELCHANGE;
		g_pTab->SetCurSel(5); 
		g_pTab->SendMessage(WM_NOTIFY, IDC_TAB_MAIN, (LPARAM)&nm);

		((CFileDlg*)g_pFileDlg)->GotoPath(szPath, szFile);
	}
}

DWORD CommonFunctions::GetChecksum(PVOID pBase, DWORD FileSize)
{
	DWORD	checksum,dwhold,dwdata;
	DWORD64 dwtemp64;
	UCHAR	_temp;

	if (pBase == NULL)
	{
		return 0;
	}
	checksum = 0;
	dwhold = 0xde20ad12;

	for(DWORD i = 0; i< FileSize; i++)
	{
		CopyMemory(&_temp, (BYTE *)pBase + i, 1);
		dwtemp64 = _temp * dwhold;
		dwdata = DWORD(dwtemp64);
		dwtemp64 = dwtemp64 >> 32;
		dwhold = DWORD(dwtemp64);
		checksum = checksum + dwdata;
		dwhold++;
	}

	return checksum;
}

void CommonFunctions::DecryptResource(PVOID pBuffer, DWORD dwSize)
{
// 	DWORD dwKey = 0;
// 	char sz32Key[7] = {'\x55','\x8b','\xec','\x33','\xc0','\x50','\xc3'};
// 	dwKey = GetChecksum(sz32Key, 7);
// 
// 	unsigned char *pFileBuffer = (unsigned char *)pBuffer;
// 	DWORD dwCount = dwSize >> 2;
// 	DWORD dwKeyModi = dwKey;
// 	DWORD dwKey16 = 0;
// 	DWORD dwRawContent = 0;
// 	DWORD dwEncryptContent = 0;
// 	for (DWORD i = 0; i < dwCount; i++)
// 	{
// 		DWORD dwContent = *(DWORD *)(pFileBuffer + i * 4);
// 		dwRawContent = dwContent;
// 		dwEncryptContent = dwContent ^ dwKeyModi;
// 		dwKeyModi = dwRawContent;
// 		if (i > 0 && i % 15 == 0)
// 		{
// 			dwKeyModi = dwKey16;
// 		}
// 		else
// 		{
// 			dwKeyModi = dwRawContent;
// 		}
// 
// 		if (i % 15 == 0)
// 		{
// 			dwKey16 = dwRawContent;
// 		}
// 
// 		*(DWORD *)(pFileBuffer + i * 4) = dwEncryptContent;
// 	}
// 
// 	DWORD dwLeft = dwSize % 4;
// 	unsigned char *pLeftBuffer = pFileBuffer + dwSize - dwLeft;
// 	for (DWORD i = 0; i < dwLeft; i++)
// 	{
// 		char cTmp = *(pFileBuffer + i);
// 		char cKey = *((unsigned char *)(&dwKeyModi) + i);
// 		cTmp = cTmp ^ cKey;
// 		*(pFileBuffer + i) = cTmp;
// 	}

// 	DWORD dwKey = 0;
// 	char sz32Key[7] = {'\x55','\x8b','\xec','\x33','\xc0','\x50','\xc3'};
// 	dwKey = GetChecksum(sz32Key, 7);
// 
// 	unsigned char *pFileBuffer = (unsigned char *)pBuffer;
// 	DWORD dwCount = dwSize >> 2;
// 	for (DWORD i = 0; i < dwCount; i++)
// 	{
// 		DWORD dwContent = *(DWORD *)(pFileBuffer + i * 4);
// 		DWORD dwEncryptContent = dwContent ^ dwKey;
// 		*(DWORD *)(pFileBuffer + i * 4) = dwEncryptContent;
// 	}

// 	char sz32Key[7] = {'\x55','\x8b','\xec','\x33','\xc0','\x50','\xc3'};
// 	DWORD dwKey = GetChecksum(sz32Key, 7);
// 
// 	unsigned char *pFileBuffer = (unsigned char *)pBuffer;
// 	DWORD dwCount = dwSize >> 2;
// 	DWORD dwRawContent = 0;
// 	for (DWORD i = 0; i < dwCount; i++)
// 	{
// 		DWORD dwContent = *(DWORD*)(pFileBuffer + i * 4);
// 
// 		if (dwContent == 0 || dwContent == dwKey)
// 		{
// 			continue;
// 		}
// 
// 		DWORD dwEncryptContent = dwContent ^ dwKey;
// 		*(DWORD*)(pFileBuffer + i * 4) = dwEncryptContent;
// 
// 		dwKey = dwEncryptContent;
// 	}

	BYTE btKey = 'm';
	BYTE *pFileBuffer = (BYTE *)pBuffer;

	for (DWORD i = 0; i < dwSize; i++)
	{
		BYTE dwContent = *(BYTE*)(pFileBuffer + i);

		if (dwContent == 0 || dwContent == btKey)
		{
			continue;
		}

		BYTE dwEncryptContent = dwContent ^ btKey;
		*(BYTE*)(pFileBuffer + i) = dwEncryptContent;

		btKey = dwEncryptContent;
	}
}

CString CommonFunctions::ReleaseResource(DWORD dwRid, WCHAR *szType)
{
	CString szRet = L"";
	
	if (dwRid == 0 || szType == NULL)
	{
		return szRet;
	}

	WCHAR szTempPath[MAX_PATH] = {0};
	GetTempPath(MAX_PATH, szTempPath);

	WCHAR szTempFile[MAX_PATH] = {0};
	GetTempFileName(szTempPath, NULL, 0, szTempFile);

// 	if (PathFileExists(szTempFile))
// 	{
// 		DeleteFile(szTempFile);
// 	}

	CString szPath = szTempFile;
	HANDLE hFile = CreateFile(szPath, 
		GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
		);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DebugLog(L"CreateFile: %s success", szPath);

		DWORD dwAttr = GetFileAttributes(szPath);
		SetFileAttributes(szPath, dwAttr | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

		HRSRC hRsc = FindResource(NULL, MAKEINTRESOURCE(dwRid), szType);
		if (hRsc)
		{
			DWORD dwResSize = SizeofResource(NULL, hRsc);
			if(dwResSize > 0)
			{
				HGLOBAL hResData = LoadResource(NULL, hRsc);
				if(hResData != NULL)
				{
					LPVOID lpResourceData = LockResource(hResData);
					if(lpResourceData != NULL)
					{
						PVOID pBuffer = malloc(dwResSize);
						if (pBuffer)
						{
							memset(pBuffer, 0, dwResSize);
							memcpy(pBuffer, lpResourceData, dwResSize);

							DecryptResource(pBuffer, dwResSize);

							DWORD dwRet;
							if (WriteFile(hFile, pBuffer, dwResSize, &dwRet, NULL))
							{
								szRet = szPath;
							}

							free(pBuffer);
						}

						FreeResource(hResData);
					}
				}
			}
		}

		CloseHandle(hFile);
	}

	return szRet;
}

BOOL CommonFunctions::EnablePrivilege(WCHAR *szPrivilege, BOOL bEnable) 
{
	BOOL bOk = FALSE; 
	HANDLE hToken;

	if (OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) 
	{
		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bOk = (GetLastError() == ERROR_SUCCESS);
		::CloseHandle(hToken);
	}

	return bOk;
}

WCHAR *CommonFunctions::CString2WString(CString szString)
{
	WCHAR *szRet = NULL;
	if (szString.IsEmpty())
	{
		return szRet;
	}

	DWORD dwLen = (szString.GetLength() + 1) * sizeof(WCHAR);
	szRet = (WCHAR*)malloc(dwLen);
	if (!szRet)
	{
		return szRet;
	}

	memset(szRet, 0, dwLen);

	wcsncpy_s(szRet, szString.GetLength() + 1, szString.GetBuffer(), szString.GetLength());
	szString.ReleaseBuffer();

	return szRet;
}

PVOID GetMemory(ULONG nSize)
{
	PVOID pRet = NULL;

	if (nSize <= 0)
	{
		return pRet;
	}

	pRet = malloc(nSize);
	if (pRet == NULL)
	{
		return pRet;
	}

	memset(pRet, 0, nSize);

	return pRet;
}

CString GetLongPath(CString szPath)
{
	CString strPath;

	if (szPath.Find(L'~') != -1)
	{
		WCHAR szLongPath[MAX_PATH] = {0};
		DWORD nRet = GetLongPathName(szPath, szLongPath, MAX_PATH);
		if (nRet >= MAX_PATH || nRet == 0)
		{
			strPath = szPath;
		}
		else
		{
			strPath = szLongPath;
		}
	}
	else
	{
		strPath = szPath;
	}

	return strPath;
}

CString TrimPath(WCHAR * szPath)
{
	CString strPath;

	if (szPath[1] == ':' && szPath[2] == '\\')
	{
		strPath = szPath;
	}
	else if (wcslen(szPath) > wcslen(L"\\SystemRoot\\") && 
		!_wcsnicmp(szPath, L"\\SystemRoot\\", wcslen(L"\\SystemRoot\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, szPath + wcslen(L"\\SystemRoot\\"));
	}
	else if (wcslen(szPath) > wcslen(L"system32\\") && 
		!_wcsnicmp(szPath, L"system32\\", wcslen(L"system32\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, szPath/* + wcslen(L"system32\\")*/);
	}
	else if (wcslen(szPath) > wcslen(L"\\??\\") &&
		!_wcsnicmp(szPath, L"\\??\\", wcslen(L"\\??\\")))
	{
		strPath = szPath + wcslen(L"\\??\\");
	}
	else if (wcslen(szPath) > wcslen(L"%ProgramFiles%") &&
		!_wcsnicmp(szPath, L"%ProgramFiles%", wcslen(L"%ProgramFiles%")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
		{
			CString szTemp = szSystemDir;
			szTemp = szTemp.Left(szTemp.Find('\\'));
			szTemp += L"\\Program Files";
			szTemp += szPath + wcslen(L"%ProgramFiles%"); 
			strPath = szTemp;
		}
	}
	else
	{
		strPath = szPath;
	}

	strPath = GetLongPath(strPath);

	return strPath;
}

WCHAR *CString2WString(CString szString)
{
	WCHAR *szRet = NULL;
	if (szString.IsEmpty())
	{
		return szRet;
	}

	DWORD dwLen = (szString.GetLength() + 1) * sizeof(WCHAR);
	szRet = (WCHAR*)malloc(dwLen);
	if (!szRet)
	{
		return szRet;
	}

	memset(szRet, 0, dwLen);

	wcsncpy_s(szRet, szString.GetLength() + 1, szString.GetBuffer(), szString.GetLength());
	szString.ReleaseBuffer();

	return szRet;
}

void FixPeDump(PVOID pBuffer)
{
	if (!pBuffer)
	{
		return;
	}

	PIMAGE_DOS_HEADER       PEDosHeader;
	PIMAGE_FILE_HEADER      PEFileHeader;
	PIMAGE_OPTIONAL_HEADER  PEOptionalHeader;
	PIMAGE_SECTION_HEADER   PESectionHeaders;
	PULONG                  PEMagic;

	__try
	{
		PEDosHeader      = (PIMAGE_DOS_HEADER) pBuffer;
		PEMagic          = (PULONG)                 ( (ULONG)pBuffer + PEDosHeader->e_lfanew );
		PEFileHeader     = (PIMAGE_FILE_HEADER)     ( (ULONG)pBuffer + PEDosHeader->e_lfanew + sizeof(ULONG) );
		PEOptionalHeader = (PIMAGE_OPTIONAL_HEADER) ( (ULONG)pBuffer + PEDosHeader->e_lfanew + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) );
		PESectionHeaders = (PIMAGE_SECTION_HEADER)  ( (ULONG)pBuffer + PEDosHeader->e_lfanew + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER));

		if (PEFileHeader->NumberOfSections > 0)
		{
			ULONG SectionAlignment = PEOptionalHeader->SectionAlignment;
			ULONG VirtualAddress = PESectionHeaders->VirtualAddress;

			for (ULONG i = 0; i < PEFileHeader->NumberOfSections; i++)
			{
				ULONG VirtualSize = PESectionHeaders[i].Misc.VirtualSize;
				ULONG DivVirtualSize = VirtualSize % SectionAlignment;

				if ( DivVirtualSize )
				{
					VirtualSize = SectionAlignment + VirtualSize - DivVirtualSize;
				}

				PESectionHeaders[i].SizeOfRawData = VirtualSize;
				PESectionHeaders[i].PointerToRawData = VirtualAddress;

				VirtualAddress += VirtualSize;
			}
		}
	}
	__except(1)
	{

	}
}

CString PathGetImageName(CString szPath)
{
	CString szImage = L"";
	
	if (szPath.IsEmpty())
	{
		return szImage;
	}
	
	szImage = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);

	return szImage;
}