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
// ProcessHookDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ProcessHookDlg.h"
#include <shlwapi.h>
#include <DbgHelp.h>

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine\\BeaEngine.h"
#pragma comment(lib, "BeaEngine\\BeaEngine.lib")

#define WM_PROCESS_HOOK_EDIT_DATA	 WM_USER + 7

/////////////////////////////////////////////////////////////////////////

#define LDRP_RELOCATION_INCREMENT   0x1
#define LDRP_RELOCATION_FINAL       0x2

#define IMAGE_REL_BASED_SECTION               6
#define IMAGE_REL_BASED_REL32                 7

/////////////////////////////////////////////////////////////////////////

// CProcessHookDlg 对话框

IMPLEMENT_DYNAMIC(CProcessHookDlg, CDialog)

CProcessHookDlg::CProcessHookDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessHookDlg::IDD, pParent)
	, m_szStatus(_T(""))
{
	m_pEprocess = 0;
	m_nPid = 0;
	m_szPath = L"";
	m_DumpList.clear();
	m_hThread = NULL;
	m_bQuit = FALSE;
}

CProcessHookDlg::~CProcessHookDlg()
{
	ClearDumpList();

	if (m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void CProcessHookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROCESS_HOOK_STATUS, m_szStatus);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CProcessHookDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CProcessHookDlg::OnBnClickedOk)
	ON_MESSAGE(WM_PROCESS_HOOK_EDIT_DATA, OnUpdateData)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CProcessHookDlg::OnNMRclickList)
	ON_COMMAND(ID_SDT_REFRESH, &CProcessHookDlg::OnSdtRefresh)
	ON_COMMAND(ID_SDT_DIS_CURRENT, &CProcessHookDlg::OnSdtDisCurrent)
	ON_COMMAND(ID_SDT_RESTORE, &CProcessHookDlg::OnSdtRestore)
	ON_COMMAND(ID_SDT_RESTORE_ALL, &CProcessHookDlg::OnSdtRestoreAll)
	ON_COMMAND(ID_SDT_SHUXING, &CProcessHookDlg::OnSdtShuxing)
	ON_COMMAND(ID_SDT_LOCATION_MODULE, &CProcessHookDlg::OnSdtLocationModule)
	ON_COMMAND(ID_SDT_VERIFY_SIGN, &CProcessHookDlg::OnSdtVerifySign)
	ON_COMMAND(ID_SDT_TEXT, &CProcessHookDlg::OnSdtText)
	ON_COMMAND(ID_SDT_EXCEL, &CProcessHookDlg::OnSdtExcel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CProcessHookDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_PROCESS_HOOK_STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CProcessHookDlg 消息处理程序

void CProcessHookDlg::OnBnClickedOk()
{
}

void CProcessHookDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

DWORD __stdcall EnumHookProc( 
	LPVOID lpThreadParameter)
{
	CProcessHookDlg *Dlg = (CProcessHookDlg*)lpThreadParameter;
	if (Dlg)
	{
		Dlg->EnumProcessHooks();
	}

	return 0;
}

BOOL CProcessHookDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	CString szImage = m_szPath.Right(m_szPath.GetLength() - m_szPath.ReverseFind('\\') - 1);
	CString szWinText;
	szWinText.Format(L"%s (%s | %d | 0x%08X)", szProcessHook[g_enumLang], szImage, m_nPid, m_pEprocess);
	SetWindowText(szWinText);
	
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szHookAddress[g_enumLang], LVCFMT_LEFT, 230);
	m_list.InsertColumn(1, szHookType[g_enumLang], LVCFMT_LEFT, 60);
	m_list.InsertColumn(2, szCurrentEntry[g_enumLang], LVCFMT_LEFT, 190);
	m_list.InsertColumn(3, szModule[g_enumLang], LVCFMT_LEFT, 300);
	
	DWORD dwThreadId = 0;
	m_hThread = CreateThread(
		NULL,
		0,
		EnumHookProc,
		this,
		0,
		&dwThreadId
		);

	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CProcessHookDlg::ClearDumpList()
{
	for (list<DUMP_INFO>::iterator ir = m_DumpList.begin();
		ir != m_DumpList.end();
		ir++)
	{
		if (ir->pSectionBuffer)
		{
			free(ir->pSectionBuffer);
			ir->pSectionBuffer = NULL;
		}

		if (ir->pReloadBuffer)
		{
			free(ir->pReloadBuffer);
			ir->pReloadBuffer = NULL;
		}
	}

	m_DumpList.clear();
}

void CProcessHookDlg::FixImprtAndReacDir()
{
	for (list<DUMP_INFO>::iterator ir = m_DumpList.begin();
		!m_bQuit && ir != m_DumpList.end();
		ir++)
	{
		if (ir->pReloadBuffer && ir->nBase)
		{
			if(FixImportTable((PBYTE)ir->pReloadBuffer, ir->nBase) &&
				FixBaseRelocTable((PBYTE)ir->pReloadBuffer, ir->nBase))
			{
				ir->bFixed = TRUE;
			}
		}
	}
}

void CProcessHookDlg::EnumProcessHooks()
{
	if ( m_pEprocess == 0 || m_nPid == 0 /* || m_szPath.IsEmpty()*//* || !PathFileExists(m_szPath)*/ )
	{
		return;
	}

	m_szStatus = szInitializing[g_enumLang];
	SendMessage(WM_PROCESS_HOOK_EDIT_DATA);

	m_IatHookList.clear();
	m_InlineHookInfo.clear();
	m_list.DeleteAllItems();
	ClearDumpList();

	m_clsListModules.EnumModulesByPeb(m_nPid, m_pEprocess, m_vectorModules);
	
	for ( vector <MODULE_INFO>::iterator DllIter = m_vectorModules.begin( ); 
		!m_bQuit && DllIter != m_vectorModules.end( ); 
		DllIter++)
	{
		MODULE_INFO ModuleItem = *DllIter;
		CString strPath = ModuleItem.Path;

		if (strPath.IsEmpty() || !PathFileExists(strPath))
		{
			continue;
		}

		PVOID pSectionBuffer = DumpDllModuleSection(ModuleItem.Base, ModuleItem.Size);
		if (!pSectionBuffer)
		{
			continue;
		}

		PVOID pReloadBuffer = MapFileWithoutFix(strPath);
		if (!pReloadBuffer)
		{
			free(pSectionBuffer);
			continue;
		}

		DUMP_INFO Dump;
		Dump.nBase = ModuleItem.Base;
		Dump.nSize = ModuleItem.Size;
		Dump.szPath = strPath;
		Dump.pSectionBuffer = pSectionBuffer;
		Dump.pReloadBuffer = pReloadBuffer;
		Dump.bFixed = FALSE;
		m_DumpList.push_back(Dump);
	}

	DebugLog(L"Dump modules finished");
	FixImprtAndReacDir();

	for (list<DUMP_INFO>::iterator ir = m_DumpList.begin();
		!m_bQuit && ir != m_DumpList.end();
		ir++)
	{
		m_szStatus.Format(L"%s: %s", szScaning[g_enumLang], ir->szPath);
		SendMessage(WM_PROCESS_HOOK_EDIT_DATA);
		
		if (ir->bFixed)
		{
			DebugLog(L"scan: %s", ir->szPath);

			EnumIatHook(ir->pSectionBuffer, ir->szPath, ir->nBase);
			EnumInlineHook(ir->pSectionBuffer, ir->pReloadBuffer, ir->nBase);
		}
	}

	if (!m_bQuit)
	{
		ShowScanResult();
	}

//	ClearDumpList();
}

