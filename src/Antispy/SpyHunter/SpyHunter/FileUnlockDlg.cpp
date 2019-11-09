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
// FileUnlockDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FileUnlockDlg.h"


// CFileUnlockDlg dialog

IMPLEMENT_DYNAMIC(CFileUnlockDlg, CDialog)

CFileUnlockDlg::CFileUnlockDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileUnlockDlg::IDD, pParent)
{
	m_pLockFileInfos = NULL;
	m_LockedFilesVector.clear();
	m_szNtFilePath = NULL;
	m_bDirectory = FALSE;
}

CFileUnlockDlg::~CFileUnlockDlg()
{
	m_LockedFilesVector.clear();
}

void CFileUnlockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CFileUnlockDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFileUnlockDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFileUnlockDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CFileUnlockDlg::OnNMRclickList)
	ON_COMMAND(ID_UNLOCK_REFRESH, &CFileUnlockDlg::OnUnlockRefresh)
	ON_COMMAND(ID_UNLOCK_UNLOCK, &CFileUnlockDlg::OnUnlockUnlock)
	ON_COMMAND(ID_UNLOCK_UNLOCK_ALL, &CFileUnlockDlg::OnUnlockUnlockAll)
	ON_COMMAND(ID_UNLOCK_KILL_PROCESS, &CFileUnlockDlg::OnUnlockKillProcess)
	ON_COMMAND(ID_UNLOCK_JMP_TO_PROCESS, &CFileUnlockDlg::OnUnlockJmpToProcess)
	ON_COMMAND(ID_UNLOCK_JMP_TO_FILE, &CFileUnlockDlg::OnUnlockJmpToFile)
	ON_COMMAND(ID_UNLOCK_EXPORT_TEXT, &CFileUnlockDlg::OnUnlockExportText)
	ON_COMMAND(ID_UNLOCK_EXPORT_EXCEL, &CFileUnlockDlg::OnUnlockExportExcel)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CFileUnlockDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CFileUnlockDlg message handlers

void CFileUnlockDlg::OnBnClickedOk()
{
}

void CFileUnlockDlg::OnBnClickedCancel()
{
	OnCancel();
}

