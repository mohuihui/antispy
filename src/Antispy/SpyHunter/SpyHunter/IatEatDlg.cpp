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
// IatEatDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "IatEatDlg.h"
#include <shlwapi.h>

// CIatEatDlg 对话框

IMPLEMENT_DYNAMIC(CIatEatDlg, CDialog)

CIatEatDlg::CIatEatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIatEatDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_vectorDrivers.clear();
	m_nHookCnt = 0;
}

CIatEatDlg::~CIatEatDlg()
{
	m_vectorDrivers.clear();
}

void CIatEatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_IAT_EAT_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CIatEatDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_SDT_REFRESH, &CIatEatDlg::OnSdtRefresh)
	ON_COMMAND(ID_SDT_DIS_CURRENT, &CIatEatDlg::OnSdtDisCurrent)
	ON_COMMAND(ID_SDT_DIS_ORIGIN, &CIatEatDlg::OnSdtDisOrigin)
	ON_COMMAND(ID_SDT_RESTORE, &CIatEatDlg::OnSdtRestore)
	ON_COMMAND(ID_SDT_RESTORE_ALL, &CIatEatDlg::OnSdtRestoreAll)
	ON_COMMAND(ID_SDT_SHUXING, &CIatEatDlg::OnSdtShuxing)
	ON_COMMAND(ID_SDT_LOCATION_MODULE, &CIatEatDlg::OnSdtLocationModule)
	ON_COMMAND(ID_SDT_VERIFY_SIGN, &CIatEatDlg::OnSdtVerifySign)
	ON_COMMAND(ID_SDT_TEXT, &CIatEatDlg::OnSdtText)
	ON_COMMAND(ID_SDT_EXCEL, &CIatEatDlg::OnSdtExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CIatEatDlg::OnNMRclickList)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CIatEatDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CIatEatDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_IAT_EAT_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CIatEatDlg 消息处理程序

