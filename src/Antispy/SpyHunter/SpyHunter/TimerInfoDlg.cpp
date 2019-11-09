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
// TimerInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "TimerInfoDlg.h"
#include <algorithm>

// CTimerInfoDlg 对话框

IMPLEMENT_DYNAMIC(CTimerInfoDlg, CDialog)

CTimerInfoDlg::CTimerInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTimerInfoDlg::IDD, pParent)
	, m_szText(_T(""))
{
	m_szImage = L"";
	m_dwEprocess = 0;
	m_dwPid = 0;
	m_bShowAllTimer = FALSE;
	m_nPreItem = 0;
}

CTimerInfoDlg::~CTimerInfoDlg()
{
}

void CTimerInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_TIMER_INFO_CNT, m_szText);
	DDX_Control(pDX, IDC_EDIT, m_edit);
}


BEGIN_MESSAGE_MAP(CTimerInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTimerInfoDlg::OnBnClickedOk)
	ON_COMMAND(ID_TIMER_REFRESH, &CTimerInfoDlg::OnTimerRefresh)
	ON_COMMAND(ID_TIMER_SHOW_ALL_TIMER, &CTimerInfoDlg::OnTimerShowAllTimer)
	ON_COMMAND(ID_TIMER_REMOVE, &CTimerInfoDlg::OnTimerRemove)
	ON_COMMAND(ID_TIMER_EDIT_TIMER, &CTimerInfoDlg::OnTimerEditTimer)
	ON_COMMAND(ID_TIMER_CHECK_SIGN, &CTimerInfoDlg::OnTimerCheckSign)
	ON_COMMAND(ID_TIMER_CHECK_ATTRIBUTE, &CTimerInfoDlg::OnTimerCheckAttribute)
	ON_COMMAND(ID_TIMER_LOCATION_EXPLORER, &CTimerInfoDlg::OnTimerLocationExplorer)
	ON_COMMAND(ID_TIMER_EXPORT_TEXT, &CTimerInfoDlg::OnTimerExportText)
	ON_COMMAND(ID_TIMER_EXPORT_EXCEL, &CTimerInfoDlg::OnTimerExportExcel)
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_TIMER_SHOW_ALL_TIMER, &CTimerInfoDlg::OnUpdateTimerShowAllTimer)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CTimerInfoDlg::OnNMRclickList)
	ON_WM_INITMENUPOPUP()
	ON_EN_KILLFOCUS(IDC_EDIT, &CTimerInfoDlg::OnEnKillfocusEdit)
	ON_COMMAND(ID_TIMER_DISASSEMBLE, &CTimerInfoDlg::OnTimerDisassemble)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CTimerInfoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_TIMER_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CTimerInfoDlg 消息处理程序

void CTimerInfoDlg::OnBnClickedOk()
{
}

void CTimerInfoDlg::OnTimerRefresh()
{
	EnumProcessTimers();
}

void CTimerInfoDlg::OnTimerShowAllTimer()
{
	if (!m_bShowAllTimer)
	{
		m_bShowAllTimer = TRUE;
	}	
	else
	{
		m_bShowAllTimer = FALSE;
	}

	EnumProcessTimers();
}

void CTimerInfoDlg::OnTimerRemove()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	while (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szTimerObject = m_list.GetItemText(iIndex, 0);
// 		for ( vector <TIMER>::iterator Iter = m_TimerVector.begin( ); Iter != m_TimerVector.end( ); Iter++ )
// 		{
// 			CString strTimerObject;
// 			strTimerObject.Format(L"0x%08X", Iter->TimerObject);
// 			if (!strTimerObject.CompareNoCase(szTimerObject))
// 			{
// 				COMMUNICATE_TIMER ct;
// 				ct.op.RemoveTimer.TimerObject = Iter->TimerObject;
// 				ct.OpType = enumRemoveTimer;
// 				m_driver.CommunicateDriver(&ct, sizeof(COMMUNICATE_TIMER), NULL, 0, NULL);
// 				break;
// 			}	
// 		}
// 
// 		m_list.DeleteItem(iIndex);
// 		pos = m_list.GetFirstSelectedItemPosition();
// 	}
}

