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
// ListProcessDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ListProcessDlg.h"
#define STRSAFE_NO_DEPRECATE 
#include <strsafe.h>
#include <tlhelp32.h>
#include <algorithm>
#include "DllModuleDlg.h"
#include "ThreadDlg.h"
#include "HandleDlg.h"
#include "Function.h"
#include "ProcessMemoryDlg.h"
#include "ProcessWndDlg.h"
#include "ProcessTimerDlg.h"
#include "HotKeyDlg.h"
#include "ProcessPrivilegesDlg.h"
#include <shlwapi.h>
#include "FindModuleDlg.h"
#include "NotSignModuleDlg.h"
#include "ProcessInfoDlg.h"
#include "ProcessHookDlg.h"
#include <DbgHelp.h>
#include "SetConfigDlg.h"
#include "FindWindow.h"

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Version.lib")

typedef enum _PROCESS_HEADER_INDEX_
{
	eProcessImageName,
	eProcessPid,
	eProcessParentPid,
	eProcessPath,
//	eProcessEprocess,
	eProcessR3Access,
	eProcessFileCompany,
	eProcessFileDescription,
	eProcessCreateTime,
}PROCESS_HEADER_INDEX;

typedef enum _PROCESS_TREE_HEADER_INDEX_
{
	eProcessTreeImageName,
	eProcessTreePid,
//	eProcessParentPid,
	eProcessTreePath,
//	eProcessEprocess,
	eProcessTreeR3Access,
	eProcessTreeFileCompany,
	eProcessTreeFileDescription,
	eProcessTreeCreateTime,
}PROCESS_TREE_HEADER_INDEX;

////////////////////////////////////////////////////////////////////

// CListProcessDlg 对话框

IMPLEMENT_DYNAMIC(CListProcessDlg, CDialog)

CListProcessDlg::CListProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CListProcessDlg::IDD, pParent)
	, m_szProcessCnt(_T(""))
{
	m_bShowAsTreeList = FALSE;
	m_nProcessCnt = m_nHideCnt = m_nDenyAccessCnt = 0;
	m_bShowTipWindow = TRUE;
	m_clrHideProc = RGB( 255, 0, 0 );					// 隐藏进程默认是 红色
	m_clrMicrosoftProc = RGB(0, 0, 0);					// 微软进程默认是 黑色
	m_clrNormalProc = RGB( 0, 0, 255 );					// 正常的进程默认是 蓝色
	m_clrMicroHaveOtherModuleProc = RGB( 255, 140, 0 );	// 微软带有非微软模块默认是 橘黄色
	m_clrNoSigniture = RGB( 180, 0, 255 );				// 默认是紫色
	m_bShowBelow = TRUE;								// 点击列表头的时候暂停显示下方数据
	m_nBelowDialog = 0;									// 总共有几个窗口
	m_nCurSel = eDllModuleDlg;							// 下面的窗口现在选的是哪
}

CListProcessDlg::~CListProcessDlg()
{
	m_Ring0ProcessList.clear();
	m_Ring3ProcessList.clear();
}

void CListProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROCESS_CNT, m_szProcessCnt);
	DDX_Control(pDX, IDC_TAB, m_tab);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_processList);
	DDX_Control(pDX, IDC_PROCESS_TREE, m_processTree);
}


BEGIN_MESSAGE_MAP(CListProcessDlg, CDialog)
	ON_MESSAGE(WM_RESIZE_ALL_PROC_WND, ResizeAllProcWnd)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_PROCESS, &CListProcessDlg::OnNMCustomdrawProcessList) // list控件的颜色绘制
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CListProcessDlg::OnNMRclickProcessList)			// list控件的右键菜单
	ON_COMMAND(ID_PROCESS_REFRESH, &CListProcessDlg::OnProcessRefresh)
	ON_COMMAND(ID_CHECK_SIGN, &CListProcessDlg::OnCheckSign)
	ON_COMMAND(ID_CHECK_ALL_SIGN, &CListProcessDlg::OnCheckAllSign)
	ON_COMMAND(ID_KILL_PROCESS, &CListProcessDlg::OnKillProcess)
	ON_COMMAND(ID_KILL_DELETE, &CListProcessDlg::OnKillDelete)
	ON_COMMAND(ID_SUSPEND_PROCESS, &CListProcessDlg::OnSuspendProcess)
	ON_COMMAND(ID_RESUME_PROCESS, &CListProcessDlg::OnResumeProcess)
	ON_COMMAND(ID_COPY_PROCESS_NAME, &CListProcessDlg::OnCopyProcessName)
	ON_COMMAND(ID_COPY_PROCESS_PATH, &CListProcessDlg::OnCopyProcessPath)
	ON_COMMAND(ID_CHECK_ATTRIBUTE, &CListProcessDlg::OnCheckAttribute)
	ON_COMMAND(ID_LOCATION_EXPLORER, &CListProcessDlg::OnLocationExplorer)
	ON_COMMAND(ID_SEARCH_ONLINE, &CListProcessDlg::OnSearchOnline)
	ON_WM_SIZE()
	ON_COMMAND(ID_SHOW_ALL_PROCESS_INFO, &CListProcessDlg::OnShowAllProcessInfo)
	ON_MESSAGE(WM_VERIFY_SIGN_OVER, VerifySignOver)
	ON_MESSAGE(WM_CLICK_LIST_HEADER_START, OnClickListHeaderStart)
	ON_MESSAGE(WM_CLICK_LIST_HEADER_END, OnClickListHeaderEnd)
	ON_MESSAGE(WM_ENUM_TIMER, MsgEnumTimer)
	ON_MESSAGE(WM_ENUM_MODULES, MsgEnumModules)
	ON_MESSAGE(WM_ENUM_HANDLES, MsgEnumHandles)
	ON_MESSAGE(WM_ENUM_THREADS, MsgEnumThreads)
	ON_MESSAGE(WM_ENUM_WINDOWS, MsgEnumWindows)
	ON_MESSAGE(WM_ENUM_HOTKEYS, MsgEnumHotKeys)
	ON_MESSAGE(WM_ENUM_MEMORY, MsgEnumMemory)
	ON_MESSAGE(WM_ENUM_PRIVILEGES, MsgEnumPrivileges)
	ON_COMMAND(ID_EXPORT_EXCEL, &CListProcessDlg::OnExportExcel)
	ON_COMMAND(ID_EXPORT_TEXT, &CListProcessDlg::OnExportText)
	ON_COMMAND(ID_LISTPROCESS_DUMP, &CListProcessDlg::OnDumpProcessMemory)
	ON_COMMAND(ID_PROCESS_FIND_MODULE, &CListProcessDlg::OnProcessFindModule)
	ON_COMMAND(ID_PROCESS_FIND_UNSIGNED_MODUES, &CListProcessDlg::OnProcessFindUnsignedModues)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROCESS, &CListProcessDlg::OnNMDblclkProcessList)			// 进程列表双击显示进程详情 
	ON_COMMAND(ID_PROCESS_HOOK, &CListProcessDlg::OnProcessHook)
	ON_COMMAND(ID_CREATE_MINIDUMP, &CListProcessDlg::OnCreateMinidump)
	ON_COMMAND(ID_CREATE_FULLDUMP, &CListProcessDlg::OnCreateFulldump)
	ON_NOTIFY(NM_RCLICK, IDC_COLUMNTREE, &CListProcessDlg::OnRclickedColumntree)
	ON_NOTIFY(NM_RCLICK, IDC_PROCESS_TREE, &CListProcessDlg::OnRclickedColumntree)			// 进程树控件的右键菜单
	ON_COMMAND(ID_SHOW_PROCESS_TREE, &CListProcessDlg::OnShowProcessTree)
	ON_UPDATE_COMMAND_UI(ID_SHOW_PROCESS_TREE, &CListProcessDlg::OnUpdateShowProcessTree)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_KILL_PROCESS_TREE, &CListProcessDlg::OnKillProcessTree)
	ON_COMMAND(ID_INJECT_DLL, &CListProcessDlg::OnInjectDll)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PROCESS, &CListProcessDlg::OnLvnItemchangedProcessList)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PROCESS_TREE, &CListProcessDlg::OnTvnSelchangedTreeList) // 进程树形控件改变了选项
	ON_NOTIFY(NM_CLICK, IDC_PROCESS_TREE, &CListProcessDlg::OnNMClickTreeList) // 进程树形控件改变了选项
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CListProcessDlg::OnProcessLocationAtFileManager)
	ON_MESSAGE(WM_UPDATE_PROCESS_DATA, OnUpdateProcessInfoStatus)
	ON_COMMAND(ID_LOOK_WINDOW_FOR_PROCESS, &CListProcessDlg::OnLookWindowForProcess)
	ON_WM_LBUTTONUP()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CListProcessDlg::OnTcnSelchangeTab)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PROCESS, &CListProcessDlg::OnNMClickListProcess)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CListProcessDlg)
  	EASYSIZE(IDC_PROCESS_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
 	EASYSIZE(IDC_TAB, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_X_SP, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_LIST_PROCESS, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_PROCESS_TREE, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CListProcessDlg 消息处理程序

// 
// 初始化进程信息下面的标签窗口
// 
void CListProcessDlg::InitProcInfoWndTab(CDialog *pDlg)
{
	if (!pDlg)
	{
		return;
	}

// 	CRect rc;
// 	m_tab.GetClientRect(rc);
// 	rc.top += 20;
// 	rc.bottom -= 2;
// 	rc.left += 0;
// 	rc.right -= 2;

	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 22;
	rc.bottom -= 3;
	rc.left += 2;
	rc.right -= 3;

	pDlg->MoveWindow(&rc);

	// 把对话框对象指针保存起来
	m_pBelowDialog[m_nBelowDialog] = pDlg;

	// 显示初始页面
	if (m_nBelowDialog == 0)
	{
		m_pBelowDialog[m_nBelowDialog]->ShowWindow(SW_SHOW);
	}
	else
	{
		m_pBelowDialog[m_nBelowDialog]->ShowWindow(SW_HIDE);
	}

	m_nBelowDialog++;
}

//
// 初始化对话框例程
//
BOOL CListProcessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_ySplitter.BindWithControl(this, IDC_X_SP);
	m_ySplitter.SetMinHeight(100, 50);

	// 初始化进程列表
 	m_ProImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 2, 2); 
	m_processList.SetImageList (&m_ProImageList, LVSIL_SMALL);
	m_processList.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_processList.InsertColumn(eProcessImageName, szImageName[g_enumLang], LVCFMT_LEFT, 130);
	m_processList.InsertColumn(eProcessPid, szPid[g_enumLang], LVCFMT_LEFT, 65);
	m_processList.InsertColumn(eProcessParentPid, szParentPid[g_enumLang], LVCFMT_LEFT, 65);
	m_processList.InsertColumn(eProcessPath, szImagePath[g_enumLang], LVCFMT_LEFT, 290);
// 	m_processList.InsertColumn(eProcessEprocess, szProcessObject[g_enumLang], LVCFMT_LEFT, 80);
	m_processList.InsertColumn(eProcessR3Access, szR3Access[g_enumLang], LVCFMT_CENTER, 75);
	m_processList.InsertColumn(eProcessFileCompany, szFileCorporation[g_enumLang], LVCFMT_LEFT, 140);
	m_processList.InsertColumn(eProcessFileDescription, szFileDescriptionNew[g_enumLang], LVCFMT_LEFT, 190);
	m_processList.InsertColumn(eProcessCreateTime, szProcessStartedTime[g_enumLang], LVCFMT_LEFT, 140);

	// 初始化快捷键句柄
	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR));

	// 初始化默认的进程图标
	m_hProcessIcon = LoadIcon(NULL, IDI_APPLICATION);

	// 初始化树形列表控件
	m_imgList.Create(16, 16, ILC_COLOR32|ILC_MASK, 2, 2); 
	UINT uTreeStyle = TVS_HASBUTTONS | /*TVS_HASLINES|*/ TVS_LINESATROOT |/* TVS_CHECKBOXES |*/ TVS_FULLROWSELECT;
	
	// 初始化带模块的进程树形控件
	m_processTree.GetTreeCtrl().SetImageList(&m_imgList, TVSIL_NORMAL);
	m_processTree.GetTreeCtrl().ModifyStyle(0, uTreeStyle);
	m_processTree.SetFirstColumnMinWidth(170);
	m_processTree.InsertColumn(eProcessTreeImageName, szImageName[g_enumLang], LVCFMT_LEFT, 220);
	m_processTree.InsertColumn(eProcessTreePid, szPid[g_enumLang], LVCFMT_LEFT, 65);
//	m_processTree.InsertColumn(eProcessParentPid, szParentPid[g_enumLang], LVCFMT_LEFT, 65);
	m_processTree.InsertColumn(eProcessTreePath, szImagePath[g_enumLang], LVCFMT_LEFT, 260);
