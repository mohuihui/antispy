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
#include <list>
#include "afxwin.h"
#include ".\hive\OpHive.h"
#include "RegFindDlg.h"
#include "ConnectDriver.h"
#include "xySplitterWnd.h"

using namespace std;

typedef struct _KEY_VALUE_INFO{
	ULONG   TitleIndex;
	ULONG   Type;
	ULONG   DataOffset;
	ULONG   DataLength;
	ULONG   NameLength;
	WCHAR   Name[1024];
} KEY_VALUE_INFO, *PKEY_VALUE_INFO;


// CRegistryDlg 对话框

class CRegistryDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CRegistryDlg)

public:
	CRegistryDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRegistryDlg();

// 对话框数据
	enum { IDD = IDD_REGISTRY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	BOOL InitMytree();
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult);
	CString GetKeyType(ULONG Type);
	CString GetKeyData(ULONG Type, WCHAR *Data, ULONG DataLength);
	void EnumSubKeys(CString szKey, HTREEITEM hItem, BOOL bSubSubKey = FALSE);
	void DeleteSubTree(HTREEITEM TreeItem);
	void EnumCurrentUserSubKeys(HTREEITEM hChild);
	BOOL GetCurrentUserKeyPath(OUT WCHAR *szPath);
	void EnumSubSubKeys(CString szKey, HTREEITEM hItem);
	void EnumValues(CString szKey);
	void InitRegistry();
	afx_msg void OnNMRclickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRegRefresh();
	afx_msg void OnRegLookFor();
	afx_msg void OnRegExport();
	afx_msg void OnRegDelete();
	afx_msg void OnRegRename();
	afx_msg void OnRegNewKey();
	afx_msg void OnRegCopyKeyName();
	afx_msg void OnRegCopyFullKeyName();
	void ShellExportRegister(  CString strItem, CString strFileName );
	void UpdateKey(CString szKey, HTREEITEM hItem);
	afx_msg void OnStringValue();
	afx_msg void OnBinaryValue();
	afx_msg void OnDwordValue();
	afx_msg void OnMuiStringValue();
	afx_msg void OnExpandStringValue();
	void SetValueKey(ULONG Type, PVOID pData, ULONG DataSize);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRegListRefresh();
	afx_msg void OnRegListExport();
	afx_msg void OnRegListModify();
	afx_msg void OnRegListDelete();
	afx_msg void OnRegListRename();
	afx_msg void OnRegListCopyValue();
	afx_msg void OnRegListCopyValueData();
	CString StatusString2KeyPath();
	void ModifyValue(CString szValue, ULONG Type, PVOID pData, ULONG DataSize);
	void CreateValueKey(CString szValue, ULONG Type, PVOID pData, ULONG DataSize);
	VOID InitializeComboBox();
	afx_msg void OnBnClickedGoto();
	CString ComboString2KeyPath();
	HTREEITEM GetTreeSubItemByName(HTREEITEM PatentItem, CString szName);
	void JmpToReg(CString szKey, CString szData);
	CString Key2KeyPath(CString szKey);
	PVOID GetValueInfo(CString szKey, CString szValue);
public:
	CSortListCtrl m_list;
	CTreeCtrl m_tree;
	CImageList m_TreeImageList;
	CRegistry m_Registry;
	CommonFunctions m_Function;
	WCHAR* m_szHKeyCurrentUser;
	CImageList m_ImageList;
	CString m_szComboText;
	HTREEITEM m_RightClickItem;
	CComboBox m_ComboBox;
	CString m_szStatus;
	CButton m_BtnGoto;
	HTREEITEM m_hChild;
	CString m_szJmpKey;
	BOOL m_bUseHive;
	COperateHive m_OpHive;
	CConnectDriver m_Driver;
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	ULONG GetValueType(CString szType);
	void ExportValue(CString szKey, CString szValue, CString szPath);
	afx_msg void OnRegUseHive();
	afx_msg void OnUpdateRegUseHive(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	BOOL WCharToMChar(IN LPCWSTR lpcwszStr, IN OUT LPSTR* ppszStr);
	CString m_szAddress;
	afx_msg void OnRegAddToQuickAddress();
	void JmpToReg_Registry_Xxx(CString szKey, CString szData);
	void JmpToReg_HKEY_Xxx(CString szKey, CString szData);
	int m_nComboBoxCnt;
	CRegFindDlg *m_pFindRegDlg; 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnSetfocusCombo();
	afx_msg void OnCbnKillfocusCombo();
	afx_msg void OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMKillfocusList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnDropdownCombo();
	CXTPButton m_btnBack;
	CXTPButton m_btnForward;
	CBitmap m_bitmap;
	CImageList m_imageList;
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedForward();
	void InitButtom();
	void UpdateHistroy(CString szHistroy);
	vector<CString> m_historyVector;
	ULONG m_nCurrentHistroy;
	BOOL m_bBackOrForward;
	afx_msg void OnRegInport();
	void ShellImportRegister( LPCTSTR lpImportFile);
	afx_msg void OnRegQueryLockInfo();
	void LookForRegLockInfo(CString szFilePath);
	CxSplitterWnd   m_xSplitter;
};