void CTimerInfoDlg::OnTimerEditTimer()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		m_nPreItem = iIndex;
		CRect rc;
		m_list.GetSubItemRect(iIndex, 1, LVIR_LABEL, rc);
		rc.left += 3;
		rc.top += 1;
		rc.right += 3;
		rc.bottom += 4;
		CString szOutTime = m_list.GetItemText(iIndex, 1);
		m_edit.SetWindowText(szOutTime);
		m_edit.MoveWindow(&rc);
		m_edit.ShowWindow(SW_SHOW);
		m_edit.SetFocus();
		m_edit.CreateSolidCaret(1, rc.Height() - 5);
		m_edit.ShowCaret(); 
		m_edit.SetSel(-1);
	}
}

void CTimerInfoDlg::OnTimerCheckSign()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.SignVerify(szPath);
	}
}

void CTimerInfoDlg::OnTimerCheckAttribute()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 3);

		m_Functions.OnCheckAttribute(szPath);
	}
}

void CTimerInfoDlg::OnTimerLocationExplorer()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 3);

		m_Functions.LocationExplorer(szPath);
	}
}

void CTimerInfoDlg::OnTimerExportText()
{
	m_Functions.ExportListToTxt(&m_list, m_szText);
}

void CTimerInfoDlg::OnTimerExportExcel()
{
	WCHAR szTimer[] = {'T','i','m','e','r','\0'};
	m_Functions.ExportListToExcel(&m_list, szTimer, m_szText);
}

void CTimerInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CTimerInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szTimerObject[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(1, szTimeOutValue[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(2, szCallback[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(3, szTimerModulePath[g_enumLang], LVCFMT_LEFT, 250);
	m_list.InsertColumn(4, szTid[g_enumLang], LVCFMT_LEFT, 65);
	m_list.InsertColumn(5, szPid[g_enumLang], LVCFMT_LEFT, 65);
	m_list.InsertColumn(6, szProcesseName[g_enumLang], LVCFMT_LEFT, 120);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

VOID CTimerInfoDlg::EnumProcessTimers()
{
// 	ULONG nCnt = 500;
// 	PPROCESS_TIMER_INFO pTimerInfo = NULL;
// 	OPERATE_TYPE ot = enumListProcessTimer;
// 	BOOL bRet = FALSE;
// 
// 	m_list.DeleteAllItems();
// 	m_szText.Format(szTimerState[g_enumLang], L"", 0);
// 	UpdateData(FALSE);
// 
// 	m_TimerVector.clear();
// 
// 	do 
// 	{
// 		ULONG nSize = sizeof(PROCESS_TIMER_INFO) + nCnt * sizeof(TIMER);
// 
// 		if (pTimerInfo)
// 		{
// 			free(pTimerInfo);
// 			pTimerInfo = NULL;
// 		}
// 
// 		pTimerInfo = (PPROCESS_TIMER_INFO)malloc(nSize);
// 
// 		if (pTimerInfo)
// 		{
// 			memset(pTimerInfo, 0, nSize);
// 			pTimerInfo->nCnt = nCnt;
// 			bRet = m_driver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pTimerInfo, nSize, NULL);
// 		}
// 
// 		nCnt = pTimerInfo->nRetCnt + 10;
// 
// 	} while (!bRet && pTimerInfo->nRetCnt > pTimerInfo->nCnt);
// 
// 	if (pTimerInfo->nCnt >= pTimerInfo->nRetCnt)
// 	{
// 		for (ULONG i = 0; i < pTimerInfo->nRetCnt; i++)
// 		{
// 			m_TimerVector.push_back(pTimerInfo->timers[i]);
// 		}
// 	}
// 
// 	if (pTimerInfo)
// 	{
// 		free(pTimerInfo);
// 		pTimerInfo = NULL;
// 	}
// 
// 	SortByEprocess();
// 	InsertTimers();
}

VOID CTimerInfoDlg::InsertTimers()
{
// 	ULONG nTimerCnt = 0;
// 
// 	if (m_bShowAllTimer)
// 	{
// 		vector<PROCESS_INFO> vectorProcess;
// 		CListProcess ListProc;
// 		ListProc.EnumProcess(vectorProcess);
// 
// 		ULONG nPeprocess = 0;
// 		PROCESS_INFO info;
// 		BOOL bModule = FALSE;
// 
// 		for ( vector <TIMER>::iterator Iter = m_TimerVector.begin( ); Iter != m_TimerVector.end( ); Iter++ )
// 		{
// 			// 首先找到这个定时器属于哪个进程
// 			if (nPeprocess != Iter->pEprocess)
// 			{
// 				info = FindProcessItem(vectorProcess, Iter->pEprocess);
// 				nPeprocess = info.ulEprocess;
// 				bModule = FALSE;
// 			}
// 
// 			CString szProcessName, szPid;
// 			CString strPath = info.szPath;
// 			szProcessName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1); 
// 			szPid.Format(L"%d", info.ulPid);
// 
// // 			m_dllDlg.m_uPid = info.ulPid;
// // 			m_dllDlg.m_pEprocess = info.ulEprocess;
// // 			m_dllDlg.GetDllModulesInfo();
// 
// 			CString szTimerObject, szCallback, szInterval, szTid, szModePath;
// 
// 			szTimerObject.Format(L"0x%08X", Iter->TimerObject);
// 
// 			if (Iter->pCallback)
// 			{
// 				if (!bModule)
// 				{
// 					GetProcessModuleBySnap(info.ulPid);
// 					bModule = TRUE;
// 				}
// 
// 				szCallback.Format(L"0x%08X", Iter->pCallback);
// 				szModePath = GetModulePathByCallbackAddress(Iter->pCallback);
// 			}
// 			else
// 			{
// 				szCallback = L"-";
// 			}
// 
// 			szInterval.Format(L"%.2f", Iter->nInterval / 1000.0);
// 			szTid.Format(L"%d", Iter->tid);
// 
// 			int n = m_list.InsertItem(m_list.GetItemCount(), szTimerObject);
// 			m_list.SetItemText(n, 1, szInterval);
// 			m_list.SetItemText(n, 2, szCallback);
// 			m_list.SetItemText(n, 3, szModePath);
// 			m_list.SetItemText(n, 4, szTid);
// 			m_list.SetItemText(n, 5, szPid);
// 			m_list.SetItemText(n, 6, szProcessName);
// 
// 			nTimerCnt++;
// 		}
// 
// 		m_szText.Format(szTimerState[g_enumLang], szAllProcesses[g_enumLang], nTimerCnt);
// 		UpdateData(FALSE);
// 	}
// 	else
// 	{
// // 		m_dllDlg.m_uPid = m_dwPid;
// // 		m_dllDlg.m_pEprocess = m_dwEprocess;
// // 		m_dllDlg.GetDllModulesInfo();
// 
// 		BOOL bEnumModule = FALSE;
// 		for ( vector <TIMER>::iterator Iter = m_TimerVector.begin( ); Iter != m_TimerVector.end( ); Iter++ )
// 		{
// 			if (Iter->pEprocess == m_dwEprocess)
// 			{
// 				CString szTimerObject, szCallback, szInterval, szTid, szModePath, szPid;
// 
// 				szTimerObject.Format(L"0x%08X", Iter->TimerObject);
// 
// 				if (Iter->pCallback)
// 				{
// 					if (!bEnumModule)
// 					{
// 						GetProcessModuleBySnap(m_dwPid);
// 						bEnumModule = TRUE;
// 					}
// 
// 					szCallback.Format(L"0x%08X", Iter->pCallback);
// 					szModePath = GetModulePathByCallbackAddress(Iter->pCallback);
// 				}
// 				else
// 				{
// 					szCallback = L"-";
// 				}
// 
// 				szInterval.Format(L"%.2f", Iter->nInterval / 1000.0);
// 				szTid.Format(L"%d", Iter->tid);
// 				szPid.Format(L"%d", m_dwPid);
// 
// 				int n = m_list.InsertItem(m_list.GetItemCount(), szTimerObject);
// 				m_list.SetItemText(n, 1, szInterval);
// 				m_list.SetItemText(n, 2, szCallback);
// 				m_list.SetItemText(n, 3, szModePath);
// 				m_list.SetItemText(n, 4, szTid);
// 				m_list.SetItemText(n, 5, szPid);
// 				m_list.SetItemText(n, 6, m_szImage);
// 
// 				nTimerCnt++;
// 			}
// 		}
// 
// 		m_szText.Format(szTimerState[g_enumLang], m_szImage, nTimerCnt);
// 		UpdateData(FALSE);
// 	}
}

CString CTimerInfoDlg::GetModulePathByCallbackAddress(DWORD dwAddress)
{
	if (dwAddress >= 0x80000000)
	{
		if (m_CommonDriverList.size() == 0)
		{
			GetDriver();
		}

		return GetDriverPath(dwAddress);
	}
	else
	{
// 		for ( vector <MODULE_INFO>::iterator Iter = m_dllDlg.m_VadRootModuleVector.begin( ); Iter != m_dllDlg.m_VadRootModuleVector.end( ); Iter++)
// 		{	
// 			MODULE_INFO entry = *Iter;
// 			if (dwAddress >= entry.Base && dwAddress <= (entry.Base + entry.Size))
// 			{
// 				return m_Functions.TrimPath(entry.Path);
// 			}
// 		}
// 
// 		for ( vector <MODULE_INFO>::iterator Iter = m_dllDlg.m_PebModuleVector.begin( ); Iter != m_dllDlg.m_PebModuleVector.end( ); Iter++)
// 		{	
// 			MODULE_INFO entry = *Iter;
// 			if (dwAddress >= entry.Base && dwAddress <= (entry.Base + entry.Size))
// 			{
// 				return m_Functions.TrimPath(entry.Path);
// 			}
// 		}

		for ( list<TIMER_MODULE_INFO>::iterator Iter =  m_TimerModuleList.begin( ); 
			Iter != m_TimerModuleList.end( ); 
			Iter++)
		{	
			if (dwAddress >= Iter->nBase && dwAddress <= (Iter->nBase + Iter->nSize))
			{
				return Iter->szModule;
			}
		}
	}

	return NULL;
}

void CTimerInfoDlg::OnUpdateTimerShowAllTimer(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAllTimer);
}

void CTimerInfoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_TIMER_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_TIMER_EXPORT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_TIMER_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_TIMER_SHOW_ALL_TIMER, szShowAllProcessesTimer[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_TIMER_DISASSEMBLE, szDisassembleCallback[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_TIMER_REMOVE, szRemove[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_TIMER_EDIT_TIMER, szModifyTimeOutValue[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
// 	menu.AppendMenu(MF_STRING, ID_TIMER_CHECK_SIGN, szVerifyStartModuleSignature[g_enumLang]);
// 	menu.AppendMenu(MF_STRING, ID_TIMER_CHECK_ATTRIBUTE, szStartModuleProperties[g_enumLang]);
// 	menu.AppendMenu(MF_STRING, ID_TIMER_LOCATION_EXPLORER, szFindStartModuleInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_TIMER_CHECK_ATTRIBUTE, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_TIMER_LOCATION_EXPLORER, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_TIMER_CHECK_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_TIMER_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_TIMER_DISASSEMBLE, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_TIMER_REMOVE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_TIMER_CHECK_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_TIMER_LOCATION_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_TIMER_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(13, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount()) // 如果m_list是空的，那么除了“刷新”，其他全部置灰
	{
		for (int i = 4; i < 14; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetSelectedCount()) 
		{
			menu.EnableMenuItem(ID_TIMER_DISASSEMBLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_REMOVE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_EDIT_TIMER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_CHECK_ATTRIBUTE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_LOCATION_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (m_list.GetSelectedCount() >= 2)
		{
			menu.EnableMenuItem(ID_TIMER_DISASSEMBLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_EDIT_TIMER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_CHECK_ATTRIBUTE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_TIMER_LOCATION_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			if (pos != NULL)
			{
				int nItem = m_list.GetNextSelectedItem(pos);
				CString szModule = m_list.GetItemText(nItem, 3);
				if (szModule.IsEmpty())
				{
					menu.EnableMenuItem(ID_TIMER_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_TIMER_CHECK_ATTRIBUTE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_TIMER_LOCATION_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				CString szAddress = m_list.GetItemText(nItem, 2);
				if (szAddress.IsEmpty() || !szAddress.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_TIMER_DISASSEMBLE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
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

void CTimerInfoDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

void CTimerInfoDlg::OnEnKillfocusEdit()
{
// 	CString szOutTime;
// 	m_edit.GetWindowText(szOutTime);
// 	if (!szOutTime.IsEmpty())
// 	{
// 		double dbTimeOut = _wtof(szOutTime);
// 		if (dbTimeOut != 0.0)
// 		{
// 			ULONG ulTime = (ULONG)(dbTimeOut * 1000);
// 			if (ulTime >= USER_TIMER_MINIMUM && ulTime <= USER_TIMER_MAXIMUM)
// 			{
// 				CString szTimerObject = m_list.GetItemText(m_nPreItem, 0);
// 				for ( vector <TIMER>::iterator Iter = m_TimerVector.begin( ); Iter != m_TimerVector.end( ); Iter++ )
// 				{
// 					CString strTimerObject;
// 					strTimerObject.Format(L"0x%08X", Iter->TimerObject);
// 					if (!strTimerObject.CompareNoCase(szTimerObject))
// 					{
// 						COMMUNICATE_TIMER ct;
// 						ct.op.ModifyTimeOut.TimerObject = Iter->TimerObject;
// 						ct.op.ModifyTimeOut.TimeOut = ulTime;
// 						ct.OpType = enumModifyTimeOut;
// 						if (m_driver.CommunicateDriver(&ct, sizeof(COMMUNICATE_TIMER), NULL, 0, NULL))
// 						{
// 							CString szTime;
// 							szTime.Format(L"%.2f", dbTimeOut);
// 							m_list.SetItemText(m_nPreItem, 1, szTime);
// 						}
// 
// 						break;
// 					}
// 				}
// 			}
// 		}
// 	}
// 
// 	m_edit.ShowWindow(SW_HIDE);
}

BOOL CTimerInfoDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->hwnd == (GetDlgItem(IDC_EDIT))->GetSafeHwnd() && pMsg->message == WM_KEYDOWN)
	{		
		char c = (char)pMsg->wParam; 
		if (c == VK_RETURN)
		{
			OnEnKillfocusEdit();
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CTimerInfoDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

CString CTimerInfoDlg::GetDriverPath(ULONG pCallback)
{
	CString szPath;

	for (vector<DRIVER_INFO>::iterator itor = m_CommonDriverList.begin(); itor != m_CommonDriverList.end(); itor++)
	{
		ULONG nBase = itor->nBase;
		ULONG nEnd = itor->nBase + itor->nSize;

		if (pCallback >= nBase && pCallback <= nEnd)
		{
			szPath = itor->szDriverPath;
			break;
		}
	}

	return szPath;
}

void CTimerInfoDlg::OnTimerDisassemble()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 2);
		CString szPid = m_list.GetItemText(nItem, 5);

		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		DWORD dwPid = 0;

		if (!szPid.IsEmpty())
		{
			swscanf_s(szPid.GetBuffer(), L"%d", &dwPid);
		}

		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress, 100, dwPid, 0);
		}
	}
}

BOOL CTimerInfoDlg::GetProcessModuleBySnap(DWORD dwPID) 
{ 
	BOOL bRet    =    FALSE; 
	BOOL bFound    =    TRUE; 
	HANDLE hModuleSnap = NULL; 
	MODULEENTRY32 me32 ={0}; 

	m_TimerModuleList.clear();
	hModuleSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if(hModuleSnap == INVALID_HANDLE_VALUE) 
	{    
		return FALSE; 
	} 

	me32.dwSize = sizeof(MODULEENTRY32); 
	if(::Module32First(hModuleSnap, &me32))//获得第一个模块 
	{ 
		do{ 

			TIMER_MODULE_INFO info;
			info.nBase = (ULONG)me32.modBaseAddr;
			info.nSize = me32.modBaseSize;
			info.szModule = me32.szExePath;
			m_TimerModuleList.push_back(info);

		}while(::Module32Next(hModuleSnap, &me32)); 
	}//递归枚举模块 

	CloseHandle(hModuleSnap); 
	return bFound; 
} 

PROCESS_INFO CTimerInfoDlg::FindProcessItem(vector<PROCESS_INFO> &vectorProcess, ULONG pEprocess)
{
	PROCESS_INFO item;
	memset(&item, 0, sizeof(PROCESS_INFO));

	if (vectorProcess.size() > 0 && pEprocess)
	{
		for ( vector <PROCESS_INFO>::iterator IterProcess = vectorProcess.begin( ); 
			IterProcess != vectorProcess.end( ); 
			IterProcess++ )
		{
			if (IterProcess->ulEprocess == pEprocess)
			{
				item = *IterProcess;
			}
		}
	}

	return item;
}

// BOOL UDgreaterEprocessInfo( TIMER elem1, TIMER elem2 )
// {
// 	return elem1.pEprocess < elem2.pEprocess;
// }

VOID CTimerInfoDlg::SortByEprocess()
{
//	sort( m_TimerVector.begin( ), m_TimerVector.end( ), UDgreaterEprocessInfo );
}