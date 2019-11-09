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
// ProcessMemoryDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ProcessMemoryDlg.h"
#include "DumpMemoryDlg.h"
#include "ModifyProtectDlg.h"
#include <algorithm>
#include <shlwapi.h>

typedef enum _MEMORY_HEADER_INDEX
{
	eMemoryBase,
	eMemorySize,
	eMemoryProtect,
	eMemoryState,
	eMemoryType,
	eMmeoryModuleName,
}MEMORY_HEADER_INDEX;

// CProcessMemoryDlg 对话框

IMPLEMENT_DYNAMIC(CProcessMemoryDlg, CDialog)

CProcessMemoryDlg::CProcessMemoryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessMemoryDlg::IDD, pParent)
{
	m_szImage = L"";
	m_dwPid = 0;
	m_pEprocess = 0;
	m_nMemoryCnt = 0;
	m_hDlgWnd = NULL;
}

CProcessMemoryDlg::~CProcessMemoryDlg()
{
	m_vectorMemory.clear();
	m_vectorProtectType.clear();
}

void CProcessMemoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CProcessMemoryDlg, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_MEMORY_Refresh, &CProcessMemoryDlg::OnMemoryRefresh)
	ON_COMMAND(ID_MEMORY_DUMP, &CProcessMemoryDlg::OnMemoryDump)
	ON_COMMAND(ID_MEMORY_ZERO_MEMORY, &CProcessMemoryDlg::OnMemoryZeroMemory)
	ON_COMMAND(ID_MEMORY_CHANGE_PROTECT, &CProcessMemoryDlg::OnMemoryChangeProtect)
	ON_COMMAND(ID_MEMORY_FREE_MEMORY, &CProcessMemoryDlg::OnMemoryFreeMemory)
	ON_COMMAND(ID_MEMORY_TEXT, &CProcessMemoryDlg::OnMemoryText)
	ON_COMMAND(ID_MEMORY_EXCEL, &CProcessMemoryDlg::OnMemoryExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CProcessMemoryDlg::OnNMRclickList)
	ON_MESSAGE(WM_MODIFY_PROTECTION, OnModifyProtectionComplete)
	ON_COMMAND(ID_MEMORY_DISASSEMBLE, &CProcessMemoryDlg::OnMemoryDisassemble)
	ON_COMMAND(ID_MEMORY_HEX_EDIT, &CProcessMemoryDlg::OnMemoryHexEdit)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CProcessMemoryDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CProcessMemoryDlg 消息处理程序

