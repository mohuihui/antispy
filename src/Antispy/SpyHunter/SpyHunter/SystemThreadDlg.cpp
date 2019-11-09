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
// SystemThreadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "SystemThreadDlg.h"
#include <algorithm>

typedef enum _SYSTEM_THREAD_HEADER_INDEX
{
	eSystemThreadId,
	eSystemThreadObject,
	eSystemThreadPriority,
	eSystemThreadStartAddress,
	eSystemThreadSwitchTimes,
	eSystemThreadStatus,
	eSystemThreadStartModule,
	eSystemThreadFileCorporation
}SYSTEM_THREAD_HEADER_INDEX;

// CSystemThreadDlg 对话框

IMPLEMENT_DYNAMIC(CSystemThreadDlg, CDialog)

CSystemThreadDlg::CSystemThreadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSystemThreadDlg::IDD, pParent)
	, m_szState(_T(""))
{
	m_nCnt = 0;
	m_SystemProcessPid = 0;
	m_SystemProcessEPROCESS = 0;
}

CSystemThreadDlg::~CSystemThreadDlg()
{
	m_vectorThreadsTemp.clear();
}

void CSystemThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_SYSTEM_THREAD_CNT, m_szState);
}


BEGIN_MESSAGE_MAP(CSystemThreadDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CSystemThreadDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CSystemThreadDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_THREAD_REFRESH, &CSystemThreadDlg::OnThreadRefresh)
	ON_COMMAND(ID_KILL_THREAD, &CSystemThreadDlg::OnKillThread)
	ON_COMMAND(ID_SUSPEND_THREAD, &CSystemThreadDlg::OnSuspendThread)
	ON_COMMAND(ID_RESUME_THREAD, &CSystemThreadDlg::OnResumeThread)
	ON_COMMAND(ID_FORCE_RESUME_THREAD, &CSystemThreadDlg::OnDissmThreadStartAddress)
	ON_COMMAND(ID_CHECK_SIGN, &CSystemThreadDlg::OnCheckSign)
	ON_COMMAND(ID_CHECK_ATTRIBUTE, &CSystemThreadDlg::OnCheckAttribute)
	ON_COMMAND(ID_LOCATION_EXPLORER, &CSystemThreadDlg::OnLocationExplorer)
	ON_COMMAND(ID_EXPORT_TEXT, &CSystemThreadDlg::OnExportText)
	ON_COMMAND(ID_EXPORT, &CSystemThreadDlg::OnExport)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CSystemThreadDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CSystemThreadDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_SYSTEM_THREAD_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CSystemThreadDlg 消息处理程序

void CSystemThreadDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

