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
// ServiceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ServiceDlg.h"
#include <algorithm>


// CServiceDlg 对话框

IMPLEMENT_DYNAMIC(CServiceDlg, CDialog)

CServiceDlg::CServiceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServiceDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_nServicesReturned = 0;
	m_pServices = NULL;
	m_ServiceList.clear();
	m_bShowAll = FALSE;
}

CServiceDlg::~CServiceDlg()
{
	if (m_pServices)
	{
		free(m_pServices);
		m_pServices = NULL;
	}

	m_ServiceList.clear();
}

void CServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_SERVICE_CNT, m_szStatus);
}


BEGIN_MESSAGE_MAP(CServiceDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CServiceDlg::OnBnClickedOk)
	ON_MESSAGE(WM_VERIFY_SIGN_OVER, OnCloseWindow)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CServiceDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CServiceDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_SERVICE_REFRESH, &CServiceDlg::OnServiceRefresh)
	ON_COMMAND(ID_SERVICE_START, &CServiceDlg::OnServiceStart)
	ON_COMMAND(ID_SERVICE_STOP, &CServiceDlg::OnServiceStop)
	ON_COMMAND(ID_SERVICE_RESTART, &CServiceDlg::OnServiceRestart)
	ON_COMMAND(ID_SERVICE_AUTO, &CServiceDlg::OnServiceAuto)
	ON_COMMAND(ID_SERVICE_MAD, &CServiceDlg::OnServiceMad)
	ON_COMMAND(ID_SERVICE_DISABLED, &CServiceDlg::OnServiceDisabled)
	ON_COMMAND(ID_SERVICE_DELETE, &CServiceDlg::OnServiceDelete)
	ON_COMMAND(ID_SERVICE_SHUXING, &CServiceDlg::OnServiceShuxing)
	ON_COMMAND(ID_SERVICE_LOCATE_MODULE, &CServiceDlg::OnServiceLocateModule)
	ON_COMMAND(ID_SERVICE_VERIFY_SIGN, &CServiceDlg::OnServiceVerifySign)
	ON_COMMAND(ID_SERVICE_TEXT, &CServiceDlg::OnServiceText)
	ON_COMMAND(ID_SERVICE_EXCEL, &CServiceDlg::OnServiceExcel)
	ON_COMMAND(ID_SERVICE_DELETE_REG_AND_FILE, &CServiceDlg::OnServiceDeleteRegAndFile)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_SERVICE_JMP_TO_REG, &CServiceDlg::OnServiceJmpToReg)
	ON_COMMAND(ID_SERVICE_NOT_SHOW_MICROSOFT_MODULE, &CServiceDlg::OnServiceNotShowMicrosoftModule)
	ON_UPDATE_COMMAND_UI(ID_SERVICE_NOT_SHOW_MICROSOFT_MODULE, &CServiceDlg::OnUpdateServiceNotShowMicrosoftModule)
	ON_COMMAND(ID_SERVICE_VERIFY_ALL_SIGN, &CServiceDlg::OnServiceVerifyAllSign)
	ON_COMMAND(ID_SERVICE_BOOT, &CServiceDlg::OnServiceBoot)
	ON_COMMAND(ID_SERVICE_SYSTEM, &CServiceDlg::OnServiceSystem)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CServiceDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CServiceDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_SERVICE_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CServiceDlg 消息处理程序

void CServiceDlg::OnBnClickedOk()
{
}

void CServiceDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CServiceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ProImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_ProImageList, LVSIL_SMALL);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szName[g_enumLang], LVCFMT_LEFT, 140);
	m_list.InsertColumn(1, szDispalyName[g_enumLang], LVCFMT_LEFT, 190);
	m_list.InsertColumn(2, szDescription[g_enumLang], LVCFMT_LEFT, 220);
	m_list.InsertColumn(3, szStatus[g_enumLang], LVCFMT_LEFT, 70);
	m_list.InsertColumn(4, szStartupType[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(5, szServiceModule[g_enumLang], LVCFMT_LEFT, 320);
	m_list.InsertColumn(6, szFileCorporation[g_enumLang], LVCFMT_LEFT, 150);

	m_hProcessIcon = reinterpret_cast<HICON>(::LoadImage ( AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PROCESS_ICON),IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR ));

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

CString CServiceDlg::GetServiceDll(CString szKey)
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

CString CServiceDlg::GetServiceModule(CString szKey)
{
	CString szModule;

	if (!szKey.IsEmpty())
	{
		CString szParameters = REG_SERVICE;
		szParameters += szKey;
		szParameters += L"\\Parameters";
		
		szModule = GetServiceDll(szParameters);
		
		if (szModule.IsEmpty())
		{
			CString szImagePath = REG_SERVICE;
			szImagePath += szKey;

			szModule = GetImagePath(szImagePath);
		}
	}

	return szModule;
}

CString CServiceDlg::GetImagePath(CString szKey)
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

CString CServiceDlg::GetServiceCurrentStatus(ULONG nStatus)
{
	CString szStatus;

	switch(nStatus)
	{
	case SERVICE_PAUSED:  
		break;

	case SERVICE_RUNNING:
		szStatus = szStarted[g_enumLang];
		break;

	case SERVICE_STOPPED:
		break;

	case SERVICE_CONTINUE_PENDING:
		break;

	case SERVICE_PAUSE_PENDING:
		break;

	case SERVICE_START_PENDING:
		break;

	case SERVICE_STOP_PENDING:
		break;

	default:
		break;
	}
	
	return szStatus;
}

BOOL CServiceDlg::GetStartType(CString szKey, DWORD *dwType)
{
	BOOL bRet = FALSE;

	if (szKey.IsEmpty() || dwType == NULL)
	{
		return bRet;
	}

	szKey = REG_SERVICE + CString(L"\\") + szKey;
	WCHAR szStart[] = {'S','t','a','r','t','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);
	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szStart) * sizeof(WCHAR) &&
			!ir->szName.CompareNoCase(szStart))
		{
			*dwType = *(PULONG)ir->pData;
			bRet = TRUE;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return bRet;
}

CString CServiceDlg::GetStartTypeString(CString szKey)
{
	CString szType;

	if (!szKey.IsEmpty())
	{
		DWORD dwType = 0;

		if (GetStartType(szKey, &dwType))
		{
			switch (dwType)
			{
			case SERVICE_AUTO_START:
				szType = szAutoStart[g_enumLang];
				break;

			case SERVICE_BOOT_START:
				szType = szBoot[g_enumLang];
				break;

			case SERVICE_DEMAND_START:
				szType = szManual[g_enumLang];
				break;

			case SERVICE_DISABLED:
				szType = szDisabled[g_enumLang];
				break;

			case SERVICE_SYSTEM_START:
				szType = szSystemType[g_enumLang];
				break;
			}
		}
	}	

	return szType;
}

void CServiceDlg::EnumServers()
{
	m_SignVerifyDlg.m_NotSignDataList.clear();
	m_SignVerifyDlg.m_NotSignItemList.clear();

	m_szStatus.Format(L"%s%d", szServiceCnt[g_enumLang], 0);
	UpdateData(FALSE);
	m_list.DeleteAllItems();
	m_ServiceNameList.clear();

	if (!g_bLoadDriverOK)
	{
		return;
	}

	int nImageCount = m_ProImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ProImageList.Remove(0); 
	}

	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schSCManager) 
	{
		return;
	}

	m_nServicesReturned = 0;
	DWORD nBytesNeeded = 0;
	BOOL bStatus = EnumServicesStatus(schSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &nBytesNeeded, &m_nServicesReturned, NULL);

	if (!bStatus && nBytesNeeded > 0)
	{
		if (m_pServices)
		{
			free(m_pServices);
			m_pServices = NULL;
		}

		m_pServices = (LPENUM_SERVICE_STATUS)malloc(nBytesNeeded + sizeof(ENUM_SERVICE_STATUS));
		if (!m_pServices)
		{
			CloseServiceHandle(schSCManager);
			return;
		}

		memset(m_pServices, 0, nBytesNeeded + sizeof(ENUM_SERVICE_STATUS));
		bStatus = EnumServicesStatus(schSCManager, 
			SERVICE_WIN32, 
			SERVICE_STATE_ALL, 
			m_pServices, 
			nBytesNeeded + sizeof(ENUM_SERVICE_STATUS), 
			&nBytesNeeded, 
			&m_nServicesReturned, 
			NULL);

		if(bStatus)
		{
			if (m_bShowAll)
			{
				for(DWORD i = 0; i < m_nServicesReturned; i++)
				{
					CString szModule = GetServiceModule(m_pServices[i].lpServiceName);

					SHFILEINFO sfi;
					DWORD_PTR nRet;
					ZeroMemory(&sfi, sizeof(SHFILEINFO));
					nRet = SHGetFileInfo(szModule, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
					nRet ? m_ProImageList.Add ( sfi.hIcon ) : m_ProImageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
					if (nRet)
					{
						DestroyIcon(sfi.hIcon);
					}

					int n = m_list.InsertItem(m_list.GetItemCount(), m_pServices[i].lpServiceName, i);
					m_list.SetItemText(n ,1, m_pServices[i].lpDisplayName);
					m_list.SetItemText(n, 2, m_Function.GetFileDescription(szModule));
					m_list.SetItemText(n, 3, GetServiceCurrentStatus(m_pServices[i].ServiceStatus.dwCurrentState));
					m_list.SetItemText(n, 4, GetStartTypeString(m_pServices[i].lpServiceName));
					m_list.SetItemText(n, 5, szModule);
					m_list.SetItemText(n, 6, m_Function.GetFileCompanyName(szModule));
					
					m_list.SetItemData(n, n);
					m_ServiceNameList.push_back(m_pServices[i].lpServiceName);
				}

				m_szStatus.Format(L"%s%d", szServiceCnt[g_enumLang], m_nServicesReturned);
			}
			else
			{
				for(DWORD i = 0; i < m_nServicesReturned; i++)
				{
					CString szModule = GetServiceModule(m_pServices[i].lpServiceName);
					if (!m_Function.IsMicrosoftAppByPath(szModule))
					{
						SHFILEINFO sfi;
						DWORD_PTR nRet;
						ZeroMemory(&sfi, sizeof(SHFILEINFO));
						nRet = SHGetFileInfo(szModule, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
						nRet ? m_ProImageList.Add ( sfi.hIcon ) : m_ProImageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
						if (nRet)
						{
							DestroyIcon(sfi.hIcon);
						}

						int n = m_list.GetItemCount();
						m_list.InsertItem(m_list.GetItemCount(), m_pServices[i].lpServiceName, n);
						m_list.SetItemText(n ,1, m_pServices[i].lpDisplayName);
						m_list.SetItemText(n, 2, m_Function.GetFileDescription(szModule));
						m_list.SetItemText(n, 3, GetServiceCurrentStatus(m_pServices[i].ServiceStatus.dwCurrentState));
						m_list.SetItemText(n, 4, GetStartTypeString(m_pServices[i].lpServiceName));
						m_list.SetItemText(n, 5, szModule);
						m_list.SetItemText(n, 6, m_Function.GetFileCompanyName(szModule));

						m_list.SetItemData(n, n);
						m_ServiceNameList.push_back(m_pServices[i].lpServiceName);
					}
				}

				m_szStatus.Format(L"%s%d/%d", szServiceCnt[g_enumLang], m_list.GetItemCount(), m_nServicesReturned);
			}

			UpdateData(FALSE);
		}
	}
	
	CloseServiceHandle(schSCManager);
}


CString CServiceDlg::GetDispalyName(CString szKey)
{
	CString szDisplayName = L"";

	if ( szKey.IsEmpty() )
	{
		return szDisplayName;
	}

	szKey = REG_SERVICE + szKey;
	WCHAR szDispay[] = {'D','i','s','p','l','a','y','N','a','m','e','\0'};

	list<VALUE_INFO> valueList;
	m_Registry.EnumValues(szKey, valueList);

	for (list<VALUE_INFO>::iterator ir = valueList.begin();
		ir != valueList.end();
		ir++)
	{
		if (ir->nNameLen == wcslen(szDispay) * sizeof(WCHAR) && 
			!ir->szName.CompareNoCase(szDispay))
		{
			szDisplayName = (WCHAR*)ir->pData;
			break;
		}
	}

	m_Registry.FreeValueList(valueList);
	return szDisplayName;
}

BOOL CServiceDlg::IsServiceStarted(CString szDisplayName, int *pStarted)
{
	BOOL bRet = FALSE;

	if (!szDisplayName.IsEmpty() && pStarted && m_pServices)
	{
		for(DWORD i = 0; i < m_nServicesReturned; i++)
		{
			if (!szDisplayName.CompareNoCase(m_pServices[i].lpDisplayName))
			{
				SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

				if (hSCManager)
				{
					SC_HANDLE hService = 
						OpenService(hSCManager, 
						m_pServices[i].lpServiceName, 
						SERVICE_ALL_ACCESS);

					if (hService)
					{
						SERVICE_STATUS st;
						if (QueryServiceStatus(hService, &st))
						{
							if (st.dwCurrentState == SERVICE_STOPPED)
							{
								*pStarted = 0;
							}
							else if (st.dwCurrentState == SERVICE_RUNNING)
							{
								*pStarted = 1;
							}

							bRet = TRUE;
						}

						CloseServiceHandle(hService);
					}

					CloseServiceHandle(hSCManager);
				}

				break;
			}
		}
	}

	return bRet;
}

void CServiceDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_SERVICE_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_SERVICE_EXCEL, szExcel[g_enumLang]);

	CMenu starttype;
	starttype.CreatePopupMenu();
	starttype.AppendMenu(MF_STRING, ID_SERVICE_BOOT, szBootStart[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_SYSTEM, szSystemStart[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_AUTO, szServiceAutomatic[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_MAD, szServiceManual[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_DISABLED, szServiceDisabled[g_enumLang]);
	
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SERVICE_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SERVICE_NOT_SHOW_MICROSOFT_MODULE, szHideMicrosoftService[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SERVICE_START, szServiceStart[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SERVICE_STOP, szServiceStop[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SERVICE_RESTART, szServiceRestart[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)starttype.m_hMenu, szStartType[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SERVICE_JMP_TO_REG, szLocateInRegedit[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SERVICE_DELETE, szServiceDeleteRegistry[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SERVICE_DELETE_REG_AND_FILE, szServiceDeleteRegistryAndFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SERVICE_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SERVICE_LOCATE_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SERVICE_VERIFY_SIGN, szVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SERVICE_VERIFY_ALL_SIGN, szVerifyAllModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_SERVICE_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_SERVICE_JMP_TO_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(ID_SERVICE_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_SERVICE_DELETE_REG_AND_FILE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_SERVICE_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_SERVICE_LOCATE_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_SERVICE_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_SERVICE_VERIFY_ALL_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(22, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount()) // 如果m_list是空的，那么除了“刷新”，其他全部置灰
	{
		for (int i = 2; i < 23; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Function.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				int dwStatus = -1;

				if (IsServiceStarted(m_list.GetItemText(nItem, 1), &dwStatus))
				{
					switch (dwStatus)
					{
					case -1:
						menu.EnableMenuItem(ID_SERVICE_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						menu.EnableMenuItem(ID_SERVICE_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						menu.EnableMenuItem(ID_SERVICE_RESTART, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						break;

					case 0:
						menu.EnableMenuItem(ID_SERVICE_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						menu.EnableMenuItem(ID_SERVICE_RESTART, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						break;

					case 1:
						menu.EnableMenuItem(ID_SERVICE_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
						break;
					}				
				}
				else
				{
					menu.EnableMenuItem(ID_SERVICE_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_SERVICE_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_SERVICE_RESTART, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				CString szStartup = m_list.GetItemText(nItem, 4);
				if (!szStartup.CompareNoCase(szAutoStart[g_enumLang]))
				{
					menu.CheckMenuItem(ID_SERVICE_AUTO, MF_BYCOMMAND | MF_CHECKED);
					menu.EnableMenuItem(ID_SERVICE_AUTO, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);	
				}
				else if (!szStartup.CompareNoCase(szManual[g_enumLang]))
				{
					menu.CheckMenuItem(ID_SERVICE_MAD, MF_BYCOMMAND | MF_CHECKED);
					menu.EnableMenuItem(ID_SERVICE_MAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
				else if (!szStartup.CompareNoCase(szDisabled[g_enumLang]))
				{
					menu.CheckMenuItem(ID_SERVICE_DISABLED, MF_BYCOMMAND | MF_CHECKED);
					menu.EnableMenuItem(ID_SERVICE_DISABLED, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
				else if (!szStartup.CompareNoCase(szSystemType[g_enumLang]))
				{
					menu.CheckMenuItem(ID_SERVICE_SYSTEM, MF_BYCOMMAND | MF_CHECKED);
					menu.EnableMenuItem(ID_SERVICE_SYSTEM, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
				else if (!szStartup.CompareNoCase(szBoot[g_enumLang]))
				{
					menu.CheckMenuItem(ID_SERVICE_BOOT, MF_BYCOMMAND | MF_CHECKED);
					menu.EnableMenuItem(ID_SERVICE_BOOT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}
		else
		{
			for (int i = 2; i < 22; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}

		menu.EnableMenuItem(ID_SERVICE_VERIFY_ALL_SIGN, MF_BYCOMMAND | MF_ENABLED);
	}

	menu.EnableMenuItem(ID_SERVICE_NOT_SHOW_MICROSOFT_MODULE, MF_BYCOMMAND | MF_ENABLED);

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();
	starttype.DestroyMenu();

	*pResult = 0;
}

void CServiceDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		CString szCompany = m_list.GetItemText(nItem, 6);
		if (IsNotSignItemData((ULONG)m_list.GetItemData(nItem)))
		{
			clrNewTextColor = g_NotSignedItemClr;//RGB( 180, 0, 255 );
		}
		else if (!m_Function.IsMicrosoftApp(szCompany))
		{
			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CServiceDlg::OnServiceRefresh()
{
	EnumServers();
}

BOOL CServiceDlg::ServiceControl(CString szDisplayName, int ControlType)
{
	BOOL bRet = FALSE;

	if (!szDisplayName.IsEmpty() && m_pServices)
	{
		for(DWORD i = 0; i < m_nServicesReturned; i++)
		{
			if (!szDisplayName.CompareNoCase(m_pServices[i].lpDisplayName))
			{
				SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

				if (hSCManager)
				{
					SC_HANDLE hService = 
						OpenService(hSCManager, 
						m_pServices[i].lpServiceName, 
						SERVICE_ALL_ACCESS);

					if (hService)
					{
						switch (ControlType)
						{
						case 1:
							{
								bRet = StartService(hService, NULL, NULL);
							}
							break;

						case 2:
							{
								SERVICE_STATUS st;
								bRet = ControlService(hService, SERVICE_CONTROL_STOP, &st);
							}
							break;

						case 3:
							{
								SERVICE_STATUS st;
								bRet = ControlService(hService, SERVICE_CONTROL_STOP, &st);
								if (bRet)
								{
									bRet = StartService(hService, NULL, NULL);
								}
							}
							break;
						}

						CloseServiceHandle(hService);
					}

					CloseServiceHandle(hSCManager);
				}

				break;
			}
		}
	}

	return bRet;
}

void CServiceDlg::OnServiceStart()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ServiceControl(szService, 1))
		{
			m_list.SetItemText(nItem, 3, szStarted[g_enumLang]);
		}
	}
}

void CServiceDlg::OnServiceStop()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ServiceControl(szService, 2))
		{
			m_list.SetItemText(nItem, 3, L"");
		}
	}
}

void CServiceDlg::OnServiceRestart()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ServiceControl(szService, 3))
		{
		//	m_list.SetItemText(nItem, 1, szStarted[g_enumLang])
		}
	}
}

BOOL CServiceDlg::ModifyStartupType(CString szServiceName, ULONG Type)
{
	BOOL bRet = FALSE;

	if (!szServiceName.IsEmpty() && m_pServices)
	{
		for(DWORD i = 0; i < m_nServicesReturned; i++)
		{
			if (!szServiceName.CompareNoCase(m_pServices[i].lpDisplayName))
			{
				WCHAR szStart[] = {'S','t','a','r','t','\0'};
				CString szKeyPath = REG_SERVICE;
				szKeyPath += m_pServices[i].lpServiceName;
				bRet = m_Registry.ModifyValue(szKeyPath, szStart, REG_DWORD, &Type, sizeof(DWORD));
				break;
			}
		}
	}

	return bRet;
}

void CServiceDlg::OnServiceAuto()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ModifyStartupType(szService, SERVICE_AUTO_START))
		{
			m_list.SetItemText(nItem, 4, szAutoStart[g_enumLang]);
		}
	}
}

void CServiceDlg::OnServiceMad()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ModifyStartupType(szService, SERVICE_DEMAND_START))
		{
			m_list.SetItemText(nItem, 4, szManual[g_enumLang]);
		}
	}
}

void CServiceDlg::OnServiceDisabled()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ModifyStartupType(szService, SERVICE_DISABLED))
		{
			m_list.SetItemText(nItem, 4, szDisabled[g_enumLang]);
		}
	}
}

void CServiceDlg::OnServiceShuxing()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Function.OnCheckAttribute(szPath);
	}
}

void CServiceDlg::OnServiceLocateModule()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Function.LocationExplorer(szPath);
	}
}

void CServiceDlg::OnServiceVerifySign()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Function.SignVerify(szPath);
	}
}

void CServiceDlg::OnServiceText()
{
	m_Function.ExportListToTxt(&m_list, m_szStatus);
}

void CServiceDlg::OnServiceExcel()
{
	WCHAR szService[] = {'S','e','r','v','i','c','e','\0'};
	m_Function.ExportListToExcel(&m_list, szService, m_szStatus);
}

BOOL CServiceDlg::DeleteService(CString szServiceName)
{
	BOOL bRet = FALSE;
	
	if (!szServiceName.IsEmpty() && m_pServices)
	{
		for(DWORD i = 0; i < m_nServicesReturned; i++)
		{
			if (!szServiceName.CompareNoCase(m_pServices[i].lpDisplayName))
			{
				SC_HANDLE schSCManager;
				SC_HANDLE schService;

				schSCManager = OpenSCManager( 
					NULL,                    // local computer
					NULL,                    // ServicesActive database 
					SC_MANAGER_ALL_ACCESS);  // full access rights 

				if (NULL == schSCManager) 
				{
					return FALSE;
				}

				// Get a handle to the service.

				schService = OpenService( 
					schSCManager,       // SCM database 
					m_pServices[i].lpServiceName,          // name of service 
					DELETE);            // need delete access 

				if (schService == NULL)
				{ 
					CloseServiceHandle(schSCManager);
					return FALSE;
				}

				ServiceControl(szServiceName, 2);
				if (! ::DeleteService(schService) ) 
				{
					bRet = FALSE; 
				}
				else 
				{
					bRet = TRUE;
				} 
				
				// Delete the service.

				CloseServiceHandle(schService); 
				CloseServiceHandle(schSCManager);
				break;
			}
		}
	}

	return bRet;
}

void CServiceDlg::OnServiceDelete()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szService[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			return;
		}

		CString szService = m_list.GetItemText(nItem, 1);
		if (DeleteService(szService))
		{
			m_list.DeleteItem(nItem);
		}
	}
}

void CServiceDlg::OnServiceDeleteRegAndFile()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szService[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			return;
		}
		
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Function.KernelDeleteFile(szPath);

		CString szService = m_list.GetItemText(nItem, 1);
		if (DeleteService(szService))
		{
			m_list.DeleteItem(nItem);
		}
	}
}

void CServiceDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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
		for(UINT i=0; i < count; i++)  
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

void CServiceDlg::OnServiceJmpToReg()
{
	int iItem = m_Function.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szServiceName = m_ServiceNameList.at(m_list.GetItemData(iItem));
		if (!szServiceName.IsEmpty())
		{
			CString szKey = SERVICE_KEY_NAME;
			szKey += L"\\";
			szKey += szServiceName;
			m_Function.JmpToMyRegistry(szKey, NULL);	
		}
	}
}

void CServiceDlg::OnServiceNotShowMicrosoftModule()
{
	m_bShowAll = !m_bShowAll;
	EnumServers();
}

void CServiceDlg::OnUpdateServiceNotShowMicrosoftModule(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(!m_bShowAll);
}

void CServiceDlg::OnServiceVerifyAllSign()
{
	m_SignVerifyDlg.m_NotSignDataList.clear();
	m_SignVerifyDlg.m_NotSignItemList.clear();
	m_SignVerifyDlg.m_bSingle = FALSE;
	m_SignVerifyDlg.m_pList = &m_list;
	m_SignVerifyDlg.m_nPathSubItem = 5;
	m_SignVerifyDlg.m_hWinWnd = this->m_hWnd;
	m_SignVerifyDlg.DoModal();
}

