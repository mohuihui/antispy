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
// ThreadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ThreadDlg.h"
#include "DllModuleDlg.h"
#include <algorithm>
#include "SignVerifyDlg.h"
#include "AssemblyDlg.h"

typedef enum _THREAD_HEADER_INDEX_
{
	eThreadId,
	eThreadObject,
	eThreadTeb,
	eThreadPriority,
	eThreadStartAddress,
	eThreadSwitchTimes,
	eThreadStatus,
	eThreadStartModule,
}THREAD_HEADER_INDEX;

// CThreadDlg 对话框

IMPLEMENT_DYNAMIC(CThreadDlg, CDialog)

CThreadDlg::CThreadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CThreadDlg::IDD, pParent)
{
	m_nThreadsCnt = 0;
	m_pEprocess = 0;
	m_hDlgWnd = NULL;
}

CThreadDlg::~CThreadDlg()
{
	m_vectorThreadsAll.clear();
	m_vectorDrivers.clear();
}

void CThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CThreadDlg, CDialog)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CThreadDlg::OnNMCustomdrawList)
	ON_WM_SIZE()
	ON_COMMAND(ID_THREAD_REFRESH, &CThreadDlg::OnThreadRefresh)
	ON_COMMAND(ID_KILL_THREAD, &CThreadDlg::OnKillThread)
	ON_COMMAND(ID_SUSPEND_THREAD, &CThreadDlg::OnSuspendThread)
	ON_COMMAND(ID_FORCE_SUSPEND_THREAD, &CThreadDlg::OnDissmThreadStartAddress)
	ON_COMMAND(ID_LOCATION_EXPLORER, &CThreadDlg::OnLocationExplorer)
	ON_COMMAND(ID_CHECK_ATTRIBUTE, &CThreadDlg::OnCheckAttribute)
	ON_COMMAND(ID_CHECK_SIGN, &CThreadDlg::OnCheckSign)
	ON_COMMAND(ID_EXPORT, &CThreadDlg::OnExport)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CThreadDlg::OnNMRclickList)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_EXPORT_TEXT, &CThreadDlg::OnExportText)
	ON_COMMAND(ID_RESUME_THREAD, &CThreadDlg::OnResumeThread)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CThreadDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CThreadDlg 消息处理程序


void CThreadDlg::SelectThread(DWORD dwTid)
{
	if (dwTid == 0)
	{
		return;
	}

	CString szTid;
	szTid.Format(L"%d", dwTid);

	DWORD dwCnt = m_list.GetItemCount();
	for (DWORD i = 0; i < dwCnt; i++)
	{
		if (!(m_list.GetItemText(i, 0)).CompareNoCase(szTid))
		{
			m_list.EnsureVisible(i, FALSE);
			m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			m_list.SetFocus();
			break;
		}
	}
}

BOOL CThreadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(eThreadId, szThreadId[g_enumLang], LVCFMT_LEFT, 50);
	m_list.InsertColumn(eThreadObject, szThreadObject[g_enumLang], LVCFMT_LEFT, 83);
	m_list.InsertColumn(eThreadTeb, szTeb[g_enumLang], LVCFMT_LEFT, 75);
	m_list.InsertColumn(eThreadPriority, szPriority[g_enumLang], LVCFMT_LEFT, 50);
	m_list.InsertColumn(eThreadStartAddress, szStartAddress[g_enumLang], LVCFMT_LEFT, 78);
	m_list.InsertColumn(eThreadSwitchTimes, szSwitchTimes[g_enumLang], LVCFMT_LEFT, 70);
	m_list.InsertColumn(eThreadStatus, szThreadStatus[g_enumLang], LVCFMT_LEFT, 75);
	m_list.InsertColumn(eThreadStartModule, szStartModule[g_enumLang], LVCFMT_LEFT, 520);
	
	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// 根据起始地址获取驱动的路径