// 
// 初始化对话框
//
BOOL CSystemThreadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_list.InsertColumn(eSystemThreadId, szThreadId[g_enumLang], LVCFMT_LEFT, 70);
	m_list.InsertColumn(eSystemThreadObject, szThreadObject[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(eSystemThreadPriority, szPriority[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(eSystemThreadStartAddress, szStartAddress[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(eSystemThreadSwitchTimes, szSwitchTimes[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eSystemThreadStatus, szThreadStatus[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(eSystemThreadStartModule, szStartModule[g_enumLang], LVCFMT_LEFT, 390);
	m_list.InsertColumn(eSystemThreadFileCorporation, szFileCorporation[g_enumLang], LVCFMT_LEFT, 180);
	
	m_szState.Format(szSystemThreadState[g_enumLang], 0);
	UpdateData(FALSE);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// 枚举系统线程
//
BOOL CSystemThreadDlg::ListProcessThread()
{
	m_szState.Format(szSystemThreadState[g_enumLang], 0);
	UpdateData(FALSE);

	// 获得系统进程的pid
	if (!m_SystemProcessEPROCESS)
	{
		if (!GetSystemProcessInfo())
		{
			return FALSE;
		}
	}

	if (!m_SystemProcessEPROCESS)
	{
		return FALSE;
	}

	// 清零
	m_list.DeleteAllItems();
	m_nCnt = 0;
	m_vectorThreadsTemp.clear();
	m_vectorDrivers.clear();
	m_vectorThreads.clear();

	// 枚举线程
	m_clsThreads.ListThreads(m_SystemProcessPid, m_SystemProcessEPROCESS, m_vectorThreadsTemp);
	if (m_vectorThreadsTemp.empty())
	{
		return TRUE;
	}

	// 根据tid排序
	SortByTid();

	// 枚举驱动模块
	m_clsDrivers.ListDrivers(m_vectorDrivers);

	// 插入信息
	for ( vector <THREAD_INFO>::iterator Iter = m_vectorThreadsTemp.begin( ); 
		Iter != m_vectorThreadsTemp.end( ); 
		Iter++ )
	{
		AddThreadItem(*Iter);
	}

	m_szState.Format(szSystemThreadState[g_enumLang], m_nCnt);
	UpdateData(FALSE);

	return TRUE;
}

//
// 获取系统进程的pid,和EPROCESS
//
BOOL CSystemThreadDlg::GetSystemProcessInfo()
{
	BOOL bRet = FALSE;
	vector<PROCESS_INFO> vectorProcess;
	BOOL bRetTemp = m_clsEnumProcess.EnumProcess(vectorProcess);

	if (bRetTemp && vectorProcess.size() > 0)
	{
		for (vector<PROCESS_INFO>::iterator ir = vectorProcess.begin();
			ir != vectorProcess.end();
			ir++)
		{
			WCHAR szSystem[] = {'S', 'y', 's', 't', 'e', 'm', '\0'};
			if (!_wcsnicmp(ir->szPath, szSystem, wcslen(szSystem)))
			{
				m_SystemProcessPid = ir->ulPid;
				m_SystemProcessEPROCESS = ir->ulEprocess;
				bRet = TRUE;
				break;
			}
		}
	}
	
	return bRet;
}

//
// 插入线程信息
//
VOID CSystemThreadDlg::AddThreadItem(THREAD_INFO ThreadInfo)
{
	if (ThreadInfo.State == Terminated)
	{
		return;
	}

	CString szTid, szEthread, szTeb, szPriority, szWin32StartAddress, szContextSwitches, szState, szModule;

	szTid.Format(L"%d", ThreadInfo.Tid);
	szEthread.Format(L"0x%08X", ThreadInfo.pThread);
	szTeb.Format(L"0x%08X", ThreadInfo.Teb);
	szPriority.Format(L"%d", ThreadInfo.Priority);
	szWin32StartAddress.Format(L"0x%08X", ThreadInfo.Win32StartAddress);
	szContextSwitches.Format(L"%d", ThreadInfo.ContextSwitches);
	szModule = GetDriverPathByCallbackAddress(ThreadInfo.Win32StartAddress);

	switch (ThreadInfo.State)
	{
	case Initialized:
		szState = L"Initialized";
		break;

	case Ready:
		szState = L"Ready";
		break;

	case Running:
		szState = L"Running";
		break;

	case Standby:
		szState = L"Standby";
		break;

	case Terminated:
		szState = L"Terminated";
		break;

	case Waiting:
		szState = L"Waiting";
		break;

	case Transition:
		szState = L"Transition";
		break;

	case DeferredReady:
		szState = L"Deferred Ready";
		break;

	case GateWait:
		szState = L"Gate Wait";
		break;

	default:
		szState = L"UnKnown";
		break;
	}

	int n = m_list.InsertItem(m_list.GetItemCount(), szTid);
	m_list.SetItemText(n, eSystemThreadObject, szEthread);
	m_list.SetItemText(n, eSystemThreadPriority, szPriority);
	m_list.SetItemText(n, eSystemThreadStartAddress, szWin32StartAddress);
	m_list.SetItemText(n, eSystemThreadSwitchTimes, szContextSwitches);
	m_list.SetItemText(n, eSystemThreadStatus, szState);
	m_list.SetItemText(n, eSystemThreadStartModule, szModule);
	m_list.SetItemText(n, eSystemThreadFileCorporation, m_Functions.GetFileCompanyName(szModule));

	m_vectorThreads.push_back(ThreadInfo);
	m_list.SetItemData(n, n);
	m_nCnt++;
}

//
// 根据线程的起始地址,找到相应的驱动模块
//
CString CSystemThreadDlg::GetDriverPathByCallbackAddress(ULONG pCallback)
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

BOOL SortSystemThreadTid( THREAD_INFO elem1, THREAD_INFO elem2 )
{
	return elem1.Tid < elem2.Tid;
}

// 
// 根据tid大小排序
//
VOID CSystemThreadDlg::SortByTid()
{
	sort( m_vectorThreadsTemp.begin(), m_vectorThreadsTemp.end(), SortSystemThreadTid );
}

//
// 判断线程是否被挂起
//
BOOL CSystemThreadDlg::IsThreadSuspened(ULONG pThread)
{
	BOOL bSuspend = FALSE;

	if (pThread)
	{
		ULONG nRet = m_clsThreads.GetSuspenCount(pThread);
		if (nRet)
		{
			bSuspend = TRUE;
		}
	}

	return bSuspend;
}

//
// 右键菜单
//
void CSystemThreadDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_EXPORT, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_THREAD_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_KILL_THREAD, szKillThread[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SUSPEND_THREAD, szSuspendThread[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_RESUME_THREAD, szResumeThread[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FORCE_RESUME_THREAD, szDisassemblyStartAddress[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_ATTRIBUTE, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CHECK_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_LOCATION_EXPLORER, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_THREAD_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_KILL_THREAD, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FORCE_KILL_THREAD, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FORCE_RESUME_THREAD, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_LOCATION_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(15, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	// 如果m_list是空的，那么除了“刷新”，其他全部置灰
	if (!m_list.GetItemCount()) 
	{
		for (int i = 2; i < 16; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt != 1) 
		{
			for (int i = 2; i < 14; i++) // 如果没有选中
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
			
			if (nCnt >= 2) // 如果选中了大于2个
			{
				menu.EnableMenuItem(ID_KILL_THREAD, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
			}
		} 
		else  // 如果选中的是一个
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				int nData = (int)m_list.GetItemData(nItem);
				THREAD_INFO info = m_vectorThreads.at(nData);

				// 挂起计数不为0
				if (IsThreadSuspened(info.pThread)) 
				{
					menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
				}
				else
				{
					menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_ENABLED);
					menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				// 如果线程所在模块为空
				CString szStartModule = m_list.GetItemText(nItem, eSystemThreadStartModule);
				if (szStartModule.IsEmpty())
				{
					menu.EnableMenuItem(ID_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_LOCATION_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				// 如果等于0，那么就把反汇编线程入口置灰
				if (info.Win32StartAddress == 0)
				{
					menu.EnableMenuItem(ID_FORCE_RESUME_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
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

//
// item颜色
//
void CSystemThreadDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		clrNewTextColor = g_MicrosoftItemClr;
		clrNewBkColor = RGB( 255, 255, 255 );	

		CString szPath = m_list.GetItemText(nItem, eSystemThreadStartModule); 		
		if (szPath.IsEmpty())
		{
			clrNewTextColor = g_HiddenOrHookItemClr;
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

//
// 刷新
//
void CSystemThreadDlg::OnThreadRefresh()
{
	ListProcessThread();
}

//
// 杀线程
//
void CSystemThreadDlg::OnKillThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(iIndex);
		THREAD_INFO info = m_vectorThreads.at(nData);
		m_clsThreads.KillThread(info.pThread);
		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

//
// 挂起线程
//
void CSystemThreadDlg::OnSuspendThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(iIndex);
		THREAD_INFO info = m_vectorThreads.at(nData);
		m_clsThreads.SuspendThread(info.pThread);
	}
}

//
// 恢复线程
//
void CSystemThreadDlg::OnResumeThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(iIndex);
		THREAD_INFO info = m_vectorThreads.at(nData);
		m_clsThreads.ResumeThread(info.pThread);
	}
}

//
// 反汇编线程入口
//
void CSystemThreadDlg::OnDissmThreadStartAddress()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		THREAD_INFO info = m_vectorThreads.at(nData);
		if (info.Win32StartAddress > 0)
		{
			m_Functions.Disassemble(info.Win32StartAddress,
				100,
				m_SystemProcessPid,
				m_SystemProcessEPROCESS);
		}
	}
}

//
// 检查模块签名
//
void CSystemThreadDlg::OnCheckSign()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, eSystemThreadStartModule);
		m_Functions.SignVerify(szPath);
	}
}

//
// 检查模块属性
//
void CSystemThreadDlg::OnCheckAttribute()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, eSystemThreadStartModule);
		m_Functions.OnCheckAttribute(szPath);
	}
}

//
// 定位到explorer
//
void CSystemThreadDlg::OnLocationExplorer()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, eSystemThreadStartModule);
		m_Functions.LocationExplorer(szPath);
	}
}

// 
// 导出到文本
//
void CSystemThreadDlg::OnExportText()
{
	m_Functions.ExportListToTxt(&m_list, m_szState);
}

//
// 导出到excel
//
void CSystemThreadDlg::OnExport()
{
	WCHAR szThread[] = {'T','h','r','e','a','d','\0'};
	m_Functions.ExportListToExcel(&m_list, szThread, m_szState);
}

//
// 屏蔽按键
//
BOOL CSystemThreadDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

//
// 定位到AS文件管理器
// 
void CSystemThreadDlg::OnProcessLocationAtFileManager()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, eSystemThreadStartModule);
		m_Functions.JmpToFile(szPath);
	}
}
