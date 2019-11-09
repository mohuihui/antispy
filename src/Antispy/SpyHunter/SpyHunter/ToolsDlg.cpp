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
// ToolsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ToolsDlg.h"

CDialog *g_pToolsDialog[10];

// CToolsDlg 对话框

IMPLEMENT_DYNAMIC(CToolsDlg, CDialog)

CToolsDlg::CToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CToolsDlg::IDD, pParent)
{
	m_nCurSel = 0;
}

CToolsDlg::~CToolsDlg()
{
}

void CToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CToolsDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CToolsDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CToolsDlg)
	EASYSIZE(IDC_TAB, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CToolsDlg 消息处理程序

void CToolsDlg::OnSize(UINT nType, int cx, int cy)
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

		m_HexEditorDlg.MoveWindow(&rc);
		m_DisassemblerDlg.MoveWindow(&rc);
		m_AdvancedToolsDlg.MoveWindow(&rc);
		m_UserDlg.MoveWindow(&rc);
		m_MbrDlg.MoveWindow(&rc);
		m_FileAssociationDlg.MoveWindow(&rc);
		m_IFEODlg.MoveWindow(&rc);
		m_IMEDlg.MoveWindow(&rc);
	}
}

BOOL CToolsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 插入名称
	m_tab.InsertItem(0, szToolsHexEditor[g_enumLang]);
	m_tab.InsertItem(1, szToolsDisassembler[g_enumLang]);
	m_tab.InsertItem(2, szUsers[g_enumLang]);
	m_tab.InsertItem(3, szFileAssociation[g_enumLang]);
	m_tab.InsertItem(4, szIFEO[g_enumLang]);
	m_tab.InsertItem(5, szIME[g_enumLang]);
	m_tab.InsertItem(6, szMBR[g_enumLang]);
	m_tab.InsertItem(7, szAdvancedTools[g_enumLang]);
	
	// 创建对话框
	m_HexEditorDlg.Create(IDD_HEX_EDITOR_DIALOG, &m_tab);
	m_DisassemblerDlg.Create(IDD_DISASSEMBLER_DIALOG, &m_tab);
	m_AdvancedToolsDlg.Create(IDD_ADVANCED_TOOLS_DIALOG, &m_tab);
	m_UserDlg.Create(IDD_USER_DIALOG, &m_tab);
	m_MbrDlg.Create(IDD_MBR_DIALOG, &m_tab);
	m_FileAssociationDlg.Create(IDD_FILE_ASSOCIATION_DIALOG, &m_tab);
	m_IFEODlg.Create(IDD_IFEO_DIALOG, &m_tab);
	m_IMEDlg.Create(IDD_IME_DIALOG, &m_tab);

	// 设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 22;
	rc.bottom -= 3;
	rc.left += 2;
	rc.right -= 3;

	m_HexEditorDlg.MoveWindow(&rc);
	m_DisassemblerDlg.MoveWindow(&rc);
	m_AdvancedToolsDlg.MoveWindow(&rc);
	m_UserDlg.MoveWindow(&rc);
	m_MbrDlg.MoveWindow(&rc);
	m_FileAssociationDlg.MoveWindow(&rc);
	m_IFEODlg.MoveWindow(&rc);
	m_IMEDlg.MoveWindow(&rc);

	// 把对话框对象指针保存起来
	g_pToolsDialog[0] = &m_HexEditorDlg;
	g_pToolsDialog[1] = &m_DisassemblerDlg;
	g_pToolsDialog[2] = &m_UserDlg;
	g_pToolsDialog[3] = &m_FileAssociationDlg;
	g_pToolsDialog[4] = &m_IFEODlg;
	g_pToolsDialog[5] = &m_IMEDlg;
	g_pToolsDialog[6] = &m_MbrDlg;
	g_pToolsDialog[7] = &m_AdvancedToolsDlg;
	
	// 	显示初始页面
	g_pToolsDialog[0]->ShowWindow(SW_SHOW);
 	g_pToolsDialog[1]->ShowWindow(SW_HIDE);
	g_pToolsDialog[2]->ShowWindow(SW_HIDE);
	g_pToolsDialog[3]->ShowWindow(SW_HIDE);
	g_pToolsDialog[4]->ShowWindow(SW_HIDE);
	g_pToolsDialog[5]->ShowWindow(SW_HIDE);
	g_pToolsDialog[6]->ShowWindow(SW_HIDE);
	g_pToolsDialog[7]->ShowWindow(SW_HIDE);

	// 当前选择
	m_nCurSel = 0;
	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CToolsDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CToolsDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	g_pToolsDialog[m_nCurSel]->ShowWindow(SW_HIDE);
	m_nCurSel = m_tab.GetCurSel();
	g_pToolsDialog[m_nCurSel]->ShowWindow(SW_SHOW);

	if (m_nCurSel == 0)
	{
		m_HexEditorDlg.ListProcess();
	}
	if (m_nCurSel == 1)
	{
		m_DisassemblerDlg.ListProcess();
	}
	else if (m_nCurSel == 2)
	{
		m_UserDlg.ListUsers();
	}
	else if (m_nCurSel == 3)
	{
		m_FileAssociationDlg.ListFileAssociation();
	}
	else if (m_nCurSel == 4)
	{
		m_IFEODlg.ListIFEO();
	}
	else if (m_nCurSel == 5)
	{
		m_IMEDlg.ListIme();
	}
	else if (m_nCurSel == 6)
	{

	}

	*pResult = 0;
}