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
// KernelEntryDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "KernelEntryDlg.h"


// CKernelEntryDlg 对话框

IMPLEMENT_DYNAMIC(CKernelEntryDlg, CDialog)

CKernelEntryDlg::CKernelEntryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKernelEntryDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_HookVector.clear();
	m_CommonDriverList.clear();
	m_nHookedCnt = 0;
	m_KiFastCallEntry = 0;
}

CKernelEntryDlg::~CKernelEntryDlg()
{
	m_HookVector.clear();
	m_CommonDriverList.clear();
}

void CKernelEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_KERNEL_ENTRY_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CKernelEntryDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CKernelEntryDlg::OnBnClickedOk)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CKernelEntryDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CKernelEntryDlg::OnNMCustomdrawList)
	ON_WM_SIZE()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_SDT_REFRESH, &CKernelEntryDlg::OnSdtRefresh)
	ON_COMMAND(ID_SDT_SHUXING, &CKernelEntryDlg::OnSdtShuxing)
	ON_COMMAND(ID_SDT_LOCATION_MODULE, &CKernelEntryDlg::OnSdtLocationModule)
	ON_COMMAND(ID_SDT_VERIFY_SIGN, &CKernelEntryDlg::OnSdtVerifySign)
	ON_COMMAND(ID_SDT_TEXT, &CKernelEntryDlg::OnSdtText)
	ON_COMMAND(ID_SDT_EXCEL, &CKernelEntryDlg::OnSdtExcel)
	ON_COMMAND(ID_SDT_DIS_CURRENT, &CKernelEntryDlg::OnSdtDisCurrent)
	ON_COMMAND(ID_SDT_DIS_ORIGIN, &CKernelEntryDlg::OnSdtDisOrigin)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CKernelEntryDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CKernelEntryDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_KERNEL_ENTRY_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CKernelEntryDlg 消息处理程序

void CKernelEntryDlg::OnBnClickedOk()
{
}

void CKernelEntryDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_SDT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_SDT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SDT_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_DIS_ORIGIN, szDisassemblerHookAddress[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SDT_DIS_CURRENT, szDisassemblerCurrentEntry[g_enumLang]);
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
		menu.SetMenuItemBitmaps(11, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 12; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 10; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);

			if (nItem != -1)
			{
				CString szModule = m_list.GetItemText(nItem, 2);

				if (szModule.IsEmpty() || 
					!szModule.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_SDT_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
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

void CKernelEntryDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CKernelEntryDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CKernelEntryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szHookAddress[g_enumLang], LVCFMT_LEFT, 250);
	m_list.InsertColumn(1, szCurrentEntry[g_enumLang], LVCFMT_LEFT, 250);
	m_list.InsertColumn(2, szModule[g_enumLang], LVCFMT_LEFT, 305);
	m_list.InsertColumn(3, szHookType[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(4, szCurrentValue[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(5, szOriginalValue[g_enumLang], LVCFMT_LEFT, 150);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CKernelEntryDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

void CKernelEntryDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

CString CKernelEntryDlg::GetDriverPath(ULONG pCallback)
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

void CKernelEntryDlg::GetKernelEntryHooks()
{
	OPERATE_TYPE opType = enumKernelEntryHook;
	BOOL bRet = FALSE;

	m_nHookedCnt = 0;
	m_list.DeleteAllItems();
	m_HookVector.clear();

	ULONG nSize = 100 * sizeof(KERNEL_ENTRY_HOOK_INFO);
	PKERNEL_ENTRY_HOOK_INFO pKernelEntryHookInfo = (PKERNEL_ENTRY_HOOK_INFO)malloc(nSize);

	if (pKernelEntryHookInfo)
	{
		memset(pKernelEntryHookInfo, 0, nSize);
		bRet = m_Driver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pKernelEntryHookInfo, nSize, NULL);
	}

	if (bRet)
	{
		GetDriver();
		
		InsetFirstKernelHookItem(pKernelEntryHookInfo[0]);

		for (ULONG i = 1; i < 100; i++)
		{
			if (!pKernelEntryHookInfo[i].OriginAddress)
			{
				break;
			}

			m_HookVector.push_back(pKernelEntryHookInfo[i]);
		}
	}

	if (pKernelEntryHookInfo)
	{
		free(pKernelEntryHookInfo);
		pKernelEntryHookInfo = NULL;
	}

	for (vector<KERNEL_ENTRY_HOOK_INFO>::iterator itor = m_HookVector.begin(); itor != m_HookVector.end(); itor++)
	{
		InsertKernelHookItem(*itor);
	}

	m_szStatus.Format(szKernelEntryStatus[g_enumLang], m_nHookedCnt);
	UpdateData(FALSE);
}

void CKernelEntryDlg::InsetFirstKernelHookItem(KERNEL_ENTRY_HOOK_INFO item)
{
	CString szHookAddress, szCurrentEntry, szHookType, szModule, szCurrentValue, szOriginalValue;
	BOOL bHooked = FALSE;

	m_KiFastCallEntry = item.OriginAddress;
	szHookAddress.Format(L"0x%08X", item.HookAddress);
	szCurrentEntry.Format(L"0x%08X", item.OriginAddress);
	szModule = GetDriverPath(item.HookAddress);

	if (item.HookAddress != item.OriginAddress)
	{
		bHooked = TRUE;
		szHookAddress.Format(L"0x%08X", item.OriginAddress);
		szCurrentEntry.Format(L"0x%08X", item.HookAddress);
		szModule = GetDriverPath(item.HookAddress);
		szHookType = L"entry hook";

		if (szModule.IsEmpty())
		{
			ULONG nRet = m_Functions.GetInlineAddress(item.HookAddress);
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
					szModule = szPathTemp;
					szCurrentEntry += szCurrentEntryTemp;
					break;
				}

				nRet = m_Functions.GetInlineAddress(nRet);
			}
		}
	}
	else
	{
		CString szTemp = L"KiFastCallEntry (";
		szTemp += szHookAddress;

		szHookAddress.Empty();
		szHookAddress = szTemp;
		szHookAddress += L")";

		szHookType = L"-";
	}

	if (szModule.IsEmpty())
	{
		szModule = szUnknowModule[g_enumLang];
	}

	int n = m_list.InsertItem(m_list.GetItemCount(), szHookAddress);
	m_list.SetItemText(n, 1, szCurrentEntry);
	m_list.SetItemText(n, 2, szModule);
	m_list.SetItemText(n, 3, szHookType);
	m_list.SetItemText(n, 4, L"-");
	m_list.SetItemText(n, 5, L"-");

	if (bHooked)
	{
		m_list.SetItemData(n, enumHooked);
		m_nHookedCnt++;
	}
}

void CKernelEntryDlg::InsertKernelHookItem(KERNEL_ENTRY_HOOK_INFO item)
{
	CString szHookAddress, szCurrentEntry, szHookType, szModule, szCurrentValue, szOriginalValue;

	szHookAddress.Format(L"KiFastCallEntry+%d (0x%08X) [len %d]", item.OriginAddress - m_KiFastCallEntry, item.OriginAddress, item.PatchLen);
	
	if (item.HookAddress)
	{
		szCurrentEntry.Format(L"0x%08X", item.HookAddress);
		szModule = GetDriverPath(item.HookAddress);

		if (szModule.IsEmpty())
		{
			ULONG nRet = m_Functions.GetInlineAddress(item.HookAddress);
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
					szModule = szPathTemp;
					szCurrentEntry += szCurrentEntryTemp;
					break;
				}

				nRet = m_Functions.GetInlineAddress(nRet);
			}
		}
	}
	else
	{
		szCurrentEntry = L"-";
		szModule = L"-";
	}
	
	if (szModule.IsEmpty())
	{
		szModule = szUnknowModule[g_enumLang];
	}

	for (ULONG i = 0; i < item.PatchLen; i++)
	{
		CString szCurrentValueTemp, szOriginalValueTemp;
		
		szCurrentValueTemp.Format(L"%02X ", item.NowBytes[i]);
		szCurrentValue += szCurrentValueTemp;

		szOriginalValueTemp.Format(L"%02X ", item.OriginBytes[i]);
		szOriginalValue += szOriginalValueTemp;
	}

	int n = m_list.InsertItem(m_list.GetItemCount(), szHookAddress);
	m_list.SetItemText(n, 1, szCurrentEntry);
	m_list.SetItemText(n, 2, szModule);
	m_list.SetItemText(n, 3, L"inline hook");
	m_list.SetItemText(n, 4, szCurrentValue);
	m_list.SetItemText(n, 5, szOriginalValue);
	m_list.SetItemData(n, enumHooked);

	m_nHookedCnt++;
}

void CKernelEntryDlg::OnSdtRefresh()
{
	GetKernelEntryHooks();
}

void CKernelEntryDlg::OnSdtShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CKernelEntryDlg::OnSdtLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.LocationExplorer(szPath);
	}
}

void CKernelEntryDlg::OnSdtVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.SignVerify(szPath);
	}
}

void CKernelEntryDlg::OnSdtText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CKernelEntryDlg::OnSdtExcel()
{
	WCHAR szKernelEntry[] = {'K','e','r','n','e','l','E','n','t','r','y','\0'};
	m_Functions.ExportListToExcel(&m_list, szKernelEntry, m_szStatus);
}

// Current Address
void CKernelEntryDlg::OnSdtDisCurrent()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 1);
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

// HookAddress
void CKernelEntryDlg::OnSdtDisOrigin()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 0);
		szAddress = szAddress.Right(szAddress.GetLength() - szAddress.Find('(') - 1);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

BOOL CKernelEntryDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CKernelEntryDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.JmpToFile(szPath);
	}
}
