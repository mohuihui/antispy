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
#include "ConnectDriver.h"

// CModifyProtectDlg 对话框

class CModifyProtectDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyProtectDlg)

public:
	CModifyProtectDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModifyProtectDlg();

// 对话框数据
	enum { IDD = IDD_MODIFY_PROTECTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_cobmo;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedModifyOk();
	CConnectDriver m_Driver;
	ULONG m_pid;
	ULONG m_pEprocess;
	ULONG m_base;
	ULONG m_size;
	HWND m_hMemoryWnd;
	afx_msg void OnBnClickedModifyCancel();
};
