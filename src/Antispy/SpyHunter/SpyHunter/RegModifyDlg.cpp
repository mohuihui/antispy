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
// RegModifyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "RegModifyDlg.h"


// CRegModifyDlg 对话框

IMPLEMENT_DYNAMIC(CRegModifyDlg, CDialog)

CRegModifyDlg::CRegModifyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegModifyDlg::IDD, pParent)
	, m_szValueNameStatic(_T(""))
	, m_szValueName(_T(""))
	, m_szValueDataStatic(_T(""))
	, m_szValueData(_T(""))
{

}

CRegModifyDlg::~CRegModifyDlg()
{
}

void CRegModifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VALUE_NAME_STATIC, m_szValueNameStatic);
	DDX_Text(pDX, IDC_EDIT_VALUE_NAME, m_szValueName);
	DDX_Text(pDX, IDC_VALUE_DATA_STATIC, m_szValueDataStatic);
	DDX_Text(pDX, IDC_EDIT_VALUE_DATA, m_szValueData);
}


BEGIN_MESSAGE_MAP(CRegModifyDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CRegModifyDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CRegModifyDlg 消息处理程序

void CRegModifyDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	OnOK();
}

BOOL CRegModifyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);
	GetDlgItem(IDCANCEL)->SetWindowText(szCancel[g_enumLang]);

	SetWindowText(szRegEditString[g_enumLang]);
	m_szValueDataStatic = szRegValueData[g_enumLang];
	m_szValueNameStatic = szRegValueName[g_enumLang];

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
