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
#include "FileFunc.h"
#include "afxcmn.h"
#include "Function.h"
#include "ConnectDriver.h"

// CMbrDlg dialog

class CMbrDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CMbrDlg)

public:
	CMbrDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMbrDlg();

// Dialog Data
	enum { IDD = IDD_MBR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void InitPhysicDrives();
	PVOID ReadMBR();
	CComboBox m_Drivers;
	CFileFunc m_FileFunc;
	CommonFunctions m_Functions;
	CConnectDriver m_Driver;
	CListCtrl m_list;
	CString m_szStatus;
	afx_msg void OnBnClickedBtnReadMbr();
	afx_msg void OnBnClickedBtnSaveMbr();
	afx_msg void OnBnClickedBtnRestoreMbr();
	afx_msg void OnBnClickedBtnRestoreDefaultMbr();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void RestoreMbrFormBackupFile(CString szPath);
	HANDLE GetDiskHandle();
	PVOID GetDefaultMBR(DWORD* dwSize);
	BOOL CheckMBR();
	BOOL WriteMBR(PVOID pBuffer, ULONG nWriteBytes);
	afx_msg void OnDisemblyExportText();
	afx_msg void OnDisemblyExportExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
};
