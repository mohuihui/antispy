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
// ProcessImageInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ProcessImageInfoDlg.h"
#include <windns.h>
#include <Shlwapi.h>

// CProcessImageInfoDlg dialog

IMPLEMENT_DYNAMIC(CProcessImageInfoDlg, CDialog)

CProcessImageInfoDlg::CProcessImageInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessImageInfoDlg::IDD, pParent)
	, m_szFileDescribe(_T(""))
	, m_szFileCompany(_T(""))
	, m_szVersion(_T(""))
	, m_szProcPath(_T(""))
	, m_szFileSize(_T(""))
	, m_szTime(_T(""))
	, m_szCommandLine(_T(""))
	, m_szCurrentDirectory(_T(""))
	, m_szStartedTime(_T(""))
	, m_szPebAddress(_T(""))
	, m_szParent(_T(""))
{
	m_szPath = L"";
	m_dwPid = 0;
}

CProcessImageInfoDlg::~CProcessImageInfoDlg()
{
}

void CProcessImageInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESS_ICON, m_ProcIcon);
	DDX_Text(pDX, IDC_DESCRIPTER_EDIT, m_szFileDescribe);
	DDX_Text(pDX, IDC_COMPANE_EDIT, m_szFileCompany);
	DDX_Text(pDX, IDC_VERSION_EDIT, m_szVersion);
	DDX_Text(pDX, IDC_PATH_EDIT, m_szProcPath);
	DDX_Text(pDX, IDC_SIZE_EDIT, m_szFileSize);
	DDX_Text(pDX, IDC_TIME_EDIT, m_szTime);
	DDX_Text(pDX, IDC_COMMAND_LINE_EDIT, m_szCommandLine);
	DDX_Text(pDX, IDC_CURRENT_DIRECTORY_EDIT, m_szCurrentDirectory);
	DDX_Text(pDX, IDC_STARTED_EDIT, m_szStartedTime);
	DDX_Text(pDX, IDC_PEB_ADDRESS_EDIT, m_szPebAddress);
	DDX_Text(pDX, IDC_PARENT_EDIT, m_szParent);
}


