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
// MemoryInfoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "MemoryInfoDlg.h"
#include "DumpMemoryDlg.h"
#include "ModifyProtectDlg.h"
#include <algorithm>
#include <shlwapi.h>

// CMemoryInfoDlg 对话框

IMPLEMENT_DYNAMIC(CMemoryInfoDlg, CDialog)

CMemoryInfoDlg::CMemoryInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMemoryInfoDlg::IDD, pParent)
	, m_szText(_T(""))
{
	m_dwPid = 0;
	m_pEprocess = 0;
}

CMemoryInfoDlg::~CMemoryInfoDlg()
{
	m_ProcessMemoryVector.clear();
	m_ProtectTypeVector.clear();
}

void CMemoryInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_MEMORY_INFO_CNT, m_szText);
}


BEGIN_MESSAGE_MAP(CMemoryInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMemoryInfoDlg::OnBnClickedOk)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CMemoryInfoDlg::OnNMRclickList)
	ON_COMMAND(ID_MEMORY_Refresh, &CMemoryInfoDlg::OnMemoryRefresh)
	ON_COMMAND(ID_MEMORY_DUMP, &CMemoryInfoDlg::OnMemoryDump)
	ON_COMMAND(ID_MEMORY_DUMP_SELECT, &CMemoryInfoDlg::OnMemoryDumpSelect)
	ON_COMMAND(ID_MEMORY_ZERO_MEMORY, &CMemoryInfoDlg::OnMemoryZeroMemory)
	ON_COMMAND(ID_MEMORY_CHANGE_PROTECT, &CMemoryInfoDlg::OnMemoryChangeProtect)
	ON_COMMAND(ID_MEMORY_FREE_MEMORY, &CMemoryInfoDlg::OnMemoryFreeMemory)
	ON_COMMAND(ID_MEMORY_TEXT, &CMemoryInfoDlg::OnMemoryText)
	ON_COMMAND(ID_MEMORY_EXCEL, &CMemoryInfoDlg::OnMemoryExcel)
	ON_WM_SIZE()
	ON_COMMAND(ID_MEMORY_DISASSEMBLE, &CMemoryInfoDlg::OnMemoryDisassemble)
	ON_COMMAND(ID_MEMORY_HEX_EDIT, &CMemoryInfoDlg::OnMemoryHexEdit)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CMemoryInfoDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_MEMORY_INFO_CNT, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CMemoryInfoDlg 消息处理程序

void CMemoryInfoDlg::OnBnClickedOk()
{
}

