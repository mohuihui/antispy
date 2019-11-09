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
// HashCalcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HashCalcDlg.h"
#include "KMD5.h"
#include "SHA1.h"

#define  WM_UPDATE_MD5_UI		WM_USER + 109
#define  WM_UPDATE_SHA1_UI		WM_USER + 110

// CHashCalcDlg dialog

IMPLEMENT_DYNAMIC(CHashCalcDlg, CDialog)

CHashCalcDlg::CHashCalcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHashCalcDlg::IDD, pParent)
	, m_szPath(_T(""))
	, m_szMd5(_T(""))
	, m_szMd5Compare(_T(""))
	, m_szSha1(_T(""))
	, m_szSha1Compare(_T(""))
{
	m_hMd5Thread = NULL;
	m_hSHA1Thread = NULL;
}

CHashCalcDlg::~CHashCalcDlg()
{
}

void CHashCalcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_PATH, m_szPath);
	DDX_Text(pDX, IDC_EDIT_MD5, m_szMd5);
	DDX_Text(pDX, IDC_EDIT_MD5_COMPARE, m_szMd5Compare);
	DDX_Text(pDX, IDC_EDIT_SHA1, m_szSha1);
	DDX_Text(pDX, IDC_EDIT_SHA1_COMPARE, m_szSha1Compare);
}


BEGIN_MESSAGE_MAP(CHashCalcDlg, CDialog)
	ON_BN_CLICKED(IDC_COMPARE, &CHashCalcDlg::OnBnClickedCompare)
	ON_BN_CLICKED(IDOK, &CHashCalcDlg::OnBnClickedOk)
	ON_MESSAGE(WM_UPDATE_MD5_UI, &CHashCalcDlg::UpdateUIMd5)
	ON_MESSAGE(WM_UPDATE_SHA1_UI, &CHashCalcDlg::UpdateUISha1)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CHashCalcDlg message handlers

LRESULT CHashCalcDlg::UpdateUIMd5(WPARAM, LPARAM)
{
	UpdateData(FALSE);
	return 0;
}

LRESULT CHashCalcDlg::UpdateUISha1(WPARAM, LPARAM)
{
	UpdateData(FALSE);
	return 0;
}

void CHashCalcDlg::OnBnClickedCompare()
{
	UpdateData(TRUE);

	CString szMd51 = m_szMd5;
	CString szMd52 = m_szMd5Compare;
	CString szSHA11 = m_szSha1;
	CString szSHA12 = m_szSha1Compare;

	szMd51.MakeUpper();
	szMd52.MakeUpper();
	szSHA11.MakeUpper();
	szSHA12.MakeUpper();

	BOOL bMd5 = szMd51.CompareNoCase(szMd52) == 0;
	BOOL bSHA11 = szSHA11.CompareNoCase(szSHA12) == 0;
	
	CString szOutput;
	WCHAR szMd5[] = {'M','D','5',':','\t','\0'};
	WCHAR szSHA1[] = {'S','H','A','1',':','\t','\0'};
	WCHAR szSame[] = {'S','a','m','e','\0'};
	WCHAR szDiffent[] = {'D','i','f','f','e','r','e','n','t','\0'};

	szOutput += szMd5;
	if (bMd5)
	{
		if (g_enumLang == enumEnglish)
		{
			szOutput += szSame;
		}
		else
		{
			szOutput += L"相同";
		}
	}
	else
	{
		if (g_enumLang == enumEnglish)
		{
			szOutput += szDiffent;
		}
		else
		{
			szOutput += L"不相同";
		}
	}

	szOutput += L"\r\n";
	szOutput += szSHA1;

	if (bSHA11)
	{
		if (g_enumLang == enumEnglish)
		{

			szOutput += szSame;
		}
		else
		{
			szOutput += L"相同";
		}
	}
	else
	{
		if (g_enumLang == enumEnglish)
		{

			szOutput += szDiffent;
		}
		else
		{
			szOutput += L"不相同";
		}
	}

	MessageBox(szOutput, szToolName, MB_ICONINFORMATION | MB_OK);
}

