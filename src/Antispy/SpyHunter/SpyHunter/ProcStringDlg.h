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


// CProcStringDlg dialog

class CProcStringDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CProcStringDlg)

public:
	CProcStringDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProcStringDlg();

// Dialog Data
	enum { IDD = IDD_PROCESS_STRING_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szLength;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	int m_nRadio;
	afx_msg void OnBnClickedBtnStart();
	BOOL m_bPrivate;
	BOOL m_bImage;
	BOOL m_bMapped;
};
