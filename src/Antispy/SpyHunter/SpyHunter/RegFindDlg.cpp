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
// RegFindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "RegFindDlg.h"

#define  WM_SEARCH_FINISH   WM_USER + 105
#define  WM_UPDATE_UI		WM_USER + 106

// CRegFindDlg dialog

IMPLEMENT_DYNAMIC(CRegFindDlg, CDialog)

CRegFindDlg::CRegFindDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegFindDlg::IDD, pParent)
	, m_bKeys(TRUE)
	, m_bValues(TRUE)
	, m_bData(TRUE)
	, m_bMachCase(FALSE)
	, m_bMachWholeString(FALSE)
	, m_szFindWhat(_T(""))
	, m_szSearchInKey(_T(""))
	, m_nRadio(0)
	, m_szSearchResults(_T(""))
{
	m_szFindWhatUpper = L"";
	m_nCnt = 0;
	m_bStop = FALSE;
	m_hThread = NULL;
}

CRegFindDlg::~CRegFindDlg()
{
	if (m_hThread)
	{
		CloseHandle(m_hThread);
	}
}

void CRegFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Check(pDX, IDC_CHECK_KEY, m_bKeys);
	DDX_Check(pDX, IDC_CHECK_VALUES, m_bValues);
	DDX_Check(pDX, IDC_CHECK_DATA, m_bData);
	DDX_Check(pDX, IDC_CHECK_MACH_CASE, m_bMachCase);
	DDX_Check(pDX, IDC_CHECK_MACH_WHOLE_STRING, m_bMachWholeString);
	DDX_Text(pDX, IDC_EDIT_FIND_WHAT, m_szFindWhat);
	DDX_Text(pDX, IDC_EDIT2, m_szSearchInKey);
	DDX_Radio(pDX, IDC_RADIO1, m_nRadio);
	DDX_Text(pDX, IDC_STATIC_FIND_RESULT, m_szSearchResults);
}


BEGIN_MESSAGE_MAP(CRegFindDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CRegFindDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CRegFindDlg::OnBnClickedCancel)
	ON_WM_NCDESTROY()
	ON_BN_CLICKED(IDC_BUTTON_START, &CRegFindDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CRegFindDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_CHECK_KEY, &CRegFindDlg::OnBnClickedCheckKey)
	ON_BN_CLICKED(IDC_CHECK_VALUES, &CRegFindDlg::OnBnClickedCheckValues)
	ON_BN_CLICKED(IDC_CHECK_DATA, &CRegFindDlg::OnBnClickedCheckData)
	ON_MESSAGE(WM_SEARCH_FINISH, &CRegFindDlg::SearchFinished)
	ON_MESSAGE(WM_UPDATE_UI, &CRegFindDlg::UpdateUI)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CRegFindDlg::OnNMRclickList)
	ON_COMMAND(ID_AUTORUN_JMP_TO_REG, &CRegFindDlg::OnAutorunJmpToReg)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CRegFindDlg::OnNMDblclkList)
	ON_BN_CLICKED(IDC_RADIO2, &CRegFindDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO1, &CRegFindDlg::OnBnClickedRadio1)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CRegFindDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CRegFindDlg message handlers