// 	m_processTree.InsertColumn(eProcessEprocess, szProcessObject[g_enumLang], LVCFMT_LEFT, 85);
	m_processTree.InsertColumn(eProcessTreeR3Access, szR3Access[g_enumLang], LVCFMT_CENTER, 75);
	m_processTree.InsertColumn(eProcessTreeFileCompany, szFileCorporation[g_enumLang], LVCFMT_LEFT, 150);
	m_processTree.InsertColumn(eProcessTreeFileDescription, szFileDescriptionNew[g_enumLang], LVCFMT_LEFT, 170);
	m_processTree.InsertColumn(eProcessTreeCreateTime, szProcessStartedTime[g_enumLang], LVCFMT_LEFT, 150);
	
	m_bShowAsTreeList = g_Config.GetShowProcessTreeAsDefault();
	AdjustShowWindow();

	// 初始化tip提示框
	m_tipWindow.Create(this);
	m_tipWindow.ShowWindow(SW_HIDE);
	m_bShowTipWindow = g_Config.GetShowProcessDetailInfo();

	// 设置不同进程显示的颜色
	m_clrHideProc = g_Config.GetHideProcColor();
	m_clrMicrosoftProc = g_Config.GetMicrosoftProcColor();
	m_clrNormalProc = g_Config.GetNormalProcColor();
	m_clrMicroHaveOtherModuleProc = g_Config.GetMicroHaveOtherModuleProcColor();
	m_clrNoSigniture = g_Config.GetNoSignatureModuleColor();
	
	m_processTree.GetTreeCtrl().SetItemColor(
		m_clrNormalProc,
		m_clrHideProc,
		m_clrMicrosoftProc,
		m_clrMicroHaveOtherModuleProc);

	m_tab.InsertItem(eDllModuleDlg, szModuleInfo[g_enumLang]);
	m_tab.InsertItem(eHandleDlg, szHandleInfo[g_enumLang]);
	m_tab.InsertItem(eThreadDlg, szThreadInfo[g_enumLang]);
	m_tab.InsertItem(eMemoryDlg, szMemoryInfo[g_enumLang]);
	m_tab.InsertItem(eWndDlg, szWindowInfo[g_enumLang]);
	m_tab.InsertItem(eHotKeyDlg, szHotKeyInfo[g_enumLang]);
	m_tab.InsertItem(eTimerDlg, szTimerInfo[g_enumLang]);
	m_tab.InsertItem(ePrivilegeDlg, szPrivilegeInfo[g_enumLang]);

	m_ModuleDlg.Create(IDD_DLL_MODULE_DIALOG, &m_tab);
	m_HandleDlg.Create(IDD_HANDLE_DIALOG, &m_tab);
	m_ThreadDlg.Create(IDD_THREAD_DIALOG, &m_tab);
	m_MemoryDlg.Create(IDD_MEMORY_DIALOG, &m_tab);
	m_WndDlg.Create(IDD_PROCESS_WND_DIALOG, &m_tab);
	m_HotKeyDlg.Create(IDD_HOTKEY_DIALOG, &m_tab);
	m_TimerDlg.Create(IDD_PROCESS_TIMER_DIALOG, &m_tab);
	m_PrivilegeDlg.Create(IDD_PROCESS_PRIVILEGES_DIALOG, &m_tab);

	InitProcInfoWndTab(&m_ModuleDlg);
	InitProcInfoWndTab(&m_HandleDlg);
	InitProcInfoWndTab(&m_ThreadDlg);
	InitProcInfoWndTab(&m_MemoryDlg);
	InitProcInfoWndTab(&m_WndDlg);
	InitProcInfoWndTab(&m_HotKeyDlg);
	InitProcInfoWndTab(&m_TimerDlg);
	InitProcInfoWndTab(&m_PrivilegeDlg);

	ListProcess();
	
	g_pProcessDlg = this;

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// ring3 使用常规方法枚举进程
//
BOOL CListProcessDlg::GetRing3ProcessList()
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	BOOL bRet = FALSE;

	// 首先清空下链表
	m_Ring3ProcessList.clear();

	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap != INVALID_HANDLE_VALUE )
	{
		pe32.dwSize = sizeof( PROCESSENTRY32 );

		if( Process32First( hProcessSnap, &pe32 ) )
		{
			do
			{
				m_Ring3ProcessList.push_back(pe32.th32ProcessID);
			} while( Process32Next( hProcessSnap, &pe32 ) );

			bRet = TRUE;
		}

		CloseHandle( hProcessSnap );
	}

	return bRet;
}

//
// ring0的pid跟ring3的对比，得出是否是隐藏进程
//
BOOL CListProcessDlg::IsProcessHide(ULONG pid)
{
	BOOL bHide = FALSE;

	if (!m_Ring3ProcessList.empty())
	{
		list<ULONG>::iterator findItr;
		findItr = find(m_Ring3ProcessList.begin(), m_Ring3ProcessList.end(), pid);
		if (findItr == m_Ring3ProcessList.end())
		{
			bHide = TRUE;
		}
	}

	return bHide;
}

//
// 以结束进程权限打开，看是否成功
//
BOOL CListProcessDlg::CanOpenProcess(DWORD dwPid)
{
	BOOL bOpen = TRUE;

	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_VM_OPERATION, TRUE, dwPid);
	if (hProcess)
	{
		CloseHandle(hProcess);
	}
	else
	{
		bOpen = FALSE;
	}

	return bOpen;
}

//
// 采用Snapshot的方法(快速)来判断是否有非微软的模块
//
BOOL CListProcessDlg::IsHaveNotMicrosoftModule(DWORD dwPID) 
{ 
	BOOL bRet    =    FALSE; 
	HANDLE hModuleSnap = NULL; 
	MODULEENTRY32 me32 = {0}; 

	hModuleSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if(hModuleSnap == INVALID_HANDLE_VALUE || dwPID == 0) 
	{    
		return FALSE; 
	} 

	me32.dwSize = sizeof(MODULEENTRY32); 
	if(::Module32First(hModuleSnap, &me32))
	{ 
		do{ 

			if (!m_Functions.IsMicrosoftAppByPath(m_Functions.TrimPath(me32.szExePath)))
			{
				bRet = TRUE;
				break;
			}

		}while(::Module32Next(hModuleSnap, &me32)); 
	}

	CloseHandle(hModuleSnap); 
	return bRet; 
} 

//
// 添加进程
//
void CListProcessDlg::AddProcessItem(PROCESS_INFO item)
{
	if (item.ulPid == 0 && item.ulParentPid == 0 && item.ulEprocess == 0)
	{
		return;
	}

	CString strPid, strParentPid, strEprocess, strCompany, strDescription, strR3Access, szDescription, szStartedTime;

	// 处理进程路径，pid
	CString strPath = TrimPath(item.szPath);
	CString strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
	item.ulParentPid ? strParentPid.Format(L"%d", item.ulParentPid) : strParentPid.Format(L"-");

	// 处理进程图标
	SHFILEINFO sfi;
	DWORD_PTR nRet;
	HICON hIcon = NULL;

	ZeroMemory(&sfi, sizeof(SHFILEINFO));

	nRet = SHGetFileInfo(strPath, 
		FILE_ATTRIBUTE_NORMAL, 
		&sfi, 
		sizeof(SHFILEINFO), 
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
		);

	int nIcon = -1;
	if (nRet)
	{
		nIcon = m_ProImageList.Add ( sfi.hIcon );
		DestroyIcon(sfi.hIcon);
	}
	else
	{
		nIcon = m_ProImageList.Add ( m_hProcessIcon );
	}

	if (CanOpenProcess(item.ulPid))
	{
		strR3Access = L"-";
	}
	else
	{
		strR3Access = szDeny[g_enumLang];
		m_nDenyAccessCnt++;
	}

	strEprocess.Format(L"0x%08X", item.ulEprocess);
	strPid.Format(L"%d", item.ulPid);
	strCompany = m_Functions.GetFileCompanyName(strPath);
	szDescription = m_Functions.GetFileDescription(strPath);
	szStartedTime = FileTime2SystemTime(item.ulCreateTime);

	int nCnt = m_processList.GetItemCount();
	int i = m_processList.InsertItem(nCnt, strImageName, nIcon);
	m_processList.SetItemText(i, eProcessPid, strPid);
	m_processList.SetItemText(i, eProcessParentPid, strParentPid);
	m_processList.SetItemText(i, eProcessPath, strPath);
//	m_processList.SetItemText(i, eProcessEprocess, strEprocess);
	m_processList.SetItemText(i, eProcessR3Access, strR3Access);
	m_processList.SetItemText(i, eProcessFileCompany, strCompany);
	m_processList.SetItemText(i, eProcessFileDescription, szDescription);
	m_processList.SetItemText(i, eProcessCreateTime, szStartedTime);
	
	// 默认黑色
	ITEM_COLOR_INFO clrInfo;
	clrInfo.textClr = enumBlack;

	// 如果是隐藏的，那么显示红色
	if (IsProcessHide(item.ulPid))
	{
		m_nHideCnt++;
		clrInfo.textClr = enumRed;
	}

	// 如果不是微软的模块，那么就显示蓝色
	else if (!(m_Functions.IsMicrosoftApp(strCompany) || 
		!strPath.CompareNoCase( L"System" ) || 
		!strPath.CompareNoCase(L"Idle")))
	{
		clrInfo.textClr = enumBlue;
	}

	// 如果微软进程被注入了其他模块，那么显示土黄色
	if (clrInfo.textClr == enumBlack && IsHaveNotMicrosoftModule(item.ulPid))
	{
		clrInfo.textClr = enumTuhuang;
	}
	
	m_vectorColor.push_back(clrInfo);
	m_vectorProcesses.push_back(item);

	m_processList.SetItemData(i, nCnt);

	m_nProcessCnt++;
}

//
// 以List的形式展示进程
//
void CListProcessDlg::ShowProcessAsList()
{
	m_szProcessCnt.Format(szProcessState[g_enumLang], 0, 0, 0);
	SendMessage(WM_UPDATE_PROCESS_DATA);

	// 清空image列表、进程列表，初始化默认图标
	m_processList.DeleteAllItems();
	m_vectorColor.clear();
	m_vectorProcesses.clear();

	int nImageCount = m_ProImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ProImageList.Remove(0);
	}

	// 初始清零
	m_nDenyAccessCnt = m_nProcessCnt = m_nHideCnt = 0;
	
	// 内核获取系统全部进程
	m_clsProcess.EnumProcess(m_Ring0ProcessList);
	if (m_Ring0ProcessList.empty())
	{
		return;
	}

	// 用CreateToolhelp32Snapshot的方法获取应用层的进程
	GetRing3ProcessList();
	
	// 根据启动时间排序
	m_clsProcess.SortByProcessCreateTime(m_Ring0ProcessList);
	
	for ( vector <PROCESS_INFO>::iterator Iter = m_Ring0ProcessList.begin( ); 
		Iter != m_Ring0ProcessList.end( ); 
		Iter++)
	{
		AddProcessItem(*Iter);
	}

	m_szProcessCnt.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

BOOL UDgreaterEx( PROCESS_INFO elem1, PROCESS_INFO elem2 )
{
	return elem1.ulParentPid < elem2.ulParentPid;
}

//
// 进程树模式下，根据父进程ID排个序
//
VOID CListProcessDlg::SortByParentPid(vector<PROCESS_INFO> &ProcessListEx)
{
	sort( ProcessListEx.begin( ), ProcessListEx.end( ), UDgreaterEx );
}

