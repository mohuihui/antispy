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
// HotKeyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HotKeyDlg.h"

CString szVkArray[] = {
	L"",
	L"SOH",
	L"STX",
	L"ETX",
	L"EOT",
	L"ENQ",
	L"ACK",
	L"BEL",
	L"Backspace",
	L"Tab",
	L"LF",
	L"VT",
	L"FF",
	L"Enter",
	L"SO",
	L"SI",
	L"Shift",
	L"Ctrl",
	L"Alt",
	L"Pause",
	L"Caps Lock",
	L"NAK",
	L"SYN",
	L"ETB",
	L"CAN",
	L"EM",
	L"SUB",
	L"Esc",
	L"FS",
	L"GS",
	L"RS",
	L"US",
	L"Space",
	L"PageUp",
	L"PageDown",
	L"End",
	L"Home",
	L"Left",
	L"Up",
	L"Right",
	L"Down",
	L"Select",
	L"Print",
	L"Execute",
	L"Snapshot",
	L"Insert",
	L"Delete",
	L"Help",
	L"0",
	L"1",
	L"2",
	L"3",
	L"4",
	L"5",
	L"6",
	L"7",
	L"8",
	L"9",
	L":",
	L";",
	L"<",
	L"=",
	L">",
	L"?",
	L"@",
	L"A",
	L"B",
	L"C",
	L"D",
	L"E",
	L"F",
	L"G",
	L"H",
	L"I",
	L"J",
	L"K",
	L"L",
	L"M",
	L"N",
	L"O",
	L"P",
	L"Q",
	L"R",
	L"S",
	L"T",
	L"U",
	L"V",
	L"W",
	L"X",
	L"Y",
	L"Z",
	L"LWin",
	L"RWin",
	L"Apps",
	L"",
	L"Sleep",
	L"NumPad0",
	L"NumPad1",
	L"NumPad2",
	L"NumPad3",
	L"NumPad4",
	L"NumPad5",
	L"NumPad6",
	L"NumPad7",
	L"NumPad8",
	L"NumPad9",
	L"*",
	L"+",
	L"Separator",
	L"-",
	L"Decimal",
	L"Divide",
	L"F1",
	L"F2",
	L"F3",
	L"F4",
	L"F5",
	L"F6",
	L"F7",
	L"F8",
	L"F9",
	L"F10",
	L"F11",
	L"F12",
	L"F13",
	L"F14",
	L"F15",
	L"F16"
};

typedef enum _HOTKEY_HEADER_INDEX
{
	eHotKeyObject,
	eHotKeyId,
	eHotKeyKey,
	eHotKeyTid,
	eHotKeyPid,
	eHotKeyProcessName,
}HOTKEY_HEADER_INDEX;

// CHotKeyDlg 对话框

IMPLEMENT_DYNAMIC(CHotKeyDlg, CDialog)

CHotKeyDlg::CHotKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHotKeyDlg::IDD, pParent)
{
	m_dwEprocess = 0;
	m_dwPid = 0;
	m_szImage = L"";
	m_bShowAll = FALSE;
	m_nCnt = 0;
	m_hDlgWnd = NULL;
}

CHotKeyDlg::~CHotKeyDlg()
{
	m_vectorHotKeysTemp.clear();
}

void CHotKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CHotKeyDlg, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_HOTKEY_REFRESH, &CHotKeyDlg::OnHotkeyRefresh)
	ON_COMMAND(ID_HOTKEY_SHOW_ALL_PROCESS, &CHotKeyDlg::OnHotkeyShowAllProcess)
	ON_UPDATE_COMMAND_UI(ID_HOTKEY_SHOW_ALL_PROCESS, &CHotKeyDlg::OnUpdateHotkeyShowAllProcess)
	ON_COMMAND(ID_HOTKEY_REMOVE, &CHotKeyDlg::OnHotkeyRemove)
	ON_COMMAND(ID_HOTKEY_TEXT, &CHotKeyDlg::OnHotkeyText)
	ON_COMMAND(ID_HOTKEY_EXCEL, &CHotKeyDlg::OnHotkeyExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CHotKeyDlg::OnNMRclickList)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CHotKeyDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CHotKeyDlg 消息处理程序

