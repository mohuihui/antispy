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
// GetFilePathDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "GetFilePathDlg.h"


// CGetFilePathDlg 对话框

IMPLEMENT_DYNAMIC(CGetFilePathDlg, CDialog)

CGetFilePathDlg::CGetFilePathDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGetFilePathDlg::IDD, pParent)
	, m_szPath(_T(""))
	, m_szStaticInputPath(_T(""))
{
	m_bDirectoty = FALSE;
	szFileName = L"";
}

CGetFilePathDlg::~CGetFilePathDlg()
{
}

void CGetFilePathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PATH, m_szPath);
	DDX_Text(pDX, IDC_STATIC_INPUT_PATH, m_szStaticInputPath);
	DDX_Control(pDX, IDC_BROWSE, m_btnBrowse);
}


BEGIN_MESSAGE_MAP(CGetFilePathDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSE, &CGetFilePathDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, &CGetFilePathDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CGetFilePathDlg 消息处理程序

void CGetFilePathDlg::OnBnClickedBrowse()
{
	if (!m_bDirectoty)
	{
		CFileDialog fileDlg(
			TRUE, 
			0, 
			(LPWSTR)NULL, 
			0, 
			L"All Files (*.*)|*.*||",
			0
			);

		if (IDOK == fileDlg.DoModal())
		{
			m_szPath = fileDlg.GetPathName();
		}
	}
	else
	{
		WCHAR szPath[MAX_PATH] = {0}; 
		BROWSEINFO bi;   
		bi.hwndOwner = m_hWnd;   
		bi.pidlRoot = NULL;   
		bi.pszDisplayName = szPath;   
		bi.lpszTitle = NULL;   
		bi.ulFlags = 0;   
		bi.lpfn = NULL;   
		bi.lParam = 0;   
		bi.iImage = 0;   

		//弹出选择目录对话框
		LPITEMIDLIST lp = SHBrowseForFolder(&bi);  

		if(lp && SHGetPathFromIDList(lp, szPath))   
		{
			CString szTemp = szPath;
			if (szTemp.GetAt(szTemp.GetLength() - 1) == '\\')
			{
				m_szPath.Format(L"%s%s", szPath, szFileName);
			}
			else
			{
				m_szPath.Format(L"%s\\%s", szPath, szFileName);
			}
		}
	}

	UpdateData(FALSE);
}

void CGetFilePathDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	OnOK();
}

BOOL CGetFilePathDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(szInputFilePath[g_enumLang]);
	m_szStaticInputPath = szStaticInputFilePath[g_enumLang];
	m_btnBrowse.SetWindowText(szFileBrowse[g_enumLang]);
	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);
	GetDlgItem(IDCANCEL)->SetWindowText(szCancel[g_enumLang]);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}