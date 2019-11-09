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
// IEPluginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "IEPluginDlg.h"
#include "CommonStruct.h"
#include <Sddl.h>
#include <shlwapi.h>
#include "RegStringTool.h"

// CIEPluginDlg dialog

IMPLEMENT_DYNAMIC(CIEPluginDlg, CDialog)

CIEPluginDlg::CIEPluginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIEPluginDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_nBHO = m_nExtensions = m_nDistributionUnits = m_nURLSearchHooks = m_nToolbar = m_nActiveX = 0;
	m_bStartCheck = FALSE;
	m_szCurrentUserKey = L"";
}

CIEPluginDlg::~CIEPluginDlg()
{
}

void CIEPluginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_IE_PLUGIN_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CIEPluginDlg, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_PLUGINE_REFRESH, &CIEPluginDlg::OnPlugineRefresh)
	ON_COMMAND(ID_PLUGINE_DELETE_REG, &CIEPluginDlg::OnPlugineDeleteReg)
	ON_COMMAND(ID_PLUGINE_DELETE_REG_AND_FILE, &CIEPluginDlg::OnPlugineDeleteRegAndFile)
	ON_COMMAND(ID_PLUGINE_JMP_TO_REG, &CIEPluginDlg::OnPlugineJmpToReg)
	ON_COMMAND(ID_PLUGINE_COPY_NAME, &CIEPluginDlg::OnPlugineCopyName)
	ON_COMMAND(ID_PLUGINE_COPY_PATH, &CIEPluginDlg::OnPlugineCopyPath)
	ON_COMMAND(ID_PLUGINE_CHECK_SIGN, &CIEPluginDlg::OnPlugineCheckSign)
	ON_COMMAND(ID_PLUGINE_SHUXING, &CIEPluginDlg::OnPlugineShuxing)
	ON_COMMAND(ID_PLUGINE_SEARCH_ONLINE, &CIEPluginDlg::OnPlugineSearchOnline)
	ON_COMMAND(ID_PLUGINE_JMP_TO_EXPLORER, &CIEPluginDlg::OnPlugineJmpToExplorer)
	ON_COMMAND(ID_PLUGINE_JMP_TO_ANTISPY, &CIEPluginDlg::OnPlugineJmpToAntispy)
	ON_COMMAND(ID_PLUGINE_TEXT, &CIEPluginDlg::OnPlugineText)
	ON_COMMAND(ID_PLUGINE_EXCEL, &CIEPluginDlg::OnPlugineExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CIEPluginDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CIEPluginDlg::OnNMCustomdrawList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CIEPluginDlg::OnLvnItemchangedList)
	ON_COMMAND(ID_PLUGINE_DISABLE, &CIEPluginDlg::OnPlugineDisable)
	ON_COMMAND(ID_PLUGINE_ENABLE, &CIEPluginDlg::OnPlugineEnable)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CIEPluginDlg::OnNMDblclkList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CIEPluginDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_IE_PLUGIN_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CIEPluginDlg message handlers

void CIEPluginDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CIEPluginDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CIEPluginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/ | LVS_EX_CHECKBOXES);
	m_list.InsertColumn(0, szType[g_enumLang], LVCFMT_LEFT, 190);
	m_list.InsertColumn(1, szName[g_enumLang], LVCFMT_LEFT, 300);
	m_list.InsertColumn(2, szModulePath[g_enumLang], LVCFMT_LEFT, 500);
	m_list.InsertColumn(3, szFileCorporation[g_enumLang], LVCFMT_LEFT, 180);

	m_szCurrentUserKey = m_RegCommonFunc.GetCurrentUserKeyPath();

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CIEPluginDlg::AddPluginInfo(IE_PLUGIN_TYPE Type,
								 CString szName,
								 CString szPath,
								 CString szKeyPath,
								 CString szValueName,
								 PLUGIN_STATUS nStatus)
{
	IE_PLUGIN_INFO info;
	info.nStatus = nStatus;
	info.Type = Type;
	info.szName = szName;
	info.szPath = szPath;

	if (info.szName.IsEmpty() && !info.szPath.IsEmpty())
	{
		info.szName = info.szPath.Right(info.szPath.GetLength() - info.szPath.ReverseFind('\\') - 1);
	}

	info.szKeyPath = szKeyPath;
	info.szValueName = szValueName;

	m_IEPluginVector.push_back(info);
}