void CProcessHookDlg::EnumIatHook(PVOID pSection, CString szPath, DWORD nBase)
{
	if (pSection == NULL || szPath.IsEmpty() || !nBase)
	{
		return;
	}

	PIMAGE_IMPORT_DESCRIPTOR ImageImportDescriptor = NULL;
	PIMAGE_BOUND_IMPORT_DESCRIPTOR ImageBoundImportDescriptor = NULL;
	PIMAGE_THUNK_DATA ImageThunkData = NULL, FirstThunk = NULL;
	PIMAGE_IMPORT_BY_NAME ImortByName = NULL;
	DWORD ImportSize = 0;
	DWORD dwImportFunctionAddress = 0;
	CString szHookedModule = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
	szHookedModule.MakeLower();

	ImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(pSection, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportSize);
	if (ImageImportDescriptor == NULL || ImportSize == 0)
	{
		return;
	}

	list<BOUND_IMPORT_INFO> m_BoundList;
	ImageBoundImportDescriptor = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(pSection, TRUE, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, &ImportSize);
	if (ImageBoundImportDescriptor && ImportSize)
	{
		GetImageBoundImport(ImageBoundImportDescriptor, m_BoundList);
	}

	while (ImageImportDescriptor->OriginalFirstThunk && ImageImportDescriptor->Name)
	{
		CHAR szImportModule[MAX_PATH] = {0};
		strcpy_s(szImportModule, MAX_PATH, (CHAR*)((ULONG)pSection + ImageImportDescriptor->Name));
		CString wsImportModule = m_Functions.ms2ws(szImportModule);

		DWORD dwImportModuleBase = 0;
		GetDllModuleBase(wsImportModule, &dwImportModuleBase);

		if (dwImportModuleBase == 0)
		{
			ImageImportDescriptor++;
			continue;
		}

		PVOID pBuffer = GetReloadBuffer(dwImportModuleBase);
		if (pBuffer == NULL)
		{
			ImageImportDescriptor++;
			continue;
		}

		ImageBoundImportDescriptor = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(pBuffer, TRUE, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, &ImportSize);
		if (ImageBoundImportDescriptor && ImportSize)
		{
			GetImageBoundImport(ImageBoundImportDescriptor, m_BoundList);
		}

		ImageThunkData = (PIMAGE_THUNK_DATA)((ULONG)pSection + ImageImportDescriptor->OriginalFirstThunk);
		FirstThunk = (PIMAGE_THUNK_DATA)((ULONG)pSection + ImageImportDescriptor->FirstThunk);
		while(!IsBadReadPtr(ImageThunkData, sizeof(IMAGE_THUNK_DATA)) && 
			!IsBadReadPtr(FirstThunk, sizeof(IMAGE_THUNK_DATA)) &&
			ImageThunkData->u1.Ordinal)
		{
			// import by name
			if(!IMAGE_SNAP_BY_ORDINAL32(ImageThunkData->u1.Ordinal))
			{
				ImortByName = (PIMAGE_IMPORT_BY_NAME)((ULONG)pSection + ImageThunkData->u1.AddressOfData);
				if (IsBadStringPtrA((CHAR*)ImortByName->Name, 1))
				{
					break;
				}

				dwImportFunctionAddress = FirstThunk->u1.Function;
				DWORD dwImportFuncOffset = GetImportFuncOffset(pBuffer, (CHAR*)ImortByName->Name);
				if (dwImportFunctionAddress && dwImportFuncOffset)
				{
					DWORD dwOriginAddress = dwImportModuleBase + dwImportFuncOffset;
					if ( dwImportFunctionAddress != dwOriginAddress )
					{
						// is bound import
						if (!IsBoundImport(wsImportModule, dwImportFunctionAddress, m_BoundList))
						{
							IAT_HOOK_INFO iat;
							iat.szHookedModule = szHookedModule;
							iat.szExpModule = wsImportModule.MakeLower();
							iat.szFunction = m_Functions.ms2ws((CHAR*)ImortByName->Name);
							iat.dwHookAddress = dwImportFunctionAddress;
							iat.dwOriginAddress = dwOriginAddress;
							iat.dwIatAddress = (ULONG)FirstThunk -  (ULONG)pSection + nBase;
							m_IatHookList.push_back(iat);
						}
					}
				}
			}

			FirstThunk++;
			ImageThunkData++;
		}

		ImageImportDescriptor++;
	}
}

CString CProcessHookDlg::GetFunctionName(PVOID pSection, DWORD dwOffset)
{
	if (!pSection || !dwOffset)
	{
		return NULL;
	}

	CString szName;
	PULONG NameTableBase;
	PUSHORT NameOrdinalTableBase;
	PULONG AddressTableBase;
	LONG High;
	LONG Low;
	ULONG ExportSize;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) ImageDirectoryEntryToData ((PVOID)pSection, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize);
	if (ExportDirectory == NULL) 
	{
		return NULL;
	}

	NameTableBase = (PULONG)((ULONG)pSection + (ULONG)ExportDirectory->AddressOfNames);
	NameOrdinalTableBase = (PUSHORT)((ULONG)pSection + (ULONG)ExportDirectory->AddressOfNameOrdinals);
	AddressTableBase = (PULONG)((ULONG)pSection + (ULONG)ExportDirectory->AddressOfFunctions);
	
	if (IsBadReadPtr((PVOID)NameTableBase, sizeof(ULONG)) ||
		IsBadReadPtr((PVOID)NameOrdinalTableBase, sizeof(USHORT)) ||
		IsBadReadPtr((PVOID)AddressTableBase, sizeof(ULONG)) )
	{
		return NULL;
	}

	High = ExportDirectory->NumberOfNames - 1;
	
	for (Low = 0; Low < High; Low++)
	{
		if (IsBadReadPtr((PVOID)(NameOrdinalTableBase + Low), sizeof(USHORT)))
		{
			return NULL;
		}

		USHORT OrdinalNumber = NameOrdinalTableBase[Low];
		if ((ULONG)OrdinalNumber <= ExportDirectory->NumberOfFunctions) 
		{
			if (IsBadReadPtr((PVOID)(AddressTableBase + OrdinalNumber), sizeof(ULONG)))
			{
				return NULL;
			}

			DWORD nOfffset = AddressTableBase[OrdinalNumber];
			if (nOfffset == dwOffset)
			{
				szName = m_Functions.ms2ws((PCHAR)((ULONG)pSection + NameTableBase[Low]));
				break;
			}
		}
	}
	
	return szName;
}

// 也不只是IAT了, 凡是section表中在text段中的都排除, eg. idata段等.
DWORD CProcessHookDlg::PassIATAddress(PVOID pSection, ULONG pVA, ULONG nSize)
{
	if (pSection == NULL || !pVA || !nSize)
	{
		return 0;
	}

	DWORD dwTemp1 = 0, dwTemp2 = 0;
	DWORD dwRet = 0;
	DWORD ImportSize = 0;
	PVOID pRet = NULL;
	pRet = ImageDirectoryEntryToData(pSection, TRUE, IMAGE_DIRECTORY_ENTRY_IAT, &ImportSize);
	if (pRet && ImportSize)
	{
		if ( (ULONG)pRet == (ULONG)pSection + pVA && 
			(ULONG)pRet + ImportSize <= (ULONG)pSection + pVA + nSize )
		{
			dwRet += ImportSize;
		}
	}

	pRet = ImageDirectoryEntryToData(pSection, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ImportSize);
	if (pRet && ImportSize)
	{
		if ( (ULONG)pRet == (ULONG)pSection + pVA + dwRet &&
			(ULONG)pRet <= (ULONG)pSection + pVA + nSize)
		{
			dwRet += ImportSize;
		}
	}

	return dwRet;
}


void CProcessHookDlg::EnumInlineHook(PVOID pSection, PVOID pReload, DWORD dwBase)
{
	if (pSection == NULL || pReload == NULL || dwBase == 0)
	{
		return;
	}

	PIMAGE_EXPORT_DIRECTORY ReloadImageExportDirectory = NULL;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	ULONG ExportSize = 0;
	PULONG pReloadAddressOfNames = NULL;
	PUSHORT pReloadAddressOfNameOrdinals = NULL;
	PULONG ReloadAddressOfFunctionsRVA = 0;
	ULONG i = 0, iCnt = 0;
	PIMAGE_SECTION_HEADER NtSection = NULL;

	ImageDosHeader = (PIMAGE_DOS_HEADER)pReload;
	if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return;
	}

	ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)pReload + ImageDosHeader->e_lfanew);
	if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		return;
	}
	
	CHAR szText[] = {'.','t','e','x','t','\0'};
	NtSection = IMAGE_FIRST_SECTION( ImageNtHeaders );
	for (i = 0; i < ImageNtHeaders->FileHeader.NumberOfSections; i++) 
	{
		if ( (NtSection->Characteristics & IMAGE_SCN_CNT_CODE) &&
			!strcmp((CHAR*)NtSection->Name, szText)) // 是代码段
		{
			ULONG nOffset = PassIATAddress(pReload, NtSection->VirtualAddress, NtSection->Misc.VirtualSize);
			if (nOffset == 0xffffffff)
			{
				return;
			}

			ULONG ReloadAddress = NtSection->VirtualAddress + (ULONG)pReload + nOffset;
			ULONG OriginAddress = NtSection->VirtualAddress + (ULONG)pSection + nOffset;
			ULONG Size = NtSection->Misc.VirtualSize - nOffset;
			if (nOffset)
			{
				DebugLog(L"pass: %d", nOffset);
			}
			
			if (Size > 0 && ReloadAddress > 0 && OriginAddress > 0)
			{
				ULONG n = 0, nDifCnt = 0, pDifAddress = 0;;

				for (n = 0;
					n < Size && !IsBadReadPtr((PVOID)(ReloadAddress + n), sizeof(PVOID)) && !IsBadReadPtr((PVOID)(OriginAddress + n), sizeof(PVOID)); 
					n++)
				{
					if ( *((PBYTE)ReloadAddress + n) != *((PBYTE)OriginAddress + n) &&
						*(PULONG)(ReloadAddress + n) != 0 &&
						*(PULONG)(ReloadAddress + sizeof(ULONG) + n) != 0) 
					{
						if (!nDifCnt)
						{
							pDifAddress = (ULONG)OriginAddress + n;
						}
						
						DebugLog(L"reload: %x, now:%x", *((PBYTE)ReloadAddress + n), *((PBYTE)OriginAddress + n));
						nDifCnt++;
					}
					else 
					{
						if (nDifCnt > 0)
						{
							INLINE_HOOK_INFO info;
							info.dwHookOffset = pDifAddress - (ULONG)pSection;
							info.dwLen = nDifCnt;
							info.dwBase = dwBase;
							info.dwReload = (DWORD)pReload;
							m_InlineHookInfo.push_back(info);
						}
						
						nDifCnt = 0;
					}	
				}	
			}

			break;
		}

		++NtSection;
	}
}

