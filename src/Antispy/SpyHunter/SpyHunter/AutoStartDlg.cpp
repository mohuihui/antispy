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
// AutoStartDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "AutoStartDlg.h"
#include <Sddl.h>
#include <shlwapi.h>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include "RegistryDlg.h"
#include <algorithm>
#include "RegStringTool.h"


// CAutoStartDlg 对话框

IMPLEMENT_DYNAMIC(CAutoStartDlg, CDialog)

CAutoStartDlg::CAutoStartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoStartDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_AutostartList.clear();
	m_szHKeyCurrentUser = L"";
	m_nStartup = 0;
	m_nExplorer = 0;
	m_nWinlogon = 0;
	m_nInternetExplorer = 0;
	m_nKnowDlls = 0;
	m_AutoStartType = eNone;
	m_nPrintMonitors = 0;
	m_nNetworkProvider = 0;
	m_nWinsockProvider = 0;
	m_nSecurityProvider = 0;
	m_nTask = 0;
	m_szStartupPath = L"";
	m_bHideMicrosoft = FALSE;
	m_szCommonStartupPath = L"";
	m_nService = 0;
	m_nDriver = 0;
	m_bFirst = TRUE;
	m_bStartCheck = FALSE;
}

CAutoStartDlg::~CAutoStartDlg()
{
	m_AutostartList.clear();
}

void CAutoStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_AUTO_START_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CAutoStartDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAutoStartDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_MESSAGE(WM_VERIFY_SIGN_OVER, OnCloseWindow)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, &CAutoStartDlg::OnTvnSelchangedTree)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, &CAutoStartDlg::OnNMRclickTree)
	ON_NOTIFY(NM_CLICK, IDC_TREE, &CAutoStartDlg::OnNMClickTree)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CAutoStartDlg::OnLvnItemchangedList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CAutoStartDlg::OnNMRclickList)
	ON_COMMAND(ID_AUTORUN_JMP_TO_REG, &CAutoStartDlg::OnAutorunJmpToReg)
	ON_COMMAND(ID_AUTORUN_LOCATION_EXPLORER, &CAutoStartDlg::OnAutorunLocationExplorer)
	ON_COMMAND(ID_AUTORUN_DEL_REG, &CAutoStartDlg::OnAutorunDelReg)
	ON_COMMAND(ID_AUTORUN_DEL_REG_AND_FILE, &CAutoStartDlg::OnAutorunDelRegAndFile)
	ON_COMMAND(ID_AUTORUN_COPY_NAME, &CAutoStartDlg::OnAutorunCopyName)
	ON_COMMAND(ID_AUTORUN_COPY_PATH, &CAutoStartDlg::OnAutorunCopyPath)
	ON_COMMAND(ID_AUTORUN_SEARCH_ONLINE, &CAutoStartDlg::OnAutorunSearchOnline)
	ON_COMMAND(ID_AUTORUN_CHECK_ATTRIBUTE, &CAutoStartDlg::OnAutorunCheckAttribute)
	ON_COMMAND(ID_AUTORUN_CHECK_SIGN, &CAutoStartDlg::OnAutorunCheckSign)
	ON_COMMAND(ID_AUTORUN_CHECK_ALL_SIGN, &CAutoStartDlg::OnAutorunCheckAllSign)
	ON_COMMAND(ID_AUTORUN_TEXT, &CAutoStartDlg::OnAutorunText)
	ON_COMMAND(ID_AUTORUN_EXCEL, &CAutoStartDlg::OnAutorunExcel)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CAutoStartDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_AUTORUN_HIDE_MICROSOFT_MODULE, &CAutoStartDlg::OnAutorunHideMicrosoftModule)
	ON_UPDATE_COMMAND_UI(ID_AUTORUN_HIDE_MICROSOFT_MODULE, &CAutoStartDlg::OnUpdateAutorunHideMicrosoftModule)
	ON_WM_INITMENUPOPUP()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CAutoStartDlg::OnNMDblclkList)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CAutoStartDlg::OnProcessLocationAtFileManager)
	ON_COMMAND(ID_AUTORUN_REFRESH, &CAutoStartDlg::OnAutorunRefresh)
	ON_COMMAND(ID_AUTORUN_DISABLE, &CAutoStartDlg::OnAutorunDisable)
	ON_COMMAND(ID_AUTORUN_ENABLE, &CAutoStartDlg::OnAutorunEnable)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CAutoStartDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_TREE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
	EASYSIZE(IDC_AUTO_START_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CAutoStartDlg 消息处理程序

void CAutoStartDlg::OnBnClickedOk()
{
}

void CAutoStartDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

#define MAX_ICON 13
BOOL CAutoStartDlg::InitTree()
{
	HICON icon[MAX_ICON];
	icon[0] = AfxGetApp()->LoadIcon (IDI_AUTORUN_ROOT);
 	icon[1] = AfxGetApp()->LoadIcon (IDI_STARTUP);
	icon[2] = AfxGetApp()->LoadIcon (IDI_WIN_LOGON);
 	icon[3] = AfxGetApp()->LoadIcon (IDI_EXPLORER);
	icon[4] = AfxGetApp()->LoadIcon (IDI_IE);
	icon[5] = AfxGetApp()->LoadIcon (IDI_SERVICE);
	icon[6] = AfxGetApp()->LoadIcon (IDI_DRIVER);
	icon[7] = AfxGetApp()->LoadIcon (IDI_KNOWDLLS);
	icon[8] = AfxGetApp()->LoadIcon (IDI_PRINT);
	icon[9] = AfxGetApp()->LoadIcon (IDI_NETWORK);
	icon[10] = AfxGetApp()->LoadIcon (IDI_WINSOCK);
	icon[11] = AfxGetApp()->LoadIcon (IDI_SECURITY);
	icon[12] = AfxGetApp()->LoadIcon (IDI_TASK);
	
	m_TreeImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 2); 
	for(int n = 0; n < MAX_ICON; n++)
	{
		m_TreeImageList.Add(icon[n]);
		m_tree.SetImageList(&m_TreeImageList, LVSIL_NORMAL);
	}

	DWORD dwStyle = GetWindowLong(m_tree.m_hWnd, GWL_STYLE);
	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	::SetWindowLong (m_tree.m_hWnd, GWL_STYLE, dwStyle);

	HTREEITEM Computer = m_tree.InsertItem(szAll[g_enumLang], 0, 0);
	m_tree.InsertItem(szStartup[g_enumLang], 1, 1, Computer, TVI_LAST);
	m_tree.InsertItem(szWinlogon[g_enumLang], 2, 2, Computer, TVI_LAST);
 	m_tree.InsertItem(szExplorer[g_enumLang], 3, 3, Computer, TVI_LAST);
 	m_tree.InsertItem(szInternetExplorer[g_enumLang], 4, 4, Computer, TVI_LAST);
	m_tree.InsertItem(szAutoRunServices[g_enumLang], 5, 5, Computer, TVI_LAST);
	m_tree.InsertItem(szAutoRunDrivers[g_enumLang], 6, 6, Computer, TVI_LAST);
	m_tree.InsertItem(szKnowDlls[g_enumLang], 7, 7, Computer, TVI_LAST);
	m_tree.InsertItem(szPrintMonitors[g_enumLang], 8, 8, Computer, TVI_LAST);
	m_tree.InsertItem(szNetworkProviders[g_enumLang], 9, 9, Computer, TVI_LAST);
	m_tree.InsertItem(szWinsockProviders[g_enumLang], 10, 10, Computer, TVI_LAST);
	m_tree.InsertItem(szSecurityProviders[g_enumLang], 11, 11, Computer, TVI_LAST);
	m_tree.InsertItem(szScheduledTasks[g_enumLang], 12, 12, Computer, TVI_LAST);

	m_tree.Expand(Computer, TVE_EXPAND);

	return TRUE;
}

BOOL CAutoStartDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitTree();
	
	// 获取下当前用户的注册表路劲
	m_szHKeyCurrentUser = m_RegCommonFunc.GetCurrentUserKeyPath();

	// 设置列表框
	m_ProImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ProImageList, LVSIL_SMALL);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/ | LVS_EX_CHECKBOXES);
	m_list.InsertColumn(0, szName[g_enumLang], LVCFMT_LEFT, 190);
	m_list.InsertColumn(1, szDescription[g_enumLang], LVCFMT_LEFT, 240);
	m_list.InsertColumn(2, szFile[g_enumLang], LVCFMT_LEFT, 380);
	m_list.InsertColumn(3, szFileCorporation[g_enumLang], LVCFMT_LEFT, 170);

// 	m_xSplitter.BindWithControl(this, IDC_X_SP);
// 	m_xSplitter.SetMinWidth(0, 0);
// 
// 	m_xSplitter.AttachAsLeftPane(IDC_TREE);
// 	m_xSplitter.AttachAsRightPane(IDC_LIST);
// 	m_xSplitter.AttachAsRightPane(IDC_AUTO_START_STATUS);
// 
// 	m_xSplitter.RecalcLayout();

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CAutoStartDlg::EnumWinlogonCommonEx(CString szKey, CString szValue, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty() || szValue.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen &&
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == szValue.GetLength() && 
			wcslen((WCHAR*)(ir->pData)) > 0 &&
			!szValue.CompareNoCase(ir->szName))
		{
			AddAutroRunItem(
				type, 
				ir->szName, 
				m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type),
				szKey,
				ir->szName,
				nStatus);

			m_nWinlogon++;
		}
	}

	m_Registry.FreeValueList(valueList);
}

void CAutoStartDlg::EnumWinlogonCommon(CString szKey, CString szValue, AUTO_START_TYPE type)
{
	if (szKey.IsEmpty() || szValue.IsEmpty())
	{
		return;
	}
	
	EnumWinlogonCommonEx(szKey, szValue, type, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumWinlogonCommonEx(szKey, szValue, type, eDisable);
}

CString CAutoStartDlg::GetWinlogonModule(CString szKey)
{
	CString szModule = L"";

	if (szKey.IsEmpty())
	{
		return szModule;
	}

	WCHAR szDllName[] = {'D','l','l','N','a','m','e','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen &&
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == wcslen(szDllName) &&
			(ir->Type == REG_SZ || ir->Type == REG_EXPAND_SZ) &&
			!ir->szName.CompareNoCase(szDllName))
		{
			szModule = m_Function.RegParsePath((WCHAR*)(ir->pData), ir->Type);
			break;
		}
	}

	m_Registry.FreeValueList(valueList);

	return szModule;
}

void CAutoStartDlg::EnumWinlogonKeyEx(CString szKey, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus)
{
	if ( szKey.IsEmpty() )
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szName = ir->szName;
			CString szPath = GetWinlogonModule(szKey + CString(L"\\") + szName);

			if (!szPath.IsEmpty())
			{
				AddAutroRunItem(type,
					szName,
					szPath,
					szKey + CString(L"\\") + szName,
					L"",
					nStatus);

				m_nWinlogon++;
			}
		}
	}
}

void CAutoStartDlg::EnumWinlogonKey(CString szKey, AUTO_START_TYPE type)
{
	if ( szKey.IsEmpty() )
	{
		return;
	}
	
	EnumWinlogonKeyEx(szKey, type, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumWinlogonKeyEx(szKey, type, eDisable);
}

void CAutoStartDlg::EnumWindowsLoadAndRun()
{
	CString szCuKey;
	if (!m_szHKeyCurrentUser.IsEmpty())
	{
		szCuKey = m_szHKeyCurrentUser;
		szCuKey += L"\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
	}

	if (!szCuKey.IsEmpty())
	{
		EnumWinlogonCommon(szCuKey, L"Load", eHKCU_WINDOWS_LOAD);
		EnumWinlogonCommon(szCuKey, L"Run", eHKCU_WINDOWS_RUN);
	}
}

void CAutoStartDlg::EnumWinlogon()
{
	m_nWinlogon = 0;

	CString szLmKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
	CString szCuKey;
	if (!m_szHKeyCurrentUser.IsEmpty())
	{
		szCuKey = m_szHKeyCurrentUser;
		szCuKey += L"\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
	}

	EnumWindowsLoadAndRun();

	if (!szCuKey.IsEmpty())
	{
		EnumWinlogonCommon(szCuKey, L"Shell", eHKCU_WINLOGON_SHELL);
	}

	EnumWinlogonCommon(szLmKey, L"Shell", eHKLM_WINLOGON_SHELL);
	EnumWinlogonCommon(szLmKey, L"AppSetup", eHKLM_WINLOGON_APPSETUP);
	EnumWinlogonCommon(szLmKey, L"GinaDLL", eHKLM_WINLOGON_GINADLL);
	EnumWinlogonCommon(szLmKey, L"LsaStart", eHKLM_WINLOGON_LSASTART);
	EnumWinlogonCommon(szLmKey, L"SaveDumpStart", eHKLM_WINLOGON_SaveDumpStart);
	EnumWinlogonCommon(szLmKey, L"ServiceControllerStart", eHKLM_WINLOGON_ServiceControllerStart);
	EnumWinlogonCommon(szLmKey, L"System", eHKLM_WINLOGON_System);
	EnumWinlogonCommon(szLmKey, L"Taskman", eHKLM_WINLOGON_Taskman);
	EnumWinlogonCommon(szLmKey, L"UIHost", eHKLM_WINLOGON_UIHost);
	EnumWinlogonCommon(szLmKey, L"Userinit", eHKLM_WINLOGON_Userinit);
	EnumWinlogonKey(szLmKey + L"\\Notify", eHKLM_WINLOGON_Notify);
	EnumWinlogonKey(szLmKey + L"\\GPExtensions", eHKLM_WINLOGON_GPExtensions);
}

BOOL CAutoStartDlg::ReadShortcut(LPWSTR lpwLnkFile, LPWSTR lpDescFile) 
{ 
	BOOL bReturn = FALSE; 
	IShellLink *pShellLink; 

	bReturn = (CoInitialize(NULL) == S_OK); 
	if(bReturn) 
	{ 
		bReturn = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
			IID_IShellLink, (void **)&pShellLink) >= 0; 
		if(bReturn) 
		{ 
			IPersistFile *ppf; 
			bReturn = pShellLink->QueryInterface(IID_IPersistFile, (void **)&ppf) >= 0; 
			if(bReturn) 
			{ 
				bReturn = ppf->Load(lpwLnkFile, TRUE) >= 0; 
				if(bReturn) 
				{ 
					pShellLink->GetPath(lpDescFile, MAX_PATH, NULL, 0); 
				} 

				ppf->Release(); 
			} 

			pShellLink->Release(); 
		} 

		CoUninitialize(); 
	} 

	return bReturn; 
}

