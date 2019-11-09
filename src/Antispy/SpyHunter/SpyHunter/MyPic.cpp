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
// MyPic.cpp : implementation file
//

#include "stdafx.h"
#include "MyPic.h"
#include "resource.h"
#include "FindWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyPic
CMyPic::CMyPic()
{
	m_pDlg = NULL;
	m_bLButtonDown = FALSE;
}

CMyPic::~CMyPic()
{
}


BEGIN_MESSAGE_MAP(CMyPic, CStatic)
	//{{AFX_MSG_MAP(CMyPic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyPic message handlers

void CMyPic::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	
	HCURSOR hc = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR));
	::SetCursor(hc);

	HICON hicon2 = LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_DRAG_EMPTY));
	this->SetIcon(hicon2);

	m_bLButtonDown = TRUE;
	g_pMainDlg->ShowWindow(FALSE);

	CStatic::OnLButtonDown(nFlags, point);
}

HWND g_hWnd = NULL;
void CMyPic::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();

	HICON hicon = LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_DRAG));
	this->SetIcon(hicon);
	
	g_pMainDlg->ShowWindow(TRUE);

 	CFindWindow *pDlg = (CFindWindow *)m_pDlg;
	pDlg->OnBnClickedBtnFindProcess();
	
	m_bLButtonDown = FALSE;

	CStatic::OnLButtonUp(nFlags, point);
}

void CMyPic::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bLButtonDown == FALSE || m_pDlg == NULL)
	{
		CStatic::OnMouseMove(nFlags, point);
		return;
	}

	POINT pnt;

//	RECT rc;
	HWND DeskHwnd = ::GetDesktopWindow();    //取得桌面句柄
	HDC DeskDC = ::GetWindowDC(DeskHwnd);     //取得桌面设备场景

	int oldRop2 = SetROP2(DeskDC, R2_NOTXORPEN);
	::GetCursorPos(&pnt);                //取得鼠标坐标
	HWND UnHwnd = ::WindowFromPoint(pnt) ;    //取得鼠标指针处窗口句柄

	HWND hWnd = UnHwnd;
	HWND grayHwnd = ::GetWindow(hWnd, GW_CHILD);
	RECT tempRc;
	BOOL bFind=FALSE;
	while (grayHwnd)
	{
		::GetWindowRect(grayHwnd, &tempRc);
		if(::PtInRect(&tempRc,pnt))
		{
			bFind = TRUE;
			break;
		}
		else
		{
			grayHwnd = ::GetWindow(grayHwnd, GW_HWNDNEXT);
		}
	}//while

	if(bFind == TRUE)
	{
		bFind= FALSE;
		hWnd = grayHwnd;
	}
	else
		;//Wnd=UnHwnd

	if (hWnd != g_hWnd)
	{
		g_hWnd = hWnd;
		ULONG nPID = 0, nTid = 0;
		nTid = GetWindowThreadProcessId(g_hWnd, &nPID);	
		
		CString szPid, szTid;
		szPid.Format(L"%d", nPID);
		szTid.Format(L"%d", nTid);

		CFindWindow *pDlg = (CFindWindow *)m_pDlg;
		pDlg->GetDlgItem(IDC_EDIT_PID)->SetWindowText(szPid);
		pDlg->GetDlgItem(IDC_EDIT_TID)->SetWindowText(szTid);
	}

// 	::GetWindowRect(g_hWnd, &rc);      //'获得窗口矩形
// 
// 	if( rc.left < 0 ) rc.left = 0;
// 	if (rc.top < 0 ) rc.top = 0;
// 	//If() rc.Right > Screen.Width / 15 Then rc.Right = Screen.Width / 15
// 	//If rc.Bottom > Screen.Height / 15 Then rc.Bottom = Screen.Height / 15
// 	HPEN newPen = ::CreatePen(0, 3, RGB(125,0,125));    //建立新画笔,载入DeskDC
// 	HGDIOBJ oldPen = ::SelectObject(DeskDC, newPen);
// 	::Rectangle(DeskDC, rc.left, rc.top, rc.right, rc.bottom);  //在指示窗口周围显示闪烁矩形
// 	//::Beep(100,100);
// // 	Sleep(1000);    //设置闪烁时间间隔
// 	::Rectangle( DeskDC, rc.left, rc.top, rc.right, rc.bottom);

// 	::SetROP2(DeskDC, oldRop2);
// 	::SelectObject( DeskDC, oldPen);
// 	::DeleteObject(newPen);
// 	::ReleaseDC( DeskHwnd, DeskDC);
// 	DeskDC = NULL;

	CStatic::OnMouseMove(nFlags, point);
}

void CMyPic::SetFindWndDlg(CDialog* pDlg)
{
	if (pDlg)
	{
		m_pDlg = pDlg;
	}
}