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
#include "..\\..\\Common\Common.h"
#include <vector>
#include "Function.h"
#include "ListProcess.h"
#include "ListWnd.h"

using namespace std;
// CProcessWndDlg 对话框

class CProcessWndDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CProcessWndDlg)

public:
	CProcessWndDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessWndDlg();

// 对话框数据
	enum { IDD = IDD_PROCESS_WND_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	VOID EnumProcessWnds(ULONG *nCnt = NULL);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void AddWndItem(WND_INFO item, CString szImageName);
	afx_msg void OnWindowRefresh();
	afx_msg void OnWindowShowAllProcess();
	afx_msg void OnUpdateWindowShowAllProcess(CCmdUI *pCmdUI);
	afx_msg void OnWindowShow();
	afx_msg void OnWindowHide();
	afx_msg void OnWindowMax();
	afx_msg void OnWindowMin();
	afx_msg void OnWindowActive();
	afx_msg void OnWindowFreeze();
	afx_msg void OnWindowClose();
	afx_msg void OnWindowTopAll();
	afx_msg void OnWindowText();
	afx_msg void OnWindowExcel();
	afx_msg void OnWindowCancelTop();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	void ShowWindw(int nCmdShow);
	void EnableWindw(BOOL bEnable);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd);
public:
	CSortListCtrl m_list;
	ULONG m_dwPid;
	CString m_szImage;
	vector<WND_INFO> m_vectorWndsTemp;
	vector<WND_INFO> m_vectorWnds;
	CommonFunctions m_Functions;
	BOOL m_bShowAllProcess;
	CListWnds m_clsWnds;
	ULONG m_nCnt;
	HWND m_hDlgWnd;
};
