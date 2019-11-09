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
// UpdateDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "UpdateDlg.h"
#include <afxinet.h>
#include "DownloadCallback.h"
#include "unzip.h"
#include "shlwapi.h"

#define WM_UPDATE_EDIT_DATA	 WM_USER + 6
CDownloadCallback g_DownloadCallback;

// CUpdateDlg 对话框

IMPLEMENT_DYNAMIC(CUpdateDlg, CDialog)

CUpdateDlg::CUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateDlg::IDD, pParent)
	, m_szUpdateLogEdit(_T(""))
{
	m_bUpdate = FALSE;
	m_szUpdateURL = L"";
	m_szVersion = L"";
	m_szUpdateLog = L"";
	m_szRarPath = L"";
	m_szNewVersionPath = L"";
	m_bStopDownload = FALSE;
	g_DownloadCallback.m_pDlg = NULL;
	g_DownloadCallback.m_bStop = FALSE;
	m_hConnectThread = NULL;
	m_hDownloadThread = NULL;
}

CUpdateDlg::~CUpdateDlg()
{
	if (m_hConnectThread)
	{
		CloseHandle(m_hConnectThread);
		m_hConnectThread = NULL;
	}

	if (m_hDownloadThread)
	{
		CloseHandle(m_hDownloadThread);
		m_hDownloadThread = NULL;
	}
}

void CUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Text(pDX, IDC_EDIT_UPDATE_LOG, m_szUpdateLogEdit);
	DDX_Control(pDX, IDOK, m_btnUpdate);
	DDX_Control(pDX, IDC_EDIT_UPDATE_LOG, m_OutputEdit);
}


BEGIN_MESSAGE_MAP(CUpdateDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CUpdateDlg::OnBnClickedOk)
	ON_MESSAGE(WM_UPDATE_EDIT_DATA, OnUpdateData)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CUpdateDlg 消息处理程序

DWORD WINAPI DownLoadProc(LPVOID pParam)
{
	CUpdateDlg* pDlg = (CUpdateDlg*)pParam;
	if (pDlg && !(pDlg->m_bStopDownload))
	{
		if (!pDlg->OnDownLoad())
		{
			if (pDlg->m_bStopDownload)
			{
				return 0;
			}

			pDlg->m_Progress.ShowWindow(FALSE);

			pDlg->m_szUpdateLogEdit += L"\r\n";
			pDlg->m_szUpdateLogEdit += L"-------------------------------------------------------------\r\n";
			pDlg->m_szUpdateLogEdit += szDownloadError[g_enumLang];
			if (!pDlg->m_bStopDownload)
			{
				pDlg->SendMessage(WM_UPDATE_EDIT_DATA);
			}
		}
		else
		{
			if (pDlg->m_bStopDownload)
			{
				return 0;
			}

			pDlg->m_Progress.ShowWindow(FALSE);

			if (!pDlg->StartUnpack(pDlg->m_szRarPath))
			{
				pDlg->m_szUpdateLogEdit += L"\r\n";
				pDlg->m_szUpdateLogEdit += L"-------------------------------------------------------------\r\n";
				pDlg->m_szUpdateLogEdit += szUpdateFailed[g_enumLang];
				if (!pDlg->m_bStopDownload)
				{
					pDlg->SendMessage(WM_UPDATE_EDIT_DATA);
				}
			}
			else
			{
				pDlg->m_btnUpdate.SetWindowText(szGotoNewVersion[g_enumLang]);
				pDlg->m_btnUpdate.EnableWindow(TRUE);
				pDlg->m_szUpdateLogEdit += L"\r\n";
				pDlg->m_szUpdateLogEdit += L"-------------------------------------------------------------\r\n";
				pDlg->m_szUpdateLogEdit += szUpdateSuccess[g_enumLang];
				if (!pDlg->m_bStopDownload)
				{
					pDlg->SendMessage(WM_UPDATE_EDIT_DATA);
				}
			}
		}
	}

	return 0;
}

