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
// FileAssociationDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FileAssociationDlg.h"

#define FILE_ASSOC_SUSPICIOUS 1

// CFileAssociationDlg 对话框

IMPLEMENT_DYNAMIC(CFileAssociationDlg, CDialog)

CFileAssociationDlg::CFileAssociationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileAssociationDlg::IDD, pParent)
{
	m_szCurrentUserKey = L"";
	m_nCnt = 0;
}

CFileAssociationDlg::~CFileAssociationDlg()
{
}

void CFileAssociationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CFileAssociationDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CFileAssociationDlg::OnNMCustomdrawList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CFileAssociationDlg::OnNMRclickList)
	ON_COMMAND(ID_FILE_ASSOC_REFRESH, &CFileAssociationDlg::OnFileAssocRefresh)
	ON_COMMAND(ID_FILE_ASSOC_FIX, &CFileAssociationDlg::OnFileAssocFix)
	ON_COMMAND(ID_FILE_ASSOC_FIX_ALL, &CFileAssociationDlg::OnFileAssocFixAll)
	ON_COMMAND(ID_FILE_ASSOC_LOCATION_REG, &CFileAssociationDlg::OnFileAssocLocationReg)
	ON_COMMAND(ID_FILE_ASSOC_EXPORT_TEXT, &CFileAssociationDlg::OnFileAssocExportText)
	ON_COMMAND(ID_FILE_ASSOC_EXPORT_EXCEL, &CFileAssociationDlg::OnFileAssocExportExcel)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CFileAssociationDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CFileAssociationDlg 消息处理程序

void CFileAssociationDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CFileAssociationDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CFileAssociationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0, szName[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(1, szPath[g_enumLang], LVCFMT_LEFT, 380);
	m_list.InsertColumn(2, szRegPath[g_enumLang], LVCFMT_LEFT, 600);
	
	m_szCurrentUserKey = m_RegCommon.GetCurrentUserKeyPath();
	InitClassRoot();
	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CFileAssociationDlg::InitClassRoot()
{
	FILE_ASSOCIATION info;
	WCHAR szWindows[MAX_PATH] = {0};
	GetWindowsDirectory(szWindows, MAX_PATH - 1);

	WCHAR szbat1[] = {'.','b','a','t','\0'};
	WCHAR szbat2[] = {'b','a','t','f','i','l','e','\0'};
	WCHAR szbat3[] = {'\"','%','1','\"',' ','%','*','\0'};
	info.szKey = szbat1;
	info.szValue = szbat2;
	info.szCmdOpenValue = szbat3;
	m_FileAssocList.push_back(info);

	WCHAR szcmd1[] = {'.','c','m','d','\0'};
	WCHAR szcmd2[] = {'c','m','d','f','i','l','e','\0'};
	WCHAR szcmd3[] = {'\"','%','1','\"',' ','%','*','\0'};
	info.szKey = szcmd1;
	info.szValue = szcmd2;
	info.szCmdOpenValue = szcmd3;
	m_FileAssocList.push_back(info);

	WCHAR szcom1[] = {'.','c','o','m','\0'};
	WCHAR szcom2[] = {'c','o','m','f','i','l','e','\0'};
	WCHAR szcom3[] = {'\"','%','1','\"',' ','%','*','\0'};
	info.szKey = szcom1;
	info.szValue = szcom2;
	info.szCmdOpenValue = szcom3;
	m_FileAssocList.push_back(info);

	WCHAR szexe1[] = {'.','e','x','e','\0'};
	WCHAR szexe2[] = {'e','x','e','f','i','l','e','\0'};
	WCHAR szexe3[] = {'\"','%','1','\"',' ','%','*','\0'};
	info.szKey = szexe1;
	info.szValue = szexe2;
	info.szCmdOpenValue = szexe3;
	m_FileAssocList.push_back(info);

	WCHAR szscr1[] = {'.','s','c','r','\0'};
	WCHAR szscr2[] = {'s','c','r','f','i','l','e','\0'};
	WCHAR szscr3[] = {'\"','%','1','\"',' ','/','S','\0'};
	info.szKey = szscr1;
	info.szValue = szscr2;
	info.szCmdOpenValue = szscr3;
	m_FileAssocList.push_back(info);

	WCHAR sztxt1[] = {'.','t','x','t','\0'};
	WCHAR sztxt2[] = {'t','x','t','f','i','l','e','\0'};
	WCHAR sztxt3[] = {'%','S','y','s','t','e','m','R','o','o','t','%','\\','S','y','s','t','e','m','3','2','\\','N','O','T','E','P','A','D','.','E','X','E',' ','%','1','\0'};
	info.szKey = sztxt1;
	info.szValue = sztxt2;
	info.szCmdOpenValue = sztxt3;
	m_FileAssocList.push_back(info);

	WCHAR szini1[] = {'.','i','n','i','\0'};
	WCHAR szini2[] = {'i','n','i','f','i','l','e','\0'};
	WCHAR szini3[] = {'%','S','y','s','t','e','m','R','o','o','t','%','\\','s','y','s','t','e','m','3','2','\\','N','O','T','E','P','A','D','.','E','X','E',' ','%','1','\0'};
	info.szKey = szini1;
	info.szValue = szini2;
	info.szCmdOpenValue = szini3;
	m_FileAssocList.push_back(info);

	WCHAR szpif1[] = {'.','p','i','f','\0'};
	WCHAR szpif2[] = {'p','i','f','f','i','l','e','\0'};
	WCHAR szpif3[] = {'\"','%','1','\"',' ','%','*','\0'};
	info.szKey = szpif1;
	info.szValue = szpif2;
	info.szCmdOpenValue = szpif3;
	m_FileAssocList.push_back(info);

	WCHAR szreg1[] = {'.','r','e','g','\0'};
	WCHAR szreg2[] = {'r','e','g','f','i','l','e','\0'};
	WCHAR szreg3[] = {'r','e','g','e','d','i','t','.','e','x','e',' ','\"','%','1','\"','\0'};
	info.szKey = szreg1;
	info.szValue = szreg2;
	info.szCmdOpenValue = szreg3;
	m_FileAssocList.push_back(info);

	WCHAR szinf1[] = {'.','i','n','f','\0'};
	WCHAR szinf2[] = {'i','n','f','f','i','l','e','\0'};
	WCHAR szinf3[] =  {'%','S','y','s','t','e','m','R','o','o','t','%','\\','s','y','s','t','e','m','3','2','\\','N','O','T','E','P','A','D','.','E','X','E',' ','%','1','\0'};
	info.szKey = szinf1;
	info.szValue = szinf2;
	info.szCmdOpenValue = szinf3;
	m_FileAssocList.push_back(info);

	WCHAR szhlp1[] = {'.','h','l','p','\0'};
	WCHAR szhlp2[] = {'h','l','p','f','i','l','e','\0'};
	WCHAR szhlp3[] = {'%','S','y','s','t','e','m','R','o','o','t','%','\\','S','y','s','t','e','m','3','2','\\','w','i','n','h','l','p','3','2','.','e','x','e',' ','%','1','\0'};
	info.szKey = szhlp1;
	info.szValue = szhlp2;
	info.szCmdOpenValue = szhlp3;
	m_FileAssocList.push_back(info);

	WCHAR szchm1[] = {'.','c','h','m','\0'};
	WCHAR szchm2[] = {'c','h','m','.','f','i','l','e','\0'};
	WCHAR szchm3[] = {'\"','%','S','y','s','t','e','m','R','o','o','t','%','\\','h','h','.','e','x','e','\"',' ','%','1','\0'};
	info.szKey = szchm1;
	info.szValue = szchm2;
	info.szCmdOpenValue = szchm3;
	m_FileAssocList.push_back(info);
	
	WCHAR szvbs1[] = {'.','v','b','s','\0'};
	WCHAR szvbs2[] = {'v','b','s','f','i','l','e','\0'};
	if (g_WinVersion == enumWINDOWS_2K ||
		g_WinVersion == enumWINDOWS_XP ||
		g_WinVersion == enumWINDOWS_2K3 ||
		g_WinVersion == enumWINDOWS_2K3_SP1_SP2)
	{
		WCHAR szvbs3[] = {'%','S','y','s','t','e','m','R','o','o','t','%','\\','S','y','s','t','e','m','3','2','\\','W','S','c','r','i','p','t','.','e','x','e',' ','\"','%','1','\"',' ','%','*','\0'};
		info.szCmdOpenValue = szvbs3;
	}
	else
	{
		WCHAR szvbs3[] = {'\"','%','S','y','s','t','e','m','R','o','o','t','%','\\','S','y','s','t','e','m','3','2','\\','W','S','c','r','i','p','t','.','e','x','e','\"',' ','\"','%','1','\"',' ','%','*','\0'};
		info.szCmdOpenValue = szvbs3;
	}

	info.szKey = szvbs1;
	info.szValue = szvbs2;
	m_FileAssocList.push_back(info);

	WCHAR szjs1[] = {'.','j','s','\0'};
	WCHAR szjs2[] = {'j','s','f','i','l','e','\0'};
	if (g_WinVersion == enumWINDOWS_2K ||
		g_WinVersion == enumWINDOWS_XP ||
		g_WinVersion == enumWINDOWS_2K3 ||
		g_WinVersion == enumWINDOWS_2K3_SP1_SP2)
	{
		WCHAR szjs3[] = {'%','S','y','s','t','e','m','R','o','o','t','%','\\','S','y','s','t','e','m','3','2','\\','W','S','c','r','i','p','t','.','e','x','e',' ','\"','%','1','\"',' ','%','*','\0'};
		info.szCmdOpenValue = szjs3;
	}
	else
	{
		WCHAR szjs3[] = {'\"','%','S','y','s','t','e','m','R','o','o','t','%','\\','S','y','s','t','e','m','3','2','\\','W','S','c','r','i','p','t','.','e','x','e','\"',' ','\"','%','1','\"',' ','%','*','\0'};
		info.szCmdOpenValue = szjs3;
	}
	
	info.szKey = szjs1;
	info.szValue = szjs2;
	m_FileAssocList.push_back(info);

	WCHAR szlnk1[] = {'.','l','n','k','\0'};
	WCHAR szlnk2[] = {'l','n','k','f','i','l','e','\0'};
	WCHAR szlnk3[] = L"";
	info.szKey = szlnk1;
	info.szValue = szlnk2;
	info.szCmdOpenValue = szlnk3;
 	m_FileAssocList.push_back(info);
}

CString CFileAssociationDlg::GetRegPath(CString szKey)
{
	CString szRet = L"";
	if (szKey.IsEmpty())
	{
		return szRet;
	}

	WCHAR szRootKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','\0'};
	WCHAR szCmdOpen[] = {'\\','s','h','e','l','l','\\','o','p','e','n','\\','c','o','m','m','a','n','d','\0'};
	CString szOpenKey = (CString)szRootKey + szKey + (CString)szCmdOpen;

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szOpenKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->szName.IsEmpty() && ir->nNameLen == 0 && ir->nDataLen > 0)
		{		
			szRet = (WCHAR*)ir->pData;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);

	return szRet;
}