BOOL CRegFindDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_CHECK_KEY)->SetWindowText(szSearchKeys[g_enumLang]);
	GetDlgItem(IDC_CHECK_VALUES)->SetWindowText(szSearchValues[g_enumLang]);
	GetDlgItem(IDC_CHECK_DATA)->SetWindowText(szSearchData[g_enumLang]);
	GetDlgItem(IDC_CHECK_MACH_CASE)->SetWindowText(szSearchMachCase[g_enumLang]);
	GetDlgItem(IDC_CHECK_MACH_WHOLE_STRING)->SetWindowText(szSearchMachWholeString[g_enumLang]);
	GetDlgItem(IDC_STATIC_FIND_WHAT)->SetWindowText(szFindWhat[g_enumLang]);
	GetDlgItem(IDC_RADIO1)->SetWindowText(szSearchInKey[g_enumLang]);
	GetDlgItem(IDC_RADIO2)->SetWindowText(szSearchTheEntireRegistry[g_enumLang]);
	GetDlgItem(IDC_STATIC_WHERE_TO_SEARCH)->SetWindowText(szWhereToSearch[g_enumLang]);
	GetDlgItem(IDC_BUTTON_START)->SetWindowText(szRegistryFind[g_enumLang]);
	GetDlgItem(IDC_BUTTON_STOP)->SetWindowText(szRegistryStop[g_enumLang]);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	m_szSearchResults = szFindResult[g_enumLang];
	m_szSearchInKey.IsEmpty() ? m_nRadio = 1 : m_nRadio = 0;
	GetDlgItem(IDC_EDIT2)->EnableWindow(!m_nRadio);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szKey[g_enumLang], LVCFMT_LEFT, 300);
	m_list.InsertColumn(1, szValue[g_enumLang], LVCFMT_LEFT, 80);
	m_list.InsertColumn(2, szData[g_enumLang], LVCFMT_LEFT, 120);
	
	SetWindowText(szRegistryFind[g_enumLang]);
	UpdateData(FALSE);
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRegFindDlg::OnBnClickedOk()
{
}

// 销毁窗口
void CRegFindDlg::OnBnClickedCancel()
{
	DestroyWindow();
}

// 最后删除自身
void CRegFindDlg::OnNcDestroy()
{
	CDialog::OnNcDestroy();
	delete this;
}

void CRegFindDlg::ChangeKongjianStatus(BOOL bFuck)
{
	GetDlgItem(IDC_CHECK_KEY)->EnableWindow(bFuck);
	GetDlgItem(IDC_CHECK_VALUES)->EnableWindow(bFuck);
	GetDlgItem(IDC_CHECK_DATA)->EnableWindow(bFuck);
	GetDlgItem(IDC_CHECK_MACH_CASE)->EnableWindow(bFuck);
	GetDlgItem(IDC_CHECK_MACH_WHOLE_STRING)->EnableWindow(bFuck);
	GetDlgItem(IDC_RADIO1)->EnableWindow(bFuck);
	GetDlgItem(IDC_RADIO2)->EnableWindow(bFuck);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(bFuck);
	GetDlgItem(IDC_EDIT_FIND_WHAT)->EnableWindow(bFuck);
	GetDlgItem(IDC_EDIT2)->EnableWindow(bFuck);
	GetDlgItem(IDC_EDIT_FIND_WHAT)->EnableWindow(bFuck);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(!bFuck);
}

DWORD WINAPI SearchRegistryProc(PVOID pParam)
{
	if (pParam)
	{
		CRegFindDlg *pDlg = (CRegFindDlg*)pParam;
		pDlg->SearchRegistry();
	}

	return 0;
}

void CRegFindDlg::InsertKeys(HKEY hRoot, CString szSubKey, CString szSubSubKey)
{
	if (!hRoot)
	{
		return;
	}

	CString szKeyPath = GetRootKeyString(hRoot);
	szKeyPath += L"\\" + szSubKey + L"\\" + szSubSubKey;
	int nItem = m_list.InsertItem(m_list.GetItemCount(), szKeyPath);
	m_list.SetItemText(nItem, 1, L"");
	m_list.SetItemText(nItem, 2, L"");
	m_nCnt++;
}

void CRegFindDlg::InsertVlaues(HKEY hRoot, CString szSubKey, CString szValue)
{
	if (!hRoot || szValue.IsEmpty())
	{
		return;
	}

	CString szKeyPath = GetRootKeyString(hRoot);
	szKeyPath += L"\\" + szSubKey;
	int nItem = m_list.InsertItem(m_list.GetItemCount(), szKeyPath);
	m_list.SetItemText(nItem, 1, szValue);
	m_list.SetItemText(nItem, 2, L"");
	m_nCnt++;
}

