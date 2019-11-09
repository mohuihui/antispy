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
// AGUpdateDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AGUpdate.h"
#include "AGUpdateDlg.h"
#include "Utils\MusicBoxConst.h"
#include "unzip.h"
#include "DownloadCallback.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL g_bAllEnd = FALSE;

COleDateTime CAGUpdateDlg::m_StartDownTime;
CNEProgressCtrl CAGUpdateDlg::m_DownloadProgress;
CNEProgressCtrl CAGUpdateDlg::m_UpdateProgress;
CNELabel CAGUpdateDlg::m_FileSizeLabel;
CNELabel CAGUpdateDlg::m_DownloadSpeedLabel;
CNELabel CAGUpdateDlg::m_FileCountLabel;
CNELabel CAGUpdateDlg::m_RemainTimeLabel;
CNELabel CAGUpdateDlg::m_DownloadProgressLabel;
CNELabel CAGUpdateDlg::m_UpdateProgressLabel;
CCriticalSection CAGUpdateDlg::m_cs;
CString CAGUpdateDlg::m_szFileSize = _T("0M/0M");;
CString CAGUpdateDlg::m_szDwonSpeed = _T("0.000KB/S");
CString CAGUpdateDlg::m_szFileCount = _T("1/1");
CString CAGUpdateDlg::m_szRemainTime = _T("00:00:00");
CString CAGUpdateDlg::m_szUpdateProgress = _T("0%");
CString CAGUpdateDlg::m_szDownloadProgress = _T("0%");
int CAGUpdateDlg::m_nPos = 0;
BOOL CAGUpdateDlg::m_bExit = FALSE;

// CAGUpdateDlg 对话框

CAGUpdateDlg::CAGUpdateDlg(CWnd* pParent /*=NULL*/)
	: CAbstractMusicEventDialog(CAGUpdateDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetLayoutDialogID(LAYOUT_DIALOG_MAIN);
	m_szDownloadURL = _T("");
	m_szDownloadDir = _T("");
	m_szSaveFileName = _T("");

}

void CAGUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CAbstractMusicEventDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_MAIN_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BUTTON_MAIN_MIN, m_btnMin);
	DDX_Control(pDX, IDC_STATIC_PATCH_UPDATE_PROCESS , m_DownloadProgress);
	DDX_Control(pDX, IDC_STATIC_PATCH_DOWNLOAD_PROCESS, m_UpdateProgress);
	DDX_Control(pDX, IDC_STATIC_FILE_SIZE, m_FileSizeLabel);
	DDX_Control(pDX, IDC_STATIC_DOWNLOAD_SPEED, m_DownloadSpeedLabel);
	DDX_Control(pDX, IDC_STATIC_FILE_COUNT, m_FileCountLabel);
	DDX_Control(pDX, IDC_STATIC_REMAIN_TIME, m_RemainTimeLabel);
	DDX_Control(pDX, IDC_STATIC_UPDATE_PROGRESS, m_DownloadProgressLabel);
	DDX_Control(pDX, IDC_STATIC_DOWNLOAD_PROGRESS_LABEL, m_UpdateProgressLabel);
}

BEGIN_MESSAGE_MAP(CAGUpdateDlg, CAbstractMusicEventDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_MAIN_MIN, &CAGUpdateDlg::OnBnClickedButtonMainMin)
	ON_BN_CLICKED(IDC_BUTTON_MAIN_CLOSE, &CAGUpdateDlg::OnBnClickedButtonMainClose)
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CAGUpdateDlg 消息处理程序