//
CString CThreadDlg::GetDriverPath(ULONG pCallback)
{
	CString szPath = L"";

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

//
// 添加项
//
void CThreadDlg::AddThreadItem(THREAD_INFO ThreadInfo)
{
	if (ThreadInfo.State == Terminated)
	{
		return;
	}

	CString szTid, szEthread, szTeb, szPriority, szWin32StartAddress, szContextSwitches, szState, szModule;

	szTid.Format(L"%d", ThreadInfo.Tid);
	szEthread.Format(L"0x%08X", ThreadInfo.pThread);
	if (ThreadInfo.Teb == 0)
	{
		szTeb = L"-";
	}
	else
	{
		szTeb.Format(L"0x%08X", ThreadInfo.Teb);
	}

	szPriority.Format(L"%d", ThreadInfo.Priority);
	szWin32StartAddress.Format(L"0x%08X", ThreadInfo.Win32StartAddress);
	szContextSwitches.Format(L"%d", ThreadInfo.ContextSwitches);
	szModule = GetModulePathByThreadStartAddress(ThreadInfo.Win32StartAddress);

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
	m_list.SetItemText(n, eThreadObject, szEthread);
	m_list.SetItemText(n, eThreadTeb, szTeb);
	m_list.SetItemText(n, eThreadPriority, szPriority);
	m_list.SetItemText(n, eThreadStartAddress, szWin32StartAddress);
	m_list.SetItemText(n, eThreadSwitchTimes, szContextSwitches);
	m_list.SetItemText(n, eThreadStatus, szState);
	m_list.SetItemText(n, eThreadStartModule, szModule);
	
	ITEM_COLOR clr = enumBlack;
	if (szModule.IsEmpty())
	{
		clr = enumRed;
	}

	m_vectorColor.push_back(clr);
	m_vectorThreads.push_back(ThreadInfo);
	m_list.SetItemData(n, m_nThreadsCnt);
	m_nThreadsCnt++;
}

//
// 根据线程的起始地址找到模块路径
//
CString CThreadDlg::GetModulePathByThreadStartAddress(DWORD dwBase)
{
	CString szRet = L"";

	if (dwBase < 0x80000000)
	{
		for ( vector <MODULE_INFO>::iterator Iter = m_vectorModules.begin( ); 
			Iter != m_vectorModules.end( ); 
			Iter++)
		{	
			MODULE_INFO entry = *Iter;
			if (dwBase >= entry.Base && dwBase <= (entry.Base + entry.Size))
			{
				szRet = entry.Path;
			}
		}
	}
	else
	{
		if (m_vectorDrivers.empty())
		{
			m_clsDrivers.ListDrivers(m_vectorDrivers);
		}

		szRet = GetDriverPath(dwBase);
	}

	return szRet;
}

//
// 枚举线程
//
BOOL CThreadDlg::EnumThreads(ULONG *nCnt)
{
	m_list.DeleteAllItems();
	m_nThreadsCnt = 0;
	m_vectorThreads.clear();
	m_vectorColor.clear();

	// 枚举线程
	m_clsThreads.ListThreads(m_uPid, m_pEprocess, m_vectorThreadsAll);
	if (m_vectorThreadsAll.empty())
	{
		return FALSE;
	}

	// 根据tid排序
	SortByTid();

	// 枚举模块
	m_clsModules.EnumModulesByPeb(m_uPid, m_pEprocess, m_vectorModules);
	
	for ( vector <THREAD_INFO>::iterator Iter = m_vectorThreadsAll.begin( ); 
		Iter != m_vectorThreadsAll.end( ); 
		Iter++ )
	{
		AddThreadItem(*Iter);
	}
	
	if (nCnt)
	{
		*nCnt = m_nThreadsCnt;
	}

	return TRUE;
}

BOOL SortPid( THREAD_INFO elem1, THREAD_INFO elem2 )
{
	return elem1.Tid < elem2.Tid;
}

//
// 根据tid的大小进行排序
//
VOID CThreadDlg::SortByTid()
{
	sort( m_vectorThreadsAll.begin(), m_vectorThreadsAll.end(), SortPid );
}

// 
// 颜色控制
//
void CThreadDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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
		if (nItem != -1)
		{	
			COLORREF clrNewTextColor, clrNewBkColor;
			clrNewTextColor = g_MicrosoftItemClr;//RGB( 0, 0, 0 );
			clrNewBkColor = RGB( 255, 255, 255 );

			int nData = (int)m_list.GetItemData(nItem);
			ITEM_COLOR clr = m_vectorColor.at(nData);
			if (clr == enumRed)
			{
				clrNewTextColor = g_HiddenOrHookItemClr;
			}

			pLVCD->clrText = clrNewTextColor;
			pLVCD->clrTextBk = clrNewBkColor;
		}
			
		*pResult = CDRF_DODEFAULT;
	}
}

void CThreadDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

//
// 右键菜单 - 刷新
//
void CThreadDlg::OnThreadRefresh()
{
//	EnumThreads();
	::SendMessage(m_hDlgWnd, WM_ENUM_THREADS, NULL, NULL);
}

