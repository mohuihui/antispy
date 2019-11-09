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
#include <vector>
#include "Function.h"
#include "ListPrivileges.h"

using namespace std;

// CProcessPrivilegesDlg 对话框

class CProcessPrivilegesDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CProcessPrivilegesDlg)

public:
	CProcessPrivilegesDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessPrivilegesDlg();

// 对话框数据
	enum { IDD = IDD_PROCESS_PRIVILEGES_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	virtual BOOL OnInitDialog();
	VOID GetProcessPrivileges(ULONG *nEnable = NULL, ULONG *nDisable = NULL);
	VOID SortByPrivilegeName();
	VOID InsertPrivilegesItem();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPrivilegeRefresh();
	afx_msg void OnPrivilegeEnable();
	afx_msg void OnPrivilegeDisable();
	afx_msg void OnPrivilegeText();
	afx_msg void OnPrivilegeExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	void SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd);
	DWORD m_dwPid;
	DWORD m_process;
	CString m_szImage;
	vector<LUID_AND_ATTRIBUTES> m_vectorPrivileges;
	CommonFunctions m_Functions;
	CListPrivileges m_clsPrivileges;
	ULONG m_nEnabled;
	ULONG m_nDisabled;
	HWND m_hDlgWnd;
};