//
// 判断一个进程是否有父进程，需要满足两个条件，进程ID一样，并且创建时间先后要一致
//
BOOL CListProcessDlg::IsHaveParent(PROCESS_INFO_EX info)
{
	BOOL bRet = FALSE;

	// 如果父进程PID是0，则没有父进程
	if (info.ulParentPid == 0)
	{
		return bRet;
	}
	
	// 遍历进程列表看是否父进程还在
	for ( vector<PROCESS_INFO_EX>::iterator Iter = m_ProcessListEx.begin( ); 
		Iter != m_ProcessListEx.end( ); 
		Iter++)
	{
		// 进程ID一样，并且创建时间先后要一致
		if (info.ulParentPid == Iter->ulPid && info.nCreateTime > Iter->nCreateTime)
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}

//
// 插入子项
//
void CListProcessDlg::InsertTreeProcessChildItem(PROCESS_INFO_EX* pItem, HTREEITEM hRoot)
{
	for ( vector<PROCESS_INFO_EX>::iterator Iter = m_ProcessListEx.begin( ); 
		Iter != m_ProcessListEx.end( ); 
		Iter++)
	{
		// 如果已经插入，或者ParentPid不等于传进来的PID
		if (Iter->bInsert == TRUE				|| 
			Iter->ulParentPid != pItem->ulPid	|| 
			Iter->nCreateTime <= pItem->nCreateTime)
		{
			continue;
		}
	
		AddProcessTreeItem(&(*Iter), hRoot);
	}
}

//
// 转化进程,将PROCESS_INFO结构转化成PROCESS_INFO_EX结构的链表
//
void CListProcessDlg::TransferProcessInfo2ProcessInfoEx()
{
	for ( vector <PROCESS_INFO>::iterator Iter = m_Ring0ProcessList.begin( ); Iter != m_Ring0ProcessList.end( ); Iter++ )
	{
		PROCESS_INFO item = *Iter;

		if (item.ulPid == 0 && item.ulParentPid == 0 && item.ulEprocess == 0)
		{
			continue;
		}

		CString strPath = TrimPath(item.szPath);

		// 处理进程图标
		SHFILEINFO sfi;
		DWORD_PTR nRet;

		ZeroMemory(&sfi, sizeof(SHFILEINFO));

		nRet = SHGetFileInfo(strPath, 
			FILE_ATTRIBUTE_NORMAL, 
			&sfi, 
			sizeof(SHFILEINFO), 
			SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
			);	

		int nIcon = -1;
		if (nRet)
		{
			nIcon = m_imgList.Add ( sfi.hIcon );
			DestroyIcon(sfi.hIcon);
		}
		else
		{
			nIcon = m_imgList.Add ( m_hProcessIcon );
		}

		PROCESS_INFO_EX info;
		info.ulPid = item.ulPid;
		info.ulParentPid = item.ulParentPid;
		info.ulEprocess = item.ulEprocess;
		info.nIconOrder = nIcon;
		info.szPath = strPath;
		info.bInsert = FALSE;
		info.nCreateTime = item.ulCreateTime;
		m_ProcessListEx.push_back(info);
	}
}

//
// 添加一个树形列表的进程项
//
void CListProcessDlg::AddProcessTreeItem(PROCESS_INFO_EX* Iter, HTREEITEM hItem)
{
	CString strPid, strParentPid, strEprocess, strR3Access;
	BOOL bHide = FALSE;
	HTREEITEM hRoot;

	// 处理进程路径，pid
	CString strPath = Iter->szPath;
	CString strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
	Iter->ulParentPid ? strParentPid.Format(L"%d", Iter->ulParentPid) : strParentPid.Format(L"-");

	if (CanOpenProcess(Iter->ulPid))
	{
		strR3Access = L"-";
	}
	else
	{
		m_nDenyAccessCnt++;
		strR3Access = szDeny[g_enumLang];
	}

	strEprocess.Format(L"0x%08X", Iter->ulEprocess);
	strPid.Format(L"%d", Iter->ulPid);
	CString strCompany = m_Functions.GetFileCompanyName(strPath);
	CString szDescription = m_Functions.GetFileDescription(strPath);
	CString m_szStartedTime = FileTime2SystemTime(Iter->nCreateTime);
	
	if (hItem == NULL)
	{
		hRoot = m_processTree.GetTreeCtrl().InsertItem(strImageName, Iter->nIconOrder, Iter->nIconOrder);
	}
	else
	{
		hRoot = m_processTree.GetTreeCtrl().InsertItem(strImageName, Iter->nIconOrder, Iter->nIconOrder, hItem);
	}
	
	m_processTree.SetItemText(hRoot, eProcessTreePid, strPid);
// 	m_processTree.SetItemText(hRoot, eProcessParentPid, strParentPid);
	m_processTree.SetItemText(hRoot, eProcessTreePath, strPath);
//	m_processTree.SetItemText(hRoot, eProcessEprocess, strEprocess);
	m_processTree.SetItemText(hRoot, eProcessTreeR3Access, strR3Access);
	m_processTree.SetItemText(hRoot, eProcessTreeFileCompany, strCompany);
	m_processTree.SetItemText(hRoot, eProcessTreeFileDescription, szDescription);
	m_processTree.SetItemText(hRoot, eProcessTreeCreateTime, m_szStartedTime);

	// 默认为黑色
	ITEM_COLOR_INFO clrInfo;
	clrInfo.textClr = enumBlack;

	if (IsProcessHide(Iter->ulPid))
	{
		m_nHideCnt++;
		clrInfo.textClr = enumRed;
	}
	else if ( !(m_Functions.IsMicrosoftApp(strCompany) || 
		!strPath.CompareNoCase( L"System" ) || 
		!strPath.CompareNoCase(L"Idle")) )
	{
		clrInfo.textClr = enumBlue;
	}

	// 如果下方显示模块列表，并且微软进程被注入了其他模块，那么显示土黄色
	if (clrInfo.textClr == enumBlack && IsHaveNotMicrosoftModule(Iter->ulPid))
	{
		clrInfo.textClr = enumTuhuang;
	}

	// 此项已插入，做个标记
	Iter->bInsert = TRUE;
	Iter->hItem = hRoot;

	// 将新的项push到列表里
	m_vectorProcessEx.push_back(*Iter);
	m_vectorColor.push_back(clrInfo);
	m_processTree.GetTreeCtrl().SetItemData(hRoot, m_nProcessCnt);
	
	m_nProcessCnt++;

	// 如果PID不为0，那么就插入子项
	if (Iter->ulPid != 0)
	{
		InsertTreeProcessChildItem(Iter, hRoot);
		m_processTree.GetTreeCtrl().Expand(hRoot, TVE_EXPAND);
	}
}

//
// 以进程树的形式显示进程列表
//
void CListProcessDlg::ShowProcessAsTreeList()
{
	m_szProcessCnt.Format(szProcessState[g_enumLang], 0, 0, 0);
	SendMessage(WM_UPDATE_PROCESS_DATA);

	if (IsWindow(m_processTree.GetTreeCtrl().m_hWnd))
	{
		m_processTree.GetTreeCtrl().DeleteAllItems();
	}

	// 清空image列表、进程列表，初始化默认图标
	m_ProcessListEx.clear();
	m_vectorColor.clear();
	m_vectorProcessEx.clear();
	
	m_nDenyAccessCnt = m_nProcessCnt = m_nHideCnt = 0;

	int nImageCount = m_imgList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_imgList.Remove(0);   
	}
	
	m_clsProcess.EnumProcess(m_Ring0ProcessList);
	if (m_Ring0ProcessList.empty())
	{
		return;
	}

	// 根据父进程ID排个序
	SortByParentPid(m_Ring0ProcessList);

	// 获取ring3进程列表
	GetRing3ProcessList(); 
	
	// 转化进程
	TransferProcessInfo2ProcessInfoEx();
	if (m_ProcessListEx.empty())
	{
		return;
	}

	// 第一次遍历插入没有父进程的项
	for ( vector<PROCESS_INFO_EX>::iterator Iter = m_ProcessListEx.begin( ); 
		Iter != m_ProcessListEx.end( ); 
		Iter++)
	{
		// 判断是否已经插入
		if (Iter->bInsert != FALSE)
		{
			continue;
		}

		// 判断是否有父进程，没有则直接以父进程插入
		if (!IsHaveParent(*Iter))
		{
			AddProcessTreeItem(&(*Iter), NULL);
		}
	}
	
	m_szProcessCnt.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 枚举进程函数，分为进程树模式和列表模式两种显示进程列表
//
VOID CListProcessDlg::ListProcess()
{
	if (m_bShowAsTreeList)
	{
		m_processTree.ShowWindow(FALSE);
		ShowProcessAsTreeList();
		m_processTree.ShowWindow(TRUE);
		m_processTree.Invalidate(TRUE);
	}
	else
	{
		ShowProcessAsList();
	}
}

//
// 进程list模式下颜色显示
//
void CListProcessDlg::OnNMCustomdrawProcessList(NMHDR *pNMHDR, LRESULT *pResult)
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
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		clrNewTextColor = m_clrMicrosoftProc;
		clrNewBkColor = RGB( 255, 255, 255 );	

		int nData = (int)m_processList.GetItemData(nItem);
		ITEM_COLOR clrInfo = m_vectorColor.at(nData).textClr;
		if (clrInfo == enumRed)
		{
			clrNewTextColor = m_clrHideProc;
		}
		else if ( clrInfo == enumBlue )
		{
			clrNewTextColor = m_clrNormalProc;
		}
		else if (clrInfo == enumTuhuang)
		{
			clrNewTextColor = m_clrMicroHaveOtherModuleProc;
		}
		else if (clrInfo == enumPurple)
		{
			clrNewTextColor = m_clrNoSigniture;
		}
		
 		pLVCD->clrText = clrNewTextColor;
 		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

//
// 进程List模式下的右键菜单
//
void CListProcessDlg::OnNMRclickProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_EXPORT_EXCEL, szExcel[g_enumLang]);
	
	WCHAR szMiniDump[] = {'M','i','n','i',' ','D','u','m','p','\0'};
	WCHAR szFullDump[] = {'F','u','l','l',' ','D','u','m','p','\0'};

	CMenu createdump;
	createdump.CreatePopupMenu();
	createdump.AppendMenu(MF_STRING, ID_CREATE_MINIDUMP, szMiniDump);
	createdump.AppendMenu(MF_STRING, ID_CREATE_FULLDUMP, szFullDump);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PROCESS_REFRESH, szProcessRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SHOW_PROCESS_TREE, szShowProcessTree[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
 	menu.AppendMenu(MF_STRING, ID_SHOW_ALL_PROCESS_INFO, szShowAllProcessInfo[g_enumLang]);
 	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_KILL_PROCESS, szKillProcess[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_KILL_DELETE, szKillAndDeleteFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SUSPEND_PROCESS, szSuspend[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_RESUME_PROCESS, szResume[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PROCESS_HOOK, szScanHooks[g_enumLang]);
 	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_INJECT_DLL, szInjectDll[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_LISTPROCESS_DUMP, szProcessDump[g_enumLang]);
	menu.AppendMenu(MF_POPUP, (UINT)createdump.m_hMenu, szCreateDump[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_PROCESS_NAME, szCopyImageName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_COPY_PROCESS_PATH, szCopyImagePath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_ATTRIBUTE, szProcessProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SEARCH_ONLINE, szSearchProcessOnline[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_LOCATION_EXPLORER, szFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_SIGN, szVerifyProcessSignature[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CHECK_ALL_SIGN, szVerifyAllProcessSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_LOOK_WINDOW_FOR_PROCESS, szFindProcessByWindwo[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_FIND_MODULE, szFindModule[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_FIND_UNSIGNED_MODUES, szFindUnsignedModules[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	// 设置默认项
	menu.SetDefaultItem(ID_SHOW_ALL_PROCESS_INFO);
	
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	
	DebugLog(L"x: %d, y: %d", x, y);

	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_PROCESS_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_LOCATION_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_KILL_PROCESS, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_KILL_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_SEARCH_ONLINE, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_COPY_PROCESS_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_COPY_PROCESS_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_SHOW_ALL_PROCESS_INFO, MF_BYCOMMAND, &m_bmDetalInof, &m_bmDetalInof);
		menu.SetMenuItemBitmaps(ID_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_CHECK_ALL_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_PROCESS_FIND_MODULE, MF_BYCOMMAND, &m_bmLookFor, &m_bmLookFor);
		menu.SetMenuItemBitmaps(ID_PROCESS_FIND_UNSIGNED_MODUES, MF_BYCOMMAND, &m_bmLookFor, &m_bmLookFor);
		menu.SetMenuItemBitmaps(ID_LOOK_WINDOW_FOR_PROCESS, MF_BYCOMMAND, &m_bmLookFor, &m_bmLookFor);

		menu.SetMenuItemBitmaps(16, MF_BYPOSITION, &m_bmWindbg, &m_bmWindbg);
		menu.SetMenuItemBitmaps(34, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	// 如果m_list是空的，那么除了“刷新”，其他全部置灰
	if (!m_processList.GetItemCount()) 
	{
		for (int i = 2; i < 35; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		int nSelCnt = m_processList.GetSelectedCount();
		if ( nSelCnt != 1 )
		{
			for (int i = 2; i < 33; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_LOOK_WINDOW_FOR_PROCESS, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PROCESS_FIND_MODULE, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PROCESS_FIND_UNSIGNED_MODUES, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_CHECK_ALL_SIGN, MF_BYCOMMAND | MF_ENABLED);

			if (nSelCnt >= 2)
			{
				menu.EnableMenuItem(ID_KILL_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_KILL_DELETE, MF_BYCOMMAND | MF_ENABLED);
			}
		}

		// 如果选择了一项
		else
		{
			SUSPEND_OR_RESUME_TYPE SorType = ProcessMenuResumeOrSuspend();
			switch (SorType)
			{
			case enumAllFalse:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				break;

			case enumAllOk:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				break;

			case enumResume:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				break;

			case enumSuspend:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				break;
			}
		}
	}
	
	menu.EnableMenuItem(ID_SHOW_PROCESS_TREE, MF_BYCOMMAND | MF_ENABLED);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

//
// 进程刷新菜单
//
void CListProcessDlg::OnProcessRefresh()
{
	ListProcess();
}

//
// 验证单个进程签名
//
void CListProcessDlg::OnCheckSign()
{
	CString szPath = L"";

	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem == -1)
		{
			return;
		}

		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return;
		}
		
		szPath = pInfo->szPath;
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (!hItem)
		{
			return;
		}

		PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
		if (!pInfo)
		{
			return;
		}

		szPath = pInfo->szPath;
	}

	if (!szPath.IsEmpty()/* && PathFileExists(szPath)*/)
	{
		m_SignVerifyDlg.m_bSingle = TRUE;
		m_SignVerifyDlg.m_path = szPath;
		m_SignVerifyDlg.DoModal();
	}
}

//
// 验证所有进程的签名
//
void CListProcessDlg::OnCheckAllSign()
{
	m_SignVerifyDlg.m_NotSignItemList.clear();
	m_SignVerifyDlg.m_bSingle = FALSE;
	m_SignVerifyDlg.m_pList = &m_processList;;
	m_SignVerifyDlg.m_nPathSubItem = eProcessPath;
	m_SignVerifyDlg.m_hWinWnd = this->m_hWnd;
	m_SignVerifyDlg.DoModal();
}

//
// 进程列表模式下，根据item得到相对应PROCESS_INFO的信息
//
PPROCESS_INFO CListProcessDlg::GetProcessInfoByItem(int nItem)
{
	PPROCESS_INFO pInfo = NULL;
	
	if (nItem == -1)
	{
		return pInfo;
	}

	int nData = (int)m_processList.GetItemData(nItem);
	pInfo = &m_vectorProcesses.at(nData);
	
	return pInfo;
}

//
// 进程进程树模式下，根据item得到相对应PROCESS_INFO_EX的信息
//
PPROCESS_INFO_EX CListProcessDlg::GetProcessInfoExByItem(HTREEITEM nItem)
{
	PPROCESS_INFO_EX pInfo = NULL;

	if (nItem == NULL)
	{
		return pInfo;
	}
	
	int nData = (int)m_processTree.GetTreeCtrl().GetItemData(nItem);
	pInfo = &m_vectorProcessEx.at(nData);
	
	return pInfo;
}

//
// 右键菜单 - 结束进程
//
void CListProcessDlg::OnKillProcess()
{
	//
	// 以下是弹框确认
	//

	// 列表模式下
	if (!m_bShowAsTreeList)
	{
		int nCnt = m_processList.GetSelectedCount();

		// 如果是单选
		if ( nCnt == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_processList);
			if (nItem != -1)
			{
				CString szImage = m_processList.GetItemText(nItem, eProcessImageName);
				CString szShow;
				szShow.Format(szAreYOUSureKillProcess[g_enumLang], szImage);

				if (MessageBox(szShow, szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
				{
					return;
				}
			}
		}

		// 多选
		else if ( nCnt > 1)
		{
			if (MessageBox(szAreYOUSureKillTheseProcess[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
			{
				return;
			}
		}

		// 没有选中
		else
		{
			return;
		}
	}

	// 进程树模式下
	else
	{
		CString szImage = m_processTree.GetItemText(m_processTree.GetTreeCtrl().GetSelectedItem(), eProcessTreeImageName);
		CString szShow;
		szShow.Format(szAreYOUSureKillProcess[g_enumLang], szImage);

		if (MessageBox(szShow, szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
	}
	
	//
	// 真正开始结束进程
	//

	// 列表模式下遍历全部选中的项
	if (!m_bShowAsTreeList)
	{
		POSITION pos = m_processList.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = m_processList.GetNextSelectedItem(pos);
			PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);

			if (pInfo)
			{
				m_clsProcess.KillProcess(0, pInfo->ulEprocess);
			}
			
			m_processList.DeleteItem(nItem);
			pos = m_processList.GetFirstSelectedItemPosition();
		}
	}

	// 进程树模式下
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (!hItem)
		{
			return;
		}
		
		PPROCESS_INFO_EX info = GetProcessInfoExByItem(hItem);
		if (!info)
		{
			return;
		}

		m_clsProcess.KillProcess(0, info->ulEprocess);
		
		// 如果有子项，那么刷新下进程树
		if (m_processTree.GetTreeCtrl().ItemHasChildren(hItem))
		{
			Sleep(500);
			OnProcessRefresh();
		}
		else
		{
			m_processTree.GetTreeCtrl().DeleteItem(hItem);
		}
	}
}

// 
// 右键菜单 - 结束进程并删除文件
//
void CListProcessDlg::OnKillDelete()
{
	//
	// 以下是弹框确认
	//

	// 列表模式下
	if (!m_bShowAsTreeList)
	{
		int nCnt = m_processList.GetSelectedCount();
		if ( nCnt == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_processList);
			if (nItem != -1)
			{
				CString szImage = m_processList.GetItemText(nItem, eProcessImageName);
				CString szShow;
				szShow.Format(szAreYOUSureKillAndDeleteProcess[g_enumLang], szImage);

				if (MessageBox(szShow, szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
				{
					return;
				}
			}
		}
		else if ( nCnt > 1)
		{
			if (MessageBox(szAreYOUSureKillAndDeleteProcesses[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
			{
				return;
			}
		}
		else
		{
			return;
		}

		//
		// 开始删除文件并结束进程
		//
		POSITION pos = m_processList.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = m_processList.GetNextSelectedItem(pos);
			PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);

			if (pInfo)
			{
				// 结束进程
				m_clsProcess.KillProcess(0, pInfo->ulEprocess);

				Sleep(100);

				// 删除文件
				m_Functions.KernelDeleteFile(pInfo->szPath);

				Sleep(100);
			}
						
			m_processList.DeleteItem(nItem);
			pos = m_processList.GetFirstSelectedItemPosition();
		}
	}

	// 进程树模式
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (!hItem)
		{
			return;
		}
		
		CString szImage = m_processTree.GetItemText(hItem, eProcessTreeImageName);
		CString szShow;
		szShow.Format(szAreYOUSureKillAndDeleteProcess[g_enumLang], szImage);
		if (MessageBox(szShow, szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}

		PPROCESS_INFO_EX info = GetProcessInfoExByItem(hItem);
		if (!info)
		{
			return;
		}
		
		// 结束进程
		m_clsProcess.KillProcess(0, info->ulEprocess);
		
		Sleep(100);

		// 删除文件
		m_Functions.KernelDeleteFile(info->szPath);
		
		// 如果有子项，那么刷新下进程树
		if (m_processTree.GetTreeCtrl().ItemHasChildren(hItem))
		{
			Sleep(500);
			OnProcessRefresh();
		}
		else
		{
			m_processTree.GetTreeCtrl().DeleteItem(hItem);
		}
	}
}

//
// 暂停进程运行
//
void CListProcessDlg::OnSuspendProcess()
{
	// 列表模式
	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return;
		}
		
		m_clsProcess.SuspendProcess(pInfo->ulPid, pInfo->ulEprocess);
	}

	// 进程树模式
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
			if (!pInfo)
			{
				return;
			}

			m_clsProcess.SuspendProcess(pInfo->ulPid, pInfo->ulEprocess);
		}
	}
}

//
// 恢复进程运行
//
void CListProcessDlg::OnResumeProcess()
{
	// 列表模式
	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return;
		}

		m_clsProcess.ResumeProcess(pInfo->ulPid, pInfo->ulEprocess);
	}

	// 进程树模式
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
			if (!pInfo)
			{
				return;
			}

			m_clsProcess.ResumeProcess(pInfo->ulPid, pInfo->ulEprocess);
		}
	}
}

//
// 拷贝进程名
//
void CListProcessDlg::OnCopyProcessName()
{
	CString szImageName;

	if (!m_bShowAsTreeList)
	{
		POSITION pos = m_processList.GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			int nItem = m_processList.GetNextSelectedItem(pos);
			szImageName = m_processList.GetItemText(nItem, eProcessImageName);
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			szImageName = m_processTree.GetItemText(hItem, eProcessTreeImageName);
		}
	}

	m_Functions.SetStringToClipboard(szImageName);
}

//
// 拷贝进程路径
//
void CListProcessDlg::OnCopyProcessPath()
{
	CString szPath;

	if (!m_bShowAsTreeList)
	{
		POSITION pos = m_processList.GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			int nItem = m_processList.GetNextSelectedItem(pos);
			szPath = m_processList.GetItemText(nItem, eProcessPath);
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			szPath = m_processTree.GetItemText(hItem, eProcessTreePath);
		}
	}

	m_Functions.SetStringToClipboard(szPath);
}

//
// 查看进程属性
//
void CListProcessDlg::OnCheckAttribute()
{
	CString szPath;

	if (!m_bShowAsTreeList)
	{
		POSITION pos = m_processList.GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			int nItem = m_processList.GetNextSelectedItem(pos);
			szPath = m_processList.GetItemText(nItem, eProcessPath);
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			szPath = m_processTree.GetItemText(hItem, eProcessTreePath);
		}
	}

	m_Functions.OnCheckAttribute(szPath);
}

//
// 定位进程
//
void CListProcessDlg::OnLocationExplorer()
{
	CString szPath = L"";

	if (!m_bShowAsTreeList)
	{
		POSITION pos = m_processList.GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			int nItem = m_processList.GetNextSelectedItem(pos);
			szPath = m_processList.GetItemText(nItem, eProcessPath);
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			szPath = m_processTree.GetItemText(hItem, eProcessTreePath);
		}
	}

	m_Functions.LocationExplorer(szPath);
}

//
// 在线搜索进程名
//
void CListProcessDlg::OnSearchOnline()
{
	CString szImageName;

	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem != -1)
		{
			szImageName = m_processList.GetItemText(nItem, eProcessImageName);
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			szImageName = m_processTree.GetItemText(hItem, eProcessTreeImageName);
		}
	}

	m_Functions.SearchOnline(szImageName);
}