BOOL CMemoryInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szAddress[g_enumLang], LVCFMT_LEFT, 95);
	m_list.InsertColumn(1, szSize[g_enumLang], LVCFMT_LEFT, 95);
	m_list.InsertColumn(2, L"Protect", LVCFMT_LEFT, 130);
	m_list.InsertColumn(3, L"State", LVCFMT_LEFT, 95);
	m_list.InsertColumn(4, L"Type", LVCFMT_LEFT, 95);
	m_list.InsertColumn(5, szModuleName[g_enumLang], LVCFMT_LEFT, 230);

	InitMemoryProtect();

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMemoryInfoDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_MEMORY_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_MEMORY_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_MEMORY_Refresh, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_HEX_EDIT, szMemoryHexEdit[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_DISASSEMBLE, szDisassemble[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_DUMP, szMemoryDump[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_MEMORY_DUMP_SELECT, szMemoryDumpSelect[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_ZERO_MEMORY, szMemoryZero[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_MEMORY_CHANGE_PROTECT, szMemoryModifyProtection[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_MEMORY_FREE_MEMORY, szMemoryFree[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_MEMORY_Refresh, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_MEMORY_DISASSEMBLE, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(14, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 15; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (!m_list.GetSelectedCount()) 
		{
			for (int i = 2; i < 14; i++) // 如果没有选中, "导出到..."功能可用
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else
		{
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			if (pos)
			{
				int nItem = m_list.GetNextSelectedItem(pos);
				CString strProtect = m_list.GetItemText(nItem, 2);
				CString strState = m_list.GetItemText(nItem, 3);

				if (strProtect.Find(L"Write") == -1)
				{
					menu.EnableMenuItem(ID_MEMORY_ZERO_MEMORY, MF_GRAYED | MF_DISABLED);
				}

				if (strProtect.Find(L"Execute") == -1)
				{
					menu.EnableMenuItem(ID_MEMORY_DISASSEMBLE, MF_GRAYED | MF_DISABLED);
				}

				if (!strState.CompareNoCase(L"Free") && strProtect.Find(L"No") != -1)
				{
					menu.EnableMenuItem(ID_MEMORY_FREE_MEMORY, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_MEMORY_DUMP_SELECT, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_MEMORY_CHANGE_PROTECT, MF_GRAYED | MF_DISABLED);
				}

				if (!strState.CompareNoCase(L"Reserve"))
				{
					menu.EnableMenuItem(ID_MEMORY_DUMP_SELECT, MF_GRAYED | MF_DISABLED);
					menu.EnableMenuItem(ID_MEMORY_CHANGE_PROTECT, MF_GRAYED | MF_DISABLED);
				}

				menu.EnableMenuItem(ID_MEMORY_DISASSEMBLE, MF_ENABLED);
			}
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CMemoryInfoDlg::OnMemoryRefresh()
{
	ListProcessMemory();
}

void CMemoryInfoDlg::OnMemoryDump()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szAddress = m_list.GetItemText(iIndex, 0);
		for ( vector <MEMORY_INFO>::iterator Iter = m_ProcessMemoryVector.begin( ); Iter != m_ProcessMemoryVector.end( ); Iter++ )
		{
			CString strBase;
			strBase.Format(L"0x%08X", Iter->Base);
			if (!strBase.CompareNoCase(szAddress))
			{
				CDumpMemoryDlg DumpDlg;
				DumpDlg.m_dwPid = m_dwPid;
				DumpDlg.m_pEprocess = m_pEprocess;
				DumpDlg.m_szAddress.Format(L"%08X", Iter->Base);
				DumpDlg.m_szSize.Format(L"%08X", Iter->Size);
				DumpDlg.DoModal();
				break;
			}	
		}
	}
}

void CMemoryInfoDlg::OnMemoryDumpSelect()
{
	BOOL bCopy = FALSE;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = m_list.GetNextSelectedItem(pos);
		CString szAddress = m_list.GetItemText(nItem, 0);
		CString szFileName;
		szFileName.Format(L"memory_%s_%s", m_list.GetItemText(nItem, 0), m_list.GetItemText(nItem, 1));
		szFileName = szFileName + L".dat";

		CFileDialog fileDlg(
			FALSE, 
			0, 
			szFileName, 
			0, 
			L"Data Files (*.dat)|*.dat;|All Files (*.*)|*.*||",
			0
			);

		if (IDOK == fileDlg.DoModal())
		{
			CString szFilePath = fileDlg.GetFileName();

			if ( !PathFileExists(szFilePath) ||
				(PathFileExists(szFilePath) && MessageBox(szFileExist[g_enumLang], szDumpMemory[g_enumLang], MB_YESNO | MB_ICONQUESTION) == IDYES))
			{
				for ( vector <MEMORY_INFO>::iterator Iter = m_ProcessMemoryVector.begin( ); Iter != m_ProcessMemoryVector.end( ); Iter++ )
				{
					CString strBase;
					strBase.Format(L"0x%08X", Iter->Base);
					if (!strBase.CompareNoCase(szAddress))
					{
						bCopy = Dump(Iter->Base, Iter->Size, szFilePath);
						break;
					}	
				}

				if (bCopy)
				{
					MessageBox(szDumpOK[g_enumLang], szDumpMemory[g_enumLang]);
				}
				else
				{
					MessageBox(szDumpFailed[g_enumLang], szDumpMemory[g_enumLang]);
				}
			}
		}
	}
}

void CMemoryInfoDlg::OnMemoryZeroMemory()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		if (MessageBox(szMemoryDangerous[g_enumLang], szMemoryZeroMemory[g_enumLang], MB_YESNO | MB_ICONWARNING) == IDNO)
		{
			return;
		}

		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szAddress = m_list.GetItemText(iIndex, 0);
		for ( vector <MEMORY_INFO>::iterator Iter = m_ProcessMemoryVector.begin( ); Iter != m_ProcessMemoryVector.end( ); Iter++ )
		{
			CString strBase;
			strBase.Format(L"0x%08X", Iter->Base);
			if (!strBase.CompareNoCase(szAddress))
			{
				COMMUNICATE_MEMORY cm;
				cm.OpType = enumZeroMemory;
				cm.op.ZeroMemory.Base = Iter->Base;
				cm.op.ZeroMemory.Size = Iter->Size;
				cm.op.ZeroMemory.nPid = m_dwPid;
				cm.op.ZeroMemory.pEprocess = m_pEprocess;
				BOOL bRet = m_Driver.CommunicateDriver(&cm, sizeof(COMMUNICATE_MEMORY), NULL, 0, NULL);

				if (!bRet)
				{
					MessageBox(szZeroMemoryFailed[g_enumLang], szZeroMemory[g_enumLang], MB_OK);
				}
				else
				{
					MessageBox(szZeroMemoryOk[g_enumLang], szZeroMemory[g_enumLang], MB_OK);
				}

				break;
			}	
		}
	}
}

void CMemoryInfoDlg::OnMemoryChangeProtect()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		int iIndex  = m_list.GetNextSelectedItem(pos);
		CString szAddress = m_list.GetItemText(iIndex, 0);
		for ( vector <MEMORY_INFO>::iterator Iter = m_ProcessMemoryVector.begin( ); Iter != m_ProcessMemoryVector.end( ); Iter++ )
		{
			CString strBase;
			strBase.Format(L"0x%08X", Iter->Base);
			if (!strBase.CompareNoCase(szAddress))
			{
				CModifyProtectDlg ModifyDlg;
				ModifyDlg.m_size = Iter->Size;
				ModifyDlg.m_base = Iter->Base;
				ModifyDlg.m_pid = m_dwPid;
				ModifyDlg.m_pEprocess = m_pEprocess;
				ModifyDlg.m_hMemoryWnd = this->m_hWnd;
				ModifyDlg.DoModal();
				break;
			}	
		}
	}
}

void CMemoryInfoDlg::OnMemoryFreeMemory()
{
	if (MessageBox(szMemoryDangerous[g_enumLang], szMemoryFreeMemory[g_enumLang], MB_YESNO | MB_ICONWARNING) == IDYES)
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		if (pos)
		{
			int iIndex  = m_list.GetNextSelectedItem(pos);
			CString szAddress = m_list.GetItemText(iIndex, 0);
			for ( vector <MEMORY_INFO>::iterator Iter = m_ProcessMemoryVector.begin( ); Iter != m_ProcessMemoryVector.end( ); Iter++ )
			{
				CString strBase;
				strBase.Format(L"0x%08X", Iter->Base);
				if (!strBase.CompareNoCase(szAddress))
				{
					COMMUNICATE_MEMORY cm;
					cm.OpType = enumFreeMemory;
					cm.op.FreeMemory.Base = Iter->Base;
					cm.op.FreeMemory.Size = Iter->Size;
					cm.op.FreeMemory.nPid = m_dwPid;
					cm.op.FreeMemory.pEprocess = m_pEprocess;
					BOOL bRet = m_Driver.CommunicateDriver(&cm, sizeof(COMMUNICATE_MEMORY), NULL, 0, NULL);
					if (bRet)
					{
						m_list.DeleteItem(iIndex);
					}
					else
					{	
						MessageBox(szMemoryFreeFailed[g_enumLang], szMemoryFreeMemory[g_enumLang], MB_OK | MB_ICONERROR);
					}
					break;
				}	
			}
		}
	}
}

void CMemoryInfoDlg::OnMemoryText()
{
	m_Functions.ExportListToTxt(&m_list, m_szText);
}

void CMemoryInfoDlg::OnMemoryExcel()
{
	WCHAR szMemory[] = {'M','e','m','o','r','y','\0'};
	m_Functions.ExportListToExcel(&m_list, szMemory, m_szText);
}

BOOL CMemoryInfoDlg::ListProcessMemory()
{
	m_szText.Format(szMemoryInfoState[g_enumLang], 0);
	UpdateData(FALSE);

	m_list.DeleteAllItems();
	m_clsListModules.EnumModulesByPeb(m_dwPid, m_pEprocess, m_vectorModules);

	ULONG nMemoryCnt = 0;
	GetProcessMemoryInfo();

	if (!m_ProcessMemoryVector.empty())
	{
		SortByBase();

		ULONG i = 0;
		for ( vector <MEMORY_INFO>::iterator Iter = m_ProcessMemoryVector.begin( ); Iter != m_ProcessMemoryVector.end( ); Iter++, i++ )
		{
			MEMORY_INFO item = *Iter;
			CString strBase, strSize, strProtect, strType, strImageName, strState;

			strBase.Format(L"0x%08X", item.Base);
			strSize.Format(L"0x%08X", item.Size);
			strProtect = GetMemoryProtect(item.Protect);
			strType = GetMempryType(item.Type);
			strState = GetMempryState(item.State);
			if (item.Type == MEM_IMAGE)
			{
				strImageName = GetModuleImageName(item.Base);
			}

			m_list.InsertItem(i, strBase);
			m_list.SetItemText(i, 1, strSize);
			m_list.SetItemText(i, 2, strProtect);
			m_list.SetItemText(i, 3, strState);
			m_list.SetItemText(i, 4, strType);
			m_list.SetItemText(i, 5, strImageName);

			nMemoryCnt++;
		}
	}

	m_szText.Format(szMemoryInfoState[g_enumLang], nMemoryCnt);
	UpdateData(FALSE);

	return TRUE;
}

BOOL ModuleInfoBaseSort( MEMORY_INFO elem1, MEMORY_INFO elem2 )
{
	return elem1.Base < elem2.Base;
}

VOID CMemoryInfoDlg::SortByBase()
{
	sort(m_ProcessMemoryVector.begin( ), m_ProcessMemoryVector.end( ), ModuleInfoBaseSort);
}

CString CMemoryInfoDlg::GetMemoryProtect(ULONG Protect)
{
	BOOL bFirst = TRUE;
	CString strProtect;

	for ( vector <PROTECT>::iterator Iter = m_ProtectTypeVector.begin( ); Iter != m_ProtectTypeVector.end( ); Iter++ )
	{
		PROTECT item = *Iter;
		if (item.uType & Protect)
		{
			if (bFirst == TRUE)
			{
				strProtect = item.szTypeName;
				bFirst = FALSE;
			}
			else
			{
				strProtect += L" & ";
				strProtect += item.szTypeName;
			}
		}
	}

	return strProtect;
}

CString CMemoryInfoDlg::GetMempryState(ULONG State)
{
	CString szState;

	if (State == MEM_COMMIT)
	{
		szState = L"Commit";
	}
	else if (State == MEM_FREE)
	{
		szState = L"Free";
	}
	else if (State == MEM_RESERVE)
	{
		szState = L"Reserve";
	}
	else if (State == MEM_DECOMMIT)
	{
		szState = L"Decommit";
	}
	else if (State == MEM_RELEASE)
	{
		szState = L"Release";
	}

	return szState;
}

CString CMemoryInfoDlg::GetMempryType(ULONG Type)
{
	CString szType;

	if (Type == MEM_PRIVATE)
	{
		szType = L"Private";
	}
	else if (Type == MEM_MAPPED)
	{
		szType = L"Map";
	}
	else if (Type == MEM_IMAGE)
	{
		szType = L"Image";
	}

	return szType;
}

CString CMemoryInfoDlg::GetModuleImageName(DWORD dwBase)
{
	CString szRet = L"";

	for ( vector <MODULE_INFO>::iterator Iter = m_vectorModules.begin( ); Iter != m_vectorModules.end( ); Iter++)
	{	
		MODULE_INFO entry = *Iter;

		if (dwBase >= entry.Base && dwBase <= (entry.Base + entry.Size))
		{
			CString strPath = Iter->Path;
			szRet = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
		}
	}

	return szRet;
}

BOOL CMemoryInfoDlg::Dump(ULONG Base, ULONG Size, CString szFilePath)
{
 	BOOL bRet = FALSE;
// 	PVOID pBuffer = malloc(Size);
// 	if (!pBuffer)
// 	{
// 		return bRet;
// 	}
// 
// 	COMMUNICATE_PROCESS_MODULE_DUMP cpmd;
// 
// 	cpmd.Base = Base;
// 	cpmd.Size = Size;
// 	cpmd.nPid = m_dwPid;
// 	cpmd.pEprocess = m_pEprocess;
// 	cpmd.OpType = enumDumpDllModule;
// 
// 	bRet = m_Driver.CommunicateDriver(&cpmd, sizeof(COMMUNICATE_PROCESS_MODULE_DUMP), pBuffer, Size, NULL);
// 	if (bRet)
// 	{
// 		CFile file;
// 		TRY 
// 		{
// 			if (file.Open(szFilePath,  CFile::modeCreate | CFile::modeWrite))
// 			{
// 				file.Write(pBuffer, Size);
// 				file.Close();
// 				bRet = TRUE;
// 			}
// 		}
// 		CATCH_ALL( e )
// 		{
// 			file.Abort();   // close file safely and quietly
// 			//THROW_LAST();
// 		}
// 		END_CATCH_ALL
// 	}
// 
// 	free(pBuffer);
// 	pBuffer = NULL;

	return bRet;
}

void CMemoryInfoDlg::GetProcessMemoryInfo()
{
// 	COMMUNICATE_MEMORY cm;
// 	PPROCESS_MEMORY pModuleMemory = NULL;
// 	ULONG nCnt = 1000;
// 	BOOL bRet = FALSE;
// 
// 	m_ProcessMemoryVector.clear();
// 
// 	cm.OpType = enumListProcessMemory;
// 	cm.op.GetMemory.nPid = m_dwPid;
// 	cm.op.GetMemory.pEprocess = m_pEprocess;
// 
// 	do 
// 	{
// 		ULONG nSize = 0;
// 
// 		if (pModuleMemory)
// 		{
// 			free(pModuleMemory);
// 			pModuleMemory = NULL;
// 		}
// 
// 		nSize = sizeof(PROCESS_MEMORY) + nCnt * sizeof(MEMORY_INFO);
// 
// 		pModuleMemory = (PPROCESS_MEMORY)malloc(nSize);
// 		if (!pModuleMemory)
// 		{
// 			break;
// 		}
// 
// 		memset(pModuleMemory, 0, nSize);
// 
// 		pModuleMemory->nCnt = nCnt;
// 		pModuleMemory->nRetCnt = 0;
// 
// 		bRet = m_Driver.CommunicateDriver(&cm, sizeof(COMMUNICATE_MEMORY), pModuleMemory, nSize, NULL);
// 		nCnt = pModuleMemory->nRetCnt + 100;
// 
// 	} while (pModuleMemory->nRetCnt > pModuleMemory->nCnt);
// 
// 	if (bRet && pModuleMemory)
// 	{
// 		for (ULONG i = 0; i < pModuleMemory->nRetCnt; i++)
// 		{
// 			m_ProcessMemoryVector.push_back(pModuleMemory->Memorys[i]);
// 		}
// 	}
// 
// 	if (pModuleMemory)
// 	{
// 		free(pModuleMemory);
// 		pModuleMemory = NULL;
// 	}
}

VOID CMemoryInfoDlg::InitMemoryProtect()
{
	PROTECT protect;

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_NOACCESS;
	wcsncpy_s(protect.szTypeName, L"No Access", wcslen(L"No Access"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_READONLY;
	wcsncpy_s(protect.szTypeName, L"Read", wcslen(L"Read"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_READWRITE;
	wcsncpy_s(protect.szTypeName, L"ReadWrite", wcslen(L"ReadWrite"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_WRITECOPY;
	wcsncpy_s(protect.szTypeName, L"WriteCopy", wcslen(L"WriteCopy"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE;
	wcsncpy_s(protect.szTypeName, L"Execute", wcslen(L"Execute"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_READ;
	wcsncpy_s(protect.szTypeName, L"ReadExecute", wcslen(L"ReadExecute"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_READWRITE;
	wcsncpy_s(protect.szTypeName, L"ReadWriteExecute", wcslen(L"ReadWriteExecute"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_WRITECOPY;
	wcsncpy_s(protect.szTypeName, L"WriteCopyExecute", wcslen(L"WriteCopyExecute"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_GUARD;
	wcsncpy_s(protect.szTypeName, L"Guard", wcslen(L"Guard"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_NOCACHE;
	wcsncpy_s(protect.szTypeName, L"No Cache", wcslen(L"No Cache"));
	m_ProtectTypeVector.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_WRITECOMBINE;
	wcsncpy_s(protect.szTypeName, L"WriteCombine", wcslen(L"WriteCombine"));
	m_ProtectTypeVector.push_back(protect);
}

void CMemoryInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CMemoryInfoDlg::OnMemoryDisassemble()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 0);
		if (!szAddress.IsEmpty())
		{
			DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
			if (dwAddress > 0)
			{
				m_Functions.Disassemble(dwAddress, 100, m_dwPid, m_pEprocess);
			}
		}
	}
}

void CMemoryInfoDlg::OnMemoryHexEdit()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 0);
		if (!szAddress.IsEmpty())
		{
			DWORD dwAddress = m_Functions.HexStringToLong(szAddress);
			if (dwAddress > 0)
			{
				m_Functions.HexEdit(dwAddress, 0x1000, m_dwPid, m_pEprocess);
			}
		}
	}
}
