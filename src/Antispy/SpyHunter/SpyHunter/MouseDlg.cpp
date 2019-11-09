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
// MouseDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "MouseDlg.h"


// CMouseDlg 对话框

IMPLEMENT_DYNAMIC(CMouseDlg, CDialog)

CMouseDlg::CMouseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMouseDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_bOnlyShowHooks = FALSE;
	m_nHookedCnt = 0;
	m_MouClassHookVector.clear();
	m_CommonDriverList.clear();
}

CMouseDlg::~CMouseDlg()
{
	m_MouClassHookVector.clear();
}

void CMouseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_MOUCLASS_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CMouseDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMouseDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CMouseDlg::OnNMCustomdrawList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CMouseDlg::OnNMRclickList)
	ON_COMMAND(ID_SDT_REFRESH, &CMouseDlg::OnSdtRefresh)
	ON_COMMAND(ID_SDT_ONLY_SHOW_HOOK, &CMouseDlg::OnSdtOnlyShowHook)
	ON_UPDATE_COMMAND_UI(ID_SDT_ONLY_SHOW_HOOK, &CMouseDlg::OnUpdateSdtOnlyShowHook)
	ON_COMMAND(ID_SDT_DIS_CURRENT, &CMouseDlg::OnSdtDisCurrent)
	ON_COMMAND(ID_SDT_DIS_ORIGIN, &CMouseDlg::OnSdtDisOrigin)
	ON_COMMAND(ID_SDT_RESTORE, &CMouseDlg::OnSdtRestore)
	ON_COMMAND(ID_SDT_RESTORE_ALL, &CMouseDlg::OnSdtRestoreAll)
	ON_COMMAND(ID_SDT_SHUXING, &CMouseDlg::OnSdtShuxing)
	ON_COMMAND(ID_SDT_LOCATION_MODULE, &CMouseDlg::OnSdtLocationModule)
	ON_COMMAND(ID_SDT_VERIFY_SIGN, &CMouseDlg::OnSdtVerifySign)
	ON_COMMAND(ID_SDT_TEXT, &CMouseDlg::OnSdtText)
	ON_COMMAND(ID_SDT_EXCEL, &CMouseDlg::OnSdtExcel)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CMouseDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CMouseDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_MOUCLASS_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CMouseDlg 消息处理程序

void CMouseDlg::OnBnClickedOk()
{
}

void CMouseDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CMouseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szIndex[g_enumLang], LVCFMT_LEFT, 60);
	m_list.InsertColumn(1, szFunction[g_enumLang], LVCFMT_LEFT, 290);
	m_list.InsertColumn(2, szOriginalEntry[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(3, szHookType[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(4, szCurrentEntry[g_enumLang], LVCFMT_LEFT, 260);
	m_list.InsertColumn(5, szModule[g_enumLang], LVCFMT_LEFT, 350);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMouseDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

CString CMouseDlg::GetDriverPath(ULONG pCallback)
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

void CMouseDlg::GetMouclassDispatch()
{
	OPERATE_TYPE opType = enumMouClassHook;
	BOOL bRet = FALSE;
	ULONG nTotalCount = 0;

	m_list.DeleteAllItems();
	m_MouClassHookVector.clear();

	ULONG nSize = 28 * sizeof(DISPATCH_HOOK_INFO);
	PDISPATCH_HOOK_INFO pFsdHookInfo = (PDISPATCH_HOOK_INFO)malloc(nSize);

	if (pFsdHookInfo)
	{
		memset(pFsdHookInfo, 0, nSize);
		bRet = m_Driver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pFsdHookInfo, nSize, NULL);
	}

	if (bRet)
	{
		GetDriver();

		if (pFsdHookInfo[0].pNowAddress)
		{
			for (ULONG i = 0; i < 28; i++)
			{
				m_MouClassHookVector.push_back(pFsdHookInfo[i]);
			}
		}
	}

	if (pFsdHookInfo)
	{
		free(pFsdHookInfo);
		pFsdHookInfo = NULL;
	}

	m_nHookedCnt = 0;

	for (vector<DISPATCH_HOOK_INFO>::iterator itor = m_MouClassHookVector.begin(); itor != m_MouClassHookVector.end(); itor++)
	{
		InsertMouclassItem(*itor);
		nTotalCount++;
	}

	m_szStatus.Format(szMouStatus[g_enumLang], nTotalCount, m_nHookedCnt);
	UpdateData(FALSE);
}

void CMouseDlg::InsertMouclassItem(DISPATCH_HOOK_INFO HookInfo)
{
	CString szHooked, szCurrentEntry, szIndex, szOriginalEntry, szName, szPath;
	BOOL bHooked = FALSE;
	int n = 0;

	szName = szDispatchName[HookInfo.nIndex];
	szIndex.Format(L"%d", HookInfo.nIndex);
	szOriginalEntry.Format(L"0x%08X", HookInfo.pOriginAddress);

	if (HookInfo.pNowAddress != HookInfo.pOriginAddress &&
		HookInfo.pInlineHookAddress)
	{
		CString szCurrentEntryTemp;
		CString szEntry1, szEntry2;
		CString szPath1, szPath2;

		szPath1 = GetDriverPath(HookInfo.pNowAddress);
		szEntry1.Format(L"0x%08X", HookInfo.pNowAddress);

		if (HookInfo.pInlineHookAddress == 1)
		{
			szEntry2.Format(L"0x%08X", HookInfo.pNowAddress);
			szPath2 = GetDriverPath(HookInfo.pNowAddress);
		}
		else
		{
			szEntry2.Format(L"0x%08X", HookInfo.pInlineHookAddress);
			szPath2 = GetDriverPath(HookInfo.pInlineHookAddress);

			if (szPath2.IsEmpty())
			{
				ULONG nRet = m_Functions.GetInlineAddress(HookInfo.pInlineHookAddress);

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
			CString szInlineHookTemp;
			ULONG nRet = m_Functions.GetInlineAddress(HookInfo.pNowAddress);
			for (int i = 0; i < 5; i++)
			{
				if (!nRet)
				{
					break;
				}

				CString szRet;
				szRet.Format(L"0x%08X", nRet);
				szInlineHookTemp += L"->";
				szInlineHookTemp += szRet;

				CString szPathTemp = GetDriverPath(nRet);
				if (!szPathTemp.IsEmpty())
				{
					szPath1 = szPathTemp;
					szEntry1 += szInlineHookTemp;
					break;
				}

				nRet = m_Functions.GetInlineAddress(nRet);
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

		szCurrentEntry.Format(L"(mouclass)%s, (inline)%s", szEntry1, szEntry2);
		szPath.Format(L"(mouclass)%s, (inline)%s", szPath1, szPath2);
		szHooked = L"mouclass & inline";
		bHooked = TRUE;
	}
	else if (HookInfo.pNowAddress != HookInfo.pOriginAddress)
	{
		szCurrentEntry.Format(L"0x%08X", HookInfo.pNowAddress);
		szPath = GetDriverPath(HookInfo.pNowAddress);
		szHooked = L"mouclass hook";

		if (szPath.IsEmpty())
		{
			CString szInlineHookTemp;
			ULONG nRet = m_Functions.GetInlineAddress(HookInfo.pNowAddress);
			for (int i = 0; i < 5; i++)
			{
				if (!nRet)
				{
					break;
				}

				CString szRet;
				szRet.Format(L"0x%08X", nRet);
				szInlineHookTemp += L"->";
				szInlineHookTemp += szRet;

				CString szPathTemp = GetDriverPath(nRet);
				if (!szPathTemp.IsEmpty())
				{
					szPath = szPathTemp;
					szCurrentEntry += szInlineHookTemp;
					break;
				}

				nRet = m_Functions.GetInlineAddress(nRet);
			}
		}

		if (szPath.IsEmpty())
		{
			szPath = szUnknowModule[g_enumLang];
		}

		bHooked = TRUE;
	}
	else if (HookInfo.pInlineHookAddress)
	{
		if (HookInfo.pInlineHookAddress == 1)
		{
			szCurrentEntry.Format(L"0x%08X", HookInfo.pNowAddress);
			szPath = GetDriverPath(HookInfo.pNowAddress);
		}
		else
		{
			szCurrentEntry.Format(L"0x%08X", HookInfo.pInlineHookAddress);
			szPath = GetDriverPath(HookInfo.pInlineHookAddress);
			CString szCurrentEntryTemp;

			if (szPath.IsEmpty())
			{
				ULONG nRet = m_Functions.GetInlineAddress(HookInfo.pInlineHookAddress);

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

		szHooked = L"inline hook";
		bHooked = TRUE;
	}
	else
	{
		szPath = GetDriverPath(HookInfo.pNowAddress);
		szCurrentEntry.Format(L"0x%08X", HookInfo.pNowAddress);
		szHooked = L"-";
		bHooked = FALSE;
	}

	if (m_bOnlyShowHooks)
	{
		if (bHooked)
		{
			n = m_list.InsertItem(m_list.GetItemCount(), szIndex);
			m_list.SetItemText(n, 1, szName);
			m_list.SetItemText(n, 2, szOriginalEntry);
			m_list.SetItemText(n, 3, szHooked);
			m_list.SetItemText(n, 4, szCurrentEntry);
			m_list.SetItemText(n, 5, szPath);
		}
	}
	else
	{
		n = m_list.InsertItem(m_list.GetItemCount(), szIndex);
		m_list.SetItemText(n, 1, szName);
		m_list.SetItemText(n, 2, szOriginalEntry);
		m_list.SetItemText(n, 3, szHooked);
		m_list.SetItemText(n, 4, szCurrentEntry);
		m_list.SetItemText(n, 5, szPath);
	}

	if (bHooked)
	{
		m_nHookedCnt++;
		m_list.SetItemData(n, enumHooked);
	}
}	

void CMouseDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CMouseDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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
				CString szHookType = m_list.GetItemText(nIndex, 3);
				if (szHookType.CompareNoCase(L"-"))
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
				CString szHookType = m_list.GetItemText(nItem, 3);

				if (!szHookType.CompareNoCase(L"-"))
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

void CMouseDlg::OnSdtRefresh()
{
	GetMouclassDispatch();
}

void CMouseDlg::OnSdtOnlyShowHook()
{
	m_bOnlyShowHooks = !m_bOnlyShowHooks;
	OnSdtRefresh();
}

void CMouseDlg::OnUpdateSdtOnlyShowHook(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bOnlyShowHooks);
}

void CMouseDlg::OnSdtDisCurrent()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 4);
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

void CMouseDlg::OnSdtDisOrigin()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 2);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

void CMouseDlg::OnSdtRestore()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();

	while (pos)
	{
		int nItem  = m_list.GetNextSelectedItem(pos);
		CString szHookType = m_list.GetItemText(nItem, 3);

		if (szHookType.CompareNoCase(L"-"))
		{
			CString szIndex = m_list.GetItemText(nItem, 0);
			ULONG nIndex = _wtoi(szIndex);
			DISPATCH_HOOK cs;

			cs.OpType = enumRestoreDispatchHook;
			cs.op.Restore.DispatchType = DispatchMouclass;
			cs.op.Restore.nIndex = nIndex;
			m_Driver.CommunicateDriver(&cs, sizeof(DISPATCH_HOOK), NULL, 0, NULL);
		}

	}

	OnSdtRefresh();
}

void CMouseDlg::OnSdtRestoreAll()
{
	int nCnt = m_list.GetItemCount();

	for (int i = 0; i < nCnt; i++)
	{
		CString szHookType = m_list.GetItemText(i, 3);

		if (szHookType.CompareNoCase(L"-"))
		{
			CString szIndex = m_list.GetItemText(i, 0);
			ULONG nIndex = _wtoi(szIndex);
			DISPATCH_HOOK cs;

			cs.OpType = enumRestoreDispatchHook;
			cs.op.Restore.DispatchType = DispatchMouclass;
			cs.op.Restore.nIndex = nIndex;
			m_Driver.CommunicateDriver(&cs, sizeof(DISPATCH_HOOK), NULL, 0, NULL);
		}
	}

	OnSdtRefresh();
}

void CMouseDlg::OnSdtShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.OnCheckAttribute(szPath);
	}
}

void CMouseDlg::OnSdtLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.LocationExplorer(szPath);
	}
}

void CMouseDlg::OnSdtVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.SignVerify(szPath);
	}
}

void CMouseDlg::OnSdtText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CMouseDlg::OnSdtExcel()
{
	WCHAR szMouse[] = {'M','o','u','s','e','\0'};
	m_Functions.ExportListToExcel(&m_list, szMouse, m_szStatus);
}

void CMouseDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

BOOL CMouseDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CMouseDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);

		if (!_wcsnicmp(szPath.GetBuffer(), L"(", 1))
		{
			szPath = szPath.Left(szPath.GetLength() - szPath.ReverseFind(',') - 2);
			szPath = szPath.Right(szPath.GetLength() - szPath.ReverseFind(')') - 1);
		}

		m_Functions.JmpToFile(szPath);
	}
}
