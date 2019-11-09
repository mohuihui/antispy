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
#pragma once
#include "afxcmn.h"
#include "Function.h"
#include "..\\..\\Common\Common.h"
#include "FileDlg.h"
#include "ListProcess.h"
#include <algorithm>
#include <list>
#include <vector>
#include "ConnectDriver.h"
using namespace std;

typedef struct _LOCK_FILL_PROCESS_INFO
{
	ULONG nHandle;
	ULONG nPid;
	ULONG pEprocess;
	CString szFile;
	CString szProcess;
}LOCK_FILL_PROCESS_INFO, *PLOCK_FILL_PROCESS_INFO;

// CFileUnlockDlg dialog

class CFileUnlockDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CFileUnlockDlg)

public:
	CFileUnlockDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileUnlockDlg();

// Dialog Data
	enum { IDD = IDD_FILE_UNLOCK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CSortListCtrl m_list;
	virtual BOOL OnInitDialog();
	VOID SortByPid();
	BOOL GetProcessList();
	void ShowLockedFileInformation(PLOCKED_FILE_INFO pLockFileInfos);
	PLOCKED_FILE_INFO m_pLockFileInfos;
	vector<LOCKED_FILES> m_LockedFilesVector;
	CommonFunctions m_Functions;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void ConvertVolumePaths( IN PWCHAR DeviceName, IN PWCHAR VolumeName );
	BOOL InitDevice2Path();
	CString FixFilePath(WCHAR* szPath);
	CImageList m_ImageList;
	vector <NT_AND_DOS_VOLUME_NAME> m_nt_and_dos_valume_list;
	CListProcess m_clsListProcess;
	vector<PROCESS_INFO> m_vectorProcess;
	vector<LOCK_FILL_PROCESS_INFO> m_lockFileProcessList;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUnlockRefresh();
	afx_msg void OnUnlockUnlock();
	afx_msg void OnUnlockUnlockAll();
	afx_msg void OnUnlockKillProcess();
	afx_msg void OnUnlockJmpToProcess();
	afx_msg void OnUnlockJmpToFile();
	afx_msg void OnUnlockExportText();
	afx_msg void OnUnlockExportExcel();
	WCHAR *m_szNtFilePath;
	BOOL m_bDirectory;
	CConnectDriver m_Driver;
};