//
// 当窗口拉伸时，更新进程列表下方的信息窗口
//
void CListProcessDlg::UpdateBelowDlg()
{
	if (m_tab.m_hWnd && m_ModuleDlg.GetSafeHwnd())
	{
//		CRect rc;
// 		m_tab.GetClientRect(rc);
// 		rc.top += 20;
// 		rc.bottom -= 2;
// 		rc.left += 0;
// 		rc.right -= 2;

		CRect rc;
		m_tab.GetClientRect(rc);
		rc.top += 22;
		rc.bottom -= 3;
		rc.left += 2;
		rc.right -= 3;
		
		for (ULONG i = 0; i < m_nBelowDialog; i++)
		{
			m_pBelowDialog[i]->MoveWindow(&rc);
		}
	}
}

//
// 系统消息响应函数
//
void CListProcessDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
	UpdateBelowDlg();
}

//
// 右键菜单 - 显示进程详细信息
//
void CListProcessDlg::OnShowAllProcessInfo()
{
	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if ( -1 == nItem)
		{
			return;
		}

		ShowProcessListDetailInfo(m_Functions.GetSelectItem(&m_processList));
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (!hItem)
		{
			return;
		}

		ShowProcessTreeDetailInfo(GetTreeListSelectItem());
	}
}

//
// 根据传进来的ITEM判断是否有签名
//
BOOL CListProcessDlg::IsNotSignItem(ULONG nItem)
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

int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{ 
	return lParam1 < lParam2;
}

//
// 批量验证签名结束后，会发送一个WM_VERIFY_SIGN_OVER的自定义消息
//
LRESULT CListProcessDlg::VerifySignOver(WPARAM wParam, LPARAM lParam)
{
	ULONG nCnt = m_processList.GetItemCount();

	for (ULONG i = 0; i < nCnt; i++)
	{
		if (IsNotSignItem(i))
		{
			// 设置颜色
			int nData = (int)m_processList.GetItemData(i);
			m_vectorColor.at(nData).textClr = enumPurple;

			// 设置是否Vrified
			CString szComp = m_processList.GetItemText(i, eProcessFileCompany);
			CString szCompTemp = szNotVerified[g_enumLang] + szComp;
			m_processList.SetItemText(i, eProcessFileCompany, szCompTemp);
		}
		else
		{
			// 设置是否Vrified
			CString szComp = m_processList.GetItemText(i, eProcessFileCompany);
			CString szCompTemp = szVerified[g_enumLang] + szComp;
			m_processList.SetItemText(i, eProcessFileCompany, szCompTemp);
		}
	}

//	m_processList.SortItems(MyCompareProc, NULL);

	return 0;
}

//
// 点击列表头的时候会触发这个消息，那么此时进程列表的下方数据是不需要枚举的
//
LRESULT CListProcessDlg::OnClickListHeaderStart(WPARAM wParam, LPARAM lParam)
{
	m_bShowBelow = FALSE;
	return 0;
}

//
// 点击列表头的时候会触发这个消息，表明排序已经完成
//
LRESULT CListProcessDlg::OnClickListHeaderEnd(WPARAM wParam, LPARAM lParam)
{
	m_bShowBelow = TRUE;
	return 0;
}

//
// 导出信息到EXCEL表格
//
void CListProcessDlg::OnExportExcel()
{
	WCHAR szProcess[] = {'P','r','o','c','e','s','s','\0'};
	m_Functions.ExportListToExcel(&m_processList, szProcess, m_szProcessCnt);
}

//
// 导出信息到TXT文件
//
void CListProcessDlg::OnExportText()
{
	m_Functions.ExportListToTxt(&m_processList, m_szProcessCnt);
}

