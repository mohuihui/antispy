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
// MbrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "MbrDlg.h"
#include <winioctl.h> 
#include "libdasm.h"

// CMbrDlg dialog

IMPLEMENT_DYNAMIC(CMbrDlg, CDialog)

CMbrDlg::CMbrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMbrDlg::IDD, pParent)
{
	m_szStatus = L"";
}

CMbrDlg::~CMbrDlg()
{
}

void CMbrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO, m_Drivers);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CMbrDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMbrDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_READ_MBR, &CMbrDlg::OnBnClickedBtnReadMbr)
	ON_BN_CLICKED(IDC_BTN_SAVE_MBR, &CMbrDlg::OnBnClickedBtnSaveMbr)
	ON_BN_CLICKED(IDC_BTN_RESTORE_MBR, &CMbrDlg::OnBnClickedBtnRestoreMbr)
	ON_BN_CLICKED(IDC_BTN_RESTORE_DEFAULT_MBR, &CMbrDlg::OnBnClickedBtnRestoreDefaultMbr)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_COMMAND(ID_DISEMBLY_EXPORT_TEXT, &CMbrDlg::OnDisemblyExportText)
	ON_COMMAND(ID_DISEMBLY_EXPORT_EXCEL, &CMbrDlg::OnDisemblyExportExcel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CMbrDlg::OnNMRclickList)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CMbrDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_KEEPSIZE, ES_BORDER, 0)
END_EASYSIZE_MAP
// CMbrDlg message handlers

void CMbrDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
// 	OnOK();
}

BOOL CMbrDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}

#define FILE_OPEN                       0x00000001
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020

void CMbrDlg::InitPhysicDrives()
{
	WCHAR szPhysicDriver[] = {'\\','D','o','s','D','e','v','i','c','e','s','\\','P','H','Y','S','I','C','A','L','D','R','I','V','E','%','d','\0'};
// 	WCHAR szPhysicDriver[] = {'\\','\\','.','\\','P','H','Y','S','I','C','A','L','D','R','I','V','E','%','d','\0'};

	for (int i = 0; i < 4; i++)
	{
		WCHAR szTemp[MAX_PATH] = {0};
		wsprintf(szTemp, szPhysicDriver, i);
	
		HANDLE hDevice = m_FileFunc.CreateEx(szTemp,
			wcslen(szTemp) * sizeof(WCHAR),
			GENERIC_READ | GENERIC_WRITE,
			FILE_ATTRIBUTE_DEVICE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			FILE_OPEN,
			FILE_SYNCHRONOUS_IO_NONALERT);

// 		CString szTemp;
// 		szTemp.Format(szPhysicDriver, i); 
// 
// 		HANDLE hDevice = CreateFile( szTemp, 
// 			GENERIC_READ | GENERIC_WRITE, 
// 			FILE_SHARE_READ | FILE_SHARE_WRITE, 
// 			NULL, 
// 			OPEN_EXISTING, 
// 			0, 
// 			NULL 
// 			); 

		if (hDevice != NULL) 
		{
// 		if (hDevice != INVALID_HANDLE_VALUE) 
// 		{
			int nCnt = m_Drivers.GetCount();
			CString szAddString;
			szAddString.Format(L"%s%d", szDiskName[g_enumLang], nCnt + 1);
			m_Drivers.InsertString(nCnt, szAddString);

			CloseHandle(hDevice);
		} 
	}

	int nCnt = m_Drivers.GetCount();
	if (nCnt)
	{
		m_Drivers.SetCurSel(0);
	}
}

BOOL CMbrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitPhysicDrives();

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/);
	m_list.InsertColumn(0, szAddress[g_enumLang], LVCFMT_LEFT, 150);