BOOL CServiceDlg::IsNotSignItem(ULONG nItem)
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

VOID CServiceDlg::SortItem()
{
	ULONG nCnt = m_list.GetItemCount();

	for (ULONG i = 0; i < nCnt; i++)
	{
		if (IsNotSignItem(i))
		{
			// 设置是否Vrified
			CString szComp = m_list.GetItemText(i, 6);
			CString szCompTemp = szNotVerified[g_enumLang] + szComp;
			m_list.SetItemText(i, 6, szCompTemp);
		}
		else
		{
			// 设置是否Vrified
			CString szComp = m_list.GetItemText(i, 6);
			CString szCompTemp = szVerified[g_enumLang] + szComp;
			m_list.SetItemText(i, 6, szCompTemp);
		}
	}
}

LRESULT CServiceDlg::OnCloseWindow(WPARAM wParam, LPARAM lParam)
{
	SortItem();
	return 0;
}

BOOL CServiceDlg::IsNotSignItemData(ULONG nItem)
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

// 设置为boot方式启动
void CServiceDlg::OnServiceBoot()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ModifyStartupType(szService, SERVICE_BOOT_START))
		{
			m_list.SetItemText(nItem, 4, szBoot[g_enumLang]);
		}
	}
}

// 设置为系统启动
void CServiceDlg::OnServiceSystem()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, 1);
		if (ModifyStartupType(szService, SERVICE_SYSTEM_START))
		{
			m_list.SetItemText(nItem, 4, szSystemType[g_enumLang]);
		}
	}
}

void CServiceDlg::JmpToService(CString szService)
{
	m_bShowAll = TRUE;
	EnumServers();

	DWORD dwCnt = m_list.GetItemCount();
	for (DWORD i = 0; i < dwCnt; i++)
	{
		int nData = (int)m_list.GetItemData(i);
		CString name = m_ServiceNameList.at(nData);
		if (!name.CompareNoCase(szService))
		{
			m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);
			m_list.SetFocus();
			break;
		}
	}
}

BOOL CServiceDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CServiceDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 5);
		m_Function.JmpToFile(szPath);
	}
}