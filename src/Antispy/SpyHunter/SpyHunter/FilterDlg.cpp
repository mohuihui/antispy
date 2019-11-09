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
// FilterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FilterDlg.h"
#include "SignVerifyDlg.h"

// CFilterDlg 对话框

IMPLEMENT_DYNAMIC(CFilterDlg, CDialog)

CFilterDlg::CFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
}

CFilterDlg::~CFilterDlg()
{
	m_FilterVector.clear();
}

void CFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_FILTER_CNT, m_szStatus);
}


BEGIN_MESSAGE_MAP(CFilterDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFilterDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CFilterDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CFilterDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_FILTER_REFRESH, &CFilterDlg::OnFilterRefresh)
	ON_COMMAND(ID_FILTER_DELETE, &CFilterDlg::OnFilterDelete)
	ON_COMMAND(ID_FILTER_SHUXING, &CFilterDlg::OnFilterShuxing)
	ON_COMMAND(ID_FILTER_LOCATION_MODULE, &CFilterDlg::OnFilterLocationModule)
	ON_COMMAND(ID_FILTER_VERIFY_SIGN, &CFilterDlg::OnFilterVerifySign)
	ON_COMMAND(ID_FILTER_TEXT, &CFilterDlg::OnFilterText)
	ON_COMMAND(ID_FILTER_EXCEL, &CFilterDlg::OnFilterExcel)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CFilterDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CFilterDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_FILTER_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CFilterDlg 消息处理程序

void CFilterDlg::OnBnClickedOk()
{
}

void CFilterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CFilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szDevice[g_enumLang], LVCFMT_LEFT, 120);
	m_list.InsertColumn(1, szType[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(2, szDriverName[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(3, szDriverPath[g_enumLang], LVCFMT_LEFT, 410);
	m_list.InsertColumn(4, szAttachDeviceName[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(5, szFileCorporation[g_enumLang], LVCFMT_LEFT, 180);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CFilterDlg::EnumFilterDriver()
{
	m_FilterVector.clear();
	m_list.DeleteAllItems();

	OPERATE_TYPE opType = enumListFilter;
	ULONG nCnt = 100;
	PFILTER_DRIVER pFilterInfo = NULL;
	BOOL bRet = FALSE;

	m_szStatus.Format(szFilterDriverCnt[g_enumLang], 0);
	UpdateData(FALSE);

	do 
	{
		ULONG nSize = sizeof(FILTER_DRIVER) + nCnt * sizeof(FILTER_INFO);

		if (pFilterInfo)
		{
			free(pFilterInfo);
			pFilterInfo = NULL;
		}

		pFilterInfo = (PFILTER_DRIVER)malloc(nSize);

		if (pFilterInfo)
		{
			memset(pFilterInfo, 0, nSize);
			pFilterInfo->nCnt = nCnt;
			bRet = m_Driver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pFilterInfo, nSize, NULL);
		}

		nCnt = pFilterInfo->nRetCnt + 10;

	} while (pFilterInfo->nRetCnt > pFilterInfo->nCnt);

	if (bRet && pFilterInfo->nCnt >= pFilterInfo->nRetCnt)
	{
		for (ULONG i = 0; i < pFilterInfo->nRetCnt; i++)
		{
			m_FilterVector.push_back(pFilterInfo->Filter[i]);
		}
	}

	if (pFilterInfo)
	{
		free(pFilterInfo);
		pFilterInfo = NULL;
	}

	InsertFilter();
}

void CFilterDlg::InsertFilter()
{
	ULONG nCnt = 0;

	for (vector<FILTER_INFO>::iterator itor = m_FilterVector.begin(); itor != m_FilterVector.end(); itor++)
	{
		CString szDevice, szType, szPath;
		
		szDevice.Format(L"0x%08X", itor->pDeviceObject);
		szPath = itor->szPath;

		switch (itor->Type)
		{
		case enumFile:
			szType = L"File";
			break;
		case enumDisk:
			szType = L"Disk";
			break;
		case enumVolume:
			szType = L"Volume";
			break;
		case enumKeyboard:
			szType = L"Keyboard";
			break;
		case enumMouse:
			szType = L"Mouse";
			break;
		case enumI8042prt:
			szType = L"I8042prt";
			break;
		case enumTcpip:
			szType = L"Tcpip";
			break;
		case enumNDIS:
			szType = L"Ndis";
			break;
		case enumPnpManager:
			szType = L"PnpManager";
			break;
		default:
			szType = L"UnKnow";
			break;
		}

		int n = m_list.InsertItem(m_list.GetItemCount(), szDevice);
		m_list.SetItemText(n, 1, szType);
		m_list.SetItemText(n, 2, itor->szDriverName);
		m_list.SetItemText(n, 3, szPath);
		m_list.SetItemText(n, 4, itor->szAttachedDriverName);
		m_list.SetItemText(n, 5, m_Functions.GetFileCompanyName(szPath));

		nCnt++;
	}

	m_szStatus.Format(szFilterDriverCnt[g_enumLang], nCnt);
	UpdateData(FALSE);
}

void CFilterDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_FILTER_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_FILTER_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_FILTER_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FILTER_DELETE, szDeleteDriverFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FILTER_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_FILTER_VERIFY_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_FILTER_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_FILTER_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_FILTER_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FILTER_LOCATION_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_FILTER_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_FILTER_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(10, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 11; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 10; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
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

void CFilterDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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
		CString strCompany;
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		clrNewTextColor = g_MicrosoftItemClr;//RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );	

		strCompany = m_list.GetItemText(nItem, 5); 		
		if (!m_Functions.IsMicrosoftApp(strCompany))
		{
			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CFilterDlg::OnFilterRefresh()
{
	EnumFilterDriver();
}

void CFilterDlg::OnFilterDelete()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szFilter[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		if (!szPath.IsEmpty())
		{
			BOOL bRet = m_Functions.KernelDeleteFile(szPath);
			if (bRet)
			{
				MessageBox(szDeleteFileSucess[g_enumLang], NULL, MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(szDeleteFileFailed[g_enumLang], NULL, MB_OK | MB_ICONERROR);
			}
		}
	}
}

void CFilterDlg::OnFilterShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CFilterDlg::OnFilterLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.LocationExplorer(szPath);
	}
}

void CFilterDlg::OnFilterVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.SignVerify(szPath);
	}
}

void CFilterDlg::OnFilterText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CFilterDlg::OnFilterExcel()
{
	WCHAR szFilter[] = {'F','i','l','t','e','r','\0'};
	m_Functions.ExportListToExcel(&m_list, szFilter, m_szStatus);
}

BOOL CFilterDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CFilterDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.JmpToFile(szPath);
	}
}
