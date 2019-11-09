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
#include "ListProcess.h"
#include "ListDriver.h"
#include "ListThread.h"

// CSystemThreadDlg 对话框

class CSystemThreadDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CSystemThreadDlg)

public:
	CSystemThreadDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSystemThreadDlg();

// 对话框数据
	enum { IDD = IDD_SYSTEM_THREAD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	VOID SortByTid();
	VOID AddThreadItem(THREAD_INFO ThreadInfo);
	BOOL ListProcessThread();
	CString GetDriverPathByCallbackAddress(ULONG pCallback);
	BOOL GetSystemProcessInfo();
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL IsThreadSuspened(ULONG pThread);
	afx_msg void OnThreadRefresh();
	afx_msg void OnKillThread();
	afx_msg void OnSuspendThread();
	afx_msg void OnResumeThread();
	afx_msg void OnDissmThreadStartAddress();
	afx_msg void OnCheckSign();
	afx_msg void OnCheckAttribute();
	afx_msg void OnLocationExplorer();
	afx_msg void OnExportText();
	afx_msg void OnExport();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();

private:
	CSortListCtrl m_list;
	CString m_szState;
	vector<THREAD_INFO> m_vectorThreadsTemp;
	vector<THREAD_INFO> m_vectorThreads;
	DWORD m_nCnt;
	ULONG m_SystemProcessPid;
	ULONG m_SystemProcessEPROCESS;
	CommonFunctions m_Functions;
	vector<DRIVER_INFO> m_vectorDrivers;
	CListDrivers m_clsDrivers;
	CListThreads m_clsThreads;
	CListProcess m_clsEnumProcess;
};
