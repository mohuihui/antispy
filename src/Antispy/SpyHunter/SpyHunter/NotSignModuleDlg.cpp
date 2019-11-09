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
// NotSignModuleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "NotSignModuleDlg.h"
#include "DllModuleDlg.h"
#include <winsvc.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <Mscat.h>
#include <shlwapi.h>
#include "ListModules.h"

#define WM_UPDATE_NOSIGN_DLG     WM_USER + 7

// CNotSignModuleDlg 对话框

IMPLEMENT_DYNAMIC(CNotSignModuleDlg, CDialog)

CNotSignModuleDlg::CNotSignModuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNotSignModuleDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_hThread = NULL;
	m_hIcon = NULL;
	m_bExit = FALSE;
}

CNotSignModuleDlg::~CNotSignModuleDlg()
{
// 	m_bExit = TRUE;
// 	if (m_hThread)
// 	{
// // 		if (WaitForSingleObject(m_hThread, 1) != WAIT_OBJECT_0)
// // 		{
// // 			TerminateThread(m_hThread, 0);
// // 		}
// 		WaitForSingleObject(m_hThread, INFINITE);
// 		CloseHandle(m_hThread);
// 		m_hThread = NULL;
// 	}
}

void CNotSignModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CNotSignModuleDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CNotSignModuleDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_COMMAND(ID_NOSIGNMODULE_REFRESH, &CNotSignModuleDlg::OnNosignmoduleRefresh)
	ON_COMMAND(ID_NOSIGNMODULE_DELETE_MODULE, &CNotSignModuleDlg::OnNosignmoduleDeleteModule)
	ON_COMMAND(ID_NOSIGNMODULE_UNLOAD, &CNotSignModuleDlg::OnNosignmoduleUnload)
	ON_COMMAND(ID_NOSIGNMODULE_UNLOAD_ALL, &CNotSignModuleDlg::OnNosignmoduleUnloadAll)
	ON_COMMAND(ID_NOSIGNMODULE_COPY_MODULE_NAME, &CNotSignModuleDlg::OnNosignmoduleCopyModuleName)
	ON_COMMAND(ID_NOSIGNMODULE_COPY_MODULE_PATH, &CNotSignModuleDlg::OnNosignmoduleCopyModulePath)
	ON_COMMAND(ID_NOSIGNMODULE_CHECK_SHUXING, &CNotSignModuleDlg::OnNosignmoduleCheckShuxing)
	ON_COMMAND(ID_NOSIGNMODULE_IN_EXPLORER, &CNotSignModuleDlg::OnNosignmoduleInExplorer)
	ON_COMMAND(ID_NOSIGNMODULE_TEXT, &CNotSignModuleDlg::OnNosignmoduleText)
	ON_COMMAND(ID_NOSIGNMODULE_EXCEL, &CNotSignModuleDlg::OnNosignmoduleExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CNotSignModuleDlg::OnNMRclickList)
	ON_MESSAGE(WM_UPDATE_NOSIGN_DLG, OnUpdateData)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CNotSignModuleDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CNotSignModuleDlg 消息处理程序

void CNotSignModuleDlg::OnBnClickedOk()
{
}

