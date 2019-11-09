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
// ProcessPrivilegesDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ProcessPrivilegesDlg.h"
#include <algorithm>

typedef enum _PRIVILEGE_HEADER_INDEX
{
	ePrivilegeName,
	ePrivilegeDescription,
	ePrivilegeStatus,
}PRIVILEGE_HEADER_INDEX;

// CProcessPrivilegesDlg 对话框

IMPLEMENT_DYNAMIC(CProcessPrivilegesDlg, CDialog)

CProcessPrivilegesDlg::CProcessPrivilegesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessPrivilegesDlg::IDD, pParent)
{
	m_dwPid = 0;
	m_process = 0;
	m_szImage = L"";
	m_nEnabled = m_nDisabled = 0;
	m_hDlgWnd = NULL;
}

CProcessPrivilegesDlg::~CProcessPrivilegesDlg()
{
	m_vectorPrivileges.clear();
}

void CProcessPrivilegesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(CProcessPrivilegesDlg, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_PRIVILEGE_REFRESH, &CProcessPrivilegesDlg::OnPrivilegeRefresh)
	ON_COMMAND(ID_PRIVILEGE_ENABLE, &CProcessPrivilegesDlg::OnPrivilegeEnable)
	ON_COMMAND(ID_PRIVILEGE_DISABLE, &CProcessPrivilegesDlg::OnPrivilegeDisable)
	ON_COMMAND(ID_PRIVILEGE_TEXT, &CProcessPrivilegesDlg::OnPrivilegeText)
	ON_COMMAND(ID_PRIVILEGE_EXCEL, &CProcessPrivilegesDlg::OnPrivilegeExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CProcessPrivilegesDlg::OnNMRclickList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CProcessPrivilegesDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CProcessPrivilegesDlg 消息处理程序

//
// 设置进程信息
//
void CProcessPrivilegesDlg::SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd)
{
	m_dwPid = nPid;
	m_process = pEprocess;
	m_szImage = szImage;
	m_hDlgWnd = hWnd;
}

BOOL CProcessPrivilegesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(ePrivilegeName, szPrivilege[g_enumLang], LVCFMT_LEFT, 350);
	m_list.InsertColumn(ePrivilegeDescription, szPrivilegeDescription[g_enumLang], LVCFMT_LEFT, 400);
	m_list.InsertColumn(ePrivilegeStatus, szPrivilegeStatus[g_enumLang], LVCFMT_CENTER, 200);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// 枚举进程权限
//
VOID CProcessPrivilegesDlg::GetProcessPrivileges(ULONG *nEnable, ULONG *nDisable)
{	
	// 清零一下
	m_vectorPrivileges.clear();
	m_list.DeleteAllItems();
	m_nEnabled = m_nDisabled = 0;

	// 获取进程全部权限
	m_clsPrivileges.EnumPrivileges(m_dwPid, m_process, m_vectorPrivileges);
	if (m_vectorPrivileges.empty())
	{
		return;
	}

	// 根据权限排序
	SortByPrivilegeName();

	// 插入数据项
	InsertPrivilegesItem();

	if (nEnable)
	{
		*nEnable = m_nEnabled;
	}

	if (nDisable)
	{
		*nDisable = m_nDisabled;
	}
}

BOOL PrivilegesSort( LUID_AND_ATTRIBUTES elem1, LUID_AND_ATTRIBUTES elem2 )
{
	return elem1.Attributes > elem2.Attributes;
}

//
// 根据权限排个序
//
VOID CProcessPrivilegesDlg::SortByPrivilegeName()
{
	sort(m_vectorPrivileges.begin( ), m_vectorPrivileges.end( ), PrivilegesSort);
}

