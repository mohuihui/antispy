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
// FindWindow.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FindWindow.h"
#include "ListProcessDlg.h"

// CFindWindow dialog

IMPLEMENT_DYNAMIC(CFindWindow, CDialog)

CFindWindow::CFindWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CFindWindow::IDD, pParent)
	, m_szPid(_T(""))
	, m_szTid(_T(""))
	, m_szShuoming(_T(""))
	, m_szStaticPid(_T(""))
	, m_szStaticTid(_T(""))
{
	m_hMutex = NULL;
}

CFindWindow::~CFindWindow()
{
}

void CFindWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PID, m_szPid);
	DDX_Text(pDX, IDC_EDIT_TID, m_szTid);
	//	DDX_Control(pDX, IDC_PIC, m_picture);
	DDX_Text(pDX, IDC_STATIC_TEXT, m_szShuoming);
	DDX_Text(pDX, IDC_STATIC_PID, m_szStaticPid);
	DDX_Text(pDX, IDC_STATIC_TID, m_szStaticTid);
}


BEGIN_MESSAGE_MAP(CFindWindow, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_FIND_PROCESS, &CFindWindow::OnBnClickedBtnFindProcess)
	ON_BN_CLICKED(IDC_BTN_KILL_PROCESS, &CFindWindow::OnBnClickedBtnKillProcess)
	ON_BN_CLICKED(IDC_BTN_KILL_THREAD, &CFindWindow::OnBnClickedBtnKillThread)
	ON_BN_CLICKED(IDC_BTN_FIND_THREAD, &CFindWindow::OnBnClickedBtnFindThread)
	ON_BN_CLICKED(IDCANCEL, &CFindWindow::OnBnClickedCancel)
END_MESSAGE_MAP()


// CFindWindow message handlers

BOOL CFindWindow::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_picture.SubclassDlgItem(IDC_PIC, this);
	m_picture.SetFindWndDlg(this);

	SetWindowText(szFindWindow[g_enumLang]);
	m_szShuoming = szFindWindowShuoming[g_enumLang];
	m_szStaticPid = szFindWindowPid[g_enumLang];
	m_szStaticTid = szFindWindowTid[g_enumLang];
	GetDlgItem(IDC_BTN_FIND_PROCESS)->SetWindowText(szFindProcess[g_enumLang]);
	GetDlgItem(IDC_BTN_FIND_THREAD)->SetWindowText(szFindThread[g_enumLang]);
	GetDlgItem(IDC_BTN_KILL_PROCESS)->SetWindowText(szFindWindowsKillProcess[g_enumLang]);
	GetDlgItem(IDC_BTN_KILL_THREAD)->SetWindowText(szFindWindowsKillThread[g_enumLang]);
	
	UpdateData(FALSE);

	WCHAR szFind[] = {'m','Z','f','F','i','n','d','W','n','d','\0'};
	m_hMutex = CreateMutex(NULL, TRUE, szFind);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFindWindow::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CFindWindow::OnDestroy()
{
	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}

	CDialog::OnDestroy();
	delete this;
}

//
// 定位到进程列表
//
void CFindWindow::OnBnClickedBtnFindProcess()
{
	CListProcessDlg* pDlg = (CListProcessDlg*)g_pProcessDlg;
	if (pDlg)
	{
		UpdateData(TRUE);
		DWORD dwPid = _wtoi(m_szPid);
		if (dwPid != 0)
		{
			pDlg->GotoProcess(dwPid);
		}
	}
}

//
// 点击结束进程
//
void CFindWindow::OnBnClickedBtnKillProcess()
{
	if (MessageBox(szAreYOUSureKillTheProcesses[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	
	CListProcessDlg* pDlg = (CListProcessDlg*)g_pProcessDlg;
	if (pDlg)
	{
		UpdateData(TRUE);
		DWORD dwPid = _wtoi(m_szPid);
		if (dwPid != 0)
		{
			pDlg->KillProcessByFindWindow(dwPid);
		}
	}
}

//
// 结束线程
//
void CFindWindow::OnBnClickedBtnKillThread()
{
	if (MessageBox(szAreYOUSureKillTheThread[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	CListProcessDlg* pDlg = (CListProcessDlg*)g_pProcessDlg;
	if (pDlg)
	{
		UpdateData(TRUE);
		DWORD dwTid = _wtoi(m_szTid);
		if (dwTid != 0)
		{
			HANDLE hThread = OpenThread(SYNCHRONIZE | THREAD_TERMINATE, FALSE, dwTid);
			if (hThread)
			{
				TerminateThread(hThread, 0);
				CloseHandle(hThread);
			}
		}
	}
}

void CFindWindow::OnBnClickedBtnFindThread()
{
	CListProcessDlg* pDlg = (CListProcessDlg*)g_pProcessDlg;
	if (pDlg)
	{
		UpdateData(TRUE);
		DWORD dwPid = _wtoi(m_szPid);
		DWORD dwTid = _wtoi(m_szTid);
		if (dwPid != 0 && dwTid != 0)
		{
			pDlg->GotoThread(dwPid, dwTid);
		}
	}
}

// 销毁窗口
void CFindWindow::OnBnClickedCancel()
{
	DestroyWindow();
}