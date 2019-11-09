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
// UserDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "UserDlg.h"
#include <lm.h>
#pragma comment(lib, "netapi32.lib")

// CUserDlg 对话框

IMPLEMENT_DYNAMIC(CUserDlg, CDialog)

CUserDlg::CUserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserDlg::IDD, pParent)
	, m_szStatus(_T(""))
{

}

CUserDlg::~CUserDlg()
{
}

void CUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_USER_STATUS, m_szStatus);
}


BEGIN_MESSAGE_MAP(CUserDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CUserDlg::OnNMRclickList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, &CUserDlg::OnNMCustomdrawList)
	ON_COMMAND(ID_USERS_REFRESH, &CUserDlg::OnUsersRefresh)
	ON_COMMAND(ID_USERS_DELETE, &CUserDlg::OnUsersDelete)
	ON_COMMAND(ID_USER_EXPORT_TEXT, &CUserDlg::OnUserExportText)
	ON_COMMAND(ID_USER_EXPORT_EXCEL, &CUserDlg::OnUserExportExcel)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CUserDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_USER_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP

// CUserDlg 消息处理程序

void CUserDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CUserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szUserName[g_enumLang], LVCFMT_LEFT, 500);
	m_list.InsertColumn(1, szUserPrivileges[g_enumLang], LVCFMT_LEFT, 670);

	m_szStatus.Format(szUserStatus[g_enumLang], 0, 0);
	UpdateData(FALSE);

//	ListUsers();

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CUserDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_USER_EXPORT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_USER_EXPORT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_USERS_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_USERS_DELETE, szDelete[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_USERS_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_USERS_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
		menu.SetMenuItemBitmaps(4, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (m_list.GetItemCount() == 0)
	{
		for (int i = 1; i < 5; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else if (m_list.GetSelectedCount() == 0)
	{
		menu.EnableMenuItem(ID_USERS_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CUserDlg::OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

	*pResult = CDRF_DODEFAULT;

	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF clrNewTextColor, clrNewBkColor;
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		clrNewTextColor = g_MicrosoftItemClr;
		clrNewBkColor = RGB( 255, 255, 255 );	

		DWORD dwData = (DWORD)m_list.GetItemData(nItem);
		if (dwData == 1)
		{
			clrNewTextColor = g_HiddenOrHookItemClr;
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}

BOOL CUserDlg::IsHideUser(CString szName)
{
	BOOL bHide = TRUE;
	
	if (szName.IsEmpty())
	{
		return bHide;
	}

	for (list<CString>::iterator ir = m_listNormalUsers.begin();
		ir != m_listNormalUsers.end();
		ir++)
	{
		CString szName1 = *ir;

		if (!szName.CompareNoCase(szName1))
		{
			bHide = FALSE;
			break;
		}
	}

	return bHide;
}

void CUserDlg::ListUsers()
{
	m_list.DeleteAllItems();
	m_listAllUsers.clear();

	NormalListUsers();

	WCHAR szKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','A','M','\\','S','A','M','\\','D','o','m','a','i','n','s','\\','A','c','c','o','u','n','t','\\','U','s','e','r','s','\\','N','a','m','e','s','\0'};
	list<KEY_INFO> keyList;
	m_Registry.EnumKeys(szKey, keyList);

	DWORD dwHideUser = 0, dwAllUser = 0;

	for (list<KEY_INFO>::iterator ir = keyList.begin();
		ir != keyList.end();
		ir++)
	{
		LPUSER_INFO_3 bufptr = NULL;     
		NetUserGetInfo(0, ir->szName, 3, (LPBYTE*)&bufptr);    
		
		WCHAR szUnknow[] = {'U','n','k','n','o','w','\0'};
		WCHAR szGuest[] = {'G','u','e','s','t','\0'};
		WCHAR szUser[] = {'U','s','e','r','\0'};
		WCHAR szAdministrator[] = {'A','d','m','i','n','i','s','t','r','a','t','o','r','\0'};

		CString szPreivilege = szUnknow;
		
		if (bufptr != NULL)
		{
			switch (bufptr->usri3_priv)
			{
			case USER_PRIV_GUEST:
				szPreivilege = szGuest;
				break;

			case USER_PRIV_USER:
				szPreivilege = szUser;
				break;

			case USER_PRIV_ADMIN:
				szPreivilege = szAdministrator;
				break;
			}

			NetApiBufferFree(bufptr);
		}

		int n = m_list.InsertItem(m_list.GetItemCount(), ir->szName);
		m_list.SetItemText(n, 1, szPreivilege);
		
		if (IsHideUser(ir->szName))
		{
			dwHideUser++;
			m_list.SetItemData(n, 1);
		}

		dwAllUser++;
	}

	m_szStatus.Format(szUserStatus[g_enumLang], dwAllUser, dwHideUser);
	UpdateData(FALSE);
}

void CUserDlg::NormalListUsers()
{
	LPUSER_INFO_1 pBuf = NULL;
	LPUSER_INFO_1 pTmpBuf = NULL;

	DWORD dwLevel = 1;
	DWORD dwPrefMaxLen   = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead  = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0; 
//	DWORD dwTotalCount   = 0;
	NET_API_STATUS nStatus;

	m_listNormalUsers.clear();

	do
	{
		nStatus = NetUserEnum(NULL, dwLevel, FILTER_NORMAL_ACCOUNT, // global users
			(LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{    
				for (DWORD i = 0; i < dwEntriesRead; i++)
				{        
					if (pTmpBuf == NULL)
					{
						break;
					}    

// 					LPUSER_INFO_3 bufptr = NULL;     
// 					NetUserGetInfo(0, pTmpBuf->usri1_name, 3, (LPBYTE*)&bufptr);    

					m_listNormalUsers.push_back(pTmpBuf->usri1_name);
					
// 					if (bufptr != NULL)
// 					{
// 						NetApiBufferFree(bufptr);
// 					}

// 					userName[i] = WCharsToMultiBytes(bufptr->usri3_name);
// 					if (userName[i]== NULL)
// 					{
// 						return 1;
// 					}  

					pTmpBuf++;
//					dwTotalCount++;
				}
			}
		}
		else
		{
			return/* 1*/;
		}

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}

	} while (nStatus == ERROR_MORE_DATA); 
}

BOOL CUserDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

void CUserDlg::OnUsersRefresh()
{
	ListUsers();
}

void CUserDlg::OnUsersDelete()
{
	if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szName = m_list.GetItemText(nItem, 0);
		NetUserDel(NULL, szName);
	}

	OnUsersRefresh();
}

void CUserDlg::OnUserExportText()
{
	m_Function.ExportListToTxt(&m_list, m_szStatus);
}

void CUserDlg::OnUserExportExcel()
{
	WCHAR szTitle[] = {'U','s','e','r','s','\0'};
	m_Function.ExportListToExcel(&m_list, szTitle, m_szStatus);
}