BEGIN_MESSAGE_MAP(CProcessImageInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CProcessImageInfoDlg::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CProcessImageInfoDlg message handlers

void CProcessImageInfoDlg::OnBnClickedOk()
{
}

void CProcessImageInfoDlg::InitString()
{
	m_szFileDescribe = L"";
	m_szFileCompany = L"";
	m_szVersion = L"";
	m_szProcPath = L"";
	m_szFileSize = L"";
	m_szTime = L"";
}

void CProcessImageInfoDlg::FinishString()
{
	if (m_szFileDescribe.IsEmpty())
	{
		m_szFileDescribe = L"N/A";
	}

	if (m_szFileCompany.IsEmpty())
	{
		m_szFileCompany = L"N/A";
	}

	if (m_szVersion.IsEmpty())
	{
		m_szVersion = L"N/A";
	}
	if (m_szProcPath.IsEmpty())
	{
		m_szProcPath = L"N/A";
	}
	if (m_szFileSize.IsEmpty())
	{
		m_szFileSize = L"N/A";
	}

	if (m_szTime.IsEmpty())
	{
		m_szTime = L"N/A";
	}

	if (m_szCommandLine.IsEmpty())
	{
		m_szCommandLine = L"N/A";
	}

	if (m_szCurrentDirectory.IsEmpty())
	{
		m_szCurrentDirectory = L"N/A";
	}

	if (m_szStartedTime.IsEmpty())
	{
		m_szStartedTime = L"N/A";
	}

	if (m_szPebAddress.IsEmpty())
	{
		m_szPebAddress = L"N/A";
	}

	if (m_szParent.IsEmpty())
	{
		m_szParent = L"N/A";
	}
}

void CProcessImageInfoDlg::ShowInformation()
{
	if (m_szPath.IsEmpty())
	{
		return;
	}
	
	InitString();

	if (!PathFileExists(m_szPath))
	{
		(GetDlgItem(IDC_PROCESS_ICON))->ShowWindow(FALSE);
		(GetDlgItem(IDC_STATIC_FILE_NOT_FOUND))->ShowWindow(TRUE);
		(GetDlgItem(IDC_STATIC_FIND_FILE))->ShowWindow(FALSE);
		(GetDlgItem(IDC_STATIC_DELETE_FILE))->ShowWindow(FALSE);
	}
	else
	{
		// image
		SetProcessIcon();
		GetProcessCompanyAndDescribe();
		GetFileVersion();
		GetFilePath();
		GetFileSize();
		GetFileTimes();
	}

	// process
	GetProcessCmdLine();
	GetProcessCurrentDirectory();
	GetPebAddress();
	GetProcessStartTime();

	FinishString();

	UpdateData(FALSE);
}

BOOL CProcessImageInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_STATIC_IMAGE, szImageFile[g_enumLang]);
	SetDlgItemText(IDC_STATIC_PROCESS, szProcess[g_enumLang]);
	SetDlgItemText(IDC_STATIC_FILE_SZIE, szFileSize[g_enumLang]);
	SetDlgItemText(IDC_STATIC_FILE_VERSION, szFileVersion[g_enumLang]);
	SetDlgItemText(IDC_STATIC_DESCRIPT, szFileDescription[g_enumLang]);
	SetDlgItemText(IDC_STATIC_COMPANE, szFileCompany[g_enumLang]);
	SetDlgItemText(IDC_STATIC_FILE_PATH, szFilePath[g_enumLang]);
	SetDlgItemText(IDC_STATIC_TIME, szFileTime[g_enumLang]);
	SetDlgItemText(IDC_STATIC_FILE_NOT_FOUND, szFileNotFound[g_enumLang]);

	SetDlgItemText(IDC_STATIC_COMMAND_LINE, szCommandLine[g_enumLang]);
	SetDlgItemText(IDC_STATIC_CURRENT_DIRECTORY, szCurrentDirectory[g_enumLang]);
	SetDlgItemText(IDC_STATIC_START_TIME, szStartTime[g_enumLang]);
	SetDlgItemText(IDC_STATIC_PEB, szPEB[g_enumLang]);
	SetDlgItemText(IDC_STATIC_PARENT, szParent[g_enumLang]);

	SetDlgItemText(IDC_STATIC_FIND_FILE, szFileFind[g_enumLang]);
	SetDlgItemText(IDC_STATIC_DELETE_FILE, szFileDelete[g_enumLang]);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessImageInfoDlg::SetProcessIcon()
{
	SHFILEINFO shInfo;
	memset(&shInfo, 0, sizeof(shInfo));
	SHGetFileInfo(m_szPath, FILE_ATTRIBUTE_NORMAL, &shInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
	m_ProcIcon.SetIcon(shInfo.hIcon);
}

void CProcessImageInfoDlg::GetProcessCompanyAndDescribe()
{
	m_szFileCompany = m_Functions.GetFileCompanyName(m_szPath);
	m_szFileDescribe = m_Functions.GetFileDescription(m_szPath);
}

void CProcessImageInfoDlg::GetFileVersion()
{	
	TCHAR szVersionBuffer[8192] = _T("");   
	DWORD dwVerSize = 0;   
	DWORD dwHandle = 0;   
	QWORD dwVersion = 0;

	dwVerSize = GetFileVersionInfoSize(m_szPath, &dwHandle);   

	if (dwVerSize)
	{
		if (GetFileVersionInfo(m_szPath, dwHandle, dwVerSize, szVersionBuffer))   
		{   
			VS_FIXEDFILEINFO * pInfo;   
			unsigned int nInfoLen;   
			if (VerQueryValue(szVersionBuffer, _T("\\"), (void**)&pInfo, &nInfoLen))
			{	
				dwVersion = pInfo->dwFileVersionMS;
				dwVersion = dwVersion << 32;
				dwVersion |= pInfo->dwFileVersionLS;
			}   
		}  
	} 	

	if (dwVersion)
	{
		DWORD dwV1,dwV2,dwV3,dwV4;
		QWORD dwStackVersion = dwVersion;

		dwV1 = (DWORD)(dwStackVersion & 0xffff);
		dwStackVersion >>= 16;
		dwV2 = (DWORD)(dwStackVersion & 0xffff);
		dwStackVersion >>= 16;
		dwV3 = (DWORD)(dwStackVersion & 0xffff);
		dwStackVersion >>= 16;
		dwV4 = (DWORD)(dwStackVersion & 0xffff);

		m_szVersion.Format(L"%d.%d.%d.%d", dwV4, dwV3, dwV2, dwV1);
	}
}

void CProcessImageInfoDlg::GetFilePath()
{
	m_szProcPath = m_szPath;
}

void CProcessImageInfoDlg::GetFileSize()
{
	HANDLE hFile = CreateFile(
		m_szPath, 
		GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx( hFile, &FileSize))
		{
			m_szFileSize.Format(L"%d KB", FileSize.QuadPart / 1024);
		}
		CloseHandle(hFile);
	}
}