// 由程序的Lnk路径得到对应的绝对路径
CString CAutoStartDlg::ParseLnk2Path(CString pszLnkName)
{
	CString szRet;
	::CoInitialize(NULL);
	IShellLink* pShell = NULL;
	IPersistFile* pFile = NULL;
	if(S_OK == ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShell))
	{
		if(S_OK == pShell->QueryInterface(IID_IPersistFile, (LPVOID*)&pFile))
		{
			pFile->Load(pszLnkName, STGM_READWRITE);
			TCHAR szPath[MAX_PATH] = {0};
			pShell->GetPath(szPath, MAX_PATH, NULL, SLGP_UNCPRIORITY);
			szRet = CString(szPath);
		}
	}

	pShell->Release();
	pFile->Release();
	::CoUninitialize();
	return szRet;
}

void CAutoStartDlg::AddAutroRunItem(AUTO_START_TYPE nType,
									CString szName,
									CString szPath,
									CString szKeyPath,
									CString szValueName,
									AUTO_RUN_STATUS nStatus)
{
	AUTO_START_INFO info;
	info.Type = nType;
	info.szName = szName;
	info.szPath = szPath;
	info.szKeyPath = szKeyPath;
	info.szValueName = szValueName;
	info.nStatus = nStatus;
	if (szName.IsEmpty() && !szPath.IsEmpty())
	{
		info.szName = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
	}
	m_AutostartList.push_back(info);
}

void CAutoStartDlg::EnumStartupDirEx(CString szDir, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus)
{
	if (szDir.IsEmpty() || !PathFileExists(szDir))
	{
		return;
	}

	WCHAR szLnk[] = {'.','L','N','K','\0'};
	WCHAR szDesktop[] = {'d','e','s','k','t','o','p','.','i','n','i','\0'};
	WCHAR szDot[] = {'*','.','*','\0'};

	if (szDir.Right(1) != L'\\')
	{
		szDir += L"\\";
	}
	szDir += szDot;

	CFileFind cff;
	BOOL bRet = cff.FindFile(szDir);
	while (bRet)
	{
		bRet = cff.FindNextFile();

		if (cff.IsDirectory() || 
			cff.IsDots() || 
			!(cff.GetFileName()).CompareNoCase(szDesktop)
			)
		{
			continue;
		}

		CString szPath;
		CString szTempName = cff.GetFileName();
		szTempName.MakeUpper();
		if (szTempName.Find(szLnk) != -1)
		{
			WCHAR StartupPath[MAX_PATH] = {0};
			if (ReadShortcut(cff.GetFilePath().GetBuffer(), StartupPath))
			{
				szPath = StartupPath;
			}
			else
			{
				szPath = ParseLnk2Path(cff.GetFilePath());
				if (szPath.IsEmpty())
				{
					szPath = cff.GetFilePath();
				}
			}

			cff.GetFilePath().ReleaseBuffer();
		}
		else
		{
			szPath = cff.GetFilePath();
		}

		AddAutroRunItem(
			type, 
			cff.GetFileTitle(),
			szPath, 
			L"",
			L"",
			nStatus);

		m_nStartup++;
	}

	cff.Close();
}

void CAutoStartDlg::EnumStartupDirectory()
{
	if (m_szStartupPath.IsEmpty())
	{
		WCHAR szStartupPath[MAX_PATH] = {0};
		if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szStartupPath, CSIDL_STARTUP, FALSE))
		{
			m_szStartupPath = szStartupPath;
		}
	}

	if (!m_szStartupPath.IsEmpty())
	{
		EnumStartupDirEx(m_szStartupPath, eStartupDirectory, eOK);

		CString szPathTemp = m_szStartupPath;
		szPathTemp = szPathTemp.Left(szPathTemp.ReverseFind('\\'));
		szPathTemp += CString(L"\\") + szAntiSpyDisableAutoruns;
		EnumStartupDirEx(szPathTemp, eStartupDirectory, eDisable);
	}
}

void CAutoStartDlg::EnumCommonStartupDirectory()
{
	if (m_szCommonStartupPath.IsEmpty())
	{
		WCHAR szStartupPath[MAX_PATH] = {0};
		if (SHGetSpecialFolderPath(this->GetSafeHwnd(), szStartupPath, CSIDL_COMMON_STARTUP, FALSE))
		{
			m_szCommonStartupPath = szStartupPath;
		}
	}

	if (!m_szCommonStartupPath.IsEmpty())
	{
		EnumStartupDirEx(m_szCommonStartupPath, eCommonStartupDirectory, eOK);

		CString szPathTemp = m_szCommonStartupPath;
		szPathTemp = szPathTemp.Left(szPathTemp.ReverseFind('\\'));
		szPathTemp += CString(L"\\") + szAntiSpyDisableAutoruns;
		EnumStartupDirEx(szPathTemp, eCommonStartupDirectory, eDisable);
	}
}

void CAutoStartDlg::EnumHKCURun()
{
	if (!m_szHKeyCurrentUser.IsEmpty())
	{
		CString HKCU_RUN = m_szHKeyCurrentUser;
		HKCU_RUN += L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
		EnumValue(HKCU_RUN, eHKCU_RUN);
	}
}

void CAutoStartDlg::EnumHKCURunOnce()
{
	if (!m_szHKeyCurrentUser.IsEmpty())
	{
		CString HKCU_RUN = m_szHKeyCurrentUser;
		HKCU_RUN += L"\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
		EnumValue(HKCU_RUN, eHKCU_RUNONCE);
	}
}

void CAutoStartDlg::EnumHKCURunOnceEx()
{
	if (!m_szHKeyCurrentUser.IsEmpty())
	{
		CString HKCU_RUN = m_szHKeyCurrentUser;
		HKCU_RUN += L"\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx";
		EnumValue(HKCU_RUN, eHKCU_RUNONCEEX);
	}
}

void CAutoStartDlg::EnumHKCUExplorerRun()
{
	if (!m_szHKeyCurrentUser.IsEmpty())
	{
		CString HKCU_RUN = m_szHKeyCurrentUser;
		HKCU_RUN += L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run";
		EnumValue(HKCU_RUN, eHKCU_EXPLORER_RUN);
	}
}

// 枚举正常的启动项
void CAutoStartDlg::EnumStartup()
{
	m_nStartup = 0;

	// 当前用户启动文件夹
	EnumStartupDirectory();

	// 公用的启动文件夹
	EnumCommonStartupDirectory();

	// HKLM_RUN
	EnumValue(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", eHKLM_RUN);

	// HKCU_RUN
	EnumHKCURun();

	// HKLM_RUNONCE
	EnumValue(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", eHKLM_RUNONCE);

	// HKCU_RUNONCE
	EnumHKCURunOnce();
	
	// HKLM_RUNONCE_SETUP
	EnumValue(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce\\Setup", eHKLM_RUNONCE_SETUP);

	// HKCU_RUNONCEEX
	EnumHKCURunOnceEx();
	
	// HKLM_RUNONCEEX
	EnumValue(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx", eHKLM_RUNONCEEX);

	// HKCU_EXPLORER_RUN
	EnumHKCUExplorerRun();

	// HKLM_EXPLORER_RUN
	EnumValue(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run", eHKLM_EXPLORER_RUN);
}

void CAutoStartDlg::EnumSharedTaskSchedulerEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen &&  ir->nDataLen)
		{
			CString szName = ir->szName;
			CString szModule = m_RegCommonFunc.GetCLSIDModule(szName);
			if (!szModule.IsEmpty())
			{
				AddAutroRunItem(eSharedTaskScheduler,
					(WCHAR*)ir->pData,
					szModule,
					szKey,
					szName,
					nStatus);

				m_nExplorer++;
			}
		}
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\SharedTaskScheduler
void CAutoStartDlg::EnumSharedTaskScheduler()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler";
	EnumSharedTaskSchedulerEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumSharedTaskSchedulerEx(szKey, eDisable);
}

void CAutoStartDlg::EnumShellExecuteHooksEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && ir->nDataLen)
		{
			CString szName = ir->szName;
			CString szModule = m_RegCommonFunc.GetCLSIDModule(szName);
			if (!szModule.IsEmpty())
			{
				AddAutroRunItem(eShellExecuteHooks,
					(WCHAR*)ir->pData,
					szModule,
					szKey,
					szName,
					nStatus);

				m_nExplorer++;
			}
		}
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellExecuteHooks
void CAutoStartDlg::EnumShellExecuteHooks()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks";
	EnumShellExecuteHooksEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumShellExecuteHooksEx(szKey, eDisable);
}

CString CAutoStartDlg::GetContextMenuHandlersCLSID(CString szKey)
{
	CString szCLSID = L"";
	if (szKey.IsEmpty())
	{
		return szCLSID;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == 0 && ir->nDataLen > 0 )
		{
			szCLSID = (WCHAR*)ir->pData;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szCLSID;
}

void CAutoStartDlg::EnumContextMenuHandlerEx(CString szKey, AUTO_START_TYPE Type, AUTO_RUN_STATUS nStatus)
{
	if(szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if ( ir->nNameLen )
		{
			CString szName = ir->szName;
			CString szKeyPath = szKey + CString(L"\\") + szName; 
			CString szCLSIDKey = GetContextMenuHandlersCLSID(szKeyPath);
			if (!szCLSIDKey.IsEmpty())
			{
				CString szModule = m_RegCommonFunc.GetCLSIDModule(szCLSIDKey);
				if (!szModule.IsEmpty())
				{
					AddAutroRunItem(Type,
						szName,
						szModule,
						szKeyPath,
						L"",
						nStatus);

					m_nExplorer++;
				}
			}
		}
	}
}

void CAutoStartDlg::EnumContextMenuHandler(CString szKey, AUTO_START_TYPE Type)
{
	if(szKey.IsEmpty())
	{
		return;
	}
	
	EnumContextMenuHandlerEx(szKey, Type, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumContextMenuHandlerEx(szKey, Type, eDisable);
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\*\shellex\ContextMenuHandlers
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\Background\shellex\ContextMenuHandlers
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\shellex\ContextMenuHandlers
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Drive\shellex\ContextMenuHandlers
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Folder\shellex\ContextMenuHandlers
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\shellex\CopyHookHandlers
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\shellex\DragDropHandlers
void CAutoStartDlg::EnumContextMenuHandlers()
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','*','\\','s','h','e','l','l','e','x','\\','C','o','n','t','e','x','t','M','e','n','u','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szKey, eContextMenuHandlers);

	WCHAR szKey2[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','A','l','l','F','i','l','e','s','y','s','t','e','m','O','b','j','e','c','t','s','\\','s','h','e','l','l','e','x','\\','C','o','n','t','e','x','t','M','e','n','u','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szKey2, eContextMenuHandlers2);

	WCHAR szKey3[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','D','i','r','e','c','t','o','r','y','\\','B','a','c','k','g','r','o','u','n','d','\\','s','h','e','l','l','e','x','\\','C','o','n','t','e','x','t','M','e','n','u','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szKey3, eContextMenuHandlers3);

	WCHAR szKey4[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','D','i','r','e','c','t','o','r','y','\\','s','h','e','l','l','e','x','\\','C','o','n','t','e','x','t','M','e','n','u','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szKey4, eContextMenuHandlers4);

	WCHAR szKey5[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','D','r','i','v','e','\\','s','h','e','l','l','e','x','\\','C','o','n','t','e','x','t','M','e','n','u','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szKey5, eContextMenuHandlers5);

	WCHAR szKey6[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','F','o','l','d','e','r','\\','s','h','e','l','l','e','x','\\','C','o','n','t','e','x','t','M','e','n','u','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szKey6, eContextMenuHandlers6);

	WCHAR szKeyCopyHookHandlers[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','D','i','r','e','c','t','o','r','y','\\','s','h','e','l','l','e','x','\\','C','o','p','y','H','o','o','k','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szKeyCopyHookHandlers, eCopyHookHandlers);

	WCHAR szDragDropHandlers[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','l','a','s','s','e','s','\\','D','i','r','e','c','t','o','r','y','\\','s','h','e','l','l','e','x','\\','D','r','a','g','D','r','o','p','H','a','n','d','l','e','r','s','\0'};
	EnumContextMenuHandler(szDragDropHandlers, eDragDropHandlers);
}

void CAutoStartDlg::EnumShellServiceObjectDelayLoadEx( CString szKey, AUTO_RUN_STATUS nStatus )
{
	if (szKey.IsEmpty())
	{
		return;
	}
	
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if ( ir->nDataLen > 0 )
		{
			CString szData = (WCHAR*)ir->pData;
			CString szModule = m_RegCommonFunc.GetCLSIDModule(szData);
			if (!szModule.IsEmpty())
			{
				AddAutroRunItem(eShellServiceObjectDelayLoad,
					ir->szName,
					szModule,
					szKey,
					ir->szName,
					nStatus);

				m_nExplorer++;
			}
		}	
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\ShellServiceObjectDelayLoad
void CAutoStartDlg::EnumShellServiceObjectDelayLoad()
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','S','h','e','l','l','S','e','r','v','i','c','e','O','b','j','e','c','t','D','e','l','a','y','L','o','a','d','\0'};
	CString szTemp = szKey;
	EnumShellServiceObjectDelayLoadEx( szTemp, eOK );

	szTemp = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumShellServiceObjectDelayLoadEx( szTemp, eDisable );
}

CString CAutoStartDlg::GetFilterOrHandlerCLSID(CString szKey)
{
	CString szCLSID = L"";
	if (szKey.IsEmpty())
	{
		return szCLSID;
	}
	
	WCHAR szCLSIDTemp[] = {'C','L','S','I','D','\0'};
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && 
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == wcslen(szCLSIDTemp) &&
			!ir->szName.CompareNoCase(szCLSIDTemp))
		{
			szCLSID = (WCHAR*)ir->pData;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szCLSID;
}

void CAutoStartDlg::EnumFilterOrHandlerEx(CString szKey, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szName = ir->szName;
			CString szKeyPath = szKey + CString(L"\\") + szName;
			CString szCLSIDKey = GetFilterOrHandlerCLSID(szKeyPath);
			if (!szCLSIDKey.IsEmpty())
			{
				CString szModule = m_RegCommonFunc.GetCLSIDModule(szCLSIDKey);
				if (!szModule.IsEmpty())
				{
					AddAutroRunItem(type,
						szName,
						szModule,
						szKeyPath,
						L"",
						nStatus);

					m_nExplorer++;
				}
			}
		}
	}
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\PROTOCOLS\Filter
// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\PROTOCOLS\Handler
void CAutoStartDlg::EnumFilterOrHandler(ULONG Type)
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Classes\\PROTOCOLS\\";

	if (Type == 1) szKey += L"Filter";
	else if (Type == 2)	szKey += L"Handler";
	else return;

	EnumFilterOrHandlerEx(szKey, Type == 1 ? eFilter : eHandler, eOK);

	szKey += CString("\\") + szAntiSpyDisableAutoruns;
	EnumFilterOrHandlerEx(szKey, Type == 1 ? eFilter : eHandler, eDisable);
}

void CAutoStartDlg::EnumApprovedEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && ir->szName.GetAt(0) == '{')
		{
			CString szCLSID = ir->szName;
			CString szModule = m_RegCommonFunc.GetCLSIDModule(szCLSID);
			if (!szModule.IsEmpty())
			{
				AddAutroRunItem(eApproved,
					(WCHAR*)ir->pData,
					szModule,
					szKey,
					szCLSID,
					nStatus);

				m_nExplorer++;
			}
		}
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved
void CAutoStartDlg::EnumApproved()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";
	EnumApprovedEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumApprovedEx(szKey, eDisable);
}

