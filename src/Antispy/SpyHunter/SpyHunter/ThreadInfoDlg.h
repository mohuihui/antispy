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
#include "ThreadDlg.h"
#include "..\\..\\Common\Common.h"
#include <vector>
#include "ListModules.h"
#include "ListDriver.h"
using namespace std;
// CThreadInfoDlg 对话框

class CThreadInfoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CThreadInfoDlg)

public:
	CThreadInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CThreadInfoDlg();

// 对话框数据
	enum { IDD = IDD_THREAD_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);

public:
	CSortListCtrl m_list;
	CConnectDriver m_Driver;
	DWORD m_uPid;
	DWORD m_pEprocess;
	vector<THREAD_INFO> m_ThreadVector;
	list<DWORD> m_normaTidList;
	DWORD m_nModuleCnt;
	DWORD m_nHideCnt;
	DWORD m_nNotHaveModule;
	BOOL m_bShowDeadThread;
	CommonFunctions m_Functions;
	CString m_szState;
	vector<DRIVER_INFO> m_CommonDriverList;
	CListModules m_clsListModules;
	vector<MODULE_INFO> m_vectorModules;
	CListDrivers m_clsDrivers;
	afx_msg void OnThreadRefresh();
	afx_msg void OnKillThread();
	afx_msg void OnForceKillThread();
	afx_msg void OnSuspendThread();
	afx_msg void OnResumeThread();
	afx_msg void OnForceResumeThread();
	afx_msg void OnCheckSign();
	afx_msg void OnCheckAttribute();
	afx_msg void OnLocationExplorer();
	afx_msg void OnExportText();
	afx_msg void OnExport();
	VOID SortByTid();
	BOOL ListProcessThread();
	BOOL IsThreadHide(DWORD tid);
	BOOL Ring3ListProcessThreads(DWORD dwOwnerPID);
	VOID GetProcessThreadInfo();
	CString GetModulePathByThreadStartAddress(DWORD dwBase);
	VOID InsertThreadIntoList(THREAD_INFO ThreadInfo);
	BOOL SuspenedOrResume(ULONG pThread, BOOL bSuspend);
	BOOL ForceSuspenedOrResume(ULONG pThread, BOOL bSuspend);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	ULONG IsMulitsThreadSuspened();
	BOOL IsThreadSuspened(ULONG pThread);
	void GetDriver();
	CString GetDriverPath(ULONG pCallback);
};
