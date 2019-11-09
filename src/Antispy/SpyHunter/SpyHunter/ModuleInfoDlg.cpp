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
// ModuleInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ModuleInfoDlg.h"
#include "SpyHunter.h"
#include <shlwapi.h>
#include <algorithm>
#include <string>
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")

using namespace std;

// CModuleInfoDlg 对话框

IMPLEMENT_DYNAMIC(CModuleInfoDlg, CDialog)

CModuleInfoDlg::CModuleInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModuleInfoDlg::IDD, pParent)
	, m_szText(_T(""))
{
	m_uPid = 0;
	m_pEprocess = 0;
	m_bShowMicroModule = TRUE;
	m_nModuleCnt = 0;
	m_nHideCnt = 0;
	m_nNotMicro = 0;
	m_szProcessPath = L"";
}

CModuleInfoDlg::~CModuleInfoDlg()
{
	m_VadRootModuleVector.clear();
	m_PebModuleVector.clear();
}

void CModuleInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_MODULE_INFO_CNT, m_szText);
}


BEGIN_MESSAGE_MAP(CModuleInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CModuleInfoDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CModuleInfoDlg::OnNMRclickList)
	ON_COMMAND(ID_DLLMODULE_REFRESH, &CModuleInfoDlg::OnDllmoduleRefresh)
	ON_COMMAND(ID_NOT_SHOW_MICROMODULE, &CModuleInfoDlg::OnNotShowMicromodule)
	ON_UPDATE_COMMAND_UI(ID_NOT_SHOW_MICROMODULE, &CModuleInfoDlg::OnUpdateNotShowMicromodule)
	ON_COMMAND(ID_COPY_DLL_MEMORY, &CModuleInfoDlg::OnCopyDllMemory)
	ON_COMMAND(ID_UNLOAD_MODULE, &CModuleInfoDlg::OnUnloadModule)
	ON_COMMAND(ID_UNLOAD_ALL_MODULES, &CModuleInfoDlg::OnUnloadAllModules)
	ON_COMMAND(ID_DELETE_MODULE, &CModuleInfoDlg::OnDeleteModule)
	ON_COMMAND(ID_CHECK_SIGN, &CModuleInfoDlg::OnCheckSign)
	ON_COMMAND(ID_CHECK_ALL_SIGN, &CModuleInfoDlg::OnCheckAllSign)
	ON_COMMAND(ID_COPY_MODULE_NAME, &CModuleInfoDlg::OnCopyModuleName)
	ON_COMMAND(ID_COPY_MODULE_PATH, &CModuleInfoDlg::OnCopyModulePath)
	ON_COMMAND(ID_CHECK_MODULE_ATTRIBUTE, &CModuleInfoDlg::OnCheckModuleAttribute)
	ON_COMMAND(ID_LOCATION_MODULE, &CModuleInfoDlg::OnLocationModule)
	ON_COMMAND(ID_SEARCH_MODULE_GOOGL, &CModuleInfoDlg::OnSearchModuleGoogl)
	ON_COMMAND(ID_EXPORT_TEXT, &CModuleInfoDlg::OnExportText)
	ON_COMMAND(ID_EXCEPT_EXCEL, &CModuleInfoDlg::OnExceptExcel)
	ON_WM_INITMENUPOPUP()
	ON_MESSAGE(WM_VERIFY_SIGN_OVER, OnCloseWindow)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CModuleInfoDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_UNLOAD_AND_DELETE_MODULE, &CModuleInfoDlg::OnUnloadAndDeleteModule)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CModuleInfoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_MODULE_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CModuleInfoDlg 消息处理程序

void CModuleInfoDlg::OnBnClickedOk()
{
}

