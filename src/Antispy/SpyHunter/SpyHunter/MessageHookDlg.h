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
#include <list>
#include <vector>

using namespace std;

// CMessageHookDlg 对话框

class CMessageHookDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CMessageHookDlg)

public:
	CMessageHookDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMessageHookDlg();

// 对话框数据
	enum { IDD = IDD_MESSAGE_HOOK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_szStatus;
	CSortListCtrl m_list;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void EnumMessageHook();
	void InsertMessageHookItem(MESSAGE_HOOK_INFO item);
	CString GetTypeName(HOOK_TYPE iHook);
public:
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnMessageFlresh();
	afx_msg void OnMessageOnlyShowGlobalHook();
	afx_msg void OnUpdateMessageOnlyShowGlobalHook(CCmdUI *pCmdUI);
	afx_msg void OnMessageUnhook();
	afx_msg void OnMessageDisasm();
	afx_msg void OnMessageLocationFile();
	afx_msg void OnMessageText();
	afx_msg void OnMessageExcel();
	DWORD m_dwCnt;
	DWORD m_dwGlobalCnt;
	BOOL m_bOnlyShowGlobal;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
