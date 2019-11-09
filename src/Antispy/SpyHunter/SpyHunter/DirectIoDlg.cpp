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
// DirectIoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "DirectIoDlg.h"
#include "SignVerifyDlg.h"

// CDirectIoDlg 对话框

IMPLEMENT_DYNAMIC(CDirectIoDlg, CDialog)

CDirectIoDlg::CDirectIoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDirectIoDlg::IDD, pParent)
	, m_szStatus(_T(""))
{

}

CDirectIoDlg::~CDirectIoDlg()
{
	m_DirectIoProcessVector.clear();
}

void CDirectIoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_DIRECT_IO_CNT, m_szStatus);
}


BEGIN_MESSAGE_MAP(CDirectIoDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CDirectIoDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CDirectIoDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_DIRECTIO_REFRESH, &CDirectIoDlg::OnDirectioRefresh)
	ON_COMMAND(ID_DIRECTIO_DENY_DIRECT_IO, &CDirectIoDlg::OnDirectioDenyDirectIo)
	ON_COMMAND(ID_DIRECTIO_KILL_PROCESS, &CDirectIoDlg::OnDirectioKillProcess)
	ON_COMMAND(ID_DIRECTIO_KILL_AND_DELETE_PROCESS, &CDirectIoDlg::OnDirectioKillAndDeleteProcess)
	ON_COMMAND(ID_DIRECTIO_LOCAL_FILE, &CDirectIoDlg::OnDirectioLocalFile)
	ON_COMMAND(ID_DIRECTIO_SHUXING, &CDirectIoDlg::OnDirectioShuxing)
	ON_COMMAND(ID_DIRECTIO_VERIFY_SIGN, &CDirectIoDlg::OnDirectioVerifySign)
	ON_COMMAND(ID_DIRECTIO_TEXT, &CDirectIoDlg::OnDirectioText)
	ON_COMMAND(ID_DIRECTIO_EXCEL, &CDirectIoDlg::OnDirectioExcel)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CDirectIoDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDirectIoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_DIRECT_IO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CDirectIoDlg 消息处理程序


void CDirectIoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CDirectIoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szImageName[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, szPid[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(2, szProcessObject[g_enumLang], LVCFMT_LEFT, 120);
	m_list.InsertColumn(3, szDirectIOReason[g_enumLang], LVCFMT_LEFT, 290);
	m_list.InsertColumn(4, szImagePath[g_enumLang], LVCFMT_LEFT, 380);
	m_list.InsertColumn(5, szFileCorporation[g_enumLang], LVCFMT_LEFT, 160);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDirectIoDlg::GetDirectIoProcess()
{
	OPERATE_TYPE opType = enumGetDirectIoProcess;
	ULONG nCnt = 100;
	PDIRECT_IO_PROCESS pProcessInfo = NULL;
	BOOL bRet = FALSE;

	m_szStatus.Format(szDirectIOProcessCnt[g_enumLang], 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_DirectIoProcessVector.clear();

	do 
	{
		ULONG nSize = sizeof(DIRECT_IO_PROCESS) + nCnt * sizeof(DIRECT_IO_PROCESS_INFO);

		if (pProcessInfo)
		{
			free(pProcessInfo);
			pProcessInfo = NULL;
		}

		pProcessInfo = (PDIRECT_IO_PROCESS)malloc(nSize);

		if (pProcessInfo)
		{
			memset(pProcessInfo, 0, nSize);
			pProcessInfo->nCnt = nCnt;
			bRet = m_Driver.CommunicateDriver(&opType, sizeof(OPERATE_TYPE), (PVOID)pProcessInfo, nSize, NULL);
		}

		nCnt = pProcessInfo->nRetCnt + 10;

	} while (pProcessInfo->nRetCnt > pProcessInfo->nCnt);

	if (bRet && pProcessInfo->nCnt >= pProcessInfo->nRetCnt)
	{
		for (ULONG i = 0; i < pProcessInfo->nRetCnt; i++)
		{
			m_DirectIoProcessVector.push_back(pProcessInfo->Process[i]);
		}
	}

	if (pProcessInfo)
	{
		free(pProcessInfo);
		pProcessInfo = NULL;
	}

	InsertDirectIoProcess();
}

CString CDirectIoDlg::GetDirectIoReason(DIRECT_IO_TYPE Type)
{
	CString szReason;
	
	if (Type & enumIOPL)
	{
		szReason = szIOPLIsTrue[g_enumLang];
		szReason += L" & ";
	}

	if (Type & enumTSSLarge)
	{
		szReason += szTssIsLargerThanNormal[g_enumLang];
		szReason += L" & ";
	}

	if (Type & enumIOPLOffsetChange)
	{
		szReason += szIoMapIsChanged[g_enumLang];
	}
	
	if (!szReason.IsEmpty())
	{
		szReason.TrimRight(' ');
		szReason.TrimRight('&');
		szReason.TrimRight(' ');
	}

	return szReason;
}

void CDirectIoDlg::InsertDirectIoProcess()
{
	for (vector<DIRECT_IO_PROCESS_INFO>::iterator itor = m_DirectIoProcessVector.begin(); itor != m_DirectIoProcessVector.end(); itor++)
	{
		CString szImage, szPath, szFileCorporation, szPid, szEprocess, szReason;

		szPath = m_Functions.TrimPath(itor->szPath);
		szImage = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
		szPid.Format(L"%d", itor->Pid);
		szEprocess.Format(L"0x%08X", itor->pEprocess);
		szReason = GetDirectIoReason(itor->Type);
		
		int n = m_list.InsertItem(m_list.GetItemCount(), szImage);
		m_list.SetItemText(n, 1, szPid);
		m_list.SetItemText(n, 2, szEprocess);
		m_list.SetItemText(n, 3, szReason);
		m_list.SetItemText(n, 4, szPath);
		m_list.SetItemText(n, 5, m_Functions.GetFileCompanyName(szPath));
	}

	m_szStatus.Format(szDirectIOProcessCnt[g_enumLang], m_DirectIoProcessVector.size());
	UpdateData(FALSE);
}

void CDirectIoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_DIRECTIO_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_DIRECTIO_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_DIRECTIO_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DIRECTIO_DENY_DIRECT_IO, szDisableDirectIo[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DIRECTIO_KILL_PROCESS, szDirectIOKillProcess[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DIRECTIO_KILL_AND_DELETE_PROCESS, szKillAndDeleteFile[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DIRECTIO_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DIRECTIO_VERIFY_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DIRECTIO_LOCAL_FILE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_DIRECTIO_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_DIRECTIO_KILL_PROCESS, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_DIRECTIO_KILL_AND_DELETE_PROCESS, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_FORCE_RESUME_THREAD, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_DIRECTIO_LOCAL_FILE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_DIRECTIO_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_DIRECTIO_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
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
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CDirectIoDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
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
		if (!m_Functions.IsMicrosoftApp(strCompany) && m_list.GetItemText(nItem, 4).CompareNoCase(L"System"))
		{
			clrNewTextColor = g_NormalItemClr;//RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

void CDirectIoDlg::OnDirectioRefresh()
{
	GetDirectIoProcess();
}

void CDirectIoDlg::OnDirectioDenyDirectIo()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szEprocess = m_list.GetItemText(nItem, 2);

		for ( vector <DIRECT_IO_PROCESS_INFO>::iterator Iter = m_DirectIoProcessVector.begin( ); Iter != m_DirectIoProcessVector.end( ); Iter++)
		{
			CString strEprocess;
			strEprocess.Format(L"0x%08X", Iter->pEprocess);
			if (!strEprocess.CompareNoCase(szEprocess))
			{
				COMMUNICATE_DIRECTIO cd;
				cd.OpType = enumDisableDirectIo;
				cd.op.Disable.pEprocess = Iter->pEprocess;
				cd.op.Disable.pid = Iter->Pid;
				cd.op.Disable.Type = Iter->Type;
				m_Driver.CommunicateDriver(&cd, sizeof(COMMUNICATE_DIRECTIO), NULL, 0, NULL);

				break;
			}
		}

		OnDirectioRefresh();
	}
}

void CDirectIoDlg::OnDirectioKillProcess()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();

	if (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szProcessObject = m_list.GetItemText(iIndex, 2);
		ULONG pEprocess = m_Functions.HexStringToLong(szProcessObject);

		if (pEprocess)
		{
			COMMUNICATE_PROCESS cP;
			cP.OpType = enumKillProcess;
			cP.op.KillProcess.pEprocess = pEprocess;
			m_Driver.CommunicateDriver(&cP, sizeof(COMMUNICATE_PROCESS), NULL, 0, NULL);

			m_list.DeleteItem(iIndex);
		}
	}
}

void CDirectIoDlg::OnDirectioKillAndDeleteProcess()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);
		if (!szPath.IsEmpty())
		{
			m_Functions.KernelDeleteFile(szPath);
		}
	}

	OnDirectioKillProcess();
}

void CDirectIoDlg::OnDirectioLocalFile()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);

		if (!szPath.IsEmpty())
		{
			m_Functions.LocationExplorer(szPath);
		}
	}
}

void CDirectIoDlg::OnDirectioShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);

		if (!szPath.IsEmpty())
		{
			m_Functions.OnCheckAttribute(szPath);
		}
	}
}

void CDirectIoDlg::OnDirectioVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);

		if (!szPath.IsEmpty())
		{
			m_Functions.SignVerify(szPath);
		}
	}
}

void CDirectIoDlg::OnDirectioText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CDirectIoDlg::OnDirectioExcel()
{
	WCHAR szDirectIO[] = {'D','i','r','e','c','t','I','O','\0'};
	m_Functions.ExportListToExcel(&m_list, szDirectIO, m_szStatus);
}

BOOL CDirectIoDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CDirectIoDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 4);

		if (!szPath.IsEmpty())
		{
			m_Functions.JmpToFile(szPath);
		}
	}
}