BOOL CModuleInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ModuleImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ModuleImageList, LVSIL_SMALL);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szModuleName[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(1, szBase[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(2, szSize[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(3, szModulePath[g_enumLang], LVCFMT_LEFT, 310);
	m_list.InsertColumn(4, szFileCorporation[g_enumLang], LVCFMT_LEFT, 140);

	m_hModuleIcon = reinterpret_cast<HICON>(::LoadImage ( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PROCESS_ICON),IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR ));

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CModuleInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CModuleInfoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_EXCEPT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_DLLMODULE_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_NOT_SHOW_MICROMODULE, szNotShowMicrosoftModules[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_UNLOAD_MODULE, szUnLoadModule[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_UNLOAD_ALL_MODULES, szUnLoadModuleInAllProcess[g_enumLang]);
// 	menu.AppendMenu(MF_SEPARATOR);
// 	menu.AppendMenu(MF_STRING, ID_COPY_DLL_MEMORY, szDumpMemory[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DELETE_MODULE, szDeleteModuleFile[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_UNLOAD_AND_DELETE_MODULE, szUnloadAndDeleteModuleFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_DLL_MEMORY, szDumpMemory[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_MODULE_NAME, szCopyModuleName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_COPY_MODULE_PATH, szCopyModulePath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_MODULE_ATTRIBUTE, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SEARCH_MODULE_GOOGL, szSearchOnlineGoogle[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_SIGN, szVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CHECK_ALL_SIGN, szVerifyAllModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_DLLMODULE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_LOCATION_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_DELETE_MODULE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_UNLOAD_AND_DELETE_MODULE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_SEARCH_MODULE_GOOGL, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_CHECK_MODULE_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_COPY_MODULE_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_COPY_MODULE_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_CHECK_ALL_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(22, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_nModuleCnt) // 如果m_nModuleCnt模块数为0，那么除了“刷新”，其他全部置灰
	{
		for (int i = 2; i < 23; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetItemCount()) // 如果m_list是空的,但是m_nModuleCnt模块数不为0,那么"不显示微软模块"菜单被选中,故显示之.
		{
			for (int i = 2; i < 23; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_NOT_SHOW_MICROMODULE, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{
			ULONG nCnt = 0;
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			while (pos)
			{
				int nItem = m_list.GetNextSelectedItem(pos);
				nCnt++;
			}

			if (nCnt != 1)
			{
				for (int i = 2; i < 20; i++)
				{
					menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
				}

				menu.EnableMenuItem(ID_NOT_SHOW_MICROMODULE, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_CHECK_ALL_SIGN, MF_BYCOMMAND | MF_ENABLED);
			}
		}
	}

	menu.EnableMenuItem(ID_NOT_SHOW_MICROMODULE, MF_BYCOMMAND | MF_ENABLED);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

BOOL ModuleBaseSortInfo( MODULE_INFO elem1, MODULE_INFO elem2 )
{
	return elem1.Base < elem2.Base;
}

VOID CModuleInfoDlg::SortByBase()
{
	sort(m_PebModuleVector.begin( ), m_PebModuleVector.end( ), ModuleBaseSortInfo);
	sort(m_VadRootModuleVector.begin( ), m_VadRootModuleVector.end( ), ModuleBaseSortInfo);
}

BOOL CModuleInfoDlg::ListProcessModules()
{
	m_szText.Format(szModuleInfoState[g_enumLang], 0, 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	GetDllModulesInfo();
	SortByBase();

	if (m_bShowMicroModule)
	{
		ListAllModules();
	}
	else
	{
		ListMicrosoftModules();
	}

	return TRUE;
}

void CModuleInfoDlg::GetDllModulesInfo()
{
// 	COMMUNICATE_PROCESS_MODULE cpm;
// 	ULONG nCnt = 200;
// 	PPROCESS_MODULE_INFO pModuleInfo = NULL;
// 	BOOL bRet = FALSE;
// 
// 	m_VadRootModuleVector.clear();
// 	m_PebModuleVector.clear();
// 
// 	cpm.OpType = enumListProcessModule;
// 	cpm.EnumType = enumByPeb;
// 	cpm.nPid = m_uPid;
// 	cpm.pEprocess = m_pEprocess;
// 
// 	do 
// 	{
// 		ULONG nSize = 0;
// 
// 		if (pModuleInfo)
// 		{
// 			free(pModuleInfo);
// 			pModuleInfo = NULL;
// 		}
// 
// 		nSize = sizeof(PROCESS_MODULE_INFO) + nCnt * sizeof(MODULE_INFO);
// 
// 		pModuleInfo = (PPROCESS_MODULE_INFO)malloc(nSize);
// 		if (!pModuleInfo)
// 		{
// 			break;
// 		}
// 
// 		memset(pModuleInfo, 0, nSize);
// 
// 		pModuleInfo->nCnt = nCnt;
// 		pModuleInfo->nRetCnt = 0;
// 
// 		bRet = m_Driver.CommunicateDriver(&cpm, sizeof(COMMUNICATE_PROCESS_MODULE), pModuleInfo, nSize, NULL);
// 		nCnt = pModuleInfo->nRetCnt + 10;
// 
// 	} while (pModuleInfo->nRetCnt > pModuleInfo->nCnt);
// 
// 	if (bRet && pModuleInfo)
// 	{
// 		for (ULONG i = 0; i < pModuleInfo->nRetCnt; i++)
// 		{
// 			m_PebModuleVector.push_back(pModuleInfo->Modules[i]);
// 		}
// 	}
// 
// 	if (pModuleInfo)
// 	{
// 		free(pModuleInfo);
// 		pModuleInfo = NULL;
// 	}
// 
// //	EnumDllByQueryMemory();
// 	ListModuleByLdrpHashTable();

// 	cpm.OpType = enumListProcessModule;
// 	cpm.EnumType = enumByVadRoot;
// 	cpm.nPid = m_uPid;
// 	cpm.pEprocess = m_pEprocess;
// 
// 	do 
// 	{
// 		ULONG nSize = 0;
// 
// 		if (pModuleInfo)
// 		{
// 			free(pModuleInfo);
// 			pModuleInfo = NULL;
// 		}
// 
// 		nSize = sizeof(PROCESS_MODULE_INFO) + nCnt * sizeof(MODULE_INFO);
// 
// 		pModuleInfo = (PPROCESS_MODULE_INFO)malloc(nSize);
// 		if (!pModuleInfo)
// 		{
// 			break;
// 		}
// 
// 		memset(pModuleInfo, 0, nSize);
// 
// 		pModuleInfo->nCnt = nCnt;
// 		pModuleInfo->nRetCnt = 0;
// 
// 		bRet = m_Driver.CommunicateDriver(&cpm, sizeof(COMMUNICATE_PROCESS_MODULE), pModuleInfo, nSize, NULL);
// 		nCnt = pModuleInfo->nRetCnt + 10;
// 
// 	} while (pModuleInfo->nRetCnt > pModuleInfo->nCnt);
// 
// 	if (bRet && pModuleInfo)
// 	{
// 		for (ULONG i = 0; i < pModuleInfo->nRetCnt; i++)
// 		{
// 			m_VadRootModuleVector.push_back(pModuleInfo->Modules[i]);
// 		}
// 	}
// 
// 	if (pModuleInfo)
// 	{
// 		free(pModuleInfo);
// 		pModuleInfo = NULL;
// 	}
}

VOID CModuleInfoDlg::ListAllModules()
{
// 	ULONG i = 0;
// 	
// 	m_nHideCnt = m_nModuleCnt = m_nNotMicro = 0;
// 	m_list.DeleteAllItems();
// 	int  nImageCount = m_ModuleImageList.GetImageCount();
// 	for(int j = 0; j < nImageCount; j++)   
// 	{   
// 		m_ModuleImageList.Remove(0);   
// 	}
// 
// 	// 如果两个链表都为空，那么返回false
// 	if (m_VadRootModuleVector.size() == 0 && m_PebModuleVector.size() == 0)
// 	{
// 		return;
// 	}
// 
// 	for ( vector <MODULE_INFO>::iterator Iter = m_VadRootModuleVector.begin( ); Iter != m_VadRootModuleVector.end( ); Iter++, i++ )
// 	{
// 		MODULE_INFO item = *Iter;
// 		CString strPath, strImageName, strBase, strSize, strCompany;
// 
// 		strPath = m_Functions.TrimPath(item.Path);
// 		strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
// 		strBase.Format(L"0x%08X", item.Base);
// 		strSize.Format(L"0x%08X", /*GetModuleSize(item.Base)*/item.Size);
// 		strCompany = m_Functions.GetFileCompanyName(strPath);
// 
// 		SHFILEINFO sfi;
// 		DWORD_PTR nRet;
// 		ZeroMemory(&sfi,sizeof(SHFILEINFO));
// 		nRet = SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
// 		nRet ? m_ModuleImageList.Add ( sfi.hIcon ) : m_ModuleImageList.Add ( m_hModuleIcon );
// 		if (nRet)
// 		{
// 			DestroyIcon(sfi.hIcon);
// 		}
// 
// 		m_list.InsertItem(i, strImageName, i);
// 		m_list.SetItemText(i, 1, strBase);
// 		m_list.SetItemText(i, 2, strSize);
// 		m_list.SetItemText(i, 3, strPath);
// 		m_list.SetItemText(i, 4, strCompany);
// 
// 		if (IsModuleHide(item))
// 		{
// 			m_nHideCnt++;
// 			m_list.SetItemData(i, enumDllHide);
// 		}
// 		else if (!m_Functions.IsMicrosoftApp(strCompany))
// 		{
// 			m_list.SetItemData(i, enumDllNotMicrosoft);
// 			m_nNotMicro++;
// 		}
// 		else
// 		{
// 			m_list.SetItemData(i, enumNone);
// 		}
// 
// 		m_nModuleCnt++;
// 	}
// 
// 	if (!m_VadRootModuleVector.size() && m_PebModuleVector.size())
// 	{
// 		for ( vector <MODULE_INFO>::iterator Iter = m_PebModuleVector.begin( ); Iter != m_PebModuleVector.end( ); Iter++, i++ )
// 		{
// 			MODULE_INFO item = *Iter;
// 			CString strPath, strImageName, strBase, strSize, strCompany;
// 
// 			strPath = m_Functions.TrimPath(item.Path);
// 			strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
// 			strBase.Format(L"0x%08X", item.Base);
// 			strSize.Format(L"0x%08X", item.Size);
// 			strCompany = m_Functions.GetFileCompanyName(strPath);
// 
// 			SHFILEINFO sfi;
// 			DWORD_PTR nRet;
// 			ZeroMemory(&sfi,sizeof(SHFILEINFO));
// 			nRet = SHGetFileInfo(strPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON);
// 			nRet ? m_ModuleImageList.Add ( sfi.hIcon ) : m_ModuleImageList.Add ( m_hModuleIcon );
// 
// 			m_list.InsertItem(i, strImageName, i);
// 			m_list.SetItemText(i, 1, strBase);
// 			m_list.SetItemText(i, 2, strSize);
// 			m_list.SetItemText(i, 3, strPath);
// 			m_list.SetItemText(i, 4, strCompany);
// 
//  			DestroyIcon(sfi.hIcon);
// 
// 			if (!m_Functions.IsMicrosoftApp(strCompany))
// 			{
// 				m_list.SetItemData(i, enumDllNotMicrosoft);
// 				m_nNotMicro++;
// 			}
// 			else
// 			{
// 				m_list.SetItemData(i, enumNone);
// 			}
// 
// 			m_nModuleCnt++;
// 		}
// 	}
// 
// 	FixProcessPath();
// 	m_szText.Format(szModuleInfoState[g_enumLang], m_nModuleCnt, m_nHideCnt);
// 	UpdateData(FALSE);
}

VOID CModuleInfoDlg::ListMicrosoftModules()
{
//	HICON hIcon = reinterpret_cast<HICON>(::LoadImage ( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PROCESS_ICON),IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR ));
// 	int  nImageCount = m_ModuleImageList.GetImageCount();
// 	for(int j = 0; j < nImageCount; j++)   
// 	{   
// 		m_ModuleImageList.Remove(0);   
// 	}
// 
// 	m_nHideCnt = m_nModuleCnt = m_nNotMicro = 0;
// 	m_list.DeleteAllItems();
// 
// 	// 如果两个链表都为空，那么返回false
// 	if (m_VadRootModuleVector.size() == 0 && m_PebModuleVector.size() == 0)
// 	{
// 		return;
// 	}
// 
// 	for ( vector <MODULE_INFO>::iterator Iter = m_VadRootModuleVector.begin( ); Iter != m_VadRootModuleVector.end( ); Iter++)
// 	{
// 		MODULE_INFO item = *Iter;
// 		CString strPath, strImageName, strBase, strSize, strCompany;
// 
// 		strPath = m_Functions.TrimPath(item.Path);
// 		strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
// 		strBase.Format(L"0x%08X", item.Base);
// 		strSize.Format(L"0x%08X", /*GetModuleSize(item.Base)*/item.Size);
// 		strCompany = m_Functions.GetFileCompanyName(strPath);
// 
// 		if (!m_Functions.IsMicrosoftApp(strCompany) || IsModuleHide(item))
// 		{
// 			SHFILEINFO sfi;
// 			DWORD_PTR nRet;
// 			ZeroMemory(&sfi,sizeof(SHFILEINFO));
// 			nRet = SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES); /*SHGetFileInfo(strPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON);*/
// 			nRet ? m_ModuleImageList.Add ( sfi.hIcon ) : m_ModuleImageList.Add ( m_hModuleIcon );
// 			if (nRet)
// 			{
// 				DestroyIcon(sfi.hIcon);
// 			}
// 
// 			int nCnt = m_list.GetItemCount();
// 			int n = m_list.InsertItem(nCnt, strImageName, nCnt);
// 			m_list.SetItemText(n, 1, strBase);
// 			m_list.SetItemText(n, 2, strSize);
// 			m_list.SetItemText(n, 3, strPath);
// 			m_list.SetItemText(n, 4, strCompany);
// 
// 			if (IsModuleHide(item))
// 			{
// 				m_nHideCnt++;
// 				m_list.SetItemData(n, enumDllHide);
// 			}
// 			else
// 			{
// 				m_list.SetItemData(n, enumDllNotMicrosoft);
// 			}
// 
// 			m_nNotMicro++;
// 		}
// 
// 		m_nModuleCnt++;
// 	}
// 
// 	if (!m_VadRootModuleVector.size() && m_PebModuleVector.size())
// 	{
// 		for ( vector <MODULE_INFO>::iterator Iter = m_PebModuleVector.begin( ); Iter != m_PebModuleVector.end( ); Iter++)
// 		{
// 			MODULE_INFO item = *Iter;
// 			CString strPath, strImageName, strBase, strSize, strCompany;
// 
// // 			strPath = GetModuleName(item);
// // 			if (strPath.IsEmpty())
// // 			{
// 				strPath = m_Functions.TrimPath(item.Path);
// //			}
// 
// 			strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
// 
// 			strBase.Format(L"0x%08X", item.Base);
// 			strSize.Format(L"0x%08X", item.Size);
// 			strCompany = m_Functions.GetFileCompanyName(strPath);
// 
// 			if (!m_Functions.IsMicrosoftApp(strCompany))
// 			{
// 				SHFILEINFO sfi;
// 				DWORD_PTR nRet;
// 				ZeroMemory(&sfi,sizeof(SHFILEINFO));
// 				nRet = SHGetFileInfo(strPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON);
// 				nRet ? m_ModuleImageList.Add ( sfi.hIcon ) : m_ModuleImageList.Add ( m_hModuleIcon );
// 
// 				int nCnt = m_list.GetItemCount();
// 				int n = m_list.InsertItem(nCnt, strImageName, nCnt);
// 				m_list.SetItemText(n, 1, strBase);
// 				m_list.SetItemText(n, 2, strSize);
// 				m_list.SetItemText(n, 3, strPath);
// 				m_list.SetItemText(n, 4, strCompany);
// 
// 				DestroyIcon(sfi.hIcon);
// 
// // 				if (IsModuleHide(item))
// // 				{
// // 					m_nHideCnt++;
// // 					m_list.SetItemData(n, enumDllHide);
// // 				}
// // 				else
// // 				{
// 					m_list.SetItemData(n, enumDllNotMicrosoft);
// //				}
// 
// 				m_nNotMicro++;
// 			}
// 
// 			m_nModuleCnt++;
// 		}
// 	}
// 
// 	FixProcessPath();
// 	m_szText.Format(szModuleInfoState[g_enumLang], m_nModuleCnt, m_nHideCnt);
// 	UpdateData(FALSE);
}

CString CModuleInfoDlg::GetModuleName(MODULE_INFO item)
{
	CString szPath;

	for ( vector <MODULE_INFO>::iterator Iter = m_VadRootModuleVector.begin( ); Iter != m_VadRootModuleVector.end( ); Iter++)
	{	
		MODULE_INFO entry = *Iter;

		if (item.Base == entry.Base)
		{
			if (wcschr(entry.Path, L'~'))
			{
				WCHAR szLongPath[MAX_PATH] = {0};
			//	GetLongPathName(entry.Path, szLongPath, MAX_PATH);
				DWORD nRet = GetLongPathName(entry.Path, szLongPath, MAX_PATH);
				if (nRet >= MAX_PATH || nRet == 0)
				{
					szPath = entry.Path;
				}
				else
				{
					szPath = szLongPath;
				}
			//	szPath = szLongPath;
			}
			else
			{
				szPath = entry.Path;
			}
		}
	}

	return szPath;
}

BOOL CModuleInfoDlg::IsModuleHide(MODULE_INFO item)
{
	BOOL bHide = FALSE;
	BOOL bFind = FALSE;

	for ( vector <MODULE_INFO>::iterator Iter = m_PebModuleVector.begin( ); Iter != m_PebModuleVector.end( ); Iter++ )
	{	
		MODULE_INFO entry = *Iter;

		if (item.Base == entry.Base)
		{
			CString szPath = m_Functions.TrimPath(item.Path);
			CString strPath = m_Functions.TrimPath(entry.Path);

			// 如果大小不一样或者路径不一样,则报告
			if ((item.Size != entry.Size) || szPath.CompareNoCase(strPath))
			{
				bHide = TRUE;
			}

			bFind = TRUE;
			break;
		}
	}

	if (!bHide)
	{
		bHide = !bFind;
	}

	return bHide;
}

void CModuleInfoDlg::OnDllmoduleRefresh()
{
	ListProcessModules();
}

void CModuleInfoDlg::OnNotShowMicromodule()
{
	m_bShowMicroModule = !m_bShowMicroModule;

	if (m_bShowMicroModule)
	{
		ListAllModules();
	}
	else
	{
		ListMicrosoftModules();
	}
}

void CModuleInfoDlg::OnUpdateNotShowMicromodule(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowMicroModule);
}

void CModuleInfoDlg::OnCopyDllMemory()
{
// 	BOOL bCopy = FALSE;
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	if (pos != NULL)
// 	{
// 		int nItem = m_list.GetNextSelectedItem(pos);
// 		CString szImageName = m_list.GetItemText(nItem, 0);
// 		WCHAR szFile[MAX_PATH] = {0};
// 		wcsncpy_s(szFile, MAX_PATH, szImageName.GetBuffer(), szImageName.GetLength());
// 
// 		CFileDialog fileDlg(
// 			FALSE, 
// 			0, 
// 			(LPWSTR)szFile, 
// 			0, 
// 			L"(*.exe; *.dll; *.sys; *.oct)|*.exe;*.dll;*.sys;*.oct|All Files (*.*)|*.*||",
// 			0
// 			);
// 
// 		if (IDOK == fileDlg.DoModal())
// 		{
// 			CString szFilePath = fileDlg.GetFileName();
// 
// 			if ( !PathFileExists(szFilePath) ||
// 				(PathFileExists(szFilePath) && MessageBox(szFileExist[g_enumLang], szDumpMemory[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES))
// 			{
// 				for ( vector <MODULE_INFO>::iterator Iter = m_PebModuleVector.begin( ); Iter != m_PebModuleVector.end( ); Iter++)
// 				{
// 					MODULE_INFO item = *Iter;
// 					CString szBase;
// 
// 					szBase.Format(L"0x%08X", item.Base);
// 					if (szBase == m_list.GetItemText(nItem, 1))
// 					{
// 						PVOID pBuffer = malloc(item.Size);
// 						if (!pBuffer)
// 						{
// 							break;
// 						}
// 
// 						COMMUNICATE_PROCESS_MODULE_DUMP cpmd;
// 
// 						cpmd.Base = item.Base;
// 						cpmd.Size = item.Size;
// 						cpmd.nPid = m_uPid;
// 						cpmd.pEprocess = m_pEprocess;
// 						cpmd.OpType = enumDumpDllModule;
// 
// 						BOOL bRet = m_Driver.CommunicateDriver(&cpmd, sizeof(COMMUNICATE_PROCESS_MODULE_DUMP), pBuffer, item.Size, NULL);
// 						if (bRet)
// 						{
// 							if (MessageBox(szFixDump[g_enumLang], szDumpMemory[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES)
// 							{
// 								PIMAGE_DOS_HEADER       PEDosHeader;
// 								PIMAGE_FILE_HEADER      PEFileHeader;
// 								PIMAGE_OPTIONAL_HEADER  PEOptionalHeader;
// 								PIMAGE_SECTION_HEADER   PESectionHeaders;
// 								PULONG                  PEMagic;
// 
// 								PEDosHeader      = (PIMAGE_DOS_HEADER) pBuffer;
// 								PEMagic          = (PULONG)                 ( (ULONG)pBuffer + PEDosHeader->e_lfanew );
// 								PEFileHeader     = (PIMAGE_FILE_HEADER)     ( (ULONG)pBuffer + PEDosHeader->e_lfanew + sizeof(ULONG) );
// 								PEOptionalHeader = (PIMAGE_OPTIONAL_HEADER) ( (ULONG)pBuffer + PEDosHeader->e_lfanew + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) );
// 								PESectionHeaders = (PIMAGE_SECTION_HEADER)  ( (ULONG)pBuffer + PEDosHeader->e_lfanew + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER));
// 
// 								if (PEFileHeader->NumberOfSections > 0)
// 								{
// 									ULONG SectionAlignment = PEOptionalHeader->SectionAlignment;
// 									ULONG VirtualAddress = PESectionHeaders->VirtualAddress;
// 
// 									for (ULONG i = 0; i < PEFileHeader->NumberOfSections; i++)
// 									{
// 										ULONG VirtualSize = PESectionHeaders[i].Misc.VirtualSize;
// 										ULONG DivVirtualSize = VirtualSize % SectionAlignment;
// 
// 										if ( DivVirtualSize )
// 										{
// 											VirtualSize = SectionAlignment + VirtualSize - DivVirtualSize;
// 										}
// 
// 										PESectionHeaders[i].SizeOfRawData = VirtualSize;
// 										PESectionHeaders[i].PointerToRawData = VirtualAddress;
// 
// 										VirtualAddress += VirtualSize;
// 									}
// 								}
// 							}
// 
// 							CFile file;
// 							TRY 
// 							{
// 								if (file.Open(szFilePath,  CFile::modeCreate | CFile::modeWrite))
// 								{
// 									file.Write(pBuffer, item.Size);
// 									file.Close();
// 									bCopy = TRUE;
// 								}
// 							}
// 							CATCH_ALL( e )
// 							{
// 								file.Abort();   // close file safely and quietly
// 								//THROW_LAST();
// 							}
// 							END_CATCH_ALL
// 						}
// 
// 						if (bCopy)
// 						{
// 							MessageBox(szDumpOK[g_enumLang], szDumpMemory[g_enumLang]);
// 						}
// 						else
// 						{
// 							MessageBox(szDumpFailed[g_enumLang], szDumpMemory[g_enumLang]);
// 						}
// 
// 						free(pBuffer);
// 						pBuffer = NULL;
// 						break;
// 					}
// 				}
// 			}
// 		}
// 	}
}

void CModuleInfoDlg::OnUnloadModule()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	if (pos != NULL)
// 	{
// 		int nItem = m_list.GetNextSelectedItem(pos);
// 
// 		for ( vector <MODULE_INFO>::iterator Iter = m_PebModuleVector.begin( ); Iter != m_PebModuleVector.end( ); Iter++)
// 		{
// 			MODULE_INFO item = *Iter;
// 			CString szBase;
// 
// 			szBase.Format(L"0x%08X", item.Base);
// 			if (szBase == m_list.GetItemText(nItem, 1))
// 			{
// 				COMMUNICATE_PROCESS_MODULE_UNLOAD cpmu;
// 
// 				cpmu.Base = item.Base;
// 				cpmu.nPid = m_uPid;
// 				cpmu.pEprocess = m_pEprocess;
// 				cpmu.OpType = enumUnloadDllModule;
// 
// 				BOOL bRet = m_Driver.CommunicateDriver(&cpmu, sizeof(COMMUNICATE_PROCESS_MODULE_UNLOAD), NULL, 0, NULL);
// 				if (bRet)
// 				{
// 					m_Functions.UnloadModuleInLdrpHashTable(m_uPid, m_pEprocess, item.Base);
// 					m_list.DeleteItem(nItem);
// 				}
// 
// 				break;
// 			}
// 		}
// 	}
}

void CModuleInfoDlg::OnUnloadAllModules()
{
	if (MessageBox(szGlobalUnloadModule[g_enumLang], szUnloadModule[g_enumLang], MB_YESNO | MB_ICONWARNING) == IDNO)
	{
		return;
	}

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szImageName = m_list.GetItemText(nItem, 0);

		if (!szImageName.IsEmpty())
		{
			vector<PROCESS_INFO> vectorProcess;
			CListProcess ListProc;
			ListProc.EnumProcess(vectorProcess);

			for ( vector <PROCESS_INFO>::iterator Iter = vectorProcess.begin( ); Iter != vectorProcess.end( ); Iter++ )
			{
				PROCESS_INFO ProcessItem = *Iter;

				if (ProcessItem.ulPid == 0 && ProcessItem.ulParentPid == 0 && ProcessItem.ulEprocess == 0)
				{
					continue;
				}

				vector<MODULE_INFO> vectorModules;
				CListModules clsListModules;
				clsListModules.EnumModulesByPeb(ProcessItem.ulPid, ProcessItem.ulEprocess, vectorModules);

				for ( vector <MODULE_INFO>::iterator DllModuleIter = vectorModules.begin( ); DllModuleIter != vectorModules.end( ); DllModuleIter++)
				{
					MODULE_INFO ModuleItem = *DllModuleIter;
					CString strPath, strImageName;

					strPath = ModuleItem.Path;
					strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

					if (!strImageName.CompareNoCase(szImageName))
					{
// 						COMMUNICATE_PROCESS_MODULE_UNLOAD cpmu;
// 						cpmu.Base = ModuleItem.Base;
// 						cpmu.nPid = ProcessItem.ulPid;
// 						cpmu.pEprocess = ProcessItem.ulEprocess;
// 						cpmu.OpType = enumUnloadDllModule;
// 						DllModuleDlg.UnloadModule(&cpmu);
// 						m_Functions.UnloadModuleInLdrpHashTable(ProcessItem.ulPid, ProcessItem.ulEprocess, ModuleItem.Base);
					}
				}
			}
		}
	}

	ListProcessModules();
}

void CModuleInfoDlg::OnDeleteModule()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szModuleInfo[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		BOOL bRet = m_Functions.KernelDeleteFile(m_list.GetItemText(nItem, 3));
		if (bRet)
		{
			MessageBox(szDeleteFileSucess[g_enumLang], NULL, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(szDeleteFileFailed[g_enumLang], NULL, MB_OK | MB_ICONERROR);
		}
	}
}

void CModuleInfoDlg::OnCheckSign()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 3);

		m_SignVerifyDlg.m_bSingle = TRUE;
		m_SignVerifyDlg.m_path = szPath;
		m_SignVerifyDlg.DoModal();
	}
}

void CModuleInfoDlg::OnCheckAllSign()
{
	m_SignVerifyDlg.m_NotSignItemList.clear();
	m_SignVerifyDlg.m_bSingle = FALSE;
	m_SignVerifyDlg.m_pList = &m_list;
	m_SignVerifyDlg.m_nPathSubItem = 3;
	m_SignVerifyDlg.m_hWinWnd = this->m_hWnd;
	m_SignVerifyDlg.DoModal();
}

void CModuleInfoDlg::OnCopyModuleName()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 0);
		m_Functions.SetStringToClipboard(szPath);
	}
}