CString CAutoStartDlg::GetShellIconOverlayIdentifiersModulePath(CString szKey)
{
	CString szRet = L"";
	if (szKey.IsEmpty())
	{
		return szRet;
	}
	
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == 0 && ir->nDataLen > 0 && *(WCHAR*)ir->pData == '{')
		{
			CString szCLSID = (WCHAR*)ir->pData;
			szRet = m_RegCommonFunc.GetCLSIDModule(szCLSID);
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szRet;
}

void CAutoStartDlg::EnumShellIconOverlayIdentifiersEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->szName.GetLength() > 0)
		{
			CString szName = ir->szName;
			AddAutroRunItem(eShellIconOverlayIdentifiers,
				szName,
				GetShellIconOverlayIdentifiersModulePath(szKey + L"\\" + szName),
				szKey + L"\\" + szName,
				L"",
				nStatus);

			m_nExplorer++;
		}
	}
}
// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers
void CAutoStartDlg::EnumShellIconOverlayIdentifiers()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers";
	EnumShellIconOverlayIdentifiersEx(szKey, eOK);
	
	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumShellIconOverlayIdentifiersEx(szKey, eDisable);
}	

void CAutoStartDlg::EnumExplorer()
{
	m_nExplorer = 0;

	EnumSharedTaskScheduler();
	EnumShellExecuteHooks();
	EnumShellIconOverlayIdentifiers();
	EnumContextMenuHandlers();
	EnumShellServiceObjectDelayLoad();
	EnumFilterOrHandler(1);
	EnumFilterOrHandler(2);
	EnumApproved();
}

void CAutoStartDlg::EnumBrowserHelperObjectsEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen && ir->szName.GetAt(0) == '{')
		{
			CString szName = ir->szName;
			CString szModule = m_RegCommonFunc.GetCLSIDModule(szName);
			if (!szModule.IsEmpty())
			{
				AddAutroRunItem(
					eBrowserHelperObjects,
					m_RegCommonFunc.GetCLSIDDescription(szName),
					szModule,
					szKey + CString(L"\\") + szName,
					L"",
					nStatus);

				m_nInternetExplorer++;
			}
		}
	}
}

void CAutoStartDlg::EnumBrowserHelperObjects()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects";
	EnumBrowserHelperObjectsEx(szKey, eOK);

	szKey += CString("\\") + szAntiSpyDisableAutoruns;
	EnumBrowserHelperObjectsEx(szKey, eDisable);
}

void CAutoStartDlg::EnumURLSearchHooksEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->szName.GetAt(0) == '{')
		{
			CString szName = ir->szName;

			AddAutroRunItem(eURLSearchHooks,
				m_RegCommonFunc.GetCLSIDDescription(szName),
				m_RegCommonFunc.GetCLSIDModule(szName),
				szKey,
				szName,
				nStatus);

			m_nInternetExplorer++;
		}
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\URLSearchHooks
void CAutoStartDlg::EnumURLSearchHooks()
{
	CString szCuKey;
	WCHAR szKey[] = {'S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','I','n','t','e','r','n','e','t',' ','E','x','p','l','o','r','e','r','\\','U','R','L','S','e','a','r','c','h','H','o','o','k','s','\0'};

	if (!m_szHKeyCurrentUser.IsEmpty())
	{
		szCuKey = m_szHKeyCurrentUser;
	}
	
	if (szCuKey.IsEmpty())
	{
		return;
	}
	
	szCuKey += CString(L"\\") + szKey;
	EnumURLSearchHooksEx(szCuKey, eOK);

	szCuKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumURLSearchHooksEx(szCuKey, eDisable);
}

CString CAutoStartDlg::GetBrowserExtensionsButtonText(CString szKey)
{
	CString szRet;

	if (szKey.IsEmpty())
	{
		return szRet;
	}
	
	WCHAR szButtonText[] = {'B','u','t','t','o','n','T','e','x','t','\0'};
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szButtonText) * sizeof(WCHAR) && 
			!ir->szName.CompareNoCase(szButtonText) &&
			ir->nDataLen > 0 &&
			wcslen((WCHAR*)ir->pData) > 0)
		{
			szRet = (WCHAR*)(ir->pData);
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szRet;
}

CString CAutoStartDlg::GetBrowserExtensionsModulePath(CString szKey)
{
	CString szRet;

	if (szKey.IsEmpty())
	{
		return szRet;
	}

	WCHAR szButtonText[] = {'I','c','o','n','\0'};
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szButtonText) * sizeof(WCHAR) && 
			!ir->szName.CompareNoCase(szButtonText) &&
			ir->nDataLen > 0 &&
			wcslen((WCHAR*)ir->pData) > 0)
		{
			szRet = m_Function.RegParsePath((WCHAR*)(ir->pData), ir->Type);
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szRet;
}

void CAutoStartDlg::EnumBrowserExtensionsEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->szName.GetAt(0) == '{')
		{
			CString szKeyPath = szKey + CString(L"\\") + ir->szName;

			AddAutroRunItem(eExtensions,
				GetBrowserExtensionsButtonText(szKeyPath),
				GetBrowserExtensionsModulePath(szKeyPath),
				szKeyPath,
				L"",
				nStatus);

			m_nInternetExplorer++;
		}
	}
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Internet Explorer\Extensions
void CAutoStartDlg::EnumBrowserExtensions()
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','I','n','t','e','r','n','e','t',' ','E','x','p','l','o','r','e','r','\\','E','x','t','e','n','s','i','o','n','s','\0'};
	CString szTemp = szKey;
	EnumBrowserExtensionsEx(szTemp, eOK);

	szTemp += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumBrowserExtensionsEx(szTemp, eDisable);
}

void CAutoStartDlg::EnumInternetExplorer()
{
	m_nInternetExplorer = 0;
	EnumBrowserHelperObjects();
	EnumURLSearchHooks();
	EnumBrowserExtensions();
}

void CAutoStartDlg::EnumKnowDllsEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nDataLen > 0 && ir->Type == REG_SZ && wcslen((WCHAR*)ir->pData) > 0)
		{
			CString szModule = m_Function.RegParsePath((WCHAR*)ir->pData, REG_SZ);
			if (!szModule.IsEmpty())
			{
				AddAutroRunItem(eKnowDlls,
					ir->szName,
					szModule,
					szKey,
					ir->szName,
					nStatus);

				m_nKnowDlls++;
			}
		}
	}

	m_Registry.FreeValueList(valueList);
}

void CAutoStartDlg::EnumKnowDlls()
{
	m_nKnowDlls = 0;

	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs";
	EnumKnowDllsEx(szKey, eOK);
	
	szKey += CString("\\") + szAntiSpyDisableAutoruns;
	EnumKnowDllsEx(szKey, eDisable);
}

CString CAutoStartDlg::GetPrintMonitorModule(CString szKey)
{
	CString szModule = L"";
	
	if (szKey.IsEmpty())
	{
		return szModule;
	}
	
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	
	WCHAR szDriver[] = {'D','r','i','v','e','r','\0'};

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && 
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == wcslen(szDriver) &&
			!ir->szName.CompareNoCase(szDriver))
		{
			szModule = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			break;
		}
	}
	
	m_Registry.FreeValueList(valueList);
	return szModule;
}

void CAutoStartDlg::EnumPrintMonitorsEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen)
		{
			CString szName = ir->szName;
			CString szKeyPath = szKey + CString(L"\\") + szName;
			CString szModule = GetPrintMonitorModule(szKeyPath);
			if (!szModule.IsEmpty())
			{
				AddAutroRunItem(ePrintMonitors,
					szName,
					szModule,
					szKeyPath,
					L"",
					nStatus);

				m_nPrintMonitors++;
			}
		}
	}
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Print\Monitors
void CAutoStartDlg::EnumPrintMonitors()					
{
	m_nPrintMonitors = 0;
	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors";
	EnumPrintMonitorsEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumPrintMonitorsEx(szKey, eDisable);
}

CString CAutoStartDlg::GetNetworkProviderModule(CString szKey)
{
	CString szModule = L"";
	if (szKey.IsEmpty())
	{
		return szModule;
	}

	CString szKeyPath = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\";
	szKeyPath += szKey;
	szKeyPath += L"\\NetworkProvider";

	WCHAR szProviderPath[] = {'P','r','o','v','i','d','e','r','P','a','t','h','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKeyPath, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && 
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == wcslen(szProviderPath) &&
			!ir->szName.CompareNoCase(szProviderPath))
		{
			szModule = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			break;
		}

	}

	m_Registry.FreeValueList(valueList);
	return szModule;
}

