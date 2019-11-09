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
#include "afxwin.h"
#include "ConnectDriver.h"
#include "DllModuleDlg.h"
#include "Function.h"
#include <vector>
#include "ProcessTimerDlg.h"
#include "ListProcess.h"
#include "ListDriver.h"
using namespace std;

// CTimerInfoDlg 对话框

class CTimerInfoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CTimerInfoDlg)

public:
	CTimerInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTimerInfoDlg();

// 对话框数据
	enum { IDD = IDD_TIMER_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimerRefresh();
	afx_msg void OnTimerShowAllTimer();
	afx_msg void OnTimerRemove();
	afx_msg void OnTimerEditTimer();
	afx_msg void OnTimerCheckSign();
	afx_msg void OnTimerCheckAttribute();
	afx_msg void OnTimerLocationExplorer();
	afx_msg void OnTimerExportText();
	afx_msg void OnTimerExportExcel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	VOID EnumProcessTimers();
	VOID InsertTimers();
	CString GetModulePathByCallbackAddress(DWORD dwAddress);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnEnKillfocusEdit();
	afx_msg void OnUpdateTimerShowAllTimer(CCmdUI *pCmdUI);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CString GetDriverPath(ULONG pCallback);
	void GetDriver();

	CSortListCtrl m_list;
	CString m_szImage;
	DWORD m_dwPid;
	DWORD m_dwEprocess;
	CConnectDriver m_driver;
	CString m_szText;
	CDllModuleDlg m_dllDlg;
//	vector<TIMER> m_TimerVector;
	CommonFunctions m_Functions;
	BOOL m_bShowAllTimer;
	ULONG m_nPreItem;
	CEdit m_edit;
	CListDrivers m_clsDrivers;
	vector<DRIVER_INFO> m_CommonDriverList;
	afx_msg void OnTimerDisassemble();
	BOOL GetProcessModuleBySnap(DWORD dwPID);
	PROCESS_INFO FindProcessItem(vector<PROCESS_INFO> &vectorProcess, ULONG pEprocess);
	VOID SortByEprocess();
	list<TIMER_MODULE_INFO> m_TimerModuleList;
};
