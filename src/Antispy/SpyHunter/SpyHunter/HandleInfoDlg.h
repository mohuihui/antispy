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
#include "ConnectDriver.h"
#include "..\\..\\Common\Common.h"
#include "Function.h"
#include <vector>

using namespace std;

// CHandleInfoDlg 对话框

class CHandleInfoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CHandleInfoDlg)

public:
	CHandleInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHandleInfoDlg();

// 对话框数据
	enum { IDD = IDD_HANDLE_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CSortListCtrl m_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHandleRefresh();
	afx_msg void OnHandleClose();
	afx_msg void OnHandleForceClose();
	afx_msg void OnHandleExportText();
	afx_msg void OnHandleExportExcel();
	void KillHandle(BOOL bForce);
	VOID SortByHandle();
	VOID GetProcessHandleInfo();
	BOOL ListProcessHandle();
	VOID InsertHandleIntoList(HANDLE_INFO HandleInfo);
	vector<HANDLE_INFO> m_HandleVector;
	DWORD m_dwPid;
	DWORD m_pEprocess;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	CString m_szState;
};
