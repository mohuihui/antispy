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
// Ring0HookDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "Ring0HookDlg.h"

CDialog *g_pRing0HookDialog[20];

// CRing0HookDlg 对话框

IMPLEMENT_DYNAMIC(CRing0HookDlg, CDialog)

CRing0HookDlg::CRing0HookDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRing0HookDlg::IDD, pParent)
{
	m_nCurSel = 0;
}

CRing0HookDlg::~CRing0HookDlg()
{
}

void CRing0HookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CRing0HookDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CRing0HookDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CRing0HookDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CRing0HookDlg)
	EASYSIZE(IDC_TAB, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CRing0HookDlg 消息处理程序

void CRing0HookDlg::OnBnClickedOk()
{
}

void CRing0HookDlg::OnSize(UINT nType, int cx, int cy)
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

		m_SsdtDlg.MoveWindow(&rc);
 		m_ShadowSsdtDlg.MoveWindow(&rc);
 		m_FileSystemDlg.MoveWindow(&rc);
 		m_KbdclassDlg.MoveWindow(&rc);
 		m_MouseDlg.MoveWindow(&rc);
 		m_DiskDlg.MoveWindow(&rc);
 		m_AtapiDlg.MoveWindow(&rc);
 		m_AcpiDlg.MoveWindow(&rc);
 		m_TcpipDlg.MoveWindow(&rc);
		m_IdtDlg.MoveWindow(&rc);
		m_ObjectHookDlg.MoveWindow(&rc);
		m_KernelEntryDlg.MoveWindow(&rc);
		m_IatEatDlg.MoveWindow(&rc);
		m_ModifyCodeDlg.MoveWindow(&rc);
		m_MessageHookDlg.MoveWindow(&rc);
	}
}