BOOL CNotSignModuleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetWindowText(szFindNotSignatureModule[g_enumLang]);

	m_ModuleImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ModuleImageList, LVSIL_SMALL);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szModuleName[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(1, szModulePath[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(2, szPid[g_enumLang], LVCFMT_LEFT, 65);
	m_list.InsertColumn(3, szProcessObject[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(4, szProcessName[g_enumLang], LVCFMT_LEFT, 100);

	m_hIcon = reinterpret_cast<HICON>(::LoadImage ( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PROCESS_ICON),IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR ));
	
	EnumNoSignModules();

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CheckFileSignature(LPCWSTR lpFileName, HANDLE hFile)
{
	BOOL bRet = FALSE;
	WINTRUST_DATA wd = { 0 };
	WINTRUST_FILE_INFO wfi = { 0 };
	WINTRUST_CATALOG_INFO wci = { 0 };
	CATALOG_INFO ci = { 0 };
	HCATADMIN hCatAdmin = NULL;

	if (!lpFileName || hFile == 0 || hFile == INVALID_HANDLE_VALUE)
	{
		return bRet;
	}

	if (!CryptCATAdminAcquireContext(&hCatAdmin, NULL, 0))
	{
		return bRet;
	}

	DWORD dwCnt = 100;
	BYTE byHash[100];
	if (CryptCATAdminCalcHashFromFileHandle(hFile, &dwCnt, byHash, 0))
	{
		LPWSTR pszMemberTag = new WCHAR[dwCnt * 2 + 1];
		if (pszMemberTag)
		{
			for (DWORD dw = 0; dw < dwCnt; dw++)
			{
				wsprintfW(&pszMemberTag[dw * 2], L"%02X", byHash[dw]);
			}

			HCATINFO hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, byHash, dwCnt, 0, NULL);
			if (NULL == hCatInfo)
			{
				wfi.cbStruct       = sizeof( WINTRUST_FILE_INFO );

				wfi.pcwszFilePath  = lpFileName;

				//	wfi.pcwszFilePath  = NULL;
				//	wfi.hFile          = hFile;
				wfi.hFile  = NULL;
				wfi.pgKnownSubject = NULL;

				wd.cbStruct            = sizeof( WINTRUST_DATA );
				wd.dwUnionChoice       = WTD_CHOICE_FILE;
				wd.pFile               = &wfi;
				wd.dwUIChoice          = WTD_UI_NONE;
				wd.fdwRevocationChecks = WTD_REVOKE_NONE;
				wd.dwStateAction       = WTD_STATEACTION_IGNORE;
				wd.dwProvFlags         = WTD_SAFER_FLAG;
				wd.hWVTStateData       = NULL;
				wd.pwszURLReference    = NULL;
			}
			else
			{
				CryptCATCatalogInfoFromContext(hCatInfo, &ci, 0);
				wci.cbStruct             = sizeof(WINTRUST_CATALOG_INFO);
				wci.pcwszCatalogFilePath = ci.wszCatalogFile;
				wci.pcwszMemberFilePath  = lpFileName;
				//	wci.pcwszMemberFilePath  = NULL;
				//	wci.hMemberFile			 = hFile;
				wci.hMemberFile			 = NULL;

				wci.pcwszMemberTag       = pszMemberTag;

				wd.cbStruct            = sizeof(WINTRUST_DATA);
				wd.dwUnionChoice       = WTD_CHOICE_CATALOG;
				wd.pCatalog            = &wci;
				wd.dwUIChoice          = WTD_UI_NONE;
				wd.fdwRevocationChecks = WTD_STATEACTION_VERIFY;
				wd.dwProvFlags         = 0;
				wd.hWVTStateData       = NULL;
				wd.pwszURLReference    = NULL;
			}

			GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
			HRESULT hr  = WinVerifyTrust(NULL, &action, &wd);
			bRet        = SUCCEEDED( hr );

			if (hCatInfo)
			{
				CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
			}

			delete[] pszMemberTag;
		}
	}

	CryptCATAdminReleaseContext(hCatAdmin, 0);

	return bRet;
} 

void CNotSignModuleDlg::EnumNoSignModulesProc()
{
	m_bExit = FALSE;
	ULONG nCntFuck = 0;

	vector<PROCESS_INFO> vectorProcess;
	CListProcess ListProc;
	ListProc.EnumProcess(vectorProcess);
	
	CString strTitle;
	strTitle = szFindNotSignatureModule[g_enumLang];
	strTitle += L" ";
	strTitle += szScaning[g_enumLang];
	SetWindowText(strTitle);

	for ( vector <PROCESS_INFO>::iterator Iter = vectorProcess.begin( ); 
		!m_bExit && Iter != vectorProcess.end( ); 
		Iter++ )
	{
		PROCESS_INFO ProcessItem = *Iter;

		if (ProcessItem.ulPid == 0 && ProcessItem.ulParentPid == 0 && ProcessItem.ulEprocess == 0)
		{
			continue;
		}

		CListModules clsListModules;
		vector<MODULE_INFO> vectorModules;
		clsListModules.EnumModulesByPeb(ProcessItem.ulPid, ProcessItem.ulEprocess, vectorModules);
		
		CString strProcessPath = ProcessItem.szPath;
		CString strProcessName = strProcessPath.Right(strProcessPath.GetLength() - strProcessPath.ReverseFind('\\') - 1);

		for ( vector <MODULE_INFO>::iterator DllModuleIter = vectorModules.begin( ); 
			!m_bExit && DllModuleIter != vectorModules.end( ); 
			DllModuleIter++)
		{
			MODULE_INFO ModuleItem = *DllModuleIter;
			CString  strImageName;
			CString strPath = ModuleItem.Path;
			
			if (strPath.IsEmpty())
			{
				continue;
			}

			CString szTitle;
			szTitle.Format(L"[%s] %s", strProcessName, strPath);
			m_szStatus.Empty();
			m_szStatus = szTitle;

			SendMessage(WM_UPDATE_NOSIGN_DLG);

			LPCWSTR lpFileName = strPath.GetBuffer();
			HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

			if (hFile == INVALID_HANDLE_VALUE || 
				!CheckFileSignature(lpFileName, hFile))
			{
				SHFILEINFO sfi;
				DWORD_PTR nRet;
				CString szPEPROCESS;
				CString szPid;
				CString strProcessPath = ProcessItem.szPath;
				CString strProcessName = strProcessPath.Right(strProcessPath.GetLength() - strProcessPath.ReverseFind('\\') - 1);	

				ZeroMemory(&sfi,sizeof(SHFILEINFO));
				nRet = SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
				nRet ? m_ModuleImageList.Add ( sfi.hIcon ) : m_ModuleImageList.Add ( m_hIcon );
				if (nRet)
				{
					DestroyIcon(sfi.hIcon);
				}

				strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);	
				szPid.Format(L"%d", ProcessItem.ulPid);
				szPEPROCESS.Format(L"0x%08X", ProcessItem.ulEprocess);

				int nCnt = m_list.GetItemCount();
				int n = m_list.InsertItem(nCnt, strImageName, nCnt);
				m_list.SetItemText(n, 1, strPath);
				m_list.SetItemText(n, 2, szPid);
				m_list.SetItemText(n, 3, szPEPROCESS);
				m_list.SetItemText(n, 4, strProcessName);
				m_list.SetItemData(n, ModuleItem.Base);

				nCntFuck++;
			}
		}
	}

	if (!m_bExit)
	{
		strTitle.Empty();
		strTitle = szFindNotSignatureModule[g_enumLang];
		SetWindowText(strTitle);
		m_szStatus.Format(szNoSignatureModuleCount[g_enumLang], nCntFuck);
		SendMessage(WM_UPDATE_NOSIGN_DLG);
	}
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	CNotSignModuleDlg *Dlg = (CNotSignModuleDlg *)lpParameter;
	if (Dlg)
	{
		Dlg->EnumNoSignModulesProc();
	}

	return 0;
}

BOOL CNotSignModuleDlg::OpenCryptSvc()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	BOOL bRet = FALSE;

	if (hSCManager)
	{
		SC_HANDLE hCryptSvc = OpenService(hSCManager, L"CryptSvc", SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_PAUSE_CONTINUE);
		if (hCryptSvc)
		{
			SERVICE_STATUS st;
			if (QueryServiceStatus(hCryptSvc, &st))
			{
				if (st.dwCurrentState == SERVICE_STOPPED)
				{
					StartService(hCryptSvc, NULL, NULL);
				}
				else if (st.dwCurrentState == SERVICE_PAUSED)
				{
					ControlService(hCryptSvc, SERVICE_CONTROL_CONTINUE, &st);
				}
			}

			if (QueryServiceStatus(hCryptSvc, &st))
			{
				if (st.dwCurrentState == SERVICE_RUNNING)
				{
					bRet = TRUE;
				}
			}

			CloseServiceHandle(hCryptSvc);
		}

		CloseServiceHandle(hSCManager);
	}

	return bRet;
}

void CNotSignModuleDlg::EnumNoSignModules()
{
	if (m_hThread)
	{
		if (WaitForSingleObject(m_hThread, 1) != WAIT_OBJECT_0)
		{
			TerminateThread(m_hThread, 0);
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	m_list.DeleteAllItems();
	int  nImageCount = m_ModuleImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ModuleImageList.Remove(0);   
	}

	OpenCryptSvc();
	m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
}

void CNotSignModuleDlg::OnClose()
{
// 	if (m_hThread)
// 	{
// 		if (WaitForSingleObject(m_hThread, 1) != WAIT_OBJECT_0)
// 		{
// 			TerminateThread(m_hThread, 0);
// 		}
// 		
// 		CloseHandle(m_hThread);
// 		m_hThread = NULL;
// 	}

	m_bExit = TRUE;
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	CDialog::OnClose();
}

void CNotSignModuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CNotSignModuleDlg::OnNosignmoduleRefresh()
{
	EnumNoSignModules();
}

void CNotSignModuleDlg::OnNosignmoduleDeleteModule()
{
	MessageBox(L"功能还在开发中，敬请期待...");
}

void CNotSignModuleDlg::OnNosignmoduleUnload()
{
// 	int nItem = m_Functions.GetSelectItem(&m_list);
// 	if (nItem != -1)
// 	{
// 		ULONG Pid = _wtoi(m_list.GetItemText(nItem, 2));
// 		ULONG pEprocess = 0;
// 		swscanf_s(m_list.GetItemText(nItem, 3).GetBuffer(wcslen(L"0x")), L"%x", &pEprocess);
// 
// 		COMMUNICATE_PROCESS_MODULE_UNLOAD cpmu;
// 		cpmu.Base = (ULONG)m_list.GetItemData(nItem);
// 		cpmu.nPid = Pid;
// 		cpmu.pEprocess = pEprocess;
// 		cpmu.OpType = enumUnloadDllModule;
// 
// 		CDllModuleDlg DllModuleDlg;
// 		if (DllModuleDlg.UnloadModule(&cpmu))
// 		{
// 			m_list.DeleteItem(nItem);
// 		}
// 	}
}

void CNotSignModuleDlg::OnNosignmoduleUnloadAll()
{
// 	if (MessageBox(szGlobalUnloadModule[g_enumLang], szUnloadModule[g_enumLang], MB_YESNO | MB_ICONWARNING) == IDNO)
// 	{
// 		return;
// 	}
// 
// 	ULONG nItem = 0;
// 	while (m_list.GetItemCount())
// 	{
// 		ULONG Pid = _wtoi(m_list.GetItemText(nItem, 2));
// 		ULONG pEprocess = 0;
// 		swscanf_s(m_list.GetItemText(nItem, 3).GetBuffer(wcslen(L"0x")), L"%x", &pEprocess);
// 
// 		COMMUNICATE_PROCESS_MODULE_UNLOAD cpmu;
// 		cpmu.Base = (ULONG)m_list.GetItemData(nItem);
// 		cpmu.nPid = Pid;
// 		cpmu.pEprocess = pEprocess;
// 		cpmu.OpType = enumUnloadDllModule;
// 
// 		CDllModuleDlg DllModuleDlg;
// 		DllModuleDlg.UnloadModule(&cpmu);
// 		m_list.DeleteItem(nItem);
// 	}	
}

void CNotSignModuleDlg::OnNosignmoduleCopyModuleName()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 0);
		m_Functions.SetStringToClipboard(szPath);
	}
}