void CHashCalcDlg::CalcSha1()
{
	m_szSha1 = szFileHashCalcing[g_enumLang];
	SendMessage(WM_UPDATE_MD5_UI);

	CSHA1 Sha1;
	TCHAR szRet[100] = {0};
	Sha1.HashFile(m_szPath, szRet);
	m_szSha1 = szRet;
	m_szSha1.MakeUpper();
	SendMessage(WM_UPDATE_MD5_UI);
}

void CHashCalcDlg::CalcMD5()
{
	m_szMd5 = szFileHashCalcing[g_enumLang];
	SendMessage(WM_UPDATE_MD5_UI);
	
	KMD5 MD5;
	m_szMd5 = MD5.GetMD5Str(m_szPath);
	m_szMd5.MakeUpper();
	SendMessage(WM_UPDATE_MD5_UI);
}

DWORD WINAPI CalcMd5Proc(PVOID p)
{
	CHashCalcDlg *pDlg = (CHashCalcDlg*)p;
	if (pDlg)
	{
		pDlg->CalcMD5();
	}

	return 0;
}

DWORD WINAPI CalcSha1Proc(PVOID p)
{
	CHashCalcDlg *pDlg = (CHashCalcDlg*)p;
	if (pDlg)
	{
		pDlg->CalcSha1();
	}

	return 0;
}

void CHashCalcDlg::CalcHash()
{
	if (m_hMd5Thread)
	{
		if (WaitForSingleObject(m_hMd5Thread, 1) == WAIT_TIMEOUT)
		{
			TerminateThread(m_hMd5Thread, 0);
		}
		
		CloseHandle(m_hMd5Thread);
		m_hMd5Thread = NULL;
	}

	if (m_hSHA1Thread)
	{
		if (WaitForSingleObject(m_hSHA1Thread, 1) == WAIT_TIMEOUT)
		{
			TerminateThread(m_hSHA1Thread, 0);
		}

		CloseHandle(m_hSHA1Thread);
		m_hSHA1Thread = NULL;
	}

	if (!m_szPath.IsEmpty() && PathFileExists(m_szPath))
	{
		m_hMd5Thread = CreateThread(NULL, 0, CalcMd5Proc, this, 0, NULL);
		m_hSHA1Thread = CreateThread(NULL, 0, CalcSha1Proc, this, 0, NULL);;
	}
}

void CHashCalcDlg::OnBnClickedOk()
{
	CalcHash();
}

BOOL CHashCalcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(szFileHashDlgTitle[g_enumLang]);
	GetDlgItem(IDC_STATIC_FILE)->SetWindowText(szFileHashFile[g_enumLang]);
	GetDlgItem(IDC_STATIC_COMPARE_MD5)->SetWindowText(szFileHashCompare[g_enumLang]);
	GetDlgItem(IDC_STATIC_COMPARE_SHA1)->SetWindowText(szFileHashCompare[g_enumLang]);
	GetDlgItem(IDC_COMPARE)->SetWindowText(szFileHashBtnCompare[g_enumLang]);
	GetDlgItem(IDOK)->SetWindowText(szFileHashBtnCalc[g_enumLang]);

	CalcHash();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CHashCalcDlg::OnClose()
{
	if (m_hMd5Thread)
	{
		if (WaitForSingleObject(m_hMd5Thread, 1) == WAIT_TIMEOUT)
		{
			TerminateThread(m_hMd5Thread, 0);
		}

		CloseHandle(m_hMd5Thread);
		m_hMd5Thread = NULL;
	}

	if (m_hSHA1Thread)
	{
		if (WaitForSingleObject(m_hSHA1Thread, 1) == WAIT_TIMEOUT)
		{
			TerminateThread(m_hSHA1Thread, 0);
		}

		CloseHandle(m_hSHA1Thread);
		m_hSHA1Thread = NULL;
	}

	CDialog::OnClose();
}