CString CProcessHookDlg::GetModuleImageName(DWORD dwBase)
{
	CString szImage;
	
	if (dwBase)
	{
		for (list<DUMP_INFO>::iterator ir = m_DumpList.begin();
			ir != m_DumpList.end();
			ir++)
		{
			if (ir->nBase == dwBase)
			{
				CString szPath = ir->szPath;
				szImage = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
				szImage.MakeLower();
				break;
			}
		}
	}

	return szImage;
}

void CProcessHookDlg::ShowScanResult()
{
	ULONG nCnt = 0;

	// iat hook
	for (list<IAT_HOOK_INFO>::iterator itor = m_IatHookList.begin();
		itor != m_IatHookList.end();
		itor++)
	{
		CString szModule = GetDllModulePath(itor->dwHookAddress);
		CString szImag = szModule.Right(szModule.GetLength() - szModule.ReverseFind('\\') - 1);
		if (szImag.CompareNoCase(itor->szExpModule))
		{
			CString szFunction = itor->szHookedModule;
			szFunction += L": _imp_";
			szFunction += itor->szFunction;
			szFunction += L"[";
			szFunction += itor->szExpModule;
			szFunction += L"]";

			CString szCurrent;
			szCurrent.Format(L"0x%08X -> 0x%08X", itor->dwOriginAddress, itor->dwHookAddress);

			int n = m_list.InsertItem(m_list.GetItemCount(), szFunction);
			m_list.SetItemText(n, 1, L"iat");
			m_list.SetItemText(n, 2, szCurrent);
			m_list.SetItemText(n, 3, szModule);

			m_list.SetItemData(n, itor->dwIatAddress);

			DebugLog(L"%s -> 0x%08X", szFunction, itor->dwIatAddress);

			nCnt++;
		}
	}

	// inline hook
	for (list<INLINE_HOOK_INFO>::iterator itor = m_InlineHookInfo.begin();
		itor != m_InlineHookInfo.end();
		itor++)
	{
		PVOID pBuffer = GetOriginBuffer(itor->dwBase);
 		CString szModule = GetModuleImageName(itor->dwBase);
		CString szFunc = GetFunctionName(pBuffer, itor->dwHookOffset);
		if (!szFunc.IsEmpty())
		{
			szModule += L": ";
			szModule += szFunc;
		}
		else
		{	
			CString szTemp;
			szTemp.Format(L"%s+0x%X", szModule, itor->dwHookOffset);
			szModule = szTemp;
		}
		
		szModule += L" (len:";
		CString szLen;
		szLen.Format(L"%d", itor->dwLen);
		szModule += szLen;
		szModule += L")";
		
		DWORD dwCurrent = GetInlineHookAddress(pBuffer, itor->dwBase, itor->dwHookOffset, itor->dwLen);
		CString szCurrent;
		szCurrent.Format(L"0x%08X -> ", itor->dwBase + itor->dwHookOffset);
		
		CString szHookModule;

		if (dwCurrent)
		{
			CString szJmp;
			szJmp.Format(L"0x%08X", dwCurrent);
			szCurrent += szJmp;

			szHookModule = GetDllModulePath(dwCurrent);
		}
		else
		{
			szCurrent += L"-";
		}

 		int n = m_list.InsertItem(m_list.GetItemCount(), szModule);
 		m_list.SetItemText(n, 1, L"inline");
 		m_list.SetItemText(n, 2, szCurrent);
 		m_list.SetItemText(n, 3, szHookModule);
			
		m_list.SetItemData(n, n);

		DebugLog(L"inline addr: 0x%08X", itor->dwBase + itor->dwHookOffset);

// 		CString szXx;
// 		for (int i = 0; i < itor->dwLen; i++)
// 		{
// 			CString szXxx;
// 			szXxx.Format(L"%2X ", *(PBYTE)(itor->dwReload + itor->dwHookOffset + i));
// 			szXx += szXxx;
// 		}
// 		DebugLog(szXx);

		nCnt++;
	}

	m_szStatus.Format(szScanDonePatchHooks[g_enumLang], nCnt);
	SendMessage(WM_PROCESS_HOOK_EDIT_DATA);
}

UINT CProcessHookDlg::AlignSize(UINT nSize, UINT nAlign)
{
	return ((nSize + nAlign - 1) / nAlign * nAlign);
}

BOOL CProcessHookDlg::ImageFile(PVOID FileBuffer, PVOID* ImageModuleBase)
{
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	PIMAGE_SECTION_HEADER ImageSectionHeader = NULL;
	DWORD FileAlignment = 0, SectionAlignment = 0, NumberOfSections = 0, SizeOfImage = 0, SizeOfHeaders = 0;
	DWORD Index = 0;
	PVOID ImageBase = NULL;
	DWORD SizeOfNtHeaders = 0;

	if (!FileBuffer || !ImageModuleBase)
	{
		return FALSE;
	}

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)FileBuffer + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		FileAlignment = ImageNtHeaders->OptionalHeader.FileAlignment;
		SectionAlignment = ImageNtHeaders->OptionalHeader.SectionAlignment;
		NumberOfSections = ImageNtHeaders->FileHeader.NumberOfSections;
		SizeOfImage = ImageNtHeaders->OptionalHeader.SizeOfImage;
		SizeOfHeaders = ImageNtHeaders->OptionalHeader.SizeOfHeaders;
		SizeOfImage = AlignSize(SizeOfImage, SectionAlignment);

		ImageBase = malloc( SizeOfImage );
		if (ImageBase == NULL)
		{
			return FALSE;
		}

		ZeroMemory(ImageBase, SizeOfImage);

		SizeOfNtHeaders = sizeof(ImageNtHeaders->FileHeader) + sizeof(ImageNtHeaders->Signature) + ImageNtHeaders->FileHeader.SizeOfOptionalHeader;
		ImageSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)ImageNtHeaders + SizeOfNtHeaders);

		for (Index = 0; Index < NumberOfSections; Index++)
		{
			ImageSectionHeader[Index].SizeOfRawData = AlignSize(ImageSectionHeader[Index].SizeOfRawData, FileAlignment);
			ImageSectionHeader[Index].Misc.VirtualSize = AlignSize(ImageSectionHeader[Index].Misc.VirtualSize, SectionAlignment);
		}

		if (ImageSectionHeader[NumberOfSections - 1].VirtualAddress + ImageSectionHeader[NumberOfSections - 1].SizeOfRawData > SizeOfImage)
		{
			ImageSectionHeader[NumberOfSections - 1].SizeOfRawData = SizeOfImage - ImageSectionHeader[NumberOfSections - 1].VirtualAddress;
		}

		CopyMemory(ImageBase, FileBuffer, SizeOfHeaders);

		for (Index = 0; Index < NumberOfSections; Index++)
		{
			DWORD FileOffset = ImageSectionHeader[Index].PointerToRawData;
			DWORD Length = ImageSectionHeader[Index].SizeOfRawData;
			DWORD ImageOffset = ImageSectionHeader[Index].VirtualAddress;
			CopyMemory(&((PBYTE)ImageBase)[ImageOffset], &((PBYTE)FileBuffer)[FileOffset], Length);
		}

		*ImageModuleBase = ImageBase;
	}
	__except(1)
	{
		if (ImageBase)
		{
			free(ImageBase);
			ImageBase = NULL;
		}

		*ImageModuleBase = NULL;
		return FALSE;
	}

	return TRUE;
}

PVOID CProcessHookDlg::GetReloadBuffer(DWORD dwBase)
{
	PVOID pBuffer = NULL;
	if (dwBase)
	{
		for (list<DUMP_INFO>::iterator ir = m_DumpList.begin();
			ir != m_DumpList.end();
			ir++)
		{
			if (ir->nBase == dwBase)
			{
				pBuffer = ir->pReloadBuffer;
				break;
			}
		}
	}

	return pBuffer;
}

