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
#include "ProcessImageInfoDlg.h"
#include "DllModuleDlg.h"
#include "HandleDlg.h"
#include "ThreadDlg.h"
#include "ProcessMemoryDlg.h"
#include "ProcessWndDlg.h"
#include "HotKeyDlg.h"
#include "ProcessTimerDlg.h"
#include "ProcessPrivilegesDlg.h"

// CProcessInfoDlg 对话框

class CProcessInfoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CProcessInfoDlg)

public:
	CProcessInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessInfoDlg();

// 对话框数据
	enum { IDD = IDD_PROCESS_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void InitProcInfoWndTab(CDialog *pDlg);
	void EnumProcessPrivileges(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessTimers(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessHotKeys(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessWnds(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessMemorys(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessThreads(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessHandles(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessModules(ULONG nPid, ULONG pEprocess, CString szImage);
	afx_msg LRESULT MsgEnumTimer(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumModules(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumHandles(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumThreads(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumMemory(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumWindows(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumHotKeys(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumPrivileges(WPARAM wParam, LPARAM lParam);
public:
	ULONG m_nPid;
	ULONG m_pEprocess;
	CString m_szImageName;
	CString m_szPath;
	CString m_szParentImage;
	CTabCtrl m_tab;
	CProcessImageInfoDlg m_ProcessImageInfoDlg;
	CDialog *m_pBelowDialog[20];				// 下面的窗口
	ULONG m_nBelowDialog;						// 总共有几个窗口
	int m_nCurSel;								// 下面的窗口现在选的是哪
	CDllModuleDlg m_ModuleDlg;					// 下面的模块对话框
	CHandleDlg m_HandleDlg;						// 下面的句柄对话框
	CThreadDlg m_ThreadDlg;						// 下面的线程对话框
	CProcessMemoryDlg m_MemoryDlg;				// 下面的内存对话框
	CProcessWndDlg	m_WndDlg;					// 下面的窗口对话框
	CHotKeyDlg	m_HotKeyDlg;					// 下面的热键对话框
	CProcessTimerDlg m_TimerDlg;				// 下面的定时器对话框
	CProcessPrivilegesDlg m_PrivilegeDlg;		// 下面的权限对话框
	CString m_szStatus;
};
