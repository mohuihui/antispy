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
#include "ListCallbackDlg.h"
#include "DpcTimerDlg.h"
#include "IoTimerDlg.h"
#include "SystemThreadDlg.h"
#include "WorkerThreadDlg.h"
#include "DebugRegisterDlg.h"
#include "FilterDlg.h"
#include "ObjectHijackDlg.h"
#include "DirectIoDlg.h"

// CKernelAboutDlg 对话框

class CKernelAboutDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CKernelAboutDlg)

public:
	CKernelAboutDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CKernelAboutDlg();

// 对话框数据
	enum { IDD = IDD__KERNEL_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();

public:
	CTabCtrl m_tab;
	int m_nCurSel;
	CListCallbackDlg m_ListCallbackDlg;
	CDpcTimerDlg m_DpcTimerDlg;
	CIoTimerDlg m_IoTimerDlg;
	CSystemThreadDlg m_SystemThreadDlg;
	CWorkerThreadDlg m_WorkerThread;
	CDebugRegisterDlg m_DebugRegisterDlg;
	CFilterDlg m_FilterDlg;
	CObjectHijackDlg m_ObjectHijackDlg;
	CDirectIoDlg m_DirectIoDlg;
};
