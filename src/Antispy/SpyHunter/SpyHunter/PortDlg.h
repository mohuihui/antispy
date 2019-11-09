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
#include "ListProcess.h"

using namespace std;

typedef struct _CONNECT_NET_INFO
{
	CString szProcess;
	ULONG pEprocess;
	PORT_TYPE nPortType;
	ULONG nConnectState;
	ULONG nLocalAddress;
	ULONG nLocalPort;
	ULONG nRemoteAddress;
	ULONG nRemotePort;
	ULONG nPid;
}CONNECT_NET_INFO, *PCONNECT_NET_INFO;

// CPortDlg 对话框

class CPortDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CPortDlg)

public:
	CPortDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPortDlg();

// 对话框数据
	enum { IDD = IDD_PORT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void EnumPort();
	void InsertPortInfo(PCOMMUNICATE_PORT pCp);
	CConnectDriver m_Driver;
	CSortListCtrl m_list;
	CString m_szPortStatus;
	CImageList m_imageList;
	vector<CONNECT_NET_INFO> m_portList;
	CommonFunctions m_Functions;
	CListProcess m_clsProcess;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPortRefresh();
	afx_msg void OnPortCopyAllRow();
	afx_msg void OnPortCopyRemoteAddress();
	afx_msg void OnPortWhois();
	afx_msg void OnPortShuxing();
	afx_msg void OnPortSign();
	afx_msg void OnPortExplorer();
	afx_msg void OnPortAntispy();
	afx_msg void OnPortText();
	afx_msg void OnPortExcel();
	afx_msg void OnPortKillProcess();
};
