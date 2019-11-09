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
#include "afxwin.h"


// CSetConfigDlg dialog

class CSetConfigDlg : public CDialog
{
	DECLARE_DYNAMIC(CSetConfigDlg)

public:
	CSetConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetConfigDlg();

// Dialog Data
	enum { IDD = IDD_CONFIG_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szGeneralConfig;
	CString m_szStaticTitle;
	CString m_szEditTitle;
	BOOL m_bRandomTitle;
	BOOL m_bStayOnTop;
	BOOL m_bSelfProtection;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRandomTitle();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	BOOL m_bAutoDetectNewVersion;
	CStringA ws2ms(LPWSTR szSrc, int cbMultiChar = -1);
	CHAR GetFirstChar(CString szString);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void UpdateStatus();
	afx_msg void OnBnClickedUserHotkeys();
	afx_msg void OnBnClickedButton2();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	CString m_szShowWindowHotKey;
	CString m_szStaticShowMainWindow;
	CString m_szStaticProcessConfig;
	BOOL m_bShowProcessDetailInformations;
	BOOL m_bShowLowerPaneAsDefault;
	BOOL m_bShowProcessTreeAsDeafult;

	// 进程颜色选择器
	CXTPButton m_frmProcessColor;

	CString m_szNormalPreocessColor;
	CString m_szHideProcessColor;
	CString m_szMicorsoftProcessHaveOtherModulesColor;
	CString m_szMicrosoftProcessColor;

	CXTPColorPicker m_cpNoramlProcess;
	CXTPColorPicker m_cpHideProcess;
	CXTPColorPicker m_cpMicrosoftProcess;
	CXTPColorPicker m_cpMicrosoftHaveOtherModulesProcess;

	COLORREF m_NormalProcClr;
	COLORREF m_HideProcClr;
	COLORREF m_MicorsoftProcClr;
	COLORREF m_MicorsoftProcHaveOtherModuleClr;

	// 文件颜色选择器
	CXTPButton m_frmFileColor;

	CString m_szNoramFile;
	CString m_szSystemAndHiddenFile;
	CString m_szHiddenFile;
	CString m_szSystemFile;

	CXTPColorPicker m_cpNoramlFile;
	CXTPColorPicker m_cpHiddenAndSystemFile;
	CXTPColorPicker m_cpHiddendFile;
	CXTPColorPicker m_cpSystemFile;

	COLORREF m_NormalFileClr;
	COLORREF m_HiddenFileClr;
	COLORREF m_HiddenAndSystemFileClr;
	COLORREF m_SystemFileClr;

	// 通用颜色选择器
	CXTPButton m_frmGeneranColor;

	CString m_szNormalItem;
	CString m_szMicrosoftItem;
	CString m_szHideOrHookItem;
	CString m_szNoSignItem;

	CXTPColorPicker m_cpNormalItem;
	CXTPColorPicker m_cpMicrosoftItem;
	CXTPColorPicker m_cpHideOrHookItem;
	CXTPColorPicker m_cpNoSignItem;

	COLORREF m_NormalItemClr;
	COLORREF m_HiddenOrHookItemClr;
	COLORREF m_MicrosoftItemClr;
	COLORREF m_NotSignedItemClr;
	CString m_szPS;
	BOOL m_bEnableHotkeys;
};
