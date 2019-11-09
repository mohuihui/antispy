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
// ModifyCodeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ModifyCodeDlg.h"
#include <shlwapi.h>
#include <algorithm>
#include "ShadowFunctions.h"

// CModifyCodeDlg 对话框

IMPLEMENT_DYNAMIC(CModifyCodeDlg, CDialog)

CModifyCodeDlg::CModifyCodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyCodeDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_PatchAddressList.clear();
	m_AllPatchList.clear();
	m_vectorDrivers.clear();
	m_nHookedCnt = 0;
}

CModifyCodeDlg::~CModifyCodeDlg()
{
	m_PatchAddressList.clear();
	m_vectorDrivers.clear();
	m_AllPatchList.clear();
}

void CModifyCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MODIFY_CODE_STATUS, m_szStatus);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CModifyCodeDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CModifyCodeDlg::OnNMRclickList)
	ON_COMMAND(ID_SDT_REFRESH, &CModifyCodeDlg::OnSdtRefresh)
	ON_COMMAND(ID_SDT_DIS_CURRENT, &CModifyCodeDlg::OnSdtDisCurrent)
	ON_COMMAND(ID_SDT_DIS_ORIGIN, &CModifyCodeDlg::OnSdtDisOrigin)
	ON_COMMAND(ID_SDT_RESTORE, &CModifyCodeDlg::OnSdtRestore)
	ON_COMMAND(ID_SDT_SHUXING, &CModifyCodeDlg::OnSdtShuxing)
	ON_COMMAND(ID_SDT_LOCATION_MODULE, &CModifyCodeDlg::OnSdtLocationModule)
	ON_COMMAND(ID_SDT_VERIFY_SIGN, &CModifyCodeDlg::OnSdtVerifySign)
	ON_COMMAND(ID_SDT_TEXT, &CModifyCodeDlg::OnSdtText)
	ON_COMMAND(ID_SDT_EXCEL, &CModifyCodeDlg::OnSdtExcel)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CModifyCodeDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CModifyCodeDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_MODIFY_CODE_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CModifyCodeDlg 消息处理程序

void CModifyCodeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CModifyCodeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szHookAddress[g_enumLang], LVCFMT_LEFT, 280);
	m_list.InsertColumn(1, szCurrentEntry[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(2, szModule[g_enumLang], LVCFMT_LEFT, 300);
	m_list.InsertColumn(3, szPatchLenInBytes[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(4, szCurrentValue[g_enumLang], LVCFMT_LEFT, 160);
	m_list.InsertColumn(5, szOriginalValue[g_enumLang], LVCFMT_LEFT, 160);
	
	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CModifyCodeDlg::InsertExportDirectoryItem( EXPORT_FUNCTION_PATCH_INFO ExportDirectoryHookInfo )
{
	CString szHookAddress, szCurrentEntry, szModule, szCurrentValue, szOriginalValue, szLen;

	if (ExportDirectoryHookInfo.PatchedAddress == 0)
	{
		return;
	}
	
	MODIFIED_CODE_INFO mci;
	mci.PatchedAddress = ExportDirectoryHookInfo.PatchedAddress;
	mci.PatchLen = ExportDirectoryHookInfo.PatchLen;
	memcpy(mci.OriginBytes, ExportDirectoryHookInfo.OriginBytes, MAX_PATCHED_LEN);
	m_AllPatchList.push_back(mci);

	// szHookAddress
	{
		szHookAddress.Format(/*L"%s+0x%X (0x%08X) [len %d]"*/  L"%s+0x%X (0x%08X)", 
			ExportDirectoryHookInfo.szFunction, 
			ExportDirectoryHookInfo.PatchedAddress - ExportDirectoryHookInfo.FunctionAddress, 
			ExportDirectoryHookInfo.PatchedAddress//,
			/*ExportDirectoryHookInfo.PatchLen*/);
	}

	// len
	{
		szLen.Format(L"%d", ExportDirectoryHookInfo.PatchLen);
	}

	// szCurrentEntry || szModule
	{
		if (ExportDirectoryHookInfo.HookAddress)
		{
			szCurrentEntry.Format(L"0x%08X", ExportDirectoryHookInfo.HookAddress);
			szModule = GetDriverPath(ExportDirectoryHookInfo.HookAddress);

			if (szModule.IsEmpty())
			{
				ULONG nRet = m_Functions.GetInlineAddress(ExportDirectoryHookInfo.HookAddress);
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
	}

	// szCurrentValue & szOriginalValue
	{
		for (ULONG j = 0; j < ExportDirectoryHookInfo.PatchLen; j++)
		{
			CString szCurrentValueTemp, szOriginalValueTemp;

			szCurrentValueTemp.Format(L"%02X ", ExportDirectoryHookInfo.NowBytes[j]);
			szCurrentValue += szCurrentValueTemp;

			szOriginalValueTemp.Format(L"%02X ", ExportDirectoryHookInfo.OriginBytes[j]);
			szOriginalValue += szOriginalValueTemp;
		}
	}

	// insert
	{
		int n = m_list.InsertItem(m_list.GetItemCount(), szHookAddress);
		m_list.SetItemText(n, 1, szCurrentEntry);
		m_list.SetItemText(n, 2, szModule);
		m_list.SetItemText(n, 3, szLen);
		m_list.SetItemText(n, 4, szCurrentValue);
		m_list.SetItemText(n, 5, szOriginalValue);
		m_list.SetItemData(n, ExportDirectoryHookInfo.PatchLen);
		m_nHookedCnt++;
	}
}

//
// 获得导出函数的Patch信息
//
void CModifyCodeDlg::GetExportFunctonsPatchs(ULONG Base, ULONG Size, WCHAR *szPath)
{
	m_vectorExportFuncHooks.clear();
	m_clsHooks.EnumExportFunctionsPatchs(Base, Size, szPath, m_vectorExportFuncHooks);

	for (vector<EXPORT_FUNCTION_PATCH_INFO>::iterator ir = m_vectorExportFuncHooks.begin();
		ir != m_vectorExportFuncHooks.end();
		ir++)
	{
		m_PatchAddressList.push_back(ir->PatchedAddress);
		InsertExportDirectoryItem(*ir);
	}
}

BOOL CModifyCodeDlg::IsAlreadyInList(ULONG PatchAddress)
{
	BOOL bRet = FALSE;

	if (!PatchAddress)
	{
		bRet = TRUE;
	}
	else
	{
		if (!m_PatchAddressList.empty())
		{
			list<ULONG>::iterator findItr;
			findItr = find(m_PatchAddressList.begin(), m_PatchAddressList.end(), PatchAddress);
			if (findItr != m_PatchAddressList.end())
			{
				bRet = TRUE;
			}
		}
	}

	return bRet;
}

//
// 获取SSDT函数的inline hook信息
//
void CModifyCodeDlg::GetSsdtInlineHookInfo()
{
	m_vectorSSDTHooks.clear();
	m_clsHooks.EnumSSDTInlineHooks(m_vectorSSDTHooks);

	for (vector<EXPORT_FUNCTION_PATCH_INFO>::iterator ir = m_vectorSSDTHooks.begin();
		ir != m_vectorSSDTHooks.end();
		ir++)
	{
		if (!IsAlreadyInList(ir->PatchedAddress))
		{
			m_PatchAddressList.push_back(ir->PatchedAddress);
			InsertExportDirectoryItem(*ir);
		}
	}
}

void CModifyCodeDlg::InsertShadowSsdtItem( SHADOW_INLINE_INFO ExportDirectoryHookInfo )
{
	if (ExportDirectoryHookInfo.PatchedAddress == 0)
	{
		return;
	}

	MODIFIED_CODE_INFO mci;
	mci.PatchedAddress = ExportDirectoryHookInfo.PatchedAddress;
	mci.PatchLen = ExportDirectoryHookInfo.PatchLen;
	memcpy(mci.OriginBytes, ExportDirectoryHookInfo.OriginBytes, MAX_PATCHED_LEN);
	m_AllPatchList.push_back(mci);

	PWCHAR *szFunction = NULL;

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

	if (szFunction)
	{
		CString szHookAddress, szCurrentEntry, szModule, szCurrentValue, szOriginalValue, szLen;

		// szHookAddress
		{
			szHookAddress.Format(L"%s+0x%X (0x%08X)", 
				szFunction[ExportDirectoryHookInfo.nIndex], 
				ExportDirectoryHookInfo.PatchedAddress - ExportDirectoryHookInfo.FunctionAddress, 
				ExportDirectoryHookInfo.PatchedAddress);
		}

		// len
		{
			szLen.Format(L"%d", ExportDirectoryHookInfo.PatchLen);
		}

		// szCurrentEntry || szModule
		{
			if (ExportDirectoryHookInfo.HookAddress)
			{
				szCurrentEntry.Format(L"0x%08X", ExportDirectoryHookInfo.HookAddress);
				szModule = GetDriverPath(ExportDirectoryHookInfo.HookAddress);

				if (szModule.IsEmpty())
				{
					ULONG nRet = m_Functions.GetInlineAddress(ExportDirectoryHookInfo.HookAddress);
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
		}

		// szCurrentValue & szOriginalValue
		{
			for (ULONG j = 0; j < ExportDirectoryHookInfo.PatchLen; j++)
			{
				CString szCurrentValueTemp, szOriginalValueTemp;

				szCurrentValueTemp.Format(L"%02X ", ExportDirectoryHookInfo.NowBytes[j]);
				szCurrentValue += szCurrentValueTemp;

				szOriginalValueTemp.Format(L"%02X ", ExportDirectoryHookInfo.OriginBytes[j]);
				szOriginalValue += szOriginalValueTemp;
			}
		}

		// insert
		{
			int n = m_list.InsertItem(m_list.GetItemCount(), szHookAddress);
			m_list.SetItemText(n, 1, szCurrentEntry);
			m_list.SetItemText(n, 2, szModule);
			m_list.SetItemText(n, 3, szLen);
			m_list.SetItemText(n, 4, szCurrentValue);
			m_list.SetItemText(n, 5, szOriginalValue);
			m_list.SetItemData(n, ExportDirectoryHookInfo.PatchLen);
			m_nHookedCnt++;
		}
	}
}

void CModifyCodeDlg::GetShadowSsdtInlineHookInfo()
{
	m_vectorShadowHooks.clear();
	m_clsHooks.EnumShadowInlineHooks(m_vectorShadowHooks);

	for (vector<SHADOW_INLINE_INFO>::iterator ir = m_vectorShadowHooks.begin();
		ir != m_vectorShadowHooks.end();
		ir++)
	{
		if (!IsAlreadyInList(ir->PatchedAddress))
		{
			m_PatchAddressList.push_back(ir->PatchedAddress);
			InsertShadowSsdtItem(*ir);
		}
	}
}

void CModifyCodeDlg::InsertAllOfModuleItem( MODULE_PATCH_INFO AllOfModuleHookInfo, CString szImage, ULONG Base )
{
	if ( Base && !szImage.IsEmpty() && AllOfModuleHookInfo.PatchedAddress)
	{
		MODIFIED_CODE_INFO mci;
		mci.PatchedAddress = AllOfModuleHookInfo.PatchedAddress;
		mci.PatchLen = AllOfModuleHookInfo.PatchLen;
		memcpy(mci.OriginBytes, AllOfModuleHookInfo.OriginBytes, MAX_PATCHED_LEN);
		m_AllPatchList.push_back(mci);

		CString szHookAddress, szCurrentEntry, szModule, szCurrentValue, szOriginalValue, szLen;

		// szHookAddress
		{
			szHookAddress.Format(L"%s+0x%X (0x%08X)", 
				szImage, 
				AllOfModuleHookInfo.PatchedAddress - Base, 
				AllOfModuleHookInfo.PatchedAddress);
		}

		// len
		{
			szLen.Format(L"%d", AllOfModuleHookInfo.PatchLen);
		}

		// szCurrentEntry || szModule
		{
			if (AllOfModuleHookInfo.HookAddress)
			{
				szCurrentEntry.Format(L"0x%08X", AllOfModuleHookInfo.HookAddress);
				szModule = GetDriverPath(AllOfModuleHookInfo.HookAddress);

				if (szModule.IsEmpty())
				{
					ULONG nRet = m_Functions.GetInlineAddress(AllOfModuleHookInfo.HookAddress);
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
		}

		// szCurrentValue & szOriginalValue
		{
			for (ULONG j = 0; j < AllOfModuleHookInfo.PatchLen; j++)
			{
				CString szCurrentValueTemp, szOriginalValueTemp;

				szCurrentValueTemp.Format(L"%02X ", AllOfModuleHookInfo.NowBytes[j]);
				szCurrentValue += szCurrentValueTemp;

				szOriginalValueTemp.Format(L"%02X ", AllOfModuleHookInfo.OriginBytes[j]);
				szOriginalValue += szOriginalValueTemp;
			}
		}

		// insert
		{
			int n = m_list.InsertItem(m_list.GetItemCount(), szHookAddress);
			m_list.SetItemText(n, 1, szCurrentEntry);
			m_list.SetItemText(n, 2, szModule);
			m_list.SetItemText(n, 3, szLen);
			m_list.SetItemText(n, 4, szCurrentValue);
			m_list.SetItemText(n, 5, szOriginalValue);
			m_list.SetItemData(n, AllOfModuleHookInfo.PatchLen);
			m_nHookedCnt++;
		}
	}
}

void CModifyCodeDlg::GetAllOfModuleInlineHookInfo(ULONG Base, ULONG Size, WCHAR *szPath)
{
	m_vectorModuleHooks.clear();
	m_clsHooks.EnumModuleCodeSectionPatchs(Base, Size, szPath, m_vectorModuleHooks);
	
	if (m_vectorModuleHooks.empty())
	{
		return;
	}

	CString strPath = szPath;
	CString strImage = strPath.Right(strPath.GetLength() - strPath.ReverseFind(L'\\') - 1);

	for (vector<MODULE_PATCH_INFO>::iterator ir = m_vectorModuleHooks.begin();
		ir != m_vectorModuleHooks.end();
		ir++)
	{
		if (!IsAlreadyInList(ir->PatchedAddress))
		{
			m_PatchAddressList.push_back(ir->PatchedAddress);
			InsertAllOfModuleItem(*ir, strImage, Base);
		}
	}
}

//
// 开始扫描每个内核模块的钩子
//
void CModifyCodeDlg::ScanModifiedCode()
{
	for (vector<DRIVER_INFO>::iterator itor = m_vectorDrivers.begin(); 
		itor != m_vectorDrivers.end(); 
		itor++)
	{
		// 更新下目前正在扫描的内核模块
		m_szStatus.Empty();
		m_szStatus.Format(L"[%s] %s", szScanning[g_enumLang], itor->szDriverPath);
		UpdateData(FALSE);

		CString szPath = itor->szDriverPath;

		// 如果文件存在,则枚举
		if (PathFileExists(szPath))
		{
			CString szDriverName = PathGetImageName(szPath);

			// 首先枚举导出表
			GetExportFunctonsPatchs(itor->nBase, itor->nSize, itor->szDriverPath);

			// 如果是ntoskrnl,则枚举ssdt
			if (!szDriverName.CompareNoCase(L"ntoskrnl.exe") ||
				!szDriverName.CompareNoCase(L"ntkrnlpa.exe") ||
				!szDriverName.CompareNoCase(L"ntkrnlmp.exe") ||
				!szDriverName.CompareNoCase(L"ntkrpamp.exe") )
			{
				GetSsdtInlineHookInfo();
			}

			// 如果是win32k,则枚举shadow表
			else if (!szDriverName.CompareNoCase(L"win32k.sys"))
			{
				GetShadowSsdtInlineHookInfo();
			}

			// 最后枚举整个文件
			GetAllOfModuleInlineHookInfo(itor->nBase, itor->nSize, itor->szDriverPath);
		}
	}
}

//
// 枚举内核patch
//
void CModifyCodeDlg::EnumModifiedCode()
{
	m_szStatus.Format(szPatchHooks[g_enumLang], 0);
	UpdateData(FALSE);

	// 清零
	m_list.DeleteAllItems();
	m_AllPatchList.clear();
	m_nHookedCnt = 0;
	m_vectorDrivers.clear();
	m_PatchAddressList.clear();

	// 枚举下驱动模块
	m_clsDrivers.ListDrivers(m_vectorDrivers);

	if (m_vectorDrivers.empty())
	{
		return;
	}
	
	// 枚举钩子
	ScanModifiedCode();
	
	// 跟新状态
	m_szStatus.Format(szPatchHooks[g_enumLang], m_nHookedCnt);
	UpdateData(FALSE);
}

CString CModifyCodeDlg::GetDriverPath(ULONG pCallback)
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

void CModifyCodeDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_SDT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_SDT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SDT_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_RESTORE, szRestore[g_enumLang]);
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
		menu.SetMenuItemBitmaps(ID_SDT_RESTORE, MF_BYCOMMAND, &m_bmRecover, &m_bmRecover);
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
		ULONG nSelCount = m_list.GetSelectedCount();
		if (nSelCount == 0)
		{
			for (int i = 2; i < 12; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (nSelCount == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);

			if (nItem != -1)
			{
				CString szModule = m_list.GetItemText(nItem, 2);

				if (szModule.IsEmpty() || 
					!szModule.CompareNoCase(L"-"))
				{
				//	menu.EnableMenuItem(ID_SDT_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_SDT_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_SDT_LOCATION_MODULE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_SDT_VERIFY_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_SDT_DIS_CURRENT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}
		else if (nSelCount >= 2)
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

void CModifyCodeDlg::OnSdtRefresh()
{
	EnumModifiedCode();
}

// current address
void CModifyCodeDlg::OnSdtDisCurrent()
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

// hook address
void CModifyCodeDlg::OnSdtDisOrigin()
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

void CModifyCodeDlg::OnSdtRestore()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();

	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);

		if (m_list.GetItemData(nItem) <= MAX_PATCHED_LEN)
		{
			CString szHookAddress = m_list.GetItemText(nItem, 0);
			CString szHookAddr = szHookAddress.Right(szHookAddress.GetLength() - szHookAddress.ReverseFind('(') - 1);
			ULONG HookAddress = m_Functions.HexStringToLong(szHookAddr);
			//MODIFIED_CODE_INFO ModifyCodeInfo;

			for ( list<MODIFIED_CODE_INFO>::iterator itor = m_AllPatchList.begin(); itor != m_AllPatchList.end(); itor++ )
			{
				if (itor->PatchedAddress == HookAddress)
				{
					COMMUNICATE_MODIFY_CODE cmc;
					
					memset(&cmc, 0, sizeof(COMMUNICATE_MODIFY_CODE));
					cmc.OpType = enumRestoreModifyCode;
					cmc.op.Restore.ModifyCodeInfo.PatchedAddress = HookAddress;
					cmc.op.Restore.ModifyCodeInfo.PatchLen = itor->PatchLen;
					memcpy(cmc.op.Restore.ModifyCodeInfo.OriginBytes, itor->OriginBytes, itor->PatchLen);
					m_Driver.CommunicateDriver(&cmc, sizeof(COMMUNICATE_MODIFY_CODE), NULL, 0, NULL);

					break;
				}
			}
		}

		m_list.DeleteItem(nItem);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

void CModifyCodeDlg::OnSdtShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CModifyCodeDlg::OnSdtLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.LocationExplorer(szPath);
	}
}

void CModifyCodeDlg::OnSdtVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.SignVerify(szPath);
	}
}

void CModifyCodeDlg::OnSdtText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CModifyCodeDlg::OnSdtExcel()
{
	WCHAR szModifyCode[] = {'M','o','d','i','f','y','C','o','d','e','\0'};
	m_Functions.ExportListToExcel(&m_list, szModifyCode, m_szStatus);
}

//
// 屏蔽下按键
//
BOOL CModifyCodeDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CModifyCodeDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.JmpToFile(szPath);
	}
}