//	m_list.InsertColumn(1, szHex[g_enumLang], LVCFMT_LEFT, 200);
	m_list.InsertColumn(2, szDisassembly[g_enumLang], LVCFMT_LEFT, 250);

	GetDlgItem(IDC_STATIC_STATUS_OK)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_STATUS_ERROR)->ShowWindow(FALSE);
	
	GetDlgItem(IDC_STATIC_SELECT_DISK)->SetWindowText(szMBRSelectDisk[g_enumLang]);
	GetDlgItem(IDC_STATIC_STATUS_OK)->SetWindowText(szMBRIsOk[g_enumLang]);
	GetDlgItem(IDC_STATIC_STATUS_ERROR)->SetWindowText(szMBRIsError[g_enumLang]);
	GetDlgItem(IDC_STATIC_WHAT_IS_MBR)->SetWindowText(szWhatIsMBR[g_enumLang]);
	GetDlgItem(IDC_STATIC_WHAT_CAN_RESTORE)->SetWindowText(szHowToReapirMBR[g_enumLang]);
	GetDlgItem(IDC_STATIC_ZHUYI)->SetWindowText(szMBRnote[g_enumLang]);

	GetDlgItem(IDC_BTN_READ_MBR)->SetWindowText(szReadAndCheckMBR[g_enumLang]);
	GetDlgItem(IDC_BTN_SAVE_MBR)->SetWindowText(szBackupMBR[g_enumLang]);
	GetDlgItem(IDC_BTN_RESTORE_MBR)->SetWindowText(szRestoreMBR[g_enumLang]);
	GetDlgItem(IDC_BTN_RESTORE_DEFAULT_MBR)->SetWindowText(szRestoreDefaultMBR[g_enumLang]);

	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

HANDLE CMbrDlg::GetDiskHandle()
{
//	CString szDeviceName;
//	WCHAR szPhysicDriver[] = {'\\','\\','.','\\','P','H','Y','S','I','C','A','L','D','R','I','V','E','%','d','\0'};
	CString szDevice;
	WCHAR szPhysicDriver[] = {'\\','D','o','s','D','e','v','i','c','e','s','\\','P','H','Y','S','I','C','A','L','D','R','I','V','E','%','d','\0'};

	m_Drivers.GetLBText(m_Drivers.GetCurSel(), szDevice);
	if (szDevice.IsEmpty())
	{
		return NULL;
	}

	WCHAR szTemp[MAX_PATH] = {0};
	
	for (int i = 0; i < 4; i++)
	{
		char ch = 48 + i + 1;
		if (szDevice.Find(ch) != -1)
		{
		//	szDeviceName.Format(szPhysicDriver, i);
			wsprintf(szTemp, szPhysicDriver, i);
			break;
		}
	}

	if (wcslen(szTemp) == 0)
	{
		return NULL;
	}

// 	if (szDeviceName.IsEmpty())
// 	{
// 		return NULL;
// 	}

// 	HANDLE hDevice = CreateFile( szDeviceName, 
// 		GENERIC_READ | GENERIC_WRITE, 
// 		FILE_SHARE_READ | FILE_SHARE_WRITE, 
// 		NULL, 
// 		OPEN_EXISTING, 
// 		0, 
// 		NULL 
// 		); 
// 
// 	if (hDevice == INVALID_HANDLE_VALUE) 
// 	{ 
// 		return NULL; 
// 	} 

	HANDLE hDevice = m_FileFunc.CreateEx(szTemp,
		wcslen(szTemp) * sizeof(WCHAR),
		GENERIC_READ | GENERIC_WRITE,
		FILE_ATTRIBUTE_DEVICE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT);

	return hDevice;
}