void CProcessImageInfoDlg::GetFileTimes()
{
	CFileStatus Status;
	if(CFile::GetStatus(m_szPath, Status))
	{
		CTime ctTime = Status.m_ctime;
		CTime mdTime = Status.m_mtime;
		CTime acTime = Status.m_atime;

		CString StrCreateTime;
		StrCreateTime.Format(
			L"%04d/%02d/%02d  %02d:%02d:%02d(Creation)", 
			ctTime.GetYear(),
			ctTime.GetMonth(),
			ctTime.GetDay(),
			ctTime.GetHour(),
			ctTime.GetMinute(),
			ctTime.GetSecond());

		CString szMdTime;
		szMdTime.Format(
			L"%04d/%02d/%02d  %02d:%02d:%02d(Modification)", 
			mdTime.GetYear(),
			mdTime.GetMonth(),
			mdTime.GetDay(),
			mdTime.GetHour(),
			mdTime.GetMinute(),
			mdTime.GetSecond());

		CString szAcTime;
		szAcTime.Format(
			L"%04d/%02d/%02d  %02d:%02d:%02d(Access)", 
			acTime.GetYear(),
			acTime.GetMonth(),
			acTime.GetDay(),
			acTime.GetHour(),
			acTime.GetMinute(),
			acTime.GetSecond());

		m_szTime += StrCreateTime;
		m_szTime += L"\t";
		m_szTime += szMdTime;
		m_szTime += L"\t";
		m_szTime += szAcTime;
	} 
}

BOOL CProcessImageInfoDlg::GetProcessCmdLine()
{
	if (NtQueryInformationProcess == NULL)
	{
		return FALSE;
	}

	LONG                      status;
	HANDLE                    hProcess;
	PROCESS_BASIC_INFORMATION pbi;
	PEB                       Peb;
	PROCESS_PARAMETERS        ProcParam;
	DWORD                     dwDummy;
	DWORD                     dwSize;
	LPVOID                    lpAddress;
	BOOL                      bRet = FALSE;
	PVOID					  pCmdBuffer = NULL;

	// Get process handle
	hProcess = m_ProcessFuncs.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_dwPid);
	if (!hProcess)
		return FALSE;

	// Retrieve information
	status = NtQueryInformationProcess( hProcess,
		ProcessBasicInformation,
		(PVOID)&pbi,
		sizeof(PROCESS_BASIC_INFORMATION),
		NULL
		);

	if (status)
		goto cleanup;

	if (!ReadProcessMemory( hProcess,
		pbi.PebBaseAddress,
		&Peb,
		sizeof(PEB),
		&dwDummy
		)
		)
		goto cleanup;

	if (!ReadProcessMemory( hProcess,
		Peb.ProcessParameters,
		&ProcParam,
		sizeof(PROCESS_PARAMETERS),
		&dwDummy
		)
		)
		goto cleanup;

	lpAddress = ProcParam.CommandLine.Buffer;
	dwSize = ProcParam.CommandLine.Length;
	
	pCmdBuffer = malloc(dwSize + sizeof(WCHAR));
	if (!pCmdBuffer)
		goto cleanup;

	memset(pCmdBuffer, 0, dwSize + sizeof(WCHAR));
	if (!ReadProcessMemory( hProcess,
		lpAddress,
		pCmdBuffer,
		dwSize,
		&dwDummy
		)
		)
		goto cleanup;

	bRet = TRUE;
	m_szCommandLine = (WCHAR*)pCmdBuffer;

