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
#pragma once
#include "afxcmn.h"
#include <list>
#include <set>
#include "Registry.h"
#include "Function.h"
#include "SignVerifyDlg.h"
#include "RegCommon.h"
#include "xySplitterWnd.h"

using namespace std;

typedef enum AUTO_TYPE
{
	eNone,
	eStartup,   
	eWinlogon,
	eExplorer,
	eIE,
	eSystemService,
	eSystemDriver,
	eKnowDLLs,
	ePrintMonitor,
	eNetworkProvider,
	eWinsockProvider,
	eSecurityProvider,
	eTask,
	eAll,
};

typedef enum AUTO_START_TYPE
{
	eSrartupNone,

	// Startup
	eStartupDirectory,				// C:\Users\hzminzhenfei\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
	eCommonStartupDirectory,		// C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup
	eHKLM_RUN,      				// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
	eHKCU_RUN,						// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run	
	eHKLM_RUNONCE,					// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce
	eHKCU_RUNONCE,					// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\RunOnce
	eHKLM_RUNONCE_SETUP,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce\Setup
	eHKCU_RUNONCEEX,				// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\RunOnceEx
	eHKLM_RUNONCEEX,				// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnceEx
	eHKCU_EXPLORER_RUN,				// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\Run
	eHKLM_EXPLORER_RUN,				// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\Explorer\Run

	// Winlogon			
									// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Policies\System, Shell
									// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System, Shell
									// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows, Appinit_Dlls
	eHKCU_WINDOWS_LOAD,				// HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Windows, Load
	eHKCU_WINDOWS_RUN,				// HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Windows, Run
	eHKCU_WINLOGON_SHELL,			// HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Winlogon, Shell
	eHKLM_WINLOGON_SHELL,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, Shell
	eHKLM_WINLOGON_APPSETUP,		// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, AppSetup
	eHKLM_WINLOGON_GINADLL,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, GinaDLL
	eHKLM_WINLOGON_LSASTART,		// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, LsaStart								
	eHKLM_WINLOGON_SaveDumpStart,	// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, SaveDumpStart
	eHKLM_WINLOGON_ServiceControllerStart, // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, ServiceControllerStart
	eHKLM_WINLOGON_System,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, System
	eHKLM_WINLOGON_Taskman,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, Taskman
	eHKLM_WINLOGON_UIHost,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, UIHost
	eHKLM_WINLOGON_Userinit,		// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon, Userinit
	eHKLM_WINLOGON_GPExtensions,	// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\GPExtensions
	eHKLM_WINLOGON_Notify,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Notify

	// explorer
	eInstalledComponents,
	eShellIconOverlayIdentifiers,	// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers
	eSharedTaskScheduler,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\SharedTaskScheduler
	eShellExecuteHooks,				// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellExecuteHooks
	eShellServiceObjectDelayLoad,	// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\ShellServiceObjectDelayLoad
	eContextMenuHandlers,			// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\*\shellex\ContextMenuHandlers
	eContextMenuHandlers2,			// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers
	eContextMenuHandlers3,			// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\Background\shellex\ContextMenuHandlers
	eContextMenuHandlers4,			// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\shellex\ContextMenuHandlers
	eContextMenuHandlers5,			// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Drive\shellex\ContextMenuHandlers
	eContextMenuHandlers6,			// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Folder\shellex\ContextMenuHandlers
	eCopyHookHandlers,				// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\shellex\CopyHookHandlers
	eDragDropHandlers,				// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Directory\shellex\DragDropHandlers
	eFilter,						// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\PROTOCOLS\Filter
	eHandler,						// HKEY_LOCAL_MACHINE\SOFTWARE\Classes\PROTOCOLS\Handler
	eApproved,						// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved

	// ie
	eBrowserHelperObjects,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Browser Helper Objects
	eURLSearchHooks,				// HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\URLSearchHooks
	eExtensions,					// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Internet Explorer\Extensions

	// eKnowDLLs
	eKnowDlls,						// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\KnownDLLs

	// ePrintMonitor
	ePrintMonitors,					// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Print\Monitors

	// eNetworkProvider
	eNetworkProviders,				// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\NetworkProvider\Order,ProviderOrder

	// eWinsockProvider
	eProtocolCatalog9,				// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries
	eNameSpaceCatalog5,				// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\NameSpace_Catalog5\Catalog_Entries

	// eSecurityProvider
	eSecurityProviders,				// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\SecurityProviders, SecurityProviders
	eAuthenticationPackages,		// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Lsa, Authentication Packages
	eNotificationPackages,			// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Lsa, Notification Packages
	eSecurityPackages,				// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Lsa, Security Packages
	eCredentialProviders,			// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers
	eCredentialProviderFilters,		// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Provider Filters
	ePLAPProviders,					// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\PLAP Providers

	// eTask
	eTaskDirectory,					// c:\windows\tasks

	eService,
	eDriver,
};