void CModuleInfoDlg::OnCopyModulePath()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.SetStringToClipboard(szPath);
	}
}

void CModuleInfoDlg::OnCheckModuleAttribute()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CModuleInfoDlg::OnLocationModule()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.LocationExplorer(szPath);
	}
}

void CModuleInfoDlg::OnSearchModuleGoogl()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szImage = m_list.GetItemText(nItem, 0);
		m_Functions.SearchOnline(szImage);
	}
}

void CModuleInfoDlg::OnExportText()
{
	m_Functions.ExportListToTxt(&m_list, m_szText);
}

void CModuleInfoDlg::OnExceptExcel()
{
	WCHAR szModule[] = {'M','o','d','u','l','e','\0'};
	m_Functions.ExportListToExcel(&m_list, szModule, m_szText);
}

void CModuleInfoDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

void CModuleInfoDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

// 		if (m_list.GetItemData(nItem) == enumDllHide)
// 		{
// 			clrNewTextColor = g_HiddenOrHookItemClr;//RGB( 255, 0, 0 );
// 		}
// 		else if (m_list.GetItemData(nItem) == enumDllNotMicrosoft)
// 		{
// 			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
// 		}
// 		else if (m_list.GetItemData(nItem) == enumDllNotSign)
// 		{
// 			clrNewTextColor = g_NotSignedItemClr;//RGB( 180, 0, 255 );
// 		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

