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
// SpyHunterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "SpyHunterDlg.h"
#include "..\\..\\Common\Common.h"
#include <winioctl.h>
#include <shlwapi.h>
#include "HookFunc.h"
#include "afxwin.h"
#include <afxinet.h>

WCHAR g_szPeVersion[100] = {'2','.','0','\0'};
CDialog *g_pDialog[20];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedHookTest();
	virtual BOOL OnInitDialog();
	CXTPHyperLink m_EmailHypeLink;
	CXTPHyperLink m_BlogHypeLink;
	CXTPHyperLink m_SinaHypeLink;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMAIL, m_EmailHypeLink);
	DDX_Control(pDX, IDC_WEBSITE, m_BlogHypeLink);
	DDX_Control(pDX, IDC_MICROBOLG, m_SinaHypeLink);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
//	ON_BN_CLICKED(IDC_HOOK_TEST, &CAboutDlg::OnBnClickedHookTest)
END_MESSAGE_MAP()


// CSpyHunterDlg 对话框

DWORD WINAPI CheckUpdateThreadProc(
						LPVOID lpParameter
						)
{
	CSpyHunterDlg *pDlg = (CSpyHunterDlg *)lpParameter;
	if (pDlg)
	{
		pDlg->CheckForUpdates();
	}

	return 0;
}

DWORD WINAPI GetLdrpHashTableThreadProc(
								   LPVOID lpParameter
								   )
{
	CSpyHunterDlg *pDlg = (CSpyHunterDlg *)lpParameter;
	if (pDlg)
	{
		g_pLdrpHashTable = pDlg->GetLdrpHashTable();
		DebugLog(L"LdrpHashTable: 0x%08X\n", g_pLdrpHashTable);
	}

	return 0;
}


CSpyHunterDlg::CSpyHunterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpyHunterDlg::IDD, pParent)
{
	m_nCurSel = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bSelfProtection = FALSE;
}

void CSpyHunterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_MAIN, m_tab);
}

BEGIN_MESSAGE_MAP(CSpyHunterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CSpyHunterDlg::OnBnClickedOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CSpyHunterDlg::OnTcnSelchangeTabMain)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CSpyHunterDlg)
	EASYSIZE(IDC_TAB_MAIN, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CSpyHunterDlg 消息处理程序

inline CString ms2ws(LPCSTR szSrc, int cbMultiChar=-1)
{
	CString strDst;
	if ( szSrc==NULL || cbMultiChar==0 ){
		return strDst;
	}

	WCHAR*pBuff=NULL;
	int nLen=MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,NULL,0);
	if ( nLen>0 ){
		pBuff=new WCHAR[nLen+1];
		if ( pBuff ){
			MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,pBuff,nLen);
			pBuff[nLen]=0;
			strDst = pBuff;
			delete[] pBuff;
		}
	}

	return strDst;
}

