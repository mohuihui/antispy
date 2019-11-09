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
// FileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FileDlg.h"
#include <algorithm>
#include "FileUnlockDlg.h"
#include "GetFilePathDlg.h"
#include "FileRenameDlg.h"
#include "FileRestartDeleteDlg.h"
#include "HashCalcDlg.h"
// #include "ParseNTFS.h"
// #include "ntfs/ntfs_header.h"

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

#define FILE_SYNCHRONOUS_IO_NONALERT    0x00000020


IMPLEMENT_DYNAMIC(CFileDlg, CDialog)


// CFileDlg dialog

CFileDlg::CFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileDlg::IDD, pParent)
	, m_szCombo(_T(""))
	, m_szAddress(_T(""))
{
	m_hDirectoryIcon = NULL;
	m_szCurrentPath = L"";
	m_hCurrentTreeItem = NULL;
	m_bEnumTree = TRUE;
	m_nt_and_dos_valume_list.clear();

	m_clrNormalFile = RGB(0, 0, 0);						
	m_clrHideFile = RGB( 127, 127, 127 );				
	m_clrSystemAndHideFile = RGB( 255, 0, 0 );			
	m_clrSystemFile = RGB( 255, 0, 0 );

	m_bInComboBox = FALSE;
	m_bInListCtrl = FALSE;
}

CFileDlg::~CFileDlg()
{
	m_nt_and_dos_valume_list.clear();
}

void CFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_CBString(pDX, IDC_COMBO, m_szCombo);
	DDX_Text(pDX, IDC_STATIC_ADDRESS, m_szAddress);
	DDX_Control(pDX, IDC_GOTO, m_BtnGoto);
	DDX_Control(pDX, IDC_COMBO, m_ComboBox);
}


