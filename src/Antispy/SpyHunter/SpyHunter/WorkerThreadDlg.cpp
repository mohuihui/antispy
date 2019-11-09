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
// WorkerThreadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "WorkerThreadDlg.h"


// CWorkerThreadDlg 对话框

IMPLEMENT_DYNAMIC(CWorkerThreadDlg, CDialog)

CWorkerThreadDlg::CWorkerThreadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWorkerThreadDlg::IDD, pParent)
	, m_status(_T(""))
{

}

CWorkerThreadDlg::~CWorkerThreadDlg()
{
	m_WorkerThreadVector.clear();
}

void CWorkerThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_WORKER_THREAD_CNT, m_status);
}


BEGIN_MESSAGE_MAP(CWorkerThreadDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CWorkerThreadDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_COMMAND(ID_WORKER_REFLASH, &CWorkerThreadDlg::OnWorkerReflash)
	ON_COMMAND(ID_WORK_SUSPEND_THREAD, &CWorkerThreadDlg::OnWorkSuspendThread)
	ON_COMMAND(ID_WORK_RESUME_THREAD, &CWorkerThreadDlg::OnWorkResumeThread)
	ON_COMMAND(ID_WORK_DISASM, &CWorkerThreadDlg::OnWorkDisasm)
	ON_COMMAND(ID_WORK_SHUXING, &CWorkerThreadDlg::OnWorkShuxing)
	ON_COMMAND(ID_WORK_LOCATION_MODULE, &CWorkerThreadDlg::OnWorkLocationModule)
	ON_COMMAND(ID_WORK_VERIFY_SIGN, &CWorkerThreadDlg::OnWorkVerifySign)
	ON_COMMAND(ID_WORK_TEXT, &CWorkerThreadDlg::OnWorkTimerText)
	ON_COMMAND(ID_WORK_EXCEL, &CWorkerThreadDlg::OnWorkTimerExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CWorkerThreadDlg::OnNMRclickList)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CWorkerThreadDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CWorkerThreadDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_WORKER_THREAD_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CWorkerThreadDlg 消息处理程序

void CWorkerThreadDlg::OnBnClickedOk()
{
}

void CWorkerThreadDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CWorkerThreadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szThreadObject[g_enumLang], LVCFMT_LEFT, 140);
	m_list.InsertColumn(1, szType[g_enumLang], LVCFMT_LEFT, 250);
	m_list.InsertColumn(2, szDpcTimerRoutineEntry[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(3, szModulePath[g_enumLang], LVCFMT_LEFT, 430);
	m_list.InsertColumn(4, szFileCorporation[g_enumLang], LVCFMT_LEFT, 180);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CWorkerThreadDlg::GetWorkerThread()
{
	ULONG nCnt = 100;
	PWORKER_THREAD_INFO pWorkerThreadInfo = NULL;
	OPERATE_TYPE ot = enumListWorkerThread;
	BOOL bRet = FALSE;

	m_status.Format(szWorkerThreadStatus[g_enumLang], 0, 0, 0, 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_WorkerThreadVector.clear();

	do 
	{
		ULONG nSize = sizeof(WORKER_THREAD_INFO) + nCnt * sizeof(WORKER_THREAD);

		if (pWorkerThreadInfo)
		{
			free(pWorkerThreadInfo);
			pWorkerThreadInfo = NULL;
		}

		pWorkerThreadInfo = (PWORKER_THREAD_INFO)malloc(nSize);

		if (pWorkerThreadInfo)
		{
			memset(pWorkerThreadInfo, 0, nSize);
			pWorkerThreadInfo->nCnt = nCnt;
			bRet = m_driver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pWorkerThreadInfo, nSize, NULL);
		}

		nCnt = pWorkerThreadInfo->nRetCnt + 10;

	} while (!bRet && pWorkerThreadInfo->nRetCnt > pWorkerThreadInfo->nCnt);

	if (bRet &&
		pWorkerThreadInfo->nCnt >= pWorkerThreadInfo->nRetCnt)
	{
		for (ULONG i = 0; i < pWorkerThreadInfo->nRetCnt; i++)
		{
			m_WorkerThreadVector.push_back(pWorkerThreadInfo->Thread[i]);
		}
	}

	if (pWorkerThreadInfo)
	{
		free(pWorkerThreadInfo);
		pWorkerThreadInfo = NULL;
	}

 	GetDriver();
 	InsertWorkerThreadItems();
}

CString CWorkerThreadDlg::GetDriverPathByCallbackAddress(ULONG pCallback)
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

void CWorkerThreadDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

void CWorkerThreadDlg::InsertWorkerThreadItems()
{
	ULONG nCriticalWorkQueue = 0, nDelayedWorkQueue = 0, nHyperCriticalWorkQueue = 0;

	for (vector<WORKER_THREAD>::iterator itor = m_WorkerThreadVector.begin(); itor != m_WorkerThreadVector.end(); itor++)
	{
		CString szThreadObject, szType, szModulePath, szEntry, szFileCorporation;

		szThreadObject.Format(L"0x%08X", itor->ThreadObject);
		switch (itor->Type)
		{
		case CriticalWorkQueue:
			szType = L"CriticalWorkQueue";
			nCriticalWorkQueue++;
			break;

		case DelayedWorkQueue:
			szType = L"DelayedWorkQueue";
			nDelayedWorkQueue++;
			break;

		case HyperCriticalWorkQueue:
			szType = L"HyperCriticalWorkQueue";
			nHyperCriticalWorkQueue++;
			break;
		}

		if (itor->Dispatch)
		{
			szEntry.Format(L"0x%08X", itor->Dispatch);
			szModulePath = GetDriverPathByCallbackAddress(itor->Dispatch);
			szFileCorporation = m_Functions.GetFileCompanyName(szModulePath);
		}
		else
		{
			szEntry = L"-";
			szModulePath = L"-";
		}
		
		int n = m_list.InsertItem(m_list.GetItemCount(), szThreadObject);
		m_list.SetItemText(n, 1, szType);
		m_list.SetItemText(n, 2, szEntry);
		m_list.SetItemText(n, 3, szModulePath);
		m_list.SetItemText(n, 4, szFileCorporation);
	}

	m_status.Format(szWorkerThreadStatus[g_enumLang], m_WorkerThreadVector.size(), nCriticalWorkQueue, nDelayedWorkQueue, nHyperCriticalWorkQueue);
	UpdateData(FALSE);
}

void CWorkerThreadDlg::OnWorkerReflash()
{
	GetWorkerThread();
}

BOOL CWorkerThreadDlg::SuspenedOrResume(ULONG pThread, BOOL bSuspend)
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

void CWorkerThreadDlg::OnWorkSuspendThread()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szThreadObject = m_list.GetItemText(nItem, 0);
		DWORD dwThreadObject = m_Functions.HexStringToLong(szThreadObject);
		if (dwThreadObject > 0x80000000)
		{
			SuspenedOrResume(dwThreadObject, TRUE);
		}
	}
}

