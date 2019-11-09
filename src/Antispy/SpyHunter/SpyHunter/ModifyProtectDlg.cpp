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
// ModifyProtectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "ModifyProtectDlg.h"
#include "..\\..\\Common\Common.h"

// CModifyProtectDlg 对话框

IMPLEMENT_DYNAMIC(CModifyProtectDlg, CDialog)

CModifyProtectDlg::CModifyProtectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyProtectDlg::IDD, pParent)
{
	m_pid = 0;
	m_pEprocess = 0;
	m_base = 0;
	m_size = 0;
	m_hMemoryWnd = 0;
}

CModifyProtectDlg::~CModifyProtectDlg()
{
}

void CModifyProtectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO, m_cobmo);
}


BEGIN_MESSAGE_MAP(CModifyProtectDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CModifyProtectDlg::OnBnClickedOk)
	ON_BN_CLICKED(ID_MODIFY_OK, &CModifyProtectDlg::OnBnClickedModifyOk)
	ON_BN_CLICKED(ID_MODIFY_CANCEL, &CModifyProtectDlg::OnBnClickedModifyCancel)
END_MESSAGE_MAP()


// CModifyProtectDlg 消息处理程序

BOOL CModifyProtectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetWindowText(szModifyMemoryProtection[g_enumLang]);

	m_cobmo.AddString(L"PAGE_NOACCESS");
	m_cobmo.AddString(L"PAGE_READONLY");
	m_cobmo.AddString(L"PAGE_READWRITE");
	m_cobmo.AddString(L"PAGE_WRITECOPY");
	m_cobmo.AddString(L"PAGE_EXECUTE");
	m_cobmo.AddString(L"PAGE_EXECUTE_READ");
	m_cobmo.AddString(L"PAGE_EXECUTE_READWRITE");
	m_cobmo.AddString(L"PAGE_EXECUTE_WRITECOPY");

	m_cobmo.SetItemData(0, PAGE_NOACCESS);
	m_cobmo.SetItemData(1, PAGE_READONLY);
	m_cobmo.SetItemData(2, PAGE_READWRITE);
	m_cobmo.SetItemData(3, PAGE_WRITECOPY);
	m_cobmo.SetItemData(4, PAGE_EXECUTE);
	m_cobmo.SetItemData(5, PAGE_EXECUTE_READ);
	m_cobmo.SetItemData(6, PAGE_EXECUTE_READWRITE);
	m_cobmo.SetItemData(7, PAGE_EXECUTE_WRITECOPY);

	m_cobmo.SetCurSel(6);
	
	GetDlgItem(ID_MODIFY_OK)->SetWindowText(szOK[g_enumLang]);
	GetDlgItem(ID_MODIFY_CANCEL)->SetWindowText(szCancel[g_enumLang]);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CModifyProtectDlg::OnBnClickedOk()
{
}

void CModifyProtectDlg::OnBnClickedModifyOk()
{
	BOOL bRet = FALSE;
	UpdateData(TRUE);

	int item = m_cobmo.GetCurSel();
	ULONG itemdata = (ULONG)m_cobmo.GetItemData(item);
	
	COMMUNICATE_MEMORY cm;
	cm.OpType = enumMemoryModifyProtection;
	cm.op.ModifyProtection.nPid = m_pid;
	cm.op.ModifyProtection.pEprocess = m_pEprocess;
	cm.op.ModifyProtection.Base = m_base;
	cm.op.ModifyProtection.Size = m_size;
	cm.op.ModifyProtection.ModifyType = itemdata;

	DebugLog(L"ModifyProtection -> base: 0x%08X, size: 0x%X, ModifyType: 0x%X", m_base, m_size, itemdata);

	bRet = m_Driver.CommunicateDriver(&cm, sizeof(COMMUNICATE_MEMORY), NULL, 0, NULL);
	if (bRet)
	{
		::SendMessage(m_hMemoryWnd, WM_MODIFY_PROTECTION, itemdata, NULL);
		OnCancel();
	}
	else
	{
		MessageBox(szModifyProtectionFailed[g_enumLang], szToolName, MB_OK | MB_ICONWARNING);
	}
}

void CModifyProtectDlg::OnBnClickedModifyCancel()
{
	OnCancel();
}