BOOL CAGUpdateDlg::OnInitDialog()
{
	CAbstractMusicEventDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	m_DownloadProgress.SetRange(0,100);
	m_DownloadProgress.SetPos(0);
	m_UpdateProgress.SetRange(0,100);
	m_UpdateProgress.SetPos(0);

	SetProp(m_hWnd, CMusicBoxConst::APP_PropName, CMusicBoxConst::APP_PropValue);

	DownloadFile();


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAGUpdateDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect clientRect;
	GetClientRect(&clientRect);
	CNEMemDC memdc(&dc, &clientRect, FALSE);

	// 先绘制背景
	DrawBackground(&memdc, clientRect);

	// 绘制Label
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_FILE_SIZE);
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_DOWN_SPEED);
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_FILE_COUNT);
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_REMAIN_TIME);
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_PATCH_UPDATE);
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_PATHC_DOWNLOAD);
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_LAST_VERSION);
	DrawLabel(&memdc, CMusicBoxConst::UPDATE_LAST_UPDATE_TIME);
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CAGUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAGUpdateDlg::LayoutComponents(BOOL firstInit, BOOL reloadStyle)
{
	LayoutButton(m_btnClose, CMusicBoxConst::ELEM_BUTTON_CLOSE, firstInit, reloadStyle);
	LayoutButton(m_btnMin, CMusicBoxConst::ELEM_BUTTON_MIN, firstInit, reloadStyle);
	LayoutProgressComponent(m_DownloadProgress, CMusicBoxConst::UPDATE_PATCH_UPDATE_PROGRESS, firstInit, reloadStyle);
	LayoutProgressComponent(m_UpdateProgress, CMusicBoxConst::UPDATE_PATCH_DOWNLOAD_PROGRESS, firstInit, reloadStyle);
	LayoutLabel(m_FileSizeLabel, CMusicBoxConst::UPDATE_FILE_SIZE_LABEL, firstInit, reloadStyle);
	LayoutLabel(m_DownloadSpeedLabel, CMusicBoxConst::UPDATE_DOWNLOAD_SPEED_LABEL, firstInit, reloadStyle);
	LayoutLabel(m_FileCountLabel, CMusicBoxConst::UPDATE_FILE_COUNT_LABEL, firstInit, reloadStyle);
	LayoutLabel(m_RemainTimeLabel, CMusicBoxConst::UPDATE_REMIAN_TIME_LABEL, firstInit, reloadStyle);
	LayoutLabel(m_DownloadProgressLabel, CMusicBoxConst::UPDATE_UPDATE_PROCESS_LABEL, firstInit, reloadStyle);
	LayoutLabel(m_UpdateProgressLabel, CMusicBoxConst::UPDATE_DOWNLOAD_PROCESS_LABEL, firstInit, reloadStyle);
}

DWORD CAGUpdateDlg::StartUnpackProc(const mystring& szDir, const mystring& szZipFullName)
{

	SetCurrentDirectory(szDir.c_str());

	HZIP hz = OpenZip(szZipFullName.c_str(), 0);
	if (hz)
	{
		ZIPENTRY ze; 
		GetZipItem(hz, -1, &ze); 
		int numitems = ze.index;
		m_UpdateProgress.SetRange(0, numitems);
		double persent = 100.0 / numitems;

		for (int i = 0; i < numitems; i++)
		{ 
			GetZipItem(hz, i, &ze);
			UnzipItem(hz, i, ze.name);
			m_UpdateProgress.SetPos(i+1);

			CString szText;
			szText.Format(_T("%d%s"), (int)(persent * (i + 1)), _T("%"));
			m_UpdateProgressLabel.SetText(szText);
		
			Sleep(100);
		}
	}

	CloseZip(hz);
	return 0;
}

DWORD WINAPI CAGUpdateDlg::SetLabelTextProc(LPVOID lpParameter)
{
	BOOL bExit = FALSE;

	while (!bExit && !m_bExit)
	{
		m_cs.Lock();
		
		if (!m_szUpdateProgress.CompareNoCase(_T("100%")) && m_szDwonSpeed.CompareNoCase(_T("0.000KB/S")))
		{
			bExit = TRUE;
			m_szDwonSpeed = _T("0.000KB/S");
		}

		m_DownloadProgressLabel.SetText(m_szUpdateProgress);
		m_FileSizeLabel.SetText(m_szFileSize);
		m_DownloadSpeedLabel.SetText(m_szDwonSpeed);
		m_RemainTimeLabel.SetText(m_szRemainTime);
		m_DownloadProgress.SetPos(m_nPos);

		m_cs.Unlock();
		//Sleep(1000);
	}

	CAGUpdateDlg *pAGUDlg = (CAGUpdateDlg *)lpParameter;

	if (pAGUDlg->m_szSaveFileName.find_first_of(_T(".zip")) != mystring::npos)
	{
		pAGUDlg->StartUnpackProc(pAGUDlg->m_szDownloadDir,pAGUDlg->m_szSaveFileName);
		DeleteFile(pAGUDlg->m_szSaveFileName.c_str());
	}

	g_bAllEnd = TRUE;
	pAGUDlg->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
	return 0;
}

