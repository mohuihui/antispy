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
// HotKeyInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HotKeyInfoDlg.h"

// CHotKeyInfoDlg 对话框

IMPLEMENT_DYNAMIC(CHotKeyInfoDlg, CDialog)

CHotKeyInfoDlg::CHotKeyInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHotKeyInfoDlg::IDD, pParent)
	, m_szState(_T(""))
{
	m_dwEprocess = 0;
	m_dwPid = 0;
	m_szImage = L"";
	m_bShowAll = FALSE;
	m_nCnt = 0;
}

CHotKeyInfoDlg::~CHotKeyInfoDlg()
{
	m_HotKeyVector.clear();
}

void CHotKeyInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_HOTKEY_INFO_CNT, m_szState);
}


BEGIN_MESSAGE_MAP(CHotKeyInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CHotKeyInfoDlg::OnBnClickedOk)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CHotKeyInfoDlg::OnNMRclickList)
	ON_WM_SIZE()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_HOTKEY_REFRESH, &CHotKeyInfoDlg::OnHotkeyRefresh)
	ON_UPDATE_COMMAND_UI(ID_HOTKEY_SHOW_ALL_PROCESS, &CHotKeyInfoDlg::OnUpdateHotkeyShowAllProcess)
	ON_COMMAND(ID_HOTKEY_REMOVE, &CHotKeyInfoDlg::OnHotkeyRemove)
	ON_COMMAND(ID_HOTKEY_TEXT, &CHotKeyInfoDlg::OnHotkeyText)
	ON_COMMAND(ID_HOTKEY_EXCEL, &CHotKeyInfoDlg::OnHotkeyExcel)
	ON_COMMAND(ID_HOTKEY_SHOW_ALL_PROCESS, &CHotKeyInfoDlg::OnHotkeyShowAllProcess)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CHotKeyInfoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_HOTKEY_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP


// CHotKeyInfoDlg 消息处理程序

void CHotKeyInfoDlg::OnBnClickedOk()
{
}

void CHotKeyInfoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CHotKeyInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CHotKeyInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);

	m_list.InsertColumn(0, szHotKeyObject[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(1, szHotKeyId[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(2, szHotKeyKey[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(3, szHotKeyTid[g_enumLang], LVCFMT_CENTER, 80);
	m_list.InsertColumn(4, szHotKeyPid[g_enumLang], LVCFMT_CENTER, 80);
	m_list.InsertColumn(5, szHotKeyProcessName[g_enumLang], LVCFMT_LEFT, 220);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CHotKeyInfoDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

void CHotKeyInfoDlg::OnHotkeyRefresh()
{
	// TODO: 在此添加命令处理程序代码
}

void CHotKeyInfoDlg::OnUpdateHotkeyShowAllProcess(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAll);
}

void CHotKeyInfoDlg::OnHotkeyRemove()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szHotKeyObject = m_list.GetItemText(iIndex, 0);
		for ( vector <HOTKEY_ITEM>::iterator Iter = m_HotKeyVector.begin( ); Iter != m_HotKeyVector.end( ); Iter++ )
		{
			CString strHotKeyObject;
			strHotKeyObject.Format(L"0x%08X", Iter->HotKeyObject);
			if (!strHotKeyObject.CompareNoCase(szHotKeyObject))
			{
				COMMUNICATE_HOTKEY ch;
				ch.op.Remove.HotKeyObject = Iter->HotKeyObject;
				ch.OpType = enumRemoveHotKey;
				m_driver.CommunicateDriver(&ch, sizeof(COMMUNICATE_HOTKEY), NULL, 0, NULL);
				break;
			}	
		}

		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

void CHotKeyInfoDlg::OnHotkeyText()
{
	m_Functions.ExportListToTxt(&m_list, m_szState);
}

void CHotKeyInfoDlg::OnHotkeyExcel()
{
	WCHAR szHotKey[] = {'H','o','t','K','e','y','\0'};
	m_Functions.ExportListToExcel(&m_list, szHotKey, m_szState);
}

//
// 枚举进程热键
//
VOID CHotKeyInfoDlg::EnumHotKeys()
{

	m_list.DeleteAllItems();
	m_HotKeyVector.clear();

	InsertHotKeyItems();
}

void CHotKeyInfoDlg::InsertOneItem(HOTKEY_ITEM item, DWORD dwPid, CString szImageName)
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
}

void CHotKeyInfoDlg::InsertHotKeyItems()
{
	ULONG nHotKeyCnt = 0;

	vector<PROCESS_INFO> vectorProcess;
	CListProcess ListProc;
	ListProc.EnumProcess(vectorProcess);

	for (vector<HOTKEY_ITEM>::iterator Iter = m_HotKeyVector.begin(); Iter != m_HotKeyVector.end(); Iter++)
	{
		if (m_bShowAll)
		{
			for ( vector <PROCESS_INFO>::iterator IterProcess = vectorProcess.begin( ); IterProcess != vectorProcess.end( ); IterProcess++ )
			{
				if (IterProcess->ulEprocess == Iter->pEprocess)
				{
					CString strPath = IterProcess->szPath;
					CString szProcessName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1); 
					InsertOneItem(*Iter, IterProcess->ulPid, szProcessName);
					nHotKeyCnt++;
					break;
				}
			}
		}
		else
		{
			if (Iter->pEprocess == m_dwEprocess)
			{
				InsertOneItem(*Iter, m_dwPid, m_szImage);
				nHotKeyCnt++;
			}
		}
	}

	CString szName;
	if (m_bShowAll)
	{
		szName = szAllProcesses[g_enumLang];
	}
	else
	{
		szName = m_szImage;
	}

	m_szState.Format(szHotKeyState[g_enumLang], szName, nHotKeyCnt);
	UpdateData(FALSE);
}

CString CHotKeyInfoDlg::PinJieHotKey(ULONG fsModifiers, ULONG vk)
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
		if ( szVK )
			szHotKey += szVK;
	}

	szHotKey.TrimRight(L" ");
	szHotKey.TrimRight(L"+");
	szHotKey.TrimRight(L" ");

	return szHotKey;
}

void CHotKeyInfoDlg::OnHotkeyShowAllProcess()
{
	m_bShowAll = !m_bShowAll;
	EnumHotKeys();
}
