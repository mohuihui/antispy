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
// CommandDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "CommandDlg.h"

#define WM_UPDATE_DATA				WM_USER + 5

// CCommandDlg 对话框

IMPLEMENT_DYNAMIC(CCommandDlg, CDialog)

CCommandDlg::CCommandDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommandDlg::IDD, pParent)
	, m_szOutput(_T(""))
	, m_szCommand(_T(""))
{
	m_bCreateThread = 0;
	m_CommandList.clear();
	m_dwCurrentCommand = 0;
	m_bFirstUp = FALSE;
	m_dwTotalCommand = 0;
}

CCommandDlg::~CCommandDlg()
{
	m_CommandList.clear();
}

void CCommandDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_OUTPUT_EDIT, m_szOutput);
	DDX_Text(pDX, IDC_COMMAND_EDIT, m_szCommand);
	DDX_Control(pDX, IDC_OUTPUT_EDIT, m_OutputEdit);
}


BEGIN_MESSAGE_MAP(CCommandDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CCommandDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_MESSAGE(WM_UPDATE_DATA, OnUpdateData)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CCommandDlg)
//	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_OUTPUT_EDIT, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_COMMAND_EDIT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC_COMMAND, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, 0)
END_EASYSIZE_MAP
// CCommandDlg 消息处理程序

void CCommandDlg::OnBnClickedOk()
{
}

void CCommandDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CCommandDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_szOutput += L"Help information:\r\n";
	ShowHelpInfomation();
	UpdateData(FALSE);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

DWORD WINAPI CommandThreadProc(LPVOID lpParameter)
{
	CCommandDlg* pWnd = (CCommandDlg*)lpParameter;
	if (pWnd)
	{
		pWnd->ParseCommand();
	}

	return 0;
}

void CCommandDlg::UpdateOutputEdit()
{
// 	this->LockWindowUpdate();
// 	m_OutputEdit.LockWindowUpdate();
// 	m_OutputEdit.SetRedraw(FALSE);
// 	this->SetRedraw(FALSE);
	UpdateData(FALSE);
/*	m_OutputEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);*/
 	m_OutputEdit.LineScroll (m_OutputEdit.GetLineCount(), 0);
// 	m_OutputEdit .UnlockWindowUpdate();
// 	m_OutputEdit.SetRedraw(TRUE);
// 	this->UnlockWindowUpdate();
// 	this->SetRedraw(TRUE);
}

