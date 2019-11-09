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
#include <list>
using namespace std;
#include "Registry.h"
#include "Function.h"

// CUserDlg 对话框

class CUserDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CUserDlg)

public:
	CUserDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUserDlg();

// 对话框数据
	enum { IDD = IDD_USER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	CString m_szStatus;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	void NormalListUsers();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void ListUsers();
	BOOL IsHideUser(CString szName);
	list<CString> m_listNormalUsers;
	list<CString> m_listAllUsers;
	CRegistry m_Registry;
	afx_msg void OnUsersRefresh();
	afx_msg void OnUsersDelete();
	afx_msg void OnUserExportText();
	afx_msg void OnUserExportExcel();
	CommonFunctions m_Function;
};
