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
#include <list>
#include <vector>
using namespace std;
#include "RegCommon.h"
#include "Function.h"
#include "Registry.h"

typedef struct _FILE_ASSOCIATION_
{
	CString szKey;
	CString szValue;
	CString szCmdOpenValue;
}FILE_ASSOCIATION, *PFILE_ASSOCIATION;

typedef enum _FILE_ASSOC_TYPE
{
	eClassRoot,
	eClassRootOpenCommand,
	eCurrentUserProgid,
}FILE_ASSOC_TYPE;

typedef struct _FILE_ASSOCIATION_ITEM
{
	CString szName;
	CString szRegPath;
	CString szValue;
	CString szBiaozhun;
	DWORD dwValueType;
	FILE_ASSOC_TYPE dwFileAssocType;
}FILE_ASSOCIATION_ITEM, *PFILE_ASSOCIATION_ITEM;

// CFileAssociationDlg 对话框

class CFileAssociationDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CFileAssociationDlg)

public:
	CFileAssociationDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileAssociationDlg();

// 对话框数据
	enum { IDD = IDD_FILE_ASSOCIATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CSortListCtrl m_list;
	CRegistry m_Registry;
	list<FILE_ASSOCIATION>m_FileAssocList;
	CString m_szCurrentUserKey;
	CRegCommon m_RegCommon;
	CommonFunctions m_Functions;
	ULONG m_nCnt;
	vector<FILE_ASSOCIATION_ITEM> m_vectorInfos;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void ListClassRoot();
	void ListFileAssociation();
	void InitClassRoot();
	CString GetRegPath(CString szKey);
	void ListClassRootOpenCommand();
	void ListCurrentUserProgid();
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileAssocRefresh();
	afx_msg void OnFileAssocFix();
	afx_msg void OnFileAssocFixAll();
	afx_msg void OnFileAssocLocationReg();
	afx_msg void OnFileAssocExportText();
	afx_msg void OnFileAssocExportExcel();
	BOOL Fix(int nItem);
};
