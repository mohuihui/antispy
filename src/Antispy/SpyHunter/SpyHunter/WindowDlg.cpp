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
// WindowDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "WindowDlg.h"

// CWindowDlg 对话框

IMPLEMENT_DYNAMIC(CWindowDlg, CDialog)

CWindowDlg::CWindowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWindowDlg::IDD, pParent)
	, m_szText(_T(""))
{
	m_dwPid = 0;
	m_szImage = L"";
	m_bShowAllProcess = FALSE;
}

CWindowDlg::~CWindowDlg()
{
//	m_WndList.clear();
}

void CWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_WND_INFO_CNT, m_szText);
}


BEGIN_MESSAGE_MAP(CWindowDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CWindowDlg::OnBnClickedOk)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CWindowDlg::OnNMRclickList)
	ON_COMMAND(ID_WINDOW_REFRESH, &CWindowDlg::OnWindowRefresh)
	ON_COMMAND(ID_WINDOW_SHOW_ALL_PROCESS, &CWindowDlg::OnWindowShowAllProcess)
	ON_COMMAND(ID_WINDOW_SHOW, &CWindowDlg::OnWindowShow)
	ON_COMMAND(ID_WINDOW_HIDE, &CWindowDlg::OnWindowHide)
	ON_COMMAND(ID_WINDOW_MAX, &CWindowDlg::OnWindowMax)
	ON_COMMAND(ID_WINDOW_MIN, &CWindowDlg::OnWindowMin)
	ON_COMMAND(ID_WINDOW_ACTIVE, &CWindowDlg::OnWindowActive)
	ON_COMMAND(ID_WINDOW_FREEZE, &CWindowDlg::OnWindowFreeze)
	ON_COMMAND(ID_WINDOW_CLOSE, &CWindowDlg::OnWindowClose)
	ON_COMMAND(ID_WINDOW_TOP_ALL, &CWindowDlg::OnWindowTopAll)
	ON_COMMAND(ID_WINDOW_CANCEL_TOP, &CWindowDlg::OnWindowCancelTop)
	ON_COMMAND(ID_WINDOW_TEXT, &CWindowDlg::OnWindowText)
	ON_COMMAND(ID_WINDOW_EXCEL, &CWindowDlg::OnWindowExcel)
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_WINDOW_SHOW_ALL_PROCESS, &CWindowDlg::OnUpdateWindowShowAllProcess)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CWindowDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_WND_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CWindowDlg 消息处理程序

void CWindowDlg::OnBnClickedOk()
{
}

void CWindowDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_WINDOW_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_WINDOW_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_WINDOW_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WINDOW_SHOW_ALL_PROCESS, szShowAllProcessWindow[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WINDOW_SHOW, szShowWindow[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WINDOW_HIDE, szHideWindow[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WINDOW_MAX, szShowMaximized[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WINDOW_MIN, szShowMinimized[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WINDOW_ACTIVE, szEnableWindow[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WINDOW_FREEZE, szDisableWindow[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WINDOW_CLOSE, szDestroyWindow[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WINDOW_TOP_ALL, szTopMost[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WINDOW_CANCEL_TOP, szCancelTopMost[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_WINDOW_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(16, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 4; i < 17; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetSelectedCount()) 
		{
			for (int i = 4; i < 16; i++)
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

void CWindowDlg::OnWindowRefresh()
{
	EnumProcessWnds();
}

void CWindowDlg::OnWindowShowAllProcess()
{
	m_bShowAllProcess = !m_bShowAllProcess;
	EnumProcessWnds();
}

void CWindowDlg::ShowWindw(int nCmdShow)
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	if (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szHwnd = m_list.GetItemText(iIndex, 0);
// 		for (list<PROCESS_WND_INFO>::iterator Itor = m_WndList.begin(); Itor != m_WndList.end(); Itor++)
// 		{
// 			CString strHwnd;
// 			strHwnd.Format(L"0x%08X", Itor->hWnd);
// 			if (!strHwnd.CompareNoCase(szHwnd))
// 			{
// 				::ShowWindow(Itor->hWnd, nCmdShow);
// 				break;
// 			}
// 		}
// 	}
}

void CWindowDlg::OnWindowShow()
{
	ShowWindw(SW_NORMAL);
}

void CWindowDlg::OnWindowHide()
{
	ShowWindw(SW_HIDE);
}

void CWindowDlg::OnWindowMax()
{
	ShowWindw(SW_SHOWMAXIMIZED);
}

void CWindowDlg::OnWindowMin()
{
	ShowWindw(SW_SHOWMINIMIZED);
}

void CWindowDlg::EnableWindw(BOOL bEnable)
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	if (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szHwnd = m_list.GetItemText(iIndex, 0);
// 		for (list<PROCESS_WND_INFO>::iterator Itor = m_WndList.begin(); Itor != m_WndList.end(); Itor++)
// 		{
// 			CString strHwnd;
// 			strHwnd.Format(L"0x%08X", Itor->hWnd);
// 			if (!strHwnd.CompareNoCase(szHwnd))
// 			{
// 				::EnableWindow(Itor->hWnd, bEnable);
// 				break;
// 			}
// 		}
// 	}
}

void CWindowDlg::OnWindowActive()
{
	EnableWindw(TRUE);
}

void CWindowDlg::OnWindowFreeze()
{
	EnableWindw(FALSE);
}

DWORD __stdcall DestroyWndInfoWnd(LPVOID lpThreadParameter)
{
	HWND hWnd = *(HWND *)(lpThreadParameter);
	::SendMessageW(hWnd, WM_CLOSE, 0, 0);
	::SendMessageW(hWnd, WM_DESTROY, 0, 0);
	//	::SendMessageW(hWnd, WM_QUIT, 0, 0);
	//	::SendMessageW(hWnd, 0x19u, 0, 0);
	return 0;
}

void CWindowDlg::OnWindowClose()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	if (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szHwnd = m_list.GetItemText(iIndex, 0);
// 		for (list<PROCESS_WND_INFO>::iterator Itor = m_WndList.begin(); Itor != m_WndList.end(); Itor++)
// 		{
// 			CString strHwnd;
// 			strHwnd.Format(L"0x%08X", Itor->hWnd);
// 			if (!strHwnd.CompareNoCase(szHwnd))
// 			{
// 				::DestroyWindow(Itor->hWnd);
// 				::PostMessage(Itor->hWnd, WM_CLOSE, 0, 0);
// 				::PostMessage(Itor->hWnd, WM_DESTROY, 0, 0);
// 				//	::PostMessage(Itor->hWnd, WM_QUIT, 0, 0);
// 				//	::PostMessage(Itor->hWnd, 0x19, 0, 0);
// 				DWORD dwTid = 0;
// 				HANDLE hThread = CreateThread(NULL, 0, DestroyWndInfoWnd, &(Itor->hWnd), 0, &dwTid);
// 				if (hThread)
// 				{
// 					CloseHandle(hThread);
// 				}
// 				break;
// 			}
// 		}
// 	}
}

void CWindowDlg::OnWindowTopAll()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	if (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szHwnd = m_list.GetItemText(iIndex, 0);
// 		for (list<PROCESS_WND_INFO>::iterator Itor = m_WndList.begin(); Itor != m_WndList.end(); Itor++)
// 		{
// 			CString strHwnd;
// 			strHwnd.Format(L"0x%08X", Itor->hWnd);
// 			if (!strHwnd.CompareNoCase(szHwnd))
// 			{
// 				::SetWindowPos(Itor->hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
// 				break;
// 			}
// 		}
// 	}
}

void CWindowDlg::OnWindowCancelTop()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	if (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szHwnd = m_list.GetItemText(iIndex, 0);
// 		for (list<PROCESS_WND_INFO>::iterator Itor = m_WndList.begin(); Itor != m_WndList.end(); Itor++)
// 		{
// 			CString strHwnd;
// 			strHwnd.Format(L"0x%08X", Itor->hWnd);
// 			if (!strHwnd.CompareNoCase(szHwnd))
// 			{
// 				::SetWindowPos(Itor->hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
// 				break;
// 			}
// 		}
// 	}
}

void CWindowDlg::OnWindowText()
{
	m_Functions.ExportListToTxt(&m_list, m_szText);
}

void CWindowDlg::OnWindowExcel()
{
	WCHAR szWindow[] = {'W','i','n','d','o','w','\0'};
	m_Functions.ExportListToExcel(&m_list, szWindow, m_szText);
}

BOOL CWindowDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szHwnd[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(1, szWindowTitle[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(2, szClassName[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(3, szWndVisible[g_enumLang], LVCFMT_CENTER, 70);
	m_list.InsertColumn(4, szTid[g_enumLang], LVCFMT_CENTER, 70);
	m_list.InsertColumn(5, szPid[g_enumLang], LVCFMT_CENTER, 70);
	m_list.InsertColumn(6, szProcesseName[g_enumLang], LVCFMT_LEFT, 140);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CWindowDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

// void CWindowDlg::InsertOneWnd(PROCESS_WND_INFO item, CString szImageName)
// {
// 	CString szHwnd, szPid, szTid, szVisable;
// 	HWND hWnd = item.hWnd;
// 	WCHAR szClassName[MAX_PATH] = {0};
// 	WCHAR szWindowsText[MAX_PATH] = {0};
// 
// 	szHwnd.Format(L"0x%08X", hWnd);
// 	szPid.Format(L"%d", item.uPid);
// 	szTid.Format(L"%d", item.uTid);
// 	::GetClassName(hWnd, szClassName, MAX_PATH);
// 	::GetWindowText(hWnd, szWindowsText, MAX_PATH);
// 	if (::IsWindowVisible(hWnd))
// 	{
// 		szVisable = szVisible[g_enumLang];
// 	}
// 	else
// 	{
// 		szVisable = L"-";
// 	}
// 
// 	int n = m_list.InsertItem(m_list.GetItemCount(), szHwnd);
// 	m_list.SetItemText(n, 1, szWindowsText);
// 	m_list.SetItemText(n, 2, szClassName);
// 	m_list.SetItemText(n, 3, szVisable);
// 	m_list.SetItemText(n, 4, szTid);
// 	m_list.SetItemText(n, 5, szPid);
// 	m_list.SetItemText(n, 6, szImageName);
// 	if (!szVisable.CompareNoCase(L"Yes"))
// 	{
// 		m_list.SetItemData(n, 1);
// 	}
// }

int CALLBACK CompareWinInfoVisable(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{ 
	return lParam1 < lParam2;
}

void CWindowDlg::InsertAllWnd()
{
// 	ULONG iCnt = 0;
// 
// 	vector<PROCESS_INFO> vectorProcess;
// 	CListProcess ListProc;
// 	ListProc.EnumProcess(vectorProcess);
// 
// 	for (list<PROCESS_WND_INFO>::iterator Itor = m_WndList.begin(); Itor != m_WndList.end(); Itor++)
// 	{
// 		if (m_bShowAllProcess)
// 		{
// 			for ( vector <PROCESS_INFO>::iterator IterProcess = vectorProcess.begin( ); IterProcess != vectorProcess.end( ); IterProcess++ )
// 			{
// 				if (IterProcess->ulPid == Itor->uPid)
// 				{
// 					CString strPath = IterProcess->szPath;
// 					CString szProcessName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1); 
// 					InsertOneWnd(*Itor, szProcessName);
// 					iCnt++;
// 					break;
// 				}
// 			}
// 		}
// 		else
// 		{
// 			if (Itor->uPid == m_dwPid)
// 			{
// 				InsertOneWnd(*Itor, m_szImage);
// 				iCnt++;
// 			}
// 		}
// 	}
// 
// 	m_list.SortItems(CompareWinInfoVisable, NULL);
// 
// 	CString szImage;
// 	if (m_bShowAllProcess)
// 	{
// 		szImage = szAllProcesses[g_enumLang];
// 	}
// 	else
// 	{
// 		szImage = m_szImage;
// 	}
// 
// 	m_szText.Format(szWndState[g_enumLang], szImage, iCnt);
// 	UpdateData(FALSE);
}

BOOL CALLBACK EnumWindowsInfoProc(HWND hwnd, LPARAM lParam)
{
// 	CWindowDlg *Dlg = (CWindowDlg *)lParam;
// 	PROCESS_WND_INFO wndinfo;
// 	wndinfo.hWnd = hwnd;
// 	wndinfo.uTid = GetWindowThreadProcessId(hwnd, &wndinfo.uPid);
// 	Dlg->m_WndList.push_back(wndinfo);
	return TRUE;
}

VOID CWindowDlg::EnumProcessWnds()
{
// 	BOOL bRet = FALSE;
// 	DWORD dwRet = 0;
// 	ULONG uSize = 1000 * sizeof(PROCESS_WND_INFO);
// 	PVOID pBuffer = NULL;
// 	OPERATE_TYPE ot = enumListProcessWnd;
// 
// 	m_WndList.clear();
// 	m_list.DeleteAllItems();
// 	m_szText.Format(szWndState[g_enumLang], L"", 0);
// 	UpdateData(FALSE);
// 
// 	do 
// 	{
// 		pBuffer = VirtualAlloc(NULL, uSize, MEM_COMMIT, PAGE_READWRITE);
// 		if (!pBuffer)
// 		{
// 			break;
// 		}
// 
// 		memset(pBuffer, 0, uSize);
// 		bRet = m_driver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), pBuffer, uSize, NULL);
// 		if (!bRet)
// 		{
// 			VirtualFree(pBuffer, uSize, MEM_DECOMMIT);
// 			pBuffer = NULL;
// 		}
// 
// 		uSize *= 2;
// 
// 	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);
// 
// 	if (bRet)
// 	{
// 		ULONG nCnt = *(ULONG*)pBuffer;
// 		PPROCESS_WND_INFO pPwi = (PPROCESS_WND_INFO)((ULONG)pBuffer + sizeof(ULONG));
// 
// 		for (ULONG i = 0; i < nCnt; i++)
// 		{
// 			m_WndList.push_back(pPwi[i]);
// 		}
// 	}
// 
// 	if (pBuffer)
// 	{
// 		VirtualFree(pBuffer, uSize, MEM_DECOMMIT);
// 		pBuffer = NULL;
// 	}
// 
// 	if (!m_WndList.size())
// 	{
// 		EnumWindows(EnumWindowsInfoProc, (LPARAM)this);
// 	}
// 
// 	InsertAllWnd();
}

void CWindowDlg::OnUpdateWindowShowAllProcess(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAllProcess);
}

void CWindowDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	if(!bSysMenu && pPopupMenu)  
	{
		CCmdUI cmdUI;  
		cmdUI.m_pOther = NULL;  
		cmdUI.m_pMenu = pPopupMenu;  
		cmdUI.m_pSubMenu = NULL;  

		UINT count = pPopupMenu->GetMenuItemCount();  
		cmdUI.m_nIndexMax = count;  
		for(UINT i=0; i < count; i++)  
		{  
			UINT nID = pPopupMenu->GetMenuItemID(i);  
			if(-1 == nID || 0 == nID)  
			{  
				continue;  
			}  
			cmdUI.m_nID = nID;  
			cmdUI.m_nIndex = i;  
			cmdUI.DoUpdate(this, FALSE);  
		}  
	}
}
