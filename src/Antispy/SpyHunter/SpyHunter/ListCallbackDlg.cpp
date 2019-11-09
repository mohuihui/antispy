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
// ListCallbackDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ListCallbackDlg.h"
#include "SignVerifyDlg.h"

// CListCallbackDlg 对话框

IMPLEMENT_DYNAMIC(CListCallbackDlg, CDialog)

CListCallbackDlg::CListCallbackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CListCallbackDlg::IDD, pParent)
	, m_status(_T(""))
{

}

CListCallbackDlg::~CListCallbackDlg()
{
	m_CallbackVector.clear();
	m_CommonDriverList.clear();
}

void CListCallbackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_CALLBACK_CNT, m_status);
}


BEGIN_MESSAGE_MAP(CListCallbackDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CListCallbackDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_COMMAND(ID_CALLBACK_REFRESH, &CListCallbackDlg::OnCallbackRefresh)
	ON_COMMAND(ID_CALLBACK_REMOVE, &CListCallbackDlg::OnCallbackRemove)
	ON_COMMAND(ID_CALLBACK_SHUXING, &CListCallbackDlg::OnCallbackShuxing)
	ON_COMMAND(ID_CALLBACK_LOCATE_MODULE, &CListCallbackDlg::OnCallbackLocateModule)
	ON_COMMAND(ID_CALLBACK_CHECK_SIGN, &CListCallbackDlg::OnCallbackCheckSign)
	ON_COMMAND(ID_CALLBACK_TEXT, &CListCallbackDlg::OnCallbackText)
	ON_COMMAND(ID_CALLBACK_EXCEL, &CListCallbackDlg::OnCallbackExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CListCallbackDlg::OnNMRclickList)
	ON_COMMAND(ID_CALLBACK_REMOVE_ALL, &CListCallbackDlg::OnCallbackRemoveAll)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CListCallbackDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_CALLBACK_DISASM, &CListCallbackDlg::OnCallbackDisasm)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CListCallbackDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CListCallbackDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_CALLBACK_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CListCallbackDlg 消息处理程序

void CListCallbackDlg::OnBnClickedOk()
{
}

BOOL CListCallbackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szRoutineEntry[g_enumLang], LVCFMT_LEFT, 140);
	m_list.InsertColumn(1, szNotifyType[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(2, szModulePath[g_enumLang], LVCFMT_LEFT, 420);
	m_list.InsertColumn(3, szFileCorporation[g_enumLang], LVCFMT_LEFT, 190);
	m_list.InsertColumn(4, szNote[g_enumLang], LVCFMT_LEFT, 250);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CListCallbackDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CListCallbackDlg::GetCallbacks()
{
	ULONG nCnt = 100;
	PGET_CALLBACK pCallbackInfo = NULL;
	OPERATE_TYPE ot = enumListCallback;
	BOOL bRet = FALSE;

	m_status.Format(szCallbackStatus[g_enumLang], 0, 0, 0, 0, 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_CallbackVector.clear();

	do 
	{
		ULONG nSize = sizeof(GET_CALLBACK) + nCnt * sizeof(CALLBACK_INFO);

		if (pCallbackInfo)
		{
			free(pCallbackInfo);
			pCallbackInfo = NULL;
		}

		pCallbackInfo = (PGET_CALLBACK)malloc(nSize);

		if (pCallbackInfo)
		{
			memset(pCallbackInfo, 0, nSize);
			pCallbackInfo->nCnt = nCnt;
			bRet = m_driver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pCallbackInfo, nSize, NULL);
		}

		nCnt = pCallbackInfo->nRetCnt + 10;

	} while (!bRet && pCallbackInfo->nRetCnt > pCallbackInfo->nCnt);

	if (bRet &&
		pCallbackInfo->nCnt >= pCallbackInfo->nRetCnt)
	{
		for (ULONG i = 0; i < pCallbackInfo->nRetCnt; i++)
		{
			m_CallbackVector.push_back(pCallbackInfo->Callbacks[i]);
		}
	}

	if (pCallbackInfo)
	{
		free(pCallbackInfo);
		pCallbackInfo = NULL;
	}
	
	GetDriver();
	InsertCallbackItem();
}

void CListCallbackDlg::InsertCallbackItem()
{
	ULONG nCreateProcess = 0, nCreateThread = 0, nLoadImage = 0, nShutdown = 0, nRegister = 0;

	for (vector<CALLBACK_INFO>::iterator itor = m_CallbackVector.begin(); itor != m_CallbackVector.end(); itor++)
	{
		CString szCallbackAddress, szType, szPath, szComp, szNote;
	
		szCallbackAddress.Format(L"0x%08X", itor->pCallbackAddress);
		int n = m_list.InsertItem(m_list.GetItemCount(), szCallbackAddress);

		switch (itor->type)
		{
		case enumCreateProcess:
			szType = L"CreateProcess";
			szNote = L"-";
			nCreateProcess++;
			break;

		case enumCreateThread:
			szType = L"CreateThread";
			szNote = L"-";
			nCreateThread++;
			break;

		case enumLoadImage:
			szType = L"LoadImage";
			szNote = L"-";
			nLoadImage++;
			break;

		case enumShutdown:
			szType = L"Shutdown";
			szNote.Format(L"DEVICE_OBJECT: 0x%08X", itor->Note);
			nShutdown++;
			break;

		case enumCmCallBack:
			szType = L"Registry";
			szNote = L"-";
			nRegister++;
			break;
		}

		szPath = GetDriverPathByCallbackAddress(itor->pCallbackAddress);
		szComp = m_Functions.GetFileCompanyName(szPath);

		m_list.SetItemText(n, 1, szType);
		m_list.SetItemText(n, 2, szPath);
		m_list.SetItemText(n, 3, szComp);
		m_list.SetItemText(n, 4, szNote);

		m_list.SetItemData(n, itor->type);
	}

	m_status.Format(szCallbackStatus[g_enumLang], nCreateProcess, nCreateThread, nLoadImage, nRegister, nShutdown);
	UpdateData(FALSE);
}

void CListCallbackDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

CString CListCallbackDlg::GetDriverPathByCallbackAddress(ULONG pCallback)
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

void CListCallbackDlg::OnCallbackRefresh()
{
	GetCallbacks();
}

void CListCallbackDlg::OnCallbackRemove()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szCallback = m_list.GetItemText(iIndex, 0);

		for ( vector <CALLBACK_INFO>::iterator Iter = m_CallbackVector.begin( ); Iter != m_CallbackVector.end( ); Iter++ )
		{
			CString strCallback;
			strCallback.Format(L"0x%08X", Iter->pCallbackAddress);
			if (!strCallback.CompareNoCase(szCallback))
			{
				COMMUNICATE_CALLBACK cC;
				cC.OpType = enumRemoveCallback;
				cC.Type = (CALLBACK_TYPE)m_list.GetItemData(iIndex);
				cC.Callback = Iter->pCallbackAddress;
				cC.Note = Iter->Note;
				m_driver.CommunicateDriver(&cC, sizeof(COMMUNICATE_CALLBACK), NULL, 0, NULL);
				break;
			}	
		}

		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}

//	OnCallbackRefresh();
}

