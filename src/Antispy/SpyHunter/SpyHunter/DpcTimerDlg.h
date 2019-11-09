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

// CDpcTimerDlg 对话框

class CDpcTimerDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CDpcTimerDlg)

public:
	CDpcTimerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDpcTimerDlg();

// 对话框数据
	enum { IDD = IDD_DPC_TIMER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void GetDpcTimer();
	void GetDriver();
	CString GetDriverPathByCallbackAddress(ULONG pCallback);
	void InsertDpcTimers();
	CSortListCtrl m_list;
	CString m_status;
	vector<DPC_TIMER> m_DpcTimerVector;
	CConnectDriver m_driver;
	vector<DRIVER_INFO> m_CommonDriverList;
	CommonFunctions m_Functions;
	CListDrivers m_clsDrivers;
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDpctimer33087();
	afx_msg void OnDpctimer33088();
	afx_msg void OnDpctimer33089();
	afx_msg void OnDpctimer33091();
	afx_msg void OnDpctimer33092();
	afx_msg void OnDpctimerVerifySin();
	afx_msg void OnExportToText();
	afx_msg void OnExportToExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDpctimerDisasm();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
