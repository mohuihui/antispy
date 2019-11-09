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
#include <vector>
#include <list>
#include "ListDriver.h"
#include "ListHooks.h"

using namespace std;

// CModifyCodeDlg 对话框

class CModifyCodeDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CModifyCodeDlg)

public:
	CModifyCodeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModifyCodeDlg();

// 对话框数据
	enum { IDD = IDD_MODIFY_CODE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void EnumModifiedCode();
	CString GetDriverPath(ULONG pCallback);
	void GetAllOfModuleInlineHookInfo(ULONG Base, ULONG Size, WCHAR *szPath);
	void GetExportFunctonsPatchs(ULONG Base, ULONG Size, WCHAR *szPath);
 	void InsertExportDirectoryItem( EXPORT_FUNCTION_PATCH_INFO ExportDirectoryHookInfo );
	BOOL IsAlreadyInList(ULONG PatchAddress);
	void GetSsdtInlineHookInfo();
	void InsertShadowSsdtItem( SHADOW_INLINE_INFO ExportDirectoryHookInfo );
	void GetShadowSsdtInlineHookInfo();
	void InsertAllOfModuleItem( MODULE_PATCH_INFO AllOfModuleHookInfo, CString szImage, ULONG Base );
	void ScanModifiedCode();

public:
	CString m_szStatus;
	CSortListCtrl m_list;
	list<ULONG> m_PatchAddressList;
	list<MODIFIED_CODE_INFO> m_AllPatchList;
	CConnectDriver m_Driver;
	CommonFunctions m_Functions;
	vector<DRIVER_INFO> m_vectorDrivers;
	ULONG m_nHookedCnt;
	CListDrivers m_clsDrivers;
	CListHooks m_clsHooks;
	vector<EXPORT_FUNCTION_PATCH_INFO> m_vectorExportFuncHooks;
	vector<EXPORT_FUNCTION_PATCH_INFO> m_vectorSSDTHooks;
	vector<SHADOW_INLINE_INFO> m_vectorShadowHooks;
	vector<MODULE_PATCH_INFO> m_vectorModuleHooks;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSdtRefresh();
	afx_msg void OnSdtDisCurrent();
	afx_msg void OnSdtDisOrigin();
	afx_msg void OnSdtRestore();
	afx_msg void OnSdtShuxing();
	afx_msg void OnSdtLocationModule();
	afx_msg void OnSdtVerifySign();
	afx_msg void OnSdtText();
	afx_msg void OnSdtExcel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnProcessLocationAtFileManager();
};