void CFileAssociationDlg::ListClassRoot()
{
	WCHAR szRootKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','\0'};
	WCHAR szRoot[] = {'H','K','E','Y','_','C','L','A','S','S','E','S','_','R','O','O','T','\\','\0'};

	for (list<FILE_ASSOCIATION>::iterator itor = m_FileAssocList.begin();
		itor != m_FileAssocList.end();
		itor++)
	{
		CString szKey = (CString)szRootKey + itor->szKey, szValueData = L"", szBiaozhun = L"";
		list<VALUE_INFO> valueList;
		
		m_Registry.EnumValues(szKey, valueList);
		if (valueList.empty())
		{
			continue;
		}
		
		DWORD dwValueType = 0;

		for (list<VALUE_INFO>::iterator ir = valueList.begin();
			ir != valueList.end();
			ir++)
		{
			if (ir->szName.IsEmpty() && ir->nNameLen == 0 && ir->nDataLen > 0)
			{		
				szValueData = (WCHAR*)ir->pData;
				szBiaozhun = itor->szValue;
				dwValueType = ir->Type;
				break;
			}
		}

		m_Registry.FreeValueList(valueList);

		CString szPath = GetRegPath(szValueData);
// 		if (szPath.IsEmpty())
// 		{
// 			szPath = itor->szValue;
// 		}

		int nItem = m_list.GetItemCount();
		m_list.InsertItem(nItem, itor->szKey);
		m_list.SetItemText(nItem, 1, szPath);
		m_list.SetItemText(nItem, 2, szRoot + itor->szKey);

		if (szValueData.CompareNoCase(szBiaozhun))
		{
			m_list.SetItemData(nItem, FILE_ASSOC_SUSPICIOUS);
			m_nCnt++;
		}

		FILE_ASSOCIATION_ITEM info;
		info.szName = itor->szKey;
		info.szRegPath = szKey;
		info.szValue = L"";
		info.dwValueType = dwValueType;
		info.dwFileAssocType = eClassRoot;
		info.szBiaozhun = itor->szValue;
		m_vectorInfos.push_back(info);
	}
}

