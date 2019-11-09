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
// HexEditorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "HexEditorDlg.h"

// CHexEditorDlg 对话框

IMPLEMENT_DYNAMIC(CHexEditorDlg, CDialog)

CHexEditorDlg::CHexEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHexEditorDlg::IDD, pParent)
	, m_bReadOnly(TRUE)
	, m_bKernelAddress(FALSE)
	, m_szStaticAddress(_T(""))
	, m_szStaticSize(_T(""))
	, m_szAddress(_T(""))
	, m_szSize(_T(""))
	, m_szPid(_T(""))
	, m_szStaticPid(_T(""))
{
	m_dwSize = 0;
	m_dwAddress = 0;
	m_dwPid = 0;
	m_pHexBuffer = NULL;
}	

CHexEditorDlg::~CHexEditorDlg()
{
	if (m_pHexBuffer)
	{
		free(m_pHexBuffer);
		m_pHexBuffer = NULL;
	}
}

void CHexEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_READ_ONLY, m_bReadOnly);
	DDX_Check(pDX, IDC_CHECK_KERNEL_ADDRESS, m_bKernelAddress);
	DDX_Text(pDX, IDC_STATIC_ADDRESS, m_szStaticAddress);
	DDX_Text(pDX, IDC_STATIC_SIZE, m_szStaticSize);
	DDX_Text(pDX, IDC_EDIT_ADDRESS, m_szAddress);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_szSize);
	DDX_Text(pDX, IDC_EDIT_PID, m_szPid);
	DDX_Text(pDX, IDC_STATIC_PID, m_szStaticPid);
	DDX_Control(pDX, IDC_HEX_EDIT, m_editHex);
	DDX_Control(pDX, IDC_CHECK_READ_ONLY, m_btnReadOnly);
	DDX_Control(pDX, IDC_CHECK_KERNEL_ADDRESS, m_btnKernelAddress);
	DDX_Control(pDX, IDC_PROCESS_LIST, m_ProcessList);
}


