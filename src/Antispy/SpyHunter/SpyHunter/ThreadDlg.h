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
#include "ListModules.h"
#include <vector>
#include <tlhelp32.h>
#include "Function.h"
#include "ListThread.h"
#include "ListModules.h"
#include "ListDriver.h"

using namespace std;
// CThreadDlg 对话框

typedef enum _KTHREAD_STATE
{
	Initialized,
	Ready,
	Running,
	Standby,
	Terminated,
	Waiting,
	Transition,
	DeferredReady,
	GateWait
} KTHREAD_STATE, *PKTHREAD_STATE;

typedef enum _THREAD_TYPE_
{
	enumThreadNone,
	enumThreadNotHaveModule,
	enumThreadHide
}THREAD_TYPE;

class CThreadDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CThreadDlg)

public:
	CThreadDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CThreadDlg();

// 对话框数据
	enum { IDD = IDD_THREAD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL EnumThreads(ULONG *nCnt = NULL);
	VOID AddThreadItem(THREAD_INFO ThreadInfo);
	VOID SortByTid();
	CString GetModulePathByThreadStartAddress(DWORD dwBase);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnThreadRefresh();
	afx_msg void OnKillThread();
	afx_msg void OnSuspendThread();
	afx_msg void OnDissmThreadStartAddress();
	afx_msg void OnLocationExplorer();
	afx_msg void OnCheckAttribute();
	afx_msg void OnCheckSign();
	afx_msg void OnExport();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnExportText();
	BOOL IsThreadSuspened(ULONG pThread);
	BOOL SuspenedOrResume(ULONG pThread, BOOL bSuspend);
	afx_msg void OnResumeThread();
	CString GetDriverPath(ULONG pCallback);
	void SelectThread(DWORD dwTid);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd);
public:
	CSortListCtrl m_list;
	DWORD m_uPid;
	DWORD m_pEprocess;
	CString m_szImage;
	DWORD m_nThreadsCnt;
	CommonFunctions m_Functions;
	vector<THREAD_INFO> m_vectorThreadsAll;
	vector<THREAD_INFO> m_vectorThreads;
	vector<MODULE_INFO> m_vectorModules;
	vector<DRIVER_INFO> m_vectorDrivers;
	vector<ITEM_COLOR> m_vectorColor;
	CListThreads m_clsThreads;
	CListModules m_clsModules;
	CListDrivers m_clsDrivers;
	HWND m_hDlgWnd;
};
