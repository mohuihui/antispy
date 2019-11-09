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


// CHashCalcDlg dialog

class CHashCalcDlg : public CDialog
{
	DECLARE_DYNAMIC(CHashCalcDlg)

public:
	CHashCalcDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHashCalcDlg();

// Dialog Data
	enum { IDD = IDD_FILE_HASH_CALC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCompare();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	void CalcHash();
	void CalcMD5();
	void CalcSha1();
	afx_msg LRESULT UpdateUIMd5(WPARAM, LPARAM);
	afx_msg LRESULT UpdateUISha1(WPARAM, LPARAM);
	CString m_szMd5;
	CString m_szMd5Compare;
	CString m_szSha1;
	CString m_szSha1Compare;
	CString m_szPath;
	HANDLE m_hMd5Thread;
	HANDLE m_hSHA1Thread;
	afx_msg void OnClose();
};
