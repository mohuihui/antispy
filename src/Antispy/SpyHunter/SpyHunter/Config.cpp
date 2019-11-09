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
#include "Config.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DESCRIBE				";This is a configuration file for AntiSpy.\r\n" \
								";The name of this file should be in accordance with that of AntiSpy which you may have renamed otherwise.\r\n" \
								";For example, if AntiSpy is named \"AntiVirus.exe\", you must rename the config file as \"AntiVirus.dat\".\r\n" \
								";Written by Mzf, Mar 26,2013.\r\n\r\n"

#define PROCESS_CONFIG			";---------------------------------------- Process ------------------------------------\r\n\r\n"

#define GENERAL_CONFIG			";---------------------------------------- General ------------------------------------\r\n\r\n"

#define REGISTRY_CONFIG			";----------------------------- Quick position of registry ----------------------------\r\n\r\n"

#define COLOR_CONFIG			";----------------------------------------- Colors -------------------------------------\r\n\r\n"

#define HOTKEY_CONFIG			";---------------------------------------- Hot Keys -----------------------------------\r\n\r\n"

#define TOPMOST					";This field determines whether the window of AntiSpy is topmost or not.\r\n" \
								";Zero means the window is non-topmost, elsewise it is set to be topmost.\r\n"

#define TITLE_NAME				";This field determines the title name of the mainly window of AntiSpy.\r\n"

#define REG_PATH				";These fields will be added to quick positioning of registry tab.\r\n"

#define RANDOM_TITILE			";This field determines whether the title name is a random string.\r\n" \
								";Zero means the title name as TitleName field, elsewise it is a random string.\r\n"

#define AUTODETECT_NEW_VERSION	";This field determines whether automatic detect new version online when AntiSpy startup.\r\n" 

#define SHOW_MAIN_WINDOW		";If you set the \'A\', means the hot key is Ctrl + Alt + Shift + A.\r\n"
								/*";This field determines what's the hot key for showing AntiSpy main window.\r\n" \
								";If you set the \'A\', means the hot key is Ctrl + Alt + Shift + A.\r\n"*/

#define SHOW_MODULES_DLG		";This field determines what's the hot key for showing process modules dialog.\r\n" \
								";If you set the \'A\', means the hot key is Shift + A.\r\n"

#define SHOW_THREADS_DLG		";This field determines what's the hot key for showing process threads dialog.\r\n"

#define SHOW_HANDLES_DLG		";This field determines what's the hot key for showing process handles dialog.\r\n"

#define SHOW_WINDOWS_DLG		";This field determines what's the hot key for showing process windows dialog.\r\n"

#define SHOW_MEMORY_DLG			";This field determines what's the hot key for showing process memory dialog.\r\n"

#define SHOW_HOTKEYS_DLG		";This field determines what's the hot key for showing process hotkeys dialog.\r\n"

#define SHOW_PRIVILEGES_DLG		";This field determines what's the hot key for showing process privileges dialog.\r\n"

#define SHOW_TIMERS_DLG			";This field determines what's the hot key for showing process timers dialog.\r\n"

