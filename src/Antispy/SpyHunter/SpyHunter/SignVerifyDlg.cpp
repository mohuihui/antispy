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
// SignVerifyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "SignVerifyDlg.h"
#include <winsvc.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <Mscat.h>
#include <shlwapi.h>
#include <process.h>
#include "Function.h"

#pragma comment(lib, "WinTrust.lib")

BOOL g_bExit = FALSE;

// CSignVerifyDlg 对话框

IMPLEMENT_DYNAMIC(CSignVerifyDlg, CDialog)

CSignVerifyDlg::CSignVerifyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSignVerifyDlg::IDD, pParent)
	, m_path(_T(""))
{
	m_pList = NULL;
	m_bSingle = TRUE;
	m_nPathSubItem = 0;
	m_NotSignDataList.clear();
	m_NotSignItemList.clear();
}

CSignVerifyDlg::~CSignVerifyDlg()
{
	m_NotSignItemList.clear();
	m_NotSignDataList.clear();
}

void CSignVerifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSignVerifyDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSignVerifyDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON, &CSignVerifyDlg::OnBnClickedButton)
	ON_MESSAGE(WM_VERIFY_SIGN_OVER, SiginVerifyOver)
END_MESSAGE_MAP()


// CSignVerifyDlg 消息处理程序

void CSignVerifyDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	return;
}

//
// 真正验证签名的函数
//
BOOL CheckFileTrust(LPCWSTR lpFileName, HANDLE hFile)
{
	BOOL bRet = FALSE;
	WINTRUST_DATA wd = { 0 };
	WINTRUST_FILE_INFO wfi = { 0 };
	WINTRUST_CATALOG_INFO wci = { 0 };
	CATALOG_INFO ci = { 0 };
	HCATADMIN hCatAdmin = NULL;

	if (!lpFileName || hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		return bRet;
	}

	if (!CryptCATAdminAcquireContext(&hCatAdmin, NULL, 0))
	{
		return bRet;
	}

	DWORD dwCnt = 100;
	BYTE byHash[100];
	if (CryptCATAdminCalcHashFromFileHandle(hFile, &dwCnt, byHash, 0))
	{
		LPWSTR pszMemberTag = new WCHAR[dwCnt * 2 + 1];
		if (pszMemberTag)
		{
			for (DWORD dw = 0; dw < dwCnt; dw++)
			{
				wsprintfW(&pszMemberTag[dw * 2], L"%02X", byHash[dw]);
			}

			HCATINFO hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, byHash, dwCnt, 0, NULL);
			if (NULL == hCatInfo)
			{
				wfi.cbStruct       = sizeof( WINTRUST_FILE_INFO );
				
				wfi.pcwszFilePath  = lpFileName;

			//	wfi.pcwszFilePath  = NULL;
			//	wfi.hFile          = hFile;
				wfi.hFile  = NULL;
				wfi.pgKnownSubject = NULL;

				wd.cbStruct            = sizeof( WINTRUST_DATA );
				wd.dwUnionChoice       = WTD_CHOICE_FILE;
				wd.pFile               = &wfi;
				wd.dwUIChoice          = WTD_UI_NONE;
				wd.fdwRevocationChecks = WTD_REVOKE_NONE;
				wd.dwStateAction       = WTD_STATEACTION_IGNORE;
				wd.dwProvFlags         = WTD_SAFER_FLAG;
				wd.hWVTStateData       = NULL;
				wd.pwszURLReference    = NULL;
			}
			else
			{
				CryptCATCatalogInfoFromContext(hCatInfo, &ci, 0);
				wci.cbStruct             = sizeof(WINTRUST_CATALOG_INFO);
				wci.pcwszCatalogFilePath = ci.wszCatalogFile;
				wci.pcwszMemberFilePath  = lpFileName;
			//	wci.pcwszMemberFilePath  = NULL;
			//	wci.hMemberFile			 = hFile;
				wci.hMemberFile			 = NULL;

				wci.pcwszMemberTag       = pszMemberTag;

				wd.cbStruct            = sizeof(WINTRUST_DATA);
				wd.dwUnionChoice       = WTD_CHOICE_CATALOG;
				wd.pCatalog            = &wci;
				wd.dwUIChoice          = WTD_UI_NONE;
				wd.fdwRevocationChecks = WTD_STATEACTION_VERIFY;
				wd.dwProvFlags         = 0;
				wd.hWVTStateData       = NULL;
				wd.pwszURLReference    = NULL;
			}

			GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
			HRESULT hr  = WinVerifyTrust(NULL, &action, &wd);
			bRet        = SUCCEEDED( hr );

			if (hCatInfo)
			{
				CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
			}

			delete[] pszMemberTag;
		}
	}

	CryptCATAdminReleaseContext(hCatAdmin, 0);

	return bRet;
} 

