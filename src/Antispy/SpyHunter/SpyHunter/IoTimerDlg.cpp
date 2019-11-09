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
// IoTimerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "IoTimerDlg.h"
#include "SignVerifyDlg.h"

// CIoTimerDlg 对话框

IMPLEMENT_DYNAMIC(CIoTimerDlg, CDialog)

CIoTimerDlg::CIoTimerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIoTimerDlg::IDD, pParent)
	, m_status(_T(""))
{

}

CIoTimerDlg::~CIoTimerDlg()
{
	m_IoTimerVector.clear();
}

void CIoTimerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_IO_TIMER_CNT, m_status);
}


BEGIN_MESSAGE_MAP(CIoTimerDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CIoTimerDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_COMMAND(ID_IOTIMER_REFRESH, &CIoTimerDlg::OnIotimerRefresh)
	ON_COMMAND(ID_IOTIMER_STOP, &CIoTimerDlg::OnIotimerStop)
	ON_COMMAND(ID_IOTIMER_START, &CIoTimerDlg::OnIotimerStart)
	ON_COMMAND(ID_IOTIMER_REMOVE, &CIoTimerDlg::OnIotimerRemove)
	ON_COMMAND(ID_IOTIMER_REMOVE_ALL, &CIoTimerDlg::OnIotimerRemoveAll)
	ON_COMMAND(ID_IOTIMER_DIS_ENTRY, &CIoTimerDlg::OnIotimerDisEntry)
	ON_COMMAND(ID_IOTIMER_SHUXING, &CIoTimerDlg::OnIotimerShuxing)
	ON_COMMAND(ID_IOTIMER_LOCATE_MODULE, &CIoTimerDlg::OnIotimerLocateModule)
	ON_COMMAND(ID_IOTIMER_VERIFY_SIGN, &CIoTimerDlg::OnIotimerVerifySign)
	ON_COMMAND(ID_IOTIMER_TEXT, &CIoTimerDlg::OnIotimerText)
	ON_COMMAND(ID_IOTIMER_EXCEL, &CIoTimerDlg::OnIotimerExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CIoTimerDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CIoTimerDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CIoTimerDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CIoTimerDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_IO_TIMER_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CIoTimerDlg 消息处理程序

void CIoTimerDlg::OnBnClickedOk()
{
}

BOOL CIoTimerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szDpcTimerObject[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(1, szDeviceObject[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(2, szIoTimerStatus[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(3, szDpcTimerRoutineEntry[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(4, szModulePath[g_enumLang], LVCFMT_LEFT, 480);
	m_list.InsertColumn(5, szFileCorporation[g_enumLang], LVCFMT_LEFT, 180);

 	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CIoTimerDlg::GetIoTimers()
{
	ULONG nCnt = 100;
	PIO_TIMER_INFO pIoTimerInfo = NULL;
	OPERATE_TYPE ot = enumListIoTimer;
	BOOL bRet = FALSE;

	m_status.Format(szIoTimerCnt[g_enumLang], 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_IoTimerVector.clear();

	do 
	{
		ULONG nSize = sizeof(IO_TIMER_INFO) + nCnt * sizeof(IO_TIMERS);

		if (pIoTimerInfo)
		{
			free(pIoTimerInfo);
			pIoTimerInfo = NULL;
		}

		pIoTimerInfo = (PIO_TIMER_INFO)malloc(nSize);

		if (pIoTimerInfo)
		{
			memset(pIoTimerInfo, 0, nSize);
			pIoTimerInfo->nCnt = nCnt;
			bRet = m_driver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pIoTimerInfo, nSize, NULL);
		}

		nCnt = pIoTimerInfo->nRetCnt + 10;

	} while (!bRet && pIoTimerInfo->nRetCnt > pIoTimerInfo->nCnt);

	if (bRet &&
		pIoTimerInfo->nCnt >= pIoTimerInfo->nRetCnt)
	{
		for (ULONG i = 0; i < pIoTimerInfo->nRetCnt; i++)
		{
			m_IoTimerVector.push_back(pIoTimerInfo->IoTimer[i]);
		}
	}

	if (pIoTimerInfo)
	{
		free(pIoTimerInfo);
		pIoTimerInfo = NULL;
	}

 	GetDriver();
 	InsertIoTimers();
}

void CIoTimerDlg::InsertIoTimers()
{
	ULONG i = 0;

	for (vector<IO_TIMERS>::iterator itor = m_IoTimerVector.begin(); itor != m_IoTimerVector.end(); itor++)
	{
		CString szTimer, szDeviceObject, szPath, szStatus, szDispatch;

		szTimer.Format(L"0x%08X", itor->TimerObject);
		szDeviceObject.Format(L"0x%08X", itor->DeviceObject);
		szDispatch.Format(L"0x%08X", itor->TimeDispatch);
		szPath = GetDriverPathByCallbackAddress(itor->TimeDispatch);

		if (itor->Status)
		{
			szStatus = szIoTimerStatusWorking[g_enumLang];
		}
		else
		{
			szStatus = szIoTimerStatusStop[g_enumLang];
		}


		int n = m_list.InsertItem(m_list.GetItemCount(), szTimer);
		m_list.SetItemText(n, 1, szDeviceObject);
		m_list.SetItemText(n, 2, szStatus);
		m_list.SetItemText(n, 3, szDispatch);
		m_list.SetItemText(n, 4, szPath);
		m_list.SetItemText(n, 5, m_Functions.GetFileCompanyName(szPath));
		
		m_list.SetItemData(n, itor->TimerEntry);

		i++;
	}

	m_status.Format(szIoTimerCnt[g_enumLang], i);
	UpdateData(FALSE);
}

CString CIoTimerDlg::GetDriverPathByCallbackAddress(ULONG pCallback)
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

void CIoTimerDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

void CIoTimerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CIoTimerDlg::OnIotimerRefresh()
{
	GetIoTimers();
}

void CIoTimerDlg::OnIotimerStop()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		
		CString szDevice = m_list.GetItemText(nItem, 1);
		ULONG pDevice = m_Functions.HexStringToLong(szDevice);
		if (pDevice)
		{
			COMMUNICATE_IO_TIMER cit;
			cit.OpType = enumStopOrStartIoTimer;
			cit.op.StopOrStartTimer.bStart = FALSE;
			cit.op.StopOrStartTimer.pDeviceObject = pDevice;
			
			if (m_driver.CommunicateDriver(&cit, sizeof(COMMUNICATE_IO_TIMER), NULL, 0, NULL))
			{
				m_list.SetItemText(nItem, 2, szIoTimerStatusStop[g_enumLang]);
			}
		}
	}
}

void CIoTimerDlg::OnIotimerStart()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{

		CString szDevice = m_list.GetItemText(nItem, 1);
		ULONG pDevice = m_Functions.HexStringToLong(szDevice);
		if (pDevice)
		{
			COMMUNICATE_IO_TIMER cit;
			cit.OpType = enumStopOrStartIoTimer;
			cit.op.StopOrStartTimer.bStart = TRUE;
			cit.op.StopOrStartTimer.pDeviceObject = pDevice;

			if (m_driver.CommunicateDriver(&cit, sizeof(COMMUNICATE_IO_TIMER), NULL, 0, NULL))
			{
				m_list.SetItemText(nItem, 2, szIoTimerStatusWorking[g_enumLang]);
			}
		}
	}
}

void CIoTimerDlg::OnIotimerRemove()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		ULONG pTimerEntry = (ULONG)m_list.GetItemData(iIndex);

		if (pTimerEntry)
		{
			COMMUNICATE_IO_TIMER cit;
			cit.OpType = enumRemoveIoTimer;
			cit.op.Remove.TimerEntry = pTimerEntry;
			m_driver.CommunicateDriver(&cit, sizeof(COMMUNICATE_IO_TIMER), NULL, 0, NULL);
		}

		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

void CIoTimerDlg::OnIotimerRemoveAll()
{
	ULONG nCnt = m_list.GetItemCount();

	for (ULONG i = 0; i < nCnt; i++)
	{
		ULONG pTimerEntry = (ULONG)m_list.GetItemData(i);

		if (pTimerEntry)
		{
			COMMUNICATE_IO_TIMER cit;
			cit.OpType = enumRemoveIoTimer;
			cit.op.Remove.TimerEntry = pTimerEntry;
			m_driver.CommunicateDriver(&cit, sizeof(COMMUNICATE_IO_TIMER), NULL, 0, NULL);
		}
	}

	OnIotimerRefresh();
}

void CIoTimerDlg::OnIotimerDisEntry()
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

void CIoTimerDlg::OnIotimerShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CIoTimerDlg::OnIotimerLocateModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.LocationExplorer(szPath);
	}
}

void CIoTimerDlg::OnIotimerVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.SignVerify(szPath);
	}
}

void CIoTimerDlg::OnIotimerText()
{
	m_Functions.ExportListToTxt(&m_list, m_status);
}

void CIoTimerDlg::OnIotimerExcel()
{
	WCHAR szIoTimer[] = {'I','o','T','i','m','e','r','\0'};
	m_Functions.ExportListToExcel(&m_list, szIoTimer, m_status);
}

void CIoTimerDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_IOTIMER_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_IOTIMER_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_IOTIMER_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_START, szStart[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_STOP, szStop[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_REMOVE, szRemove[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_REMOVE_ALL, szRemoveAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_DIS_ENTRY, szDisassembleEntry[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_VERIFY_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_IOTIMER_LOCATE_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_IOTIMER_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_IOTIMER_REMOVE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_IOTIMER_REMOVE_ALL, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_IOTIMER_DIS_ENTRY, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_IOTIMER_LOCATE_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_IOTIMER_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_IOTIMER_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(16, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 17; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 15; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			menu.EnableMenuItem(ID_IOTIMER_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IOTIMER_LOCATE_MODULE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IOTIMER_VERIFY_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IOTIMER_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IOTIMER_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_IOTIMER_DIS_ENTRY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			int n = m_Functions.GetSelectItem(&m_list);
			if (n != -1)
			{
				CString szStatus = m_list.GetItemText(n, 2);
				if (!szStatus.CompareNoCase(szIoTimerStatusWorking[g_enumLang]))
				{
					menu.EnableMenuItem(ID_IOTIMER_START, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
				else
				{	
					menu.EnableMenuItem(ID_IOTIMER_STOP, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}

		menu.EnableMenuItem(ID_IOTIMER_REMOVE_ALL, MF_BYCOMMAND | MF_ENABLED);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CIoTimerDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

BOOL CIoTimerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CIoTimerDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		m_Functions.JmpToFile(szPath);
	}
}