BOOL CProcessMemoryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	WCHAR szProtect[] = {'P','r','o','t','e','c','t','\0'};
	WCHAR szState[] = {'S','t','a','t','e','\0'};
	WCHAR szType[] = {'T','y','p','e','\0'};

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(eMemoryBase, szAddress[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eMemorySize, szSize[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eMemoryProtect, szProtect, LVCFMT_LEFT, 150);
	m_list.InsertColumn(eMemoryState, szState, LVCFMT_LEFT, 110);
	m_list.InsertColumn(eMemoryType, szType, LVCFMT_LEFT, 110);
	m_list.InsertColumn(eMmeoryModuleName, szModuleName[g_enumLang], LVCFMT_LEFT, 430);

	InitMemoryProtect();

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// 设置进程信息
//
void CProcessMemoryDlg::SetProcessInfo(DWORD dwPid, DWORD pEprocess, CString szImage, HWND hWnd)
{
	m_dwPid = dwPid;
	m_pEprocess = pEprocess;
	m_szImage = szImage;
	m_hDlgWnd = hWnd;
}

//
// 枚举进程内存
//
BOOL CProcessMemoryDlg::EnumMemorys(ULONG* nCnt)
{
	m_list.DeleteAllItems();
	m_nMemoryCnt = 0;

	// 获取进程内存
	m_clsMemory.GetMemorys(m_dwPid, m_pEprocess, m_vectorMemory);
	if (m_vectorMemory.empty())
	{
		return FALSE;	
	}

	// 获取模块信息
	m_clsListModules.EnumModulesByPeb(m_dwPid, m_pEprocess, m_vectorModules);
	
	for ( vector <MEMORY_INFO>::iterator Iter = m_vectorMemory.begin( ); 
		Iter != m_vectorMemory.end( ); 
		Iter++ )
	{
		MEMORY_INFO item = *Iter;
		CString strBase, strSize, strProtect, strType, strImageName, strState;

		strBase.Format(L"0x%08X", item.Base);
		strSize.Format(L"0x%08X", item.Size);
		strProtect = GetMemoryProtect(item.Protect);
		strType = GetMempryType(item.Type);
		strState = GetMempryState(item.State);

		if (item.Type == MEM_IMAGE)
		{
			strImageName = GetModuleImageName(item.Base);
		}
		
		int n = m_list.GetItemCount();
		int i = m_list.InsertItem(n, strBase);
		m_list.SetItemText(i, eMemorySize, strSize);
		m_list.SetItemText(i, eMemoryProtect, strProtect);
		m_list.SetItemText(i, eMemoryState, strState);
		m_list.SetItemText(i, eMemoryType, strType);
		m_list.SetItemText(i, eMmeoryModuleName, strImageName);
		
		m_list.SetItemData(i, i);
		m_nMemoryCnt++;
	}

	if (nCnt)
	{
		*nCnt = m_nMemoryCnt;
	}

	return TRUE;
}

//
// 初始化内存Protect属性
//
VOID CProcessMemoryDlg::InitMemoryProtect()
{
	PROTECT protect;

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_NOACCESS;
	wcsncpy_s(protect.szTypeName, L"No Access", wcslen(L"No Access"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_READONLY;
	wcsncpy_s(protect.szTypeName, L"Read", wcslen(L"Read"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_READWRITE;
	wcsncpy_s(protect.szTypeName, L"ReadWrite", wcslen(L"ReadWrite"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_WRITECOPY;
	wcsncpy_s(protect.szTypeName, L"WriteCopy", wcslen(L"WriteCopy"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE;
	wcsncpy_s(protect.szTypeName, L"Execute", wcslen(L"Execute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_READ;
	wcsncpy_s(protect.szTypeName, L"ReadExecute", wcslen(L"ReadExecute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_READWRITE;
	wcsncpy_s(protect.szTypeName, L"ReadWriteExecute", wcslen(L"ReadWriteExecute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_WRITECOPY;
	wcsncpy_s(protect.szTypeName, L"WriteCopyExecute", wcslen(L"WriteCopyExecute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_GUARD;
	wcsncpy_s(protect.szTypeName, L"Guard", wcslen(L"Guard"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_NOCACHE;
	wcsncpy_s(protect.szTypeName, L"No Cache", wcslen(L"No Cache"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_WRITECOMBINE;
	wcsncpy_s(protect.szTypeName, L"WriteCombine", wcslen(L"WriteCombine"));
	m_vectorProtectType.push_back(protect);
}

//
// 获取内存Protect属性
//
CString CProcessMemoryDlg::GetMemoryProtect(ULONG Protect)
{
	BOOL bFirst = TRUE;
	CString strProtect = L"";

	for ( vector <PROTECT>::iterator Iter = m_vectorProtectType.begin( ); 
		Iter != m_vectorProtectType.end( ); 
		Iter++ )
	{
		PROTECT item = *Iter;
		if (item.uType & Protect)
		{
			if (bFirst == TRUE)
			{
				strProtect = item.szTypeName;
				bFirst = FALSE;
			}
			else
			{
				strProtect += L" & ";
				strProtect += item.szTypeName;
			}
		}
	}

	return strProtect;
}

//
// 获取内存状态属性
//
CString CProcessMemoryDlg::GetMempryState(ULONG State)
{
	CString szState = L"";

	if (State == MEM_COMMIT)
	{
		szState = L"Commit";
	}
	else if (State == MEM_FREE)
	{
		szState = L"Free";
	}
	else if (State == MEM_RESERVE)
	{
		szState = L"Reserve";
	}
	else if (State == MEM_DECOMMIT)
	{
		szState = L"Decommit";
	}
	else if (State == MEM_RELEASE)
	{
		szState = L"Release";
	}

	return szState;
}

//
// 获取内存类型属性
//
CString CProcessMemoryDlg::GetMempryType(ULONG Type)
{
	CString szType = L"";

	if (Type == MEM_PRIVATE)
	{
		szType = L"Private";
	}
	else if (Type == MEM_MAPPED)
	{
		szType = L"Map";
	}
	else if (Type == MEM_IMAGE)
	{
		szType = L"Image";
	}

	return szType;
}

void CProcessMemoryDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

//
// 右键菜单 - 刷新
//
void CProcessMemoryDlg::OnMemoryRefresh()
{
//	EnumMemorys();
	::SendMessage(m_hDlgWnd, WM_ENUM_MEMORY, NULL, NULL);
}

//
// 拷贝内存
//
void CProcessMemoryDlg::OnMemoryDump()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	int nData = (int)m_list.GetItemData(nItem);
	MEMORY_INFO info = m_vectorMemory.at(nData);

	CDumpMemoryDlg DumpDlg;
	DumpDlg.m_dwPid = m_dwPid;
	DumpDlg.m_pEprocess = m_pEprocess;
	DumpDlg.m_szAddress.Format(L"%08X", info.Base);
	DumpDlg.m_szSize.Format(L"%08X", info.Size);
	DumpDlg.DoModal();
}

//
// 清零内存
//
void CProcessMemoryDlg::OnMemoryZeroMemory()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	if (MessageBox(szMemoryDangerous[g_enumLang], szToolName, MB_YESNO | MB_ICONWARNING) == IDNO)
	{
		return;
	}
	
	int nData = (int)m_list.GetItemData(nItem);
	MEMORY_INFO info = m_vectorMemory.at(nData);
	if (!m_clsMemory.ZeroOfMemory(m_dwPid, m_pEprocess, info.Base, info.Size))
	{
		MessageBox(szZeroMemoryFailed[g_enumLang], szToolName, MB_OK);
	}
	else
	{
		MessageBox(szZeroMemoryOk[g_enumLang], szToolName, MB_OK);
	}
}

//
// 改变保护属性
//
void CProcessMemoryDlg::OnMemoryChangeProtect()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	int nData = (int)m_list.GetItemData(nItem);
	MEMORY_INFO info = m_vectorMemory.at(nData);

	CModifyProtectDlg ModifyDlg;
	ModifyDlg.m_size = info.Size;
	ModifyDlg.m_base = info.Base;
	ModifyDlg.m_pid = m_dwPid;
	ModifyDlg.m_pEprocess = m_pEprocess;
	ModifyDlg.m_hMemoryWnd = this->m_hWnd;
	ModifyDlg.DoModal();
}

//
// 释放内存
//
void CProcessMemoryDlg::OnMemoryFreeMemory()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	if (MessageBox(szMemoryDangerous[g_enumLang], szToolName, MB_YESNO | MB_ICONWARNING) == IDNO)
	{
		return;
	}

	int nData = (int)m_list.GetItemData(nItem);
	MEMORY_INFO info = m_vectorMemory.at(nData);
	if (m_clsMemory.FreeMemory(m_dwPid, m_pEprocess, info.Base, info.Size))
	{
		m_list.DeleteItem(nItem);
	}
	else
	{	
		MessageBox(szMemoryFreeFailed[g_enumLang], szToolName, MB_OK | MB_ICONERROR);
	}

}

//
// 导出到text
//
void CProcessMemoryDlg::OnMemoryText()
{
	CString szStatus;
	szStatus.Format(szMemoryInfoState[g_enumLang], m_nMemoryCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;

	m_Functions.ExportListToTxt(&m_list, szText);
}

//
// 导出到excel
//
void CProcessMemoryDlg::OnMemoryExcel()
{
	WCHAR szMemory[] = {'M','e','m','o','r','y','\0'};
	CString szStatus;
	szStatus.Format(szMemoryInfoState[g_enumLang], m_nMemoryCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;
	m_Functions.ExportListToExcel(&m_list, szMemory, szText);
}

//
// 根据内存地址获得image name
//
CString CProcessMemoryDlg::GetModuleImageName(DWORD dwBase)
{
	CString strImageName = L"";

	for ( vector <MODULE_INFO>::iterator Iter = m_vectorModules.begin( ); Iter != m_vectorModules.end( ); Iter++)
	{	
		MODULE_INFO entry = *Iter;

		if (dwBase >= entry.Base && dwBase <= (entry.Base + entry.Size))
		{
			CString strPath = entry.Path;
			strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
			break;
		}
	}

	return strImageName;
}

//
// 右键菜单
//
void CProcessMemoryDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_MEMORY_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_MEMORY_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_MEMORY_Refresh, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_HEX_EDIT, szMemoryHexEdit[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_DISASSEMBLE, szDisassemble[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_DUMP, szMemoryDump[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_MEMORY_ZERO_MEMORY, szMemoryZero[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_MEMORY_FREE_MEMORY, szMemoryFree[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_MEMORY_CHANGE_PROTECT, szMemoryModifyProtection[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_MEMORY_Refresh, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_MEMORY_DISASSEMBLE, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
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
		if (!m_list.GetSelectedCount()) 
		{
			for (int i = 2; i < 11; i++) // 如果没有选中, "导出到..."功能可用
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				CString strProtect = m_list.GetItemText(nItem, eMemoryProtect);
				CString strState = m_list.GetItemText(nItem, eMemoryState);

				// 如果不是可写的,那么"清零"置灰
				if (strProtect.Find(L"Write") == -1)
				{
					menu.EnableMenuItem(ID_MEMORY_ZERO_MEMORY, MF_GRAYED | MF_DISABLED);
				}

				if (!strState.CompareNoCase(L"Free") && strProtect.Find(L"No") != -1)
				{
					menu.EnableMenuItem(ID_MEMORY_FREE_MEMORY, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_MEMORY_CHANGE_PROTECT, MF_GRAYED | MF_DISABLED);
				}

				if (!strState.CompareNoCase(L"Reserve"))
				{
					menu.EnableMenuItem(ID_MEMORY_CHANGE_PROTECT, MF_GRAYED | MF_DISABLED);
				}

				menu.EnableMenuItem(ID_MEMORY_DISASSEMBLE, MF_ENABLED);
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
// 修改内存属性成功了
//
LRESULT CProcessMemoryDlg::OnModifyProtectionComplete(WPARAM wParam, LPARAM lParam)
{
	CString szProtection = GetMemoryProtect((ULONG)wParam);
	if (!szProtection.IsEmpty())
	{
		int nItem = m_Functions.GetSelectItem(&m_list);
		if (nItem != -1)
		{
			m_list.SetItemText(nItem, eMemoryProtect, szProtection);
		}
	}
	
	return 0;
}

//
// 反汇编器
//
void CProcessMemoryDlg::OnMemoryDisassemble()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		MEMORY_INFO info = m_vectorMemory.at(nData);
		m_Functions.Disassemble(info.Base, 100, m_dwPid, m_pEprocess);
	}
}

//
// 内存编辑器
//
void CProcessMemoryDlg::OnMemoryHexEdit()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		MEMORY_INFO info = m_vectorMemory.at(nData);
		m_Functions.HexEdit(info.Base, 0x1000, m_dwPid, m_pEprocess);
	}
}

//
// 屏蔽按键
//
BOOL CProcessMemoryDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}