VOID VerifyProc(LPVOID lpParameter)
{
	CSignVerifyDlg * dlg = (CSignVerifyDlg*)lpParameter;
	if (!dlg)
	{
		return;
	}

	//
	// 如果是验单个进程
	//
	if (dlg->m_bSingle)
	{
		CString szPath = szVerifyDlgImagePath[g_enumLang] + dlg->m_path;
		::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_PATH), szPath);
		::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_STATUS), szVerifyStatusVerifyNow[g_enumLang]);
		
		WCHAR *szPathTemp = CString2WString(dlg->m_path);
		if (!szPathTemp)
		{
			return;
		}
		
		HANDLE hFile = CreateFile(szPathTemp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if (CheckFileTrust(szPathTemp, hFile))
			{
				if (IsWindow(dlg->m_hWnd))
				{
					::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_STATUS), szVerifyStatusVerifySigned[g_enumLang]);
				}
			}
			else
			{
				if (IsWindow(dlg->m_hWnd))
				{
					::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_STATUS), szVerifyStatusVerifyNotSigned[g_enumLang]);
				}
			}

			CloseHandle(hFile);
		}

		// 打开文件失败
		else
		{
			if (IsWindow(dlg->m_hWnd))
			{
				::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_STATUS), szVerifyStatusVerifyNotSigned[g_enumLang]);
			}
		}

		if (IsWindow(dlg->m_hWnd))
		{
			::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_BUTTON), szOK[g_enumLang]);
		}

		free(szPathTemp);
		szPathTemp = NULL;
	}

	// 验证多个文件
	else
	{
		dlg->m_NotSignItemList.clear();
		dlg->m_NotSignDataList.clear();
		ULONG nCnt = dlg->m_pList->GetItemCount();

		::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_STATUS), szVerifysignatureNowPleaseWait[g_enumLang]);

		for (ULONG i = 0; i < nCnt; i++)
		{
			if (g_bExit)
			{
				_endthread();
			}

			CString szPath = dlg->m_pList->GetItemText(i, dlg->m_nPathSubItem);

			if (szPath != L"Idle" && szPath != L"System")
			{
				if (::IsWindow(dlg->m_hWnd))
				{
					CString szTemp = szVerifyDlgImagePath[g_enumLang] + szPath + L"                                                  ";
					::SetWindowText(::GetDlgItem(dlg->m_hWnd, IDC_PATH), szTemp);
				}

				WCHAR *szPathTemp = CString2WString(szPath);
				if (szPathTemp)
				{
					HANDLE hFile = CreateFile(szPathTemp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						if (!CheckFileTrust(szPathTemp, hFile))
						{
							dlg->m_NotSignItemList.push_back((ULONG)i);
							dlg->m_NotSignDataList.push_back((ULONG)dlg->m_pList->GetItemData(i));
						}

						CloseHandle(hFile);
					}
					else
					{
						dlg->m_NotSignItemList.push_back((ULONG)i);
						dlg->m_NotSignDataList.push_back((ULONG)dlg->m_pList->GetItemData(i));
					}

					free(szPathTemp);
				}
			}
		}

		if (::IsWindow(dlg->m_hWnd))
		{
			SendMessage(dlg->m_hWnd, WM_VERIFY_SIGN_OVER, NULL, NULL);
		}
	}

	_endthread();
}

BOOL CSignVerifyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置窗口标题
	SetWindowText(szVerifySignature[g_enumLang]);

	// 开启验签名的服务
	OpenCryptSvc();

	// 去掉关闭按钮的窗体样式
	SetWindowLong(this->m_hWnd, GWL_STYLE, GetWindowLong(this->m_hWnd, GWL_STYLE) & ~WS_SYSMENU );
	
	g_bExit = FALSE;

	// 设置按钮文本为取消
	::SetWindowText(::GetDlgItem(m_hWnd, IDC_BUTTON), szCancel[g_enumLang]);

	// 开个线程开始验签名
	_beginthread(VerifyProc, 0, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//
// 首先开启下验签名的服务
//
BOOL CSignVerifyDlg::OpenCryptSvc()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	BOOL bRet = FALSE;

	if (hSCManager)
	{
		WCHAR szCryptSvc[] = {'C','r','y','p','t','S','v','c','\0'};
		SC_HANDLE hCryptSvc = OpenService(hSCManager, szCryptSvc, SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_PAUSE_CONTINUE);

		if (hCryptSvc)
		{
			SERVICE_STATUS st;
			if (QueryServiceStatus(hCryptSvc, &st))
			{
				if (st.dwCurrentState == SERVICE_STOPPED)
				{
					StartService(hCryptSvc, NULL, NULL);
				}
				else if (st.dwCurrentState == SERVICE_PAUSED)
				{
					ControlService(hCryptSvc, SERVICE_CONTROL_CONTINUE, &st);
				}
			}

			if (QueryServiceStatus(hCryptSvc, &st))
			{
				if (st.dwCurrentState == SERVICE_RUNNING)
				{
					bRet = TRUE;
				}
			}

			CloseServiceHandle(hCryptSvc);
		}

		CloseServiceHandle(hSCManager);
	}

	return bRet;
}

void CSignVerifyDlg::OnBnClickedButton()
{
	g_bExit = TRUE;
	OnCancel();
}

LRESULT CSignVerifyDlg::SiginVerifyOver(WPARAM wParam, LPARAM lParam)
{
	::SendMessage(m_hWinWnd, WM_VERIFY_SIGN_OVER, NULL, NULL);
	g_bExit = TRUE;
	OnCancel();
	return 0;
}

BOOL CSignVerifyDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}