PVOID CProcessHookDlg::GetOriginBuffer(DWORD dwBase)
{
	PVOID pBuffer = NULL;
	if (dwBase)
	{
		for (list<DUMP_INFO>::iterator ir = m_DumpList.begin();
			ir != m_DumpList.end();
			ir++)
		{
			if (ir->nBase == dwBase)
			{
				pBuffer = ir->pSectionBuffer;
				break;
			}
		}
	}

	return pBuffer;
}

BOOL CProcessHookDlg::GetPeImportTable(BYTE *ImageBase, PIMAGE_IMPORT_DESCRIPTOR *ImageImportDescriptor)
{
	BOOL bRet = FALSE;

	if (!ImageBase || !ImageImportDescriptor)
	{
		return bRet;
	}
	
	*ImageImportDescriptor = NULL;

	__try
	{
		PIMAGE_DOS_HEADER ImageDosHeader = NULL;
		PIMAGE_NT_HEADERS ImageNtHeaders = NULL;

		ImageDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return bRet;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)ImageBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return bRet;
		}
		
		DWORD ImportSize = 0;
		*ImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(ImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportSize);
		bRet = TRUE;
	}
	__except(1)
	{
		*ImageImportDescriptor = NULL;
		bRet = FALSE;
	}

	return bRet;
}

BOOL CProcessHookDlg::FixImportTable(BYTE *ImageBase, DWORD ExistImageBase)
{
	PIMAGE_IMPORT_DESCRIPTOR ImageImportDescriptor = NULL;
	PIMAGE_THUNK_DATA ImageThunkData = NULL, FirstThunk = NULL;
	PIMAGE_IMPORT_BY_NAME ImortByName = NULL;
	DWORD ImportSize = 0;
	DWORD FunctionAddress = 0;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;

	if (!GetPeImportTable(ImageBase, &ImageImportDescriptor))
	{
		return FALSE;
	}

	if (ImageImportDescriptor == NULL)
	{
		return TRUE;
	}

	while (ImageImportDescriptor->OriginalFirstThunk && ImageImportDescriptor->Name)
	{
		CHAR ModuleName[MAX_PATH] = {0};
		strcpy_s(ModuleName, MAX_PATH, (CHAR*)(ImageBase + ImageImportDescriptor->Name));
		CString szModule = m_Functions.ms2ws(ModuleName);
		DWORD ModuleBase = 0;
		GetDllModuleBase(szModule, &ModuleBase);

		if (ModuleBase == 0)
		{
			szModule.MakeUpper();
			if (szModule.Find(L"API-MS-WIN") == -1)
			{
				return FALSE;
			}

			ImageImportDescriptor++;
			continue;
		}

		PVOID pReloadBuffer = GetReloadBuffer(ModuleBase);
		if (pReloadBuffer == NULL)
		{
			return FALSE;
			ImageImportDescriptor++;
			continue;
		}

		ImageThunkData = (PIMAGE_THUNK_DATA)(ImageBase + ImageImportDescriptor->OriginalFirstThunk);
		FirstThunk = (PIMAGE_THUNK_DATA)(ImageBase + ImageImportDescriptor->FirstThunk);
		while(ImageThunkData && FirstThunk && ImageThunkData->u1.Ordinal)
		{
			// 序号导入
			if(IMAGE_SNAP_BY_ORDINAL32(ImageThunkData->u1.Ordinal))
			{
				FunctionAddress = (DWORD)MiFindExportedRoutine((PVOID)pReloadBuffer, FALSE, NULL, ImageThunkData->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32);
				if (FunctionAddress == 0)
				{
					DebugLog(L"can't find funcion Index %d", ImageThunkData->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32);
					FirstThunk++;
					ImageThunkData++;
					continue;
				}

				FunctionAddress = FunctionAddress - (DWORD)pReloadBuffer + ModuleBase;
				FirstThunk->u1.Function = FunctionAddress;
			}
			else // 函数名导入
			{
				ImortByName = (PIMAGE_IMPORT_BY_NAME)(ImageBase+ImageThunkData->u1.AddressOfData);
				FunctionAddress = (DWORD)MiFindExportedRoutine((PVOID)pReloadBuffer, TRUE, (CHAR*)ImortByName->Name, 0);

				if (FunctionAddress == 0)
				{
					DebugLog(L"can't Funcion Name:%s", m_Functions.ms2ws((CHAR*)ImortByName->Name));
					return FALSE;
				}

				FunctionAddress = FunctionAddress - (DWORD)pReloadBuffer + ModuleBase;
				FirstThunk->u1.Function = FunctionAddress;
			}

			FirstThunk++;
			ImageThunkData++;
		}

		ImageImportDescriptor++;
	}

	return TRUE;
}

BOOL CProcessHookDlg::InsertOriginalFirstThunk(DWORD ImageBase, DWORD ExistImageBase, PIMAGE_THUNK_DATA FirstThunk)
{
	DWORD Offset = 0;
	PIMAGE_THUNK_DATA OriginalFirstThunk = NULL;

	Offset = (DWORD)FirstThunk - ImageBase;
	OriginalFirstThunk = (PIMAGE_THUNK_DATA)(ExistImageBase + Offset);

	while (OriginalFirstThunk->u1.Function)
	{
		DebugLog(L"Fuction Address:%X\n",OriginalFirstThunk->u1.Function);
		FirstThunk->u1.Function = OriginalFirstThunk->u1.Function;
		OriginalFirstThunk++;
		FirstThunk++;
	}

	return TRUE;
}

void CProcessHookDlg::GetDllModuleBase(CString szModuleName, DWORD *dwRet)
{
	if (szModuleName.IsEmpty() || !dwRet)
	{
		return;
	}

// 	for ( vector <MODULE_INFO>::iterator DllModuleIter = m_DllModuleDlg.m_PebModuleVector.begin(); 
// 		DllModuleIter != m_DllModuleDlg.m_PebModuleVector.end(); 
// 		DllModuleIter++)
// 	{
// 		CString szPath = DllModuleIter->Path;
// 		CString szName = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);
// 
// 		if (!szName.CompareNoCase(szModuleName))
// 		{
// 			*dwRet = DllModuleIter->Base;
// 			break;
// 		}
// 	}

	for ( list <DUMP_INFO>::iterator ir = m_DumpList.begin(); 
		ir != m_DumpList.end(); 
		ir++)
	{
		CString szPath = ir->szPath;
		CString szName = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);

		if (!szName.CompareNoCase(szModuleName))
		{
			*dwRet = ir->nBase;
			break;
		}
	}
}

PVOID CProcessHookDlg::MiFindExportedRoutine (
					   IN PVOID DllBase,
					   BOOL ByName,
					   IN char *RoutineName,
					   DWORD Ordinal
					   )

{
	USHORT OrdinalNumber = 0;
	PULONG NameTableBase = NULL;
	PUSHORT NameOrdinalTableBase = NULL;
	PULONG AddressTableBase = NULL;
	LONG High = 0;
	LONG Low = 0;
	LONG Middle = 0;
	LONG Result = 0;
	ULONG ExportSize = 0;
	PVOID FunctionAddress = NULL;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;

	if (!DllBase)
	{
		return NULL;
	}

	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)DllBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return NULL;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)DllBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return NULL;
		}

		ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) ImageDirectoryEntryToData (
			DllBase,
			TRUE,
			IMAGE_DIRECTORY_ENTRY_EXPORT,
			&ExportSize);

		if (ExportDirectory == NULL) {
			return NULL;
		}

		//
		// Initialize the pointer to the array of RVA-based ansi export strings.
		//

		NameTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);

		//
		// Initialize the pointer to the array of USHORT ordinal numbers.
		//

		NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

		AddressTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

		if (!ByName)
		{
			return (PVOID)AddressTableBase[Ordinal];
		}

		//
		// Lookup the desired name in the name table using a binary search.
		//

		Low = 0;
		Middle = 0;
		High = ExportDirectory->NumberOfNames - 1;

		while (High >= Low) {

			//
			// Compute the next probe index and compare the import name
			// with the export name entry.
			//

			Middle = (Low + High) >> 1;

			Result = strcmp (RoutineName,
				(PCHAR)DllBase + NameTableBase[Middle]);

			if (Result < 0) {
				High = Middle - 1;
			}
			else if (Result > 0) {
				Low = Middle + 1;
			}
			else {
				break;
			}
		}
		//
		// If the high index is less than the low index, then a matching
		// table entry was not found. Otherwise, get the ordinal number
		// from the ordinal table.
		//

		if (High < Low) {
			return NULL;
		}

		OrdinalNumber = NameOrdinalTableBase[Middle];

		//
		// If the OrdinalNumber is not within the Export Address Table,
		// then this image does not implement the function.  Return not found.
		//

		if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
			return NULL;
		}

		//
		// Index into the array of RVA export addresses by ordinal number.
		//

		FunctionAddress = (PVOID)((PCHAR)DllBase + AddressTableBase[OrdinalNumber]);

		//
		// Forwarders are not used by the kernel and HAL to each other.
		//

		// 	if (FunctionAddress >= (PVOID)ExportDirectory &&
		// 		FunctionAddress <= (PVOID)((PCHAR)ExportDirectory + ExportSize)) 
		// 	{
		// 		return NULL;
		// 	}

		// 	ASSERT ((FunctionAddress <= (PVOID)ExportDirectory) ||
		// 		(FunctionAddress >= (PVOID)((PCHAR)ExportDirectory + ExportSize)));
	}
	__except(1)
	{
		FunctionAddress = NULL;
	}

	return FunctionAddress;
}

