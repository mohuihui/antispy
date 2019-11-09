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
// FindModuleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FindModuleDlg.h"
#include "DllModuleDlg.h"
#include <algorithm>
#include "DllModuleDlg.h"
#include "HandleDlg.h"

// CFindModuleDlg 对话框

#define WM_UPDATE_DLLS_OR_MODULES				WM_USER + 115

IMPLEMENT_DYNAMIC(CFindModuleDlg, CDialog)

CFindModuleDlg::CFindModuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindModuleDlg::IDD, pParent)
	, m_szFindModuleName(_T(""))
	, m_szStatus(_T(""))
	, m_nRadio(0)
{
	m_hThread = NULL;
	m_nTotalCnt = 0;
}

CFindModuleDlg::~CFindModuleDlg()
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
}

void CFindModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_EDIT, m_szFindModuleName);
	DDX_Text(pDX, IDC_STATUS, m_szStatus);
	DDX_Control(pDX, IDC_EDIT, m_edit);
	DDX_Radio(pDX, IDC_RADIO_MODULE, m_nRadio);
}


BEGIN_MESSAGE_MAP(CFindModuleDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFindModuleDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_FIND_BUTTON, &CFindModuleDlg::OnBnClickedFindButton)
	ON_COMMAND(ID_FIND_REFRESH, &CFindModuleDlg::OnFindRefresh)
	ON_COMMAND(ID_FIND_DELETE_MODULE, &CFindModuleDlg::OnFindDeleteModule)
	ON_COMMAND(ID_FIND_UNLOAD, &CFindModuleDlg::OnFindUnload)
	ON_COMMAND(ID_FIND_UNLOAD_ALL, &CFindModuleDlg::OnFindUnloadAll)
	ON_COMMAND(ID_FIND_COPY_MODULE_NAME, &CFindModuleDlg::OnFindCopyModuleName)
	ON_COMMAND(ID_FIND_COPY_MODULE_PATH, &CFindModuleDlg::OnFindCopyModulePath)
	ON_COMMAND(ID_FIND_CHECK_SIGN, &CFindModuleDlg::OnFindCheckSign)
	ON_COMMAND(ID_FIND_CHECK_SHUXING, &CFindModuleDlg::OnFindCheckShuxing)
	ON_COMMAND(ID_FIND_IN_EXPLORER, &CFindModuleDlg::OnFindInExplorer)
	ON_COMMAND(ID_FIND_TEXT, &CFindModuleDlg::OnFindText)
	ON_COMMAND(ID_FIND_EXCEL, &CFindModuleDlg::OnFindExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CFindModuleDlg::OnNMRclickList)
	ON_WM_SIZE()
	ON_COMMAND(ID_HANDLE_CLOSE, &CFindModuleDlg::OnHandleClose)
	ON_COMMAND(ID_HANDLE_FORCE_CLOSE, &CFindModuleDlg::OnHandleForceClose)
	ON_MESSAGE(WM_UPDATE_DLLS_OR_MODULES, OnUpdateData)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CFindModuleDlg)
	EASYSIZE(IDC_SZMODULE_NAME, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_EDIT, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_FIND_BUTTON, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CFindModuleDlg 消息处理程序

void CFindModuleDlg::OnBnClickedOk()
{
}

BOOL CFindModuleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetWindowText(szFindModuleDlg[g_enumLang]);

	GetDlgItem(IDC_SZMODULE_NAME)->SetWindowText(szFindDllModuleName[g_enumLang]);
	GetDlgItem(IDC_FIND_BUTTON)->SetWindowText(szFind[g_enumLang]);
	GetDlgItem(IDC_RADIO_MODULE)->SetWindowText(szFindDllModule[g_enumLang]);
	GetDlgItem(IDC_RADIO_HANDLE)->SetWindowText(szFindHnadles[g_enumLang]);

	m_ModuleImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ModuleImageList, LVSIL_SMALL);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szProcessName[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, szPid[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(2, szName[g_enumLang], LVCFMT_LEFT, 460);
	m_list.InsertColumn(3, szHandle[g_enumLang], LVCFMT_LEFT, 100);
	
	m_hModuleIcon = LoadIcon(NULL, IDI_APPLICATION);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

DWORD WINAPI EnumThread(PVOID p)
{
	if (p)
	{
		CFindModuleDlg* pDlg = (CFindModuleDlg*)p;
		pDlg->EnumProc();
	}

	return 0;
}

void CFindModuleDlg::EnumModules(PROCESS_INFO info)
{
	if (info.ulPid == 0)
	{
		return;
	}

	BOOL bRet    =    FALSE; 
	HANDLE hModuleSnap = NULL; 
	MODULEENTRY32 me32 ={0}; 
	CString szProcPath = info.szPath;
	SHFILEINFO sfi;
	DWORD_PTR nRet;
	CString strImageName = szProcPath.Right(szProcPath.GetLength() - szProcPath.ReverseFind('\\') - 1);	

	ZeroMemory(&sfi, sizeof(SHFILEINFO));
	nRet = SHGetFileInfo(szProcPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	int nImage = nRet ? m_ModuleImageList.Add ( sfi.hIcon ) : m_ModuleImageList.Add ( m_hModuleIcon );
	if (nRet)
	{
		DestroyIcon(sfi.hIcon);
	}

	hModuleSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, info.ulPid);
	if(hModuleSnap == INVALID_HANDLE_VALUE) 
	{    
		return; 
	} 

	me32.dwSize = sizeof(MODULEENTRY32); 

	if(::Module32First(hModuleSnap, &me32))
	{ 
		do{ 

			CString szDll = m_Functions.TrimPath(me32.szExePath);
			szDll.MakeLower();

			if (szDll.Find(m_szFindModuleName) != -1)
			{
				int nCnt = m_list.GetItemCount();
				m_list.InsertItem(nCnt, strImageName, nImage);

				CString szPid;
				szPid.Format(L"%d", info.ulPid);
				m_list.SetItemText(nCnt, 1, szPid);

				m_list.SetItemText(nCnt, 2, szDll);

				CString szHandle;
				szHandle.Format(L"0x%08X", me32.hModule);
				m_list.SetItemText(nCnt, 3, szHandle);

				m_list.SetItemData(nCnt, m_nTotalCnt);

				DLL_INFO_EX infoex;
				infoex.dwBase = (ULONG)me32.hModule;
				infoex.dwEprocess = info.ulEprocess;
				infoex.dwPid = info.ulPid;
				m_vectorDLLs.push_back(infoex);

				m_nTotalCnt++;
			}

		}while(::Module32Next(hModuleSnap, &me32)); 
	}

	CloseHandle(hModuleSnap); 
}

void CFindModuleDlg::EnumHandles(PROCESS_INFO info)
{
	if (info.ulPid == 0)
	{
		return;
	}

	CString szProcPath = info.szPath;
	CString strImageName = szProcPath.Right(szProcPath.GetLength() - szProcPath.ReverseFind('\\') - 1);	
	SHFILEINFO sfi;
	DWORD_PTR nRet;
	
	ZeroMemory(&sfi, sizeof(SHFILEINFO));
	nRet = SHGetFileInfo(szProcPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	int nImage = nRet ? m_ModuleImageList.Add ( sfi.hIcon ) : m_ModuleImageList.Add ( m_hModuleIcon );
	if (nRet)
	{
		DestroyIcon(sfi.hIcon);
	}

	CHandleDlg handleDlg;
	handleDlg.m_dwPid = info.ulPid;
	handleDlg.m_pEprocess = info.ulEprocess;
	handleDlg.EnumProcessHandles();

	for ( vector <HANDLE_INFO>::iterator Iter = handleDlg.m_HandleVector.begin( ); 
		Iter != handleDlg.m_HandleVector.end( ); 
		Iter++ )
	{
		HANDLE_INFO HandleInfo = *Iter;
		CString szHandleName = HandleInfo.HandleName;
		if (szHandleName.IsEmpty())
		{
			continue;
		}

		szHandleName.MakeLower();

		if (szHandleName.Find(m_szFindModuleName) != -1)
		{
			int n = m_list.InsertItem(m_list.GetItemCount(), strImageName, nImage);

			CString szPid;
			szPid.Format(L"%d", info.ulPid);
			m_list.SetItemText(n, 1, szPid);

			m_list.SetItemText(n, 2, HandleInfo.HandleName);

			CString szHandle;
			szHandle.Format(L"0x%04X", HandleInfo.Handle);
			m_list.SetItemText(n, 3, szHandle);
			
			m_list.SetItemData(n, m_nTotalCnt);

			HANDLE_INFO_EX hInfoEx;
			hInfoEx.dwEprocess = info.ulEprocess;
			hInfoEx.dwPid = info.ulPid;
			hInfoEx.Handle = HandleInfo.Handle;
			hInfoEx.Object = HandleInfo.Object;
			m_vectorHandles.push_back(hInfoEx);

			m_nTotalCnt++;
		}
	}
}

void CFindModuleDlg::EnumProc()
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

		m_szStatus.Format(L"%s %s", szSearching[g_enumLang], ProcessItem.szPath);
		SendMessage(WM_UPDATE_DLLS_OR_MODULES);

		if (m_nRadio == 0)
		{
			EnumModules(ProcessItem);
		}
		else if (m_nRadio == 1)
		{
			EnumHandles(ProcessItem);
		}
	}

	WCHAR *szStatus = NULL;
	if (m_nRadio == 0)
	{
		szStatus = szSearchDoneDlls[g_enumLang];
	}
	else
	{
		szStatus = szSearchDoneHandles[g_enumLang];
	}

	m_szStatus.Format(szStatus, m_nTotalCnt);
	SendMessage(WM_UPDATE_DLLS_OR_MODULES);
}

void CFindModuleDlg::OnBnClickedFindButton()
{
	UpdateData(TRUE);

	m_list.DeleteAllItems();
	m_nTotalCnt = 0;
	m_vectorHandles.clear();
	m_vectorDLLs.clear();

	int  nImageCount = m_ModuleImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ModuleImageList.Remove(0);   
	}

	if (m_szFindModuleName.IsEmpty())
	{
		MessageBox(szFindDllsOrModulesInputHaveWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
		return;
	}

	m_szFindModuleName.MakeLower();

	if (m_hThread)
	{
		if (WaitForSingleObject(m_hThread, 1) != WAIT_OBJECT_0)
		{
			TerminateThread(m_hThread, 0);
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	m_hThread = CreateThread(NULL, 0, EnumThread, this, 0, NULL);
}

void CFindModuleDlg::OnFindRefresh()
{
	OnBnClickedFindButton();
}

void CFindModuleDlg::OnFindDeleteModule()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szFindModuleDlg[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		BOOL bRet= m_Functions.KernelDeleteFile(m_list.GetItemText(nItem, 2));
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

void CFindModuleDlg::OnFindUnload()
{
// 	int nItem = m_Functions.GetSelectItem(&m_list);
// 	if (nItem != -1)
// 	{
// 		int nData = (int)m_list.GetItemData(nItem);
// 		DLL_INFO_EX info = m_vectorDLLs.at(nData);
// 
// 		COMMUNICATE_PROCESS_MODULE_UNLOAD cpmu;
// 		cpmu.Base = info.dwBase;
// 		cpmu.nPid = info.dwPid;
// 		cpmu.pEprocess = info.dwEprocess;
// 		cpmu.OpType = enumUnloadDllModule;
// 
// 		CDllModuleDlg DllModuleDlg;
// 		if (DllModuleDlg.UnloadModule(&cpmu))
// 		{
// 			m_list.DeleteItem(nItem);
// 		}
// 	}
}

void CFindModuleDlg::OnFindUnloadAll()
{
// 	if (MessageBox(szGlobalUnloadModule[g_enumLang], szUnloadModule[g_enumLang], MB_YESNO | MB_ICONWARNING) == IDNO)
// 	{
// 		return;
// 	}
// 
// 	while (m_list.GetItemCount())
// 	{
// 		int nData = (int)m_list.GetItemData(0);
// 		DLL_INFO_EX info = m_vectorDLLs.at(nData);
// 
// 		COMMUNICATE_PROCESS_MODULE_UNLOAD cpmu;
// 		cpmu.Base = info.dwBase;
// 		cpmu.nPid = info.dwPid;
// 		cpmu.pEprocess = info.dwEprocess;
// 		cpmu.OpType = enumUnloadDllModule;
// 
// 		CDllModuleDlg DllModuleDlg;
// 		DllModuleDlg.UnloadModule(&cpmu);
// 
// 		m_list.DeleteItem(0);
// 	}	
}

void CFindModuleDlg::OnFindCopyModuleName()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.SetStringToClipboard(szPath);
	}
}

void CFindModuleDlg::OnFindCopyModulePath()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.SetStringToClipboard(szPath);
	}
}

