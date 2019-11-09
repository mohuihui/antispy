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
// CMyEdit
class CMyEdit : public CEdit
{
	DECLARE_DYNAMIC(CMyEdit)

public:
	CMyEdit();
	virtual ~CMyEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	void SetBackColor(COLORREF rgb);//设置文本框背景色
	void SetTextColor(COLORREF rgb);//设置文本框的字体颜色
	void SetTextFont(const LOGFONT &lf);//设置字体
	COLORREF GetBackColor(void){return m_crBackGnd;}//获取当前背景色
	COLORREF GetTextColor(void){return m_crText;}//获取当前文本色
	BOOL GetTextFont(LOGFONT &lf);//获取当前字体

private:
	COLORREF m_crText;//字体的颜色
	COLORREF m_crBackGnd;//字体的背景颜色
	CFont m_font;//字体
	CBrush m_brBackGnd;//整个文本区的画刷
	CFont* m_pFont;

	//控件自己的消息反射函数CtlColor,绘制控件之前调用
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};