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
// ListDriverDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ListDriverDlg.h"
#include <algorithm>
#include <shlwapi.h>
// CListDriverDlg 对话框

typedef enum _DRIVER_HEADER_INDEX
{
	eDriverName,
	eDriverBase,
	eDriverSize,
	eDriverObject,
	eDriverPath,
	eDriverServiceName,
	eDriverStartupType,
	eDriverLoadOrder,
	eDriverFileCorporation
}DRIVER_HEADER_INDEX;

IMPLEMENT_DYNAMIC(CListDriverDlg, CDialog)

CListDriverDlg::CListDriverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CListDriverDlg::IDD, pParent)
	, m_Status(_T(""))
{
}

CListDriverDlg::~CListDriverDlg()
{
	m_vectorDrivers.clear();
	m_vectorRing3Drivers.clear();
	m_vectorServiceKeys.clear();
}

void CListDriverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_DRIVER_CNT, m_Status);
}


BEGIN_MESSAGE_MAP(CListDriverDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CListDriverDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_DRIVER_REFRESH, &CListDriverDlg::OnDriverRefresh)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CListDriverDlg::OnNMRclickList)
	ON_COMMAND(ID_UNLOAD_DRIVER, &CListDriverDlg::OnUnloadDriver)
	ON_COMMAND(ID_DELETE_DRIVER_FILE, &CListDriverDlg::OnDeleteDriverFile)
	ON_COMMAND(ID_DELETE_DRIVER_FILE_AND_REG, &CListDriverDlg::OnDeleteDriverFileAndReg)
	ON_COMMAND(ID_COPY_DRIVER_MEMORY, &CListDriverDlg::OnCopyDriverMemory)
	ON_COMMAND(ID_DRIVER_SIGN_CHECK, &CListDriverDlg::OnDriverSignCheck)
	ON_COMMAND(ID_DRIVER_ALL_SIGN_CHECK, &CListDriverDlg::OnDriverAllSignCheck)
	ON_COMMAND(ID_COPY_DRIVER_NAME, &CListDriverDlg::OnCopyDriverName)
	ON_COMMAND(ID_COPY_DRIVER_PATH, &CListDriverDlg::OnCopyDriverPath)
	ON_COMMAND(ID_LOCATE_REG, &CListDriverDlg::OnLocateReg)
	ON_COMMAND(ID_FIND_DRIVER, &CListDriverDlg::OnFindDriver)
	ON_COMMAND(ID_DRIVER_SHUXING, &CListDriverDlg::OnDriverShuxing)
	ON_COMMAND(ID_DRIVER_SEARCH, &CListDriverDlg::OnDriverSearch)
	ON_COMMAND(ID_DRIVER_TEXT, &CListDriverDlg::OnDriverText)
	ON_COMMAND(ID_DRIVER_EXCEL, &CListDriverDlg::OnDriverExcel)
	ON_MESSAGE(WM_VERIFY_SIGN_OVER, SignVerifyOver)
	ON_COMMAND(ID_SERVICE_BOOT, &CListDriverDlg::OnServiceBoot)
	ON_COMMAND(ID_SERVICE_SYSTEM, &CListDriverDlg::OnServiceSystem)
	ON_COMMAND(ID_SERVICE_AUTO, &CListDriverDlg::OnServiceAuto)
	ON_COMMAND(ID_SERVICE_MAD, &CListDriverDlg::OnServiceMad)
	ON_COMMAND(ID_SERVICE_DISABLED, &CListDriverDlg::OnServiceDisabled)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CListDriverDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CListDriverDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_DRIVER_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CListDriverDlg 消息处理程序

