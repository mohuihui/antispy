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
// SpiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "SpiDlg.h"

// CSpiDlg dialog

IMPLEMENT_DYNAMIC(CSpiDlg, CDialog)

CSpiDlg::CSpiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpiDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_nWinsockProvider = 0;
}

CSpiDlg::~CSpiDlg()
{
}

void CSpiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_SPI_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CSpiDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CSpiDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CSpiDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_SPI_REFRESH, &CSpiDlg::OnSpiRefresh)
	ON_COMMAND(ID_SPI_SHUXING, &CSpiDlg::OnSpiShuxing)
	ON_COMMAND(ID_SPI_SIGN, &CSpiDlg::OnSpiSign)
	ON_COMMAND(ID_SPI_JMP_TO_EXOLORER, &CSpiDlg::OnSpiJmpToExolorer)
	ON_COMMAND(ID_SPI_JMP_TO_ANTISPY, &CSpiDlg::OnSpiJmpToAntispy)
	ON_COMMAND(ID_SPI_TEXT, &CSpiDlg::OnSpiText)
	ON_COMMAND(ID_SPI_EXCEL, &CSpiDlg::OnSpiExcel)
	ON_COMMAND(ID_SPI_FIX_LSP, &CSpiDlg::OnSpiFixLsp)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CSpiDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_SPI_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CSpiDlg message handlers

void CSpiDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CSpiDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CSpiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szName[g_enumLang], LVCFMT_LEFT, 350);
	m_list.InsertColumn(1, szModulePath[g_enumLang], LVCFMT_LEFT, 300);
	m_list.InsertColumn(2, szFileCorporation[g_enumLang], LVCFMT_LEFT, 170);
	m_list.InsertColumn(3, szFileDescriptionNew[g_enumLang], LVCFMT_LEFT, 360);

//	ReleaseOriginLSPRegHive();

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

CString CSpiDlg::GetNameSpace_Catalog5NameAndModule(CString szKey, CString &szModule)
{
	CString szName = L"";
	if (szKey.IsEmpty())
	{
		szModule = L"";
		return szName;
	}

	WCHAR szDisplayString[] = {'D','i','s','p','l','a','y','S','t','r','i','n','g','\0'};
	WCHAR LibraryPath[] = {'L','i','b','r','a','r','y','P','a','t','h','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->nDataLen > 0)
		{
			if (ir->nNameLen / sizeof(WCHAR) == wcslen(szDisplayString) &&
				!ir->szName.CompareNoCase(szDisplayString))
			{
				szName = (WCHAR*)ir->pData;
			}
			else if (ir->nNameLen / sizeof(WCHAR) == wcslen(LibraryPath) &&
				!ir->szName.CompareNoCase(LibraryPath))
			{
				szModule = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			}
		}

		if (!szName.IsEmpty() && !szModule.IsEmpty())
		{
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szName;
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\NameSpace_Catalog5\Catalog_Entries
void CSpiDlg::EnumNameSpace_Catalog5()
{
	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\NameSpace_Catalog5\\Catalog_Entries";

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szKeyPath = szKey + CString(L"\\") + ir->szName;
			CString szPath;
			CString szName = GetNameSpace_Catalog5NameAndModule(szKeyPath, szPath);

			if (!szPath.IsEmpty())
			{
				AddSpiInfo(szName, szPath);
			}
		}
	}
}

CString CSpiDlg::MByteToWChar(IN LPCSTR lpcszStr)
{
	CString szRet = L"";

	if (NULL == lpcszStr)
	{
		return szRet;
	}

	DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);
	WCHAR* lpwszStr = (WCHAR *)new WCHAR[dwNum];
	if (lpwszStr == NULL)
	{
		return szRet;
	}

	memset(lpwszStr, 0, dwNum * sizeof(WCHAR));

	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwNum);  
	lpwszStr[dwNum - 1] = _T('\0');

	szRet = lpwszStr;

	delete []lpwszStr;
	lpwszStr = NULL;

	return szRet;
}

