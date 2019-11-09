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
// FileRenameDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FileRenameDlg.h"


// CFileRenameDlg 对话框

IMPLEMENT_DYNAMIC(CFileRenameDlg, CDialog)

CFileRenameDlg::CFileRenameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileRenameDlg::IDD, pParent)
	, m_szFileName(_T(""))
	, m_szStaticInputPath(_T(""))
{

}

CFileRenameDlg::~CFileRenameDlg()
{
}

void CFileRenameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_NAME, m_szFileName);
	DDX_Text(pDX, IDC_STATIC_FILE_PATH, m_szStaticInputPath);
}


BEGIN_MESSAGE_MAP(CFileRenameDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFileRenameDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CFileRenameDlg 消息处理程序

void CFileRenameDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	OnOK();
}

BOOL CFileRenameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(szInputFileName[g_enumLang]);
	m_szStaticInputPath = szStaticInputFileName[g_enumLang];
	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);
	GetDlgItem(IDCANCEL)->SetWindowText(szCancel[g_enumLang]);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