BOOL CFileUnlockDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	InitDevice2Path();

	SetWindowText(szFileUnlocker[g_enumLang]);
	
	m_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ImageList, LVSIL_SMALL);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szFileLockedProcess[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, szFileLockedProcessPid[g_enumLang], LVCFMT_LEFT, 75);
	m_list.InsertColumn(2, szFileLockedHandle[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(3, szFileLockedFilePath[g_enumLang], LVCFMT_LEFT, 430);

	ShowLockedFileInformation(m_pLockFileInfos);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFileUnlockDlg::GetProcessList()
{
 	return m_clsListProcess.EnumProcess(m_vectorProcess);
}

BOOL UDgreaterPid( LOCKED_FILES elem1, LOCKED_FILES elem2 )
{
	return elem1.nPid < elem2.nPid;
}

VOID CFileUnlockDlg::SortByPid()
{
	sort( m_LockedFilesVector.begin( ), m_LockedFilesVector.end( ), UDgreaterPid );
}

CString CFileUnlockDlg::FixFilePath(WCHAR* szPath)
{
	CString szRet;
	if (!szPath)
	{
		return szRet;
	}

	for (vector<NT_AND_DOS_VOLUME_NAME>::iterator ir = m_nt_and_dos_valume_list.begin();
		ir != m_nt_and_dos_valume_list.end();
		ir++)
	{
		WCHAR *szNtNameTemp = ir->szDosName.GetBuffer();

		if (!_wcsnicmp(szPath, szNtNameTemp, ir->szDosName.GetLength()))
		{
			ir->szDosName.ReleaseBuffer();
			szRet = ir->szNtName;
			szRet += (szPath + ir->szDosName.GetLength() + 1);
			break;
		}

		ir->szDosName.ReleaseBuffer();
	}

	return szRet;
}

void CFileUnlockDlg::ShowLockedFileInformation(PLOCKED_FILE_INFO pLockFileInfos)
{
	if (!pLockFileInfos || pLockFileInfos->nRetCount <= 0)
	{
		return;
	}
	
	int nImageCount = m_ImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ImageList.Remove(0);   
	}

	m_lockFileProcessList.clear();
	m_list.DeleteAllItems();
	m_LockedFilesVector.clear();
	for (ULONG i = 0; i < pLockFileInfos->nRetCount; i++)
	{
		m_LockedFilesVector.push_back(pLockFileInfos->files[i]);
	}
	
	if (m_LockedFilesVector.size() == 0)
	{
		return;
	}

	SortByPid();
	GetProcessList();

	CString szImage, szPath;
	DWORD dwPrePid = 0;
	int nIcon = -1;
	DWORD dwEprocess = 0;
	for (vector<LOCKED_FILES>::iterator ir = m_LockedFilesVector.begin();
		ir != m_LockedFilesVector.end();
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

		CString szFile = FixFilePath(ir->szFilePath);
		m_list.SetItemText(nItem, 3, szFile);

		LOCK_FILL_PROCESS_INFO info;
		info.nHandle = ir->nHandle;
		info.nPid = ir->nPid;
		info.pEprocess = dwEprocess;
		info.szProcess = szPath;
		info.szFile = szFile;
		m_lockFileProcessList.push_back(info);

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

	szTitle.Format(L"%s%s %d)", szFileUnlocker[g_enumLang], szTemp, pLockFileInfos->nRetCount);
	SetWindowText(szTitle);
}

void CFileUnlockDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

// 取得dos name和nt name匹配
// eg. "\Device\HarddiskVolume1"  -  "C:\"
BOOL CFileUnlockDlg::InitDevice2Path()
{
	BOOL   bRet               = FALSE;  
	DWORD  CharCount           = 0;
	WCHAR  DeviceName[MAX_PATH] = L"";
	DWORD  Error              = ERROR_SUCCESS;
	HANDLE FindHandle          = INVALID_HANDLE_VALUE;
	BOOL   Found              = FALSE;
	size_t Index              = 0;
	BOOL   Success                = FALSE;
	WCHAR  VolumeName[MAX_PATH] = L"";

	//
	//  Enumerate all volumes in the system.
	FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

	if (FindHandle == INVALID_HANDLE_VALUE)
	{
		Error = GetLastError();
		wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
		return bRet;
	}

	for (;;)
	{
		//
		//  Skip the \\?\ prefix and remove the trailing backslash.
		Index = wcslen(VolumeName) - 1;

		if (VolumeName[0]     != L'\\' ||
			VolumeName[1]     != L'\\' ||
			VolumeName[2]     != L'?'  ||
			VolumeName[3]     != L'\\' ||
			VolumeName[Index] != L'\\') 
		{
			Error = ERROR_BAD_PATHNAME;
			wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
			break;
		}

		//
		//  QueryDosDeviceW doesn't allow a trailing backslash,
		//  so temporarily remove it.
		VolumeName[Index] = L'\0';
		CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 
		VolumeName[Index] = L'\\';
		if ( CharCount == 0 ) 
		{
			Error = GetLastError();
			wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
			break;
		}
		ConvertVolumePaths(DeviceName, VolumeName);
		//
		//  Move on to the next volume.
		Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));


		if ( !Success ) 
		{
			Error = GetLastError();
			if (Error != ERROR_NO_MORE_FILES) 
			{
				wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
				break;
			}

			//
			//  Finished iterating
			//  through all the volumes.
			Error = ERROR_SUCCESS;
			break;
		}
	}

	FindVolumeClose(FindHandle);
	FindHandle = INVALID_HANDLE_VALUE;
	return bRet;
}

void CFileUnlockDlg::ConvertVolumePaths(
								  IN PWCHAR DeviceName,
								  IN PWCHAR VolumeName
								  )

{
	DWORD  CharCount = MAX_PATH + 1;
	PWCHAR Names     = NULL;
	PWCHAR NameIdx      = NULL;
	BOOL   Success      = FALSE;


	for (;;) 
	{
		//
		//  Allocate a buffer to hold the paths.
		Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];

		if ( !Names ) 
		{
			//
			//  If memory can't be allocated, return.
			return;
		}



		//
		//  Obtain all of the paths
		//  for this volume.
		Success = GetVolumePathNamesForVolumeNameW(
			VolumeName, Names, CharCount, &CharCount
			);


		if ( Success ) 
		{
			break;
		}


		if ( GetLastError() != ERROR_MORE_DATA ) 
		{
			break;
		}

		//
		//  Try again with the
		//  new suggested size.
		delete [] Names;
		Names = NULL;
	}


	if ( Success )
	{
		//
		//  Display the various paths.
		for ( NameIdx = Names; 
			NameIdx[0] != L'\0'; 
			NameIdx += wcslen(NameIdx) + 1 ) 
		{
			NT_AND_DOS_VOLUME_NAME name;
			name.szDosName = DeviceName;
			name.szNtName = NameIdx;
			m_nt_and_dos_valume_list.push_back(name);
			//	g_mapDevice2Path[DeviceName] = NameIdx;
		}
	}

	if ( Names != NULL ) 
	{
		delete [] Names;
		Names = NULL;
	}
}

void CFileUnlockDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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
	menu.AppendMenu(MF_STRING, ID_UNLOCK_JMP_TO_FILE, szFileFindFile[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_UNLOCK_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_UNLOCK_KILL_PROCESS, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_UNLOCK_JMP_TO_PROCESS, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_UNLOCK_JMP_TO_FILE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 9; i++)
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

// 刷新
void CFileUnlockDlg::OnUnlockRefresh()
{
	if (!m_szNtFilePath)
	{
		return;
	}

	COMMUNICATE_FILE cf;
	cf.OpType = enumQueryFileLockInfo;
	cf.op.QueryFileLock.szPath = m_szNtFilePath;
	cf.op.QueryFileLock.nPathLen = wcslen(m_szNtFilePath) * sizeof(WCHAR);
	cf.op.QueryFileLock.bDirectory = m_bDirectory;

	DWORD dwCnt, dwCntTemp;
	PLOCKED_FILE_INFO pInfos = NULL;
	BOOL bRet = FALSE;

	dwCnt = dwCntTemp = 0x1000;

	do 
	{
		dwCnt = dwCntTemp;
		DWORD dwLen = sizeof(LOCKED_FILE_INFO) + dwCnt * sizeof(LOCKED_FILES);
		if (pInfos)
		{
			free(pInfos);
			pInfos = NULL;
		}

		pInfos = (PLOCKED_FILE_INFO)malloc(dwLen);
		if (!pInfos)
		{
			return;
		}

		memset(pInfos, 0, dwLen);

		bRet = m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), pInfos, dwLen, NULL);

	} while ( !bRet && (dwCntTemp = pInfos->nRetCount) > dwCnt );

	if (bRet && pInfos->nRetCount > 0)
	{
		DebugLog(L"nRetCount: %d", pInfos->nRetCount);
		ShowLockedFileInformation(pInfos);
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

		szTitle.Format(L"%s%s %d)", szFileUnlocker[g_enumLang], szTemp, 0);
		SetWindowText(szTitle);

		m_list.DeleteAllItems();
	}

	free(pInfos);
}

// 解锁
void CFileUnlockDlg::OnUnlockUnlock()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(iIndex);
		LOCK_FILL_PROCESS_INFO info = m_lockFileProcessList.at(nData);

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

// 解锁全部
void CFileUnlockDlg::OnUnlockUnlockAll()
{
	ULONG nCnt = m_list.GetItemCount();
	for (ULONG nItem = 0; nItem < nCnt; nItem++)
	{
		int nData = (int)m_list.GetItemData(nItem);
		LOCK_FILL_PROCESS_INFO info = m_lockFileProcessList.at(nData);

		COMMUNICATE_FILE cf;
		cf.OpType = enumCloseFileHandle;
		cf.op.CloseHandle.nPid = info.nPid;
		cf.op.CloseHandle.pEprocess = info.pEprocess;
		cf.op.CloseHandle.nHandle = info.nHandle;

		m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), NULL, 0, NULL);
	}

	m_list.DeleteAllItems();
}

// 结束锁定进程
void CFileUnlockDlg::OnUnlockKillProcess()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szImage = m_list.GetItemText(nItem, 0);
		CString szShow;
		szShow.Format(szAreYOUSureKillProcess[g_enumLang], szImage);

		if (MessageBox(szShow, L"AntiSpy", MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}

		int nData = (int)m_list.GetItemData(nItem);
		LOCK_FILL_PROCESS_INFO info = m_lockFileProcessList.at(nData);

		m_clsListProcess.KillProcess(0, info.pEprocess);
		
		OnUnlockRefresh();
	}
}

// 定位到锁定进程
void CFileUnlockDlg::OnUnlockJmpToProcess()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		LOCK_FILL_PROCESS_INFO info = m_lockFileProcessList.at(nData);
		m_Functions.LocationExplorer(info.szProcess);
	}
}

// 定位到锁定文件
void CFileUnlockDlg::OnUnlockJmpToFile()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		LOCK_FILL_PROCESS_INFO info = m_lockFileProcessList.at(nData);
		m_Functions.LocationExplorer(info.szFile);
	}
}

// 导出Text
void CFileUnlockDlg::OnUnlockExportText()
{
	// TODO: Add your command handler code here
}

// 导出Excel
void CFileUnlockDlg::OnUnlockExportExcel()
{
	// TODO: Add your command handler code here
}
