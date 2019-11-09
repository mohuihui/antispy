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
#include "ListHandle.h"

using namespace std;

// CHandleDlg 对话框

class CHandleDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CHandleDlg)

public:
	CHandleDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHandleDlg();

// 对话框数据
	enum { IDD = IDD_HANDLE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	VOID EnumProcessHandles(ULONG *nCnt = NULL);
	VOID AddHandleItem(HANDLE_INFO HandleInfo);
	CString GetHandleObjectType(ULONG Object, ULONG* uTypeIndex);
public:
	CSortListCtrl m_list;
	vector<HANDLE_INFO> m_HandleVector;
	DWORD m_dwPid;
	DWORD m_pEprocess;
	CString m_szImage;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	CListHandles m_clsHandles;
	ULONG m_nCnt;
	HWND m_hDlgWnd;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHandleRefresh();
	afx_msg void OnHandleClose();
	afx_msg void OnHandleForceClose();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHandleExportText();
	afx_msg void OnHandleExportExcel();
	void KillHandle(BOOL bForce);
	void SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
