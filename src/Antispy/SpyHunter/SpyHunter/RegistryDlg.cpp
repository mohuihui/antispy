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
// RegistryDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "RegistryDlg.h"
#include <Sddl.h>
#include <shlwapi.h>
#include "RenameKeyDlg.h"
#include "RegModifyDlg.h"
#include "ModifyDwordDlg.h"
#include "HexEditDlg.h"
#include "Comfunc.h"
#include "RegUnlockDlg.h"

// CRegistryDlg 对话框

IMPLEMENT_DYNAMIC(CRegistryDlg, CDialog)

CRegistryDlg::CRegistryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegistryDlg::IDD, pParent)
	, m_szStatus(_T(""))
	, m_szAddress(_T(""))
{
	m_szHKeyCurrentUser = NULL;
	m_szComboText = _T("");
	m_hChild = NULL;
	m_RightClickItem = NULL;
	m_bUseHive = FALSE;
	m_nComboBoxCnt = 0;
	m_pFindRegDlg = NULL;
	m_nCurrentHistroy = 0;
	m_bBackOrForward = FALSE;
}

CRegistryDlg::~CRegistryDlg()
{
	if (m_szHKeyCurrentUser)
	{
		free(m_szHKeyCurrentUser);
		m_szHKeyCurrentUser = NULL;
	}
}

void CRegistryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_Text(pDX, IDC_REGISTRY_STATUS, m_szStatus);
	DDX_Control(pDX, IDC_COMBO, m_ComboBox);
	DDX_CBString(pDX, IDC_COMBO, m_szComboText);
	DDX_Control(pDX, IDC_GOTO, m_BtnGoto);
	DDX_Text(pDX, IDC_STATIC_ADDRESS, m_szAddress);
	DDX_Control(pDX, IDC_BACK, m_btnBack);
	DDX_Control(pDX, IDC_FORWARD, m_btnForward);
}


BEGIN_MESSAGE_MAP(CRegistryDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CRegistryDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, &CRegistryDlg::OnTvnSelchangedTree)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE, &CRegistryDlg::OnTvnItemexpandingTree)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, &CRegistryDlg::OnNMRclickTree)
	ON_COMMAND(ID_REG_REFRESH, &CRegistryDlg::OnRegRefresh)
	ON_COMMAND(ID_REG_LOOK_FOR, &CRegistryDlg::OnRegLookFor)
	ON_COMMAND(ID_REG_EXPORT, &CRegistryDlg::OnRegExport)
	ON_COMMAND(ID_REG_DELETE, &CRegistryDlg::OnRegDelete)
	ON_COMMAND(ID_REG_RENAME, &CRegistryDlg::OnRegRename)
	ON_COMMAND(ID_REG_NEW_KEY, &CRegistryDlg::OnRegNewKey)
	ON_COMMAND(ID_REG_COPY_KEY_NAME, &CRegistryDlg::OnRegCopyKeyName)
	ON_COMMAND(ID_REG_COPY_FULL_KEY_NAME, &CRegistryDlg::OnRegCopyFullKeyName)
	ON_COMMAND(ID_STRING_VALUE, &CRegistryDlg::OnStringValue)
	ON_COMMAND(ID_BINARY_VALUE, &CRegistryDlg::OnBinaryValue)
	ON_COMMAND(ID_DWORD_VALUE, &CRegistryDlg::OnDwordValue)
	ON_COMMAND(ID_MUI_STRING_VALUE, &CRegistryDlg::OnMuiStringValue)
	ON_COMMAND(ID_EXPAND_STRING_VALUE, &CRegistryDlg::OnExpandStringValue)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CRegistryDlg::OnNMRclickList)
	ON_COMMAND(ID_REG_LIST_REFRESH, &CRegistryDlg::OnRegListRefresh)
	ON_COMMAND(ID_REG_LIST_EXPORT, &CRegistryDlg::OnRegListExport)
	ON_COMMAND(ID_REG_LIST_MODIFY, &CRegistryDlg::OnRegListModify)
	ON_COMMAND(ID_REG_LIST_DELETE, &CRegistryDlg::OnRegListDelete)
	ON_COMMAND(ID_REG_LIST_RENAME, &CRegistryDlg::OnRegListRename)
	ON_COMMAND(ID_REG_LIST_COPY_VALUE, &CRegistryDlg::OnRegListCopyValue)
	ON_COMMAND(ID_REG_LIST_COPY_VALUE_DATA, &CRegistryDlg::OnRegListCopyValueData)
	ON_BN_CLICKED(IDC_GOTO, &CRegistryDlg::OnBnClickedGoto)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CRegistryDlg::OnNMDblclkList)
	ON_COMMAND(ID_REG_USE_HIVE, &CRegistryDlg::OnRegUseHive)
	ON_UPDATE_COMMAND_UI(ID_REG_USE_HIVE, &CRegistryDlg::OnUpdateRegUseHive)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_REG_ADD_TO_QUICK_ADDRESS, &CRegistryDlg::OnRegAddToQuickAddress)
	ON_CBN_SETFOCUS(IDC_COMBO, &CRegistryDlg::OnCbnSetfocusCombo)
	ON_CBN_KILLFOCUS(IDC_COMBO, &CRegistryDlg::OnCbnKillfocusCombo)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST, &CRegistryDlg::OnNMSetfocusList)
	ON_NOTIFY(NM_KILLFOCUS, IDC_LIST, &CRegistryDlg::OnNMKillfocusList)
	ON_CBN_DROPDOWN(IDC_COMBO, &CRegistryDlg::OnCbnDropdownCombo)
	ON_BN_CLICKED(IDC_BACK, &CRegistryDlg::OnBnClickedBack)
	ON_BN_CLICKED(IDC_FORWARD, &CRegistryDlg::OnBnClickedForward)
	ON_COMMAND(ID_REG_INPORT, &CRegistryDlg::OnRegInport)
	ON_COMMAND(ID_REG_QUERY_LOCK_INFO, &CRegistryDlg::OnRegQueryLockInfo)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CRegistryDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_TREE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
//	EASYSIZE(IDC_TREE1, ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_REGISTRY_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC_ADDRESS, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_COMBO, ES_BORDER, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_GOTO, ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
	EASYSIZE(IDC_X_SP, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
END_EASYSIZE_MAP
// CRegistryDlg 消息处理程序

void CRegistryDlg::OnBnClickedOk()
{
}

void CRegistryDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CRegistryDlg::DeleteSubTree(HTREEITEM TreeItem)
{
	if(TreeItem == NULL)   
	{
		return;   
	}

	if(m_tree.ItemHasChildren(TreeItem))
	{
		HTREEITEM hNext, hChild = m_tree.GetChildItem(TreeItem);
		while(hChild != NULL)
		{
			hNext = m_tree.GetNextSiblingItem(hChild);
			m_tree.DeleteItem(hChild);
			hChild = hNext; 
		}
	}
}

void CRegistryDlg::EnumSubKeys(CString szKey, HTREEITEM hItem, BOOL bSubSubKey/* = FALSE */)
{
	if ( !szKey.IsEmpty() && hItem != NULL )
	{
		DeleteSubTree(hItem);

		MZF_UNICODE_STRING unKey;

		if (m_Function.InitUnicodeString(&unKey, szKey.GetBuffer()))
		{
			HANDLE hKey;
			MZF_OBJECT_ATTRIBUTES oa;

			InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

			if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
			{
				for (ULONG i = 0; ; i++)
				{
					ULONG nRetLen = 0;
					BOOL bRet = m_Registry.EnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &nRetLen);

					if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
					{
						break;
					}
					else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
					{
						PKEY_BASIC_INFORMATION pBuffer = (PKEY_BASIC_INFORMATION)malloc(nRetLen + 0x100);
						if (pBuffer)
						{
							memset(pBuffer, 0, nRetLen + 0x100);
							bRet = m_Registry.EnumerateKey(hKey, i, KeyBasicInformation, pBuffer, nRetLen + 0x100, &nRetLen);

							if (bRet)
							{
								m_tree.InsertItem(pBuffer->Name, 1, 2, hItem, TVI_LAST);
							}

							free(pBuffer);

							// 如果是枚举子项的子项，那么只要插入一个就可以了，
							// 因为前面已经有了一个+号了，主要为了速度优化。
							if (bRet && bSubSubKey)
							{
								break;
							}
						}
					}
				}

				CloseHandle(hKey);
			}

			m_Function.FreeUnicodeString(&unKey);
		}
	}
}

void CRegistryDlg::EnumCurrentUserSubKeys(HTREEITEM hChild)
{
	DeleteSubTree(hChild);
	
	if (!m_szHKeyCurrentUser)
	{
		m_szHKeyCurrentUser = (WCHAR*)malloc(1024 * sizeof(WCHAR));
		if (m_szHKeyCurrentUser)
		{
			memset(m_szHKeyCurrentUser, 0, 1024 * sizeof(WCHAR));

			if (!GetCurrentUserKeyPath(m_szHKeyCurrentUser))
			{
				wcscpy_s(m_szHKeyCurrentUser, 1024, L"\\Registry\\User\\.Default");
			}
		}
	}

	if (m_szHKeyCurrentUser && wcslen(m_szHKeyCurrentUser) > 0)
	{
		EnumSubKeys(m_szHKeyCurrentUser, hChild, TRUE);
	}
}

BOOL CRegistryDlg::GetCurrentUserKeyPath(OUT WCHAR *szCurrentUserPath)
{
	if (!szCurrentUserPath)
	{
		return FALSE;
	}

	HANDLE TokenHandle;
	UCHAR Buffer[256] = {0};
	PSID_AND_ATTRIBUTES SidBuffer;
	ULONG Length;
	BOOL Status = FALSE;

	Status = OpenThreadToken(GetCurrentThread(),
		TOKEN_QUERY,
		TRUE,
		&TokenHandle);

	if (!Status)
	{
		Status = OpenProcessToken(GetCurrentProcess(),
			TOKEN_QUERY,
			&TokenHandle);

		if (!Status) 
		{
			return Status;
		}
	}

	SidBuffer = (PSID_AND_ATTRIBUTES)Buffer;
	Status = GetTokenInformation(TokenHandle,
		TokenUser,
		(PVOID)SidBuffer,
		sizeof(Buffer),
		&Length);

	CloseHandle(TokenHandle);
	if (!Status) return Status;

	LPTSTR StringSid;
	Status = ConvertSidToStringSid(SidBuffer[0].Sid, &StringSid);
	if (!Status) return Status;

	WCHAR szUser[] = {'\\','R','E','G','I','S','T','R','Y','\\','U','S','E','R','\\','\0'};

	Length = wcslen(StringSid) * sizeof(WCHAR) + sizeof(/*L"\\REGISTRY\\USER\\"*/szUser);

	WCHAR *szPath = (WCHAR *)malloc(Length);
	if (!szPath)
	{
		LocalFree((HLOCAL)StringSid);
		return FALSE;
	}

	memset(szPath, 0, Length);
	wcscpy_s(szPath, Length / sizeof(WCHAR),/* L"\\REGISTRY\\USER\\"*/szUser);
	wcscat_s(szPath, Length / sizeof(WCHAR), StringSid);
	
	LocalFree((HLOCAL)StringSid);
	
	wcscpy_s(szCurrentUserPath, 1024, szPath);
	free(szPath);

	return TRUE;
}

BOOL CRegistryDlg::InitMytree()
{
	int i_count = 3;
	HICON icon[4];
	icon[0] = AfxGetApp()->LoadIcon (IDI_COMPUTER);
	icon[1] = AfxGetApp()->LoadIcon (IDI_CLOSE_DIRECTORY);
	icon[2] = AfxGetApp()->LoadIcon (IDI_OPEN_DIRECTORY);

 	m_TreeImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 2); 
	for(int n = 0; n < i_count; n++)
	{
		m_TreeImageList.Add(icon[n]);
		m_tree.SetImageList(&m_TreeImageList, LVSIL_NORMAL);
	}

	DWORD dwStyle = GetWindowLong(m_tree.m_hWnd, GWL_STYLE);
	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	::SetWindowLong (m_tree.m_hWnd, GWL_STYLE, dwStyle);

	HTREEITEM Computer = m_tree.InsertItem(szMyComputer[g_enumLang], 0, 0);
	HTREEITEM sub_son0 = m_tree.InsertItem(L"HKEY_CLASSES_ROOT", 1, 2, Computer, TVI_LAST);
	HTREEITEM sub_son1 = m_tree.InsertItem(L"HKEY_CURRENT_USER", 1, 2, Computer, TVI_LAST);
	HTREEITEM sub_son2 = m_tree.InsertItem(L"HKEY_LOCAL_MACHINE", 1, 2, Computer, TVI_LAST);
	HTREEITEM sub_son3 = m_tree.InsertItem(L"HKEY_USERS", 1, 2, Computer, TVI_LAST);
	HTREEITEM sub_son4 = m_tree.InsertItem(L"HKEY_CURRENT_CONFIG", 1, 2, Computer, TVI_LAST);
	
	m_tree.Expand(Computer, TVE_EXPAND);

	return TRUE;
}