void CRegFindDlg::InsertData(HKEY hRoot, CString szSubKey, CString szValue, DWORD dwType, PBYTE pData, DWORD dwDataLen)
{
	if (!hRoot || !pData || dwDataLen <= 0)
	{
		return;
	}
	
	CString szRet, szCmp;
	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		szCmp = szRet = (WCHAR*)pData;
		break;

	case REG_DWORD:
		{
			szCmp.Format(L"%d", *(PULONG)pData);
			if (m_szFindWhat.GetLength() == szCmp.GetLength())
			{
				BOOL bNumber = TRUE;
				for (int i = 0; i < m_szFindWhat.GetLength(); i++)
				{
					WCHAR ch = m_szFindWhat.GetAt(i);
					if (ch < '0' || ch > '9')
					{
						bNumber = FALSE;
						break;
					}
				}

				if (bNumber && *(PULONG)pData == _wtoi(m_szFindWhat))
				{
					szRet.Format(L"0x%08X (%d)", *(PULONG)pData, *(PULONG)pData);
				}
				else
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
		break;

	case REG_DWORD_BIG_ENDIAN:
		{
			BYTE Value[4] = {0};
			Value[0] = *((PBYTE)pData + 3);
			Value[1] = *((PBYTE)pData + 2);
			Value[2] = *((PBYTE)pData + 1);
			Value[3] = *((PBYTE)pData + 0);
			szCmp.Format(L"%d", *(PULONG)Value);
			szRet.Format(L"0x%08X (%d)", *(PULONG)Value, *(PULONG)Value);
		}
		break;

	case REG_MULTI_SZ:
		{
			DWORD len = 0;
			while (wcslen((WCHAR*)pData + len))
			{
				szRet += ((WCHAR*)pData + len);
				szRet += L" ";
				len += wcslen((WCHAR*)pData + len) + 1;
			}

			szCmp = szRet;
		}
		break;

	default:
		return;
	}

	if (szCmp.IsEmpty())
	{
		return;
	}

	CString strSubName = szCmp;
	BOOL bInsert = FALSE;

	// 区分大小写 & 匹配全字符
	if (m_bMachCase && m_bMachWholeString)
	{
		if (!strSubName.Compare(m_szFindWhat))
		{
			bInsert = TRUE;
		}
	}

	// 区分大小写,但是不全字匹配
	else if (m_bMachCase && !m_bMachWholeString)
	{
		if (-1 != strSubName.Find(m_szFindWhat))
		{
			bInsert = TRUE;
		}
	}	

	// 不区分大小写 & 全字匹配
	else if (!m_bMachCase && m_bMachWholeString)
	{
		if (!strSubName.CompareNoCase(m_szFindWhat))
		{
			bInsert = TRUE;
		}
	}

	// 不区分大小写 & 不全字匹配
	else if (!m_bMachCase && !m_bMachWholeString)
	{
		CString szTemp = strSubName;
		szTemp.MakeUpper();

		if (szTemp.Find(m_szFindWhatUpper) != -1)
		{
			bInsert = TRUE;
		}
	}

	if (bInsert)
	{
		CString szKeyPath = GetRootKeyString(hRoot);
		szKeyPath += L"\\" + szSubKey;
		int nItem = m_list.InsertItem(m_list.GetItemCount(), szKeyPath);
		m_list.SetItemText(nItem, 1, szValue);
		m_list.SetItemText(nItem, 2, szRet);
		m_nCnt++;
	}
}

void CRegFindDlg::EnumKeys(HKEY hRoot, CString szSubKey)
{
	if (!hRoot)
	{
		return;
	}
	
	szSubKey.TrimLeft('\\');
	HKEY hKeyTemp = NULL;
	LONG nRet = RegOpenKeyEx(hRoot, szSubKey, 0, KEY_READ, &hKeyTemp);
	if (nRet != ERROR_SUCCESS)
	{
		return;
	}

	DWORD dwSubKeys = 0, dwSubValues = 0;
	LONG lRet = ::RegQueryInfoKey(hKeyTemp, NULL, NULL, NULL, &dwSubKeys, NULL, NULL, &dwSubValues, NULL, NULL, NULL, NULL);
	if (lRet != ERROR_SUCCESS)
	{
		RegCloseKey(hKeyTemp);
		return;
	}

	// 如果查找项
	if (m_bKeys)
	{
		for (DWORD dwIndex = 0; !m_bStop && dwIndex < dwSubKeys; dwIndex++)
		{
			DWORD dwLen = 1024;
			TCHAR szSubName[1024];
			memset(szSubName, 0, 1024 * sizeof(TCHAR));

			lRet = RegEnumKey(hKeyTemp, dwIndex, szSubName, dwLen);
			if (lRet == ERROR_SUCCESS)
			{ 
				CString strSubName = CString(szSubName);

				// 区分大小写 & 匹配全字符
				if (m_bMachCase && m_bMachWholeString)
				{
					if (!strSubName.Compare(m_szFindWhat))
					{
						InsertKeys(hRoot, szSubKey, strSubName);
					}
				}

				// 区分大小写,但是不全字匹配
				else if (m_bMachCase && !m_bMachWholeString)
				{
					if (-1 != strSubName.Find(m_szFindWhat))
					{
						InsertKeys(hRoot, szSubKey, strSubName);
					}
				}	
				
				// 不区分大小写 & 全字匹配
				else if (!m_bMachCase && m_bMachWholeString)
				{
					if (!strSubName.CompareNoCase(m_szFindWhat))
					{
						InsertKeys(hRoot, szSubKey, strSubName);
					}
				}

				// 不区分大小写 & 不全字匹配
				else if (!m_bMachCase && !m_bMachWholeString)
				{
					CString szTemp = strSubName;
					szTemp.MakeUpper();

					if (szTemp.Find(m_szFindWhatUpper) != -1)
					{
						InsertKeys(hRoot, szSubKey, strSubName);
					}
				}
				
				// 递归枚举
				EnumKeys(hRoot, szSubKey + L"\\" + szSubName);
			}
		}
	}
	
	// 如果值和内容都不需要,那么直接返回了
	if (!m_bData && !m_bValues)
	{
		RegCloseKey(hKeyTemp);
		return;
	}
	
	// 枚举值
	for (DWORD dwIndex = 0; !m_bStop && dwIndex < dwSubValues; dwIndex++)
	{
		DWORD dwLen = 1024;
		TCHAR szSubName[1024];
		memset(szSubName, 0, 1024 * sizeof(TCHAR));
		
		DWORD dwType = 0, dwDataLen = 32767;
		BYTE pData[32767] = {0};
		lRet = RegEnumValue(hKeyTemp, dwIndex, szSubName, &dwLen, NULL, &dwType, pData, &dwDataLen);
		if (lRet == ERROR_SUCCESS)
		{ 
			// 如果枚举值被选中了
			if (m_bValues)
			{
				CString strSubName = CString(szSubName);

				// 区分大小写 & 匹配全字符
				if (m_bMachCase && m_bMachWholeString)
				{
					if (!strSubName.Compare(m_szFindWhat))
					{
						InsertVlaues(hRoot, szSubKey, strSubName);
					}
				}

				// 区分大小写,但是不全字匹配
				else if (m_bMachCase && !m_bMachWholeString)
				{
					if (-1 != strSubName.Find(m_szFindWhat))
					{
						InsertVlaues(hRoot, szSubKey, strSubName);
					}
				}	

				// 不区分大小写 & 全字匹配
				else if (!m_bMachCase && m_bMachWholeString)
				{
					if (!strSubName.CompareNoCase(m_szFindWhat))
					{
						InsertVlaues(hRoot, szSubKey, strSubName);
					}
				}

				// 不区分大小写 & 不全字匹配
				else if (!m_bMachCase && !m_bMachWholeString)
				{
					CString szTemp = strSubName;
					szTemp.MakeUpper();

					if (szTemp.Find(m_szFindWhatUpper) != -1)
					{
						InsertVlaues(hRoot, szSubKey, strSubName);
					}
				}
			}

			// 如果枚举数据被选中了
			if (m_bData)
			{
				InsertData(hRoot, szSubKey, szSubName, dwType, pData, dwDataLen);
			}
		}
	}

	RegCloseKey(hKeyTemp);
}

void CRegFindDlg::SearchRegistry()
{
	m_bStop = FALSE;
	m_nCnt = 0;
	m_szSearchResults = szRegistrySearchNow[g_enumLang];
	SendMessage(WM_UPDATE_UI);

	// 搜索指定键
	if (m_nRadio == 0)
	{
		HKEY hKey = GetRootKey(m_szSearchInKey);
		CString szSubKey;
		if (m_szSearchInKey.Find('\\') != -1)
		{
			szSubKey = m_szSearchInKey.Right(m_szSearchInKey.GetLength() - m_szSearchInKey.Find('\\') - 1);
		}

		EnumKeys(hKey, szSubKey);
	}

	// 搜索整个注册表
	else
	{
		EnumKeys(HKEY_CLASSES_ROOT, NULL);
		EnumKeys(HKEY_CURRENT_USER, NULL);
		EnumKeys(HKEY_LOCAL_MACHINE, NULL);
		EnumKeys(HKEY_USERS, NULL);
		EnumKeys(HKEY_CURRENT_CONFIG, NULL);
	}

	m_szSearchResults.Format(szFindResults[g_enumLang], m_nCnt);
	SendMessage(WM_UPDATE_UI);
	SendMessage(WM_SEARCH_FINISH);
}

CString CRegFindDlg::GetRootKeyString(HKEY hRoot)
{
	CString szRet;
	if (!hRoot)
	{
		return szRet;
	}

	if (hRoot == HKEY_CLASSES_ROOT)
	{
		szRet = L"HKEY_CLASSES_ROOT";
	}
	else if (hRoot == HKEY_CURRENT_USER)
	{
		szRet = L"HKEY_CURRENT_USER";
	}
	else if (hRoot == HKEY_LOCAL_MACHINE)
	{
		szRet = L"HKEY_LOCAL_MACHINE";
	}
	else if (hRoot == HKEY_USERS)
	{
		szRet = L"HKEY_USERS";
	}
	else if (hRoot == HKEY_CURRENT_CONFIG)
	{
		szRet = L"HKEY_CURRENT_CONFIG";
	}

	return szRet;
}

HKEY CRegFindDlg::GetRootKey(CString szKey)
{
	HKEY hRet = NULL;
	
	if (szKey.IsEmpty())
	{
		return hRet;
	}
	
	CString szKeyRoot;
	if (szKey.Find('\\') != -1)
	{
		szKeyRoot = szKey.Left(szKey.Find('\\'));
	}
	else
	{
		szKeyRoot = szKey;
	}
	
	if (!szKeyRoot.CompareNoCase(L"HKEY_CLASSES_ROOT"))
	{
		hRet = HKEY_CLASSES_ROOT;
	}
	else if (!szKeyRoot.CompareNoCase(L"HKEY_CURRENT_USER"))
	{
		hRet = HKEY_CURRENT_USER;
	}
	else if (!szKeyRoot.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
	{
		hRet = HKEY_LOCAL_MACHINE;
	}
	else if (!szKeyRoot.CompareNoCase(L"HKEY_USERS"))
	{
		hRet = HKEY_USERS;
	}
	else if (!szKeyRoot.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
	{
		hRet = HKEY_CURRENT_CONFIG;
	}

	return hRet;
}

// 开始搜索
void CRegFindDlg::OnBnClickedButtonStart()
{
	UpdateData(TRUE);
	m_list.DeleteAllItems();

	// 判断搜索条件是否已经填写
	if (m_szFindWhat.IsEmpty())
	{
		MessageBox(szRegistryPleaseInputFindWhat[g_enumLang], L"AntiSpy", MB_OK | MB_ICONWARNING);
		return;
	}

	// 如果是在某个键值中查询
	if (m_nRadio == 0)
	{
		//	首先判断是否填写
		if (m_szSearchInKey.IsEmpty())
		{
			MessageBox(szRegistryPleaseInputWhatTheKeyToFind[g_enumLang], L"AntiSpy", MB_OK | MB_ICONWARNING);
			return;
		}

		// 判断根键是否存在
		HKEY hKey = GetRootKey(m_szSearchInKey);
		if (!hKey)
		{
			MessageBox(szCanNotOpenKey[g_enumLang], L"AntiSpy", MB_OK | MB_ICONERROR);
			return;
		}

		// 判断能否打开该键
		CString szSubKey;
		if (m_szSearchInKey.Find('\\') != -1)
		{
			szSubKey = m_szSearchInKey.Right(m_szSearchInKey.GetLength() - m_szSearchInKey.Find('\\') - 1);
		}
		HKEY hKeyTemp = NULL;
		LONG nRet = RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hKeyTemp);

		if (ERROR_SUCCESS != nRet)
		{
			MessageBox(szCanNotOpenKey[g_enumLang], L"AntiSpy", MB_OK | MB_ICONERROR);
			return;
		}

		RegCloseKey(hKeyTemp);
	}

	// 保存一份大写字母的查找内容,如果查找忽略大小写,就可以使用这份了
	m_szFindWhatUpper = m_szFindWhat;
	m_szFindWhatUpper.MakeUpper();

	// 通过了前面的检查，开始搜索
	ChangeKongjianStatus(FALSE);
	
	if (m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	DWORD dwTid = 0;
	m_hThread = CreateThread(NULL, 0, SearchRegistryProc, this, 0, &dwTid);
}

// 停止搜索
void CRegFindDlg::OnBnClickedButtonStop()
{
	m_bStop = TRUE;
	ChangeKongjianStatus(TRUE);
}

void CRegFindDlg::OnBnClickedCheckKey()
{
	UpdateData(TRUE);
	if (!m_bKeys && !m_bValues && !m_bData)
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	}
}

void CRegFindDlg::OnBnClickedCheckValues()
{
	UpdateData(TRUE);
	if (!m_bKeys && !m_bValues && !m_bData)
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	}
}

void CRegFindDlg::OnBnClickedCheckData()
{
	UpdateData(TRUE);
	if (!m_bKeys && !m_bValues && !m_bData)
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	}
}

