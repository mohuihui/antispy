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
// ProcStringDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ProcStringDlg.h"


// CProcStringDlg dialog

IMPLEMENT_DYNAMIC(CProcStringDlg, CDialog)

CProcStringDlg::CProcStringDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcStringDlg::IDD, pParent)
	, m_szLength(_T(""))
	, m_nRadio(FALSE)
	, m_bPrivate(FALSE)
	, m_bImage(FALSE)
	, m_bMapped(FALSE)
{
	
}

CProcStringDlg::~CProcStringDlg()
{
}

void CProcStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LENGTH, m_szLength);
	DDX_Radio(pDX, IDC_RADIO_UNICODE, m_nRadio);
	DDX_Check(pDX, IDC_CHECK_PRIVATE, m_bPrivate);
	DDX_Check(pDX, IDC_CHECK_IMAGE, m_bImage);
	DDX_Check(pDX, IDC_CHECK_MAPPED, m_bMapped);
}


BEGIN_MESSAGE_MAP(CProcStringDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_START, &CProcStringDlg::OnBnClickedBtnStart)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CProcStringDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CProcStringDlg message handlers

BOOL CProcStringDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CProcStringDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	GetDlgItem(IDC_STATIC_LENGTH)->SetWindowText(szMinimumLength[g_enumLang]);
	GetDlgItem(IDC_STATIC_STRING_TYPE)->SetWindowText(szStringType[g_enumLang]);
	GetDlgItem(IDC_STATIC_REGION_TYPE)->SetWindowText(szMemoryRegionsType[g_enumLang]);
	
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProcStringDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CProcStringDlg::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here
}
