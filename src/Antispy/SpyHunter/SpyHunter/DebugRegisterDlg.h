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
#include <vector>
#include "ConnectDriver.h"
#include "Function.h"
#include "..\\..\\Common\Common.h"
#include "ListDriver.h"
using namespace std;


// CDebugRegisterDlg 对话框

class CDebugRegisterDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CDebugRegisterDlg)

public:
	CDebugRegisterDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDebugRegisterDlg();

// 对话框数据
	enum { IDD = IDD_DEBUG_REGISTER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void GetDebugRegister();
	CSortListCtrl m_list;
	CConnectDriver m_driver;
	vector<DRIVER_INFO> m_CommonDriverList;
	CommonFunctions m_Functions;
	CListDrivers m_clsDrivers;
	afx_msg void OnDebugRegisterRefresh();
	afx_msg void OnDebugRegisterRecover();
	afx_msg void OnDebugregisterShuxing();
	afx_msg void OnDebugregisterLocation();
	afx_msg void OnDebugregisterVerifySigin();
	afx_msg void OnDebugRegText();
	afx_msg void OnDebugRegExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	void InsertDrbugRegister(ULONG n, PDEBUG_REGISTER Registers);
	void InsertDrxRegister(ULONG i, ULONG Drx, ULONG Dr7);
	VOID GetBreakPointLengthAndRW(ULONG i, ULONG Dr7, int* nLen, int* nWR);
	void GetDriver();
	CString GetDriverPathByCallbackAddress(ULONG pCallback);
	afx_msg void OnDebugregisterRecoverAll();
	afx_msg void OnDebugregisterDisasm();
	CString m_szStatus;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