BOOL CProcessHookDlg::FixBaseRelocTable (
    PVOID NewImageBase,
	DWORD ExistImageBase
    )
/*++

Routine Description:

    This routine relocates an image file that was not loaded into memory
    at the preferred address.

Arguments:

    NewBase - Supplies a pointer to the image base.

    AdditionalBias - An additional quantity to add to all fixups.  The
                     32-bit X86 loader uses this when loading 64-bit images
                     to specify a NewBase that is actually a 64-bit value.

    LoaderName - Indicates which loader routine is being called from.

    Success - Value to return if relocation successful.

    Conflict - Value to return if can't relocate.

    Invalid - Value to return if relocations are invalid.

Return Value:

    Success if image is relocated.
    Conflict if image can't be relocated.
    Invalid if image contains invalid fixups.

--*/

{
    LONGLONG Diff;
    ULONG TotalCountBytes = 0;
    ULONG_PTR VA;
    ULONGLONG OriginalImageBase;
    ULONG SizeOfBlock;
    PUSHORT NextOffset = NULL;
    PIMAGE_BASE_RELOCATION NextBlock;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)NewImageBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)NewImageBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		switch (ImageNtHeaders->OptionalHeader.Magic) {

		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:

			OriginalImageBase =
				((PIMAGE_NT_HEADERS32)ImageNtHeaders)->OptionalHeader.ImageBase;
			break;

		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:

			OriginalImageBase =
				((PIMAGE_NT_HEADERS64)ImageNtHeaders)->OptionalHeader.ImageBase;
			break;

		default:
			return FALSE;
		}

		//
		// Locate the relocation section.
		//

		NextBlock = (PIMAGE_BASE_RELOCATION)ImageDirectoryEntryToData(
			NewImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &TotalCountBytes);

		// 如果没有导入表，直接返回TRUE
		if (!NextBlock || !TotalCountBytes)
		{
			return TRUE;
		}

		Diff = (ULONG_PTR)ExistImageBase - OriginalImageBase;
		while (TotalCountBytes)
		{
			SizeOfBlock = NextBlock->SizeOfBlock;
			TotalCountBytes -= SizeOfBlock;
			SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
			SizeOfBlock /= sizeof(USHORT);
			NextOffset = (PUSHORT)((PCHAR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));

			VA = (ULONG_PTR)NewImageBase + NextBlock->VirtualAddress;

			if ( !(NextBlock = LdrProcessRelocationBlockLongLong( VA,
				SizeOfBlock,
				NextOffset,
				Diff)) ) 
			{
				return FALSE;
			}
		}
	}
	__except(1)
	{
		return FALSE;
	}

    return TRUE;
}

PIMAGE_BASE_RELOCATION CProcessHookDlg::LdrProcessRelocationBlockLongLong(
								  IN ULONG_PTR VA,
								  IN ULONG SizeOfBlock,
								  IN PUSHORT NextOffset,
								  IN LONGLONG Diff
								  )
{
	PUCHAR FixupVA;
	USHORT Offset;
	LONG Temp;
	ULONGLONG Value64;

	while (SizeOfBlock--) {

		Offset = *NextOffset & (USHORT)0xfff;
		FixupVA = (PUCHAR)(VA + Offset);

		//
		// Apply the fixups.
		//

		switch ((*NextOffset) >> 12) {

			case IMAGE_REL_BASED_HIGHLOW :
				//
				// HighLow - (32-bits) relocate the high and low half
				//      of an address.
				//
				*(LONG UNALIGNED *)FixupVA += (ULONG) Diff;
				break;

			case IMAGE_REL_BASED_HIGH :
				//
				// High - (16-bits) relocate the high half of an address.
				//
				Temp = *(PUSHORT)FixupVA << 16;
				Temp += (ULONG) Diff;
				*(PUSHORT)FixupVA = (USHORT)(Temp >> 16);
				break;

			case IMAGE_REL_BASED_HIGHADJ :
				//
				// Adjust high - (16-bits) relocate the high half of an
				//      address and adjust for sign extension of low half.
				//

				//
				// If the address has already been relocated then don't
				// process it again now or information will be lost.
				//
				if (Offset & LDRP_RELOCATION_FINAL) {
					++NextOffset;
					--SizeOfBlock;
					break;
				}

				Temp = *(PUSHORT)FixupVA << 16;
				++NextOffset;
				--SizeOfBlock;
				Temp += (LONG)(*(PSHORT)NextOffset);
				Temp += (ULONG) Diff;
				Temp += 0x8000;
				*(PUSHORT)FixupVA = (USHORT)(Temp >> 16);

				break;

			case IMAGE_REL_BASED_LOW :
				//
				// Low - (16-bit) relocate the low half of an address.
				//
				Temp = *(PSHORT)FixupVA;
				Temp += (ULONG) Diff;
				*(PUSHORT)FixupVA = (USHORT)Temp;
				break;

			case IMAGE_REL_BASED_IA64_IMM64:

				//
				// Align it to bundle address before fixing up the
				// 64-bit immediate value of the movl instruction.
				//

				FixupVA = (PUCHAR)((ULONG_PTR)FixupVA & ~(15));
				Value64 = (ULONGLONG)0;

				//
				// Extract the lower 32 bits of IMM64 from bundle
				//


				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM7B_INST_WORD_X,
					EMARCH_ENC_I17_IMM7B_SIZE_X,
					EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM7B_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM9D_INST_WORD_X,
					EMARCH_ENC_I17_IMM9D_SIZE_X,
					EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM9D_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM5C_INST_WORD_X,
					EMARCH_ENC_I17_IMM5C_SIZE_X,
					EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM5C_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IC_INST_WORD_X,
					EMARCH_ENC_I17_IC_SIZE_X,
					EMARCH_ENC_I17_IC_INST_WORD_POS_X,
					EMARCH_ENC_I17_IC_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM41a_INST_WORD_X,
					EMARCH_ENC_I17_IMM41a_SIZE_X,
					EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41a_VAL_POS_X);

				EXT_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41b_INST_WORD_X),
					EMARCH_ENC_I17_IMM41b_SIZE_X,
					EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41b_VAL_POS_X);
				EXT_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41c_INST_WORD_X),
					EMARCH_ENC_I17_IMM41c_SIZE_X,
					EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41c_VAL_POS_X);
				EXT_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_SIGN_INST_WORD_X),
					EMARCH_ENC_I17_SIGN_SIZE_X,
					EMARCH_ENC_I17_SIGN_INST_WORD_POS_X,
					EMARCH_ENC_I17_SIGN_VAL_POS_X);
				//
				// Update 64-bit address
				//

				Value64+=Diff;

				//
				// Insert IMM64 into bundle
				//

				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM7B_INST_WORD_X),
					EMARCH_ENC_I17_IMM7B_SIZE_X,
					EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM7B_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM9D_INST_WORD_X),
					EMARCH_ENC_I17_IMM9D_SIZE_X,
					EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM9D_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM5C_INST_WORD_X),
					EMARCH_ENC_I17_IMM5C_SIZE_X,
					EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM5C_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IC_INST_WORD_X),
					EMARCH_ENC_I17_IC_SIZE_X,
					EMARCH_ENC_I17_IC_INST_WORD_POS_X,
					EMARCH_ENC_I17_IC_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41a_INST_WORD_X),
					EMARCH_ENC_I17_IMM41a_SIZE_X,
					EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41a_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41b_INST_WORD_X),
					EMARCH_ENC_I17_IMM41b_SIZE_X,
					EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41b_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41c_INST_WORD_X),
					EMARCH_ENC_I17_IMM41c_SIZE_X,
					EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41c_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_SIGN_INST_WORD_X),
					EMARCH_ENC_I17_SIGN_SIZE_X,
					EMARCH_ENC_I17_SIGN_INST_WORD_POS_X,
					EMARCH_ENC_I17_SIGN_VAL_POS_X);
				break;

			case IMAGE_REL_BASED_DIR64:

				*(ULONGLONG UNALIGNED *)FixupVA += Diff;

				break;

			case IMAGE_REL_BASED_MIPS_JMPADDR :
				//
				// JumpAddress - (32-bits) relocate a MIPS jump address.
				//
				Temp = (*(PULONG)FixupVA & 0x3ffffff) << 2;
				Temp += (ULONG) Diff;
				*(PULONG)FixupVA = (*(PULONG)FixupVA & ~0x3ffffff) |
					((Temp >> 2) & 0x3ffffff);

				break;

			case IMAGE_REL_BASED_ABSOLUTE :
				//
				// Absolute - no fixup required.
				//
				break;

			case IMAGE_REL_BASED_SECTION :
				//
				// Section Relative reloc.  Ignore for now.
				//
				break;

			case IMAGE_REL_BASED_REL32 :
				//
				// Relative intrasection. Ignore for now.
				//
				break;

			default :
				//
				// Illegal - illegal relocation type.
				//

				return (PIMAGE_BASE_RELOCATION)NULL;
		}
		++NextOffset;
	}
	return (PIMAGE_BASE_RELOCATION)NextOffset;
}