// L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects";
void CIEPluginDlg::EnumBHOByRegPath(CString szKey, PLUGIN_STATUS nPluginStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->szName.GetAt(0) == '{')
		{
			AddPluginInfo(
				enumBHO,
				m_RegCommonFunc.GetCLSIDDescription(ir->szName),
				m_RegCommonFunc.GetCLSIDModule(ir->szName),
				szKey + CString(L"\\") + ir->szName,
				L"",
				nPluginStatus);

			m_nBHO++;
		}
	}
}

void CIEPluginDlg::EnumBHO()
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','E','x','p','l','o','r','e','r','\\','B','r','o','w','s','e','r',' ','H','e','l','p','e','r',' ','O','b','j','e','c','t','s','\0'};
	
	// 首先枚举正常的
	EnumBHOByRegPath(szKey, enumPluginOK);
	
	// 然后枚举被Disable掉的
	CString szKeyTemp = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumBHOByRegPath(szKeyTemp, enumPluginDisable);
}

CString CIEPluginDlg::GetBrowserExtensionsButtonText(CString szKey)
{
	CString szRet = L"";
	
	if (szKey.IsEmpty())
	{
		return szRet;
	}
	
	WCHAR szButtonText[] = {'B','u','t','t','o','n','T','e','x','t','\0'};
	list<VALUE_INFO> valueList;
	
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szButtonText) * sizeof(WCHAR) &&
			!ir->szName.CompareNoCase(szButtonText))
		{
			szRet = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szRet;
}

CString CIEPluginDlg::GetBrowserExtensionsModulePath(CString szKey)
{
	CString szRet;

	if (szKey.IsEmpty())
	{
		return szRet;
	}

	WCHAR szButtonText[] = {'I','c','o','n','\0'};
	list<VALUE_INFO> valueList;

	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szButtonText) * sizeof(WCHAR) &&
			!ir->szName.CompareNoCase(szButtonText))
		{
			szRet = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szRet;
}

void CIEPluginDlg::EnumBrowserExtensionsRegPath(CString szKey, PLUGIN_STATUS nPluginStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin(); ir != keyList.end(); ir++)
	{
		if (ir->nNameLen > 0 && ir->szName.GetAt(0) == '{')
		{
			CString szKeyPath = szKey + CString(L"\\") + ir->szName;

			AddPluginInfo(
				enumBrowserExtensions,
				GetBrowserExtensionsButtonText(szKeyPath),
				GetBrowserExtensionsModulePath(szKeyPath),
				szKeyPath,
				L"",
				nPluginStatus);

			m_nExtensions++;
		}
	}
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Internet Explorer\Extensions
void CIEPluginDlg::EnumBrowserExtensions()
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','I','n','t','e','r','n','e','t',' ','E','x','p','l','o','r','e','r','\\','E','x','t','e','n','s','i','o','n','s','\0'};

	// 首先枚举正常的
	EnumBrowserExtensionsRegPath(szKey, enumPluginOK);

	// 然后枚举被Disable掉的
	CString szKeyTemp = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumBrowserExtensionsRegPath(szKeyTemp, enumPluginDisable);
}

void CIEPluginDlg::EnumURLSearchHooksRegPath(CString szKey, PLUGIN_STATUS nPluginStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> m_valueList;
	m_Registry.EnumValues(szKey, m_valueList);

	for (list<VALUE_INFO>::iterator ir = m_valueList.begin(); ir != m_valueList.end(); ir++)
	{
		if (ir->nNameLen > 0 && ir->szName[0] == '{')
		{
			AddPluginInfo(
				enumURLSearchHooks,
				m_RegCommonFunc.GetCLSIDDescription(ir->szName),
				m_RegCommonFunc.GetCLSIDModule(ir->szName),
				szKey,
				ir->szName,
				nPluginStatus);

			m_nURLSearchHooks++;
		}
	}

	m_Registry.FreeValueList(m_valueList);
}

// HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\URLSearchHooks
void CIEPluginDlg::EnumURLSearchHooks()
{
	if (m_szCurrentUserKey.IsEmpty())
	{
		return;
	}

	WCHAR szKey[] = {'S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','I','n','t','e','r','n','e','t',' ','E','x','p','l','o','r','e','r','\\','U','R','L','S','e','a','r','c','h','H','o','o','k','s','\0'};
	CString szRegKey = m_szCurrentUserKey + CString(L"\\") + szKey;
	EnumURLSearchHooksRegPath(szRegKey, enumPluginOK);

	szRegKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumURLSearchHooksRegPath(szRegKey, enumPluginDisable);
}

void CIEPluginDlg::EnumDistributionUnitsRegPath(CString szKey, PLUGIN_STATUS nPluginStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->szName[0] == '{')
		{
			AddPluginInfo(enumDistributionUnits,
				m_RegCommonFunc.GetCLSIDDescription(ir->szName),
				m_RegCommonFunc.GetCLSIDModule(ir->szName),
				szKey + L"\\" + ir->szName,
				L"",
				nPluginStatus);

			m_nDistributionUnits++;
		}
	}
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Code Store Database\Distribution Units
void CIEPluginDlg::EnumDistributionUnits()
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','C','o','d','e',' ','S','t','o','r','e',' ','D','a','t','a','b','a','s','e','\\','D','i','s','t','r','i','b','u','t','i','o','n',' ','U','n','i','t','s','\0'};
	EnumDistributionUnitsRegPath(szKey, enumPluginOK);

	CString szRegKey = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumDistributionUnitsRegPath(szRegKey, enumPluginDisable);
}

void CIEPluginDlg::EnumToolbarRegPath(CString szKey, PLUGIN_STATUS nPluginStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->szName[0] == '{')
		{
			AddPluginInfo(enumToolbar,
				m_RegCommonFunc.GetCLSIDDescription(ir->szName),
				m_RegCommonFunc.GetCLSIDModule(ir->szName),
				szKey,
				ir->szName,
				nPluginStatus);

			m_nToolbar++;
		}
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Internet Explorer\Toolbar
void CIEPluginDlg::EnumToolbar()
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','I','n','t','e','r','n','e','t',' ','E','x','p','l','o','r','e','r','\\','T','o','o','l','b','a','r','\0'};
	EnumToolbarRegPath(szKey, enumPluginOK);

	CString szRegKey = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumToolbarRegPath(szRegKey, enumPluginDisable);
}

void CIEPluginDlg::EnumActiveXRegPath(CString szKey, PLUGIN_STATUS nPluginStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->szName[0] == '{')
		{
			CString szName = m_RegCommonFunc.GetCLSIDDescription(ir->szName);
			CString szPath = m_RegCommonFunc.GetCLSIDModule(ir->szName);
			if (szName.IsEmpty())
			{
				szName = ir->szName;
			}

			AddPluginInfo(
				enumActiveX, 
				szName,
				szPath,
				szKey + CString(L"\\") + ir->szName,
				L"",
				nPluginStatus);

			m_nActiveX++;
		}
	}
}

// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Ext\Stats
void CIEPluginDlg::EnumActiveX()
{
	if (m_szCurrentUserKey.IsEmpty())
	{
		return;
	}

	WCHAR szKey[] = {'S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','E','x','t','\\','S','t','a','t','s','\0'};
	CString szKeyPath = m_szCurrentUserKey + CString(L"\\") + szKey;
	EnumActiveXRegPath(szKeyPath, enumPluginOK);

	szKeyPath +=  CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumActiveXRegPath(szKeyPath, enumPluginDisable);
}

void CIEPluginDlg::EnumPlugins()
{
	if (!g_bLoadDriverOK)
	{
		return;
	}

	m_bStartCheck = FALSE;

	m_nBHO = m_nExtensions = m_nURLSearchHooks = m_nDistributionUnits = m_nToolbar = m_nActiveX = 0;
	m_list.DeleteAllItems();
	m_IEPluginVector.clear();
	
	EnumBHO();
	EnumBrowserExtensions();
	EnumURLSearchHooks();
	EnumDistributionUnits();
	EnumToolbar();
	EnumActiveX();

	InsertPlugins();

	m_bStartCheck = TRUE;
}

