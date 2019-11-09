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
#include "..\\..\\Common\Common.h"
#include <vector>
#include <algorithm>
#include "ConnectDriver.h"
#include "RegCommon.h"
#include "ListProcess.h"

using namespace std;

typedef struct _LOCK_REG_PROCESS_INFO
{
	ULONG nHandle;
	ULONG nPid;
	ULONG pEprocess;
	CString szProcess;
}LOCK_REG_PROCESS_INFO, *PLOCK_REG_PROCESS_INFO;

// CRegUnlockDlg dialog

class CRegUnlockDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CRegUnlockDlg)

public:
	CRegUnlockDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegUnlockDlg();

// Dialog Data
	enum { IDD = IDD_REG_UNLOCK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void ShowLockedRegInformation(PLOCKED_REG_INFO pLockFileInfos);
	VOID SortByPid();
	BOOL GetProcessList();
	CImageList m_ImageList;
	CSortListCtrl m_list;
	PLOCKED_REG_INFO m_pRegLockedInfo;
	vector<LOCKED_REGS> m_vectorLockedInfo;
	vector<LOCK_REG_PROCESS_INFO> m_vectorProcessInfo;
	CListProcess m_clsListProcess;
	CString m_szCurrentUserKey;
	CRegCommon m_RegCommon;
	WCHAR* m_szRegKey;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	vector<PROCESS_INFO> m_vectorProcess;
	afx_msg void OnUnlockRefresh();
	afx_msg void OnUnlockUnlock();
	afx_msg void OnUnlockUnlockAll();
	afx_msg void OnUnlockKillProcess();
	afx_msg void OnUnlockJmpToProcess();
	afx_msg void OnUnlockExportText();
	afx_msg void OnUnlockExportExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
};
