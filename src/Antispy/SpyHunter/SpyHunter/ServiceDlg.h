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
#include "Registry.h"
#include "Function.h"
#include <winsvc.h>
#include "SignVerifyDlg.h"

typedef struct _SERVICE_INFO
{
	CString szServiceName;
	CString szDispatchName;
	CString szPath;
	CString szStartType;
}SERVICE_INFO, *PSERVICE_INFO;

// CServiceDlg 对话框

class CServiceDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CServiceDlg)

public:
	CServiceDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CServiceDlg();

// 对话框数据
	enum { IDD = IDD_SERVICE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	LRESULT OnCloseWindow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void EnumServers();
	CString GetServiceCurrentStatus(ULONG nStatus);
	CString GetServiceDll(CString szKey);
	CString GetServiceModule(CString szKey);
	CString GetImagePath(CString szKey);
	BOOL GetStartType(CString szKey, DWORD *dwType);
	CString GetStartTypeString(CString szKey);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnServiceRefresh();
	afx_msg void OnServiceStart();
	afx_msg void OnServiceStop();
	afx_msg void OnServiceRestart();
	afx_msg void OnServiceAuto();
	afx_msg void OnServiceMad();
	afx_msg void OnServiceDisabled();
	afx_msg void OnServiceDelete();
	afx_msg void OnServiceShuxing();
	afx_msg void OnServiceLocateModule();
	afx_msg void OnServiceVerifySign();
	afx_msg void OnServiceText();
	afx_msg void OnServiceExcel();
	afx_msg void OnServiceDeleteRegAndFile();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	BOOL IsServiceStarted(CString szDisplayName, int *pStarted);
	BOOL ServiceControl(CString szDisplayName, int ControlType);
	BOOL ModifyStartupType(CString szServiceName, ULONG Type);
	BOOL DeleteService(CString szServiceName);
	CString GetDispalyName(CString szKey);
public:
	CSortListCtrl m_list;
	CRegistry m_Registry;
	CommonFunctions m_Function;
	CImageList m_ProImageList;
	HICON m_hProcessIcon;
	CString m_szStatus;
	DWORD m_nServicesReturned;
	LPENUM_SERVICE_STATUS m_pServices;
	vector<SERVICE_INFO> m_ServiceList;
	afx_msg void OnServiceJmpToReg();
	vector<CString> m_ServiceNameList;
	afx_msg void OnServiceNotShowMicrosoftModule();
	afx_msg void OnUpdateServiceNotShowMicrosoftModule(CCmdUI *pCmdUI);
	BOOL m_bShowAll;
	afx_msg void OnServiceVerifyAllSign();
	VOID SortItem();
	BOOL IsNotSignItem(ULONG nItem);
	BOOL IsNotSignItemData(ULONG nItem);
	CSignVerifyDlg m_SignVerifyDlg;
	afx_msg void OnServiceBoot();
	afx_msg void OnServiceSystem();
	void JmpToService(CString szService);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
