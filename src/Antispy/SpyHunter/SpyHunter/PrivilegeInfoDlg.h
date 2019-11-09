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
#include "..\\..\\Common\Common.h"
#include "ConnectDriver.h"
#include <vector>
#include "Function.h"
using namespace std;

// CPrivilegeInfoDlg 对话框

class CPrivilegeInfoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CPrivilegeInfoDlg)

public:
	CPrivilegeInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPrivilegeInfoDlg();

// 对话框数据
	enum { IDD = IDD_PRIVILEGE_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	VOID GetProcessPrivileges();
	VOID SortByPrivilegeName();
	VOID InsertPrivilegesItem();
public:
	CSortListCtrl m_list;
	DWORD m_dwPid;
	DWORD m_process;
	CConnectDriver m_driver;
	vector<LUID_AND_ATTRIBUTES> m_PrivilegesVector;
	CommonFunctions m_Functions;
	CString m_szText;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPrivilegeRefresh();
	afx_msg void OnPrivilegeEnable();
	afx_msg void OnPrivilegeDisable();
	afx_msg void OnPrivilegeText();
	afx_msg void OnPrivilegeExcel();
};