int CALLBACK ModuleInfoCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{ 
	return lParam1 < lParam2;
}

BOOL CModuleInfoDlg::IsNotSignItem(ULONG nItem)
{
	BOOL bRet = FALSE;

	if (!m_SignVerifyDlg.m_NotSignItemList.empty())
	{
		list<ULONG>::iterator findItr;
		findItr = find(m_SignVerifyDlg.m_NotSignItemList.begin(), m_SignVerifyDlg.m_NotSignItemList.end(), nItem);
		if (findItr != m_SignVerifyDlg.m_NotSignItemList.end())
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

VOID CModuleInfoDlg::SortItem()
{
// 	ULONG nCnt = m_list.GetItemCount();
// 
// 	for (ULONG i = 0; i < nCnt; i++)
// 	{
// 		if (IsNotSignItem(i))
// 		{
// 			m_list.SetItemData(i, enumDllNotSign);
// 
// 			// 设置是否Vrified
// 			CString szComp = m_list.GetItemText(i, 4);
// 			CString szCompTemp = szNotVerified[g_enumLang] + szComp;
// 			m_list.SetItemText(i, 4, szCompTemp);
// 		}
// 		else
// 		{
// 			// 设置是否Vrified
// 			CString szComp = m_list.GetItemText(i, 4);
// 			CString szCompTemp = szVerified[g_enumLang] + szComp;
// 			m_list.SetItemText(i, 4, szCompTemp);
// 		}
// 	}
// 
// 	m_list.SortItems(ModuleInfoCompareProc, NULL);
}

LRESULT CModuleInfoDlg::OnCloseWindow(WPARAM wParam, LPARAM lParam)
{
	SortItem();
	return 0;
}

///////////////////////////////////////////////////////////////////////

void CModuleInfoDlg::OnUnloadAndDeleteModule()
{
	if (MessageBox(szAreYouSureUnloadAndDeleteModule[g_enumLang], szModuleInfo[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	BOOL bRet = FALSE;
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		bRet = m_Functions.KernelDeleteFile(m_list.GetItemText(nItem, 3));
	}

	Sleep(1000);

	OnUnloadModule();

	if (bRet)
	{
		MessageBox(szDeleteFileSucess[g_enumLang], NULL, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(szDeleteFileFailed[g_enumLang], NULL, MB_OK | MB_ICONERROR);
	}
}

void CModuleInfoDlg::PrintDll(HANDLE hProcess, LIST_ENTRY *LdrpHashTable)
{
	if (!hProcess || !LdrpHashTable)
	{
		return;
	}

	LIST_ENTRY *LdrpHashTableTemp = NULL, *pListEntry = NULL, *pListHead = NULL;
	UNICODE_STRING *pDllFullPath = NULL;
	LDR_DATA_TABLE_ENTRY LdrDataEntry;
	DWORD size = sizeof(LIST_ENTRY) * 32;
	PVOID pTemp = malloc(size);
	if (!pTemp)
	{
		return;
	}

	memset(pTemp, 0, size);

	// 读Hash表
	if(!m_ProcessFunc.ReadProcessMemory(hProcess, LdrpHashTable, pTemp, size, &size))
		goto END;

	LdrpHashTableTemp = (LIST_ENTRY *)pTemp;
	for(int i = 0; i < 32; i++)
	{
		pListEntry = LdrpHashTableTemp + i;
		pListEntry = pListEntry->Flink;
		pListHead = LdrpHashTable + i;	//指向某hash节点头

		while(pListEntry != pListHead)
		{
			if(!m_ProcessFunc.ReadProcessMemory(hProcess, (BYTE *)pListEntry - 0x3c, &LdrDataEntry, sizeof(LdrDataEntry), &size))
				goto END;

// 			if (LdrDataEntry.SizeOfImage > 0)
// 			{
				pDllFullPath = &LdrDataEntry.FullDllName;
				WCHAR *pString = (WCHAR*)malloc(pDllFullPath->MaximumLength);
				if (!pString)
				{
					goto END;
				}

				memset(pString, 0, pDllFullPath->MaximumLength);
				if(!m_ProcessFunc.ReadProcessMemory(hProcess, pDllFullPath->Buffer, pString, pDllFullPath->Length, &size))
				{
					free(pString);
					pString = NULL;
					goto END;
				}

				MODULE_INFO info;
				info.Base = (ULONG)LdrDataEntry.DllBase;
				info.Size = LdrDataEntry.SizeOfImage;
				CString szPath = m_Functions.TrimPath(pString);
				wcsncpy_s(info.Path, MAX_PATH, szPath, szPath.GetLength());
				m_VadRootModuleVector.push_back(info);

				DebugLog(L"base: 0x%08X, size: 0x%X, %s", LdrDataEntry.DllBase, LdrDataEntry.SizeOfImage, pString);

				free(pString);
				pString = NULL;
/*			}*/

			pListEntry = LdrDataEntry.HashLinks.Flink; // 通过节点的hashlinks来递进
		}
	}

END:
	if(!LdrpHashTableTemp)
		free(LdrpHashTableTemp);
}

int CModuleInfoDlg::ListModuleByLdrpHashTable()
{
	if(!g_pLdrpHashTable)
	{
		DebugLog(L"找不到Hash表");
		return 0;
	}

	HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_READ, FALSE, m_uPid);
	if(hProcess == NULL)
	{
		DebugLog(L"打开进程出错");
		return 0;
	}

	PrintDll(hProcess, g_pLdrpHashTable);
	CloseHandle(hProcess);

	return 0;
}