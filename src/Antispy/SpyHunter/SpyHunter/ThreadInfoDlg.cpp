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
// ThreadInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ThreadInfoDlg.h"
#include "DllModuleDlg.h"
#include <algorithm>
#include "SignVerifyDlg.h"
#include "AssemblyDlg.h"

// CThreadInfoDlg 对话框

IMPLEMENT_DYNAMIC(CThreadInfoDlg, CDialog)

CThreadInfoDlg::CThreadInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CThreadInfoDlg::IDD, pParent)
	, m_szState(_T(""))
{
	m_nModuleCnt = 0;
	m_nNotHaveModule = 0;
	m_nHideCnt = 0;
	m_pEprocess = 0;
	m_bShowDeadThread = FALSE;
}

CThreadInfoDlg::~CThreadInfoDlg()
{
	m_normaTidList.clear();
	m_ThreadVector.clear();
	m_CommonDriverList.clear();
}

void CThreadInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_THREAD_INFO_CNT, m_szState);
}


BEGIN_MESSAGE_MAP(CThreadInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CThreadInfoDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CThreadInfoDlg::OnNMRclickList)
	ON_COMMAND(ID_THREAD_REFRESH, &CThreadInfoDlg::OnThreadRefresh)
	ON_COMMAND(ID_KILL_THREAD, &CThreadInfoDlg::OnKillThread)
	ON_COMMAND(ID_FORCE_KILL_THREAD, &CThreadInfoDlg::OnForceKillThread)
	ON_COMMAND(ID_SUSPEND_THREAD, &CThreadInfoDlg::OnSuspendThread)
	ON_COMMAND(ID_RESUME_THREAD, &CThreadInfoDlg::OnResumeThread)
	ON_COMMAND(ID_FORCE_RESUME_THREAD, &CThreadInfoDlg::OnForceResumeThread)
	ON_COMMAND(ID_CHECK_SIGN, &CThreadInfoDlg::OnCheckSign)
	ON_COMMAND(ID_CHECK_ATTRIBUTE, &CThreadInfoDlg::OnCheckAttribute)
	ON_COMMAND(ID_LOCATION_EXPLORER, &CThreadInfoDlg::OnLocationExplorer)
	ON_COMMAND(ID_EXPORT_TEXT, &CThreadInfoDlg::OnExportText)
	ON_COMMAND(ID_EXPORT, &CThreadInfoDlg::OnExport)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CThreadInfoDlg::OnNMCustomdrawList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CThreadInfoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_THREAD_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CThreadInfoDlg 消息处理程序

void CThreadInfoDlg::OnBnClickedOk()
{
}

void CThreadInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CThreadInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szThreadId[g_enumLang], LVCFMT_LEFT, 50);
	m_list.InsertColumn(1, szThreadObject[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(2, szTeb[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(3, szPriority[g_enumLang], LVCFMT_LEFT, 50);
	m_list.InsertColumn(4, szStartAddress[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(5, szSwitchTimes[g_enumLang], LVCFMT_LEFT, 85);
	m_list.InsertColumn(6, szThreadStatus[g_enumLang], LVCFMT_LEFT, 75);
	m_list.InsertColumn(7, szStartModule[g_enumLang], LVCFMT_LEFT, 250);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CThreadInfoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_EXPORT, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_THREAD_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	// 	menu.AppendMenu(MF_STRING, ID_SHOW_DEAD_THREAD, szShowTerminatedThreads[g_enumLang]);
	// 	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_KILL_THREAD, szKillThread[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_FORCE_KILL_THREAD, szForceKillThread[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SUSPEND_THREAD, szSuspendThread[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_RESUME_THREAD, szResumeThread[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
//	menu.AppendMenu(MF_STRING, ID_FORCE_SUSPEND_THREAD, szDisassemblyStartAddress[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_FORCE_RESUME_THREAD, szDisassemblyStartAddress[g_enumLang]);
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
		menu.SetMenuItemBitmaps(ID_FORCE_KILL_THREAD, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FORCE_RESUME_THREAD, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_LOCATION_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
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
		ULONG nCnt = 0;
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = m_list.GetNextSelectedItem(pos);
			nCnt++;
		}

		if (nCnt != 1) 
		{
			for (int i = 2; i < 13; i++) // 如果没有选中
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

//			menu.EnableMenuItem(ID_SHOW_DEAD_THREAD, MF_BYCOMMAND | MF_ENABLED);

			if (nCnt >= 2) // 如果选中了大于2个
			{
				menu.EnableMenuItem(ID_KILL_THREAD, MF_BYCOMMAND | MF_ENABLED);
				menu.EnableMenuItem(ID_FORCE_KILL_THREAD, MF_BYCOMMAND | MF_ENABLED);

				ULONG nRet = IsMulitsThreadSuspened();
				if (nRet == 1)
				{
					menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
				}
				else if (nRet == 2)
				{
					menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_ENABLED);
					menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		} 
		else  // 如果选中的是一个
		{
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			if (pos != NULL)
			{
				int nItem = m_list.GetNextSelectedItem(pos);
				CString szThreadObject = m_list.GetItemText(nItem, 1);

				for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
				{
					CString strThreadObject;
					strThreadObject.Format(L"0x%08X", Iter->pThread);
					if (!strThreadObject.CompareNoCase(szThreadObject))
					{
						if (Iter->State == Terminated) 
						{
							menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							menu.EnableMenuItem(ID_KILL_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							menu.EnableMenuItem(ID_FORCE_KILL_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						}
						else if (IsThreadSuspened(Iter->pThread)) // 挂起计数不为0
						{
							menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
							menu.EnableMenuItem(ID_FORCE_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
						}
						else
						{
							menu.EnableMenuItem(ID_SUSPEND_THREAD, MF_BYCOMMAND | MF_ENABLED);
							menu.EnableMenuItem(ID_RESUME_THREAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							menu.EnableMenuItem(ID_FORCE_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
						}

						CString szStartModule = m_list.GetItemText(nItem, 7);
						if (szStartModule.IsEmpty())
						{
							menu.EnableMenuItem(ID_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							menu.EnableMenuItem(ID_CHECK_ATTRIBUTE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							menu.EnableMenuItem(ID_LOCATION_EXPLORER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						}

						break;
					}	
				}
			}
		}

		if (nCnt)
		{
			menu.EnableMenuItem(ID_FORCE_RESUME_THREAD, MF_BYCOMMAND | MF_ENABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CThreadInfoDlg::OnThreadRefresh()
{
	ListProcessThread();
}

void CThreadInfoDlg::OnKillThread()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	while (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szThreadObject = m_list.GetItemText(iIndex, 1);
// 		for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
// 		{
// 			CString strThreadObject;
// 			strThreadObject.Format(L"0x%08X", Iter->pThread);
// 			if (!strThreadObject.CompareNoCase(szThreadObject))
// 			{
// 				COMMUNICATE_KILL_THREAD ckt;
// 				ckt.bForceKill = FALSE;
// 				ckt.pThread = Iter->pThread;
// 				ckt.OpType = enumKillThread;
// 				m_Driver.CommunicateDriver(&ckt, sizeof(COMMUNICATE_KILL_THREAD), NULL, 0, NULL);
// 				break;
// 			}	
// 		}
// 
// 		m_list.DeleteItem(iIndex);
// 		pos = m_list.GetFirstSelectedItemPosition();
// 	}
}

void CThreadInfoDlg::OnForceKillThread()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	while (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szThreadObject = m_list.GetItemText(iIndex, 1);
// 		for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
// 		{
// 			CString strThreadObject;
// 			strThreadObject.Format(L"0x%08X", Iter->pThread);
// 			if (!strThreadObject.CompareNoCase(szThreadObject))
// 			{
// 				COMMUNICATE_KILL_THREAD ckt;
// 				ckt.bForceKill = TRUE;
// 				ckt.pThread = Iter->pThread;
// 				ckt.OpType = enumKillThread;
// 				m_Driver.CommunicateDriver(&ckt, sizeof(COMMUNICATE_KILL_THREAD), NULL, 0, NULL);
// 				break;
// 			}	
// 		}
// 
// 		m_list.DeleteItem(iIndex);
// 		pos = m_list.GetFirstSelectedItemPosition();
// 	}
}

void CThreadInfoDlg::OnSuspendThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szThreadObject = m_list.GetItemText(iIndex, 1);
		for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
		{
			CString strThreadObject;
			strThreadObject.Format(L"0x%08X", Iter->pThread);
			if (!strThreadObject.CompareNoCase(szThreadObject))
			{
				SuspenedOrResume(Iter->pThread, TRUE);
				break;
			}	
		}
	}
}

void CThreadInfoDlg::OnResumeThread()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szThreadObject = m_list.GetItemText(iIndex, 1);
		for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
		{
			CString strThreadObject;
			strThreadObject.Format(L"0x%08X", Iter->pThread);
			if (!strThreadObject.CompareNoCase(szThreadObject))
			{
				SuspenedOrResume(Iter->pThread, FALSE);
				break;
			}	
		}
	}
}

void CThreadInfoDlg::OnForceResumeThread()
{
// 	POSITION pos = m_list.GetFirstSelectedItemPosition();
// 	while (pos)
// 	{
// 		int iIndex  = m_list.GetNextSelectedItem(pos);
// 		CString szThreadObject = m_list.GetItemText(iIndex, 1);
// 		for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
// 		{
// 			CString strThreadObject;
// 			strThreadObject.Format(L"0x%08X", Iter->pThread);
// 			if (!strThreadObject.CompareNoCase(szThreadObject))
// 			{
// 				ForceSuspenedOrResume(Iter->pThread, FALSE);
// 				break;
// 			}	
// 		}
// 	}

	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 4);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			CAssemblyDlg AssmDlg;
			AssmDlg.m_dwPid = m_uPid;
			AssmDlg.m_dwEprocess = m_pEprocess;
			AssmDlg.m_dwAddress = dwAddress;
			AssmDlg.DoModal();
		}
	}
}

void CThreadInfoDlg::OnCheckSign()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 7);
		m_Functions.SignVerify(szPath);
	}
}

void CThreadInfoDlg::OnCheckAttribute()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 7);

		m_Functions.OnCheckAttribute(szPath);
	}
}

void CThreadInfoDlg::OnLocationExplorer()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szPath = m_list.GetItemText(nItem, 7);

		m_Functions.LocationExplorer(szPath);
	}
}

void CThreadInfoDlg::OnExportText()
{
	m_Functions.ExportListToTxt(&m_list, m_szState);
}

void CThreadInfoDlg::OnExport()
{
	WCHAR szThread[] = {'T','h','r','e','a','d','\0'};
	m_Functions.ExportListToExcel(&m_list, szThread, m_szState);
}

void CThreadInfoDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

CString CThreadInfoDlg::GetDriverPath(ULONG pCallback)
{
	CString szPath;

	for (vector<DRIVER_INFO>::iterator itor = m_CommonDriverList.begin(); itor != m_CommonDriverList.end(); itor++)
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

VOID CThreadInfoDlg::InsertThreadIntoList(THREAD_INFO ThreadInfo)
{
	if (!m_bShowDeadThread && ThreadInfo.State == Terminated)
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
	m_list.SetItemText(n, 1, szEthread);
	m_list.SetItemText(n, 2, szTeb);
	m_list.SetItemText(n, 3, szPriority);
	m_list.SetItemText(n, 4, szWin32StartAddress);
	m_list.SetItemText(n, 5, szContextSwitches);
	m_list.SetItemText(n, 6, szState);
	m_list.SetItemText(n, 7, szModule);

	if (szModule.IsEmpty())
	{
		m_list.SetItemData(n, enumThreadNotHaveModule);
		m_nNotHaveModule++;
	}
	else if (IsThreadHide(ThreadInfo.Tid) && (ThreadInfo.State != Terminated))
	{
		m_list.SetItemData(n, enumThreadHide);
		m_nHideCnt++;
	}

	m_nModuleCnt++;
}

CString CThreadInfoDlg::GetModulePathByThreadStartAddress(DWORD dwBase)
{
	CString szRet = L"";

	if (dwBase < 0x80000000)
	{
		for ( vector <MODULE_INFO>::iterator Iter = m_vectorModules.begin( ); Iter != m_vectorModules.end( ); Iter++)
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
		if (m_CommonDriverList.size() == 0)
		{
			GetDriver();
		}

		szRet = GetDriverPath(dwBase);
	}

	return szRet;
}

VOID CThreadInfoDlg::GetProcessThreadInfo()
{
// 	COMMUNICATE_PROCESS_THREAD cpt;
// 	ULONG nCnt = 100;
// 	PPROCESS_THREAD pModuleThread = NULL;
// 	BOOL bRet = FALSE;
// 
// 	m_ThreadVector.clear();
// 	m_clsListModules.EnumModulesByPeb(m_uPid, m_pEprocess, m_vectorModules);
// 
// 	cpt.OpType = enumListProcessThread;
// 	cpt.nPid = m_uPid;
// 	cpt.pEprocess = m_pEprocess;
// 
// 	do 
// 	{
// 		ULONG nSize = 0;
// 
// 		if (pModuleThread)
// 		{
// 			free(pModuleThread);
// 			pModuleThread = NULL;
// 		}
// 
// 		nSize = sizeof(PROCESS_THREAD) + nCnt * sizeof(THREAD_INFO);
// 
// 		pModuleThread = (PPROCESS_THREAD)malloc(nSize);
// 		if (!pModuleThread)
// 		{
// 			break;
// 		}
// 
// 		memset(pModuleThread, 0, nSize);
// 
// 		pModuleThread->nCnt = nCnt;
// 		pModuleThread->nRetCnt = 0;
// 
// 		bRet = m_Driver.CommunicateDriver(&cpt, sizeof(COMMUNICATE_PROCESS_THREAD), pModuleThread, nSize, NULL);
// 		nCnt = pModuleThread->nRetCnt + 10;
// 
// 	} while (pModuleThread->nRetCnt > pModuleThread->nCnt);
// 
// 	if (bRet && pModuleThread)
// 	{
// 		for (ULONG i = 0; i < pModuleThread->nRetCnt; i++)
// 		{
// 			m_ThreadVector.push_back(pModuleThread->Threads[i]);
// 		}
// 	}
// 
// 	if (pModuleThread)
// 	{
// 		free(pModuleThread);
// 		pModuleThread = NULL;
// 	}
}

BOOL CThreadInfoDlg::Ring3ListProcessThreads(DWORD dwOwnerPID)   
{   
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;   
	THREADENTRY32 te32;   

	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);   
	if(hThreadSnap == INVALID_HANDLE_VALUE)   
	{
		return FALSE;  
	}

	te32.dwSize = sizeof(THREADENTRY32);   

	if(!Thread32First(hThreadSnap, &te32))   
	{ 
		CloseHandle(hThreadSnap);
		return FALSE; 
	} 

	do   
	{   
		if(te32.th32OwnerProcessID == dwOwnerPID) 
		{ 
			m_normaTidList.push_back(te32.th32ThreadID);
		} 
	}   while(Thread32Next(hThreadSnap, &te32) );   

	CloseHandle(hThreadSnap); 
	return TRUE; 
} 

BOOL CThreadInfoDlg::IsThreadHide(DWORD tid)
{
	BOOL bHide = FALSE;

	list<DWORD>::iterator findItr;
	findItr = find(m_normaTidList.begin(), m_normaTidList.end(), tid);
	if (findItr == m_normaTidList.end())
	{
		bHide = TRUE;
	}

	return bHide;
}

BOOL CThreadInfoDlg::ListProcessThread()
{
	m_szState.Format(szThreadInfoState[g_enumLang], 0, 0, 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_nModuleCnt = m_nHideCnt = m_nNotHaveModule = 0;
	m_normaTidList.clear();

	GetProcessThreadInfo();
	if (!m_ThreadVector.empty())
	{
		SortByTid();
		Ring3ListProcessThreads(m_uPid);

		for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
		{
			InsertThreadIntoList(*Iter);
		}
	}

	m_szState.Format(szThreadInfoState[g_enumLang], m_nModuleCnt, m_nHideCnt, m_nNotHaveModule);
	UpdateData(FALSE);

	return TRUE;
}

BOOL SortTid( THREAD_INFO elem1, THREAD_INFO elem2 )
{
	return elem1.Tid < elem2.Tid;
}

VOID CThreadInfoDlg::SortByTid()
{
	sort( m_ThreadVector.begin(), m_ThreadVector.end(), SortTid );
}

BOOL CThreadInfoDlg::SuspenedOrResume(ULONG pThread, BOOL bSuspend)
{
	BOOL bRet = FALSE;
// 	if (pThread)
// 	{
// 		COMMUNICATE_SUSPENDORRESUME_THREAD cst;
// 		cst.pThread = pThread;
// 		cst.bSuspend = bSuspend;
// 		cst.OpType = enumResumeOrSuspendThread;
// 		bRet = m_Driver.CommunicateDriver(&cst, sizeof(COMMUNICATE_SUSPENDORRESUME_THREAD), NULL, 0, NULL);
// 	}

	return bRet;
}

BOOL CThreadInfoDlg::ForceSuspenedOrResume(ULONG pThread, BOOL bSuspend)
{
	BOOL bRet = FALSE;
// 	if (pThread)
// 	{
// 		COMMUNICATE_SUSPENDORRESUME_THREAD cst;
// 		cst.pThread = pThread;
// 		cst.bSuspend = bSuspend;
// 		cst.OpType = enumForceResumeOrSuspendThread;
// 		bRet = m_Driver.CommunicateDriver(&cst, sizeof(COMMUNICATE_SUSPENDORRESUME_THREAD), NULL, 0, NULL);
// 	}

	return bRet;
}
void CThreadInfoDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		if (m_list.GetItemData(nItem) == enumThreadHide)
		{
			clrNewTextColor = g_HiddenOrHookItemClr;//RGB( 255, 0, 0 );
		}
		else if (m_list.GetItemData(nItem) == enumThreadNotHaveModule)
		{
			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

// 0 - Suspend、Resume都为灰
// 1 - Suspend为灰、Resume正常
// 2 - Suspend正常、Resume为灰
ULONG CThreadInfoDlg::IsMulitsThreadSuspened()
{
	BOOL bRet = FALSE, bNowRet = FALSE, bFirst = TRUE;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szThreadObject = m_list.GetItemText(iIndex, 1);
		for ( vector <THREAD_INFO>::iterator Iter = m_ThreadVector.begin( ); Iter != m_ThreadVector.end( ); Iter++ )
		{
			CString strThreadObject;
			strThreadObject.Format(L"0x%08X", Iter->pThread);
			if (!strThreadObject.CompareNoCase(szThreadObject))
			{
				if (Iter->State == Terminated)
				{
					return 0;
				}

				if (bFirst)
				{
					bRet = bNowRet = IsThreadSuspened(Iter->pThread);
					bFirst = FALSE;
				}
				else
				{
					bNowRet = IsThreadSuspened(Iter->pThread);
					if (bNowRet != bRet)
					{
						return 0;
					}
				}

				break;
			}	
		}
	}

	if (bNowRet)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

BOOL CThreadInfoDlg::IsThreadSuspened(ULONG pThread)
{
	BOOL bSuspend = FALSE;

// 	if (pThread)
// 	{
// 		COMMUNICATE_GETSUSPENDCNT_THREAD cgt;
// 		ULONG nRet = 0;
// 
// 		cgt.OpType = enumGetThreadSuspendCnt;
// 		cgt.pThread = pThread;
// 		BOOL bRet = m_Driver.CommunicateDriver(&cgt, sizeof(COMMUNICATE_GETSUSPENDCNT_THREAD), &nRet, sizeof(ULONG), NULL);
// 		if (nRet)
// 		{
// 			bSuspend = TRUE;
// 		}
// 	}

	return bSuspend;
}