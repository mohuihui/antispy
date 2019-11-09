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
#include "ConnectDriver.h"
#include "Function.h"

// CAssemblyDlg 对话框

class CAssemblyDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CAssemblyDlg)

public:
	CAssemblyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAssemblyDlg();

// 对话框数据
	enum { IDD = IDD_ASSEMBLY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	BOOL DumpMemoryToRing3(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess);
	void Disassembly(PBYTE pBuffer, ULONG Address, ULONG nSize);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL IsImediate();
	afx_msg void OnDisemblyReflash();
	afx_msg void OnDisemblyImediate();
	afx_msg void OnDisemblyCopyFullrow();
	afx_msg void OnDisemblyCopyAddress();
	afx_msg void OnDisemblyCopyHex();
	afx_msg void OnDisemblyCopyCode();
	afx_msg void OnDisemblyExportText();
	afx_msg void OnDisemblyExportExcel();
	afx_msg void OnDisemblyLowercase();
	afx_msg void OnUpdateDisemblyLowercase(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	CString DisasmMemory(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess);
	CString DisassemblyString(PBYTE pBuffer, ULONG Address, ULONG nSize);
public:
	CListCtrl m_list;
	DWORD m_dwPid;
	DWORD m_dwEprocess;
	DWORD m_dwAddress;
	DWORD m_dwSize;
	CString m_szPid;
	CString m_szAddress;
	CString m_szSize;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	afx_msg void OnSize(UINT nType, int cx, int cy);
};