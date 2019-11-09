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


// CAboutToolDlg 对话框

class CAboutToolDlg : public CDialog
{
	DECLARE_DYNAMIC(CAboutToolDlg)

public:
	CAboutToolDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAboutToolDlg();

// 对话框数据
	enum { IDD = IDD_ABOUT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	void CheckForUpdate();
	CString m_szCheckUpdate;
	virtual BOOL OnInitDialog();
	CString m_szContactAuthor;
	CString m_szAuthor;
	CString m_szEmail;
	CString m_szSina;
	CString m_szTencent;
	CString m_szInformationAndThanks;
	CString m_szInformation;
	CString m_szSystem;
	CString m_szThanks;
	CString m_szCopyright;
	CString m_szMianze;
	CString m_szUpdate;
	BOOL CheckForUpdates();
	CString m_szWebsite;
	CString m_szVersion;

	CXTPHyperLink m_FengquHypeLike;
	CXTPHyperLink m_52PojieHypeLink;
	CXTPHyperLink m_BolgHypeLink;
	CXTPHyperLink m_SinaHypeLink;
	CXTPHyperLink m_TencetHypeLink;
	CXTPHyperLink m_EmailHypeLink;
/*	CXTPHyperLink m_CheckUpdateHypeLink;*/
	CString m_szAntiSpyDotCn;
	CXTPHyperLink m_AntiSpyDotCnHypeLink;
	CString m_szQQ;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void GotoUpdateUrl();
};