BOOL CUpdateDlg::OnDownLoad()
{
	BOOL bRet = FALSE;
	m_Progress.SetPos(0);

	DWORD dwLen = MAX_PATH;
	DWORD dwRet = 0;
	WCHAR *szPath = NULL;

	do 
	{
		if (szPath != NULL)
		{
			free(szPath);
			szPath = NULL;
		}

		szPath = (WCHAR*)malloc(dwLen * sizeof(WCHAR));
		if (szPath == NULL)
		{
			return FALSE;
		}
		
		memset(szPath, 0, dwLen * sizeof(WCHAR));

		dwRet = GetTempPath(dwLen - 1, szPath);
		if (dwRet <= dwLen - 1)
		{
			break;
		}

		dwLen = dwRet + 1;

	} while (1);

	if (szPath)
	{
		CString strNewFile = szPath;
		if (szPath[wcslen(szPath)-1] != '\\')
		{
			strNewFile += L"\\";
		}

		if (szPath)
		{
			free(szPath);
			szPath = NULL;
		}

		CString szName = m_szUpdateURL.Right(m_szUpdateURL.GetLength() - m_szUpdateURL.ReverseFind('/') - 1);
		strNewFile += szName;

		HRESULT hr = E_FAIL;
// 		CDownloadCallback callback;
		g_DownloadCallback.m_pDlg = this;
		g_DownloadCallback.m_bStop = FALSE;
		
	//	m_szUpdateURL = L"http://antispy.googlecode.com/files/AntiSpy1.1.zip";
		hr = URLDownloadToFile ( 
			NULL, 
			m_szUpdateURL/*L"http://tj.yun.ftn.qq.com/ftn_handler/cfd1c00143c0ed7463069fe878545f5fb5755b21754455d6d66a403af409574b3b6c7ddf418678da7631271afb0c82576fc60b0e212021f5b1450415f1d063c0/AntiSpy1.1.zip"*/, 
			strNewFile, 
			0, 
			&g_DownloadCallback  
			);

		if (SUCCEEDED(hr))
		{
			if (PathFileExists(strNewFile))
			{
				bRet = TRUE;
				m_szRarPath = strNewFile;
			//	StartUnpack(strNewFile);
			}
		}

	//	DeleteFile(strNewFile);
	}

	return bRet;
}

BOOL CUpdateDlg::StartUnpack(CString szRARPath)
{
	BOOL bRet = FALSE;
	if (szRARPath.IsEmpty() || !PathFileExists(szRARPath))
	{
		return bRet;
	}

	DWORD dwLen = MAX_PATH;
	DWORD dwRet = 0;
	WCHAR *szPath = NULL;
	BOOL bUnpack = FALSE;

	do 
	{
		if (szPath != NULL)
		{
			free(szPath);
			szPath = NULL;
		}

		szPath = (WCHAR*)malloc(dwLen * sizeof(WCHAR));
		if (szPath == NULL)
		{
			return FALSE;
		}

		memset(szPath, 0, dwLen * sizeof(WCHAR));

		dwRet = GetModuleFileName(NULL, szPath, dwLen - 1);
		if (dwRet <= dwLen - 1)
		{
			break;
		}

		dwLen = dwRet + 1;

	} while (1);
	
	CString szUnpackPath;
	if (szPath && wcslen(szPath) > 0)
	{
		szUnpackPath = szPath;
		szUnpackPath = szUnpackPath.Left(szUnpackPath.ReverseFind('\\'));
		szUnpackPath += L"\\";
		szUnpackPath += m_szVersion;
	
		if (!PathFileExists(szUnpackPath))
		{
			if (!CreateDirectory(szUnpackPath, NULL))
			{
				if (szPath)
				{
					free (szPath);
					szPath = NULL;
				}

				return FALSE;
			}
		}

		m_szNewVersionPath = szUnpackPath;
// 		SetCurrentDirectory(szUnpackPath);
// 		HZIP hz = OpenZip(szRARPath, 0);
// 		if (hz)
// 		{
// 			ZIPENTRY ze; 
// 			GetZipItem(hz, -1, &ze); 
// 			int numitems = ze.index;
// 			for (int i = 0; i < numitems; i++)
// 			{ 
// 				GetZipItem(hz, i, &ze);
// 				UnzipItem(hz, i, ze.name);
// 			}
// 			
// 			bRet = TRUE;
// 			bUnpack = TRUE;
// 			CloseZip(hz);
// 		}

		HZIP hz;
		hz = OpenZip(szRARPath, 0);

		if (hz) 
		{
			SetUnzipBaseDir(hz, szUnpackPath);
			ZIPENTRY ze; 
			GetZipItem(hz, -1, &ze); 
			int numitems = ze.index;

			for (int zi=0; zi<numitems; zi++) 
			{ 
				GetZipItem(hz,zi,&ze);
				UnzipItem(hz,zi,ze.name);
			}

			bRet = TRUE;
			bUnpack = TRUE;
			CloseZip(hz);
		}
	}

	if (!bUnpack && !szUnpackPath.IsEmpty())
	{
		CString szName = szRARPath.Right(szRARPath.GetLength() - szRARPath.ReverseFind('\\') - 1);
		szUnpackPath += L"\\";
		szUnpackPath += szName;

		if (CopyFile(szRARPath, szUnpackPath, FALSE))
		{
			bRet = TRUE;
		}
	}

	if (szPath)
	{
		free (szPath);
		szPath = NULL;
	}

	DeleteFile(szRARPath);
	return bRet;
}

