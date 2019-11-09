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
// NetworkDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "NetworkDlg.h"

CDialog *g_pNetworkDialog[10];

// CNetworkDlg 对话框

IMPLEMENT_DYNAMIC(CNetworkDlg, CDialog)

CNetworkDlg::CNetworkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetworkDlg::IDD, pParent)
{
	m_nCurSel = 0;
}

CNetworkDlg::~CNetworkDlg()
{
}

void CNetworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CNetworkDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CNetworkDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CNetworkDlg)
	EASYSIZE(IDC_TAB, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CNetworkDlg 消息处理程序

void CNetworkDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;

	if (m_tab.m_hWnd)
	{
		CRect rc;
		m_tab.GetClientRect(rc);
		rc.top += 22;
		rc.bottom -= 3;
		rc.left += 2;
		rc.right -= 3;

		m_PortDlg.MoveWindow(&rc);
		m_HostDlg.MoveWindow(&rc);
		m_SpiDlg.MoveWindow(&rc);
		m_IeRClickDlg.MoveWindow(&rc);
		m_IEPluginDlg.MoveWindow(&rc);
	}
}

BOOL CNetworkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_tab.InsertItem(0 , szPort[g_enumLang]);
	m_tab.InsertItem(1, szHostsFile[g_enumLang]);
	m_tab.InsertItem(2, szSPI[g_enumLang]);
	m_tab.InsertItem(3, szIeShell[g_enumLang]);
	m_tab.InsertItem(4, szIePlugin[g_enumLang]);
	
	m_PortDlg.Create(IDD_PORT_DIALOG, &m_tab);
	m_HostDlg.Create(IDD_HOST_DIALOG, &m_tab);
	m_SpiDlg.Create(IDD_SPI_DIALOG, &m_tab);
	m_IeRClickDlg.Create(IDD_IE_RCLICK_DIALOG, &m_tab);
	m_IEPluginDlg.Create(IDD_IE_PLUGIN_DIALOG, &m_tab);

	// 设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 22;
	rc.bottom -= 3;
	rc.left += 2;
	rc.right -= 3;

	m_PortDlg.MoveWindow(&rc);
	m_HostDlg.MoveWindow(&rc);
	m_SpiDlg.MoveWindow(&rc);
	m_IeRClickDlg.MoveWindow(&rc);
	m_IEPluginDlg.MoveWindow(&rc);

	// 把对话框对象指针保存起来
	g_pNetworkDialog[0] = &m_PortDlg;
	g_pNetworkDialog[1] = &m_HostDlg;
	g_pNetworkDialog[2] = &m_SpiDlg;
	g_pNetworkDialog[3] = &m_IeRClickDlg;	
	g_pNetworkDialog[4] = &m_IEPluginDlg;	

	// 显示初始页面
	g_pNetworkDialog[0]->ShowWindow(SW_SHOW);
	g_pNetworkDialog[1]->ShowWindow(SW_HIDE);
	g_pNetworkDialog[2]->ShowWindow(SW_HIDE);
	g_pNetworkDialog[3]->ShowWindow(SW_HIDE);
	g_pNetworkDialog[4]->ShowWindow(SW_HIDE);

	// 当前选择
	m_nCurSel = 0;
	INIT_EASYSIZE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CNetworkDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	g_pNetworkDialog[m_nCurSel]->ShowWindow(SW_HIDE);
	m_nCurSel = m_tab.GetCurSel();
	g_pNetworkDialog[m_nCurSel]->ShowWindow(SW_SHOW);

	if (m_nCurSel == 0)
	{
		m_PortDlg.EnumPort();
	}
	else if (m_nCurSel == 1)
	{
		m_HostDlg.ReadHostsFile();
	}
	else if (m_nCurSel == 2)
	{
		m_SpiDlg.EnumWinsockProviders();
	}
	else if (m_nCurSel == 3)
	{
		m_IeRClickDlg.EnumMenuExtKeys();
	}
	else if (m_nCurSel == 4)
	{
		m_IEPluginDlg.EnumPlugins();
	}

	*pResult = 0;
}

BOOL CNetworkDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}