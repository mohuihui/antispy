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
#include "Function.h"
#include "ConnectDriver.h"
#include "afxwin.h"
#include <vector>
#include "MzfEdit.h"

using namespace std;

// CCommandDlg 对话框

class CCommandDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CCommandDlg)

public:
	CCommandDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCommandDlg();

// 对话框数据
	enum { IDD = IDD_COMMAND_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void ParseCommand();
	void ShowHelpInfomation();
	afx_msg LRESULT OnUpdateData(WPARAM wParam, LPARAM lParam);  
	void Unassemble();
	void DisplayMemory();
	PBYTE DumpMemory(DWORD Address, DWORD Size = 100, DWORD Pid = 0, DWORD pEprocess = 0);
	void ShowDD(PBYTE pBuffer, DWORD dwAddress, DWORD dwSize);
	void ShowDB(PBYTE pBuffer, DWORD dwAddress, DWORD dwSize);
	void ShowDW(PBYTE pBuffer, DWORD dwAddress, DWORD dwSize);
	void TrimSpace(CString &szString);
	void UpdateOutputEdit();
	CString m_szOutput;
	CString m_szCommand;
	LONG m_bCreateThread;
	CommonFunctions m_Function;
	CConnectDriver m_Driver;
	CMyEdit m_OutputEdit;
	vector<CString> m_CommandList;
	DWORD m_dwCurrentCommand;
	DWORD m_dwTotalCommand;
	DWORD m_bFirstUp;
};