CString CIEPluginDlg::GetPluginTypeName(IE_PLUGIN_TYPE type)
{
	CString szRet;
	
	switch (type)
	{
	case enumBHO:
		{
			WCHAR szBHO[] = {'B','r','o','w','s','e','r',' ','H','e','l','p','e','r',' ','O','b','j','e','c','t','s','\0'};
			szRet = szBHO;
		}
		break;

	case enumBrowserExtensions:
		{
			WCHAR szExtensions[] = {'B','r','o','w','s','e','r',' ','E','x','t','e','n','s','i','o','n','s','\0'};
			szRet = szExtensions;
		}
		break;

	case enumURLSearchHooks:
		{
			WCHAR szURLSearchHooks[] = {'U','R','L',' ','S','e','a','r','c','h',' ','H','o','o','k','s','\0'};
			szRet = szURLSearchHooks;
		}
		break;

	case enumDistributionUnits:
		{
			WCHAR szDistributionUnits[] = {'D','i','s','t','r','i','b','u','t','i','o','n',' ','U','n','i','t','s','\0'};
			szRet = szDistributionUnits;
		}
		break;

	case enumToolbar:
		{
			WCHAR szToolbar[] = {'T','o','o','l','b','a','r','\0'};
			szRet = szToolbar;
		}
		break;

	case enumActiveX:
		{
			WCHAR szActiveX[] = {'A','c','t','i','v','e','X','\0'};
			szRet = szActiveX;
		}
		break;
	}

	return szRet;
}

void CIEPluginDlg::InsertPlugins()
{
	ULONG n = 0;

	for (vector<IE_PLUGIN_INFO>::iterator ir = m_IEPluginVector.begin();
		ir != m_IEPluginVector.end();
		ir++)
	{
		CString szType = GetPluginTypeName(ir->Type);
	
		int nItem = m_list.GetItemCount();
		m_list.InsertItem(nItem, szType);
		m_list.SetItemText(nItem, 1, ir->szName);
		m_list.SetItemText(nItem, 2, ir->szPath);
		m_list.SetItemText(nItem, 3, m_Function.GetFileCompanyName(ir->szPath));
		
		if (ir->nStatus == enumPluginOK)
		{
			m_list.SetCheck(nItem);
		}

		m_list.SetItemData(nItem, n);
		n++;
	}

	m_szStatus.Format(szIePluginStatus[g_enumLang], 
		m_nBHO + m_nExtensions + m_nURLSearchHooks + m_nDistributionUnits + m_nToolbar + m_nActiveX,
		m_nBHO,
		m_nExtensions,
		m_nURLSearchHooks,
		m_nDistributionUnits,
		m_nToolbar,
		m_nActiveX);

	UpdateData(FALSE);
}

void CIEPluginDlg::OnPlugineRefresh()
{
	EnumPlugins();
}

void CIEPluginDlg::OnPlugineDeleteReg()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_ICONQUESTION | MB_YESNO) != IDYES)
	{
		return;
	}

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			int nData = (int)m_list.GetItemData(nItem);;
			IE_PLUGIN_INFO info = m_IEPluginVector.at(nData);
			CString szKey = info.szKeyPath;
			CString szValue = info.szValueName;

			if (szValue.IsEmpty() && !szKey.IsEmpty())
			{
				m_Registry.DeleteKeyAndSubKeys(szKey);
			}
			else if (!szValue.IsEmpty() && !szKey.IsEmpty())		
			{
				m_Registry.DeleteValue(szKey, szValue);
			}
		}
	}
	
	OnPlugineRefresh();
}

void CIEPluginDlg::OnPlugineDeleteRegAndFile()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_ICONQUESTION | MB_YESNO) != IDYES)
	{
		return;
	}

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			int nData = (int)m_list.GetItemData(nItem);;
			IE_PLUGIN_INFO info = m_IEPluginVector.at(nData);
			CString szKey = info.szKeyPath;
			CString szValue = info.szValueName;

			if (szValue.IsEmpty() && !szKey.IsEmpty())
			{
				m_Registry.DeleteKeyAndSubKeys(szKey);
			}
			else if (!szValue.IsEmpty() && !szKey.IsEmpty())		
			{
				m_Registry.DeleteValue(szKey, szValue);
			}

			CString szFile = info.szPath;
			if (!szFile.IsEmpty() && PathFileExists(szFile))
			{
				m_Function.KernelDeleteFile(szFile);
			}
		}
	}

	OnPlugineRefresh();
}

