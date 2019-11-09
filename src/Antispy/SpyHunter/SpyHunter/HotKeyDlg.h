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
#include <vector>
#include "Function.h"
#include "ListProcess.h"
#include "ListHotkey.h"

using namespace std;

// CHotKeyDlg 对话框

extern CString szVkArray[];

class CHotKeyDlg : public CDialog
{
	DECLARE_EASYSIZE

	DECLARE_DYNAMIC(CHotKeyDlg)

public:
	CHotKeyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHotKeyDlg();

// 对话框数据
	enum { IDD = IDD_HOTKEY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	VOID EnumHotKeys(ULONG *nCnt = NULL);
	CString PinJieHotKey(ULONG fsModifiers, ULONG vk);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void InsertHotKeyItems();
	void InsertOneItem(HOTKEY_ITEM item, DWORD dwPid, CString szImageName);
	afx_msg void OnHotkeyRefresh();
	afx_msg void OnHotkeyShowAllProcess();
	afx_msg void OnUpdateHotkeyShowAllProcess(CCmdUI *pCmdUI);
	afx_msg void OnHotkeyRemove();
	afx_msg void OnHotkeyText();
	afx_msg void OnHotkeyExcel();
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

public:
	CSortListCtrl m_list;
	CConnectDriver m_driver;
	DWORD m_dwEprocess;
	DWORD m_dwPid;
	CString m_szImage;
	vector<HOTKEY_ITEM> m_vectorHotKeysTemp;
	vector<HOTKEY_ITEM> m_vectorHotKeys;
	BOOL m_bShowAll;
	CommonFunctions m_Functions;
	ULONG m_nCnt;
	CListHotKeys m_clsHotKeys;
	HWND m_hDlgWnd;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetProcessInfo(ULONG nPid, ULONG pEprocess, CString szImage, HWND hWnd);
};
