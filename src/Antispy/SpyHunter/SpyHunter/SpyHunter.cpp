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
// SpyHunter.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "SpyHunterDlg.h"
// #include <vld.h>
// #pragma comment(lib, "vld.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSpyHunterApp

BEGIN_MESSAGE_MAP(CSpyHunterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSpyHunterApp 构造

CSpyHunterApp::CSpyHunterApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CSpyHunterApp 对象

CSpyHunterApp theApp;


// CSpyHunterApp 初始化

BOOL CSpyHunterApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

//	CoInitialize(NULL);

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
// 	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	// 读取配置文件
	g_Config.ReadConfig();
	
	g_NormalItemClr = g_Config.GetNormalModuleColor();
	g_HiddenOrHookItemClr = g_Config.GetHiddenOrHooksModuleColor();
	g_MicrosoftItemClr = g_Config.GetMicrosoftModuleColor(); 	
	g_NotSignedItemClr = g_Config.GetNoSignatureModuleColor();

	// 获取系统语言
	g_enumLang = GetLanguageID();

	// 根据文件名创建互斥量,只允许一个实例运行
	WCHAR szPath[MAX_PATH * 2] = {0};
	GetModuleFileName(NULL, szPath, MAX_PATH * 2);
	if (wcslen(szPath))
	{
		CString strPath = szPath;
		CString szImage = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
		if (!szImage.IsEmpty())
		{
			m_hMutex = CreateMutex(NULL, TRUE, szImage);
			if (m_hMutex && GetLastError() == ERROR_ALREADY_EXISTS) 
			{
				MessageBox(NULL, szAlreadyRunning[g_enumLang], L"AntiSpy", MB_OK | MB_ICONWARNING);
				return FALSE; //退出本次运行
			} 
		}
	}

	// 设置异常捕获函数
	m_ExceptionManager.SetUnHandleException();

	// 显示主窗体
	CSpyHunterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此处放置处理何时用“确定”来关闭
		//  对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用“取消”来关闭
		//  对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

LANGUAGE_TYPE CSpyHunterApp::GetLanguageID()
{
//	return enumEnglish; 
	LANGUAGE_TYPE languageId = enumEnglish;
	LCID nLangID = GetUserDefaultUILanguage();
	switch (nLangID)
	{
	case 0x0409:		//LANG_ENGLISH  SUBLANG_ENGLISH_US
	case 0x0C09:		//SUBLANG_ENGLISH_AUS
	case 0x2809:		//SUBLANG_ENGLISH_BELIZE
	case 0x1009:		//SUBLANG_ENGLISH_CAN
	case 0x2409:		//SUBLANG_ENGLISH_CARIBBEAN
	case 0x4009:		 //SUBLANG_ENGLISH_INDIA
	case 0x1809:		//SUBLANG_ENGLISH_EIRE
	case 0x2009:		//SUBLANG_ENGLISH_JAMAICA
	case 0x4409:		//SUBLANG_ENGLISH_MALAYSIA
	case 0x1409:		//SUBLANG_ENGLISH_NZ
	case 0x3409:		//SUBLANG_ENGLISH_PHILIPPINES
	case 0x4809:		//SUBLANG_ENGLISH_SINGAPORE
	case 0x1c09:		//SUBLANG_ENGLISH_SOUTH_AFRICA
	case 0x2C09:		//SUBLANG_ENGLISH_TRINIDAD
	case 0x0809:		//SUBLANG_ENGLISH_UK
	case 0x3009:		//SUBLANG_ENGLISH_ZIMBABWE
		languageId = enumEnglish;
		break;

	case 0x0C04:		//SUBLANG_CHINESE_HONGKONG
	case 0x1404:		//SUBLANG_CHINESE_MACAU
	case 0x1004:		//SUBLANG_CHINESE_SINGAPORE
	case 0x0804:		//SUBLANG_CHINESE_SIMPLIFIED
	case 0x0404:		//SUBLANG_CHINESE_TRADITIONAL
		languageId = enumChinese;
		break;

	default :
		languageId = enumEnglish;
		break;
	}

	return languageId;
}

int CSpyHunterApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	g_Config.WriteConfig();

	return CWinApp::ExitInstance();
}