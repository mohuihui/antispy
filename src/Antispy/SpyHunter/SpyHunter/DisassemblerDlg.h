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
#include "afxcmn.h"
#include "ConnectDriver.h"
#include "Function.h"
#include "ListProcess.h"

// CDisassemblerDlg 对话框

class CDisassemblerDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CDisassemblerDlg)

public:
	CDisassemblerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDisassemblerDlg();

// 对话框数据
	enum { IDD = IDD_DISASSEMBLER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL DumpMemoryToRing3(DWORD Address, DWORD Size = 0x1000, DWORD Pid = 0, DWORD pEprocess = 0);
	void Disassembly(PBYTE pBuffer, ULONG Address, ULONG nSize);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL IsImediate();

public:
	BOOL m_bKernelMode;
	CString m_szAddress;
	CString m_szSize;
	CString m_szPid;
	CString m_szStaticAddress;
	CString m_szStaticSize;
	CString m_szStaticPid;
	CButton m_btnKernelMode;
	DWORD m_dwSize;
	DWORD m_dwAddress;
	DWORD m_dwPid;
	CListCtrl m_list;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	CImageList m_ProImageList;
	afx_msg void OnBnClickedCheckKernelAddress();
	afx_msg void OnDisemblyReflash();
	afx_msg void OnDisemblyImediate();
	afx_msg void OnDisemblyCopyFullrow();
	afx_msg void OnDisemblyCopyAddress();
	afx_msg void OnDisemblyCopyHex();
	afx_msg void OnDisemblyCopyCode();
	afx_msg void OnDisemblyLowercase();
	afx_msg void OnDisemblyExportText();
	afx_msg void OnDisemblyExportExcel();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdateDisemblyLowercase(CCmdUI *pCmdUI);
	CSortListCtrl m_ProcessList;
	afx_msg void OnNMRclickProcessList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProcessRefresh();
	void ListProcess();
	afx_msg void OnLvnItemchangedProcessList(NMHDR *pNMHDR, LRESULT *pResult);
};