void CIEPluginDlg::OnPlugineJmpToReg()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);;
		IE_PLUGIN_INFO info = m_IEPluginVector.at(nData);
		m_Function.JmpToMyRegistry(info.szKeyPath, info.szValueName);
	}
}

void CIEPluginDlg::OnPlugineCopyName()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szName = m_list.GetItemText(nItem, 1);
		m_Function.SetStringToClipboard(szName);
	}
}

void CIEPluginDlg::OnPlugineCopyPath()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szName = m_list.GetItemText(nItem, 2);
		m_Function.SetStringToClipboard(szName);
	}
}

void CIEPluginDlg::OnPlugineCheckSign()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szName = m_list.GetItemText(nItem, 2);
		m_Function.SignVerify(szName);
	}
}

void CIEPluginDlg::OnPlugineShuxing()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szName = m_list.GetItemText(nItem, 2);
		m_Function.OnCheckAttribute(szName);
	}
}

void CIEPluginDlg::OnPlugineSearchOnline()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szName = m_list.GetItemText(nItem, 1);
		m_Function.SearchOnline(szName);
	}
}

void CIEPluginDlg::OnPlugineJmpToExplorer()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szName = m_list.GetItemText(nItem, 2);
		m_Function.LocationExplorer(szName);
	}
}

void CIEPluginDlg::OnPlugineJmpToAntispy()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szName = m_list.GetItemText(nItem, 2);
		m_Function.JmpToFile(szName);
	}
}

void CIEPluginDlg::OnPlugineText()
{
	m_Function.ExportListToTxt(&m_list, m_szStatus);
}

void CIEPluginDlg::OnPlugineExcel()
{
	WCHAR szIEPlug[] = {'I','E','P','l','u','g','i','n','\0'};
	m_Function.ExportListToExcel(&m_list, szIEPlug, m_szStatus);
}

void CIEPluginDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_PLUGINE_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_PLUGINE_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PLUGINE_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_DISABLE, szDisable[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_ENABLE, szEnable[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_DELETE_REG, szIEPluginDeleteReg[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_DELETE_REG_AND_FILE, szIEPluginDeleteFileAndReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_JMP_TO_REG, szJmpToReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_COPY_NAME, szIEPluginCopyName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_COPY_PATH, szIEPluginCopyPath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_SHUXING, szIEPluginShuxing[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_SEARCH_ONLINE, szIEPluginSearchOnline[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_CHECK_SIGN, szIEPluginSign[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_JMP_TO_EXPLORER, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_JMP_TO_ANTISPY, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	menu.SetDefaultItem(ID_PLUGINE_JMP_TO_REG);

	// 设置菜单图标
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_PLUGINE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_PLUGINE_DELETE_REG, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_PLUGINE_DELETE_REG_AND_FILE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_PLUGINE_JMP_TO_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(ID_PLUGINE_COPY_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_PLUGINE_COPY_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_PLUGINE_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_PLUGINE_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_PLUGINE_SEARCH_ONLINE, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_PLUGINE_JMP_TO_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PLUGINE_JMP_TO_ANTISPY, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(20, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 21; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt > 1)
		{
			for (int i = 2; i < 20; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_PLUGINE_DELETE_REG, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PLUGINE_DELETE_REG_AND_FILE, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PLUGINE_DISABLE, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PLUGINE_ENABLE, MF_BYCOMMAND | MF_ENABLED);
		}
		else if (nCnt == 1)
		{
			int nItem = m_Function.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				CString szAddr = m_list.GetItemText(nItem, 2);
				if (szAddr.IsEmpty())
				{
					menu.EnableMenuItem(ID_PLUGINE_COPY_PATH, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PLUGINE_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PLUGINE_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PLUGINE_JMP_TO_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PLUGINE_JMP_TO_ANTISPY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				szAddr = m_list.GetItemText(nItem, 1);
				if (szAddr.IsEmpty())
				{
					menu.EnableMenuItem(ID_PLUGINE_COPY_NAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PLUGINE_SEARCH_ONLINE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				if (m_list.GetCheck(nItem))
				{
					menu.EnableMenuItem(ID_PLUGINE_ENABLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
				else
				{
					menu.EnableMenuItem(ID_PLUGINE_DISABLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}
		}
		else if (nCnt == 0)
		{
			for (int i = 2; i < 20; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
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

void CIEPluginDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		CString szCompane = m_list.GetItemText(nItem, 3);
		if (!m_Function.IsMicrosoftApp(szCompane))
		{
			clrNewTextColor = g_NormalItemClr;
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

BOOL CIEPluginDlg::DisableOrEnableAutoruns(PIE_PLUGIN_INFO info, BOOL bEnable)
{
	BOOL bRet = FALSE;
	CString szKeyPath;
	CString szKey = info->szKeyPath;
	CString szValue = info->szValueName;
	CString szDisableKey;

	// 该启动信息是一个键
	if (szValue.IsEmpty() && !szKey.IsEmpty())
	{
		m_Registry.EnableOrDisableKey(szKey, bEnable);
		
		CString szKeyName = szKey.Right(szKey.GetLength() - szKey.ReverseFind('\\') - 1);
		if (!bEnable)
		{
			szKeyPath = szKey.Left(szKey.ReverseFind('\\'));
			szKeyPath += CString(L"\\") + szAntiSpyDisableAutoruns + CString(L"\\") + szKeyName;
		}
		else
		{
			szKeyPath = szKey.Left(szKey.Find(szAntiSpyDisableAutoruns) - 1);
			szKeyPath += CString(L"\\") + szKeyName;
		}

		szDisableKey = szKey.Left(szKey.ReverseFind('\\'));
	}
	
	// 启动项是一个值
	else if (!szValue.IsEmpty() && !szKey.IsEmpty())
	{
		m_Registry.EnableOrDisableValue(szKey, szValue, bEnable);

		if (!bEnable)
		{
			szKeyPath = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
		}
		else
		{
			szKeyPath = szKey.Left(szKey.Find(szAntiSpyDisableAutoruns) - 1);
		}

		szDisableKey = szKey;
	}

	// 修改原始项的键路径
	info->szKeyPath = szKeyPath;

	// 修改这一项的状态
	bEnable ? info->nStatus = enumPluginOK : info->nStatus = enumPluginDisable;

	// 如果是恢复项，那么最后判断AntiSpyDisableAutoruns键是否为空了，如果为空就删除之
	if (bEnable && !szDisableKey.IsEmpty())
	{
		list<KEY_INFO> keyList;
		m_Registry.EnumKeys(szDisableKey, keyList);
		if (keyList.size() != 0)
		{
			return bRet;
		}

		list<VALUE_INFO> valueList;
		m_Registry.EnumValues(szDisableKey, valueList);
		if (valueList.size() != 0)
		{
			m_Registry.FreeValueList(valueList);
			return bRet;
		}

		m_Registry.FreeValueList(valueList);
		m_Registry.DeleteKeyAndSubKeys(szDisableKey);
	}

	return bRet;
}

void CIEPluginDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_bStartCheck)
	{
		LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		*pResult = 0;

		if (pNMListView->uOldState == 0 && pNMListView->uNewState == 0)
			return;    // No change

		// Old check box state
		BOOL bPrevState = (BOOL)(((pNMListView->uOldState & 
			LVIS_STATEIMAGEMASK)>>12)-1);  
		if (bPrevState < 0)    // On startup there's no previous state 
			bPrevState = 0; // so assign as false (unchecked)

		// New check box state
		BOOL bChecked = (BOOL)(((pNMListView->uNewState & LVIS_STATEIMAGEMASK)>>12)-1);   
		if (bChecked < 0) // On non-checkbox notifications assume false
			bChecked = 0; 

		if (bPrevState == bChecked) // No change in check box
			return;

		// Now bChecked holds the new check box state
		int nData = (int)m_list.GetItemData(pNMListView->iItem);
		DisableOrEnableAutoruns(&m_IEPluginVector.at(nData), bChecked);
	}
	
	*pResult = 0;
}

void CIEPluginDlg::OnPlugineDisable()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1 && m_list.GetCheck(nItem))
		{
			m_list.SetCheck(nItem, FALSE);
		}
	}
}

void CIEPluginDlg::OnPlugineEnable()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1 && !m_list.GetCheck(nItem))
		{
			m_list.SetCheck(nItem, TRUE);
		}
	}
}

void CIEPluginDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnPlugineJmpToReg();
	*pResult = 0;
}
