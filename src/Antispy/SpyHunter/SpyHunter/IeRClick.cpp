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
// IeRClick.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "IeRClick.h"
#include "CommonStruct.h"
#include <Sddl.h>
#include <shlwapi.h>
#include "RegStringTool.h"

// CIeRClick 对话框

IMPLEMENT_DYNAMIC(CIeRClick, CDialog)

CIeRClick::CIeRClick(CWnd* pParent /*=NULL*/)
	: CDialog(CIeRClick::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_szCurrentKey = L"";
	m_bStartCheck = FALSE;
}

CIeRClick::~CIeRClick()
{
}

void CIeRClick::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_IE_RCLICK_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CIeRClick, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_CLICK_REFRESH, &CIeRClick::OnClickRefresh)
	ON_COMMAND(ID_CLICK_DELETE_REG, &CIeRClick::OnClickDeleteReg)
	ON_COMMAND(ID_CLICK_DELETE_REG_AND_FILE, &CIeRClick::OnClickDeleteRegAndFile)
	ON_COMMAND(ID_CLICK_JMP_TO_EXPLORER, &CIeRClick::OnClickJmpToExplorer)
	ON_COMMAND(ID_CLICK_JMP_TO_ANTISPY, &CIeRClick::OnClickJmpToAntispy)
	ON_COMMAND(ID_RCLICK_TEXT, &CIeRClick::OnRclickText)
	ON_COMMAND(ID_RCLICK_EXCEL, &CIeRClick::OnRclickExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CIeRClick::OnNMRclickList)
	ON_COMMAND(ID_CLICK_JMP_TO_REGISTRY, &CIeRClick::OnClickJmpToRegistry)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CIeRClick::OnLvnItemchangedList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CIeRClick::OnNMDblclkList)
	ON_COMMAND(ID_PLUGINE_DISABLE, &CIeRClick::OnPlugineDisable)
	ON_COMMAND(ID_PLUGINE_ENABLE, &CIeRClick::OnPlugineEnable)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CIeRClick)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_IE_RCLICK_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CIeRClick 消息处理程序

void CIeRClick::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CIeRClick::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CIeRClick::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/ | LVS_EX_CHECKBOXES);
	m_list.InsertColumn(0, szName[g_enumLang], LVCFMT_LEFT, 350);
	m_list.InsertColumn(1, szModulePath[g_enumLang], LVCFMT_LEFT, 820);

	m_szCurrentKey = m_RegCommonFunc.GetCurrentUserKeyPath();

	INIT_EASYSIZE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CIeRClick::EnumMenuExtKeys()
{
	if (!g_bLoadDriverOK || m_szCurrentKey.IsEmpty())
	{
		return;
	}

	m_bStartCheck = FALSE;

	m_infoList.clear();
	m_list.DeleteAllItems();

	CString szKeyPath = m_szCurrentKey + CString(L"\\Software\\Microsoft\\Internet Explorer\\MenuExt");
	
	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKeyPath, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szPath = szKeyPath + CString(L"\\") + ir->szName;
		
			IE_RCLICK_INFO info;
			info.status = eRclickOk;
			info.szKey = szPath;

			m_infoList.push_back(info);
		}
	}

	szKeyPath = m_szCurrentKey + CString(L"\\Software\\Microsoft\\Internet Explorer\\");
	szKeyPath += szAntiSpyDisableAutoruns;

	m_Registry.EnumKeys(szKeyPath, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szPath = szKeyPath + CString(L"\\") + ir->szName;

			IE_RCLICK_INFO info;
			info.status = eRclickDisable;
			info.szKey = szPath;

			m_infoList.push_back(info);
		}
	}

	InsertValues();

	m_bStartCheck = TRUE;
}

void CIeRClick::InsertValues()
{
	ULONG n = 0, nVercor = 0;

	for (vector<IE_RCLICK_INFO>::iterator ir = m_infoList.begin();
		ir != m_infoList.end();
		ir++
		)
	{
		CString szPath = ir->szKey;
		list<VALUE_INFO> valueList;

		m_Registry.EnumValues(szPath, valueList);
		for (list<VALUE_INFO>::iterator valueIr = valueList.begin();
			valueIr != valueList.end();
			valueIr++)
		{
			if (valueIr->nNameLen == 0 &&
				valueIr->nDataLen > 0 &&
				wcslen((PWCHAR)valueIr->pData) > 0 )
			{
				CString szFilePath = m_Function.RegParsePath((WCHAR*)valueIr->pData, valueIr->Type);
				CString szName = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
				int nItem = m_list.InsertItem(m_list.GetItemCount(), szName);
				m_list.SetItemText(nItem, 1, szFilePath);
				m_list.SetItemData(nItem, nVercor);

				if (ir->status == eRclickOk)
				{
					m_list.SetCheck(nItem);
				}

				n++;
				break;
			}
		}

		nVercor++;
	}

	m_szStatus.Format(szIeShellStatus[g_enumLang], n);
	UpdateData(FALSE);
}

void CIeRClick::OnClickRefresh()
{
	EnumMenuExtKeys();
}

void CIeRClick::OnClickDeleteReg()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	POSITION nPos = m_list.GetFirstSelectedItemPosition();
	while (nPos)
	{
		int nItem = m_list.GetNextSelectedItem(nPos);
		if (nItem != -1)
		{
			int nData = (int)m_list.GetItemData(nItem);
			CString szKey = m_infoList.at(nData).szKey;
			m_Registry.DeleteKeyAndSubKeys(szKey);
		}
	}

	OnClickRefresh();
}