//
// 插入项
//
VOID CProcessPrivilegesDlg::InsertPrivilegesItem()
{
	for ( vector <LUID_AND_ATTRIBUTES>::iterator Iter = m_vectorPrivileges.begin( ); 
		Iter != m_vectorPrivileges.end( ); 
		Iter++)
	{
		LUID_AND_ATTRIBUTES item = *Iter;
		CString szPrivilege, szFlags, szDisplayName;
		WCHAR PrivilegeName[MAX_PATH] = {0};
		WCHAR DisplayName[MAX_PATH] = {0};
		DWORD dwRet1 = MAX_PATH, dwRet2 = MAX_PATH;
		DWORD lpLanguageId = 0;

		LookupPrivilegeName(NULL, &(item.Luid), PrivilegeName, &dwRet1);
		szPrivilege = PrivilegeName;
		LookupPrivilegeDisplayName(NULL, PrivilegeName, DisplayName, &dwRet2, &lpLanguageId);
		szDisplayName = DisplayName;

		if (item.Attributes & 1)
		{
			szFlags = L"Default Enabled";
			m_nEnabled++;
		}
		else if ( item.Attributes & 2 )
		{
			szFlags = L"Enabled";
			m_nEnabled++;
		}
		else
		{
			szFlags = L"-"; //L"Disabled";
			m_nDisabled++;
		}

		int n = m_list.InsertItem(m_list.GetItemCount(), szPrivilege);
		m_list.SetItemText(n, ePrivilegeDescription, szDisplayName);
		m_list.SetItemText(n, ePrivilegeStatus, szFlags);

		m_list.SetItemData(n, n);
	}
}

void CProcessPrivilegesDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

//
// 右键刷新
//
void CProcessPrivilegesDlg::OnPrivilegeRefresh()
{
//	GetProcessPrivileges();
	::SendMessage(m_hDlgWnd, WM_ENUM_PRIVILEGES, NULL, NULL);
}

//
// Enable一项
//
void CProcessPrivilegesDlg::OnPrivilegeEnable()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szPrivilege = m_list.GetItemText(nItem, ePrivilegeName);
	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	if (LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid))
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (m_clsPrivileges.EnablePrivilege(m_dwPid, m_process, &tp))
		{
			m_list.SetItemText(nItem, ePrivilegeStatus, L"Enabled");
		}
	}
}

//
// disbale一项
//
void CProcessPrivilegesDlg::OnPrivilegeDisable()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szPrivilege = m_list.GetItemText(nItem, ePrivilegeName);
	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	if (LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid))
	{
		tp.Privileges[0].Attributes = 0;
		if (m_clsPrivileges.EnablePrivilege(m_dwPid, m_process, &tp))
		{
			m_list.SetItemText(nItem, ePrivilegeStatus, L"-");
		}
	}
}

// 
// 导出到文本
//
void CProcessPrivilegesDlg::OnPrivilegeText()
{
	CString m_szText;
	m_szText.Format(szPrivilegeInfoState[g_enumLang], m_nEnabled, m_nDisabled);
	CString szText = L"[" + m_szImage + L"] " + m_szText;
	m_Functions.ExportListToTxt(&m_list, szText);
}

//
// 导出到excel
//
void CProcessPrivilegesDlg::OnPrivilegeExcel()
{
	WCHAR szPrivilege[] = {'P','r','i','v','i','l','e','g','e','\0'};
	CString m_szText;
	m_szText.Format(szPrivilegeInfoState[g_enumLang], m_nEnabled, m_nDisabled);
	CString szText = L"[" + m_szImage + L"] " + m_szText;
	m_Functions.ExportListToExcel(&m_list, szPrivilege, szText);
}

//
// 右键菜单
//
void CProcessPrivilegesDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_PRIVILEGE_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_PRIVILEGE_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PRIVILEGE_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PRIVILEGE_ENABLE, szPrivilegeEnable[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PRIVILEGE_DISABLE, szPrivilegeDisable[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_PRIVILEGE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
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
		if (!m_list.GetSelectedCount()) 
		{
			menu.EnableMenuItem(ID_PRIVILEGE_ENABLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_PRIVILEGE_DISABLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else
		{
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			if (pos)
			{
				int iIndex  = m_list.GetNextSelectedItem(pos);
				CString szStatus = m_list.GetItemText(iIndex, 2);
				if (!szStatus.CompareNoCase(L"Enabled"))
				{
					menu.EnableMenuItem(ID_PRIVILEGE_ENABLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
				else if (!szStatus.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_PRIVILEGE_DISABLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
				else
				{
					menu.EnableMenuItem(ID_PRIVILEGE_ENABLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PRIVILEGE_DISABLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
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
