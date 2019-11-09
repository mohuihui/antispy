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
#include "Registry.h"
#include "RegCommon.h"
#include <vector>
using namespace std;

typedef enum _IE_PLUGIN_TYPE
{
	enumBHO,
	enumBrowserExtensions,
	enumURLSearchHooks,
	enumDistributionUnits,
	enumToolbar,
	enumActiveX,
}IE_PLUGIN_TYPE;

typedef enum PLUGIN_STATUS
{
	enumPluginOK,
	enumPluginDisable,
};

typedef struct _IE_PLUGIN_INFO 
{
	IE_PLUGIN_TYPE Type;
	CString szName;
	CString szPath;
	CString szKeyPath;   // ¼ü
	CString szValueName;  // Öµ
	PLUGIN_STATUS nStatus;
}IE_PLUGIN_INFO, *PIE_PLUGIN_INFO;

// CIEPluginDlg dialog

class CIEPluginDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CIEPluginDlg)

public:
	CIEPluginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIEPluginDlg();

// Dialog Data
	enum { IDD = IDD_IE_PLUGIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void EnumPlugins();
	void EnumBHO();
	void InsertPlugins();
	CString GetPluginTypeName(IE_PLUGIN_TYPE type);
	void EnumBrowserExtensions();
	CString GetBrowserExtensionsButtonText(CString szKey);
	CString GetBrowserExtensionsModulePath(CString szKey);
	void EnumURLSearchHooks();
	void EnumDistributionUnits();
	void EnumToolbar();
	void EnumActiveX();
	afx_msg void OnPlugineRefresh();
	afx_msg void OnPlugineDeleteReg();
	afx_msg void OnPlugineDeleteRegAndFile();
	afx_msg void OnPlugineJmpToReg();
	afx_msg void OnPlugineCopyName();
	afx_msg void OnPlugineCopyPath();
	afx_msg void OnPlugineCheckSign();
	afx_msg void OnPlugineShuxing();
	afx_msg void OnPlugineSearchOnline();
	afx_msg void OnPlugineJmpToExplorer();
	afx_msg void OnPlugineJmpToAntispy();
	afx_msg void OnPlugineText();
	afx_msg void OnPlugineExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL DisableAutoruns(IE_PLUGIN_INFO info);
	void AddPluginInfo(IE_PLUGIN_TYPE Type, CString szName, CString szPath, CString szKeyPath, CString szValueName, PLUGIN_STATUS nStatus);
	BOOL DisableOrEnableAutoruns(PIE_PLUGIN_INFO info, BOOL bEnable);
	BOOL EnableOrDisableKey(CString szKeyRoot, BOOL bEnable);
	void EnumBHOByRegPath(CString szKey, PLUGIN_STATUS nPluginStatus);
	void EnumBrowserExtensionsRegPath(CString szKey, PLUGIN_STATUS nPluginStatus);
	void EnumURLSearchHooksRegPath(CString szKey, PLUGIN_STATUS nPluginStatus);
	void EnumDistributionUnitsRegPath(CString szKey, PLUGIN_STATUS nPluginStatus);
	void EnumToolbarRegPath(CString szKey, PLUGIN_STATUS nPluginStatus);
	void EnumActiveXRegPath(CString szKey, PLUGIN_STATUS nPluginStatus);
	afx_msg void OnPlugineDisable();
	afx_msg void OnPlugineEnable();
public:
	CSortListCtrl m_list;
	CString m_szStatus;
	CommonFunctions m_Function;
	CRegistry m_Registry;
	ULONG m_nBHO;
	ULONG m_nExtensions;
	ULONG m_nURLSearchHooks;
	ULONG m_nDistributionUnits;
	ULONG m_nToolbar;
	ULONG m_nActiveX;
	vector<IE_PLUGIN_INFO> m_IEPluginVector;
	BOOL m_bStartCheck;
	CRegCommon m_RegCommonFunc;
	CString m_szCurrentUserKey;
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
};