BOOL CRing0HookDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_tab.InsertItem(0 , szSSDT[g_enumLang]);
 	m_tab.InsertItem(1 , szShadowSSDT[g_enumLang]);
 	m_tab.InsertItem(2, szFSD[g_enumLang]);
 	m_tab.InsertItem(3 , szKbdClass[g_enumLang]);
 	m_tab.InsertItem(4, szMouClass[g_enumLang]);
 	m_tab.InsertItem(5 , szDisk[g_enumLang]);
 	m_tab.InsertItem(6, szAtapi[g_enumLang]);
 	m_tab.InsertItem(7 , szAcpi[g_enumLang]);
 	m_tab.InsertItem(8, szTcpip[g_enumLang]);
	m_tab.InsertItem(9, szIdt[g_enumLang]);
	m_tab.InsertItem(10, szObjectHook[g_enumLang]);
	m_tab.InsertItem(11, szKernelEntry[g_enumLang]);
	m_tab.InsertItem(12, szIatEat[g_enumLang]);
	m_tab.InsertItem(13, szModifyedCode[g_enumLang]);
	m_tab.InsertItem(14 , szMessageHook[g_enumLang]);

	m_SsdtDlg.Create(IDD_SSDT_DIALOG, &m_tab);
 	m_ShadowSsdtDlg.Create(IDD_SHADOW_SSDT_DIALOG, &m_tab);
	m_FileSystemDlg.Create(IDD_FILE_SYSTEM_DIALOG, &m_tab);
 	m_KbdclassDlg.Create(IDD_KBDCLASS_DIALOG, &m_tab);
 	m_MouseDlg.Create(IDD_MOUSE_DIALOG, &m_tab);
 	m_DiskDlg.Create(IDD_DISK_DIALOG, &m_tab);
 	m_AtapiDlg.Create(IDD_ATAPI_DIALOG, &m_tab);
 	m_AcpiDlg.Create(IDD_ACPI_DIALOG, &m_tab);
 	m_TcpipDlg.Create(IDD_TCPIP_DIALOG, &m_tab);
 	m_IdtDlg.Create(IDD_IDT_DIALOG, &m_tab);
	m_ObjectHookDlg.Create(IDD_OBJECT_HOOK_DIALOG, &m_tab);
	m_KernelEntryDlg.Create(IDD_KERNRL_ENTRY_DIALOG, &m_tab);
	m_IatEatDlg.Create(IDD_EAT_IAT_DIALOG, &m_tab);
	m_ModifyCodeDlg.Create(IDD_MODIFY_CODE_DIALOG, &m_tab);
	m_MessageHookDlg.Create(IDD_MESSAGE_HOOK_DIALOG, &m_tab);

	// 设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 22;
	rc.bottom -= 3;
	rc.left += 2;
	rc.right -= 3;

	m_SsdtDlg.MoveWindow(&rc);
 	m_ShadowSsdtDlg.MoveWindow(&rc);
 	m_FileSystemDlg.MoveWindow(&rc);
 	m_KbdclassDlg.MoveWindow(&rc);
 	m_MouseDlg.MoveWindow(&rc);
 	m_DiskDlg.MoveWindow(&rc);
 	m_AtapiDlg.MoveWindow(&rc);
 	m_AcpiDlg.MoveWindow(&rc);
 	m_TcpipDlg.MoveWindow(&rc);
	m_IdtDlg.MoveWindow(&rc);
	m_ObjectHookDlg.MoveWindow(&rc);
	m_KernelEntryDlg.MoveWindow(&rc);
	m_IatEatDlg.MoveWindow(&rc);
	m_ModifyCodeDlg.MoveWindow(&rc);
	m_MessageHookDlg.MoveWindow(&rc);

	// 把对话框对象指针保存起来
	g_pRing0HookDialog[0] = &m_SsdtDlg;
 	g_pRing0HookDialog[1] = &m_ShadowSsdtDlg;
 	g_pRing0HookDialog[2] = &m_FileSystemDlg;
 	g_pRing0HookDialog[3] = &m_KbdclassDlg;
 	g_pRing0HookDialog[4] = &m_MouseDlg;
 	g_pRing0HookDialog[5] = &m_DiskDlg;
 	g_pRing0HookDialog[6] = &m_AtapiDlg;
	g_pRing0HookDialog[7] = &m_AcpiDlg;
	g_pRing0HookDialog[8] = &m_TcpipDlg;
	g_pRing0HookDialog[9] = &m_IdtDlg;
	g_pRing0HookDialog[10] = &m_ObjectHookDlg;
	g_pRing0HookDialog[11] = &m_KernelEntryDlg;
	g_pRing0HookDialog[12] = &m_IatEatDlg;
	g_pRing0HookDialog[13] = &m_ModifyCodeDlg;
	g_pRing0HookDialog[14] = &m_MessageHookDlg;

	// 显示初始页面
	g_pRing0HookDialog[0]->ShowWindow(SW_SHOW);
 	g_pRing0HookDialog[1]->ShowWindow(SW_HIDE);
 	g_pRing0HookDialog[2]->ShowWindow(SW_HIDE);
 	g_pRing0HookDialog[3]->ShowWindow(SW_HIDE);
 	g_pRing0HookDialog[4]->ShowWindow(SW_HIDE);
 	g_pRing0HookDialog[5]->ShowWindow(SW_HIDE);
 	g_pRing0HookDialog[6]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[7]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[8]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[9]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[10]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[11]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[12]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[13]->ShowWindow(SW_HIDE);
	g_pRing0HookDialog[14]->ShowWindow(SW_HIDE);	

	// 当前选择
	m_nCurSel = 0;

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CRing0HookDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	g_pRing0HookDialog[m_nCurSel]->ShowWindow(SW_HIDE);
	m_nCurSel = m_tab.GetCurSel();
	g_pRing0HookDialog[m_nCurSel]->ShowWindow(SW_SHOW);

	if (m_nCurSel == 0)
	{
		m_SsdtDlg.GetSsdtHooks();
	}
	else if (m_nCurSel == 1)
	{
		m_ShadowSsdtDlg.GetShadowSsdtHooks();
	}
	else if (m_nCurSel == 2)
	{
		m_FileSystemDlg.GetFsdDispatch();
	}
	else if (m_nCurSel == 3)
	{
		m_KbdclassDlg.GetKbdclassDispatch();
	}
	else if (m_nCurSel == 4)
	{
		m_MouseDlg.GetMouclassDispatch();
	}
	else if (m_nCurSel == 5)
	{
		m_DiskDlg.GetClasspnpDispatch();
	}
	else if (m_nCurSel == 6)
	{
		m_AtapiDlg.GetDispatch();
	}
	else if (m_nCurSel == 7)
	{
		m_AcpiDlg.GetDispatch();
	}
	else if (m_nCurSel == 8)
	{
		m_TcpipDlg.GetDispatch();
	}
	else if (m_nCurSel == 9)
	{
		m_IdtDlg.GetIdt();
	}
	else if (m_nCurSel == 10)
	{
		m_ObjectHookDlg.GetObjectHook();
	}
	else if (m_nCurSel == 11)
	{
		m_KernelEntryDlg.GetKernelEntryHooks();
	}
	else if (m_nCurSel == 12)
	{
		m_IatEatDlg.GetIatEatHook();
	}
	else if (m_nCurSel == 13)
	{
		m_ModifyCodeDlg.EnumModifiedCode();
	}
	else if (m_nCurSel == 14)
	{
		m_MessageHookDlg.EnumMessageHook();
	}

	*pResult = 0;
}
