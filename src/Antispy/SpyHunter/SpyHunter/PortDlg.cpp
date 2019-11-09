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
// PortDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "PortDlg.h"
#include <winsock.h>

// CPortDlg 对话框

IMPLEMENT_DYNAMIC(CPortDlg, CDialog)

CPortDlg::CPortDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPortDlg::IDD, pParent)
	, m_szPortStatus(_T(""))
{

}

CPortDlg::~CPortDlg()
{
}

void CPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_PORT_STATUS, m_szPortStatus);
}


BEGIN_MESSAGE_MAP(CPortDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CPortDlg::OnNMRclickList)
	ON_COMMAND(ID_PORT_REFRESH, &CPortDlg::OnPortRefresh)
	ON_COMMAND(ID_PORT_COPY_ALL_ROW, &CPortDlg::OnPortCopyAllRow)
	ON_COMMAND(ID_PORT_COPY_REMOTE_ADDRESS, &CPortDlg::OnPortCopyRemoteAddress)
	ON_COMMAND(ID_PORT_WHOIS, &CPortDlg::OnPortWhois)
	ON_COMMAND(ID_PORT_SHUXING, &CPortDlg::OnPortShuxing)
	ON_COMMAND(ID_PORT_SIGN, &CPortDlg::OnPortSign)
	ON_COMMAND(ID_PORT_EXPLORER, &CPortDlg::OnPortExplorer)
	ON_COMMAND(ID_PORT_ANTISPY, &CPortDlg::OnPortAntispy)
	ON_COMMAND(ID_PORT_TEXT, &CPortDlg::OnPortText)
	ON_COMMAND(ID_PORT_EXCEL, &CPortDlg::OnPortExcel)
	ON_COMMAND(ID_PORT_KILL_PROCESS, &CPortDlg::OnPortKillProcess)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CPortDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_PORT_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CPortDlg 消息处理程序

void CPortDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CPortDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_imageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 2, 2); 
	m_list.SetImageList (&m_imageList, LVSIL_SMALL);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szPortProcess[g_enumLang], LVCFMT_LEFT, 180);
	m_list.InsertColumn(1, szPortPID[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(2, szPortProtocol[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(3, szPortLocalAddress[g_enumLang], LVCFMT_LEFT, 210);
	m_list.InsertColumn(4, szPortLocalPort[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(5, szPortRemoteAddress[g_enumLang], LVCFMT_LEFT, 210);
	m_list.InsertColumn(6, szPortRemotePort[g_enumLang], LVCFMT_LEFT, 100);
	m_list.InsertColumn(7, szPortState[g_enumLang], LVCFMT_LEFT, 180);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CPortDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CPortDlg::EnumPort()
{	
	if (!g_bLoadDriverOK)
	{
		return;
	}

	ULONG dwCnt, dwCntTemp;
	PCOMMUNICATE_PORT pPortInfo = NULL;
	BOOL bRet = FALSE;
	OPERATE_TYPE OpType = enumEnumPort;

	dwCnt = dwCntTemp = 1000;

	do 
	{
		dwCnt = dwCntTemp;
		DWORD dwLen = sizeof(COMMUNICATE_PORT) + dwCnt * sizeof(PORT_INFO);
		if (pPortInfo)
		{
			free(pPortInfo);
			pPortInfo = NULL;
		}

		pPortInfo = (PCOMMUNICATE_PORT)malloc(dwLen);
		if (!pPortInfo)
		{
			return;
		}

		memset(pPortInfo, 0, dwLen);

		bRet = m_Driver.CommunicateDriver(&OpType, sizeof(OPERATE_TYPE), pPortInfo, dwLen, NULL);

	} while ( !bRet && (dwCntTemp = pPortInfo->nCnt) > dwCnt );

	if (bRet && pPortInfo->nCnt > 0)
	{
		InsertPortInfo(pPortInfo);
		DebugLog(L"nRetCount: %d", pPortInfo->nCnt);
	}

	free(pPortInfo);
}

WCHAR *szConnectStatus[] = {
	L"UNKNOW",
	L"CLOSED",
	L"LISTENING",
	L"SYN_SENT",
	L"SYN_RECEIVED",
	L"ESTABLISHED",
	L"FIN_WAIT1",
	L"FIN_WAIT2",
	L"CLOSE_WAIT",
	L"CLOSING",
	L"LAST_ACK",
	L"TIME_WAIT",
};

void CPortDlg::InsertPortInfo(PCOMMUNICATE_PORT pCp)
{
	if (!pCp)
	{
		return;
	}
	
	if (pCp->nCnt <= 0)
	{
		return;
	}
	
	m_portList.clear();
	m_list.DeleteAllItems();

	vector<PROCESS_INFO> vectorProcess;
	m_clsProcess.EnumProcess(vectorProcess);
	
	int  nImageCount = m_imageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_imageList.Remove(0);   
	}

	ULONG nCnt = pCp->nCnt;
	PPORT_INFO pInfo = pCp->Ports;
	WCHAR szUdp[] = {'U','d','p','\0'};
	WCHAR szTcp[] = {'T','c','p','\0'};

	for (ULONG i = 0; i < nCnt; i++)
	{
		CONNECT_NET_INFO info;

		for ( vector <PROCESS_INFO>::iterator IterProcess = vectorProcess.begin( ); 
			IterProcess != vectorProcess.end( ); 
			IterProcess++ )
		{
			if (IterProcess->ulPid == pInfo[i].nPid)
			{
				info.szProcess = IterProcess->szPath;
				info.pEprocess = IterProcess->ulEprocess;
				break;
			}
		}

		DebugLog(L"state: %d", pInfo[i].nConnectState);
		info.nConnectState = pInfo[i].nConnectState;
		info.nLocalAddress = pInfo[i].nLocalAddress;
		info.nLocalPort = pInfo[i].nLocalPort;
		info.nPid = pInfo[i].nPid;
		info.nPortType = pInfo[i].nPortType;
		info.nRemoteAddress = pInfo[i].nRemoteAddress;
		info.nRemotePort = pInfo[i].nRemotePort;

		m_portList.push_back(info);
	}

	ULONG nUdp = 0, nTcp = 0;
	for (vector<CONNECT_NET_INFO>::iterator ir = m_portList.begin();
		ir != m_portList.end();
		ir++)
	{
		SHFILEINFO sfi;
		DWORD_PTR nRet;
		ZeroMemory(&sfi,sizeof(SHFILEINFO));
		nRet = SHGetFileInfo(ir->szProcess, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		//nRet ? m_imageList.Add ( sfi.hIcon ) : m_imageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
		sfi.hIcon ? m_imageList.Add ( sfi.hIcon ) : m_imageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
		if (nRet)
		{
			DestroyIcon(sfi.hIcon);
		}

		CString strPath = ir->szProcess;
		CString szImage = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

		int nItem = m_list.GetItemCount();
		m_list.InsertItem(nItem, szImage, nItem);

		CString szPid;
		szPid.Format(L"%d", ir->nPid);
		m_list.SetItemText(nItem, 1, szPid);

		CString szProtocol;
		if (ir->nPortType == enumTcp)
			szProtocol = szTcp;
		else if (ir->nPortType == enumUdp)
			szProtocol = szUdp;
		m_list.SetItemText(nItem, 2, szProtocol);

		char lpszLocalAddr[100] = {0};
		WCHAR lpwzLocalAddr[100] = {0};
		struct in_addr InLocaleAddr;
		memcpy(&InLocaleAddr, &ir->nLocalAddress, 4);
		wsprintfA(lpszLocalAddr, "%s", inet_ntoa(InLocaleAddr));
		MultiByteToWideChar(
			CP_ACP,
			0, 
			lpszLocalAddr,
			-1, 
			lpwzLocalAddr, 
			strlen(lpszLocalAddr)
			);
		CString szLocalAddress = lpwzLocalAddr;
		m_list.SetItemText(nItem, 3, szLocalAddress);

		CString szLocalPort;
		szLocalPort.Format(L"%d", ir->nLocalPort);
		m_list.SetItemText(nItem, 4, szLocalPort);
		
		if (ir->nPortType == enumTcp)
		{
			char lpszRemoteAddr[100] = {0};
			WCHAR lpwzRemoteAddr[100] = {0};
			struct in_addr InRemoteAddr;
			memcpy(&InRemoteAddr, &ir->nRemoteAddress, 4);
			wsprintfA(lpszRemoteAddr, "%s", inet_ntoa(InRemoteAddr));
			MultiByteToWideChar(
				CP_ACP,
				0, 
				lpszRemoteAddr,
				-1, 
				lpwzRemoteAddr, 
				strlen(lpszRemoteAddr)
				);
			CString szRemoteAddress = lpwzRemoteAddr;
			m_list.SetItemText(nItem, 5, szRemoteAddress);

			CString szRemotePort;
			szRemotePort.Format(L"%d", ir->nRemotePort);
			m_list.SetItemText(nItem, 6, szRemotePort);
			
			m_list.SetItemText(nItem, 7, szConnectStatus[ir->nConnectState]);

			nTcp++;
		}
		else if (ir->nPortType == enumUdp)
		{
			m_list.SetItemText(nItem, 5, L"*");
			m_list.SetItemText(nItem, 6, L"*");

			nUdp++;
		}
		
		m_list.SetItemData(nItem, (DWORD_PTR)nItem);
	}

	m_szPortStatus.Format(szTcpUdpStatus[g_enumLang], nTcp, nUdp);
	UpdateData(FALSE);
}

void CPortDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_PORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_PORT_EXCEL, szExcel[g_enumLang]);

	WCHAR szWhois[] = {'W','h','o','i','s','.','.','.','\0'};
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PORT_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PORT_KILL_PROCESS, szPortKillProcess[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PORT_COPY_ALL_ROW, szPortCopyFullRow[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PORT_COPY_REMOTE_ADDRESS, szPortCopyRemoteAddress[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PORT_WHOIS, szWhois);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PORT_SHUXING, szPortProcessProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PORT_SIGN, szPortVerifyProcessSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PORT_EXPLORER, szPortFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PORT_ANTISPY, szPortFindInAntiSpy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	// 设置菜单图标
	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_PORT_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_PORT_EXPLORER, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PORT_KILL_PROCESS, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_PORT_ANTISPY, MF_BYCOMMAND, &m_bmAntiSpy, &m_bmAntiSpy);
		menu.SetMenuItemBitmaps(ID_PORT_WHOIS, MF_BYCOMMAND, &m_bmSearch, &m_bmSearch);
		menu.SetMenuItemBitmaps(ID_PORT_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_PORT_COPY_ALL_ROW, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_PORT_COPY_REMOTE_ADDRESS, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(ID_PORT_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(15, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 16; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		
		ULONG nCnt = m_list.GetSelectedCount();
		if (nCnt > 1)
		{
			for (int i = 2; i < 15; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_PORT_COPY_ALL_ROW, MF_BYCOMMAND | MF_ENABLED);
		}
		else if (nCnt == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				CString szAddr = m_list.GetItemText(nItem, 5);
				if (szAddr.IsEmpty() || !szAddr.CompareNoCase(L"*"))
				{
					menu.EnableMenuItem(ID_PORT_WHOIS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_PORT_COPY_REMOTE_ADDRESS, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}
		else if (nCnt == 0)
		{
			for (int i = 2; i < 15; i++)
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

void CPortDlg::OnPortRefresh()
{
	EnumPort();
}

void CPortDlg::OnPortCopyAllRow()
{
	CHeaderCtrl *pHeaderCtrl = m_list.GetHeaderCtrl();
	int nColCnt = pHeaderCtrl->GetItemCount();
	CString szString;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		for (int i = 0; i < nColCnt; i++)
		{
			CString szStringTemp = m_list.GetItemText(iIndex, i);
			szString += szStringTemp;
			if (i == 0)
			{	
				szString += L"\t";
			}
			else
			{
				if (szStringTemp.GetLength() >= 16)
				{
					szString += L"\t";
				}
				else if (szStringTemp.GetLength() >= 8)
				{
					szString += L"\t\t";
				}
				else
				{
					szString += L"\t\t\t";
				}
			}	
		}

		szString += L"\r\n";
	}

	m_Functions.SetStringToClipboard(szString);
}

void CPortDlg::OnPortCopyRemoteAddress()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szString = m_list.GetItemText(nItem, 5);
		m_Functions.SetStringToClipboard(szString);
	}
}

void CPortDlg::OnPortWhois()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szString = m_list.GetItemText(nItem, 5);
		CString strCmd;
		WCHAR szUrl[] = {'h','t','t','p',':','/','/','w','h','o','i','s','.','d','o','m','a','i','n','t','o','o','l','s','.','c','o','m','/','%','s','\0'};
		WCHAR szOpen[] = {'o','p','e','n','\0'};
		strCmd.Format(szUrl, szString);
		ShellExecuteW(NULL, szOpen, strCmd, NULL, NULL, SW_SHOW);
	}
}

void CPortDlg::OnPortShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		CString szPath = m_portList.at(nData).szProcess;
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CPortDlg::OnPortSign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		CString szPath = m_portList.at(nData).szProcess;
		m_Functions.SignVerify(szPath);
	}
}

void CPortDlg::OnPortExplorer()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		CString szPath = m_portList.at(nData).szProcess;
		m_Functions.LocationExplorer(szPath);
	}
}

void CPortDlg::OnPortAntispy()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		CString szPath = m_portList.at(nData).szProcess;
		m_Functions.JmpToFile(szPath);
	}
}

void CPortDlg::OnPortText()
{
	m_Functions.ExportListToTxt(&m_list, m_szPortStatus);
}

void CPortDlg::OnPortExcel()
{
	WCHAR szProt[] = {'P','o','r','t','\0'};
	m_Functions.ExportListToExcel(&m_list, szProt, m_szPortStatus);
}

void CPortDlg::OnPortKillProcess()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		ULONG dwEprocess = m_portList.at(nData).pEprocess;

		m_clsProcess.KillProcess(0, dwEprocess);

		OnPortRefresh();
	}
}