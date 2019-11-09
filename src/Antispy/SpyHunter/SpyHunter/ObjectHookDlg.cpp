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
// ObjectHookDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ObjectHookDlg.h"


// CObjectHookDlg 对话框

IMPLEMENT_DYNAMIC(CObjectHookDlg, CDialog)

CObjectHookDlg::CObjectHookDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectHookDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_DispatchVector.clear();
	m_CommonDriverList.clear();
	m_TypeVector.clear();
	m_bOnlyShowHooks = FALSE;
	m_nHookedCnt = 0;
	m_nTotalCnt = 0;
}

CObjectHookDlg::~CObjectHookDlg()
{
	m_DispatchVector.clear();
	m_CommonDriverList.clear();
	m_TypeVector.clear();
}

void CObjectHookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_OBJECT_HOOK_STATUS, m_szStatus);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CObjectHookDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CObjectHookDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_WM_INITMENUPOPUP()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CObjectHookDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CObjectHookDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_SDT_REFRESH, &CObjectHookDlg::OnSdtRefresh)
	ON_COMMAND(ID_SDT_ONLY_SHOW_HOOK, &CObjectHookDlg::OnSdtOnlyShowHook)
	ON_UPDATE_COMMAND_UI(ID_SDT_ONLY_SHOW_HOOK, &CObjectHookDlg::OnUpdateSdtOnlyShowHook)
	ON_COMMAND(ID_SDT_DIS_CURRENT, &CObjectHookDlg::OnSdtDisCurrent)
	ON_COMMAND(ID_SDT_DIS_ORIGIN, &CObjectHookDlg::OnSdtDisOrigin)
	ON_COMMAND(ID_SDT_RESTORE, &CObjectHookDlg::OnSdtRestore)
	ON_COMMAND(ID_SDT_RESTORE_ALL, &CObjectHookDlg::OnSdtRestoreAll)
	ON_COMMAND(ID_SDT_SHUXING, &CObjectHookDlg::OnSdtShuxing)
	ON_COMMAND(ID_SDT_LOCATION_MODULE, &CObjectHookDlg::OnSdtLocationModule)
	ON_COMMAND(ID_SDT_VERIFY_SIGN, &CObjectHookDlg::OnSdtVerifySign)
	ON_COMMAND(ID_SDT_TEXT, &CObjectHookDlg::OnSdtText)
	ON_COMMAND(ID_SDT_EXCEL, &CObjectHookDlg::OnSdtExcel)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CObjectHookDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CObjectHookDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_OBJECT_HOOK_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CObjectHookDlg 消息处理程序

void CObjectHookDlg::OnBnClickedOk()
{
}

void CObjectHookDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CObjectHookDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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
		for(UINT i=0; i<count; i++)  
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

