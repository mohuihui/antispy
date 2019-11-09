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
#include "afxwin.h"
#include "ConnectDriver.h"
#include "Function.h"
#include "ProcessFunc.h"
#include "afxcmn.h"
#include <list>
#include "ListProcess.h"

using namespace std;

typedef struct _MODIFY_INFO
{
	DWORD dwAddress;
	BYTE btContent;
}MODIFY_INFO,*PMODIFY_INFO;

// CHexEditorDlg 对话框

class CHexEditorDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CHexEditorDlg)

public:
	CHexEditorDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHexEditorDlg();

// 对话框数据
	enum { IDD = IDD_HEX_EDITOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCheckReadOnly();
	afx_msg void OnBnClickedCheckKernelAddress();
	BOOL DumpMemoryToRing3(DWORD Address, DWORD Size = 0x1000, DWORD Pid = 0, DWORD pEprocess = 0);
	afx_msg void OnEnChangeHexEdit();
//	DWORD GetModifyAddress(PVOID pBuffer);
	DWORD GetModifyAddress(PVOID pBuffer, list<MODIFY_INFO> &ModifyList);
public:
	BOOL m_bReadOnly;
	BOOL m_bKernelAddress;
	CString m_szStaticAddress;
	CString m_szStaticSize;
	CString m_szAddress;
	CString m_szSize;
	CString m_szPid;
	DWORD m_dwSize;
	DWORD m_dwPid;
	CString m_szStaticPid;
	CXTPHexEdit m_editHex;
	DWORD m_dwAddress;
	CButton m_btnReadOnly;
	CButton m_btnKernelAddress;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	CProcessFunc m_ProcessFunc;
	PVOID m_pHexBuffer;
	CListCtrl m_ProcessList;
	CImageList m_ProImageList;
	afx_msg void OnProcessRefresh();
	afx_msg void OnNMRclickProcessList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedProcessList(NMHDR *pNMHDR, LRESULT *pResult);
	void ListProcess();
	afx_msg void OnBnClickedWriteMemory();
	BOOL CommWithDriver(PBYTE pBuffer, DWORD dwAddress, DWORD dwLen);
	BOOL ModifyUserAddress();
	BOOL ModifyKernelAddress();
};