CString CSpiDlg::GetProtocolCatalog9NameAndModule(CString szKey, CString &szModule)
{
	CString szName = L"";

	if (szKey.IsEmpty())
	{
		szModule = L"";
		return szName;
	}

	WCHAR szPackedCatalogItem[] = {'P','a','c','k','e','d','C','a','t','a','l','o','g','I','t','e','m','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && 
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == wcslen(szPackedCatalogItem) &&
			!ir->szName.CompareNoCase(szPackedCatalogItem))
		{
			szModule = MByteToWChar((CHAR*)ir->pData);
			if (!szModule.IsEmpty())
			{
				szName = (WCHAR*)(((PBYTE)ir->pData) + 0x178);
			}

			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szName;
}

void CSpiDlg::AddSpiInfo(CString szName, CString szPath)
{
	SPI_INFO info;

	info.szName = szName;
	info.szPath = szPath;

	if (szName.IsEmpty() && !szPath.IsEmpty())
	{
		info.szName = info.szPath.Right(info.szPath.GetLength() - info.szPath.ReverseFind('\\') - 1);
	}

	m_SpiInfoVector.push_back(info);
	m_nWinsockProvider++;
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries
void CSpiDlg::EnumProtocol_Catalog9()
{
	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries";

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen)
		{
			CString szKeyPath = szKey + CString(L"\\") + ir->szName;
			CString szPath = L""; 
			CString szName = GetProtocolCatalog9NameAndModule(szKeyPath, szPath);

			if (!szPath.IsEmpty())
			{
				AddSpiInfo(szName, m_Function.RegParsePath(szPath.GetBuffer(), REG_SZ));
				szPath.ReleaseBuffer();
			}
		}
	}
}

void CSpiDlg::EnumWinsockProviders()					
{
	if (!g_bLoadDriverOK)
	{
		return;
	}

	m_SpiInfoVector.clear();
	m_nWinsockProvider = 0;
	m_list.DeleteAllItems();

	EnumProtocol_Catalog9();
	EnumNameSpace_Catalog5();

	InsertSpiInfo();
}

void CSpiDlg::InsertSpiInfo()
{
	for (vector<SPI_INFO>::iterator ir = m_SpiInfoVector.begin();
		ir != m_SpiInfoVector.end();
		ir++)
	{
		int nItem = m_list.GetItemCount();
		m_list.InsertItem(nItem, ir->szName);
		m_list.SetItemText(nItem, 1, ir->szPath);
		m_list.SetItemText(nItem, 2, m_Function.GetFileCompanyName(ir->szPath));
		m_list.SetItemText(nItem, 3, m_Function.GetFileDescription(ir->szPath));

		m_list.SetItemData(nItem, nItem);
	}
	
	WCHAR szSpi[] = {'L','S','P',':',' ','%','d','\0'};
	m_szStatus.Format(szSpi, m_nWinsockProvider);
	UpdateData(FALSE);
}

void CSpiDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_SPI_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_SPI_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SPI_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SPI_FIX_LSP, szFixLsp[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SPI_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SPI_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SPI_JMP_TO_EXOLORER, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SPI_JMP_TO_ANTISPY, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	// ÉèÖÃ²Ëµ¥Í¼±ê
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_SPI_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_SPI_JMP_TO_EXOLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_SPI_JMP_TO_ANTISPY, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_SPI_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_SPI_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(10, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 11; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{

		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt > 1 || nCnt == 0)
		{
			for (int i = 2; i < 10; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
	}

	menu.EnableMenuItem(ID_SPI_FIX_LSP, MF_BYCOMMAND | MF_ENABLED);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CSpiDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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
		CString strCompany, strPath;
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		clrNewTextColor = g_MicrosoftItemClr;//RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );	
		
		int nData = (int)m_list.GetItemData(nItem);
		CString szPath = m_SpiInfoVector.at(nData).szPath;
		if (!m_Function.IsMicrosoftAppByPath(szPath))
		{
			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CSpiDlg::OnSpiRefresh()
{
	EnumWinsockProviders();
}

CString CSpiDlg::GetCurrentItemPath()
{
	CString szPath = L"";

	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		szPath = m_SpiInfoVector.at(nData).szPath;
	}

	return szPath;
}

void CSpiDlg::OnSpiShuxing()
{
	CString szPath = GetCurrentItemPath();
	m_Function.OnCheckAttribute(szPath);
}

void CSpiDlg::OnSpiSign()
{
	CString szPath = GetCurrentItemPath();
	m_Function.SignVerify(szPath);
}

void CSpiDlg::OnSpiJmpToExolorer()
{
	CString szPath = GetCurrentItemPath();
	m_Function.LocationExplorer(szPath);
}

void CSpiDlg::OnSpiJmpToAntispy()
{
	CString szPath = GetCurrentItemPath();
	m_Function.JmpToFile(szPath);
}

void CSpiDlg::OnSpiText()
{
	m_Function.ExportListToTxt(&m_list, m_szStatus);
}

void CSpiDlg::OnSpiExcel()
{
	WCHAR szSPI[] = {'L','S','P','\0'};
	m_Function.ExportListToExcel(&m_list, szSPI, m_szStatus);
}

CString CSpiDlg::ReleaseOriginLSPRegHive()
{
	TCHAR szKernel[] = {'R','E','S','\0'};
	ULONG nResId = 0;
	switch (g_WinVersion)
	{
	case enumWINDOWS_2K:
		nResId = IDR_RES_LSP_W2K;
		break;

	case enumWINDOWS_XP:
		nResId = IDR_RES_LSP_XP;
		break;

	case enumWINDOWS_2K3:
	case enumWINDOWS_2K3_SP1_SP2:
		nResId = IDR_RES_LSP_W2K3;
		break;

	case enumWINDOWS_VISTA_SP1_SP2:
		nResId = IDR_RES_LSP_W2K8;
		break;

	case enumWINDOWS_VISTA:
	case enumWINDOWS_7:
		nResId = IDR_RES_LSP_WIN7;
		break;

	case enumWINDOWS_8:
		nResId = IDR_RES_LSP_WIN8;
		break;
	}

	return m_Function.ReleaseResource(nResId, szKernel);
}

void CSpiDlg::OnSpiFixLsp()
{
	CString szRegHive = ReleaseOriginLSPRegHive();
	if (szRegHive.IsEmpty())
	{
		return;
	}
	
	m_Function.EnablePrivilege(SE_BACKUP_NAME, TRUE);
	m_Function.EnablePrivilege(SE_RESTORE_NAME, TRUE);
	
	HKEY hKey = NULL;
// 	LONG nRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
// 		L"SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9",
// 		0, 
// 		KEY_ALL_ACCESS,
// 		&hKey);
	
	WCHAR szCatalog[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','S','e','r','v','i','c','e','s','\\','W','i','n','S','o','c','k','2','\\','P','a','r','a','m','e','t','e','r','s','\\','P','r','o','t','o','c','o','l','_','C','a','t','a','l','o','g','9','\0'};
	LONG nRet = RegCreateKey(HKEY_LOCAL_MACHINE, 
		szCatalog,//L"SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9",
		&hKey);

	BOOL bRet = FALSE;

	if (nRet == ERROR_SUCCESS)
	{
		nRet = RegRestoreKey(hKey, szRegHive, REG_FORCE_RESTORE);
		if (nRet == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}

		RegCloseKey(hKey);
	}

	m_Function.EnablePrivilege(SE_BACKUP_NAME, FALSE);
	m_Function.EnablePrivilege(SE_RESTORE_NAME, FALSE);

	DeleteFile(szRegHive);

	if (bRet)
	{
		MessageBox(szRepairLSPOK[g_enumLang], szToolName, MB_OK);
		OnSpiRefresh();
	}
	else
	{
		MessageBox(szRepairLSPFailed[g_enumLang], szToolName, MB_OK);
	}
}