//
// 结束线程
//
void CThreadDlg::OnKillThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(nItem);
		THREAD_INFO info = m_vectorThreads.at(nData);

		m_clsThreads.KillThread(info.pThread);

		m_list.DeleteItem(nItem);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

//
// 挂起线程
//
void CThreadDlg::OnSuspendThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(nItem);
		THREAD_INFO info = m_vectorThreads.at(nData);
		SuspenedOrResume(info.pThread, TRUE);
	}
}

//
// 反汇编函数入口
//
void CThreadDlg::OnDissmThreadStartAddress()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		THREAD_INFO info = m_vectorThreads.at(nData);
		if (info.Win32StartAddress > 0)
		{
			CAssemblyDlg AssmDlg;
			AssmDlg.m_dwPid = m_uPid;
			AssmDlg.m_dwEprocess = m_pEprocess;
			AssmDlg.m_dwAddress = info.Win32StartAddress;
			AssmDlg.DoModal();
		}
	}
}

//
// 定位线程模块
//
void CThreadDlg::OnLocationExplorer()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, eThreadStartModule);
		m_Functions.LocationExplorer(szPath);
	}
}

//
// 检查模块属性
//
void CThreadDlg::OnCheckAttribute()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, eThreadStartModule);
		m_Functions.OnCheckAttribute(szPath);
	}
}

//
// 检查模块数字签名
//
void CThreadDlg::OnCheckSign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, eThreadStartModule);
		m_Functions.SignVerify(szPath);
	}
}

//
// 导出到Excel
//
void CThreadDlg::OnExport()
{
	WCHAR szThread[] = {'T','h','r','e','a','d','\0'};
	CString szStatus;
	szStatus.Format(szThreadsInfoState[g_enumLang], m_nThreadsCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;
	m_Functions.ExportListToExcel(&m_list, szThread, szText);
}

//
// 右键菜单
//
void CThreadDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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
 	menu.AppendMenu(MF_STRING, ID_FORCE_SUSPEND_THREAD, szDisassemblyStartAddress[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CHECK_ATTRIBUTE, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_LOCATION_EXPLORER, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CHECK_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
 	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_THREAD_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_KILL_THREAD, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FORCE_SUSPEND_THREAD, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_LOCATION_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(13, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	// 如果m_list是空的，那么除了“刷新”，其他全部置灰
	if (!m_list.GetItemCount()) 
	{
		for (int i = 2; i < 14; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt != 1) 
		{
			// 如果没有选中
			for (int i = 2; i < 12; i++) 
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			// 如果选中了大于2个
			if (nCnt >= 2)
			{
				menu.EnableMenuItem(ID_KILL_THREAD, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
			}
		} 

		// 如果选中的是一个
		else  
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
				CString szStartModule = m_list.GetItemText(nItem, eThreadStartModule);
				if (szStartModule.IsEmpty())
				{
					menu.EnableMenuItem(ID_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_LOCATION_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				// 如果等于0，那么就把反汇编线程入口置灰
				if (info.Win32StartAddress == 0)
				{
					menu.EnableMenuItem(ID_FORCE_SUSPEND_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
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
// 导出到文本
//
void CThreadDlg::OnExportText()
{
	CString szStatus;
	szStatus.Format(szThreadsInfoState[g_enumLang], m_nThreadsCnt);
	CString szText = L"[" + m_szImage + L"] " + szStatus;
	m_Functions.ExportListToTxt(&m_list, szText);
}

//
// 判断线程是否是挂起状态
//
BOOL CThreadDlg::IsThreadSuspened(ULONG pThread)
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
// 挂起或者恢复线程
//
BOOL CThreadDlg::SuspenedOrResume(ULONG pThread, BOOL bSuspend)
{
	BOOL bRet = FALSE;
	if (pThread)
	{
		if (bSuspend)
		{
			bRet = m_clsThreads.SuspendThread(pThread);
		}
		else
		{
			bRet = m_clsThreads.ResumeThread(pThread);
		}
	}

	return bRet;
}

//
// 挂起线程
//
void CThreadDlg::OnResumeThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(nItem);
		THREAD_INFO info = m_vectorThreads.at(nData);
		SuspenedOrResume(info.pThread, FALSE);
	}
}

//
// 屏蔽按键
//
BOOL CThreadDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

//
// 设置进程信息
//
void CThreadDlg::SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd)
{
	m_uPid = nPid;
	m_pEprocess = pEprocess;
	m_szImage = szImage;
	m_hDlgWnd = hWnd;
}