#define SHOW_PROCESS_INFO		";This field determines whether automatic show process detail informations when the mouse over a process item.\r\n" \
								";Zero means it will not display, elsewise it will display the informations as a small window.\r\n"

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define STAYONTOP					L"StayOnTop"
#define SELFDEFENSE					L"SelfDefense"
#define TITLENAME					L"TitleName"
#define REGPATH						L"RegPath"
#define RANDOMTITLE					L"RandomTitle"
#define AUTODETECTNEWVERSION		L"AutoDetectNewVersion"
#define SHOWMAINWINDOWHOTKEY		L"ShowMainWindowHotKey"
#define SHOWMODULESHOTKEY			L"ShowModulesHotKey"
#define SHOWTHREADSHOTKEY			L"ShowThreadsHotKey"
#define SHOWWINDOWSHOTKEY			L"ShowWindowsHotKey"
#define SHOWHANDLESHOTKEY			L"ShowHandlesHotKey"
#define SHOWMEMORYHOTKEY			L"ShowMemoryHotKey"
#define SHOWHOTKEYSHOTKEY			L"ShowHotkeysHotKey"
#define SHOWPRIVILEGESHOTKEY		L"ShowPrivilegesHotKey"
#define SHOWTIMERSHOTKEY			L"ShowTimersHotKey"
#define SHOWPROCESSDETAILINFO		L"ShowProcessDetailInformations"
#define SHOWLOWERPANE				L"ShowLowerPaneAsDeafult"
#define SHOWPROCESSTREE				L"ShowProcessTreeAsDefault"
#define MICROSOFTPROCCOLOR			L"MicrosoftProcessColor"
#define NORMALPROCCOLOR				L"NormalProcessColor"
#define HIDEPROCCOLOR				L"HiddenProcessColor"
#define HAVEMODULEPROCCOLOR			L"HaveOtherModulesProcessColor"
#define NORMALFILECOLOR				L"NormalFileColor"
#define HIDDENFILECOLOR				L"HiddenFileColor"
#define SYSTEMFILECOLOR				L"SystemFileColor"
#define SYSTEMANDHIDDENFILECOLOR	L"SystemAndHiddenColor" 	 
#define NORMALMODULECOLOR			L"NormalModuleColor"
#define MICROSOFTMODULECOLOR		L"MicrosoftModuleColor"
#define HIDDENORHOOKCOLOR			L"HiddenObjectsOrHooksColor"
#define NOSIGNATUREMODULECOLOR		L"NoSignatureModuleColor"
#define USEHOTKEYS					L"EnableHotKeys"

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
	m_szTitle = L"";
	m_RegPathList.clear();
	m_bStayOnTop = FALSE;
	m_bRandomTitle = FALSE;
	m_bSlefProtection = FALSE;
	m_bAutoDetectNewVersion = TRUE;
	m_bShowProcessDetailInfo = TRUE;
	m_chHotKeyForMainWindow = 'A';
	m_bShowLowerPane = FALSE;
	m_bShowProcessTree = FALSE;

	m_clrHideProc = RGB( 255, 0, 0 );					// 隐藏进程默认是 红色
	m_clrMicrosoftProc = RGB(0, 0, 0);					// 微软进程默认是 黑色
	m_clrNormalProc = RGB( 0, 0, 255 );					// 正常的进程默认是 蓝色
	m_clrMicroHaveOtherModuleProc = RGB( 255, 130, 0 );	// 微软带有非微软模块默认是 橘黄色

	m_clrNormalFile = RGB(0, 0, 0);						// 正常文件 黑色
	m_clrHideFile = RGB( 127, 127, 127 );				// 隐藏文件 灰色
	m_clrSystemAndHideFile = RGB( 255, 0, 0 );			// 隐藏&系统 红色
	m_clrSystemFile = RGB( 255, 0, 0 );					// 系统文件	红色

	m_NormalItemClr = RGB( 0, 0, 255 );				// 非微软的文件默认显示蓝色
	m_HiddenOrHookItemClr = RGB( 255, 0, 0 );		// 隐藏或者钩子默认显示红色
	m_MicrosoftItemClr = RGB(0, 0, 0);				// 微软模块默认显示黑色
	m_NotSignedItemClr = RGB( 180, 0, 255 );		// 未签名项默认显示紫色

	m_bUseHotKeys = FALSE;
}

CConfig::~CConfig()
{
	m_RegPathList.clear();
}

CString CConfig::GetConfigFilePath()
{
	CString szRet;
	WCHAR szExePath[MAX_PATH] = {0};

	GetModuleFileName(NULL, szExePath, MAX_PATH);
	if (wcslen(szExePath))
	{
		WCHAR *szTemp = wcsrchr(szExePath, '.');
		if (szTemp)
		{
			*szTemp = '\0';

			szRet = szExePath;
			szRet += L".dat";
		}
	}

	return szRet;
}