void CUpdateDlg::OnBnClickedOk()
{
	CString szBtn;
	m_btnUpdate.GetWindowText(szBtn);
	
	if (!szBtn.CompareNoCase(szUpdateNow[g_enumLang]))
	{
		m_szNewVersionPath = L"";
		m_bUpdate = FALSE;
		m_btnUpdate.EnableWindow(FALSE);
		m_Progress.ShowWindow(TRUE);
		m_szRarPath = L"";

		DWORD dwTid = 0;
		m_hDownloadThread = CreateThread(NULL, 0, DownLoadProc, this, 0, &dwTid);
	}
	else
	{
		if (!m_szNewVersionPath.IsEmpty())
		{
			ShellExecute(NULL, NULL, m_szNewVersionPath, NULL, NULL, SW_SHOW);
		}

		OnOK();
	}
}

DWORD WINAPI UpdateProc(LPVOID pParam)
{
	CUpdateDlg *pDlg = (CUpdateDlg*)pParam;
	
	if (pDlg)
	{
		if ( !pDlg->CheckForUpdates() && !(pDlg->m_bStopDownload) )
		{
			pDlg->m_szUpdateLogEdit += L"\r\n";
			pDlg->m_szUpdateLogEdit += L"-------------------------------------------------------------\r\n";
			pDlg->m_szUpdateLogEdit += szUpdateError[g_enumLang];

			if (!(pDlg->m_bStopDownload))
			{
				pDlg->SendMessage(WM_UPDATE_EDIT_DATA);
			}
		}
	}

	return 0;
}

BOOL CUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(szUpdate[g_enumLang]);
	m_btnUpdate.EnableWindow(FALSE);
	m_Progress.ShowWindow(FALSE);
	
	m_btnUpdate.SetWindowText(szUpdateNow[g_enumLang]);
	m_szUpdateLogEdit = szCheckForUpdate[g_enumLang];
	UpdateData(FALSE);
	
	DWORD dwTid = 0;
	m_hConnectThread = CreateThread(NULL, 0, UpdateProc, this, 0, &dwTid);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

inline CString ms2ws(LPCSTR szSrc, int cbMultiChar=-1)
{
	CString strDst;
	if ( szSrc==NULL || cbMultiChar==0 ){
		return strDst;
	}

	WCHAR*pBuff=NULL;
	int nLen=MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,NULL,0);
	if ( nLen>0 ){
		pBuff=new WCHAR[nLen+1];
		if ( pBuff ){
			MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,pBuff,nLen);
			pBuff[nLen]=0;
			strDst = pBuff;
			delete[] pBuff;
		}
	}

	return strDst;
}