PVOID CMbrDlg::ReadMBR() 
{ 
// 	HANDLE hDevice = GetDiskHandle();
// 	if (hDevice == NULL) 
// 	{ 
// 		return NULL; 
// 	} 
// 
// 	DWORD bread,count;
// 	DeviceIoControl(hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &count, NULL); 
// 
// 	DISK_GEOMETRY Geometry; 
// 	if (!DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &Geometry, sizeof(DISK_GEOMETRY), &count, NULL))
// 	{
// 		CloseHandle(hDevice);
// 		return NULL;
// 	}
// 	
// 	ULONG nSectorSize = Geometry.BytesPerSector;
// 	PVOID pBuffer = malloc(nSectorSize);
// 	if (pBuffer == NULL)
// 	{
// 		CloseHandle(hDevice);
// 		return NULL;
// 	}
// 
// 	memset(pBuffer, 0, nSectorSize);
// 
// 	//BOOL bRet = ReadFile(hDevice, pBuffer, nSectorSize, &bread, NULL); 
// 	BOOL bRet = m_FileFunc.ReadEx(hDevice, pBuffer, nSectorSize); 
// 	if (bRet == FALSE) 
// 	{ 
// 		CloseHandle(hDevice);
// 		free(pBuffer);
// 		return NULL;
// 	} 
// 
// 	DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &count,NULL); 
// 	CloseHandle(hDevice); 
	
	CString szDevice;
	WCHAR szPhysicDriver[] = {'\\','D','e','v','i','c','e','\\','H','a','r','d','d','i','s','k','%','d','\\','D','R','0','\0'};

	m_Drivers.GetLBText(m_Drivers.GetCurSel(), szDevice);
	if (szDevice.IsEmpty())
	{
		return NULL;
	}

	WCHAR szTemp[MAX_PATH] = {0};

	for (int i = 0; i < 4; i++)
	{
		char ch = 48 + i + 1;
		if (szDevice.Find(ch) != -1)
		{
			wsprintf(szTemp, szPhysicDriver, i);
			break;
		}
	}

	if (wcslen(szTemp) == 0)
	{
		return NULL;
	}

	PVOID pBuffer = malloc(512);
	if (pBuffer == NULL)
	{
		return NULL;
	}

	memset(pBuffer, 0, 512);

	COMMUNICATE_MBR cmbr;
	cmbr.OpType = enumReadMBR;
	cmbr.op.Read.nReadBytes = 512;
	cmbr.op.Read.pBuffer = pBuffer;
	cmbr.op.Read.szDiskName = szTemp;
	m_Driver.CommunicateDriver(&cmbr, sizeof(COMMUNICATE_MBR), NULL, 0, NULL);
	return pBuffer;
}

BOOL CMbrDlg::CheckMBR()
{
	BOOL bRet = FALSE;
	PVOID pNowMBR = ReadMBR();

	DWORD dwSize = 0;
	PVOID pOriginMBR = GetDefaultMBR(&dwSize);
	
	if (pOriginMBR && pNowMBR && dwSize)
	{
		if (g_WinVersion == enumWINDOWS_XP || g_WinVersion == enumWINDOWS_2K3 || g_WinVersion == enumWINDOWS_2K3_SP1_SP2)
		{
			dwSize -= 4;
		}

		if (!memcmp(pOriginMBR, pNowMBR, dwSize))
		{
			bRet = TRUE;
		}
	}

	if (pNowMBR)
	{
		free(pNowMBR);
	}

	if (pOriginMBR)
	{
		free(pOriginMBR);
	}

	return bRet;
}

