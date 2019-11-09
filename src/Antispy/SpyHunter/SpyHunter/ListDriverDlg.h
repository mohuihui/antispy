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
#include <vector>
#include "..\\..\\Common\Common.h"
#include "ConnectDriver.h"
#include "Function.h"
#include "SignVerifyDlg.h"
#include "Registry.h"
#include "ListDriver.h"

using namespace std;

typedef enum _SYSTEM_INFORMATION_CLASS   
{   
    SystemModuleInformation = 11
} SYSTEM_INFORMATION_CLASS;   

typedef struct tagSYSTEM_MODULE_INFORMATION {
    ULONG Reserved[2];
    PVOID Base;
    ULONG Size;
    ULONG Flags;
    USHORT Index;
    USHORT Unknown;
    USHORT LoadCount;
    USHORT ModuleNameOffset;
    CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef 
NTSTATUS (WINAPI *pfnZwQuerySystemInformation)
				(IN SYSTEM_INFORMATION_CLASS, 
				IN OUT PVOID, 
				IN ULONG, 
				OUT PULONG OPTIONAL);   


typedef enum _DRIVER_COLOR_
{
	enumHideDriver, // 红
	enumNotMircsoftDriver, // 蓝
	enumNormalDriver, // 黑
	enumNoSignDriver, // 紫色
}DRIVER_COLOR;

// CListDriverDlg 对话框

typedef struct _SERVICE_KEY_
{
	CString szKey;
	CString szPath;
}SERVICE_KEY, *PSERVICE_KEY;

class CListDriverDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CListDriverDlg)

public:
	CListDriverDlg(CWnd* pParent = NULL);   
	virtual ~CListDriverDlg();

// 对话框数据
	enum { IDD = IDD_DRIVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void ListDrivers();
	void InsertDriverIntem();
	void Ring3GetDriverModules();
	BOOL IsHideDriver(ULONG Base);
	CSortListCtrl m_list;
	vector<DRIVER_INFO> m_vectorDrivers;
	vector<ULONG> m_vectorRing3Drivers;
	vector<ITEM_COLOR> m_vectorColor;
	CConnectDriver m_Driver;
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	VOID GetServiceKeys();
	CString GetServiceKeyImagePathValue(CString szKey);
	CString GetServiceName(CString szPath);
	CString m_Status;
	CommonFunctions m_Funcionts;
	list<SERVICE_KEY> m_vectorServiceKeys;
	CSignVerifyDlg m_SignVerifyDlg;
	CRegistry m_Registry;
	CListDrivers m_clsDriver;
	afx_msg void OnDriverRefresh();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUnloadDriver();
	afx_msg void OnDeleteDriverFile();
	afx_msg void OnDeleteDriverFileAndReg();
	afx_msg void OnCopyDriverMemory();
	afx_msg void OnDriverSignCheck();
	afx_msg void OnDriverAllSignCheck();
	afx_msg void OnCopyDriverName();
	afx_msg void OnCopyDriverPath();
	afx_msg void OnLocateReg();
	afx_msg void OnFindDriver();
	afx_msg void OnDriverShuxing();
	afx_msg void OnDriverSearch();
	afx_msg void OnDriverText();
	afx_msg void OnDriverExcel();
	BOOL IsNotSignItem(ULONG nItem);
	afx_msg LRESULT SignVerifyOver(WPARAM wParam, LPARAM lParam);  
	BOOL IsDriverCanUnload();
	BOOL GetStartType(CString szKey, DWORD *dwType);
	afx_msg void OnServiceBoot();
	afx_msg void OnServiceSystem();
	afx_msg void OnServiceAuto();
	afx_msg void OnServiceMad();
	afx_msg void OnServiceDisabled();
	BOOL ModifyStartupType(CString szServiceName, ULONG Type);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
	CString GetDriverStartupType(CString szService);
};