void CConfig::WriteConfig()
{
	CString szPath = GetConfigFilePath();
	
	if (szPath.IsEmpty())
	{
		return;
	}

	char szTemp[MAX_PATH] = {0};
	CFile fileWrite; //(szPath, CFile::modeCreate | CFile::modeWrite);

	TRY
	{
		// 打开文件
		if (!fileWrite.Open( szPath, CFile::modeCreate | CFile::modeWrite ))
		{
			return;
		}

		// 首先写最上面的描述文字
		fileWrite.Write(DESCRIBE, strlen(DESCRIBE));
		
		// 写通用设置文字
		fileWrite.Write(GENERAL_CONFIG, strlen(GENERAL_CONFIG));

		// 是否自动联网检测新版本
	//	fileWrite.Write(AUTODETECT_NEW_VERSION, strlen(AUTODETECT_NEW_VERSION));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "AutoDetectNewVersion = ", m_bAutoDetectNewVersion);
		fileWrite.Write(szTemp, strlen(szTemp));

	//	fileWrite.Write(SELFDEFENSE, strlen(SELFDEFENSE));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "SelfDefense = ", m_bSlefProtection);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 是否总在最前
	//	fileWrite.Write(TOPMOST, strlen(TOPMOST));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "StayOnTop = ", m_bStayOnTop);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 是否是随机名字标题
	//	fileWrite.Write(RANDOM_TITILE, strlen(RANDOM_TITILE));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "RandomTitle = ", m_bRandomTitle);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 写标题
		fileWrite.Write(TITLE_NAME, strlen(TITLE_NAME));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%ws\r\n\r\n", "TitleName = ", m_szTitle);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 写进程tab设置文字
		fileWrite.Write(PROCESS_CONFIG, strlen(PROCESS_CONFIG));
		
		// 是否飘窗显示进程详细信息
	//	fileWrite.Write(SHOW_PROCESS_INFO, strlen(SHOW_PROCESS_INFO));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "ShowProcessDetailInformations = ", m_bShowProcessDetailInfo);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 是否默认显示下方模块列表
	//	fileWrite.Write(";\r\n", strlen(";\r\n"));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "ShowLowerPaneAsDeafult = ", m_bShowLowerPane);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 是否默认显示进程树
	//	fileWrite.Write(";\r\n", strlen(";\r\n"));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "ShowProcessTreeAsDefault = ", m_bShowProcessTree);
		fileWrite.Write(szTemp, strlen(szTemp));

		fileWrite.Write(HOTKEY_CONFIG, strlen(HOTKEY_CONFIG));
		
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%d\r\n\r\n", "EnableHotKeys = ", m_bUseHotKeys);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 显示主窗口的热键
		fileWrite.Write(SHOW_MAIN_WINDOW, strlen(SHOW_MAIN_WINDOW));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s%c\r\n\r\n", "ShowMainWindowHotKey = ", m_chHotKeyForMainWindow);
		fileWrite.Write(szTemp, strlen(szTemp));

		// 写颜色配置文字
	// 	fileWrite.Write("\r\n", strlen("\r\n"));
		fileWrite.Write(COLOR_CONFIG, strlen(COLOR_CONFIG));

		// 微软进程颜色
		fileWrite.Write(";Process colors\r\n", strlen(";Process colors\r\n"));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"MicrosoftProcessColor = ", 
			GetRValue(m_clrMicrosoftProc),
			GetGValue(m_clrMicrosoftProc),
			GetBValue(m_clrMicrosoftProc));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 正常进程颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"NormalProcessColor = ", 
			GetRValue(m_clrNormalProc),
			GetGValue(m_clrNormalProc),
			GetBValue(m_clrNormalProc));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 隐藏进程颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"HiddenProcessColor = ", 
			GetRValue(m_clrHideProc),
			GetGValue(m_clrHideProc),
			GetBValue(m_clrHideProc));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 微软进程带有非微软模块颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"HaveOtherModulesProcessColor = ", 
			GetRValue(m_clrMicroHaveOtherModuleProc),
			GetGValue(m_clrMicroHaveOtherModuleProc),
			GetBValue(m_clrMicroHaveOtherModuleProc));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 正常文件颜色
		fileWrite.Write(";File colors\r\n", strlen(";File colors\r\n"));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"NormalFileColor = ", 
			GetRValue(m_clrNormalFile),
			GetGValue(m_clrNormalFile),
			GetBValue(m_clrNormalFile));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 隐藏文件颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"HiddenFileColor = ", 
			GetRValue(m_clrHideFile),
			GetGValue(m_clrHideFile),
			GetBValue(m_clrHideFile));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 系统文件颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"SystemFileColor = ", 
			GetRValue(m_clrSystemFile),
			GetGValue(m_clrSystemFile),
			GetBValue(m_clrSystemFile));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 系统 & 隐藏文件颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"SystemAndHiddenColor = ", 
			GetRValue(m_clrSystemAndHideFile),
			GetGValue(m_clrSystemAndHideFile),
			GetBValue(m_clrSystemAndHideFile));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 通用颜色,非微软模块颜色
		fileWrite.Write(";General colors\r\n", strlen(";General colors\r\n"));
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"NormalModuleColor = ", 
			GetRValue(m_NormalItemClr),
			GetGValue(m_NormalItemClr),
			GetBValue(m_NormalItemClr));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 微软模块颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"MicrosoftModuleColor = ", 
			GetRValue(m_MicrosoftItemClr),
			GetGValue(m_MicrosoftItemClr),
			GetBValue(m_MicrosoftItemClr));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 隐藏模块,隐藏对象,hooks颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"HiddenObjectsOrHooksColor = ", 
			GetRValue(m_HiddenOrHookItemClr),
			GetGValue(m_HiddenOrHookItemClr),
			GetBValue(m_HiddenOrHookItemClr));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 微软进程带有非微软模块颜色
		memset(szTemp, 0, MAX_PATH);
		sprintf_s(szTemp, MAX_PATH, "%s#%02x%02x%02x\r\n\r\n", 
			"NoSignatureModuleColor = ", 
			GetRValue(m_NotSignedItemClr),
			GetGValue(m_NotSignedItemClr),
			GetBValue(m_NotSignedItemClr));
		fileWrite.Write(szTemp, strlen(szTemp));

		// 写注册表快速定位的文字
		fileWrite.Write(REGISTRY_CONFIG, strlen(REGISTRY_CONFIG));

		// 注册表快速定位项
		fileWrite.Write(REG_PATH, strlen(REG_PATH));
		for (std::list<CString>::iterator itor = m_RegPathList.begin();
			itor != m_RegPathList.end();
			itor++)
		{
			char szTemp[4096] = {0};
			sprintf_s(szTemp, 4096, "%s%ws\r\n", "RegPath = ", *itor);
			fileWrite.Write(szTemp, strlen(szTemp));
		}

		// 最后关闭文件
		fileWrite.Close();
	}
	CATCH_ALL( e )
	{
		fileWrite.Abort();   // close file safely and quietly
		//THROW_LAST();
	}
	END_CATCH_ALL
}