DWORD CProcessHookDlg::FindExportedRoutineInReloadModule ( IN ULONG DllBase, IN char *RoutineName )
{
	USHORT OrdinalNumber;
	PULONG NameTableBase;
	PUSHORT NameOrdinalTableBase;
	PULONG AddressTableBase;
	LONG High;
	LONG Low;
	LONG Middle;
	LONG Result;
	ULONG ExportSize;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory;

	if (!DllBase || !RoutineName)
	{
		return 0;
	}

	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	DWORD FunctionAddress = 0;

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)DllBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return 0;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)DllBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return 0;
		}

		ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) ImageDirectoryEntryToData (
			(PVOID)DllBase,
			TRUE,
			IMAGE_DIRECTORY_ENTRY_EXPORT,
			&ExportSize);

		if (ExportDirectory == NULL) {
			return 0;
		}

		//
		// Initialize the pointer to the array of RVA-based ansi export strings.
		//

		NameTableBase = (PULONG)(DllBase + (ULONG)ExportDirectory->AddressOfNames);

		//
		// Initialize the pointer to the array of USHORT ordinal numbers.
		//

		NameOrdinalTableBase = (PUSHORT)(DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

		AddressTableBase = (PULONG)(DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

		//
		// Lookup the desired name in the name table using a binary search.
		//

		Low = 0;
		Middle = 0;
		High = ExportDirectory->NumberOfNames - 1;

		while (High >= Low) {

			//
			// Compute the next probe index and compare the import name
			// with the export name entry.
			//

			Middle = (Low + High) >> 1;

			char *zx = (PCHAR)(DllBase + NameTableBase[Middle]);
			Result = strcmp (RoutineName, zx);

			if (Result < 0) {
				High = Middle - 1;
			}
			else if (Result > 0) {
				Low = Middle + 1;
			}
			else {
				break;
			}
		}
		//
		// If the high index is less than the low index, then a matching
		// table entry was not found. Otherwise, get the ordinal number
		// from the ordinal table.
		//

		if (High < Low) {
			return 0;
		}

		OrdinalNumber = NameOrdinalTableBase[Middle];

		//
		// If the OrdinalNumber is not within the Export Address Table,
		// then this image does not implement the function.  Return not found.
		//

		if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) 
		{
			return 0;
		}

		//
		// Index into the array of RVA export addresses by ordinal number.
		//


		FunctionAddress = (DWORD)((PCHAR)DllBase + AddressTableBase[OrdinalNumber]);

		//
		// Forwarders are not used by the kernel and HAL to each other.
		//

		if (FunctionAddress >= (DWORD)ExportDirectory && FunctionAddress <= (DWORD)((PCHAR)ExportDirectory + ExportSize))
		{
			return 0;
		}

		// 	ASSERT ((FunctionAddress <= (PVOID)ExportDirectory) ||
		// 		(FunctionAddress >= (PVOID)((PCHAR)ExportDirectory + ExportSize)));

		FunctionAddress = FunctionAddress - (DWORD)DllBase;
	}
	__except(1)
	{
		FunctionAddress = 0;
	}

	return FunctionAddress;
}

PVOID CProcessHookDlg::DumpProcessSection()
{
	BOOL bRet = FALSE;
	PVOID pBuffer = NULL;
	ULONG nSize = sizeof(ULONG);
	BOOL bFirst = TRUE;

	do 
	{
		if (!bFirst)
		{
			nSize = *(PULONG)pBuffer;
		}

		if (pBuffer)
		{
			free(pBuffer);
			pBuffer = NULL;
		}

		pBuffer = malloc(nSize);

		if (pBuffer)
		{
			memset(pBuffer, 0, nSize);
			COMMUNICATE_PROCESS cp;
			cp.OpType = enumDumpProcess;
			cp.op.DumpProcess.nPid = m_nPid;
			cp.op.DumpProcess.pEprocess = m_pEprocess;
			bRet = m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCESS), pBuffer, nSize, NULL);
		}

		bFirst = FALSE;
	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pBuffer)
	{
		return pBuffer;
	}

	return NULL;
}

PVOID CProcessHookDlg::DumpDllModuleSection(DWORD dwBase, DWORD dwSize)
{
 	PVOID pSection = NULL;

	if (dwBase <= 0 || dwSize <= 0)
	{
		return pSection;
	}
	
	return m_clsListModules.DumpModuleMemory(m_nPid, m_pEprocess, dwBase, dwSize);
}

VOID CProcessHookDlg::GetImageBoundImport(PIMAGE_BOUND_IMPORT_DESCRIPTOR ImageBoundImportDescriptor, list<BOUND_IMPORT_INFO> &m_BoundList)
{
	if (ImageBoundImportDescriptor)
	{
		for (int i = 0; i < 0x20; i++)
		{
			if (IsBadReadPtr((PBYTE)ImageBoundImportDescriptor + i * sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR), sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR)) ||
				ImageBoundImportDescriptor[i].TimeDateStamp == 0 || 
				ImageBoundImportDescriptor[i].OffsetModuleName == 0)
			{
				break;
			}

			if (IsBadStringPtrA((PCHAR)ImageBoundImportDescriptor + ImageBoundImportDescriptor[i].OffsetModuleName, 1))
			{
				break;
			}

			CString szModule = m_Functions.ms2ws((PCHAR)ImageBoundImportDescriptor + ImageBoundImportDescriptor[i].OffsetModuleName);

			BOUND_IMPORT_INFO info;
			info.dwRefBase = info.dwRefSize = 0;
			info.szModule = szModule;
			info.szRefModule = szModule;
			info.dwRefTimeDateStamp = ImageBoundImportDescriptor[i].TimeDateStamp;
			GetDllModuleBaseAndSize(info.szRefModule, &info.dwRefBase, &info.dwRefSize);
			m_BoundList.push_back(info);

			if (ImageBoundImportDescriptor[i].NumberOfModuleForwarderRefs > 0)
			{
				int nRefs = ImageBoundImportDescriptor[i].NumberOfModuleForwarderRefs;

				for (int j = 1; j <= nRefs; j++)
				{
					i += j;

					BOUND_IMPORT_INFO info;
					info.dwRefBase = info.dwRefSize = 0;
					info.szModule = szModule;

					if (IsBadReadPtr((PBYTE)ImageBoundImportDescriptor + i * sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR), sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR)) ||
						ImageBoundImportDescriptor[i].TimeDateStamp == 0 || 
						ImageBoundImportDescriptor[i].OffsetModuleName == 0)
					{
						break;
					}

					if (IsBadStringPtrA((PCHAR)ImageBoundImportDescriptor + ImageBoundImportDescriptor[i].OffsetModuleName, 1))
					{
						break;
					}

					info.szRefModule = m_Functions.ms2ws((PCHAR)ImageBoundImportDescriptor + ImageBoundImportDescriptor[i].OffsetModuleName);
					info.dwRefTimeDateStamp = ImageBoundImportDescriptor[i].TimeDateStamp;
					GetDllModuleBaseAndSize(info.szRefModule, &info.dwRefBase, &info.dwRefSize);
					m_BoundList.push_back(info);
				}
			}
		}
	}
}

DWORD CProcessHookDlg::GetImportFuncOffset(PVOID ImageBase, CHAR *szFunc)
{
	DWORD dwFunc = 0;
	if (!ImageBase || !szFunc)
	{
		return 0;
	}
	
	dwFunc = FindExportedRoutineInReloadModule((ULONG)ImageBase, szFunc);

	return dwFunc;
}