LRESULT CRegFindDlg::SearchFinished(WPARAM, LPARAM)
{
	OnBnClickedButtonStop();
	return 0;
}

LRESULT CRegFindDlg::UpdateUI(WPARAM, LPARAM)
{
	UpdateData(FALSE);
	return 0;
}

void CRegFindDlg::OnClose()
{
	m_bStop = TRUE;
	if (m_hThread)
	{
		if (WaitForSingleObject(m_hThread, 500) != WAIT_OBJECT_0)
		{
			TerminateThread(m_hThread, 0);
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	CDialog::OnClose();
}

void CRegFindDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_list.GetItemCount() && -1 != m_Functions.GetSelectItem(&m_list))
	{
		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, ID_AUTORUN_JMP_TO_REG, szJmpToReg[g_enumLang]);

		int x = GetSystemMetrics(SM_CXMENUCHECK);
		int y = GetSystemMetrics(SM_CYMENUCHECK);
		if (x >= 15 && y >= 15)
		{
			// 设置菜单图标
			menu.SetMenuItemBitmaps(ID_AUTORUN_JMP_TO_REG, MF_BYCOMMAND, &m_bmReg, &m_bmReg);
		}

		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
		menu.DestroyMenu();
	}
	
	*pResult = 0;
}

void CRegFindDlg::OnAutorunJmpToReg()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szKey = m_list.GetItemText(nItem, 0);
		CString szValue = m_list.GetItemText(nItem, 1);
		m_Functions.JmpToMyRegistry(szKey, szValue);
	}
}

void CRegFindDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CRegFindDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szKey = m_list.GetItemText(nItem, 0);
		CString szValue = m_list.GetItemText(nItem, 1);
		m_Functions.JmpToMyRegistry(szKey, szValue);
	}
	*pResult = 0;
}

void CRegFindDlg::OnBnClickedRadio2()
{
	UpdateData();
	if (m_nRadio == 1)
	{
		GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
	}
}

void CRegFindDlg::OnBnClickedRadio1()
{
	UpdateData();
	if (m_nRadio == 0)
	{
		GetDlgItem(IDC_EDIT2)->EnableWindow(TRUE);
	}
}
