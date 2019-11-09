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
// DebugRegisterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "DebugRegisterDlg.h"


// CDebugRegisterDlg 对话框

IMPLEMENT_DYNAMIC(CDebugRegisterDlg, CDialog)

CDebugRegisterDlg::CDebugRegisterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugRegisterDlg::IDD, pParent)
	, m_szStatus(_T(""))
{

}

CDebugRegisterDlg::~CDebugRegisterDlg()
{
}

void CDebugRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_DEBUG_REGISTER_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CDebugRegisterDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDebugRegisterDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_COMMAND(ID_DEBUG_REGISTER_REFRESH, &CDebugRegisterDlg::OnDebugRegisterRefresh)
	ON_COMMAND(ID_DEBUG_REGISTER_RECOVER, &CDebugRegisterDlg::OnDebugRegisterRecover)
	ON_COMMAND(ID_DEBUGREGISTER_SHUXING, &CDebugRegisterDlg::OnDebugregisterShuxing)
	ON_COMMAND(ID_DEBUGREGISTER_LOCATION, &CDebugRegisterDlg::OnDebugregisterLocation)
	ON_COMMAND(ID_DEBUGREGISTER_VERIFY_SIGIN, &CDebugRegisterDlg::OnDebugregisterVerifySigin)
	ON_COMMAND(ID_DEBUG_REG_TEXT, &CDebugRegisterDlg::OnDebugRegText)
	ON_COMMAND(ID_DEBUG_REG_EXCEL, &CDebugRegisterDlg::OnDebugRegExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CDebugRegisterDlg::OnNMRclickList)
	ON_COMMAND(ID_DEBUGREGISTER_RECOVER_ALL, &CDebugRegisterDlg::OnDebugregisterRecoverAll)
	ON_COMMAND(ID_DEBUGREGISTER_DISASM, &CDebugRegisterDlg::OnDebugregisterDisasm)
	ON_COMMAND(ID_PROCESS_LOCATION_AT_FILE_MANAGER, &CDebugRegisterDlg::OnProcessLocationAtFileManager)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDebugRegisterDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_DEBUG_REGISTER_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CDebugRegisterDlg 消息处理程序

void CDebugRegisterDlg::OnBnClickedOk()
{
}

void CDebugRegisterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CDebugRegisterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szRegister[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(1, szRegisterValue[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(2, szBreakPointStatus[g_enumLang], LVCFMT_LEFT, 150);
	m_list.InsertColumn(3, szBreakOn[g_enumLang], LVCFMT_LEFT, 230);
	m_list.InsertColumn(4, szBreakpointLength[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(5, szBreakpointAddress[g_enumLang], LVCFMT_LEFT, 160);
	m_list.InsertColumn(6, szModulePath[g_enumLang], LVCFMT_LEFT, 355);
/*	m_list.InsertColumn(7, szFileCorporation[g_enumLang], LVCFMT_LEFT, 120);*/

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDebugRegisterDlg::GetDebugRegister()
{	
	if (!g_bLoadDriverOK)
	{
		return;
	}

//	PDEBUG_REGISTER_INFO pDebugRegisterInfo = NULL;
	PDEBUG_REGISTER pDebugRegisterInfo = NULL;
	OPERATE_TYPE ot = enumListDebugRegister;
	BOOL bRet = FALSE;

// 	m_status.Format(szIoTimerCnt[g_enumLang], 0);
// 	UpdateData(FALSE);

	m_list.DeleteAllItems();
/*	m_IoTimerVector.clear();*/

	do 
	{
		ULONG nSize = /*sizeof(DEBUG_REGISTER_INFO) + 32 * */sizeof(DEBUG_REGISTER);

		if (pDebugRegisterInfo)
		{
			free(pDebugRegisterInfo);
			pDebugRegisterInfo = NULL;
		}

//		pDebugRegisterInfo = (PDEBUG_REGISTER_INFO)malloc(nSize);
		pDebugRegisterInfo = (PDEBUG_REGISTER)malloc(nSize);

		if (pDebugRegisterInfo)
		{
			memset(pDebugRegisterInfo, 0, nSize);
//			pDebugRegisterInfo->nCnt = 0;
			bRet = m_driver.CommunicateDriver(&ot, sizeof(OPERATE_TYPE), (PVOID)pDebugRegisterInfo, nSize, NULL);
		}

	} while (0);

// 	if (pDebugRegisterInfo->nCnt)
// 	{
		GetDriver();
		InsertDrbugRegister( 0, pDebugRegisterInfo );

// 		for (ULONG i = 0; i < pDebugRegisterInfo->nCnt; i++)
// 		{
//			InsertDrbugRegister( 0, &(pDebugRegisterInfo->DebugRegister[0]));
// 		}
//	}

	if (pDebugRegisterInfo)
	{
		free(pDebugRegisterInfo);
		pDebugRegisterInfo = NULL;
	}
}

VOID CDebugRegisterDlg::GetBreakPointLengthAndRW(ULONG i, ULONG Dr7, int* nLen, int* nWR)
{
	if (Dr7 && nLen && nWR)
	{
		switch (i)
		{
		case 0:
			*nLen = Dr7 >> 18 & 3;
			*nWR = Dr7 >> 16 & 3;
			break;

		case 1:
			*nLen = Dr7 >> 22 & 3;
			*nWR = Dr7 >> 20 & 3;
			break;

		case 2:
			*nLen = Dr7 >> 26 & 3;
			*nWR = Dr7 >> 24 & 3;
			break;

		case 3:
			*nLen = Dr7 >> 30 & 3;
			*nWR = Dr7 >> 28 & 3;
			break;

		default: 
			*nLen = -1;
			*nWR = -1;
		}
	}
}

void CDebugRegisterDlg::InsertDrxRegister(ULONG i, ULONG Drx, ULONG Dr7)
{
	CString szDrx, szDrxValue, szAddress, szPath;
	szDrx.Format(L"DR%d", i);
	szDrxValue.Format(L"0x%08X", Drx);

	int n = m_list.InsertItem(m_list.GetItemCount(), szDrx);
	m_list.SetItemText(n, 1, szDrxValue);

	if (Drx == 0)
	{
		m_list.SetItemText(n, 2, L"-");
		m_list.SetItemText(n, 3, L"-");
		m_list.SetItemText(n, 4, L"-");
		m_list.SetItemText(n, 5, L"-");
		m_list.SetItemText(n, 6, L"");
/*		m_list.SetItemText(n, 7, L"");*/
	}
	else
	{
		CString szRW, szLen;
		CString szStatus;

		if (Dr7 >> i * 2 & 3)
		{
			if (Dr7 >> i * 2 & 1)
			{
				szStatus = L"Local Enable";
			}
			else if (Dr7 >> i * 2 & 2 )
			{
				szStatus = L"Global Enable";
			}

			int nLen = -1, nRW = -1;
			GetBreakPointLengthAndRW(i, Dr7, &nLen, &nRW);

			switch (nRW)
			{
			case 0:
				szAddress = szDrxValue;
				szRW = L"Instruction Execution";
				szPath = GetDriverPathByCallbackAddress(Drx);
				break;

			case 1:
				szAddress = szDrxValue;
				szRW = L"Data Writes";
				szPath = GetDriverPathByCallbackAddress(Drx);
				break;

			case 2:
				szAddress.Format(L"0x%X I/O Port Is Listend", Drx);
				szRW = L"I/O Reads or Wirtes";
				szPath = L"-";
				break;

			case 3:
				szAddress = szDrxValue;
				//szRW = L"Data Reads or Writes But Not Instruction Fetches";
				szRW = L"Data Reads or Writes";
				szPath = GetDriverPathByCallbackAddress(Drx);
				break;

			default:
				szRW = L"-";
			}

			switch (nLen)
			{
			case 0:
				szLen = L"1 byte";
				break;

			case 1:
				szLen = L"2 bytes";
				break;

			case 2:
				szLen = L"Undefined";
				break;

			case 3:
				szLen = L"4 bytes";
				break;

			default:
				szLen = L"-";
			}
		}
		else
		{
			szStatus = L"BreakPoint Disabled";
			szRW = L"-";
			szLen = L"-";
			szAddress = L"-";
			szPath = L"-";
		}

		m_list.SetItemText(n, 2, szStatus);
		m_list.SetItemText(n, 3, szRW);
		m_list.SetItemText(n, 4, szLen);
		m_list.SetItemText(n, 5, szAddress);
		m_list.SetItemText(n, 6, szPath);
/*		m_list.SetItemText(n, 7, L"");*/
	}
}

void CDebugRegisterDlg::InsertDrbugRegister(ULONG i, PDEBUG_REGISTER Registers)
{
	CString szDr6, szDr7;
	CString szCpuI;

// 	szCpuI.Format(L"CPU %d:", i);
// 	int n = m_list.InsertItem(m_list.GetItemCount(), szCpuI);
// 	m_list.SetItemText(n, 1, L" ");
// 	m_list.SetItemText(n, 2, L" ");
// 	m_list.SetItemText(n, 3, L" ");
// 	m_list.SetItemText(n, 4, L" ");
// 	m_list.SetItemText(n, 5, L" ");
// 	m_list.SetItemText(n, 6, L" ");

	szDr6.Format(L"0x%08X", Registers->DR6);
	szDr7.Format(L"0x%08X", Registers->DR7);

	InsertDrxRegister(0, Registers->DR0, Registers->DR7);
	InsertDrxRegister(1, Registers->DR1, Registers->DR7);
	InsertDrxRegister(2, Registers->DR2, Registers->DR7);
	InsertDrxRegister(3, Registers->DR3, Registers->DR7);

	int n = m_list.InsertItem(m_list.GetItemCount(), L"DR6");
	m_list.SetItemText(n, 1, szDr6);
	m_list.SetItemText(n, 2, L"");
	m_list.SetItemText(n, 3, L"");
	m_list.SetItemText(n, 4, L"");
	m_list.SetItemText(n, 5, L"");
	m_list.SetItemText(n, 6, L"");

	n = m_list.InsertItem(m_list.GetItemCount(), L"DR7");
	m_list.SetItemText(n, 1, szDr7);
	m_list.SetItemText(n, 2, L"");
	m_list.SetItemText(n, 3, L"");
	m_list.SetItemText(n, 4, L"");
	m_list.SetItemText(n, 5, L"");
	m_list.SetItemText(n, 6, L"");

// 	n = m_list.InsertItem(m_list.GetItemCount(), L"");
// 	m_list.SetItemText(n, 1, L" ");
// 	m_list.SetItemText(n, 2, L" ");
// 	m_list.SetItemText(n, 3, L" ");
// 	m_list.SetItemText(n, 4, L" ");
// 	m_list.SetItemText(n, 5, L" ");
// 	m_list.SetItemText(n, 6, L" ");

	int nCnt = 0;
	if (Registers->DR0)
	{
		nCnt++;
	}
	if (Registers->DR1)
	{
		nCnt++;
	}
	if (Registers->DR2)
	{
		nCnt++;
	}
	if (Registers->DR3)
	{
		nCnt++;
	}
	m_szStatus.Format(szHardwareBreakpoint[g_enumLang], nCnt);
	UpdateData(FALSE);
}

CString CDebugRegisterDlg::GetDriverPathByCallbackAddress(ULONG pCallback)
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

void CDebugRegisterDlg::GetDriver()
{
	m_clsDrivers.ListDrivers(m_CommonDriverList);
}

void CDebugRegisterDlg::OnDebugRegisterRefresh()
{
	GetDebugRegister();
}

void CDebugRegisterDlg::OnDebugRegisterRecover()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1 && nItem < 4)
	{
		CString szDRX = m_list.GetItemText(nItem, 0);
		DRX_REG eDRX = enumDRUnknow;

		if (!szDRX.CompareNoCase(L"DR0"))
		{
			eDRX = enumDR0;
		}
		else if (!szDRX.CompareNoCase(L"DR1"))
		{
			eDRX = enumDR1;
		}
		else if (!szDRX.CompareNoCase(L"DR2"))
		{
			eDRX = enumDR2;
		}
		else if (!szDRX.CompareNoCase(L"DR3"))
		{
			eDRX = enumDR3;
		}

		if (eDRX != enumDRUnknow)
		{
			RECOVER_DEBUG_REGISTER rer;
			rer.OpType = enumRecoverDebugRegister;
			rer.RegType = eDRX;
			m_driver.CommunicateDriver(&rer, sizeof(RECOVER_DEBUG_REGISTER), NULL, 0, NULL);
			OnDebugRegisterRefresh();
		}
	}
}

void CDebugRegisterDlg::OnDebugregisterShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CDebugRegisterDlg::OnDebugregisterLocation()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);
		m_Functions.LocationExplorer(szPath);
	}
}