void CAutoStartDlg::EnumNetworkProviderEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	CString szModules = L"";
	WCHAR szProviderOrder[] = {'P','r','o','v','i','d','e','r','O','r','d','e','r','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && 
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == wcslen(szProviderOrder) &&
			!ir->szName.CompareNoCase(szProviderOrder))
		{
			szModules = (WCHAR*)ir->pData;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);

	if (szModules.IsEmpty())
	{
		return;
	}

	int nFind = -1;

	do 
	{
		nFind = szModules.Find(',');
		if (nFind != -1)
		{
			CString szName = szModules.Left(nFind);
			if (!szName.IsEmpty())
			{
				AddAutroRunItem(eNetworkProviders,
					szName,
					GetNetworkProviderModule(szName),
					szKey,
					szProviderOrder,
					nStatus);

				m_nNetworkProvider++;
			}
		}
		else if (nFind == -1 && !szModules.IsEmpty())
		{
			AddAutroRunItem(eNetworkProviders,
				szModules,
				GetNetworkProviderModule(szModules),
				szKey,
				szProviderOrder,
				nStatus);

			m_nNetworkProvider++;
		}

		szModules = szModules.Right(szModules.GetLength() - nFind - 1);

	} while (nFind != -1);
}

// // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\NetworkProvider\Order,ProviderOrder
void CAutoStartDlg::EnumNetworkProvider()
{
	m_nNetworkProvider = 0;

	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order";
	EnumNetworkProviderEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumNetworkProviderEx(szKey, eDisable);
}

CString CAutoStartDlg::MByteToWChar(IN LPCSTR lpcszStr)
{
	CString szRet = L"";

	if (NULL == lpcszStr)
	{
		return szRet;
	}

	DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);
	WCHAR* lpwszStr = (WCHAR *)new WCHAR[dwNum];
	if (lpwszStr == NULL)
	{
		return szRet;
	}

	memset(lpwszStr, 0, dwNum * sizeof(WCHAR));

	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwNum);  
	lpwszStr[dwNum - 1] = _T('\0');

	szRet = lpwszStr;

	delete []lpwszStr;
	lpwszStr = NULL;

	return szRet;
}

CString CAutoStartDlg::GetProtocolCatalog9NameAndModule(CString szKey, CString &szModule)
{
	CString szName = L"";

	if (szKey.IsEmpty())
	{
		szModule = L"";
		return szName;
	}

	WCHAR szPackedCatalogItem[] = {'P','a','c','k','e','d','C','a','t','a','l','o','g','I','t','e','m','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen && 
			ir->nDataLen && 
			ir->nNameLen / sizeof(WCHAR) == wcslen(szPackedCatalogItem) &&
			!ir->szName.CompareNoCase(szPackedCatalogItem))
		{
			szModule = MByteToWChar((CHAR*)ir->pData);
			if (!szModule.IsEmpty())
			{
				szName = (WCHAR*)(((PBYTE)ir->pData) + 0x178);
			}

			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szName;
}

void CAutoStartDlg::EnumProtocol_Catalog9Ex(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen)
		{
			CString szKeyPath = szKey + CString(L"\\") + ir->szName;
			CString szPath = L"";
			CString szName = GetProtocolCatalog9NameAndModule(szKeyPath, szPath);

			if (!szPath.IsEmpty())
			{
				AddAutroRunItem(eProtocolCatalog9,
					szName,
					m_Function.RegParsePath(szPath.GetBuffer(), REG_SZ),
					szKeyPath,
					L"",
					nStatus);

				szPath.ReleaseBuffer();
				m_nWinsockProvider++;
			}
		}
	}
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries
void CAutoStartDlg::EnumProtocol_Catalog9()
{
	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries";
	EnumProtocol_Catalog9Ex(szKey, eOK);
	
	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumProtocol_Catalog9Ex(szKey, eDisable);
}

CString CAutoStartDlg::GetNameSpace_Catalog5NameAndModule(CString szKey, CString &szModule)
{
	CString szName = L"";
	if (szKey.IsEmpty())
	{
		szModule = L"";
		return szName;
	}

	WCHAR szDisplayString[] = {'D','i','s','p','l','a','y','S','t','r','i','n','g','\0'};
	WCHAR LibraryPath[] = {'L','i','b','r','a','r','y','P','a','t','h','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->nDataLen > 0)
		{
			if (ir->nNameLen / sizeof(WCHAR) == wcslen(szDisplayString) &&
				!ir->szName.CompareNoCase(szDisplayString))
			{
				szName = (WCHAR*)ir->pData;
			}
			else if (ir->nNameLen / sizeof(WCHAR) == wcslen(LibraryPath) &&
				!ir->szName.CompareNoCase(LibraryPath))
			{
				szModule = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			}
		}

		if (!szName.IsEmpty() && !szModule.IsEmpty())
		{
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szName;
}

void CAutoStartDlg::EnumNameSpace_Catalog5Ex(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szKeyPath = szKey + CString(L"\\") + ir->szName;
			CString szPath;
			CString szName = GetNameSpace_Catalog5NameAndModule(szKeyPath, szPath);

			if (!szPath.IsEmpty())
			{
				AddAutroRunItem(eNameSpaceCatalog5,
					szName,
					szPath,
					szKeyPath,
					L"",
					nStatus);

				m_nWinsockProvider++;
			}
		}
	}
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\NameSpace_Catalog5\Catalog_Entries
void CAutoStartDlg::EnumNameSpace_Catalog5()
{
	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\NameSpace_Catalog5\\Catalog_Entries";
	EnumNameSpace_Catalog5Ex(szKey, eOK);
	
	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumNameSpace_Catalog5Ex(szKey, eDisable);
}

void CAutoStartDlg::EnumWinsockProviders()					
{
	m_nWinsockProvider = 0;

	EnumProtocol_Catalog9();
	EnumNameSpace_Catalog5();
}

void CAutoStartDlg::EnumSecurityProviderEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	WCHAR szSecurityProviders[] = {'S','e','c','u','r','i','t','y','P','r','o','v','i','d','e','r','s','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->nDataLen > 0 &&
			ir->nNameLen / sizeof(WCHAR) == wcslen(szSecurityProviders) &&
			!ir->szName.CompareNoCase(szSecurityProviders))
		{
			int nFind = -1;
			CString szModules = (WCHAR*)ir->pData;

			do 
			{
				nFind = szModules.Find(',');
				if (nFind != -1)
				{
					CString szName = szModules.Left(nFind);
					szName.TrimRight(' ');
					szName.TrimLeft(' ');

					if (!szName.IsEmpty())
					{
						AddAutroRunItem(eSecurityProviders,
							szName,
							m_Function.RegParsePath(szName),
							szKey,
							szSecurityProviders,
							nStatus);

						m_nSecurityProvider++;
					}
				}
				else if (nFind == -1 && !szModules.IsEmpty())
				{
					CString szName = szModules;
					szName.TrimRight(' ');
					szName.TrimLeft(' ');

					AddAutroRunItem(eSecurityProviders,
						szName,
						m_Function.RegParsePath(szName),
						szKey,
						szSecurityProviders,
						nStatus);

					m_nSecurityProvider++;
				}

				szModules = szModules.Right(szModules.GetLength() - nFind - 1);

			} while (nFind != -1);

			break;
		}
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\SecurityProviders,SecurityProviders
void CAutoStartDlg::EnumSecurityProvider()
{
	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\SecurityProviders";
	EnumSecurityProviderEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumSecurityProviderEx(szKey, eDisable);
}

void CAutoStartDlg::EnumLASPackagesEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	WCHAR szAuthenticationPackages[] = {'A','u','t','h','e','n','t','i','c','a','t','i','o','n',' ','P','a','c','k','a','g','e','s','\0'};
	WCHAR szSecurityPackages[] = {'S','e','c','u','r','i','t','y',' ','P','a','c','k','a','g','e','s','\0'};
	WCHAR szNotificationPackages[] = {'N','o','t','i','f','i','c','a','t','i','o','n',' ','P','a','c','k','a','g','e','s','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if ( ir->nNameLen > 0 && ir->nDataLen > 0 )
		{
			AUTO_START_TYPE Type = eSrartupNone;

			if (ir->nNameLen / sizeof(WCHAR) == wcslen(szAuthenticationPackages) &&
				!ir->szName.CompareNoCase(szAuthenticationPackages))
			{
				Type = eAuthenticationPackages;
			}
			else if (ir->nNameLen / sizeof(WCHAR) == wcslen(szSecurityPackages) &&
				!ir->szName.CompareNoCase(szSecurityPackages))
			{
				Type = eSecurityPackages;
			}
			else if (ir->nNameLen / sizeof(WCHAR) == wcslen(szNotificationPackages) &&
				!ir->szName.CompareNoCase(szNotificationPackages))
			{
				Type = eNotificationPackages;
			}

			if (Type != eSrartupNone)
			{
				WCHAR* Data = (WCHAR*)ir->pData;
				DWORD len = 0;
				while (wcslen(Data + len))
				{
					CString szName = (Data + len);
					CString szPath = szName + L".dll";

					szPath = m_Function.RegParsePath(szPath);

					CString szValueName = L"";
					if (Type == eAuthenticationPackages)
					{
						szValueName = szAuthenticationPackages;
					}
					else if (Type == eSecurityPackages)
					{
						szValueName = szSecurityPackages;
					}
					else if (Type == eNotificationPackages)
					{
						szValueName = szNotificationPackages;
					}

					AddAutroRunItem(Type,
						szName,
						szPath,
						szKey,
						szValueName,
						nStatus);

					m_nSecurityProvider++;

					len += wcslen(Data + len) + 1;
				}
			}
		}
	}

	m_Registry.FreeValueList(valueList);
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Lsa,Security Packages
void CAutoStartDlg::EnumLASPackages()
{
	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Lsa";
	EnumLASPackagesEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumLASPackagesEx(szKey, eDisable);
}

void CAutoStartDlg::EnumCredentialProvidersEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString strName = ir->szName;
			CString szKeyPath = szKey + CString(L"\\") + strName;
			CString szPath = m_RegCommonFunc.GetCLSIDModule(strName);
			CString szName = GetCredentialProvidersName(szKeyPath);

			if (!szPath.IsEmpty())
			{
				AddAutroRunItem(eCredentialProviders,
					szName,
					szPath,
					szKeyPath,
					L"",
					eOK);

				m_nSecurityProvider++;
			}
		}
	}
}

// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers
void CAutoStartDlg::EnumCredentialProviders()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers";
	EnumCredentialProvidersEx(szKey, eOK);
	
	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumCredentialProvidersEx(szKey, eDisable);
}

CString CAutoStartDlg::GetCredentialProvidersName(CString szKey)
{
	CString szName = L"";

	if (szKey.IsEmpty())
	{
		return szName;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == 0 && ir->nDataLen > 0)
		{
			szName = (WCHAR*)ir->pData;
			break;
		}

	}

	m_Registry.FreeValueList(valueList);
	return szName;
}

void CAutoStartDlg::EnumCredentialProviderFiltersEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString strName = ir->szName;
			CString szKeyPath = szKey + CString(L"\\") + strName;
			CString szPath = m_RegCommonFunc.GetCLSIDModule(strName);
			CString szName = GetCredentialProvidersName(szKeyPath);

			if (!szPath.IsEmpty())
			{
				AddAutroRunItem(eCredentialProviderFilters,
					szName,
					szPath,
					szKeyPath,
					L"",
					nStatus);

				m_nSecurityProvider++;
			}
		}
	}
}

void CAutoStartDlg::EnumCredentialProviderFilters()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Provider Filters";
	EnumCredentialProviderFiltersEx(szKey, eOK);
	
	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumCredentialProviderFiltersEx(szKey, eDisable);
}

void CAutoStartDlg::EnumPLAPProvidersEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString strName = ir->szName;
			CString szKeyPath = szKey + CString(L"\\") + strName;
			CString szPath = m_RegCommonFunc.GetCLSIDModule(strName);
			CString szName = GetCredentialProvidersName(szKeyPath);

			if (!szPath.IsEmpty())
			{
				AddAutroRunItem(ePLAPProviders,
					szName,
					szPath,
					szKeyPath,
					L"",
					nStatus);

				m_nSecurityProvider++;
			}
		}
	}
}

void CAutoStartDlg::EnumPLAPProviders()
{
	CString szKey = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\PLAP Providers";
	EnumPLAPProvidersEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumPLAPProvidersEx(szKey, eDisable);
}

void CAutoStartDlg::EnumSecurityProviders()
{
	m_nSecurityProvider = 0;
	EnumSecurityProvider();
	EnumLASPackages();
	EnumCredentialProviders();
	EnumCredentialProviderFilters();
	EnumPLAPProviders();
}

