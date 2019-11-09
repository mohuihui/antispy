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
// DpcTimerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "DpcTimerDlg.h"
#include "SignVerifyDlg.h"

// CDpcTimerDlg 对话框

IMPLEMENT_DYNAMIC(CDpcTimerDlg, CDialog)

CDpcTimerDlg::CDpcTimerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDpcTimerDlg::IDD, pParent)
	, m_status(_T(""))
{

}

CDpcTimerDlg::~CDpcTimerDlg()
{
}

void CDpcTimerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_DPC_TIMER_CNT, m_status);
}


BEGIN_MESSAGE_MAP(CDpcTimerDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDpcTimerDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CDpcTimerDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_DPCTIMER_33087, &CDpcTimerDlg::OnDpctimer33087)
	ON_COMMAND(ID_DPCTIMER_33088, &CDpcTimerDlg::OnDpctimer33088)
	ON_COMMAND(ID_DPCTIMER_33089, &CDpcTimerDlg::OnDpctimer33089)
// 	ON_COMMAND(ID_DPCTIMER_33090, &CDpcTimerDlg::OnDpctimer33090)
	ON_COMMAND(ID_DPCTIMER_33091, &CDpcTimerDlg::OnDpctimer33091)
	ON_COMMAND(ID_DPCTIMER_LOCATION_MODULE, &CDpcTimerDlg::OnDpctimer33092)
	ON_COMMAND(ID_DPCTIMER_VERIFY_SIGN, &CDpcTimerDlg::OnDpctimerVerifySin)
	ON_COMMAND(ID_DPC_TIMER_TEXT, &CDpcTimerDlg::OnExportToText)
	ON_COMMAND(ID_DPC_TIMER_EXCEL, &CDpcTimerDlg::OnExportToExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CDpcTimerDlg::OnNMRclickList)
	ON_COMMAND(ID_DPCTIMER_DISASM, &CDpcTimerDlg::OnDpctimerDisasm)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CDpcTimerDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDpcTimerDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_DPC_TIMER_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CDpcTimerDlg 消息处理程序

void CDpcTimerDlg::OnBnClickedOk()
{
}

void CDpcTimerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CDpcTimerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szDpcTimerObject[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(1, szDpc[g_enumLang], LVCFMT_LEFT, 140);
	m_list.InsertColumn(2, szPeriod[g_enumLang], LVCFMT_LEFT, 120);
	m_list.InsertColumn(3, szDpcTimerRoutineEntry[g_enumLang], LVCFMT_LEFT, 170);
	m_list.InsertColumn(4, szModulePath[g_enumLang], LVCFMT_LEFT, 440);
	m_list.InsertColumn(5, szFileCorporation[g_enumLang], LVCFMT_LEFT, 180);
	
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDpcTimerDlg::GetDpcTimer()
{
	ULONG nCnt = 200;
	PDPC_TIMER_INFO pDpcTimerInfo = NULL;
	OPERATE_TYPE ot = enumListDpcTimer;
	BOOL bRet = FALSE;

	m_status.Format(szDpcTimerStatus[g_enumLang], 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_DpcTimerVector.clear();

	do 
	{
		ULONG nSize = sizeof(DPC_TIMER_INFO) + nCnt * sizeof(DPC_TIMER);

		if (pDpcTimerInfo)
		{
			free(pDpcTimerInfo);
			pDpcTimerInfo = NULL;
		}

		pDpcTimerInfo = (PDPC_TIMER_INFO)malloc(nSize);

		if (pDpcTimerInfo)
		{
			memset(pDpcTimerInfo, 0, nSize);
			pDpcTimerInfo->nCnt = nCnt;
			bRet = m_driver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pDpcTimerInfo, nSize, NULL);
		}

		nCnt = pDpcTimerInfo->nRetCnt + 10;

	} while (!bRet && pDpcTimerInfo->nRetCnt > pDpcTimerInfo->nCnt);

	if (bRet &&
		pDpcTimerInfo->nCnt >= pDpcTimerInfo->nRetCnt)
	{
		for (ULONG i = 0; i < pDpcTimerInfo->nRetCnt; i++)
		{
			m_DpcTimerVector.push_back(pDpcTimerInfo->DpcTimer[i]);
		}
	}

	if (pDpcTimerInfo)
	{
		free(pDpcTimerInfo);
		pDpcTimerInfo = NULL;
	}

	GetDriver();
	InsertDpcTimers();
}

void CDpcTimerDlg::InsertDpcTimers()
{
	ULONG i = 0;
	for (vector<DPC_TIMER>::iterator itor = m_DpcTimerVector.begin(); itor != m_DpcTimerVector.end(); itor++)
	{
		CString szTimerObject, szPeriod, szDispatch, szPath, szDpc;
		
		szTimerObject.Format(L"0x%08X", itor->TimerObject);
		szPeriod.Format(L"%d", itor->Period / 1000);
		szDispatch.Format(L"0x%08X", itor->TimeDispatch);
		szPath = GetDriverPathByCallbackAddress(itor->TimeDispatch);
		szDpc.Format(L"0x%08X", itor->Dpc);

		int n = m_list.InsertItem(m_list.GetItemCount(), szTimerObject);
		m_list.SetItemText(n, 1, szDpc);
		m_list.SetItemText(n, 2, szPeriod);
		m_list.SetItemText(n, 3, szDispatch);
		m_list.SetItemText(n, 4, szPath);
		m_list.SetItemText(n, 5, m_Functions.GetFileCompanyName(szPath));

		i++;
	}

	m_status.Format(szDpcTimerStatus[g_enumLang], i);
	UpdateData(FALSE);
}

CString CDpcTimerDlg::GetDriverPathByCallbackAddress(ULONG pCallback)
{
	CString szPath;

	for (vector<DRIVER_INFO>::iterator itor = m_CommonDriverList.begin(); itor != m_CommonDriverList.end(); itor++)
	{
		ULONG nBase = itor->nBase;
		ULONG nEnd = itor->nBase + itor->nSize;

		if (pCallback >= nBase && pCallback <= nEnd)
		{
			szPath = itor->szDriverPath;
			break;
		}
	}

	return szPath;
}

void CDpcTimerDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

void CDpcTimerDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CDpcTimerDlg::OnDpctimer33087()
{
	GetDpcTimer();
}

void CDpcTimerDlg::OnDpctimer33088()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szTimer = m_list.GetItemText(iIndex, 0);
		ULONG pTimer = m_Functions.HexStringToLong(szTimer);

		if (pTimer)
		{
			COMMUNICATE_DPCTIMER cd;
			cd.OpType = enumRemoveDpcTimer;
			cd.pTimer = pTimer;
			m_driver.CommunicateDriver(&cd, sizeof(COMMUNICATE_DPCTIMER), NULL, 0, NULL);
		}

		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

void CDpcTimerDlg::OnDpctimer33089()
{
	for ( vector <DPC_TIMER>::iterator Iter = m_DpcTimerVector.begin( ); Iter != m_DpcTimerVector.end( ); Iter++ )
	{
		COMMUNICATE_DPCTIMER cd;
		cd.OpType = enumRemoveDpcTimer;
		cd.pTimer = Iter->TimerObject;
		m_driver.CommunicateDriver(&cd, sizeof(COMMUNICATE_DPCTIMER), NULL, 0, NULL);
	}

	m_list.DeleteAllItems();
}

void CDpcTimerDlg::OnDpctimer33091()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CDpcTimerDlg::OnDpctimer33092()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.LocationExplorer(szPath);
	}
}

void CDpcTimerDlg::OnDpctimerVerifySin()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.SignVerify(szPath);
	}
}

void CDpcTimerDlg::OnExportToText()
{
	m_Functions.ExportListToTxt(&m_list, m_status);
}

void CDpcTimerDlg::OnExportToExcel()
{
	WCHAR szDpcTimer[] = {'D','p','c','T','i','m','e','r','\0'};
	m_Functions.ExportListToExcel(&m_list, szDpcTimer, m_status);
}

void CDpcTimerDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_DPC_TIMER_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_DPC_TIMER_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_DPCTIMER_33087, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DPCTIMER_33088, szRemove[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DPCTIMER_33089, szRemoveAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DPCTIMER_DISASM, szDisassembleEntry[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DPCTIMER_33091, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DPCTIMER_VERIFY_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DPCTIMER_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_DPCTIMER_33087, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_DPCTIMER_33088, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_DPCTIMER_33089, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_DPCTIMER_DISASM, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_DPCTIMER_LOCATION_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_DPCTIMER_33091, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_DPCTIMER_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(13, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 14; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 13; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			menu.EnableMenuItem(ID_DPCTIMER_33091, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_DPCTIMER_LOCATION_MODULE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_DPCTIMER_VERIFY_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_DPCTIMER_DISASM, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}

		menu.EnableMenuItem(ID_DPCTIMER_33089, MF_BYCOMMAND | MF_ENABLED);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CDpcTimerDlg::OnDpctimerDisasm()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 3);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

BOOL CDpcTimerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CDpcTimerDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.JmpToFile(szPath);
	}
}