void CNotSignModuleDlg::OnNosignmoduleCopyModulePath()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Functions.SetStringToClipboard(szPath);
	}
}

void CNotSignModuleDlg::OnNosignmoduleCheckShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CNotSignModuleDlg::OnNosignmoduleInExplorer()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 1);
		m_Functions.LocationExplorer(szPath);
	}
}

void CNotSignModuleDlg::OnNosignmoduleText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CNotSignModuleDlg::OnNosignmoduleExcel()
{
	WCHAR szNoSignatureModule[] = {'N','o','S','i','g','n','a','t','u','r','e','M','o','d','u','l','e','\0'};
	m_Functions.ExportListToExcel(&m_list, szNoSignatureModule, m_szStatus);
}

void CNotSignModuleDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_NOSIGNMODULE_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_NOSIGNMODULE_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_DELETE_MODULE, szDeleteModuleFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_UNLOAD, szUnload[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_UNLOAD_ALL, szUnloadAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_COPY_MODULE_NAME, szCopyModuleName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_COPY_MODULE_PATH, szCopyModulePath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_CHECK_SHUXING, szModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_NOSIGNMODULE_IN_EXPLORER, szFindModuleInExplorer[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);
	
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_NOSIGNMODULE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_NOSIGNMODULE_DELETE_MODULE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_NOSIGNMODULE_IN_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_NOSIGNMODULE_CHECK_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_NOSIGNMODULE_COPY_MODULE_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_NOSIGNMODULE_COPY_MODULE_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(13, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (m_hThread && WaitForSingleObject(m_hThread, 1) != WAIT_OBJECT_0)
	{
		for (int i = 0; i < 14; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else if (!m_list.GetItemCount()) // 如果m_list是空的，那么除了“刷新”，其他全部置灰
	{
		for (int i = 2; i < 14; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetSelectedCount() || m_list.GetSelectedCount() >= 2)  // 没有选中
		{
			for (int i = 2; i < 13; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_NOSIGNMODULE_UNLOAD_ALL, MF_BYCOMMAND | MF_ENABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

LRESULT CNotSignModuleDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return 0;
}