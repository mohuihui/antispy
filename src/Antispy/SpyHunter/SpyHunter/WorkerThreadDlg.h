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
#include <vector>
#include "ConnectDriver.h"
#include "Function.h"
#include "..\\..\\Common\Common.h"
#include "ListDriver.h"
using namespace std;

// CWorkerThreadDlg 对话框

typedef enum _WORK_QUEUE_TYPE {
	CriticalWorkQueue,
	DelayedWorkQueue,
	HyperCriticalWorkQueue,
	MaximumWorkQueue
} WORK_QUEUE_TYPE;

class CWorkerThreadDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CWorkerThreadDlg)

public:
	CWorkerThreadDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWorkerThreadDlg();

// 对话框数据
	enum { IDD = IDD_WORKER_THREAD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void GetWorkerThread();
	void InsertWorkerThreadItems();
	void GetDriver();
	CString GetDriverPathByCallbackAddress(ULONG pCallback);
public:
	CSortListCtrl m_list;
	CString m_status;
	vector<WORKER_THREAD> m_WorkerThreadVector;	
	CConnectDriver m_driver;
	vector<DRIVER_INFO> m_CommonDriverList;
	CommonFunctions m_Functions;
	CConnectDriver m_Driver;
	CListDrivers m_clsDrivers;
	afx_msg void OnWorkerReflash();
	afx_msg void OnWorkSuspendThread();
	afx_msg void OnWorkResumeThread();
	afx_msg void OnWorkDisasm();
	afx_msg void OnWorkShuxing();
	afx_msg void OnWorkLocationModule();
	afx_msg void OnWorkVerifySign();
	afx_msg void OnWorkTimerText();
	afx_msg void OnWorkTimerExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL SuspenedOrResume(ULONG pThread, BOOL bSuspend);
	BOOL IsThreadSuspened(ULONG pThread);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