BOOL CCommandDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->hwnd == (GetDlgItem(IDC_COMMAND_EDIT))->GetSafeHwnd() && pMsg->message == WM_KEYDOWN)
	{		
		char c = (char)pMsg->wParam; 
		if (c == VK_RETURN)
		{
			if (0 == InterlockedCompareExchange(&m_bCreateThread, 1, 0))
			{
				UpdateData(TRUE);
				m_dwTotalCommand++;
				m_bFirstUp = TRUE;
				m_CommandList.push_back(m_szCommand);
				
				DWORD dwTid = 0;
				HANDLE hTread = CreateThread(NULL, 0, CommandThreadProc, this, 0, &dwTid);
				if (hTread != NULL)
				{
					CloseHandle(hTread);
				}
				else
				{
					InterlockedCompareExchange(&m_bCreateThread, 0, 1);
				}
			}
		}
		else if (c == VK_UP)
		{
			if (m_bFirstUp)
			{
				m_bFirstUp = FALSE;
				m_dwCurrentCommand = m_dwTotalCommand;
			}
			
			if (m_dwCurrentCommand)
			{
				m_dwCurrentCommand--;
				m_szCommand = m_CommandList.at(m_dwCurrentCommand);
				UpdateOutputEdit();
			}
		}
		else if (c == VK_DOWN)
		{
			if (m_bFirstUp && m_dwTotalCommand)
			{
				m_szCommand = m_CommandList.at(m_dwTotalCommand-1);
				UpdateOutputEdit();
			}
			else if (!m_bFirstUp && m_dwTotalCommand)
			{
				if (m_dwCurrentCommand < m_dwTotalCommand-1)
				{
					m_szCommand = m_CommandList.at(++m_dwCurrentCommand);
				}
				if (m_dwCurrentCommand == m_dwTotalCommand-1)
				{
					m_szCommand = m_CommandList.at(m_dwCurrentCommand);
				}
				else if (m_dwCurrentCommand == m_dwTotalCommand)
				{
					m_szCommand = m_CommandList.at(m_dwCurrentCommand-1);
				}
				else if (m_dwCurrentCommand > m_dwTotalCommand)
				{
					m_dwCurrentCommand = m_dwTotalCommand;
				}

				UpdateOutputEdit();
			}
		}
	}

	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CCommandDlg::ParseCommand()
{
	TrimSpace(m_szCommand);

	WCHAR szProcess[] = {'P','r','o','c','e','s','s','\0'};
	WCHAR szHelp[] = {'H','e','l','p','\0'};
	WCHAR szCls[] = {'c','l','s','\0'};
	WCHAR* szCommand = m_szCommand.GetBuffer();
	ULONG nCmdLen = m_szCommand.GetLength();
	
	m_szOutput.TrimRight('\n');
	m_szOutput.TrimRight('\r');
	if (!m_szOutput.IsEmpty())
	{
 		m_szOutput += L"\r\n";
		m_szOutput += L"--------------------------------------------------------------------------------\r\n";
	}

	m_szOutput += L"Command>  ";
	m_szOutput += szCommand;
	m_szOutput += L"\r\n";

	if (nCmdLen >= wcslen(szHelp) && !_wcsnicmp(szCommand, szHelp, wcslen(szHelp)))
	{
		ShowHelpInfomation();
	}
	else if (nCmdLen >= wcslen(szProcess) && !_wcsnicmp(szCommand, szProcess, wcslen(szProcess)))
	{
	//	MessageBox(L"process");
	}
	else if (nCmdLen >= wcslen(szCls) && !_wcsnicmp(szCommand, szCls, wcslen(szCls)))
	{
		m_szOutput.Empty();
	}
	else if (nCmdLen >= wcslen(L"u") && !_wcsnicmp(szCommand, L"u", wcslen(L"u")))
	{
		Unassemble();
	}
	else if (nCmdLen >= wcslen(L"d") && !_wcsnicmp(szCommand, L"d", wcslen(L"d")))
	{
		DisplayMemory();
	}

	SendMessage(/*this->m_hWnd, */WM_UPDATE_DATA, NULL, NULL);
	InterlockedCompareExchange(&m_bCreateThread, 0, 1);
}

void CCommandDlg::ShowHelpInfomation()
{
	m_szOutput += L"    Command: help\r\n";
	m_szOutput += L"       - Show some help information.\r\n";
	m_szOutput += L"    Syntax:\r\n"; 
	m_szOutput += L"        help\r\n";
// 	m_szOutput += L"    Parameters:\r\n"; 
// 	m_szOutput += L"        None.\r\n";

	m_szOutput += L"\r\n";

	m_szOutput += L"    Command: cls (Clear Screen)\r\n";
	m_szOutput += L"       - Clear the command window display.\r\n";
	m_szOutput += L"    Syntax:\r\n"; 
	m_szOutput += L"        cls\r\n";
// 	m_szOutput += L"    Parameters:\r\n"; 
// 	m_szOutput += L"        None.\r\n";

// 	m_szOutput += L"\r\n";
// 
// 	m_szOutput += L"    Command: process\r\n";
// 	m_szOutput += L"       - Displays information about all processes.\r\n";
// 	m_szOutput += L"    Syntax:\r\n"; 
// 	m_szOutput += L"        process\r\n";
// 	m_szOutput += L"    Parameters:\r\n"; 
// 	m_szOutput += L"        None.\r\n";

	m_szOutput += L"\r\n";

	m_szOutput += L"    Command: u (Unassemble)\r\n";
	m_szOutput += L"       - Display an assembly translation of the specified program code in memory.\r\n";
	m_szOutput += L"    Syntax:\r\n"; 
	m_szOutput += L"        u [address] [size] \r\n";
	m_szOutput += L"    Parameters:\r\n"; 
	m_szOutput += L"        address (Hex, >=0x80000000) - start address of the memory region.\r\n";
	m_szOutput += L"        size (Hex, Bytes) - size of the memory region.\r\n";

	m_szOutput += L"\r\n";

	m_szOutput += L"    Command: d (Display Memory)\r\n";
	m_szOutput += L"       - Display the contents of memory in the given range.\r\n";
	m_szOutput += L"    Syntax:\r\n"; 
	m_szOutput += L"        d{b,w,d} [address] [size] \r\n";
	m_szOutput += L"    Parameters:\r\n"; 
	m_szOutput += L"        db - Each displayed line consists of address followed up to 16 byte values of its contents.\r\n";
	m_szOutput += L"        dw - Each displayed line consists of address followed up to 8 word values of its contents.\r\n";
	m_szOutput += L"        dd - Each displayed line consists of address followed up to 4 double-word values of its contents.\r\n";
	m_szOutput += L"        address (Hex, >=0x80000000) - start address of the memory region.\r\n";
	m_szOutput += L"        size (Hex, Bytes) - size of the dump.\r\n";
}	

