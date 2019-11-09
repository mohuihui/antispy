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
// ProcessInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ProcessInfoDlg.h"

/*CDialog *g_pProcessInfoDialog[10];*/

// CProcessInfoDlg 对话框

IMPLEMENT_DYNAMIC(CProcessInfoDlg, CDialog)

CProcessInfoDlg::CProcessInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessInfoDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_nCurSel = 0;
	m_nPid = 0;
	m_pEprocess = 0;
	m_szImageName = L"";
	m_szPath = L"";
	m_szParentImage = L"";
	m_nBelowDialog = 0;
}

CProcessInfoDlg::~CProcessInfoDlg()
{
}

void CProcessInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
	DDX_Text(pDX, IDC_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CProcessInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CProcessInfoDlg::OnBnClickedOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CProcessInfoDlg::OnTcnSelchangeTab)
	ON_MESSAGE(WM_ENUM_TIMER, MsgEnumTimer)
	ON_MESSAGE(WM_ENUM_MODULES, MsgEnumModules)
	ON_MESSAGE(WM_ENUM_HANDLES, MsgEnumHandles)
	ON_MESSAGE(WM_ENUM_THREADS, MsgEnumThreads)
	ON_MESSAGE(WM_ENUM_WINDOWS, MsgEnumWindows)
	ON_MESSAGE(WM_ENUM_HOTKEYS, MsgEnumHotKeys)
	ON_MESSAGE(WM_ENUM_MEMORY, MsgEnumMemory)
	ON_MESSAGE(WM_ENUM_PRIVILEGES, MsgEnumPrivileges)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CProcessInfoDlg)
	EASYSIZE(IDC_TAB, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CProcessInfoDlg 消息处理程序

void CProcessInfoDlg::OnBnClickedOk()
{
}

// 
// 初始化进程信息下面的标签窗口
// 
void CProcessInfoDlg::InitProcInfoWndTab(CDialog *pDlg)
{
	if (!pDlg)
	{
		return;
	}

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

BOOL CProcessInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 初始化窗口标题（进程名 | pid | pEprocess）
	CString szImage;
	szImage.Format(L"%s ( %s | %d | 0x%08X )", szProcessInformation[g_enumLang], m_szImageName, m_nPid, m_pEprocess);
	SetWindowText(szImage);
	
	// 设置程序图标
	SHFILEINFO shInfo;
	memset(&shInfo, 0, sizeof(shInfo));
	SHGetFileInfo(m_szPath, FILE_ATTRIBUTE_NORMAL, &shInfo, sizeof(shInfo), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	SetIcon(shInfo.hIcon, FALSE);

	m_tab.InsertItem(0, szProcessImage[g_enumLang]);
	m_tab.InsertItem(1, szModuleInfo[g_enumLang]);
	m_tab.InsertItem(2, szHandleInfo[g_enumLang]);
	m_tab.InsertItem(3, szThreadInfo[g_enumLang]);
	m_tab.InsertItem(4, szMemoryInfo[g_enumLang]);
	m_tab.InsertItem(5, szWindowInfo[g_enumLang]);
	m_tab.InsertItem(6, szHotKeyInfo[g_enumLang]);
	m_tab.InsertItem(7, szTimerInfo[g_enumLang]);
	m_tab.InsertItem(8, szPrivilegeInfo[g_enumLang]);
	
	m_ProcessImageInfoDlg.Create(IDD_PROCESS_IMAGE_DIALOG, &m_tab);
	m_ModuleDlg.Create(IDD_DLL_MODULE_DIALOG, &m_tab);
	m_HandleDlg.Create(IDD_HANDLE_DIALOG, &m_tab);
	m_ThreadDlg.Create(IDD_THREAD_DIALOG, &m_tab);
	m_MemoryDlg.Create(IDD_MEMORY_DIALOG, &m_tab);
	m_WndDlg.Create(IDD_PROCESS_WND_DIALOG, &m_tab);
	m_HotKeyDlg.Create(IDD_HOTKEY_DIALOG, &m_tab);
	m_TimerDlg.Create(IDD_PROCESS_TIMER_DIALOG, &m_tab);
	m_PrivilegeDlg.Create(IDD_PROCESS_PRIVILEGES_DIALOG, &m_tab);

	InitProcInfoWndTab(&m_ProcessImageInfoDlg);
	InitProcInfoWndTab(&m_ModuleDlg);
	InitProcInfoWndTab(&m_HandleDlg);
	InitProcInfoWndTab(&m_ThreadDlg);
	InitProcInfoWndTab(&m_MemoryDlg);
	InitProcInfoWndTab(&m_WndDlg);
	InitProcInfoWndTab(&m_HotKeyDlg);
	InitProcInfoWndTab(&m_TimerDlg);
	InitProcInfoWndTab(&m_PrivilegeDlg);
 
	m_ProcessImageInfoDlg.m_szParent = m_szParentImage;
	m_ProcessImageInfoDlg.m_dwPid = m_nPid;
	m_ProcessImageInfoDlg.m_szPath = m_szPath;
	m_ProcessImageInfoDlg.ShowInformation();

	GetDlgItem(IDC_STATUS)->ShowWindow(FALSE);

	// 当前选择
	m_nCurSel = 0;
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// 枚举进程模块
//
static BOOL bFirstEnum = TRUE;
void CProcessInfoDlg::EnumProcessModules(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}
	
	ULONG nTotal = 0, nNotMicro = 0;

	if (bFirstEnum)
	{
		m_ModuleDlg.m_bNotShowMicroModule = FALSE;
		bFirstEnum = FALSE;
	}
	
	m_ModuleDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_ModuleDlg.EnumModules(&nTotal, &nNotMicro);
	
	// 更新状态
	WCHAR *szProcessModuleState[] = {
		L"Modules: %d/%d",
		L"模块：%d/%d"
	};

	m_szStatus.Format(szProcessModuleState[g_enumLang], nNotMicro, nTotal);
	UpdateData(FALSE);
}

//
// 枚举句柄
//
void CProcessInfoDlg::EnumProcessHandles(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_HandleDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_HandleDlg.EnumProcessHandles(&nCnt);

	// 更新状态
	WCHAR *szProcessHandleState[] = {
		L"Handles: %d",
		L"句柄：%d"
	};

	m_szStatus.Format(szProcessHandleState[g_enumLang], nCnt);
	UpdateData(FALSE);
}

//
// 枚举线程
//
void CProcessInfoDlg::EnumProcessThreads(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_ThreadDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_ThreadDlg.EnumThreads(&nCnt);

	// 更新状态 
	WCHAR *szProcessThreadState[] = {
		L"Threads: %d",
		L"线程：%d"
	};

 	m_szStatus.Format(szProcessThreadState[g_enumLang], nCnt);
	UpdateData(FALSE);
}

//
// 枚举进程内存
//
void CProcessInfoDlg::EnumProcessMemorys(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_MemoryDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_MemoryDlg.EnumMemorys(&nCnt);

	// 更新状态
	WCHAR *szProcessMemoryState[] = {
		L"Memorys: %d",
		L"内存：%d"
	};
	
 	m_szStatus.Format(szProcessMemoryState[g_enumLang], nCnt);
	UpdateData(FALSE);
}

//
// 枚举窗口
//
void CProcessInfoDlg::EnumProcessWnds(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_WndDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_WndDlg.EnumProcessWnds(&nCnt);
	
	// 更新状态
	if (m_WndDlg.m_bShowAllProcess)
	{
		szImage = szAllProcesses[g_enumLang];
	}

	WCHAR *szProcessWindowsState[] = {
		L"[%s] Windows: %d",
		L"[%s] 窗口：%d"
	};

 	m_szStatus.Format(szProcessWindowsState[g_enumLang], szImage, nCnt);
	UpdateData(FALSE);
}

//
// 枚举热键
//
void CProcessInfoDlg::EnumProcessHotKeys(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_HotKeyDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_HotKeyDlg.EnumHotKeys(&nCnt);
	
	// 更新状态
	if (m_HotKeyDlg.m_bShowAll)
	{
		szImage = szAllProcesses[g_enumLang];
	}

	WCHAR *szProcessHotKeysState[] = {
		L"[%s] HotKeys: %d",
		L"[%s] 热键：%d"
	};

 	m_szStatus.Format(szProcessHotKeysState[g_enumLang], szImage, nCnt);
	UpdateData(FALSE);
}

//
// 枚举定时器
//
void CProcessInfoDlg::EnumProcessTimers(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nCnt = 0;
	m_TimerDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_TimerDlg.EnumProcessTimers(&nCnt);

	// 更新状态
	if (m_TimerDlg.m_bShowAllTimer)
	{
		szImage = szAllProcesses[g_enumLang];
	}

	WCHAR *szProcessTimersState[] = {
		L"[%s] Timers: %d",
		L"[%s] 定时器：%d"
	};

 	m_szStatus.Format(szProcessTimersState[g_enumLang], szImage, nCnt);
	UpdateData(FALSE);
}


//
// 枚举进程权限
//
void CProcessInfoDlg::EnumProcessPrivileges(ULONG nPid, ULONG pEprocess, CString szImage)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return;
	}

	ULONG nEnable = 0, nDisable = 0;
	m_PrivilegeDlg.SetProcessInfo(nPid, pEprocess, szImage, this->m_hWnd);
	m_PrivilegeDlg.GetProcessPrivileges(&nEnable, &nDisable);

	// 更新状态
	WCHAR *szProcessPrivilegesState[] = {
		L"Enable: %d, Disable: %d",
		L"Enable：%d，Disable：%d"
	};

 	m_szStatus.Format(szProcessPrivilegesState[g_enumLang], nEnable, nDisable);
	UpdateData(FALSE);
}

void CProcessInfoDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_pBelowDialog[m_nCurSel]->ShowWindow(SW_HIDE);
	m_nCurSel = m_tab.GetCurSel();
	m_pBelowDialog[m_nCurSel]->ShowWindow(SW_SHOW);

	ULONG nPid = m_nPid;
	ULONG pEprocess = m_pEprocess;
	CString szImage = m_szImageName;

	GetDlgItem(IDC_STATUS)->ShowWindow(TRUE);

	switch (m_nCurSel)
	{
	case 0:
		GetDlgItem(IDC_STATUS)->ShowWindow(FALSE);
		break;

	case 1:
		EnumProcessModules(nPid, pEprocess, szImage);
		break;

	case 2:
		EnumProcessHandles(nPid, pEprocess, szImage);
		break;

	case 3:
		EnumProcessThreads(nPid, pEprocess, szImage);
		break;

	case 4:
		EnumProcessMemorys(nPid, pEprocess, szImage);
		break;

	case 5:
		EnumProcessWnds(nPid, pEprocess, szImage);
		break;

	case 6:
		EnumProcessHotKeys(nPid, pEprocess, szImage);
		break;

	case 7:
		EnumProcessTimers(nPid, pEprocess, szImage);
		break;

	case 8:
		EnumProcessPrivileges(nPid, pEprocess, szImage);
		break;
	}

	*pResult = 0;
}

void CProcessInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	UPDATE_EASYSIZE;

	if (m_tab.m_hWnd && m_ModuleDlg.GetSafeHwnd())
	{
		CRect rc;
		m_tab.GetClientRect(rc);
		rc.top += 20;
		rc.bottom -= 2;
		rc.left += 0;
		rc.right -= 2;

		for (ULONG i = 0; i < m_nBelowDialog; i++)
		{
			m_pBelowDialog[i]->MoveWindow(&rc);
		}
	}
}

//
// 下面的刷新,会发送消息来枚举定时器信息
//
LRESULT CProcessInfoDlg::MsgEnumTimer(WPARAM wParam, LPARAM lParam)
{
	EnumProcessTimers(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}

LRESULT CProcessInfoDlg::MsgEnumModules(WPARAM wParam, LPARAM lParam)
{
	EnumProcessModules(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}

LRESULT CProcessInfoDlg::MsgEnumHandles(WPARAM wParam, LPARAM lParam)
{
	EnumProcessHandles(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}

LRESULT CProcessInfoDlg::MsgEnumThreads(WPARAM wParam, LPARAM lParam)
{
	EnumProcessThreads(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}

LRESULT CProcessInfoDlg::MsgEnumMemory(WPARAM wParam, LPARAM lParam)
{
	EnumProcessMemorys(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}

LRESULT CProcessInfoDlg::MsgEnumWindows(WPARAM wParam, LPARAM lParam)
{
	EnumProcessWnds(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}

LRESULT CProcessInfoDlg::MsgEnumHotKeys(WPARAM wParam, LPARAM lParam)
{
	EnumProcessHotKeys(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}

LRESULT CProcessInfoDlg::MsgEnumPrivileges(WPARAM wParam, LPARAM lParam)
{
	EnumProcessPrivileges(m_nPid, m_pEprocess, m_szImageName);
	return 0;
}