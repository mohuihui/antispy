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
// AssemblyDlg.cpp : 实现文件
//

#include "stdafx.h"

#include "SpyHunter.h"
#include "AssemblyDlg.h"

#define _CRT_SECURE_NO_WARNINGS
#define MAINPROG                       // Place all unique variables here

#ifdef __cplusplus
extern "C"
{
#endif
#include "ODEngine/disasm.h"
#ifdef __cplusplus
}
#endif

/*#include "libdasm/libdasm.h"*/
// #define BEA_ENGINE_STATIC
// #define BEA_USE_STDCALL
// #include "BeaEngine/BeaEngine.h"
// #pragma comment(lib, "BeaEngine.lib")

// CAssemblyDlg 对话框

IMPLEMENT_DYNAMIC(CAssemblyDlg, CDialog)

CAssemblyDlg::CAssemblyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAssemblyDlg::IDD, pParent)
	, m_szAddress(_T(""))
	, m_szSize(_T(""))
	, m_szPid(_T(""))
{	
	m_dwPid = 0;
	m_dwEprocess = 0;
	m_dwAddress = 0;
	m_dwSize = 0;
	lowercase = 0; // 非小写字体 
}

CAssemblyDlg::~CAssemblyDlg()
{
}

void CAssemblyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ADDRESS, m_szAddress);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_szSize);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_EDIT_PID, m_szPid);
}


BEGIN_MESSAGE_MAP(CAssemblyDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAssemblyDlg::OnBnClickedOk)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CAssemblyDlg::OnNMRclickList)
	ON_COMMAND(ID_DISEMBLY_REFLASH, &CAssemblyDlg::OnDisemblyReflash)
	ON_COMMAND(ID_DISEMBLY_IMEDIATE, &CAssemblyDlg::OnDisemblyImediate)
	ON_COMMAND(ID_DISEMBLY_COPY_FULLROW, &CAssemblyDlg::OnDisemblyCopyFullrow)
	ON_COMMAND(ID_DISEMBLY_COPY_ADDRESS, &CAssemblyDlg::OnDisemblyCopyAddress)
	ON_COMMAND(ID_DISEMBLY_COPY_HEX, &CAssemblyDlg::OnDisemblyCopyHex)
	ON_COMMAND(ID_DISEMBLY_COPY_CODE, &CAssemblyDlg::OnDisemblyCopyCode)
	ON_COMMAND(ID_DISEMBLY_EXPORT_TEXT, &CAssemblyDlg::OnDisemblyExportText)
	ON_COMMAND(ID_DISEMBLY_EXPORT_EXCEL, &CAssemblyDlg::OnDisemblyExportExcel)
	ON_COMMAND(ID_DISEMBLY_LOWERCASE, &CAssemblyDlg::OnDisemblyLowercase)
	ON_UPDATE_COMMAND_UI(ID_DISEMBLY_LOWERCASE, &CAssemblyDlg::OnUpdateDisemblyLowercase)
	ON_WM_INITMENUPOPUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CAssemblyDlg)
//	EASYSIZE(IDC_STATIC_PID, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_EDIT_PID, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_STATIC_ADDRESS, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_EDIT_ADDRESS, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_STATIC_SIZE, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_EDIT_SIZE, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDOK, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
//	EASYSIZE(IDC_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
//	EASYSIZE(IDC_STATIC1, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_STATIC2, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_STATIC3, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
END_EASYSIZE_MAP
// CAssemblyDlg 消息处理程序

void CAssemblyDlg::OnBnClickedOk()
{
	m_list.DeleteAllItems();
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

	m_dwEprocess = 0;
	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid, m_dwEprocess);
}