BEGIN_MESSAGE_MAP(CFileDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFileDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFileDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, &CFileDlg::OnTvnSelchangedTree)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CFileDlg::OnNMCustomdrawList)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TREE, &CFileDlg::OnTvnItemexpandedTree)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CFileDlg::OnNMDblclkList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CFileDlg::OnNMRclickList)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, &CFileDlg::OnNMRclickTree)
	ON_COMMAND(ID_FILE_OPEN, &CFileDlg::OnFileOpen)
	ON_COMMAND(ID_FILE_REFRESH, &CFileDlg::OnFileRefresh)
	ON_COMMAND(ID_FILE_LOCK_INFO, &CFileDlg::OnFileLockInfo)
	ON_COMMAND(ID_FILE_DELETE, &CFileDlg::OnFileDelete)
	ON_COMMAND(ID_FILE_DELETE_AND_DENY_REBUILD, &CFileDlg::OnFileDeleteAndDenyRebuild)
	ON_COMMAND(ID_FILE_ADD_TO_RESTART_DELETE, &CFileDlg::OnFileAddToRestartDelete)
	ON_COMMAND(ID_FILE_RESTART_REPLACE, &CFileDlg::OnFileRestartReplace)
	ON_COMMAND(ID_FILE_COPY_TO, &CFileDlg::OnFileCopyTo)
	ON_COMMAND(ID_FILE_RENAME, &CFileDlg::OnFileRename)
	ON_COMMAND(ID_FILE_COPY_FILE_NAME, &CFileDlg::OnFileCopyFileName)
	ON_COMMAND(ID_FILE_COPY_FILE_PATH, &CFileDlg::OnFileCopyFilePath)
	ON_COMMAND(ID_FILE_SHUXING, &CFileDlg::OnFileShuxing)
	ON_COMMAND(ID_FILE_JMP_TO_EXPLORER, &CFileDlg::OnFileJmpToExplorer)
	ON_COMMAND(ID_FILE_DELETE_HIDE_SYSTEM_SHUXING, &CFileDlg::OnFileDeleteHideSystemShuxing)
	ON_COMMAND(ID_FILE_CHECK_VERIFY, &CFileDlg::OnFileCheckVerify)
	ON_COMMAND(ID_FILE_CHECK_ALL_VERIFY, &CFileDlg::OnFileCheckAllVerify)
	ON_COMMAND(ID_FILE_TREE_REFRESH, &CFileDlg::OnFileTreeRefresh)
	ON_COMMAND(ID_FILE_TREE_LOCK_INFO, &CFileDlg::OnFileTreeLockInfo)
	ON_COMMAND(ID_FILE_TREE_DELETE, &CFileDlg::OnFileTreeDelete)
	ON_COMMAND(ID_FILE_TREE_COPY_FILE_NAME, &CFileDlg::OnFileTreeCopyFileName)
	ON_COMMAND(ID_FILE_TREE_COPY_FILE_PATH, &CFileDlg::OnFileTreeCopyFilePath)
	ON_COMMAND(ID_FILE_TREE_SHUXING, &CFileDlg::OnFileTreeShuxing)
	ON_COMMAND(ID_FILE_TREE_GOTO_EXPLORER, &CFileDlg::OnFileTreeGotoExplorer)
	ON_COMMAND(ID_FILE_TREE_DELETE_ALL_SHUXING, &CFileDlg::OnFileTreeDeleteAllShuxing)
	ON_BN_CLICKED(IDC_GOTO, &CFileDlg::OnBnClickedGoto)
	ON_COMMAND(ID_FILE_VIEW_FILE_LOCKED_INFO, &CFileDlg::OnFileViewFileLockedInfo)
	ON_COMMAND(ID_FILE_TREE_VIEW_FILE_LOCKED_INFO, &CFileDlg::OnFileTreeViewFileLockedInfo)
	ON_COMMAND(ID_FILE_TREE_SET_HIDE_SYSTEM_ATTRIBUTE, &CFileDlg::OnFileTreeSetHideSystemAttribute)
	ON_COMMAND(ID_FILE_SET_HIDE_SYSTEM_ATTRIBUTE, &CFileDlg::OnFileSetHideSystemAttribute)
	ON_COMMAND(ID_FILE_TREE_COPY_TO, &CFileDlg::OnFileTreeCopyTo)
	ON_COMMAND(ID_FILE_TREE_RENAME, &CFileDlg::OnFileTreeRename)
	ON_CBN_DROPDOWN(IDC_COMBO, &CFileDlg::OnCbnDropdownCombo)
	ON_COMMAND(ID_FILE_CHECK_ONLINE, &CFileDlg::OnFileCheckOnline)
	ON_COMMAND(ID_FILE_CALC_MD5, &CFileDlg::OnFileCalcMd5)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST, &CFileDlg::OnNMSetfocusList)
	ON_NOTIFY(NM_KILLFOCUS, IDC_LIST, &CFileDlg::OnNMKillfocusList)
	ON_CBN_SETFOCUS(IDC_COMBO, &CFileDlg::OnCbnSetfocusCombo)
	ON_CBN_KILLFOCUS(IDC_COMBO, &CFileDlg::OnCbnKillfocusCombo)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CFileDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_X_SP, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
 	EASYSIZE(IDC_TREE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
// 	EASYSIZE(IDC_REGISTRY_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
 	EASYSIZE(IDC_STATIC_ADDRESS, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_COMBO, ES_BORDER, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_GOTO, ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
END_EASYSIZE_MAP
// CFileDlg message handlers

void CFileDlg::OnBnClickedOk()
{
}

void CFileDlg::OnBnClickedCancel()
{
}

BOOL CFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_hDirectoryIcon = AfxGetApp()->LoadIcon (IDI_CLOSE_DIRECTORY);

	if (g_bLoadDriverOK)
	{
		EmnuVolume();
		InitDevice2Path();
	}
	
	m_list.SetFocus();

	m_FileImageList.Create(16,16, TRUE|ILC_COLOR24, 0, 3);
	m_list.SetImageList(&m_FileImageList, LVSIL_SMALL);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szFileName[g_enumLang], LVCFMT_LEFT, 250);
	m_list.InsertColumn(1, szFileSizeEx[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(2, szCreationTime[g_enumLang], LVCFMT_LEFT, 125);
	m_list.InsertColumn(3, szModificationTime[g_enumLang], LVCFMT_LEFT, 125);
	m_list.InsertColumn(4, szFileType[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(5, szFileAttributes[g_enumLang], LVCFMT_LEFT, 200);
	
	m_szAddress = szRegAddress[g_enumLang];

	m_BtnGoto.SetWindowText(szGoto[g_enumLang]);
	UpdateData(FALSE);

	m_clrNormalFile = g_Config.GetNormalFileColor();
	m_clrHideFile = g_Config.GetHiddenFileColor();
	m_clrSystemAndHideFile = g_Config.GetSystemAndHiddenFileColor();
	m_clrSystemFile = g_Config.GetSystemFileColor();
	
// 	PVOID pBuffer = malloc(512);
// 	read_sector_ex(pBuffer, 0, 1, 'C');
// 	free(pBuffer);
	
//	enum_file_by_parse_disk();
//	ntfs_fuse_init();
	m_xSplitter.BindWithControl(this, IDC_X_SP);
	m_xSplitter.SetMinWidth(0, 0);
	
	m_xSplitter.AttachAsLeftPane(IDC_TREE);
	m_xSplitter.AttachAsRightPane(IDC_LIST);

	m_xSplitter.RecalcLayout();

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFileDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CFileDlg::InitComboBox()
{
	if (!g_bLoadDriverOK)
	{
		return;
	}

	int i = 0;
	WCHAR szSystemDir[MAX_PATH] = {0};

	// c:\\windows
	GetWindowsDirectory(szSystemDir, MAX_PATH);
	m_ComboBox.InsertString(i++, szSystemDir);
	
	// %SystemRoot%\Temp
	WCHAR szTemp[] = {'\\','T','e','m','p','\0'};
	wcsncat_s(szSystemDir, MAX_PATH - wcslen(szSystemDir), szTemp, wcslen(szTemp));
	m_ComboBox.InsertString(i++, szSystemDir);

	// C:\\Windows\\\System32
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	GetSystemDirectory(szSystemDir, MAX_PATH);
	m_ComboBox.InsertString(i++, szSystemDir);

	// %SystemRoot%\system32\drivers
	WCHAR szDrivers[] = {'\\','d','r','i','v','e','r','s','\0'};
	wcsncat_s(szSystemDir, MAX_PATH - wcslen(szSystemDir), szDrivers, wcslen(szDrivers));
	m_ComboBox.InsertString(i++, szSystemDir);

	// %ALLUSERSPROFILE%
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_COMMON_APPDATA, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// %APPDATA%
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_APPDATA, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Windows\Fonts
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_FONTS, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\\Program Files
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_PROGRAM_FILES, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// USERPROFILE
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_PROFILE, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Users\Mzf\Desktop
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_DESKTOP, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Users\Public\Desktop
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Users\Mzf\Favorites
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_FAVORITES, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Users\Mzf\AppData\Local
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_LOCAL_APPDATA, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// %TEMP%
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	GetTempPath(MAX_PATH, szSystemDir);
	if (szSystemDir[wcslen(szSystemDir) - 1] == '\\')
	{
		szSystemDir[wcslen(szSystemDir) - 1] = '\0';
	}
	CString szTempXx = m_Functions.GetLongPath((CString)szSystemDir);
	m_ComboBox.InsertString(i++, szTempXx);

	// C:\Users\Mzf\AppData\Roaming
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_APPDATA, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Users\Mzf\AppData\Roaming\Microsoft\Windows\SendTo
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_SENDTO, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Users\Mzf\AppData\Roaming\Microsoft\Windows\Start Menu
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_STARTMENU, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}
	
	// C:\Users\Mzf\AppData\Roaming\Microsoft\Windows\Templates
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_TEMPLATES, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\Users\Mzf\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_STARTUP, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\ProgramData\Microsoft\Windows\Start Menu
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_COMMON_STARTMENU, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\ProgramData\Microsoft\Windows\Start Menu\Programs
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_COMMON_PROGRAMS, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}

	// C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup
	memset(szSystemDir, 0, sizeof(WCHAR) * MAX_PATH);
	if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szSystemDir, CSIDL_COMMON_STARTUP, FALSE))
	{
		m_ComboBox.InsertString(i++, szSystemDir);
	}
}

BOOL CFileDlg::IsPathHaveDirectory(CString szPath)
{
	BOOL bRet = FALSE;

	if (szPath.IsEmpty())
	{
		return FALSE;
	}
	
	FixDotPath(szPath);

	if ( szPath.GetAt(szPath.GetLength() - 1) != '\\')
	{
		szPath += L"\\";
	}

	if ( !PathFileExists(szPath) )
	{
		return FALSE;
	}

	DWORD dwLen = (szPath.GetLength() + 1) * sizeof(WCHAR);
	WCHAR *strPath = (WCHAR*)malloc(dwLen);
	if (!strPath)
	{
		return bRet;
	}

	memset(strPath, 0, dwLen);
	wcsncpy_s(strPath, szPath.GetLength() + 1, szPath.GetBuffer(), szPath.GetLength());
	szPath.ReleaseBuffer();

	HANDLE hFile = m_FileFunc.Create(
		strPath, 
		dwLen, 
		FILE_LIST_DIRECTORY, 
		FILE_ATTRIBUTE_NORMAL, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (hFile)
	{	
		ULONG nBytes = sizeof(FILE_DIRECTORY_INFORMATION) + MAX_PATH * 2 * sizeof(WCHAR);
		PFILE_DIRECTORY_INFORMATION pTemp = (PFILE_DIRECTORY_INFORMATION)malloc(nBytes);
		if (!pTemp)
		{
			free(strPath);
			strPath = NULL;
			CloseHandle(hFile);
			return FALSE;
		}
		
		memset(pTemp, 0, nBytes);
		ULONG ulRet = 0;
		if (m_FileFunc.QueryDirectoryFile(hFile, pTemp, nBytes, 1, TRUE, TRUE, &ulRet))
		{
			if ( (pTemp->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				CString(pTemp->FileName).CompareNoCase(L"..") &&
				CString(pTemp->FileName).CompareNoCase(L"."))
			{
				bRet = TRUE;
			}
			else
			{
				memset(pTemp, 0, nBytes);
				while (m_FileFunc.QueryDirectoryFile(hFile, pTemp, nBytes, 1, TRUE, FALSE, &ulRet))
				{
					if ( (pTemp->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
						CString(pTemp->FileName).CompareNoCase(L"..") &&
						CString(pTemp->FileName).CompareNoCase(L"."))
					{
						bRet = TRUE;
						break;
					}

					memset(pTemp, 0, nBytes);
				}
			}
		}

		free(pTemp);
		pTemp = NULL;

		CloseHandle(hFile);
	}
	
	free(strPath);
	strPath = NULL;

	if (szPath.Find('.') != -1 && bRet == FALSE)
	{
		CFileFind finder;
		CString path;
		path.Format(L"%s/*.*", szPath);
		BOOL bWorking = finder.FindFile(path);
		while(bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDirectory() && !finder.IsDots())
			{
				bRet = TRUE;
				break;
			}
		}

		finder.Close();
	}

	return bRet;
}

void CFileDlg::EmnuVolume()
{
	int i_count = 6;
	HICON icon[10];
	icon[0] = AfxGetApp()->LoadIcon (IDI_COMPUTER);
	icon[1] = AfxGetApp()->LoadIcon (IDI_FLOOY);
	icon[2] = AfxGetApp()->LoadIcon (IDI_HARD_DISK);
	icon[3] = AfxGetApp()->LoadIcon (IDI_CDROOM);
	icon[4] = AfxGetApp()->LoadIcon (IDI_CLOSE_DIRECTORY);
	icon[5] = AfxGetApp()->LoadIcon (IDI_OPEN_DIRECTORY);

	m_TreeImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 2); 
	for(int n = 0; n < i_count; n++)
	{
		m_TreeImageList.Add(icon[n]);
		m_tree.SetImageList(&m_TreeImageList, LVSIL_NORMAL);
	}

	DWORD dwStyle = GetWindowLong(m_tree.m_hWnd, GWL_STYLE);
	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	::SetWindowLong (m_tree.m_hWnd, GWL_STYLE, dwStyle);

	HTREEITEM Computer = m_tree.InsertItem(szMyComputer[g_enumLang], 0, 0);
	
	DWORD  dwInfo = GetLogicalDrives();  
	int m = 0;  
	while(dwInfo) 
	{ 
		if (dwInfo & 1) 
		{  
			WCHAR ch = 'A' + m; 
			CString chs;
			chs.Format(L"%c:\\", ch);

			int dt = GetDriveType(chs); 
			CString szDisk;
			
			HTREEITEM hTtem = NULL;
			switch (dt)
			{
			case DRIVE_UNKNOWN:
				szDisk.Format(L"%s (%c:)", szDiskUnknow[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, Computer, TVI_LAST);
				break;

			case DRIVE_NO_ROOT_DIR:
				break;

			case DRIVE_REMOVABLE:
				szDisk.Format(L"%s (%c:)", szRemovableDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 1, 1, Computer, TVI_LAST);
				break;

			case DRIVE_FIXED:
				szDisk.Format(L"%s (%c:)", szLocalDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, Computer, TVI_LAST);
				break;

			case DRIVE_REMOTE:
				szDisk.Format(L"%s (%c:)", szRemoteDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, Computer, TVI_LAST);
				break;
				
			case DRIVE_CDROM:
				szDisk.Format(L"%s (%c:)", szCDDrive[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 3, 3, Computer, TVI_LAST);
				break;

			case DRIVE_RAMDISK:
				szDisk.Format(L"%s (%c:)", szRamDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, Computer, TVI_LAST);
				break;
			}

			if (hTtem)
			{
				FILE_VOLUME_INFO info;
				info.hItem = hTtem;
				info.szVolume = chs;
				m_VolumeList.push_back(info);

				if (IsPathHaveDirectory(chs))
				{
					m_tree.InsertItem(L"1", hTtem, TVI_LAST);
				}
			}
		}

		dwInfo = dwInfo>>1; 
		m += 1; 
	}  

	m_tree.Expand(Computer, TVE_EXPAND);
}

BOOL CFileDlg::IsVolumeItem(HTREEITEM hItem, CString &szVolume)
{
	if (hItem)
	{
		for (list<FILE_VOLUME_INFO>::iterator ir = m_VolumeList.begin();
			ir != m_VolumeList.end();
			ir++)
		{
			if (ir->hItem == hItem)
			{
				szVolume = ir->szVolume;
				return TRUE;
			}
		}
	}

	return FALSE;
}

void CFileDlg::GetFiles(CString szPath, vector<FILE_INFO> &FileInfoList)
{
	if (szPath.IsEmpty())
	{
		return;
	}

	if ( szPath.GetAt(szPath.GetLength() - 1) != '\\' )
	{
		szPath += L"\\";
	}

	if ( !PathFileExists(szPath) )
	{
		return;
	}

	DWORD dwLen = (szPath.GetLength() + 1) * sizeof(WCHAR);
	WCHAR *strPath = (WCHAR*)malloc(dwLen);
	if (!strPath)
	{
		return;
	}

	FileInfoList.clear();

	memset(strPath, 0, dwLen);
	wcsncpy_s(strPath, szPath.GetLength() + 1, szPath.GetBuffer(), szPath.GetLength());
	szPath.ReleaseBuffer();

	HANDLE hFile = m_FileFunc.Create(
		strPath, 
		dwLen, 
		FILE_LIST_DIRECTORY, 
		FILE_ATTRIBUTE_NORMAL, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (hFile)
	{	
		ULONG nBytes = sizeof(FILE_DIRECTORY_INFORMATION) + MAX_PATH * 2 * sizeof(WCHAR);
		PFILE_DIRECTORY_INFORMATION pTemp = (PFILE_DIRECTORY_INFORMATION)malloc(nBytes);
		if (!pTemp)
		{
			free(strPath);
			strPath = NULL;
			CloseHandle(hFile);
			return;
		}

		memset(pTemp, 0, nBytes);
		ULONG ulRet = 0;
		if (m_FileFunc.QueryDirectoryFile(hFile, pTemp, nBytes, 1, TRUE, TRUE, &ulRet))
		{
			CString szTemp = pTemp->FileName;
			if ( ((pTemp->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && szTemp.CompareNoCase(L".")) ||
				!(pTemp->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FILE_INFO info;
				info.CreationTime = pTemp->CreationTime;
				info.ModificationTime = pTemp->LastWriteTime;
				info.FileAttributes = pTemp->FileAttributes;
				info.FileSize = pTemp->AllocationSize;
				info.FileName = pTemp->FileName;
				FileInfoList.push_back(info);
			}

			memset(pTemp, 0, nBytes);
			while (m_FileFunc.QueryDirectoryFile(hFile, pTemp, nBytes, 1, TRUE, FALSE, &ulRet))
			{
				CString szTemp = pTemp->FileName;
				if ( ((pTemp->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && szTemp.CompareNoCase(L".")) ||
					!(pTemp->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					FILE_INFO info;
					info.CreationTime = pTemp->CreationTime;
					info.ModificationTime = pTemp->LastWriteTime;
					info.FileAttributes = pTemp->FileAttributes;
					info.FileSize = pTemp->AllocationSize;
					info.FileName = pTemp->FileName;
					FileInfoList.push_back(info);
				}

				memset(pTemp, 0, nBytes);
			}
		}

		free(pTemp);
		pTemp = NULL;

		CloseHandle(hFile);
	}

	free(strPath);
	strPath = NULL;

	if (FileInfoList.size() == 0)
	{
		CFileFind finder;
		CString path;
		path.Format(L"%s/*.*", szPath);
		BOOL bWorking = finder.FindFile(path);
		while(bWorking)
		{
			bWorking = finder.FindNextFile();
			CString szFile = finder.GetFilePath();

			ULONG FileAttributes = 0;
			if (finder.IsDirectory())
			{
				FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			}

			if ( ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (finder.GetFileName()).CompareNoCase(L".")) ||
				!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FILE_INFO info;
				info.FileAttributes = 0;

				finder.GetCreationTime((FILETIME*)&info.CreationTime);
				finder.GetLastWriteTime((FILETIME*)&info.ModificationTime);

				if (finder.IsArchived())
				{
					FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
				}

				if (finder.IsCompressed())
				{
					FileAttributes |= FILE_ATTRIBUTE_COMPRESSED;
				}

				if (finder.IsHidden())
				{
					FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
				}

				if (finder.IsNormal())
				{
					FileAttributes |= FILE_ATTRIBUTE_NORMAL;
				}

				if (finder.IsReadOnly())
				{
					FileAttributes |= FILE_ATTRIBUTE_READONLY;
				}

				if (finder.IsSystem())
				{
					FileAttributes |= FILE_ATTRIBUTE_SYSTEM;
				}

				if (finder.IsTemporary())
				{
					FileAttributes |= FILE_ATTRIBUTE_TEMPORARY;
				}

				info.FileAttributes = FileAttributes;
				info.FileSize.QuadPart = finder.GetLength();
				info.FileName = finder.GetFileName();

				FileInfoList.push_back(info);
			}
		}

		finder.Close();
	}
}

BOOL FileAttributesSort( FILE_INFO elem1, FILE_INFO elem2 )
{
	return (elem1.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) > (elem2.FileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

VOID CFileDlg::SortByFileAttributes()
{
	sort(m_FileInfoList.begin( ), m_FileInfoList.end( ), FileAttributesSort);
}

CString CFileDlg::GetFileAttributes(ULONG nAttributes)
{
	CString szRet;

	if (nAttributes & FILE_ATTRIBUTE_SYSTEM)
	{
		szRet += CString(L" & ") + szFileSYSTEM[g_enumLang];
	}

	if (nAttributes & FILE_ATTRIBUTE_HIDDEN)
	{
		szRet += CString(L" & ") + szFileHIDDEN[g_enumLang];
	}

	if (nAttributes & FILE_ATTRIBUTE_READONLY)
	{
		szRet += CString(L" & ") + szFileReadOnly[g_enumLang];
	}

// 	if (nAttributes & FILE_ATTRIBUTE_DIRECTORY)
// 	{
// 		szRet += CString(L" & ") + szFileDIRECTORY[g_enumLang];
// 	}
 
// 	if (nAttributes & FILE_ATTRIBUTE_ARCHIVE)
// 	{
// 		szRet += CString(L" & ") + szFileARCHIVE[g_enumLang];
// 	}

// 	if (nAttributes & FILE_ATTRIBUTE_NORMAL)
// 	{
// 		szRet += CString(L" & ") + szFileNORMAL[g_enumLang];
// 	}

	if (nAttributes & FILE_ATTRIBUTE_TEMPORARY)
	{
		szRet += CString(L" & ") + szFileTEMPORARY[g_enumLang];
	}

	if (nAttributes & FILE_ATTRIBUTE_COMPRESSED)
	{
		szRet += CString(L" & ") + szFileCOMPRESSED[g_enumLang];
	}

	while (szRet.GetAt(0) == '&' || szRet.GetAt(0) == ' ')
	{
		szRet.TrimLeft('&');
		szRet.TrimLeft(' ');
	}
		
	return szRet;
}

void CFileDlg::InsertFileItems(CString szPath)
{
	if (szPath.IsEmpty())
	{
		return;
	}

	if ( szPath.GetAt(szPath.GetLength() - 1) != '\\')
	{
		szPath += L"\\";
	}

	if ( !PathFileExists(szPath) )
	{
		return;
	}

	int nImageCount = m_FileImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_FileImageList.Remove(0);   
	}
	
	SHFILEINFO sfiNtdll;
	memset(&sfiNtdll, 0, sizeof(SHFILEINFO));
	WCHAR szWindows[MAX_PATH] = {0};
	GetSystemDirectory(szWindows, MAX_PATH);
	wcsncat_s(szWindows, L"\\ntdll.dll", MAX_PATH - wcslen(szWindows));
	SHGetFileInfo(szWindows, FILE_ATTRIBUTE_NORMAL, &sfiNtdll, sizeof(sfiNtdll), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
	int nDllIco = m_FileImageList.Add ( sfiNtdll.hIcon );
	int nDirectory = m_FileImageList.Add ( m_hDirectoryIcon );
	CString szDllType = sfiNtdll.szTypeName;

	for (vector<FILE_INFO>::iterator ir = m_FileInfoList.begin();
		ir != m_FileInfoList.end();
		ir++)
	{	
		int nItem = m_list.GetItemCount();
		CString szSize;
		int nIcon = -1;
		SHFILEINFO sfi;
		memset(&sfi, 0, sizeof(SHFILEINFO));

		CString szType;
		CString strPath = szPath + ir->FileName;

		if ( !(ir->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			CString szFileNameTemp = ir->FileName;
			szFileNameTemp.MakeUpper();
			if (szFileNameTemp.Find(L".DLL") == -1)
			{
				SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
				nIcon = m_FileImageList.Add ( sfi.hIcon );
				if (sfi.hIcon)
				{
					DestroyIcon(sfi.hIcon);
				}

				szType = sfi.szTypeName;
			}
			else
			{
				nIcon = nDllIco;
				szType = szDllType;
			}

			szSize.Format(L"%d KB", ir->FileSize.QuadPart / 1024);
		}
		else
		{
			nIcon = nDirectory;
			szSize = L"";
			szType = szFileDirectory[g_enumLang];
		}

		m_list.InsertItem(nItem, ir->FileName, nIcon);
		m_list.SetItemText(nItem, 1, szSize);

		CString szCreateTime;
		FILETIME ft;
		ft.dwLowDateTime = ir->CreationTime.LowPart;
		ft.dwHighDateTime = ir->CreationTime.HighPart;
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);
		szCreateTime.Format(L"%04d/%02d/%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
		m_list.SetItemText(nItem, 2, szCreateTime);

		CString szModifyTime;
		ft.dwLowDateTime = ir->ModificationTime.LowPart;
		ft.dwHighDateTime = ir->ModificationTime.HighPart;
		FileTimeToSystemTime(&ft, &st);
		szModifyTime.Format(L"%04d/%02d/%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
		m_list.SetItemText(nItem, 3, szModifyTime);

		m_list.SetItemText(nItem, 4, szType);
		m_list.SetItemText(nItem, 5, GetFileAttributes(ir->FileAttributes));

		m_list.SetItemData(nItem, nItem);
	}
}

void CFileDlg::FixDotPath(CString &szPath)
{
	if (szPath.IsEmpty())
	{
		return;
	}
		
	if (szPath.GetAt(szPath.GetLength() - 1) == '.')
	{
		switch (g_WinVersion)
		{
		case enumWINDOWS_2K:
		case enumWINDOWS_XP:
		case enumWINDOWS_2K3:
		case enumWINDOWS_2K3_SP1_SP2:
			szPath += L".\\";
			break;

		case enumWINDOWS_VISTA:
		case enumWINDOWS_VISTA_SP1_SP2:
		case enumWINDOWS_7:
		case enumWINDOWS_8:
			szPath += L"\\";
			break; 
		}
	}
}

void CFileDlg::EnumFiles(CString szPath)
{
	if (szPath.IsEmpty())
	{
		return;
	}
	
	if ( szPath.GetAt(szPath.GetLength() - 1) != '\\')
	{
		szPath += L"\\";
	}

	if ( !PathFileExists(szPath) )
	{
		return;
	}

	m_list.DeleteAllItems();
	m_FileInfoList.clear();

	GetFiles(szPath, m_FileInfoList);

	if (m_FileInfoList.size())
	{
		SortByFileAttributes();
	}

	::CoInitialize(NULL);
	InsertFileItems(szPath);
	::CoUninitialize();
}

CString CFileDlg::GetPathByTreeItem(HTREEITEM hItem)
{
	CString szKeyPath = L"";

	if (hItem == NULL)
	{
		return szKeyPath;
	}
	
	while (1)
	{
		CString szText;
		if (IsVolumeItem(hItem, szText))
		{
			szKeyPath = szText + szKeyPath;
			break;
		}
		else
		{
			szText = m_tree.GetItemText(hItem);
			FixDotPath(szText);
			if (szText.GetAt(szText.GetLength() - 1) != '\\')
			{
				szText += L"\\";
			}
			szKeyPath = szText + szKeyPath;
		}

		hItem = m_tree.GetParentItem(hItem);
	}

	szKeyPath.TrimRight('\\');
	return szKeyPath;
}

CString CFileDlg::GetComboPathByTreeItem(HTREEITEM hItem)
{
	CString szKeyPath = L"";

	if (hItem == NULL)
	{
		return szKeyPath;
	}

	while (1)
	{
		CString szText;
		if (IsVolumeItem(hItem, szText))
		{
			szKeyPath = szText + szKeyPath;
			break;
		}
		else
		{
			szText = m_tree.GetItemText(hItem);

			if (szText.GetAt(szText.GetLength() - 1) != '\\')
			{
				szText += L"\\";
			}

			szKeyPath = szText + szKeyPath;
		}

		hItem = m_tree.GetParentItem(hItem);
	}

	szKeyPath.TrimRight('\\');
	
// 	if (szKeyPath.GetLength() < wcslen(L"c:\\"))
// 	{
// 		szKeyPath += L"\\";
// 	}

	return szKeyPath;
}

void CFileDlg::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	if (m_bEnumTree && hItem != NULL && hItem != m_tree.GetRootItem())
	{
		CString szKeyPath;

		if (m_hCurrentTreeItem != NULL)
		{
			m_tree.SetItemState(m_hCurrentTreeItem, 0, TVIS_DROPHILITED);
		}

		m_list.DeleteAllItems();
		m_FileInfoList.clear();

		szKeyPath = GetPathByTreeItem(hItem);

		EnumFiles(szKeyPath);

		/*m_szCombo = */m_szCurrentPath = szKeyPath;
		
		m_szCombo = GetComboPathByTreeItem(hItem);

		TrimPath();
//		UpdateData(FALSE);

		m_ComboBox.SetWindowText(m_szCombo);

		m_hCurrentTreeItem = hItem;
	}
	else if (hItem == m_tree.GetRootItem())
	{
		m_list.DeleteAllItems();

		if (m_hCurrentTreeItem != NULL)
		{
			m_tree.SetItemState(m_hCurrentTreeItem, 0, TVIS_DROPHILITED);
		}

		m_hCurrentTreeItem = hItem;

		m_szCombo = szMyComputer[g_enumLang];
		UpdateData(FALSE);
	}

	*pResult = 0;
}

void CFileDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		clrNewTextColor = m_clrNormalFile;//RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );	

		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);

		if ( (info.FileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
			(info.FileAttributes & FILE_ATTRIBUTE_HIDDEN))
		{
			clrNewTextColor = m_clrSystemAndHideFile;// RGB( 255, 0, 0 );
		}
		else if (info.FileAttributes & FILE_ATTRIBUTE_SYSTEM)
		{
			clrNewTextColor = m_clrSystemFile;//RGB( 255, 0, 0 );
		}
		else if (info.FileAttributes & FILE_ATTRIBUTE_HIDDEN)
		{
			clrNewTextColor = m_clrHideFile;//RGB( 127, 127, 127 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CFileDlg::AddDirectoryItemToTree(CString szPath, HTREEITEM hItem)
{
	if (hItem == NULL || szPath.IsEmpty())
	{
		return;
	}
	
	if ( szPath.GetAt(szPath.GetLength() - 1) != '\\')
	{
		szPath += L"\\";
	}

	if ( !PathFileExists(szPath) )
	{
		return;
	}
	
	DebugLog(L"AddDirectoryItemToTree2: %s", szPath);
	vector<FILE_INFO> FileVector;
	GetFiles(szPath, FileVector);

	m_bEnumTree = FALSE;

	for (vector<FILE_INFO>::iterator ir = FileVector.begin();
		ir != FileVector.end();
		ir++)
	{	
		if ( (ir->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && ir->FileName.CompareNoCase(L"..") )
		{
			HTREEITEM hTemp = m_tree.InsertItem(ir->FileName, 4, 5, hItem);
			szPath.TrimRight('\\');
			CString szTemp = szPath + CString(L"\\") + ir->FileName;
			if (IsPathHaveDirectory(szTemp))
			{
				m_tree.InsertItem(L"1", hTemp, TVI_LAST);
			}
		}
	}

	m_bEnumTree = TRUE;
}

void CFileDlg::DeleteSubTree(HTREEITEM TreeItem)
{
	if(TreeItem == NULL)   
	{
		return;   
	}

	m_bEnumTree = FALSE;

	if(m_tree.ItemHasChildren(TreeItem))
	{
		HTREEITEM hNext, hChild = m_tree.GetChildItem(TreeItem);
		while(hChild != NULL)
		{
			hNext = m_tree.GetNextSiblingItem(hChild);
			m_tree.DeleteItem(hChild);
			hChild = hNext; 
		}
	}

	m_bEnumTree = TRUE;
}

void CFileDlg::OnTvnItemexpandedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);	
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	if (hItem != NULL &&
		(TVIS_EXPANDED & m_tree.GetItemState(hItem, TVIS_EXPANDED)) &&
		m_tree.GetRootItem() != hItem)
	{
		DeleteSubTree(hItem);
		CString szKeyPath = GetPathByTreeItem(hItem);
//		FixDotPath(szKeyPath);
		AddDirectoryItemToTree(szKeyPath, hItem);
	}

	*pResult = 0;
}

void CFileDlg::TrimPath()
{
	WCHAR szTemp[] = {'c',':','\\','\0'};
	if (m_szCombo.GetLength() < (int)wcslen(szTemp) && m_szCombo.GetAt(1) == ':')
	{
		m_szCombo += L"\\";
	}

	if (m_szCurrentPath.GetAt(m_szCurrentPath.GetLength() - 1) != '\\')
	{
		m_szCurrentPath += L"\\";
	}

// 	while (1)
// 	{
// 		if (m_szCurrentPath.GetAt(m_szCurrentPath.GetLength() - 1) != '\\')
// 		{
// 			break;
// 		}
// 
// 		m_szCurrentPath.TrimRight('\\');
// 	}
}

void CFileDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMListView = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int nItem = pNMListView->iItem;
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szFileName = info.FileName;

		if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!info.FileName.CompareNoCase(L".."))
			{
				OpenParentDirectory();
			}
			else
			{
				OpenDirectory(szFileName);
			}	
		}
		else
		{
			CString szPath = m_szCurrentPath;
			szPath.TrimRight('\\');
			szPath += L"\\";
			szPath += info.FileName;
			ShellExecuteW(NULL, L"open", szPath, NULL, NULL, SW_SHOW);
		}
	}

	*pResult = 0;
}

void CFileDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu menu;
	menu.CreatePopupMenu();

	if ((!m_list.GetItemCount() || !m_list.GetSelectedCount()) &&
		m_hCurrentTreeItem != NULL && m_hCurrentTreeItem != m_tree.GetRootItem())
	{
		menu.AppendMenu(MF_STRING, ID_FILE_REFRESH, szFileRefresh[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
	}
	else
	{
		if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			int nData = (int)m_list.GetItemData(nItem);
			FILE_INFO info = m_FileInfoList.at(nData);

			if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!info.FileName.CompareNoCase(L".."))
				{
					menu.AppendMenu(MF_STRING, ID_FILE_OPEN, szFileOpen[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_REFRESH, szFileRefresh[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
				}
				else
				{
					menu.AppendMenu(MF_STRING, ID_FILE_OPEN, szFileOpen[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_REFRESH, szFileRefresh[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_LOCK_INFO, szFileLookForLockInfo[g_enumLang]);
					menu.AppendMenu(MF_STRING, ID_FILE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_DELETE, szFileDelete[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_RENAME, szFileRename[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_COPY_TO, szFileCopyTo[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_COPY_FILE_NAME, szFileCopyFileName[g_enumLang]);
					menu.AppendMenu(MF_STRING, ID_FILE_COPY_FILE_PATH, szFileCopyFilePath[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_SHUXING, szFileProperties[g_enumLang]);
					menu.AppendMenu(MF_STRING, ID_FILE_JMP_TO_EXPLORER, szFileFindInExplorer[g_enumLang]);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, ID_FILE_SET_HIDE_SYSTEM_ATTRIBUTE, szFileSetFileAttribute[g_enumLang]);
					menu.AppendMenu(MF_STRING, ID_FILE_DELETE_HIDE_SYSTEM_SHUXING, szFileDeleteHideSystemProperties[g_enumLang]);
				}
			}
			else
			{
				menu.AppendMenu(MF_STRING, ID_FILE_OPEN, szFileOpen[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_REFRESH, szFileRefresh[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_LOCK_INFO, szFileLookForLockInfo[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_DELETE, szFileDelete[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_RENAME, szFileRename[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_COPY_TO, szFileCopyTo[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				//	menu.AppendMenu(MF_STRING, ID_FILE_DELETE_AND_DENY_REBUILD, szFileDeleteAndDenyRebuild[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_ADD_TO_RESTART_DELETE, szFileAddToRestartDelete[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_RESTART_REPLACE, szFileRestartAndReplace[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_COPY_FILE_NAME, szFileCopyFileName[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_COPY_FILE_PATH, szFileCopyFilePath[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_CHECK_ONLINE, szFileSearchOnline[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_CALC_MD5, szFileCalcMd5[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_CHECK_VERIFY, szFileVerifySignature[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_SHUXING, szFileProperties[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_JMP_TO_EXPLORER, szFileFindInExplorer[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_SET_HIDE_SYSTEM_ATTRIBUTE, szFileSetFileAttribute[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_DELETE_HIDE_SYSTEM_SHUXING, szFileDeleteHideSystemProperties[g_enumLang]);
			}

			menu.SetDefaultItem(ID_FILE_OPEN);
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			BOOL bHaveDirectory = FALSE;
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			while (pos)
			{
				int iIndex  = m_list.GetNextSelectedItem(pos);
				int nData = (int)m_list.GetItemData(iIndex);
				FILE_INFO info = m_FileInfoList.at(nData);
				if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					bHaveDirectory = TRUE;
					break;
				}
			}

			if (bHaveDirectory)
			{
				menu.AppendMenu(MF_STRING, ID_FILE_REFRESH, szFileRefresh[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_DELETE, szFileDelete[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_SET_HIDE_SYSTEM_ATTRIBUTE, szFileSetFileAttribute[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_DELETE_HIDE_SYSTEM_SHUXING, szFileDeleteHideSystemProperties[g_enumLang]);
			}
			else
			{
				menu.AppendMenu(MF_STRING, ID_FILE_REFRESH, szFileRefresh[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_DELETE, szFileDelete[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_ADD_TO_RESTART_DELETE, szFileAddToRestartDelete[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, ID_FILE_SET_HIDE_SYSTEM_ATTRIBUTE, szFileSetFileAttribute[g_enumLang]);
				menu.AppendMenu(MF_STRING, ID_FILE_DELETE_HIDE_SYSTEM_SHUXING, szFileDeleteHideSystemProperties[g_enumLang]);
			}
		}
	}

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_FILE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_FILE_COPY_TO, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
		menu.SetMenuItemBitmaps(ID_FILE_JMP_TO_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_FILE_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FILE_COPY_FILE_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_FILE_COPY_FILE_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_FILE_CHECK_VERIFY, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_FILE_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_FILE_CHECK_ONLINE, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	*pResult = 0;
}

void CFileDlg::OnNMRclickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint point;
	GetCursorPos(&point);
	m_tree.ScreenToClient(&point);

	HTREEITEM hItem = m_tree.HitTest(point, NULL);
	if (hItem == NULL)
	{
		return;
	}

	if (m_hCurrentTreeItem)
	{
		m_tree.SetItemState(m_hCurrentTreeItem, 0, TVIS_DROPHILITED);
	}

	m_hCurrentTreeItem = hItem;
	
//	m_bEnumTree = FALSE;
	m_tree.SelectItem(hItem);
//	m_bEnumTree = TRUE;
	m_tree.SetItemState(m_hCurrentTreeItem, TVIS_DROPHILITED, TVIS_DROPHILITED);

	CMenu menu;
	menu.CreatePopupMenu();
	if (m_hCurrentTreeItem != m_tree.GetRootItem())
	{
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_REFRESH, szFileRefresh[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_LOCK_INFO, szFileLookForLockInfo[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_DELETE, szFileDelete[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_RENAME, szFileRename[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_COPY_TO, szFileCopyTo[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_COPY_FILE_NAME, szFileCopyFileName[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_COPY_FILE_PATH, szFileCopyFilePath[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_SHUXING, szFileProperties[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_GOTO_EXPLORER, szFileFindInExplorer[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_SET_HIDE_SYSTEM_ATTRIBUTE, szFileSetFileAttribute[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_DELETE_ALL_SHUXING, szFileDeleteHideSystemProperties[g_enumLang]);

// 		m_szCurrentPath = GetPathByTreeItem(m_hCurrentTreeItem);
// 		TrimPath();
	}
	else
	{
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_REFRESH, szFileRefresh[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_FILE_TREE_VIEW_FILE_LOCKED_INFO, szFileViewRestartDeleteInfo[g_enumLang]);
	}

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_FILE_TREE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_FILE_TREE_COPY_TO, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
		menu.SetMenuItemBitmaps(ID_FILE_TREE_GOTO_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_FILE_TREE_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FILE_TREE_COPY_FILE_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_FILE_TREE_COPY_FILE_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_FILE_TREE_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	*pResult = 0;
}

CString CFileDlg::GetFilePathInListCtrl()
{
	CString szRet;
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		szRet = m_szCurrentPath;
		CString szFile = m_FileInfoList.at(nData).FileName;
		if (m_FileInfoList.at(nData).FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{	
			FixDotPath(szFile);
		}
		szRet += szFile;
	}

	return szRet;
}

void CFileDlg::OpenParentDirectory()
{
	m_szCurrentPath.TrimRight('\\');
	CString szPath = m_szCurrentPath.Left(m_szCurrentPath.ReverseFind('\\'));

	EnumFiles(szPath);

	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		if (m_hCurrentTreeItem != NULL)
		{
			m_tree.SetItemState(m_hCurrentTreeItem, 0, TVIS_DROPHILITED);
		}

		m_hCurrentTreeItem = m_tree.GetParentItem(m_hCurrentTreeItem);

		m_bEnumTree = FALSE;
		m_tree.SelectItem(m_hCurrentTreeItem);
		m_bEnumTree = TRUE;

		m_tree.EnsureVisible(m_hCurrentTreeItem);
		m_tree.SetItemState(m_hCurrentTreeItem, TVIS_DROPHILITED, TVIS_DROPHILITED);

		m_szCurrentPath/* = m_szCombo*/ = szPath;
		m_szCombo = GetComboPathByTreeItem(m_hCurrentTreeItem);

		TrimPath();
	//	UpdateData(FALSE);
		m_ComboBox.SetWindowText(m_szCombo);
	}
}

void CFileDlg::OpenDirectory(CString szFileName)
{
	CString szPath = m_szCurrentPath;
	CString szTemp = szFileName;

	FixDotPath(szTemp);
	szPath += szTemp;

	EnumFiles(szPath);

	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		DeleteSubTree(m_hCurrentTreeItem);

		DebugLog(L"%s", m_szCurrentPath);

		AddDirectoryItemToTree(m_szCurrentPath, m_hCurrentTreeItem);

		m_tree.Expand(m_hCurrentTreeItem, TVE_EXPAND);

		if(m_tree.ItemHasChildren(m_hCurrentTreeItem))
		{
			HTREEITEM hChild = m_tree.GetChildItem(m_hCurrentTreeItem);
			while(hChild != NULL)
			{
				CString szTemp = m_tree.GetItemText(hChild);
				if (!szTemp.CompareNoCase(szFileName))
				{
					if (m_hCurrentTreeItem != NULL)
					{
						m_tree.SetItemState(m_hCurrentTreeItem, 0, TVIS_DROPHILITED);
					}

					m_hCurrentTreeItem = hChild;

					m_bEnumTree = FALSE;
					m_tree.SelectItem(hChild);
					m_bEnumTree = TRUE;

					m_tree.EnsureVisible(hChild);
					m_tree.SetItemState(hChild, TVIS_DROPHILITED, TVIS_DROPHILITED);

					/*m_szCombo =*/ m_szCurrentPath + szFileName;
						
					m_szCurrentPath = szPath;
					
					m_szCombo = GetComboPathByTreeItem(m_hCurrentTreeItem);

					TrimPath();
				//	UpdateData(FALSE);
					
					m_ComboBox.SetWindowText(m_szCombo);

					break;
				}

				hChild = m_tree.GetNextSiblingItem(hChild);
			}
		}
	}
}

void CFileDlg::OnFileOpen()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szFileName = info.FileName;

		if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!szFileName.CompareNoCase(L".."))
			{
				OpenParentDirectory();
			}
			else
			{
				DebugLog(L"OpenDirectory: %s", szFileName);
				OpenDirectory(szFileName);
			}
		}
		else
		{
			CString szPath = m_szCurrentPath;
			szPath += szFileName;
			ShellExecuteW(NULL, L"open", szPath, NULL, NULL, SW_SHOW);
		}
	}
}

void CFileDlg::OnFileRefresh()
{
	if (!m_szCurrentPath.IsEmpty())
	{
		EnumFiles(m_szCurrentPath);
	}
}

void CFileDlg::LookForFileLockInfo(CString szFilePath, BOOL bDirectory)
{
	CString szDosPath;

	if (szFilePath.GetAt(1) == ':' && szFilePath.GetAt(2) == '\\')
	{
		WCHAR *strFile = szFilePath.GetBuffer();

		for (vector<NT_AND_DOS_VOLUME_NAME>::iterator ir = m_nt_and_dos_valume_list.begin();
			ir != m_nt_and_dos_valume_list.end();
			ir++)
		{
			WCHAR *szNtNameTemp = ir->szNtName.GetBuffer();

			if (!_wcsnicmp(szNtNameTemp, strFile, ir->szNtName.GetLength()))
			{
				ir->szNtName.ReleaseBuffer();
				szDosPath = ir->szDosName;
				szDosPath += szFilePath.Right(szFilePath.GetLength() - szFilePath.Find('\\'));
				break;
			}

			ir->szNtName.ReleaseBuffer();
		}

		szFilePath.ReleaseBuffer();
	}

	if (szDosPath.IsEmpty())
	{
		return;
	}

	DWORD dwLen = (szDosPath.GetLength() + 1) * sizeof(WCHAR);
	WCHAR *szPath = (WCHAR*)malloc(dwLen);
	if (!szPath)
	{
		return;
	}

	memset(szPath, 0, dwLen);
	wcsncpy_s(szPath, szDosPath.GetLength() + 1, szDosPath.GetBuffer(), szDosPath.GetLength());
	szDosPath.ReleaseBuffer();

	COMMUNICATE_FILE cf;
	cf.OpType = enumQueryFileLockInfo;
	cf.op.QueryFileLock.szPath = szPath;
	cf.op.QueryFileLock.nPathLen = wcslen(szPath) * sizeof(WCHAR);
	cf.op.QueryFileLock.bDirectory = bDirectory;

	DWORD dwCnt, dwCntTemp;
	PLOCKED_FILE_INFO pInfos = NULL;
	BOOL bRet = FALSE;

	dwCnt = dwCntTemp = 0x1000;

	do 
	{
		DebugLog(L"nRetCount: %d", dwCntTemp);
		dwCnt = dwCntTemp;
		dwLen = sizeof(LOCKED_FILE_INFO) + dwCnt * sizeof(LOCKED_FILES);
		if (pInfos)
		{
			free(pInfos);
			pInfos = NULL;
		}

		pInfos = (PLOCKED_FILE_INFO)malloc(dwLen);
		if (!pInfos)
		{
			free(szPath);
			return;
		}

		memset(pInfos, 0, dwLen);

		bRet = m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FILE), pInfos, dwLen, NULL);

	} while ( !bRet && (dwCntTemp = pInfos->nRetCount) > dwCnt );

	if (bRet && pInfos->nRetCount > 0)
	{
		DebugLog(L"nRetCount: %d", pInfos->nRetCount);

		CFileUnlockDlg dlg;
		dlg.m_szNtFilePath = szPath;
		dlg.m_bDirectory = bDirectory;
		dlg.m_pLockFileInfos = pInfos;
		dlg.DoModal();
	}
	else
	{
		MessageBox(szFileUnlocked[g_enumLang], L"AntiSpy", MB_OK | MB_ICONINFORMATION);
	}

	free(szPath);
	free(pInfos);
}

void CFileDlg::OnFileLockInfo()
{
	CString szFilePath;
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	int nData = (int)m_list.GetItemData(nItem);
	szFilePath = m_szCurrentPath;
	szFilePath += m_FileInfoList.at(nData).FileName;

	if (szFilePath.IsEmpty())
	{
		return;
	}
	
	BOOL bDirectory = m_FileInfoList.at(nData).FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	LookForFileLockInfo(szFilePath, bDirectory);
}

BOOL CFileDlg::DeleteDirectory(CString szDir)
{
 	BOOL bRet = FALSE;

	if (szDir.IsEmpty())
	{
		return bRet;
	}

	if (szDir.GetAt(szDir.GetLength() - 1) != '\\')
	{
		szDir += L"\\";
	}

	DebugLog(L"szDir1: %s", szDir);

// 	DWORD dwLen = (szDir.GetLength() + 2) * sizeof(WCHAR);
// 	WCHAR *szPath = (WCHAR*)malloc(dwLen);
// 	if (!szPath)
// 	{
// 		return bRet;
// 	}
// 
// 	memset(szPath, 0, dwLen);
// 	wcsncpy_s(szPath, szDir.GetLength() + 1, szDir.GetBuffer(), szDir.GetLength());
// 	szPath[szDir.GetLength()] = '\0';
// 	szPath[szDir.GetLength()+1] = '\0';
// 	szDir.ReleaseBuffer();
// 
// 	SHFILEOPSTRUCT FileOp;
// 	ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));
// 	FileOp.fFlags = FOF_NOCONFIRMATION;
// 	FileOp.hNameMappings = NULL; 
// 	FileOp.hwnd = NULL; 
// 	FileOp.lpszProgressTitle = NULL; 
// 	FileOp.pFrom = szPath; 
// 	FileOp.pTo = NULL; 
// 	FileOp.wFunc = FO_DELETE; 
// 	SHFileOperation(&FileOp) == 0 ? bRet = TRUE : bRet = FALSE;
// 
// 	free(szPath);
// 	szPath = NULL;

	if (!bRet)
	{
		CFileFind finder;
		CString path;
		path.Format(L"%s/*.*", szDir);
		BOOL bWorking = finder.FindFile(path);
		while(bWorking)
		{
			bWorking = finder.FindNextFile();
			CString szFile = finder.GetFileName();
			CString szFilePath = szDir + szFile;
			
			if(finder.IsDirectory() && !finder.IsDots())
			{
				DebugLog(L"delete: %s", szFilePath);
				FixDotPath(szFilePath);

				DeleteDirectory(szFilePath); 

				if (szFile.Find('.') != -1 && g_WinVersion == enumWINDOWS_XP)
				{
					RemoveDirectory(szFilePath);
				}
				else
				{
					m_Functions.KernelDeleteFile(szFilePath);
				}
			}
			else if(!finder.IsDirectory())
			{
				DebugLog(L"delete: %s", szFilePath);

				if (szFile.Find('.') != -1 && g_WinVersion == enumWINDOWS_XP)
				{
					DeleteFile(szFilePath);
				}
				else
				{
					m_Functions.KernelDeleteFile(szFilePath);
				}
			}
		}

		finder.Close();

		if (szDir.Find('.') != -1 && g_WinVersion == enumWINDOWS_XP)
		{
			RemoveDirectory(szDir);
		}
		else
		{
			m_Functions.KernelDeleteFile(szDir);
		}
	}

	return bRet;
}

void CFileDlg::OnFileDelete()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], L"AntiSpy", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szFilePath;
		szFilePath = m_szCurrentPath;
		szFilePath += m_FileInfoList.at(nData).FileName;

		FixDotPath(szFilePath);

		if (PathFileExists(szFilePath))
		{
			BOOL bDirectory = m_FileInfoList.at(nData).FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
			if (!bDirectory)
			{
				m_Functions.KernelDeleteFile(szFilePath);
			}
			else
			{
				DeleteDirectory(szFilePath);

				if (TVIS_EXPANDED & m_tree.GetItemState(m_hCurrentTreeItem, TVIS_EXPANDED))
				{
					if(m_tree.ItemHasChildren(m_hCurrentTreeItem))
					{
						CString szFile = m_FileInfoList.at(nData).FileName;
						HTREEITEM hChild = m_tree.GetChildItem(m_hCurrentTreeItem);
						while(hChild != NULL)
						{
							hChild = m_tree.GetNextSiblingItem(hChild);
							if (!szFile.CompareNoCase(m_tree.GetItemText(hChild)))
							{
								m_tree.DeleteItem(hChild);
								break;
							}
						}
					}
				}
			}
		}

		m_list.DeleteItem(nItem);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

void CFileDlg::OnFileDeleteAndDenyRebuild()
{
// 	for ()
// 	{
// 	}
}

void CFileDlg::OnFileAddToRestartDelete()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(iIndex);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szPath = m_szCurrentPath + info.FileName;
		if (!szPath.IsEmpty())
		{
			MoveFileEx(szPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
	}
}

void CFileDlg::OnFileRestartReplace()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szPath = m_szCurrentPath + info.FileName;
		if (!szPath.IsEmpty())
		{
			CGetFilePathDlg dlg;
			if (dlg.DoModal() == IDOK && !dlg.m_szPath.IsEmpty())
			{
				MoveFileEx(szPath, dlg.m_szPath, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
		}
	}
}

BOOL CFileDlg::FileCopy(CString szSrc, CString szDes)
{
	BOOL bRet = FALSE;

	if (szSrc.IsEmpty() || szDes.IsEmpty())
	{
		return FALSE;
	}

	szSrc.TrimRight('\\');
	szDes.TrimRight('\\');

	DWORD dwLen = (szSrc.GetLength() + 2) * sizeof(WCHAR);
	WCHAR *szPath = (WCHAR*)malloc(dwLen);
	if (!szPath)
	{
		return bRet;
	}

	memset(szPath, 0, dwLen);
	wcsncpy_s(szPath, szSrc.GetLength() + 1, szSrc.GetBuffer(), szSrc.GetLength());
	szPath[szSrc.GetLength()] = '\0';
	szPath[szSrc.GetLength()+1] = '\0';
	szSrc.ReleaseBuffer();

	SHFILEOPSTRUCT FileOp; 
	ZeroMemory((void*)&FileOp,sizeof(SHFILEOPSTRUCT));

	FileOp.fFlags = FOF_NOCONFIRMATION ; 
	FileOp.hNameMappings = NULL; 
	FileOp.hwnd = NULL; 
	FileOp.lpszProgressTitle = NULL; 
	FileOp.pFrom = szPath;
	FileOp.pTo = (LPWSTR)(LPCTSTR)szDes; 
	FileOp.wFunc = FO_COPY; 
	SHFileOperation(&FileOp) == 0 ? bRet = TRUE : bRet = FALSE;

	free(szPath);
	szPath = NULL;

	return bRet;
}

void CFileDlg::OnFileCopyTo()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szPath = m_szCurrentPath + info.FileName;
		if (!szPath.IsEmpty())
		{
			if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FixDotPath(szPath);
			}

			CGetFilePathDlg dlg;
			dlg.m_bDirectoty = TRUE;
			dlg.szFileName = info.FileName;
			if (dlg.DoModal() == IDOK && 
				!dlg.m_szPath.IsEmpty() &&
				szPath.CompareNoCase(dlg.m_szPath))
			{
				if (PathFileExists(dlg.m_szPath))
				{
					if (MessageBox(szFileExist[g_enumLang], szFileCopy[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
					{
						return;
					}
				}

				FileCopy(szPath, dlg.m_szPath);
			}
		//	CopyFile( szPath, dlg.m_szPath, TRUE );
		}
	}
}

void CFileDlg::OnFileRename()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szPath = m_szCurrentPath + info.FileName;
		if (!szPath.IsEmpty())
		{
			if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FixDotPath(szPath);
			}

			CFileRenameDlg dlg;
			dlg.m_szFileName = info.FileName;
			if (dlg.DoModal() == IDOK && !dlg.m_szFileName.IsEmpty())
			{
				if (dlg.m_szFileName.CompareNoCase(info.FileName))
				{
					CString szNew = m_szCurrentPath + dlg.m_szFileName;

					if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						FixDotPath(szNew);
					}
					
					if (PathFileExists(szNew))
					{
						MessageBox(szFileHasExist[g_enumLang], szFileRename[g_enumLang], MB_OK | MB_ICONWARNING);
					}
					else
					{
						if (MoveFile(szPath, szNew))
						{
							m_list.SetItemText(nItem, 0, dlg.m_szFileName);
							m_FileInfoList.at(nData).FileName = dlg.m_szFileName;
						}
					}

// 					CFile::Rename(szPath, szNew);
// 					m_list.SetItemText(nItem, 0, dlg.m_szFileName);
// 					m_FileInfoList.at(nData).FileName = dlg.m_szFileName;
				}
			}
		}
	}
}

void CFileDlg::OnFileCopyFileName()
{
	CString szRet;
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		szRet = m_FileInfoList.at(nData).FileName;
		m_Functions.SetStringToClipboard(szRet);
	}
}

void CFileDlg::OnFileCopyFilePath()
{
	CString szPath = GetFilePathInListCtrl();
	m_Functions.SetStringToClipboard(szPath);
}

void CFileDlg::OnFileShuxing()
{
	CString szPath = GetFilePathInListCtrl();
	m_Functions.OnCheckAttribute(szPath);
}

void CFileDlg::OnFileJmpToExplorer()
{
	CString szPath = GetFilePathInListCtrl();
	m_Functions.LocationExplorer(szPath);
}

void CFileDlg::OnFileDeleteHideSystemShuxing()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szFilePath, szFile;
		szFilePath = m_szCurrentPath;
		szFile = m_FileInfoList.at(nData).FileName;
		
		BOOL bDir = m_FileInfoList.at(nData).FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		if (bDir)
		{	
			FixDotPath(szFile);
		}
		
		szFilePath += szFile;

		if (SetFileNormalAttribute(szFilePath, FILE_ATTRIBUTE_NORMAL))
		{
			if (bDir)
			{
				m_FileInfoList.at(nData).FileAttributes = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY;
			}
			else
			{
				m_FileInfoList.at(nData).FileAttributes = FILE_ATTRIBUTE_NORMAL;
			}

			m_list.SetItemText(nItem, 5, L"");
		}
	}
}

void CFileDlg::OnFileCheckVerify()
{
	CString szPath = GetFilePathInListCtrl();
	m_Functions.SignVerify(szPath);
}

void CFileDlg::OnFileCheckAllVerify()
{
// 	m_SignVerifyDlg.m_NotSignDataList.clear();
// 	m_SignVerifyDlg.m_NotSignItemList.clear();
// 	m_SignVerifyDlg.m_bSingle = FALSE;
// 	m_SignVerifyDlg.m_pList = &m_list;
// 	m_SignVerifyDlg.m_nPathSubItem = 2;
// 	m_SignVerifyDlg.m_hWinWnd = this->m_hWnd;
// 	m_SignVerifyDlg.DoModal();
}

// eg. "\Device\HarddiskVolume1"  -  "C:\"
BOOL CFileDlg::InitDevice2Path()
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

void CFileDlg::ConvertVolumePaths(
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

void CFileDlg::InitTree()
{
	m_VolumeList.clear();

	HTREEITEM hRoot = m_tree.GetRootItem();
	DWORD  dwInfo = GetLogicalDrives();  
	int m = 0;  
	while(dwInfo) 
	{ 
		if (dwInfo & 1) 
		{  
			WCHAR ch = 'A' + m; 
			CString chs;
			chs.Format(L"%c:\\", ch);

			int dt = GetDriveType(chs); 
			CString szDisk;

			HTREEITEM hTtem = NULL;
			switch (dt)
			{
			case DRIVE_UNKNOWN:
				szDisk.Format(L"%s (%c:)", szDiskUnknow[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, hRoot, TVI_LAST);
				break;

			case DRIVE_NO_ROOT_DIR:
				break;

			case DRIVE_REMOVABLE:
				szDisk.Format(L"%s (%c:)", szRemovableDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 1, 1, hRoot, TVI_LAST);
				break;

			case DRIVE_FIXED:
				szDisk.Format(L"%s (%c:)", szLocalDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, hRoot, TVI_LAST);
				break;

			case DRIVE_REMOTE:
				szDisk.Format(L"%s (%c:)", szRemoteDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, hRoot, TVI_LAST);
				break;

			case DRIVE_CDROM:
				szDisk.Format(L"%s (%c:)", szCDDrive[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 3, 3, hRoot, TVI_LAST);
				break;

			case DRIVE_RAMDISK:
				szDisk.Format(L"%s (%c:)", szRamDisk[g_enumLang], ch);
				hTtem = m_tree.InsertItem(szDisk, 2, 2, hRoot, TVI_LAST);
				break;
			}

			if (hTtem)
			{
				FILE_VOLUME_INFO info;
				info.hItem = hTtem;
				info.szVolume = chs;
				m_VolumeList.push_back(info);

				if (IsPathHaveDirectory(chs))
				{
					m_tree.InsertItem(L"1", hTtem, TVI_LAST);
				}
			}
		}

		dwInfo = dwInfo>>1; 
		m += 1; 
	}  

	m_tree.Expand(hRoot, TVE_EXPAND);
}

void CFileDlg::OnFileTreeRefresh()
{
	if (m_hCurrentTreeItem == NULL)
	{
		return;
	}

	if (m_hCurrentTreeItem == m_tree.GetRootItem())
	{
		DeleteSubTree(m_hCurrentTreeItem);
		InitTree();
	}
	else
	{
		if (TVIS_EXPANDED & m_tree.GetItemState(m_hCurrentTreeItem, TVIS_EXPANDED))
		{
			DeleteSubTree(m_hCurrentTreeItem);
			AddDirectoryItemToTree(m_szCurrentPath, m_hCurrentTreeItem);
			m_tree.Expand(m_hCurrentTreeItem, TVE_EXPAND);
		}
	}
}

void CFileDlg::OnFileTreeLockInfo()
{
	if (m_hCurrentTreeItem == NULL)
	{
		return;
	}

	if (m_hCurrentTreeItem != m_tree.GetRootItem())
	{
		CString szTemp = m_szCurrentPath;
		szTemp.TrimRight('\\');
		LookForFileLockInfo(m_szCurrentPath, TRUE);
	}
}

void CFileDlg::OnFileTreeDelete()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], L"AntiSpy", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	DeleteDirectory(m_szCurrentPath);
	m_tree.DeleteItem(m_hCurrentTreeItem);
}

void CFileDlg::OnFileTreeCopyFileName()
{
	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		CString szDir = m_tree.GetItemText(m_hCurrentTreeItem);
		m_Functions.SetStringToClipboard(szDir);
	}
}

void CFileDlg::OnFileTreeCopyFilePath()
{
	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		CString szPath = m_szCurrentPath;
		if (szPath.GetLength() > (int)wcslen(L"c:\\"))
		{
			szPath.TrimRight('\\');
		}
		m_Functions.SetStringToClipboard(szPath);
	}
}

void CFileDlg::OnFileTreeShuxing()
{
	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		m_Functions.OnCheckAttribute(m_szCurrentPath);
	}
}

void CFileDlg::OnFileTreeGotoExplorer()
{
	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		m_Functions.LocationExplorer(m_szCurrentPath);
	}
}

BOOL CFileDlg::SetFileNormalAttribute(CString szPath, ULONG Attribute)
{
	BOOL bRet = FALSE;
	DWORD dwLen = (szPath.GetLength() + 1) * sizeof(WCHAR);
	WCHAR *strPath = (WCHAR*)malloc(dwLen);
	if (!strPath)
	{
		return bRet;
	}

	memset(strPath, 0, dwLen);
	wcsncpy_s(strPath, szPath.GetLength() + 1, szPath.GetBuffer(), szPath.GetLength());
	szPath.ReleaseBuffer();

	HANDLE hFile = m_FileFunc.Create(
		strPath, 
		dwLen, 
		SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES, 
		FILE_ATTRIBUTE_NORMAL, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (hFile)
	{
		COMMUNICATE_FILE file;
		file.OpType = enumSetFileNormalAttribute;
		file.op.SetFileAttribute.hFile = hFile;
		file.op.SetFileAttribute.Attribute = Attribute;
		bRet = m_Driver.CommunicateDriver(&file, sizeof(COMMUNICATE_FILE), NULL, 0, NULL);
		CloseHandle(hFile);
	}

	if (!bRet)
	{
		bRet = SetFileAttributes(szPath, Attribute);
	}

	return bRet;
}

void CFileDlg::OnFileTreeDeleteAllShuxing()
{
	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		SetFileNormalAttribute(m_szCurrentPath, FILE_ATTRIBUTE_NORMAL);
	}
}

HTREEITEM CFileDlg::GetVolumeItem(CString szVolume)
{
	HTREEITEM hItem = NULL;

	if (szVolume.IsEmpty())
	{
		return hItem;
	}

	szVolume.TrimRight('\\');

	for (list<FILE_VOLUME_INFO>::iterator ir = m_VolumeList.begin();
		ir != m_VolumeList.end();
		ir++)
	{
		CString szTemp = ir->szVolume;
		szTemp.TrimRight('\\');

		if (!szTemp.CompareNoCase(szVolume))
		{
			hItem = ir->hItem;
			break;
		}
	}

	return hItem;
}

HTREEITEM CFileDlg::GetTreeSubItemByName(HTREEITEM PatentItem, CString szName)
{
	HTREEITEM hChild = m_tree.GetChildItem(PatentItem);
	HTREEITEM RetItem = NULL;

	while(hChild != NULL)
	{
		CString szKey = m_tree.GetItemText(hChild);

		if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}

		hChild = m_tree.GetNextSiblingItem(hChild);
	}

	return RetItem;
}

void CFileDlg::OnBnClickedGoto()
{
	UpdateData(TRUE);

	if (m_szCombo.IsEmpty())
	{
		return;
	}

	WCHAR ch = m_szCombo.GetAt(m_szCombo.GetLength() - 1);
	while (ch == '\\' || ch == ' ')
	{
		m_szCombo.TrimRight(ch);
		ch = m_szCombo.GetAt(m_szCombo.GetLength() - 1);
	}

	ch = m_szCombo.GetAt(0);
	while (ch == '\\' || ch == ' ')
	{
		m_szCombo.TrimLeft(ch);
		ch = m_szCombo.GetAt(0);
	}

	if (!PathFileExists(m_szCombo))
	{
		CString szMsgBox; 
		szMsgBox = szRegistryIsNoExsit[g_enumLang];
		szMsgBox += L" \'" + m_szCombo + L"\'"; 
		MessageBox(szMsgBox, L"AntiSpy", MB_OK | MB_ICONINFORMATION);
		return;
	}

	int nTemp = m_szCombo.Find(L"\\");
	CString szVolume;
	if (nTemp != -1)
	{
		szVolume = m_szCombo.Left(m_szCombo.Find(L"\\"));
	}
	else
	{
		szVolume = m_szCombo;
	}

	HTREEITEM RootItem = m_tree.GetRootItem();
	if (RootItem == NULL)
	{
		return;
	}

	m_tree.Expand(RootItem, TVE_EXPAND);

	HTREEITEM hItem = GetVolumeItem(szVolume);
	if (!hItem)
	{
		return;
	}

	m_tree.Expand(hItem, TVE_COLLAPSE);
	DeleteSubTree(hItem);
	AddDirectoryItemToTree(szVolume, hItem);
	m_tree.Expand(hItem, TVE_EXPAND);

	CString szKeyEnd = m_szCombo.Right(m_szCombo.GetLength() - m_szCombo.Find('\\') - 1);
	BOOL bQuiet = FALSE;

	do 
	{
		CString szTemp = szKeyEnd.Left(szKeyEnd.Find('\\'));

		if (szTemp.IsEmpty())
		{
			szTemp = szKeyEnd;
			bQuiet = TRUE;
		}

		hItem = GetTreeSubItemByName(hItem, szTemp);
		if (hItem == NULL)
		{
			CString szMsgBox; 
			szMsgBox = szRegistryIsNoExsit[g_enumLang];
			szMsgBox += L" \'" + m_szCombo + L"\'"; 
			MessageBox(szMsgBox, L"AntiSpy", MB_OK | MB_ICONINFORMATION);
			break;
		}

		szVolume += L"\\" + szTemp;

		if (bQuiet)
		{
			m_tree.Expand(hItem, TVE_EXPAND);
			m_tree.Select(hItem, TVGN_FIRSTVISIBLE);
			m_tree.SelectItem(hItem);
			m_tree.SetItemState(hItem, TVIS_DROPHILITED, TVIS_DROPHILITED);
		}
		else
		{
			AddDirectoryItemToTree(szVolume, hItem);
			m_tree.Expand(hItem, TVE_EXPAND);
		}

		szKeyEnd = szKeyEnd.Right(szKeyEnd.GetLength() - szKeyEnd.Find('\\') - 1);

	} while (!bQuiet);
}

void CFileDlg::OnFileViewFileLockedInfo()
{
	CFileRestartDeleteDlg dlg;
	dlg.DoModal();
}

void CFileDlg::OnFileTreeViewFileLockedInfo()
{
	CFileRestartDeleteDlg dlg;
	dlg.DoModal();
}

void CFileDlg::OnFileTreeSetHideSystemAttribute()
{
	if (m_hCurrentTreeItem && m_tree.GetRootItem() != m_hCurrentTreeItem)
	{
		SetFileNormalAttribute(m_szCurrentPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
	}
}

void CFileDlg::OnFileSetHideSystemAttribute()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem  = m_list.GetNextSelectedItem(pos);
		int nData = (int)m_list.GetItemData(nItem);
		FILE_INFO info = m_FileInfoList.at(nData);
		CString szFilePath, szFile;
		szFilePath = m_szCurrentPath;
		szFile = m_FileInfoList.at(nData).FileName;

		BOOL bDir = m_FileInfoList.at(nData).FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		if (bDir)
		{	
			FixDotPath(szFile);
		}

		szFilePath += szFile;

		if (SetFileNormalAttribute(szFilePath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
		{
			if (bDir)
			{
				m_FileInfoList.at(nData).FileAttributes = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
			}
			else
			{
				m_FileInfoList.at(nData).FileAttributes = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
			}

			m_list.SetItemText(nItem, 5, GetFileAttributes(m_FileInfoList.at(nData).FileAttributes));
		}
	}
}

void CFileDlg::OnFileTreeCopyTo()
{
	CString szFileName = m_szCurrentPath;
	szFileName.TrimRight('\\');
	CString szPath = szFileName;

	szFileName = szFileName.Right(szFileName.GetLength() - szFileName.ReverseFind('\\') - 1);

	CGetFilePathDlg dlg;
	dlg.m_bDirectoty = TRUE;
	dlg.szFileName = szFileName;

	if (dlg.DoModal() == IDOK && 
		!dlg.m_szPath.IsEmpty() &&
		szPath.CompareNoCase(dlg.m_szPath))
	{
		if (PathFileExists(dlg.m_szPath))
		{
			if (MessageBox(szFileExist[g_enumLang], szFileCopy[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				return;
			}
		}

		FileCopy(szPath, dlg.m_szPath);
	}
}

void CFileDlg::OnFileTreeRename()
{
	CString szFileName = m_tree.GetItemText(m_hCurrentTreeItem);

	CFileRenameDlg dlg;
	dlg.m_szFileName = szFileName;

	if (dlg.DoModal() == IDOK && !dlg.m_szFileName.IsEmpty())
	{
		if (dlg.m_szFileName.CompareNoCase(szFileName))
		{
			HTREEITEM hParent = m_tree.GetParentItem(m_hCurrentTreeItem);
			if (hParent)
			{
				CString szParent = GetPathByTreeItem(hParent);
				if (!szParent.IsEmpty())
				{
					CString szNew = szParent + CString(L"\\") + dlg.m_szFileName;
					CString szOld = szParent + CString(L"\\") + szFileName;

					FixDotPath(szNew);
					FixDotPath(szOld);

					if (PathFileExists(szNew))
					{
						MessageBox(szFileHasExist[g_enumLang], szFileRename[g_enumLang], MB_OK | MB_ICONWARNING);
					}
					else
					{
						if (MoveFile(szOld, szNew))
						{
							m_tree.SetItemText(m_hCurrentTreeItem, dlg.m_szFileName);
						}
					}
				}
			}
		}
	}
}

void CFileDlg::OnCbnDropdownCombo()
{
	m_ComboBox.ResetContent();
	InitComboBox();
}

void CFileDlg::GotoPath(CString szPath, CString szFile)
{
	if (szPath.IsEmpty())
	{
		return;
	}

	WCHAR ch = szPath.GetAt(szPath.GetLength() - 1);
	while (ch == '\\' || ch == ' ')
	{
		szPath.TrimRight(ch);
		ch = szPath.GetAt(szPath.GetLength() - 1);
	}

	// qud
	ch = szPath.GetAt(0);
	while (ch == '\\' || ch == ' ')
	{
		szPath.TrimLeft(ch);
		ch = szPath.GetAt(0);
	}

	if (!PathFileExists(szPath))
	{
		CString szMsgBox; 
		szMsgBox = szRegistryIsNoExsit[g_enumLang];
		szMsgBox += L" \'" + szPath + L"\'"; 
		MessageBox(szMsgBox, L"AntiSpy", MB_OK | MB_ICONINFORMATION);
		return;
	}

	int nTemp = szPath.Find(L"\\");
	CString szVolume;
	if (nTemp != -1)
	{
		szVolume = szPath.Left(szPath.Find(L"\\"));
	}
	else
	{
		szVolume = szPath;
	}

	HTREEITEM RootItem = m_tree.GetRootItem();
	if (RootItem == NULL)
	{
		return;
	}

	m_tree.Expand(RootItem, TVE_EXPAND);

	HTREEITEM hItem = GetVolumeItem(szVolume);
	if (!hItem)
	{
		return;
	}

	m_tree.Expand(hItem, TVE_COLLAPSE);
	DeleteSubTree(hItem);
	AddDirectoryItemToTree(szVolume, hItem);
	m_tree.Expand(hItem, TVE_EXPAND);

	CString szKeyEnd = szPath.Right(szPath.GetLength() - szPath.Find('\\') - 1);
	BOOL bQuiet = FALSE;

	do 
	{
		CString szTemp = szKeyEnd.Left(szKeyEnd.Find('\\'));

		if (szTemp.IsEmpty())
		{
			szTemp = szKeyEnd;
			bQuiet = TRUE;
		}

		hItem = GetTreeSubItemByName(hItem, szTemp);
		if (hItem == NULL)
		{
			CString szMsgBox; 
			szMsgBox = szRegistryIsNoExsit[g_enumLang];
			szMsgBox += L" \'" + szPath + L"\'"; 
			MessageBox(szMsgBox, L"AntiSpy", MB_OK | MB_ICONINFORMATION);
			break;
		}

		szVolume += L"\\" + szTemp;

		if (bQuiet)
		{
			m_tree.Expand(hItem, TVE_EXPAND);
			m_tree.Select(hItem, TVGN_FIRSTVISIBLE);
			m_tree.SelectItem(hItem);
			m_tree.SetItemState(hItem, TVIS_DROPHILITED, TVIS_DROPHILITED);

			if (!szFile.IsEmpty())
			{
				DWORD dwCnt = m_list.GetItemCount();
				for (DWORD i = 0; i < dwCnt; i++)
				{
					if (!(m_list.GetItemText(i, 0)).CompareNoCase(szFile))
					{
						m_list.EnsureVisible(i, false);
						m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);
						m_list.SetFocus();
						break;
					}
				}
			}	

			break;
		}
		else
		{
			AddDirectoryItemToTree(szVolume, hItem);
			m_tree.Expand(hItem, TVE_EXPAND);
		}

		szKeyEnd = szKeyEnd.Right(szKeyEnd.GetLength() - szKeyEnd.Find('\\') - 1);

	} while (!bQuiet);
}

void CFileDlg::OnFileCheckOnline()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		CString szFile = m_FileInfoList.at(nData).FileName;
		m_Functions.SearchOnline(szFile);
	}
}

void CFileDlg::OnFileCalcMd5()
{
	CString szPath = GetFilePathInListCtrl();
	CHashCalcDlg dlg;
	dlg.m_szPath = szPath;
	dlg.DoModal();
}



BOOL CFileDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 

	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) 
	{
		if (m_bInComboBox)
		{
			return CDialog::PreTranslateMessage(pMsg);
		}
		else if (m_bInListCtrl)
		{
			OnFileOpen();
		}

		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CFileDlg::OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_bInListCtrl = TRUE;
	*pResult = 0;
}

void CFileDlg::OnNMKillfocusList(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_bInListCtrl = FALSE;
	*pResult = 0;
}

void CFileDlg::OnCbnSetfocusCombo()
{
	m_bInComboBox = TRUE;
}

void CFileDlg::OnCbnKillfocusCombo()
{
	m_bInComboBox = FALSE;
}

void CFileDlg::EnumFilesByPareseDisk(CString szPath)
{
	if (szPath.IsEmpty())
	{
		return;
	}

	if ( szPath.GetAt(szPath.GetLength() - 1) != '\\')
	{
		szPath += L"\\";
	}

	if ( !PathFileExists(szPath) )
	{
		return;
	}

	m_list.DeleteAllItems();
	m_vectorFileParaseDisk.clear();
	
// 	if (m_vectorFileParaseDisk.size())
// 	{
// 		SortByFileAttributes();
// 	}

	::CoInitialize(NULL);

//	InsertFileItems(szPath);
	::CoUninitialize();
}