void CConfig::InitRegPathList()
{
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\IPSec\\Policy\\Local");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace");
}

void CConfig::EraseSpace(CString &szString)
{
	if (szString.IsEmpty())
	{
		return;
	}

	while (szString.GetAt(0) == ' ')
	{
		szString.TrimLeft(' ');
	}
}

CString CConfig::ms2ws(LPCSTR szSrc, int cbMultiChar)
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

// WCHAR转TCHAR
CStringA CConfig::ws2ms(LPWSTR szSrc, int cbMultiChar)
{
	CStringA strDst;
	if ( szSrc==NULL || cbMultiChar==0 ){
		return strDst;
	}

	CHAR*pBuff=NULL;
	int nLen=WideCharToMultiByte(CP_ACP,NULL,szSrc,cbMultiChar,NULL,0,NULL,FALSE);
	if ( nLen>0 ){
		pBuff=new CHAR[nLen+1];
		if ( pBuff ){
			WideCharToMultiByte(CP_ACP,NULL,szSrc,cbMultiChar,pBuff,nLen,NULL,FALSE);
			pBuff[nLen]=0;
			strDst = pBuff;
			delete[] pBuff;
		}
	}

	return strDst;
}

void CConfig::ReadConfig()
{
	CString szPath = GetConfigFilePath();

	if (szPath.IsEmpty() || !PathFileExists(szPath))
	{
		InitRegPathList();
		return;
	}

	// 打开文件
	CStdioFile file;
	TRY 
	{
		if (!file.Open(szPath, CFile::modeRead))
		{
			InitRegPathList();
			return;
		}

		// 开始读文件
		CString strText = _T("");
		while(file.ReadString(strText)) // 一行一行的读取
		{
			EraseSpace(strText);

			if (strText.IsEmpty() ||
				strText.GetAt(0) == ';')
			{
				continue;
			}

			CString szTemp;
			if (strText.Find(SHOWPROCESSDETAILINFO) != -1) // 显示进程详细信息
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bShowProcessDetailInfo = _wtoi(szTemp);
				}
			}
			else if (strText.Find(SHOWLOWERPANE) != -1) // 是否默认显示下方模块列表
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bShowLowerPane = _wtoi(szTemp);
				}
			}
			else if (strText.Find(SHOWPROCESSTREE) != -1) // 是否默认显示进程树
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bShowProcessTree = _wtoi(szTemp);
				}
			}
			else if (strText.Find(USEHOTKEYS) != -1) // 是否开启快捷键
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bUseHotKeys = _wtoi(szTemp);
				}
			}
			else if (strText.Find(STAYONTOP) != -1) // 首先读是否总在最前
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bStayOnTop = _wtoi(szTemp);
				}
			}
			else if (strText.Find(TITLENAME) != -1) // 读软件标题
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_szTitle = szTemp;
				}
			}
			else if (strText.Find(RANDOMTITLE) != -1)	// 是否随机命名软件标题
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bRandomTitle = _wtoi(szTemp);
				}
			}
			else if (strText.Find(SELFDEFENSE) != -1)	// 是否开启自保
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bSlefProtection = _wtoi(szTemp);
				}
			}
			else if (strText.Find(AUTODETECTNEWVERSION) != -1)	// 是否随机命名软件标题
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_bAutoDetectNewVersion = _wtoi(szTemp);
				}
			}
			else if (strText.Find(SHOWMAINWINDOWHOTKEY) != -1)	// 显示主窗口的热键
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_chHotKeyForMainWindow = GetFirstChar(szTemp);
				}
			}
			else if (strText.Find(REGPATH) != -1)	// 读注册表快速定位
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_RegPathList.push_back(szTemp);
				}
			}
			else if (strText.Find(MICROSOFTPROCCOLOR) != -1) // 微软进程的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrMicrosoftProc = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(NORMALPROCCOLOR) != -1) // 正常进程的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrNormalProc = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(HIDEPROCCOLOR) != -1) // 隐藏进程的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrHideProc = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(HAVEMODULEPROCCOLOR) != -1) // 微软进程带有其他模块进程的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrMicroHaveOtherModuleProc = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(NORMALFILECOLOR) != -1) // 正常文件的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrNormalFile = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(HIDDENFILECOLOR) != -1) // 隐藏文件的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrHideFile = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(SYSTEMFILECOLOR) != -1) // 系统文件的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrSystemFile = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(SYSTEMANDHIDDENFILECOLOR) != -1) // 系统并且隐藏文件的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_clrSystemAndHideFile = CString2_RGB(szTemp);
				}
			}		
			else if (strText.Find(NORMALMODULECOLOR) != -1) // 正常模块的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_NormalItemClr = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(MICROSOFTMODULECOLOR) != -1) // 微软模块的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_MicrosoftItemClr = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(HIDDENORHOOKCOLOR) != -1) // 隐藏或者hook的颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_HiddenOrHookItemClr = CString2_RGB(szTemp);
				}
			}
			else if (strText.Find(NOSIGNATUREMODULECOLOR) != -1) // 未签名的模块颜色
			{
				if (strText.Find('=') != -1)
				{
					szTemp = strText.Right(strText.GetLength() - strText.Find('=') - 1);
					EraseSpace(szTemp);
					m_NotSignedItemClr = CString2_RGB(szTemp);
				}
			}
		}  

		//关闭文件
		file.Close();
	}
	CATCH_ALL( e )
	{
		file.Abort();   // close file safely and quietly
		//THROW_LAST();
	}
	END_CATCH_ALL
}

