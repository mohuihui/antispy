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
#include "Function.h"
#include "SignVerifyDlg.h"
#include <vector>
#include "ProcessFunc.h"
#include "ListProcess.h"
#include "ListModules.h"

using namespace std;

typedef struct _MODULE_INFO_EX_
{
	ULONG Base;
	ULONG Size;
	CString szPath;
	ITEM_COLOR clrItem;
}MODULE_INFO_EX, *PMODULE_INFO_EX;

// CDllModuleDlg 对话框

class CDllModuleDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CDllModuleDlg)

public:
	CDllModuleDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDllModuleDlg();

// 对话框数据
	enum { IDD = IDD_DLL_MODULE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	VOID SortByBase();
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDllmoduleRefresh();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotShowMicromodule();
	afx_msg void OnUpdateNotShowMicromodule(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	VOID ListAllModules();
	VOID ListNotMicrosoftModules();
	afx_msg void OnCopyDllMemory();
	afx_msg void OnUnloadModule();
	afx_msg void OnCheckSign();
	afx_msg void OnCheckAllSign();
	afx_msg void OnCopyModuleName();
	afx_msg void OnCopyModulePath();
	afx_msg void OnCheckModuleAttribute();
	afx_msg void OnLocationModule();
	afx_msg void OnSearchModuleOnline();
	BOOL IsNotSignItem(ULONG nItem);
	afx_msg LRESULT OnVerifyModulesEnd(WPARAM wParam, LPARAM lParam);  
	afx_msg void OnExceptExcel();
	afx_msg void OnExportText();
	afx_msg void OnDeleteModule();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void EnumModules(ULONG *nTotal = NULL, ULONG *nNotMicro = NULL);
	CString GetFileCreationTime(CString szPath);
	void AddDllModuleItem(MODULE_INFO item);
	afx_msg void OnUnloadAndDeleteModule();
	afx_msg void OnModulesLocationAtFileManager();
	void SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd);
public:
	CSortListCtrl m_list;
	DWORD m_uPid;									// 被枚举的进程ID
	DWORD m_pEprocess;								// 被枚举的进程EPROCESS
	vector<MODULE_INFO> m_vectorPebModules;			// 内核枚举PEB得到的进程列表
	CString m_szImage;
	BOOL m_bNotShowMicroModule;						// 是否显示微软的模块
	CommonFunctions m_Functions;
	CSignVerifyDlg m_SignVerifyDlg;					// 验签名的类
	ULONG m_nModuleCnt;								// 模块总的数量
	ULONG m_nNotMicro;								// 非微软的模块数量
	CImageList m_ModuleImageList;					// 模块图标
	HICON m_hModuleIcon;							// 默认的icon图标
	CProcessFunc m_ProcessFunc;
	CListModules m_clsModules;						// 枚举模块用到的类
	vector<MODULE_INFO_EX> m_vectorModules;			// 修复后得到的模块列表
	HWND m_hDlgWnd;
};
