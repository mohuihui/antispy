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
#include "ConnectDriver.h"
#include <list>
#include "Function.h"
#include "ListProcess.h"

using namespace std;

// CWindowDlg 对话框

class CWindowDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CWindowDlg)

public:
	CWindowDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWindowDlg();

// 对话框数据
	enum { IDD = IDD_WINDOW_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWindowRefresh();
	afx_msg void OnWindowShowAllProcess();
	afx_msg void OnWindowShow();
	afx_msg void OnWindowHide();
	afx_msg void OnWindowMax();
	afx_msg void OnWindowMin();
	afx_msg void OnWindowActive();
	afx_msg void OnWindowFreeze();
	afx_msg void OnWindowClose();
	afx_msg void OnWindowTopAll();
	afx_msg void OnWindowCancelTop();
	afx_msg void OnWindowText();
	afx_msg void OnWindowExcel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	VOID EnumProcessWnds();
	void InsertAllWnd();
//	void InsertOneWnd(PROCESS_WND_INFO item, CString szImageName);
	afx_msg void OnUpdateWindowShowAllProcess(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	void EnableWindw(BOOL bEnable);
	void ShowWindw(int nCmdShow);

public:
	CSortListCtrl m_list;
	ULONG m_dwPid;
	CString m_szImage;
	CConnectDriver m_driver;
//	list<PROCESS_WND_INFO> m_WndList;
	CommonFunctions m_Functions;
	BOOL m_bShowAllProcess;
	CString m_szText;
};
