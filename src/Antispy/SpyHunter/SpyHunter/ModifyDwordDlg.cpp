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
// ModifyDwordDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ModifyDwordDlg.h"


// CModifyDwordDlg 对话框

IMPLEMENT_DYNAMIC(CModifyDwordDlg, CDialog)

CModifyDwordDlg::CModifyDwordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyDwordDlg::IDD, pParent)
	, m_szValueNameStatic(_T(""))
	, m_szValueName(_T(""))
	, m_szValueDataStatic(_T(""))
	, m_szValueData(_T(""))
	, m_szBase(_T(""))
	, m_nRadio(0)
{
	
}

CModifyDwordDlg::~CModifyDwordDlg()
{
}

void CModifyDwordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VALUE_NAME_STATIC, m_szValueNameStatic);
	DDX_Text(pDX, IDC_EDIT_VALUE_NAME, m_szValueName);
	DDX_Text(pDX, IDC_VALUE_DATA_STATIC, m_szValueDataStatic);
	DDX_Text(pDX, IDC_EDIT_VALUE_DATA, m_szValueData);
	DDX_Text(pDX, IDC_STATIC_BASE, m_szBase);
}


BEGIN_MESSAGE_MAP(CModifyDwordDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CModifyDwordDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_HEX, &CModifyDwordDlg::OnBnClickedRadioHex)
	ON_BN_CLICKED(IDC_RADIO_DEC, &CModifyDwordDlg::OnBnClickedRadioDec)
END_MESSAGE_MAP()


// CModifyDwordDlg 消息处理程序

void CModifyDwordDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	OnOK();
}

BOOL CModifyDwordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);
	GetDlgItem(IDCANCEL)->SetWindowText(szCancel[g_enumLang]);
	GetDlgItem(IDC_RADIO_HEX)->SetWindowText(szHexadecimal[g_enumLang]);
	GetDlgItem(IDC_RADIO_DEC)->SetWindowText(szDecimal[g_enumLang]);

	SetWindowText(szRegEditDwordValue[g_enumLang]);
	m_szValueDataStatic = szRegValueData[g_enumLang];
	m_szValueNameStatic = szRegValueName[g_enumLang];
	m_szBase = szRegBase[g_enumLang];

	((CButton *)GetDlgItem(IDC_RADIO_HEX))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_DEC))->SetCheck(FALSE);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CModifyDwordDlg::OnBnClickedRadioHex()
{
	if (m_nRadio == 1)
	{
		m_nRadio = 0;
		UpdateData(TRUE);
		CString szData = m_szValueData;
		if (!szData.IsEmpty())
		{
			ULONG nRet = 0;
			swscanf_s(szData.GetBuffer(0), L"%d", &nRet);

			CString szShowData;
			szShowData.Format(L"%X", nRet);
			m_szValueData = szShowData;
			UpdateData(FALSE);
		}
	}
}

void CModifyDwordDlg::OnBnClickedRadioDec()
{
	if (m_nRadio == 0)
	{
		m_nRadio = 1;
		UpdateData(TRUE);
		CString szData = m_szValueData;
		if (!szData.IsEmpty())
		{
			ULONG nRet = 0;
			swscanf_s(szData.GetBuffer(0), L"%x", &nRet);

			CString szShowData;
			szShowData.Format(L"%d", nRet);
			m_szValueData = szShowData;
			UpdateData(FALSE);
		}
	}
}