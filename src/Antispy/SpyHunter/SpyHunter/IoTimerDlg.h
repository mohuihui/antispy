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

// CIoTimerDlg 对话框

class CIoTimerDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CIoTimerDlg)

public:
	CIoTimerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CIoTimerDlg();

// 对话框数据
	enum { IDD = IDD_IO_TIMER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnIotimerRefresh();
	afx_msg void OnIotimerStop();
	afx_msg void OnIotimerStart();
	afx_msg void OnIotimerRemove();
	afx_msg void OnIotimerRemoveAll();
	afx_msg void OnIotimerDisEntry();
	afx_msg void OnIotimerShuxing();
	afx_msg void OnIotimerLocateModule();
	afx_msg void OnIotimerVerifySign();
	afx_msg void OnIotimerText();
	afx_msg void OnIotimerExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	void GetIoTimers();
	void GetDriver();
	void InsertIoTimers();
	CString GetDriverPathByCallbackAddress(ULONG pCallback);
public:
	CSortListCtrl m_list;
	vector<IO_TIMERS> m_IoTimerVector;
	CConnectDriver m_driver;
	vector<DRIVER_INFO> m_CommonDriverList;
	CommonFunctions m_Functions;
	CString m_status;
	CListDrivers m_clsDrivers;
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
