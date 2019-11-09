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
#include "ProcessMemoryDlg.h"
#include "ListModules.h"

// CMemoryInfoDlg 对话框

class CMemoryInfoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CMemoryInfoDlg)

public:
	CMemoryInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMemoryInfoDlg();

// 对话框数据
	enum { IDD = IDD_MEMORY_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CSortListCtrl m_list;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);

public:
	DWORD m_dwPid;
	DWORD m_pEprocess;
	CListModules m_clsListModules;
	vector<MODULE_INFO> m_vectorModules;
	vector<MEMORY_INFO> m_ProcessMemoryVector;
	CConnectDriver m_Driver;
	vector<PROTECT> m_ProtectTypeVector;
	CommonFunctions m_Functions;
	CString m_szText;
	afx_msg void OnMemoryRefresh();
	afx_msg void OnMemoryDump();
	afx_msg void OnMemoryDumpSelect();
	afx_msg void OnMemoryZeroMemory();
	afx_msg void OnMemoryChangeProtect();
	afx_msg void OnMemoryFreeMemory();
	afx_msg void OnMemoryText();
	afx_msg void OnMemoryExcel();
	BOOL ListProcessMemory();
	VOID SortByBase();
	CString GetMemoryProtect(ULONG Protect);
	CString GetMempryState(ULONG State);
	CString GetMempryType(ULONG Type);
	CString GetModuleImageName(DWORD dwBase);
	BOOL Dump(ULONG Base, ULONG Size, CString szFilePath);
	void GetProcessMemoryInfo();
	VOID InitMemoryProtect();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMemoryDisassemble();
	afx_msg void OnMemoryHexEdit();
};