void CDebugRegisterDlg::OnDebugregisterVerifySigin()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);
		m_Functions.SignVerify(szPath);
	}
}

void CDebugRegisterDlg::OnDebugRegText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CDebugRegisterDlg::OnDebugRegExcel()
{
	WCHAR szDebugRegister[] = {'D','e','b','u','g','R','e','g','i','s','t','e','r','\0'};
	m_Functions.ExportListToExcel(&m_list, szDebugRegister, m_szStatus);
}

void CDebugRegisterDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_DEBUG_REG_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_DEBUG_REG_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_DEBUG_REGISTER_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DEBUG_REGISTER_RECOVER, szRestore[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DEBUGREGISTER_RECOVER_ALL, szRestoreAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DEBUGREGISTER_DISASM, szDisassembleBreakpoint[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DEBUGREGISTER_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DEBUGREGISTER_VERIFY_SIGIN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DEBUGREGISTER_LOCATION, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PROCESS_LOCATION_AT_FILE_MANAGER, szFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_DEBUG_REGISTER_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_DEBUG_REGISTER_RECOVER, MF_BYCOMMAND, &m_bmRecover, &m_bmRecover);
		menu.SetMenuItemBitmaps(ID_DEBUGREGISTER_RECOVER_ALL, MF_BYCOMMAND, &m_bmRecover, &m_bmRecover);
		menu.SetMenuItemBitmaps(ID_DEBUGREGISTER_DISASM, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_DEBUGREGISTER_LOCATION, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_DEBUGREGISTER_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_DEBUGREGISTER_VERIFY_SIGIN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
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
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt == 0)
		{
			for (int i = 2; i < 13; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (nCnt == 1)
		{
			int n = m_Functions.GetSelectItem(&m_list);
			if (n >= 4)
			{
				for (int i = 2; i < 12; i++)
				{
					menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
				}
			}
			else
			{
				CString szStatus = m_list.GetItemText(n, 2);
				if (!szStatus.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_DEBUG_REGISTER_RECOVER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				CString szAddress = m_list.GetItemText(n, 5);
				if (!szAddress.CompareNoCase(L"-") || szAddress.IsEmpty())
				{
					menu.EnableMenuItem(ID_DEBUGREGISTER_DISASM, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}

				CString szModule = m_list.GetItemText(n, 6);
				if (!szModule.CompareNoCase(L"-") || szModule.IsEmpty())
				{
					menu.EnableMenuItem(ID_DEBUGREGISTER_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_DEBUGREGISTER_LOCATION, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_DEBUGREGISTER_VERIFY_SIGIN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PROCESS_LOCATION_AT_FILE_MANAGER, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}
	
		CString szStatusDr0 = m_list.GetItemText(0, 2);
		CString szStatusDr1 = m_list.GetItemText(1, 2);
		CString szStatusDr2 = m_list.GetItemText(2, 2);
		CString szStatusDr3 = m_list.GetItemText(3, 2);

		if (szStatusDr0.CompareNoCase(L"-") ||
			szStatusDr1.CompareNoCase(L"-") ||
			szStatusDr2.CompareNoCase(L"-") ||
			szStatusDr3.CompareNoCase(L"-") )
		{
			menu.EnableMenuItem(ID_DEBUGREGISTER_RECOVER_ALL, MF_ENABLED);
		}
		else
		{
			menu.EnableMenuItem(ID_DEBUGREGISTER_RECOVER_ALL, MF_GRAYED | MF_DISABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CDebugRegisterDlg::OnDebugregisterRecoverAll()
{
	RECOVER_DEBUG_REGISTER rer;
	rer.OpType = enumRecoverDebugRegister;
	rer.RegType = enumDRAll;
	m_driver.CommunicateDriver(&rer, sizeof(RECOVER_DEBUG_REGISTER), NULL, 0, NULL);
	OnDebugRegisterRefresh();
}

void CDebugRegisterDlg::OnDebugregisterDisasm()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 5);
		DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
		if (dwAddress > 0)
		{
			m_Functions.Disassemble(dwAddress);
		}
	}
}

BOOL CDebugRegisterDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CDebugRegisterDlg::OnProcessLocationAtFileManager()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 6);
		m_Functions.JmpToFile(szPath);
	}
}
