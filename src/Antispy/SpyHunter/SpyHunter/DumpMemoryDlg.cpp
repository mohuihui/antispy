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
// DumpMemoryDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "DumpMemoryDlg.h"
#include <shlwapi.h>

// CDumpMemoryDlg 对话框

IMPLEMENT_DYNAMIC(CDumpMemoryDlg, CDialog)

CDumpMemoryDlg::CDumpMemoryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDumpMemoryDlg::IDD, pParent)
	, m_szAddress(_T(""))
	, m_szSize(_T(""))
{
	m_dwPid = 0;
	m_pEprocess = 0;
}

CDumpMemoryDlg::~CDumpMemoryDlg()
{
}

void CDumpMemoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ADDRESS, m_szAddress);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_szSize);
}


BEGIN_MESSAGE_MAP(CDumpMemoryDlg, CDialog)
	ON_BN_CLICKED(ID_DUMP_OK, &CDumpMemoryDlg::OnBnClickedDumpOk)
	ON_BN_CLICKED(ID_DUMP_CANCEL, &CDumpMemoryDlg::OnBnClickedDumpCancel)
END_MESSAGE_MAP()


// CDumpMemoryDlg 消息处理程序

void CDumpMemoryDlg::OnBnClickedDumpOk()
{
	ShowWindow(FALSE);

	UpdateData(TRUE);

	ULONG Base = 0, Size = 0;
	swscanf_s(m_szAddress.GetBuffer(), L"%x", &Base);
	m_szAddress.ReleaseBuffer();
	swscanf_s(m_szSize.GetBuffer(), L"%x", &Size);
	m_szSize.ReleaseBuffer();

	if (Base > 0x80000000 || (Base + Size) > 0x80000000)
	{
		MessageBox(szDumpFailed[g_enumLang], szToolName);
		ShowWindow(TRUE);
		return;
	}

	CString szFileName;
	szFileName.Format(L"memory_0x%08X_0x%08X", Base, Size);
	szFileName = szFileName + L".dat";

	CFileDialog fileDlg( FALSE, 0, szFileName, 0, L"Data Files (*.dat)|*.dat;|All Files (*.*)|*.*||", 0 );
	if (IDOK == fileDlg.DoModal())
	{
		CString szFilePath = fileDlg.GetFileName();

		if ( !PathFileExists(szFilePath) ||
			(PathFileExists(szFilePath) && MessageBox(szFileExist[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDYES))
		{
			BOOL bCopy = FALSE;
			PVOID pBuffer = GetMemory(Size);
			if (pBuffer)
			{
				COMMUNICATE_PROCESS_MODULE cpmd;

				cpmd.op.Dump.Base = Base;
				cpmd.op.Dump.Size = Size;
				cpmd.nPid = m_dwPid;
				cpmd.pEprocess = m_pEprocess;
				cpmd.OpType = enumDumpDllModule;

				BOOL bRet = g_ConnectDriver.CommunicateDriver(&cpmd, sizeof(COMMUNICATE_PROCESS_MODULE), pBuffer, Size, NULL);
				if (bRet)
				{
					CFile file;
					TRY 
					{
						if (file.Open(szFilePath, CFile::modeCreate | CFile::modeWrite))
						{
							file.Write(pBuffer, Size);
							file.Close();
							bCopy = TRUE;
						}
					}
					CATCH_ALL( e )
					{
						file.Abort();   // close file safely and quietly
						//THROW_LAST();
					}
					END_CATCH_ALL
				}

				free(pBuffer);
				pBuffer = NULL;
			}

			if (bCopy)
			{
				MessageBox(szDumpOK[g_enumLang], szToolName);
				OnCancel();
			}
			else
			{
				MessageBox(szDumpFailed[g_enumLang], szToolName);
				ShowWindow(TRUE);
			}
		}
	}
}

BOOL CDumpMemoryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(szDumpDlgMemory[g_enumLang]);
	GetDlgItem(IDC_DUMP_ADDRESS)->SetWindowText(szDumpAddress[g_enumLang]);
	GetDlgItem(IDC_DUMP_SIZE)->SetWindowText(szDumpSize[g_enumLang]);
	GetDlgItem(ID_DUMP_OK)->SetWindowText(szOk[g_enumLang]);
	GetDlgItem(ID_DUMP_CANCEL)->SetWindowText(szCancel[g_enumLang]);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDumpMemoryDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if((pMsg->hwnd == (GetDlgItem(IDC_EDIT_ADDRESS))->GetSafeHwnd() ||
		pMsg->hwnd == (GetDlgItem(IDC_EDIT_SIZE))->GetSafeHwnd()) &&   
		pMsg->message == WM_KEYDOWN)
	{ 
		UpdateData(TRUE);

		if (pMsg->hwnd == (GetDlgItem(IDC_EDIT_ADDRESS))->GetSafeHwnd())
		{
			if (m_szAddress.GetLength() >= 8 && (char)pMsg->wParam != VK_BACK)
			{
				return TRUE;
			}
		}
		else if (pMsg->hwnd == (GetDlgItem(IDC_EDIT_SIZE))->GetSafeHwnd())
		{
			if (m_szSize.GetLength() >= 8 && (char)pMsg->wParam != VK_BACK)
			{
				return TRUE;
			}
		}
		
		char c = (char)pMsg->wParam; 
		if( (c >= '0' && c <= '9') || 
			(c >= 'a' && c <= 'f') || 
			(c >= 'A' && c <= 'F') ||
			c == VK_BACK
			)
		{ 
			return CDialog::PreTranslateMessage(pMsg);
		} 
		else
		{ 
			return TRUE; 
		} 
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CDumpMemoryDlg::OnBnClickedDumpCancel()
{
	OnCancel();
}