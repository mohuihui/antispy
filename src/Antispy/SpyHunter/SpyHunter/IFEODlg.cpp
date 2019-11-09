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
// IFEODlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "IFEODlg.h"


// CIFEODlg dialog

IMPLEMENT_DYNAMIC(CIFEODlg, CDialog)

CIFEODlg::CIFEODlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIFEODlg::IDD, pParent)
	, m_szIFEOStatus(_T(""))
{

}

CIFEODlg::~CIFEODlg()
{
}

void CIFEODlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_IFEO_STATUS, m_szIFEOStatus);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CIFEODlg, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_IFEO_REFRESH, &CIFEODlg::OnIfeoRefresh)
	ON_COMMAND(ID_IFEO_DELETE, &CIFEODlg::OnIfeoDelete)
	ON_COMMAND(ID_IFEO_JMP_TO_REG, &CIFEODlg::OnIfeoJmpToReg)
	ON_COMMAND(ID_IFEO_SHUXING, &CIFEODlg::OnIfeoShuxing)
	ON_COMMAND(ID_IFEO_SIGN_VERIFY, &CIFEODlg::OnIfeoSignVerify)
	ON_COMMAND(ID_IFEO_JMP_TO_EXPLORER, &CIFEODlg::OnIfeoJmpToExplorer)
	ON_COMMAND(ID_IFEO_JMP_TO_ANTISPY, &CIFEODlg::OnIfeoJmpToAntispy)
	ON_COMMAND(ID_IEFO_TEXT, &CIFEODlg::OnIefoText)
	ON_COMMAND(ID_IEFO_EXCEL, &CIFEODlg::OnIefoExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CIFEODlg::OnNMRclickList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CIFEODlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_IFEO_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CIFEODlg message handlers

void CIFEODlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CIFEODlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CIFEODlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0, szImageHijackName[g_enumLang], LVCFMT_LEFT, 300);
	m_list.InsertColumn(1, szHijackProcessPath[g_enumLang], LVCFMT_LEFT, 580);
	m_list.InsertColumn(2, szFileCorporation[g_enumLang], LVCFMT_LEFT, 280);	

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CIFEODlg::ListIFEO()
{
	m_list.DeleteAllItems();

	WCHAR szImageKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s',' ','N','T','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','I','m','a','g','e',' ','F','i','l','e',' ','E','x','e','c','u','t','i','o','n',' ','O','p','t','i','o','n','s','\0'};
	WCHAR szExe[] = {'.','e','x','e','\0'};
	WCHAR szDebugger[] = {'D','e','b','u','g','g','e','r','\0'};
	DWORD dwCnt = 0;

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szImageKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szName = ir->szName;
			szName.MakeLower();

			if (-1 != szName.Find(szExe))
			{
				CString szKey = CString(szImageKey) + L"\\" + szName;
				list<VALUE_INFO> valueList;
				m_Registry.EnumValues(szKey, valueList);

				for (list<VALUE_INFO>::iterator itor = valueList.begin();
					itor != valueList.end();
					itor++)
				{
					if (!itor->szName.IsEmpty() && 
						itor->nNameLen > 0 && 
						!itor->szName.CompareNoCase(szDebugger))
					{		
						int nItem = m_list.GetItemCount();
						m_list.InsertItem(nItem, ir->szName);

						CString szDebug = (WCHAR*)itor->pData;
						m_list.SetItemText(nItem, 1, szDebug);

						m_list.SetItemText(nItem, 2, m_Functions.GetFileCompanyName(szDebug));

						dwCnt++;
						break;
					}
				}

				m_Registry.FreeValueList(valueList);
			}
		}
	}

	m_szIFEOStatus.Format(szImageHijacks[g_enumLang], dwCnt);
	UpdateData(FALSE);
}

void CIFEODlg::OnIfeoRefresh()
{
	ListIFEO();
}

void CIFEODlg::OnIfeoDelete()
{
	WCHAR szImageKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s',' ','N','T','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','I','m','a','g','e',' ','F','i','l','e',' ','E','x','e','c','u','t','i','o','n',' ','O','p','t','i','o','n','s','\\','\0'};
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szName = m_list.GetItemText(nItem, 0);
		if (!szName.IsEmpty())
		{
			CString szKey = szImageKey + szName;
			m_Registry.DeleteKeyAndSubKeys(szKey);
		}
	}

	OnIfeoRefresh();
}

void CIFEODlg::OnIfeoJmpToReg()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != - 1)
	{
		WCHAR szImageKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s',' ','N','T','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','I','m','a','g','e',' ','F','i','l','e',' ','E','x','e','c','u','t','i','o','n',' ','O','p','t','i','o','n','s','\\','\0'};
		CString szKey = szImageKey + m_list.GetItemText(nItem, 0);
		m_Functions.JmpToMyRegistry(szKey, NULL);
	}
}

void CIFEODlg::OnIfeoShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != - 1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CIFEODlg::OnIfeoSignVerify()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != - 1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Functions.SignVerify(szPath);
	}
}

void CIFEODlg::OnIfeoJmpToExplorer()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != - 1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Functions.LocationExplorer(szPath);
	}
}

void CIFEODlg::OnIfeoJmpToAntispy()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != - 1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Functions.JmpToFile(szPath);
	}
}

void CIFEODlg::OnIefoText()
{
	m_Functions.ExportListToTxt(&m_list, m_szIFEOStatus);
}

void CIFEODlg::OnIefoExcel()
{
	WCHAR szTitle[] = {'I','F','E','O','\0'};
	m_Functions.ExportListToExcel(&m_list, szTitle, m_szIFEOStatus);
}

void CIFEODlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_IEFO_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_IEFO_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_IFEO_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IFEO_DELETE, szDelete[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IFEO_JMP_TO_REG, szJmpToReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IFEO_SHUXING, szHijackFileProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IFEO_SIGN_VERIFY, szHijackFileSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IFEO_JMP_TO_EXPLORER, szHijackFileInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IFEO_JMP_TO_ANTISPY, szHijackFileInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	// ÉèÖÃ²Ëµ¥Í¼±ê
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_IFEO_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_IFEO_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_IFEO_JMP_TO_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(ID_IFEO_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_IFEO_SIGN_VERIFY, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_IFEO_JMP_TO_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_IFEO_JMP_TO_ANTISPY, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(12, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 13; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 12; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			menu.EnableMenuItem(ID_IFEO_JMP_TO_REG, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IFEO_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IFEO_SIGN_VERIFY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IFEO_JMP_TO_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IFEO_JMP_TO_ANTISPY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}