VOID CRegistryDlg::InitializeComboBox()
{
// 	m_ComboBox.InsertString(0, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
// 	m_ComboBox.InsertString(1, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows");
// 	m_ComboBox.InsertString(2, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks");
// 	m_ComboBox.InsertString(3, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");
// 	m_ComboBox.InsertString(4, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad");
// 	m_ComboBox.InsertString(5, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
// 	m_ComboBox.InsertString(6, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer");
// 	m_ComboBox.InsertString(7, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping");
// 	m_ComboBox.InsertString(8, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options");
// 	m_ComboBox.InsertString(9, L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager");
// 	m_ComboBox.InsertString(10, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\IPSec\\Policy\\Local");
// 	m_ComboBox.InsertString(11, L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace");

	list <CString> RegPathList;
	g_Config.GetAllRegPath(RegPathList);
	m_nComboBoxCnt = 0;

	for (list <CString>::iterator ir = RegPathList.begin(); 
		ir != RegPathList.end();
		ir++)
	{
		m_ComboBox.InsertString(m_nComboBoxCnt, *ir);
		m_nComboBoxCnt++;
	}
}

BOOL CRegistryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	if (g_bLoadDriverOK)
	{
	//	InitializeComboBox();
		InitMytree();
	}

	HICON icon[10];
	icon[0] = AfxGetApp()->LoadIcon (IDI_REG_SZ);
	icon[1] = AfxGetApp()->LoadIcon (IDI_DWORD);
	m_ImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 2); 
	for(int n = 0; n < 2; n++)
	{
		m_ImageList.Add(icon[n]);
		m_list.SetImageList(&m_ImageList, LVSIL_SMALL);
	}

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szName[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(1, szType[g_enumLang], LVCFMT_LEFT, 130);
	m_list.InsertColumn(2, szData[g_enumLang], LVCFMT_LEFT, 590);
	
	m_szAddress = szRegAddress[g_enumLang];

	m_BtnGoto.SetWindowText(szGoto[g_enumLang]);
	UpdateData(FALSE);

	// 设置相应的参数
	m_OpHive.SetList(&m_list);
	m_OpHive.SetTree(&m_tree);

// 	HBITMAP hBitmap =::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_GOTO));
// 	m_BtnGoto.SetBitmap(hBitmap);   
	
	InitButtom();
	InitRegistry();

	m_xSplitter.BindWithControl(this, IDC_X_SP);
	m_xSplitter.SetMinWidth(0, 0);

	m_xSplitter.AttachAsLeftPane(IDC_TREE);
	m_xSplitter.AttachAsRightPane(IDC_LIST);

	m_xSplitter.RecalcLayout();

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CRegistryDlg::InitButtom()
{
	m_bitmap.LoadBitmap(IDB_BACK_FORWARD_BTN);
	m_imageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 4, 1);
	m_imageList.Add(&m_bitmap, RGB(0,255,0));

	HICON hIcon = m_imageList.ExtractIcon(13);
	m_btnBack.SetIcon(CSize(16,16), hIcon);
	DestroyIcon(hIcon);

	hIcon = m_imageList.ExtractIcon(14); 
	m_btnForward.SetIcon(CSize(16,16), hIcon, hIcon);
	DestroyIcon(hIcon);

// 	hIcon = ::LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_IE));
// 	m_btnForward.SetIcon(CSize(16, 16), hIcon);

	int nVert = BS_VCENTER;
	int nHorz = BS_CENTER;

	m_btnBack.SetImageAlignment(nVert | nHorz);
	m_btnForward.SetImageAlignment(nVert | nHorz);

	m_btnBack.SetFlatStyle(TRUE);	
	m_btnForward.SetFlatStyle(TRUE);

	m_btnBack.EnableWindow(FALSE);
	m_btnForward.EnableWindow(FALSE);
}

void CRegistryDlg::InitRegistry()
{
	if (!g_bLoadDriverOK)
	{
		return;
	}

	m_list.DeleteAllItems();
	m_szComboText = m_szStatus = L"";
	UpdateData( FALSE );

	HTREEITEM RootItem = m_tree.GetRootItem();

	if (RootItem != NULL)
	{
		HTREEITEM hChild = m_tree.GetChildItem(RootItem);
		while(hChild != NULL)
		{
			m_tree.Expand(hChild, TVE_COLLAPSE);

			CString szKey = m_tree.GetItemText(hChild);

			if (!szKey.CompareNoCase(L"HKEY_CLASSES_ROOT"))
			{
				EnumSubKeys(CLASSES_ROOT, hChild, TRUE);
			}
			else if (!szKey.CompareNoCase(L"HKEY_CURRENT_USER"))
			{
				EnumCurrentUserSubKeys(hChild);
			}
			else if (!szKey.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
			{
				EnumSubKeys(LOCAL_MACHINE, hChild);
			}
			else if (!szKey.CompareNoCase(L"HKEY_USERS"))
			{
				EnumSubKeys(USERS, hChild);
			}
			else if (!szKey.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
			{
				EnumSubKeys(CURRENT_CONFIGL, hChild, TRUE);
			}
			else
			{
				break;
			}

			hChild = m_tree.GetNextSiblingItem(hChild);
		}
	}

	m_tree.Invalidate(TRUE);
}

CString CRegistryDlg::GetKeyType(ULONG Type)
{
	CString szRet;

	switch (Type)
	{
	case REG_NONE:
		szRet = L"REG_NONE";
		break;

	case REG_SZ:
		szRet = L"REG_SZ";
		break;

	case REG_EXPAND_SZ:
		szRet = L"REG_EXPAND_SZ";
		break;

	case REG_BINARY:
		szRet = L"REG_BINARY";
		break;

	case REG_DWORD:
		szRet = L"REG_DWORD";
		break;

	case REG_DWORD_BIG_ENDIAN:
		szRet = L"REG_DWORD_BIG_ENDIAN";
		break;

	case REG_LINK:
		szRet = L"REG_LINK";
		break;

	case REG_MULTI_SZ:
		szRet = L"REG_MULTI_SZ";
		break;

	case REG_RESOURCE_LIST:
		szRet = L"REG_RESOURCE_LIST";
		break;

	case REG_FULL_RESOURCE_DESCRIPTOR:
		szRet = L"REG_FULL_RESOURCE_DESCRIPTOR";
		break;

	case REG_RESOURCE_REQUIREMENTS_LIST:
		szRet = L"REG_RESOURCE_REQUIREMENTS_LIST";
		break;

	case REG_QWORD:
		szRet = L"REG_QWORD";
		break;
	
	default:
		szRet = L"Unknow";
	}

	return szRet;
}

CString CRegistryDlg::GetKeyData(ULONG Type, WCHAR *Data, ULONG DataLength)
{
	CString szRet;
	
	if (!DataLength)
	{
		return szRet;
	}

	switch (Type)
	{
// 	case REG_NONE:
// 		break;

	case REG_SZ:
	case REG_EXPAND_SZ:
		szRet = Data;
		break;

	case REG_LINK:
	case REG_NONE:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_LIST:
	case REG_BINARY:
		{
			for (ULONG i = 0; i < DataLength; i++)
			{
				CString szTemp;
				szTemp.Format(L"%02x ", *((PBYTE)Data + i));
				szRet += szTemp;
			}
		}
		break;

	case REG_DWORD:
		szRet.Format(L"0x%08X (%d)", *(PULONG)Data, *(PULONG)Data);
		break;

	case REG_DWORD_BIG_ENDIAN:
		{
			BYTE Value[4] = {0};
			Value[0] = *((PBYTE)Data + 3);
			Value[1] = *((PBYTE)Data + 2);
			Value[2] = *((PBYTE)Data + 1);
			Value[3] = *((PBYTE)Data + 0);
			szRet.Format(L"0x%08X (%d)", *(PULONG)Value, *(PULONG)Value);
		}
		break;

	//case REG_LINK:
	//	szRet = L"";
	//	break;

	case REG_MULTI_SZ:
		{
			DWORD len = 0;
			while (wcslen(Data + len))
			{
				szRet += (Data + len);
				szRet += L" ";
				len += wcslen(Data + len) + 1;
			}
		}
		break;

// 	case REG_RESOURCE_LIST:
// 		{
// 			ULONG nLen = DataLength > 50 ? 50 : DataLength;
// 			for (ULONG i = 0; i < nLen; i++)
// 			{
// 				CString szTemp;
// 				szTemp.Format(L"%02x ", *((PBYTE)Data + i));
// 				szRet += szTemp;
// 			}
// 		}
// 		break;

// 	case REG_FULL_RESOURCE_DESCRIPTOR:
// 		{
// 			ULONG nLen = DataLength > 50 ? 50 : DataLength;
// 			for (ULONG i = 0; i < nLen; i++)
// 			{
// 				CString szTemp;
// 				szTemp.Format(L"%02x ", *((PBYTE)Data + i));
// 				szRet += szTemp;
// 			}
// 		}
// 		break;

// 	case REG_RESOURCE_REQUIREMENTS_LIST:
// 		{
// 			ULONG nLen = DataLength > 50 ? 50 : DataLength;
// 			for (ULONG i = 0; i < nLen; i++)
// 			{
// 				CString szTemp;
// 				szTemp.Format(L"%02x ", *((PBYTE)Data + i));
// 				szRet += szTemp;
// 			}
// 		}
// 		break;

	case REG_QWORD:
		for (ULONG i = 0; i < 8; i++)
		{
			CString szTemp;
			szTemp.Format(L"%02x ", *((PBYTE)Data + i));
			szRet += szTemp;
		}
		break;

	default:
		szRet = L"Unknow";
	}

	return szRet;
}

void CRegistryDlg::EnumValues(CString szKey)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	MZF_UNICODE_STRING unKey;

	if (m_Function.InitUnicodeString(&unKey, szKey.GetBuffer()))
	{
		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);
		if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			m_list.DeleteAllItems();
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

						if (bRet)
						{
							CString szName, szType, szData;
							WCHAR szTempName[1024] = {0};
							
							szType = GetKeyType(pBuffer->Type);
							szData = GetKeyData(pBuffer->Type, (WCHAR*)((PBYTE)pBuffer + pBuffer->DataOffset), pBuffer->DataLength);

							// NameLength为0，但是内容不为0
							if (pBuffer->NameLength == 0)
							{
								szName = szDefault[g_enumLang];
								bDefault = TRUE;
							}
							else
							{
								wcsncpy_s(szTempName, 1024, pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));
								szName = szTempName;
							}

							int nImage = 1;
							if (pBuffer->Type == REG_SZ ||
								pBuffer->Type == REG_EXPAND_SZ ||
								pBuffer->Type == REG_MULTI_SZ)
							{
								nImage = 0;
							}

							int n = m_list.InsertItem(m_list.GetItemCount(), szName, nImage);

							m_list.SetItemText(n, 1, szType);
							m_list.SetItemText(n, 2, szData);

							if (pBuffer->NameLength == 0)
							{
								m_list.SetItemData(n, 1);
							}
						}

						free(pBuffer);
					}
				}
			}
			
			// 添加一个默认的键，内容为空
			if (!bDefault)
			{
				int n = m_list.InsertItem(0, szDefault[g_enumLang], 0);
				m_list.SetItemText(n, 1, L"REG_SZ");
				m_list.SetItemText(n, 2, szValueNoSet[g_enumLang]);
			//	m_list.SetItemData(n, REG_SZ);
				m_list.SetItemData(n, 1);
			}

			CloseHandle(hKey);
		}

		m_Function.FreeUnicodeString(&unKey);
	}
}