void CFileAssociationDlg::ListClassRootOpenCommand()
{
	WCHAR szRootKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','\0'};
	WCHAR szShellCommand[] = {'\\','S','h','e','l','l','\\','O','p','e','n','\\','C','o','m','m','a','n','d','\0'};
	WCHAR szRoot[] = {'H','K','E','Y','_','C','L','A','S','S','E','S','_','R','O','O','T','\\','\0'};

	for (list<FILE_ASSOCIATION>::iterator itor = m_FileAssocList.begin();
		itor != m_FileAssocList.end();
		itor++)
	{
		CString szKeyPath = szRoot + itor->szValue + CString(szShellCommand);
		CString szKey = (CString)szRootKey + itor->szValue + CString(szShellCommand), szValueData = L"", szBiaozhun = L"";
		list<VALUE_INFO> valueList;

		m_Registry.EnumValues(szKey, valueList);
		if (valueList.empty())
		{
			continue;
		}

		DWORD dwValueType = 0;
		for (list<VALUE_INFO>::iterator ir = valueList.begin();
			ir != valueList.end();
			ir++)
		{
			if (ir->szName.IsEmpty() && ir->nNameLen == 0 && ir->nDataLen > 0)
			{		
				szValueData = (WCHAR*)ir->pData;
				szBiaozhun = itor->szCmdOpenValue;
				dwValueType = ir->Type;
				break;
			}
		}

		m_Registry.FreeValueList(valueList);

		int nItem = m_list.GetItemCount();
		m_list.InsertItem(nItem, itor->szValue);
		m_list.SetItemText(nItem, 1, szValueData);
		m_list.SetItemText(nItem, 2, szKeyPath);

		if (szValueData.CompareNoCase(szBiaozhun))
		{
			m_list.SetItemData(nItem, FILE_ASSOC_SUSPICIOUS);
			m_nCnt++;
		}

		FILE_ASSOCIATION_ITEM info;
		info.szName = itor->szValue;
		info.szRegPath = szKey;
		info.szValue = L"";
		info.dwValueType = dwValueType;
		info.dwFileAssocType = eClassRootOpenCommand;
		info.szBiaozhun = itor->szCmdOpenValue;
		m_vectorInfos.push_back(info);
	}
}

void CFileAssociationDlg::ListCurrentUserProgid()
{
	if (m_szCurrentUserKey.IsEmpty())
	{
		return;
	}

	WCHAR strKeyPath[] = {'H','K','E','Y','_','C','U','R','R','E','N','T','_','U','S','E','R','\\','S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','E','x','p','l','o','r','e','r','\\','F','i','l','e','E','x','t','s','\\','%','s','\\','O','p','e','n','W','i','t','h','P','r','o','g','i','d','s','\0'};
	WCHAR strType[] = {'H','K','C','U',' ','%','s',' ','P','r','o','g','i','d','\0'};
	WCHAR szSoftware[] = {'\\','S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','E','x','p','l','o','r','e','r','\\','F','i','l','e','E','x','t','s','\\', '\0'};
	WCHAR szProid[] = {'\\','O','p','e','n','W','i','t','h','P','r','o','g','i','d','s','\0'};
	CString szType, szKeyPath;

	for (list<FILE_ASSOCIATION>::iterator itor = m_FileAssocList.begin();
		itor != m_FileAssocList.end();
		itor++)
	{
		szType.Format(strType, itor->szKey);
		szKeyPath.Format(strKeyPath, itor->szKey);

		CString szKey = (CString)m_szCurrentUserKey + szSoftware + itor->szKey + CString(szProid), szValueData = L"", szBiaozhun = L"";
		list<VALUE_INFO> valueList;

		m_Registry.EnumValues(szKey, valueList);
		if (valueList.empty())
		{
			continue;
		}

		for (list<VALUE_INFO>::iterator ir = valueList.begin();
			ir != valueList.end();
			ir++)
		{
			if (!ir->szName.IsEmpty() && ir->nNameLen > 0)
			{		
				szValueData = ir->szName;
				szBiaozhun = itor->szValue;

				CString szPath = GetRegPath(szValueData);

				int nItem = m_list.GetItemCount();
				m_list.InsertItem(nItem, szType);
				m_list.SetItemText(nItem, 1, szPath);
				m_list.SetItemText(nItem, 2, szKeyPath);

				if (szValueData.CompareNoCase(szBiaozhun))
				{
					m_list.SetItemData(nItem, FILE_ASSOC_SUSPICIOUS);
					m_nCnt++;
				}

				FILE_ASSOCIATION_ITEM info;
				info.szName = szType;
				info.szRegPath = szKey;
				info.szValue = ir->szName;
				info.dwValueType = ir->Type;
				info.dwFileAssocType = eCurrentUserProgid;
				info.szBiaozhun = itor->szCmdOpenValue;
				m_vectorInfos.push_back(info);
			}
		}

		m_Registry.FreeValueList(valueList);
	}
}

