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
// PrivilegeInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "PrivilegeInfoDlg.h"
#include <algorithm>

// CPrivilegeInfoDlg 对话框

IMPLEMENT_DYNAMIC(CPrivilegeInfoDlg, CDialog)

CPrivilegeInfoDlg::CPrivilegeInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrivilegeInfoDlg::IDD, pParent)
	, m_szText(_T(""))
{
	m_dwPid = 0;
	m_process = 0;
}

CPrivilegeInfoDlg::~CPrivilegeInfoDlg()
{
	m_PrivilegesVector.clear();
}

void CPrivilegeInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_PRIVILEGE_INFO_CNT, m_szText);
}


BEGIN_MESSAGE_MAP(CPrivilegeInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPrivilegeInfoDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CPrivilegeInfoDlg::OnNMRclickList)
	ON_COMMAND(ID_PRIVILEGE_REFRESH, &CPrivilegeInfoDlg::OnPrivilegeRefresh)
	ON_COMMAND(ID_PRIVILEGE_ENABLE, &CPrivilegeInfoDlg::OnPrivilegeEnable)
	ON_COMMAND(ID_PRIVILEGE_DISABLE, &CPrivilegeInfoDlg::OnPrivilegeDisable)
	ON_COMMAND(ID_PRIVILEGE_TEXT, &CPrivilegeInfoDlg::OnPrivilegeText)
	ON_COMMAND(ID_PRIVILEGE_EXCEL, &CPrivilegeInfoDlg::OnPrivilegeExcel)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CPrivilegeInfoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_PRIVILEGE_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CPrivilegeInfoDlg 消息处理程序

void CPrivilegeInfoDlg::OnBnClickedOk()
{
}

BOOL CPrivilegeInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szPrivilege[g_enumLang], LVCFMT_LEFT, 230);
	m_list.InsertColumn(1, szPrivilegeDescription[g_enumLang], LVCFMT_LEFT, 280);
	m_list.InsertColumn(2, szPrivilegeStatus[g_enumLang], LVCFMT_LEFT, 150);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CPrivilegeInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

VOID CPrivilegeInfoDlg::GetProcessPrivileges()
{
	COMMUNICATE_PRIVILEGES cpp;
	PTOKEN_PRIVILEGES pTp = NULL;
	BOOL bRet = FALSE;
	ULONG nSize = 0, nRetSize = sizeof(TOKEN_PRIVILEGES) + 100 * sizeof(LUID_AND_ATTRIBUTES);

	m_szText.Format(szPrivilegeInfoState[g_enumLang], 0, 0);
	UpdateData(FALSE);
	m_PrivilegesVector.clear();
	m_list.DeleteAllItems();

	cpp.OpType = enumListProcessPrivileges;
	cpp.op.GetPrivilege.nPid = m_dwPid;
	cpp.op.GetPrivilege.pEprocess = m_process;

	do 
	{
		nSize = nRetSize;

		if (pTp)
		{
			free(pTp);
			pTp = NULL;
		}

		pTp = (PTOKEN_PRIVILEGES)malloc(nSize);;
		if (!pTp)
		{
			break;
		}

		bRet = m_driver.CommunicateDriver(&cpp, sizeof(COMMUNICATE_PRIVILEGES), pTp, nSize, NULL);

		nRetSize = *(PULONG)pTp;

	} while (!bRet && nRetSize > nSize);

	if (bRet)
	{
		for (ULONG i = 0; i < pTp->PrivilegeCount; i++)
		{
			m_PrivilegesVector.push_back(pTp->Privileges[i]);
		}

		SortByPrivilegeName();
		InsertPrivilegesItem();
	}

	if (pTp)
	{
		free(pTp);
		pTp = NULL;
	}
}

BOOL PrivilegesInfoSort( LUID_AND_ATTRIBUTES elem1, LUID_AND_ATTRIBUTES elem2 )
{
	return elem1.Attributes > elem2.Attributes;
}

VOID CPrivilegeInfoDlg::SortByPrivilegeName()
{
	sort(m_PrivilegesVector.begin( ), m_PrivilegesVector.end( ), PrivilegesInfoSort);
}

VOID CPrivilegeInfoDlg::InsertPrivilegesItem()
{
	ULONG nEnabled = 0, nDisabled = 0;

	for ( vector <LUID_AND_ATTRIBUTES>::iterator Iter = m_PrivilegesVector.begin( ); Iter != m_PrivilegesVector.end( ); Iter++)
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
			nEnabled++;
		}
		else if ( item.Attributes & 2 )
		{
			szFlags = L"Enabled";
			nEnabled++;
		}
		else
		{
			szFlags = L"-"; //L"Disabled";
			nDisabled++;
		}

		int n = m_list.InsertItem(m_list.GetItemCount(), szPrivilege);
		m_list.SetItemText(n, 1, szDisplayName);
		m_list.SetItemText(n, 2, szFlags);
	}

	m_szText.Format(szPrivilegeInfoState[g_enumLang], nEnabled, nDisabled);
	UpdateData(FALSE);
}

void CPrivilegeInfoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CPrivilegeInfoDlg::OnPrivilegeRefresh()
{
	GetProcessPrivileges();
}

void CPrivilegeInfoDlg::OnPrivilegeEnable()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szPrivilege = m_list.GetItemText(iIndex, 0);

		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		if (LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid))
		{
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			COMMUNICATE_PRIVILEGES cpp;
			cpp.OpType = enumAdjustTokenPrivileges;
			cpp.op.AdjustPrivilege.nPid = m_dwPid;
			cpp.op.AdjustPrivilege.pEprocess = m_process;
			cpp.op.AdjustPrivilege.pPrivilegeState = &tp;

			if (m_driver.CommunicateDriver(&cpp, sizeof(COMMUNICATE_PRIVILEGES), NULL, 0, NULL))
			{
				m_list.SetItemText(iIndex, 2, L"Enabled");
			}
		}
	}
}

void CPrivilegeInfoDlg::OnPrivilegeDisable()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szPrivilege = m_list.GetItemText(iIndex, 0);

		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		if (LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid))
		{
			tp.Privileges[0].Attributes = 0;
			COMMUNICATE_PRIVILEGES cpp;
			cpp.OpType = enumAdjustTokenPrivileges;
			cpp.op.AdjustPrivilege.nPid = m_dwPid;
			cpp.op.AdjustPrivilege.pEprocess = m_process;
			cpp.op.AdjustPrivilege.pPrivilegeState = &tp;

			if (m_driver.CommunicateDriver(&cpp, sizeof(COMMUNICATE_PRIVILEGES), NULL, 0, NULL))
			{
				m_list.SetItemText(iIndex, 2, L"-");
			}
		}
	}
}

void CPrivilegeInfoDlg::OnPrivilegeText()
{
	m_Functions.ExportListToTxt(&m_list, m_szText);
}

void CPrivilegeInfoDlg::OnPrivilegeExcel()
{
	WCHAR szPrivilege[] = {'P','r','i','v','i','l','e','g','e','\0'};
	m_Functions.ExportListToExcel(&m_list, szPrivilege, m_szText);
}