BOOL CSpyHunterDlg::CheckForUpdates()
{
	BOOL bRet = FALSE;
	WCHAR szUrlChinese[] = {'h','t','t','p',':','/','/','m','z','f','2','0','0','8','.','b','l','o','g','.','1','6','3','.','c','o','m','/','b','l','o','g','/','s','t','a','t','i','c','/','3','5','5','9','9','7','8','6','2','0','1','1','4','1','8','7','2','6','8','5','0','2','/','\0'};
	CStringA strline;
	CString szSource;
	CHttpFile* myHttpFile = NULL;
	CInternetSession mySession(NULL, 0);

	CString szVersion = L"";

	try
	{
		myHttpFile = (CHttpFile*)mySession.OpenURL(szUrlChinese);
		if (myHttpFile)
		{
			BOOL bStart = FALSE;
			CHAR szEnd[] = {'n','b','w','-','b','l','o','g','-','e','n','d','\0'};
			CHAR szStart[] = {'n','b','w','-','b','l','o','g','-','s','t','a','r','t','\0'};

			while(myHttpFile->ReadString((CString&)strline))
			{
				if (strline.Find(szEnd) != -1)
				{
					bStart = FALSE;
					break;
				}

				if (bStart)
				{
					szSource += ms2ws(strline.GetBuffer());
				}

				if (strline.Find(szStart) != -1)
				{
					bStart = TRUE;
				}
			}

			myHttpFile->Close();
			mySession.Close();
		}
	}
	catch(CInternetException *IE)
	{
		if (myHttpFile)
		{
			myHttpFile->Close();
		}
		IE->Delete();
		szVersion = L"";
	}

	if (!szSource.IsEmpty())
	{
		WCHAR Version[] = {'v','e','r','s','i','o','n',':','\0'};	// version:
		WCHAR UNDIV[] = {'<','/','\0'};								// </

		// 获取版本信息
		int nDiv = -1;
		int nPos = szSource.Find(Version);
		if (nPos != -1)
		{
			nPos += wcslen(Version);
			nDiv = szSource.Find(UNDIV, nPos);
			if (nDiv != -1 && nDiv > nPos)
			{
				szVersion = szSource.Mid(nPos, nDiv-nPos);
			}
		}

		if (!szVersion.IsEmpty() && szVersion.CompareNoCase(g_szVersion)) 
		{
			bRet = TRUE;
		}
	}

	if (bRet)
	{
		if (MessageBox(szFindANewVersion[g_enumLang], szToolName, MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{
			WCHAR szOpen[] = {'o','p','e','n','\0'};
			if (g_enumLang == enumEnglish)
			{
				WCHAR szGoogleCode[] = {'h','t','t','p',':','/','/','c','o','d','e','.','g','o','o','g','l','e','.','c','o','m','/','p','/','a','n','t','i','s','p','y','/','d','o','w','n','l','o','a','d','s','/','l','i','s','t','\0'};
				ShellExecuteW(NULL, szOpen, szGoogleCode, NULL, NULL, SW_SHOW);
			}
			else
			{
				WCHAR szAntiSpyUrl[] = {'h','t','t','p',':','/','/','w','w','w','.','A','n','t','i','S','p','y','.','c','n','\0'};
				ShellExecuteW(NULL, szOpen, szAntiSpyUrl, NULL, NULL, SW_SHOW);
			}
		}
	}

	return bRet;
}

//LdrpHashTable
LIST_ENTRY *CSpyHunterDlg::GetLdrpHashTable()
{
	CHAR szNtdll[] = {'n','t','d','l','l','.','d','l','l','\0'};
	HANDLE hModule = GetModuleHandleA(szNtdll);
	BYTE *p = NULL;
	LIST_ENTRY *retval = NULL;
	CONST BYTE *pSign = NULL;
	CONST BYTE *pSign2 = NULL;
	DWORD SignLen = 0, SignLen2 = 0;
	DWORD dwVersion = 0, dwMajorVersion = 0, dwMinorVersion = 0;

	dwVersion = GetVersion();
	dwMajorVersion = LOBYTE(LOWORD(dwVersion));
	dwMinorVersion = HIBYTE(LOWORD(dwVersion));
	if(dwMajorVersion == 5 && dwMinorVersion == 0)//2k
	{
		pSign=(CONST BYTE *)"\x89\x01\x89\x56\x40\x89\x0a\x89\x48\x04\x8b\x4f\x10\x8d\x47\x0c";
		SignLen=16;
	}
	else if( (dwMajorVersion==5 && dwMinorVersion==1) ||
		(dwMajorVersion==6 && dwMinorVersion==1))//xp
	{
		pSign=(CONST BYTE *)"\x8B\x48\x04\x89\x07\x89\x4F\x04\x89\x39\x89";
		SignLen=11;
	}
	else if(dwMajorVersion==6 && dwMinorVersion==0)//vista
	{
		pSign=(CONST BYTE *)"\x89\x45\x88\x8b\x18\x3b\xd8";
		SignLen=7;
	}
// 	else if(dwMajorVersion==6 && dwMinorVersion==1)//win7
// 	{
// 		pSign=(CONST BYTE *)"\x8b\x33\x0f\x85\x8c\x94\x01";
// 		SignLen=7;
// 
// 		pSign2 = (CONST BYTE *)"\x8b\x33\x0f\x85\x6c\x94\x01";
// 		SignLen2 = 7;
// 	}

	__try
	{
		for(DWORD i = 0; i < 0x70000; i++)
		{
			if(memcmp((BYTE *)hModule+i, pSign, SignLen)==0)
			{
				p = (BYTE *)hModule + i - 4;
				retval = (LIST_ENTRY *)(*(DWORD *)p);

				if((ULONG)retval>(ULONG)hModule && (ULONG)retval<0x80000000)
				{
					break;
				}
				else
				{
					retval = NULL;
				}
			}
		}

		if (retval == NULL && pSign2 && SignLen2 > 0)
		{
			for(DWORD i = 0; i < 0x70000; i++)
			{
				if (pSign2 && SignLen2 && memcmp((BYTE *)hModule+i, pSign2, SignLen2) == 0)
				{
					p=(BYTE *)hModule+i-4;
					retval=(LIST_ENTRY *)(*(DWORD *)p);
					if((ULONG)retval>(ULONG)hModule && (ULONG)retval<0x80000000)
						break;
					else
						retval=NULL;
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		retval=NULL;
	}

	return retval;
}

void CSpyHunterDlg::InitGlobalFunctions()
{
	WCHAR szntdll[] = {'n','t','d','l','l','.','d','l','l','\0'};

	char szNtQueryInformationProcess[] = {'N','t','Q','u','e','r','y','I','n','f','o','r','m','a','t','i','o','n','P','r','o','c','e','s','s','\0'};
	NtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress( GetModuleHandle(szntdll), szNtQueryInformationProcess);
	DebugLog(L"NtQueryInformationProcess: 0x%08X", NtQueryInformationProcess);
}

BOOL CSpyHunterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//////////////////////////////////////////////////////////////////

//	SetResize(IDC_TAB_MAIN,  XTP_ANCHOR_TOPLEFT,  XTP_ANCHOR_BOTTOMRIGHT);

	g_pTab = &m_tab;

// 	获取系统语言
// 	g_enumLang = GetLanguageID();
	
	InitGlobalFunctions();

	g_pMainDlg = this;

	m_bmExplorer.LoadBitmap(IDB_EXPLORER);
	m_bmRefresh.LoadBitmap(IDB_REFRESH);
	m_bmDelete.LoadBitmap(IDB_DELETE);
	m_bmSearch.LoadBitmap(IDB_SEARCH);
	m_bmExport.LoadBitmap(IDB_EXPORT);
	m_bmShuxing.LoadBitmap(IDB_SHUXING);
	m_bmCopy.LoadBitmap(IDB_COPY);
	m_bmAntiSpy.LoadBitmap(IDB_ANTISPY);
	m_bmDetalInof.LoadBitmap(IDB_DETAL_INFO);
	m_bmLookFor.LoadBitmap(IDB_LOOKFOR);
	m_bmWindbg.LoadBitmap(IDB_WINDBG);
	m_bmSign.LoadBitmap(IDB_SIGN);
	m_bmReg.LoadBitmap(IDB_REG);
	m_bmRecover.LoadBitmap(IDB_RECOVER);
	m_bmOD.LoadBitmap(IDB_OD);

	if (IsWow64())
	{
		WCHAR szAntiSpy[] = {'A','n','t','i','S','p','y','\0'};
		// 如果是64位程序
		MessageBox(szCannotSupportX64[g_enumLang], szAntiSpy, MB_OK | MB_ICONWARNING);
	}
	else
	{
		// 获取Windows版本
		g_WinVersion = GetWindowsVersion();
		if (g_WinVersion != enumWINDOWS_UNKNOW)
		{
			// 提升权限
			if (!EnableDebugPrivilege(TRUE) || !EnableLoadDricerPrivilege(TRUE))
			{
				DebugLog(L"Adjust Privilege error");
			}

			g_bLoadDriverOK = StartDriver();
			if (!g_bLoadDriverOK)
			{
				MessageBox(szLoadDirverError[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
			}

			EnableLoadDricerPrivilege(FALSE);
		}
		else
		{
			WCHAR szEmail[] = {'m','i','n','z','h','e','n','f','e','i','@','1','6','3','.','c','o','m','\0'};
			MessageBox(szUnsupportedOS[g_enumLang], szEmail, MB_OK | MB_ICONINFORMATION);
		}
	}
	
	// 如果驱动加载成功，那么就获取LdrpHashTable的地址
	if (g_bLoadDriverOK)
	{
		DWORD dwTid = 0;
		HANDLE hThread = CreateThread(NULL, 0, GetLdrpHashTableThreadProc, this, 0, &dwTid);
		if (hThread)
		{
			CloseHandle(hThread);
		}

// 		g_pLdrpHashTable = GetLdrpHashTable();
// 		DebugLog(L"LdrpHashTable: 0x%08X\n", g_pLdrpHashTable);
	}
	
	// 如果驱动加载成功，那么就往驱动中下传PID，并且开启自我保护神马的
	if (g_bLoadDriverOK)
	{
		COMMUNICATE_SELF_PROTECT cm;
		cm.OpType = enumSetPid;
		cm.op.SetPid.nPid = GetCurrentProcessId();
		BOOL bRet = m_Driver.CommunicateDriver(&cm, sizeof(COMMUNICATE_SELF_PROTECT), NULL, 0, NULL);

		m_bSelfProtection = g_Config.GetSelfProtection();
		if (bRet && m_bSelfProtection)
		{
			// 下传pid成功，那么就准备开启自我保护	
			cm.OpType = enumStartSelfProtect;
			cm.op.StartOrStop.nMask = SELF_PROTECT_SSDT | SELF_PROTECT_SHADOW;
			cm.op.StartOrStop.bStart = TRUE;
			m_Driver.CommunicateDriver(&cm, sizeof(COMMUNICATE_SELF_PROTECT), NULL, 0, NULL);
		}
	}

	m_tab.InsertItem(0, szProcess[g_enumLang]);
	m_tab.InsertItem(1, szKernelModule[g_enumLang]);
//	m_tab.InsertItem(2, szRing0Hook[g_enumLang]);
	m_tab.InsertItem(2, szHooks[g_enumLang]);
	m_tab.InsertItem(3, szKernelMode[g_enumLang]);
// 	m_tab.InsertItem(4, szUserMode[g_enumLang]);
	m_tab.InsertItem(4, szRegistry[g_enumLang]);
	m_tab.InsertItem(5, szFile[g_enumLang]);
	m_tab.InsertItem(6, szService[g_enumLang]);
	m_tab.InsertItem(7, szAutostart[g_enumLang]);
	m_tab.InsertItem(8, szNetwork[g_enumLang]);
	m_tab.InsertItem(9, szTools[g_enumLang]);
//	m_tab.InsertItem(9, szCommand[g_enumLang]);
	m_tab.InsertItem(10, szSetConfig[g_enumLang]);
	m_tab.InsertItem(11, szAbout[g_enumLang]);
	
	m_ListProcessDlg.Create(IDD_PROCESS_DIALOG, &m_tab);
	m_ListDriverDlg.Create(IDD_DRIVER_DIALOG, &m_tab);
	m_Ring0HookDlg.Create(IDD_RING0_HOOK_DIALOG, &m_tab);
	m_KernelAboutDlg.Create(IDD__KERNEL_DIALOG, &m_tab);
//	m_UserModeDlg.Create(IDD_USER_MODE_DIALOG, &m_tab);
	m_RegistryDlg.Create(IDD_REGISTRY_DIALOG, &m_tab);
	m_ServiceDlg.Create(IDD_SERVICE_DIALOG, &m_tab);
	m_AutoStartDlg.Create(IDD_AUTOSTART_DIALOG, &m_tab);
//	m_CommandDlg.Create(IDD_COMMAND_DIALOG, &m_tab);
	m_ToolsDlg.Create(IDD_TOOLS_DIALOG, &m_tab);
	m_ConfigDlg.Create(IDD_CONFIG_DIALOG, &m_tab);
	m_AboutDlg.Create(IDD_ABOUT_DIALOG, &m_tab);
	m_FileDlg.Create(IDD_FILE_DIALOG, &m_tab);
	m_NetworkDlg.Create(IDD_NET_DIALOG, &m_tab);
	
	// 根据配置文件设置窗口标题
	CString szTitle;
	BOOL bRandomTitle = g_Config.GetRandomTitle();
	if (bRandomTitle)
	{
		// 如果是随机标题
		szTitle = GetRandTitle();
	}
	else
	{
		// 设置了标题,那么就用设置的
		szTitle = g_Config.GetTitle();
		if (szTitle.IsEmpty())
		{
			// 否则用默认的
			szTitle = L"AntiSpy";
		}
	}
	SetWindowText(szTitle);

	// 判断是否启动时检查更新
	BOOL bUpdate = g_Config.GetAutoDetectNewVersion();
	if (bUpdate)
	{
		DWORD dwTid = 0;
		HANDLE hThread = CreateThread(NULL, 0, CheckUpdateThreadProc, this, 0, &dwTid);
		if (hThread)
		{
			CloseHandle(hThread);
		}
	}

	// 根据配置文件设置窗口是否总在最前
	BOOL bStayOnTop = g_Config.GetStayOnTop();
	if (bStayOnTop)
	{
		SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// 设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 22;
	rc.bottom -= 3;
	rc.left += 2;
	rc.right -= 3;

	m_ListProcessDlg.MoveWindow(&rc);
	m_ListDriverDlg.MoveWindow(&rc);
	m_KernelAboutDlg.MoveWindow(&rc);
	m_Ring0HookDlg.MoveWindow(&rc);
//	m_UserModeDlg.MoveWindow(&rc);
	m_RegistryDlg.MoveWindow(&rc);
	m_ServiceDlg.MoveWindow(&rc);
	m_AutoStartDlg.MoveWindow(&rc);
// 	m_CommandDlg.MoveWindow(&rc);
	m_ToolsDlg.MoveWindow(&rc);
	m_ConfigDlg.MoveWindow(&rc);
	m_AboutDlg.MoveWindow(&rc);
	m_FileDlg.MoveWindow(&rc);
	m_NetworkDlg.MoveWindow(&rc);

	// 把对话框对象指针保存起来
	g_pDialog[0] = &m_ListProcessDlg;
	g_pDialog[1] = &m_ListDriverDlg;
	g_pDialog[2] = &m_Ring0HookDlg;
	g_pDialog[3] = &m_KernelAboutDlg;
// 	g_pDialog[4] = &m_UserModeDlg;
	g_pDialog[4] = &m_RegistryDlg;
	g_pDialog[5] = &m_FileDlg;
	g_pDialog[6] = &m_ServiceDlg;
	g_pDialog[7] = &m_AutoStartDlg;
	g_pDialog[8] = &m_NetworkDlg;
// 	g_pDialog[9] = &m_CommandDlg;
	g_pDialog[9] = &m_ToolsDlg;
	g_pDialog[10] = &m_ConfigDlg;
	g_pDialog[11] = &m_AboutDlg;
	
	// 显示初始页面
	g_pDialog[0]->ShowWindow(SW_SHOW);
	g_pDialog[1]->ShowWindow(SW_HIDE);
	g_pDialog[2]->ShowWindow(SW_HIDE);
	g_pDialog[3]->ShowWindow(SW_HIDE);
	g_pDialog[4]->ShowWindow(SW_HIDE);
	g_pDialog[5]->ShowWindow(SW_HIDE);
	g_pDialog[6]->ShowWindow(SW_HIDE);
	g_pDialog[7]->ShowWindow(SW_HIDE);
	g_pDialog[8]->ShowWindow(SW_HIDE);
	g_pDialog[9]->ShowWindow(SW_HIDE);
	g_pDialog[10]->ShowWindow(SW_HIDE);
	g_pDialog[11]->ShowWindow(SW_HIDE);
/*	g_pDialog[12]->ShowWindow(SW_HIDE);*/

	g_pRegistryDlg = (CWnd*)&m_RegistryDlg;
	g_pFileDlg = (CWnd*)&m_FileDlg;
	g_pServiceDlg = (CWnd*)&m_ServiceDlg;

	// Hook LoadLibraryExW
//	HookLoadLibraryExW();
	
//	SetFullScreen();

// 	ShowWindow(SW_MINIMIZE);
//	ShowWindow(SW_SHOWNORMAL);
	
	CRect rc1;
	GetClientRect(&rc1);
//	ScreenToClient(&rc1); 
	MoveWindow( &rc1 );   

	// 当前选择
	m_nCurSel = 0;
	INIT_EASYSIZE;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

CString CSpyHunterDlg::GetRandTitle()
{
	CString szResult;

	ULONG nTickCount = GetTickCount();
	srand(nTickCount);
	WCHAR szMzf[] = {'m','i','n','z','h','e','n','f','e','i','\0'};
	size_t nLen = wcslen(szMzf);
	int nTimes = (rand() & 10) + wcslen(szMzf);
	WCHAR *szName = (WCHAR*)malloc(sizeof(WCHAR) * (nLen + 1));
	if (!szName)
	{
		return szMzf;
	}

	memset(szName, 0, sizeof(WCHAR) * (nLen + 1));

	for (int i = 0; i < (int)nLen; i++)
	{
		szName[i] = (nTimes + rand()) % 26 + 97;
	}

	szName[nLen] = '\0';
	szResult = szName;
	free(szName);

	return szResult;
}

void CSpyHunterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSpyHunterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CSpyHunterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSpyHunterDlg::OnBnClickedOk()
{
	return;
}

void CSpyHunterDlg::OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult)
{
	g_pDialog[m_nCurSel]->ShowWindow(SW_HIDE);
	m_nCurSel = m_tab.GetCurSel();
	g_pDialog[m_nCurSel]->ShowWindow(SW_SHOW);

	if (m_nCurSel == 0)
	{
		m_ListProcessDlg.ListProcess();
	}
	else if (m_nCurSel == 1)
	{
		m_ListDriverDlg.ListDrivers();
	}
	else if (m_nCurSel == 2)
	{
		m_Ring0HookDlg.m_SsdtDlg.GetSsdtHooks();
	}
	else if (m_nCurSel == 3)
	{
		m_KernelAboutDlg.m_ListCallbackDlg.GetCallbacks();
	}
	else if (m_nCurSel == 4)
	{
	//	m_UserModeDlg.m_MessageHookDlg.EnumMessageHook();
	}
	else if (m_nCurSel == 5)
	{
	//	m_RegistryDlg.InitRegistry();
	}
	else if (m_nCurSel == 6)
	{
		m_ServiceDlg.EnumServers();
	}
	else if(m_nCurSel == 7)
	{
		m_AutoStartDlg.EnumAutostarts();
	}
	else if(m_nCurSel == 8)
	{
		m_NetworkDlg.m_PortDlg.EnumPort(); // 枚举端口
	}
	else if(m_nCurSel == 9)
	{
		m_ToolsDlg.m_HexEditorDlg.ListProcess();	
	}
	else if(m_nCurSel == 10) // 配置窗口
	{
		m_ConfigDlg.UpdateStatus();
	}

	*pResult = 0;
}

void CSpyHunterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;

	if (m_tab.m_hWnd)
	{
		CRect rc;
		m_tab.GetClientRect(rc);
		rc.top += 22;
		rc.bottom -= 3;
		rc.left += 2;
		rc.right -= 3;

		m_ListProcessDlg.MoveWindow(&rc);
		m_ListDriverDlg.MoveWindow(&rc);
		m_Ring0HookDlg.MoveWindow(&rc);
		m_KernelAboutDlg.MoveWindow(&rc);
//		m_UserModeDlg.MoveWindow(&rc);
		m_RegistryDlg.MoveWindow(&rc);
		m_ServiceDlg.MoveWindow(&rc);
		m_AutoStartDlg.MoveWindow(&rc);
//		m_CommandDlg.MoveWindow(&rc);
		m_ToolsDlg.MoveWindow(&rc);
		m_AboutDlg.MoveWindow(&rc);
		m_ConfigDlg.MoveWindow(&rc);
		m_FileDlg.MoveWindow(&rc);
		m_NetworkDlg.MoveWindow(&rc);
	}
}

BOOL CSpyHunterDlg::EnableDebugPrivilege(BOOL bEnable) 
{
	BOOL bOk = FALSE; 
	HANDLE hToken;

	if (OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) 
	{
		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bOk = (GetLastError() == ERROR_SUCCESS);
		::CloseHandle(hToken);
	}

	return bOk;
}

BOOL CSpyHunterDlg::EnableLoadDricerPrivilege(BOOL bEnable) 
{
	BOOL bOk = FALSE; 
	HANDLE hToken;

	if (OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) 
	{
		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_LOAD_DRIVER_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bOk = (GetLastError() == ERROR_SUCCESS);
		::CloseHandle(hToken);
	}

	return bOk;
}

WIN_VERSION CSpyHunterDlg::GetWindowsVersion()
{
	WIN_VERSION WinVersion = enumWINDOWS_UNKNOW;
	OSVERSIONINFOEX osverinfo;

	memset(&osverinfo, 0, sizeof(OSVERSIONINFOEX));
	osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (GetVersionEx((LPOSVERSIONINFO)&osverinfo))
	{
		if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 0) 
		{
			WinVersion = enumWINDOWS_2K;
		} 
		else if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 1) 
		{
			WinVersion = enumWINDOWS_XP;
		} 
		else if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 2) 
		{
			if (osverinfo.wServicePackMajor == 0) 
			{ 
				WinVersion = enumWINDOWS_2K3;
			} 
			else 
			{
				WinVersion = enumWINDOWS_2K3_SP1_SP2;
			}
		} 
		else if (osverinfo.dwMajorVersion == 6 && osverinfo.dwMinorVersion == 0) 
		{
			if (osverinfo.dwBuildNumber == 6000)
			{
				WinVersion = enumWINDOWS_VISTA;
			}
			else if (osverinfo.dwBuildNumber == 6001 || osverinfo.dwBuildNumber == 6002)
			{
				WinVersion = enumWINDOWS_VISTA_SP1_SP2;
			}
		}
		else if (osverinfo.dwMajorVersion == 6 && osverinfo.dwMinorVersion == 1)
		{
			WinVersion = enumWINDOWS_7;
		}
		else if (osverinfo.dwMajorVersion == 6 && osverinfo.dwMinorVersion == 2)
		{
			switch (osverinfo.dwBuildNumber)
			{
// 			case 8250:
// 			case 8400:
			case 9200:
				WinVersion = enumWINDOWS_8;
				break;
			}
		}
		else
		{
			WinVersion = enumWINDOWS_UNKNOW;
		}
	}

	return WinVersion;
}

/*#define STATUS_INVALID_DISPOSITION       0xC0000026L*/

BOOL CSpyHunterDlg::LoadDriver(CString szPath, CString szDriverName)
{
	BOOL bLoadDriverOK = FALSE;
	WCHAR szNtdll[] = {'n','t','d','l','l','.','d','l','l','\0'};
	HMODULE hNtdll = GetModuleHandle(szNtdll);

	if (hNtdll == NULL)
	{
		return bLoadDriverOK;
	}

	CHAR szZwLoadDriver[] = {'Z','w','L','o','a','d','D','r','i','v','e','r','\0'};
	pfnZwLoadDriver ZwLoadDriver = (pfnZwLoadDriver)GetProcAddress(hNtdll, szZwLoadDriver);
	if (ZwLoadDriver == NULL)
	{
		return bLoadDriverOK;
	}

	WCHAR szDriverFormat[] = {'\\','?','?','\\','%','s','\\','%','s','.','s','y','s','\0'};
	CString szDriverPath;
	szDriverPath.Format(szDriverFormat, szPath, szDriverName);

	WCHAR szRegFormat[] = {'S','y','s','t','e','m','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','S','e','r','v','i','c','e','s','\\','%','s','\0'};
	CString szKey;
	szKey.Format(szRegFormat, szDriverName);

	HKEY hKey;
	if ( RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS)
	{
		return bLoadDriverOK;
	}

	DebugLog(L"ZwLoadDriver: 0x%08X", ZwLoadDriver);

 	DWORD Data = 1;
	WCHAR szType[] = {'T','y','p','e','\0'};
	WCHAR szErrorControl[] = {'E','r','r','o','r','C','o','n','t','r','o','l','\0'};
	WCHAR szStart[] = {'S','t','a','r','t','\0'};
	WCHAR szImagePath[] = {'I','m','a','g','e','P','a','t','h','\0'};

	RegSetValueEx(hKey, szType, 0, REG_DWORD, (BYTE*)&Data, sizeof(DWORD));
	RegSetValueEx(hKey, szErrorControl, 0, REG_DWORD, (BYTE*)&Data, sizeof(DWORD));
	RegSetValueEx(hKey, szStart, 0, REG_DWORD, (BYTE*)&Data, sizeof(DWORD));
 	RegSetValueEx(hKey, szImagePath, 0, REG_SZ, (BYTE *)(szDriverPath.GetBuffer()), szDriverPath.GetLength() * sizeof(WCHAR));

 	RegCloseKey(hKey);

	WCHAR szXxFormat[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','y','s','t','e','m','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','S','e','r','v','i','c','e','s','\\','%','s','\0'};
	CString szKeyTemp;
	szKeyTemp.Format(szXxFormat, szDriverName);

	UNICODE_STRING unDriverPath;
	unDriverPath.Buffer = szKeyTemp.GetBuffer();
	unDriverPath.MaximumLength = szKeyTemp.GetLength() * sizeof(WCHAR);
	unDriverPath.Length = unDriverPath.MaximumLength;
	ULONG status = ZwLoadDriver(&unDriverPath);
	DebugLog(L"ZwLoadDriver status： 0x%08X", status);

	if ( !status )
	{
		bLoadDriverOK = TRUE;

// 		WCHAR szFile[] = {'\\','\\','.','\\','%','s','\0'};
// 		CString szDriver;
// 		szDriver.Format(szFile, szDriverName);
// 		HANDLE hDriver = CreateFile(szDriver, 0x80000000, 0, 0, 3, 0, 0);
// 		if ( hDriver != INVALID_HANDLE_VALUE )
// 		{
// 			DWORD dwRet = 0;
// 			VERSION_INFO vi;
// 			memset(&vi, 0, sizeof(VERSION_INFO));
// 			vi.OpType = enumCmpVersion;
// 			wcsncpy_s(vi.szVersion, 99, g_szPeVersion, wcslen(g_szPeVersion));
// 
// 			if (DeviceIoControl( hDriver, IOCTL_NEITHER_CONTROL, &vi, sizeof(VERSION_INFO), NULL, 0, &dwRet, NULL ))
// 			{
// 				bLoadDriverOK = TRUE;
// 				DebugLog(L"ZwLoadDriver success");
// 			}
// 
// 			CloseHandle(hDriver);
// 		}
	}
	else if (STATUS_INVALID_DISPOSITION == status)
	{
		MessageBox(szYourSystemUpdate[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
		
		WCHAR szDriverFormat[] = {'%','s','\\','%','s','.','s','y','s','\0'};
		CString szDriverPath;
		szDriverPath.Format(szDriverFormat, szPath, szDriverName);
		DeleteFile(szDriverPath);

		ExitProcess(0);
	}

	WCHAR szEnum[] = {'\\','E','n','u','m','\0'};
	CString strEnum = szKey;
	strEnum += szEnum;
 	RegDeleteKey(HKEY_LOCAL_MACHINE, strEnum);

	WCHAR szSecurity[] = {'\\','S','e','c','u','r','i','t','y','\0'};
	CString strSecurity = szKey;
	strSecurity += szSecurity;
	RegDeleteKey(HKEY_LOCAL_MACHINE, strSecurity);

 	RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);

	return bLoadDriverOK;
}

BOOL CSpyHunterDlg::StartDriver()
{
	WCHAR szExePath[MAX_PATH] = {0};
	BOOL bRet = FALSE;
	
	if (!GetModuleFileName(NULL, szExePath, MAX_PATH))
	{
		return FALSE;
	}

	CString szTemp = szExePath;
	CString szDir = szTemp.Left(szTemp.Find('\\') + 1);
	WCHAR szDrivers[] = {'\\','d','r','i','v','e','r','s','\0'};
	WCHAR *szPath = wcsrchr(szExePath, '\\');;
	CString szEndPath = L"";

	if ( GetDriveType(szDir) != DRIVE_FIXED )
	{
		WCHAR szDest[MAX_PATH] = {0};
		GetSystemDirectory(szDest, MAX_PATH);
		ULONG nSysDir = wcslen(szDest);
		wcsncat_s(szDest, szDrivers, MAX_PATH - nSysDir);
		szEndPath = szDest;
	}
	else
	{
		if ( szPath )
		{
			*szPath = '\0';
		}
		szEndPath = szExePath;
	}

	WCHAR szName[0x20] = {'A','n','t','i','S','p','y','\0'};

	if ( szPath )
	{
		WCHAR *szNameExt = szPath + 1;
		WCHAR *szNameTemp = wcschr(szNameExt, '.');
		if ( szNameTemp )
		{
			*szNameTemp = 0;
		}

		DWORD dwLen = wcslen(szNameExt);
		if ( dwLen > 0 && dwLen < 0x20 )
		{
			memset(szName, 0, 0x20 * sizeof(WCHAR));
			wcsncpy_s(szName, 0x20, szNameExt, 0x20 - 1);
		}
	}
	
	WCHAR szL[] = {'\\','\\','.','\\','%','s','\0'};
	CString szFile;
	szFile.Format(szL, szName);
	
	HANDLE hFile = CreateFile(szFile, 0x80000000, 0, 0, 3, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		if (dwError != ERROR_INVALID_PARAMETER)
		{

WhileLoadDriver:

			int iCnt = 5;
			do 
			{
				if (!ReleaseDriver(szEndPath, szName))
				{
					DebugLog(L"ReleaseDriver error");
					break;
				}

				if (LoadDriver(szEndPath, szName))
				{
					WCHAR szFile[] = {'\\','\\','.','\\','%','s','\0'};
					CString szDriver;
					szDriver.Format(szFile, szName);
					HANDLE hDriver = CreateFile(szDriver, 0x80000000, 0, 0, 3, 0, 0);
					if ( hDriver != INVALID_HANDLE_VALUE )
					{
						wcsncat_s(szWin32DriverName, 0x20 - wcslen(szWin32DriverName), szName, wcslen(szName));
						DeleteDriver(szEndPath, szName);
						bRet = TRUE;
						CloseHandle(hDriver);
						break;
					}
					else
					{
						DWORD dwError = GetLastError();
						if (dwError == ERROR_INVALID_PARAMETER)
						{
							break;
						}
					}
				}

				DeleteDriver(szEndPath, szName);
				GetRandDriverName(szEndPath, szName);
			} while (iCnt--);
		}
	}

	// 如果已经存在,那么就判断版本是否是一样的
	else
	{
		DWORD dwRet = 0;
		VERSION_INFO vi;
		memset(&vi, 0, sizeof(VERSION_INFO));
		vi.OpType = enumCmpVersion;
		wcsncpy_s(vi.szVersion, 99, g_szPeVersion, wcslen(g_szPeVersion));
		
		if (DeviceIoControl( hFile, IOCTL_NEITHER_CONTROL, &vi, sizeof(VERSION_INFO), NULL, 0, &dwRet, NULL ))
		{
			wcsncat_s(szWin32DriverName, 32 - wcslen(szWin32DriverName), szName, wcslen(szName));
			bRet = TRUE;
			DebugLog(L"Open Driver success");
		}
		else
		{
			// 如果版本不一致,那么继续加载
			CloseHandle(hFile);
			goto WhileLoadDriver;
		}

		CloseHandle(hFile);
	}

	return bRet;
}

CString CSpyHunterDlg::GetRandDriverName(CString szPath, WCHAR* szName)
{
	CString szResult;

	if (szName && !szPath.IsEmpty())
	{
 		Sleep(1);
		ULONG nTickCount = GetTickCount();
		srand(nTickCount);
 		WCHAR szMzf[] = {'m','i','n','z','h','e','n','f','e','i','\0'};
		size_t nLen = wcslen(szName);
		int nTimes = (rand() & 10) + wcslen(szMzf);
		
		WCHAR szFormat[] = {'%','s','\\','%','s','.','s','y','s','\0'};

		do
		{
			for (int i = 0; i < (int)nLen; i++)
			{
				szName[i] = (nTimes + rand()) % 26 + 97;
			}

			szName[nLen] = 0;
			szResult.Format(szFormat, szPath, szName);
			Sleep(1);
		}while ( PathFileExists(szResult) );
	}

	return szResult;
}

BOOL CSpyHunterDlg::ReleaseDriver( CString szPath, CString szName )
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BOOL hr = FALSE;

	if (szPath.IsEmpty() || szName.IsEmpty())
	{
		return hr;
	}

	WCHAR szFormat[] = {'%','s','\\','%','s','.','s','y','s','\0'};
	CString szDriverPath;
	szDriverPath.Format(szFormat, szPath, szName);
	
	DeleteFile(szDriverPath);

	hFile = CreateFile(szDriverPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DebugLog(L"CreateFile: %s success", szName);

		DWORD dwAttr = GetFileAttributes(szDriverPath);
		SetFileAttributes(szDriverPath, dwAttr | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

		TCHAR szKernel[] = {'K','E','R','N','E','L','\0'};
		HRSRC hRsc = FindResource(NULL, MAKEINTRESOURCE(IDR_DRIVER), szKernel);
		if (hRsc)
		{
			DWORD dwResSize = SizeofResource(NULL, hRsc);
			if(dwResSize > 0)
			{
				HGLOBAL hResData = LoadResource(NULL, hRsc);
				if(hResData != NULL)
				{
					LPVOID lpResourceData = LockResource(hResData);
					if(lpResourceData != NULL)
					{
						PVOID pBuffer = malloc(dwResSize);
						if (pBuffer)
						{
							memset(pBuffer, 0, dwResSize);
							memcpy(pBuffer, lpResourceData, dwResSize);

							m_Functions.DecryptResource(pBuffer, dwResSize);

							DWORD dwRet;
							if (WriteFile(hFile, pBuffer, dwResSize, &dwRet, NULL))
							{
								hr = TRUE;
							}

							free (pBuffer);
							pBuffer = NULL;
						}

						FreeResource(hResData);
					}
				}
			}
		}
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}
	
	if (!hr)
	{
		DeleteFile(szDriverPath);
	}

	return hr;
}

void CSpyHunterDlg::DeleteDriver(CString szPath, CString szName)
{
	if (szPath.IsEmpty() || szName.IsEmpty())
	{
		return;
	}
	
	WCHAR szFormat[] = {'%','s','\\','%','s','.','s','y','s','\0'};
	CString szDriverPath;
	szDriverPath.Format(szFormat, szPath, szName);
	if (PathFileExists(szDriverPath))
	{
		DeleteFile(szDriverPath);
	}
}

LANGUAGE_TYPE CSpyHunterDlg::GetLanguageID()
{
 //	return enumEnglish; 
	LANGUAGE_TYPE languageId = enumEnglish;
	LCID nLangID = GetUserDefaultUILanguage();
	switch (nLangID)
	{
	case 0x0409:		//LANG_ENGLISH  SUBLANG_ENGLISH_US
	case 0x0C09:		//SUBLANG_ENGLISH_AUS
	case 0x2809:		//SUBLANG_ENGLISH_BELIZE
	case 0x1009:		//SUBLANG_ENGLISH_CAN
	case 0x2409:		//SUBLANG_ENGLISH_CARIBBEAN
	case 0x4009:		 //SUBLANG_ENGLISH_INDIA
	case 0x1809:		//SUBLANG_ENGLISH_EIRE
	case 0x2009:		//SUBLANG_ENGLISH_JAMAICA
	case 0x4409:		//SUBLANG_ENGLISH_MALAYSIA
	case 0x1409:		//SUBLANG_ENGLISH_NZ
	case 0x3409:		//SUBLANG_ENGLISH_PHILIPPINES
	case 0x4809:		//SUBLANG_ENGLISH_SINGAPORE
	case 0x1c09:		//SUBLANG_ENGLISH_SOUTH_AFRICA
	case 0x2C09:		//SUBLANG_ENGLISH_TRINIDAD
	case 0x0809:		//SUBLANG_ENGLISH_UK
	case 0x3009:		//SUBLANG_ENGLISH_ZIMBABWE
		languageId = enumEnglish;
		break;

	case 0x0C04:		//SUBLANG_CHINESE_HONGKONG
	case 0x1404:		//SUBLANG_CHINESE_MACAU
	case 0x1004:		//SUBLANG_CHINESE_SINGAPORE
	case 0x0804:		//SUBLANG_CHINESE_SIMPLIFIED
	case 0x0404:		//SUBLANG_CHINESE_TRADITIONAL
		languageId = enumChinese;
		break;

	default :
		languageId = enumEnglish;
		break;
	}

	return languageId;
}

void CAboutDlg::OnBnClickedOk()
{
	OnOK();
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
 	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
// 
// 	if (pWnd->GetDlgCtrlID() == IDC_EMAIL ||
// 		pWnd->GetDlgCtrlID() == IDC_MICROBOLG ||
// 		pWnd->GetDlgCtrlID() == IDC_WEBSITE)
// 	{
// 		pDC->SetTextColor(RGB(0, 0, 255));
// 	}

	return hbr;
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
// 	CPoint PointEmail; 
// 	GetCursorPos(&PointEmail); 
// 	CPoint PointSina; 
// 	GetCursorPos(&PointSina); 
// 	CPoint PointWebsite; 
// 	GetCursorPos(&PointWebsite); 
// 
// 	//然后得到static控件rect。
// 	CRect rectEmail; 
// 	::GetClientRect(GetDlgItem(IDC_EMAIL)->GetSafeHwnd(), &rectEmail); 
// 	CRect rectSina; 
// 	::GetClientRect(GetDlgItem(IDC_MICROBOLG)->GetSafeHwnd(), &rectSina); 
// 	CRect rectWebsite; 
// 	::GetClientRect(GetDlgItem(IDC_WEBSITE)->GetSafeHwnd(), &rectWebsite); 
// 
// 	//然后把当前鼠标坐标转为相对于rect的坐标。 
// 	::ScreenToClient(GetDlgItem(IDC_EMAIL)->GetSafeHwnd(), &PointEmail); 
// 	::ScreenToClient(GetDlgItem(IDC_MICROBOLG)->GetSafeHwnd(), &PointSina); 
// 	::ScreenToClient(GetDlgItem(IDC_WEBSITE)->GetSafeHwnd(), &PointWebsite); 
// 
// 	if(rectEmail.PtInRect(PointEmail) ||
// 		rectSina.PtInRect(PointSina) ||
// 		rectWebsite.PtInRect(PointWebsite)) 
// 	{
// 		SetCursor(LoadCursor(NULL, IDC_HAND));
// 	} 

	CDialog::OnMouseMove(nFlags, point);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	//先得到当前鼠标坐标
// 	CPoint PointEmail; 
// 	GetCursorPos(&PointEmail); 
// 	CPoint PointSina; 
// 	GetCursorPos(&PointSina); 
// 	CPoint PointWebsite; 
// 	GetCursorPos(&PointWebsite); 
// 
// 	//然后得到static控件rect。
// 	CRect rectEmail; 
// 	::GetClientRect(GetDlgItem(IDC_EMAIL)->GetSafeHwnd(), &rectEmail); 
// 	CRect rectSina; 
// 	::GetClientRect(GetDlgItem(IDC_MICROBOLG)->GetSafeHwnd(), &rectSina); 
// 	CRect rectWebsite; 
// 	::GetClientRect(GetDlgItem(IDC_WEBSITE)->GetSafeHwnd(), &rectWebsite); 
// 	
// 	//然后把当前鼠标坐标转为相对于rect的坐标。 
// 	::ScreenToClient(GetDlgItem(IDC_EMAIL)->GetSafeHwnd(), &PointEmail); 
// 	::ScreenToClient(GetDlgItem(IDC_MICROBOLG)->GetSafeHwnd(), &PointSina); 
// 	::ScreenToClient(GetDlgItem(IDC_WEBSITE)->GetSafeHwnd(), &PointWebsite); 
// 
// 	if (rectEmail.PtInRect(PointEmail))
// 	{
// 		WCHAR szMailTo[] = {'m','a','i','l','t','o',':','m','i','n','z','h','e','n','f','e','i','@','1','6','3','.','c','o','m','\0'};
// 		ShellExecute(NULL,NULL, szMailTo,NULL,NULL,SW_SHOW);
// 	}
// 	else if (rectSina.PtInRect(PointSina))
// 	{
// 		WCHAR szUrl[] = {'h','t','t','p',':','/','/','w','e','i','b','o','.','c','o','m','/','m','i','n','z','h','e','n','f','e','i','\0'};	
// 		ShellExecuteW(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
// 	}
// 	else if (rectWebsite.PtInRect(PointWebsite))
// 	{
// 		WCHAR szUrl[] = {'h','t','t','p',':','/','/','w','w','w','.','k','i','n','g','p','r','o','t','e','c','t','.','c','o','m','\0'};	
// 		ShellExecuteW(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
// 	}

	CDialog::OnLButtonDown(nFlags, point);
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

BOOL CSpyHunterDlg::IsWow64()
{
	BOOL bIsWow64 = FALSE;
	WCHAR szKernel32[] = {'k','e','r','n','e','l','3','2','\0'};
	CHAR szIsWow64Process[] = {'I','s','W','o','w','6','4','P','r','o','c','e','s','s','\0'};

	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(szKernel32), szIsWow64Process );

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			// handle error
		}
	}

	return bIsWow64;
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	/*WCHAR szUrlBlog[] = {'h','t','t','p',':','/','/','w','w','w','.','K','i','n','g','P','r','o','t','e','c','t','.','c','o','m','\0'};	*/
	WCHAR szUrlBlog[] = {'h','t','t','p',':','/','/','w','w','w','.','A','n','t','i','S','p','y','.','c','n','\0'};	
	m_BlogHypeLink.SetUnderline(false);
	m_BlogHypeLink.SetURL(szUrlBlog);
	m_BlogHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));

	WCHAR szUrlSina[] = {'h','t','t','p',':','/','/','w','e','i','b','o','.','c','o','m','/','m','i','n','z','h','e','n','f','e','i','\0'};	
	m_SinaHypeLink.SetUnderline(false);
	m_SinaHypeLink.SetURL(szUrlSina);
	m_SinaHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));

	WCHAR szAntiSpyMail[] = {'A','n','t','i','S','p','y','@','1','6','3','.','c','o','m','\0'};
	WCHAR szMailTo[] = {'m','a','i','l','t','o',':','A','n','t','i','S','p','y','@','1','6','3','.','c','o','m','\0'};
	m_EmailHypeLink.SetUnderline(false);
	m_EmailHypeLink.SetURL(szMailTo);
	m_EmailHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));
	m_EmailHypeLink.SetTipText(szAntiSpyMail);

	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CSpyHunterDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (g_Config.GetEnableHotkeys())
	{
		CHAR chHotKey = g_Config.GetHotKeyForShowMainWiondow();
		RegisterHotKey(m_hWnd, 1001, MOD_SHIFT | MOD_ALT | MOD_CONTROL, chHotKey);
	}

	return 0;
}

void CSpyHunterDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (g_Config.GetEnableHotkeys())
	{
		UnregisterHotKey(m_hWnd, 1001);
	}

	// 关闭自我保护	
	if (g_bLoadDriverOK && m_bSelfProtection)
	{
		COMMUNICATE_SELF_PROTECT cm;
		cm.OpType = enumStartSelfProtect;
		cm.op.StartOrStop.nMask = SELF_PROTECT_SSDT | SELF_PROTECT_SHADOW;
		cm.op.StartOrStop.bStart = FALSE;
		m_Driver.CommunicateDriver(&cm, sizeof(COMMUNICATE_SELF_PROTECT), NULL, 0, NULL);
	}
}

LRESULT CSpyHunterDlg::OnHotKey(WPARAM wParam,LPARAM lParam)
{
	if (wParam== 1001)
	{
		ShowWindow(SW_NORMAL);
		CWnd::SetForegroundWindow(); // 使得被激活窗口出现在前景
	}

	return 0;
}

BOOL CSpyHunterDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CSpyHunterDlg::SetFullScreen()
{
	int frameWidth =  GetSystemMetrics(SM_CXFRAME);
	int frameHeight = GetSystemMetrics(SM_CYFRAME);
	int captionHeight = GetSystemMetrics(SM_CYCAPTION);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	CRect rect;
	GetClientRect(&rect);
	rect.left = rect.left - frameWidth;
	rect.top = rect.top - frameHeight - captionHeight ;
	rect.bottom = rect.top + screenHeight + 2 * frameHeight + captionHeight;
	rect.right = rect.left + screenWidth + 2 * frameWidth;
	ShowWindow(SW_HIDE);
	SetWindowPos(&wndTopMost, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
}