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
#include "ListProcess.h"

// CNotSignModuleDlg 对话框

class CNotSignModuleDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CNotSignModuleDlg)

public:
	CNotSignModuleDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNotSignModuleDlg();

// 对话框数据
	enum { IDD = IDD_NOT_SIGN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	void EnumNoSignModules();
	CSortListCtrl m_list;
	CString m_szStatus;
	HANDLE m_hThread;
	afx_msg void OnClose();
	CommonFunctions m_Functions;
	CImageList m_ModuleImageList;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL OpenCryptSvc();
	HICON m_hIcon;
	afx_msg void OnNosignmoduleRefresh();
	afx_msg void OnNosignmoduleDeleteModule();
	afx_msg void OnNosignmoduleUnload();
	afx_msg void OnNosignmoduleUnloadAll();
	afx_msg void OnNosignmoduleCopyModuleName();
	afx_msg void OnNosignmoduleCopyModulePath();
	afx_msg void OnNosignmoduleCheckShuxing();
	afx_msg void OnNosignmoduleInExplorer();
	afx_msg void OnNosignmoduleText();
	afx_msg void OnNosignmoduleExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnUpdateData(WPARAM wParam, LPARAM lParam);  
	void EnumNoSignModulesProc();
	BOOL m_bExit;
};
