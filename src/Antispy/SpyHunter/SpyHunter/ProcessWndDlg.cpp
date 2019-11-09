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
// ProcessWndDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ProcessWndDlg.h"
// CProcessWndDlg 对话框

typedef enum _WND_HEADER_INDEX
{
	eWndHandle,
	eWndTitle,
	eWndClassName,
	eWndVisable,
	eWndTid,
	eWndPid,
	eWndImageName,
}WND_HEADER_INDEX;

IMPLEMENT_DYNAMIC(CProcessWndDlg, CDialog)

CProcessWndDlg::CProcessWndDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessWndDlg::IDD, pParent)
{
	m_dwPid = 0;
	m_szImage = L"";
	m_bShowAllProcess = FALSE;
	m_nCnt = 0;
}

CProcessWndDlg::~CProcessWndDlg()
{
	m_vectorWndsTemp.clear();
}

void CProcessWndDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CProcessWndDlg, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_WINDOW_REFRESH, &CProcessWndDlg::OnWindowRefresh)
	ON_COMMAND(ID_WINDOW_SHOW_ALL_PROCESS, &CProcessWndDlg::OnWindowShowAllProcess)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_SHOW_ALL_PROCESS, &CProcessWndDlg::OnUpdateWindowShowAllProcess)
	ON_COMMAND(ID_WINDOW_SHOW, &CProcessWndDlg::OnWindowShow)
	ON_COMMAND(ID_WINDOW_HIDE, &CProcessWndDlg::OnWindowHide)
	ON_COMMAND(ID_WINDOW_MAX, &CProcessWndDlg::OnWindowMax)
	ON_COMMAND(ID_WINDOW_MIN, &CProcessWndDlg::OnWindowMin)
	ON_COMMAND(ID_WINDOW_ACTIVE, &CProcessWndDlg::OnWindowActive)
	ON_COMMAND(ID_WINDOW_FREEZE, &CProcessWndDlg::OnWindowFreeze)
	ON_COMMAND(ID_WINDOW_CLOSE, &CProcessWndDlg::OnWindowClose)
	ON_COMMAND(ID_WINDOW_TOP_ALL, &CProcessWndDlg::OnWindowTopAll)
	ON_COMMAND(ID_WINDOW_TEXT, &CProcessWndDlg::OnWindowText)
	ON_COMMAND(ID_WINDOW_EXCEL, &CProcessWndDlg::OnWindowExcel)
	ON_COMMAND(ID_WINDOW_CANCEL_TOP, &CProcessWndDlg::OnWindowCancelTop)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CProcessWndDlg::OnNMRclickList)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CProcessWndDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CProcessWndDlg 消息处理程序

