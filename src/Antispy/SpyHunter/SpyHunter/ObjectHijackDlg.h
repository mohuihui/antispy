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
#include <vector>
#include "ConnectDriver.h"
#include "Function.h"
#include "..\\..\\Common\Common.h"

using namespace std;

// CObjectHijackDlg 对话框

class CObjectHijackDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CObjectHijackDlg)

public:
	CObjectHijackDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CObjectHijackDlg();

// 对话框数据
	enum { IDD = IDD_OBJECT_HIJACK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CSortListCtrl m_list;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnObjectHijackRefresh();
	afx_msg void OnObjectHijackText();
	afx_msg void OnObjectHijackExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	VOID GetObjectHijack();
	CString m_szStatus;
	CommonFunctions m_Functions;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