void CFindModuleDlg::OnFindCheckSign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.SignVerify(szPath);
	}
}

void CFindModuleDlg::OnFindCheckShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CFindModuleDlg::OnFindInExplorer()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.LocationExplorer(szPath);
	}
}

void CFindModuleDlg::OnFindText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CFindModuleDlg::OnFindExcel()
{
	if (m_nRadio == 0)
	{
		WCHAR szFindMoule[] = {'F','i','n','d','M','o','u','l','e','\0'};
		m_Functions.ExportListToExcel(&m_list, szFindMoule, m_szStatus);
	}
	else
	{
		WCHAR szFindMoule[] = {'F','i','n','d','H','a','n','d','l','e','\0'};
		m_Functions.ExportListToExcel(&m_list, szFindMoule, m_szStatus);
	}
}

void CFindModuleDlg::PopupDllMenu()
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_FIND_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_FIND_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_FIND_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FIND_DELETE_MODULE, szDeleteModuleFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FIND_UNLOAD, szUnload[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_FIND_UNLOAD_ALL, szUnloadAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
// 	menu.AppendMenu(MF_STRING, ID_FIND_COPY_MODULE_NAME, szCopyModuleName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_FIND_COPY_MODULE_PATH, szCopyModulePath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FIND_CHECK_SIGN, szVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FIND_CHECK_SHUXING, szModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_FIND_IN_EXPLORER, szFindModuleInExplorer[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_FIND_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_FIND_DELETE_MODULE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FIND_IN_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_FIND_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_FIND_CHECK_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_FIND_COPY_MODULE_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_FIND_COPY_MODULE_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(14, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount()) // 如果m_list是空的，那么除了“刷新”，其他全部置灰
	{
		for (int i = 2; i < 15; i++)
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

			menu.EnableMenuItem(ID_FIND_UNLOAD_ALL, MF_BYCOMMAND | MF_ENABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();
}

void CFindModuleDlg::PopupHandleMenu()
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_FIND_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_FIND_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_FIND_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_HANDLE_CLOSE, szHandleClose[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_HANDLE_FORCE_CLOSE, szHandleForceClose[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_FIND_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_HANDLE_CLOSE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_HANDLE_FORCE_CLOSE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(5, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 6; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (!nCnt)
		{
			for (int i = 2; i < 5; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();
}

void CFindModuleDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_nRadio == 0)
	{
		PopupDllMenu();
	}
	else
	{
		PopupHandleMenu();
	}

	*pResult = 0;
}

void CFindModuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CFindModuleDlg::OnHandleClose()
{
	KillHandle(FALSE);
}

void CFindModuleDlg::OnHandleForceClose()
{
	KillHandle(TRUE);
}

void CFindModuleDlg::KillHandle(BOOL bForce)
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	while (pos)
// 	{
// 		int nItem  = m_list.GetNextSelectedItem(pos);
// 		int nData = (int)m_list.GetItemData(nItem);
// 		HANDLE_INFO_EX hInfoEx = m_vectorHandles.at(nData);
// 
// 		COMMUNICATE_HANDLE ch;
// 		ch.OpType = enumCloseHandle;
// 		ch.op.CloseHandle.nPid = hInfoEx.dwPid;
// 		ch.op.CloseHandle.pEprocess = hInfoEx.dwEprocess;
// 		ch.op.CloseHandle.bForceKill = bForce;
// 		ch.op.CloseHandle.hHandle = hInfoEx.Handle;
// 		ch.op.CloseHandle.pHandleObject = hInfoEx.Object;
// 		m_Driver.CommunicateDriver(&ch, sizeof(COMMUNICATE_HANDLE), NULL, 0, NULL);
// 
// 		m_list.DeleteItem(nItem);
// 		pos = m_list.GetFirstSelectedItemPosition();
// 	}
}

LRESULT CFindModuleDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return 0;
}