void CListCallbackDlg::OnCallbackShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CListCallbackDlg::OnCallbackLocateModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.LocationExplorer(szPath);
	}
}

void CListCallbackDlg::OnCallbackCheckSign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);

		m_Functions.SignVerify(szPath);
	}
}

void CListCallbackDlg::OnCallbackText()
{
	m_Functions.ExportListToTxt(&m_list, m_status);
}

void CListCallbackDlg::OnCallbackExcel()
{
	WCHAR szCallback[] = {'C','a','l','l','b','a','c','k','\0'};
	m_Functions.ExportListToExcel(&m_list, szCallback, m_status);
}

void CListCallbackDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_CALLBACK_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_CALLBACK_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_CALLBACK_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CALLBACK_REMOVE, szRemove[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CALLBACK_REMOVE_ALL, szRemoveAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CALLBACK_DISASM, szDisassembleEntry[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CALLBACK_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_CALLBACK_CHECK_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_CALLBACK_LOCATE_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_CALLBACK_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_CALLBACK_REMOVE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_CALLBACK_REMOVE_ALL, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_CALLBACK_DISASM, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_CALLBACK_LOCATE_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_CALLBACK_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_CALLBACK_CHECK_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
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
			menu.EnableMenuItem(ID_CALLBACK_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_CALLBACK_LOCATE_MODULE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_CALLBACK_CHECK_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_CALLBACK_DISASM, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}

		menu.EnableMenuItem(ID_CALLBACK_REMOVE_ALL, MF_BYCOMMAND | MF_ENABLED);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CListCallbackDlg::OnCallbackRemoveAll()
{
	ULONG i = 0;

	for ( vector <CALLBACK_INFO>::iterator Iter = m_CallbackVector.begin( ); Iter != m_CallbackVector.end( ); Iter++ )
	{
		COMMUNICATE_CALLBACK cC;
		cC.OpType = enumRemoveCallback;
		cC.Type = (CALLBACK_TYPE)m_list.GetItemData(i);
		cC.Callback = Iter->pCallbackAddress;
		cC.Note = Iter->Note;
		m_driver.CommunicateDriver(&cC, sizeof(COMMUNICATE_CALLBACK), NULL, 0, NULL);
		i++;
	}

	m_list.DeleteAllItems();
}

void CListCallbackDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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

		strCompany = m_list.GetItemText(nItem, 3); 		
		if (!m_Functions.IsMicrosoftApp(strCompany))
		{
			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CListCallbackDlg::OnCallbackDisasm()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 0);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

BOOL CListCallbackDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CListCallbackDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 2);
		m_Functions.JmpToFile(szPath);
	}
}