void CIeRClick::OnClickDeleteRegAndFile()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	POSITION nPos = m_list.GetFirstSelectedItemPosition();
	while (nPos)
	{
		int nItem = m_list.GetNextSelectedItem(nPos);
		if (nItem != -1)
		{
			int nData = (int)m_list.GetItemData(nItem);
			CString szKey = m_infoList.at(nData).szKey;
			m_Registry.DeleteKeyAndSubKeys(szKey);

			CString szFilePath = m_list.GetItemText(nItem, 1);
			m_Function.KernelDeleteFile(szFilePath);
		}
	}

	OnClickRefresh();
}

void CIeRClick::OnClickJmpToExplorer()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Function.LocationExplorer(szPath);
	}
}

void CIeRClick::OnClickJmpToAntispy()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Function.JmpToFile(szPath);
	}
}

void CIeRClick::OnRclickText()
{
	m_Function.ExportListToTxt(&m_list, m_szStatus);
}

void CIeRClick::OnRclickExcel()
{
	WCHAR sz[] = {'I','E','S','h','e','l','l','\0'};
	m_Function.ExportListToExcel(&m_list, sz, m_szStatus);
}

void CIeRClick::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_RCLICK_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_RCLICK_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_CLICK_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_DISABLE, szDisable[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PLUGINE_ENABLE, szEnable[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CLICK_DELETE_REG, szRclickDeleteReg[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CLICK_DELETE_REG_AND_FILE, szIEPluginDeleteFileAndReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CLICK_JMP_TO_REGISTRY, szJmpToReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CLICK_JMP_TO_EXPLORER, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CLICK_JMP_TO_ANTISPY, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	menu.SetDefaultItem(ID_CLICK_JMP_TO_REGISTRY);

	// 设置菜单图标
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_CLICK_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_CLICK_JMP_TO_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_CLICK_DELETE_REG, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_CLICK_DELETE_REG_AND_FILE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_CLICK_JMP_TO_REGISTRY, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(ID_CLICK_JMP_TO_ANTISPY, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(13, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 14; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt > 1)
		{
			menu.EnableMenuItem(ID_CLICK_JMP_TO_ANTISPY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_CLICK_JMP_TO_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_CLICK_JMP_TO_REGISTRY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (nCnt == 0)
		{
			for (int i = 2; i < 13; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (nCnt == 1)
		{
			int nItem = m_Function.GetSelectItem(&m_list);
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

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CIeRClick::OnClickJmpToRegistry()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		CString szPath = m_infoList.at(nData).szKey;
		m_Function.JmpToMyRegistry(szPath, NULL);
	}
}

// 复选框是否取消了
void CIeRClick::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
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

		// ....
		int nData = (int)m_list.GetItemData(pNMListView->iItem);
		DisableOrEnableAutoruns(&m_infoList.at(nData), bChecked);
	}

	*pResult = 0;
}

void CIeRClick::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnClickJmpToRegistry();
	*pResult = 0;
}

BOOL CIeRClick::EnableOrDisableKey(CString szKeyRoot, CString szDisableKey, BOOL bEnable)
{
	BOOL bRet = FALSE;

	if (szKeyRoot.IsEmpty() || szDisableKey.IsEmpty())
	{
		return bRet;
	}

	CString szKeyName = szKeyRoot.Right(szKeyRoot.GetLength() - szKeyRoot.ReverseFind('\\') - 1);
	HANDLE hKey = m_Registry.CreateKey(szDisableKey);
	if (hKey)
	{
		CloseHandle(hKey);
		hKey = NULL;

		szDisableKey += CString(L"\\") + szKeyName;
		hKey = m_Registry.CreateKey(szDisableKey);
		if (hKey)
		{
			CloseHandle(hKey);
			hKey = NULL;

			m_Registry.CopyRegTree(szDisableKey, szKeyRoot);
			m_Registry.DeleteKeyAndSubKeys(szKeyRoot);
		}
	}

	return bRet;
}


BOOL CIeRClick::DisableOrEnableAutoruns(PIE_RCLICK_INFO pInfo, BOOL bEnable)
{
	BOOL bRet = FALSE;
	if (!pInfo)
	{
		return bRet;
	}

	CString szKey = pInfo->szKey;
	CString szDisableKey;
	CString szName = szKey.Right(szKey.GetLength() - szKey.ReverseFind('\\') - 1);

	if (bEnable)
	{
		pInfo->status = eRclickOk;
		szDisableKey = m_szCurrentKey + CString(L"\\Software\\Microsoft\\Internet Explorer\\MenuExt");
	}
	else
	{
		pInfo->status = eRclickDisable;
		szDisableKey = m_szCurrentKey + CString(L"\\Software\\Microsoft\\Internet Explorer\\") + szAntiSpyDisableAutoruns;
	}

	EnableOrDisableKey(szKey, szDisableKey, bEnable);
	pInfo->szKey = szDisableKey + CString(L"\\") + szName;

	// 如果是恢复项，那么最后判断AntiSpyDisableAutoruns键是否为空了，如果为空就删除之
	if (bEnable)
	{
		CString szDisableKey = m_szCurrentKey + CString(L"\\Software\\Microsoft\\Internet Explorer\\") + szAntiSpyDisableAutoruns;
		
		list<KEY_INFO> keyList;
		m_Registry.EnumKeys(szDisableKey, keyList);

		if (keyList.size() != 0)
		{
			return bRet;
		}

		m_Registry.DeleteKeyAndSubKeys(szDisableKey);
	}

	return bRet;
}

void CIeRClick::OnPlugineDisable()
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

void CIeRClick::OnPlugineEnable()
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