//
// 拷贝进程内存
//
void CListProcessDlg::OnDumpProcessMemory()
{
	CString szImage = L"";
	DWORD dwPid = 0, dwpEprocess = 0;

	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem != -1)
		{
			PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
			if (pInfo)
			{
				szImage = m_processList.GetItemText(nItem, eProcessImageName);
				dwPid = pInfo->ulPid;
				dwpEprocess = pInfo->ulEprocess;
			}
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
			if (pInfo)
			{
				dwPid = pInfo->ulPid;
				dwpEprocess = pInfo->ulEprocess;
				szImage = m_processTree.GetItemText(hItem, eProcessTreeImageName);
			}
			
		}
	}

	if (dwPid == 0 || dwpEprocess == 0)
	{
		return;
	}

	WCHAR *szImageTemp = CString2WString(szImage);
	if (!szImageTemp)
	{
		return;
	}

	WCHAR szFile[MAX_PATH] = {0};
	wcsncpy_s(szFile, MAX_PATH, L"Dumped_", wcslen(L"Dumped_"));
	wcscat_s(szFile, MAX_PATH - wcslen(L"Dumped_"), szImageTemp);

	free(szImageTemp);

	CFileDialog fileDlg(
		FALSE, 
		0, 
		(LPWSTR)szFile, 
		0, 
		L"All Files (*.*)|*.*||",
		0
		);

	if (IDOK != fileDlg.DoModal())
	{
		return;
	}

	CString szFilePath = fileDlg.GetFileName();
	if ( !PathFileExists(szFilePath) ||
		(PathFileExists(szFilePath) && MessageBox(szFileExist[g_enumLang], szDumpMemory[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES))
	{
		BOOL bRet = FALSE;
		PVOID pBuffer = NULL;
		ULONG nSize = sizeof(ULONG);
		BOOL bFirst = TRUE;

		do 
		{
			if (!bFirst)
			{
				nSize = *(PULONG)pBuffer;
			}

			if (pBuffer)
			{
				free(pBuffer);
				pBuffer = NULL;
			}
			
			pBuffer = malloc(nSize);
			
			if (pBuffer)
			{
				memset(pBuffer, 0, nSize);
				COMMUNICATE_PROCESS cp;
				cp.OpType = enumDumpProcess;
				cp.op.DumpProcess.nPid = dwPid;
				cp.op.DumpProcess.pEprocess = dwpEprocess;
				bRet = m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCESS), pBuffer, nSize, NULL);
			}
			
			bFirst = FALSE;

		} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

		BOOL bCopy = FALSE;
		if (bRet)
		{
			// 修复PE映像
			if (MessageBox(szFixDump[g_enumLang], szDumpMemory[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				FixPeDump(pBuffer);
			}
			
			CFile file;
			TRY 
			{
				if (file.Open(szFilePath,  CFile::modeCreate | CFile::modeWrite))
				{
					file.Write(pBuffer, nSize);
					file.Close();
					bCopy = TRUE;
				}
			}
			CATCH_ALL( e )
			{
				file.Abort();   // close file safely and quietly
				//THROW_LAST();
			}
			END_CATCH_ALL
		}

		if (bCopy)
		{
			MessageBox(szDumpOK[g_enumLang], szDumpMemory[g_enumLang]);
		}
		else
		{
			MessageBox(szDumpFailed[g_enumLang], szDumpMemory[g_enumLang]);
		}

		free(pBuffer);
	}
}

//
// 查找模块或者句柄
//
void CListProcessDlg::OnProcessFindModule()
{
	CFindModuleDlg FindModuleDlg;
	FindModuleDlg.DoModal();
}

//
// 查找没有签名的模块
//
void CListProcessDlg::OnProcessFindUnsignedModues()
{
	CNotSignModuleDlg NotSignModuleDlg;
	NotSignModuleDlg.DoModal();
}

//
// 获取进程挂起、恢复状态
//
SUSPEND_OR_RESUME_TYPE CListProcessDlg::GetProcessSuspendOrResumeStatus(ULONG nPid, ULONG pEprocess)
{	
	SUSPEND_OR_RESUME_TYPE type = enumAllFalse;
	COMMUNICATE_PROCESS cp;
	RESUME_OR_SUSPEND rs;

	memset(&rs, 0, sizeof(RESUME_OR_SUSPEND));

	cp.OpType = enumGetProcessSuspendStatus;
	cp.op.GetProcessSuspendStatus.nPid = nPid;
	cp.op.GetProcessSuspendStatus.pEprocess = pEprocess;

	if (m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCESS), &rs, sizeof(RESUME_OR_SUSPEND), NULL))
	{
		if ((rs.bResume && rs.bSuspend))
		{
			type = enumAllOk;
		}
		else if (!rs.bResume && !rs.bSuspend)
		{
			type = enumAllFalse;
		}
		else if (rs.bResume)
		{
			type = enumResume;
		}
		else if (rs.bSuspend)
		{
			type = enumSuspend;
		}
	}

	return type;
}

//
// 树形控件的右键菜单获取进程挂起、恢复状态
//
SUSPEND_OR_RESUME_TYPE CListProcessDlg::IsSuspendOrResumeProcessTree()
{
	SUSPEND_OR_RESUME_TYPE type = enumAllFalse;

	// 如果没有选中
	HTREEITEM hItem = GetTreeListSelectItem();
	if (!hItem)
	{
		return type;
	}

	PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
	if (!pInfo)
	{
		return type;
	}

	// 如果选中的是我们的进程
	if (pInfo->ulPid == GetCurrentProcessId())
	{
		return type;
	}
	
	type = GetProcessSuspendOrResumeStatus(pInfo->ulPid, pInfo->ulEprocess);

	return type;
}

// 
// 获取进程的挂起和恢复状态
//
SUSPEND_OR_RESUME_TYPE CListProcessDlg::ProcessMenuResumeOrSuspend()
{
	SUSPEND_OR_RESUME_TYPE type = enumAllFalse;

	//
	// 如果没有选中，就两个都置灰
	//
	int nItem = m_Functions.GetSelectItem(&m_processList);
	if (nItem == -1)
	{
		return type;
	}
	
	PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
	if (!pInfo)
	{
		return type;
	}
	
	//
	// 如果选中的是我们的进程，就两个都置灰
	//
	if (pInfo->ulPid == GetCurrentProcessId())
	{
		return type;
	}
	
	type = GetProcessSuspendOrResumeStatus(pInfo->ulPid, pInfo->ulEprocess);

	return type;
}

//
// 系统例程
//
BOOL CListProcessDlg::PreTranslateMessage(MSG* pMsg)
{
	// 首先解析快捷按键
	if (m_hAccel)   
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))   
		{
			return(TRUE);
		}
	}
	
	if (m_bShowTipWindow)
	{
		if(pMsg->message == WM_MOUSEMOVE && pMsg->wParam == 0)
		{
			int id = ::GetDlgCtrlID(pMsg->hwnd);
			CPoint point;
			point.x = LOWORD(pMsg->lParam);
			point.y = HIWORD(pMsg->lParam);

			// 显示list模式
			if ( !m_bShowAsTreeList && id == IDC_LIST_PROCESS )
			{
				MouseOverListCtrl(point);
				return CDialog::PreTranslateMessage(pMsg);
			}

			if (m_bShowAsTreeList && id != IDC_TAB && id != IDC_X_SP)
			{
				MouseOverCtrl(point, pMsg->hwnd);
			}
		}
	}
	
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 
	if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4 )  return TRUE; //屏蔽ALT+F4
	
	return CDialog::PreTranslateMessage(pMsg);
}

//
// 进程树模式下显示tips提示
//
void CListProcessDlg::MouseOverCtrl(CPoint point, HWND hWnd)
{
	CWnd* pWnd = FromHandle(hWnd);
	if(!pWnd)
		return;

	int id = pWnd->GetDlgCtrlID();
	if(id == IDC_TAB || id == IDC_X_SP)
	{
		return;
	}

	int nCol = -1;
	HTREEITEM hItem = ListTreeHitTestEx(point, &nCol);
	if (hItem && nCol == 0)
	{
		PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
		if (!pInfo)
		{
			return;
		}

		DWORD dwPid = pInfo->ulPid;
		if (dwPid <= 0)
		{
			return;
		}

		CString szCommandLine = m_clsProcess.GetProcessCmdLine(dwPid);
		CString szCurrentDirectory = m_clsProcess.GetProcessCurrentDirectory(dwPid);
		CString szStartedTime = FileTime2SystemTime(pInfo->nCreateTime);
		CString szPebAddress = m_clsProcess.GetPebAddress(dwPid);
		CString szTitle = m_processTree.GetItemText(hItem, eProcessTreeImageName);
		CString szEprocess;
		szEprocess.Format(L"0x%08X", pInfo->ulEprocess);
		CString sItemText = L"";
		
		if (!szCommandLine.IsEmpty())
		{
			sItemText += L"Command Line:\n";
			sItemText += szCommandLine;
			sItemText += L"\n";
		}

		if (!szCurrentDirectory.IsEmpty())
		{
			sItemText += L"Current Directory:\n";
			sItemText += L"      ";
			sItemText += szCurrentDirectory;
			sItemText += L"\n";
		}

		if (!szStartedTime.IsEmpty())
		{
			sItemText += L"Started Time:\n";
			sItemText += L"      ";
			sItemText += szStartedTime;
			sItemText += L"\n";
		}

		if (!szPebAddress.IsEmpty())
		{
			sItemText += L"Peb Address:\n";
			sItemText += L"      ";
			sItemText += szPebAddress;
			sItemText += L"\n";
		}

		if (!szEprocess.IsEmpty())
		{
			sItemText += L"Process Object:\n";
			sItemText += L"      ";
			sItemText += szEprocess;
		}

		UpdateData();

		CRect rect;
		pWnd->GetClientRect(&rect);

		//Calculate the client coordinates of the dialog window
		pWnd->ClientToScreen(&rect);
		pWnd->ClientToScreen(&point);
		ScreenToClient(&rect);
		ScreenToClient(&point);

		m_tipWindow.SetTipText(szTitle, sItemText);
		m_tipWindow.ShowTipWindow(CRect(point.x - 50, point.y - 9, point.x + 50, point.y + 9) , point, TWS_XTP_DROPSHADOW | TWS_XTP_ALPHASHADOW, 0, 0, FALSE, TRUE);
	}
}

//
// 进程树模式下，判断当前鼠标所在的项及列
//
HTREEITEM CListProcessDlg::ListTreeHitTestEx(CPoint& point, int* nCol)
{
	int colnum = 0;
	CTVHITTESTINFO info;
	info.pt.x = point.x;
	info.pt.y = point.y/*+5*/;
	HTREEITEM row = m_processTree.HitTest(&info, FALSE);
	if (nCol) *nCol = -1;

	if (row)
	{
		*nCol = info.iSubItem;
	}

	return row;
}