#define TASKS_TO_RETRIEVE          5
HRESULT CAutoStartDlg::EnumTasks()
{
	m_nTask = 0;

	HRESULT hr = S_OK;
	ITaskScheduler *pITS;

	hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(
			CLSID_CTaskScheduler,
			NULL,
			/*CLSCTX_ALL,*/
			CLSCTX_INPROC_SERVER,
			IID_ITaskScheduler,
			(void **) &pITS);

		if (FAILED(hr))
		{
			CoUninitialize();
			return hr;
		}
	}
	else
	{
		return hr;
	}

	// 开始枚举
	IEnumWorkItems *pIEnum;
	hr = pITS->Enum(&pIEnum);
	if (FAILED(hr))
	{
		CoUninitialize();
		return hr;
	}

	// 取出枚举后得到的数据
	LPWSTR *lpwszNames;
	DWORD dwFetchedTasks = 0;
	while (SUCCEEDED( pIEnum->Next(TASKS_TO_RETRIEVE, &lpwszNames, &dwFetchedTasks)) && (dwFetchedTasks != 0) )
	{
		while (dwFetchedTasks)
		{
			ITask *pITask;
			wchar_t strTemp[MAX_PATH] = {0};
			SIZE_T len;

			len = wcslen(lpwszNames[--dwFetchedTasks]);
			while (len)
			{
				if (lpwszNames[dwFetchedTasks][--len] == L'.')
				{
					wcsncpy_s(strTemp, MAX_PATH, lpwszNames[dwFetchedTasks], len);
					break;
				}
			}

			hr = pITS->Activate(strTemp, IID_ITask, (IUnknown**) &pITask);
			if (FAILED(hr))
			{
				continue;
			}

			LPWSTR lpwszApplicationName;
			hr = pITask->GetApplicationName(&lpwszApplicationName);
			if (FAILED(hr))
			{
				continue;
			}
			
			AUTO_START_INFO info;
			info.Type = eTaskDirectory;
			info.szName = strTemp;
			info.szPath = lpwszApplicationName;

			if (info.szName.IsEmpty())
			{
				info.szName = info.szPath.Right(info.szPath.GetLength() - info.szPath.ReverseFind('\\') - 1);
			}

			m_AutostartList.push_back(info);
			m_nTask++;

			pITask->Release();
			CoTaskMemFree(lpwszApplicationName);
			CoTaskMemFree(lpwszNames[dwFetchedTasks]);
		}

		CoTaskMemFree(lpwszNames);
	}

	pITS->Release();
	pIEnum->Release();
	CoUninitialize();

	return hr;
}