BEGIN_MESSAGE_MAP(CHexEditorDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDOK, &CHexEditorDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_READ_ONLY, &CHexEditorDlg::OnBnClickedCheckReadOnly)
	ON_BN_CLICKED(IDC_CHECK_KERNEL_ADDRESS, &CHexEditorDlg::OnBnClickedCheckKernelAddress)
	ON_EN_CHANGE(IDC_HEX_EDIT, &CHexEditorDlg::OnEnChangeHexEdit)
	ON_COMMAND(ID_PROCESS_REFRESH, &CHexEditorDlg::OnProcessRefresh)
	ON_NOTIFY(NM_RCLICK, IDC_PROCESS_LIST, &CHexEditorDlg::OnNMRclickProcessList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROCESS_LIST, &CHexEditorDlg::OnLvnItemchangedProcessList)
	ON_BN_CLICKED(IDC_WRITE_MEMORY, &CHexEditorDlg::OnBnClickedWriteMemory)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CHexEditorDlg)
	EASYSIZE(IDC_HEX_EDIT, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
	EASYSIZE(IDC_PROCESS_LIST, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
END_EASYSIZE_MAP
// CHexEditorDlg 消息处理程序

void CHexEditorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CHexEditorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_szStaticAddress = szDumpAddress[g_enumLang];
	m_szStaticSize = szDisassmSize[g_enumLang];
	m_szStaticPid = szHexEditerPid[g_enumLang];
	GetDlgItem(IDOK)->SetWindowText(szReadMemory[g_enumLang]);
	GetDlgItem(IDC_STATIC_PROCESS_LIST)->SetWindowText(szProcessList[g_enumLang]);
	GetDlgItem(IDC_WRITE_MEMORY)->SetWindowText(szConfirmToWrite[g_enumLang]);
	GetDlgItem(IDC_WRITE_MEMORY)->EnableWindow(FALSE);

	m_ProImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 2, 2); 
	m_ProcessList.SetImageList (&m_ProImageList, LVSIL_SMALL);
	m_ProcessList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ProcessList.InsertColumn(0, szImageName[g_enumLang], LVCFMT_LEFT, 130);
	m_ProcessList.InsertColumn(1, szPid[g_enumLang], LVCFMT_LEFT, 70);

	if (m_dwSize == 0)
	{
		m_dwSize = 0x300;
	}

	// 一行显示16个字符
	m_editHex.SetBPR(16);
	// 不允许删除
	m_editHex.SetAllowDeletes(FALSE);
	// 设置只读风格
	m_editHex.ModifyStyle( m_bReadOnly ? 0 :ES_READONLY,
		m_bReadOnly ? ES_READONLY : 0 );

	m_szAddress.Format(L"%08X", m_dwAddress);
	m_szSize.Format(L"%X", m_dwSize);
	m_szPid.Format(L"%d", m_dwPid);
	m_btnReadOnly.SetWindowText(szReadOnly[g_enumLang]);
	m_btnKernelAddress.SetWindowText(szKernelAddress[g_enumLang]);
	
	UpdateData(FALSE);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CHexEditorDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	
	if(pMsg->message == WM_KEYDOWN)
	{
		int id = ::GetDlgCtrlID(pMsg->hwnd);
		if (id == IDC_HEX_EDIT)
		{
			// 如果是insert键，就直接返回
			if (VK_INSERT == pMsg->wParam)
			{
				return TRUE;
			}
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CHexEditorDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	m_dwSize = 0;
	if (!m_szSize.IsEmpty())
	{
		swscanf_s(m_szSize.GetBuffer(), L"%x", &m_dwSize);
		m_szSize.ReleaseBuffer();
	}

	m_dwAddress = 0;
	if (!m_szAddress.IsEmpty())
	{
		swscanf_s(m_szAddress.GetBuffer(), L"%x", &m_dwAddress);
		m_szAddress.ReleaseBuffer();
	}
	
	m_dwPid = 0;
	if (!m_szPid.IsEmpty())
	{
		swscanf_s(m_szPid.GetBuffer(), L"%d", &m_dwPid);
		m_szPid.ReleaseBuffer();
	}

	if (m_dwSize <= 0 || m_dwAddress <= 0)
	{
		MessageBox(szHexEditerInputHaveWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
		return;
	}

	if (m_bKernelAddress)
	{
		if (m_dwAddress < 0x80000000)
		{
			MessageBox(szHexEditerKernelModeAddressWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
			return;
		}
	}
	else
	{
		if (m_dwAddress >= 0x80000000)
		{
			MessageBox(szHexEditerUserModeAddressWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
			return;
		}

		if (m_dwPid <= 0)
		{
			MessageBox(szHexEditerUserModePidWrong[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
			return;
		}
	}

 	DumpMemoryToRing3(m_dwAddress, m_dwSize, m_dwPid);
}

static BOOL g_bFirstChange = TRUE;

void CHexEditorDlg::OnBnClickedCheckReadOnly()
{
	UpdateData(TRUE);

	m_editHex.ModifyStyle( m_bReadOnly ? 0 :ES_READONLY,
		m_bReadOnly ? ES_READONLY : 0 );

	m_editHex.Invalidate();
	m_editHex.UpdateWindow();

	if (m_bReadOnly)
	{
		GetDlgItem(IDC_WRITE_MEMORY)->EnableWindow(!m_bReadOnly);
		g_bFirstChange = TRUE;
	}
}

void CHexEditorDlg::OnBnClickedCheckKernelAddress()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_PID)->EnableWindow(!m_bKernelAddress);
	m_ProcessList.EnableWindow(!m_bKernelAddress);
}

BOOL CHexEditorDlg::DumpMemoryToRing3(DWORD Address, DWORD Size, DWORD Pid, DWORD pEprocess)
{
	BOOL bRet = FALSE;

	if (Size <= 0 || Address <= 0)
	{
		return FALSE;
	}

	if (m_pHexBuffer)
	{
		free(m_pHexBuffer);
		m_pHexBuffer = NULL;
	}

	m_pHexBuffer = malloc(Size);
	if (m_pHexBuffer)
	{
		memset(m_pHexBuffer, 0, Size);

		COMMUNICATE_DISASSEMBLY cd;
		cd.OpType = enumDisassembly;
		cd.nPid = Pid;
		cd.pEprocess = pEprocess;
		cd.Base = Address;
		cd.Size = Size;

		bRet = m_Driver.CommunicateDriver(&cd, sizeof(COMMUNICATE_DISASSEMBLY), m_pHexBuffer, Size, NULL);
		if (bRet)
		{
			m_editHex.SetAddressBase(Address);
			m_editHex.SetData((PBYTE)m_pHexBuffer, Size, -1);
			m_editHex.RedrawWindow();
		}
	}

	if (!bRet)
	{
		m_editHex.SetAddressBase(Address);
		m_editHex.SetData((PBYTE)NULL, 0, -1);
		m_editHex.RedrawWindow();
	}

	return bRet;
}

DWORD CHexEditorDlg::GetModifyAddress(PVOID pBuffer, list<MODIFY_INFO> &ModifyList)
{
	DWORD dwAddress = 0;
	if (!pBuffer)
	{
		return dwAddress;
	}
	
	ModifyList.clear();

	for (ULONG i = 0; i < m_dwSize; i++)
	{
		PBYTE pOrigin = (PBYTE)m_pHexBuffer;
		PBYTE pNow = (PBYTE)pBuffer;

		if (pOrigin[i] != pNow[i])
		{
			dwAddress = i + m_dwAddress;
			
			MODIFY_INFO info;
			info.dwAddress = dwAddress;
			info.btContent = pNow[i];
			ModifyList.push_back(info);

		//	break;
		}
	}

	MODIFY_INFO info;
	info.dwAddress = 0;
	info.btContent = 0;
	ModifyList.push_back(info);

	return dwAddress;
}

void CHexEditorDlg::OnEnChangeHexEdit()
{
	if (g_bFirstChange)
	{
		GetDlgItem(IDC_WRITE_MEMORY)->EnableWindow(TRUE);
		UpdateData(FALSE);
		g_bFirstChange = FALSE;
	}
	

// 	if (!m_bKernelAddress)
// 	{
// 		BOOL bChange = FALSE;
// 		PBYTE pBuffer = (PBYTE)malloc(m_dwSize);
// 		if (!pBuffer)
// 		{
// 			return;
// 		}
// 
// 		HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, m_dwPid);
// 		if (!hProcess)
// 		{
// 			free(pBuffer);
// 			return;
// 		}
// 
// 		memset(pBuffer, 0, m_dwSize);
// 		m_editHex.GetData(pBuffer, m_dwSize);
// 		
// 		DWORD dwAddress = GetModifyAddress(pBuffer);
// 		if (dwAddress < 0x80000000 && dwAddress >= m_dwAddress)
// 		{
// 			DWORD dwOldProtect;
// 			if (m_ProcessFunc.VirtualProtectEx(hProcess, (PVOID)dwAddress, 1, PAGE_READWRITE, &dwOldProtect))
// 			{
// 				DWORD dwRet = 0;
// 				bChange = m_ProcessFunc.WriteProcessMemory(hProcess, (PVOID)dwAddress, (PVOID)(((PBYTE)pBuffer) + dwAddress - m_dwAddress), 1, &dwRet);
// 				m_ProcessFunc.VirtualProtectEx(hProcess, (PVOID)dwAddress, 1, dwOldProtect, &dwOldProtect);
// 			}
// 		}
// 
// 		CloseHandle(hProcess);
// 
// 		if (bChange)
// 		{
// 			if (m_pHexBuffer)
// 			{
// 				free(m_pHexBuffer);
// 				m_pHexBuffer = NULL;
// 			}
// 
// 			m_pHexBuffer = pBuffer;
// 		}
// 		else
// 		{
// 			m_editHex.SetAddressBase(m_dwAddress);
// 			m_editHex.SetData((PBYTE)m_pHexBuffer, m_dwSize, -1);
// 			m_editHex.RedrawWindow();
// 
// 			free(pBuffer);
// 			pBuffer = NULL;
// 		}
// 	}
// 	else
// 	{
// 		if (g_bFirstChange)
// 		{
// 			if (MessageBox(szHexEditerModifyKernelMode[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
// 			{
// 				m_editHex.SetAddressBase(m_dwAddress);
// 				m_editHex.SetData((PBYTE)m_pHexBuffer, m_dwSize, -1);
// 				m_editHex.RedrawWindow();
// 				return;
// 			}
// 
// 			g_bFirstChange = FALSE;
// 		}
// 
// 		BOOL bChange = FALSE;
// 		PBYTE pBuffer = (PBYTE)malloc(m_dwSize);
// 		if (!pBuffer)
// 		{
// 			return;
// 		}
// 		
// 		memset(pBuffer, 0, m_dwSize);
// 		m_editHex.GetData(pBuffer, m_dwSize);
// 		
// 		DWORD dwAddress = GetModifyAddress(pBuffer);
// 		if (dwAddress > 0x80000000 && dwAddress >= m_dwAddress)
// 		{
// 			COMMUNICATE_MODIFY_KERNEL_ADDRESS mka;
// 			mka.OpType = enumModifyKernelAddress;
// 			mka.nAddress = dwAddress;
// 			mka.nLen = 1;
// 			mka.pContent = (PBYTE)(((PBYTE)pBuffer) + dwAddress - m_dwAddress);
// 			bChange = m_Driver.CommunicateDriver(&mka, sizeof(COMMUNICATE_MODIFY_KERNEL_ADDRESS), NULL , 0, NULL);
// 		}
// 
// 		if (bChange)
// 		{
// 			if (m_pHexBuffer)
// 			{
// 				free(m_pHexBuffer);
// 				m_pHexBuffer = NULL;
// 			}
// 
// 			m_pHexBuffer = pBuffer;
// 		}
// 		else
// 		{
// 			free(pBuffer);
// 			pBuffer = NULL;
// 		}
// 	}
}

void CHexEditorDlg::ListProcess()
{
	m_ProcessList.DeleteAllItems();
	int nImageCount = m_ProImageList.GetImageCount();
	for(int j = 0; j < nImageCount; j++)   
	{   
		m_ProImageList.Remove(0);   
	}

	vector<PROCESS_INFO> vectorProcess;
	CListProcess ListProc;
	ListProc.EnumProcess(vectorProcess);

	for ( vector <PROCESS_INFO>::iterator Iter = vectorProcess.begin( ); 
		Iter != vectorProcess.end( ); 
		Iter++ )
	{
		PROCESS_INFO ProcessItem = *Iter;

		if (ProcessItem.ulPid == 0 && ProcessItem.ulParentPid == 0 && ProcessItem.ulEprocess == 0)
		{
			continue;
		}

		CString strPath = m_Functions.TrimPath(ProcessItem.szPath);

		// 处理进程图标
		SHFILEINFO sfi;
		DWORD_PTR nRet;
		ZeroMemory(&sfi, sizeof(SHFILEINFO));
		nRet = SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		nRet ? m_ProImageList.Add ( sfi.hIcon ) : m_ProImageList.Add ( LoadIcon(NULL, IDI_APPLICATION) );
		if (nRet)
		{
			DestroyIcon(sfi.hIcon);
		}

		CString szPid;
		szPid.Format(L"%d", ProcessItem.ulPid);

		CString szImage = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

		int nItem = m_ProcessList.GetItemCount();
		m_ProcessList.InsertItem(nItem, szImage, nItem);
		m_ProcessList.SetItemText(nItem, 1, szPid);
	}
}

void CHexEditorDlg::OnProcessRefresh()
{
	ListProcess();
}

void CHexEditorDlg::OnNMRclickProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_PROCESS_REFRESH, szRefresh[g_enumLang]);

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		// 设置菜单图标
		menu.SetMenuItemBitmaps(ID_PROCESS_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	*pResult = 0;
}

void CHexEditorDlg::OnLvnItemchangedProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int nItem = pNMLV->iItem;
	m_szPid = m_ProcessList.GetItemText(nItem, 1);
	UpdateData(FALSE);
	*pResult = 0;
}

BOOL CHexEditorDlg::ModifyUserAddress()
{
	BOOL bChange = FALSE;
	PBYTE pBuffer = (PBYTE)malloc(m_dwSize);
	if (!pBuffer)
	{
		goto _faild;
	}

	HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, m_dwPid);
	if (!hProcess)
	{
		free(pBuffer);
		goto _faild;
	}

	memset(pBuffer, 0, m_dwSize);
	m_editHex.GetData(pBuffer, m_dwSize);

	DWORD dwOldProtect;
	if (m_ProcessFunc.VirtualProtectEx(hProcess, (PVOID)m_dwAddress, m_dwSize, PAGE_READWRITE, &dwOldProtect))
	{
		DWORD dwRet = 0;
		bChange = m_ProcessFunc.WriteProcessMemory(hProcess, (PVOID)m_dwAddress, pBuffer, m_dwSize, &dwRet);
		m_ProcessFunc.VirtualProtectEx(hProcess, (PVOID)m_dwAddress, m_dwSize, dwOldProtect, &dwOldProtect);
	}

	CloseHandle(hProcess);

	if (bChange)
	{
		if (m_pHexBuffer)
		{
			free(m_pHexBuffer);
			m_pHexBuffer = NULL;
		}

 		m_pHexBuffer = pBuffer;
	}
	else
	{	
		m_editHex.SetAddressBase(m_dwAddress);
		m_editHex.SetData((PBYTE)m_pHexBuffer, m_dwSize, -1);
		m_editHex.RedrawWindow();

		free(pBuffer);
		pBuffer = NULL;
	}

_faild:
	return bChange;
}

void CHexEditorDlg::OnBnClickedWriteMemory()
{
	BOOL bChange = FALSE;

	if (!m_bKernelAddress && m_dwAddress < 0x80000000)
	{
		if (MessageBox(szAreYouSureModifyMemory[g_enumLang], szToolName, MB_YESNO) == IDNO)
		{
			m_editHex.SetAddressBase(m_dwAddress);
			m_editHex.SetData((PBYTE)m_pHexBuffer, m_dwSize, -1);
			m_editHex.RedrawWindow();
			return;
		}

		bChange = ModifyUserAddress();
	}
	else if (m_dwAddress >= 0x80000000 && m_bKernelAddress)
 	{
		if (MessageBox(szHexEditerModifyKernelMode[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			m_editHex.SetAddressBase(m_dwAddress);
			m_editHex.SetData((PBYTE)m_pHexBuffer, m_dwSize, -1);
			m_editHex.RedrawWindow();
			return;
		}

		bChange = ModifyKernelAddress();
	}

	if (bChange)
	{
		g_bFirstChange = TRUE;
		GetDlgItem(IDC_WRITE_MEMORY)->EnableWindow(FALSE);
		MessageBox(szModifyMemorySuccess[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(szModifyMemoryFailed[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
	}
}

BOOL CHexEditorDlg::CommWithDriver(PBYTE pBuffer, DWORD dwAddress, DWORD dwLen)
{
	BOOL bRet = FALSE;
	
	if ( !pBuffer || !dwLen )
	{
		return bRet;
	}

	COMMUNICATE_MODIFY_KERNEL_ADDRESS mka;
	mka.OpType = enumModifyKernelAddress;
	mka.nAddress = dwAddress;
	mka.nLen = dwLen;
	mka.pContent = pBuffer;
	bRet = m_Driver.CommunicateDriver(&mka, sizeof(COMMUNICATE_MODIFY_KERNEL_ADDRESS), NULL , 0, NULL);

	return bRet;
}

BOOL CHexEditorDlg::ModifyKernelAddress()
{
	BOOL bChange = FALSE;
	PBYTE pBuffer = (PBYTE)malloc(m_dwSize);
	if (!pBuffer)
	{
		return bChange;
	}

	memset(pBuffer, 0, m_dwSize);
	m_editHex.GetData(pBuffer, m_dwSize);

	list<MODIFY_INFO> ModifyList;
	GetModifyAddress(pBuffer, ModifyList);

	if (ModifyList.size() > 1)
	{
		DWORD dwPreAddress = 0, dwInitAddress = 0;
		BOOL bFirst = TRUE;
		DWORD dwLen = 0;
		list<byte> btList;

		for (list<MODIFY_INFO>::iterator ir = ModifyList.begin();
			ir != ModifyList.end();
			ir++)
		{
			if (bFirst)
			{
				btList.clear();
				dwInitAddress = dwPreAddress = ir->dwAddress;
				bFirst = FALSE;
				btList.push_back(ir->btContent);
				dwLen++;
			}
			else
			{
				if (++dwPreAddress != ir->dwAddress)
				{
					PBYTE pContent = (PBYTE)malloc(dwLen);
					if (pContent == NULL)
					{
						break;
					}

					DWORD dwTemp = 0;
					for (list<byte>::iterator or = btList.begin();
						or != btList.end();
						or++)
					{
						pContent[dwTemp++] = *or;
					}

					// 如果修改失败了，那么就直接退出
					bChange = CommWithDriver(pContent, dwInitAddress, dwLen);
					if (!bChange)
					{
						free(pContent);
						break;
					}

					free(pContent);
					dwLen = 0;
					bFirst = TRUE;

					if (ir->dwAddress == 0)
					{
						break;
					}
					else
					{
						btList.clear();
						dwInitAddress = dwPreAddress = ir->dwAddress;
						bFirst = FALSE;
						btList.push_back(ir->btContent);
						dwLen++;
					}
				}
				else
				{
					btList.push_back(ir->btContent);
					dwLen++;
				}
			}
		}

		if (bChange)
		{
			if (m_pHexBuffer)
			{
				free(m_pHexBuffer);
				m_pHexBuffer = NULL;
			}

			m_pHexBuffer = pBuffer;
		}
		else
		{
			free(pBuffer);
			pBuffer = NULL;
		}
	}

	return bChange;
}