//
// 进程列表Hit测试，返回当前鼠标是在哪一个item，以及那一列
//
int CListProcessDlg::ListHitTestEx(CPoint& point, int* nCol)
{
	int colnum = 0;
	int row = m_processList.HitTest(point, NULL);

	if (nCol) *nCol = 0;

	// Make sure that the ListView is in LVS_REPORT
	if ((GetWindowLong(m_processList.m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
		return row;

	// Get the top and bottom row visible
	row = m_processList.GetTopIndex();
	int bottom = row + m_processList.GetCountPerPage();
	if (bottom > m_processList.GetItemCount())
		bottom = m_processList.GetItemCount();

	// get a pointer to the header control, if NULL return error.
	CHeaderCtrl* pHeaderCtrl = m_processList.GetHeaderCtrl();
	if (pHeaderCtrl == NULL)
		return -1;

	// Get the number of columns
	int nColumnCount = pHeaderCtrl->GetItemCount();

	// Loop through the visible rows
	for (; row <= bottom; row++)
	{
		// Get bounding rect of item and check whether point falls in it.
		CRect rect;
		m_processList.GetItemRect(row, &rect, LVIR_BOUNDS);
		if (rect.PtInRect(point))
		{
			// Now find the column
			for (colnum = 0; colnum < nColumnCount; colnum++)
			{
				int colwidth = m_processList.GetColumnWidth(Header_OrderToIndex(pHeaderCtrl->m_hWnd, colnum));
				if (point.x >= rect.left
					&& point.x <= (rect.left + colwidth))
				{
					if (nCol) *nCol = colnum;
					return row;
				}
				rect.left += colwidth;
			}
		}
	}
	return -1;
}

//
// 内核中获得的进程创建时间转化成系统时间
//
CString CListProcessDlg::FileTime2SystemTime(ULONGLONG ulCreateTile)
{
	if (!ulCreateTile)
	{
		return L"-";
	}

	CString szRet;
	SYSTEMTIME SystemTime;
	FILETIME ft, ftLocal;
	
	ft.dwLowDateTime = (ULONG)ulCreateTile;
	ft.dwHighDateTime = (ULONG)(ulCreateTile >> 32);

	FileTimeToLocalFileTime(&ft, &ftLocal);

	if (FileTimeToSystemTime(&ftLocal, &SystemTime))
	{
		szRet.Format(
			L"%04d/%02d/%02d  %02d:%02d:%02d", 
			SystemTime.wYear,
			SystemTime.wMonth,
			SystemTime.wDay,
			SystemTime.wHour,
			SystemTime.wMinute,
			SystemTime.wSecond);
	}

	return szRet;
}

//
// 进程列表模式下，显示进程tips信息
//
void CListProcessDlg::MouseOverListCtrl(CPoint point)
{
	int nItem = -1, nCol = -1;
	
	// 如果是在第一列上，就显示气泡
	if ((nItem = ListHitTestEx(point, &nCol)) != -1 && nCol == 0)
	{
		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return;
		}

		DWORD dwPid = pInfo->ulPid;
		if (dwPid <= 0)
		{
			return;
		}

		CString m_szCommandLine = m_clsProcess.GetProcessCmdLine(dwPid);
		CString m_szCurrentDirectory = m_clsProcess.GetProcessCurrentDirectory(dwPid);
		CString m_szStartedTime = FileTime2SystemTime(pInfo->ulCreateTime);
		CString m_szPebAddress = m_clsProcess.GetPebAddress(dwPid);
		CString szTitle = m_processList.GetItemText(nItem, eProcessImageName);
		CString szEprocess;
		szEprocess.Format(L"0x%08X", pInfo->ulEprocess);
		CString sItemText = L"";

		if (!m_szCommandLine.IsEmpty())
		{
			sItemText += L"Command Line:\n";
			sItemText += m_szCommandLine;
			sItemText += L"\n";
		}

		if (!m_szCurrentDirectory.IsEmpty())
		{
			sItemText += L"Current Directory:\n";
			sItemText += L"      ";
			sItemText += m_szCurrentDirectory;
			sItemText += L"\n";
		}

		if (!m_szStartedTime.IsEmpty())
		{
			sItemText += L"Started Time:\n";
			sItemText += L"      ";
			sItemText += m_szStartedTime;
			sItemText += L"\n";
		}

		if (!m_szPebAddress.IsEmpty())
		{
			sItemText += L"Peb Address:\n";
			sItemText += L"      ";
			sItemText += m_szPebAddress;
			sItemText += L"\n";
		}

		if (!szEprocess.IsEmpty())
		{
			sItemText += L"Process Object:\n";
			sItemText += L"      ";
			sItemText += szEprocess;
		}

#if _MSC_VER >= 1200 // MFC 5.0
		int nOriginalCol = m_processList.GetHeaderCtrl()->OrderToIndex(nCol);
// 		CString sItemText = m_processList.GetItemText(nItem, nOriginalCol);
// 		int iTextWidth = m_processList.GetStringWidth(sItemText) + 5; //5 pixed extra size

		CRect rect;
		m_processList.GetSubItemRect(nItem, nOriginalCol, LVIR_LABEL, rect);

		// 当它显示不起的时候才以Tips显示
// 		if(iTextWidth > rect.Width())
// 		{
			rect.top--;

			sItemText.Replace(_T("\r\n"), _T("\n")); //Avoid ugly outputted rectangle character in the tip window
			m_tipWindow.SetMargins(CSize(1,1));
			m_tipWindow.SetLineSpace(0);
			m_tipWindow.SetTipText(szTitle, sItemText);
			//Calculate the client coordinates of the dialog window
			m_processList.ClientToScreen(&rect);
			m_processList.ClientToScreen(&point);
			ScreenToClient(&rect);
			ScreenToClient(&point);

			//Show the tip window
/*			UpdateData();*/
			m_tipWindow.ShowTipWindow(rect, point, TWS_XTP_DROPSHADOW | TWS_XTP_ALPHASHADOW, 0, 0, FALSE, TRUE /*m_nDelayMillisecs, 0, false, m_bMoveTipBelowCursor*/);
//		}
#endif
	}
}

//
// 根据pid获取父进程的image name信息
//
CString CListProcessDlg::GetParentProcessInfo(DWORD dwParentPid)
{
	CString szRet;
	CString szImage;
	BOOL bFind = FALSE;
	for ( vector <PROCESS_INFO>::iterator Iter = m_Ring0ProcessList.begin( ); Iter != m_Ring0ProcessList.end( ); Iter++)
	{
		if (Iter->ulPid == dwParentPid)
		{
			CString szPath = Iter->szPath;
			szImage = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
			bFind = TRUE;
			break;
		}
	}

	if (!bFind)
	{
		szImage = szNonExistentProcess[g_enumLang];
	}

	szRet.Format(L"%s (%d)", szImage, dwParentPid);
	return szRet;
}

//
// 进程列表模式下显示详细信息
//
void CListProcessDlg::ShowProcessListDetailInfo(int nItem)
{
	if (nItem == -1)
	{
		return;
	}

	PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
	if (!pInfo)
	{
		return;
	}

	CProcessInfoDlg ProcessInfoDlg;
	ProcessInfoDlg.m_nPid = pInfo->ulPid;
	ProcessInfoDlg.m_pEprocess = pInfo->ulEprocess;
	ProcessInfoDlg.m_szImageName = m_processList.GetItemText(nItem, eProcessImageName);
	ProcessInfoDlg.m_szPath = TrimPath(pInfo->szPath);
	ProcessInfoDlg.m_szParentImage = GetParentProcessInfo(pInfo->ulParentPid);
	ProcessInfoDlg.DoModal();
}

//
// 双击进程列表
//
void CListProcessDlg::OnNMDblclkProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_processList.GetItemCount() > 0 &&
		m_processList.GetSelectedCount() == 1)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem == -1)
		{
			return;
		}
	
		ShowProcessListDetailInfo(nItem);
	}

	*pResult = 0;
}

//
// 扫描进程钩子
//
void CListProcessDlg::OnProcessHook()
{
	DWORD dwPid = 0, dwEprocess = 0;
	CString szPath = L"";

	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem == -1)
		{
			return;
		}

		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return;
		}

		dwPid = pInfo->ulPid;
		dwEprocess = pInfo->ulEprocess;
		szPath = TrimPath(pInfo->szPath);
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
			if (pInfo)
			{
				dwPid = pInfo->ulPid;
				dwEprocess = pInfo->ulEprocess;
				szPath = pInfo->szPath;
			}
		}
	}

	if (dwPid && dwEprocess)
	{
		CProcessHookDlg Dlg;
		Dlg.m_nPid = dwPid;
		Dlg.m_pEprocess = dwEprocess;
		Dlg.m_szPath = szPath;
		Dlg.DoModal();
	}
}

//
// 创建MINI dump
//
void CListProcessDlg::OnCreateMinidump()
{
	CString szPath = CreateOpenFileDlg();
	if (szPath.IsEmpty())
	{
		return;
	}

	if (CreateDump(MiniDumpNormal, szPath))
	{
		MessageBox(szCreateDumpOK[g_enumLang], szToolName, MB_OK);
	}
	else
	{
		MessageBox(szCreateDumpFailed[g_enumLang], szToolName, MB_OK);
	}
}

//
// 创建FULL dump
//
void CListProcessDlg::OnCreateFulldump()
{
	CString szPath = CreateOpenFileDlg();
	if (szPath.IsEmpty())
	{
		return;
	}

	DWORD dwFlg = 
		MiniDumpWithFullMemory | 
		MiniDumpWithHandleData | 
		MiniDumpWithUnloadedModules | 
		2048 |  // MiniDumpWithFullMemoryInfo
		4096;	// MiniDumpWithThreadInfo

	BOOL bRet = CreateDump(dwFlg, szPath);

	if (!bRet && GetLastError() == HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER))
	{
		dwFlg = MiniDumpWithFullMemory | MiniDumpWithHandleData;
		bRet = CreateDump(dwFlg, szPath);
	}
 
	if (bRet)
	{
		MessageBox(szCreateDumpOK[g_enumLang], szToolName, MB_OK);
	}
	else
	{
		MessageBox(szCreateDumpFailed[g_enumLang], szToolName, MB_OK);
	}
}

//
// 一个文件打开对话框
//
CString CListProcessDlg::CreateOpenFileDlg()
{
	CString szRet = L"";
	CString szImage = L"";

	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem != -1)
		{
			szImage = m_processList.GetItemText(nItem, eProcessImageName);
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			szImage = m_processTree.GetItemText(hItem, eProcessTreeImageName);
		}
	}

	if (!szImage.IsEmpty())
	{
		CString szFileName = szImage;
		szFileName += L".dmp";
		
		/*L"Dump Files (*.dmp)|*.dmp;|All Files (*.*)|*.*||"*/
		WCHAR szFileFilter[] = {'D','u','m','p',' ','F','i','l','e','s',' ','(','*','.','d','m','p',')','|','*','.','d','m','p',';','|','A','l','l',' ','F','i','l','e','s',' ','(','*','.','*',')','|','*','.','*','|','|','\0'};
		CFileDialog fileDlg( FALSE, 0, szFileName, 0, szFileFilter, 0 );
		if (IDOK == fileDlg.DoModal())
		{
			CString szFilePath = fileDlg.GetFileName();

			if ( !PathFileExists(szFilePath) ||
				(PathFileExists(szFilePath) && MessageBox(szFileExist[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDYES))
			{
				szRet = szFilePath;
			}
		}
	}

	return szRet;
}

//
// 创建dump
//
BOOL CListProcessDlg::CreateDump(DWORD dwFlag, CString szPath)
{
	BOOL bRet = FALSE;
	DWORD dwPid = 0;

	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem == -1)
		{
			return bRet;
		}
		
		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return bRet;
		}

		dwPid = pInfo->ulPid;
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (!hItem)
		{
			return bRet;
		}

		PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
		if (!pInfo)
		{
			return bRet;
		}

		dwPid = pInfo->ulPid;
	}
	
	if (!szPath.IsEmpty() && dwPid > 0)
	{
		HANDLE hFile = CreateFile(szPath, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		HANDLE hProcess = m_ProcessFunc.OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);

		if (hFile != INVALID_HANDLE_VALUE && hProcess)
		{
			bRet = MiniDumpWriteDump(
				hProcess,
				dwPid,
				hFile,
				(MINIDUMP_TYPE)dwFlag,
				NULL,
				NULL,
				NULL
				);
		}

		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
		}

		if (hProcess)
		{
			CloseHandle(hProcess);
		}
	}		

	return bRet;
}

//
// 进程树控件当前选择的项
//
HTREEITEM CListProcessDlg::GetTreeListSelectItem()
{
	HTREEITEM hItem = NULL;

	hItem = m_processTree.GetTreeCtrl().GetSelectedItem();

	return hItem;
}

