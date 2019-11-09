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
// HexEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HexEditDlg.h"


// CHexEditDlg dialog

IMPLEMENT_DYNAMIC(CHexEditDlg, CDialog)

CHexEditDlg::CHexEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHexEditDlg::IDD, pParent)
	, m_szValueName(_T(""))
{
	m_nDataLen = 0;
	m_pData = NULL;
	m_pRetData = NULL;
	m_nRetLen = 0;
}

CHexEditDlg::~CHexEditDlg()
{
}

void CHexEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HEXEDIT, m_editHex);
	DDX_Text(pDX, IDC_EDIT_VALUE_NAME, m_szValueName);
}


BEGIN_MESSAGE_MAP(CHexEditDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CHexEditDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CHexEditDlg message handlers

BOOL CHexEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetWindowText(szEditBinaryValue[g_enumLang]);
	
	m_editHex.SetBPR(8);
	m_editHex.SetData(m_pData, m_nDataLen, -1);
	
	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);
	GetDlgItem(IDCANCEL)->SetWindowText(szCancel[g_enumLang]);
	GetDlgItem(IDC_VALUE_NAME)->SetWindowText(szValueName[g_enumLang]);
	GetDlgItem(IDC_VALUE_DATA)->SetWindowText(szRegValueData[g_enumLang]);
	
	UpdateData(TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CHexEditDlg::OnBnClickedOk()
{
	m_nRetLen = m_editHex.GetData(NULL, 0);
	if (m_nRetLen > 0)
	{
		m_pRetData = (PBYTE)malloc(m_nRetLen);
		if (m_pRetData)
		{
			memset(m_pRetData, 0, m_nRetLen);
			m_editHex.GetData(m_pRetData, m_nRetLen);
		}
	}

	OnOK();
}