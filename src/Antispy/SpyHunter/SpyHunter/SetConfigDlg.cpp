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
// SetConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "SetConfigDlg.h"


// CSetConfigDlg dialog

IMPLEMENT_DYNAMIC(CSetConfigDlg, CDialog)

CSetConfigDlg::CSetConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetConfigDlg::IDD, pParent)
	, m_szGeneralConfig(_T(""))
	, m_szStaticTitle(_T(""))
	, m_szEditTitle(_T(""))
	, m_bRandomTitle(FALSE)
	, m_bStayOnTop(FALSE)
	, m_bAutoDetectNewVersion(FALSE)
	, m_szShowWindowHotKey(_T(""))
	, m_szStaticShowMainWindow(_T(""))
	, m_szStaticProcessConfig(_T(""))
	, m_bShowLowerPaneAsDefault(FALSE)
	, m_bShowProcessTreeAsDeafult(FALSE)
	, m_szNormalPreocessColor(_T(""))
	, m_szHideProcessColor(_T(""))
	, m_szMicorsoftProcessHaveOtherModulesColor(_T(""))
	, m_szMicrosoftProcessColor(_T(""))
	, m_szNoramFile(_T(""))
	, m_szSystemAndHiddenFile(_T(""))
	, m_szHiddenFile(_T(""))
	, m_szSystemFile(_T(""))
	, m_szNormalItem(_T(""))
	, m_szMicrosoftItem(_T(""))
	, m_szHideOrHookItem(_T(""))
	, m_szNoSignItem(_T(""))
	, m_bSelfProtection(FALSE)
	, m_szPS(_T(""))
	, m_bEnableHotkeys(FALSE)
{
	m_bShowProcessDetailInformations = TRUE;

	m_MicorsoftProcClr = RGB(0, 0, 0);
	m_NormalProcClr = RGB( 0, 0, 255 );
	m_HideProcClr = RGB( 255, 0, 0 );
	m_MicorsoftProcHaveOtherModuleClr = RGB( 255, 140, 0 );	// 微软带有非微软模块默认是 橘黄色

	m_NormalFileClr = RGB(0, 0, 0);					// 正常文件 黑色
	m_HiddenFileClr = RGB( 127, 127, 127 );			// 隐藏文件 灰色
	m_HiddenAndSystemFileClr = RGB( 255, 0, 0 );	// 隐藏&系统 红色
	m_SystemFileClr = RGB( 255, 0, 0 );				// 系统文件	红色

	m_NormalItemClr = RGB( 0, 0, 255 );				// 非微软的文件默认显示蓝色
	m_HiddenOrHookItemClr = RGB( 255, 0, 0 );		// 隐藏或者钩子默认显示红色
	m_MicrosoftItemClr = RGB(0, 0, 0);				// 微软模块默认显示黑色
	m_NotSignedItemClr = RGB( 180, 0, 255 );		// 未签名项默认显示紫色
}

CSetConfigDlg::~CSetConfigDlg()
{
}

void CSetConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_GENERAL_CONFIG, m_szGeneralConfig);
	DDX_Text(pDX, IDC_TITLE, m_szStaticTitle);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_szEditTitle);
	DDX_Check(pDX, IDC_RANDOM_TITLE, m_bRandomTitle);
	DDX_Check(pDX, IDC_STAY_ON_TOP, m_bStayOnTop);
	DDX_Check(pDX, IDC_AUTO_DETECT_NEW_VERSION, m_bAutoDetectNewVersion);
	DDX_Text(pDX, IDC_EDIT_SHOW_WIDNOW_HOT_KEY, m_szShowWindowHotKey);
	DDX_Text(pDX, IDC_STATIC_SHOW_WIDNOW_HOT_KEY, m_szStaticShowMainWindow);
	DDX_Text(pDX, IDC_STATIC_PROCESS_CONFIG, m_szStaticProcessConfig);
	DDX_Check(pDX, IDC_CHECK_SHOW_PROCESS_DETAIL_INFORMATIONS, m_bShowProcessDetailInformations);
	DDX_Check(pDX, IDC_CHECK_SHOW_MODULE_DEFAULT, m_bShowLowerPaneAsDefault);
	DDX_Check(pDX, IDC_CHECK_SHOW_PROCESS_TREE_DEFAULT, m_bShowProcessTreeAsDeafult);
	DDX_Control(pDX, IDC_CLR_NORMAL, m_cpNoramlProcess);
	DDX_Control(pDX, IDC_CLR_HIDE, m_cpHideProcess);
	DDX_Control(pDX, IDC_CLR_MICROSOFT, m_cpMicrosoftProcess);
	DDX_Control(pDX, IDC_CLR_MICROSOFT_HAVE_OHTER_MODULE, m_cpMicrosoftHaveOtherModulesProcess);
	DDX_XTPColorPicker(pDX, IDC_CLR_NORMAL, m_NormalProcClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_HIDE, m_HideProcClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_MICROSOFT, m_MicorsoftProcClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_MICROSOFT_HAVE_OHTER_MODULE, m_MicorsoftProcHaveOtherModuleClr);
	DDX_Text(pDX, IDC_STATIC_NOT_MORIOSOFT, m_szNormalPreocessColor);
	DDX_Text(pDX, IDC_STATIC_HIDE_PROCESS, m_szHideProcessColor);
	DDX_Text(pDX, IDC_STATIC_MORIOSOFT_PROCESS_HAVE_OTHER_MODULE, m_szMicorsoftProcessHaveOtherModulesColor);
	DDX_Text(pDX, IDC_STATIC_MORIOSOFT_PROCESS, m_szMicrosoftProcessColor);
	DDX_Control(pDX, IDC_STATIC_PROCESS_COLOR, m_frmProcessColor);
	DDX_Control(pDX, IDC_STATIC_FILE_COLOR, m_frmFileColor);
	DDX_Text(pDX, IDC_STATIC_NORMAL_FILE, m_szNoramFile);
	DDX_Text(pDX, IDC_STATIC_SYSTEM_AND_HIDDEND_FILE, m_szSystemAndHiddenFile);
	DDX_Text(pDX, IDC_STATIC_HIDDEN_FILE, m_szHiddenFile);
	DDX_Text(pDX, IDC_STATIC_SYSTEM_FILE, m_szSystemFile);
	DDX_Control(pDX, IDC_CLR_NORMAL_FILE, m_cpNoramlFile);
	DDX_Control(pDX, IDC_CLR_HIDDEN_AND_SYSTEM_FILE, m_cpHiddenAndSystemFile);
	DDX_Control(pDX, IDC_CLR_HIDDEN_FILE, m_cpHiddendFile);
	DDX_Control(pDX, IDC_CLR_SYSTEM_FILE, m_cpSystemFile);
	DDX_XTPColorPicker(pDX, IDC_CLR_NORMAL_FILE, m_NormalFileClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_HIDDEN_AND_SYSTEM_FILE, m_HiddenAndSystemFileClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_HIDDEN_FILE, m_HiddenFileClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_SYSTEM_FILE, m_SystemFileClr);
	DDX_Control(pDX, IDC_STATIC_GENERAL_COLOR, m_frmGeneranColor);
	DDX_Text(pDX, IDC_STATIC_NOT_MORIOSOFT_ITEM, m_szNormalItem);
	DDX_Text(pDX, IDC_STATIC_MORIOSOFT_ITEM, m_szMicrosoftItem);
	DDX_Text(pDX, IDC_STATIC_HIDE_OR_HOOK_ITEM, m_szHideOrHookItem);
	DDX_Text(pDX, IDC_STATIC_NO_SIGN_ITEM, m_szNoSignItem);
	DDX_Control(pDX, IDC_CLR_NORMAL_ITEM, m_cpNormalItem);
	DDX_Control(pDX, IDC_CLR_MICROSOFT_ITEM, m_cpMicrosoftItem);
	DDX_Control(pDX, IDC_CLR_HOOK_ITEM, m_cpHideOrHookItem);
	DDX_Control(pDX, IDC_CLR_NO_SIGN_ITEM, m_cpNoSignItem);
	DDX_XTPColorPicker(pDX, IDC_CLR_NORMAL_ITEM, m_NormalItemClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_MICROSOFT_ITEM, m_MicrosoftItemClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_HOOK_ITEM, m_HiddenOrHookItemClr);
	DDX_XTPColorPicker(pDX, IDC_CLR_NO_SIGN_ITEM, m_NotSignedItemClr);
	DDX_Check(pDX, IDC_SELF_PROTECTION, m_bSelfProtection);
	DDX_Text(pDX, IDC_STATIC_PS, m_szPS);
	DDX_Check(pDX, IDC_USER_HOTKEYS, m_bEnableHotkeys);
}

