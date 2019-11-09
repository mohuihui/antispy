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
#include "stdafx.h"
#include "MzfEdit.h"
// CMyEdit
IMPLEMENT_DYNAMIC(CMyEdit, CEdit)

CMyEdit::CMyEdit()
{
	// 初始化为系统字体和窗口颜色
// 	m_crText = GetSysColor(COLOR_WINDOWTEXT);
// 	m_crBackGnd = GetSysColor(COLOR_WINDOW);
// 	m_font.CreatePointFont(90, L"宋体"); 
// 	m_brBackGnd.CreateSolidBrush(GetSysColor(COLOR_WINDOW));

	m_crText = RGB(255,255,255);
	m_crBackGnd = RGB(0,0,0);
	m_font.CreatePointFont(150, L"宋体"); 
	m_brBackGnd.CreateSolidBrush(RGB(0,0,0));

// 	m_pFont = new CFont;
// 	m_pFont->CreateFont(-13,0,0,0,/*FW_BOLD:*/FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,FALSE,DEFAULT_PITCH,/*L"宋体"*/NULL);
}

CMyEdit::~CMyEdit()
{
// 	delete m_pFont;
// 	m_pFont = NULL;
}

BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	//WM_CTLCOLOR的消息反射
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CMyEdit 消息处理程序

void CMyEdit::SetBackColor(COLORREF rgb)
{
	//设置文字背景颜色
	m_crBackGnd = rgb;

	//释放旧的画刷
	if (m_brBackGnd.GetSafeHandle())
		m_brBackGnd.DeleteObject();
	//使用文字背景颜色创建新的画刷,使得文字框背景和文字背景一致
	m_brBackGnd.CreateSolidBrush(rgb);
	//redraw
	Invalidate(TRUE);
}
void CMyEdit::SetTextColor(COLORREF rgb)
{
	//设置文字颜色
	m_crText = rgb;
	//redraw
	Invalidate(TRUE);
}
void CMyEdit::SetTextFont(const LOGFONT &lf)
{
	//创建新的字体,并设置为CEDIT的字体
	if(m_font.GetSafeHandle())
	{
		m_font.DeleteObject();
	}
	m_font.CreateFontIndirect(&lf);
	CEdit::SetFont(&m_font);
	//redraw
	Invalidate(TRUE);
}

BOOL CMyEdit::GetTextFont(LOGFONT &lf)
{ 
	if(m_font.GetLogFont(&lf)!=0)
	{      return TRUE;   }
	return FALSE;
}

HBRUSH CMyEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
	//刷新前更改文本颜色
	pDC->SetTextColor(m_crText);

	//刷新前更改文本背景
	pDC->SetBkColor(m_crBackGnd);

//	pDC->SelectObject(m_pFont->m_hObject);
// 	HFONT hOldFont = NULL;
// 	if (m_pFont != NULL) 
// 	{
// 		hOldFont =  (HFONT)pMemDC->SelectObject(m_pFont->m_hObject);
// 	}

	//返回画刷,用来绘制整个控件背景
	return m_brBackGnd;
}

BOOL CMyEdit::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;
//	return CEdit::OnEraseBkgnd(pDC);
}