// 如果tree list 选择的项改变了，那么就枚举其值
void CRegistryDlg::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_hChild != NULL)
	{
		m_tree.SetItemState(m_hChild, 0, TVIS_DROPHILITED | TVIS_BOLD);
	}

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	CString szKeyShowPath;
	
	if(m_tree.GetRootItem() != hItem &&
		hItem != NULL)
	{
		BOOL nRet = FALSE;

		// 使用hive
		if (m_bUseHive)
		{
			HTREEITEM hItemTemp = hItem;
			CString szKeyPath, szKeyPathTemp;
			while (1)
			{
				CString szText = m_tree.GetItemText(hItemTemp);

				szKeyPath = szText + L"\\" + szKeyPath;

				if (!szText.CompareNoCase(L"HKEY_CURRENT_USER")		||
					!szText.CompareNoCase(L"HKEY_CLASSES_ROOT")		||
					!szText.CompareNoCase(L"HKEY_USERS")			||
					!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG")	||
					!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					break;
				}

				hItemTemp = m_tree.GetParentItem(hItemTemp);
			}

			if (szKeyPath.IsEmpty())
			{
				return;
			}

			szKeyPath.TrimRight('\\');
			if (szKeyPath.CompareNoCase(L"HKEY_USERS") && szKeyPath.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
			{
				WCHAR *szKey = (WCHAR *)malloc( (szKeyPath.GetLength() + 1 ) * sizeof(WCHAR) );
				if (szKey)
				{
					memset(szKey, 0, (szKeyPath.GetLength() + 1 ) * sizeof(WCHAR));
					wcsncpy_s(szKey, szKeyPath.GetLength() + 1, szKeyPath.GetBuffer(), szKeyPath.GetLength());

					char *mzKey = NULL;
					WCharToMChar(szKey, &mzKey);

					nRet = m_OpHive.ListValues_hive(mzKey);

					if (mzKey)
					{
						delete []mzKey;
						mzKey = NULL;
					}

					free(szKey);
					szKey = NULL;
				}
			}

			szKeyShowPath = szKeyPath;
		}

		// 不使用hive
		if (!m_bUseHive || !nRet)
		{
			HTREEITEM hItemTemp = hItem;
			CString szKeyPath, szKeyPathTemp;
			while (1)
			{
				CString szText = m_tree.GetItemText(hItemTemp);

				if (!szText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					szKeyPath = CLASSES_ROOT;
					szKeyShowPath = L"HKEY_CLASSES_ROOT";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					szKeyPath = m_szHKeyCurrentUser;
					szKeyShowPath = L"HKEY_CURRENT_USER";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					szKeyPath = LOCAL_MACHINE;
					szKeyShowPath = L"HKEY_LOCAL_MACHINE";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_USERS"))
				{
					szKeyPath = USERS;
					szKeyShowPath = L"HKEY_USERS";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					szKeyPath = CURRENT_CONFIGL;
					szKeyShowPath = L"HKEY_CURRENT_CONFIG";
					break;
				}
				else
				{
					szKeyPathTemp = szText + L"\\" + szKeyPathTemp;
				}

				hItemTemp = m_tree.GetParentItem(hItemTemp);
			}

			szKeyPath = szKeyPath + L"\\" + szKeyPathTemp;
			szKeyShowPath = szKeyShowPath + L"\\" + szKeyPathTemp;
			EnumValues(szKeyPath);
		}
	}
	
	szKeyShowPath.TrimRight('\\');
	m_szComboText = m_szStatus = szKeyShowPath;
	UpdateData(FALSE);

	UpdateHistroy(m_szComboText);

	*pResult = 0;
}

void CRegistryDlg::UpdateHistroy(CString szHistroy)
{
	if (!m_bBackOrForward)
	{
		m_nCurrentHistroy = m_historyVector.size();
		m_historyVector.push_back(szHistroy);

		if (m_nCurrentHistroy > 0)
		{
			m_btnForward.EnableWindow(FALSE);
			m_btnBack.EnableWindow(TRUE);
		}
	}
}

void CRegistryDlg::EnumSubSubKeys(CString szKey, HTREEITEM hItem)
{
	if (!szKey.IsEmpty() && hItem != NULL)
	{
		HTREEITEM hChild = m_tree.GetChildItem(hItem);
		while(hChild != NULL)
		{
			CString szKeyPath = szKey + L"\\" + m_tree.GetItemText(hChild);
			EnumSubKeys(szKeyPath, hChild, TRUE);  // bSubSubKey 为true,优化枚举速度
			hChild = m_tree.GetNextSiblingItem(hChild);
		}
	}
}

void CRegistryDlg::OnTvnItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	BOOL nRet = FALSE;

	// 使用hive解析
	if (m_bUseHive)
	{
		// 当是折叠状态的时候才插入子节点
		if( !(TVIS_EXPANDED & m_tree.GetItemState(hItem, TVIS_EXPANDED)) &&
			m_tree.GetRootItem() != hItem &&
			hItem != NULL)
		{
			HTREEITEM hItemTemp = hItem;
			CString szKeyPath, szKeyPathTemp;
			while (1)
			{
				CString szText = m_tree.GetItemText(hItemTemp);

				szKeyPath = szText + L"\\" + szKeyPath;

				if (!szText.CompareNoCase(L"HKEY_CURRENT_USER")		||
					!szText.CompareNoCase(L"HKEY_CLASSES_ROOT")		||
					!szText.CompareNoCase(L"HKEY_USERS")			||
					!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG")	||
					!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					break;
				}

				hItemTemp = m_tree.GetParentItem(hItemTemp);
			}

			if (szKeyPath.IsEmpty())
			{
				DeleteSubTree(hItem);
				return;
			}

			szKeyPath.TrimRight('\\');
			if (szKeyPath.CompareNoCase(L"HKEY_USERS") && szKeyPath.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
			{
				DeleteSubTree(hItem);

				WCHAR *szKey = (WCHAR *)malloc( (szKeyPath.GetLength() + 1 ) * sizeof(WCHAR) );
				if (szKey)
				{
					memset(szKey, 0, (szKeyPath.GetLength() + 1 ) * sizeof(WCHAR));
					wcsncpy_s(szKey, szKeyPath.GetLength() + 1, szKeyPath.GetBuffer(), szKeyPath.GetLength());

					char *mzKey = NULL;
					WCharToMChar(szKey, &mzKey);

					m_OpHive.SetItem(hItem);
					nRet = m_OpHive.ListSubKeys_hive(mzKey);

					if (mzKey)
					{
						delete []mzKey;
						mzKey = NULL;
					}

					free(szKey);
					szKey = NULL;
				}
			}
		}
	}

	// 如果不使用hive解析
	if (!m_bUseHive || !nRet)
	{
		// 当是折叠状态的时候才插入子节点
		if(!(TVIS_EXPANDED & m_tree.GetItemState(hItem, TVIS_EXPANDED)) &&
			m_tree.GetRootItem() != hItem &&
			hItem != NULL)
		{
			DeleteSubTree(hItem);

			HTREEITEM hItemTemp = hItem;
			CString szKeyPath, szKeyPathTemp;
			while (1)
			{
				CString szText = m_tree.GetItemText(hItemTemp);

				if (!szText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					szKeyPath = CLASSES_ROOT;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					szKeyPath = m_szHKeyCurrentUser;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					szKeyPath = LOCAL_MACHINE;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_USERS"))
				{
					szKeyPath = USERS;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					szKeyPath = CURRENT_CONFIGL;
					break;
				}
				else
				{
					szKeyPathTemp = szText + L"\\" + szKeyPathTemp;
				}

				hItemTemp = m_tree.GetParentItem(hItemTemp);
			}

			szKeyPath = szKeyPath + L"\\" + szKeyPathTemp;
			EnumSubKeys(szKeyPath, hItem);
			EnumSubSubKeys(szKeyPath, hItem);
		}
	}

	*pResult = 0;
}

BOOL CRegistryDlg::WCharToMChar(IN LPCWSTR lpcwszStr, IN OUT LPSTR* ppszStr)
{
	if (NULL == lpcwszStr)
	{
		return FALSE;
	}

	char *lpszStr = NULL;
	DWORD dwNum = WideCharToMultiByte(CP_ACP,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	lpszStr = (char *)new char[dwNum];
	if (!lpszStr)
	{
		return FALSE;
	}
	*ppszStr = lpszStr;

	memset(lpszStr,0,dwNum);
	WideCharToMultiByte(CP_ACP,NULL,lpcwszStr,-1,lpszStr,dwNum,NULL,FALSE);
	lpszStr[dwNum-1] = '\0';

	return TRUE;
}

void CRegistryDlg::OnNMRclickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (!g_bLoadDriverOK)
	{
		return;
	}

	CPoint point;

	GetCursorPos(&point);
	m_tree.ScreenToClient(&point);

	m_RightClickItem = m_tree.HitTest(point, NULL);
	CString szItemText;

	if ( m_RightClickItem != NULL )
	{ 
		szItemText = m_tree.GetItemText(m_RightClickItem);
		m_tree.SelectItem(m_RightClickItem);
	}
	
	if (m_RightClickItem == NULL ||
		!szItemText.CompareNoCase(szMyComputer[g_enumLang]))
	{
		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, ID_REG_REFRESH, szRefresh[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_LOOK_FOR, szRegFind[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_INPORT, szRegImport[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_USE_HIVE, szUseHiveAnalyze[g_enumLang]);
// 		menu.AppendMenu(MF_SEPARATOR);
// 		menu.AppendMenu(MF_STRING, ID_REG_LOOK_FOR, szRegFind[g_enumLang]);
// 		menu.AppendMenu(MF_SEPARATOR);
// 		menu.AppendMenu(MF_STRING, ID_REG_EXPORT, szRegExport[g_enumLang]);

		int x = GetSystemMetrics(SM_CXMENUCHECK);
		int y = GetSystemMetrics(SM_CYMENUCHECK);
		if (x >= 15 && y >= 15)
		{
			menu.SetMenuItemBitmaps(ID_REG_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
			menu.SetMenuItemBitmaps(ID_REG_INPORT, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
		}

		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
		menu.DestroyMenu();
	}
	else
	{
		CMenu exportto;
		exportto.CreatePopupMenu();
		exportto.AppendMenu(MF_STRING, ID_STRING_VALUE, szStringValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_BINARY_VALUE, szBinaryValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_DWORD_VALUE, szDwordValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_MUI_STRING_VALUE, szMultiStringValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_EXPAND_STRING_VALUE, szExpandableStringValue[g_enumLang]);

		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, ID_REG_REFRESH, szRefresh[g_enumLang]);
 		menu.AppendMenu(MF_SEPARATOR);
 		menu.AppendMenu(MF_STRING, ID_REG_LOOK_FOR, szRegFind[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_QUERY_LOCK_INFO, szKeyLookForLockInfo[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_INPORT, szRegImport[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_REG_EXPORT, szRegExport[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_DELETE, szRegDelete[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_REG_RENAME, szRegRename[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_NEW_KEY, szNewKey[g_enumLang]);
		menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szNewValue[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_COPY_KEY_NAME, szCopyKeyName[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_REG_COPY_FULL_KEY_NAME, szCopyFullKeyName[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_ADD_TO_QUICK_ADDRESS, szAddToQuickPositioning[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_USE_HIVE, szUseHiveAnalyze[g_enumLang]);

		int x = GetSystemMetrics(SM_CXMENUCHECK);
		int y = GetSystemMetrics(SM_CYMENUCHECK);
		if (x >= 15 && y >= 15)
		{
			menu.SetMenuItemBitmaps(ID_REG_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
			menu.SetMenuItemBitmaps(ID_REG_INPORT, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
			menu.SetMenuItemBitmaps(ID_REG_EXPORT, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
			menu.SetMenuItemBitmaps(ID_REG_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
			menu.SetMenuItemBitmaps(ID_REG_COPY_KEY_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
			menu.SetMenuItemBitmaps(ID_REG_COPY_FULL_KEY_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		}

		if (!szItemText.CompareNoCase(L"HKEY_CLASSES_ROOT") ||
			!szItemText.CompareNoCase(L"HKEY_CURRENT_USER") ||
			!szItemText.CompareNoCase(L"HKEY_LOCAL_MACHINE") ||
			!szItemText.CompareNoCase(L"HKEY_USERS") ||
			!szItemText.CompareNoCase(L"HKEY_CURRENT_CONFIG") )
		{
			menu.EnableMenuItem(ID_REG_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_REG_RENAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}

		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
		menu.DestroyMenu();
		exportto.DestroyMenu();
	}

	*pResult = 0;
}

void CRegistryDlg::OnRegRefresh()
{
	CString szItemText;

	if ( m_RightClickItem != NULL )
	{ 
		szItemText = m_tree.GetItemText(m_RightClickItem);
	}

	// 如果是computer这个根键，那么就直接初始化注册表
	if (m_RightClickItem == NULL ||
		!szItemText.CompareNoCase(szMyComputer[g_enumLang]))
	{
		InitRegistry();
	}
	else
	{
		// 当是打开状态才刷新
		if(m_RightClickItem != NULL && 
			(TVIS_EXPANDED & m_tree.GetItemState(m_RightClickItem, TVIS_EXPANDED)))
		{
			m_tree.Expand(m_RightClickItem, TVE_COLLAPSE);
			DeleteSubTree(m_RightClickItem);

			HTREEITEM hItemTemp = m_RightClickItem;
			CString szKeyPath, szKeyPathTemp;

			if (m_bUseHive)
			{
				m_szStatus.TrimRight('\\');
				if (!m_szStatus.IsEmpty())
				{
					if (m_szStatus.CompareNoCase(L"HKEY_USERS") && m_szStatus.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
					{
						WCHAR *szKey = (WCHAR *)malloc( (m_szStatus.GetLength() + 1 ) * sizeof(WCHAR) );
						if (szKey)
						{
							memset(szKey, 0, (m_szStatus.GetLength() + 1 ) * sizeof(WCHAR));
							wcsncpy_s(szKey, m_szStatus.GetLength() + 1, m_szStatus.GetBuffer(), m_szStatus.GetLength());

							char *mzKey = NULL;
							WCharToMChar(szKey, &mzKey);

							m_OpHive.SetItem(m_RightClickItem);
							BOOL nRet = m_OpHive.ListSubKeys_hive(mzKey);

							if (mzKey)
							{
								delete []mzKey;
								mzKey = NULL;
							}

							free(szKey);
							szKey = NULL;

							if (nRet)
							{
								m_tree.Expand(m_RightClickItem, TVE_EXPAND);
								return;
							}
						}
					}
				}
			}

			while (1)
			{
				CString szText = m_tree.GetItemText(hItemTemp);

				if (!szText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					szKeyPath = CLASSES_ROOT;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					szKeyPath = m_szHKeyCurrentUser;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					szKeyPath = LOCAL_MACHINE;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_USERS"))
				{
					szKeyPath = USERS;
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					szKeyPath = CURRENT_CONFIGL;
					break;
				}
				else
				{
					szKeyPathTemp = szText + L"\\" + szKeyPathTemp;
				}

				hItemTemp = m_tree.GetParentItem(hItemTemp);
			}

			szKeyPath = szKeyPath + L"\\" + szKeyPathTemp;
			EnumSubKeys(szKeyPath, m_RightClickItem);
			EnumSubSubKeys(szKeyPath, m_RightClickItem);

			m_tree.Expand(m_RightClickItem, TVE_EXPAND);
		}
	}
}

void CRegistryDlg::OnRegLookFor()
{
// 	if (m_pFindRegDlg)
// 	{
// 		delete m_pFindRegDlg;
// 		m_pFindRegDlg = NULL;
// 	}
	
	m_pFindRegDlg = new CRegFindDlg();
	m_pFindRegDlg->m_szSearchInKey = m_szStatus;
	m_pFindRegDlg->Create(IDD_FIND_REG_DIALOG);
	m_pFindRegDlg->ShowWindow(SW_SHOWNORMAL);
}

void CRegistryDlg::ShellExportRegister(
						 CString strItem,		//注册表项，如HKEY_CURRENT_USER\SofteWare
						 CString strFileName)	//导出的文件名（包括路径）
{
	CString strParameters = L"/e \"" + strFileName + L"\" \"" + strItem + L"\"";
	ShellExecute(0, L"open", L"regedit.exe", strParameters, NULL, SW_SHOWNORMAL);
}

void CRegistryDlg::OnRegExport()
{
	CString szItemText;

	if ( m_RightClickItem != NULL )
	{ 
		szItemText = m_tree.GetItemText(m_RightClickItem);

		if (szItemText.CompareNoCase(szMyComputer[g_enumLang]))
		{
			HTREEITEM hItemTemp = m_RightClickItem;
			CString szKeyPath, szKeyPathTemp;

			while (1)
			{
				CString szText = m_tree.GetItemText(hItemTemp);

				if (!szText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					szKeyPath = L"HKEY_CLASSES_ROOT";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					szKeyPath = L"HKEY_CURRENT_USER";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					szKeyPath = L"HKEY_LOCAL_MACHINE";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_USERS"))
				{
					szKeyPath = L"HKEY_USERS";
					break;
				}
				else if (!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					szKeyPath = L"HKEY_CURRENT_CONFIG";
					break;
				}
				else
				{
					szKeyPathTemp = szText + L"\\" + szKeyPathTemp;
				}

				hItemTemp = m_tree.GetParentItem(hItemTemp);
			}

			szKeyPath = szKeyPath + L"\\" + szKeyPathTemp;
			szKeyPath.TrimRight('\\');
			
			CString szFileName = szKeyPath.Right(szKeyPath.GetLength() - szKeyPath.ReverseFind(L'\\') - 1);
			CFileDialog fileDlg( FALSE, 0, szFileName, 0, L"Registration Files (*.reg)|*.reg;|All Files (*.*)|*.*||", 0 );
			if (IDOK == fileDlg.DoModal())
			{
				CString szFilePath = fileDlg.GetFileName();
				CString szExtern = szFilePath.Right(4);
				
				DebugLog(L"szFilePath: %s, szExtern: %s", szFilePath, szExtern);

				if (szExtern.CompareNoCase(L".reg"))
				{
					szFilePath += L".reg";
				}

				if ( !PathFileExists(szFilePath) ||
					(PathFileExists(szFilePath) && 
					MessageBox(szFileExist[g_enumLang], szExportRegistry[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES &&
					DeleteFile(szFilePath)))
				{
					ShellExportRegister(szKeyPath, szFilePath);

					MessageBox(szRegExportOk[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
				}
			}
		}
	}
}

void CRegistryDlg::UpdateKey(CString szKey, HTREEITEM hItem)
{
	if ( !szKey.IsEmpty() && hItem != NULL )
	{
		DeleteSubTree(hItem);

		MZF_UNICODE_STRING unKey;

		if (m_Function.InitUnicodeString(&unKey, szKey.GetBuffer()))
		{
			HANDLE hKey;
			MZF_OBJECT_ATTRIBUTES oa;

			InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

			if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
			{
				for (ULONG i = 0; ; i++)
				{
					ULONG nRetLen = 0;
					BOOL bRet = m_Registry.EnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &nRetLen);

					if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
					{
						break;
					}
					else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
					{
						PKEY_BASIC_INFORMATION pBuffer = (PKEY_BASIC_INFORMATION)malloc(nRetLen + 0x100);
						if (pBuffer)
						{
							memset(pBuffer, 0, nRetLen + 0x100);
							bRet = m_Registry.EnumerateKey(hKey, i, KeyBasicInformation, pBuffer, nRetLen + 0x100, &nRetLen);

							if (bRet)
							{
								m_tree.InsertItem(pBuffer->Name, 1, 2, hItem, TVI_LAST);
							}

							free(pBuffer);
						}
					}
				}

				CloseHandle(hKey);
			}
			else
			{
				m_tree.DeleteItem(hItem);
			}

			m_Function.FreeUnicodeString(&unKey);
		}
	}
}

void CRegistryDlg::OnRegDelete()
{
	if (m_RightClickItem != NULL)
	{
		if (MessageBox(szAreYouSureDeleteKey[g_enumLang], szDeleteKey[g_enumLang], MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
		
		HTREEITEM hItemTemp = m_RightClickItem;
		CString szKeyPath, szKeyPathTemp;

		while (1)
		{
			CString szText = m_tree.GetItemText(hItemTemp);

			if (!szText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
			{
				szKeyPath = CLASSES_ROOT;
				break;
			}
			else if (!szText.CompareNoCase(L"HKEY_CURRENT_USER"))
			{
				szKeyPath = m_szHKeyCurrentUser;
				break;
			}
			else if (!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
			{
				szKeyPath = LOCAL_MACHINE;
				break;
			}
			else if (!szText.CompareNoCase(L"HKEY_USERS"))
			{
				szKeyPath = USERS;
				break;
			}
			else if (!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
			{
				szKeyPath = CURRENT_CONFIGL;
				break;
			}
			else
			{
				szKeyPathTemp = szText + L"\\" + szKeyPathTemp;
			}

			hItemTemp = m_tree.GetParentItem(hItemTemp);
		}

		szKeyPath = szKeyPath + L"\\" + szKeyPathTemp;
		szKeyPath.TrimRight('\\');

		m_tree.Expand(m_RightClickItem, TVE_COLLAPSE);
		m_Registry.DeleteKeyAndSubKeys(szKeyPath);
		UpdateKey(szKeyPath, m_RightClickItem);
	}
}

void CRegistryDlg::OnRegRename()
{
	CString szKeyPath = StatusString2KeyPath();
	if (szKeyPath.IsEmpty())
	{
		return;
	}

	CString szKeyName = m_tree.GetItemText(m_RightClickItem);
	CRenameKeyDlg RenameKeyDlg;
	RenameKeyDlg.m_nDlgType = eRenameKey;
	RenameKeyDlg.m_szKeyName = szKeyName;
	if (RenameKeyDlg.DoModal() == IDOK)
	{
		CString szKeyNameTemp = RenameKeyDlg.m_szKeyName;
		if (szKeyNameTemp.CompareNoCase(szKeyName))
		{
			MZF_UNICODE_STRING unKey;
			if (m_Function.InitUnicodeString(&unKey, szKeyPath.GetBuffer()))
			{
				HANDLE hKey;
				MZF_OBJECT_ATTRIBUTES oa;

				InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

				if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
				{
					MZF_UNICODE_STRING unNewKey;
					if (m_Function.InitUnicodeString(&unNewKey, szKeyNameTemp.GetBuffer()))
					{
						if (m_Registry.RenameKey(hKey, &unNewKey))
						{
							m_tree.SetItemText(m_RightClickItem, szKeyNameTemp);
						}

						m_Function.FreeUnicodeString(&unNewKey);
					}
					
					CloseHandle(hKey);
				}

				m_Function.FreeUnicodeString(&unKey);
			}
		}
	}
}

void CRegistryDlg::OnRegNewKey()
{
	CString szKeyPath = StatusString2KeyPath();
	if (szKeyPath.IsEmpty())
	{
		return;
	}

	CRenameKeyDlg CreateKeyDlg;
	CString szNewKey;
	
	CreateKeyDlg.m_nDlgType = eCreateKey;
	if (CreateKeyDlg.DoModal() == IDOK)
	{
		szNewKey = CreateKeyDlg.m_szKeyName;
	}

	if (szNewKey.IsEmpty())
	{
		return;
	}

	CString szKeyPathTemp = szKeyPath + L"\\";
	szKeyPathTemp += szNewKey;

	MZF_UNICODE_STRING unKey;
	if (m_Function.InitUnicodeString(&unKey, szKeyPathTemp.GetBuffer()))
	{
		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;
		ULONG Disposition = 0;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (m_Registry.CreateKey(&hKey, KEY_ALL_ACCESS, &oa, 0, NULL, 0, &Disposition))
		{
			if (Disposition == REG_OPENED_EXISTING_KEY)
			{
				MessageBox(szThisKeyAlreadyExists[g_enumLang], szCreateKey[g_enumLang], MB_ICONERROR | MB_OK);
			}
			else
			{
// 				UpdateKey(szKeyPath, m_RightClickItem);
// 				m_tree.Expand(m_RightClickItem, TVE_EXPAND);
 				
				HTREEITEM item = m_tree.GetSelectedItem();
				m_tree.InsertItem(szNewKey, 1, 2, item, TVI_LAST);
			}
			
			CloseHandle(hKey);
		}

		m_Function.FreeUnicodeString(&unKey);
	}
}

void CRegistryDlg::OnRegCopyKeyName()
{
	CString szKeyName = m_szStatus.Right(m_szStatus.GetLength() - m_szStatus.ReverseFind('\\') - 1);
	if (szKeyName.IsEmpty())
	{
		szKeyName = m_szStatus;
	}

	m_Function.SetStringToClipboard(szKeyName);
}

void CRegistryDlg::OnRegCopyFullKeyName()
{
	m_Function.SetStringToClipboard(m_szStatus);
}

void CRegistryDlg::SetValueKey(ULONG Type, PVOID pData, ULONG DataSize)
{
	CString szKeyPath = StatusString2KeyPath();
	if (szKeyPath.IsEmpty())
	{
		return;
	}

	CRenameKeyDlg CreateKeyDlg;
	CString szValue;

	CreateKeyDlg.m_nDlgType = eSetValueKey;
	if (CreateKeyDlg.DoModal() == IDOK)
	{
		szValue = CreateKeyDlg.m_szKeyName;
	}

	if (szValue.IsEmpty())
	{
		return;
	}

	MZF_UNICODE_STRING unKey;
	if (m_Function.InitUnicodeString(&unKey, szKeyPath.GetBuffer()))
	{
		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			MZF_UNICODE_STRING unValue;

			if (m_Function.InitUnicodeString(&unValue, szValue.GetBuffer()))
			{
				m_Registry.SetValueKey(hKey, &unValue, 0, Type, pData, DataSize);
				m_Function.FreeUnicodeString(&unValue);
			}
			
			CloseHandle(hKey);
		}

		m_Function.FreeUnicodeString(&unKey);
	}

	OnRegListRefresh();
}

void CRegistryDlg::OnStringValue()
{
	SetValueKey(REG_SZ, NULL, 0);
}

void CRegistryDlg::OnBinaryValue()
{
	SetValueKey(REG_BINARY, NULL, 0);
}

void CRegistryDlg::OnDwordValue()
{
	DWORD dwValue = 0;
	SetValueKey(REG_DWORD, &dwValue, sizeof(DWORD));
}

void CRegistryDlg::OnMuiStringValue()
{
	SetValueKey(REG_MULTI_SZ, NULL, 0);
}

void CRegistryDlg::OnExpandStringValue()
{
	SetValueKey(REG_EXPAND_SZ, NULL, 0);
}

void CRegistryDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (!g_bLoadDriverOK)
	{
		return;
	}

	if (!m_szStatus.IsEmpty())
	{
		CMenu exportto;
		exportto.CreatePopupMenu();
		exportto.AppendMenu(MF_STRING, ID_STRING_VALUE, szStringValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_BINARY_VALUE, szBinaryValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_DWORD_VALUE, szDwordValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_MUI_STRING_VALUE, szMultiStringValue[g_enumLang]);
		exportto.AppendMenu(MF_STRING, ID_EXPAND_STRING_VALUE, szExpandableStringValue[g_enumLang]);

		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, ID_REG_LIST_REFRESH, szRefresh[g_enumLang]);
// 		menu.AppendMenu(MF_SEPARATOR);
// 		menu.AppendMenu(MF_STRING, ID_REG_LOOK_FOR, szRegFind[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_LIST_EXPORT, szRegExport[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_NEW_KEY, szNewKey[g_enumLang]);
		menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szNewValue[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_LIST_MODIFY, szRegModify[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_REG_LIST_DELETE, szRegDelete[g_enumLang]);
 		menu.AppendMenu(MF_STRING, ID_REG_LIST_RENAME, szRegRename[g_enumLang]);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_LIST_COPY_VALUE, szRegCopyValueName[g_enumLang]);
		menu.AppendMenu(MF_STRING, ID_REG_LIST_COPY_VALUE_DATA, szRegCopyValueData[g_enumLang]);
		
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_USE_HIVE, szUseHiveAnalyze[g_enumLang]);

		int x = GetSystemMetrics(SM_CXMENUCHECK);
		int y = GetSystemMetrics(SM_CYMENUCHECK);
		if (x >= 15 && y >= 15)
		{
			menu.SetMenuItemBitmaps(ID_REG_LIST_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
			menu.SetMenuItemBitmaps(ID_REG_LIST_EXPORT, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
			menu.SetMenuItemBitmaps(ID_REG_LIST_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
			menu.SetMenuItemBitmaps(ID_REG_LIST_COPY_VALUE, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
			menu.SetMenuItemBitmaps(ID_REG_LIST_COPY_VALUE_DATA, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		}

		if (m_list.GetSelectedCount() == 0)
		{
			menu.EnableMenuItem(ID_REG_LIST_MODIFY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_REG_LIST_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_REG_LIST_RENAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_REG_LIST_COPY_VALUE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_REG_LIST_COPY_VALUE_DATA, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Function.GetSelectItem(&m_list);
			if (nItem != -1)
			{
				if (m_list.GetItemData(nItem))
				{
					menu.EnableMenuItem(ID_REG_LIST_RENAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

					if (!(m_list.GetItemText(nItem, 2)).CompareNoCase(szValueNoSet[g_enumLang]))
					{
						menu.EnableMenuItem(ID_REG_LIST_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					}
				}

// 				CString szData = m_list.GetItemText(nItem, 2);
// 				if (!szData.CompareNoCase(szValueNoSet[g_enumLang]))
// 				{
// 					menu.EnableMenuItem(ID_REG_LIST_MODIFY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
// 					menu.EnableMenuItem(ID_REG_LIST_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
//  				menu.EnableMenuItem(ID_REG_LIST_RENAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
// 				}
			}
		}

		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
		menu.DestroyMenu();
		exportto.DestroyMenu();
	}
	else
	{
		CMenu menu;
		menu.CreatePopupMenu();
// 		menu.AppendMenu(MF_STRING, ID_REG_LOOK_FOR, szRegFind[g_enumLang]);
// 		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_REG_USE_HIVE, szUseHiveAnalyze[g_enumLang]);
		
		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
		menu.DestroyMenu();
	}

	*pResult = 0;
}

CString CRegistryDlg::StatusString2KeyPath()
{
	CString szHead = m_szStatus.Left(m_szStatus.Find('\\'));
	CString szRight = m_szStatus.Right(m_szStatus.GetLength() - m_szStatus.Find('\\'));
	CString szKeyPath;

	if (szHead.IsEmpty())
	{
		szHead = szRight;
		szRight.Empty();
	}

	if (!szHead.CompareNoCase(L"HKEY_CLASSES_ROOT"))
	{
		szKeyPath = CLASSES_ROOT;
	}
	else if (!szHead.CompareNoCase(L"HKEY_CURRENT_USER"))
	{
		szKeyPath = m_szHKeyCurrentUser;
	}
	else if (!szHead.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
	{
		szKeyPath = LOCAL_MACHINE;
	}
	else if (!szHead.CompareNoCase(L"HKEY_USERS"))
	{
		szKeyPath = USERS;
	}
	else if (!szHead.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
	{
		szKeyPath = CURRENT_CONFIGL;
	}

	szKeyPath += szRight;

	return szKeyPath;
}

void CRegistryDlg::OnRegListRefresh()
{
	if (m_bUseHive)
	{
		m_szStatus.TrimRight('\\');
		if (!m_szStatus.IsEmpty())
		{
			if (m_szStatus.CompareNoCase(L"HKEY_USERS") && m_szStatus.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
			{
				WCHAR *szKey = (WCHAR *)malloc( (m_szStatus.GetLength() + 1 ) * sizeof(WCHAR) );
				if (szKey)
				{
					memset(szKey, 0, (m_szStatus.GetLength() + 1 ) * sizeof(WCHAR));
					wcsncpy_s(szKey, m_szStatus.GetLength() + 1, m_szStatus.GetBuffer(), m_szStatus.GetLength());

					char *mzKey = NULL;
					WCharToMChar(szKey, &mzKey);

					int nRet = m_OpHive.ListValues_hive(mzKey);

					if (mzKey)
					{
						delete []mzKey;
						mzKey = NULL;
					}

					free(szKey);
					szKey = NULL;

					if (nRet == ReturnType_OK)
					{
						return;
					}
				}
			}
		}
	}

	CString szKeyPath = StatusString2KeyPath();
	EnumValues(szKeyPath);
}

void CRegistryDlg::OnRegListExport()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem == -1)
	{
		return;
	}

	CString szKeyPath = m_szStatus;
	CString szFileName = szKeyPath.Right(szKeyPath.GetLength() - szKeyPath.ReverseFind(L'\\') - 1);
	CFileDialog fileDlg( FALSE, 0, szFileName, 0, L"Registration Files (*.reg)|*.reg;|All Files (*.*)|*.*||", 0 );
	
	if (IDOK == fileDlg.DoModal())
	{
		CString szFilePath = fileDlg.GetFileName();
		CString szExtern = szFilePath.Right(4);

		if (szExtern.CompareNoCase(L".reg"))
		{
			szFilePath += L".reg";
		}

		if ( !PathFileExists(szFilePath) ||
			(PathFileExists(szFilePath) && 
			MessageBox(szFileExist[g_enumLang], szExportRegistry[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES &&
			DeleteFile(szFilePath)))
		{
			ExportReGIStry(
				szKeyPath.Left(szKeyPath.Find('\\')),
				szKeyPath.Right(szKeyPath.GetLength() - szKeyPath.Find('\\') - 1),
				szFilePath,
				m_list.GetItemText(nItem, 0));

			MessageBox(szRegExportOk[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
		}
	}
}

void CRegistryDlg::ModifyValue(CString szValue, ULONG Type, PVOID pData, ULONG DataSize)
{
	CString szKeyPath = StatusString2KeyPath();

	if (!szKeyPath.IsEmpty())
	{
		MZF_UNICODE_STRING unKey;
		if (m_Function.InitUnicodeString(&unKey, szKeyPath.GetBuffer()))
		{
			HANDLE hKey;
			MZF_OBJECT_ATTRIBUTES oa;

			InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

			if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
			{
				if (szValue.IsEmpty())
				{
					MZF_UNICODE_STRING unValue;
					unValue.Buffer = NULL;
					unValue.Length = 0;
					unValue.MaximumLength = 2;

					if (m_Registry.SetValueKey(hKey, &unValue, 0, Type, pData, DataSize))
					{
						DebugLog(L"SetValueKey Success");
					}
				}
				else 
				{
					MZF_UNICODE_STRING unValue;
					if (m_Function.InitUnicodeString(&unValue, szValue.GetBuffer()))
					{
						m_Registry.SetValueKey(hKey, &unValue, 0, Type, pData, DataSize);
						m_Function.FreeUnicodeString(&unValue);
					}
				}

				CloseHandle(hKey);
			}

			m_Function.FreeUnicodeString(&unKey);
		}
	}
}

ULONG CRegistryDlg::GetValueType(CString szType)
{
	ULONG nRet = 0;
	if (szType.IsEmpty())
	{
		return 0;
	}

	if (!szType.CompareNoCase(L"REG_NONE"))
	{
		nRet = REG_NONE;
	}
	else if (!szType.CompareNoCase(L"REG_SZ"))
	{
		nRet = REG_SZ;
	}
	else if (!szType.CompareNoCase(L"REG_EXPAND_SZ"))
	{
		nRet = REG_EXPAND_SZ;
	}
	else if (!szType.CompareNoCase(L"REG_BINARY"))
	{
		nRet = REG_BINARY;
	}
	else if (!szType.CompareNoCase(L"REG_DWORD"))
	{
		nRet = REG_DWORD;
	}
	else if (!szType.CompareNoCase(L"REG_DWORD_BIG_ENDIAN"))
	{
		nRet = REG_DWORD_BIG_ENDIAN;
	}
	else if (!szType.CompareNoCase(L"REG_LINK"))
	{
		nRet = REG_LINK;
	}
	else if (!szType.CompareNoCase(L"REG_MULTI_SZ"))
	{
		nRet = REG_MULTI_SZ;
	}
	else if (!szType.CompareNoCase(L"REG_RESOURCE_LIST"))
	{
		nRet = REG_RESOURCE_LIST;
	}
	else if (!szType.CompareNoCase(L"REG_FULL_RESOURCE_DESCRIPTOR"))
	{
		nRet = REG_FULL_RESOURCE_DESCRIPTOR;
	}
	else if (!szType.CompareNoCase(L"REG_RESOURCE_REQUIREMENTS_LIST"))
	{
		nRet = REG_RESOURCE_REQUIREMENTS_LIST;
	}
	else if (!szType.CompareNoCase(L"REG_QWORD"))
	{
		nRet = REG_QWORD;
	}
	else if (!szType.CompareNoCase(L"Unknow"))
	{
		nRet = 0;
	}

	return nRet;
}

void CRegistryDlg::OnRegListModify()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szType = m_list.GetItemText(nItem, 1);
		
		if (szType.Find(L"SZ") != -1)
		{
			CString szValue = m_list.GetItemText(nItem, 0);
			CString szData = m_list.GetItemText(nItem, 2);

			if (m_list.GetItemData(nItem) && !szData.CompareNoCase(szValueNoSet[g_enumLang]))
			{
				szData = L"";
			}

			CRegModifyDlg RegModifyDlg;
			RegModifyDlg.m_szValueData = szData;
			RegModifyDlg.m_szValueName = szValue;

			if (RegModifyDlg.DoModal() == IDOK)
			{
				CString szDataNew = RegModifyDlg.m_szValueData;
				if (m_list.GetItemData(nItem))
				{
					szValue = L"";
				}
				
				ModifyValue(szValue, GetValueType(m_list.GetItemText(nItem, 1)), szDataNew.GetBuffer(), szDataNew.GetLength() * sizeof(WCHAR));
				OnRegListRefresh();
			}
		}
		else if (!szType.CompareNoCase(L"REG_DWORD"))
		{
			CString szValue = m_list.GetItemText(nItem, 0);
			CString szData = m_list.GetItemText(nItem, 2);

			CString szShowData;
			szShowData.Format(L"%X", m_Function.HexStringToLong(szData));

			CModifyDwordDlg RegModifyDlg;
			RegModifyDlg.m_szValueData = szShowData;
			RegModifyDlg.m_szValueName = szValue;

			if (RegModifyDlg.DoModal() == IDOK)
			{
				CString szDataNew = RegModifyDlg.m_szValueData;
				int nRadio = RegModifyDlg.m_nRadio;
				DWORD dwData = 0;

				if (!nRadio) // hex
				{
					swscanf_s(szDataNew.GetBuffer(0), L"%x", &dwData);
				}
				else if (nRadio == 1) // dex
				{
					swscanf_s(szDataNew.GetBuffer(0), L"%d", &dwData);
				}

				ModifyValue(szValue, GetValueType(m_list.GetItemText(nItem, 1)), &dwData, sizeof(DWORD));
				OnRegListRefresh();
			}
		}
		else if (!szType.CompareNoCase(L"REG_BINARY") ||
			!szType.CompareNoCase(L"REG_RESOURCE_REQUIREMENTS_LIST") ||
			!szType.CompareNoCase(L"REG_FULL_RESOURCE_DESCRIPTOR") ||
			!szType.CompareNoCase(L"REG_RESOURCE_LIST"))
		{
			CString szValue = m_list.GetItemText(nItem, 0);
			CString szData = m_list.GetItemText(nItem, 2);
			ULONG nLen = szData.GetLength() + szData.GetLength() % 2 + 10;
			PBYTE pData = (PBYTE)malloc(nLen);
			ULONG nDataLen = 0;
			
			if (pData)
			{
				memset(pData, 0, nLen);
				int n = szData.Find(' ');
				while (n != -1)
				{
					swscanf_s(szData, L"%2x", &pData[nDataLen++]);
					szData = szData.Right(szData.GetLength() - n - 1);
					n = szData.Find(' ');
				}
			
				CHexEditDlg EditDlg;
				EditDlg.m_pData = pData;
				EditDlg.m_nDataLen = nDataLen;
				EditDlg.m_szValueName = szValue;

				if (EditDlg.DoModal() == IDOK)
				{
					PBYTE pRetData = EditDlg.m_pRetData;
					ULONG nRetLen = EditDlg.m_nRetLen;
					if (pRetData && nRetLen)
					{
						ModifyValue(szValue, GetValueType(m_list.GetItemText(nItem, 1)), pRetData, nRetLen);
						OnRegListRefresh();
					}

					if (pRetData)
					{
						free(pRetData);
					}
				}

				if (pData)
				{
					free(pData);
					pData = NULL;
				}
			}
		}
	}
}

void CRegistryDlg::OnRegListDelete()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szValue = m_list.GetItemText(nItem, 0);
		CString szKeyPath = StatusString2KeyPath();

		if (!szKeyPath.IsEmpty())
		{
			if (MessageBox(szAreYouSureDeleteValue[g_enumLang], szDeleteValue[g_enumLang], MB_ICONQUESTION | MB_YESNO) != IDYES)
			{
				return;
			}

			MZF_UNICODE_STRING unKey;
			BOOL bOk = FALSE;
			if (m_Function.InitUnicodeString(&unKey, szKeyPath.GetBuffer()))
			{
				HANDLE hKey;
				MZF_OBJECT_ATTRIBUTES oa;

				InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

				if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
				{
					MZF_UNICODE_STRING unValue;
					
					if (m_list.GetItemData(nItem))
					{
						unValue.Buffer = NULL;
						unValue.Length = 0;
						unValue.MaximumLength = 2;

						bOk = m_Registry.DeleteValueKey(hKey, &unValue);
					}
					else
					{
						if (m_Function.InitUnicodeString(&unValue, szValue.GetBuffer()))
						{
							bOk = m_Registry.DeleteValueKey(hKey, &unValue);
							m_Function.FreeUnicodeString(&unValue);
						//	m_list.DeleteItem(nItem);
						}
					}
					
					CloseHandle(hKey);
				}

				m_Function.FreeUnicodeString(&unKey);
			}

			if (bOk)
			{
				OnRegListRefresh();
			}
		}
	}
}

void CRegistryDlg::CreateValueKey(CString szValue, ULONG Type, PVOID pData, ULONG DataSize)
{
	CString szKeyPath = StatusString2KeyPath();
	if (szKeyPath.IsEmpty() || szValue.IsEmpty())
	{
		return;
	}

	MZF_UNICODE_STRING unKey;
	if (m_Function.InitUnicodeString(&unKey, szKeyPath.GetBuffer()))
	{
		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			MZF_UNICODE_STRING unValue;

			if (m_Function.InitUnicodeString(&unValue, szValue.GetBuffer()))
			{
				m_Registry.SetValueKey(hKey, &unValue, 0, Type, pData, DataSize);
				m_Function.FreeUnicodeString(&unValue);
			}

			CloseHandle(hKey);
		}

		m_Function.FreeUnicodeString(&unKey);
	}

	OnRegListRefresh();
}

void CRegistryDlg::OnRegListRename()
{
	CString szKeyPath = StatusString2KeyPath();
	if (szKeyPath.IsEmpty())
	{
		return;
	}

	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CRenameKeyDlg RenameValueDlg;
		RenameValueDlg.m_nDlgType = eRenameValue;
		CString szValueNameOld = m_list.GetItemText(nItem, 0);
		RenameValueDlg.m_szKeyName = szValueNameOld;
		CString szNewValueName;
		if (RenameValueDlg.DoModal() == IDOK)
		{
			szNewValueName = RenameValueDlg.m_szKeyName;
		}

		if (szNewValueName.IsEmpty())
		{
			return;
		}
		
		if (szNewValueName.CompareNoCase(szValueNameOld))
		{
			PKEY_VALUE_FULL_INFORMATION pValueInfo = (PKEY_VALUE_FULL_INFORMATION)GetValueInfo(szKeyPath, szValueNameOld);
			if (pValueInfo)
			{
				CreateValueKey(szNewValueName, pValueInfo->Type, (PVOID)((PBYTE)pValueInfo + pValueInfo->DataOffset), pValueInfo->DataLength);

				// 删除旧的键值
				MZF_UNICODE_STRING unKey;
				if (m_Function.InitUnicodeString(&unKey, szKeyPath.GetBuffer()))
				{
					HANDLE hKey;
					MZF_OBJECT_ATTRIBUTES oa;

					InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);

					if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
					{
						MZF_UNICODE_STRING unValue;

						if (m_Function.InitUnicodeString(&unValue, szValueNameOld.GetBuffer()))
						{
							m_Registry.DeleteValueKey(hKey, &unValue);
							m_Function.FreeUnicodeString(&unValue);
							m_list.DeleteItem(nItem);
						}

						CloseHandle(hKey);
					}

					m_Function.FreeUnicodeString(&unKey);
				}

				// 释放资源
				free(pValueInfo);
				pValueInfo = NULL;
			}
		}
	}	
}

void CRegistryDlg::OnRegListCopyValue()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szValue = m_list.GetItemText(nItem, 0);
		m_Function.SetStringToClipboard(szValue);
	}
}

void CRegistryDlg::OnRegListCopyValueData()
{
	int nItem = m_Function.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szData = m_list.GetItemText(nItem, 2);
		m_Function.SetStringToClipboard(szData);
	}
}

HTREEITEM CRegistryDlg::GetTreeSubItemByName(HTREEITEM PatentItem, CString szName)
{
	HTREEITEM hChild = m_tree.GetChildItem(PatentItem);
	HTREEITEM RetItem = NULL;

	while(hChild != NULL)
	{
		CString szKey = m_tree.GetItemText(hChild);

		if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}
		else if (!szKey.CompareNoCase(szName))
		{
			RetItem = hChild;
			break;
		}

		hChild = m_tree.GetNextSiblingItem(hChild);
	}
	
	return RetItem;
}

void CRegistryDlg::OnBnClickedGoto()
{
	UpdateData(TRUE);
	m_ComboBox.ResetContent();
	m_ComboBox.SetWindowText(m_szComboText);

	// 首先保存是否使用hive的原始值
	BOOL bUseHive = m_bUseHive;

	// 然后去掉使用hive
	m_bUseHive = FALSE;

	if (m_szComboText.IsEmpty())
	{
		m_bUseHive = bUseHive;
		return;
	}

	CString szKeyPath = ComboString2KeyPath();
	if (szKeyPath.IsEmpty())
	{
		m_bUseHive = bUseHive;
		return;
	}

	CString szHead = m_szComboText.Left(m_szComboText.Find(L"\\"));
	if (szHead.IsEmpty())
	{
		szHead = m_szComboText;
	}
			
	if (szHead.IsEmpty())
	{
		m_bUseHive = bUseHive;
		return;
	}

	HTREEITEM RootItem = m_tree.GetRootItem();
	if (RootItem == NULL)
	{
		m_bUseHive = bUseHive;
		return;
	}

	HTREEITEM hChild = m_tree.GetChildItem(RootItem);
	CString szRoot;

	WCHAR szHkeyRoot[] = {'H','K','E','Y','_','C','L','A','S','S','E','S','_','R','O','O','T','\0'};
	WCHAR szHKCR[] = {'H','K','C','R','\0'};

	WCHAR szHkeyCurrentUser[] = {'H','K','E','Y','_','C','U','R','R','E','N','T','_','U','S','E','R','\0'};
	WCHAR szHKCU[] = {'H','K','C','U','\0'};

	WCHAR szLocalMachine[] = {'H','K','E','Y','_','L','O','C','A','L','_','M','A','C','H','I','N','E','\0'};
	WCHAR szHKLM[] = {'H','K','L','M','\0'};

	WCHAR szHKEY_USERS[] = {'H','K','E','Y','_','U','S','E','R','S','\0'};
	WCHAR szHKU[] = {'H','K','U','\0'};

	WCHAR szHKEY_CURRENT_CONFIG[] = {'H','K','E','Y','_','C','U','R','R','E','N','T','_','C','O','N','F','I','G','\0'};
	WCHAR szHKCC[] = {'H','K','C','C','\0'};

	while(hChild != NULL)
	{
		CString szKey = m_tree.GetItemText(hChild);

		if ((!szHead.CompareNoCase(szHkeyRoot) || !szHead.CompareNoCase(szHKCR)) &&
			!szKey.CompareNoCase(szHkeyRoot))
		{
			szRoot = CLASSES_ROOT;
			break;
		}
		else if ((!szHead.CompareNoCase(szHkeyCurrentUser) || !szHead.CompareNoCase(szHKCU)) &&
			!szKey.CompareNoCase(szHkeyCurrentUser))
		{
			szRoot = m_szHKeyCurrentUser;
			break;
		}
		else if ((!szHead.CompareNoCase(szLocalMachine) || !szHead.CompareNoCase(szHKLM)) &&
			!szKey.CompareNoCase(szLocalMachine))
		{
			szRoot = LOCAL_MACHINE;
			break;
		}
		else if ((!szHead.CompareNoCase(szHKEY_USERS) || !szHead.CompareNoCase(szHKU)) &&
			!szKey.CompareNoCase(szHKEY_USERS))
		{
			szRoot = USERS;
			break;
		}
		else if ((!szHead.CompareNoCase(szHKEY_CURRENT_CONFIG) || !szHead.CompareNoCase(szHKCC)) &&
			!szKey.CompareNoCase(szHKEY_CURRENT_CONFIG))
		{
			szRoot = CURRENT_CONFIGL;
			break;
		}

		hChild = m_tree.GetNextSiblingItem(hChild);
	}

	if (hChild == NULL || szRoot.IsEmpty())
	{
		m_bUseHive = bUseHive;
		return;
	}

	// 首先展开次根键，类似HKEY_LOCAL_MACHINE，HKEY_USERS
	m_tree.Expand(hChild, TVE_COLLAPSE);
	EnumSubKeys(szRoot, hChild);
	m_tree.Expand(hChild, TVE_EXPAND);

	if (m_szComboText.Find(L"\\") != -1)
	{
		CString szKeyEnd = szKeyPath.GetBuffer() + szRoot.GetLength();
		szKeyPath.ReleaseBuffer();

		if (szKeyEnd.IsEmpty())
		{
			m_bUseHive = bUseHive;
			return;
		}

		szKeyEnd.TrimLeft('\\');

		if (szKeyEnd.IsEmpty())
		{
			m_bUseHive = bUseHive;
			return;
		}

		CString szTemp;
		BOOL bQuiet = FALSE;

		do 
		{
			szTemp = szKeyEnd.Left(szKeyEnd.Find('\\'));
			
			if (szTemp.IsEmpty())
			{
				szTemp = szKeyEnd;
				bQuiet = TRUE;
			}

			hChild = GetTreeSubItemByName(hChild, szTemp);
			if (hChild == NULL)
			{
				CString szMsgBox; 

				szMsgBox = szRegistryIsNoExsit[g_enumLang];
				szMsgBox += L" \'" + m_szComboText + L"\'"; 
				MessageBox(szMsgBox, L"Registry", MB_OK | MB_ICONINFORMATION);
				break;
			}

			szRoot += L"\\" + szTemp;
			
			if (bQuiet)
			{
				EnumValues(szRoot);
				m_tree.Expand(hChild, TVE_EXPAND);
				m_tree.Select(hChild, TVGN_FIRSTVISIBLE);
				m_tree.SelectItem(hChild);
				m_tree.SetItemState(hChild, TVIS_DROPHILITED | TVIS_BOLD, TVIS_DROPHILITED | TVIS_BOLD);
				m_hChild = hChild;
			}
			else
			{
				EnumSubKeys(szRoot, hChild);
				m_tree.Expand(hChild, TVE_EXPAND);
			}
			
			szKeyEnd = szKeyEnd.Right(szKeyEnd.GetLength() - szKeyEnd.Find('\\') - 1);

		} while (!bQuiet);

		if (bQuiet)
		{
			UpdateHistroy(m_szComboText);
		}
	}

	// 最后恢复之
	m_bUseHive = bUseHive;
}

CString CRegistryDlg::ComboString2KeyPath()
{
	CString szHead = m_szComboText.Left(m_szComboText.Find('\\'));
	CString szRight = m_szComboText.Right(m_szComboText.GetLength() - m_szComboText.Find('\\'));
	CString szKeyPath;

	if (szHead.IsEmpty())
	{
		szHead = szRight;
		szRight.Empty();
	}

	WCHAR szHkeyRoot[] = {'H','K','E','Y','_','C','L','A','S','S','E','S','_','R','O','O','T','\0'};
	WCHAR szHKCR[] = {'H','K','C','R','\0'};

	WCHAR szHkeyCurrentUser[] = {'H','K','E','Y','_','C','U','R','R','E','N','T','_','U','S','E','R','\0'};
	WCHAR szHKCU[] = {'H','K','C','U','\0'};

	WCHAR szLocalMachine[] = {'H','K','E','Y','_','L','O','C','A','L','_','M','A','C','H','I','N','E','\0'};
	WCHAR szHKLM[] = {'H','K','L','M','\0'};

	WCHAR szHKEY_USERS[] = {'H','K','E','Y','_','U','S','E','R','S','\0'};
	WCHAR szHKU[] = {'H','K','U','\0'};

	WCHAR szHKEY_CURRENT_CONFIG[] = {'H','K','E','Y','_','C','U','R','R','E','N','T','_','C','O','N','F','I','G','\0'};
	WCHAR szHKCC[] = {'H','K','C','C','\0'};

	if (!szHead.CompareNoCase(szHkeyRoot) || !szHead.CompareNoCase(szHKCR))
	{
		szKeyPath = CLASSES_ROOT;
	}
	else if (!szHead.CompareNoCase(szHkeyCurrentUser) || !szHead.CompareNoCase(szHKCU))
	{
		szKeyPath = m_szHKeyCurrentUser;
	}
	else if (!szHead.CompareNoCase(szLocalMachine) || !szHead.CompareNoCase(szHKLM))
	{
		szKeyPath = LOCAL_MACHINE;
	}
	else if (!szHead.CompareNoCase(szHKEY_USERS) || !szHead.CompareNoCase(szHKU))
	{
		szKeyPath = USERS;
	}
	else if (!szHead.CompareNoCase(szHKEY_CURRENT_CONFIG) || !szHead.CompareNoCase(szHKCC))
	{
		szKeyPath = CURRENT_CONFIGL;
	}

	szKeyPath += szRight;
	szKeyPath.TrimRight('\\');

	return szKeyPath;
}

CString CRegistryDlg::Key2KeyPath(CString szKey)
{
	CString szHead = szKey.Left(szKey.Find('\\'));
	CString szRight = szKey.Right(szKey.GetLength() - szKey.Find('\\'));
	CString szKeyPath;

	if (szHead.IsEmpty())
	{
		szHead = szRight;
		szRight.Empty();
	}

	if (!szHead.CompareNoCase(L"HKEY_CLASSES_ROOT"))
	{
		szKeyPath = CLASSES_ROOT;
	}
	else if (!szHead.CompareNoCase(L"HKEY_CURRENT_USER"))
	{
		szKeyPath = m_szHKeyCurrentUser;
	}
	else if (!szHead.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
	{
		szKeyPath = LOCAL_MACHINE;
	}
	else if (!szHead.CompareNoCase(L"HKEY_USERS"))
	{
		szKeyPath = USERS;
	}
	else if (!szHead.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
	{
		szKeyPath = CURRENT_CONFIGL;
	}

	szKeyPath += szRight;
	szKeyPath.TrimRight('\\');

	return szKeyPath;
}

// 项是以HKEY_LOCAL_MACHINE这种形式存在
void CRegistryDlg::JmpToReg_HKEY_Xxx(CString szKey, CString szData)
{
	HTREEITEM RootItem = m_tree.GetRootItem();
	if (RootItem == NULL)
	{
		return;
	}

	HTREEITEM hChild = m_tree.GetChildItem(RootItem);
	CString szRoot;

	while(hChild != NULL)
	{
		CString szKeyTemp = m_tree.GetItemText(hChild);

		if (!_wcsnicmp(szKey.GetBuffer(), L"HKEY_CLASSES_ROOT", wcslen(L"HKEY_CLASSES_ROOT")) &&
			!szKeyTemp.CompareNoCase(L"HKEY_CLASSES_ROOT"))
		{
			szRoot = CLASSES_ROOT;
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), L"HKEY_CURRENT_USER", wcslen(L"HKEY_CURRENT_USER")) &&
			!szKeyTemp.CompareNoCase(L"HKEY_CURRENT_USER"))
		{
			szRoot = m_szHKeyCurrentUser;
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), L"HKEY_LOCAL_MACHINE", wcslen(L"HKEY_LOCAL_MACHINE")) &&
			!szKeyTemp.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
		{
			szRoot = LOCAL_MACHINE;
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), L"HKEY_USERS", wcslen(L"HKEY_USERS")) &&
			!szKeyTemp.CompareNoCase(L"HKEY_USERS"))
		{
			szRoot = USERS;
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), L"HKEY_CURRENT_CONFIG", wcslen(L"HKEY_CURRENT_CONFIG")) &&
			!szKeyTemp.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
		{
			szRoot = CURRENT_CONFIGL;
			break;
		}

		hChild = m_tree.GetNextSiblingItem(hChild);
	}

	if (hChild == NULL || szRoot.IsEmpty())
	{
		return;
	}

	m_tree.Expand(hChild, TVE_COLLAPSE);
	EnumSubKeys(szRoot, hChild);
	m_tree.Expand(hChild, TVE_EXPAND);

	if (szKey.Find(L"\\") != -1)
	{
		CString szKeyEnd = szKey.Right(szKey.GetLength() - szKey.Find('\\') - 1);

		if (szKeyEnd.IsEmpty())
		{
			return;
		}

		szKeyEnd.TrimLeft('\\');

		if (szKeyEnd.IsEmpty())
		{
			return;
		}

		CString szTemp;
		BOOL bQuiet = FALSE;

		do 
		{
			szTemp = szKeyEnd.Left(szKeyEnd.Find('\\'));

			if (szTemp.IsEmpty())
			{
				szTemp = szKeyEnd;
				bQuiet = TRUE;
			}

			hChild = GetTreeSubItemByName(hChild, szTemp);
			if (hChild == NULL)
			{
				CString szMsgBox; 

				szMsgBox = szRegistryIsNoExsit[g_enumLang];
				szMsgBox += L" \'" + szKey + L"\'"; 
				MessageBox(szMsgBox, L"Registry", MB_OK | MB_ICONINFORMATION);
				break;
			}

			szRoot += L"\\" + szTemp;

			if (bQuiet)
			{
				EnumValues(szRoot);
				m_tree.Expand(hChild, TVE_EXPAND);
				m_tree.Select(hChild, TVGN_FIRSTVISIBLE);
				m_tree.SelectItem(hChild);
				m_tree.SetItemState(hChild, TVIS_DROPHILITED | TVIS_BOLD, TVIS_DROPHILITED | TVIS_BOLD);
				m_hChild = hChild;

				if (!szData.IsEmpty())
				{
					DWORD dwCnt = m_list.GetItemCount();
					for (DWORD i = 0; i < dwCnt; i++)
					{
						if (!(m_list.GetItemText(i, 0)).CompareNoCase(szData))
						{
							m_list.EnsureVisible(i, false);
							m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);
							m_list.SetFocus();
							break;
						}
					}
				}	

				break;
			}
			else
			{
				EnumSubKeys(szRoot, hChild);
				m_tree.Expand(hChild, TVE_EXPAND);
			}

			szKeyEnd = szKeyEnd.Right(szKeyEnd.GetLength() - szKeyEnd.Find('\\') - 1);

		} while (!bQuiet);
	}
}

// 项是以\Registry\Machine\SOFTWARE\Classes这种形式存在
void CRegistryDlg::JmpToReg_Registry_Xxx(CString szKey, CString szData)
{
	HTREEITEM RootItem = m_tree.GetRootItem();
	if (RootItem == NULL)
	{
		return;
	}

	HTREEITEM hChild = m_tree.GetChildItem(RootItem);
	CString szRoot;
	CString strRootKey;

	while(hChild != NULL)
	{
		CString szKeyTemp = m_tree.GetItemText(hChild);

		if (!_wcsnicmp(szKey.GetBuffer(), CLASSES_ROOT, wcslen(CLASSES_ROOT)) &&
			!szKeyTemp.CompareNoCase(L"HKEY_CLASSES_ROOT"))
		{
			szRoot = CLASSES_ROOT;
			strRootKey = L"HKEY_CLASSES_ROOT";
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), m_szHKeyCurrentUser, wcslen(m_szHKeyCurrentUser)) &&
			!szKeyTemp.CompareNoCase(L"HKEY_CURRENT_USER"))
		{
			szRoot = m_szHKeyCurrentUser;
			strRootKey = L"HKEY_CURRENT_USER";
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), LOCAL_MACHINE, wcslen(LOCAL_MACHINE)) &&
			!szKeyTemp.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
		{
			szRoot = LOCAL_MACHINE;
			strRootKey = L"HKEY_LOCAL_MACHINE";
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), USERS, wcslen(USERS)) &&
			!szKeyTemp.CompareNoCase(L"HKEY_USERS"))
		{
			szRoot = USERS;
			strRootKey = L"HKEY_USERS";
			break;
		}
		else if (!_wcsnicmp(szKey.GetBuffer(), CURRENT_CONFIGL, wcslen(CURRENT_CONFIGL)) &&
			!szKeyTemp.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
		{
			szRoot = CURRENT_CONFIGL;
			strRootKey = L"HKEY_CURRENT_CONFIG";
			break;
		}

		hChild = m_tree.GetNextSiblingItem(hChild);
	}

	if (hChild == NULL || szRoot.IsEmpty())
	{
		return;
	}

	m_tree.Expand(hChild, TVE_COLLAPSE);
	EnumSubKeys(szRoot, hChild);
	m_tree.Expand(hChild, TVE_EXPAND);
	
	if (szKey.Find(L"\\") != -1)
	{
		CString szKeyEndTemp = szKey.GetBuffer() + szRoot.GetLength();
		CString szKeyEnd = szKey.GetBuffer() + szRoot.GetLength();

		if (szKeyEnd.IsEmpty())
		{
			return;
		}

		szKeyEnd.TrimLeft('\\');

		if (szKeyEnd.IsEmpty())
		{
			return;
		}

		CString szTemp;
		BOOL bQuiet = FALSE;

		do 
		{
			szTemp = szKeyEnd.Left(szKeyEnd.Find('\\'));

			if (szTemp.IsEmpty())
			{
				szTemp = szKeyEnd;
				bQuiet = TRUE;
			}

			hChild = GetTreeSubItemByName(hChild, szTemp);
			if (hChild == NULL)
			{
				CString szMsgBox; 

				szMsgBox = szRegistryIsNoExsit[g_enumLang];
				szMsgBox += L" \'" + strRootKey + szKeyEndTemp + L"\'"; 
				MessageBox(szMsgBox, L"Registry", MB_OK | MB_ICONINFORMATION);
				break;
			}

			szRoot += L"\\" + szTemp;

			if (bQuiet)
			{
				EnumValues(szRoot);
				m_tree.Expand(hChild, TVE_EXPAND);
				m_tree.Select(hChild, TVGN_FIRSTVISIBLE);
				m_tree.SelectItem(hChild);
				m_tree.SetItemState(hChild, TVIS_DROPHILITED | TVIS_BOLD, TVIS_DROPHILITED | TVIS_BOLD);
				m_hChild = hChild;

				if (!szData.IsEmpty())
				{
					DWORD dwCnt = m_list.GetItemCount();
					for (DWORD i = 0; i < dwCnt; i++)
					{
						if (!(m_list.GetItemText(i, 0)).CompareNoCase(szData))
						{
							m_list.EnsureVisible(i, false);
							m_list.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);
							m_list.SetFocus();
						}
					}
				}	

				break;
			}
			else
			{
				EnumSubKeys(szRoot, hChild);
				m_tree.Expand(hChild, TVE_EXPAND);
			}

			szKeyEnd = szKeyEnd.Right(szKeyEnd.GetLength() - szKeyEnd.Find('\\') - 1);

		} while (!bQuiet);
	}
}

void CRegistryDlg::JmpToReg(CString szKey, CString szData)
{
	if (szKey.IsEmpty())
	{
		return;
	}

	if (szKey.GetAt(0) == '\\')
	{
		JmpToReg_Registry_Xxx(szKey, szData);
	}
	else
	{
		JmpToReg_HKEY_Xxx(szKey, szData);
	}
}

void CRegistryDlg::ExportValue(CString szKey, CString szValue, CString szPath)
{
	if (szKey.IsEmpty() || szValue.IsEmpty() || szPath.IsEmpty())
	{
		return;
	}

	CString szRoot;
	CString szSubKey;
	if (!_wcsnicmp(szKey.GetBuffer(), CLASSES_ROOT, wcslen(CLASSES_ROOT)))
	{
		szRoot = L"HKEY_CLASSES_ROOT";
		szSubKey = szKey.Right(szKey.GetLength() - wcslen(CLASSES_ROOT) - 1);
	}
	else if (!_wcsnicmp(szKey.GetBuffer(), m_szHKeyCurrentUser, wcslen(m_szHKeyCurrentUser)))
	{
		szRoot = L"HKEY_CURRENT_USER";
		szSubKey = szKey.Right(szKey.GetLength() - wcslen(m_szHKeyCurrentUser) - 1);
	}
	else if (!_wcsnicmp(szKey.GetBuffer(), LOCAL_MACHINE, wcslen(LOCAL_MACHINE)))
	{
		szRoot = L"HKEY_LOCAL_MACHINE";
		szSubKey = szKey.Right(szKey.GetLength() - wcslen(LOCAL_MACHINE) - 1);
	}
	else if (!_wcsnicmp(szKey.GetBuffer(), USERS, wcslen(USERS)))
	{
		szRoot = L"HKEY_USERS";
		szSubKey = szKey.Right(szKey.GetLength() - wcslen(USERS) - 1);
	}
	else if (!_wcsnicmp(szKey.GetBuffer(), CURRENT_CONFIGL, wcslen(CURRENT_CONFIGL)))
	{
		szRoot = L"HKEY_CURRENT_CONFIG";
		szSubKey = szKey.Right(szKey.GetLength() - wcslen(CURRENT_CONFIGL) - 1);
	}

	if (szRoot.IsEmpty() || szSubKey.IsEmpty())
	{
		return;
	}

	ExportReGIStry(szRoot, szSubKey, szPath, szValue);
}

/* 
描述：获取值的信息
参数：szKey - 键名
	  szValue - 值名
返回值：值的Buffer
*/
PVOID CRegistryDlg::GetValueInfo(CString szKey, CString szValue)
{
	PVOID pRet = NULL;

	if (szKey.IsEmpty() || szValue.IsEmpty())
	{
		return NULL;
	}

	MZF_UNICODE_STRING unKey;

	if (m_Function.InitUnicodeString(&unKey, szKey.GetBuffer()))
	{
		HANDLE hKey;
		MZF_OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &unKey, OBJ_CASE_INSENSITIVE, 0, NULL);
		if (m_Registry.OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
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

						if (bRet)
						{
							WCHAR szTempName[1024] = {0};
							wcsncpy_s(szTempName, 1024, pBuffer->Name, pBuffer->NameLength / sizeof(WCHAR));
							
							if (!szValue.CompareNoCase(szTempName))
							{
								pRet = (PVOID)pBuffer;
								break;
							}
						}

						free(pBuffer);
					}
				}
			}

			CloseHandle(hKey);
		}

		m_Function.FreeUnicodeString(&unKey);
	}

	return pRet;
}

void CRegistryDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnRegListModify();
	*pResult = 0;
}

void CRegistryDlg::OnRegUseHive()
{
	m_bUseHive = !m_bUseHive;
}

void CRegistryDlg::OnUpdateRegUseHive(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bUseHive);
}

void CRegistryDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	if(!bSysMenu && pPopupMenu)  
	{  
		CCmdUI cmdUI;  
		cmdUI.m_pOther = NULL;  
		cmdUI.m_pMenu = pPopupMenu;  
		cmdUI.m_pSubMenu = NULL;  

		UINT count = pPopupMenu->GetMenuItemCount();  
		cmdUI.m_nIndexMax = count;  
		for(UINT i=0; i<count; i++)  
		{  
			UINT nID = pPopupMenu->GetMenuItemID(i);  
			if(-1 == nID || 0 == nID)  
			{  
				continue;  
			}  
			cmdUI.m_nID = nID;  
			cmdUI.m_nIndex = i;  
			cmdUI.DoUpdate(this, FALSE);  
		}  
	}
}

void CRegistryDlg::OnRegAddToQuickAddress()
{
	if (!m_szStatus.IsEmpty())
	{
		g_Config.AddRegPath(m_szStatus);
		m_ComboBox.InsertString(m_nComboBoxCnt, m_szStatus);
		m_szComboText = m_szStatus;
		m_nComboBoxCnt++;

		UpdateData(FALSE);
	}
}

BOOL m_bInComboBox = FALSE;
BOOL m_bInListCtrl = FALSE;

BOOL CRegistryDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 

	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) 
	{
		if (m_bInComboBox)
		{
			return CDialog::PreTranslateMessage(pMsg);
		}
		else if (m_bInListCtrl)
		{
			OnRegListModify();
		}

		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CRegistryDlg::OnCbnSetfocusCombo()
{
	m_bInComboBox = TRUE;
}

void CRegistryDlg::OnCbnKillfocusCombo()
{
	m_bInComboBox = FALSE;
}

void CRegistryDlg::OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_bInListCtrl = TRUE;
	*pResult = 0;
}

void CRegistryDlg::OnNMKillfocusList(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_bInListCtrl = FALSE;
	*pResult = 0;
}

void CRegistryDlg::OnCbnDropdownCombo()
{
	m_ComboBox.ResetContent();
	InitializeComboBox();
}

void CRegistryDlg::OnBnClickedBack()
{
	m_bBackOrForward = TRUE;
		
	if (m_nCurrentHistroy > 0)
	{
		if (--m_nCurrentHistroy == 0)
		{
			m_btnBack.EnableWindow(FALSE);
		}

		CString szKey = m_historyVector.at(m_nCurrentHistroy);
		JmpToReg(szKey, NULL);
	}

	m_bBackOrForward = FALSE;
}

void CRegistryDlg::OnBnClickedForward()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRegistryDlg::OnRegInport()
{
	//为true，显示打开文件对话框
	CFileDialog fileDlg(TRUE);			
	fileDlg.m_ofn.lpstrTitle = L"Open Reg File";
	fileDlg.m_ofn.lpstrFilter = L"Registration Files(*.reg)\0*.reg\0\0";

	if (IDOK == fileDlg.DoModal())
	{
		CString szPath = fileDlg.GetPathName();
		ShellImportRegister(szPath);

		MessageBox(szRegImportOk[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
	}	
}

void CRegistryDlg::ShellImportRegister(
						 LPCTSTR lpImportFile)    //导入的注册表文件
{
	CString strItem(lpImportFile);
	CString strParameters;
	strParameters = L"/s \"" + strItem + L"\"";
	ShellExecute(NULL, L"open", L"regedit.exe",
		strParameters, NULL, SW_HIDE);
}

void CRegistryDlg::OnRegQueryLockInfo()
{
	if (m_RightClickItem == NULL)
	{
		return;
	}

	HTREEITEM hItemTemp = m_RightClickItem;
	CString szKeyPath, szKeyPathTemp;

	while (1)
	{
		CString szText = m_tree.GetItemText(hItemTemp);

		if (!szText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
		{
			szKeyPath = CLASSES_ROOT;
			break;
		}
		else if (!szText.CompareNoCase(L"HKEY_CURRENT_USER"))
		{
			szKeyPath = m_szHKeyCurrentUser;
			break;
		}
		else if (!szText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
		{
			szKeyPath = LOCAL_MACHINE;
			break;
		}
		else if (!szText.CompareNoCase(L"HKEY_USERS"))
		{
			szKeyPath = USERS;
			break;
		}
		else if (!szText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
		{
			szKeyPath = CURRENT_CONFIGL;
			break;
		}
		else
		{
			szKeyPathTemp = szText + L"\\" + szKeyPathTemp;
		}

		hItemTemp = m_tree.GetParentItem(hItemTemp);
	}

	szKeyPath = szKeyPath + L"\\" + szKeyPathTemp;
	szKeyPath.TrimRight('\\');
	
	LookForRegLockInfo(szKeyPath);
}

void CRegistryDlg::LookForRegLockInfo(CString szRegPath)
{
	if (szRegPath.IsEmpty())
	{
		return;
	}

	DWORD dwLen = (szRegPath.GetLength() + 1) * sizeof(WCHAR);
	WCHAR *szPath = (WCHAR*)malloc(dwLen);
	if (!szPath)
	{
		return;
	}

	memset(szPath, 0, dwLen);
	wcsncpy_s(szPath, szRegPath.GetLength() + 1, szRegPath.GetBuffer(), szRegPath.GetLength());
	szRegPath.ReleaseBuffer();

	COMMUNICATE_REG_LOCK cf;
	cf.OpType = enumQueryRegLockedInfo;
	cf.szRegPath = szPath;
	cf.nPathLen = wcslen(szPath) * sizeof(WCHAR);

	DWORD dwCnt = 0, dwCntTemp = 0;
	PLOCKED_REG_INFO pInfos = NULL;
	BOOL bRet = FALSE;

	dwCnt = dwCntTemp = 0x1000;

	do 
	{
		DebugLog(L"nRetCount: %d", dwCntTemp);
		dwCnt = dwCntTemp;
		dwLen = sizeof(LOCKED_REG_INFO) + dwCnt * sizeof(LOCKED_REGS);
		if (pInfos)
		{
			free(pInfos);
			pInfos = NULL;
		}

		pInfos = (PLOCKED_REG_INFO)malloc(dwLen);
		if (!pInfos)
		{
			free(szPath);
			return;
		}

		memset(pInfos, 0, dwLen);

		bRet = m_Driver.CommunicateDriver(&cf, sizeof(COMMUNICATE_REG_LOCK), pInfos, dwLen, NULL);

	} while ( !bRet && (dwCntTemp = pInfos->nRetCount) > dwCnt );

	if (bRet && pInfos->nRetCount > 0)
	{
		DebugLog(L"nRetCount: %d", pInfos->nRetCount);
		
		CRegUnlockDlg dlg;
		dlg.m_szRegKey = szPath;
		dlg.m_pRegLockedInfo = pInfos;
		dlg.DoModal();
	}
	else
	{
		MessageBox(szRegUnlocked[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
	}

	free(szPath);
	free(pInfos);
}