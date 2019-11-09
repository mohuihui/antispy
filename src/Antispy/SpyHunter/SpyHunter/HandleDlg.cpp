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
// HandleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HandleDlg.h"
#include <algorithm>

typedef enum _HANDLE_HEADER_INDEX
{
	eHandleValue,
	eHandleObject,
	eHandleType,
	eHandleRefCount,
	eHandleName
}HANDLE_HEADER_INDEX;

// CHandleDlg 对话框

IMPLEMENT_DYNAMIC(CHandleDlg, CDialog)

CHandleDlg::CHandleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHandleDlg::IDD, pParent)
{
	m_dwPid = 0;
	m_pEprocess = 0;
	m_szImage = "";
	m_nCnt = 0;
	m_hDlgWnd = NULL;
}

CHandleDlg::~CHandleDlg()
{
	m_HandleVector.clear();
}

void CHandleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CHandleDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CHandleDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_COMMAND(ID_HANDLE_REFRESH, &CHandleDlg::OnHandleRefresh)
	ON_COMMAND(ID_HANDLE_CLOSE, &CHandleDlg::OnHandleClose)
	ON_COMMAND(ID_HANDLE_FORCE_CLOSE, &CHandleDlg::OnHandleForceClose)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CHandleDlg::OnNMRclickList)
	ON_COMMAND(ID_HANDLE_EXPORT_TEXT, &CHandleDlg::OnHandleExportText)
	ON_COMMAND(ID_HANDLE_EXPORT_EXCEL, &CHandleDlg::OnHandleExportExcel)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CHandleDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CHandleDlg 消息处理程序

void CHandleDlg::OnBnClickedOk()
{
	return;
}

BOOL CHandleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(eHandleValue, szHandleValue[g_enumLang], LVCFMT_LEFT, 70);
	m_list.InsertColumn(eHandleObject, szHandleObject[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(eHandleType, szHandleType[g_enumLang], LVCFMT_LEFT, 120);
	m_list.InsertColumn(eHandleRefCount, szHandleRefCount[g_enumLang], LVCFMT_CENTER, 80);
	m_list.InsertColumn(eHandleName, szHandleName[g_enumLang], LVCFMT_LEFT, 630);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// 添加项
//
VOID CHandleDlg::AddHandleItem(HANDLE_INFO HandleInfo)
{
	CString szHandle, szObject, szReferenceCount;

	szHandle.Format(L"0x%04X", HandleInfo.Handle);
	szObject.Format(L"0x%08X", HandleInfo.Object);
	szReferenceCount.Format(L"%d", HandleInfo.ReferenceCount);

	int n = m_list.InsertItem(m_list.GetItemCount(), szHandle);
	m_list.SetItemText(n, eHandleObject, szObject);
	m_list.SetItemText(n, eHandleType, HandleInfo.ObjectName);
	m_list.SetItemText(n, eHandleRefCount, szReferenceCount);
	m_list.SetItemText(n, eHandleName, HandleInfo.HandleName);

	m_list.SetItemData(n, m_nCnt);

	m_nCnt++;
}

//
// 设置进程的一些信息
//
void CHandleDlg::SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd)
{
	m_dwPid = nPid;
	m_pEprocess = pEprocess;
	m_szImage = szImage;
	m_hDlgWnd = hWnd;
}

//
// 枚举进程句柄
//
void CHandleDlg::EnumProcessHandles(ULONG *nCnt)
{
	// 清零
	m_list.DeleteAllItems();
	m_HandleVector.clear();
	m_nCnt = 0;

	m_clsHandles.GetHandles(m_dwPid, m_pEprocess, m_HandleVector);
	if (m_HandleVector.empty())
	{
		return;
	}

	for ( vector <HANDLE_INFO>::iterator Iter = m_HandleVector.begin( ); Iter != m_HandleVector.end( ); Iter++ )
	{
		AddHandleItem(*Iter);
	}

	if (nCnt)
	{
		*nCnt = m_nCnt;
	}
}

void CHandleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

//
// 菜单刷新
//
void CHandleDlg::OnHandleRefresh()
{
//	EnumProcessHandles();
	::SendMessage(m_hDlgWnd, WM_ENUM_HANDLES, NULL, NULL);
}

//
// 关闭句柄
//
void CHandleDlg::OnHandleClose()
{
	KillHandle(FALSE);
}

//
// 强制关闭句柄
//
void CHandleDlg::OnHandleForceClose()
{
	KillHandle(TRUE);	
}

//
// 循环关闭句柄
//
void CHandleDlg::KillHandle(BOOL bForce)
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(iIndex);
		HANDLE_INFO info = m_HandleVector.at(nData);
		
		m_clsHandles.CloseHandle(m_dwPid, m_pEprocess, bForce, info.Handle, info.Object);
		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

//
// 右键菜单
//
void CHandleDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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

//
// 导出到text
//
void CHandleDlg::OnHandleExportText()
{
	CString szStatus;
	szStatus.Format(szHandleInfoState[g_enumLang], m_nCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;
	m_Functions.ExportListToTxt(&m_list, szText);
}

//
// 导出到excel
//
void CHandleDlg::OnHandleExportExcel()
{
	WCHAR szHandle[] = {'H','a','n','d','l','e','\0'};
	CString szStatus;
	szStatus.Format(szHandleInfoState[g_enumLang], m_nCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;
	m_Functions.ExportListToExcel(&m_list, szHandle, szText);
}

BOOL CHandleDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}
