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
#include "..\\..\\Common\Common.h"
#include "ConnectDriver.h"
#include "Function.h"
#include <vector>
#include "ListModules.h"
#include "ListMemory.h"

using namespace std;


// end_winnt

#define PAGE_NOACCESS          0x01     // winnt
#define PAGE_READONLY          0x02     // winnt
#define PAGE_READWRITE         0x04     // winnt
#define PAGE_WRITECOPY         0x08     // winnt
#define PAGE_EXECUTE           0x10     // winnt
#define PAGE_EXECUTE_READ      0x20     // winnt
#define PAGE_EXECUTE_READWRITE 0x40     // winnt
#define PAGE_EXECUTE_WRITECOPY 0x80     // winnt
#define PAGE_GUARD            0x100     // winnt
#define PAGE_NOCACHE          0x200     // winnt
#define PAGE_WRITECOMBINE     0x400     // winnt

// end_ntddk end_wdm

#define MEM_COMMIT           0x1000     // winnt ntddk wdm
#define MEM_RESERVE          0x2000     // winnt ntddk wdm
#define MEM_DECOMMIT         0x4000     // winnt ntddk wdm
#define MEM_RELEASE          0x8000     // winnt ntddk wdm
#define MEM_FREE            0x10000     // winnt ntddk wdm
#define MEM_PRIVATE         0x20000     // winnt ntddk wdm
#define MEM_MAPPED          0x40000     // winnt ntddk wdm
#define MEM_RESET           0x80000     // winnt ntddk wdm

typedef struct _PROTECT_
{
	ULONG uType;
	WCHAR szTypeName[50];
}PROTECT, *PPROTECT;

// CProcessMemoryDlg 对话框

class CProcessMemoryDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CProcessMemoryDlg)

public:
	CProcessMemoryDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessMemoryDlg();

// 对话框数据
	enum { IDD = IDD_MEMORY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	virtual BOOL OnInitDialog();
	BOOL EnumMemorys(ULONG* nCnt = NULL);
	VOID InitMemoryProtect();
	CString GetMemoryProtect(ULONG Protect);
	CString GetMempryState(ULONG State);
	CString GetMempryType(ULONG Type);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMemoryRefresh();
	afx_msg void OnMemoryDump();
	afx_msg void OnMemoryDumpSelect();
	afx_msg void OnMemoryZeroMemory();
	afx_msg void OnMemoryChangeProtect();
	afx_msg void OnMemoryFreeMemory();
	afx_msg void OnMemoryText();
	afx_msg void OnMemoryExcel();
	CString GetModuleImageName(DWORD dwBase);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnModifyProtectionComplete(WPARAM wParam, LPARAM lParam);  
	BOOL Dump(ULONG Base, ULONG Size, CString szFilePath);
public:
	CString m_szImage;
	DWORD m_dwPid;
	DWORD m_pEprocess;
	vector<MEMORY_INFO> m_vectorMemory;
	CConnectDriver m_Driver;
	vector<PROTECT> m_vectorProtectType;
	CommonFunctions m_Functions;
	CListModules m_clsListModules;
	vector<MODULE_INFO> m_vectorModules;
	ULONG m_nMemoryCnt;
	CListMemory m_clsMemory;
	HWND m_hDlgWnd;
	afx_msg void OnMemoryDisassemble();
	afx_msg void OnMemoryHexEdit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetProcessInfo(DWORD dwPid, DWORD pEprocess, CString szImage, HWND hWnd);
};