LRESULT CCommandDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	m_szCommand.Empty();
	UpdateOutputEdit();
	return 0;
}

void CCommandDlg::Unassemble()
{
	int nSpace = m_szCommand.Find(' ');
	if (nSpace == -1)
	{
		m_szOutput += L"    There is something wrong with the input.\r\n";
		return;
	}

	CString szAddress = m_szCommand.Right(m_szCommand.GetLength() - m_szCommand.Find(' ') - 1);
	TrimSpace(szAddress);
	szAddress.TrimLeft('0');
	szAddress.TrimLeft('x');
	szAddress.TrimLeft('X');

	if (szAddress.IsEmpty())
	{
		m_szOutput += L"    There is something wrong with the input.\r\n";
		return;
 	}
	
	DWORD dwAddress = 0, dwSize = 0;
	CString szSize;
	nSpace = szAddress.Find(' ');
	if (nSpace != -1)
	{
		szSize = szAddress.Right(szAddress.GetLength() - szAddress.Find(' ') - 1);
		TrimSpace(szSize);
		szSize.TrimLeft('0');
		szSize.TrimLeft('x');
		szSize.TrimLeft('X');
	}
	
	if (!szAddress.IsEmpty())
	{
		swscanf_s(szAddress.GetBuffer(), L"%x", &dwAddress);
		szAddress.ReleaseBuffer();
	}

	if (!szSize.IsEmpty())
	{
		swscanf_s(szSize.GetBuffer(), L"%x", &dwSize);
		szSize.ReleaseBuffer();
	}

	if (dwAddress > 0x80000000)
	{
		dwSize = dwSize > 0 ? dwSize : 100;
		CString szCode = m_Function.DisassembleString(dwAddress, dwSize);
		if (szCode.IsEmpty())
		{
			CString szFormatAddress;
			szFormatAddress.Format(L"%08X", dwAddress);
			m_szOutput += L"    Memory access error in ";
			m_szOutput += szFormatAddress;
			m_szOutput += L".\r\n";
		}
		else
		{
			m_szOutput += szCode;
		}
	}
	else
	{
		m_szOutput += L"    There is something wrong with the input.\r\n";
	}
}

PBYTE CCommandDlg::DumpMemory(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess)
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
			return (PBYTE)pBuffer;
		}
	}

	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	return NULL;
}

void CCommandDlg::TrimSpace(CString &szString)
{
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');
	szString.TrimLeft(' ');

	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
	szString.TrimRight(' ');
}

