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
// ObjectHijackDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ObjectHijackDlg.h"


// CObjectHijackDlg 对话框

IMPLEMENT_DYNAMIC(CObjectHijackDlg, CDialog)

CObjectHijackDlg::CObjectHijackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectHijackDlg::IDD, pParent)
	, m_szStatus(_T(""))
{

}

CObjectHijackDlg::~CObjectHijackDlg()
{
}

void CObjectHijackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_OBJECT_HIJACK_CNT, m_szStatus);
}


BEGIN_MESSAGE_MAP(CObjectHijackDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CObjectHijackDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_COMMAND(ID_OBJECT_HIJACK_REFRESH, &CObjectHijackDlg::OnObjectHijackRefresh)
	ON_COMMAND(ID_OBJECT_HIJACK_TEXT, &CObjectHijackDlg::OnObjectHijackText)
	ON_COMMAND(ID_OBJECT_HIJACK_EXCEL, &CObjectHijackDlg::OnObjectHijackExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CObjectHijackDlg::OnNMRclickList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CObjectHijackDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_OBJECT_HIJACK_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CObjectHijackDlg 消息处理程序

void CObjectHijackDlg::OnBnClickedOk()
{
}

void CObjectHijackDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CObjectHijackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szObject[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, szObjectType[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(2, szObjectName[g_enumLang], LVCFMT_LEFT, 330);
	m_list.InsertColumn(3, szDescription[g_enumLang], LVCFMT_LEFT, 500);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

VOID CObjectHijackDlg::GetObjectHijack()
{
	m_szStatus.Format(szObjectHijackStatus[g_enumLang], 0);
	UpdateData(FALSE);

	if (g_bLoadDriverOK)
	{
		Sleep(1000);
	}
}

void CObjectHijackDlg::OnObjectHijackRefresh()
{
	GetObjectHijack();
}

void CObjectHijackDlg::OnObjectHijackText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CObjectHijackDlg::OnObjectHijackExcel()
{
	WCHAR szObjectHijack[] = {'O','b','j','e','c','t','H','i','j','a','c','k','\0'};
	m_Functions.ExportListToExcel(&m_list, szObjectHijack, m_szStatus);
}

void CObjectHijackDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_OBJECT_HIJACK_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_OBJECT_HIJACK_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_OBJECT_HIJACK_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_OBJECT_HIJACK_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(2, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 3; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

BOOL CObjectHijackDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}