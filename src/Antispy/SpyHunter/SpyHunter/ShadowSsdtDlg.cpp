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
// ShadowSsdtDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ShadowSsdtDlg.h"
#include "ShadowFunctions.h"
#include "SsdtHookDlg.h"

// CShadowSsdtDlg 对话框

IMPLEMENT_DYNAMIC(CShadowSsdtDlg, CDialog)

CShadowSsdtDlg::CShadowSsdtDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShadowSsdtDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_nHooked = 0;
	m_bOnlyShowHooks = TRUE;
	m_vectorHooks.clear();
	m_vectorDrivers.clear();
}

CShadowSsdtDlg::~CShadowSsdtDlg()
{
	m_vectorHooks.clear();
	m_vectorDrivers.clear();
}

void CShadowSsdtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_SHADOW_SSDT_HOOK_CNT, m_szStatus);
}


BEGIN_MESSAGE_MAP(CShadowSsdtDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CShadowSsdtDlg::OnNMCustomdrawList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CShadowSsdtDlg::OnNMRclickList)
	ON_COMMAND(ID_SDT_REFRESH, &CShadowSsdtDlg::OnSdtRefresh)
	ON_COMMAND(ID_SDT_ONLY_SHOW_HOOK, &CShadowSsdtDlg::OnSdtOnlyShowHook)
	ON_UPDATE_COMMAND_UI(ID_SDT_ONLY_SHOW_HOOK, &CShadowSsdtDlg::OnUpdateSdtOnlyShowHook)
	ON_COMMAND(ID_SDT_DIS_CURRENT, &CShadowSsdtDlg::OnSdtDisCurrent)
	ON_COMMAND(ID_SDT_DIS_ORIGIN, &CShadowSsdtDlg::OnSdtDisOrigin)
	ON_COMMAND(ID_SDT_RESTORE, &CShadowSsdtDlg::OnSdtRestore)
	ON_COMMAND(ID_SDT_RESTORE_ALL, &CShadowSsdtDlg::OnSdtRestoreAll)
	ON_COMMAND(ID_SDT_SHUXING, &CShadowSsdtDlg::OnSdtShuxing)
	ON_COMMAND(ID_SDT_LOCATION_MODULE, &CShadowSsdtDlg::OnSdtLocationModule)
	ON_COMMAND(ID_SDT_VERIFY_SIGN, &CShadowSsdtDlg::OnSdtVerifySign)
	ON_COMMAND(ID_SDT_TEXT, &CShadowSsdtDlg::OnSdtText)
	ON_COMMAND(ID_SDT_EXCEL, &CShadowSsdtDlg::OnSdtExcel)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CShadowSsdtDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CShadowSsdtDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_SHADOW_SSDT_HOOK_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CShadowSsdtDlg 消息处理程序

void CShadowSsdtDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CShadowSsdtDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szIndex[g_enumLang], LVCFMT_LEFT, 60);
	m_list.InsertColumn(1, szFunction[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(2, szOriginalEntry[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(3, szHookType[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(4, szCurrentEntry[g_enumLang], LVCFMT_LEFT, 255);
	m_list.InsertColumn(5, szModule[g_enumLang], LVCFMT_LEFT, 430);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

CString CShadowSsdtDlg::GetDriverPath(ULONG pCallback)
{
	CString szPath;

	for (vector<DRIVER_INFO>::iterator itor = m_vectorDrivers.begin(); itor != m_vectorDrivers.end(); itor++)
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

void CShadowSsdtDlg::GetShadowSsdtHooks()
{
	m_szStatus.Format(szShadowSSdtStatus[g_enumLang], 0, 0);
	UpdateData(FALSE);
	
	m_nHooked = 0;
	m_vectorHooks.clear();
	m_vectorDrivers.clear();
	m_list.DeleteAllItems();

	OPERATE_TYPE opType = enumListShadowSsdtHooks;
	ULONG nCnt = 1000;
	PSHADOW_SSDT_HOOK pShadowSsdtHookInfo = NULL;
	BOOL bRet = FALSE;
	ULONG nTotalCount = 0;
	
	do 
	{
		ULONG nSize = sizeof(SHADOW_SSDT_HOOK) + nCnt * sizeof(SHADOW_SSDT_HOOK_INFO);

		if (pShadowSsdtHookInfo)
		{
			free(pShadowSsdtHookInfo);
			pShadowSsdtHookInfo = NULL;
		}

		pShadowSsdtHookInfo = (PSHADOW_SSDT_HOOK)malloc(nSize);

		if (pShadowSsdtHookInfo)
		{
			memset(pShadowSsdtHookInfo, 0, nSize);
			pShadowSsdtHookInfo->nCnt = nCnt;
			bRet = m_Driver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pShadowSsdtHookInfo, nSize, NULL);
		}

		nCnt = pShadowSsdtHookInfo->nRetCnt + 10;

	} while (pShadowSsdtHookInfo->nRetCnt > pShadowSsdtHookInfo->nCnt);

	if (bRet && pShadowSsdtHookInfo->nCnt >= pShadowSsdtHookInfo->nRetCnt)
	{
		for (ULONG i = 0; i < pShadowSsdtHookInfo->nRetCnt; i++)
		{
			//	InsertIntem(&pSsdtHookInfo->SsdtHook[i]);
			m_vectorHooks.push_back(pShadowSsdtHookInfo->ShadowSsdtHook[i]);
		}

		nTotalCount = pShadowSsdtHookInfo->nRetCnt;
	}

	if (pShadowSsdtHookInfo)
	{
		free(pShadowSsdtHookInfo);
		pShadowSsdtHookInfo = NULL;
	}

	// 枚举驱动
	m_clsDrivers.ListDrivers(m_vectorDrivers);

	for (vector<SHADOW_SSDT_HOOK_INFO>::iterator itor = m_vectorHooks.begin(); 
		itor != m_vectorHooks.end(); 
		itor++)
	{
		InsertIntem(*itor);
	}

	m_szStatus.Format(szShadowSSdtStatus[g_enumLang], nTotalCount, m_nHooked);
	UpdateData(FALSE);
}

void CShadowSsdtDlg::InsertIntem(SHADOW_SSDT_HOOK_INFO HookInfo)
{
	CString szIndex, szCurrentEntry, szOriginalEntry, szHookType, szPath;
	BOOL bHooked = FALSE;
	PWCHAR *szFunction = NULL;
	int n = 0;

	szIndex.Format(L"%d", HookInfo.nIndex);
	szOriginalEntry.Format(L"0x%08X", HookInfo.pOriginAddress);

	switch (g_WinVersion)
	{
	case enumWINDOWS_2K:
		szFunction = W2kShadow;
		break;

	case enumWINDOWS_XP:
		szFunction = XpShadow;
		break;

	case enumWINDOWS_2K3:
	case enumWINDOWS_2K3_SP1_SP2:
		szFunction = W2k3Shadow;
		break;

	case enumWINDOWS_VISTA:
	case enumWINDOWS_VISTA_SP1_SP2:
		szFunction = VistaShadow;
		break;

	case enumWINDOWS_7:
		szFunction = Win7Shadow;
		break;

	case enumWINDOWS_8:
		szFunction = Win8Shadow;
		break;
	}

	if (HookInfo.pNowAddress != HookInfo.pOriginAddress &&
		HookInfo.pInlineHookAddress)
	{
		CString szCurrentEntryTemp;
		CString szSSDTEntry, szInlineEntry;
		CString szSSDTPath, szInlinePath;

		szSSDTPath = GetDriverPath(HookInfo.pNowAddress);
		szSSDTEntry.Format(L"0x%08X", HookInfo.pNowAddress);

		if (HookInfo.pInlineHookAddress == 1)
		{
			szInlineEntry.Format(L"0x%08X", HookInfo.pNowAddress);
			szInlinePath = GetDriverPath(HookInfo.pNowAddress);
		}
		else
		{
			szInlineEntry.Format(L"0x%08X", HookInfo.pInlineHookAddress);
			szInlinePath = GetDriverPath(HookInfo.pInlineHookAddress);

			if (szInlinePath.IsEmpty())
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
						szInlinePath = szPathTemp;
						szInlineEntry += szCurrentEntryTemp;
						break;
					}

					nRet = m_Functions.GetInlineAddress(nRet);
				}
			}
		}
		
		if (szInlinePath.IsEmpty())
		{
			szInlinePath = szUnknowModule[g_enumLang];
		}

// 		if (szSSDTPath.IsEmpty())
// 		{
// 			szSSDTPath = szUnknowModule[g_enumLang];
// 		}

		if (szSSDTPath.IsEmpty())
		{
			ULONG nRet = m_Functions.GetInlineAddress(HookInfo.pNowAddress);
			CString szInlineHookTemp;
			BOOL bFind = FALSE;

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
					bFind = TRUE;
					break;
				}

				nRet = m_Functions.GetInlineAddress(nRet);
			}

			if (!bFind)
			{
				szSSDTPath = szUnknowModule[g_enumLang];
			}
		}

		szCurrentEntry.Format(L"(ssdt)%s, (inline)%s", szSSDTEntry, szInlineEntry);
		szPath.Format(L"(ssdt)%s, (inline)%s", szSSDTPath, szInlinePath);
		szHookType = L"ssdt & inline";
		bHooked = TRUE;
	}
	else if (HookInfo.pNowAddress != HookInfo.pOriginAddress)
	{
		szCurrentEntry.Format(L"0x%08X", HookInfo.pNowAddress);
		szPath = GetDriverPath(HookInfo.pNowAddress);
		szHookType = L"ssdt hook";

// 		if (szPath.IsEmpty())
// 		{
// 			szPath = szUnknowModule[g_enumLang];
// 		}

		if (szPath.IsEmpty())
		{
			ULONG nRet = m_Functions.GetInlineAddress(HookInfo.pNowAddress);
			CString szInlineHookTemp;
			BOOL bFind = FALSE;

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
					bFind = TRUE;
					break;
				}

				nRet = m_Functions.GetInlineAddress(nRet);
			}

			if (!bFind)
			{
				szPath = szUnknowModule[g_enumLang];
			}
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

		szHookType = L"inline hook";
		bHooked = TRUE;
	}
	else
	{
		szPath = GetDriverPath(HookInfo.pNowAddress);
		szCurrentEntry.Format(L"0x%08X", HookInfo.pNowAddress);
		szHookType = L"-";
		bHooked = FALSE;
	}

	if (m_bOnlyShowHooks)
	{
		if (bHooked)
		{
			n = m_list.InsertItem(m_list.GetItemCount(), szIndex);
			m_list.SetItemText(n, 1, szFunction[HookInfo.nIndex]);
			m_list.SetItemText(n, 2, szOriginalEntry);
			m_list.SetItemText(n, 3, szHookType);
			m_list.SetItemText(n, 4, szCurrentEntry);
			m_list.SetItemText(n, 5, szPath);
		}
	}
	else
	{
		n = m_list.InsertItem(m_list.GetItemCount(), szIndex);
		m_list.SetItemText(n, 1, szFunction[HookInfo.nIndex]);
		m_list.SetItemText(n, 2, szOriginalEntry);
		m_list.SetItemText(n, 3, szHookType);
		m_list.SetItemText(n, 4, szCurrentEntry);
		m_list.SetItemText(n, 5, szPath);
	}

	if (bHooked)
	{
		m_nHooked++;
		m_list.SetItemData(n, enumHooked);
	}
}

void CShadowSsdtDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CShadowSsdtDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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

		if (m_nHooked)
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

void CShadowSsdtDlg::OnSdtRefresh()
{
	GetShadowSsdtHooks();
}

void CShadowSsdtDlg::OnSdtOnlyShowHook()
{
	m_bOnlyShowHooks = !m_bOnlyShowHooks;
	GetShadowSsdtHooks();
}

void CShadowSsdtDlg::OnUpdateSdtOnlyShowHook(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bOnlyShowHooks);
}

void CShadowSsdtDlg::OnSdtDisCurrent()
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

void CShadowSsdtDlg::OnSdtDisOrigin()
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

void CShadowSsdtDlg::OnSdtRestore()
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

			COMMUNICATE_SSDT cs;
			cs.OpType = enumRestoreSdt;
			cs.op.Restore.bSsdt = 0;
			cs.op.Restore.nIndex = nIndex;
			m_Driver.CommunicateDriver(&cs, sizeof(COMMUNICATE_SSDT), NULL, 0, NULL);
		}
	}

	OnSdtRefresh();
}

void CShadowSsdtDlg::OnSdtRestoreAll()
{
	int nCnt = m_list.GetItemCount();

	for (int i = 0; i < nCnt; i++)
	{
		CString szHookType = m_list.GetItemText(i, 3);
		
		if (szHookType.CompareNoCase(L"-"))
		{
			CString szIndex = m_list.GetItemText(i, 0);
			ULONG nIndex = _wtoi(szIndex);
			COMMUNICATE_SSDT cs;

			cs.OpType = enumRestoreSdt;
			cs.op.Restore.bSsdt = 0;
			cs.op.Restore.nIndex = nIndex;
			m_Driver.CommunicateDriver(&cs, sizeof(COMMUNICATE_SSDT), NULL, 0, NULL);
		}
	}

	OnSdtRefresh();
}

void CShadowSsdtDlg::OnSdtShuxing()
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

void CShadowSsdtDlg::OnSdtLocationModule()
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

void CShadowSsdtDlg::OnSdtVerifySign()
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

void CShadowSsdtDlg::OnSdtText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CShadowSsdtDlg::OnSdtExcel()
{
	WCHAR szSSDT[] = {'S','h','a','d','o','w','S','S','D','T','\0'};
	m_Functions.ExportListToExcel(&m_list, szSSDT, m_szStatus);
}

void CShadowSsdtDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

BOOL CShadowSsdtDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CShadowSsdtDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Functions.JmpToFile(szPath);
	}
}
