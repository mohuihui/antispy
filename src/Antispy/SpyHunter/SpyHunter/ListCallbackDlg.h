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

// CListCallbackDlg 对话框

class CListCallbackDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CListCallbackDlg)

public:
	CListCallbackDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CListCallbackDlg();

// 对话框数据
	enum { IDD = IDD_CALLBACK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CSortListCtrl m_list;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void GetCallbacks();
	void InsertCallbackItem();
	void GetDriver();
	CString GetDriverPathByCallbackAddress(ULONG pCallback);
	vector<CALLBACK_INFO> m_CallbackVector;
	CConnectDriver m_driver;
	vector<DRIVER_INFO> m_CommonDriverList;
	CommonFunctions m_Functions;
	CString m_status;
	CListDrivers m_clsDrivers;
	afx_msg void OnCallbackRefresh();
	afx_msg void OnCallbackRemove();
	afx_msg void OnCallbackShuxing();
	afx_msg void OnCallbackLocateModule();
	afx_msg void OnCallbackCheckSign();
	afx_msg void OnCallbackText();
	afx_msg void OnCallbackExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCallbackRemoveAll();
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCallbackDisasm();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
