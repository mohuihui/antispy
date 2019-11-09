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
// RegUnlockDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "RegUnlockDlg.h"


// CRegUnlockDlg dialog

IMPLEMENT_DYNAMIC(CRegUnlockDlg, CDialog)

CRegUnlockDlg::CRegUnlockDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegUnlockDlg::IDD, pParent)
{
	m_pRegLockedInfo = NULL;
	m_szCurrentUserKey = L"";
	m_szRegKey = NULL;
}

CRegUnlockDlg::~CRegUnlockDlg()
{
}

void CRegUnlockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CRegUnlockDlg, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(ID_UNLOCK_REFRESH, &CRegUnlockDlg::OnUnlockRefresh)
	ON_COMMAND(ID_UNLOCK_UNLOCK, &CRegUnlockDlg::OnUnlockUnlock)
	ON_COMMAND(ID_UNLOCK_UNLOCK_ALL, &CRegUnlockDlg::OnUnlockUnlockAll)
	ON_COMMAND(ID_UNLOCK_KILL_PROCESS, &CRegUnlockDlg::OnUnlockKillProcess)
	ON_COMMAND(ID_UNLOCK_JMP_TO_PROCESS, &CRegUnlockDlg::OnUnlockJmpToProcess)
	ON_COMMAND(ID_UNLOCK_EXPORT_TEXT, &CRegUnlockDlg::OnUnlockExportText)
	ON_COMMAND(ID_UNLOCK_EXPORT_EXCEL, &CRegUnlockDlg::OnUnlockExportExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CRegUnlockDlg::OnNMRclickList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CRegUnlockDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CRegUnlockDlg message handlers

BOOL CRegUnlockDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(szRegUnlocker[g_enumLang]);

	m_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ImageList, LVSIL_SMALL);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szFileLockedProcess[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, szFileLockedProcessPid[g_enumLang], LVCFMT_LEFT, 75);
	m_list.InsertColumn(2, szFileLockedHandle[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(3, szLockedRegistryPath[g_enumLang], LVCFMT_LEFT, 430);

	ShowLockedRegInformation(m_pRegLockedInfo);

	INIT_EASYSIZE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRegUnlockDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL RegUDgreaterPid( LOCKED_REGS elem1, LOCKED_REGS elem2 )
{
	return elem1.nPid < elem2.nPid;
}

VOID CRegUnlockDlg::SortByPid()
{
	sort( m_vectorLockedInfo.begin( ), m_vectorLockedInfo.end( ), RegUDgreaterPid );
}

BOOL CRegUnlockDlg::GetProcessList()
{
	return m_clsListProcess.EnumProcess(m_vectorProcess);
}

void CRegUnlockDlg::ShowLockedRegInformation(PLOCKED_REG_INFO pLockRegInfos)
{
	if (!pLockRegInfos || pLockRegInfos->nRetCount <= 0)
	{
		return;
	}

	int nImageCount = m_ImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ImageList.Remove(0);   
	}

	m_vectorProcessInfo.clear();
	m_list.DeleteAllItems();
	m_vectorLockedInfo.clear();
	for (ULONG i = 0; i < pLockRegInfos->nRetCount; i++)
	{
		m_vectorLockedInfo.push_back(pLockRegInfos->regs[i]);
	}

	if (m_vectorLockedInfo.size() == 0)
	{
		return;
	}

	SortByPid();
	GetProcessList();

	CString szImage, szPath;
	DWORD dwPrePid = 0;
	int nIcon = -1;
	DWORD dwEprocess = 0;
	for (vector<LOCKED_REGS>::iterator ir = m_vectorLockedInfo.begin();
		ir != m_vectorLockedInfo.end();
		ir++)
	{
		if (dwPrePid != ir->nPid)
		{
			dwPrePid = ir->nPid;

			for ( vector <PROCESS_INFO>::iterator itor = m_vectorProcess.begin( ); 
				itor != m_vectorProcess.end( ); 
				itor++ )
			{
				if (itor->ulPid == dwPrePid)
				{
					dwEprocess = itor->ulEprocess;
					szPath = itor->szPath;
					if (szPath.ReverseFind('\\') != -1)
					{
						szImage = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
					}
					else
					{
						szImage = szPath;
					}

					// 处理进程图标
					SHFILEINFO sfi;
					ZeroMemory(&sfi, sizeof(SHFILEINFO));
					SHGetFileInfo(szPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
					if (sfi.hIcon)
					{
						nIcon = m_ImageList.Add ( sfi.hIcon );
						DestroyIcon(sfi.hIcon);
					}
					else
					{
						nIcon = m_ImageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
					}

					break;
				}
			}
		}	

		int nItem = m_list.GetItemCount();
		m_list.InsertItem(nItem, szImage, nIcon);

		CString szPid;
		szPid.Format(L"%d", ir->nPid);
		m_list.SetItemText(nItem, 1, szPid);

		CString szHandle;
		szHandle.Format(L"0x%08X", ir->nHandle);
		m_list.SetItemText(nItem, 2, szHandle);

		CString szReg = m_RegCommon.NtKeyPath2DosKeyPath(ir->szRegPath);
		m_list.SetItemText(nItem, 3, szReg);

		LOCK_REG_PROCESS_INFO info;
		info.nHandle = ir->nHandle;
		info.nPid = ir->nPid;
		info.pEprocess = dwEprocess;
		info.szProcess = szPath;
		m_vectorProcessInfo.push_back(info);

		m_list.SetItemData(nItem, nItem);
	}	

	CString szTitle, szTemp;
	if (g_enumLang == enumEnglish)
	{
		szTemp = L" (Handles:";
	}
	else
	{
		szTemp = L" (句柄数:";
	}

	szTitle.Format(L"%s%s %d)", szRegUnlocker[g_enumLang], szTemp, pLockRegInfos->nRetCount);
	SetWindowText(szTitle);
}

void CRegUnlockDlg::OnUnlockRefresh()
{
	if (!m_szRegKey)
	{
		return;
	}

	COMMUNICATE_REG_LOCK cf;
	cf.OpType = enumQueryRegLockedInfo;
	cf.szRegPath = m_szRegKey;
	cf.nPathLen = wcslen(m_szRegKey) * sizeof(WCHAR);

	DWORD dwCnt = 0, dwCntTemp = 0, dwLen;
	PLOCKED_REG_INFO pInfos = NULL;
	BOOL bRet = FALSE;

	dwCnt = dwCntTemp = 0x1000;

	do 
	{
		DebugLog(L"nRetCount: %d", dwCntTemp);
		dwCnt = dwCntTemp;
		dwLen = sizeof(LOCKED_REG_INFO) + dwCnt * sizeof(LOCKED_REGS);
		if (pInfos)
		{
			free(pInfos);
			pInfos = NULL;
		}

		pInfos = (PLOCKED_REG_INFO)malloc(dwLen);
		if (!pInfos)
		{
			return;
		}

		memset(pInfos, 0, dwLen);

		bRet = m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_REG_LOCK), pInfos, dwLen, NULL);

	} while ( !bRet && (dwCntTemp = pInfos->nRetCount) > dwCnt );

	if (bRet && pInfos->nRetCount > 0)
	{
		DebugLog(L"nRetCount: %d", pInfos->nRetCount);
		ShowLockedRegInformation(pInfos);
	}
	else
	{
		CString szTitle, szTemp;
		if (g_enumLang == enumEnglish)
		{
			szTemp = L" (Handles:";
		}
		else
		{
			szTemp = L" (句柄数:";
		}

		szTitle.Format(L"%s%s %d)", szRegUnlocker[g_enumLang], szTemp, 0);
		SetWindowText(szTitle);

		m_list.DeleteAllItems();
	}

	free(pInfos);
}