void CConfig::AddRegPath(CString szRegPath)
{
	if (szRegPath.IsEmpty())
	{
		return;
	}

	m_RegPathList.push_back(szRegPath);
}

void CConfig::GetAllRegPath(std::list <CString> &RegPathList)
{
	RegPathList.clear();

	for (std::list <CString>::iterator itor = m_RegPathList.begin();
		itor != m_RegPathList.end();
		itor++)
	{
		RegPathList.push_back(*itor);
	}
}

CHAR CConfig::GetFirstChar(CString szString)
{
	CHAR chRet = 0;
	if (szString.IsEmpty())
	{
		return chRet;
	}

	WCHAR *szFuck = (WCHAR *)malloc((szString.GetLength() + 1)* sizeof(WCHAR));
	if (szFuck)
	{
		memset(szFuck, 0, (szString.GetLength() + 1)* sizeof(WCHAR));
		memcpy(szFuck, szString.GetBuffer(), szString.GetLength() * sizeof(WCHAR));
		CStringA szTempA = ws2ms(szFuck);
		szTempA.MakeUpper();
		chRet = szTempA.GetAt(0);
		free(szFuck);
		szFuck = NULL;
	}

	return chRet;
}

int CConfig::CString2_int(const CString & str,int radix,int defaultValue){
	if(str.IsEmpty())
		return defaultValue;
	return _tcstol(str,0,radix);
}