void CAGUpdateDlg::OnBnClickedButtonMainMin()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void CAGUpdateDlg::OnBnClickedButtonMainClose()
{
	m_bExit = TRUE;
	SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
}

void CAGUpdateDlg::OnSize(UINT nType, int cx, int cy)
{
	CAbstractMusicEventDialog::OnSize(nType, cx, cy);

}

void CAGUpdateDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_CLOSE)
	{
		m_bExit = TRUE;

	}

	CAbstractMusicEventDialog::OnSysCommand(nID, lParam);

}

void CAGUpdateDlg::ProgressUpdate(  ULONG nMaxBytes, ULONG nDoneBytes )
{
	TracklogU(_T("Proc:%d  Max:%x  Done:%x"),(int)(100.0 * nDoneBytes / nMaxBytes),nMaxBytes,nDoneBytes);
	if (nMaxBytes > 0 && nDoneBytes <= nMaxBytes)
	{
		m_cs.Lock();

		m_szUpdateProgress.Format(_T("%d%s"), (int)(100.0 * nDoneBytes / nMaxBytes), _T("%")); // 格式化下载进度百分比
		m_szFileSize.Format(_T("%.2lfM/%.2lfM"), nDoneBytes*1.0/1024/1024, nMaxBytes*1.0/1024/1024); // 格式化文件大小
		m_nPos = (int)(nDoneBytes * 100.0 / nMaxBytes);								// 设置进度条

		COleDateTimeSpan dlElapsed = COleDateTime::GetCurrentTime() - m_StartDownTime;
		double nTimeSpan = dlElapsed.GetTotalSeconds() > 0.0 ? dlElapsed.GetTotalSeconds() : 1.0;
		double dbKbSec = nDoneBytes / 1024 / nTimeSpan;
		m_szDwonSpeed.Format(_T("%dKB/S"),(int)dbKbSec);				 // 格式化下载速度（KB/秒）

		dbKbSec = dbKbSec > 0.0 ? dbKbSec : 1.0;
		int nRemainTime = (int)((nMaxBytes - nDoneBytes) / dbKbSec / 1024);
		int nSecond = nRemainTime % 60;
		int nMinute = nRemainTime / 60 % 60;
		int nHour = nRemainTime / 60 / 60;
		m_szRemainTime.Format(_T("%02d:%02d:%02d"), nHour, nMinute,nSecond ); // 格式化剩余下载时间

		m_cs.Unlock();
	}
}

#define  DOWNLOAD_BUFFER_SIZE	0x4000

