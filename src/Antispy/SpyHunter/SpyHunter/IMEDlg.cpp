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
// IMEDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "IMEDlg.h"


// CIMEDlg dialog

IMPLEMENT_DYNAMIC(CIMEDlg, CDialog)

CIMEDlg::CIMEDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIMEDlg::IDD, pParent)
	, m_szStatus(_T(""))
{

}

CIMEDlg::~CIMEDlg()
{
}

void CIMEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_IME_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CIMEDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CIMEDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CIMEDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_IME_REFRESH, &CIMEDlg::OnImeRefresh)
	ON_COMMAND(ID_IME_DELETE, &CIMEDlg::OnImeDelete)
	ON_COMMAND(ID_IME_SHUXING_IME, &CIMEDlg::OnImeShuxingIme)
	ON_COMMAND(ID_IME_VERIFY_IME, &CIMEDlg::OnImeVerifyIme)
	ON_COMMAND(ID_IME_JMP_TO_IME, &CIMEDlg::OnImeJmpToIme)
	ON_COMMAND(ID_IME_ANTISPY_IME, &CIMEDlg::OnImeAntispyIme)
	ON_COMMAND(ID_IME_SHUXING_LAYOUT, &CIMEDlg::OnImeShuxingLayout)
	ON_COMMAND(ID_IME_VERIFY_LAYOUT, &CIMEDlg::OnImeVerifyLayout)
	ON_COMMAND(ID_IME_JMP_TO_LAYOUT, &CIMEDlg::OnImeJmpToLayout)
	ON_COMMAND(ID_IME_ANTISPY_LAYOUT, &CIMEDlg::OnImeAntispyLayout)
	ON_COMMAND(ID_IME_TEXT, &CIMEDlg::OnImeText)
	ON_COMMAND(ID_IME_EXCEL, &CIMEDlg::OnImeExcel)
	ON_COMMAND(ID_IME_JMP_TO_REG, &CIMEDlg::OnImeJmpToReg)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CIMEDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_IME_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CIMEDlg message handlers

void CIMEDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CIMEDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CIMEDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0, szName[g_enumLang], LVCFMT_LEFT, 230);
	m_list.InsertColumn(1, szIMEPath[g_enumLang], LVCFMT_LEFT, 320);
	m_list.InsertColumn(2, szIMEFileCorporation[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(3, szLayoutPath[g_enumLang], LVCFMT_LEFT, 320);
	m_list.InsertColumn(4, szLayoutFileCompany[g_enumLang], LVCFMT_LEFT, 150);

	m_szCurrentUserKey = m_RegCommon.GetCurrentUserKeyPath();

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CIMEDlg::GetImeInfo(CString szKey, 
						 CString& szLayoutText, 
						 CString& szLayoutFile, 
						 CString& szImeFile)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	WCHAR strLayoutText[] = {'L','a','y','o','u','t',' ','T','e','x','t','\0'};
	WCHAR strLayoutFile[] = {'L','a','y','o','u','t',' ','F','i','l','e','\0'};
	WCHAR strImeFile[] = {'I','m','e',' ','F','i','l','e','\0'};
	WCHAR strKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','K','e','y','b','o','a','r','d',' ','L','a','y','o','u','t','s','\\','\0'};
	CString szKeyPath = strKey + szKey;
	
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKeyPath, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (!ir->szName.IsEmpty() && ir->nNameLen > 0)
		{		
			if (!ir->szName.CompareNoCase(strLayoutText))
			{
				szLayoutText = (WCHAR*)ir->pData;
			}
			else if (!ir->szName.CompareNoCase(strLayoutFile))
			{
				szLayoutFile = (WCHAR*)ir->pData;
			}
			else if (!ir->szName.CompareNoCase(strImeFile))
			{
				szImeFile = (WCHAR*)ir->pData;
			}
		}

		if (!szLayoutText.IsEmpty() &&
			!szLayoutFile.IsEmpty() &&
			!szImeFile.IsEmpty())
		{
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
}

void CIMEDlg::ListIme()
{
	m_list.DeleteAllItems();
	m_vectorIME.clear();

	if (m_szCurrentUserKey.IsEmpty())
	{
		return;
	}

	WCHAR szKey[] = {'\\','K','e','y','b','o','a','r','d',' ','L','a','y','o','u','t','\\','P','r','e','l','o','a','d','\0'};
	CString strKey = m_szCurrentUserKey + szKey;
	WCHAR szSystem32[MAX_PATH] = {0};
	GetSystemDirectory(szSystem32, MAX_PATH - 1);

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(strKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (!ir->szName.IsEmpty() && ir->nNameLen > 0 && ir->nDataLen > 0)
		{		
			IME_INFO info;
			info.szId = (WCHAR*)ir->pData;
			GetImeInfo(info.szId, info.szName, info.szLayoutFilePath, info.szImePath);
			if (!info.szName.IsEmpty())
			{
				if (!info.szImePath.IsEmpty())
				{
					info.szImePath = szSystem32 + CString(L"\\") + info.szImePath;
				}
				
				if (!info.szLayoutFilePath.IsEmpty())
				{
					info.szLayoutFilePath = szSystem32 + CString(L"\\") + info.szLayoutFilePath;
				}
				
				m_vectorIME.push_back(info);
			}
		}
	}

	m_Registry.FreeValueList(valueList);
	
	if (m_vectorIME.size() > 0)
	{
		AddItems();
	}

	m_szStatus.Format(szImeStatus[g_enumLang], m_vectorIME.size());
	UpdateData(FALSE);
}

void CIMEDlg::AddItems()
{
	DWORD dwCnt = 0;

	for (vector<IME_INFO>::iterator ir = m_vectorIME.begin();
		ir != m_vectorIME.end();
		ir++)
	{
		int nItem = m_list.GetItemCount();
		m_list.InsertItem(nItem, ir->szName);
		
		if (!ir->szImePath.IsEmpty())
		{
			CString szIME = ir->szImePath;			
			m_list.SetItemText(nItem, 1, szIME);
			m_list.SetItemText(nItem, 2, m_Functions.GetFileCompanyName(szIME));
		}
		
		if (!ir->szLayoutFilePath.IsEmpty())
		{
			CString szLayoutFile = ir->szLayoutFilePath;			
			m_list.SetItemText(nItem, 3, szLayoutFile);
			m_list.SetItemText(nItem, 4, m_Functions.GetFileCompanyName(szLayoutFile));
		}
		
		m_list.SetItemData(nItem, dwCnt);
		dwCnt++;
	}
}

void CIMEDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_IME_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_IME_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_IME_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IME_DELETE, szDelete[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IME_JMP_TO_REG, szJmpToReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IME_SHUXING_IME, szIMEFileProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IME_VERIFY_IME, szVerifyIMEFileSignature[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IME_JMP_TO_IME, szFileIMEFileInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IME_ANTISPY_IME, szFileIMEFileInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IME_SHUXING_LAYOUT, szLayoutFileProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IME_VERIFY_LAYOUT, szVerifyLayoutFileSignature[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IME_JMP_TO_LAYOUT, szFileLayoutFileInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IME_ANTISPY_LAYOUT, szFileLayoutFileInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	// ÉèÖÃ²Ëµ¥Í¼±ê
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_IME_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_IME_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_IME_JMP_TO_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(ID_IME_SHUXING_LAYOUT, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_IME_SHUXING_IME, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_IME_VERIFY_IME, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_IME_VERIFY_LAYOUT, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_IME_JMP_TO_IME, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_IME_JMP_TO_LAYOUT, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_IME_ANTISPY_IME, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_IME_ANTISPY_LAYOUT, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(16, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 17; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 16; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			for (int i = 2; i < 16; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
			
			menu.EnableMenuItem(ID_IME_DELETE, MF_BYCOMMAND | MF_ENABLED);
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				DWORD dwData = (DWORD)m_list.GetItemData(nItem);
				IME_INFO info = m_vectorIME.at(dwData);
				if (info.szImePath.IsEmpty())
				{
					for (int i = 6; i < 10; i++)
					{
						menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
					}
				}

				if (info.szLayoutFilePath.IsEmpty())
				{
					for (int i = 11; i < 15; i++)
					{
						menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
					}
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

void CIMEDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		clrNewTextColor = g_MicrosoftItemClr;
		clrNewBkColor = RGB( 255, 255, 255 );	

		IME_INFO info;
		DWORD dwData = (DWORD)m_list.GetItemData(nItem);
		info = m_vectorIME.at(dwData);

		BOOL bIme = TRUE, bLayout = TRUE;
		if (!info.szImePath.IsEmpty())
		{
			bIme = m_Functions.IsMicrosoftAppByPath(info.szImePath);
		}

		if (!info.szLayoutFilePath.IsEmpty())
		{
			bLayout = m_Functions.IsMicrosoftAppByPath(info.szLayoutFilePath);
		}

		if (!bIme || !bLayout)
		{
			clrNewTextColor = g_NormalItemClr;
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CIMEDlg::OnImeRefresh()
{
	ListIme();
}

void CIMEDlg::DeleteIME(CString szImeId)
{
	if (szImeId.IsEmpty())
	{
		return;
	}

	WCHAR strKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','K','e','y','b','o','a','r','d',' ','L','a','y','o','u','t','s','\\','\0'};
	CString szKeyPath = strKey + szImeId;
	m_Registry.DeleteKeyAndSubKeys(szKeyPath);
}

void CIMEDlg::OnImeDelete()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			DWORD dwData = (DWORD)m_list.GetItemData(nItem);
			IME_INFO info = m_vectorIME.at(dwData);
			DeleteIME(info.szId);
		}
	}

	OnImeRefresh();
}

void CIMEDlg::OnImeShuxingIme()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 1);
		m_Functions.OnCheckAttribute(szIme);
	}
}

void CIMEDlg::OnImeVerifyIme()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 1);
		m_Functions.SignVerify(szIme);
	}
}

void CIMEDlg::OnImeJmpToIme()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 1);
		m_Functions.LocationExplorer(szIme);
	}
}

void CIMEDlg::OnImeAntispyIme()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 1);
		m_Functions.JmpToFile(szIme);
	}
}

void CIMEDlg::OnImeShuxingLayout()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 3);
		m_Functions.OnCheckAttribute(szIme);
	}
}

void CIMEDlg::OnImeVerifyLayout()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 3);
		m_Functions.SignVerify(szIme);
	}
}

void CIMEDlg::OnImeJmpToLayout()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 3);
		m_Functions.LocationExplorer(szIme);
	}
}

void CIMEDlg::OnImeAntispyLayout()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szIme =m_list.GetItemText(nItem, 3);
		m_Functions.JmpToFile(szIme);
	}
}

void CIMEDlg::OnImeText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CIMEDlg::OnImeExcel()
{
	WCHAR szTitle[] = {'I','M','E','\0'};
	m_Functions.ExportListToExcel(&m_list, szTitle, m_szStatus);
}

void CIMEDlg::OnImeJmpToReg()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		DWORD dwData = (DWORD)m_list.GetItemData(nItem);
		IME_INFO info = m_vectorIME.at(dwData);
		CString szId = info.szId;
		WCHAR strKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','K','e','y','b','o','a','r','d',' ','L','a','y','o','u','t','s','\\','\0'};
		CString szKeyPath = strKey + szId;
		m_Functions.JmpToMyRegistry(szKeyPath, NULL);
	}
}
