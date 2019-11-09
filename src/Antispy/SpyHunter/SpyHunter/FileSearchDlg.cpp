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
// FileSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FileSearchDlg.h"


// CFileSearchDlg dialog

IMPLEMENT_DYNAMIC(CFileSearchDlg, CDialog)

CFileSearchDlg::CFileSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileSearchDlg::IDD, pParent)
{

}

CFileSearchDlg::~CFileSearchDlg()
{
}

void CFileSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFileSearchDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFileSearchDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CFileSearchDlg message handlers

void CFileSearchDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
