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
#include "Function.h"

// CRegFindDlg dialog

class CRegFindDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CRegFindDlg)

public:
	CRegFindDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegFindDlg();

// Dialog Data
	enum { IDD = IDD_FIND_REG_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	BOOL m_bKeys;
	BOOL m_bValues;
	BOOL m_bData;
	BOOL m_bMachCase;
	BOOL m_bMachWholeString;
	CString m_szFindWhat;
	CString m_szFindWhatUpper;		// ´óÐ´µÄËÑË÷ÄÚÈÝ
	CString m_szSearchInKey;
	int m_nRadio;
	ULONG m_nCnt;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNcDestroy();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	void ChangeKongjianStatus(BOOL bFuck);
	void SearchRegistry();
	HKEY GetRootKey(CString szKey);
	afx_msg void OnBnClickedCheckKey();
	afx_msg void OnBnClickedCheckValues();
	afx_msg void OnBnClickedCheckData();
	void EnumKeys(HKEY hRoot, CString szSubKey);
	CString GetRootKeyString(HKEY hRoot);
	void InsertKeys(HKEY hRoot, CString szSubKey, CString szSubSubKey);
	void InsertVlaues(HKEY hRoot, CString szSubKey, CString szValue);
	void InsertData(HKEY hRoot, CString szSubKey, CString szValue, DWORD dwType, PBYTE pData, DWORD dwDataLen);
	CString m_szSearchResults;
	afx_msg LRESULT SearchFinished(WPARAM,LPARAM);
	afx_msg LRESULT UpdateUI(WPARAM,LPARAM);
	BOOL m_bStop;
	HANDLE m_hThread;
	afx_msg void OnClose();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnAutorunJmpToReg();
	CommonFunctions m_Functions;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio1();
};
