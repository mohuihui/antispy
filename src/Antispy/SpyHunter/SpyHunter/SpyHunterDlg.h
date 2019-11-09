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
// SpyHunterDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "ListProcessDlg.h"
#include "ListDriverDlg.h"
#include "KernelAboutDlg.h"
#include "Ring0HookDlg.h"
/*#include "UserModeDlg.h"*/
#include "RegistryDlg.h"
#include "ServiceDlg.h"
#include "AutoStartDlg.h"
#include "AboutToolDlg.h"
#include "ConnectDriver.h"
#include "SetConfigDlg.h"
#include "FileDlg.h"
#include "NetworkDlg.h"
#include "ToolsDlg.h"
#include "Function.h"

// CSpyHunterDlg 对话框
class CSpyHunterDlg : public CDialog
{
	DECLARE_EASYSIZE
// 构造
public:
	CSpyHunterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SPYHUNTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnHotKey(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL EnableDebugPrivilege(BOOL bEnable);
	WIN_VERSION GetWindowsVersion();
	BOOL LoadDriver(CString szPath, CString szDriverName);
	BOOL StartDriver();
	CString GetRandDriverName(CString szPath, WCHAR* szName);
	BOOL ReleaseDriver( CString szPath, CString szName );
	void DeleteDriver(CString szPath, CString szName);
	BOOL EnableLoadDricerPrivilege(BOOL bEnable);
	LANGUAGE_TYPE GetLanguageID();
	BOOL IsWow64();
	CString GetRandTitle();
	BOOL CheckForUpdates();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	CString GetFirstDriverName(CString szPath, WCHAR* szName);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LIST_ENTRY * GetLdrpHashTable();
	void SetFullScreen();
	void InitGlobalFunctions();
public:
	CTabCtrl m_tab;
	int m_nCurSel;
	CListDriverDlg m_ListDriverDlg;
	CListProcessDlg m_ListProcessDlg;
	CRing0HookDlg m_Ring0HookDlg;
	CKernelAboutDlg m_KernelAboutDlg;
//	CUserModeDlg m_UserModeDlg;
	CRegistryDlg m_RegistryDlg;
	CServiceDlg m_ServiceDlg;
	CAutoStartDlg m_AutoStartDlg;
//	CCommandDlg m_CommandDlg;
	CAboutToolDlg m_AboutDlg;
	CConnectDriver m_Driver;
	CSetConfigDlg m_ConfigDlg;
	CFileDlg m_FileDlg;
	CNetworkDlg m_NetworkDlg;
	CToolsDlg m_ToolsDlg;
	BOOL m_bSelfProtection;
	CommonFunctions m_Functions;
};
