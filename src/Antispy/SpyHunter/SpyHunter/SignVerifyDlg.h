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

#include <list>

using namespace std;

// CSignVerifyDlg 对话框

class CSignVerifyDlg : public CDialog
{
	DECLARE_DYNAMIC(CSignVerifyDlg)

public:
	CSignVerifyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSignVerifyDlg();

// 对话框数据
	enum { IDD = IDD_SIGNVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	BOOL OpenCryptSvc();
	afx_msg void OnBnClickedButton();
	afx_msg LRESULT SiginVerifyOver(WPARAM wParam, LPARAM lParam);  

public:
	CString m_path;
	CSortListCtrl* m_pList;
	HWND m_hWinWnd;
	BOOL m_bSingle;	// 是否验单个进程
	ULONG m_nPathSubItem;
	list<ULONG> m_NotSignItemList;
	list<ULONG> m_NotSignDataList;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
