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
// AdvancedToolsDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "AdvancedToolsDlg.h"
#include "..\\..\\Common\Common.h"

// CAdvancedToolsDlg �Ի���

IMPLEMENT_DYNAMIC(CAdvancedToolsDlg, CDialog)

CAdvancedToolsDlg::CAdvancedToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvancedToolsDlg::IDD, pParent)
	, m_bForbidCreteThread(FALSE)
	, m_bForbidCreteProcess(FALSE)
	, m_bForbidCreteFile(FALSE)
	, m_bForbidSwitchDesktop(FALSE)
	, m_bForbidCreateKey(FALSE)
	, m_bForbidSetKeyValue(FALSE)
	, m_bForbidLoadImage(FALSE)
	, m_bForbidSystemShutdown(FALSE)
	, m_bForbidModifySystemTime(FALSE)
	, m_bForbidLockWorkStation(FALSE)
	, m_bForbidLoadDriver(FALSE)
	, m_bForbidInjectModule(FALSE)
	, m_bReboot(FALSE)
	, m_bForceReboot(FALSE)
	, m_bPorweOff(FALSE)
	, m_bEnableRegistry(FALSE)
	, m_bEnableTaskTools(FALSE)
	, m_bEnableCmd(FALSE)
	, m_bEnableControlPane(FALSE)
	, m_bEnableToolbarMenu(FALSE)
	, m_bClearAllGroupPolicies(FALSE)
	, m_bEnableRun(FALSE)
	, m_bForbidScreenCapture(FALSE)
	, m_bBackupDriver(FALSE)
{
//	m_hCommEvent = NULL;
//	hCommThread = NULL;
}

CAdvancedToolsDlg::~CAdvancedToolsDlg()
{
// 	if (m_hCommEvent)
// 	{
// 		CloseHandle(m_hCommEvent);
// 		m_hCommEvent = NULL;
// 	}

// 	if (hCommThread)
// 	{
// 	}
}

void CAdvancedToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_NOT_CREATE_THREAD, m_bForbidCreteThread);
	DDX_Check(pDX, IDC_NOT_CREATE_PROCESS, m_bForbidCreteProcess);
	DDX_Check(pDX, IDC_NOT_CREATE_FILE, m_bForbidCreteFile);
	DDX_Check(pDX, IDC_NOT_SWITCH_DESKTOP, m_bForbidSwitchDesktop);
	DDX_Check(pDX, IDC_NOT_CREATE_KEY, m_bForbidCreateKey);
	DDX_Check(pDX, IDC_NOT_SET_VALUE, m_bForbidSetKeyValue);
	DDX_Check(pDX, IDC_NOT_LOAD_IMAGE, m_bForbidLoadImage);
	DDX_Check(pDX, IDC_NOT_SYSTEM_SHUTDOWN, m_bForbidSystemShutdown);
	DDX_Check(pDX, IDC_NOT_MODIFT_TIME, m_bForbidModifySystemTime);
	DDX_Check(pDX, IDC_NOT_LOCK_WORKSTATION, m_bForbidLockWorkStation);
	DDX_Check(pDX, IDC_NOT_LOAD_DRIVER, m_bForbidLoadDriver);
	DDX_Check(pDX, IDC_NOT_INJECT_MODULE, m_bForbidInjectModule);
	DDX_Check(pDX, IDC_SYSTEM_REBOOT, m_bReboot);
	DDX_Check(pDX, IDC_FORCE_SYSTEM_REBOOT, m_bForceReboot);
	DDX_Check(pDX, IDC_SYSTEM_SHUTDOWN, m_bPorweOff);
	DDX_Check(pDX, IDC_ENABLE_REGISTRY_TOOLS, m_bEnableRegistry);
	DDX_Check(pDX, IDC_ENABLE_TASK_TOOLS, m_bEnableTaskTools);
	DDX_Check(pDX, IDC_ENABLE_MY_COMPUTER, m_bEnableCmd);
	DDX_Check(pDX, IDC_ENABLE_CONTROL_PANE, m_bEnableControlPane);
	DDX_Check(pDX, IDC_ENABLE_START_MENU, m_bEnableToolbarMenu);
	DDX_Check(pDX, IDC_CLEAR_ALL_GLOUP_POLICIES, m_bClearAllGroupPolicies);
	DDX_Check(pDX, IDC_ENABLE_RUN, m_bEnableRun);
	DDX_Check(pDX, IDC_NOT_SCREEN_CAPTURE, m_bForbidScreenCapture);
	DDX_Check(pDX, IDC_BACKUO_DRIVER, m_bBackupDriver);
}


