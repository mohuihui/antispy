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
#include "ListDriver.h"
using namespace std;


// CKbdclassDlg 对话框

class CKbdclassDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CKbdclassDlg)

public:
	CKbdclassDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CKbdclassDlg();

// 对话框数据
	enum { IDD = IDD_KBDCLASS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	void GetKbdclassDispatch();
	void GetDriver();
	CString GetDriverPath(ULONG pCallback);
	void InsertKbdclassItem(DISPATCH_HOOK_INFO HookInfo);
	CSortListCtrl m_list;
	CString m_szStatus;
	vector<DISPATCH_HOOK_INFO> m_KbdClassHookVector;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	vector<DRIVER_INFO> m_CommonDriverList;
	BOOL m_bOnlyShowHooks;
	ULONG m_nHookedCnt;
	CListDrivers m_clsDrivers;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSdtRefresh();
	afx_msg void OnSdtOnlyShowHook();
	afx_msg void OnUpdateSdtOnlyShowHook(CCmdUI *pCmdUI);
	afx_msg void OnSdtDisCurrent();
	afx_msg void OnSdtDisOrigin();
	afx_msg void OnSdtRestore();
	afx_msg void OnSdtRestoreAll();
	afx_msg void OnSdtShuxing();
	afx_msg void OnSdtLocationModule();
	afx_msg void OnSdtVerifySign();
	afx_msg void OnSdtText();
	afx_msg void OnSdtExcel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
