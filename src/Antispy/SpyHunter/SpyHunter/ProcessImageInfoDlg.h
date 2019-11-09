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
#include "Function.h"
#include "ProcessFunc.h"

// CProcessImageInfoDlg dialog

class CProcessImageInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CProcessImageInfoDlg)

public:
	CProcessImageInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProcessImageInfoDlg();

// Dialog Data
	enum { IDD = IDD_PROCESS_IMAGE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	void ShowInformation();
	void SetProcessIcon();
	virtual BOOL OnInitDialog();
	void GetProcessCompanyAndDescribe();
	void GetFileVersion();
	void GetFilePath();
	void GetFileSize();
	void GetFileTimes();
	BOOL GetProcessCmdLine();
	BOOL GetProcessCurrentDirectory();
	BOOL GetPebAddress();
	void GetProcessStartTime();
	CString m_szPath;
	DWORD m_dwPid;
	CStatic m_ProcIcon;
	CString m_szFileDescribe;
	CString m_szFileCompany;
	CommonFunctions m_Functions;
	CString m_szVersion;
	CString m_szProcPath;
	CString m_szFileSize;
	CString m_szTime;
	CString m_szCommandLine;
	CString m_szCurrentDirectory;
	CString m_szStartedTime;
	CString m_szPebAddress;
	CString m_szParent;
	CProcessFunc m_ProcessFuncs;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	void InitString();
	void FinishString();
};
