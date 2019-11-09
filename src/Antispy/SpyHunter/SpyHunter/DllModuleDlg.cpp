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
// DllModuleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "DllModuleDlg.h"
#include <algorithm>
#include <shlwapi.h>
#include "SignVerifyDlg.h"
#include <Psapi.h>
#include "ListModules.h"
#pragma comment(lib, "Psapi.lib")

using namespace std;

typedef enum _DLL_HEADER_INDEX_
{
	eDllName,
	eDllBase,
	eDllSize,
	eDllPath,
	eDllFileCompany,
	eDllFileDescription,
	eDllCreateTime,
};

// CDllModuleDlg 对话框

IMPLEMENT_DYNAMIC(CDllModuleDlg, CDialog)

CDllModuleDlg::CDllModuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDllModuleDlg::IDD, pParent)
{
	m_uPid = 0;
	m_pEprocess = 0;
	m_szImage = L"";
	m_bNotShowMicroModule = TRUE;
	m_nModuleCnt = 0;
	m_nNotMicro = 0;
	m_hDlgWnd = NULL;
}

CDllModuleDlg::~CDllModuleDlg()
{
	m_vectorPebModules.clear();
}

void CDllModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CDllModuleDlg, CDialog)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CDllModuleDlg::OnNMCustomdrawList)
	ON_WM_SIZE()
	ON_COMMAND(ID_DLLMODULE_REFRESH, &CDllModuleDlg::OnDllmoduleRefresh)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CDllModuleDlg::OnNMRclickList)
	ON_COMMAND(ID_NOT_SHOW_MICROMODULE, &CDllModuleDlg::OnNotShowMicromodule)
	ON_UPDATE_COMMAND_UI(ID_NOT_SHOW_MICROMODULE, &CDllModuleDlg::OnUpdateNotShowMicromodule)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_COPY_DLL_MEMORY, &CDllModuleDlg::OnCopyDllMemory)
	ON_COMMAND(ID_UNLOAD_MODULE, &CDllModuleDlg::OnUnloadModule)
	ON_COMMAND(ID_CHECK_SIGN, &CDllModuleDlg::OnCheckSign)
	ON_COMMAND(ID_CHECK_ALL_SIGN, &CDllModuleDlg::OnCheckAllSign)
	ON_COMMAND(ID_COPY_MODULE_NAME, &CDllModuleDlg::OnCopyModuleName)
	ON_COMMAND(ID_COPY_MODULE_PATH, &CDllModuleDlg::OnCopyModulePath)
	ON_COMMAND(ID_CHECK_MODULE_ATTRIBUTE, &CDllModuleDlg::OnCheckModuleAttribute)
	ON_COMMAND(ID_LOCATION_MODULE, &CDllModuleDlg::OnLocationModule)
	ON_COMMAND(ID_SEARCH_MODULE_GOOGL, &CDllModuleDlg::OnSearchModuleOnline)
	ON_MESSAGE(WM_VERIFY_SIGN_OVER, OnVerifyModulesEnd)
	ON_COMMAND(ID_EXCEPT_EXCEL, &CDllModuleDlg::OnExceptExcel)
	ON_COMMAND(ID_EXPORT_TEXT, &CDllModuleDlg::OnExportText)
	ON_COMMAND(ID_DELETE_MODULE, &CDllModuleDlg::OnDeleteModule)
	ON_COMMAND(ID_UNLOAD_AND_DELETE_MODULE, &CDllModuleDlg::OnUnloadAndDeleteModule)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CDllModuleDlg::OnModulesLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDllModuleDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CDllModuleDlg 消息处理程序