void CIatEatDlg::InsertListItem(IAT_EAT_HOOK_INFO pIatEatHookInfo)
{
	CString szFunction, szHookAddress, szOriginalEntry, szHookType, szCurrentEntry, szModule;
	
	// szFunction
	{
		if (pIatEatHookInfo.HookType == enumIat)
		{
			CString szOriginAddressPath = GetDriverPath(pIatEatHookInfo.OriginAddress);
			CString szOriginAddressModue;

			if (szOriginAddressPath.IsEmpty())
			{
				szOriginAddressModue = szUnknowModule[g_enumLang];
			}
			else 
			{
				szOriginAddressModue = szOriginAddressPath.Right(szOriginAddressPath.GetLength() - szOriginAddressPath.ReverseFind('\\') - 1);
			}

			szFunction.Format(L"%s (%s)", pIatEatHookInfo.szFunction, szOriginAddressModue);
		}
		else if (pIatEatHookInfo.HookType == enumEat)
		{
			szFunction = pIatEatHookInfo.szFunction;
		}
	}
	
	// szHookAddress
	{
		CString szHookAddressPath = GetDriverPath(pIatEatHookInfo.HookAddress);
		CString szHookAddressModule;

		if (szHookAddressPath.IsEmpty())
		{
			szHookAddressModule = szUnknowModule[g_enumLang];
		}
		else 
		{
			szHookAddressModule = szHookAddressPath.Right(szHookAddressPath.GetLength() - szHookAddressPath.ReverseFind('\\') - 1);
		}

		szHookAddress.Format(L"0x%08X (%s)", pIatEatHookInfo.HookAddress, szHookAddressModule);
	}

	// szOriginalEntry & szCurrentEntry & szHookType
	{
		szOriginalEntry.Format(L"0x%08X", pIatEatHookInfo.OriginAddress);
		szCurrentEntry.Format(L"0x%08X", pIatEatHookInfo.NowAddress);

		if (pIatEatHookInfo.HookType == enumIat)
		{
			szHookType = L"iat hook";
		}
		else if (pIatEatHookInfo.HookType == enumEat)
		{
			szHookType = L"eat hook";
		}
	}

	// szModule
	{
		szModule = GetDriverPath(pIatEatHookInfo.NowAddress);
		if (szModule.IsEmpty())
		{
			ULONG nRet = m_Functions.GetInlineAddress(pIatEatHookInfo.NowAddress);
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

	// insert
	{
		int n = m_list.InsertItem(m_list.GetItemCount(), szFunction);
		m_list.SetItemText(n, 1, szHookAddress);
		m_list.SetItemText(n, 2, szOriginalEntry);
		m_list.SetItemText(n, 3, szHookType);
		m_list.SetItemText(n, 4, szCurrentEntry);
		m_list.SetItemText(n, 5, szModule);
		m_nHookCnt++;
	}
}

void CIatEatDlg::EnumIatEatProc()
{
	for (vector<DRIVER_INFO>::iterator itor = m_vectorDrivers.begin(); 
		itor != m_vectorDrivers.end(); 
		itor++)
	{
		m_szStatus.Empty();
		m_szStatus.Format(L"[%s] %s", szScanning[g_enumLang], itor->szDriverPath);
		UpdateData(FALSE);
		
		// 微软模块才枚举IAT\EAT钩子
		if (PathFileExists(itor->szDriverPath) && 
			m_Functions.IsMicrosoftAppByPath(itor->szDriverPath))
		{
			m_clsHooks.EnumIatEatHooks(itor->nBase,
				itor->nSize,
				itor->szDriverPath,
				m_IatEatHooks);

			for (vector<IAT_EAT_HOOK_INFO>::iterator itorHook = m_IatEatHooks.begin(); 
				itorHook != m_IatEatHooks.end(); 
				itorHook++)
			{
				InsertListItem(*itorHook);
			}
		}
	}
}

//
// 枚举IAT EAT HOOKS
//
void CIatEatDlg::GetIatEatHook()
{
	// 清零
	m_szStatus.Empty();
	m_szStatus.Format(szIatEatStatus[g_enumLang], 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_nHookCnt = 0;
	m_vectorDrivers.clear();
	m_IatEatHooks.clear();

	// 枚举驱动
	m_clsDrivers.ListDrivers(m_vectorDrivers);
	if (m_vectorDrivers.empty())
	{
		return;
	}
	
	// 真正的枚举函数
	EnumIatEatProc();

	m_szStatus.Empty();
	m_szStatus.Format(szIatEatStatus[g_enumLang], m_nHookCnt);
	UpdateData(FALSE);
}

CString CIatEatDlg::GetDriverPath(ULONG pCallback)
{
	CString szPath = L"";

	for (vector<DRIVER_INFO>::iterator itor = m_vectorDrivers.begin(); 
		itor != m_vectorDrivers.end(); 
		itor++)
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

void CIatEatDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CIatEatDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

BOOL CIatEatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szFunction[g_enumLang], LVCFMT_LEFT, 250);
	m_list.InsertColumn(1, szHookAddress[g_enumLang], LVCFMT_LEFT, 170);
	m_list.InsertColumn(2, szOriginalEntry[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(3, szHookType[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(4, szCurrentEntry[g_enumLang], LVCFMT_LEFT, 245);
	m_list.InsertColumn(5, szModule[g_enumLang], LVCFMT_LEFT, 300);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CIatEatDlg::OnSdtRefresh()
{
	GetIatEatHook();
}

void CIatEatDlg::OnSdtDisCurrent()
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

void CIatEatDlg::OnSdtDisOrigin()
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

ULONG CIatEatDlg::GetDriverBase(ULONG pCallback)
{
	ULONG Base = 0;

	for (vector<DRIVER_INFO>::iterator itor = m_vectorDrivers.begin(); itor != m_vectorDrivers.end(); itor++)
	{
		ULONG nBase = itor->nBase;
		ULONG nEnd = itor->nBase + itor->nSize;

		if (pCallback >= nBase && pCallback <= nEnd)
		{
			Base = nBase;
			break;
		}
	}

	return Base;
}


void CIatEatDlg::OnSdtRestore()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();

	while (pos)
	{
		int nItem  = m_list.GetNextSelectedItem(pos);
		CString szHookAddress = m_list.GetItemText(nItem, 1);
		CString szOriginDispatch = m_list.GetItemText(nItem, 2);
		CString szHookAddr = szHookAddress.Left(szHookAddress.GetLength() - szHookAddress.ReverseFind('('));
		ULONG HookAddress = m_Functions.HexStringToLong(szHookAddr);
		CString szHookType = m_list.GetItemText(nItem, 3);
		ULONG OriginDispatch = 0;
		
		if (!szHookType.CompareNoCase(L"eat hook"))
		{
			ULONG Base = GetDriverBase(HookAddress);
			ULONG Origin = m_Functions.HexStringToLong(szOriginDispatch);

			if (Base && Origin)
			{
				OriginDispatch = Origin - Base;
			}
		}
		else
		{
			OriginDispatch = m_Functions.HexStringToLong(szOriginDispatch);
		}

		if (OriginDispatch && HookAddress)
		{
			COMMUNICATE_IAT_EAT_HOOK cieh;
			memset(&cieh, 0, sizeof(COMMUNICATE_IAT_EAT_HOOK));
			cieh.OpType = enumRestoreIatEatHook;
			cieh.op.Restore.HookAddress = HookAddress;
			cieh.op.Restore.OriginDispatch = OriginDispatch;
			
			m_Driver.CommunicateDriver(&cieh, sizeof(COMMUNICATE_IAT_EAT_HOOK), NULL, 0, NULL);
		}
	}

	OnSdtRefresh();
}

void CIatEatDlg::OnSdtRestoreAll()
{
	ULONG nCnt = m_list.GetItemCount();

	for (ULONG nItem = 0; nItem < nCnt; nItem++)
	{
		CString szHookAddress = m_list.GetItemText(nItem, 1);
		CString szOriginDispatch = m_list.GetItemText(nItem, 2);
		CString szHookAddr = szHookAddress.Left(szHookAddress.GetLength() - szHookAddress.ReverseFind('('));
		ULONG HookAddress = m_Functions.HexStringToLong(szHookAddr);
		CString szHookType = m_list.GetItemText(nItem, 3);
		ULONG OriginDispatch = 0;

		if (!szHookType.CompareNoCase(L"eat hook"))
		{
			ULONG Base = GetDriverBase(HookAddress);
			ULONG Origin = m_Functions.HexStringToLong(szOriginDispatch);

			if (Base && Origin)
			{
				OriginDispatch = Origin - Base;
			}
		}
		else
		{
			OriginDispatch = m_Functions.HexStringToLong(szOriginDispatch);
		}

		if (OriginDispatch && HookAddress)
		{
			COMMUNICATE_IAT_EAT_HOOK cieh;
			memset(&cieh, 0, sizeof(COMMUNICATE_IAT_EAT_HOOK));
			cieh.OpType = enumRestoreIatEatHook;
			cieh.op.Restore.HookAddress = HookAddress;
			cieh.op.Restore.OriginDispatch = OriginDispatch;

			m_Driver.CommunicateDriver(&cieh, sizeof(COMMUNICATE_IAT_EAT_HOOK), NULL, 0, NULL);
		}
	}

	OnSdtRefresh();
}

void CIatEatDlg::OnSdtShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CIatEatDlg::OnSdtLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Functions.LocationExplorer(szPath);
	}
}

void CIatEatDlg::OnSdtVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Functions.SignVerify(szPath);
	}
}

void CIatEatDlg::OnSdtText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CIatEatDlg::OnSdtExcel()
{
	WCHAR szIatEat[] = {'I','a','t','E','a','t','\0'};
	m_Functions.ExportListToExcel(&m_list, szIatEat, m_szStatus);
}

void CIatEatDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_SDT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_SDT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SDT_REFRESH, szRefresh[g_enumLang]);
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
		menu.SetMenuItemBitmaps(14, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 15; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 13; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			if (m_nHookCnt)
			{
				menu.EnableMenuItem(ID_SDT_RESTORE_ALL, MF_BYCOMMAND | MF_ENABLED);
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
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

//
// 屏蔽按键
//
BOOL CIatEatDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CIatEatDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Functions.JmpToFile(szPath);
	}
}