BOOL CProcessWndDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(eWndHandle, szHwnd[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eWndTitle, szWindowTitle[g_enumLang], LVCFMT_LEFT, 220);
	m_list.InsertColumn(eWndClassName, szClassName[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(eWndVisable, szWndVisible[g_enumLang], LVCFMT_CENTER, 80);
	m_list.InsertColumn(eWndTid, szTid[g_enumLang], LVCFMT_CENTER, 80);
	m_list.InsertColumn(eWndPid, szPid[g_enumLang], LVCFMT_CENTER, 80);
	m_list.InsertColumn(eWndImageName, szProcesseName[g_enumLang], LVCFMT_LEFT, 250);
	
	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	CProcessWndDlg *Dlg = (CProcessWndDlg *)lParam;
	if (Dlg == NULL)
	{
		return FALSE;
	}

	WND_INFO wndinfo;
	wndinfo.hWnd = hwnd;
	wndinfo.uTid = GetWindowThreadProcessId(hwnd, &wndinfo.uPid);
	Dlg->m_vectorWndsTemp.push_back(wndinfo);

	return TRUE;
}

//
// 设置进程信息
//
void CProcessWndDlg::SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd)
{
	m_dwPid = nPid;
	m_szImage = szImage;
	m_hDlgWnd = hWnd;
}

//
// 枚举窗口
//
VOID CProcessWndDlg::EnumProcessWnds(ULONG *nCnt)
{
	m_nCnt = 0;
	m_list.DeleteAllItems();
	m_vectorWnds.clear();
	m_vectorWndsTemp.clear();

	//
	// 内核枚举窗口
	//
	m_clsWnds.EnumWnds(m_vectorWndsTemp);

	// 枚举失败，那么就ring3枚举
	if (!m_vectorWndsTemp.size())
	{
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}

	// 如果是显示全部，那么就枚举下进程
	vector<PROCESS_INFO> vectorProcess;
	if (m_bShowAllProcess)
	{
		CListProcess ListProc;
		ListProc.EnumProcess(vectorProcess);
	}

	for (vector<WND_INFO>::iterator Itor = m_vectorWndsTemp.begin(); 
		Itor != m_vectorWndsTemp.end(); 
		Itor++)
	{
		// 如果全部显示窗口
		if (m_bShowAllProcess)
		{
			for ( vector <PROCESS_INFO>::iterator IterProcess = vectorProcess.begin( ); 
				IterProcess != vectorProcess.end( ); 
				IterProcess++ )
			{
				if (IterProcess->ulPid == Itor->uPid)
				{
					CString strPath = IterProcess->szPath;
					CString szProcessName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1); 
					AddWndItem(*Itor, szProcessName);
					m_nCnt++;
					break;
				}
			}
		}
		
		// 只显示当前进程
		else
		{
			if (Itor->uPid == m_dwPid)
			{
				AddWndItem(*Itor, m_szImage);
				m_nCnt++;
			}
		}
	}

	if (nCnt)
	{
		*nCnt = m_nCnt;
	}
}

void CProcessWndDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

//
// 添加一项
//
void CProcessWndDlg::AddWndItem(WND_INFO item, CString szImageName)
{
	CString szHwnd, szPid, szTid, szVisable;
	HWND hWnd = item.hWnd;
	WCHAR szClassName[MAX_PATH] = {0};
	WCHAR szWindowsText[MAX_PATH] = {0};

	szHwnd.Format(L"0x%08X", hWnd);
	szPid.Format(L"%d", item.uPid);
	szTid.Format(L"%d", item.uTid);
	::GetClassName(hWnd, szClassName, MAX_PATH);
	::GetWindowText(hWnd, szWindowsText, MAX_PATH);

	if (::IsWindowVisible(hWnd))
	{
		szVisable = szVisible[g_enumLang];
	}
	else
	{
		szVisable = L"-";
	}

	int n = m_list.InsertItem(m_list.GetItemCount(), szHwnd);
	m_list.SetItemText(n, eWndTitle, szWindowsText);
	m_list.SetItemText(n, eWndClassName, szClassName);
	m_list.SetItemText(n, eWndVisable, szVisable);
	m_list.SetItemText(n, eWndTid, szTid);
	m_list.SetItemText(n, eWndPid, szPid);
	m_list.SetItemText(n, eWndImageName, szImageName);
	
	m_vectorWnds.push_back(item);
	m_list.SetItemData(n, n);
}

//
// 刷新
//
void CProcessWndDlg::OnWindowRefresh()
{
//	EnumProcessWnds();
	::SendMessage(m_hDlgWnd, WM_ENUM_WINDOWS, NULL, NULL);
}

//
// 显示全部进程的窗口
//
void CProcessWndDlg::OnWindowShowAllProcess()
{
	m_bShowAllProcess = !m_bShowAllProcess;
//	EnumProcessWnds();
	OnWindowRefresh();
}

//
// 显示全部进程的窗口 打钩
//
void CProcessWndDlg::OnUpdateWindowShowAllProcess(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAllProcess);
}