void CMbrDlg::OnBnClickedBtnReadMbr()
{
	m_list.DeleteAllItems();

	PVOID pMbr = ReadMBR();
	if (pMbr == NULL)
	{	
		MessageBox(szReadMbrFalied[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
		return;
	}

	if (CheckMBR())
	{
		CString szText;
		CString szDevice;
		m_Drivers.GetLBText(m_Drivers.GetCurSel(), szDevice);
		szText.Format(szMBRIsOk[g_enumLang], szDevice);
		m_szStatus = szText;
		GetDlgItem(IDC_STATIC_STATUS_OK)->SetWindowText(szText);
		GetDlgItem(IDC_STATIC_STATUS_OK)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_STATUS_ERROR)->ShowWindow(FALSE);
	}
	else
	{
		CString szText;
		CString szDevice;
		m_Drivers.GetLBText(m_Drivers.GetCurSel(), szDevice);
		szText.Format(szMBRIsError[g_enumLang], szDevice);
		m_szStatus = szText;
		GetDlgItem(IDC_STATIC_STATUS_ERROR)->SetWindowText(szText);
		GetDlgItem(IDC_STATIC_STATUS_OK)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_STATUS_ERROR)->ShowWindow(TRUE);
	}

	DWORD dwBase = 0x7c00;
	DWORD dwLen = 0;
	for (int i = 0; i < 512; i += dwLen)
	{
		INSTRUCTION	Inst;
		memset(&Inst, 0, sizeof(INSTRUCTION));
		get_instruction(&Inst, (PBYTE)pMbr+i, MODE_16);
	
		if (Inst.length == 0)
		{
			break;
		}

		char string[1024] = {0};
		int length = 1024;
		get_instruction_string(&Inst, FORMAT_INTEL, dwBase + i, string, length);
		
		CString szAddress;
		szAddress.Format(L"0x%08X", dwBase + i);
		int nItem = m_list.InsertItem(m_list.GetItemCount(), szAddress);

		CString szAsm;
		szAsm = m_Functions.ms2ws(string);
		m_list.SetItemText(nItem, 1, szAsm);

		if (Inst.type == INSTRUCTION_TYPE_RET)
		{
			break;
		}

		dwLen = Inst.length;
	}

	free(pMbr);
}