void CCommandDlg::DisplayMemory()
{
	int nSpace = m_szCommand.Find(' ');
	if (nSpace == -1)
	{
		m_szOutput += L"    There is something wrong with the input.\r\n";
		return;
	}

	CString szAddress = m_szCommand.Right(m_szCommand.GetLength() - m_szCommand.Find(' ') - 1);
	TrimSpace(szAddress);
	szAddress.TrimLeft('0');
	szAddress.TrimLeft('x');
	szAddress.TrimLeft('X');

	if (szAddress.IsEmpty())
	{
		m_szOutput += L"    There is something wrong with the input.\r\n";
		return;
	}

	DWORD dwAddress = 0, dwSize = 0;
	CString szSize;
	nSpace = szAddress.Find(' ');
	if (nSpace != -1)
	{
		szSize = szAddress.Right(szAddress.GetLength() - szAddress.Find(' ') - 1);
		TrimSpace(szSize);
		szSize.TrimLeft('0');
		szSize.TrimLeft('x');
		szSize.TrimLeft('X');
	}

	if (!szAddress.IsEmpty())
	{
		swscanf_s(szAddress.GetBuffer(), L"%x", &dwAddress);
		szAddress.ReleaseBuffer();
	}

	if (!szSize.IsEmpty())
	{
		swscanf_s(szSize.GetBuffer(), L"%x", &dwSize);
		szSize.ReleaseBuffer();
	}

	if (dwAddress > 0x80000000)
	{
		dwSize = dwSize > 0 ? dwSize : 100;

		WCHAR *szCommand = m_szCommand.GetBuffer();
		if (!_wcsnicmp(szCommand, L"dw", 2))
		{
			int xx = dwSize % sizeof(WORD);
			dwSize = xx==0 ? dwSize : dwSize + (sizeof(WORD)-xx);
		}
		else if (!_wcsnicmp(szCommand, L"dd", 2))
		{
			int xx = dwSize % sizeof(DWORD);
			dwSize = xx==0 ? dwSize : dwSize + (sizeof(DWORD)-xx);
		}
		m_szCommand.ReleaseBuffer();	

		PBYTE pBuffer = DumpMemory(dwAddress, dwSize);
		if (pBuffer)
		{
			WCHAR *szCommand = m_szCommand.GetBuffer();
			if (!_wcsnicmp(szCommand, L"db", 2))
			{
				ShowDB(pBuffer, dwAddress, dwSize);
			}
			else if (!_wcsnicmp(szCommand, L"dw", 2))
			{
				ShowDW(pBuffer, dwAddress, dwSize);
			}
			else if (!_wcsnicmp(szCommand, L"dd", 2))
			{
				ShowDD(pBuffer, dwAddress, dwSize);
			}
			m_szCommand.ReleaseBuffer();	
			free(pBuffer);
			pBuffer = NULL;
		}
	}
}

void CCommandDlg::ShowDD(PBYTE pBuffer, DWORD dwAddress, DWORD dwSize)
{
	dwSize = dwSize - (dwSize % sizeof(DWORD));
	if (pBuffer && dwSize && dwAddress)
	{
		int nCnt = dwSize / sizeof(DWORD);
		for (int n = 0; n < nCnt; n++)
		{
			if ((n % 4) == 0)
			{
				CString szAddress;
				szAddress.Format(L"    %08X", dwAddress + (n / 4) * 16);
				m_szOutput += szAddress;
				m_szOutput += L"    ";
			}
			
			CString szDD;
			szDD.Format(L"    %08X", *(DWORD *)(pBuffer + n * 4));
			m_szOutput += szDD;

			if (((n+1)%4)==0)
			{
				m_szOutput += L"\r\n";
			}
		}
	}
}

void CCommandDlg::ShowDB(PBYTE pBuffer, DWORD dwAddress, DWORD dwSize)
{
	if (pBuffer && dwSize && dwAddress)
	{
		for (int n = 0; n < (int)dwSize; n++)
		{
			if ((n % 16) == 0)
			{
				CString szAddress;
				szAddress.Format(L"    %08X", dwAddress + n);
				m_szOutput += szAddress;
				m_szOutput += L"    ";
			}

			CString szDD;
			szDD.Format(L"    %02X", *(BYTE *)(pBuffer + n * sizeof(BYTE)));
			m_szOutput += szDD;

			if (((n+1)%16)==0)
			{
				m_szOutput += L"\r\n";
			}
		}
	}
}

void CCommandDlg::ShowDW(PBYTE pBuffer, DWORD dwAddress, DWORD dwSize)
{
	dwSize = dwSize - (dwSize % sizeof(WORD));
	if (pBuffer && dwSize && dwAddress)
	{
		int nCnt = dwSize / sizeof(WORD);
		for (int n = 0; n < nCnt; n++)
		{
			if ((n%8) == 0)
			{
				CString szAddress;
				szAddress.Format(L"    %08X", dwAddress + (n / 8) * 16);
				m_szOutput += szAddress;
				m_szOutput += L"    ";
			}

			CString szDD;
			szDD.Format(L"    %04X", *(WORD *)(pBuffer + n * sizeof(WORD)));
			m_szOutput += szDD;

			if (((n+1)%8)==0)
			{
				m_szOutput += L"\r\n";
			}
		}
	}
}