BEGIN_MESSAGE_MAP(CSetConfigDlg, CDialog)
	ON_BN_CLICKED(IDC_RANDOM_TITLE, &CSetConfigDlg::OnBnClickedRandomTitle)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, &CSetConfigDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSetConfigDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON2, &CSetConfigDlg::OnBnClickedButton2)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_USER_HOTKEYS, &CSetConfigDlg::OnBnClickedUserHotkeys)
END_MESSAGE_MAP()


// CSetConfigDlg message handlers

BOOL CSetConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_pSetConfigDlg = (CWnd*)this;

	// 设置控件文本
	m_szGeneralConfig = szGeneralConfiguration[g_enumLang];
	m_szStaticTitle = szTitleName[g_enumLang];
	m_szStaticShowMainWindow = szHotKeyForMainWindow[g_enumLang];
	m_szStaticProcessConfig = szProcessListConfiguration[g_enumLang];
		
	// 进程颜色，static控件文字
	m_szNormalPreocessColor = szNormalProcess[g_enumLang];
	m_szHideProcessColor = szHideProcess[g_enumLang];
	m_szMicorsoftProcessHaveOtherModulesColor = szMirosoftProcessHaveOtherModule[g_enumLang];
	m_szMicrosoftProcessColor = szMirosoftProcess[g_enumLang];

	// 文件颜色，static控件文字
	m_szNoramFile = szNormalFile[g_enumLang];
	m_szSystemAndHiddenFile = szSystemAndHidden[g_enumLang];
	m_szHiddenFile = szHiddenOnly[g_enumLang];
	m_szSystemFile = szSystemOnly[g_enumLang];
	
	// 通用颜色，static控件文字
	m_szNormalItem = szColorNormalItem[g_enumLang];
	m_szMicrosoftItem = szColorMicrosoftItem[g_enumLang];
	m_szHideOrHookItem = szColorHiddenOrHooksItem[g_enumLang];
	m_szNoSignItem = szColorNotSignedItem[g_enumLang];

	m_szPS = szConfigPS[g_enumLang];

	GetDlgItem(IDC_STAY_ON_TOP)->SetWindowText(szStayOnTop[g_enumLang]);
	GetDlgItem(IDC_SELF_PROTECTION)->SetWindowText(szSelfProtection[g_enumLang]);
	GetDlgItem(IDC_RANDOM_TITLE)->SetWindowText(szRandomTitle[g_enumLang]);
	GetDlgItem(IDC_AUTO_DETECT_NEW_VERSION)->SetWindowText(szAutoDetectNewVersion[g_enumLang]);
	GetDlgItem(IDC_CHECK_SHOW_PROCESS_DETAIL_INFORMATIONS)->SetWindowText(szShowProcessDetailInformations[g_enumLang]);
	GetDlgItem(IDC_CHECK_SHOW_MODULE_DEFAULT)->SetWindowText(szDeafultShowLowPaner[g_enumLang]);
	GetDlgItem(IDC_CHECK_SHOW_PROCESS_TREE_DEFAULT)->SetWindowText(szDeafultShowProcessTree[g_enumLang]);
	
	GetDlgItem(IDC_HOTKEY_CONFIG)->SetWindowText(szHotKeysConfiguration[g_enumLang]);
	GetDlgItem(IDC_STATIC_COLOR_SET)->SetWindowText(szColorConfiguration[g_enumLang]);
	GetDlgItem(IDC_STATIC_PROCESS_COLOR)->SetWindowText(szColorProcess[g_enumLang]);
	GetDlgItem(IDC_STATIC_FILE_COLOR)->SetWindowText(szColorFile[g_enumLang]);
	GetDlgItem(IDC_STATIC_GENERAL_COLOR)->SetWindowText(szColorGeneral[g_enumLang]);
	GetDlgItem(IDC_USER_HOTKEYS)->SetWindowText(szEnableHotkeys[g_enumLang]);

	// 根据配置文件,设置控件相关信息
	m_szEditTitle = g_Config.GetTitle();
	m_bRandomTitle = g_Config.GetRandomTitle();
	m_bStayOnTop = g_Config.GetStayOnTop();
	m_bAutoDetectNewVersion = g_Config.GetAutoDetectNewVersion();
	m_bSelfProtection = g_Config.GetSelfProtection();
	m_bEnableHotkeys = g_Config.GetEnableHotkeys();
	
	// 如果快捷键不能使用，就置灰
	if (!m_bEnableHotkeys)
	{
		GetDlgItem(IDC_EDIT_SHOW_WIDNOW_HOT_KEY)->EnableWindow(FALSE);
	}

	// 获取快捷键
	m_szShowWindowHotKey.Format(L"%c", g_Config.GetHotKeyForShowMainWiondow());

	// 显示设置，默认显示进程树等
	m_bShowProcessDetailInformations = g_Config.GetShowProcessDetailInfo();
	m_bShowLowerPaneAsDefault = g_Config.GetShowLowerPaneAsDefault();
	m_bShowProcessTreeAsDeafult = g_Config.GetShowProcessTreeAsDefault();

	GetDlgItem(IDC_EDIT_TITLE)->EnableWindow(!m_bRandomTitle);

	// 获取进程颜色
	m_NormalProcClr = g_Config.GetNormalProcColor();
	m_HideProcClr = g_Config.GetHideProcColor();
	m_MicorsoftProcClr = g_Config.GetMicrosoftProcColor();
	m_MicorsoftProcHaveOtherModuleClr = g_Config.GetMicroHaveOtherModuleProcColor();
	
	// 获取文件颜色
	m_NormalFileClr = g_Config.GetNormalFileColor();
	m_HiddenFileClr = g_Config.GetHiddenFileColor();
	m_HiddenAndSystemFileClr = g_Config.GetSystemAndHiddenFileColor();
	m_SystemFileClr = g_Config.GetSystemFileColor();

	// 获取通用的颜色设置
	m_NormalItemClr = g_Config.GetNormalModuleColor();
	m_HiddenOrHookItemClr = g_Config.GetHiddenOrHooksModuleColor();
	m_MicrosoftItemClr = g_Config.GetMicrosoftModuleColor();
	m_NotSignedItemClr = g_Config.GetNoSignatureModuleColor();

	m_cpNoramlProcess.SetColor(m_NormalProcClr);
	m_cpNoramlProcess.SetDefaultColor(GetSysColor(COLOR_WINDOW));
	m_cpNoramlProcess.ShowText(FALSE);

	m_cpHideProcess.SetColor(m_HideProcClr);
	m_cpHideProcess.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpHideProcess.ShowText(FALSE);

	m_cpMicrosoftProcess.SetColor(m_MicorsoftProcClr);
	m_cpMicrosoftProcess.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpMicrosoftProcess.ShowText(FALSE);

	m_cpMicrosoftHaveOtherModulesProcess.SetColor(m_MicorsoftProcHaveOtherModuleClr);
	m_cpMicrosoftHaveOtherModulesProcess.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpMicrosoftHaveOtherModulesProcess.ShowText(FALSE);

	m_cpNoramlFile.SetColor(m_NormalFileClr);
	m_cpNoramlFile.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpNoramlFile.ShowText(FALSE);

	m_cpHiddenAndSystemFile.SetColor(m_HiddenAndSystemFileClr);
	m_cpHiddenAndSystemFile.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpHiddenAndSystemFile.ShowText(FALSE);

	m_cpHiddendFile.SetColor(m_HiddenFileClr);
	m_cpHiddendFile.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpHiddendFile.ShowText(FALSE);

	m_cpSystemFile.SetColor(m_SystemFileClr);
	m_cpSystemFile.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpSystemFile.ShowText(FALSE);

	m_cpNormalItem.SetColor(m_NormalItemClr);
	m_cpNormalItem.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpNormalItem.ShowText(FALSE);

	m_cpMicrosoftItem.SetColor(m_MicrosoftItemClr);
	m_cpMicrosoftItem.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpMicrosoftItem.ShowText(FALSE);

	m_cpHideOrHookItem.SetColor(m_HiddenOrHookItemClr);
	m_cpHideOrHookItem.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpHideOrHookItem.ShowText(FALSE);

	m_cpNoSignItem.SetColor(m_NotSignedItemClr);
	m_cpNoSignItem.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));
	m_cpNoSignItem.ShowText(FALSE);

	m_frmProcessColor.SetBorderStyle(xtpGroupBoxSingleLine);
	m_frmProcessColor.SetUseVisualStyle(FALSE);

	m_frmFileColor.SetBorderStyle(xtpGroupBoxSingleLine);
	m_frmFileColor.SetUseVisualStyle(FALSE);
	
	m_frmGeneranColor.SetBorderStyle(xtpGroupBoxSingleLine);
	m_frmGeneranColor.SetUseVisualStyle(FALSE);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSetConfigDlg::OnBnClickedRandomTitle()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_TITLE)->EnableWindow(!m_bRandomTitle);
}


