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
// ResourceEncryptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ResourceEncrypt.h"
#include "ResourceEncryptDlg.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CResourceEncryptDlg dialog




CResourceEncryptDlg::CResourceEncryptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResourceEncryptDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CResourceEncryptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(CResourceEncryptDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CResourceEncryptDlg::OnBnClickedOk)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CResourceEncryptDlg message handlers

BOOL CResourceEncryptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, L"文件路径", LVCFMT_LEFT, 390);
	m_list.InsertColumn(1, L"加密状态", LVCFMT_LEFT, 150);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CResourceEncryptDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CResourceEncryptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CResourceEncryptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD CResourceEncryptDlg::GetChecksum(PVOID pBase, DWORD FileSize)
{
	DWORD	checksum,dwhold,dwdata;
	DWORD64 dwtemp64;
	UCHAR	_temp;

	if (pBase == NULL)
	{
		return 0;
	}
	checksum = 0;
	dwhold = 0xde20ad12;

	for(DWORD i = 0; i< FileSize; i++)
	{
		CopyMemory(&_temp, (BYTE *)pBase + i, 1);
		dwtemp64 = _temp * dwhold;
		dwdata = DWORD(dwtemp64);
		dwtemp64 = dwtemp64 >> 32;
		dwhold = DWORD(dwtemp64);
		checksum = checksum + dwdata;
		dwhold++;
	}

	return checksum;
}

void CResourceEncryptDlg::DecryptResource(PVOID pBuffer, DWORD dwSize)
{
// 	char sz32Key[7] = {'\x55','\x8b','\xec','\x33','\xc0','\x50','\xc3'};
// 	DWORD dwKey = GetChecksum(sz32Key, 7);
// 	BYTE btKey = (BYTE)(dwKey / 'M');
// 
// 	unsigned char *pFileBuffer = (unsigned char *)pBuffer;
// 	DWORD dwCount = dwSize >> 2;
// 	DWORD dwRawContent = 0;
// 	for (DWORD i = 0; i < dwCount; i++)
// 	{
// 		DWORD dwContent = *(DWORD*)(pFileBuffer + i * 4);
// 		
// 		if (dwContent == 0 || dwContent == dwKey)
// 		{
// 			continue;
// 		}
// 
// 		DWORD dwEncryptContent = dwContent ^ dwKey;
// 		*(DWORD*)(pFileBuffer + i * 4) = dwEncryptContent;
// 
// 		dwKey = dwContent;
// 	}
	

// 	char sz32Key[7] = {'\x55','\x8b','\xec','\x33','\xc0','\x50','\xc3'};
// 	DWORD dwKey = GetChecksum(sz32Key, 7);
	BYTE btKey = /*(BYTE)(dwKey / */'m'/*)*/;
	DWORD i = 0;
	BYTE *pFileBuffer = (BYTE *)pBuffer;
	DWORD dwRawContent = 0;
	for (; i < dwSize; i++)
	{
		BYTE dwContent = *(BYTE*)(pFileBuffer + i);
		
		if (dwContent == 0 || dwContent == btKey)
		{
			continue;
		}

		BYTE dwEncryptContent = dwContent ^ btKey;
		*(BYTE*)(pFileBuffer + i) = dwEncryptContent;

		btKey = dwContent;
	}
}

void CResourceEncryptDlg::OnBnClickedOk()
{
	int nCnt = m_list.GetItemCount();
	for ( int i = 0; i < nCnt; i++ )
	{
		BOOL bCopy = FALSE;
		CString szPath = m_list.GetItemText(i, 0);
		HANDLE hFile = CreateFile(szPath, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwSzie = GetFileSize(hFile, NULL);
			if (dwSzie > 0)
			{
				PVOID pBuffer = malloc(dwSzie);
				if (pBuffer != NULL)
				{
					memset(pBuffer, 0, dwSzie);
					DWORD dwRet= 0;
					if (ReadFile(hFile, pBuffer, dwSzie, &dwRet, NULL))
					{
						CloseHandle(hFile);
						hFile = INVALID_HANDLE_VALUE;

						DecryptResource(pBuffer, dwSzie);
						
						CString szPathBak = szPath + L".bak";
						if (CopyFile(szPath, szPathBak, TRUE))
						{
							CFile file;
							TRY 
							{
								if (file.Open(szPath, CFile::modeCreate | CFile::modeWrite))
								{
									file.Write(pBuffer, dwSzie);
									file.Close();
									bCopy = TRUE;
								}
							}
							CATCH_ALL( e )
							{
								file.Abort();   // close file safely and quietly
								THROW_LAST();
							}
							END_CATCH_ALL
						}
					}

					free(pBuffer);
				}
			}
		}

		if (bCopy)
		{
			m_list.SetItemText(i, 1, L"成功");
		}
		else
		{
			m_list.SetItemText(i, 1, L"失败");
		}

		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
	}
}

void CResourceEncryptDlg::AddFiles(CString szPath)
{
	if (szPath.IsEmpty() || !PathFileExists(szPath))
	{
		return;
	}
	
	int nItem = m_list.InsertItem(m_list.GetItemCount(), szPath);
}

void CResourceEncryptDlg::OnDropFiles(HDROP hDropInfo)
{
	m_list.DeleteAllItems();

	UINT count = 0;          
	WCHAR filePath[MAX_PATH] = {0};            
	count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);          
	if(count)           
	{
		for(UINT i = 0; i < count; i++)                    
		{
			int pathLen = DragQueryFile(hDropInfo, i, filePath, sizeof(filePath));       
			AddFiles(filePath);
		}
	}
	DragFinish(hDropInfo); 

	CDialog::OnDropFiles(hDropInfo);
}
