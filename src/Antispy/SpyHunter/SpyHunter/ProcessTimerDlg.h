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
#include "Function.h"
#include <vector>
#include "afxwin.h"
#include "ListProcess.h"
#include "ListTimer.h"
#include "ListDriver.h"

using namespace std;

typedef struct _TIMER_MODULE_INFO_
{
	ULONG nBase;
	ULONG nSize;
	CString szModule;
}TIMER_MODULE_INFO, *PTIMER_MODULE_INFO;

// CProcessTimerDlg 对话框

class CProcessTimerDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CProcessTimerDlg)

public:
	CProcessTimerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessTimerDlg();

// 对话框数据
	enum { IDD = IDD_PROCESS_TIMER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	VOID EnumProcessTimers(ULONG *nCnt = NULL);
	CString GetModulePathByCallbackAddress(DWORD dwAddress);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimerRefresh();
	afx_msg void OnTimerShowAllTimer();
	afx_msg void OnTimerRemove();
	afx_msg void OnTimerEditTimer();
	afx_msg void OnTimerExportText();
	afx_msg void OnTimerExportExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateTimerShowAllTimer(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	VOID InsertTimers();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimerCheckSign();
	afx_msg void OnTimerCheckAttribute();
	afx_msg void OnTimerLocationExplorer();
	afx_msg void OnEnKillfocusEdit();
	CString GetDriverPath(ULONG pCallback);
	afx_msg void OnTimerDisassemble();
	VOID SortByEprocess();
	PROCESS_INFO FindProcessItem(vector<PROCESS_INFO> &vectorProcess, ULONG pEprocess);
	BOOL GetProcessModuleBySnap(DWORD dwPID);
	void SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd);

public:
	CSortListCtrl m_list;
	CString m_szImage;
	DWORD m_dwPid;
	DWORD m_dwEprocess;
	vector<TIMER_INFO> m_vectorTimersTemp;
	vector<TIMER_INFO> m_vectorTimers;
	ULONG m_nCnt;
	CommonFunctions m_Functions;
	BOOL m_bShowAllTimer;
	CEdit m_edit;
	ULONG m_nPreItem;
	vector<DRIVER_INFO> m_vectorDrivers;
	list<TIMER_MODULE_INFO> m_TimerModuleList;
	CListTimers m_clsTimers;
	CListDrivers m_clsDrivers;
	HWND m_hDlgWnd;
};