BOOL CUpdateDlg::CheckForUpdates()
{
	if (!m_bStopDownload)
	{
		SendMessage(WM_UPDATE_EDIT_DATA);
	}
	
	BOOL bRet = FALSE;
	WCHAR szUrlChinese[] = {'h','t','t','p',':','/','/','m','z','f','2','0','0','8','.','b','l','o','g','.','1','6','3','.','c','o','m','/','b','l','o','g','/','s','t','a','t','i','c','/','3','5','5','9','9','7','8','6','2','0','1','1','4','1','8','7','2','6','8','5','0','2','/','\0'};
	WCHAR szUrlEnglsh[] = {'h','t','t','p',':','/','/','m','z','f','2','0','0','8','.','b','l','o','g','.','1','6','3','.','c','o','m','/','b','l','o','g','/','s','t','a','t','i','c','/','3','5','5','9','9','7','8','6','2','0','1','1','2','2','1','6','1','6','4','8','5','3','8','/','\0'};
	WCHAR *szUrl = NULL;
	CStringA strline;
	CString szSource;
	CHttpFile* myHttpFile = NULL;
	CInternetSession mySession(NULL, 0);

	m_bUpdate = FALSE;
	m_szUpdateURL = L"";
	m_szVersion = L"";
	m_szUpdateLog = L"";
	
	if (g_enumLang == enumChinese)
	{
		szUrl = szUrlChinese /*L"http://www.trojantesting.com/trojanpage.html"*/;
	}
	else
	{
		szUrl = szUrlEnglsh;
	}

	try
	{
		myHttpFile = (CHttpFile*)mySession.OpenURL(szUrl);
		if (myHttpFile)
		{
			m_szUpdateLogEdit += L"\r\n";
			m_szUpdateLogEdit += szConnectedOK[g_enumLang];

			if (!m_bStopDownload)
			{
				SendMessage(WM_UPDATE_EDIT_DATA);
				Sleep(1000);
			}
			
			BOOL bStart = FALSE;
			CHAR szEnd[] = {'n','b','w','-','b','l','o','g','-','e','n','d','\0'};
			CHAR szStart[] = {'n','b','w','-','b','l','o','g','-','s','t','a','r','t','\0'};
		
			while(myHttpFile->ReadString((CString&)strline))
			{
				if (strline.Find(szEnd) != -1)
				{
					bStart = FALSE;
					break;
				}

				if (bStart)
				{
					szSource += ms2ws(strline.GetBuffer());
				}

				if (strline.Find(szStart) != -1)
				{
					bStart = TRUE;
				}
			}

			myHttpFile->Close();
			mySession.Close();
		}
	}
	catch(CInternetException *IE)
	{
		if (myHttpFile)
		{
			myHttpFile->Close();
		}
		IE->Delete();
	}

	if (!szSource.IsEmpty())
	{
		WCHAR Version[] = {'v','e','r','s','i','o','n',':','\0'};	// version:
		WCHAR DIV[] = {'<','d','i','v','>','\0'};					// <div>
		WCHAR Update[] = {'u','p','d','a','t','e',':','\0'};		// update:
	//	WCHAR UNDIV[] = {'<','/','d','i','v','>','\0'};				// </div>
		WCHAR UNDIV[] = {'<','/','\0'};								// </
		WCHAR rn[] = {'\r','\n','\0'};								// \r\n
		WCHAR szRUL[] = {'U','R','L',':','\0'};						// URL:

		// 获取版本信息
		int nDiv = -1;
		int nPos = szSource.Find(Version);
		if (nPos != -1)
		{
			nPos += wcslen(Version);
			nDiv = szSource.Find(UNDIV, nPos);
			if (nDiv != -1 && nDiv > nPos)
			{
				m_szVersion = szSource.Mid(nPos, nDiv-nPos);
			}
		}
		
		if (m_szVersion.IsEmpty()) return FALSE;

		// 获取下载地址
		nPos = szSource.Find(szRUL, nPos);
		if (nPos != -1)
		{
			nPos += wcslen(szRUL);
			nDiv = szSource.Find(UNDIV, nPos);
			if (nDiv != -1 && nDiv > nPos)
			{
				m_szUpdateURL = szSource.Mid(nPos, nDiv-nPos);
			}
		}

		if (m_szUpdateURL.IsEmpty()) return FALSE;
		
		// 获取更新日志
		nPos = szSource.Find(Update, nPos);
		while (nPos != -1)
		{
			nPos = szSource.Find(DIV, nPos);
			if (nPos == -1)
			{
				break;
			}

			nPos += wcslen(DIV);
			nDiv = szSource.Find(UNDIV, nPos);
			if (nDiv != -1 && nDiv > nPos)
			{
				m_szUpdateLog += szSource.Mid(nPos, nDiv - nPos);
				m_szUpdateLog += rn;
			}
			else
			{
				break;
			}
		}

		m_szUpdateLog.TrimRight('\n');
		m_szUpdateLog.TrimRight('\r');
		if (m_szUpdateLog.IsEmpty()) return FALSE;
	
		// 获取更新信息都正常的情况下，返回TRUE
		bRet = TRUE;
		m_bUpdate = TRUE;
	}

	if (!m_bStopDownload)
	{
		SendMessage(WM_UPDATE_EDIT_DATA);
	}
	
	return bRet;
}