//WCHAR转TCHAR
CStringA CSetConfigDlg::ws2ms(LPWSTR szSrc, int cbMultiChar)
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

CHAR CSetConfigDlg::GetFirstChar(CString szString)
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

// 窗口关闭的时候保存设置
void CSetConfigDlg::OnDestroy()
{
	UpdateData(TRUE);

	if (m_szEditTitle.GetLength() > 50)
	{
		m_szEditTitle = m_szEditTitle.Left(50);
	}

	g_Config.SetRandomTitle(m_bRandomTitle);
	g_Config.SetStayOnTop(m_bStayOnTop);
	g_Config.SetTitle(m_szEditTitle);
	g_Config.SetAutoDetectNewVersion(m_bAutoDetectNewVersion);
	g_Config.SetHotKeyForShowMainWiondow(GetFirstChar(m_szShowWindowHotKey));
	g_Config.SetShowProcessDetailInfo(m_bShowProcessDetailInformations);
	g_Config.SetShowProcessTreeAsDefault(m_bShowProcessTreeAsDeafult);
//	g_Config.SetShowLowerPaneAsDefault(m_bShowLowerPaneAsDefault);
	
	g_Config.SetHideProcColor(m_HideProcClr);
	g_Config.SetMicroHaveOtherModuleProcColor(m_MicorsoftProcHaveOtherModuleClr);
	g_Config.SetMicrosoftProcColor(m_MicorsoftProcClr);
	g_Config.SetNormalProcColor(m_NormalProcClr);

	g_Config.SetNormalFileColor(m_NormalFileClr);
	g_Config.SetHiddenFileColor(m_HiddenFileClr);
	g_Config.SetSystemFileColor(m_SystemFileClr);
	g_Config.SetSystemAndHiddenFileColor(m_HiddenAndSystemFileClr);

	g_Config.SetNormalModuleColor(m_NormalItemClr);
	g_Config.SetMicrosoftModuleColor(m_MicrosoftItemClr);
	g_Config.SetHiddenOrHooksModuleColor(m_HiddenOrHookItemClr);
	g_Config.SetNoSignatureModuleColor(m_NotSignedItemClr);
	g_Config.SetSelfProtection(m_bSelfProtection);

	g_Config.SetEnableHotkeys(m_bEnableHotkeys);

	CDialog::OnDestroy();
}

void CSetConfigDlg::OnBnClickedOk()
{
	return;
}

void CSetConfigDlg::OnBnClickedCancel()
{
	return;
}

BOOL CSetConfigDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CSetConfigDlg::UpdateStatus()
{
	UpdateData(FALSE);
}

void CSetConfigDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
}

HBRUSH CSetConfigDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_PS)
	{
		pDC->SetTextColor(RGB(255, 0, 0));
	}

	return hbr;
}

void CSetConfigDlg::OnBnClickedUserHotkeys()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_EDIT_SHOW_WIDNOW_HOT_KEY)->EnableWindow(m_bEnableHotkeys);
}
