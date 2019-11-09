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
#include "Function.h"
#include "SignVerifyDlg.h"
#include <vector>
#include "ProcessFunc.h"
#include "ListProcess.h"
#include "listmodules.h"
#include "DllModuleDlg.h"

// CModuleInfoDlg 对话框

class CModuleInfoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CModuleInfoDlg)

public:
	CModuleInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModuleInfoDlg();

// 对话框数据
	enum { IDD = IDD_MODULE_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDllmoduleRefresh();
	BOOL ListProcessModules();
	void GetDllModulesInfo();
	VOID ListAllModules();
	VOID ListMicrosoftModules();
	BOOL IsModuleHide(MODULE_INFO item);
	CString GetModuleName(MODULE_INFO item);
public:
	CSortListCtrl m_list;
	DWORD m_uPid;
	DWORD m_pEprocess;
	CConnectDriver m_Driver;
	vector<MODULE_INFO> m_VadRootModuleVector;
	vector<MODULE_INFO> m_PebModuleVector;
	BOOL m_bShowMicroModule;
	CommonFunctions m_Functions;
	CSignVerifyDlg m_SignVerifyDlg;
	ULONG m_nModuleCnt;
	ULONG m_nNotMicro;
	ULONG m_nHideCnt;
	CImageList m_ModuleImageList;
	CString m_szText;
	afx_msg void OnNotShowMicromodule();
	afx_msg void OnUpdateNotShowMicromodule(CCmdUI *pCmdUI);
	afx_msg void OnCopyDllMemory();
	afx_msg void OnUnloadModule();
	afx_msg void OnUnloadAllModules();
	afx_msg void OnDeleteModule();
	afx_msg void OnCheckSign();
	afx_msg void OnCheckAllSign();
	afx_msg void OnCopyModuleName();
	afx_msg void OnCopyModulePath();
	afx_msg void OnCheckModuleAttribute();
	afx_msg void OnLocationModule();
	afx_msg void OnSearchModuleGoogl();
	afx_msg void OnExportText();
	afx_msg void OnExceptExcel();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnCloseWindow(WPARAM wParam, LPARAM lParam);  
	VOID SortItem();
	BOOL IsNotSignItem(ULONG nItem);
	HICON m_hModuleIcon;
	CProcessFunc m_ProcessFunc;
	afx_msg void OnUnloadAndDeleteModule();
	void PrintDll(HANDLE hProcess, LIST_ENTRY *LdrpHashTable);
	int ListModuleByLdrpHashTable();
	LIST_ENTRY * GetHashTableAddress();
	VOID SortByBase();
	CString m_szProcessPath;
};