void CMbrDlg::OnBnClickedBtnSaveMbr()
{
	TCHAR szDate[MAX_PATH] = {0};	
	GetDateFormat(NULL, 0, NULL, _T("yyyyMMdd"), szDate, MAX_PATH);
	WCHAR szBin[] = {'_','M','B','R','.','b','i','n','\0'};
	CString szMbrName = szDate + CString(szBin);

	CFileDialog fileDlg(
		FALSE, 
		0, 
		(LPWSTR)szMbrName.GetBuffer(), 
		0, 
		L"Bin Files (*.bin)|*.bin||",
		0
		);

	szMbrName.ReleaseBuffer();

	if (IDOK != fileDlg.DoModal())
	{
		return;
	}
	
	CString szFilePath = fileDlg.GetFileName();
	if (szFilePath.Find('.') == -1)
	{
		WCHAR szBin[] = {'.','b','i','n','\0'};
		szFilePath += szBin;
	}

	BOOL bCopy = FALSE;
	if ( !PathFileExists(szFilePath) ||
		(PathFileExists(szFilePath) && MessageBox(szFileExist[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDYES))
	{
		PVOID pMbr = ReadMBR();
		if (pMbr)
		{
			CFile file;
			TRY 
			{
				if (file.Open(szFilePath,  CFile::modeCreate | CFile::modeWrite))
				{
					file.Write(pMbr, 512);
					file.Close();
					bCopy = TRUE;
				}
			}
			CATCH_ALL( e )
			{
				file.Abort();   // close file safely and quietly
				//THROW_LAST();
			}
			END_CATCH_ALL
			
			free(pMbr);
		}
	}
	
	if (bCopy)
	{
		MessageBox(szBackupMBROK[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(szBackupMBRERROR[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
	}
}

BOOL CMbrDlg::WriteMBR(PVOID pBuffer, ULONG nWriteBytes)
{
	BOOL bRet = FALSE;
	if (pBuffer == NULL || !nWriteBytes)
	{
		return bRet;
	}

	CString szDevice;
	WCHAR szPhysicDriver[] = {'\\','D','e','v','i','c','e','\\','H','a','r','d','d','i','s','k','%','d','\\','D','R','0','\0'};

	m_Drivers.GetLBText(m_Drivers.GetCurSel(), szDevice);
	if (szDevice.IsEmpty())
	{
		return NULL;
	}

	WCHAR szTemp[MAX_PATH] = {0};

	for (int i = 0; i < 4; i++)
	{
		char ch = 48 + i + 1;
		if (szDevice.Find(ch) != -1)
		{
			wsprintf(szTemp, szPhysicDriver, i);
			break;
		}
	}

	if (wcslen(szTemp) == 0)
	{
		return bRet;
	}

	COMMUNICATE_MBR cmbr;
	cmbr.OpType = enumWriteMBR;
	cmbr.op.Write.nWriteBytes = nWriteBytes;
	cmbr.op.Write.pBuffer = pBuffer;
	cmbr.op.Write.szDiskName = szTemp;
	bRet = m_Driver.CommunicateDriver(&cmbr, sizeof(COMMUNICATE_MBR), NULL, 0, NULL);

	return bRet;
}

void CMbrDlg::RestoreMbrFormBackupFile(CString szPath)
{
	if (szPath.IsEmpty())
	{
		return;
	}

	HANDLE hFile = CreateFile(szPath, 
		GENERIC_READ | SYNCHRONIZE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	CString szMessage = szRestoreMBRERROR[g_enumLang];

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hFile, NULL);
		if (dwSize == 512)
		{
			PVOID pBuffer = malloc(512);

			if (pBuffer)
			{
				memset(pBuffer, 0, 512);
				
			//	DWORD dwRet = 0;
			//	if (ReadFile(hFile, pBuffer, 512, &dwRet, NULL))
				if (m_FileFunc.ReadEx(hFile, pBuffer, 512))
				{
					if (*(WORD*)((PBYTE)pBuffer + 510) != 0xAA55 || *((PBYTE)pBuffer + 0x1BE) != 0x80)
					{
						szMessage = szMbrFileInvalid[g_enumLang];
					}
					else
					{
// 						HANDLE hDevice = GetDiskHandle();
// 						if (hDevice != NULL)
// 						{
							/*if (WriteFile(hDevice, pBuffer, 512, &dwRet, NULL))*/
							//if (m_FileFunc.WriteEx(hDevice, pBuffer, 512))
							if (WriteMBR(pBuffer, 512))
							{
								szMessage = szRestoreMBROK[g_enumLang];
							}
// 
// 							CloseHandle(hDevice);
// 						}
					}
				}
				
				free(pBuffer);
			}
		}
		else
		{
			szMessage = szMbrFileInvalid[g_enumLang];
		}

		CloseHandle(hFile);
	}

	if (!szMessage.CompareNoCase(szRestoreMBROK[g_enumLang]))
	{
		MessageBox(szMessage, szToolName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(szMessage, szToolName, MB_OK | MB_ICONWARNING);
	}
}

void CMbrDlg::OnBnClickedBtnRestoreMbr()
{
	if (MessageBox(szRestoreMBRNotify[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	
	CFileDialog fileDlg(TRUE);			
	fileDlg.m_ofn.lpstrTitle = L"Select MBR Backup File";
	fileDlg.m_ofn.lpstrFilter = L"Bin Files(*.bin)\0*.bin\0All Files(*.*)\0*.*\0\0";

	if (IDOK == fileDlg.DoModal())
	{
		CString strPath = fileDlg.GetPathName();
		if (PathFileExists(strPath))
		{
			WCHAR szPath[MAX_PATH] = {0};
			wcsncpy_s(szPath, strPath.GetBuffer(), strPath.GetLength());
			strPath.ReleaseBuffer();

			RestoreMbrFormBackupFile(strPath);
		}
	}
}

PVOID CMbrDlg::GetDefaultMBR(DWORD* dwSize)
{
	if (dwSize == NULL)
	{
		return NULL;
	}

	DWORD dwRid = 0;
	switch (g_WinVersion)
	{
	case enumWINDOWS_XP:
	case enumWINDOWS_2K3:
	case enumWINDOWS_2K3_SP1_SP2:
		dwRid = IDR_RES_MBR_XP;
	break;

	case enumWINDOWS_VISTA:
	case enumWINDOWS_VISTA_SP1_SP2:
	case enumWINDOWS_7:
	case enumWINDOWS_8:
		dwRid = IDR_RES_MBR_WIN7;
		break;
	}

	if (dwRid == 0)
	{
		return NULL;
	}

	PVOID pBuffer = NULL;

	WCHAR szType[] = {'R','E','S','\0'};
	HRSRC hRsc = FindResource(NULL, MAKEINTRESOURCE(dwRid), szType);
	if (hRsc)
	{
		DWORD dwResSize = SizeofResource(NULL, hRsc);
		if(dwResSize > 0)
		{
			HGLOBAL hResData = LoadResource(NULL, hRsc);
			if(hResData != NULL)
			{
				LPVOID lpResourceData = LockResource(hResData);
				if(lpResourceData != NULL)
				{
					pBuffer = malloc(dwResSize);
					if (pBuffer)
					{
						*dwSize = dwResSize;
						memset(pBuffer, 0, dwResSize);
						memcpy(pBuffer, lpResourceData, dwResSize);

						m_Functions.DecryptResource(pBuffer, dwResSize);
					}

					FreeResource(hResData);
				}
			}
		}
	}

	return pBuffer;
}

void CMbrDlg::OnBnClickedBtnRestoreDefaultMbr()
{
	if (MessageBox(szRestoreMBRNotify[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	DWORD dwSize = 0;
	PVOID pMbr = GetDefaultMBR(&dwSize);

	if (!pMbr || dwSize <= 0)
	{
		MessageBox(szRestoreMBRERROR[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
		return;
	}

	BOOL bRet = FALSE;
// 	HANDLE hDevice = GetDiskHandle();
// 	if (hDevice)
// 	{
		PVOID pNowMbr = ReadMBR();
		if (pNowMbr)
		{
			if (*((PBYTE)pNowMbr + 0x1BE) != 0x80)
			{
				MessageBox(szDiskPartitionInfoError[g_enumLang], szToolName, MB_OK | MB_ICONERROR);
				free(pNowMbr);
				free(pMbr);
				return;
			}
			else
			{
				//			DWORD dwRet = 0;
				memcpy(pNowMbr, pMbr, dwSize);
				bRet = WriteMBR(pNowMbr, 512);
				//if (m_FileFunc.WriteEx(hDevice, pNowMbr, 512))
				//if (WriteFile(hDevice, pNowMbr, 512, &dwRet, NULL))
				// 			{
				// 				bRet = TRUE;
				// 			}
			}

			free(pNowMbr);
		}
		
// 		CloseHandle(hDevice);
// 	}

	free(pMbr);
	
	if (bRet)
	{
		MessageBox(szRestoreMBROK[g_enumLang], szToolName, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(szRestoreMBRERROR[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
	}
}

HBRUSH CMbrDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_ZHUYI ||
		pWnd->GetDlgCtrlID() == IDC_STATIC_STATUS_ERROR
		)
	{
		pDC->SetTextColor(RGB(255, 0, 0));
	}
	else if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUS_OK)
	{
		pDC->SetTextColor(RGB(0, 0x80, 0));
	}

	return hbr;
}

void CMbrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CMbrDlg::OnDisemblyExportText()
{
	m_Functions.ExportListToTxt((CSortListCtrl*)&m_list, m_szStatus);
}

void CMbrDlg::OnDisemblyExportExcel()
{
	WCHAR szTitle[] = {'M','B','R','\0'};
	m_Functions.ExportListToExcel((CSortListCtrl*)&m_list, szTitle, m_szStatus);
}

void CMbrDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_DISEMBLY_EXPORT_TEXT, szExportToText[g_enumLang]);
	menu.AppendMenu(MF_STRING, ID_DISEMBLY_EXPORT_EXCEL, szExportToExcel[g_enumLang]);
	
	if (m_list.GetItemCount() == 0)
	{
		menu.EnableMenuItem(ID_DISEMBLY_EXPORT_TEXT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		menu.EnableMenuItem(ID_DISEMBLY_EXPORT_EXCEL, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}

	int x = GetSystemMetrics(SM_CXMENUCHECK);
	int y = GetSystemMetrics(SM_CYMENUCHECK);
	if (x >= 15 && y >= 15)
	{
		menu.SetMenuItemBitmaps(ID_DISEMBLY_EXPORT_TEXT, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
		menu.SetMenuItemBitmaps(ID_DISEMBLY_EXPORT_EXCEL, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
	}

	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	menu.DestroyMenu();

	*pResult = 0;
}