BOOL CListDriverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(eDriverName, szDriverName[g_enumLang], LVCFMT_LEFT, 120);
	m_list.InsertColumn(eDriverBase, szBase[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eDriverSize, szSize[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eDriverObject, szDriverObject[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(eDriverPath, szDriverPath[g_enumLang], LVCFMT_LEFT, 270);
	m_list.InsertColumn(eDriverServiceName, szServiceName[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(eDriverStartupType, szStartupType[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(eDriverLoadOrder, szLoadOrder[g_enumLang], LVCFMT_CENTER, 70);
	m_list.InsertColumn(eDriverFileCorporation, szFileCorporation[g_enumLang], LVCFMT_LEFT, 140);
	
	m_Status.Format(szDriverStatus[g_enumLang], 0, 0, 0);
	UpdateData(FALSE);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CListDriverDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

//
// 枚举驱动
//
void CListDriverDlg::ListDrivers()
{
	m_Status.Format(szDriverStatus[g_enumLang], 0, 0, 0);
	UpdateData(FALSE);

	// 清零
	m_vectorDrivers.clear();
	m_list.DeleteAllItems();
	m_vectorColor.clear();
	m_vectorRing3Drivers.clear();
	m_vectorServiceKeys.clear();

	// 枚举
	m_clsDriver.ListDrivers(m_vectorDrivers);
	if (m_vectorDrivers.empty())
	{
		return;
	}

	// 插入驱动项
	InsertDriverIntem();
}

//
// 与Ring3枚举的结果对比, 判断驱动模块是否是隐藏的
//
BOOL CListDriverDlg::IsHideDriver(ULONG Base)
{
	BOOL bHide = TRUE;

	for (vector<ULONG>::iterator itor = m_vectorRing3Drivers.begin(); 
		itor != m_vectorRing3Drivers.end(); 
		itor++)
	{
		if (*itor == Base)
		{
			bHide = FALSE;
			break;
		}
	}

	return bHide;
}

//
// 根据模块文件获得service name
//
CString CListDriverDlg::GetServiceName(CString szPath)
{
	CString szServiceName = L"";

	if (!szPath.IsEmpty())
	{
		for (list<SERVICE_KEY>::iterator itor = m_vectorServiceKeys.begin(); itor != m_vectorServiceKeys.end(); itor++)
		{
			if (!szPath.CompareNoCase(itor->szPath))
			{
				szServiceName = itor->szKey;
				break;
			}
		}
	}

	return szServiceName;
}

//
// 获取驱动文件的启动类型
//
CString CListDriverDlg::GetDriverStartupType(CString szService)
{
	CString szType = L"-";
	
	if (!szService.IsEmpty())
	{
		DWORD dwType = 0;
		if (GetStartType(szService, &dwType))
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

//
// 插入驱动项
//
void CListDriverDlg::InsertDriverIntem()
{
	// ring3调用ZwQuerySystemInformation函数获得系统模块
	Ring3GetDriverModules();

	// 枚举services键值,获得系统中全部服务信息
	GetServiceKeys();

	ULONG nHideDriverCnt = 0;

	for (vector<DRIVER_INFO>::iterator itor = m_vectorDrivers.begin(); 
		itor != m_vectorDrivers.end(); 
		itor++)
	{
		ITEM_COLOR item_clr = enumBlack;

		CString szBase, szSize, szDriverObject, szServiceName, szLoadOrder, szComp, szStartup = L"-";
		CString szDriverPath = itor->szDriverPath;
		CString szDriverName = szDriverPath.Right(szDriverPath.GetLength() - szDriverPath.ReverseFind('\\') - 1);
		szBase.Format(L"0x%08X", itor->nBase);
		szSize.Format(L"0x%08X", itor->nSize);
		szLoadOrder.Format(L"%d", itor->nLodeOrder);
		szServiceName = GetServiceName(szDriverPath);
		szStartup = GetDriverStartupType(szServiceName);
		
		if (PathFileExists(szDriverPath))
		{
			szComp = m_Funcionts.GetFileCompanyName(szDriverPath);
		}
		else
		{
			szComp = szFileNotFound[g_enumLang];
		}

		if (itor->nDriverObject)
		{
			szDriverObject.Format(L"0x%08X", itor->nDriverObject);
		}
		else
		{
			szDriverObject = L"-";
		}

		if ( itor->DriverType == enumHaveOrder )
		{
			// 是否是隐藏模块
			if (IsHideDriver(itor->nBase))
			{
				nHideDriverCnt++;
				item_clr = enumRed;
			}

			// 如果不是隐藏的模块, 那么判断是否是微软模块
			if (item_clr == enumBlack && !m_Funcionts.IsMicrosoftApp(szComp))
			{
				item_clr = enumBlue;
			}
		}
		else if ( itor->DriverType == enumHide )
		{
			szLoadOrder = L"-";
			nHideDriverCnt++;
			item_clr = enumRed;
		}
		
		int n = m_list.InsertItem(m_list.GetItemCount(), szDriverName);
		m_list.SetItemText(n, eDriverBase, szBase);
		m_list.SetItemText(n, eDriverSize, szSize);
		m_list.SetItemText(n, eDriverObject, szDriverObject);
		m_list.SetItemText(n, eDriverPath, szDriverPath);
		m_list.SetItemText(n, eDriverServiceName, szServiceName);
		m_list.SetItemText(n, eDriverStartupType, szStartup);
		m_list.SetItemText(n, eDriverLoadOrder, szLoadOrder);
		m_list.SetItemText(n, eDriverFileCorporation, szComp);
		
		m_vectorColor.push_back(item_clr);
		m_list.SetItemData(n, n);
	}
	
	m_Status.Format(szDriverStatus[g_enumLang], m_list.GetItemCount(), nHideDriverCnt, 0);
	UpdateData(FALSE);
}

//
// r3通过函数ZwQuerySystemInformation来枚举内核模块
//
void CListDriverDlg::Ring3GetDriverModules()
{
	m_vectorRing3Drivers.clear();

	WCHAR szNtdll[] = {'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l', '\0'};
	HINSTANCE ntdll_dll = GetModuleHandle(szNtdll);

	if (ntdll_dll == NULL) 
	{
		return;
	}  

	pfnZwQuerySystemInformation ZwQuerySystemInformation = NULL;
	CHAR szZwQuerySystemInformation[] = 
		{'Z', 'w', 'Q', 'u', 'e', 'r', 'y', 'S', 'y', 's', 't', 'e', 'm', 'I', 'n', 'f', 'o', 'r', 'm', 'a', 't', 'i', 'o', 'n', '\0'};

	ZwQuerySystemInformation = (pfnZwQuerySystemInformation)GetProcAddress(ntdll_dll, "ZwQuerySystemInformation");   
	if ( ZwQuerySystemInformation != NULL )   
	{
		DWORD dwNeedSize = 0;
		NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &dwNeedSize);

		if (status == STATUS_INFO_LENGTH_MISMATCH) 
		{        
			PBYTE pBuffer = new BYTE[dwNeedSize];

			if (pBuffer)
			{
				status = ZwQuerySystemInformation(SystemModuleInformation, pBuffer, dwNeedSize, &dwNeedSize);
				if (status == STATUS_SUCCESS) 
				{
					ULONG count = *((ULONG*)pBuffer);
					PSYSTEM_MODULE_INFORMATION pmi = (PSYSTEM_MODULE_INFORMATION)(pBuffer + sizeof(ULONG));
					for (ULONG i = 0; i < count; i++) 
					{
						m_vectorRing3Drivers.push_back((ULONG)(pmi->Base));
						pmi++;
					}
				}

				delete []pBuffer;
				pBuffer = NULL;
			}
		}
	}
}

//
// 颜色
//
void CListDriverDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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
		int nData = (int)m_list.GetItemData(nItem);
		ITEM_COLOR clr = m_vectorColor.at(nData);

		clrNewTextColor = g_MicrosoftItemClr;//RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );	
		
		if ( clr == enumBlue )
		{
			clrNewTextColor = g_NormalItemClr;
		}
		else if ( clr == enumRed )
		{
			clrNewTextColor = g_HiddenOrHookItemClr;
		}
		else if ( clr == enumPurple )
		{
			clrNewTextColor = g_NotSignedItemClr;
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

//
// 根据服务键值,获得其ImagePath
//
CString CListDriverDlg::GetServiceKeyImagePathValue(CString szKey)
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
			szPath = m_Funcionts.RegParsePath((WCHAR*)ir->pData, ir->Type);
			break;
		}
	}
	
	m_Registry.FreeValueList(valueList);

	return szPath;
}

//
// 获取services键值的全部内容
//
VOID CListDriverDlg::GetServiceKeys()
{
	m_vectorServiceKeys.clear();

	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(SERVICE_KEY_NAME, keyList);
	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		if (ir->nNameLen > 0)
		{
			CString szKey = SERVICE_KEY_NAME + CString(L"\\") + ir->szName;
			CString szImagePathValue = GetServiceKeyImagePathValue(szKey);
			if (!szImagePathValue.IsEmpty())
			{
				SERVICE_KEY sk;
				sk.szKey = ir->szName;
				sk.szPath = szImagePathValue;
				m_vectorServiceKeys.push_back(sk);
			}
		}
	}
}

//
// 右键刷新
//
void CListDriverDlg::OnDriverRefresh()
{
	ListDrivers();
}

//
// 右键菜单
//
void CListDriverDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_DRIVER_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_DRIVER_EXCEL, szExcel[g_enumLang]);

	CMenu starttype;
	starttype.CreatePopupMenu();
	starttype.AppendMenu(MF_STRING, ID_SERVICE_BOOT, szBootStart[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_SYSTEM, szSystemStart[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_AUTO, szServiceAutomatic[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_MAD, szServiceManual[g_enumLang]);
	starttype.AppendMenu(MF_STRING, ID_SERVICE_DISABLED, szServiceDisabled[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_DRIVER_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_DRIVER_MEMORY, szDriverDump[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)starttype.m_hMenu, szStartType[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_UNLOAD_DRIVER, szDriverUnload[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DELETE_DRIVER_FILE, szDeleteDriverFile[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DELETE_DRIVER_FILE_AND_REG, szDeleteDriverFileAndReg[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_LOCATE_REG, szLocateInRegedit[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_COPY_DRIVER_NAME, szCopyDriverName[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_COPY_DRIVER_PATH, szCopyDriverPath[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DRIVER_SHUXING, szDriverProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DRIVER_SEARCH, szSearchOnlineDriver[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FIND_DRIVER, szFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DRIVER_SIGN_CHECK, szVerifyDriverSignature[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DRIVER_ALL_SIGN_CHECK, szVerifyAllDriverSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_DRIVER_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_UNLOAD_DRIVER, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_DELETE_DRIVER_FILE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_DELETE_DRIVER_FILE_AND_REG, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FIND_DRIVER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_DRIVER_SEARCH, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_DRIVER_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_COPY_DRIVER_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_COPY_DRIVER_PATH, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_DRIVER_SIGN_CHECK, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_DRIVER_ALL_SIGN_CHECK, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(ID_LOCATE_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		menu.SetMenuItemBitmaps(25, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	// 如果m_list是空的，那么除了“刷新”，其他全部置灰
	if (!m_list.GetItemCount()) 
	{
		for (int i = 2; i < 26; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetSelectedCount())
		{
			for (int i = 2; i < 24; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_DRIVER_ALL_SIGN_CHECK, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{
			int nItem = m_Funcionts.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				CString szService = m_list.GetItemText(nItem, eDriverServiceName);
				if (szService.IsEmpty())
				{
					menu.EnableMenuItem(ID_LOCATE_REG, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_DELETE_DRIVER_FILE_AND_REG, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(4, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(6, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
				}
				else
				{
					DWORD dwType = 0;
					if (GetStartType(szService, &dwType))
					{
						switch (dwType)
						{
						case SERVICE_AUTO_START:
							menu.CheckMenuItem(ID_SERVICE_AUTO, MF_BYCOMMAND | MF_CHECKED);
							menu.EnableMenuItem(ID_SERVICE_AUTO, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							break;

						case SERVICE_BOOT_START:
							menu.CheckMenuItem(ID_SERVICE_BOOT, MF_BYCOMMAND | MF_CHECKED);
							menu.EnableMenuItem(ID_SERVICE_BOOT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							break;

						case SERVICE_DEMAND_START:
							menu.CheckMenuItem(ID_SERVICE_MAD, MF_BYCOMMAND | MF_CHECKED);
							menu.EnableMenuItem(ID_SERVICE_MAD, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							break;

						case SERVICE_DISABLED:
							menu.CheckMenuItem(ID_SERVICE_DISABLED, MF_BYCOMMAND | MF_CHECKED);
							menu.EnableMenuItem(ID_SERVICE_DISABLED, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							break;

						case SERVICE_SYSTEM_START:
							menu.CheckMenuItem(ID_SERVICE_SYSTEM, MF_BYCOMMAND | MF_CHECKED);
							menu.EnableMenuItem(ID_SERVICE_SYSTEM, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
							break;
						}
					}
				}

				// 判断该驱动对象能否被卸载
				if (!IsDriverCanUnload())
				{
					menu.EnableMenuItem(ID_UNLOAD_DRIVER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();
	starttype.DestroyMenu();

	*pResult = 0;
}

//
// 卸载驱动
//
void CListDriverDlg::OnUnloadDriver()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szDriverObject = m_list.GetItemText(nItem, eDriverObject);
		if (szDriverObject.IsEmpty() ||
			!szDriverObject.CompareNoCase(L"-"))
		{
			return;
		}

		int nData = (int)m_list.GetItemData(nItem);
		DRIVER_INFO info = m_vectorDrivers.at(nData);
		if (info.nDriverObject)
		{
			if (MessageBox(szAreYouSureUnloadDriver[g_enumLang], szUnloadDriver[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				if (m_clsDriver.UnLoadDriver(info.nDriverObject))
				{
					m_list.DeleteItem(nItem);
				}
			}
		}
	}
}

//
// 删除模块文件
//
void CListDriverDlg::OnDeleteDriverFile()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		BOOL bRet = m_Funcionts.KernelDeleteFile(m_list.GetItemText(nItem, eDriverPath));
		if (bRet)
		{
			MessageBox(szDeleteFileSucess[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(szDeleteFileFailed[g_enumLang], szToolName, MB_OK | MB_ICONERROR);
		}
	}
}

//
// 删除文件和注册表
//
void CListDriverDlg::OnDeleteDriverFileAndReg()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	int iItem = m_Funcionts.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szServiceName = m_list.GetItemText(iItem, eDriverServiceName);
		if (!szServiceName.IsEmpty())
		{
			CString szKey = SERVICE_KEY_NAME + CString(L"\\") + szServiceName;
			m_Registry.DeleteKeyAndSubKeys(szKey);	
		}

		BOOL bRet = m_Funcionts.KernelDeleteFile(m_list.GetItemText(iItem, eDriverPath));
		if (bRet)
		{
			MessageBox(szDeleteFileSucess[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(szDeleteFileFailed[g_enumLang], szToolName, MB_OK | MB_ICONERROR);
		}
	}
}

//
// 拷贝驱动模块内存
//
void CListDriverDlg::OnCopyDriverMemory()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}
		
	WCHAR szDumped[] = {'D','u','m','p','e','d','_','\0'};
	CString szImageName = m_list.GetItemText(nItem, eDriverName);
	WCHAR szFile[MAX_PATH] = {0};
	wcsncpy_s(szFile, MAX_PATH, szDumped, wcslen(szDumped));
	wcscat_s(szFile, MAX_PATH - wcslen(szDumped), szImageName.GetBuffer());
	szImageName.ReleaseBuffer();

	CFileDialog fileDlg(
		FALSE, 
		0, 
		(LPWSTR)szFile, 
		0, 
		L"All Files (*.*)|*.*||",
		0
		);

	if (IDOK != fileDlg.DoModal())
	{
		return;
	}

	CString szFilePath = fileDlg.GetFileName();
	if ( !PathFileExists(szFilePath) ||
		(PathFileExists(szFilePath) && 
		MessageBox(szFileExist[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDYES))
	{
		int nData = (int)m_list.GetItemData(nItem);
		DRIVER_INFO info = m_vectorDrivers.at(nData);
		ULONG nBase = info.nBase, nSize = info.nSize;
		BOOL bRet = FALSE;

		DebugLog(L"Dump Driver->  Base: 0x%08X, Size: 0x%08X", nBase, nSize);

		if (nBase && nSize)
		{
			ULONG Size = nSize + 0x1000;
			PVOID pOutBuffer = GetMemory(Size);
			if (pOutBuffer)
			{
				COMMUNICATE_DRIVER cd;
				cd.OpType = enumDumpDriver;
				cd.op.DumpMemory.nSize = nSize;
				cd.op.DumpMemory.pBase = nBase;

				bRet = m_Driver.CommunicateDriver(&cd, sizeof(COMMUNICATE_DRIVER), pOutBuffer, Size, NULL);

				if (bRet)
				{
					bRet = FALSE;

					if (MessageBox(szFixDump[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						FixPeDump(pOutBuffer);
					}
					
					CFile file;
					TRY 
					{
						if (file.Open(szFilePath,  CFile::modeCreate | CFile::modeWrite))
						{
							file.Write(pOutBuffer, nSize);
							file.Close();
							bRet = TRUE;
						}
					}
					CATCH_ALL( e )
					{
						bRet = FALSE;
						file.Abort();   // close file safely and quietly
						//THROW_LAST();
					}
					END_CATCH_ALL
				}

				free(pOutBuffer);
				pOutBuffer = NULL;
			}
		}

		if (bRet)
		{
			MessageBox(szDumpOK[g_enumLang], szToolName);
		}
		else
		{
			MessageBox(szDumpFailed[g_enumLang], szToolName);
		}
	}
}

//
// 验证驱动模块签名
//
void CListDriverDlg::OnDriverSignCheck()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, eDriverPath);
		m_SignVerifyDlg.m_bSingle = TRUE;
		m_SignVerifyDlg.m_path = szPath;
		m_SignVerifyDlg.DoModal();
	}
}

//
// 验证所有的模块签名
//
void CListDriverDlg::OnDriverAllSignCheck()
{
	m_SignVerifyDlg.m_NotSignItemList.clear();
	m_SignVerifyDlg.m_bSingle = FALSE;
	m_SignVerifyDlg.m_pList = &m_list;
	m_SignVerifyDlg.m_nPathSubItem = eDriverPath;
	m_SignVerifyDlg.m_hWinWnd = this->m_hWnd;
	m_SignVerifyDlg.DoModal();
}

//
// 拷贝驱动image name
//
void CListDriverDlg::OnCopyDriverName()
{
	int iItem = m_Funcionts.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szDriverPath = m_list.GetItemText(iItem, eDriverName);
		m_Funcionts.SetStringToClipboard(szDriverPath);
	}
}

//
// 拷贝驱动path
//
void CListDriverDlg::OnCopyDriverPath()
{
	int iItem = m_Funcionts.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szDriverPath = m_list.GetItemText(iItem, eDriverPath);
		m_Funcionts.SetStringToClipboard(szDriverPath);
	}
}

//
// 定位到注册表
//
void CListDriverDlg::OnLocateReg()
{
	int iItem = m_Funcionts.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szServiceName = m_list.GetItemText(iItem, eDriverServiceName);
		if (!szServiceName.IsEmpty())
		{
			CString szKey = SERVICE_KEY_NAME;
			szKey += L"\\";
			szKey += szServiceName;
			m_Funcionts.JmpToMyRegistry(szKey, NULL);	
		}
	}
}

//
// 定位到驱动
//
void CListDriverDlg::OnFindDriver()
{
	int iItem = m_Funcionts.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szDriverPath = m_list.GetItemText(iItem, eDriverPath);
		m_Funcionts.LocationExplorer(szDriverPath);
	}
}

//
// 查看驱动文件属性
//
void CListDriverDlg::OnDriverShuxing()
{
	int iItem = m_Funcionts.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szDriverPath = m_list.GetItemText(iItem, eDriverPath);
		m_Funcionts.OnCheckAttribute(szDriverPath);
	}
}

//
// 搜索image name
//
void CListDriverDlg::OnDriverSearch()
{
	int iItem = m_Funcionts.GetSelectItem(&m_list);
	if (iItem != -1)
	{
		CString szDriverName = m_list.GetItemText(iItem, eDriverName);
		m_Funcionts.SearchOnline(szDriverName);
	}
}

//
// 导出到text
//
void CListDriverDlg::OnDriverText()
{
	m_Funcionts.ExportListToTxt(&m_list, m_Status);
}

//
// 导出到excel
//
void CListDriverDlg::OnDriverExcel()
{
	WCHAR szDriver[] = {'D','r','i','v','e','r','\0'};
	m_Funcionts.ExportListToExcel(&m_list, szDriver, m_Status);
}

//
// 根据item判断是否有签名
//
BOOL CListDriverDlg::IsNotSignItem(ULONG nItem)
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

//
// 批量验签名结束之后会发送这个消息
//
LRESULT CListDriverDlg::SignVerifyOver(WPARAM wParam, LPARAM lParam)
{
	ULONG nCnt = m_list.GetItemCount();

	for (ULONG i = 0; i < nCnt; i++)
	{
		CString szComp = m_list.GetItemText(i, eDriverFileCorporation);
		CString szCompTemp = L"";

		if (IsNotSignItem(i))
		{
			// 设置是否Vrified
			szCompTemp = szNotVerified[g_enumLang] + szComp;
			m_vectorColor.at(i) = enumPurple;
		}
		else
		{
			// 设置是否Vrified
			szCompTemp = szVerified[g_enumLang] + szComp;
		}

		m_list.SetItemText(i, eDriverFileCorporation, szCompTemp);
	}

	return 0;
}

//
// 查看驱动对象是否为空,为空则不能卸载
//
BOOL CListDriverDlg::IsDriverCanUnload()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szDriverObject = m_list.GetItemText(nItem, eDriverObject);

		if (szDriverObject.IsEmpty() ||
			!szDriverObject.CompareNoCase(L"-"))
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}

	return FALSE;
}

//
// 获取驱动文件的启动类型
//
BOOL CListDriverDlg::GetStartType(CString szKey, DWORD *dwType)
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

void CListDriverDlg::OnServiceBoot()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, eDriverServiceName);
		ModifyStartupType(szService, SERVICE_BOOT_START);
	}
}

void CListDriverDlg::OnServiceSystem()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, eDriverServiceName);
		ModifyStartupType(szService, SERVICE_SYSTEM_START);
	}
}

void CListDriverDlg::OnServiceAuto()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, eDriverServiceName);
		ModifyStartupType(szService, SERVICE_AUTO_START);
	}
}

void CListDriverDlg::OnServiceMad()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, eDriverServiceName);
		ModifyStartupType(szService, SERVICE_DEMAND_START);
	}
}

void CListDriverDlg::OnServiceDisabled()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szService = m_list.GetItemText(nItem, eDriverServiceName);
		ModifyStartupType(szService, SERVICE_DISABLED);
	}
}

BOOL CListDriverDlg::ModifyStartupType(CString szServiceName, ULONG Type)
{
	BOOL bRet = FALSE;

	if (szServiceName.IsEmpty())
	{
		return bRet;
	}

	CString szKeyPath = REG_SERVICE + CString(L"\\") + szServiceName;
	WCHAR szStart[] = {'S','t','a','r','t','\0'};

	bRet = m_Registry.ModifyValue(szKeyPath,
		szStart,
		REG_DWORD,
		&Type,
		sizeof(DWORD));
	
	if (bRet)
	{
		CString szType = L"-";
		switch (Type)
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

		int nItem = m_Funcionts.GetSelectItem(&m_list);
		if (nItem != -1)
		{
			m_list.SetItemText(nItem, eDriverStartupType, szType);
		}
	}

	return bRet;
}

//
// 屏蔽按键
//
BOOL CListDriverDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

//
// 定位到as文件管理器
//
void CListDriverDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Funcionts.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, eDriverPath);
		m_Funcionts.JmpToFile(szPath);
	}
}