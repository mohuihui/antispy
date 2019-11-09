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
#include "Registry.h"
#include "Function.h"
#include <list>
using namespace std;

typedef struct _FILE_PENDING_INFO 
{
	WCHAR szFile[MAX_PATH];
	WCHAR szNewFile[MAX_PATH];
}FILE_PENDING_INFO, *PFILE_PENDING_INFO;

// CFileRestartDeleteDlg dialog

class CFileRestartDeleteDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CFileRestartDeleteDlg)

public:
	CFileRestartDeleteDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileRestartDeleteDlg();

// Dialog Data
	enum { IDD = IDD_FILE_RESTART_DELETE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CSortListCtrl m_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void ViewFileRestartDeleteInfo();
	void GetPendingFileRenameOperations();
	CRegistry m_Registry;
	CommonFunctions m_Functions;
	CString m_szStatus;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPendngRefresh();
	afx_msg void OnPendngDelete();
	afx_msg void OnPendngDeleteAll();
	afx_msg void OnPendngGotofile();
	afx_msg void OnPendngLookShuxing();
	afx_msg void OnPendngGotoNewFile();
	afx_msg void OnPendngLookNewFileShuxing();
	afx_msg void OnPendingExportText();
	afx_msg void OnPendingExportExcel();
	void ModifyValue(PVOID pData, ULONG DataSize);
	CString TerPath(CString szPath);
	list<FILE_PENDING_INFO> m_infoVector;
};