void CFileAssociationDlg::ListFileAssociation()
{
	m_list.DeleteAllItems();
	m_vectorInfos.clear();
	m_nCnt = 0;

	ListClassRoot();
	ListClassRootOpenCommand();
	ListCurrentUserProgid();
}	

void CFileAssociationDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		if (m_list.GetItemData(nItem) == FILE_ASSOC_SUSPICIOUS)
		{
			clrNewTextColor = g_HiddenOrHookItemClr;
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CFileAssociationDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_FILE_ASSOC_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_FILE_ASSOC_EXPORT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_FILE_ASSOC_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FILE_ASSOC_FIX, szRepair[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_FILE_ASSOC_FIX_ALL, szRepairAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FILE_ASSOC_LOCATION_REG, szJmpToReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	// 设置菜单图标
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_FILE_ASSOC_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_FILE_ASSOC_LOCATION_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(7, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 8; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		int nSel = m_list.GetSelectedCount();
		if (nSel == 0)
		{
			for (int i = 2; i < 7; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (nSel > 1)
		{
			menu.EnableMenuItem(ID_FILE_ASSOC_LOCATION_REG, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (nSel == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			if (m_list.GetItemData(nItem) != FILE_ASSOC_SUSPICIOUS)
			{
				menu.EnableMenuItem(ID_FILE_ASSOC_FIX, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			}
		}

		if (m_nCnt == 0)
		{
			menu.EnableMenuItem(ID_FILE_ASSOC_FIX_ALL, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CFileAssociationDlg::OnFileAssocRefresh()
{
	ListFileAssociation();
}

void CFileAssociationDlg::OnFileAssocFix()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			Fix(nItem);
		}
	}

	OnFileAssocRefresh();
}

BOOL CFileAssociationDlg::Fix(int nItem)
{
	BOOL bRet = FALSE;
	if (nItem == -1)
	{
		return bRet;
	}
	
	CString szName = m_list.GetItemText(nItem, 0);
	for (vector<FILE_ASSOCIATION_ITEM>::iterator ir =  m_vectorInfos.begin();
		ir != m_vectorInfos.end();
		ir++)
	{
		if (!szName.CompareNoCase(ir->szName))
		{
			switch (ir->dwFileAssocType)
			{
			case eClassRoot:
			case eClassRootOpenCommand:
				{
					WCHAR *szBiaozhun = m_Functions.CString2WString(ir->szBiaozhun);
					if (szBiaozhun)
					{
						bRet = m_Registry.ModifyNULLNameValue(ir->szRegPath, 
							ir->dwValueType, 
							(PVOID)szBiaozhun, 
							(ir->szBiaozhun.GetLength() + 1) * sizeof(WCHAR));

						free(szBiaozhun);
					}
				}
				break;

			case eCurrentUserProgid:
				{
					m_Registry.DeleteKeyAndSubKeys(ir->szRegPath);
					bRet = TRUE;
				}
				break;
			}

			break;
		}
	}

	return bRet;
}

void CFileAssociationDlg::OnFileAssocFixAll()
{
	int nCnt = m_list.GetItemCount();
	for (int i = 0; i < nCnt; i++)
	{
		if (m_list.GetItemData(i) != FILE_ASSOC_SUSPICIOUS)
		{
			continue;
		}

		Fix(i);
	}

	OnFileAssocRefresh();
}

void CFileAssociationDlg::OnFileAssocLocationReg()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szKey = m_list.GetItemText(nItem, 2);
		if (!szKey.IsEmpty())
		{
			m_Functions.JmpToMyRegistry(szKey, NULL);
		}
	}
}

void CFileAssociationDlg::OnFileAssocExportText()
{
	m_Functions.ExportListToTxt(&m_list, NULL);
}

void CFileAssociationDlg::OnFileAssocExportExcel()
{
	WCHAR szFileAssociation[] = {'F','i','l','e','A','s','s','o','c','i','a','t','i','o','n','\0'};
	m_Functions.ExportListToExcel(&m_list, szFileAssociation, NULL);
}