BOOL CDllModuleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
		
	m_ModuleImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ModuleImageList, LVSIL_SMALL);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(eDllName, szModuleName[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(eDllBase, szBase[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(eDllSize, szSize[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(eDllPath, szModulePath[g_enumLang], LVCFMT_LEFT, 340);
	m_list.InsertColumn(eDllFileCompany, szFileCorporation[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(eDllFileDescription, szFileDescriptionNew[g_enumLang], LVCFMT_LEFT, 180);

	m_hModuleIcon = LoadIcon(NULL, IDI_APPLICATION);
	
 	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL ModuleBaseSort( MODULE_INFO elem1, MODULE_INFO elem2 )
{
	return elem1.Base < elem2.Base;
}

//
// 根据base排序
//
VOID CDllModuleDlg::SortByBase()
{
	sort(m_vectorPebModules.begin( ), m_vectorPebModules.end( ), ModuleBaseSort);
}

//
// 模块列表颜色显示
//
void CDllModuleDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );
		COLORREF clrNewTextColor, clrNewBkColor;
		
		clrNewTextColor = g_MicrosoftItemClr;
		clrNewBkColor = RGB( 255, 255, 255 );	

		if (nItem != -1)
		{
			int nData = (int)m_list.GetItemData(nItem);
			ITEM_COLOR clrText = m_vectorModules.at(nData).clrItem;
			
		//	DebugLog(L"nItem: %d, nData: %d", nItem, nData);

			if (clrText == enumBlue)
			{
				clrNewTextColor = g_NormalItemClr;
			}
			else if (clrText == enumPurple)
			{
				clrNewTextColor = g_NotSignedItemClr;
			}
		}
		
		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

//
// 系统消息WM_SIZE处理函数
//
void CDllModuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

// 
// 右键菜单 - 刷新
//
void CDllModuleDlg::OnDllmoduleRefresh()
{
//	EnumModules(m_uPid, m_pEprocess);
	::SendMessage(m_hDlgWnd, WM_ENUM_MODULES, NULL, NULL);
}

//
// 右键菜单
//
void CDllModuleDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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
// 	menu.AppendMenu(MF_STRING, ID_DELETE_MODULE, szDeleteModuleFile[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_UNLOAD_AND_DELETE_MODULE, szUnloadAndDeleteModuleFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_DLL_MEMORY, szDumpMemory[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_MODULE_NAME, szCopyModuleName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_COPY_MODULE_PATH, szCopyModulePath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_MODULE_ATTRIBUTE, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SEARCH_MODULE_GOOGL, szSearchOnlineGoogle[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
 	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
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
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
// 		menu.SetMenuItemBitmaps(ID_DELETE_MODULE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_UNLOAD_MODULE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_UNLOAD_AND_DELETE_MODULE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_SEARCH_MODULE_GOOGL, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_CHECK_MODULE_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_COPY_MODULE_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_COPY_MODULE_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_CHECK_ALL_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(21, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	// 如果模块数为0，那么除了“刷新”，其他全部置灰
	if (!m_list.GetItemCount()) 
	{
		for (int i = 2; i < 22; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt != 1)
		{
			for (int i = 2; i < 20; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
			
			menu.EnableMenuItem(ID_CHECK_ALL_SIGN, MF_BYCOMMAND | MF_ENABLED);
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

//
// 添加新的项到list列表
//
void CDllModuleDlg::AddDllModuleItem(MODULE_INFO item)
{
	CString strBase, strSize, strCompany;
	CString strPath = TrimPath(item.Path);
	CString strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

	strBase.Format(L"0x%08X", item.Base);
	strSize.Format(L"0x%08X", item.Size);
	strCompany = m_Functions.GetFileCompanyName(strPath);

	SHFILEINFO sfi;
	DWORD_PTR nRet;

	ZeroMemory(&sfi,sizeof(SHFILEINFO));

	nRet = SHGetFileInfo(strPath, 
		FILE_ATTRIBUTE_NORMAL, 
		&sfi, 
		sizeof(SHFILEINFO), 
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
		);

	int nIcon = -1;
	if (nRet)
	{
		nIcon = m_ModuleImageList.Add ( sfi.hIcon );
		DestroyIcon(sfi.hIcon);
	}
	else
	{
		nIcon = m_ModuleImageList.Add ( m_hModuleIcon );
	}

	int i = m_list.GetItemCount();
	m_list.InsertItem(i, strImageName, nIcon);
	m_list.SetItemText(i, eDllBase, strBase);
	m_list.SetItemText(i, eDllSize, strSize);
	m_list.SetItemText(i, eDllPath, strPath);
	m_list.SetItemText(i, eDllFileCompany, strCompany);
	m_list.SetItemText(i, eDllFileDescription, m_Functions.GetFileDescription(strPath));

	MODULE_INFO_EX info;
	info.Base = item.Base;
	info.Size = item.Size;
	info.szPath = strPath;
	info.clrItem = enumBlack;

	if (!m_Functions.IsMicrosoftApp(strCompany))
	{
		info.clrItem = enumBlue;
		m_nNotMicro++;
	}
	
	m_list.SetItemData(i, i);
	m_vectorModules.push_back(info);
}

//
// 枚举所有的模块
//
void CDllModuleDlg::ListAllModules()
{
	for ( vector <MODULE_INFO>::iterator Iter = m_vectorPebModules.begin( ); Iter != m_vectorPebModules.end( ); Iter++ )
	{
		AddDllModuleItem(*Iter);
		m_nModuleCnt++;
	}
}

//
// 枚举非微软的模块
//
void CDllModuleDlg::ListNotMicrosoftModules()
{
	for ( vector <MODULE_INFO>::iterator Iter = m_vectorPebModules.begin( ); Iter != m_vectorPebModules.end( ); Iter++)
	{
		MODULE_INFO item = *Iter;

		if (!m_Functions.IsMicrosoftAppByPath(item.Path))
		{
			AddDllModuleItem(item);
		}

		m_nModuleCnt++;
	}
}

//
// 右键菜单 - 显示微软模块
//
void CDllModuleDlg::OnNotShowMicromodule()
{
	m_bNotShowMicroModule = !m_bNotShowMicroModule;
	OnDllmoduleRefresh();
}

//
// 右键菜单 - 显示微软模块选项
//
void CDllModuleDlg::OnUpdateNotShowMicromodule(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bNotShowMicroModule);
}

//
// 系统菜单
//
void CDllModuleDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

//
// 拷贝模块内存
//
void CDllModuleDlg::OnCopyDllMemory()
{
	BOOL bCopy = FALSE;
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}
	
	CString szImageName = m_list.GetItemText(nItem, eDllName);
	int nData = (int)m_list.GetItemData(nItem);
	MODULE_INFO_EX info = m_vectorModules.at(nData);
	m_clsModules.DumpModuleMemoryToFile(m_uPid, m_pEprocess, info.Base, info.Size, szImageName);
}

//
// 卸载模块
//
void CDllModuleDlg::OnUnloadModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1 )
	{
		return;
	}

	if (MessageBox(szSureToUnloadModule[g_enumLang], szUnloadModule[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	int nData = (int)m_list.GetItemData(nItem);
	MODULE_INFO_EX info = m_vectorModules.at(nData);
	if (m_clsModules.UnloadModule(m_uPid, m_pEprocess, info.Base))
	{
		m_list.DeleteItem(nItem);
	}
}

//
// 验证文件签名
//
void CDllModuleDlg::OnCheckSign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szPath = m_list.GetItemText(nItem, eDllPath);
	m_SignVerifyDlg.m_bSingle = TRUE;
	m_SignVerifyDlg.m_path = szPath;
	m_SignVerifyDlg.DoModal();
}

//
// 验证所有模块签名
//
void CDllModuleDlg::OnCheckAllSign()
{
	m_SignVerifyDlg.m_NotSignItemList.clear();
	m_SignVerifyDlg.m_bSingle = FALSE;
	m_SignVerifyDlg.m_nPathSubItem = eDllPath;
	m_SignVerifyDlg.m_pList = &m_list;
	m_SignVerifyDlg.m_hWinWnd = this->m_hWnd;
	m_SignVerifyDlg.DoModal();
}

//
// 拷贝模块名
//
void CDllModuleDlg::OnCopyModuleName()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szName = m_list.GetItemText(nItem, eDllName);
	m_Functions.SetStringToClipboard(szName);
}

//
// 拷贝模块路径
//
void CDllModuleDlg::OnCopyModulePath()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szPath = m_list.GetItemText(nItem, eDllPath);
	m_Functions.SetStringToClipboard(szPath);
}

//
// 查看模块属性
//
void CDllModuleDlg::OnCheckModuleAttribute()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szPath = m_list.GetItemText(nItem, eDllPath);
	m_Functions.OnCheckAttribute(szPath);
}

//
// 在Explorer中定位文件
//
void CDllModuleDlg::OnLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szPath = m_list.GetItemText(nItem, eDllPath);
	m_Functions.LocationExplorer(szPath);
}

//
// 在线搜索模块名
//
void CDllModuleDlg::OnSearchModuleOnline()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szImage = m_list.GetItemText(nItem, eDllName);
	m_Functions.SearchOnline(szImage);
}

//
// 在未签名模块的列表中搜索
//
BOOL CDllModuleDlg::IsNotSignItem(ULONG nItem)
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

//
// 验证全部签名结束后调用此函数
//
LRESULT CDllModuleDlg::OnVerifyModulesEnd(WPARAM wParam, LPARAM lParam)
{
	ULONG nCnt = m_list.GetItemCount();

	for (ULONG i = 0; i < nCnt; i++)
	{
		if (IsNotSignItem(i))
		{
			// 修改颜色
			int nData = (int)m_list.GetItemData(i);
			m_vectorModules.at(nData).clrItem = enumPurple;

			// 设置是否Vrified
			CString szComp = m_list.GetItemText(i, eDllFileCompany);
			CString szCompTemp = szNotVerified[g_enumLang] + szComp;
			m_list.SetItemText(i, eDllFileCompany, szCompTemp);
		}
		else
		{
			// 设置是否Vrified
			CString szComp = m_list.GetItemText(i, eDllFileCompany);
			CString szCompTemp = szVerified[g_enumLang] + szComp;
			m_list.SetItemText(i, eDllFileCompany, szCompTemp);
		}
	}


	return 0;
}

//
// 导出到excel
//
void CDllModuleDlg::OnExceptExcel()
{
 	WCHAR szModule[] = {'M','o','d','u','l','e','\0'};
	CString szStatus;
	szStatus.Format(szModuleInfoState[g_enumLang], m_nModuleCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;
 	m_Functions.ExportListToExcel(&m_list, szModule, szText);
}

//
// 导出到文本
//
void CDllModuleDlg::OnExportText()
{
	CString szStatus;
	szStatus.Format(szModuleInfoState[g_enumLang], m_nModuleCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;
	m_Functions.ExportListToTxt(&m_list, szText);
}

//
// 删除模块文件
//
void CDllModuleDlg::OnDeleteModule()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		BOOL bRet = m_Functions.KernelDeleteFile(m_list.GetItemText(nItem, eDllPath));
		if (bRet)
		{
			MessageBox(szDeleteFileSucess[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(szDeleteFileFailed[g_enumLang], szToolName, MB_OK | MB_ICONERROR);
		}
	}
}

///////////////////////////////////////////////////////////////////////

//
// 卸载并删除模块文件
//
void CDllModuleDlg::OnUnloadAndDeleteModule()
{
	if (MessageBox(szAreYouSureUnloadAndDeleteModule[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	// 删除模块
	BOOL bRet = FALSE;
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		bRet = m_Functions.KernelDeleteFile(m_list.GetItemText(nItem, eDllPath));
	}

	Sleep(300);

	// 卸载模块
	int nData = (int)m_list.GetItemData(nItem);
	MODULE_INFO_EX info = m_vectorModules.at(nData);
	if (m_clsModules.UnloadModule(m_uPid, m_pEprocess, info.Base))
	{
		m_list.DeleteItem(nItem);
	}

	if (bRet)
	{
		MessageBox(szDeleteFileSucess[g_enumLang], NULL, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(szDeleteFileFailed[g_enumLang], NULL, MB_OK | MB_ICONERROR);
	}
}

//
// 定位到AntiSpy文件管理器
//
void CDllModuleDlg::OnModulesLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, eDllPath);
		m_Functions.JmpToFile(szPath);
	}
}

void CDllModuleDlg::SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd)
{
	// 设定新的pid和peprocess
	m_uPid = nPid;
	m_pEprocess = pEprocess;
	m_szImage = szImage;
	m_hDlgWnd = hWnd;
}

//
// 枚举模块函数
//
void CDllModuleDlg::EnumModules(ULONG *nTotal, ULONG *nNotMicro)
{
	// 清理
	m_list.DeleteAllItems();
	m_nModuleCnt = m_nNotMicro = 0;
	m_vectorModules.clear();

	int  nImageCount = m_ModuleImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ModuleImageList.Remove(0);   
	}
	
	// 开始枚举
	m_clsModules.EnumModulesByPeb(m_uPid, m_pEprocess, m_vectorPebModules);
	if (m_vectorPebModules.empty())
	{
		return;
	}

//	SortByBase();

	if (!m_bNotShowMicroModule)
	{
		ListAllModules();
	}
	else
	{
		ListNotMicrosoftModules();
	}

	if (nTotal)
	{
		*nTotal = m_nModuleCnt;
	}

	if (nNotMicro)
	{
		*nNotMicro = m_nNotMicro;
	}
}

//
// 获取文件的创建时间
//
CString CDllModuleDlg::GetFileCreationTime(CString szPath)
{
	if (szPath.IsEmpty() || !PathFileExists(szPath))
	{
		return NULL;
	}

	CString StrCreateTime;
	CFileStatus Status;
	if(CFile::GetStatus(szPath, Status))
	{
		CTime ctTime = Status.m_ctime;

		StrCreateTime.Format(
			L"%04d/%02d/%02d  %02d:%02d:%02d", 
			ctTime.GetYear(),
			ctTime.GetMonth(),
			ctTime.GetDay(),
			ctTime.GetHour(),
			ctTime.GetMinute(),
			ctTime.GetSecond());
	} 

	return StrCreateTime;
}

//
// 屏蔽下按键
//
BOOL CDllModuleDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}