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
// HostDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HostDlg.h"
#include <shlwapi.h>

// CHostDlg 对话框

IMPLEMENT_DYNAMIC(CHostDlg, CDialog)

CHostDlg::CHostDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHostDlg::IDD, pParent)
	, m_szHosts(_T(""))
{
	
}

CHostDlg::~CHostDlg()
{
}

void CHostDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT, m_szHosts);
}


BEGIN_MESSAGE_MAP(CHostDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CHostDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_REFRESH, &CHostDlg::OnBnClickedBtnRefresh)
	ON_BN_CLICKED(IDC_SURE, &CHostDlg::OnBnClickedSure)
	ON_BN_CLICKED(IDC_OPEN_NOTEPAD, &CHostDlg::OnBnClickedOpenNotepad)
	ON_BN_CLICKED(IDC_HOST_RESRT, &CHostDlg::OnBnClickedHostResrt)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CHostDlg)
	EASYSIZE(IDC_EDIT, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_BTN_REFRESH, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_SURE, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_HOST_RESRT, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_OPEN_NOTEPAD, ES_KEEPSIZE, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CHostDlg 消息处理程序

void CHostDlg::OnBnClickedOk()
{
}

void CHostDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CHostDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	//if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CHostDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_BTN_REFRESH)->SetWindowText(szRefresh[g_enumLang]);
	GetDlgItem(IDC_SURE)->SetWindowText(szSave[g_enumLang]);
	GetDlgItem(IDC_OPEN_NOTEPAD)->SetWindowText(szOpenWithNotepad[g_enumLang]);
	GetDlgItem(IDC_HOST_RESRT)->SetWindowText(szResetToDefult[g_enumLang]);
	INIT_EASYSIZE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CHostDlg::ReadHostsFile()
{
	if (!g_bLoadDriverOK)
	{
		return;
	}

	m_szHosts = L"";

	WCHAR szHosts[MAX_PATH] = {0};
	GetSystemDirectory(szHosts, MAX_PATH);
	WCHAR szHost[] = {'\\','d','r','i','v','e','r','s','\\','e','t','c','\\','h','o','s','t','s','\0'};
	wcsncat_s(szHosts, MAX_PATH - wcslen(szHosts), szHost, wcslen(szHost));

	if (PathFileExists(szHosts))
	{
		// 打开文件
// 		CStdioFile file;
// 		if (!file.Open(szHosts, CFile::modeRead))
// 		{
// 			return ;
// 		}
// 
// 		// 开始读文件
// 		CString strText = _T("");
// 		while(file.ReadString(strText)) // 一行一行的读取
// 		{
// 			m_szHosts += strText;
// 			m_szHosts += L"\r\n";
// 		}  
// 
// 		//关闭文件
// 		file.Close();

		CFile* pFile = NULL;
		// Constructing a CFile object with this override may throw
		// a CFile exception, and won't throw any other exceptions.
		// Calling CString::Format() may throw a CMemoryException,
		// so we have a catch block for such exceptions, too. Any
		// other exception types this function throws will be
		// routed to the calling function.
		TRY
		{
			pFile = new CFile(szHosts, CFile::modeRead | CFile::shareDenyNone);
			if (pFile)
			{
				ULONGLONG dwLength = pFile->GetLength() + 10;
				PVOID pBuffer = malloc((size_t)dwLength);
				if (pBuffer)
				{
					memset(pBuffer, 0, (size_t)dwLength);
					pFile->Read(pBuffer, (UINT)pFile->GetLength());
					m_szHosts = m_Functions.ms2ws((LPCSTR)pBuffer);
					free(pBuffer);
				}
			}
		}
		CATCH(CFileException, pEx)
		{
			// Simply show an error message to the user.
			pEx->ReportError();
		}
		AND_CATCH(CMemoryException, pEx)
		{
			// We can't recover from this memory exception, so we'll
			// just terminate the app without any cleanup. Normally, an
			// an application should do everything it possibly can to
			// clean up properly and _not_ call AfxAbort().
			AfxAbort();
		}
		END_CATCH

		// If an exception occurs in the CFile constructor,
		// the language will free the memory allocated by new
		// and will not complete the assignment to pFile.
		// Thus, our clean-up code needs to test for NULL.
		if (pFile != NULL)   {
			pFile->Close();
			delete pFile;
		}
	}

	UpdateData(FALSE);
}

void CHostDlg::OnBnClickedBtnRefresh()
{
	ReadHostsFile();
}

void CHostDlg::OnBnClickedSure()
{
	UpdateData(TRUE);

	WCHAR szHosts[MAX_PATH] = {0};
	GetSystemDirectory(szHosts, MAX_PATH);
	WCHAR szHost[] = {'\\','d','r','i','v','e','r','s','\\','e','t','c','\\','h','o','s','t','s','\0'};
	wcsncat_s(szHosts, MAX_PATH - wcslen(szHosts), szHost, wcslen(szHost));

	if (PathFileExists(szHosts))
	{
		// 打开文件
		CFile file;
		TRY 
		{
			if (!file.Open(szHosts, CFile::modeReadWrite))
			{
				return;
			}

			WCHAR *szFuck = (WCHAR *)malloc((m_szHosts.GetLength() + 1) * sizeof(WCHAR));
			if (szFuck)
			{
				memset(szFuck, 0, (m_szHosts.GetLength() + 1) * sizeof(WCHAR));
				wcsncpy_s(szFuck, m_szHosts.GetLength() + 1, m_szHosts.GetBuffer(), m_szHosts.GetLength());
				m_szHosts.ReleaseBuffer();

				file.SetLength(0);
				file.SeekToBegin();

				CStringA szTmep = ws2ms((LPWSTR)szFuck);
				file.Write(szTmep.GetBuffer(), szTmep.GetLength());

				free(szFuck);
				szFuck = NULL;
			}
		
			//关闭文件
			file.Close();
		}
		CATCH_ALL( e )
		{
			file.Abort();   // close file safely and quietly
			//THROW_LAST();
		}
		END_CATCH_ALL
	}
}

// WCHAR转TCHAR
CStringA CHostDlg::ws2ms(LPWSTR szSrc, int cbMultiChar)
{
	CStringA strDst;
	if ( szSrc==NULL || cbMultiChar==0 ){
		return strDst;
	}

	CHAR*pBuff=NULL;
	int nLen=WideCharToMultiByte(CP_ACP,NULL,szSrc,cbMultiChar,NULL,0,NULL,FALSE);
	if ( nLen>0 ){
		pBuff=new CHAR[nLen+1];
		if ( pBuff ){
			WideCharToMultiByte(CP_ACP,NULL,szSrc,cbMultiChar,pBuff,nLen,NULL,FALSE);
			pBuff[nLen]=0;
			strDst = pBuff;
			delete[] pBuff;
		}
	}

	return strDst;
}

void CHostDlg::OnBnClickedOpenNotepad()
{
	WCHAR szHosts[MAX_PATH] = {0};
	GetSystemDirectory(szHosts, MAX_PATH);
	WCHAR szHost[] = {'\\','d','r','i','v','e','r','s','\\','e','t','c','\\','h','o','s','t','s','\0'};
	wcsncat_s(szHosts, MAX_PATH - wcslen(szHosts), szHost, wcslen(szHost));

	WCHAR szOpen[] = {'o','p','e','n','\0'};
	WCHAR szNotepad[] = {'n','o','t','e','p','a','d','.','e','x','e','\0'};
	ShellExecute(0, szOpen, szNotepad, szHosts, NULL, SW_SHOWNORMAL);
}

void CHostDlg::OnBnClickedHostResrt()
{
	if (MessageBox(szAreSureResetToDefult[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	CHAR szReset[] = {'#',' ','r','e','s','e','t',' ','t','o',' ','d','e','f','a','u','l','t',' ','b','y',' ','A','n','t','i','S','p','y',' ','(','a','n','t','i','s','p','y','@','1','6','3','.','c','o','m',')','\0'};
	CHAR szDefault[] = {'1','2','7','.','0','.','0','.','1',' ','l','o','c','a','l','h','o','s','t','\0'};

	WCHAR szHosts[MAX_PATH] = {0};
	GetSystemDirectory(szHosts, MAX_PATH);
	WCHAR szHost[] = {'\\','d','r','i','v','e','r','s','\\','e','t','c','\\','h','o','s','t','s','\0'};
	wcsncat_s(szHosts, MAX_PATH - wcslen(szHosts), szHost, wcslen(szHost));

	if (PathFileExists(szHosts))
	{
		// 打开文件
		CFile file;
		TRY 
		{
			if (!file.Open(szHosts, CFile::modeReadWrite))
			{
				return;
			}

			file.SetLength(0);
			file.SeekToBegin();

			file.Write(szReset, strlen(szReset));
			file.Write("\r\n", strlen("\r\n"));
			file.Write(szDefault, strlen(szDefault));

			//关闭文件
			file.Close();
		}
		CATCH_ALL( e )
		{
			file.Abort();   // close file safely and quietly
			//THROW_LAST();
		}
		END_CATCH_ALL
	}

	OnBnClickedBtnRefresh();
}
