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
// FileRestartDeleteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "FileRestartDeleteDlg.h"
#include "CommonStruct.h"

// CFileRestartDeleteDlg dialog

IMPLEMENT_DYNAMIC(CFileRestartDeleteDlg, CDialog)

CFileRestartDeleteDlg::CFileRestartDeleteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileRestartDeleteDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_infoVector.clear();
}

CFileRestartDeleteDlg::~CFileRestartDeleteDlg()
{
	m_infoVector.clear();
}

void CFileRestartDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_PENDING_CNT, m_szStatus);
}


BEGIN_MESSAGE_MAP(CFileRestartDeleteDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFileRestartDeleteDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CFileRestartDeleteDlg::OnNMRclickList)
	ON_COMMAND(ID_PENDNG_REFRESH, &CFileRestartDeleteDlg::OnPendngRefresh)
	ON_COMMAND(ID_PENDNG_DELETE, &CFileRestartDeleteDlg::OnPendngDelete)
	ON_COMMAND(ID_PENDNG_DELETE_ALL, &CFileRestartDeleteDlg::OnPendngDeleteAll)
	ON_COMMAND(ID_PENDNG_GOTO_file, &CFileRestartDeleteDlg::OnPendngGotofile)
	ON_COMMAND(ID_PENDNG_LOOK_SHUXING, &CFileRestartDeleteDlg::OnPendngLookShuxing)
	ON_COMMAND(ID_PENDNG_GOTO_NEW_FILE, &CFileRestartDeleteDlg::OnPendngGotoNewFile)
	ON_COMMAND(ID_PENDNG_LOOK_NEW_FILE_SHUXING, &CFileRestartDeleteDlg::OnPendngLookNewFileShuxing)
	ON_COMMAND(ID_PENDING_EXPORT_TEXT, &CFileRestartDeleteDlg::OnPendingExportText)
	ON_COMMAND(ID_PENDING_EXPORT_EXCEL, &CFileRestartDeleteDlg::OnPendingExportExcel)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CFileRestartDeleteDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_PENDING_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CFileRestartDeleteDlg message handlers

void CFileRestartDeleteDlg::OnBnClickedOk()
{
}

BOOL CFileRestartDeleteDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetWindowText(szFileRestartDeleteInfo[g_enumLang]);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szFileRestartDeleteType[g_enumLang], LVCFMT_LEFT, 120);
	m_list.InsertColumn(1, szFileRestartFilePath[g_enumLang], LVCFMT_LEFT, 305);
	m_list.InsertColumn(2, szFileRestartNewFilePath[g_enumLang], LVCFMT_LEFT, 305);

	ViewFileRestartDeleteInfo();

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFileRestartDeleteDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CFileRestartDeleteDlg::ViewFileRestartDeleteInfo()
{
	m_list.DeleteAllItems();
	m_infoVector.clear();
	GetPendingFileRenameOperations();
}

CString CFileRestartDeleteDlg::TerPath(CString szPath)
{
	CString szRet;
	if (szPath.IsEmpty())
	{
		return szRet;
	}
	
	WCHAR ch = szPath.GetAt(0);
	while (ch == '?' || ch == '\\')
	{
		szPath.TrimLeft(ch);
		ch = szPath.GetAt(0);
	}
	
	szRet = szPath;
	return szRet;
}

// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager
// PendingFileRenameOperations
void CFileRestartDeleteDlg::GetPendingFileRenameOperations()
{
	MZF_UNICODE_STRING unKey;
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','S','e','s','s','i','o','n',' ','M','a','n','a','g','e','r','\0'};
	WCHAR szValue[] = {'P','e','n','d','i','n','g','F','i','l','e','R','e','n','a','m','e','O','p','e','r','a','t','i','o','n','s','\0'};
	WCHAR szTemp[] = {'\\','?','?','\\','\0'};

	m_szStatus.Format(szFilePendingStatus[g_enumLang], 0, 0);

	if (m_Functions.InitUnicodeString(&unKey, szKey))
	{
		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);
		if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			BOOL bDefault = FALSE;

			for (ULONG i = 0; ; i++)
			{
				ULONG nRetLen = 0;
				BOOL bRet = m_Registry.EnumerateValueKey(hKey, i, KeyValueFullInformation, NULL, 0, &nRetLen);
				if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
				{
					break;
				}
				else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
				{
					PKEY_VALUE_FULL_INFORMATION pBuffer = (PKEY_VALUE_FULL_INFORMATION)malloc(nRetLen + 0x100);
					if (pBuffer)
					{
						memset(pBuffer, 0, nRetLen + 0x100);
						bRet = m_Registry.EnumerateValueKey(hKey, i, KeyValueFullInformation, pBuffer, nRetLen + 0x100, &nRetLen);

						if (bRet && 
							pBuffer->Type == REG_MULTI_SZ && 
							pBuffer->NameLength / sizeof(WCHAR) == wcslen(szValue))
						{
							WCHAR szTempName[1024] = {0};
							wcsncpy_s(szTempName, 1024, pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));

							if (!_wcsnicmp(szTempName, szValue, wcslen(szValue)) && 
								pBuffer->DataLength > 0)
							{
								ULONG nDel = 0, nReplace = 0;
								DWORD len = 0;
								WCHAR *Data = (WCHAR*)((PBYTE)pBuffer + pBuffer->DataOffset);
								while (wcslen(Data + len))
								{
									FILE_PENDING_INFO info;

									CString szType, path1, path2;
									CString path1Temp = Data + len;
									wcsncpy_s(info.szFile, MAX_PATH, Data + len, wcslen(Data + len));

									len += wcslen(Data + len) + 1;
									CString path2Temp = Data + len;
									wcsncpy_s(info.szNewFile, MAX_PATH, Data + len, wcslen(Data + len));

									len += wcslen(Data + len) + 1;

									if (!path1Temp.IsEmpty())
									{
										path1 = TerPath(path1Temp);

										int nItem = m_list.GetItemCount();

										if (path2Temp.IsEmpty())
										{
											szType = szFilePendingDelayDelete[g_enumLang];
											path2 = L"-";
											nDel++;
										}
										else
										{
											path2 = TerPath(path2Temp);
											szType = szFilePendingDelayReplace[g_enumLang];
											nReplace++;
										}

										m_list.InsertItem(nItem, szType);
										m_list.SetItemText(nItem, 1, path1);
										m_list.SetItemText(nItem, 2, path2);

										m_list.SetItemData(nItem, nItem);
										m_infoVector.push_back(info);
									}
									
								}
								
								m_szStatus.Format(szFilePendingStatus[g_enumLang], nDel, nReplace);
								free(pBuffer);
								break;
							}
						}

						free(pBuffer);
					}
				}
			}

			CloseHandle(hKey);
		}

		m_Functions.FreeUnicodeString(&unKey);
	}

	UpdateData(FALSE);
}

void CFileRestartDeleteDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_PENDING_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_PENDING_EXPORT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PENDNG_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PENDNG_DELETE, szFilePendingDeleteCurrent[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PENDNG_DELETE_ALL, szFilePendingDeleteAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PENDNG_GOTO_file, szFilePendingFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PENDNG_LOOK_SHUXING, szFilePendingProperties[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_PENDNG_GOTO_NEW_FILE, szFilePendingFindNewInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_PENDNG_LOOK_NEW_FILE_SHUXING, szFilePendingNewFileProperties[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// …Ë÷√≤Àµ•Õº±Í
		menu.SetMenuItemBitmaps(ID_PENDNG_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_PENDNG_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_PENDNG_DELETE_ALL, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(ID_PENDNG_GOTO_file, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PENDNG_GOTO_NEW_FILE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_PENDNG_LOOK_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_PENDNG_LOOK_NEW_FILE_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(11, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 12; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetSelectedCount())
		{
			for (int i = 2; i < 11; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}

			menu.EnableMenuItem(ID_PENDNG_DELETE_ALL, MF_BYCOMMAND | MF_ENABLED);
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				CString szPath2 = m_list.GetItemText(nItem, 2);
				if (!szPath2.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_PENDNG_GOTO_NEW_FILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
					menu.EnableMenuItem(ID_PENDNG_LOOK_NEW_FILE_SHUXING, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}
		}
		else
		{
			for (int i = 2; i < 11; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
	
			menu.EnableMenuItem(ID_PENDNG_DELETE, MF_BYCOMMAND | MF_ENABLED);
			menu.EnableMenuItem(ID_PENDNG_DELETE_ALL, MF_BYCOMMAND | MF_ENABLED);
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CFileRestartDeleteDlg::OnPendngRefresh()
{
	ViewFileRestartDeleteInfo();
}

void CFileRestartDeleteDlg::ModifyValue(PVOID pData, ULONG DataSize)
{
	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','S','e','s','s','i','o','n',' ','M','a','n','a','g','e','r','\0'};
	WCHAR szValue[] = {'P','e','n','d','i','n','g','F','i','l','e','R','e','n','a','m','e','O','p','e','r','a','t','i','o','n','s','\0'};
	WCHAR szTemp[] = {'\\','?','?','\\','\0'};

	MZF_UNICODE_STRING unKey;
	if (m_Functions.InitUnicodeString(&unKey, szKey))
	{
		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			MZF_UNICODE_STRING unValue;
			if (m_Functions.InitUnicodeString(&unValue, szValue))
			{
				m_Registry.SetValueKey(hKey, &unValue, 0, REG_MULTI_SZ, pData, DataSize);
				m_Functions.FreeUnicodeString(&unValue);
			}

			CloseHandle(hKey);
		}

		m_Functions.FreeUnicodeString(&unKey);
	}
}

void CFileRestartDeleteDlg::OnPendngDelete()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		int nData = (int)m_list.GetItemData(nItem);
		int i = 0;

		for (list<FILE_PENDING_INFO>::iterator ir = m_infoVector.begin(); 
			ir != m_infoVector.end();
			ir++)
		{
			if (i == nData)
			{
				m_infoVector.erase(ir);
				break;
			}

			i++;
		}
	}

	ULONG nMax = 32766, nLen = 0;
	WCHAR *szBuffer = (WCHAR*)malloc(nMax);
	if (szBuffer == NULL)
	{
		return;
	}

	memset(szBuffer, 0, nMax);

	for (list<FILE_PENDING_INFO>::iterator ir = m_infoVector.begin(); 
		ir != m_infoVector.end();
		ir++)
	{
		wcsncpy_s(szBuffer + nLen, MAX_PATH * 2, ir->szFile, wcslen(ir->szFile));
		nLen += wcslen(ir->szFile) + 1;
		wcsncpy_s(szBuffer + nLen, MAX_PATH * 2, ir->szNewFile, wcslen(ir->szNewFile));
		nLen += wcslen(ir->szNewFile) + 1;
	}

	nLen = (nLen + 2) * sizeof(WCHAR);

	ModifyValue((PVOID)szBuffer, nLen);
	m_list.DeleteItem(nItem);

	free(szBuffer);
	szBuffer = NULL;
}

void CFileRestartDeleteDlg::OnPendngDeleteAll()
{
	ModifyValue(NULL, 0);
	m_list.DeleteAllItems();
}

void CFileRestartDeleteDlg::OnPendngGotofile()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		m_Functions.LocationExplorer(m_list.GetItemText(nItem, 1));
	}
}

void CFileRestartDeleteDlg::OnPendngLookShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		m_Functions.OnCheckAttribute(m_list.GetItemText(nItem, 1));
	}
}

void CFileRestartDeleteDlg::OnPendngGotoNewFile()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		m_Functions.LocationExplorer(m_list.GetItemText(nItem, 2));
	}
}

void CFileRestartDeleteDlg::OnPendngLookNewFileShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		m_Functions.OnCheckAttribute(m_list.GetItemText(nItem, 2));
	}
}

void CFileRestartDeleteDlg::OnPendingExportText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CFileRestartDeleteDlg::OnPendingExportExcel()
{
	WCHAR szPend[] = {'P','e','n','d','i','n','g','O','p','e','r','a','t','i','o','n','\0'};
	m_Functions.ExportListToExcel(&m_list, szPend, m_szStatus);
}