int CAGUpdateDlg::InternetGetFile (const mystring& szUrl,const mystring& szFileName)
{
	DWORD dwSizeRead = 0;   
	TCHAR strAgent[] = _T("Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)");
	HINTERNET hOpen = NULL;
	hOpen = InternetOpen(strAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (hOpen)
	{		  
		TCHAR szHead[] =_T("Accept: text/html, application/xhtml+xml, */*");		
		VOID* szTemp[DOWNLOAD_BUFFER_SIZE];
		HINTERNET  hConnect = NULL;
		CFile file;   
		if ( hConnect = InternetOpenUrl ( hOpen, szUrl.c_str(), szHead,_tcslen (szHead), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0))
		{        
			DWORD dwByteToRead = 0;
			DWORD dwSizeOfRq = 4;
			DWORD dwBytes = 0;
			if (HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
				(LPVOID)&dwByteToRead, &dwSizeOfRq, NULL))
			{
				if (dwByteToRead > 0)
				{
					if  (file.Open(szFileName.c_str(),CFile::modeWrite|CFile::modeCreate) )
					{
						while(InternetReadFile (hConnect, szTemp, DOWNLOAD_BUFFER_SIZE,  &dwSizeRead))
						{
							//if (!InternetReadFile (hConnect, szTemp, DOWNLOAD_BUFFER_SIZE,  &dwSizeRead))
							//{
							//	file.Close();
							//	return -1;
							//}
							//InternetReadFile (hConnect, szTemp, DOWNLOAD_BUFFER_SIZE,  &dwSize);
							if (dwSizeRead == 0)
								break;
							else
								file.Write(szTemp,dwSizeRead);
							dwBytes += dwSizeRead;             

							ProgressUpdate(dwByteToRead,dwBytes);
						};
						file.Close();
					}
				}
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hOpen);   //关闭句柄
	}

	return 0;
}

DWORD WINAPI CAGUpdateDlg::DownloadProc(LPVOID lpParameter)
{
	
	CDownloadCallback callback;
	HRESULT hr = E_FAIL;

	callback.m_pDlg = (CAGUpdateDlg *)lpParameter;
	m_StartDownTime = COleDateTime::GetCurrentTime();

	if (((CAGUpdateDlg *)lpParameter)->m_szSaveFileName.length() > 0 && 
		((CAGUpdateDlg *)lpParameter)->m_szDownloadURL.length() > 0)
	{
		//hr = URLDownloadToFile ( 
		//	NULL, 
		//	((CAGUpdateDlg *)lpParameter)->m_szDownloadURL.c_str(), 
		//	((CAGUpdateDlg *)lpParameter)->m_szSaveFileName.c_str(), 
		//	0, 
		//	NULL  
		//	);
		((CAGUpdateDlg *)lpParameter)->InternetGetFile(((CAGUpdateDlg *)lpParameter)->m_szDownloadURL, ((CAGUpdateDlg *)lpParameter)->m_szSaveFileName);
	}

	if ( !SUCCEEDED(hr) )
	{
		LPTSTR lpszErrorMessage;
		CString sMsg;

		if ( FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, hr, 
			MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			(LPTSTR) &lpszErrorMessage, 0, NULL ))
		{
			sMsg.Format ( _T("Download failed.  Error = 0x%08lX\n\n%s"), (DWORD) hr, lpszErrorMessage );
			LocalFree ( lpszErrorMessage );
		}
		else
		{
			sMsg.Format ( _T("Download failed.  Error = 0x%08lX\n\nNo message available."), (DWORD) hr );
		}
		m_bExit = TRUE;		
	}

	return 0;
}

BOOL CAGUpdateDlg::DownloadFile()
{
	BOOL bDownloaded = FALSE;
	DWORD dwThread = 0;
	if (m_szDownloadURL.length() > 0 && m_szSaveFileName.length() > 0)
	{
		HANDLE hThreadDownloadProc = CreateThread(NULL, 0, DownloadProc, this, 0, &dwThread);


		HANDLE hThreadSetLabelTextProc = CreateThread(NULL, 0, SetLabelTextProc, this, 0, &dwThread);

		if (hThreadDownloadProc)
		{
			CloseHandle(hThreadDownloadProc);
		}	
		if (hThreadSetLabelTextProc)
		{
			CloseHandle(hThreadSetLabelTextProc);
		}
	}
	return bDownloaded;
}




BOOL CAGUpdateDlg::SetDownloadInfo(const mystring& szDownloadURL, const mystring& szDownloadDir)
{
	m_szDownloadURL = szDownloadURL;
	m_szDownloadDir = szDownloadDir;
	int nPos = szDownloadURL.rfind(_T('='));
	if (nPos != string::npos)
	{
		m_szSaveFileName = szDownloadDir + szDownloadURL.substr(++nPos);
	}
	else
	{
		nPos = szDownloadURL.rfind(_T('/'));
		if (nPos != string::npos)
		{
			m_szSaveFileName = szDownloadDir + szDownloadURL.substr(++nPos);
		}
	}
	return TRUE;
}