//
// 进程树控件右键菜单
//
void CListProcessDlg::OnRclickedColumntree(LPNMHDR pNMHDR, LRESULT* pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_EXPORT_EXCEL, szExcel[g_enumLang]);

	WCHAR szMiniDump[] = {'M','i','n','i',' ','D','u','m','p','\0'};
	WCHAR szFullDump[] = {'F','u','l','l',' ','D','u','m','p','\0'};

	CMenu createdump;
	createdump.CreatePopupMenu();
	createdump.AppendMenu(MF_STRING, ID_CREATE_MINIDUMP, szMiniDump);
	createdump.AppendMenu(MF_STRING, ID_CREATE_FULLDUMP, szFullDump);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PROCESS_REFRESH, szProcessRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SHOW_PROCESS_TREE, szShowProcessTree[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SHOW_ALL_PROCESS_INFO, szShowAllProcessInfo[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_KILL_PROCESS, szKillProcess[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_KILL_DELETE, szKillAndDeleteFile[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_KILL_PROCESS_TREE, szKillProcessTree[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SUSPEND_PROCESS, szSuspend[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_RESUME_PROCESS, szResume[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PROCESS_HOOK, szScanHooks[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_INJECT_DLL, szInjectDll[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_LISTPROCESS_DUMP, szProcessDump[g_enumLang]);
	menu.AppendMenu(MF_POPUP, (UINT)createdump.m_hMenu, szCreateDump[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_PROCESS_NAME, szCopyImageName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_COPY_PROCESS_PATH, szCopyImagePath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_ATTRIBUTE, szProcessProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SEARCH_ONLINE, szSearchProcessOnline[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_LOCATION_EXPLORER, szFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_SIGN, szVerifyProcessSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_LOOK_WINDOW_FOR_PROCESS, szFindProcessByWindwo[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_FIND_MODULE, szFindModule[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_FIND_UNSIGNED_MODUES, szFindUnsignedModules[g_enumLang]);
	
	// 设置默认菜单
	menu.SetDefaultItem(ID_SHOW_ALL_PROCESS_INFO);

	// 设置菜单图标
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_PROCESS_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_LOCATION_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_KILL_PROCESS, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_KILL_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_KILL_PROCESS_TREE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_SEARCH_ONLINE, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_COPY_PROCESS_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_COPY_PROCESS_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_SHOW_ALL_PROCESS_INFO, MF_BYCOMMAND, &m_bmDetalInof, &m_bmDetalInof);
		menu.SetMenuItemBitmaps(ID_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(17, MF_BYPOSITION, &m_bmWindbg, &m_bmWindbg);
		menu.SetMenuItemBitmaps(ID_LOOK_WINDOW_FOR_PROCESS, MF_BYCOMMAND, &m_bmLookFor, &m_bmLookFor);
		menu.SetMenuItemBitmaps(ID_PROCESS_FIND_MODULE, MF_BYCOMMAND, &m_bmLookFor, &m_bmLookFor);
		menu.SetMenuItemBitmaps(ID_PROCESS_FIND_UNSIGNED_MODUES, MF_BYCOMMAND, &m_bmLookFor, &m_bmLookFor);
	}

	CPoint pt;
	::GetCursorPos(&pt);

	m_processTree.ScreenToClient(&pt);

	CTVHITTESTINFO htinfo;
	htinfo.pt = pt;
	HTREEITEM hItem = m_processTree.HitTest(&htinfo);
	if (hItem)
	{
		m_processTree.GetTreeCtrl().SelectItem(hItem);
	}

	if (!m_imgList.GetImageCount()) // 如果m_list是空的，那么除了“刷新”，其他全部置灰
	{
		for (int i = 2; i < 33; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!hItem)
		{
			for (int i = 2; i < 33; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
			
			menu.EnableMenuItem(ID_LOOK_WINDOW_FOR_PROCESS, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PROCESS_FIND_MODULE, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PROCESS_FIND_UNSIGNED_MODUES, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{
			SUSPEND_OR_RESUME_TYPE SorType = IsSuspendOrResumeProcessTree();
			switch (SorType)
			{
			case enumAllFalse:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				break;

			case enumAllOk:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				break;

			case enumResume:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				break;

			case enumSuspend:
				menu.EnableMenuItem(ID_SUSPEND_PROCESS, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_RESUME_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				break;
			}
		}
	}
	
	menu.EnableMenuItem(ID_SHOW_PROCESS_TREE, MF_BYCOMMAND | MF_ENABLED);

	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

//
// 进程树模式下显示进程详细信息
//
void CListProcessDlg::ShowProcessTreeDetailInfo(HTREEITEM hItem)
{
	if (!hItem)
	{
		return;
	}

	PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
	if (!pInfo)
	{
		return;
	}

	DebugLog(L"new DblclkColumntree");

	CProcessInfoDlg ProcessInfoDlg;
	ProcessInfoDlg.m_nPid = pInfo->ulPid;
	ProcessInfoDlg.m_pEprocess = pInfo->ulEprocess;
	ProcessInfoDlg.m_szImageName = m_processTree.GetItemText(hItem, eProcessTreeImageName);
	ProcessInfoDlg.m_szPath = pInfo->szPath;
	ProcessInfoDlg.m_szParentImage = GetParentProcessInfo(pInfo->ulParentPid);
	ProcessInfoDlg.DoModal();
}

//
// 树形控件双击弹出进程详细信息
//
void CListProcessDlg::DblclkColumntree(HTREEITEM hItem)
{
	ShowProcessTreeDetailInfo(hItem);
}

//
// 右键菜单 - 显示进程树
//
void CListProcessDlg::OnShowProcessTree()
{
	m_bShowAsTreeList = !m_bShowAsTreeList;
	AdjustShowWindow();
	
	((CSetConfigDlg*)g_pSetConfigDlg)->m_bShowProcessTreeAsDeafult = m_bShowAsTreeList;
	((CSetConfigDlg*)g_pSetConfigDlg)->UpdateData(FALSE);

	OnProcessRefresh();
}

//
// 右键菜单 - 是否选中显示进程树
//
void CListProcessDlg::OnUpdateShowProcessTree(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAsTreeList);
}

//
// 必须的系统函数
//
void CListProcessDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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
// 遍历链表，递归结束子进程
//
void CListProcessDlg::KillSubProcess(DWORD dwPid)
{
	for ( vector<PROCESS_INFO_EX>::iterator Iter = m_vectorProcessEx.begin( ); 
		Iter != m_vectorProcessEx.end( ); 
		Iter++)
	{
		if (Iter->ulParentPid == dwPid)
		{
			KillSubProcess(Iter->ulPid);
			m_clsProcess.KillProcess(0, Iter->ulEprocess);
		}
	}
}

//
// 结束进程树
//
void CListProcessDlg::OnKillProcessTree()
{
	HTREEITEM hItem = GetTreeListSelectItem();
	if (!hItem)
	{
		return;
	}
	
	// 弹框确认
	CString szImage = m_processTree.GetItemText(hItem, eProcessTreeImageName);
	CString szShow;
	szShow.Format(szAreYouSureKillDescendants[g_enumLang], szImage);
	if (MessageBox(szShow, szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}
	
	PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
	if (!pInfo)
	{
		return;
	}

	KillSubProcess(pInfo->ulPid);
	m_clsProcess.KillProcess(0, pInfo->ulEprocess);
	
	Sleep(500);
	OnProcessRefresh();
}

//
// 远程注入模块
//
void CListProcessDlg::OnInjectDll()
{
	ULONG nPid = 0;

	if (m_bShowAsTreeList)
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem == NULL)
		{
			return;
		}
		
		PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
		if (!pInfo)
		{
			return;
		}

		nPid = pInfo->ulPid;
	}
	else
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem == -1)
		{
			return;
		}
		
		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return;
		}

		nPid = pInfo->ulPid;
	}
	
	if (nPid == 0)
	{
		return;
	}
	
	/*L"DLL Files(*.dll)\0*.dll\0All Files(*.*)\0*.*\0\0"*/
	WCHAR szDLLFilter[] = {'D','L','L',' ','F','i','l','e','s','(','*','.','d','l','l',')','\0','*','.','d','l','l','\0','A','l','l',' ','F','i','l','e','s','(','*','.','*',')','\0','*','.','*','\0','\0','\0'};
	WCHAR szInjectDLL[] = {'I','n','j','e','c','t',' ','D','L','L','\0'};
	CFileDialog fileDlg(TRUE);			
	fileDlg.m_ofn.lpstrTitle = szInjectDLL;
	fileDlg.m_ofn.lpstrFilter = szDLLFilter;

	if (IDOK != fileDlg.DoModal())
	{
		return;
	}

	CString strPath = fileDlg.GetPathName();
	if (PathFileExists(strPath))
	{
		WCHAR szPath[MAX_PATH] = {0};
		wcsncpy_s(szPath, strPath.GetBuffer(), strPath.GetLength());
		strPath.ReleaseBuffer();

		if (!m_clsProcess.InjectDllByRemoteThread(szPath, nPid))
		{
			::MessageBox(NULL, szInjectDllModuleFailed[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			::MessageBox(NULL, szInjectDllModuleOk[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
		}
	}	
}

//
// 调整界面的展示,即什么时候该显示哪些控件
//
void CListProcessDlg::AdjustShowWindow()
{
	// 如果是以树形控件显示
	if (m_bShowAsTreeList)
	{
		m_processTree.ShowWindow(TRUE);
		m_processList.ShowWindow(FALSE);

		m_ySplitter.AttachAsAbovePane(IDC_PROCESS_TREE);
		m_ySplitter.AttachAsBelowPane(IDC_TAB);
		m_ySplitter.RecalcLayout();
	}
	
	else
	{
		m_processList.ShowWindow(TRUE);
		m_processTree.ShowWindow(FALSE);
		
		m_ySplitter.AttachAsAbovePane(IDC_LIST_PROCESS);
		m_ySplitter.AttachAsBelowPane(IDC_TAB);
		m_ySplitter.RecalcLayout();
	}
}

//
// 枚举进程模块
//
void CListProcessDlg::EnumProcessModules(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nTotal = 0, nNotMicro = 0;
	m_ModuleDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_ModuleDlg.EnumModules(&nTotal, &nNotMicro);

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);
	
	CString szModuleStatus;
	szModuleStatus.Format(szProcessModuleState[g_enumLang], szImage, nNotMicro, nTotal);
	
	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 枚举句柄
//
void CListProcessDlg::EnumProcessHandles(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}
	
	ULONG nCnt = 0;
	m_HandleDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_HandleDlg.EnumProcessHandles(&nCnt);

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);

	CString szModuleStatus;
	szModuleStatus.Format(szProcessHandleState[g_enumLang], szImage, nCnt);
	
	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 枚举线程
//
void CListProcessDlg::EnumProcessThreads(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}
	
	ULONG nCnt = 0;
	m_ThreadDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_ThreadDlg.EnumThreads(&nCnt);

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);

	CString szModuleStatus;
	szModuleStatus.Format(szProcessThreadState[g_enumLang], szImage, nCnt);

	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 枚举进程内存
//
void CListProcessDlg::EnumProcessMemorys(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_MemoryDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_MemoryDlg.EnumMemorys(&nCnt);

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);

	CString szModuleStatus;
	szModuleStatus.Format(szProcessMemoryState[g_enumLang], szImage, nCnt);

	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);

}

//
// 枚举窗口
//
void CListProcessDlg::EnumProcessWnds(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_WndDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_WndDlg.EnumProcessWnds(&nCnt);

	if (m_WndDlg.m_bShowAllProcess)
	{
		szImage = szAllProcesses[g_enumLang];
	}

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);

	CString szModuleStatus;
	szModuleStatus.Format(szProcessWindowsState[g_enumLang], szImage, nCnt);

	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 枚举热键
//
void CListProcessDlg::EnumProcessHotKeys(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_HotKeyDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_HotKeyDlg.EnumHotKeys(&nCnt);

	if (m_HotKeyDlg.m_bShowAll)
	{
		szImage = szAllProcesses[g_enumLang];
	}

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);

	CString szModuleStatus;
	szModuleStatus.Format(szProcessHotKeysState[g_enumLang], szImage, nCnt);

	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 枚举定时器
//
void CListProcessDlg::EnumProcessTimers(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}
	
	ULONG nCnt = 0;
	m_TimerDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_TimerDlg.EnumProcessTimers(&nCnt);
	
	if (m_TimerDlg.m_bShowAllTimer)
	{
		szImage = szAllProcesses[g_enumLang];
	}

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);

	CString szModuleStatus;
	szModuleStatus.Format(szProcessTimersState[g_enumLang], szImage, nCnt);

	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 获取当前进程的pid, eprocess, 和image name
//
CString CListProcessDlg::GetCurrentProcessInfo(ULONG &nPid, ULONG &pEprocess)
{
	CString szImage = L"";

	if (!m_bShowAsTreeList)
	{
		if (m_processList.GetSelectedCount() == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_processList);
			if (nItem != -1)
			{
				PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
				if (pInfo)
				{
					nPid = pInfo->ulPid;
					pEprocess = pInfo->ulEprocess;
					szImage = m_processList.GetItemText(nItem, eProcessImageName);
				}
			}
		}
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
			if (pInfo)
			{
				nPid = pInfo->ulPid;
				pEprocess = pInfo->ulEprocess;
				szImage = m_processTree.GetItemText(hItem, eProcessTreeImageName);
			}
		}
	}

	return szImage; 
}

//
// 下面的刷新,会发送消息来枚举定时器信息
//
LRESULT CListProcessDlg::MsgEnumTimer(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessTimers(nPid, pEprocess, szImage);
	return 0;
}

LRESULT CListProcessDlg::MsgEnumModules(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessModules(nPid, pEprocess, szImage);
	return 0;
}

LRESULT CListProcessDlg::MsgEnumHandles(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessHandles(nPid, pEprocess, szImage);
	return 0;
}

LRESULT CListProcessDlg::MsgEnumThreads(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessThreads(nPid, pEprocess, szImage);
	return 0;
}

LRESULT CListProcessDlg::MsgEnumMemory(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessMemorys(nPid, pEprocess, szImage);
	return 0;
}

LRESULT CListProcessDlg::MsgEnumWindows(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessWnds(nPid, pEprocess, szImage);
	return 0;
}

LRESULT CListProcessDlg::MsgEnumHotKeys(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessHotKeys(nPid, pEprocess, szImage);
	return 0;
}

LRESULT CListProcessDlg::MsgEnumPrivileges(WPARAM wParam, LPARAM lParam)
{
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	EnumProcessPrivileges(nPid, pEprocess, szImage);
	return 0;
}

//
// 枚举进程权限
//
void CListProcessDlg::EnumProcessPrivileges(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nEnable = 0, nDisable = 0;
	m_PrivilegeDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_PrivilegeDlg.GetProcessPrivileges(&nEnable, &nDisable);

	// 更新状态
	CString szText;
	szText.Format(szProcessState[g_enumLang], m_nProcessCnt, m_nHideCnt, m_nDenyAccessCnt);

	CString szModuleStatus;
	szModuleStatus.Format(szProcessPrivilegesState[g_enumLang], szImage, nEnable, nDisable);

	m_szProcessCnt = szText + szModuleStatus;
	SendMessage(WM_UPDATE_PROCESS_DATA);
}

//
// 进程列表模式下更改选中的进程项
//
static int preListItem = 0;
void CListProcessDlg::OnLvnItemchangedProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
// 	if (!m_bShowBelow)
// 	{
// 		*pResult = 0;
// 		return;
// 	}
// 
// 	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
// 	
// 	int nItem = pNMLV->iItem;
// 	if ( m_processList.GetSelectedCount() == 1 && nItem != -1 && nItem != preListItem ) // 选中一个的时候才枚举
// 	{
// 		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
// 		if (!pInfo)
// 		{
// 			return;
// 		}
// 
// 		CString szImage = m_processList.GetItemText(nItem, eProcessImageName);
// 		ULONG nPid = pInfo->ulPid;
// 		ULONG pEprocess = pInfo->ulEprocess;
// 
// 		switch (m_nCurSel)
// 		{
// 		case eDllModuleDlg:
// 			EnumProcessModules(nPid, pEprocess, szImage);
// 			break;
// 
// 		case eHandleDlg:
// 			EnumProcessHandles(nPid, pEprocess, szImage);
// 			break;
// 		}
// 		
// 		preListItem = nItem;
// 	}

	*pResult = 0;
}

static HTREEITEM preTreeItem = NULL;
void CListProcessDlg::OnTvnSelchangedTreeList(NMHDR *pNMHDR, LRESULT *pResult)
{
// 	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
// 
// 	if (!m_bShowBelow)
// 	{
// 		*pResult = 0;
// 		return;
// 	}
// 	
// 	HTREEITEM hItem = GetTreeListSelectItem();
// 	if (hItem && hItem != preTreeItem)
// 	{
// 		PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
// 		if (!pInfo)
// 		{
// 			return;
// 		}
// 
// 		CString szImage = m_processTree.GetItemText(hItem, eProcessTreeImageName);
// 		ULONG nPid = pInfo->ulPid;
// 		ULONG pEprocess = pInfo->ulEprocess;
// 
// 		switch (m_nCurSel)
// 		{
// 		case eDllModuleDlg:
// 			EnumProcessModules(nPid, pEprocess, szImage);
// 			break;
// 
// 		case eHandleDlg:
// 			EnumProcessHandles(nPid, pEprocess, szImage);
// 			break;
// 		}
// 
// 		preTreeItem = hItem;
// 	}

	*pResult = 0;
}

//
// 定位到AntiSpy文件管理器
//
void CListProcessDlg::OnProcessLocationAtFileManager()
{
	CString szPath = L"";

	if (!m_bShowAsTreeList)
	{
		int nItem = m_Functions.GetSelectItem(&m_processList);
		if (nItem == -1)
		{
			return;
		}

		szPath = m_processList.GetItemText(nItem, eProcessPath);
	}
	else
	{
		HTREEITEM hItem = GetTreeListSelectItem();
		if (hItem)
		{
			szPath = m_processTree.GetItemText(hItem, eProcessTreePath);
		}
	}

	m_Functions.JmpToFile(szPath);
}

//
// 发送自定义消息WM_UPDATE_PROCESS_DATA来更新界面上的信息
//
LRESULT CListProcessDlg::OnUpdateProcessInfoStatus(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return 0;
}

//
// 右键菜单 - 根据窗口查找进程
//
void CListProcessDlg::OnLookWindowForProcess()
{
	WCHAR szFind[] = {'m','Z','f','F','i','n','d','W','n','d','\0'};
	HANDLE hMutex = CreateMutex(NULL, TRUE, szFind);
	if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS) 
	{
		CloseHandle(hMutex);
		return;
	} 

	if (hMutex)
	{
		CloseHandle(hMutex);
		hMutex = NULL;
	}

	CFindWindow *m_pFindRegDlg = new CFindWindow();
	m_pFindRegDlg->Create(IDD_FIND_WINDOW_DIALOG);
	m_pFindRegDlg->ShowWindow(SW_SHOWNORMAL);
}

//
// 根据鼠标指针找到窗口所在的PID所对应的进程
//
void CListProcessDlg::GotoProcess(DWORD dwPid)
{
	if (!dwPid)
	{
		return;
	}

	// 如果是列表框模式
	if (!m_bShowAsTreeList)
	{
		CString szPid;
		szPid.Format(L"%d", dwPid);

		// 循环进程列表，把先前选中的项都设为未选中
		POSITION pos = m_processList.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = m_processList.GetNextSelectedItem(pos);
			m_processList.SetItemState(nItem, 0, -1);
		}

		DWORD dwCnt = m_processList.GetItemCount();
		for (DWORD i = 0; i < dwCnt; i++)
		{
			// 比较PID是否一致，如果一致，那么就设置新的项为选中
			if (!(m_processList.GetItemText(i, eProcessPid)).CompareNoCase(szPid))
			{
				m_processList.EnsureVisible(i, FALSE);
				m_processList.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				m_processList.SetFocus();
				break;
			}
		}
	}
	else
	{
		for (vector<PROCESS_INFO_EX>::iterator ir = m_vectorProcessEx.begin();
			ir != m_vectorProcessEx.end();
			ir++)
		{
			if (ir->ulPid == dwPid)
			{
				m_processTree.GetTreeCtrl().Select(ir->hItem, TVGN_FIRSTVISIBLE);
				m_processTree.GetTreeCtrl().SelectItem(ir->hItem);
				m_processTree.GetTreeCtrl().SetFocus();
				break;
			}
		}
	}
}

//
// 根据鼠标指针找到窗口所在的TID所对应的线程
//
void CListProcessDlg::GotoThread(DWORD dwPid, DWORD dwTid)
{
	// 检查下参数
	if (!dwPid || !dwTid)
	{
		return;
	}

	// 首先定位下进程
	GotoProcess(dwPid);

	// 开始切换TAB
	NMHDR nm; 
	nm.hwndFrom = m_tab.m_hWnd; 
	nm.code = TCN_SELCHANGE;
	m_tab.SetCurSel(eThreadDlg); 
	m_tab.SendMessage(WM_NOTIFY, IDC_TAB_MAIN, (LPARAM)&nm);
	
	//
	// 定位到相应的线程
	//
	CString szTid;
	szTid.Format(L"%d", dwTid);

	// 循环进程列表，把先前选中的项都设为未选中
	POSITION pos = m_ThreadDlg.m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_ThreadDlg.m_list.GetNextSelectedItem(pos);
		m_ThreadDlg.m_list.SetItemState(nItem, 0, -1);
	}

	DWORD dwCnt = m_ThreadDlg.m_list.GetItemCount();
	for (DWORD i = 0; i < dwCnt; i++)
	{
		// 比较PID是否一致，如果一致，那么就设置新的项为选中
		if (!(m_ThreadDlg.m_list.GetItemText(i, 0)).CompareNoCase(szTid))
		{
			m_ThreadDlg.m_list.EnsureVisible(i, FALSE);
			m_ThreadDlg.m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			m_ThreadDlg.m_list.SetFocus();
			break;
		}
	}
}

//
// 根据PID结束相应的进程，并删除相应的项
//
void CListProcessDlg::KillProcessByFindWindow(DWORD dwPid)
{
	if (!dwPid)
	{
		return;
	}
	
	if (!m_bShowAsTreeList)
	{
		CString szPid;
		szPid.Format(L"%d", dwPid);

		DWORD dwCnt = m_processList.GetItemCount();
		for (DWORD i = 0; i < dwCnt; i++)
		{
			if (!(m_processList.GetItemText(i, eProcessPid)).CompareNoCase(szPid))
			{
				PPROCESS_INFO pInfo = GetProcessInfoByItem(i);
				if (pInfo)
				{
					m_clsProcess.KillProcess(0, pInfo->ulEprocess);
					m_processList.DeleteItem(i);
				}

				break;
			}
		}
	}
	else
	{
		HTREEITEM hPre = GetTreeListSelectItem();
		PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hPre);
		if (pInfo)
		{
			m_clsProcess.KillProcess(0, pInfo->ulEprocess);

			// 如果有子项，那么刷新下进程树
			if (m_processTree.GetTreeCtrl().ItemHasChildren(hPre))
			{
				Sleep(500);
				OnProcessRefresh();
			}
			else
			{
				m_processTree.GetTreeCtrl().DeleteItem(hPre);
			}
		}
	}
}

//
// 分割栏再被拖动的时候会发送一个自定义WM_RESIZE_ALL_PROC_WND消息，
// 用来重新布局下面的窗口
//
LRESULT CListProcessDlg::ResizeAllProcWnd(WPARAM wParam, LPARAM lParam)
{
	UpdateBelowDlg();
	return 0;
}

//
// 下面的进程信息tab发生切换
//
void CListProcessDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_pBelowDialog[m_nCurSel]->ShowWindow(SW_HIDE);
	m_nCurSel = (BELOW_DLG_INDEX)m_tab.GetCurSel();
	m_pBelowDialog[m_nCurSel]->ShowWindow(SW_SHOW);
	
	ULONG nPid = 0, pEprocess = 0;
	CString szImage = GetCurrentProcessInfo(nPid, pEprocess);
	
	switch (m_nCurSel)
	{
	case eDllModuleDlg:
		EnumProcessModules(nPid, pEprocess, szImage);
		break;

	case eHandleDlg:
		EnumProcessHandles(nPid, pEprocess, szImage);
		break;

	case eThreadDlg:
		EnumProcessThreads(nPid, pEprocess, szImage);
		break;

	case eMemoryDlg:
		EnumProcessMemorys(nPid, pEprocess, szImage);
		break;

	case eWndDlg:
		EnumProcessWnds(nPid, pEprocess, szImage);
		break;

	case eHotKeyDlg:
		EnumProcessHotKeys(nPid, pEprocess, szImage);
		break;

	case eTimerDlg:
		EnumProcessTimers(nPid, pEprocess, szImage);
		break;

	case ePrivilegeDlg:
		EnumProcessPrivileges(nPid, pEprocess, szImage);
		break;
	}

	*pResult = 0;
}

//
// 鼠标左键点击列表框才枚举
//
void CListProcessDlg::OnNMClickListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (!m_bShowBelow)
	{
		*pResult = 0;
		return;
	}

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	int nItem = pNMLV->iItem;
	if ( m_processList.GetSelectedCount() == 1 && nItem != -1 && nItem != preListItem ) // 选中一个的时候才枚举
	{
		PPROCESS_INFO pInfo = GetProcessInfoByItem(nItem);
		if (!pInfo)
		{
			return;
		}

		CString szImage = m_processList.GetItemText(nItem, eProcessImageName);
		ULONG nPid = pInfo->ulPid;
		ULONG pEprocess = pInfo->ulEprocess;

		switch (m_nCurSel)
		{
		case eDllModuleDlg:
			EnumProcessModules(nPid, pEprocess, szImage);
			break;

		case eHandleDlg:
			EnumProcessHandles(nPid, pEprocess, szImage);
			break;

		case eThreadDlg:
			EnumProcessThreads(nPid, pEprocess, szImage);
			break;

		case eMemoryDlg:
			EnumProcessMemorys(nPid, pEprocess, szImage);
			break;

		case eWndDlg:
			EnumProcessWnds(nPid, pEprocess, szImage);
			break;

		case eHotKeyDlg:
			EnumProcessHotKeys(nPid, pEprocess, szImage);
			break;

		case eTimerDlg:
			EnumProcessTimers(nPid, pEprocess, szImage);
			break;

		case ePrivilegeDlg:
			EnumProcessPrivileges(nPid, pEprocess, szImage);
			break;
		}

		preListItem = nItem;
	}

	*pResult = 0;
}