CString CProcessHookDlg::GetDllModulePath(DWORD dwRet)
{
	CString strPath;

	if (!dwRet)
	{
		return NULL;
	}

	for ( vector <MODULE_INFO>::iterator itor = m_vectorModules.begin(); 
		itor != m_vectorModules.end(); 
		itor++)
	{
		if (dwRet >= itor->Base && dwRet <= itor->Base + itor->Size)
		{
			MODULE_INFO info = *itor;
			strPath = info.Path;
			if (strPath.IsEmpty())
			{
				strPath = m_Functions.TrimPath(itor->Path);
			}

			break;
		}
	}

	return strPath;
}

void CProcessHookDlg::GetDllModuleBaseAndSize(CString szModuleName, DWORD *dwBase, DWORD *dwSize)
{
	if (szModuleName.IsEmpty() || !dwBase || !dwSize)
	{
		return;
	}

	for ( vector <MODULE_INFO>::iterator DllModuleIter = m_vectorModules.begin(); 
		DllModuleIter != m_vectorModules.end(); 
		DllModuleIter++)
	{
		CString szPath = DllModuleIter->Path;
		CString szName = szPath.Right(szPath.GetLength() - szPath.ReverseFind('\\') - 1);

		if (!szName.CompareNoCase(szModuleName))
		{
			*dwBase = DllModuleIter->Base;
			*dwSize = DllModuleIter->Size;
			break;
		}
	}
}

BOOL CProcessHookDlg::IsBoundImport(CString szModule, DWORD dwFunction, list<BOUND_IMPORT_INFO> BoundList)
{
	BOOL bRet = FALSE;
	if (BoundList.size() == 0 || szModule.IsEmpty() || dwFunction <= 0)
	{
		return bRet;
	}

	for (list<BOUND_IMPORT_INFO>::iterator itor = BoundList.begin(); 
		itor != BoundList.end(); 
		itor ++)
	{
		if (!szModule.CompareNoCase(itor->szModule) && 
			dwFunction >= itor->dwRefBase &&
			dwFunction <= itor->dwRefBase + itor->dwRefSize)
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}

BOOL CProcessHookDlg::IspInCodeSection(DWORD dwAddress, DWORD pBuffer, DWORD dwBase)
{
	BOOL bRet = FALSE;
	if (!pBuffer || !dwBase || !dwAddress)
	{
		return bRet;
	}

	ULONG i = 0;
	PIMAGE_SECTION_HEADER NtSection = NULL;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)pBuffer;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)(pBuffer + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		NtSection = IMAGE_FIRST_SECTION( ImageNtHeaders );
		for (i = 0; i < ImageNtHeaders->FileHeader.NumberOfSections; i++) 
		{
			if ( (NtSection->Characteristics & IMAGE_SCN_CNT_CODE) && 
				dwAddress > NtSection->VirtualAddress + dwBase && 
				dwAddress < NtSection->VirtualAddress + dwBase + NtSection->Misc.VirtualSize)
			{
				bRet = TRUE;
				break;
			}

			++NtSection;
		}
	}
	__except(1)
	{
		bRet = FALSE;
	}

	return bRet;
}

BOOL CProcessHookDlg::IsInCodeSection(DWORD dwAddress)
{
	if (!dwAddress)
	{
		return FALSE;
	}

	DWORD pBuffer = 0;
	DWORD dwBase = 0;

	for (list<DUMP_INFO>::iterator ir = m_DumpList.begin();
		ir != m_DumpList.end();
		ir++)
	{
		if (dwAddress >= ir->nBase && dwAddress <= ir->nBase + ir->nSize)
		{
			pBuffer = (DWORD)ir->pSectionBuffer;
			dwBase = ir->nBase;
			break;
		}
	}

	return IspInCodeSection(dwAddress, pBuffer, dwBase);
}

BOOL CProcessHookDlg::IsExecuteSection(DWORD dwFunction)
{
	BOOL bRet = FALSE;

	if (!dwFunction)
	{
		return bRet;
	}

	HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_OPERATION , FALSE, m_nPid);
	if (hProcess)
	{
		DWORD dwOld = 0;
		if (m_ProcessFunc.VirtualProtectEx(hProcess, (LPVOID)dwFunction, 1, PAGE_EXECUTE_READWRITE, &dwOld))
		{
			if ((dwOld & PAGE_EXECUTE) ||
				(dwOld & PAGE_EXECUTE_READ) ||
				(dwOld & PAGE_EXECUTE_READWRITE) ||
				(dwOld & PAGE_EXECUTE_WRITECOPY) ||
				(dwOld & PAGE_READWRITE) ||
				(dwOld & PAGE_WRITECOPY))
			{
				bRet = TRUE;
			}

			m_ProcessFunc.VirtualProtectEx(hProcess, (LPVOID)dwFunction, 1, dwOld, &dwOld);
		}
		
		CloseHandle(hProcess);
	}

	return bRet;
}

ULONG CProcessHookDlg::GetInlineHookAddress(PVOID pBuffer, ULONG dwBase, ULONG dwHookOffset, ULONG nCodeLen)
{
	ULONG nRet = 0;
	DISASM Instruction;
	ULONG len = 0;

	memset (&Instruction, 0, sizeof(DISASM));
	Instruction.VirtualAddr = dwBase + dwHookOffset;
	Instruction.EIP = (UIntPtr)((PBYTE)pBuffer + dwHookOffset);

	CString szAddress;
	for (ULONG i = 0; i < nCodeLen; i += len)
	{
		len = Disasm(&Instruction);
		if (len == 0)
		{
			break;
		}

		CHAR *aasm = Instruction.CompleteInstr;
		CString szAsm = m_Functions.ms2ws(aasm);
		BOOL bOk = FALSE;

		if (szAsm.Find('j') != -1) // 跳转
		{
			bOk = TRUE;
		}
		else if (szAsm.Find(L"mov") != -1) // mov 
		{
			bOk = TRUE;
		}
		else if (szAsm.Find(L"call") != -1)
		{
			bOk = TRUE;
		}
		else if (szAsm.Find(L"push") != -1)
		{
			bOk = TRUE;
		}

		if (bOk && szAsm.Find(' ') != -1)
		{
			int n = szAsm.Find(' ');
			szAddress = szAsm.Right(szAsm.GetLength() -  n - 1);
		}
	}
	
	if (!szAddress.IsEmpty())
	{
		swscanf_s(szAddress, L"%x", &nRet);
	}
	else
	{
		if (nCodeLen < 5)
		{
			nRet = GetInlineHookAddress(pBuffer, dwBase, dwHookOffset - (5 - nCodeLen), 5);
		}
	}

	if (nRet)
	{
		if (!IsExecuteSection(nRet))
		{
			nRet = 0;
		}
	}

 	return nRet;
}

PVOID CProcessHookDlg::MapFileWithoutFix(CString szPath)
{
	if (szPath.IsEmpty() || !PathFileExists(szPath))
	{
		return NULL;
	}

	HANDLE hFile = CreateFile(
		szPath, 
		GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
		NULL, 
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	DWORD dwSize = GetFileSize(hFile, NULL);
	if (dwSize == 0)
	{
		CloseHandle(hFile);
		return NULL;
	}

	PVOID pBuffer = malloc(dwSize);
	if (!pBuffer)
	{
		CloseHandle(hFile);
		return NULL;
	}

	DWORD dwRet = 0;
	if (!ReadFile(hFile, pBuffer, dwSize, &dwRet, NULL))
	{
		CloseHandle(hFile);
		free(pBuffer);
		return NULL;
	}

	CloseHandle(hFile);

	PVOID ImageBase = NULL;
	if(!ImageFile((PBYTE)pBuffer, &ImageBase) || ImageBase == NULL)
	{
		free(pBuffer);
		return NULL;
	}

	DebugLog(L"New ImageBase: 0x%08X", ImageBase);
	free(pBuffer);

	return ImageBase;
}

LRESULT CProcessHookDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	UpdateData(FALSE);
	return 0;
}

void CProcessHookDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu exportto;
	exportto.CreatePopupMenu();
	exportto.AppendMenu(MF_STRING, ID_SDT_TEXT, szText[g_enumLang]);
	exportto.AppendMenu(MF_STRING, ID_SDT_EXCEL, szExcel[g_enumLang]);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_SDT_REFRESH, szRefresh[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_DIS_CURRENT, szDisassemblerCurrentEntry[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_RESTORE, szRestore[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SDT_RESTORE_ALL, szRestoreAll[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SDT_SHUXING, szCallbackModuleProperties[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SDT_LOCATION_MODULE, szCallbackFindInExplorer[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_SDT_VERIFY_SIGN, szCallbackVerifyModuleSignature[g_enumLang]);
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)exportto.m_hMenu, szExportTo[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_SDT_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
		menu.SetMenuItemBitmaps(ID_SDT_DIS_CURRENT, MF_BYCOMMAND, &m_bmOD, &m_bmOD);
		menu.SetMenuItemBitmaps(ID_SDT_RESTORE, MF_BYCOMMAND, &m_bmRecover, &m_bmRecover);
		menu.SetMenuItemBitmaps(ID_SDT_RESTORE_ALL, MF_BYCOMMAND, &m_bmRecover, &m_bmRecover);
		menu.SetMenuItemBitmaps(ID_SDT_LOCATION_MODULE, MF_BYCOMMAND, &m_bmExplorer, &m_bmExplorer);
		menu.SetMenuItemBitmaps(ID_SDT_SHUXING, MF_BYCOMMAND, &m_bmShuxing, &m_bmShuxing);
		menu.SetMenuItemBitmaps(ID_SDT_VERIFY_SIGN, MF_BYCOMMAND, &m_bmSign, &m_bmSign);
		menu.SetMenuItemBitmaps(11, MF_BYPOSITION, &m_bmExport, &m_bmExport);
	}

	if (m_hThread && WaitForSingleObject(m_hThread, 1) != WAIT_OBJECT_0)
	{
		for (int i = 0; i < 12; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else if (!m_list.GetItemCount())
	{
		for (int i = 2; i < 12; i++)
		{
			menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
		}
	}
	else
	{
		if (m_list.GetSelectedCount() == 0)
		{
			for (int i = 2; i < 10; i++)
			{
				menu.EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() > 1)
		{
			menu.EnableMenuItem(ID_SDT_SHUXING, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_LOCATION_MODULE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_VERIFY_SIGN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_DIS_CURRENT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			menu.EnableMenuItem(ID_SDT_DIS_ORIGIN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

			BOOL bHooked = FALSE;
			POSITION pos = m_list.GetFirstSelectedItemPosition();
			while (pos)
			{
				int nIndex  = m_list.GetNextSelectedItem(pos);
				CString szHookType = m_list.GetItemText(nIndex, 3);
				if (szHookType.CompareNoCase(L"-"))
				{
					bHooked = TRUE;
					break;
				}
			}

			if (!bHooked)
			{
				menu.EnableMenuItem(ID_SDT_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			}
		}
		else if (m_list.GetSelectedCount() == 1)
		{
			int nItem = m_Functions.GetSelectItem(&m_list);

			if (nItem != -1)
			{
				CString szHookType = m_list.GetItemText(nItem, 3);

				if (!szHookType.CompareNoCase(L"-"))
				{
					menu.EnableMenuItem(ID_SDT_RESTORE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
				}
			}
		}

		menu.EnableMenuItem(ID_SDT_RESTORE_ALL, MF_BYCOMMAND | MF_ENABLED);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();
	exportto.DestroyMenu();

	*pResult = 0;
}

void CProcessHookDlg::OnSdtRefresh()
{
	if (m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	DWORD dwThreadId = 0;
	m_hThread = CreateThread(
		NULL,
		0,
		EnumHookProc,
		this,
		0,
		&dwThreadId
		);
}

void CProcessHookDlg::OnSdtDisCurrent()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szAddress = m_list.GetItemText(nItem, 2);
		if (!szAddress.IsEmpty())
		{
			CString szCurrent = szAddress.Right(szAddress.GetLength() - szAddress.ReverseFind(' ') - 1);
			if (!szCurrent.IsEmpty())
			{
				DWORD dwCurrent = m_Functions.HexStringToLong(szCurrent);
				if (dwCurrent > 0 && dwCurrent < 0x80000000)
				{
					DebugLog(L"dwCurrent: 0x%08X", dwCurrent);
					m_Functions.Disassemble(dwCurrent, 100, m_nPid, m_pEprocess);
				}
			}
		}
	}
}

void CProcessHookDlg::Restore(int nItem)
{
	CString szHookType = m_list.GetItemText(nItem, 1);

	if (!szHookType.CompareNoCase(L"iat"))
	{
		ULONG nIatAddress = (ULONG)m_list.GetItemData(nItem);
		CString szOrigin = m_list.GetItemText(nItem, 2);
		ULONG nOrigin = m_Functions.HexStringToLong(szOrigin);

		DebugLog(L"restore iat: iat: 0x%08X, nOrigin: 0x%08X", nIatAddress, nOrigin);

		if (nIatAddress && nOrigin)
		{
			HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, m_nPid);
			if (hProcess)
			{
				BOOL bSuspend = m_ProcessFunc.SuspendProcess(hProcess);

				DWORD dwProtect = 0;
				m_ProcessFunc.VirtualProtectEx ( hProcess, (LPVOID)nIatAddress, sizeof(DWORD), PAGE_READWRITE, &dwProtect );

				SIZE_T nRet = 0;
				m_ProcessFunc.WriteProcessMemory(hProcess, (LPVOID)nIatAddress, (LPVOID)&nOrigin, sizeof(DWORD), &nRet);

				m_ProcessFunc.VirtualProtectEx ( hProcess, (LPVOID)nIatAddress, sizeof(DWORD), dwProtect, &dwProtect );

				if (bSuspend)
				{
					m_ProcessFunc.ResumeProcess(hProcess);
				}

				CloseHandle(hProcess);
			}
		}
	}
	else if (!szHookType.CompareNoCase(L"inline"))
	{
		int n = (int)m_list.GetItemData(nItem);
		int i = 0;
		DebugLog(L"n: %d", n);

		for (list<INLINE_HOOK_INFO>::iterator itor = m_InlineHookInfo.begin();
			itor != m_InlineHookInfo.end();
			itor++)
		{
			if (i == n)
			{
				DWORD dwHookAddr = itor->dwBase + itor->dwHookOffset;
				DWORD dwOrigin = itor->dwReload + itor->dwHookOffset;
				DWORD dwLen = itor->dwLen;

// 				CString szXx;
// 				for (int aa = 0; aa < itor->dwLen; aa++)
// 				{
// 					CString szXxx;
// 					szXxx.Format(L"%2X ", *(PBYTE)(dwOrigin + aa));
// 					szXx += szXxx;
// 				}
// 				DebugLog(szXx);
				
				if (dwHookAddr && dwOrigin && dwLen)
				{
					DebugLog(L"inline: dwHookAddr: 0x%08X, dwOrigin: 0x%08X, dwLen: %d", dwHookAddr, dwOrigin, dwLen);

					HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, m_nPid);
					if (hProcess)
					{
						BOOL bSuspend = m_ProcessFunc.SuspendProcess(hProcess);

						DWORD dwProtect = 0;
						m_ProcessFunc.VirtualProtectEx ( hProcess, (LPVOID)dwHookAddr, dwLen, PAGE_EXECUTE_READWRITE, &dwProtect );

						SIZE_T nRet = 0;
						m_ProcessFunc.WriteProcessMemory(hProcess, (LPVOID)dwHookAddr, (LPVOID)dwOrigin, dwLen, &nRet);

						m_ProcessFunc.VirtualProtectEx ( hProcess, (LPVOID)dwHookAddr, dwLen, dwProtect, &dwProtect );

						if (bSuspend)
						{
							m_ProcessFunc.ResumeProcess(hProcess);
						}

						CloseHandle(hProcess);
					}
 				}
				
				break;
			}

			i++;
		}
	}
}

void CProcessHookDlg::OnSdtRestore()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex = m_list.GetNextSelectedItem(pos);
		Restore(iIndex);
	}

	pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int iIndex = m_list.GetNextSelectedItem(pos);
		m_list.DeleteItem(iIndex);
		pos = m_list.GetFirstSelectedItemPosition();
	}
}

void CProcessHookDlg::OnSdtRestoreAll()
{
	int nItem = m_list.GetItemCount();
	for (int i = 0; i < nItem; i++)
	{
		Restore(i);
	}

	m_list.DeleteAllItems();
}

void CProcessHookDlg::OnSdtShuxing()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.OnCheckAttribute(szPath);
	}
}

void CProcessHookDlg::OnSdtLocationModule()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.LocationExplorer(szPath);
	}
}

void CProcessHookDlg::OnSdtVerifySign()
{
	int nItem = m_Functions.GetSelectItem(&m_list);
	if (nItem != -1)
	{
		CString szPath = m_list.GetItemText(nItem, 3);
		m_Functions.SignVerify(szPath);
	}
}

void CProcessHookDlg::OnSdtText()
{
	m_Functions.ExportListToTxt(&m_list, m_szStatus);
}

void CProcessHookDlg::OnSdtExcel()
{
	WCHAR szProcessHooks[] = {'P','r','o','c','e','s','s','H','o','o','k','s','\0'};
	m_Functions.ExportListToExcel(&m_list, szProcessHooks, m_szStatus);
}

void CProcessHookDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_bQuit = TRUE;
	
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
	}
	
	CDialog::OnClose();
}