BEGIN_MESSAGE_MAP(CAdvancedToolsDlg, CDialog)
	ON_BN_CLICKED(IDC_NOT_CREATE_THREAD, &CAdvancedToolsDlg::OnBnClickedNotCreateThread)
	ON_BN_CLICKED(IDC_NOT_CREATE_PROCESS, &CAdvancedToolsDlg::OnBnClickedNotCreateProcess)
	ON_BN_CLICKED(IDC_NOT_CREATE_FILE, &CAdvancedToolsDlg::OnBnClickedNotCreateFile)
	ON_BN_CLICKED(IDC_NOT_SWITCH_DESKTOP, &CAdvancedToolsDlg::OnBnClickedNotSwitchDesktop)
	ON_BN_CLICKED(IDC_NOT_CREATE_KEY, &CAdvancedToolsDlg::OnBnClickedNotCreateKey)
	ON_BN_CLICKED(IDC_NOT_SET_VALUE, &CAdvancedToolsDlg::OnBnClickedNotSetValue)
	ON_BN_CLICKED(IDC_NOT_LOAD_IMAGE, &CAdvancedToolsDlg::OnBnClickedNotLoadImage)
	ON_BN_CLICKED(IDC_NOT_SYSTEM_SHUTDOWN, &CAdvancedToolsDlg::OnBnClickedNotSystemShutdown)
	ON_BN_CLICKED(IDC_NOT_MODIFT_TIME, &CAdvancedToolsDlg::OnBnClickedNotModiftTime)
	ON_BN_CLICKED(IDC_NOT_LOCK_WORKSTATION, &CAdvancedToolsDlg::OnBnClickedNotLockWorkstation)
	ON_BN_CLICKED(IDC_NOT_LOAD_DRIVER, &CAdvancedToolsDlg::OnBnClickedNotLoadDriver)
	ON_BN_CLICKED(IDC_NOT_INJECT_MODULE, &CAdvancedToolsDlg::OnBnClickedNotInjectModule)
	ON_BN_CLICKED(IDC_SYSTEM_REBOOT, &CAdvancedToolsDlg::OnBnClickedSystemReboot)
	ON_BN_CLICKED(IDC_FORCE_SYSTEM_REBOOT, &CAdvancedToolsDlg::OnBnClickedForceSystemReboot)
	ON_BN_CLICKED(IDC_SYSTEM_SHUTDOWN, &CAdvancedToolsDlg::OnBnClickedSystemShutdown)
	ON_BN_CLICKED(IDC_ENABLE, &CAdvancedToolsDlg::OnBnClickedEnable)
	ON_BN_CLICKED(IDC_BTN_REPAIR_SAFEBOOT, &CAdvancedToolsDlg::OnBnClickedBtnRepairSafeboot)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_NOT_SCREEN_CAPTURE, &CAdvancedToolsDlg::OnBnClickedNotScreenCapture)
	ON_BN_CLICKED(IDC_BACKUO_DRIVER, &CAdvancedToolsDlg::OnBnClickedBackuoDriver)
END_MESSAGE_MAP()


// CAdvancedToolsDlg ��Ϣ��������

void CAdvancedToolsDlg::OnBnClickedNotCreateThread()
{
	UpdateData(TRUE);
	CommDriver(eForbid_CreateThread, m_bForbidCreteThread);
}

void CAdvancedToolsDlg::OnBnClickedNotCreateProcess()
{
	UpdateData(TRUE);
	CommDriver(eForbid_CreateProcess, m_bForbidCreteProcess);
}

BOOL CAdvancedToolsDlg::CommDriver(FORBID_TYPE nType, BOOL bDeny)
{
	BOOL bRet = FALSE;

	COMMUNICATE_FORBID cf;
	cf.OpType = enumForbidTypes;
	cf.ForbidType = nType;
	cf.bDeny = bDeny;
	bRet = m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_FORBID), NULL, 0, NULL);

	return bRet;
}

void CAdvancedToolsDlg::OnBnClickedNotCreateFile()
{
	UpdateData(TRUE);
	CommDriver(eForbid_CreateFile, m_bForbidCreteFile);
}

void CAdvancedToolsDlg::OnBnClickedNotSwitchDesktop()
{
	UpdateData(TRUE);
	CommDriver(eForbid_SwitchDesktop, m_bForbidSwitchDesktop);
}

