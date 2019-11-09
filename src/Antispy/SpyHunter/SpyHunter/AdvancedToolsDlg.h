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
#pragma once
#include "ConnectDriver.h"
#include "Registry.h"
#include "Function.h"

// CAdvancedToolsDlg 对话框

class CAdvancedToolsDlg : public CDialog
{
	DECLARE_DYNAMIC(CAdvancedToolsDlg)

public:
	CAdvancedToolsDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAdvancedToolsDlg();

// 对话框数据
	enum { IDD = IDD_ADVANCED_TOOLS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BOOL CommDriver(FORBID_TYPE nType, BOOL bDeny);
	afx_msg void OnBnClickedNotCreateThread();
	afx_msg void OnBnClickedNotCreateProcess();
	afx_msg void OnBnClickedNotCreateFile();
	afx_msg void OnBnClickedNotSwitchDesktop();
	afx_msg void OnBnClickedNotCreateKey();
	afx_msg void OnBnClickedNotSetValue();
	afx_msg void OnBnClickedNotLoadImage();
	afx_msg void OnBnClickedNotSystemShutdown();
	afx_msg void OnBnClickedNotModiftTime();
	afx_msg void OnBnClickedNotLockWorkstation();
	afx_msg void OnBnClickedNotLoadDriver();
	afx_msg void OnBnClickedNotInjectModule();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSystemReboot();
	afx_msg void OnBnClickedForceSystemReboot();
	afx_msg void OnBnClickedSystemShutdown();
	BOOL CommDriverPower(POWER_TYPE nType);
	afx_msg void OnBnClickedEnable();
	void EnableTools(HKEY hRootKey, WCHAR *szSubKey, WCHAR *szValue);
	afx_msg void OnBnClickedBtnRepairSafeboot();
	CString ReleaseOriginRegHive();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedNotScreenCapture();
	afx_msg void OnBnClickedBackuoDriver();

private:
	CommonFunctions m_Function;
	CConnectDriver m_Driver;
	BOOL m_bForbidCreteThread;
	BOOL m_bForbidModifySystemTime;
	BOOL m_bForbidLockWorkStation;
	BOOL m_bForbidLoadDriver;
	BOOL m_bForbidLoadImage;
	BOOL m_bForbidSystemShutdown;
	BOOL m_bForbidSetKeyValue;
	BOOL m_bForbidCreateKey;
	BOOL m_bForbidSwitchDesktop;
	BOOL m_bForbidCreteProcess;
	BOOL m_bForbidCreteFile;
	BOOL m_bForbidInjectModule;
	BOOL m_bReboot;
	BOOL m_bForceReboot;
	BOOL m_bPorweOff;
	BOOL m_bEnableRun;
	BOOL m_bEnableRegistry;
	BOOL m_bEnableTaskTools;
	BOOL m_bEnableCmd;
	BOOL m_bEnableControlPane;
	BOOL m_bEnableToolbarMenu;
	BOOL m_bClearAllGroupPolicies;
	BOOL m_bForbidScreenCapture;
	BOOL m_bBackupDriver;
//	HANDLE m_hCommEvent;
// 	HANDLE hCommThread;
};
