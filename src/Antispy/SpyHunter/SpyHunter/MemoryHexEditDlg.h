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
#include "ConnectDriver.h"
#include "Function.h"
#include "afxwin.h"
#include "ProcessFunc.h"

// CMemoryHexEditDlg dialog

class CMemoryHexEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CMemoryHexEditDlg)

public:
	CMemoryHexEditDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMemoryHexEditDlg();

// Dialog Data
	enum { IDD = IDD_MEMORY_HEX_EDIT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szAddress;
	CString m_szSize;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	DWORD m_dwPid;
	DWORD m_dwEprocess;
	DWORD m_dwAddress;
	DWORD m_dwSize;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	BOOL DumpMemoryToRing3(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess);
	CXTPHexEdit m_editHex;
	afx_msg void OnEnChangeHexEdit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CProcessFunc m_ProcessFunc;
	BOOL m_bReadOnly;
	afx_msg void OnBnClickedCheckReadOnly();
	CButton m_btnReadOnly;
};
