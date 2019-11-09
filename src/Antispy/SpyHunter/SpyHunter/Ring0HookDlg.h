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
#include "SsdtHookDlg.h"
#include "ShadowSsdtDlg.h"
#include "FileSystemDlg.h"
#include "KbdclassDlg.h"
#include "MouseDlg.h"
#include "DiskDlg.h"
#include "AtapiDlg.h"
#include "AcpiDlg.h"
#include "TcpipDlg.h"
#include "IdtDlg.h"
#include "ObjectHookDlg.h"
#include "KernelEntryDlg.h"
#include "IatEatDlg.h"
#include "ModifyCodeDlg.h"
#include "MessageHookDlg.h"

// CRing0HookDlg 对话框

class CRing0HookDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CRing0HookDlg)

public:
	CRing0HookDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRing0HookDlg();

// 对话框数据
	enum { IDD = IDD_RING0_HOOK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
public:
	CTabCtrl m_tab;
	ULONG m_nCurSel;
	CSsdtHookDlg m_SsdtDlg;
	CShadowSsdtDlg m_ShadowSsdtDlg;
	CFileSystemDlg m_FileSystemDlg;
	CKbdclassDlg m_KbdclassDlg;
	CMouseDlg m_MouseDlg;
	CDiskDlg m_DiskDlg;
	CAtapiDlg m_AtapiDlg;
	CAcpiDlg m_AcpiDlg;
	CTcpipDlg m_TcpipDlg;
	CIdtDlg m_IdtDlg;
	CObjectHookDlg m_ObjectHookDlg;
	CKernelEntryDlg m_KernelEntryDlg;
	CIatEatDlg m_IatEatDlg;
	CModifyCodeDlg m_ModifyCodeDlg;
	CMessageHookDlg m_MessageHookDlg;
};