LRESULT CUpdateDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	if (m_bUpdate)
	{
		m_szUpdateLogEdit += L"\r\n";
		m_szUpdateLogEdit += L"-------------------------------------------------------------\r\n";

		// 如果已经是最新版本
		if (!m_szVersion.CompareNoCase(g_szVersion))
		{
			m_szUpdateLogEdit += szAlreadyHaveRecentVersion[g_enumLang];
		}
		else // 否则更新
		{
			m_szUpdateLogEdit += szCurrentVersion[g_enumLang]; //L"当前版本：";//L"Current version: ";
			m_szUpdateLogEdit += g_szVersion;
			m_szUpdateLogEdit += L"\t";
			m_szUpdateLogEdit += szLatestVersion[g_enumLang]; //L"\tLatest version: ";
			m_szUpdateLogEdit += m_szVersion;
			m_szUpdateLogEdit += L"\r\n";
			m_szUpdateLogEdit += L"-------------------------------------------------------------\r\n";
			m_szUpdateLogEdit += szUpdateLog[g_enumLang];
			m_szUpdateLogEdit += L"\r\n";
			m_szUpdateLogEdit += m_szUpdateLog;
			
			m_btnUpdate.EnableWindow(TRUE);
		}
	}
	
	UpdateData(FALSE);
 	SendDlgItemMessage(IDC_EDIT_UPDATE_LOG, EM_SETSEL, -1, -1);
  	m_OutputEdit.LineScroll (m_OutputEdit.GetLineCount(), 0);
	return 0;
}

void CUpdateDlg::ProgressUpdate(  ULONG nMaxBytes, ULONG nDoneBytes )
{
	if (nMaxBytes > 0 && nDoneBytes <= nMaxBytes)
	{
		int nPos = (int)(nDoneBytes * 100.0 / nMaxBytes);
		m_Progress.SetPos(nPos);
	}
}

void CUpdateDlg::OnClose()
{
	m_bStopDownload = TRUE;
// 	EnterCriticalSection(&g_DownloadCallback.m_cs);
	g_DownloadCallback.m_bStop = TRUE;
// 	LeaveCriticalSection(&g_DownloadCallback.m_cs);
	
	Sleep(100);

	if (m_hConnectThread)
	{
		WaitForSingleObject(m_hConnectThread, INFINITE);
		CloseHandle(m_hConnectThread);
		m_hConnectThread = NULL;
	}
	
	if (m_hDownloadThread)
	{
		WaitForSingleObject(m_hDownloadThread, INFINITE);
		CloseHandle(m_hDownloadThread);
		m_hDownloadThread = NULL;
	}
	
	CDialog::OnClose();
}