void CRegUnlockDlg::OnUnlockUnlock()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(iIndex);
		LOCK_REG_PROCESS_INFO info = m_vectorProcessInfo.at(nData);

		COMMUNICATE_FILE cf;
		cf.OpType = enumCloseFileHandle;
		cf.op.CloseHandle.nPid = info.nPid;
		cf.op.CloseHandle.pEprocess = info.pEprocess;
		cf.op.CloseHandle.nHandle = info.nHandle;

		m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), NULL, 0, NULL);

		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

void CRegUnlockDlg::OnUnlockUnlockAll()
{
	ULONG nCnt = m_list.GetItemCount();
	for (ULONG nItem = 0; nItem < nCnt; nItem++)
	{
		int nData = (int)m_list.GetItemData(nItem);
		LOCK_REG_PROCESS_INFO info = m_vectorProcessInfo.at(nData);

		COMMUNICATE_FILE cf;
		cf.OpType = enumCloseFileHandle;
		cf.op.CloseHandle.nPid = info.nPid;
		cf.op.CloseHandle.pEprocess = info.pEprocess;
		cf.op.CloseHandle.nHandle = info.nHandle;

		m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), NULL, 0, NULL);
	}

	m_list.DeleteAllItems();
}

void CRegUnlockDlg::OnUnlockKillProcess()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szImage = m_list.GetItemText(nItem, 0);
		CString szShow;
		szShow.Format(szAreYOUSureKillProcess[g_enumLang], szImage);

		if (MessageBox(szShow, szToolName, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}

		int nData = (int)m_list.GetItemData(nItem);
		LOCK_REG_PROCESS_INFO info = m_vectorProcessInfo.at(nData);

		m_clsListProcess.KillProcess(0, info.pEprocess);

		OnUnlockRefresh();
	}
}

void CRegUnlockDlg::OnUnlockJmpToProcess()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		LOCK_REG_PROCESS_INFO info = m_vectorProcessInfo.at(nData);
		m_Functions.LocationExplorer(info.szProcess);
	}
}

void CRegUnlockDlg::OnUnlockExportText()
{
	// TODO: 在此添加命令处理程序代码
}

void CRegUnlockDlg::OnUnlockExportExcel()
{
	// TODO: 在此添加命令处理程序代码
}

void CRegUnlockDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_UNLOCK_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_UNLOCK_UNLOCK, szFileUnlock[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_UNLOCK_UNLOCK_ALL, szFileUnlockAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_UNLOCK_KILL_PROCESS, szFileKillLockProcess[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_UNLOCK_JMP_TO_PROCESS, szFileFindProcess[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_UNLOCK_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_UNLOCK_KILL_PROCESS, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_UNLOCK_JMP_TO_PROCESS, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 8; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 9; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			menu.EnableMenuItem(ID_UNLOCK_JMP_TO_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_UNLOCK_JMP_TO_FILE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_UNLOCK_KILL_PROCESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}

		menu.EnableMenuItem(ID_UNLOCK_UNLOCK_ALL, MF_BYCOMMAND | MF_ENABLED);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	*pResult = 0;
}
