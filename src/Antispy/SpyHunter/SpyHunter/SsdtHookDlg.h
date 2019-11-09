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
#include "afxcmn.h"
#include "ConnectDriver.h"
#include "Function.h"
#include "SignVerifyDlg.h"
#include "..\\..\\Common\Common.h"
#include <vector>
#include "ListHooks.h"
#include "ListDriver.h"

using namespace std;

// CSsdtHookDlg 对话框

class CSsdtHookDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CSsdtHookDlg)

public:
	CSsdtHookDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSsdtHookDlg();

// 对话框数据
	enum { IDD = IDD_SSDT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	CString GetDriverPath(ULONG pCallback);
	void GetSsdtHooks();
	void InsertIntem(SSDT_HOOK_INFO HookInfo);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSdtRefresh();
	afx_msg void OnSdtOnlyShowHook();
	afx_msg void OnSdtDisCurrent();
	afx_msg void OnSdtDisOrigin();
	afx_msg void OnSdtRestore();
	afx_msg void OnSdtRestoreAll();
	afx_msg void OnSdtShuxing();
	afx_msg void OnSdtLocationModule();
	afx_msg void OnSdtVerifySign();
	afx_msg void OnSdtText();
	afx_msg void OnSdtExcel();
	afx_msg void OnUpdateSdtOnlyShowHook(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();

public:
	BOOL m_bOnlyShowHooks;
	CSortListCtrl m_list;
	vector<SSDT_HOOK_INFO> m_vectorHooks;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	vector<DRIVER_INFO> m_vectorDrivers;
	CString m_szStatus;
	ULONG m_nHooked;
	CListHooks m_clsHooks;
	CListDrivers m_clsDrivers;
};
