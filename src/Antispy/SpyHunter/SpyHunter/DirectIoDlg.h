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

using namespace std;

// CDirectIoDlg 对话框

class CDirectIoDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CDirectIoDlg)

public:
	CDirectIoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDirectIoDlg();

// 对话框数据
	enum { IDD = IDD_DIRECT_IO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void GetDirectIoProcess();
	CString GetDirectIoReason(DIRECT_IO_TYPE Type);
	void InsertDirectIoProcess();
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	vector<DIRECT_IO_PROCESS_INFO> m_DirectIoProcessVector;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDirectioRefresh();
	afx_msg void OnDirectioDenyDirectIo();
	afx_msg void OnDirectioKillProcess();
	afx_msg void OnDirectioKillAndDeleteProcess();
	afx_msg void OnDirectioLocalFile();
	afx_msg void OnDirectioShuxing();
	afx_msg void OnDirectioVerifySign();
	afx_msg void OnDirectioText();
	afx_msg void OnDirectioExcel();
	CString m_szStatus;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
