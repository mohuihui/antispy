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
// DisassemblerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "DisassemblerDlg.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "ODEngine/disasm.h"
#ifdef __cplusplus
}
#endif

// CDisassemblerDlg 对话框

IMPLEMENT_DYNAMIC(CDisassemblerDlg, CDialog)

CDisassemblerDlg::CDisassemblerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisassemblerDlg::IDD, pParent)
	, m_bKernelMode(FALSE)
	, m_szAddress(_T(""))
	, m_szSize(_T(""))
	, m_szPid(_T(""))
	, m_szStaticAddress(_T(""))
	, m_szStaticSize(_T(""))
	, m_szStaticPid(_T(""))
{
	m_dwSize = 0;
	m_dwAddress = 0;
	m_dwPid = 0;
}

CDisassemblerDlg::~CDisassemblerDlg()
{
}

void CDisassemblerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_KERNEL_ADDRESS, m_bKernelMode);
	DDX_Text(pDX, IDC_EDIT_ADDRESS, m_szAddress);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_szSize);
	DDX_Text(pDX, IDC_EDIT_PID, m_szPid);
	DDX_Text(pDX, IDC_STATIC_ADDRESS, m_szStaticAddress);
	DDX_Text(pDX, IDC_STATIC_SIZE, m_szStaticSize);
	DDX_Text(pDX, IDC_STATIC_PID, m_szStaticPid);
	DDX_Control(pDX, IDC_CHECK_KERNEL_ADDRESS, m_btnKernelMode);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_PROCESS_LIST, m_ProcessList);
}


BEGIN_MESSAGE_MAP(CDisassemblerDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDisassemblerDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CDisassemblerDlg::OnNMRclickList)
	ON_BN_CLICKED(IDC_CHECK_KERNEL_ADDRESS, &CDisassemblerDlg::OnBnClickedCheckKernelAddress)
	ON_COMMAND(ID_DISEMBLY_REFLASH, &CDisassemblerDlg::OnDisemblyReflash)
	ON_COMMAND(ID_DISEMBLY_IMEDIATE, &CDisassemblerDlg::OnDisemblyImediate)
	ON_COMMAND(ID_DISEMBLY_COPY_FULLROW, &CDisassemblerDlg::OnDisemblyCopyFullrow)
	ON_COMMAND(ID_DISEMBLY_COPY_ADDRESS, &CDisassemblerDlg::OnDisemblyCopyAddress)
	ON_COMMAND(ID_DISEMBLY_COPY_HEX, &CDisassemblerDlg::OnDisemblyCopyHex)
	ON_COMMAND(ID_DISEMBLY_COPY_CODE, &CDisassemblerDlg::OnDisemblyCopyCode)
	ON_COMMAND(ID_DISEMBLY_LOWERCASE, &CDisassemblerDlg::OnDisemblyLowercase)
	ON_COMMAND(ID_DISEMBLY_EXPORT_TEXT, &CDisassemblerDlg::OnDisemblyExportText)
	ON_COMMAND(ID_DISEMBLY_EXPORT_EXCEL, &CDisassemblerDlg::OnDisemblyExportExcel)
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI(ID_DISEMBLY_LOWERCASE, &CDisassemblerDlg::OnUpdateDisemblyLowercase)
	ON_NOTIFY(NM_RCLICK, IDC_PROCESS_LIST, &CDisassemblerDlg::OnNMRclickProcessList)
	ON_COMMAND(ID_PROCESS_REFRESH, &CDisassemblerDlg::OnProcessRefresh)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROCESS_LIST, &CDisassemblerDlg::OnLvnItemchangedProcessList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDisassemblerDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
	EASYSIZE(IDC_PROCESS_LIST, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
END_EASYSIZE_MAP
// CDisassemblerDlg 消息处理程序


void CDisassemblerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CDisassemblerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szAddress[g_enumLang], LVCFMT_LEFT, 90);
	m_list.InsertColumn(1, szHex[g_enumLang], LVCFMT_LEFT, 170);
	m_list.InsertColumn(2, szDisassembly[g_enumLang], LVCFMT_LEFT, 230);

	m_ProImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 2, 2); 
	m_ProcessList.SetImageList (&m_ProImageList, LVSIL_SMALL);
	m_ProcessList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ProcessList.InsertColumn(0, szImageName[g_enumLang], LVCFMT_LEFT, 130);
	m_ProcessList.InsertColumn(1, szPid[g_enumLang], LVCFMT_LEFT, 70);

	m_szStaticAddress = szDumpAddress[g_enumLang];
	m_szStaticSize = szDisassmSize[g_enumLang];
	m_szStaticPid = szHexEditerPid[g_enumLang];
	GetDlgItem(IDOK)->SetWindowText(szDisasm[g_enumLang]);
	GetDlgItem(IDC_STATIC_PROCESS_LIST)->SetWindowText(szProcessList[g_enumLang]);
	GetDlgItem(IDC_STATIC_DISASM_CODE)->SetWindowText(szDisasmCode[g_enumLang]);

	if (m_dwSize == 0)
	{
		m_dwSize = 0x300;
	}

	m_szAddress.Format(L"%08X", m_dwAddress);
	m_szSize.Format(L"%X", m_dwSize);
	m_szPid.Format(L"%d", m_dwPid);
	m_btnKernelMode.SetWindowText(szKernelAddress[g_enumLang]);

	UpdateData(FALSE);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDisassemblerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	return CDialog::PreTranslateMessage(pMsg);
}

void CDisassemblerDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	
	m_dwSize = 0;
	if (!m_szSize.IsEmpty())
	{
		swscanf_s(m_szSize.GetBuffer(), L"%x", &m_dwSize);
		m_szSize.ReleaseBuffer();
	}

	m_dwAddress = 0;
	if (!m_szAddress.IsEmpty())
	{
		swscanf_s(m_szAddress.GetBuffer(), L"%x", &m_dwAddress);
		m_szAddress.ReleaseBuffer();
	}

	m_dwPid = 0;
	if (!m_szPid.IsEmpty())
	{
		swscanf_s(m_szPid.GetBuffer(), L"%d", &m_dwPid);
		m_szPid.ReleaseBuffer();
	}

	if (m_dwSize <= 0 || m_dwAddress <= 0)
	{
		MessageBox(szHexEditerInputHaveWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
		return;
	}

	if (m_bKernelMode)
	{
		if (m_dwAddress < 0x80000000)
		{
			MessageBox(szHexEditerKernelModeAddressWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
			return;
		}
	}
	else
	{
		if (m_dwAddress >= 0x80000000)
		{
			MessageBox(szHexEditerUserModeAddressWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
			return;
		}

		if (m_dwPid <= 0)
		{
			MessageBox(szHexEditerUserModePidWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
			return;
		}
	}

	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid);
}

BOOL CDisassemblerDlg::DumpMemoryToRing3(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess)
{
	BOOL bRet = FALSE;
	if (Size <= 0)
	{
		return FALSE;
	}

	if (m_bKernelMode)
	{
		if (Address < 0x80000000)
		{
			return FALSE;
		}
	}

	PVOID pBuffer = malloc(Size);
	if (pBuffer)
	{
		memset(pBuffer, 0, Size);

		COMMUNICATE_DISASSEMBLY cd;
		cd.OpType = enumDisassembly;
		cd.nPid = Pid;
		cd.pEprocess = pEprocess;
		cd.Base = Address;
		cd.Size = Size;

		bRet = m_Driver.CommunicateDriver(&cd, sizeof(COMMUNICATE_DISASSEMBLY), pBuffer, Size, NULL);
		if (bRet)
		{
			m_list.DeleteAllItems();
			Disassembly((PBYTE)pBuffer, Address, Size);
		}
	}

	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	return bRet;
}

void CDisassemblerDlg::Disassembly(PBYTE pBuffer, ULONG Address, ULONG nSize)
{
	if (!pBuffer || !nSize)
	{
		return;
	}

	if (m_bKernelMode)
	{
		if (Address < 0x80000000)
		{
			return;
		}
	}

	int nLen = 0;

	ideal=0; putdefseg=0;
	while (nLen <= (int)nSize)
	{
		t_disasm da;

		memset(&da, 0, sizeof(t_disasm));
		ulong nLenTemp = ODDisasm((char*)(pBuffer+nLen),
			nSize - nLen,
			Address + nLen,
			&da,
			DISASM_CODE);

		if (nLenTemp == 0) 
		{
			break;
		}

		CString szAddress, szDisass, szHex;

		szAddress.Format(L"0x%08X", da.ip);
		szHex = m_Functions.ms2ws(da.dump);
		szDisass = m_Functions.ms2ws(da.result);
		int nItem = m_list.InsertItem(m_list.GetItemCount(), szAddress);
		m_list.SetItemText(nItem, 1, szHex);
		m_list.SetItemText(nItem, 2, szDisass);

		if (da.jmpconst)
		{
			m_list.SetItemData(nItem, da.jmpconst);
		}
		else if (da.adrconst)
		{
			m_list.SetItemData(nItem, da.adrconst);
		}
		else if (da.immconst)
		{
			m_list.SetItemData(nItem, da.immconst);
		}

		nLen += nLenTemp;
	}
}

BOOL CDisassemblerDlg::IsImediate()
{
	BOOL bRet = FALSE;
	int nItem = m_Functions.GetSelectItem((CSortListCtrl*)&m_list);

	if (nItem != -1)
	{
		if (m_list.GetItemData(nItem))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

void CDisassemblerDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_DISEMBLY_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_DISEMBLY_EXPORT_EXCEL, szExcel[g_enumLang]);

	CMenu copy;
	copy.CreatePopupMenu();
	copy.AppendMenu(MF_STRING, ID_DISEMBLY_COPY_FULLROW, szCopyFullLine[g_enumLang]);
	copy.AppendMenu(MF_STRING, ID_DISEMBLY_COPY_ADDRESS, szCopyAddress[g_enumLang]);
	copy.AppendMenu(MF_STRING, ID_DISEMBLY_COPY_HEX, szCopyHex[g_enumLang]);
	copy.AppendMenu(MF_STRING, ID_DISEMBLY_COPY_CODE, szCopyCode[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_DISEMBLY_REFLASH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DISEMBLY_IMEDIATE, szFollowImediate[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_DISEMBLY_LOWERCASE, szLowerCase[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)copy.m_hMenu, szCopy[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_DISEMBLY_REFLASH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_DISEMBLY_IMEDIATE, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(6, MF_BYPOSITION, &m_bmCopy, &m_bmCopy);
		menu.SetMenuItemBitmaps(8, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 9; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 8; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			menu.EnableMenuItem(ID_DISEMBLY_IMEDIATE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			if (!IsImediate())
			{
				menu.EnableMenuItem(ID_DISEMBLY_IMEDIATE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			}
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();
	copy.DestroyMenu();

	*pResult = 0;
}

void CDisassemblerDlg::OnBnClickedCheckKernelAddress()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_PID)->EnableWindow(!m_bKernelMode);
	m_ProcessList.EnableWindow(!m_bKernelMode);
}

void CDisassemblerDlg::OnDisemblyReflash()
{
	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid);
}

void CDisassemblerDlg::OnDisemblyImediate()
{
	int nItem = m_Functions.GetSelectItem((CSortListCtrl*)&m_list);
	if (nItem != -1)
	{
		m_dwAddress = (DWORD)m_list.GetItemData(nItem);
		if (m_dwAddress > 0)
		{
			if (m_bKernelMode)
			{
				if (m_dwAddress < 0x80000000)
				{
					return;
				}
			}

			m_szAddress.Format(L"%08X", m_dwAddress);
			UpdateData(FALSE);
			OnDisemblyReflash();
		}
	}
}

void CDisassemblerDlg::OnDisemblyCopyFullrow()
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

void CDisassemblerDlg::OnDisemblyCopyAddress()
{
	CString szString;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		szString += m_list.GetItemText(iIndex, 0);
		szString += L"\r\n";
	}

	m_Functions.SetStringToClipboard(szString);
}

void CDisassemblerDlg::OnDisemblyCopyHex()
{
	CString szString;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		szString += m_list.GetItemText(iIndex, 1);
		szString += L"\r\n";
	}

	m_Functions.SetStringToClipboard(szString);
}

void CDisassemblerDlg::OnDisemblyCopyCode()
{
	CString szString;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		szString += m_list.GetItemText(iIndex, 2);
		szString += L"\r\n";
	}

	m_Functions.SetStringToClipboard(szString);
}

void CDisassemblerDlg::OnDisemblyLowercase()
{
	lowercase = !lowercase;
	OnDisemblyReflash();
}

void CDisassemblerDlg::OnDisemblyExportText()
{
	CString szStatus;
	szStatus.Format(L"Pid: %d, Address: 0x%08X, Size: %d", m_dwPid, m_dwAddress, m_dwSize);
	m_Functions.ExportListToTxt((CSortListCtrl*)&m_list, szStatus);
}

void CDisassemblerDlg::OnDisemblyExportExcel()
{
	CString szStatus;
	szStatus.Format(L"Pid: %d, Address: 0x%08X, Size: %d", m_dwPid, m_dwAddress, m_dwSize);
	WCHAR szDisasm[] = {'D','i','s','a','s','m','\0'};
	m_Functions.ExportListToExcel((CSortListCtrl*)&m_list, szDisasm, szStatus);
}

void CDisassemblerDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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
		for(UINT i=0; i<count; i++)  
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

void CDisassemblerDlg::OnUpdateDisemblyLowercase(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(lowercase);
}

void CDisassemblerDlg::OnNMRclickProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PROCESS_REFRESH, szRefresh[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_PROCESS_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	*pResult = 0;
}

void CDisassemblerDlg::ListProcess()
{
	m_ProcessList.DeleteAllItems();
	int nImageCount = m_ProImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ProImageList.Remove(0);   
	}

	vector<PROCESS_INFO> vectorProcess;
	CListProcess ListProc;
	ListProc.EnumProcess(vectorProcess);

	for ( vector <PROCESS_INFO>::iterator Iter = vectorProcess.begin( ); 
		Iter != vectorProcess.end( ); 
		Iter++ )
	{
		PROCESS_INFO ProcessItem = *Iter;

		if (ProcessItem.ulPid == 0 && ProcessItem.ulParentPid == 0 && ProcessItem.ulEprocess == 0)
		{
			continue;
		}

		CString strPath = m_Functions.TrimPath(ProcessItem.szPath);

		// 处理进程图标
		SHFILEINFO sfi;
		DWORD_PTR nRet;
		ZeroMemory(&sfi, sizeof(SHFILEINFO));
		nRet = SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		nRet ? m_ProImageList.Add ( sfi.hIcon ) : m_ProImageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
		if (nRet)
		{
			DestroyIcon(sfi.hIcon);
		}
	
		CString szPid;
		szPid.Format(L"%d", ProcessItem.ulPid);
		
		CString szImage = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

		int nItem = m_ProcessList.GetItemCount();
		m_ProcessList.InsertItem(nItem, szImage, nItem);
		m_ProcessList.SetItemText(nItem, 1, szPid);
	}
}

void CDisassemblerDlg::OnProcessRefresh()
{
	ListProcess();
}

void CDisassemblerDlg::OnLvnItemchangedProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int nItem = pNMLV->iItem;
	m_szPid = m_ProcessList.GetItemText(nItem, 1);
	UpdateData(FALSE);
	*pResult = 0;
}
