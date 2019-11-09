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
#include "MyPic.h"

// CFindWindow dialog

class CFindWindow : public CDialog
{
	DECLARE_DYNAMIC(CFindWindow)

public:
	CFindWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFindWindow();

// Dialog Data
	enum { IDD = IDD_FIND_WINDOW_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szPid;
	CString m_szTid;
	CMyPic m_picture;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CString m_szShuoming;
	CString m_szStaticPid;
	CString m_szStaticTid;
	HANDLE m_hMutex;
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnFindProcess();
	afx_msg void OnBnClickedBtnKillProcess();
	afx_msg void OnBnClickedBtnKillThread();
	afx_msg void OnBnClickedBtnFindThread();
	afx_msg void OnBnClickedCancel();
};
