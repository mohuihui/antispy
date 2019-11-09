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
// MemoryHexEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "MemoryHexEditDlg.h"


// CMemoryHexEditDlg dialog

IMPLEMENT_DYNAMIC(CMemoryHexEditDlg, CDialog)

CMemoryHexEditDlg::CMemoryHexEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMemoryHexEditDlg::IDD, pParent)
	, m_szAddress(_T(""))
	, m_szSize(_T(""))
	, m_bReadOnly(TRUE)
{
	m_dwPid = 0;
	m_dwEprocess = 0;
	m_dwAddress = 0;
	m_dwSize = 0;
}

CMemoryHexEditDlg::~CMemoryHexEditDlg()
{
}

void CMemoryHexEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ADDRESS, m_szAddress);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_szSize);
	DDX_Control(pDX, IDC_HEX_EDIT, m_editHex);
	DDX_Check(pDX, IDC_CHECK_READ_ONLY, m_bReadOnly);
	DDX_Control(pDX, IDC_CHECK_READ_ONLY, m_btnReadOnly);
}


BEGIN_MESSAGE_MAP(CMemoryHexEditDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMemoryHexEditDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_HEX_EDIT, &CMemoryHexEditDlg::OnEnChangeHexEdit)
	ON_BN_CLICKED(IDC_CHECK_READ_ONLY, &CMemoryHexEditDlg::OnBnClickedCheckReadOnly)
END_MESSAGE_MAP()


// CMemoryHexEditDlg message handlers

void CMemoryHexEditDlg::OnBnClickedOk()
{
	UpdateData();

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

	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid, m_dwEprocess);
}

BOOL CMemoryHexEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_STATIC_ADDRESS)->SetWindowText(szDumpAddress[g_enumLang]);
	GetDlgItem(IDC_STATIC_SIZE)->SetWindowText(szDisassmSize[g_enumLang]);
	GetDlgItem(IDOK)->SetWindowText(szOK[g_enumLang]);

	SetWindowText(szMemoryHexEdit[g_enumLang]);

	if (m_dwSize == 0)
	{
		m_dwSize = 0x1000;
	}

	// 一行显示16个字符
	m_editHex.SetBPR(16);
	// 不允许删除
	m_editHex.SetAllowDeletes(FALSE);
	// 设置只读风格
	m_editHex.ModifyStyle( m_bReadOnly ? 0 :ES_READONLY,
		m_bReadOnly ? ES_READONLY : 0 );

	m_szAddress.Format(L"08%X", m_dwAddress);
	m_szSize.Format(L"%X", m_dwSize);
	m_btnReadOnly.SetWindowText(szReadOnly[g_enumLang]);

	UpdateData(FALSE);

	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid, m_dwEprocess);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CMemoryHexEditDlg::DumpMemoryToRing3(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess)
{
	BOOL bRet = FALSE;

	if (Size > 0)
	{
		if (Address >= 0x80000000)
		{
			return FALSE;
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
			m_editHex.SetAddressBase(m_dwAddress);
			m_editHex.SetData((PBYTE)pBuffer, Size, -1);
			m_editHex.RedrawWindow();
		}
	}

	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	return bRet;
}

void CMemoryHexEditDlg::OnEnChangeHexEdit()
{
	PBYTE pBuffer = (PBYTE)malloc(m_dwSize);
	if (!pBuffer)
	{
		return;
	}
	
	HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, m_dwPid);
	if (!hProcess)
	{
		free(pBuffer);
		return;
	}

	memset(pBuffer, 0, m_dwSize);
	m_editHex.GetData(pBuffer, m_dwSize);

	DWORD dwOldProtect;
	if (m_ProcessFunc.VirtualProtectEx(hProcess, (PVOID)m_dwAddress, m_dwSize, PAGE_READWRITE, &dwOldProtect))
	{
		DWORD dwRet = 0;
		m_ProcessFunc.WriteProcessMemory(hProcess, (PVOID)m_dwAddress, pBuffer, m_dwSize, &dwRet);

		m_ProcessFunc.VirtualProtectEx(hProcess, (PVOID)m_dwAddress, m_dwSize, dwOldProtect, &dwOldProtect);
	}

	CloseHandle(hProcess);
	free(pBuffer);
}

// 如果是HEX控件，并且是insert插入键，那么就直接返回
BOOL CMemoryHexEditDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		int id = ::GetDlgCtrlID(pMsg->hwnd);
		if (id == IDC_HEX_EDIT)
		{
			// 如果是insert键，就直接返回
			if (VK_INSERT == pMsg->wParam)
			{
				return TRUE;
			}
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CMemoryHexEditDlg::OnBnClickedCheckReadOnly()
{
	UpdateData(TRUE);

	m_editHex.ModifyStyle( m_bReadOnly ? 0 :ES_READONLY,
		m_bReadOnly ? ES_READONLY : 0 );

	m_editHex.Invalidate();
	m_editHex.UpdateWindow();
}
