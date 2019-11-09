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
#include "Registry.h"
#include <vector>
using namespace std;

typedef struct _SPI_INFO 
{
	CString szName;
	CString szPath;
}SPI_INFO , *PSPI_INFO ;


// CSpiDlg dialog

class CSpiDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CSpiDlg)

public:
	CSpiDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpiDlg();

// Dialog Data
	enum { IDD = IDD_SPI_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void EnumNameSpace_Catalog5();
	void EnumProtocol_Catalog9();
	void EnumWinsockProviders();
	CString GetNameSpace_Catalog5NameAndModule(CString szKey, CString &szModule);
	CString GetProtocolCatalog9NameAndModule(CString szKey, CString &szModule);
	CString MByteToWChar(IN LPCSTR lpcszStr);
	void InsertSpiInfo();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSpiRefresh();
	afx_msg void OnSpiShuxing();
	afx_msg void OnSpiSign();
	afx_msg void OnSpiJmpToExolorer();
	afx_msg void OnSpiJmpToAntispy();
	afx_msg void OnSpiText();
	afx_msg void OnSpiExcel();
	CString GetCurrentItemPath();
	void AddSpiInfo(CString szName, CString szPath);
	CString ReleaseOriginLSPRegHive();
public:
	CSortListCtrl m_list;
	CString m_szStatus;
	ULONG m_nWinsockProvider;
	CommonFunctions m_Function;
	CRegistry m_Registry;
	vector<SPI_INFO> m_SpiInfoVector;
	afx_msg void OnSpiFixLsp();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};