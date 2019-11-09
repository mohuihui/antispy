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
#if !defined(AFX_MYPIC_H__C586B8DF_EDB7_4F31_9FE7_850DCAC74579__INCLUDED_)
#define AFX_MYPIC_H__C586B8DF_EDB7_4F31_9FE7_850DCAC74579__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyPic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyPic window

class CMyPic : public CStatic
{
// Construction
public:
	CMyPic();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyPic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyPic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyPic)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CStatic m_mypic;
	BOOL m_bLButtonDown;
	CDialog *m_pDlg;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	void SetFindWndDlg(CDialog* pDlg);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYPIC_H__C586B8DF_EDB7_4F31_9FE7_850DCAC74579__INCLUDED_)