typedef enum _AUTO_RUN_STATUS_
{
	eOK,
	eDisable,
	eDelete,
}AUTO_RUN_STATUS;

typedef struct _AUTO_START_INFO 
{
	AUTO_START_TYPE Type;
	CString szName;
	CString szPath;

	CString szKeyPath;		// 键
	CString szValueName;	// 值

	AUTO_RUN_STATUS nStatus;
}AUTO_START_INFO, *PAUTO_START_INFO;

// CAutoStartDlg 对话框

class CAutoStartDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CAutoStartDlg)

public:
	CAutoStartDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAutoStartDlg();

// 对话框数据
	enum { IDD = IDD_AUTOSTART_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void EnumAutostarts();
	void EnumStartup();
	void EnumValue(CString szKey, AUTO_START_TYPE Type);
	void ParsePath(WCHAR *szPath, ULONG dwType, set<wstring> &AppSet);
	BOOL GetCurrentUserKeyPath(OUT WCHAR *szCurrentUserPath);
	void EnumWinlogon();
	void EnumStartupDirectory();
	BOOL ReadShortcut(LPWSTR lpwLnkFile, LPWSTR lpDescFile);
	BOOL InitTree();
	void EnumExplorer();
	void EnumSharedTaskScheduler();
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickTree(NMHDR *pNMHDR, LRESULT *pResult);
	void ShowAutostarts();
	void EnumShellExecuteHooks();
	CString GetContextMenuHandlersCLSID(CString szKey);
	void EnumContextMenuHandlers();
	void EnumContextMenuHandler(CString szKey, AUTO_START_TYPE Type);
	void EnumShellServiceObjectDelayLoad();
	void EnumFilter();
	CString GetFilterOrHandlerCLSID(CString szKey);
	void EnumFilterOrHandler(ULONG Type);
	void EnumFilterOrHandlerEx(CString szKey, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus);
	void EnumApproved();
	void EnumInternetExplorer();
	void EnumBrowserHelperObjects();
	void EnumKnowDlls();
	void EnumPrintMonitors();
	CString GetPrintMonitorModule(CString szKey);
	void EnumNetworkProvider();
	CString GetNetworkProviderModule(CString szKey);
	CString GetProtocolCatalog9NameAndModule(CString szKey, CString &szModule);
	void EnumWinsockProviders();
	CString MByteToWChar(IN LPCSTR lpcszStr);
	void EnumProtocol_Catalog9();
	void EnumNameSpace_Catalog5();
	CString GetNameSpace_Catalog5NameAndModule(CString szKey, CString &szModule);
	void EnumSecurityProvider();
	void EnumSecurityProviders();
	void EnumLASPackages();
	HRESULT EnumTasks();
	afx_msg void OnNMClickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnAutorunJmpToReg();
	afx_msg void OnAutorunLocationExplorer();
	afx_msg void OnAutorunDelReg();
	afx_msg void OnAutorunDelRegAndFile();
	afx_msg void OnAutorunCopyName();
	afx_msg void OnAutorunCopyPath();
	afx_msg void OnAutorunSearchOnline();
	afx_msg void OnAutorunCheckAttribute();
	afx_msg void OnAutorunCheckSign();
	afx_msg void OnAutorunCheckAllSign();
	afx_msg void OnAutorunText();
	afx_msg void OnAutorunExcel();
	LRESULT OnCloseWindow(WPARAM wParam, LPARAM lParam);
	VOID SortItem();
	BOOL IsNotSignItemData(ULONG nItem);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	void DeleteFile(CString szFilePath);
	void DelReg();
	CString ParseLnk2Path(CString pszLnkName);
	afx_msg void OnAutorunHideMicrosoftModule();
	afx_msg void OnUpdateAutorunHideMicrosoftModule(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	void ShellExportRegister( CString strItem, CString strFileName);
	BOOL IsNotSignItem(ULONG nItem);
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
	void EnumHKCURun();
	void EnumHKCURunOnce();
	void EnumHKCUExplorerRun();
	void EnumCommonStartupDirectory();
	void EnumHKCURunOnceEx();
	void EnumWinlogonCommon(CString szKey, CString szValue, AUTO_START_TYPE type);
	void EnumWinlogonKey(CString szKey, AUTO_START_TYPE type);
	CString GetWinlogonModule(CString szKey);
	void EnumWindowsLoadAndRun();
	void EnumShellIconOverlayIdentifiers();
	CString GetShellIconOverlayIdentifiersModulePath(CString szKey);
	void EnumURLSearchHooks();
	void EnumBrowserExtensions();
	CString GetBrowserExtensionsModulePath(CString szKey);
	CString GetBrowserExtensionsButtonText(CString szKey);
	CString GetCredentialProvidersName(CString szKey);
	void EnumCredentialProviders();
	void EnumPLAPProviders();
	void EnumCredentialProviderFilters();
	DWORD IsServiceOrDrivers(CString szKey);
	void EnumServiceAndDrivers();
	void UpdateStatus();
	void AddAutroRunItem(AUTO_START_TYPE nType, CString szName, CString szPath, CString szKeyPath, CString szValueName, AUTO_RUN_STATUS nStatus = eOK);
	void CheckOrNoCheckItem(NMHDR *pNMHDR);
	BOOL DisableOrEnableAutoruns(PAUTO_START_INFO info, BOOL bEnable, int nItem);
	afx_msg void OnAutorunRefresh();
	void EnumSharedTaskSchedulerEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumValueEx(CString szKey, AUTO_START_TYPE Type, AUTO_RUN_STATUS nStatus);
	void EnumWinlogonCommonEx(CString szKey, CString szValue, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus);
	void EnumWinlogonKeyEx(CString szKey, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus);
	void EnumShellExecuteHooksEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumShellIconOverlayIdentifiersEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumContextMenuHandlerEx(CString szKey, AUTO_START_TYPE Type, AUTO_RUN_STATUS nStatus);
	void EnumShellServiceObjectDelayLoadEx( CString szKey, AUTO_RUN_STATUS nStatus );
	void EnumApprovedEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumBrowserHelperObjectsEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumURLSearchHooksEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumBrowserExtensionsEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumServiceAndDriversEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumKnowDllsEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumPrintMonitorsEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumNetworkProviderEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumProtocol_Catalog9Ex(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumNameSpace_Catalog5Ex(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumSecurityProviderEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumLASPackagesEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumCredentialProvidersEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumCredentialProviderFiltersEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void EnumPLAPProvidersEx(CString szKey, AUTO_RUN_STATUS nStatus);
	void UpdateCnt(AUTO_START_TYPE Type);
	afx_msg void OnAutorunDisable();
	afx_msg void OnAutorunEnable();
	void EnumStartupDirEx(CString szDir, AUTO_START_TYPE type, AUTO_RUN_STATUS nStatus);
	CString GetDirectoryFileName(CString szPath, CString szTtileName);
	CString GetServiceDll(CString szKey);
	CString GetServiceModule(CString szKey);
	CString GetImagePath(CString szKey);
public:
	CTreeCtrl m_tree;
	CSortListCtrl m_list;
	CString m_szStatus;	
	vector<AUTO_START_INFO> m_AutostartList;
	CRegistry m_Registry;
	CommonFunctions m_Function;
	CImageList m_ProImageList;
	CString m_szHKeyCurrentUser;
	CImageList m_TreeImageList;
	DWORD m_nStartup;
	DWORD m_nWinlogon;
	DWORD m_nExplorer;
	DWORD m_nInternetExplorer;
	DWORD m_nPrintMonitors;
	DWORD m_nKnowDlls;
	DWORD m_nNetworkProvider;
	DWORD m_nWinsockProvider;
	DWORD m_nSecurityProvider;
	DWORD m_nTask;
	DWORD m_nService;
	DWORD m_nDriver;
	AUTO_TYPE m_AutoStartType;
	CString m_szStartupPath;
	CString m_szCommonStartupPath;
	CSignVerifyDlg m_SignVerifyDlg;
	BOOL m_bFirst;
	CRegCommon m_RegCommonFunc;
	BOOL m_bStartCheck;
	BOOL m_bHideMicrosoft;
	CxSplitterWnd   m_xSplitter;
};