COLORREF CConfig::CString2_RGB(const CString & str)
{
	if(str.Left(1) != _T("#")) return CLR_INVALID;
	if(str.GetLength() == 4)
	{
		CString t;
		t.Format(_T("%c%c%c%c%c%c"),str.GetAt(3),str.GetAt(3),str.GetAt(2),str.GetAt(2),str.GetAt(1),str.GetAt(1));
		return CString2_int(t,16);
	}

	if( str.GetLength() == 7)
	{
		CString t;
		t.Format(_T("%c%c%c%c%c%c"),str.GetAt(5),str.GetAt(6),str.GetAt(3),str.GetAt(4),str.GetAt(1),str.GetAt(2));
		return CString2_int(t,16);
	}

	return CLR_INVALID;
}

COLORREF CConfig::GetHideProcColor() 
{ 
	if (m_clrHideProc == CLR_INVALID)
	{
		return RGB( 255, 0, 0 );
	}

	return m_clrHideProc; 
}

COLORREF CConfig::GetMicrosoftProcColor() 
{ 
	if (m_clrMicrosoftProc == CLR_INVALID)
	{
		return RGB( 0, 0, 0);
	}

	return m_clrMicrosoftProc;
}

COLORREF CConfig::GetNormalProcColor() 
{
	if (m_clrNormalProc == CLR_INVALID)
	{
		return RGB( 0, 0, 255 );
	}

	return m_clrNormalProc;
}

COLORREF CConfig::GetMicroHaveOtherModuleProcColor() 
{ 
	if (m_clrMicroHaveOtherModuleProc == CLR_INVALID)
	{
		return RGB( 255, 140, 0 );
	}

	return m_clrMicroHaveOtherModuleProc;
}

COLORREF CConfig::GetHiddenFileColor()
{
	if (m_clrHideFile == CLR_INVALID)
	{
		return RGB( 127, 127, 127 );
	}

	return m_clrHideFile;
}

COLORREF CConfig::GetNormalFileColor()
{
	if (m_clrNormalFile == CLR_INVALID)
	{
		return RGB(0, 0, 0);
	}

	return m_clrNormalFile;
}

COLORREF CConfig::GetSystemFileColor()
{
	if (m_clrSystemFile == CLR_INVALID)
	{
		return RGB( 255, 0, 0 );
	}
	
	return m_clrSystemFile;
}

COLORREF CConfig::GetSystemAndHiddenFileColor()
{
	if (m_clrSystemAndHideFile == CLR_INVALID)
	{
		return RGB( 255, 0, 0 );
	}

	return m_clrSystemAndHideFile;
}

COLORREF CConfig::GetNormalModuleColor()
{
	if (m_NormalItemClr == CLR_INVALID)
	{
		return RGB( 0, 0, 255 );
	}

	return m_NormalItemClr;
}

COLORREF CConfig::GetHiddenOrHooksModuleColor()
{
	if (m_HiddenOrHookItemClr == CLR_INVALID)
	{
		return RGB( 255, 0, 0 );
	}

	return m_HiddenOrHookItemClr;
}

COLORREF CConfig::GetNoSignatureModuleColor()
{
	if (m_NotSignedItemClr == CLR_INVALID)
	{
		return RGB( 180, 0, 255 );
	}

	return m_NotSignedItemClr;
}

COLORREF CConfig::GetMicrosoftModuleColor()
{
	if (m_MicrosoftItemClr == CLR_INVALID)
	{
		return RGB(0, 0, 0);
	}

	return m_MicrosoftItemClr;
}