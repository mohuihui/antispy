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
// KernelCallbackTableDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "KernelCallbackTableDlg.h"


// CKernelCallbackTableDlg 对话框

IMPLEMENT_DYNAMIC(CKernelCallbackTableDlg, CDialog)

CKernelCallbackTableDlg::CKernelCallbackTableDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKernelCallbackTableDlg::IDD, pParent)
	, m_szStatus(_T(""))
{

}

CKernelCallbackTableDlg::~CKernelCallbackTableDlg()
{
}

void CKernelCallbackTableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KERNRL_CALLBACK_TABLE__STATUS, m_szStatus);
	DDX_Control(pDX, IDC_LIST, m_list);
}


BEGIN_MESSAGE_MAP(CKernelCallbackTableDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CKernelCallbackTableDlg::OnBnClickedOk)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CKernelCallbackTableDlg)
	EASYSIZE(IDC_LIST, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_KERNRL_CALLBACK_TABLE__STATUS, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_STATIC, ES_BORDER, ES_KEEPSIZE, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CKernelCallbackTableDlg 消息处理程序

void CKernelCallbackTableDlg::OnBnClickedOk()
{
}

void CKernelCallbackTableDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

BOOL CKernelCallbackTableDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CKernelCallbackTableDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}