BOOL CObjectHookDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szObjectName[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(1, szObjectAddress[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(2, szFunction[g_enumLang], LVCFMT_LEFT, 170);
	m_list.InsertColumn(3, szOriginalEntry[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(4, szHookType[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(5, szCurrentEntry[g_enumLang], LVCFMT_LEFT, 190);
	m_list.InsertColumn(6, szModule[g_enumLang], LVCFMT_LEFT, 340);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CObjectHookDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_SDT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_SDT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SDT_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_ONLY_SHOW_HOOK, szOnlyShowHooks[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_DIS_CURRENT, szDisassemblerCurrentEntry[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SDT_DIS_ORIGIN, szDisassemblerOrginalEntry[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_RESTORE, szRestore[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SDT_RESTORE_ALL, szRestoreAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_SHUXING, szCallbackModuleProperties[g_enumLang]);
// 	menu.AppendMenu(MF_STRING, ID_SDT_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SDT_VERIFY_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	// 设置菜单图标
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_SDT_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_SDT_DIS_CURRENT, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_SDT_DIS_ORIGIN, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_SDT_LOCATION_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_SDT_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_SDT_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_SDT_RESTORE, MF_BYCOMMAND, &m_bmRecover, &m_bmRecover);
		menu.SetMenuItemBitmaps(ID_SDT_RESTORE_ALL, MF_BYCOMMAND, &m_bmRecover, &m_bmRecover);
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
			for (int i = 2; i < 15; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			menu.EnableMenuItem(ID_SDT_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_LOCATION_MODULE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_VERIFY_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_DIS_CURRENT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_DIS_ORIGIN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

			BOOL bHooked = FALSE;
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			while (pos)
			{
				int nIndex  = m_list.GetNextSelectedItem(pos);
				CString szHookType = m_list.GetItemText(nIndex, 4);
				if (szHookType.CompareNoCase(L"-") &&
					szHookType.CompareNoCase(L"unknow type"))
				{
					bHooked = TRUE;
					break;
				}
			}

			if (!bHooked)
			{
				menu.EnableMenuItem(ID_SDT_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);

			if (nItem != -1)
			{
				CString szOriginEntry = m_list.GetItemText(nItem, 3);
				CString szHookType = m_list.GetItemText(nItem, 4);

				if (!szOriginEntry.CompareNoCase(L"0x00000000") || !szOriginEntry.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_SDT_DIS_ORIGIN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				CString szCurrentEntry = m_list.GetItemText(nItem, 5);
				if (!szCurrentEntry.CompareNoCase(L"0x00000000") || !szCurrentEntry.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_SDT_DIS_CURRENT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				if (!szOriginEntry.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_SDT_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
				else if (!szHookType.CompareNoCase(L"-") || !szHookType.CompareNoCase(L"unknow type"))
				{
					menu.EnableMenuItem(ID_SDT_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}

		if (m_nHookedCnt)
		{
			menu.EnableMenuItem(ID_SDT_RESTORE_ALL, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{
			menu.EnableMenuItem(ID_SDT_RESTORE_ALL, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
	}

	menu.EnableMenuItem(ID_SDT_ONLY_SHOW_HOOK, MF_BYCOMMAND | MF_ENABLED);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CObjectHookDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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
		int bHooked = 0;
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		clrNewTextColor = g_MicrosoftItemClr;//RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );	

		bHooked = (int)m_list.GetItemData(nItem); 		
		if (bHooked == enumHooked)
		{
			clrNewTextColor = g_HiddenOrHookItemClr;//RGB( 255, 0, 0 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CObjectHookDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

CString CObjectHookDlg::GetDriverPath(ULONG pCallback)
{
	CString szPath;

	if (pCallback)
	{
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
	}

	return szPath;
}

void CObjectHookDlg::GetObjectHook()
{
	OPERATE_TYPE opType = enumObjectHook;
	BOOL bRet = FALSE;
	
	m_nHookedCnt = 0;
	m_nTotalCnt = 0;

	GetObjectTypes();

	m_list.DeleteAllItems();
	m_DispatchVector.clear();

	ULONG nSize = eMaxType * sizeof(OBJECT_HOOK_INOF);
	POBJECT_HOOK_INOF pObjectHookInfo = (POBJECT_HOOK_INOF)malloc(nSize);

	if (pObjectHookInfo)
	{
		memset(pObjectHookInfo, 0, nSize);
		bRet = m_Driver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pObjectHookInfo, nSize, NULL);
	}

	if (bRet)
	{
		GetDriver();

		for (ULONG i = 0; i < eMaxType; i++)
		{
			m_DispatchVector.push_back(pObjectHookInfo[i]);
		}
	}

	if (pObjectHookInfo)
	{
		free(pObjectHookInfo);
		pObjectHookInfo = NULL;
	}

	m_nHookedCnt = 0;
	
	for (vector<OBJECT_HOOK_INOF>::iterator itor = m_DispatchVector.begin(); itor != m_DispatchVector.end(); itor++)
	{
		InsertObjectHookItem(*itor);
	}

	GetSelfCreateTypeHookInfo();

	m_szStatus.Format(szObjectHookStatus[g_enumLang], m_nTotalCnt, m_nHookedCnt);
	UpdateData(FALSE);
}

void CObjectHookDlg::GetSelfCreateTypeHookInfo()
{
	for (vector<OBJECT_TYPE_INFO>::iterator ItorType = m_TypeVector.begin(); ItorType != m_TypeVector.end(); ItorType++)
	{
		BOOL bFind = FALSE;

		for (vector<OBJECT_HOOK_INOF>::iterator ItorHook = m_DispatchVector.begin(); ItorHook != m_DispatchVector.end(); ItorHook++)
		{
			if (ItorHook->TypeObject == ItorType->ObjectType)
			{
				bFind = TRUE;
				break;
			}
		}

		if (!bFind)
		{
			COMMUNICATE_OBJECT_HOOK coh;
			coh.OpType = enumSelfCreateObjectHook;
			coh.op.GetSlefCreateType.pType = ItorType->ObjectType;

			ULONG nSize = sizeof(OBJECT_HOOK_INOF);
			POBJECT_HOOK_INOF pObjectHookInfo = (POBJECT_HOOK_INOF)malloc(nSize);

			if (pObjectHookInfo)
			{
				memset(pObjectHookInfo, 0, nSize);
				BOOL bRet = m_Driver.CommunicateDriver(&coh, sizeof(COMMUNICATE_OBJECT_HOOK), (PVOID)pObjectHookInfo, nSize, NULL);
				if (bRet)
				{
					InsertSelfCreateObjectHookItem(pObjectHookInfo);
				}

				free(pObjectHookInfo);
			}
		}
	}
}

void CObjectHookDlg::InsertSelfCreateObjectHookItem(POBJECT_HOOK_INOF pHookInfo)
{
	if (pHookInfo)
	{
		CString szObjectName, szCurrentEntry, szOriginalEntry, szPath, szHookType, szFunction, szObjectAddress;

		szObjectName = GetObjectName2(pHookInfo->TypeObject);
		szObjectAddress.Format(L"0x%08X", pHookInfo->TypeObject);
		szOriginalEntry.Format(L"-");

		for (ULONG i = 0; i < 8; i++)
		{
			BOOL bHooked = TRUE;
			ULONG NowAddress = 0, OriginAddress = 0, InlineHookAddress = 0;
			
			switch (i)
			{
			case 0:
				{
					szFunction = L"DumpProcedure";
					NowAddress = pHookInfo->DumpProcedure;
				}
				break;

			case 1:
				{
					szFunction = L"OpenProcedure";
					NowAddress = pHookInfo->OpenProcedure;
				}
				break;

			case 2:
				{
					szFunction = L"CloseProcedure";
					NowAddress = pHookInfo->CloseProcedure;

				}
				break;

			case 3:
				{
					szFunction = L"DeleteProcedure";
					NowAddress = pHookInfo->DeleteProcedure;
				}
				break;

			case 4:
				{
					szFunction = L"ParseProcedure";
					NowAddress = pHookInfo->ParseProcedure;
				}
				break;

			case 5:
				{
					szFunction = L"SecurityProcedure";
					NowAddress = pHookInfo->SecurityProcedure;
				}
				break;

			case 6:
				{
					szFunction = L"QueryNameProcedure";
					NowAddress = pHookInfo->QueryNameProcedure;
				}
				break;

			case 7:
				{
					szFunction = L"OkayToCloseProcedure";
					NowAddress = pHookInfo->OkayToCloseProcedure;
				}
				break;
			}

			if (!NowAddress)
			{
				continue;
			}

			szCurrentEntry.Format(L"0x%08X", NowAddress);
			szHookType = L"unknow type";

			szPath = GetDriverPath(NowAddress);
			if (szPath.IsEmpty())
			{
				ULONG nRet = m_Functions.GetInlineAddress(NowAddress);
				CString szCurrentEntryTemp;

				for (int i = 0; i < 5; i++)
				{
					if (!nRet)
					{
						break;
					}

					CString szRet;
					szRet.Format(L"0x%08X", nRet);
					szCurrentEntryTemp += L"->";
					szCurrentEntryTemp += szRet;

					CString szPathTemp = GetDriverPath(nRet);
					if (!szPathTemp.IsEmpty())
					{
						szPath = szPathTemp;
						szCurrentEntry += szCurrentEntryTemp;
						break;
					}

					nRet = m_Functions.GetInlineAddress(nRet);
				}
			}

			if (szPath.IsEmpty())
			{
				szPath = szUnknowModule[g_enumLang];
			}

			if (m_Functions.IsMicrosoftAppByPath(szPath) &&
				!szHookType.CompareNoCase(L"unknow Type"))
			{
				bHooked = FALSE;
				szOriginalEntry = L"-";
				szHookType = L"-";
			}

			if (m_bOnlyShowHooks)
			{
				if (bHooked)
				{
					int n = m_list.InsertItem(m_list.GetItemCount(), szObjectName);
					m_list.SetItemText(n, 1, szObjectAddress);
					m_list.SetItemText(n, 2, szFunction);
					m_list.SetItemText(n, 3, szOriginalEntry);
					m_list.SetItemText(n, 4, szHookType);
					m_list.SetItemText(n, 5, szCurrentEntry);
					m_list.SetItemText(n, 6, szPath);

					m_list.SetItemData(n, bHooked);
				}
			}
			else
			{
				int n = m_list.InsertItem(m_list.GetItemCount(), szObjectName);
				m_list.SetItemText(n, 1, szObjectAddress);
				m_list.SetItemText(n, 2, szFunction);
				m_list.SetItemText(n, 3, szOriginalEntry);
				m_list.SetItemText(n, 4, szHookType);
				m_list.SetItemText(n, 5, szCurrentEntry);
				m_list.SetItemText(n, 6, szPath);

				m_list.SetItemData(n, bHooked);
			}

			m_nTotalCnt ++;
		}
	}	
}

void CObjectHookDlg::InsertObjectHookItem(OBJECT_HOOK_INOF HookInfo)
{
	if (HookInfo.TypeObject == 0)
	{
		return;
	}

	CString szObjectName, szCurrentEntry, szOriginalEntry, szPath, szHookType, szFunction, szObjectAddress;

	szObjectName = GetObjectName(HookInfo);
	szObjectAddress.Format(L"0x%08X", HookInfo.TypeObject);

	for (ULONG i = 0; i < 8; i++)
	{
		BOOL bHooked = FALSE;
		ULONG NowAddress = 0, OriginAddress = 0, InlineHookAddress = 0;

		switch (i)
		{
		case 0:
			{
				szFunction = L"DumpProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginDumpProcedure);

				NowAddress = HookInfo.DumpProcedure;
				OriginAddress = HookInfo.OriginDumpProcedure;
				InlineHookAddress = HookInfo.InlineDumpProcedure;
			}
			break;

		case 1:
			{
				szFunction = L"OpenProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginOpenProcedure);

				NowAddress = HookInfo.OpenProcedure;
				OriginAddress = HookInfo.OriginOpenProcedure;
				InlineHookAddress = HookInfo.InlineOpenProcedure;
			}
			break;

		case 2:
			{
				szFunction = L"CloseProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginCloseProcedure);

				NowAddress = HookInfo.CloseProcedure;
				OriginAddress = HookInfo.OriginCloseProcedure;
				InlineHookAddress = HookInfo.InlineCloseProcedure;
			}
			break;

		case 3:
			{
				szFunction = L"DeleteProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginDeleteProcedure);

				NowAddress = HookInfo.DeleteProcedure;
				OriginAddress = HookInfo.OriginDeleteProcedure;
				InlineHookAddress = HookInfo.InlineDeleteProcedure;
			}
			break;

		case 4:
			{
				szFunction = L"ParseProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginParseProcedure);

				NowAddress = HookInfo.ParseProcedure;
				OriginAddress = HookInfo.OriginParseProcedure;
				InlineHookAddress = HookInfo.InlineParseProcedure;
			}
			break;

		case 5:
			{
				szFunction = L"SecurityProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginSecurityProcedure);

				NowAddress = HookInfo.SecurityProcedure;
				OriginAddress = HookInfo.OriginSecurityProcedure;
				InlineHookAddress = HookInfo.InlineSecurityProcedure;
			}
			break;

		case 6:
			{
				szFunction = L"QueryNameProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginQueryNameProcedure);

				NowAddress = HookInfo.QueryNameProcedure;
				OriginAddress = HookInfo.OriginQueryNameProcedure;
				InlineHookAddress = HookInfo.InlineQueryNameProcedure;
			}
			break;

		case 7:
			{
				szFunction = L"OkayToCloseProcedure";
				szOriginalEntry.Format(L"0x%08X", HookInfo.OriginOkayToCloseProcedure);

				NowAddress = HookInfo.OkayToCloseProcedure;
				OriginAddress = HookInfo.OriginOkayToCloseProcedure;
				InlineHookAddress = HookInfo.InlineOkayToCloseProcedure;
			}
			break;
		}

		if (!NowAddress && !OriginAddress)
		{
			continue;
		}

		if (NowAddress && OriginAddress)
		{
			if (NowAddress != OriginAddress && InlineHookAddress)
			{
				CString szEntry1, szEntry2;
				CString szPath1, szPath2;

				szPath1 = GetDriverPath(NowAddress);
				szEntry1.Format(L"0x%08X", NowAddress);
				
				if (szPath1.IsEmpty())
				{
					ULONG nRet = m_Functions.GetInlineAddress(NowAddress);
					CString szTemp;

					for (int i = 0; i < 5; i++)
					{
						if (!nRet)
						{
							break;
						}

						CString szRet;
						szRet.Format(L"0x%08X", nRet);
						szTemp += L"->";
						szTemp += szRet;

						CString szPathTemp = GetDriverPath(nRet);
						if (!szPathTemp.IsEmpty())
						{
							szPath1 = szPathTemp;
							szEntry1 += szTemp;
							break;
						}

						nRet = m_Functions.GetInlineAddress(nRet);
					}
				}

				if (InlineHookAddress == 1)
				{
					szEntry2.Format(L"0x%08X", NowAddress);
					szPath2 = GetDriverPath(NowAddress);
				}
				else
				{
					szEntry2.Format(L"0x%08X", InlineHookAddress);
					szPath2 = GetDriverPath(InlineHookAddress);

					if (szPath2.IsEmpty())
					{
						ULONG nRet = m_Functions.GetInlineAddress(InlineHookAddress);
						CString szCurrentEntryTemp;

						for (int i = 0; i < 5; i++)
						{
							if (!nRet)
							{
								break;
							}

							CString szRet;
							szRet.Format(L"0x%08X", nRet);
							szCurrentEntryTemp += L"->";
							szCurrentEntryTemp += szRet;

							CString szPathTemp = GetDriverPath(nRet);
							if (!szPathTemp.IsEmpty())
							{
								szPath2 = szPathTemp;
								szEntry2 += szCurrentEntryTemp;
								break;
							}

							nRet = m_Functions.GetInlineAddress(nRet);
						}
					}
				}

				if (szPath1.IsEmpty())
				{
					szPath1 = szUnknowModule[g_enumLang];
				}

				if (szPath2.IsEmpty())
				{
					szPath2 = szUnknowModule[g_enumLang];
				}

				szCurrentEntry.Format(L"(object)%s, (inline)%s", szEntry1, szEntry2);
				szPath.Format(L"(object)%s, (inline)%s", szPath1, szPath2);
				szHookType = L"object & inline";
				bHooked = TRUE;
			}
			else if (NowAddress != OriginAddress)
			{
				szCurrentEntry.Format(L"0x%08X", NowAddress);
				szHookType = L"object hook";
				bHooked = TRUE;

				szPath = GetDriverPath(NowAddress);
				if (szPath.IsEmpty())
				{
					ULONG nRet = m_Functions.GetInlineAddress(NowAddress);
					CString szCurrentEntryTemp;

					for (int i = 0; i < 5; i++)
					{
						if (!nRet)
						{
							break;
						}

						CString szRet;
						szRet.Format(L"0x%08X", nRet);
						szCurrentEntryTemp += L"->";
						szCurrentEntryTemp += szRet;

						CString szPathTemp = GetDriverPath(nRet);
						if (!szPathTemp.IsEmpty())
						{
							szPath = szPathTemp;
							szCurrentEntry += szCurrentEntryTemp;
							break;
						}

						nRet = m_Functions.GetInlineAddress(nRet);
					}
				}

				if (szPath.IsEmpty())
				{
					szPath = szUnknowModule[g_enumLang];
				}
			}
			else if (InlineHookAddress)
			{
				if (InlineHookAddress == 1)
				{
					szCurrentEntry.Format(L"0x%08X",NowAddress);
					szPath = GetDriverPath(NowAddress);
				}
				else
				{
					szCurrentEntry.Format(L"0x%08X", InlineHookAddress);
					szHookType = L"inline hook";
					bHooked = TRUE;

					szPath = GetDriverPath(InlineHookAddress);
					if (szPath.IsEmpty())
					{
						ULONG nRet = m_Functions.GetInlineAddress(InlineHookAddress);
						CString szCurrentEntryTemp;

						for (int i = 0; i < 5; i++)
						{
							if (!nRet)
							{
								break;
							}

							CString szRet;
							szRet.Format(L"0x%08X", nRet);
							szCurrentEntryTemp += L"->";
							szCurrentEntryTemp += szRet;

							CString szPathTemp = GetDriverPath(nRet);
							if (!szPathTemp.IsEmpty())
							{
								szPath.Empty();
								szPath = szPathTemp;
								szCurrentEntry += szCurrentEntryTemp;
								break;
							}

							nRet = m_Functions.GetInlineAddress(nRet);
						}
					}
				}

				if (szPath.IsEmpty())
				{
					szPath = szUnknowModule[g_enumLang];
				}
			}
			else
			{
				szCurrentEntry.Format(L"0x%08X", NowAddress);
				szHookType = L"-";
				bHooked = FALSE;

				szPath = GetDriverPath(NowAddress);
				if (szPath.IsEmpty())
				{
					ULONG nRet = m_Functions.GetInlineAddress(NowAddress);
					CString szCurrentEntryTemp;

					for (int i = 0; i < 5; i++)
					{
						if (!nRet)
						{
							break;
						}

						CString szRet;
						szRet.Format(L"0x%08X", nRet);
						szCurrentEntryTemp += L"->";
						szCurrentEntryTemp += szRet;

						CString szPathTemp = GetDriverPath(nRet);
						if (!szPathTemp.IsEmpty())
						{
							szPath = szPathTemp;
							szCurrentEntry += szCurrentEntryTemp;
							break;
						}

						nRet = m_Functions.GetInlineAddress(nRet);
					}
				}
			}
		}
		else
		{
			szCurrentEntry.Format(L"0x%08X", NowAddress);
			szHookType = L"object hook";
			bHooked = TRUE;
			
			szPath = GetDriverPath(NowAddress);
			if (szPath.IsEmpty())
			{
				ULONG nRet = m_Functions.GetInlineAddress(NowAddress);
				CString szCurrentEntryTemp;

				for (int i = 0; i < 5; i++)
				{
					if (!nRet)
					{
						break;
					}

					CString szRet;
					szRet.Format(L"0x%08X", nRet);
					szCurrentEntryTemp += L"->";
					szCurrentEntryTemp += szRet;

					CString szPathTemp = GetDriverPath(nRet);
					if (!szPathTemp.IsEmpty())
					{
						szPath = szPathTemp;
						szCurrentEntry += szCurrentEntryTemp;
						break;
					}

					nRet = m_Functions.GetInlineAddress(nRet);
				}
			}
		}

		if (szPath.IsEmpty())
		{
			szPath = szUnknowModule[g_enumLang];
		}

		CString szImage = szPath.Right(szPath.GetLength() - szPath.ReverseFind(L'\\') - 1);
		if (!szImage.CompareNoCase(L"win32k.sys") ||
			!szImage.CompareNoCase(L"fltmgr.sys"))
		{
			if (OriginAddress == 0)
			{
				bHooked = FALSE;
				szOriginalEntry = L"-";
				szHookType = L"-";
			}
		}
		
		if (m_bOnlyShowHooks)
		{
			if (bHooked)
			{
				int n = m_list.InsertItem(m_list.GetItemCount(), szObjectName);
				m_list.SetItemText(n, 1, szObjectAddress);
				m_list.SetItemText(n, 2, szFunction);
				m_list.SetItemText(n, 3, szOriginalEntry);
				m_list.SetItemText(n, 4, szHookType);
				m_list.SetItemText(n, 5, szCurrentEntry);
				m_list.SetItemText(n, 6, szPath);

				m_list.SetItemData(n, bHooked);
			}
		}
		else
		{
			int n = m_list.InsertItem(m_list.GetItemCount(), szObjectName);
			m_list.SetItemText(n, 1, szObjectAddress);
			m_list.SetItemText(n, 2, szFunction);
			m_list.SetItemText(n, 3, szOriginalEntry);
			m_list.SetItemText(n, 4, szHookType);
			m_list.SetItemText(n, 5, szCurrentEntry);
			m_list.SetItemText(n, 6, szPath);

			m_list.SetItemData(n, bHooked);
		}

		m_nHookedCnt += bHooked ? 1 : 0;
		m_nTotalCnt ++;
	}
}

CString CObjectHookDlg::GetObjectName(OBJECT_HOOK_INOF HookInfo)
{
	CString szObject;

	switch (HookInfo.Type)
	{
	case eDirectoryType:
		szObject = L"Directory";
		break;

	case eMutantType:
		szObject = L"Mutant";
		break;

	case eThreadType:
		szObject = L"Thread";
		break;

	case eFilterCommunicationPortType:
		szObject = L"FilterCommunicationPort";
		break;

	case eControllerType:
		szObject = L"Controller";
		break;

	case eProfileType:
		szObject = L"Profile";
		break;

	case eEventType:
		szObject = L"Event";
		break;

	case eTypeType:
		szObject = L"Type";
		break;

	case eSectionType:
		szObject = L"Section";
		break;

	case eEventPairType:
		szObject = L"EventPair";
		break;

	case eSymbolicLinkType:
		szObject = L"SymbolicLink";
		break;

	case eDesktopType:
		szObject = L"Desktop";
		break;

	case eTimerType:
		szObject = L"Timer";
		break;

	case eFileType:
		szObject = L"File";
		break;

	case eWindowStationType:
		szObject = L"WindowStation";
		break;

	case eDriverType:
		szObject = L"Driver";
		break;

	case eWmiGuidType:
		szObject = L"WmiGuid";
		break;

	case eKeyedEventType:
		szObject = L"KeyedEvent";
		break;

	case eDeviceType:
		szObject = L"Device";
		break;

	case eTokenType:
		szObject = L"Token";
		break;

	case eDebugObjectType:
		szObject = L"DebugObject";
		break;

	case eIoCompletionType:
		szObject = L"IoCompletion";
		break;

	case eProcessType:
		szObject = L"Process";
		break;

	case eAdapterType:
		szObject = L"Adapter";
		break;

	case eKeyType:
		szObject = L"Key";
		break;

	case eJobType:
		szObject = L"Job";
		break;

	case eWaitablePortType:
		szObject = L"WaitablePort";
		break;

	case ePortType:
		if (g_WinVersion == enumWINDOWS_VISTA ||
			g_WinVersion == enumWINDOWS_VISTA_SP1_SP2 ||
			g_WinVersion == enumWINDOWS_7 ||
			g_WinVersion == enumWINDOWS_8)
		{
			szObject = L"ALPC Port";
		}
		else
		{
			szObject = L"Port";
		}
		break;

	case eCallbackType:
		szObject = L"Callback";
		break;

	case eFilterConnectionPortType:
		szObject = L"FilterConnectionPort";
		break;

	case eSemaphoreType:
		szObject = L"Semaphore";
		break;

	default:
		szObject = L"UnKnow Type";
		break;
	}

	return szObject;
}

void CObjectHookDlg::GetObjectTypes()
{
	OPERATE_TYPE opType = enumGetObjectTypes;
	POBJECT_TYPE_INFO pTypeInfo = NULL;
	BOOL bRet = FALSE;
	
	m_TypeVector.clear();

	ULONG nSize = sizeof(OBJECT_TYPE_INFO) * 100;

	if (pTypeInfo)
	{
		free(pTypeInfo);
		pTypeInfo = NULL;
	}

	pTypeInfo = (POBJECT_TYPE_INFO)malloc(nSize);

	if (pTypeInfo)
	{
		memset(pTypeInfo, 0, nSize);
		bRet = m_Driver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pTypeInfo, nSize, NULL);
	}

	if (bRet)
	{
		for (ULONG i = 0; pTypeInfo[i].ObjectType && wcslen(pTypeInfo[i].szTypeName) > 0; i++)
		{
			m_TypeVector.push_back(pTypeInfo[i]);
		}
	}

	if (pTypeInfo)
	{
		free(pTypeInfo);
		pTypeInfo = NULL;
	}
}

CString CObjectHookDlg::GetObjectName2(ULONG Object)
{
	for (vector<OBJECT_TYPE_INFO>::iterator itor = m_TypeVector.begin(); itor != m_TypeVector.end(); itor++)
	{
		if (Object == itor->ObjectType)
		{
			return itor->szTypeName;
			break;
		}
	}

	return NULL;
}

void CObjectHookDlg::OnSdtRefresh()
{
	GetObjectHook();
}

void CObjectHookDlg::OnSdtOnlyShowHook()
{
	m_bOnlyShowHooks = !m_bOnlyShowHooks;
	GetObjectHook();
}

void CObjectHookDlg::OnUpdateSdtOnlyShowHook(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bOnlyShowHooks);
}

void CObjectHookDlg::OnSdtDisCurrent()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 5);
		if (szAddress.Find(')') != -1)
		{
			szAddress = szAddress.Right(szAddress.GetLength() - szAddress.Find(')') - 1);
		}
		else if (szAddress.Find('>' != -1))
		{
			szAddress = szAddress.Right(szAddress.GetLength() - szAddress.Find('>') - 1);
		}

		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

void CObjectHookDlg::OnSdtDisOrigin()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 3);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

void CObjectHookDlg::OnSdtRestore()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();

	while (pos)
	{
		int nItem  = m_list.GetNextSelectedItem(pos);
		CString szOriginProcedure = m_list.GetItemText(nItem, 3);
		CString szHookType = m_list.GetItemText(nItem, 4);

		if (szOriginProcedure.CompareNoCase(L"-") &&
			(szHookType.CompareNoCase(L"-") && szHookType.CompareNoCase(L"unknow type")))
		{
			CString szFunction = m_list.GetItemText(nItem, 2);
			ENUM_OBJECT_PROCEDURE ProcedureType = enumProcedureUnknow;

			if (!szFunction.CompareNoCase(L"DumpProcedure"))
			{
				ProcedureType = enumDumpProcedure;
			}
			else if (!szFunction.CompareNoCase(L"OpenProcedure"))
			{
				ProcedureType = enumOpenProcedure;
			}
			else if (!szFunction.CompareNoCase(L"CloseProcedure"))
			{
				ProcedureType = enumCloseProcedure;
			}
			else if (!szFunction.CompareNoCase(L"DeleteProcedure"))
			{
				ProcedureType = enumDeleteProcedure;
			}
			else if (!szFunction.CompareNoCase(L"ParseProcedure"))
			{
				ProcedureType = enumParseProcedure;
			}
			else if (!szFunction.CompareNoCase(L"SecurityProcedure"))
			{
				ProcedureType = enumSecurityProcedure;
			}
			else if (!szFunction.CompareNoCase(L"QueryNameProcedure"))
			{
				ProcedureType = enumQueryNameProcedure;
			}
			else if (!szFunction.CompareNoCase(L"OkayToCloseProcedure"))
			{
				ProcedureType = enumOkayToCloseProcedure;
			}

			DebugLog(L"ProcedureType: %d", ProcedureType);
			if (ProcedureType != enumProcedureUnknow)
			{
				CString szObject = m_list.GetItemText(nItem, 1);

				DebugLog(L"ObjectType: 0x%08X, OriginProcedure: 0x%08X", m_Functions.HexStringToLong(szObject), m_Functions.HexStringToLong(szOriginProcedure));

				COMMUNICATE_OBJECT_HOOK cs;
				cs.OpType = enumRestoreObjectHook;
				cs.op.Restore.ObjectType = m_Functions.HexStringToLong(szObject);
				cs.op.Restore.ObjectProcedureType = ProcedureType;
				cs.op.Restore.OriginProcedure = m_Functions.HexStringToLong(szOriginProcedure);
				m_Driver.CommunicateDriver(&cs, sizeof(COMMUNICATE_OBJECT_HOOK), NULL, 0, NULL);
			}
		}
	}

	OnSdtRefresh();
}

void CObjectHookDlg::OnSdtRestoreAll()
{
	int nCnt = m_list.GetItemCount();

	for (int i = 0; i < nCnt; i++)
	{
		CString szFunction = m_list.GetItemText(i, 2);
		CString szOriginProcedure = m_list.GetItemText(i, 3);
		CString szHookType = m_list.GetItemText(i, 4);

		if (szOriginProcedure.CompareNoCase(L"-") &&
			(szHookType.CompareNoCase(L"-") && szHookType.CompareNoCase(L"unknow type")))
		{
			ENUM_OBJECT_PROCEDURE ProcedureType = enumProcedureUnknow;

			if (!szFunction.CompareNoCase(L"DumpProcedure"))
			{
				ProcedureType = enumDumpProcedure;
			}
			else if (!szFunction.CompareNoCase(L"OpenProcedure"))
			{
				ProcedureType = enumOpenProcedure;
			}
			else if (!szFunction.CompareNoCase(L"CloseProcedure"))
			{
				ProcedureType = enumCloseProcedure;
			}
			else if (!szFunction.CompareNoCase(L"DeleteProcedure"))
			{
				ProcedureType = enumDeleteProcedure;
			}
			else if (!szFunction.CompareNoCase(L"ParseProcedure"))
			{
				ProcedureType = enumParseProcedure;
			}
			else if (!szFunction.CompareNoCase(L"SecurityProcedure"))
			{
				ProcedureType = enumSecurityProcedure;
			}
			else if (!szFunction.CompareNoCase(L"QueryNameProcedure"))
			{
				ProcedureType = enumQueryNameProcedure;
			}
			else if (!szFunction.CompareNoCase(L"OkayToCloseProcedure"))
			{
				ProcedureType = enumOkayToCloseProcedure;
			}

			DebugLog(L"ProcedureType: %d", ProcedureType);
			if (ProcedureType != enumProcedureUnknow)
			{
				CString szObject = m_list.GetItemText(i, 1);
				CString szOriginProcedure = m_list.GetItemText(i, 3);

				DebugLog(L"ObjectType: 0x%08X, OriginProcedure: 0x%08X", m_Functions.HexStringToLong(szObject), m_Functions.HexStringToLong(szOriginProcedure));

				COMMUNICATE_OBJECT_HOOK cs;
				cs.OpType = enumRestoreObjectHook;
				cs.op.Restore.ObjectType = m_Functions.HexStringToLong(szObject);
				cs.op.Restore.ObjectProcedureType = ProcedureType;
				cs.op.Restore.OriginProcedure = m_Functions.HexStringToLong(szOriginProcedure);
				m_Driver.CommunicateDriver(&cs, sizeof(COMMUNICATE_OBJECT_HOOK), NULL, 0, NULL);
			}
		}
	}

	OnSdtRefresh();
}

void CObjectHookDlg::OnSdtShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.OnCheckAttribute(szPath);
	}
}

void CObjectHookDlg::OnSdtLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.LocationExplorer(szPath);
	}
}

void CObjectHookDlg::OnSdtVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.SignVerify(szPath);
	}
}

void CObjectHookDlg::OnSdtText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CObjectHookDlg::OnSdtExcel()
{
	WCHAR szObjectHook[] = {'O','b','j','e','c','t','H','o','o','k','\0'};
	m_Functions.ExportListToExcel(&m_list, szObjectHook, m_szStatus);
}

BOOL CObjectHookDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CObjectHookDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.JmpToFile(szPath);
	}
}
