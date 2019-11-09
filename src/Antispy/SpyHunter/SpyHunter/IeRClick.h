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
#include <set>
#include <list>
using namespace std;
#include "Function.h"
#include "Registry.h"
#include "RegCommon.h"

typedef enum IE_RCLICK_STATUS{
	eRclickOk,
	eRclickDisable,
};

typedef struct _IE_RCLICK_INFO
{
	IE_RCLICK_STATUS status;
	CString szKey;
}IE_RCLICK_INFO, *PIE_RCLICK_INFO;

// CIeRClick 对话框

class CIeRClick : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CIeRClick)

public:
	CIeRClick(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CIeRClick();

// 对话框数据
	enum { IDD = IDD_IE_RCLICK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	CString m_szStatus;
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void EnumMenuExtKeys();
	void InsertValues();
	vector<IE_RCLICK_INFO> m_infoList;
	CommonFunctions m_Function;
	CRegistry m_Registry;
	CString m_szCurrentKey;
	CRegCommon m_RegCommonFunc;
	afx_msg void OnClickRefresh();
	afx_msg void OnClickDeleteReg();
	afx_msg void OnClickDeleteRegAndFile();
	afx_msg void OnClickJmpToExplorer();
	afx_msg void OnClickJmpToAntispy();
	afx_msg void OnRclickText();
	afx_msg void OnRclickExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickJmpToRegistry();
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL EnableOrDisableKey(CString szKeyRoot, CString szDisableKey, BOOL bEnable);
	BOOL DisableOrEnableAutoruns(PIE_RCLICK_INFO pInfo, BOOL bEnable);
	BOOL m_bStartCheck;
	afx_msg void OnPlugineDisable();
	afx_msg void OnPlugineEnable();
};
