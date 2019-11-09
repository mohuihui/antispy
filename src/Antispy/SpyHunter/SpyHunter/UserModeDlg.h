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
#include "MessageHookDlg.h"
#include "ProcessHookDlg.h"
#include "KernelCallbackTableDlg.h"
#include "ProcessHijackDlg.h"

// CUserModeDlg 对话框

class CUserModeDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CUserModeDlg)

public:
	CUserModeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUserModeDlg();

// 对话框数据
	enum { IDD = IDD_USER_MODE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CTabCtrl m_tab;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	ULONG m_nCurSel;
	CMessageHookDlg m_MessageHookDlg;
	CProcessHookDlg m_ProcessHookDlg;
	CKernelCallbackTableDlg m_KernelCallbackTableDlg;
	CProcessHijackDlg m_ProcessHijackDlg;
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