void CAdvancedToolsDlg::OnBnClickedNotCreateKey()
{
	UpdateData(TRUE);
	CommDriver(eForbid_CreateKey, m_bForbidCreateKey);

	GetDlgItem(IDC_NOT_SET_VALUE)->EnableWindow(m_bForbidCreateKey);
	if (!m_bForbidCreateKey)
	{
		m_bForbidSetKeyValue = m_bForbidCreateKey;
	}

	UpdateData(FALSE);
}

void CAdvancedToolsDlg::OnBnClickedNotSetValue()
{
	UpdateData(TRUE);
	CommDriver(eForbid_SetKeyValue, m_bForbidSetKeyValue);
}

void CAdvancedToolsDlg::OnBnClickedNotLoadImage()
{
	UpdateData(TRUE);
	CommDriver(eForbid_CreateImage, m_bForbidLoadImage);
}

void CAdvancedToolsDlg::OnBnClickedNotSystemShutdown()
{
	UpdateData(TRUE);
	CommDriver(eForbid_Shutdown, m_bForbidSystemShutdown);
}

void CAdvancedToolsDlg::OnBnClickedNotModiftTime()
{
	UpdateData(TRUE);
	CommDriver(eForbid_ModifySystemTime, m_bForbidModifySystemTime);
}

void CAdvancedToolsDlg::OnBnClickedNotLockWorkstation()
{
	UpdateData(TRUE);
	CommDriver(eForbid_LockWorkStation, m_bForbidLockWorkStation);
}

void CAdvancedToolsDlg::OnBnClickedNotLoadDriver()
{
	UpdateData(TRUE);
	CommDriver(eForbid_LoadDriver, m_bForbidLoadDriver);
}

void CAdvancedToolsDlg::OnBnClickedNotInjectModule()
{
	UpdateData(TRUE);
	CommDriver(eForbid_InjectModule, m_bForbidInjectModule);
}

BOOL CAdvancedToolsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_STATIC_FORBID)->SetWindowText(szForbid[g_enumLang]);
	GetDlgItem(IDC_NOT_CREATE_PROCESS)->SetWindowText(szForbidCreatingProcess[g_enumLang]);
	GetDlgItem(IDC_NOT_CREATE_THREAD)->SetWindowText(szForbidCreatingThread[g_enumLang]);
	GetDlgItem(IDC_NOT_CREATE_FILE)->SetWindowText(szForbidCreatingFile[g_enumLang]);
	GetDlgItem(IDC_NOT_CREATE_KEY)->SetWindowText(szForbidCreatingKeyAndValue[g_enumLang]);
	GetDlgItem(IDC_NOT_SET_VALUE)->SetWindowText(szForbidSettingKeyValue[g_enumLang]);
	GetDlgItem(IDC_NOT_SWITCH_DESKTOP)->SetWindowText(szForbidSwitchingDesktop[g_enumLang]);
	GetDlgItem(IDC_NOT_LOAD_IMAGE)->SetWindowText(szForbidLoadingImage[g_enumLang]);
	GetDlgItem(IDC_NOT_INJECT_MODULE)->SetWindowText(szForbidGlobalMessageHook[g_enumLang]);
	GetDlgItem(IDC_NOT_LOAD_DRIVER)->SetWindowText(szForbidLoadingDriver[g_enumLang]);
	GetDlgItem(IDC_NOT_MODIFT_TIME)->SetWindowText(szForbidModifyingSystemTime[g_enumLang]);
	GetDlgItem(IDC_NOT_LOCK_WORKSTATION)->SetWindowText(szForbidLockingWorkStation[g_enumLang]);
	GetDlgItem(IDC_NOT_SYSTEM_SHUTDOWN)->SetWindowText(szForbidSystemShutdown[g_enumLang]);
	GetDlgItem(IDC_NOT_SET_VALUE)->EnableWindow(FALSE);

	GetDlgItem(IDC_STATIC_POWER)->SetWindowText(szPower[g_enumLang]);
	GetDlgItem(IDC_SYSTEM_REBOOT)->SetWindowText(szReboot[g_enumLang]);
	GetDlgItem(IDC_FORCE_SYSTEM_REBOOT)->SetWindowText(szForceReboot[g_enumLang]);
	GetDlgItem(IDC_SYSTEM_SHUTDOWN)->SetWindowText(szPowerOff[g_enumLang]);

	GetDlgItem(IDC_STATIC_ENABLE)->SetWindowText(szToolsEnable[g_enumLang]);
	GetDlgItem(IDC_ENABLE_REGISTRY_TOOLS)->SetWindowText(szEnableRegistryTools[g_enumLang]);
	GetDlgItem(IDC_ENABLE_TASK_TOOLS)->SetWindowText(szEnableTaskTools[g_enumLang]);
	GetDlgItem(IDC_ENABLE_MY_COMPUTER)->SetWindowText(szEnableCmd[g_enumLang]);
	GetDlgItem(IDC_ENABLE_CONTROL_PANE)->SetWindowText(szEnableControlPane[g_enumLang]);
	GetDlgItem(IDC_ENABLE_START_MENU)->SetWindowText(szEnableStartMenu[g_enumLang]);
	GetDlgItem(IDC_CLEAR_ALL_GLOUP_POLICIES)->SetWindowText(szClearAllGroupPolicies[g_enumLang]);
	GetDlgItem(IDC_ENABLE)->SetWindowText(szOk[g_enumLang]);
	GetDlgItem(IDC_ENABLE_RUN)->SetWindowText(szEanbleRun[g_enumLang]);

	GetDlgItem(IDC_STATIC_SAFEBOOT)->SetWindowText(szSafeboot[g_enumLang]);
	GetDlgItem(IDC_BTN_REPAIR_SAFEBOOT)->SetWindowText(szRepair[g_enumLang]);
	
	GetDlgItem(IDC_STATIC_ANTI_LOGGER)->SetWindowText(szAntiLogger[g_enumLang]);