//
// ShowWindow的具体实现
//
void CProcessWndDlg::ShowWindw(int nCmdShow)
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		WND_INFO info = m_vectorWnds.at(nData);
		::ShowWindow(info.hWnd, nCmdShow);
	}
}

//
// EnableWindw的具体实现
//
void CProcessWndDlg::EnableWindw(BOOL bEnable)
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		WND_INFO info = m_vectorWnds.at(nData);
		::EnableWindow(info.hWnd, bEnable);
	}
}

//
// 显示窗口
//
void CProcessWndDlg::OnWindowShow()
{
	ShowWindw(SW_NORMAL);
}

//
// 隐藏窗口
//
void CProcessWndDlg::OnWindowHide()
{
	ShowWindw(SW_HIDE);
}

//
// 最大化窗口
//
void CProcessWndDlg::OnWindowMax()
{
	ShowWindw(SW_SHOWMAXIMIZED);
}

//
// 最小化窗口
//
void CProcessWndDlg::OnWindowMin()
{
	ShowWindw(SW_SHOWMINIMIZED);
}

//
// 激活窗口
//
void CProcessWndDlg::OnWindowActive()
{
	EnableWindw(TRUE);
}

//
// 冰冻窗口
//
void CProcessWndDlg::OnWindowFreeze()
{
	EnableWindw(FALSE);
}

DWORD __stdcall DestroyWnd(LPVOID lpThreadParameter)
{
	HWND hWnd = *(HWND *)(lpThreadParameter);
	ULONG nMax = 1000;

	do 
	{
		::SendMessageW(hWnd, WM_CLOSE, 0, 0);
		::SendMessageW(hWnd, WM_DESTROY, 0, 0);

	} while (nMax--);
	
	return 0;
}

//
// 关闭窗口
//
void CProcessWndDlg::OnWindowClose()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		WND_INFO info = m_vectorWnds.at(nData);
		::DestroyWindow(info.hWnd);
		::PostMessage(info.hWnd, WM_CLOSE, 0, 0);
		::PostMessage(info.hWnd, WM_DESTROY, 0, 0);

		// 开个线程去轰炸她
		DWORD dwTid = 0;
		HANDLE hThread = CreateThread(NULL, 0, DestroyWnd, &(info.hWnd), 0, &dwTid);
		if (hThread)
		{
			CloseHandle(hThread);
		}
	}
}

//
// 置顶窗口
//
void CProcessWndDlg::OnWindowTopAll()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		WND_INFO info = m_vectorWnds.at(nData);
		::SetWindowPos(info.hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

//
// 导出到文本
//
void CProcessWndDlg::OnWindowText()
{
	CString szImage = L"";

	if (m_bShowAllProcess)
	{
		szImage = szAllProcesses[g_enumLang];
	}
	else
	{
		szImage = m_szImage;
	}

	CString szStatus;
	szStatus.Format(szWndStateNew[g_enumLang], m_nCnt);
	CString szText = L"[" + szImage + L"] " + szStatus;

	m_Functions.ExportListToTxt(&m_list, szText);
}

//
// 导出到excel
//
void CProcessWndDlg::OnWindowExcel()
{
	WCHAR szWindow[] = {'W','i','n','d','o','w','\0'};

	CString szImage = L"";

	if (m_bShowAllProcess)
	{
		szImage = szAllProcesses[g_enumLang];
	}
	else
	{
		szImage = m_szImage;
	}

	CString szStatus;
	szStatus.Format(szWndStateNew[g_enumLang], m_nCnt);
	CString szText = L"[" + szImage + L"] " + szStatus;

	m_Functions.ExportListToExcel(&m_list, szWindow, szText);
}

//
// 取消置顶窗口
//
void CProcessWndDlg::OnWindowCancelTop()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		WND_INFO info = m_vectorWnds.at(nData);
		::SetWindowPos(info.hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

//
// 右键菜单
//
void CProcessWndDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CProcessWndDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

//
// 屏蔽下按键
//
BOOL CProcessWndDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}
