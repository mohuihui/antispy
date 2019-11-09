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
#include "ConnectDriver.h"
#include "Function.h"
#include "SignVerifyDlg.h"
#include "..\\..\\Common\Common.h"
#include <list>
#include "ProcessFunc.h"
#include "ListModules.h"

using namespace std;

typedef LONG NTSTATUS;   
// CProcessHookDlg 对话框

typedef struct _IAT_HOOK_INFO
{
	CString szHookedModule; // 被hook的模块名
	CString szExpModule;	// 导出函数的模块
	CString szFunction;		// 函数名
	DWORD dwHookAddress;	// hook后的函数地址
	DWORD dwOriginAddress;	// 原始函数地址
	ULONG dwIatAddress;
}IAT_HOOK_INFO, *PIAT_HOOK_INFO;

typedef struct _INLINE_HOOK_INFO
{
	DWORD dwBase;		// 模块基地址
	DWORD dwHookOffset;	// hook后的函数偏移
	DWORD dwLen;		// hook长度
	DWORD dwReload;
}INLINE_HOOK_INFO, *PINLINE_HOOK_INFO;

typedef struct _DUMP_INFO_
{
	CString szPath;
	DWORD nBase;
	DWORD nSize;
	PVOID pSectionBuffer;
	PVOID pReloadBuffer;
	BOOL bFixed;
}DUMP_INFO, *PDUMP_INFO;

typedef struct _BOUNT_IMPORT_INFO
{
	CString szModule;
	CString szRefModule;
	DWORD dwRefTimeDateStamp;
	DWORD dwRefBase;
	DWORD dwRefSize;
}BOUND_IMPORT_INFO, *PBOUNT_IMPORT_INFO;

class CProcessHookDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CProcessHookDlg)

public:
	CProcessHookDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessHookDlg();

// 对话框数据
	enum { IDD = IDD_PROCESS_HOOK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void EnumProcessHooks();
	ULONG EnumImportDirectoryHookInfo( ULONG OriginBase, ULONG ReloadBase);
 	BOOL ImageFile(PVOID FileBuffer, PVOID* ImageModuleBase);
	UINT AlignSize(UINT nSize, UINT nAlign);
	BOOL FixImportTable(BYTE *ImageBase, DWORD ExistImageBase);
	BOOL InsertOriginalFirstThunk(DWORD ImageBase, DWORD ExistImageBase, PIMAGE_THUNK_DATA FirstThunk);
	PVOID MiFindExportedRoutine ( IN PVOID DllBase, BOOL ByName, IN char *RoutineName, DWORD Ordinal );
	BOOL FixBaseRelocTable ( PVOID NewImageBase, DWORD ExistImageBase );
	PIMAGE_BASE_RELOCATION LdrProcessRelocationBlockLongLong( IN ULONG_PTR VA, IN ULONG SizeOfBlock, IN PUSHORT NextOffset, IN LONGLONG Diff );
	DWORD FindExportedRoutineInReloadModule ( IN ULONG DllBase, IN char *RoutineName );
	PVOID DumpProcessSection();
	DWORD GetImportFuncOffset(PVOID ImageBase, CHAR *szFunc);
	CString GetDllModulePath(DWORD dwRet);
	PVOID DumpDllModuleSection(DWORD dwBase, DWORD dwSize);
	void EnumInlineHook(PVOID pSection, PVOID pReload, DWORD dwBase);
	VOID GetImageBoundImport(PIMAGE_BOUND_IMPORT_DESCRIPTOR ImageBoundImportDescriptor, list<BOUND_IMPORT_INFO> &m_BoundList);
	void GetDllModuleBaseAndSize(CString szModuleName, DWORD *dwBase, DWORD *dwSize);
	BOOL IsBoundImport(CString szModule, DWORD dwFunction, list<BOUND_IMPORT_INFO> BoundList);
	BOOL IsInCodeSection(DWORD dwAddress);
	ULONG GetInlineHookAddress(PVOID pBuffer, ULONG dwBase, ULONG dwHookOffset, ULONG nCodeLen);
	PVOID MapFileWithoutFix(CString szPath);
	afx_msg LRESULT OnUpdateData(WPARAM wParam, LPARAM lParam);  
	void FixImprtAndReacDir();
	PVOID GetReloadBuffer(DWORD dwBase);
	void GetDllModuleBase(CString szModuleName, DWORD *dwRet);
	void ClearDumpList();
	void ShowScanResult();
//	void EnumIatHook(PVOID pSection, CString szPath);
	void EnumIatHook(PVOID pSection, CString szPath, DWORD nBase);
	CString GetFunctionName(PVOID pSection, DWORD dwOffset);
	CString GetModuleImageName(DWORD dwBase);
// 	DWORD PassIATAddress(PVOID pSection, ULONG pVA);
	DWORD PassIATAddress(PVOID pSection, ULONG pVA, ULONG nSize);
	PVOID GetOriginBuffer(DWORD dwBase);

public:
	CString m_szStatus;
	CSortListCtrl m_list;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	CProcessFunc m_ProcessFunc;
	ULONG m_pEprocess;
	ULONG m_nPid;
	CListModules m_clsListModules;
	vector<MODULE_INFO> m_vectorModules;
	CString m_szPath;
	list<IAT_HOOK_INFO> m_IatHookList;
	list<DUMP_INFO> m_DumpList;
	list<INLINE_HOOK_INFO> m_InlineHookInfo;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSdtRefresh();
	afx_msg void OnSdtDisCurrent();
	afx_msg void OnSdtRestore();
	afx_msg void OnSdtRestoreAll();
	afx_msg void OnSdtShuxing();
	afx_msg void OnSdtLocationModule();
	afx_msg void OnSdtVerifySign();
	afx_msg void OnSdtText();
	afx_msg void OnSdtExcel();
	void Restore(int nItem);
	HANDLE m_hThread;
	BOOL m_bQuit;
	afx_msg void OnClose();
	BOOL IsExecuteSection(DWORD dwFunction);
	BOOL GetPeImportTable(BYTE *ImageBase, PIMAGE_IMPORT_DESCRIPTOR *ImageImportDescriptor);
	BOOL IspInCodeSection(DWORD dwAddress, DWORD pBuffer, DWORD dwBase);
};
