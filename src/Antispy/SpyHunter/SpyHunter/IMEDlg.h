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
#include "RegCommon.h"
#include <vector>
using namespace std;
#include "Registry.h"
#include "Function.h"

typedef struct _IME_INFO
{
	CString szId;
	CString szName;
	CString szImePath;
	CString szLayoutFilePath;
}IME_INFO, *PIME_INFO;

// CIMEDlg dialog

class CIMEDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CIMEDlg)

public:
	CIMEDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIMEDlg();

// Dialog Data
	enum { IDD = IDD_IME_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	CString m_szStatus;
	CString m_szCurrentUserKey;
	CRegCommon m_RegCommon;
	vector<IME_INFO> m_vectorIME;
	CRegistry m_Registry;
	CommonFunctions m_Functions;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void GetImeInfo(CString szKey, CString& szLayoutText, CString& szLayoutFile, CString& szImeFile);
	void ListIme();
	void AddItems();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnImeRefresh();
	afx_msg void OnImeDelete();
	afx_msg void OnImeShuxingIme();
	afx_msg void OnImeVerifyIme();
	afx_msg void OnImeJmpToIme();
	afx_msg void OnImeAntispyIme();
	afx_msg void OnImeShuxingLayout();
	afx_msg void OnImeVerifyLayout();
	afx_msg void OnImeJmpToLayout();
	afx_msg void OnImeAntispyLayout();
	afx_msg void OnImeText();
	afx_msg void OnImeExcel();
	void DeleteIME(CString szImeId);
	afx_msg void OnImeJmpToReg();
};