BOOL CAssemblyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szAddress[g_enumLang], LVCFMT_LEFT, 110);
	m_list.InsertColumn(1, szHex[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(2, szDisassembly[g_enumLang], LVCFMT_LEFT, 300);

	GetDlgItem(IDC_STATIC_ADDRESS)->SetWindowText(szDumpAddress[g_enumLang]);
	GetDlgItem(IDC_STATIC_SIZE)->SetWindowText(szDisassmSize[g_enumLang]);
	GetDlgItem(IDC_STATIC_PID)->SetWindowText(szDisassmPID[g_enumLang]);
	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);

	SetWindowText(szDisassembler[g_enumLang]);

	if (m_dwSize == 0)
	{
		m_dwSize = 100;
	}
	
	m_szAddress.Format(L"%08X", m_dwAddress);
	m_szSize.Format(L"%X", m_dwSize);
	m_szPid.Format(L"%d", m_dwPid);
	
	UpdateData(FALSE);
	
	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid, m_dwEprocess);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CAssemblyDlg::DumpMemoryToRing3(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess)
{
	BOOL bRet = FALSE;

	if (Size > 0)
	{
		if (Address >= 0x80000000)
		{
			bRet = TRUE;
		}
		else if (Address < 0x80000000 && (Pid || pEprocess))
		{
			bRet = TRUE;
		}
	}

	PVOID pBuffer = malloc(Size);
	if (bRet && pBuffer)
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
		//	MessageBox(L"Disassembly ok");
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

//多字节字符串转UNICODE字符串
inline CString ms2ws(LPCSTR szSrc, int cbMultiChar=-1)
{
	CString strDst;
	if (szSrc && strlen(szSrc) > 0)
	{
		int nLen = MultiByteToWideChar(CP_ACP, 0, szSrc, cbMultiChar, NULL, 0);
		if ( nLen > 0 )
		{
			WCHAR *pBuff = (WCHAR *)malloc((nLen+1)*sizeof(WCHAR));
			if ( pBuff )
			{
				MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,pBuff,nLen);
				pBuff[nLen]=0;
				strDst = pBuff;
				free(pBuff);
			}
		}
	}

	return strDst;
}

void CAssemblyDlg::Disassembly(PBYTE pBuffer, ULONG Address, ULONG nSize)
{
	if (!pBuffer || !nSize || !Address)
	{
		return;
	}

// 	int nLen = 0;
// 	DISASM MyDisasm;
// 	memset (&MyDisasm, 0, sizeof(DISASM));
// 	MyDisasm.EIP = (UIntPtr)pBuffer;
// 	MyDisasm.VirtualAddr = (ULONG)Address;
// 
// 	while (nLen <= (int)nSize)
// 	{
// 		int nLenTemp = Disasm(&MyDisasm);
// 		if (nLenTemp == UNKNOWN_OPCODE) 
// 		{
// 			break;
// 		}
// 		
// 		if (strlen(MyDisasm.CompleteInstr) <= 0)
// 		{
// 			break;
// 		}
// 
// 		CString szAddress, szDisass, szHex;
// 		szAddress.Format(L"0x%08X", MyDisasm.VirtualAddr);
// 		szDisass = ms2ws(MyDisasm.CompleteInstr);
// 		int nItem = m_list.InsertItem(m_list.GetItemCount(), szAddress);
// 		m_list.SetItemText(nItem, 2, szDisass);
// 
// 		nLen += nLenTemp;
// 		MyDisasm.EIP = MyDisasm.EIP + nLenTemp;
// 		MyDisasm.VirtualAddr = (ULONGLONG)Address + nLen;
// 	}

// 	int nLen = 0;
// 	int iLength = 0;
// 	char szAsm[256] = {0};
// 	INSTRUCTION inst;
// 	while (nLen <= (int)nSize)
// 	{
// 		memset(&inst, 0, sizeof(INSTRUCTION));
// 		iLength = get_instruction(&inst, (BYTE *)pBuffer + nLen, MODE_32);
// 		get_instruction_string(&inst, FORMAT_INTEL, Address + nLen, szAsm, 64);
// 
// 		CString szAddress, szDisass, szHex;
// 		szAddress.Format(L"0x%08X", Address + nLen);
// 		szDisass = ms2ws(szAsm);
// 		int nItem = m_list.InsertItem(m_list.GetItemCount(), szAddress);
// 		m_list.SetItemText(nItem, 2, szDisass);
// 
// 		nLen += iLength;
// 	//	printf("%s\n", szAsm);
// 	}

	int nLen = 0;
// 	DISASM MyDisasm;
// 	memset (&MyDisasm, 0, sizeof(DISASM));
// 	MyDisasm.EIP = (UIntPtr)pBuffer;
// 	MyDisasm.VirtualAddr = (ULONG)Address;

// 	printf("%3i  %-24s  %-24s   (MASM)\n",l,da.dump,da.result);

	ideal=0; putdefseg=0;
	while (nLen <= (int)nSize)
	{
		t_disasm da;
		memset(&da, 0, sizeof(t_disasm));
		ulong nLenTemp = //Disasm(&MyDisasm);

		ODDisasm((char*)(pBuffer+nLen),
			nSize - nLen,
			Address + nLen,
			&da,
			DISASM_CODE);

		if (nLenTemp == 0) 
		{
			break;
		}
		
// 		if (strlen(MyDisasm.CompleteInstr) <= 0)
// 		{
// 			break;
// 		}

		CString szAddress, szDisass, szHex;
// 		szAddress.Format(L"0x%08X", MyDisasm.VirtualAddr);
		szAddress.Format(L"0x%08X", da.ip);
		szHex = ms2ws(da.dump);
// 		szDisass = ms2ws(MyDisasm.CompleteInstr);
		szDisass = ms2ws(da.result);
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
//  		MyDisasm.EIP = MyDisasm.EIP + nLenTemp;
//  		MyDisasm.VirtualAddr = (ULONGLONG)Address + nLen;
	}
}

