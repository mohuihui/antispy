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
#include "Function.h"
#include "afxwin.h"
#include "Function.h"
#include "SignVerifyDlg.h"
#include "ConnectDriver.h"
#include <vector>
#include "ListProcess.h"

using namespace std;

// CFindModuleDlg 对话框

typedef struct _HANDLE_INFO_EX_
{
	ULONG Handle;
	ULONG Object;
	ULONG dwPid;
	ULONG dwEprocess;
}HANDLE_INFO_EX, *PHANDLE_INFO_EX;

typedef struct _DLL_INFO_EX_
{
	ULONG dwBase;
	ULONG dwPid;
	ULONG dwEprocess;
}DLL_INFO_EX, *PDLL_INFO_EX;

class CFindModuleDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CFindModuleDlg)

public:
	CFindModuleDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFindModuleDlg();

// 对话框数据
	enum { IDD = IDD_SEARCH_DLL_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CSortListCtrl m_list;
	afx_msg void OnBnClickedFindButton();
	CString m_szFindModuleName;
	CommonFunctions m_Functions;
	CImageList m_ModuleImageList;
	CString m_szStatus;
	CEdit m_edit;
	afx_msg void OnFindRefresh();
	afx_msg void OnFindDeleteModule();
	afx_msg void OnFindUnload();
	afx_msg void OnFindUnloadAll();
	afx_msg void OnFindCopyModuleName();
	afx_msg void OnFindCopyModulePath();
	afx_msg void OnFindCheckSign();
	afx_msg void OnFindCheckShuxing();
	afx_msg void OnFindInExplorer();
	afx_msg void OnFindText();
	afx_msg void OnFindExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void EnumProc();
	void EnumModules(PROCESS_INFO info);
	void EnumHandles(PROCESS_INFO info);
	void PopupDllMenu();
	void PopupHandleMenu();
	HICON m_hModuleIcon;
	int m_nRadio;
	HANDLE m_hThread;
	ULONG m_nTotalCnt;
	afx_msg void OnHandleClose();
	afx_msg void OnHandleForceClose();
	void KillHandle(BOOL bForce);
	vector<HANDLE_INFO_EX> m_vectorHandles;
	CConnectDriver m_Driver;
	vector<DLL_INFO_EX> m_vectorDLLs;
	afx_msg LRESULT OnUpdateData(WPARAM wParam, LPARAM lParam);  
};
