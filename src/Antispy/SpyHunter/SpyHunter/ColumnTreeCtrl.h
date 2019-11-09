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
/*****************************************************************************
* CColumnTreeCtrl
* Version: 1.1 
* Date: February 18, 2008
* Author: Oleg A. Krivtsov
* E-mail: olegkrivtsov@mail.ru
* Based on ideas implemented in Michal Mecinski's CColumnTreeCtrl class 
* (see copyright note below).
*
*****************************************************************************/

/*********************************************************
* Multi-Column Tree View
* Version: 1.1
* Date: October 22, 2003
* Author: Michal Mecinski
* E-mail: mimec@mimec.w.pl
* WWW: http://www.mimec.w.pl
*
* You may freely use and modify this code, but don't remove
* this copyright note.
*
* There is no warranty of any kind, express or implied, for this class.
* The author does not take the responsibility for any damage
* resulting from the use of it.
*
* Let me know if you find this code useful, and
* send me any modifications and bug reports.
*
* Copyright (C) 2003 by Michal Mecinski
*********************************************************/
#pragma once

#include "resource.h"

#define _OWNER_DRAWN_TREE  // comment this line if you want to use standard drawing code

#ifdef _OWNER_DRAWN_TREE
#ifndef IDB_TREEBTNS
	#error You should insert IDB_TREEBTNS bitmap to project resources. See control documentation for more info.
#endif //IDB_TREEBTNS
#endif //_OWNER_DRAWN_TREE

typedef struct _CTVHITTESTINFO { 
  POINT pt; 
  UINT flags; 
  HTREEITEM hItem; 
  int iSubItem;
} CTVHITTESTINFO;


class CCustomTreeChildCtrl : public CTreeCtrl
{
	friend class CColumnTreeCtrl;

	DECLARE_DYNAMIC(CCustomTreeChildCtrl)

public:

	/*
	 *  Construction/destruction
	 */
	
	CCustomTreeChildCtrl();
	virtual ~CCustomTreeChildCtrl();

	/*
	 * Operations
	 */
	
	BOOL GetBkImage(LVBKIMAGE* plvbkImage) const;
	BOOL SetBkImage(LVBKIMAGE* plvbkImage);

protected:
	DECLARE_MESSAGE_MAP()

	int m_nFirstColumnWidth; // the width of the first column 
	int m_nOffsetX;      	 // offset of this window inside the parent 
	LVBKIMAGE m_bkImage;	 // information about background image
	CImageList m_imgBtns;	 // tree buttons images (IDB_TREEBTNS)

	BOOL CheckHit(CPoint point);

	
	/*
	 * Message Handlers
	 */

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	BOOL OnToolTipNeedText( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );

	/*
	 * Custom drawing related methods
	 */
	
#ifdef _OWNER_DRAWN_TREE
	LRESULT CustomDrawNotify(LPNMTVCUSTOMDRAW lpnm);
	LRESULT OwnerDraw(CDC* pDC);
	int OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
#endif //_OWNER_DRAWN_TREE

public:
	void SetItemColor(COLORREF clrNormal, COLORREF clrHide, COLORREF clrMicrosoft, COLORREF clrMicroHaveOtherModules)
	{
		m_clrNormal = clrNormal;
		m_clrHide = clrHide;
		m_clrMicrosoft = clrMicrosoft;
		m_clrMicroHaveOtherModules = clrMicroHaveOtherModules;
	}

private:
	COLORREF m_clrNormal;
	COLORREF m_clrHide;
	COLORREF m_clrMicrosoft;
	COLORREF m_clrMicroHaveOtherModules;
};


class CColumnTreeCtrl : public CStatic
{
public:
	DECLARE_DYNCREATE(CColumnTreeCtrl)
	
	/*
	 * Construction/destruction
	 */
	 
	CColumnTreeCtrl();
	virtual ~CColumnTreeCtrl();

	// explicit construction 
	BOOL Create(DWORD dwStyle , const RECT& rect, CWnd* pParentWnd, UINT nID);

	virtual void PreSubclassWindow();

		/*
	 *  Operations
	 */

	virtual void AssertValid( ) const;

	CCustomTreeChildCtrl& GetTreeCtrl() { return m_Tree; }
	CHeaderCtrl& GetHeaderCtrl() { return m_Header; }

	int InsertColumn(int nCol,LPCTSTR lpszColumnHeading, int nFormat=0, int nWidth=-1, int nSubItem=-1);
	BOOL DeleteColumn(int nCol);

	void SetFirstColumnMinWidth(UINT uMinWidth);
		
	CString GetItemText(HTREEITEM hItem, int nSubItem);
	void SetItemText(HTREEITEM hItem, int nSubItem, LPCTSTR lpszText);

	HTREEITEM HitTest(CPoint pt, UINT* pFlags=NULL) const;
	HTREEITEM HitTest(CTVHITTESTINFO* pHitTestInfo, BOOL bRclick = TRUE) const;
protected:
	
	DECLARE_MESSAGE_MAP()

	enum ChildrenIDs { HeaderID = 1, TreeID = 2, HScrollID = 3, Header2ID = 4};
	
	CCustomTreeChildCtrl m_Tree;
	CScrollBar m_horScroll;
	CHeaderCtrl m_Header;
	CHeaderCtrl m_Header2;
	
	int m_cyHeader;
	int m_cxTotal;
	int m_xPos;
	int m_xOffset;
	int m_uMinFirstColWidth;
	BOOL m_bHeaderChangesBlocked;

	enum{MAX_COLUMN_COUNT=16}; // change this value if you need more than 16 columns

	int m_arrColWidths[MAX_COLUMN_COUNT];
	DWORD m_arrColFormats[MAX_COLUMN_COUNT];
	
	virtual void Initialize();
	void UpdateColumns();
	void RepositionControls();

	virtual void OnDraw(CDC* pDC) {}
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHeaderItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCancelMode();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC); 
};