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
#include "Function.h"

// CHostDlg 对话框

class CHostDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CHostDlg)

public:
	CHostDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHostDlg();

// 对话框数据
	enum { IDD = IDD_HOST_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnBnClickedSure();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void ReadHostsFile();
	CStringA ws2ms(LPWSTR szSrc, int cbMultiChar = -1);
	CString m_szHosts;
	afx_msg void OnBnClickedOpenNotepad();
	CommonFunctions m_Functions;
	afx_msg void OnBnClickedHostResrt();
};