BOOL CHotKeyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	
	m_list.InsertColumn(eHotKeyObject, szHotKeyObject[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eHotKeyId, szHotKeyId[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eHotKeyKey, szHotKeyKey[g_enumLang], LVCFMT_LEFT, 300);
	m_list.InsertColumn(eHotKeyTid, szHotKeyTid[g_enumLang], LVCFMT_CENTER, 90);
	m_list.InsertColumn(eHotKeyPid, szHotKeyPid[g_enumLang], LVCFMT_CENTER, 90);
	m_list.InsertColumn(eHotKeyProcessName, szHotKeyProcessName[g_enumLang], LVCFMT_LEFT, 300);
	
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CHotKeyDlg::SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd)
{	
	m_dwPid = nPid;
	m_dwEprocess = pEprocess;
	m_szImage = szImage;
	m_hDlgWnd = hWnd;
}

//
// 枚举进程热键
//
VOID CHotKeyDlg::EnumHotKeys(ULONG *nCnt)
{
	m_nCnt = 0;
	m_list.DeleteAllItems();
	m_vectorHotKeys.clear();
	m_vectorHotKeysTemp.clear();

	//
	// 枚举
	// 
	m_clsHotKeys.EnumHotKeys(m_vectorHotKeysTemp);
	if (m_vectorHotKeysTemp.empty())
	{
		return;
	}

	InsertHotKeyItems();

	if (nCnt)
	{
		*nCnt = m_nCnt;
	}
}

//
// 插入一项
//
void CHotKeyDlg::InsertOneItem(HOTKEY_ITEM item, DWORD dwPid, CString szImageName)
{
	CString szHotKeyObject, szId, szPid, szTid, szvk;

	szHotKeyObject.Format(L"0x%08X", item.HotKeyObject);
	szId.Format(L"0x%08X", item.nId);
	szvk = PinJieHotKey(item.fsModifiers, item.vk);
	szTid.Format(L"%d", item.tid);
	szPid.Format(L"%d", dwPid);

	int n = m_list.InsertItem(m_list.GetItemCount(), szHotKeyObject);
	m_list.SetItemText(n, 1, szId);
	m_list.SetItemText(n, 2, szvk);
	m_list.SetItemText(n, 3, szTid);
	m_list.SetItemText(n, 4, szPid);
	m_list.SetItemText(n, 5, szImageName);

	m_list.SetItemData(n, m_nCnt);
	m_vectorHotKeys.push_back(item);
	m_nCnt++;
}

//
// 插入项
//
void CHotKeyDlg::InsertHotKeyItems()
{
	//
	// 如果是枚举全部，那么就枚举下进程
	//
	vector<PROCESS_INFO> vectorProcess;
	if (m_bShowAll)
	{
		CListProcess ListProc;
		ListProc.EnumProcess(vectorProcess);
	}
	
	for (vector<HOTKEY_ITEM>::iterator Iter = m_vectorHotKeysTemp.begin(); 
		Iter != m_vectorHotKeysTemp.end(); 
		Iter++)
	{
		if (m_bShowAll)
		{
			for ( vector <PROCESS_INFO>::iterator IterProcess = vectorProcess.begin( ); 
				IterProcess != vectorProcess.end( ); 
				IterProcess++ )
			{
				if (IterProcess->ulEprocess == Iter->pEprocess)
				{
					CString strPath = IterProcess->szPath;
					CString szProcessName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1); 
					InsertOneItem(*Iter, IterProcess->ulPid, szProcessName);
					break;
				}
			}
		}
		else
		{
			if (Iter->pEprocess == m_dwEprocess)
	 		{
				InsertOneItem(*Iter, m_dwPid, m_szImage);
			}
		}
	}
}

//
// 拼接成hotkey
//
CString CHotKeyDlg::PinJieHotKey(ULONG fsModifiers, ULONG vk)
{
	CString szHotKey, szVK;

	if ( fsModifiers & 8 )
		szHotKey += L"Win + ";

	if ( fsModifiers & 2 )
		szHotKey += L"Ctrl + ";

	if ( fsModifiers & 1 )
		szHotKey += L"Alt + ";

	if ( fsModifiers & 4 )
		szHotKey += L"Shift + ";

	if ( vk < 0x80 )
	{
		szVK = szVkArray[vk];
		if ( !szVK.IsEmpty() )
			szHotKey += szVK;
	}

	szHotKey.TrimRight(L" ");
	szHotKey.TrimRight(L"+");
	szHotKey.TrimRight(L" ");

	return szHotKey;
}

void CHotKeyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

//
// 右键 - 刷新
//
void CHotKeyDlg::OnHotkeyRefresh()
{
//	EnumHotKeys();
	::SendMessage(m_hDlgWnd, WM_ENUM_HOTKEYS, NULL, NULL);
}

//
// 显示全部进程
//
void CHotKeyDlg::OnHotkeyShowAllProcess()
{
	m_bShowAll = !m_bShowAll;
//	EnumHotKeys();
	OnHotkeyRefresh();
}

//
//	显示全部热键前面的钩
//
void CHotKeyDlg::OnUpdateHotkeyShowAllProcess(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAll);
}

//
// 移除热键
//
void CHotKeyDlg::OnHotkeyRemove()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(nItem);
		HOTKEY_ITEM info = m_vectorHotKeys.at(nData);

		m_clsHotKeys.RemoveHotKey(info.HotKeyObject);

		m_list.DeleteItem(nItem);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

//
// 导出到text
//
void CHotKeyDlg::OnHotkeyText()
{
	CString szImage = L"";

	if (m_bShowAll)
	{
		szImage = szAllProcesses[g_enumLang];
	}
	else
	{
		szImage = m_szImage;
	}

	CString szStatus;
	szStatus.Format(szHotKeyStateNew[g_enumLang], m_nCnt);
	CString szText = L"[" + szImage + L"] " + szStatus;

	m_Functions.ExportListToTxt(&m_list, szText);
}

//
// 导出到excel
//
void CHotKeyDlg::OnHotkeyExcel()
{
	WCHAR szHotKey[] = {'H','o','t','K','e','y','\0'};

	CString szImage = L"";

	if (m_bShowAll)
	{
		szImage = szAllProcesses[g_enumLang];
	}
	else
	{
		szImage = m_szImage;
	}

	CString szStatus;
	szStatus.Format(szHotKeyStateNew[g_enumLang], m_nCnt);
	CString szText = L"[" + szImage + L"] " + szStatus;

	m_Functions.ExportListToExcel(&m_list, szHotKey, szText);
}

void CHotKeyDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_HOTKEY_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_HOTKEY_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_HOTKEY_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_HOTKEY_SHOW_ALL_PROCESS, szShowAllProcessesHotKey[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_HOTKEY_REMOVE, szRemove[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_HOTKEY_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_HOTKEY_REMOVE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(6, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 4; i < 7; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetSelectedCount()) 
		{
			menu.EnableMenuItem(ID_HOTKEY_REMOVE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
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
// 系统相关
//
void CHotKeyDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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
BOOL CHotKeyDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}
