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


// CHexEditDlg dialog

class CHexEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CHexEditDlg)

public:
	CHexEditDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHexEditDlg();

// Dialog Data
	enum { IDD = IDD_HEX_EDIT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CXTPHexEdit m_editHex;
	virtual BOOL OnInitDialog();
	CString m_szValueName;
	ULONG m_nDataLen;
	PBYTE m_pData;
	PBYTE m_pRetData;
	ULONG m_nRetLen;

	afx_msg void OnBnClickedOk();
};
