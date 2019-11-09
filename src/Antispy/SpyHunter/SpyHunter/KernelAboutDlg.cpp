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
// KernelAboutDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "KernelAboutDlg.h"

CDialog *g_pKernelAboutDialog[20];

// CKernelAboutDlg 对话框

IMPLEMENT_DYNAMIC(CKernelAboutDlg, CDialog)

CKernelAboutDlg::CKernelAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKernelAboutDlg::IDD, pParent)
{
	m_nCurSel = 0;
}

CKernelAboutDlg::~CKernelAboutDlg()
{
}

void CKernelAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CKernelAboutDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CKernelAboutDlg::OnBnClickedOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CKernelAboutDlg::OnTcnSelchangeTab)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CKernelAboutDlg)
	EASYSIZE(IDC_TAB, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CKernelAboutDlg 消息处理程序

void CKernelAboutDlg::OnBnClickedOk()
{
}

BOOL CKernelAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_tab.InsertItem(0 , szNotifyRoutine[g_enumLang]);
 	m_tab.InsertItem(1 , szDpcTimer[g_enumLang]);
 	m_tab.InsertItem(2, szIoTimer[g_enumLang]);
	m_tab.InsertItem(3 , szSystemThread[g_enumLang]);
	m_tab.InsertItem(4, szWorkerThread[g_enumLang]);
	m_tab.InsertItem(5 , szDebugRegisters[g_enumLang]);
	m_tab.InsertItem(6, szFilter[g_enumLang]);
//	m_tab.InsertItem(7 , szObjectHijack[g_enumLang]);
	m_tab.InsertItem(7, szDirectIO[g_enumLang]);

	m_ListCallbackDlg.Create(IDD_CALLBACK_DIALOG, &m_tab);
	m_DpcTimerDlg.Create(IDD_DPC_TIMER_DIALOG, &m_tab);
	m_IoTimerDlg.Create(IDD_IO_TIMER_DIALOG, &m_tab);
	m_SystemThreadDlg.Create(IDD_SYSTEM_THREAD_DIALOG, &m_tab);
	m_WorkerThread.Create(IDD_WORKER_THREAD_DIALOG, &m_tab);
	m_DebugRegisterDlg.Create(IDD_DEBUG_REGISTER_DIALOG, &m_tab);
	m_FilterDlg.Create(IDD_FILTER_DIALOG, &m_tab);
//	m_ObjectHijackDlg.Create(IDD_OBJECT_HIJACK_DIALOG, &m_tab);
	m_DirectIoDlg.Create(IDD_DIRECT_IO_DIALOG, &m_tab);

	// 设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 22;
	rc.bottom -= 3;
	rc.left += 2;
	rc.right -= 3;
 
 	m_ListCallbackDlg.MoveWindow(&rc);
	m_DpcTimerDlg.MoveWindow(&rc);
	m_IoTimerDlg.MoveWindow(&rc);
	m_SystemThreadDlg.MoveWindow(&rc);
	m_WorkerThread.MoveWindow(&rc);
	m_DebugRegisterDlg.MoveWindow(&rc);
	m_FilterDlg.MoveWindow(&rc);
// 	m_ObjectHijackDlg.MoveWindow(&rc);
	m_DirectIoDlg.MoveWindow(&rc);

	// 把对话框对象指针保存起来
	g_pKernelAboutDialog[0] = &m_ListCallbackDlg;
	g_pKernelAboutDialog[1] = &m_DpcTimerDlg;
	g_pKernelAboutDialog[2] = &m_IoTimerDlg;
	g_pKernelAboutDialog[3] = &m_SystemThreadDlg;
	g_pKernelAboutDialog[4] = &m_WorkerThread;
	g_pKernelAboutDialog[5] = &m_DebugRegisterDlg;
	g_pKernelAboutDialog[6] = &m_FilterDlg;
	g_pKernelAboutDialog[7] = &m_DirectIoDlg;//m_ObjectHijackDlg;
// 	g_pKernelAboutDialog[8] = &m_DirectIoDlg;

	// 显示初始页面
	g_pKernelAboutDialog[0]->ShowWindow(SW_SHOW);
	g_pKernelAboutDialog[1]->ShowWindow(SW_HIDE);
	g_pKernelAboutDialog[2]->ShowWindow(SW_HIDE);
	g_pKernelAboutDialog[3]->ShowWindow(SW_HIDE);
	g_pKernelAboutDialog[4]->ShowWindow(SW_HIDE);
	g_pKernelAboutDialog[5]->ShowWindow(SW_HIDE);
	g_pKernelAboutDialog[6]->ShowWindow(SW_HIDE);
	g_pKernelAboutDialog[7]->ShowWindow(SW_HIDE);
// 	g_pKernelAboutDialog[8]->ShowWindow(SW_HIDE);

	// 当前选择
	m_nCurSel = 0;
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CKernelAboutDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	g_pKernelAboutDialog[m_nCurSel]->ShowWindow(SW_HIDE);
	m_nCurSel = m_tab.GetCurSel();
	g_pKernelAboutDialog[m_nCurSel]->ShowWindow(SW_SHOW);

	if (m_nCurSel == 0)
	{
		m_ListCallbackDlg.GetCallbacks();
	}
	else if (m_nCurSel == 1)
	{
		m_DpcTimerDlg.GetDpcTimer();
	}
	else if (m_nCurSel == 2)
	{
		m_IoTimerDlg.GetIoTimers();
	}
	else if (m_nCurSel == 3)
	{
		m_SystemThreadDlg.ListProcessThread();
	}
	else if (m_nCurSel == 4)
	{
		m_WorkerThread.GetWorkerThread();
	}
	else if (m_nCurSel == 5)
	{
		m_DebugRegisterDlg.GetDebugRegister();
	}
	else if (m_nCurSel == 6)
	{
		m_FilterDlg.EnumFilterDriver();
	}
// 	else if (m_nCurSel == 7)
// 	{
// 		m_ObjectHijackDlg.GetObjectHijack();
// 	}
	else if (m_nCurSel == 7)
	{
		m_DirectIoDlg.GetDirectIoProcess();
	}
// 	else if (m_nCurSel == 9)
// 	{
// 	}

	*pResult = 0;
}

void CKernelAboutDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;

	if (m_tab.m_hWnd)
	{
		CRect rc;
		m_tab.GetClientRect(rc);
		rc.top += 22;
		rc.bottom -= 3;
		rc.left += 2;
		rc.right -= 3;

		m_ListCallbackDlg.MoveWindow(&rc);
		m_DpcTimerDlg.MoveWindow(&rc);
		m_IoTimerDlg.MoveWindow(&rc);
		m_SystemThreadDlg.MoveWindow(&rc);
		m_WorkerThread.MoveWindow(&rc);
		m_DebugRegisterDlg.MoveWindow(&rc);
		m_FilterDlg.MoveWindow(&rc);
//		m_ObjectHijackDlg.MoveWindow(&rc);
		m_DirectIoDlg.MoveWindow(&rc);
	}
}
