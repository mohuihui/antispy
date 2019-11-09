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
#include "Registry.h"
#include <list>
using namespace std;
#include "Function.h"

// CIFEODlg dialog

class CIFEODlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CIFEODlg)

public:
	CIFEODlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIFEODlg();

// Dialog Data
	enum { IDD = IDD_IFEO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szIFEOStatus;
	CSortListCtrl m_list;
	CRegistry m_Registry;
	CommonFunctions m_Functions;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void ListIFEO();
	afx_msg void OnIfeoRefresh();
	afx_msg void OnIfeoDelete();
	afx_msg void OnIfeoJmpToReg();
	afx_msg void OnIfeoShuxing();
	afx_msg void OnIfeoSignVerify();
	afx_msg void OnIfeoJmpToExplorer();
	afx_msg void OnIfeoJmpToAntispy();
	afx_msg void OnIefoText();
	afx_msg void OnIefoExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
};
