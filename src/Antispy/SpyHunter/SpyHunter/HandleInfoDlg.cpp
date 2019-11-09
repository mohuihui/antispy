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
// HandleInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HandleInfoDlg.h"
#include <algorithm>

// CHandleInfoDlg 对话框

IMPLEMENT_DYNAMIC(CHandleInfoDlg, CDialog)

CHandleInfoDlg::CHandleInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHandleInfoDlg::IDD, pParent)
	, m_szState(_T(""))
{
	m_dwPid = 0;
	m_pEprocess = 0;
}

CHandleInfoDlg::~CHandleInfoDlg()
{
	m_HandleVector.clear();
}

void CHandleInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_HANDLE_INFO_CNT, m_szState);
}


BEGIN_MESSAGE_MAP(CHandleInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CHandleInfoDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CHandleInfoDlg::OnNMRclickList)
	ON_COMMAND(ID_HANDLE_REFRESH, &CHandleInfoDlg::OnHandleRefresh)
	ON_COMMAND(ID_HANDLE_CLOSE, &CHandleInfoDlg::OnHandleClose)
	ON_COMMAND(ID_HANDLE_FORCE_CLOSE, &CHandleInfoDlg::OnHandleForceClose)
	ON_COMMAND(ID_HANDLE_EXPORT_TEXT, &CHandleInfoDlg::OnHandleExportText)
	ON_COMMAND(ID_HANDLE_EXPORT_EXCEL, &CHandleInfoDlg::OnHandleExportExcel)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CHandleInfoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_HANDLE_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CHandleInfoDlg 消息处理程序

void CHandleInfoDlg::OnBnClickedOk()
{
}

BOOL CHandleInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szHandleValue[g_enumLang], LVCFMT_LEFT, 60);
	m_list.InsertColumn(1, szHandleObject[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(2, szHandleType[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(3, szHandleRefCount[g_enumLang], LVCFMT_CENTER, 80);
	m_list.InsertColumn(4, szHandleName[g_enumLang], LVCFMT_LEFT, 410);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CHandleInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CHandleInfoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_HANDLE_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_HANDLE_EXPORT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_HANDLE_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_HANDLE_CLOSE, szHandleClose[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_HANDLE_FORCE_CLOSE, szHandleForceClose[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_HANDLE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_HANDLE_CLOSE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_HANDLE_FORCE_CLOSE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(5, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 6; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = 0;
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = m_list.GetNextSelectedItem(pos);
			nCnt++;
		}

		if (!nCnt)
		{
			for (int i = 2; i < 5; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CHandleInfoDlg::OnHandleRefresh()
{
	ListProcessHandle();
}

void CHandleInfoDlg::OnHandleClose()
{
	KillHandle(FALSE);
}

void CHandleInfoDlg::OnHandleForceClose()
{
	KillHandle(TRUE);
}

void CHandleInfoDlg::OnHandleExportText()
{
	m_Functions.ExportListToTxt(&m_list, m_szState);
}

void CHandleInfoDlg::OnHandleExportExcel()
{
	WCHAR szHandle[] = {'H','a','n','d','l','e','\0'};
	m_Functions.ExportListToExcel(&m_list, szHandle, m_szState);
}

void CHandleInfoDlg::KillHandle(BOOL bForce)
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	while (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString strHandle = m_list.GetItemText(iIndex, 0);
// 		for ( vector <HANDLE_INFO>::iterator Iter = m_HandleVector.begin( ); Iter != m_HandleVector.end( ); Iter++ )
// 		{
// 			CString szHandle;
// 			szHandle.Format(L"0x%04X", Iter->Handle);
// 
// 			if (!strHandle.CompareNoCase(szHandle))
// 			{
// 				COMMUNICATE_HANDLE ch;
// 				ch.OpType = enumCloseHandle;
// 				ch.op.Close.nPid = m_dwPid;
// 				ch.op.Close.pEprocess = m_pEprocess;
// 				ch.op.Close.bForceKill = bForce;
// 				ch.op.CloseHandle.hHandle = Iter->Handle;
// 				ch.op.CloseHandle.pHandleObject = Iter->Object;
// 				m_Driver.CommunicateDriver(&ch, sizeof(COMMUNICATE_HANDLE), NULL, 0, NULL);
// 				break;
// 			}	
// 		}
// 
// 		m_list.DeleteItem(iIndex);
// 		pos = m_list.GetFirstSelectedItemPosition();
// 	}
}

BOOL CHandleInfoDlg::ListProcessHandle()
{
	m_szState.Format(szHandleInfoState[g_enumLang], 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_HandleVector.clear();

	ULONG nHandleCnt = 0;

	GetProcessHandleInfo();
	if (!m_HandleVector.empty())
	{
		//	SortByHandle();

		for ( vector <HANDLE_INFO>::iterator Iter = m_HandleVector.begin( ); Iter != m_HandleVector.end( ); Iter++ )
		{
			InsertHandleIntoList(*Iter);
			nHandleCnt++;
		}
	}

	m_szState.Format(szHandleInfoState[g_enumLang], nHandleCnt);
	UpdateData(FALSE);

	return TRUE;
}

VOID CHandleInfoDlg::GetProcessHandleInfo()
{
}

BOOL SortHandleInfoType( HANDLE_INFO elem1, HANDLE_INFO elem2 )
{
	return wcscmp(elem1.ObjectName, elem2.ObjectName);
}

VOID CHandleInfoDlg::SortByHandle()
{
	sort( m_HandleVector.begin(), m_HandleVector.end(), SortHandleInfoType );
}

VOID CHandleInfoDlg::InsertHandleIntoList(HANDLE_INFO HandleInfo)
{
	CString szHandle, szObject, szReferenceCount;

	szHandle.Format(L"0x%04X", HandleInfo.Handle);
	szObject.Format(L"0x%08X", HandleInfo.Object);
	szReferenceCount.Format(L"%d", HandleInfo.ReferenceCount);

	int n = m_list.InsertItem(m_list.GetItemCount(), szHandle);
	m_list.SetItemText(n, 1, szObject);
	m_list.SetItemText(n, 2, HandleInfo.ObjectName);
	m_list.SetItemText(n, 3, szReferenceCount);
	m_list.SetItemText(n, 4, HandleInfo.HandleName);
}