void CWorkerThreadDlg::OnWorkResumeThread()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szThreadObject = m_list.GetItemText(nItem, 0);
		DWORD dwThreadObject = m_Functions.HexStringToLong(szThreadObject);
		if (dwThreadObject > 0x80000000)
		{
			SuspenedOrResume(dwThreadObject, FALSE);
		}
	}
}

BOOL CWorkerThreadDlg::IsThreadSuspened(ULONG pThread)
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

void CWorkerThreadDlg::OnWorkDisasm()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 2);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

void CWorkerThreadDlg::OnWorkShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szModule = m_list.GetItemText(nItem, 3);
		m_Functions.OnCheckAttribute(szModule);
	}
}

void CWorkerThreadDlg::OnWorkLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szModule = m_list.GetItemText(nItem, 3);
		m_Functions.LocationExplorer(szModule);
	}
}

void CWorkerThreadDlg::OnWorkVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szModule = m_list.GetItemText(nItem, 3);
		m_Functions.SignVerify(szModule);
	}
}

void CWorkerThreadDlg::OnWorkTimerText()
{
	m_Functions.ExportListToTxt(&m_list, m_status);
}

void CWorkerThreadDlg::OnWorkTimerExcel()
{
	WCHAR szWorkerThread[] = {'W','o','r','k','e','r','T','h','r','e','a','d','\0'};
	m_Functions.ExportListToExcel(&m_list, szWorkerThread, m_status);
}

void CWorkerThreadDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_WORK_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_WORK_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_WORKER_REFLASH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WORK_SUSPEND_THREAD, szSuspendThread[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WORK_RESUME_THREAD, szResumeThread[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WORK_DISASM, szDisassembleEntry[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WORK_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_WORK_VERIFY_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_WORK_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_WORKER_REFLASH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_WORK_DISASM, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_WORK_LOCATION_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_WORK_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_WORK_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(13, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount()) // 如果m_list是空的，那么除了“刷新”，其他全部置灰
	{
		for (int i = 2; i < 14; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt == 0) 
		{
			for (int i = 2; i < 13; i++) 
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		} 
		else if (nCnt == 1) 
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				CString szAddress = m_list.GetItemText(nItem, 2);
				if (szAddress.IsEmpty() || !szAddress.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_WORK_DISASM, MF_GRAYED | MF_DISABLED);
				}

				CString szModule = m_list.GetItemText(nItem, 3);
				if (szModule.IsEmpty() || !szModule.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_WORK_SHUXING, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_WORK_LOCATION_MODULE, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_WORK_VERIFY_SIGN, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_GRAYED | MF_DISABLED);
				}

				CString szThread = m_list.GetItemText(nItem, 0);
				DWORD dwThread = m_Functions.HexStringToLong(szThread);
				if (dwThread > 0x80000000)
				{
					if (IsThreadSuspened(dwThread))
					{
						menu.EnableMenuItem(ID_WORK_SUSPEND_THREAD, MF_GRAYED | MF_DISABLED);
					}
					else
					{
						menu.EnableMenuItem(ID_WORK_RESUME_THREAD, MF_GRAYED | MF_DISABLED);
					}
				}
				else
				{
					menu.EnableMenuItem(ID_WORK_SUSPEND_THREAD, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_WORK_RESUME_THREAD, MF_GRAYED | MF_DISABLED);
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

BOOL CWorkerThreadDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CWorkerThreadDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szModule = m_list.GetItemText(nItem, 3);
		m_Functions.JmpToFile(szModule);
	}
}