cleanup:

	CloseHandle (hProcess);
	if (pCmdBuffer)
	{
		free(pCmdBuffer);
		pCmdBuffer = NULL;
	}

	return bRet;
} 

BOOL CProcessImageInfoDlg::GetProcessCurrentDirectory()
{
	if (NtQueryInformationProcess == NULL)
	{
		return FALSE;
	}

	LONG                      status;
	HANDLE                    hProcess;
	PROCESS_BASIC_INFORMATION pbi;
	PEB                       Peb;
	PROCESS_PARAMETERS        ProcParam;
	DWORD                     dwDummy;
	DWORD                     dwSize;
	LPVOID                    lpAddress;
	BOOL                      bRet = FALSE;
	PVOID					  pCmdBuffer = NULL;

	// Get process handle
	hProcess = m_ProcessFuncs.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_dwPid);
	if (!hProcess)
		return FALSE;

	// Retrieve information
	status = NtQueryInformationProcess( hProcess,
		ProcessBasicInformation,
		(PVOID)&pbi,
		sizeof(PROCESS_BASIC_INFORMATION),
		NULL
		);

	if (status)
		goto cleanup;

	if (!ReadProcessMemory( hProcess,
		pbi.PebBaseAddress,
		&Peb,
		sizeof(PEB),
		&dwDummy
		)
		)
		goto cleanup;

	if (!ReadProcessMemory( hProcess,
		Peb.ProcessParameters,
		&ProcParam,
		sizeof(PROCESS_PARAMETERS),
		&dwDummy
		)
		)
		goto cleanup;

	lpAddress = ProcParam.CurrentDirectory.Buffer;
	dwSize = ProcParam.CurrentDirectory.Length;

	pCmdBuffer = malloc(dwSize + sizeof(WCHAR));
	if (!pCmdBuffer)
		goto cleanup;

	memset(pCmdBuffer, 0, dwSize + sizeof(WCHAR));
	if (!ReadProcessMemory( hProcess,
		lpAddress,
		pCmdBuffer,
		dwSize,
		&dwDummy
		)
		)
		goto cleanup;

	bRet = TRUE;
	m_szCurrentDirectory = (WCHAR*)pCmdBuffer;

cleanup:

	CloseHandle (hProcess);
	if (pCmdBuffer)
	{
		free(pCmdBuffer);
		pCmdBuffer = NULL;
	}

	return bRet;
} 

BOOL CProcessImageInfoDlg::GetPebAddress()
{
	if (NtQueryInformationProcess == NULL)
	{
		return FALSE;
	}

	LONG                      status;
	HANDLE                    hProcess;
	PROCESS_BASIC_INFORMATION pbi;
	BOOL                      bRet = FALSE;

	// Get process handle
	hProcess = m_ProcessFuncs.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_dwPid);
	if (!hProcess)
		return FALSE;

	// Retrieve information
	status = NtQueryInformationProcess( hProcess,
		ProcessBasicInformation,
		(PVOID)&pbi,
		sizeof(PROCESS_BASIC_INFORMATION),
		NULL
		);

	if (status)
		goto cleanup;
	
	bRet = TRUE;
	m_szPebAddress.Format(L"0x%08X", pbi.PebBaseAddress);