void CAutoStartDlg::ShowAutostarts()
{
	m_bStartCheck = FALSE;

	m_SignVerifyDlg.m_NotSignDataList.clear();
	m_SignVerifyDlg.m_NotSignItemList.clear();
	m_list.DeleteAllItems();
	int nImageCount = m_ProImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ProImageList.Remove(0);   
	}

	ULONG nCnt = 0;
	for (vector <AUTO_START_INFO>::iterator Itor = m_AutostartList.begin( ); 
		Itor != m_AutostartList.end( ); 
		Itor++, nCnt++)
	{
		if (Itor->nStatus == eDelete)
		{	
			continue;
		}

		BOOL bInsert = FALSE;

		switch (m_AutoStartType)
		{
		case eNone:
		case eStartup:
			bInsert = 
				Itor->Type == eHKLM_RUN || 
				Itor->Type == eHKCU_RUN ||
				Itor->Type == eHKLM_RUNONCE ||
				Itor->Type == eHKCU_RUNONCE ||
				Itor->Type == eHKLM_RUNONCE_SETUP ||
				Itor->Type == eHKCU_RUNONCEEX ||
				Itor->Type == eHKLM_RUNONCEEX ||
				Itor->Type == eHKCU_EXPLORER_RUN ||
				Itor->Type == eHKLM_EXPLORER_RUN ||
				Itor->Type == eStartupDirectory ||
				Itor->Type == eCommonStartupDirectory;
			break;

		case eWinlogon:
			bInsert = 
				Itor->Type == eHKCU_WINDOWS_LOAD || 
				Itor->Type == eHKCU_WINDOWS_RUN || 
				Itor->Type == eHKCU_WINLOGON_SHELL || 
				Itor->Type == eHKLM_WINLOGON_SHELL ||
				Itor->Type == eHKLM_WINLOGON_APPSETUP ||
				Itor->Type == eHKLM_WINLOGON_GINADLL ||
				Itor->Type == eHKLM_WINLOGON_LSASTART ||
				Itor->Type == eHKLM_WINLOGON_SaveDumpStart ||
				Itor->Type == eHKLM_WINLOGON_ServiceControllerStart ||
				Itor->Type == eHKLM_WINLOGON_System ||
				Itor->Type == eHKLM_WINLOGON_Taskman ||
				Itor->Type == eHKLM_WINLOGON_UIHost ||
				Itor->Type == eHKLM_WINLOGON_Userinit ||
				Itor->Type == eHKLM_WINLOGON_GPExtensions ||
				Itor->Type == eHKLM_WINLOGON_Notify;
			break;

		case eExplorer:
			bInsert = 
				Itor->Type == eSharedTaskScheduler	|| 
				Itor->Type == eShellExecuteHooks	||
				Itor->Type == eShellServiceObjectDelayLoad ||
				Itor->Type == eShellIconOverlayIdentifiers ||
				Itor->Type == eContextMenuHandlers	||
				Itor->Type == eContextMenuHandlers2 ||
				Itor->Type == eContextMenuHandlers3 ||
				Itor->Type == eContextMenuHandlers4 ||
				Itor->Type == eContextMenuHandlers5 ||
				Itor->Type == eContextMenuHandlers6 ||
				Itor->Type == eCopyHookHandlers ||
				Itor->Type == eDragDropHandlers ||
				Itor->Type == eFilter ||
				Itor->Type == eHandler ||
 				Itor->Type == eApproved;
			break;

		case eIE:
			bInsert = 
				Itor->Type == eBrowserHelperObjects || 
				Itor->Type == eURLSearchHooks ||
				Itor->Type == eExtensions;
			break;

		case eKnowDLLs:
			bInsert = Itor->Type == eKnowDlls;
			break;

		case ePrintMonitor:
			bInsert = Itor->Type == ePrintMonitors;
			break;

		case eNetworkProvider:
			bInsert = Itor->Type == eNetworkProviders;
			break;

		case eWinsockProvider:
			bInsert = 
				Itor->Type == eProtocolCatalog9 ||
				Itor->Type == eNameSpaceCatalog5;
			break;

		case eSecurityProvider:
			bInsert = 
				Itor->Type == eSecurityProviders ||
				Itor->Type == eAuthenticationPackages ||
				Itor->Type == eNotificationPackages ||
				Itor->Type == eSecurityPackages ||
				Itor->Type == eCredentialProviders || 
				Itor->Type == eCredentialProviderFilters || 
				Itor->Type == ePLAPProviders;
			break;

		case eTask:
			bInsert = Itor->Type == eTaskDirectory;
			break;
		
		case eSystemService:
			bInsert = Itor->Type == eService;
			break;

		case eSystemDriver:
			bInsert = Itor->Type == eDriver;
			break;

		case eAll:
			bInsert = TRUE;
			break;
		}

		if (bInsert)
		{
			CString szCompany = m_Function.GetFileCompanyName(Itor->szPath);
			if (m_bHideMicrosoft)
			{
				if (!m_Function.IsMicrosoftApp(szCompany))
				{
					SHFILEINFO sfi;
					DWORD_PTR nRet;
					ZeroMemory(&sfi, sizeof(SHFILEINFO));
					nRet = SHGetFileInfo(Itor->szPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
					nRet ? m_ProImageList.Add ( sfi.hIcon ) : m_ProImageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
					if (nRet)
					{
						DestroyIcon(sfi.hIcon);
					}

					int i = m_list.GetItemCount();
					m_list.InsertItem(i, Itor->szName, i);
					m_list.SetItemText(i, 1, m_Function.GetFileDescription(Itor->szPath));
					m_list.SetItemText(i, 2, Itor->szPath);
					m_list.SetItemText(i, 3, szCompany);

					m_list.SetItemData(i, nCnt);
					
					if (Itor->nStatus == eOK)
					{
						m_list.SetCheck(i);
					}
				}
			}
			else
			{
				SHFILEINFO sfi;
				DWORD_PTR nRet;
				ZeroMemory(&sfi, sizeof(SHFILEINFO));
				nRet = SHGetFileInfo(Itor->szPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
				nRet ? m_ProImageList.Add ( sfi.hIcon ) : m_ProImageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
				if (nRet)
				{
					DestroyIcon(sfi.hIcon);
				}

				int i = m_list.GetItemCount();
				m_list.InsertItem(i, Itor->szName, i);
				m_list.SetItemText(i, 1, m_Function.GetFileDescription(Itor->szPath));
				m_list.SetItemText(i, 2, Itor->szPath);
				m_list.SetItemText(i, 3, szCompany);

				m_list.SetItemData(i, nCnt);

				if (Itor->nStatus == eOK)
				{
					m_list.SetCheck(i);
				}
			}
		}
	}

	m_bStartCheck = TRUE;
}

#define IS_DRIVER	1
#define IS_SERVICE	2

DWORD CAutoStartDlg::IsServiceOrDrivers(CString szKey)
{
	DWORD dwRet = 0;

	if (szKey.IsEmpty())
	{
		return dwRet;
	}
	
	WCHAR szType[] = {'T','y','p','e','\0'};
	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen > 0 && ir->nDataLen > 0 && !ir->szName.CompareNoCase(szType))
		{
			ULONG nType = *(ULONG*)(ir->pData);
			if (/*nType == 1 || nType == 2*/ nType & SERVICE_DRIVER)
			{
				dwRet = IS_DRIVER;
			}
			else if (/*nType == 0x10 || nType == 0x20*/ nType & SERVICE_WIN32)
			{	
				dwRet = IS_SERVICE;
			}

			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return dwRet;
}

CString CAutoStartDlg::GetServiceDll(CString szKey)
{
	CString szData = L"";

	if (szKey.IsEmpty())
	{
		return szData;
	}

	WCHAR szServiceDll[] = {'S','e','r','v','i','c','e','D','l','l','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szServiceDll) * sizeof(WCHAR) && 
			!ir->szName.CompareNoCase(szServiceDll))
		{
			szData = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szData;
}

CString CAutoStartDlg::GetServiceModule(CString szKey)
{
	CString szModule;

	if (!szKey.IsEmpty())
	{
		CString szParameters = szKey + L"\\Parameters";

		szModule = GetServiceDll(szParameters);

		if (szModule.IsEmpty())
		{
			szModule = GetImagePath(szKey);
		}
	}

	return szModule;
}

CString CAutoStartDlg::GetImagePath(CString szKey)
{
	CString szPath = L"";

	if (szKey.IsEmpty())
	{
		return szPath;
	}

	WCHAR szImagePath[] = {'I', 'm', 'a', 'g', 'e', 'P', 'a', 't', 'h', '\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szImagePath) * sizeof(WCHAR) &&
			!ir->szName.CompareNoCase(szImagePath))
		{
			szPath = m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type);
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szPath;
}

void CAutoStartDlg::EnumServiceAndDriversEx(CString szKey, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<KEY_INFO> keyLisy;
	m_Registry.EnumKeys(szKey, keyLisy);
	for (list<KEY_INFO>::iterator ir = keyLisy.begin();
		ir != keyLisy.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szName = ir->szName;
			CString szKeyPath = szKey + CString(L"\\") + szName;
			CString szPath;

			DWORD dwRet = IsServiceOrDrivers(szKeyPath);

			if ( dwRet == IS_DRIVER)
			{
				szPath = GetImagePath(szKeyPath);
				if (szPath.IsEmpty())
				{
					WCHAR szSystem[MAX_PATH] = {0};
					GetSystemDirectory(szSystem, MAX_PATH);
					szPath = szSystem + CString(L"\\drivers\\") + szName + CString(L".sys");
				}
			}
			else if (dwRet == IS_SERVICE)
			{
				szPath = GetServiceModule(szKeyPath);
			}

			if (!szPath.IsEmpty())
			{
				AddAutroRunItem( 
					dwRet == IS_DRIVER ? eDriver : eService,
					szName,
					szPath,
					szKeyPath,
					L"",
					nStatus);

				dwRet == IS_SERVICE ? m_nService++ : m_nDriver++;
			}
		}
	}
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services
void CAutoStartDlg::EnumServiceAndDrivers()
{
	m_nDriver = m_nService = 0;

	CString szKey = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\services";
	EnumServiceAndDriversEx(szKey, eOK);

	szKey += CString(L"\\") + szAntiSpyDisableAutoruns;
	EnumServiceAndDriversEx(szKey, eDisable);
}

void CAutoStartDlg::EnumAutostarts()
{
	if (!g_bLoadDriverOK || !m_bFirst)
	{
		return;
	}

	m_AutostartList.clear();
	m_list.DeleteAllItems();
	m_AutoStartType = eNone;
	
	EnumStartup();
	EnumWinlogon();
	EnumExplorer();
	EnumInternetExplorer();
	EnumServiceAndDrivers();
	EnumKnowDlls();
	EnumPrintMonitors();
	EnumNetworkProvider();
	EnumWinsockProviders();
	EnumSecurityProviders();
	EnumTasks();

	HTREEITEM hItemRoot = m_tree.GetRootItem();
	if(hItemRoot != NULL && m_tree.ItemHasChildren(hItemRoot))
	{ 
		CString szAllText;
		szAllText.Format(L"%s (%d)", szAll[g_enumLang], 
			m_nStartup + m_nExplorer + m_nInternetExplorer + 
			m_nKnowDlls + m_nNetworkProvider + m_nPrintMonitors + 
			m_nWinsockProvider + m_nSecurityProvider + m_nTask + 
			m_nWinlogon + m_nDriver + m_nService);

		m_tree.SetItemText(hItemRoot, szAllText);

		HTREEITEM hChildItem = m_tree.GetChildItem(hItemRoot);

		while(hChildItem != NULL)     
		{   
			CString szItemText = m_tree.GetItemText(hChildItem);

			if (szItemText.Find(szStartup[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szStartup[g_enumLang], m_nStartup);
				m_tree.SelectItem(hChildItem);
			}
			else if (szItemText.Find(szWinlogon[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szWinlogon[g_enumLang], m_nWinlogon);
			}
			else if (szItemText.Find(szExplorer[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szExplorer[g_enumLang], m_nExplorer);
			}
			else if (szItemText.Find(szInternetExplorer[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szInternetExplorer[g_enumLang], m_nInternetExplorer);
			}
			else if (szItemText.Find(szAutoRunServices[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szAutoRunServices[g_enumLang], m_nService);
			}
			else if (szItemText.Find(szAutoRunDrivers[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szAutoRunDrivers[g_enumLang], m_nDriver);
			}
			else if (szItemText.Find(szKnowDlls[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szKnowDlls[g_enumLang], m_nKnowDlls);
			}
			else if (szItemText.Find(szPrintMonitors[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szPrintMonitors[g_enumLang], m_nPrintMonitors);
			}
			else if (szItemText.Find(szNetworkProviders[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szNetworkProviders[g_enumLang], m_nNetworkProvider);
			}
			else if (szItemText.Find(szWinsockProviders[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szWinsockProviders[g_enumLang], m_nWinsockProvider);
			}
			else if (szItemText.Find(szSecurityProviders[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szSecurityProviders[g_enumLang], m_nSecurityProvider);
			}
			else if (szItemText.Find(szScheduledTasks[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szScheduledTasks[g_enumLang], m_nTask);
			}

			m_tree.SetItemText(hChildItem, szItemText);

			hChildItem = m_tree.GetNextItem(hChildItem, TVGN_NEXT);
		}
	}      

	ShowAutostarts();
	m_tree.SetFocus();
	m_bFirst = FALSE;
}

void CAutoStartDlg::ParsePath(WCHAR *szPath, ULONG dwType, set<wstring> &AppSet)
{
	AppSet.clear();

	if ( szPath == NULL || wcslen(szPath) <=0 )
	{
		return;
	}
	
	if ( dwType == REG_SZ || dwType == REG_EXPAND_SZ )
	{
		ParseRegistryPathFromType( (PWCHAR)szPath, NULL, dwType, AppSet );
	}
	else if ( dwType == REG_MULTI_SZ )
	{
		ParseRegistryPathFromType( NULL, (PWCHAR)szPath, dwType, AppSet );
	}
}

void CAutoStartDlg::EnumValueEx(CString szKey, AUTO_START_TYPE Type, AUTO_RUN_STATUS nStatus)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen &&  ir->nDataLen)
		{
			AddAutroRunItem(Type, 
				ir->szName, 
				m_Function.RegParsePath((WCHAR*)ir->pData, ir->Type),
				szKey,
				ir->szName,
				nStatus);

			m_nStartup++;
		}
	}

	m_Registry.FreeValueList(valueList);
}

void CAutoStartDlg::EnumValue(CString szKey, AUTO_START_TYPE Type)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	EnumValueEx(szKey, Type, eOK);

	szKey += CString("\\") + szAntiSpyDisableAutoruns;
	EnumValueEx(szKey, Type, eDisable);
}

void CAutoStartDlg::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	if (hItem)
	{
		if (hItem == m_tree.GetRootItem())
		{
			m_AutoStartType = eAll;
		}
		else
		{
			CString szItemText = m_tree.GetItemText(hItem);
			if (szItemText.Find(szStartup[g_enumLang]) != -1)
			{
				m_AutoStartType = eStartup;
			}
			else if (szItemText.Find(szWinlogon[g_enumLang]) != -1)
			{
				m_AutoStartType = eWinlogon;	
			}
			else if (szItemText.Find(szExplorer[g_enumLang]) != -1)
			{
				m_AutoStartType = eExplorer;	
			}
			else if (szItemText.Find(szInternetExplorer[g_enumLang]) != -1)
			{
				m_AutoStartType = eIE;
			}
			else if (szItemText.Find(szAutoRunDrivers[g_enumLang]) != -1)
			{
				m_AutoStartType = eSystemDriver;
			}
			else if (szItemText.Find(szAutoRunServices[g_enumLang]) != -1)
			{
				m_AutoStartType = eSystemService;
			}
			else if (szItemText.Find(szKnowDlls[g_enumLang]) != -1)
			{
				m_AutoStartType = eKnowDLLs;
			}
			else if (szItemText.Find(szPrintMonitors[g_enumLang]) != -1)
			{
				m_AutoStartType = ePrintMonitor;
			}
			else if (szItemText.Find(szNetworkProviders[g_enumLang]) != -1)
			{
				m_AutoStartType = eNetworkProvider;
			}
			else if (szItemText.Find(szWinsockProviders[g_enumLang]) != -1)
			{
				m_AutoStartType = eWinsockProvider;
			}
			else if (szItemText.Find(szSecurityProviders[g_enumLang]) != -1)
			{
				m_AutoStartType = eSecurityProvider;
			}
			else if (szItemText.Find(szScheduledTasks[g_enumLang]) != -1)
			{
				m_AutoStartType = eTask;
			}
		}
	}
	
	ShowAutostarts();

	*pResult = 0;
}

void CAutoStartDlg::OnNMRclickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
// 	CPoint point;
// 
// 	GetCursorPos(&point);
// 	m_tree.ScreenToClient(&point);
// 
// 	HTREEITEM Item = m_tree.HitTest(point, NULL);
// 
// 	if ( Item != NULL )
// 	{ 
// 		CString szItemText = m_tree.GetItemText(Item);
// 		m_tree.SelectItem(Item);
// 	}

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_AUTORUN_REFRESH, szAutorunRefesh[g_enumLang]);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	*pResult = 0;
}

void CAutoStartDlg::OnNMClickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint point;

	GetCursorPos(&point);
	m_tree.ScreenToClient(&point);

	HTREEITEM Item = m_tree.HitTest(point, NULL);

	if ( Item == NULL )
	{ 
		m_AutoStartType = eStartup;
	}

	*pResult = 0;
}

void CAutoStartDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	int nItem = pNMLV->iItem;
	if (nItem != -1 && m_list.GetSelectedCount())
	{
		ULONG nCnt = (ULONG)m_list.GetItemData(nItem);
		ULONG nType = (ULONG)(m_AutostartList[nCnt].Type);
		CString szStatus;

		switch (nType)
		{
		case eHKLM_RUN:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
			break;

		case eHKCU_RUN:
			szStatus = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
			break; 	

		case eHKLM_RUNONCE:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
			break;

		case eHKCU_RUNONCE:
			szStatus = L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
			break;

		case eHKLM_RUNONCE_SETUP:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce\\Setup";
			break;

		case eHKCU_RUNONCEEX:				
			szStatus = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx";
			break;

		case eHKLM_RUNONCEEX:
			szStatus = L"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx";
			break;

		case eHKCU_EXPLORER_RUN:
			szStatus = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run";
			break;

		case eHKLM_EXPLORER_RUN:
			szStatus = L"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run";
			break;

		case eStartupDirectory:
			szStatus = m_szStartupPath;  // L"C:\\Documents and Settings\\*\\「开始」菜单\\程序";
			break;

		case eCommonStartupDirectory:
			szStatus = m_szCommonStartupPath;
			break;

		// Winlogon相关
		case eHKCU_WINDOWS_LOAD:
			szStatus = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows, Load";
			break;

		case eHKCU_WINDOWS_RUN:
			szStatus = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows, Run";
			break;

		case eHKLM_WINLOGON_SHELL:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, Shell";
			break;
		
		case eHKCU_WINLOGON_SHELL:
			szStatus = L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, Shell";
			break;

		case eHKLM_WINLOGON_APPSETUP:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, AppSetup";
			break;

		case eHKLM_WINLOGON_GINADLL:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, GinaDLL";
			break;

		case eHKLM_WINLOGON_LSASTART:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, LsaStart";
			break;

		case eHKLM_WINLOGON_SaveDumpStart:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, SaveDumpStart";
			break;

		case eHKLM_WINLOGON_ServiceControllerStart:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, ServiceControllerStart";
			break;

		case eHKLM_WINLOGON_System:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, System";
			break;

		case eHKLM_WINLOGON_Taskman:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, Taskman";
			break;

		case eHKLM_WINLOGON_UIHost:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, UIHost";
			break;

		case eHKLM_WINLOGON_Userinit:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon, Userinit";
			break;

		case eHKLM_WINLOGON_Notify:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify";
			break;

		case eHKLM_WINLOGON_GPExtensions:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions";
			break;

// 		case eInstalledComponents:
// 			szStatus = L"HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run";
// 			break;

		case eSharedTaskScheduler:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SharedTaskScheduler";
			break;

		case eShellExecuteHooks:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks";
			break;

		case eShellServiceObjectDelayLoad:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad";
			break; 

		case eShellIconOverlayIdentifiers:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers";
			break;

		case eContextMenuHandlers:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\*\\shellex\\ContextMenuHandlers";
			break;

		case eContextMenuHandlers2:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\AllFilesystemObjects\\shellex\\ContextMenuHandlers";
			break; 

		case eContextMenuHandlers3:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Directory\\Background\\shellex\\ContextMenuHandlers";
			break;

		case eContextMenuHandlers4:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Directory\\shellex\\ContextMenuHandlers";
			break; 

		case eContextMenuHandlers5:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Drive\\shellex\\ContextMenuHandlers";
			break;

		case eContextMenuHandlers6:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Folder\\shellex\\ContextMenuHandlers";
			break;

		case eCopyHookHandlers:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Directory\\shellex\\CopyHookHandlers";
			break;

		case eDragDropHandlers:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Directory\\shellex\\DragDropHandlers";
			break;

		case eFilter:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PROTOCOLS\\Filter";
			break; 

		case eHandler:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PROTOCOLS\\Handler";
			break; 

		case eApproved:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";
			break; 

		case eBrowserHelperObjects:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects";
			break; 

		case eURLSearchHooks:
			szStatus = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\URLSearchHooks";
			break;

		case eExtensions:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer\\Extensions";
			break;

		case eKnowDlls:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs";
			break; 

		case ePrintMonitors:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors";
			break;

		case eNetworkProviders:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order, ProviderOrder";
			break;

		case eProtocolCatalog9:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries";
			break;

		case eNameSpaceCatalog5:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\NameSpace_Catalog5\\Catalog_Entries";
			break;

		case eSecurityProviders:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SecurityProviders, SecurityProviders";
			break;

		case eAuthenticationPackages:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa, Authentication Packages";
			break;

		case eNotificationPackages:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa, Notification Packages";
			break;

		case eSecurityPackages:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa, Security Packages";
			break;

		case eCredentialProviders:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers";
			break;

		case eCredentialProviderFilters:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Provider Filters";
			break;

		case ePLAPProviders:
			szStatus = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\PLAP Providers";
			break;

		case eService:
		case eDriver:
			szStatus = L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services";
			break;

		case eTaskDirectory:
			{
				WCHAR szPath[MAX_PATH] = {0};
				GetWindowsDirectory(szPath, MAX_PATH);
				szStatus = szPath;
				szStatus += L"\\tasks";
			}
			break; // c:\windows\tasks
		}

		m_szStatus = szStatus;
		UpdateData(FALSE);
	}
	else
	{
		m_szStatus = L"";
		UpdateData(FALSE);
	}

	CheckOrNoCheckItem(pNMHDR);

	*pResult = 0;
}

void CAutoStartDlg::CheckOrNoCheckItem(NMHDR *pNMHDR)
{
	if (m_bStartCheck)
	{
		LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

		if (pNMListView->uOldState == 0 && pNMListView->uNewState == 0)
			return;    // No change

		// Old check box state
		BOOL bPrevState = (BOOL)(((pNMListView->uOldState & 
			LVIS_STATEIMAGEMASK)>>12)-1);  
		if (bPrevState < 0)    // On startup there's no previous state 
			bPrevState = 0; // so assign as false (unchecked)

		// New check box state
		BOOL bChecked = (BOOL)(((pNMListView->uNewState & LVIS_STATEIMAGEMASK)>>12)-1);   
		if (bChecked < 0) // On non-checkbox notifications assume false
			bChecked = 0; 

		if (bPrevState == bChecked) // No change in check box
			return;

		// Now bChecked holds the new check box state
		int nData = (int)m_list.GetItemData(pNMListView->iItem);
		DisableOrEnableAutoruns(&m_AutostartList.at(nData), bChecked, pNMListView->iItem);
	}
}

BOOL CAutoStartDlg::DisableOrEnableAutoruns(PAUTO_START_INFO info, BOOL bEnable, int nItem)
{
	if (info->nStatus == eDelete)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	CString szKeyPath;
	CString szKey = info->szKeyPath;
	CString szValue = info->szValueName;
	CString szDisableKey;

	if (!szValue.IsEmpty() || !szKey.IsEmpty())
	{
		// 该启动信息是一个键
		if (szValue.IsEmpty() && !szKey.IsEmpty())
		{
			m_Registry.EnableOrDisableKey(szKey, bEnable);

			CString szKeyName = szKey.Right(szKey.GetLength() - szKey.ReverseFind('\\') - 1);
			if (!bEnable)
			{
				szKeyPath = szKey.Left(szKey.ReverseFind('\\'));
				szKeyPath += CString(L"\\") + szAntiSpyDisableAutoruns + CString(L"\\") + szKeyName;
			}
			else
			{
				szKeyPath = szKey.Left(szKey.Find(szAntiSpyDisableAutoruns) - 1);
				szKeyPath += CString(L"\\") + szKeyName;
			}

			szDisableKey = szKey.Left(szKey.ReverseFind('\\'));
		}

		// 启动项是一个值
		else if (!szValue.IsEmpty() && !szKey.IsEmpty())
		{
			m_Registry.EnableOrDisableValue(szKey, szValue, bEnable);

			if (!bEnable)
			{
				szKeyPath = szKey + CString(L"\\") + szAntiSpyDisableAutoruns;
			}
			else
			{
				szKeyPath = szKey.Left(szKey.Find(szAntiSpyDisableAutoruns) - 1);
			}

			szDisableKey = szKey;
		}

		// 修改原始项的键路径
		info->szKeyPath = szKeyPath;

		// 修改这一项的状态
		bEnable ? info->nStatus = eOK : info->nStatus = eDisable;

		// 如果是恢复项，那么最后判断AntiSpyDisableAutoruns键是否为空了，如果为空就删除之
		if (bEnable && !szDisableKey.IsEmpty())
		{
			list<KEY_INFO> keyList;
			m_Registry.EnumKeys(szDisableKey, keyList);
			if (keyList.size() != 0)
			{
				return bRet;
			}

			list<VALUE_INFO> valueList;
			m_Registry.EnumValues(szDisableKey, valueList);
			if (valueList.size() != 0)
			{
				m_Registry.FreeValueList(valueList);
				return bRet;
			}

			m_Registry.FreeValueList(valueList);
			m_Registry.DeleteKeyAndSubKeys(szDisableKey);
		}
	}
	
	// 键和值都是空的，那么是文件夹
	else
	{
		CString szDirectory;
		CString szFileName;
		CString szAntispyDisable;
		CString szSrc, szDes;
		if (info->Type == eStartupDirectory)
		{
			szDirectory = m_szStartupPath;
			szAntispyDisable = m_szStartupPath.Left(m_szStartupPath.ReverseFind('\\')) + CString(L"\\") + szAntiSpyDisableAutoruns;
		}
		else if (info->Type == eCommonStartupDirectory)
		{
			szDirectory = m_szCommonStartupPath;
			szAntispyDisable = m_szCommonStartupPath.Left(m_szCommonStartupPath.ReverseFind('\\')) + CString(L"\\") + szAntiSpyDisableAutoruns;
		}
		else if (info->Type == eTaskDirectory)
		{
			WCHAR szPath[MAX_PATH] = {0};
			GetWindowsDirectory(szPath, MAX_PATH);
			szDirectory = szPath + CString(L"\\tasks");
			szFileName = info->szName + CString(L".job");
			szAntispyDisable = szPath + CString(L"\\") + szAntiSpyDisableAutoruns;
		}

		if (bEnable)
		{
			szSrc = szAntispyDisable;
			szDes = szDirectory;
		}
		else
		{
			szSrc = szDirectory;
			szDes = szAntispyDisable;
		}

		if (szFileName.IsEmpty())
		{
			szFileName = GetDirectoryFileName(szSrc, info->szName);
		}

		if (!szFileName.IsEmpty() &&
			!szSrc.IsEmpty() &&
			!szDes.IsEmpty())
		{
			if (CreateDirectory(szDes, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
			{
				MoveFile(szSrc + CString(L"\\") + szFileName, szDes + CString(L"\\") + szFileName);
			}
		}
	}

	return bRet;
}

CString CAutoStartDlg::GetDirectoryFileName(CString szPath, CString szTtileName)
{
	CString szRet = L"";
	if (szTtileName.IsEmpty() || szPath.IsEmpty())
	{
		return szRet;
	}

	CFileFind cff;
	if (szPath.Right(1) != L'\\')
	{
		szPath += L"\\";
	}

	szPath += L"*.*";

	BOOL bRet = cff.FindFile(szPath);
	while (bRet)
	{
		bRet = cff.FindNextFile();
		if (!cff.IsDirectory() && !cff.IsDots() && (cff.GetFileName()).CompareNoCase(L"desktop.ini"))
		{
			CString szName = cff.GetFileTitle();
			if (!szName.CompareNoCase(szTtileName))
			{
				szRet = cff.GetFileName();
				break;
			}
		}
	}

	cff.Close();
	return szRet;
}

void CAutoStartDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_AUTORUN_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_AUTORUN_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_AUTORUN_JMP_TO_REG, szJmpToReg[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_LOCATION_EXPLORER, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_HIDE_MICROSOFT_MODULE, szHideMicrosofeModules[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_DISABLE, szDisableStartup[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_ENABLE, szEnableStartup[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_DEL_REG, szDeleteStartup[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_DEL_REG_AND_FILE, szDeleteStartupAndFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);

	menu.AppendMenu(MF_STRING, ID_AUTORUN_COPY_NAME, szCopyStartupName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_COPY_PATH, szCopyStartupPath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);

	menu.AppendMenu(MF_STRING, ID_AUTORUN_SEARCH_ONLINE, szStartupSearchOnline[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_CHECK_ATTRIBUTE, szCallbackModuleProperties[g_enumLang]);

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_CHECK_SIGN, szVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_AUTORUN_CHECK_ALL_SIGN, szVerifyAllModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	menu.SetDefaultItem(ID_AUTORUN_JMP_TO_REG);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_AUTORUN_JMP_TO_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(ID_AUTORUN_LOCATION_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_AUTORUN_DEL_REG, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_AUTORUN_DEL_REG_AND_FILE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_AUTORUN_COPY_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_AUTORUN_COPY_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_AUTORUN_SEARCH_ONLINE, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_AUTORUN_CHECK_ATTRIBUTE, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_AUTORUN_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_AUTORUN_CHECK_ALL_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(21, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 0; i < 22; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		int nSelCnt = m_list.GetSelectedCount();
		if (nSelCnt == 0)
		{
			for (int i = 0; i < 21; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (nSelCnt == 1)
		{
			int nItem = m_Function.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				ULONG nCnt = (ULONG)m_list.GetItemData(nItem);
				ULONG dwType = (ULONG)(m_AutostartList[nCnt].Type);
				if (dwType == eStartupDirectory ||
					dwType == eTaskDirectory ||
					dwType == eCommonStartupDirectory)
				{
					// 如果是文件夹类启动项，那么就把定位到注册表和导出启动信息置灰
					menu.EnableMenuItem(ID_AUTORUN_JMP_TO_REG, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_AUTORUN_EXPORT_REG_INFO, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				if (m_list.GetCheck(nItem))
				{
					menu.EnableMenuItem(ID_AUTORUN_ENABLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
				else
				{
					menu.EnableMenuItem(ID_AUTORUN_DISABLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}
		}
		else if (nSelCnt > 1)
		{
			for (int i = 0; i < 21; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_AUTORUN_DEL_REG, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_AUTORUN_DEL_REG_AND_FILE, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_AUTORUN_CHECK_ALL_SIGN, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_AUTORUN_ENABLE, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_AUTORUN_DISABLE, MF_BYCOMMAND | MF_ENABLED);
		}

		menu.EnableMenuItem(ID_AUTORUN_CHECK_ALL_SIGN, MF_BYCOMMAND | MF_ENABLED);
	}

	// 隐藏微软模块选项什么时候都是可以选择的
	menu.EnableMenuItem(ID_AUTORUN_HIDE_MICROSOFT_MODULE, MF_BYCOMMAND | MF_ENABLED);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CAutoStartDlg::OnAutorunJmpToReg()
{
	if (!m_szStatus.IsEmpty() && g_pRegistryDlg && g_pTab)
	{
		int nItem = m_Function.GetSelectItem(&m_list);
		if (nItem != - 1)
		{
			ULONG nCnt = (ULONG)m_list.GetItemData(nItem);
			m_Function.JmpToMyRegistry(m_AutostartList[nCnt].szKeyPath, m_AutostartList[nCnt].szValueName);	
		}
	}
}

void CAutoStartDlg::OnAutorunLocationExplorer()
{
	int n = m_Function.GetSelectItem(&m_list);
	if (n != -1)
	{
		CString szName = m_list.GetItemText(n, 2);
		if (!szName.IsEmpty())
		{
			m_Function.LocationExplorer(szName);
		}
	}
}

void CAutoStartDlg::DelReg()
{
	if (!m_szStatus.IsEmpty() && g_pRegistryDlg && g_pTab)
	{
// 		int nItem = m_Function.GetSelectItem(&m_list);
// 		if (nItem != - 1)
// 		{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = m_list.GetNextSelectedItem(pos);

			ULONG nCnt = (ULONG)m_list.GetItemData(nItem);
			CString szKeyPath = m_AutostartList[nCnt].szKeyPath;
			CString szValueName = m_AutostartList[nCnt].szValueName;
			AUTO_START_TYPE Type = m_AutostartList[nCnt].Type;
			
			m_AutostartList.at(nCnt).nStatus = eDelete;

			if (!szKeyPath.IsEmpty() && szValueName.IsEmpty())
			{
				m_Registry.DeleteKeyAndSubKeys(szKeyPath);
			}
			else if (!szKeyPath.IsEmpty() && !szValueName.IsEmpty())
			{
				m_Registry.DeleteValue(szKeyPath, szValueName);
			}
			else if (szKeyPath.IsEmpty() && szValueName.IsEmpty())
			{
				if (Type == eTaskDirectory)
				{
					CString szJob = m_szStatus;
					szJob += L"\\";
					szJob += m_list.GetItemText(nItem, 0);
					szJob += L".job";
					DeleteFile(szJob);
				}
				else if (Type == eStartupDirectory || Type == eCommonStartupDirectory)
				{
					CString szDir;

					if (Type == eStartupDirectory)
					{
						szDir = m_szStartupPath;
					}
					else if (Type == eCommonStartupDirectory)
					{
						szDir = m_szCommonStartupPath;
					}
					
					CFileFind cff;
					if (szDir.Right(1) != L'\\')
					{
						szDir += L"\\";
					}

					szDir += L"*.*";

					BOOL bRet = cff.FindFile(szDir);
					while (bRet)
					{
						bRet = cff.FindNextFile();
						if (!cff.IsDirectory() && !cff.IsDots() && (cff.GetFileName()).CompareNoCase(L"desktop.ini"))
						{
							CString szName = cff.GetFileTitle();
							if (!szName.CompareNoCase(m_list.GetItemText(nItem, 0)))
							{
								DeleteFile(cff.GetFilePath());
								break;
							}
						}
					}

					cff.Close();
				}
			}

			m_list.DeleteItem(nItem);
			pos = m_list.GetFirstSelectedItemPosition();
			UpdateCnt(Type);
		}

		UpdateStatus();
	}
}

void CAutoStartDlg::UpdateCnt(AUTO_START_TYPE Type)
{
	switch (Type)
	{
	case eHKLM_RUN:
	case eHKCU_RUN:
	case eHKLM_RUNONCE:
	case eHKCU_RUNONCE:
	case eHKLM_RUNONCE_SETUP:
	case eHKCU_RUNONCEEX:
	case eHKLM_RUNONCEEX:
	case eHKCU_EXPLORER_RUN:
	case eHKLM_EXPLORER_RUN:
	case eStartupDirectory:
	case eCommonStartupDirectory:
		m_nStartup--;
		break;

	case eHKCU_WINDOWS_LOAD:
	case eHKCU_WINDOWS_RUN:
	case eHKCU_WINLOGON_SHELL:
	case eHKLM_WINLOGON_SHELL:
	case eHKLM_WINLOGON_APPSETUP:
	case eHKLM_WINLOGON_GINADLL:
	case eHKLM_WINLOGON_LSASTART:
	case eHKLM_WINLOGON_SaveDumpStart:
	case eHKLM_WINLOGON_ServiceControllerStart:
	case eHKLM_WINLOGON_System:
	case eHKLM_WINLOGON_Taskman:
	case eHKLM_WINLOGON_UIHost:
	case eHKLM_WINLOGON_Userinit:
	case eHKLM_WINLOGON_GPExtensions:
	case eHKLM_WINLOGON_Notify:
		m_nWinlogon--;
		break;

	case eSharedTaskScheduler:
	case eShellExecuteHooks:
	case eShellServiceObjectDelayLoad:
	case eShellIconOverlayIdentifiers:
	case eContextMenuHandlers:
	case eContextMenuHandlers2:
	case  eContextMenuHandlers3:
	case eContextMenuHandlers4:
	case eContextMenuHandlers5:
	case eContextMenuHandlers6:
	case eCopyHookHandlers:
	case eDragDropHandlers:
	case eFilter:
	case eHandler:
	case eApproved:
		m_nExplorer--;
		break;

	case eBrowserHelperObjects:
	case eURLSearchHooks:
	case eExtensions:
		m_nInternetExplorer--;
		break;

	case eKnowDlls:
		m_nKnowDlls--;
		break;

	case ePrintMonitors:
		m_nPrintMonitors--;
		break;

	case eNetworkProviders:
		m_nNetworkProvider--;
		break;

	case eProtocolCatalog9:
	case eNameSpaceCatalog5:
		m_nWinsockProvider--;
		break;

	case eSecurityProviders:
	case eAuthenticationPackages:
	case eNotificationPackages:
	case eSecurityPackages:
	case eCredentialProviders:
	case eCredentialProviderFilters:
	case ePLAPProviders:
		m_nSecurityProvider--;
		break;

	case eTaskDirectory:
		m_nTask--;
		break;

	case eService:
		m_nService--;
		break;

	case eDriver:
		m_nDriver--;
		break;
	}
}

// 删除一项之后刷新树形控件的状态
void CAutoStartDlg::UpdateStatus()
{
	HTREEITEM hItemRoot = m_tree.GetRootItem();
	if(hItemRoot != NULL && m_tree.ItemHasChildren(hItemRoot))
	{ 
		CString szAllText;
		szAllText.Format(L"%s (%d)", szAll[g_enumLang], 
			m_nStartup + m_nExplorer + m_nInternetExplorer + 
			m_nKnowDlls + m_nNetworkProvider + m_nPrintMonitors + 
			m_nWinsockProvider + m_nSecurityProvider + m_nTask + 
			m_nWinlogon + m_nDriver + m_nService);

		m_tree.SetItemText(hItemRoot, szAllText);

		HTREEITEM hChildItem = m_tree.GetChildItem(hItemRoot);

		while(hChildItem != NULL)     
		{   
			CString szItemText = m_tree.GetItemText(hChildItem);

			if (szItemText.Find(szStartup[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szStartup[g_enumLang], m_nStartup);
			}
			else if (szItemText.Find(szWinlogon[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szWinlogon[g_enumLang], m_nWinlogon);
			}
			else if (szItemText.Find(szExplorer[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szExplorer[g_enumLang], m_nExplorer);
			}
			else if (szItemText.Find(szInternetExplorer[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szInternetExplorer[g_enumLang], m_nInternetExplorer);
			}
			else if (szItemText.Find(szAutoRunServices[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szAutoRunServices[g_enumLang], m_nService);
			}
			else if (szItemText.Find(szAutoRunDrivers[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szAutoRunDrivers[g_enumLang], m_nDriver);
			}
			else if (szItemText.Find(szKnowDlls[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szKnowDlls[g_enumLang], m_nKnowDlls);
			}
			else if (szItemText.Find(szPrintMonitors[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szPrintMonitors[g_enumLang], m_nPrintMonitors);
			}
			else if (szItemText.Find(szNetworkProviders[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szNetworkProviders[g_enumLang], m_nNetworkProvider);
			}
			else if (szItemText.Find(szWinsockProviders[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szWinsockProviders[g_enumLang], m_nWinsockProvider);
			}
			else if (szItemText.Find(szSecurityProviders[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szSecurityProviders[g_enumLang], m_nSecurityProvider);
			}
			else if (szItemText.Find(szScheduledTasks[g_enumLang]) != -1)
			{
				szItemText.Format(L"%s (%d)", szScheduledTasks[g_enumLang], m_nTask);
			}

			m_tree.SetItemText(hChildItem, szItemText);

			hChildItem = m_tree.GetNextItem(hChildItem, TVGN_NEXT);
		}
	}      
}

void CAutoStartDlg::OnAutorunDelReg()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szAutostart[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	
	DelReg();
}

void CAutoStartDlg::DeleteFile(CString szFilePath)
{
	/*::DeleteFile(szFilePath);*/
	if (m_Function.KernelDeleteFile(szFilePath))
	{
	//	MessageBox(szDeleteFileSucess[g_enumLang], NULL, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
	//	MessageBox(szDeleteFileFailed[g_enumLang], NULL, MB_OK | MB_ICONERROR);
	}
}

void CAutoStartDlg::OnAutorunDelRegAndFile()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szAutostart[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

// 	int nItem = m_Function.GetSelectItem(&m_list);
// 	if (nItem != -1)
// 	{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);

		CString szPath = m_list.GetItemText(nItem, 2);
		BOOL bRet = m_Function.KernelDeleteFile(szPath);
		Sleep(200);
		
// 		if (bRet)
// 		{
// 			MessageBox(szDeleteFileSucess[g_enumLang], NULL, MB_OK | MB_ICONINFORMATION);
// 		}
// 		else
// 		{
// 			MessageBox(szDeleteFileFailed[g_enumLang], NULL, MB_OK | MB_ICONERROR);
// 		}
	}

	DelReg();
}

void CAutoStartDlg::OnAutorunCopyName()
{
	int n = m_Function.GetSelectItem(&m_list);
	if (n != -1)
	{
		CString szName = m_list.GetItemText(n, 0);
		if (!szName.IsEmpty())
		{
			m_Function.SetStringToClipboard(szName);
		}
	}
}

void CAutoStartDlg::OnAutorunCopyPath()
{
	int n = m_Function.GetSelectItem(&m_list);
	if (n != -1)
	{
		CString szName = m_list.GetItemText(n, 2);
		if (!szName.IsEmpty())
		{
			m_Function.SetStringToClipboard(szName);
		}
	}
}

void CAutoStartDlg::OnAutorunSearchOnline()
{
	int n = m_Function.GetSelectItem(&m_list);
	if (n != -1)
	{
		CString szName = m_list.GetItemText(n, 0);
		if (!szName.IsEmpty())
		{
			m_Function.SearchOnline(szName);
		}
	}
}

void CAutoStartDlg::OnAutorunCheckAttribute()
{
	int n = m_Function.GetSelectItem(&m_list);
	if (n != -1)
	{
		CString szPath = m_list.GetItemText(n, 2);
		if (!szPath.IsEmpty())
		{
			m_Function.OnCheckAttribute(szPath);
		}
	}
}

void CAutoStartDlg::OnAutorunCheckSign()
{
	int n = m_Function.GetSelectItem(&m_list);
	if (n != -1)
	{
		CString szPath = m_list.GetItemText(n, 2);
		if (!szPath.IsEmpty())
		{
			m_Function.SignVerify(szPath);
		}
	}
}

void CAutoStartDlg::OnAutorunCheckAllSign()
{
	m_SignVerifyDlg.m_NotSignDataList.clear();
	m_SignVerifyDlg.m_NotSignItemList.clear();
	m_SignVerifyDlg.m_bSingle = FALSE;
	m_SignVerifyDlg.m_pList = &m_list;
	m_SignVerifyDlg.m_nPathSubItem = 2;
	m_SignVerifyDlg.m_hWinWnd = this->m_hWnd;
	m_SignVerifyDlg.DoModal();
}

void CAutoStartDlg::OnAutorunText()
{
	HTREEITEM hTreeItem = m_tree.GetSelectedItem();
	if (hTreeItem != NULL )
	{
		CString szString = m_tree.GetItemText(hTreeItem);
		m_Function.ExportListToTxt(&m_list, szString);
	}
}

void CAutoStartDlg::OnAutorunExcel()
{
	HTREEITEM hTreeItem = m_tree.GetSelectedItem();
	if (hTreeItem != NULL )
	{
		WCHAR szAutoRun[] = {'A','u','t','o','R','u','n','\0'};
		CString szString = m_tree.GetItemText(hTreeItem);
		m_Function.ExportListToExcel(&m_list, szAutoRun, szString);
	}
}

BOOL CAutoStartDlg::IsNotSignItemData(ULONG nItem)
{
	BOOL bRet = FALSE;

	if (!m_SignVerifyDlg.m_NotSignDataList.empty())
	{
		list<ULONG>::iterator findItr;
		findItr = find(m_SignVerifyDlg.m_NotSignDataList.begin(), m_SignVerifyDlg.m_NotSignDataList.end(), nItem);
		if (findItr != m_SignVerifyDlg.m_NotSignDataList.end())
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

int CALLBACK AutoRunCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{ 
	return lParam1 < lParam2;
}

BOOL CAutoStartDlg::IsNotSignItem(ULONG nItem)
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

VOID CAutoStartDlg::SortItem()
{
	ULONG nCnt = m_list.GetItemCount();

	for (ULONG i = 0; i < nCnt; i++)
	{
		if (IsNotSignItem(i))
		{
			// 设置是否Vrified
			CString szComp = m_list.GetItemText(i, 3);
			CString szCompTemp = szNotVerified[g_enumLang] + szComp;
			m_list.SetItemText(i, 3, szCompTemp);
		}
		else
		{
			// 设置是否Vrified
			CString szComp = m_list.GetItemText(i, 3);
			CString szCompTemp = szVerified[g_enumLang] + szComp;
			m_list.SetItemText(i, 3, szCompTemp);
		}
	}
}

LRESULT CAutoStartDlg::OnCloseWindow(WPARAM wParam, LPARAM lParam)
{
	SortItem();
	return 0;
}

void CAutoStartDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		clrNewTextColor = g_MicrosoftItemClr;//RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );	
		
		CString szCompane = m_list.GetItemText(nItem, 3);
		if (IsNotSignItemData((ULONG)m_list.GetItemData(nItem)))
		{
			clrNewTextColor = g_NotSignedItemClr;//RGB( 180, 0, 255 );
		}
		else if (!m_Function.IsMicrosoftApp(szCompane))
		{
			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CAutoStartDlg::OnAutorunHideMicrosoftModule()
{
	m_bHideMicrosoft = !m_bHideMicrosoft;
	ShowAutostarts();
}

void CAutoStartDlg::OnUpdateAutorunHideMicrosoftModule(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHideMicrosoft);
}

void CAutoStartDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

void CAutoStartDlg::ShellExportRegister(
									   CString strItem,		//注册表项，如HKEY_CURRENT_USER\SofteWare
									   CString strFileName)	//导出的文件名（包括路径）
{
	CString strParameters = L"/e \"" + strFileName + L"\" \"" + strItem + L"\"";
	ShellExecute(0, L"open", L"regedit.exe", strParameters, NULL, SW_SHOWNORMAL);
}

void CAutoStartDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnAutorunJmpToReg();
	*pResult = 0;
}

BOOL CAutoStartDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CAutoStartDlg::OnProcessLocationAtFileManager()
{
	int n = m_Function.GetSelectItem(&m_list);
	if (n != -1)
	{
		CString szPath = m_list.GetItemText(n, 2);
		if (!szPath.IsEmpty())
		{
			m_Function.JmpToFile(szPath);
		}
	}
}

void CAutoStartDlg::OnAutorunRefresh()
{
	m_bFirst = TRUE;
	EnumAutostarts();
	m_bFirst = FALSE;

	switch (m_AutoStartType)
	{
	case eNone:
	case eStartup:  
		break;
	case eWinlogon:  
		break;
	case eExplorer:  
		break;
	case eIE:  
		break;
	case eSystemService:  
		break;
	case eSystemDriver:  
		break;
	case eKnowDLLs:  
		break;
	case ePrintMonitor:  
		break;
	case eNetworkProvider:  
		break;
	case eWinsockProvider:  
		break;
	case eSecurityProvider:  
		break;
	case eTask:  
		break;
	case eAll:  
		break;
	}
}

void CAutoStartDlg::OnAutorunDisable()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1 && m_list.GetCheck(nItem))
		{
			m_list.SetCheck(nItem, FALSE);
		}
	}
}

void CAutoStartDlg::OnAutorunEnable()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		if (nItem != -1 && !m_list.GetCheck(nItem))
		{
			m_list.SetCheck(nItem, TRUE);
		}
	}
}