BOOL CAssemblyDlg::IsImediate()
{
	BOOL bRet = FALSE;
	int nItem = m_Functions.GetSelectItem((CSortListCtrl*)&m_list);
	
	if (nItem != -1)
	{
// 		CString szCode = m_list.GetItemText(nItem, 2);
// 		if (!szCode.IsEmpty() && szCode.GetLength() >= 12 && szCode.CompareNoCase(L"???")) 
// 		{
// 			if (szCode.Find(L"call") != -1 || szCode.Find(L"CALL") != -1)
// 			{
// 				bRet = TRUE;
// 			}
// 			else 
// 			{
// 				WCHAR* szBuffer = szCode.GetBuffer();
// 
// 				if (*(WCHAR*)szBuffer == 'j' || *(WCHAR*)szBuffer == 'J')
// 				{
// 					bRet = TRUE;
// 				}
// 
// 				szCode.ReleaseBuffer();
// 			}
// 		}

		if (m_list.GetItemData(nItem))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

void CAssemblyDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
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


void CAssemblyDlg::OnDisemblyReflash()
{
	m_list.DeleteAllItems();
	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid, m_dwEprocess);
}

void CAssemblyDlg::OnDisemblyImediate()
{
	int nItem = m_Functions.GetSelectItem((CSortListCtrl*)&m_list);
	if (nItem != -1)
	{
		m_dwAddress = (DWORD)m_list.GetItemData(nItem);
		if (m_dwAddress > 0)
		{
			m_szAddress.Format(L"%08X", m_dwAddress);
			UpdateData(FALSE);
			OnDisemblyReflash();
		}
	}
}

void CAssemblyDlg::OnDisemblyCopyFullrow()
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

void CAssemblyDlg::OnDisemblyCopyAddress()
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

void CAssemblyDlg::OnDisemblyCopyHex()
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

void CAssemblyDlg::OnDisemblyCopyCode()
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

void CAssemblyDlg::OnDisemblyExportText()
{
	CString szStatus;
	szStatus.Format(L"Pid: %d, Address: 0x%08X, Size: %d", m_dwPid, m_dwAddress, m_dwSize);
	m_Functions.ExportListToTxt((CSortListCtrl*)&m_list, szStatus);
}

void CAssemblyDlg::OnDisemblyExportExcel()
{
	CString szStatus;
	szStatus.Format(L"Pid: %d, Address: 0x%08X, Size: %d", m_dwPid, m_dwAddress, m_dwSize);
	WCHAR szDisasm[] = {'D','i','s','a','s','m','\0'};
	m_Functions.ExportListToExcel((CSortListCtrl*)&m_list, szDisasm, szStatus);
}

void CAssemblyDlg::OnDisemblyLowercase()
{
	lowercase = !lowercase;
	OnDisemblyReflash();
}

void CAssemblyDlg::OnUpdateDisemblyLowercase(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(lowercase);
}

void CAssemblyDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
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

CString CAssemblyDlg::DisasmMemory(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess)
{
	BOOL bRet = FALSE;
	CString szCode;

	if (Size > 0)
	{
		if (Address >= 0x80000000)
		{
			bRet = TRUE;
		}
// 		else if (Address < 0x80000000 && (Pid || pEprocess))
// 		{
// 			bRet = TRUE;
// 		}
	}

	PVOID pBuffer = malloc(Size);
	if (bRet && pBuffer)
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
			szCode = DisassemblyString((PBYTE)pBuffer, Address, Size);
		}
	}

	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	return szCode;
}

CString CAssemblyDlg::DisassemblyString(PBYTE pBuffer, ULONG Address, ULONG nSize)
{
	CString szCode;
	if (!pBuffer || !nSize || !Address)
	{
		return NULL;
	}

	int nLen = 0;
	ideal=0; putdefseg=0; lowercase = 1;
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
		szAddress.Format(L"%08X", da.ip);
		szHex = ms2ws(da.dump);
		szDisass = ms2ws(da.result);
		
		szCode += L"    ";
		szCode += szAddress;
		szCode += L"\t";
		szCode += szHex;
		if (szHex.GetLength() >= 16)
		{
			szCode += L"\t";
		}
		else if (szHex.GetLength() >= 8)
		{
			szCode += L"\t\t";
		}
		else
		{
			szCode += L"\t\t\t";
		}
		szCode += szDisass;
		szCode += L"\r\n";

		nLen += nLenTemp;
	}

	return szCode;
}

void CAssemblyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}
