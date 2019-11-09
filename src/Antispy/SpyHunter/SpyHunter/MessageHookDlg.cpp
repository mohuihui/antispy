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
// MessageHookDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "MessageHookDlg.h"


// CMessageHookDlg 对话框

IMPLEMENT_DYNAMIC(CMessageHookDlg, CDialog)

CMessageHookDlg::CMessageHookDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMessageHookDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_dwCnt = 0;
	m_dwGlobalCnt = 0;
	m_bOnlyShowGlobal = TRUE;
}

CMessageHookDlg::~CMessageHookDlg()
{
}

void CMessageHookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MESSAGE_HOOK_STATUS, m_szStatus);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CMessageHookDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMessageHookDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CMessageHookDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CMessageHookDlg::OnNMCustomdrawList)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_MESSAGE_FLRESH, &CMessageHookDlg::OnMessageFlresh)
	ON_COMMAND(ID_MESSAGE_ONLY_SHOW_GLOBAL_HOOK, &CMessageHookDlg::OnMessageOnlyShowGlobalHook)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_ONLY_SHOW_GLOBAL_HOOK, &CMessageHookDlg::OnUpdateMessageOnlyShowGlobalHook)
	ON_COMMAND(ID_MESSAGE_UNHOOK, &CMessageHookDlg::OnMessageUnhook)
	ON_COMMAND(ID_MESSAGE_DISASM, &CMessageHookDlg::OnMessageDisasm)
	ON_COMMAND(ID_MESSAGE_LOCATION_FILE, &CMessageHookDlg::OnMessageLocationFile)
	ON_COMMAND(ID_MESSAGE_TEXT, &CMessageHookDlg::OnMessageText)
	ON_COMMAND(ID_MESSAGE_EXCEL, &CMessageHookDlg::OnMessageExcel)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CMessageHookDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CMessageHookDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_MESSAGE_HOOK_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CMessageHookDlg 消息处理程序

void CMessageHookDlg::OnBnClickedOk()
{
}

void CMessageHookDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CMessageHookDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szHandle[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(1, szType[g_enumLang], LVCFMT_LEFT, 140);
	m_list.InsertColumn(2, szFunctionAddress[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(3, szModuleName[g_enumLang], LVCFMT_LEFT, 190);
	m_list.InsertColumn(4, szTid[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(5, szPid[g_enumLang], LVCFMT_CENTER, 100);
	m_list.InsertColumn(6, szProcessPath[g_enumLang], LVCFMT_LEFT, 420);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMessageHookDlg::EnumMessageHook()
{
	m_list.DeleteAllItems();
	m_dwCnt = 0;
	m_dwGlobalCnt = 0;

	ULONG uWowHandler = (ULONG)GetProcAddress(LoadLibraryW(L"user32.dll"), "UserRegisterWowHandlers");
	if (uWowHandler)
	{
		COMMUNICATE_MESSAGE_HOOK Cmh;

		Cmh.op.Get.UserRegisterWowHandlers = uWowHandler;
		Cmh.OpType = enumMessageHook;
		
		ULONG nCnt = 200;
		PMESSAGE_HOOK pMessageHookInfo = NULL;
		BOOL bRet = FALSE;

		do 
		{
			ULONG nSize = sizeof(MESSAGE_HOOK) + nCnt * sizeof(MESSAGE_HOOK_INFO);

			if (pMessageHookInfo)
			{
				free(pMessageHookInfo);
				pMessageHookInfo = NULL;
			}

			pMessageHookInfo = (PMESSAGE_HOOK)malloc(nSize);

			if (pMessageHookInfo)
			{
				memset(pMessageHookInfo, 0, nSize);
				pMessageHookInfo->nCnt = nCnt;
				bRet = m_Driver.CommunicateDriver(&Cmh, sizeof(COMMUNICATE_MESSAGE_HOOK), (PVOID)pMessageHookInfo, nSize, NULL);
			}

			nCnt = pMessageHookInfo->nRetCnt + 10;

		} while (pMessageHookInfo->nRetCnt > pMessageHookInfo->nCnt);

		DebugLog(L"nCnt: %d, nRetCnt: %d", pMessageHookInfo->nCnt, pMessageHookInfo->nRetCnt);

		if (bRet && pMessageHookInfo->nCnt >= pMessageHookInfo->nRetCnt)
		{
			for (ULONG i = 0; i < pMessageHookInfo->nRetCnt; i++)
			{
				InsertMessageHookItem(pMessageHookInfo->MessageHookInfo[i]);
			}
		}

		if (pMessageHookInfo)
		{
			free(pMessageHookInfo);
			pMessageHookInfo = NULL;
		}
	}

	m_szStatus.Format(szMessageHookStatus[g_enumLang], m_dwCnt, m_dwGlobalCnt);
	UpdateData(FALSE);
}

CString CMessageHookDlg::GetTypeName(HOOK_TYPE iHook)
{
	CString szHookType;

	switch(iHook)
	{
	case R_WH_MSGFILTER:
		szHookType = L"WH_MSGFILTER";
		break;

	case R_WH_JOURNALRECORD: 
		szHookType = L"WH_JOURNALRECORD";
		break;

	case R_WH_JOURNALPLAYBACK:
		szHookType = L"WH_JOURNALPLAYBACK";
		break;

	case R_WH_KEYBOARD:
		szHookType = L"WH_KEYBOARD";
		break;

	case R_WH_GETMESSAGE:
		szHookType = L"WH_GETMESSAGE";
		break;

	case R_WH_CALLWNDPROC: 
		szHookType = L"WH_CALLWNDPROC";
		break;

	case R_WH_CBT: 
		szHookType = L"WH_CBT";
		break;

	case R_WH_SYSMSGFILTER: 
		szHookType = L"WH_SYSMSGFILTER";
		break;

	case R_WH_MOUSE: 
		szHookType = L"WH_MOUSE";
		break;

	case R_WH_HARDWARE: 
		szHookType = L"WH_HARDWARE";
		break;

	case R_WH_DEBUG: 
		szHookType = L"WH_DEBUG";
		break;

	case R_WH_SHELL: 
		szHookType = L"WH_SHELL";
		break;

	case R_WH_FOREGROUNDIDLE: 
		szHookType = L"WH_FOREGROUNDIDLE";
		break;

	case R_WH_CALLWNDPROCRET: 
		szHookType = L"WH_KEYBOARD_LL";
		break;

	case R_WH_KEYBOARD_LL: 
		szHookType = L"WH_MSGFILTER";
		break;

	case R_WH_MOUSE_LL: 
		szHookType = L"WH_MOUSE_LL";
		break;

	default:
		szHookType = L"Unknow Type";
	}
	
	return szHookType;
}

void CMessageHookDlg::InsertMessageHookItem(MESSAGE_HOOK_INFO item)
{
	CString szHandle, szType, szFunctionAddress, szModuleName, szTid, szPid, szProcessPath;

	szHandle.Format(L"0x%08X", item.Handle);
	szType = GetTypeName(item.iHook);
	szFunctionAddress.Format(L"0x%08X", item.offPfn);
	szTid.Format(L"%d", item.Tid);
	szPid.Format(L"%d", item.Pid);
	szProcessPath = item.szPath;

	DebugLog(L"szHandle:%s, szType: %s", szHandle, szType);

	if (!wcslen(item.ModuleName))
	{
		szModuleName = szProcessPath.Right(szProcessPath.GetLength() - szProcessPath.ReverseFind(L'\\') - 1);
	}
	else
	{
		szModuleName = item.ModuleName;
	}

	int n = 0;
	if (m_bOnlyShowGlobal)
	{
		if (item.flags & 1)
		{
			n = m_list.InsertItem(m_list.GetItemCount(), szHandle);
			m_list.SetItemText(n, 1, szType);
			m_list.SetItemText(n, 2, szFunctionAddress);
			m_list.SetItemText(n, 3, szModuleName);
			m_list.SetItemText(n, 4, szTid);
			m_list.SetItemText(n, 5, szPid);
			m_list.SetItemText(n, 6, szProcessPath);
		}
	}
	else
	{
		n = m_list.InsertItem(m_list.GetItemCount(), szHandle);
		m_list.SetItemText(n, 1, szType);
		m_list.SetItemText(n, 2, szFunctionAddress);
		m_list.SetItemText(n, 3, szModuleName);
		m_list.SetItemText(n, 4, szTid);
		m_list.SetItemText(n, 5, szPid);
		m_list.SetItemText(n, 6, szProcessPath);
	}

	if (item.flags & 1)
	{
		m_list.SetItemData(n, 1);
		m_dwGlobalCnt++;
	}

	m_dwCnt++;
}

void CMessageHookDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_MESSAGE_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_MESSAGE_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_MESSAGE_FLRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MESSAGE_ONLY_SHOW_GLOBAL_HOOK, szOnlyShowGlobalHook[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MESSAGE_UNHOOK, szUnhook[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MESSAGE_DISASM, szDisassembleMessageHook[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MESSAGE_LOCATION_FILE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_MESSAGE_FLRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_MESSAGE_UNHOOK, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_MESSAGE_DISASM, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_MESSAGE_LOCATION_FILE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(11, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 12; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt == 0)
		{
			for (int i = 2; i < 10; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (nCnt > 1)
		{
			menu.EnableMenuItem(ID_MESSAGE_LOCATION_FILE, MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_MESSAGE_DISASM, MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_GRAYED | MF_DISABLED);
		}
	}

	menu.EnableMenuItem(ID_MESSAGE_ONLY_SHOW_GLOBAL_HOOK, MF_ENABLED);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CMessageHookDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

	*pResult = CDRF_DODEFAULT;

	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF clrNewTextColor, clrNewBkColor;
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		clrNewTextColor = g_MicrosoftItemClr;//RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );	

		if (m_list.GetItemData(nItem) == 1)
		{
			clrNewTextColor = g_HiddenOrHookItemClr;//RGB( 255, 0, 0 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CMessageHookDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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
		for(UINT i=0; i<count; i++)  
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

void CMessageHookDlg::OnMessageFlresh()
{
	EnumMessageHook();
}

void CMessageHookDlg::OnMessageOnlyShowGlobalHook()
{
	m_bOnlyShowGlobal = !m_bOnlyShowGlobal;
	OnMessageFlresh();
}

void CMessageHookDlg::OnUpdateMessageOnlyShowGlobalHook(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bOnlyShowGlobal);
}

void CMessageHookDlg::OnMessageUnhook()
{
// 	int nItem = m_Functions.GetSelectItem(&m_list);
// 	if (nItem != -1)
// 	{
// 		CString szHandle = m_list.GetItemText(nItem, 0);
// 		DWORD dwHandle = m_Functions.HexStringToLong(szHandle);
// 		if (dwHandle > 0)
// 		{
// 			UnhookWindowsHookEx((HHOOK)dwHandle);
// 		}
// 	}

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szHandle = m_list.GetItemText(iIndex, 0);
		DWORD dwHandle = m_Functions.HexStringToLong(szHandle);
		if (dwHandle > 0)
		{
			UnhookWindowsHookEx((HHOOK)dwHandle);
		}
	}

	OnMessageFlresh();
}

void CMessageHookDlg::OnMessageDisasm()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPid = m_list.GetItemText(nItem, 5);
		DWORD dwPid = _wtoi(szPid);
		CString szAddress = m_list.GetItemText(nItem, 2);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress, 100, dwPid);
		}
	}
}

void CMessageHookDlg::OnMessageLocationFile()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);
		m_Functions.LocationExplorer(szPath);
	}
}

static LRESULT CALLBACK LL_hook_proc( int code, WPARAM wparam, LPARAM lparam )
{
	return CallNextHookEx( 0, code, wparam, lparam );
}


void CMessageHookDlg::OnMessageText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CMessageHookDlg::OnMessageExcel()
{
	WCHAR szMessageHook[] = {'M','e','s','s','a','g','e','H','o','o','k','\0'};
	m_Functions.ExportListToExcel(&m_list, szMessageHook, m_szStatus);
}

BOOL CMessageHookDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CMessageHookDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);
		m_Functions.JmpToFile(szPath);
	}
}