cleanup:

	CloseHandle (hProcess);

	return bRet;
} 

void CProcessImageInfoDlg::GetProcessStartTime()
{
	HANDLE hProcess = m_ProcessFuncs.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_dwPid);
	if (!hProcess)
		return;

	FILETIME ftCt, a, b, c;
	if (GetProcessTimes( hProcess, &ftCt, &a, &b, &c))
	{
		FILETIME ftCtLocal;
		if (FileTimeToLocalFileTime(&ftCt, &ftCtLocal))
		{
			SYSTEMTIME SystemTime;
			if (FileTimeToSystemTime(&ftCtLocal, &SystemTime))
			{
				m_szStartedTime.Format(
					L"%04d/%02d/%02d  %02d:%02d:%02d", 
					SystemTime.wYear,
					SystemTime.wMonth,
					SystemTime.wDay,
					SystemTime.wHour,
					SystemTime.wMinute,
					SystemTime.wSecond);
			}
		}
	}
}

HBRUSH CProcessImageInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_FIND_FILE ||
		pWnd->GetDlgCtrlID() == IDC_STATIC_DELETE_FILE)
	{
		pDC->SetTextColor(RGB(0, 0, 255));
	}

	return hbr;
}


void CProcessImageInfoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	//先得到当前鼠标坐标
	CPoint PointEmail; 
	GetCursorPos(&PointEmail); 
	CPoint PointSina; 
	GetCursorPos(&PointSina); 

	//然后得到static控件rect。
	CRect rectEmail; 
	::GetClientRect(GetDlgItem(IDC_STATIC_FIND_FILE)->GetSafeHwnd(), &rectEmail); 
	CRect rectSina; 
	::GetClientRect(GetDlgItem(IDC_STATIC_DELETE_FILE)->GetSafeHwnd(), &rectSina); 

	//然后把当前鼠标坐标转为相对于rect的坐标。 
	::ScreenToClient(GetDlgItem(IDC_STATIC_FIND_FILE)->GetSafeHwnd(), &PointEmail); 
	::ScreenToClient(GetDlgItem(IDC_STATIC_DELETE_FILE)->GetSafeHwnd(), &PointSina); 

	if(rectEmail.PtInRect(PointEmail) ||
		rectSina.PtInRect(PointSina) ) 
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
	} 

	CDialog::OnMouseMove(nFlags, point);
}

void CProcessImageInfoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	//先得到当前鼠标坐标
	CPoint PointEmail; 
	GetCursorPos(&PointEmail); 
	CPoint PointSina; 
	GetCursorPos(&PointSina); 

	//然后得到static控件rect。
	CRect rectEmail; 
	::GetClientRect(GetDlgItem(IDC_STATIC_FIND_FILE)->GetSafeHwnd(), &rectEmail); 
	CRect rectSina; 
	::GetClientRect(GetDlgItem(IDC_STATIC_DELETE_FILE)->GetSafeHwnd(), &rectSina); 

	//然后把当前鼠标坐标转为相对于rect的坐标。 
	::ScreenToClient(GetDlgItem(IDC_STATIC_FIND_FILE)->GetSafeHwnd(), &PointEmail); 
	::ScreenToClient(GetDlgItem(IDC_STATIC_DELETE_FILE)->GetSafeHwnd(), &PointSina); 

	if (rectEmail.PtInRect(PointEmail))
	{
		m_Functions.LocationExplorer(m_szPath);
	}
	else if (rectSina.PtInRect(PointSina))
	{
		if (MessageBox(szAreYouSureDeleteValue[g_enumLang], L"AntiSpy", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			BOOL bRet = m_Functions.KernelDeleteFile(m_szPath);
			if (bRet)
			{
				MessageBox(szDeleteFileSucess[g_enumLang], NULL, MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(szDeleteFileFailed[g_enumLang], NULL, MB_OK | MB_ICONERROR);
			}
		}
	}

	CDialog::OnLButtonDown(nFlags, point);
}