// 	m_hCommEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
// 	if (m_hCommEvent != NULL)
// 	{
// 		BOOL bRet = FALSE;
// 		BACKUP_DRIVER_EVENT cf;
// 		cf.OpType = enumSetBackupDriverEvent;
// 		cf.hEvent = m_hCommEvent;
// 		bRet = m_Driver.CommunicateDriver(&cf, sizeof(BACKUP_DRIVER_EVENT), NULL, 0, NULL);
// 
// 		if (bRet == FALSE)
// 		{
// 			CloseHandle(m_hCommEvent);
// 			m_hCommEvent = NULL;
// 			GetDlgItem(IDC_BACKUO_DRIVER)->EnableWindow(FALSE);
// 		}
// 	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CAdvancedToolsDlg::CommDriverPower(POWER_TYPE nType)
{
	BOOL bRet = FALSE;

	COMMUNICATE_POWER cf;
	cf.OpType = enumPowerTypes;
	cf.PowerType = nType;
	bRet = m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_POWER), NULL, 0, NULL);

	return bRet;
}

void CAdvancedToolsDlg::OnBnClickedSystemReboot()
{
	UpdateData(TRUE);
	if (m_bReboot)
	{
		if (MessageBox(szAreYouSureReboot[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			m_bReboot = FALSE;
			UpdateData(FALSE);
			return;
		}
		
		CommDriverPower(ePower_Reboot);
	}
}

void CAdvancedToolsDlg::OnBnClickedForceSystemReboot()
{
	UpdateData(TRUE);
	if (m_bForceReboot)
	{
		if (MessageBox(szAreYouSureReboot[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			m_bForceReboot = FALSE;
			UpdateData(FALSE);
			return ;
		}

		CommDriverPower(ePower_Force_Reboot);
	}
}

void CAdvancedToolsDlg::OnBnClickedSystemShutdown()
{
	UpdateData(TRUE);
	if (m_bPorweOff)
	{
		if (MessageBox(szAreYouSureShutdown[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			m_bPorweOff = FALSE;
			UpdateData(FALSE);
			return ;
		}

		CommDriverPower(ePower_PowerOff);
	}
}

void CAdvancedToolsDlg::EnableTools(HKEY hRootKey, WCHAR *szSubKey, WCHAR *szValue)
{
	if (hRootKey == NULL || szSubKey == NULL || szValue == NULL)
	{
		return;
	}

	HKEY hKey = NULL;
	LONG nRet = RegOpenKeyEx(hRootKey, szSubKey, 0, KEY_ALL_ACCESS, &hKey);
	if (nRet == ERROR_SUCCESS)
	{
		DWORD dwType = 0, dwData = 0, dwRet = sizeof(DWORD), 
		nRet = RegQueryValueEx(hKey, szValue, NULL, &dwType, (PBYTE)&dwData, &dwRet);
		if (nRet == ERROR_SUCCESS/* && dwType == REG_DWORD*/ && dwData != 0)
		{
			dwData = 0;
			RegSetValueEx( hKey, szValue, NULL, dwType, (PBYTE)&dwData, sizeof(DWORD));
		}
		else if (nRet == ERROR_FILE_NOT_FOUND)
		{
			
		}

		RegCloseKey(hKey);
	}
}

void CAdvancedToolsDlg::OnBnClickedEnable()
{
	UpdateData(TRUE);
	
	if (!m_bEnableRegistry &&
		!m_bEnableTaskTools &&
		!m_bEnableCmd &&
		!m_bEnableControlPane &&
		!m_bEnableRun &&
		!m_bEnableToolbarMenu )
	{
		MessageBox(szPleaseChoseEnable[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
		return;
	}	

	WCHAR szSubKeyExploer[] = {'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','P','o','l','i','c','i','e','s','\\','E','x','p','l','o','r','e','r','\0'};
	WCHAR szSubKeySystem[] = {'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','P','o','l','i','c','i','e','s','\\','S','y','s','t','e','m','\0'};

	if (m_bEnableRegistry)
	{
		WCHAR szDisableRegistryTools[] = {'D','i','s','a','b','l','e','R','e','g','i','s','t','r','y','T','o','o','l','s','\0'};
		EnableTools(HKEY_CURRENT_USER, szSubKeySystem, szDisableRegistryTools);
	}

	if (m_bEnableTaskTools)
	{
		WCHAR szDisableTaskmgr[] = {'D','i','s','a','b','l','e','T','a','s','k','m','g','r','\0'};
		EnableTools(HKEY_CURRENT_USER, szSubKeySystem, szDisableTaskmgr);
	}

	if (m_bEnableCmd)
	{
		WCHAR szDisableCMD[] = {'D','i','s','a','b','l','e','C','M','D','\0'};
		EnableTools(HKEY_CURRENT_USER, szSubKeySystem, szDisableCMD);
	}

	if (m_bEnableControlPane)
	{
		WCHAR szNoControlPanel[] = {'N','o','C','o','n','t','r','o','l','P','a','n','e','l','\0'};
		EnableTools(HKEY_CURRENT_USER, szSubKeyExploer, szNoControlPanel);
	}

	if (m_bEnableRun)
	{
		WCHAR szNoRun[] = {'N','o','R','u','n','\0'};
		EnableTools(HKEY_CURRENT_USER, szSubKeyExploer, szNoRun);
	}

	if (m_bEnableToolbarMenu)
	{
		WCHAR szNoTrayContextMenu[] = {'N','o','T','r','a','y','C','o','n','t','e','x','t','M','e','n','u','\0'};
		EnableTools(HKEY_CURRENT_USER, szSubKeyExploer, szNoTrayContextMenu);
	}

	if (m_bClearAllGroupPolicies)
	{
		
	}

	MessageBox(szAllEnable[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);

	m_bEnableRegistry = m_bEnableTaskTools = m_bEnableCmd =
	m_bEnableControlPane = m_bEnableRun = m_bEnableToolbarMenu = FALSE;
	UpdateData(FALSE);
}

BOOL CAdvancedToolsDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

CString CAdvancedToolsDlg::ReleaseOriginRegHive()
{
	CString szRet = L"";

	ULONG nResId = 0;
	switch (g_WinVersion)
	{
	case enumWINDOWS_XP:
		nResId = IDR_RES_SAFEBOOT_XP;
		break;

	case enumWINDOWS_2K3:
	case enumWINDOWS_2K3_SP1_SP2:
		nResId = IDR_RES_SAFEBOOT_WIN2K3;
		break;

	case enumWINDOWS_VISTA:
	case enumWINDOWS_VISTA_SP1_SP2:
		nResId = IDR_RES_SAFEBOOT_VISTA;
		break;

	case enumWINDOWS_7:
		nResId = IDR_RES_SAFEBOOT_WIN7;
		break;

	case enumWINDOWS_8:
		nResId = IDR_RES_SAFEBOOT_WIN8;
		break;
	}

	WCHAR szType[] = {'R','E','S','\0'};
	szRet = m_Function.ReleaseResource(nResId, szType);
	return szRet;
}

void CAdvancedToolsDlg::OnBnClickedBtnRepairSafeboot()
{
	CString szRegHive = ReleaseOriginRegHive();
	if (szRegHive.IsEmpty())
	{
		return;
	}

	m_Function.EnablePrivilege(SE_BACKUP_NAME, TRUE);
	m_Function.EnablePrivilege(SE_RESTORE_NAME, TRUE);

	HKEY hKey = NULL;
	BOOL bRet = FALSE;
	WCHAR szSafeBoot[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','S','a','f','e','b','o','o','t','\0'};

	LONG nRet = RegCreateKey(HKEY_LOCAL_MACHINE, 
		szSafeBoot,
		&hKey);

	if (nRet == ERROR_SUCCESS)
	{
		nRet = RegRestoreKey(hKey, szRegHive, REG_FORCE_RESTORE);
		if (nRet == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}

		RegCloseKey(hKey);
	}

	m_Function.EnablePrivilege(SE_BACKUP_NAME, FALSE);
	m_Function.EnablePrivilege(SE_RESTORE_NAME, FALSE);

	DeleteFile(szRegHive);

	if (bRet)
	{
		MessageBox(szRepairSafebootOK[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(szRepairSafebootFailed[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
	}
}

void CAdvancedToolsDlg::OnDestroy()
{
	CDialog::OnDestroy();

	UpdateData(TRUE);

	if (m_bForbidCreteThread)
	{
		CommDriver(eForbid_CreateThread, FALSE);
	}

	if (m_bForbidCreteProcess)
	{
		CommDriver(eForbid_CreateProcess, FALSE);
	}

	if (m_bForbidCreteFile)
	{
		CommDriver(eForbid_CreateFile, FALSE);
	}

	if (m_bForbidSwitchDesktop)
	{
		CommDriver(eForbid_SwitchDesktop, FALSE);
	}

	if (m_bForbidCreateKey)
	{
		CommDriver(eForbid_CreateKey, FALSE);
	}

	if (m_bForbidSetKeyValue)
	{
		CommDriver(eForbid_SetKeyValue, FALSE);
	}

	if (m_bForbidLoadImage)
	{
		CommDriver(eForbid_CreateImage, FALSE);
	}

	if (m_bForbidSystemShutdown)
	{
		CommDriver(eForbid_Shutdown, FALSE);
	}

	if (m_bForbidModifySystemTime)
	{
		CommDriver(eForbid_ModifySystemTime, FALSE);
	}

	if (m_bForbidLockWorkStation)
	{
		CommDriver(eForbid_LockWorkStation, FALSE);
	}

	if (m_bBackupDriver)
	{
		CommDriver(eForbid_BackupDriver, FALSE);
	}

	if (m_bForbidLoadDriver)
	{
		CommDriver(eForbid_LoadDriver, FALSE);
	}

	if (m_bForbidInjectModule)
	{
		CommDriver(eForbid_InjectModule, FALSE);
	}

	if (m_bForbidScreenCapture)
	{
		CommDriver(eForbid_ScreenCapture, FALSE);
	}
}

void CAdvancedToolsDlg::OnBnClickedNotScreenCapture()
{
	UpdateData(TRUE);
	CommDriver(eForbid_ScreenCapture, m_bForbidScreenCapture);
}

void CAdvancedToolsDlg::OnBnClickedBackuoDriver()
{
	UpdateData(TRUE);
	
	if (m_bBackupDriver)
	{
		WCHAR szPath[MAX_PATH] = {0};
		GetModuleFileName(NULL, szPath, MAX_PATH - 1);
		CString strPath = szPath;
		strPath = strPath.Left(strPath.ReverseFind('\\'));
		BOOL bRet = FALSE;

		WCHAR szBackup[] = {'\\','B','a','c','k','u','p','D','r','i','v','e','r','\0'};
		strPath += szBackup;

		if (!PathFileExists(strPath))
		{
			bRet = CreateDirectory(strPath, NULL);
		}
		else
		{
			bRet = TRUE;
		}

		WCHAR* szBackupDriverPath = m_Function.CString2WString(strPath);
		if (bRet && szBackupDriverPath)
		{
			BACKUP_DRIVER_PATH cf;
			cf.OpType = enumSetBackupDriverPath;
			cf.szPath = szBackupDriverPath;
			cf.dwBytes = wcslen(szBackupDriverPath) * sizeof(WCHAR);
			bRet = m_Driver.CommunicateDriver(&cf, sizeof(BACKUP_DRIVER_PATH), NULL, 0, NULL);
		}
		else
		{
			bRet= FALSE;
		}

		if (szBackupDriverPath)
		{
			free(szBackupDriverPath);
			szBackupDriverPath = NULL;
		}

		if (bRet == FALSE)
		{
			m_bBackupDriver = FALSE;
			UpdateData(FALSE);
			return;
		}
	}

// 	CommDriver(eForbid_BackupDriver, m_bBackupDriver);
}