//
// 鼠标左键点击进程树才枚举
//
void CListProcessDlg::OnNMClickTreeList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (!m_bShowBelow)
	{
		*pResult = 0;
		return;
	}
	
	HTREEITEM hItem = GetTreeListSelectItem();
	if (hItem && hItem != preTreeItem)
	{
		PPROCESS_INFO_EX pInfo = GetProcessInfoExByItem(hItem);
		if (!pInfo)
		{
			return;
		}

		CString szImage = m_processTree.GetItemText(hItem, eProcessTreeImageName);
		ULONG nPid = pInfo->ulPid;
		ULONG pEprocess = pInfo->ulEprocess;

		switch (m_nCurSel)
		{
		case eDllModuleDlg:
			EnumProcessModules(nPid, pEprocess, szImage);
			break;

		case eHandleDlg:
			EnumProcessHandles(nPid, pEprocess, szImage);
			break;

		case eThreadDlg:
			EnumProcessThreads(nPid, pEprocess, szImage);
			break;

		case eMemoryDlg:
			EnumProcessMemorys(nPid, pEprocess, szImage);
			break;

		case eWndDlg:
			EnumProcessWnds(nPid, pEprocess, szImage);
			break;

		case eHotKeyDlg:
			EnumProcessHotKeys(nPid, pEprocess, szImage);
			break;

		case eTimerDlg:
			EnumProcessTimers(nPid, pEprocess, szImage);
			break;

		case ePrivilegeDlg:
			EnumProcessPrivileges(nPid, pEprocess, szImage);
			break;
		}

		preTreeItem = hItem;
	}

	*pResult = 0;
}
