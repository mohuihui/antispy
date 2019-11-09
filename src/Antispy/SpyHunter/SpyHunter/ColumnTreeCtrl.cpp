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

#include "stdafx.h"
#include "ColumnTreeCtrl.h"
#include "ListProcessDlg.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// IE 5.0 or higher required
#ifndef TVS_NOHSCROLL
	#error CColumnTreeCtrl requires IE 5.0 or higher; _WIN32_IE should be greater than 0x500.
#endif


//-------------------------------------------------------------------------------
// Helper drawing funtions.
// I tried standard GDI fucntion LineTo with PS_DOT pen style, 
// but that didn't have the effect I wanted, so I had to use these ones. 
//-------------------------------------------------------------------------------

// draws a dotted horizontal line
static void _DotHLine(HDC hdc, LONG x, LONG y, LONG w, COLORREF cr)
{
	for (; w>0; w-=2, x+=2)
		SetPixel(hdc, x, y, cr);
}

// draws a dotted vertical line
static void _DotVLine(HDC hdc, LONG x, LONG y, LONG w, COLORREF cr)
{
	for (; w>0; w-=2, y+=2)
		SetPixel(hdc, x, y, cr);
}

//--------------------------------------------------------------------------------
// CCustomTreeChildCtrl Implementation
//--------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CCustomTreeChildCtrl, CTreeCtrl)

//--------------------------------------------------------------------------------
// Construction/Destruction
//--------------------------------------------------------------------------------

CCustomTreeChildCtrl::CCustomTreeChildCtrl()
{

#ifdef _OWNER_DRAWN_TREE // only if owner-drawn 
	// init bitmap image structure
	m_bkImage.hbm=NULL;
	m_bkImage.xOffsetPercent = 0;
	m_bkImage.yOffsetPercent = 0;

	// create imagelist for tree buttons
	m_imgBtns.Create (9, 9, ILC_COLOR32|ILC_MASK,2,1);
	CBitmap* pBmpBtns = CBitmap::FromHandle(LoadBitmap(AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDB_TREEBTNS)));
	ASSERT(pBmpBtns);
	m_imgBtns.Add(pBmpBtns,RGB(255,0,255));
#endif //_OWNER_DRAWN_TREE

	m_clrNormal = RGB( 0, 0, 255 );					// 正常的进程默认是 蓝色
	m_clrHide = RGB( 255, 0, 0 );					// 隐藏进程默认是 红色
	m_clrMicrosoft = RGB(0, 0, 0);					// 微软进程默认是 黑色
	m_clrMicroHaveOtherModules = RGB( 255, 168, 0 );	// 微软带有非微软模块默认是 橘黄色
}

CCustomTreeChildCtrl::~CCustomTreeChildCtrl()
{
}


BEGIN_MESSAGE_MAP(CCustomTreeChildCtrl, CTreeCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	//ON_NOTIFY( TTN_NEEDTEXT, OnToolTipNeedText )
	//ON_WM_STYLECHANGED()
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

// gets control's background image
BOOL CCustomTreeChildCtrl::GetBkImage(LVBKIMAGE* plvbkImage) const
{
	memcpy(plvbkImage, &m_bkImage, sizeof(LVBKIMAGE));
	return TRUE;
}

// sets background image for control
BOOL CCustomTreeChildCtrl::SetBkImage(LVBKIMAGE* plvbkImage)
{
	memcpy(&m_bkImage, plvbkImage, sizeof(LVBKIMAGE));
	Invalidate();
	return TRUE;
}


//---------------------------------------------------------------------------
// Message Handlers
//---------------------------------------------------------------------------


void CCustomTreeChildCtrl::OnTimer(UINT_PTR nIDEvent)
{
	// Do nothing.
	// CTreeCtrl sends this message to scroll the bitmap in client area
	// which also causes background bitmap scrolling,
	// so we don't pass this message to the base class.
}

void CCustomTreeChildCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// CTreeCtrl may scroll the bitmap up or down in several cases,
	// so we need to invalidate entire client area
	Invalidate();

	//... and pass to the base class
	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCustomTreeChildCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// mask left click if outside the real item's label
	if (CheckHit(point))
	{
		HTREEITEM hItem = HitTest(point);
		if(hItem)
		{
#ifdef _OWNER_DRAWN_TREE
			// if the clicked item is partially visible we won't process
			// the message to avoid background bitmap scrolling
			// TODO: need to avoid scrolling and process the message
			CRect rcItem, rcClient;
			GetClientRect(&rcClient);
			GetItemRect(hItem,&rcItem,FALSE);
			if(rcItem.bottom>rcClient.bottom)
			{
				Invalidate();
				EnsureVisible(hItem);
				SelectItem(hItem);
				//CTreeCtrl::OnLButtonDown(nFlags, point);
				return;
			}
#endif //_OWNER_DRAWN_TREE

			// select the clicked item
			SelectItem(hItem);
		}
		
		// call standard handler	
		CTreeCtrl::OnLButtonDown(nFlags, point);
	}
	else  
	{
		// if clicked outside the item's label
		// than set focus to contol window
		SetFocus(); 
	}
	
}

void CCustomTreeChildCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// process this message only if double-clicked the real item's label
	// mask double click if outside the real item's label
// 	if (CheckHit(point))
// 	{
// 		HTREEITEM hItem = HitTest(point);
// 		if(hItem)
// 		{
// #ifdef _OWNER_DRAWN_TREE
// 			// if the clicked item is partially visible we should invalidate
// 			// entire client area to avoid background bitmap scrolling
// 			CRect rcItem, rcClient;
// 			GetClientRect(&rcClient);
// 			GetItemRect(hItem,&rcItem,FALSE);
// 			if(rcItem.bottom>rcClient.bottom)
// 			{
// 				Invalidate();
// 				CTreeCtrl::OnLButtonDown(nFlags, point);
// 				return;
// 			}
// #endif //_OWNER_DRAWN_TREE
// 
// 			SelectItem(hItem);
// 		}
// 
// 		// call standard message handler
// 		CTreeCtrl::OnLButtonDblClk(nFlags, point);
// 	}	
// 	else 
// 	{
// 		// if clicked outside the item's label
// 		// than set focus to contol window
// 		SetFocus();
// 	}

	if (CheckHit(point))
	{
		HTREEITEM hItem = HitTest(point);
		if(hItem)
		{
			SelectItem(hItem); // 先设置选中项
		}

		((CListProcessDlg*)g_pProcessDlg)->DblclkColumntree(hItem);
	//	CString szText = GetItemText(hItem);
		// call standard message handler
	//	((CListProcessDlg*)g_pProcessDlg)->DblclkColumntree(szText);
	//	CTreeCtrl::OnLButtonDblClk(-1, CPoint(0,0));
	}	
	else 
	{
		// if clicked outside the item's label
		// than set focus to contol window
		SetFocus();
	}
}

void CCustomTreeChildCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// mask mouse move if outside the real item's label
	if (CheckHit(point))
	{
		// call standard handler
		CTreeCtrl::OnMouseMove(nFlags, point);
	}
	
}


#ifdef _OWNER_DRAWN_TREE // this code is only for owner-drawn contol

//----------------------------------------------------------------------
// Sends NM_CUSTOMDRAW notification to the parent (CColumnTreeCtrl)
// The idea is to use one custom drawing code for both custom-drawn and
// owner-drawn controls
//----------------------------------------------------------------------

LRESULT CCustomTreeChildCtrl::CustomDrawNotify(LPNMTVCUSTOMDRAW lpnm)
{
	lpnm->nmcd.hdr.hwndFrom = GetSafeHwnd();
	lpnm->nmcd.hdr.code = NM_CUSTOMDRAW;
	lpnm->nmcd.hdr.idFrom = GetWindowLong(m_hWnd, GWL_ID);
	return GetParent()->SendMessage(WM_NOTIFY,(WPARAM)0,(LPARAM)lpnm);
}

//---------------------------------------------------------------------------
// Performs painting in the client's area.
// The pDC parameter is the memory device context created in OnPaint handler.
//---------------------------------------------------------------------------

LRESULT CCustomTreeChildCtrl::OwnerDraw(CDC* pDC)
{
	NMTVCUSTOMDRAW nm;	// this structure is used by NM_CUSTOMDRAW message
	DWORD dwFlags;		// custom-drawing flags

	DWORD dwRet;		// current custom-drawing operation's return value
	CRect rcClient;		// client's area rectangle

	// get client's rectangle
	GetClientRect(&rcClient);

	// initialize the structure
	memset(&nm,0,sizeof(NMTVCUSTOMDRAW));

	// set current drawing stage to pre-paint
	nm.nmcd.dwDrawStage = CDDS_PREPAINT; 
	nm.nmcd.hdc = pDC->m_hDC;
	nm.nmcd.rc = rcClient;
	
	// notify the parent (CColumnTreeCtrl) about pre-paint stage
    dwFlags = (DWORD)CustomDrawNotify(&nm); // CDDS_PREPAINT
	
	//
	// draw control's background
	//

	// set control's background color
	COLORREF crBkgnd = IsWindowEnabled()?pDC->GetBkColor():GetSysColor(COLOR_BTNFACE);
	// ... and fill the background rectangle
	pDC->FillSolidRect( rcClient, crBkgnd ); 

	if(m_bkImage.hbm && IsWindowEnabled())
	{
		// draw background bitmap

		int xOffset = m_nOffsetX;
		int yOffset = rcClient.left;
		int yHeight = rcClient.Height();

		SCROLLINFO scroll_info;
		// Determine window viewport to draw into taking into account
		// scrolling position
		if ( GetScrollInfo( SB_VERT, &scroll_info, SIF_POS | SIF_RANGE ) )
		{
			yOffset = -scroll_info.nPos;
			yHeight = max( scroll_info.nMax+1, rcClient.Height());
		}
	
		// create temporary memory DC for background bitmap
		CDC dcTemp;
		dcTemp.CreateCompatibleDC(pDC);
		BITMAP bm;
		::GetObject(m_bkImage.hbm,sizeof(BITMAP),&bm);
		CBitmap* pOldBitmap = 
			dcTemp.SelectObject( CBitmap::FromHandle(m_bkImage.hbm) ); 
		
		// copy the background bitmap from temporary DC to painting DC
		float x = (float)m_bkImage.xOffsetPercent/100*(float)rcClient.Width();
		float y = (float)m_bkImage.yOffsetPercent/100*(float)rcClient.Height();
		pDC->BitBlt(/*xOffset*/+(int)x, 
			/*yOffset+*/(int)y, 
			bm.bmWidth, bm.bmHeight, &dcTemp, 0, 0, SRCCOPY);
		
		// clean up
		dcTemp.SelectObject(pOldBitmap);
			
	}

	// notify the parent about post-erase drawing stage
	if(dwFlags&CDRF_NOTIFYPOSTERASE)
	{
		nm.nmcd.dwDrawStage = CDDS_POSTERASE;
		dwRet = (DWORD)CustomDrawNotify(&nm); // CDDS_POSTERASE
	}
	
	// select correct font
	CFont* pOldFont = pDC->SelectObject(GetFont());
	
	// get control's image lists
	CImageList* pstateList = GetImageList(TVSIL_STATE);
	CImageList* pimgList = GetImageList(TVSIL_NORMAL);
	
	// here we will store dimensions of the images
	CSize iconSize, stateImgSize;

	// retreive information about item images
	if(pimgList)
	{
		// get icons dimensions
		IMAGEINFO ii;
		if(pimgList->GetImageInfo(0, &ii))
			iconSize = CSize(ii.rcImage.right-ii.rcImage.left,
			ii.rcImage.bottom-ii.rcImage.top);
	}		
	
	// retrieve information about state images
	if(pstateList)
	{
		// get icons dimensions
		IMAGEINFO ii;
		if(pstateList->GetImageInfo(0, &ii))
			stateImgSize = CSize(ii.rcImage.right-ii.rcImage.left,
			ii.rcImage.bottom-ii.rcImage.top);
	}

	//
	// draw all visible items
	//

	HTREEITEM hItem = GetFirstVisibleItem();

	while(hItem)
	{
		// set transparent background mode
		int nOldBkMode = pDC->SetBkMode(TRANSPARENT);

		// get CTreeCtrl's style
		DWORD dwStyle = GetStyle();

		// get current item's state
		DWORD dwState = GetItemState(hItem,0xFFFF);

		BOOL bEnabled = IsWindowEnabled();
		BOOL bSelected = dwState&TVIS_SELECTED;
		BOOL bHasFocus = (GetFocus() && GetFocus()->m_hWnd==m_hWnd)?TRUE:FALSE;
			

		// Update NMCUSTOMDRAW structure. 
		// We won't draw entire items here (only item icons and lines), 
		// all other work will be done in parent's notifications handlers.
		// This allows to use one code for both custom-drawn and owner-drawn controls.

		nm.nmcd.dwItemSpec = (DWORD_PTR)hItem;
		
		// set colors for item's background and text
		if(bEnabled)
		{	
			COLORREF clrNewTextColor = m_clrMicrosoft;
			int nData = (int)GetItemData(hItem);
			ITEM_COLOR itmClr = ((CListProcessDlg*)g_pProcessDlg)->m_vectorColor.at(nData).textClr;

			if ( itmClr == enumBlue )
			{
				clrNewTextColor = m_clrNormal;
			}
			else if (itmClr == enumRed)
			{
				clrNewTextColor = m_clrHide;
			}
			else if (itmClr == enumTuhuang)
			{
				clrNewTextColor = m_clrMicroHaveOtherModules;
			}

			if(bHasFocus)
			{
				nm.clrTextBk = bSelected?GetSysColor(COLOR_HIGHLIGHT):crBkgnd;
				nm.clrText = bSelected?::GetSysColor(bSelected?COLOR_HIGHLIGHTTEXT:COLOR_MENUTEXT):clrNewTextColor;
				nm.nmcd.uItemState = dwState | (bSelected?CDIS_FOCUS:0);
			}
			else
			{
				if(GetStyle()&TVS_SHOWSELALWAYS)
				{
					nm.clrTextBk = bSelected?GetSysColor(COLOR_INACTIVEBORDER):crBkgnd;
					nm.clrText = /*bSelected?::GetSysColor(COLOR_MENUTEXT):*/clrNewTextColor;
				}
				else
				{
					nm.clrTextBk = crBkgnd;
					nm.clrText = clrNewTextColor;//0xff00ff;//::GetSysColor(COLOR_MENUTEXT);
				}
			}
		}
		else
		{
			nm.clrTextBk = bSelected?GetSysColor(COLOR_BTNSHADOW):crBkgnd;
			nm.clrText = ::GetSysColor(COLOR_GRAYTEXT);
		}
		
		
		// set item's rectangle
		GetItemRect(hItem,&nm.nmcd.rc,0);

		// set clipping rectangle
		CRgn rgn;
		rgn.CreateRectRgn(nm.nmcd.rc.left, nm.nmcd.rc.top, 
			nm.nmcd.rc.left+m_nFirstColumnWidth, nm.nmcd.rc.bottom);
		pDC->SelectClipRgn(&rgn);

		dwRet = CDRF_DODEFAULT;

		// notify the parent about item pre-paint drawing stage
		if(dwFlags&CDRF_NOTIFYITEMDRAW)
		{
			nm.nmcd.dwDrawStage = CDDS_ITEMPREPAINT;
			dwRet = (DWORD)CustomDrawNotify(&nm); // CDDS_ITEMPREPAINT

		}

		if(!(dwFlags&CDRF_SKIPDEFAULT))
		{
			//
			// draw item's icons and dotted lines
			//

			CRect rcItem;
			int nImage,nSelImage;
			
			GetItemRect(hItem,&rcItem,TRUE);
			GetItemImage(hItem,nImage,nSelImage);
				
			int x = rcItem.left-3;
			int yCenterItem = rcItem.top + (rcItem.bottom - rcItem.top)/2; 

			// draw item icon
			if(pimgList)
			{
				int nImageIndex = dwState&TVIS_SELECTED?nImage:nSelImage;
				x-=iconSize.cx+1;
				pimgList->Draw(pDC,nImageIndex,
					CPoint(x, yCenterItem-iconSize.cy/2),ILD_TRANSPARENT);
			}
			
			// draw item state icon
			if(GetStyle()&TVS_CHECKBOXES && pstateList!=NULL)
			{
				// get state image index
				DWORD dwStateImg = GetItemState(hItem,TVIS_STATEIMAGEMASK)>>12;
				
				x-=stateImgSize.cx;

				pstateList->Draw(pDC,dwStateImg,
					CPoint(x, yCenterItem-stateImgSize.cy/2),ILD_TRANSPARENT);
			}

			
			if(dwStyle&TVS_HASLINES)
			{
				//
				// draw dotted lines 
				//

				// create pen
				CPen pen;
				pen.CreatePen(PS_DOT,1,GetLineColor());
				CPen* pOldPen = pDC->SelectObject(&pen);
				
				HTREEITEM hItemParent = GetParentItem(hItem);

				if(hItemParent!=NULL ||dwStyle&TVS_LINESATROOT)
				{
					_DotHLine(pDC->m_hDC,x-iconSize.cx/2-2,yCenterItem,
						iconSize.cx/2+2,RGB(128,128,128));
				}
				
				if(hItemParent!=NULL ||	dwStyle&TVS_LINESATROOT && GetPrevSiblingItem(hItem)!=NULL)
				{
					_DotVLine(pDC->m_hDC,x-iconSize.cx/2-2,rcItem.top,
						yCenterItem-rcItem.top, RGB(128,128,128));
				}
				
				if(GetNextSiblingItem(hItem)!=NULL)
				{
					_DotVLine(pDC->m_hDC,x-iconSize.cx/2-2,yCenterItem,
						rcItem.bottom-yCenterItem,RGB(128,128,128));
				}

				int x1 = x-iconSize.cx/2-2;
				
				while(hItemParent!=NULL )
				{
					x1-=iconSize.cx+3;
					if(GetNextSiblingItem(hItemParent)!=NULL)
					{
						_DotVLine(pDC->m_hDC,x1,rcItem.top,rcItem.Height(),RGB(128,128,128));
					}
					hItemParent = GetParentItem(hItemParent);
				}
			
				// clean up
				pDC->SelectObject(pOldPen);
				
			}

			if(dwStyle&TVS_HASBUTTONS && ItemHasChildren(hItem))
			{
				// draw buttons
				int nImg = GetItemState(hItem,TVIF_STATE)&TVIS_EXPANDED?1:0;
				m_imgBtns.Draw(pDC, nImg, CPoint(x-iconSize.cx/2-6,yCenterItem-4), 
					ILD_TRANSPARENT);
			}

		}

		pDC->SelectClipRgn(NULL);

		// notify parent about item post-paint stage
		if(dwRet&CDRF_NOTIFYPOSTPAINT)
		{
			nm.nmcd.dwDrawStage = CDDS_ITEMPOSTPAINT;
			dwRet = (DWORD)CustomDrawNotify(&nm); // CDDS_ITEMPOSTPAINT
		}

		// clean up
		pDC->SetBkMode(nOldBkMode);

		// get the next visible item
		hItem = GetNextVisibleItem(hItem);
	};

	//clean up

	pDC->SelectObject(pOldFont);
	
	return 0;
}

int CCustomTreeChildCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// inavlidate entire client's area to avoid bitmap scrolling
	Invalidate();

	// ... and call standard message handler
	return CTreeCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

void CCustomTreeChildCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	// inavlidate entire client's area to avoid bitmap scrolling
	Invalidate();
	
	// ... and call standard message handler
	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}


#endif //_OWNER_DRAWN_TREE


void CCustomTreeChildCtrl::OnPaint()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	CPaintDC dc(this);
	
	CDC dcMem;
	CBitmap bmpMem;

	// use temporary bitmap to avoid flickering
	dcMem.CreateCompatibleDC(&dc);
	if (bmpMem.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height()))
	{
		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);

		// paint the window onto the memory bitmap

#ifdef _OWNER_DRAWN_TREE	// if owner-drawn
		OwnerDraw(&dcMem);	// use our code
#else						// else use standard code
		CWnd::DefWindowProc(WM_PAINT, (WPARAM)dcMem.m_hDC, 0);
#endif //_OWNER_DRAWN_TREE

		// copy it to the window's DC
		dc.BitBlt(0, 0, rcClient.right, rcClient.bottom, &dcMem, 0, 0, SRCCOPY);

		dcMem.SelectObject(pOldBmp);

		bmpMem.DeleteObject();
	}
	dcMem.DeleteDC();

}

BOOL CCustomTreeChildCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;	// do nothing
}

BOOL CCustomTreeChildCtrl::CheckHit(CPoint point)
{
	// return TRUE if should pass the message to CTreeCtrl

	UINT fFlags;
	HTREEITEM hItem = HitTest(point, &fFlags);

	CRect rcItem,rcClient;
	GetClientRect(rcClient);
	GetItemRect(hItem, &rcItem, TRUE);

	if (fFlags & TVHT_ONITEMICON ||
		fFlags & TVHT_ONITEMBUTTON ||
		fFlags & TVHT_ONITEMSTATEICON)
		return TRUE;

	if(GetStyle()&TVS_FULLROWSELECT)
	{
		rcItem.right = rcClient.right;
		if(rcItem.PtInRect(point)) 
			return TRUE;
		
		return FALSE;
	}


	// verify the hit result
	if (fFlags & TVHT_ONITEMLABEL)
	{
		rcItem.right = m_nFirstColumnWidth;
		// check if within the first column
		if (!rcItem.PtInRect(point))
			return FALSE;
		
		CString strSub;
		AfxExtractSubString(strSub, GetItemText(hItem), 0, '\t');

		CDC* pDC = GetDC();
		pDC->SelectObject(GetFont());
		rcItem.right = rcItem.left + pDC->GetTextExtent(strSub).cx + 6;
		ReleaseDC(pDC);

		// check if inside the label's rectangle
		if (!rcItem.PtInRect(point))
			return FALSE;
		
		return TRUE;
	}

	return FALSE;
}

BOOL CCustomTreeChildCtrl::OnToolTipNeedText( UINT id, NMHDR * pTTTStruct, LRESULT * pResult )
{
	return FALSE;
}



#define COLUMN_MARGIN		1		// 1 pixel between coumn edge and text bounding rectangle

// default minimum width for the first column
#ifdef _OWNER_DRAWN_TREE
	#define DEFMINFIRSTCOLWIDTH 0 // we use clipping rgn, so we can have zero-width column		
#else
	#define DEFMINFIRSTCOLWIDTH 100	// here we need to avoid zero-width first column	
#endif

IMPLEMENT_DYNCREATE(CColumnTreeCtrl, CStatic)

BEGIN_MESSAGE_MAP(CColumnTreeCtrl, CStatic)
	//ON_MESSAGE(WM_CREATE,OnCreateWindow)
	
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_SETTINGCHANGE()
	ON_WM_ENABLE()
END_MESSAGE_MAP()


CColumnTreeCtrl::CColumnTreeCtrl()
{
	// initialize members
	m_uMinFirstColWidth = DEFMINFIRSTCOLWIDTH;
	m_bHeaderChangesBlocked = FALSE;
	m_xOffset = 0;
}

CColumnTreeCtrl::~CColumnTreeCtrl()
{
}

BOOL CColumnTreeCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CStatic::Create(_T(""), dwStyle, rect, pParentWnd, nID);
	Initialize();
	return TRUE;
}

void CColumnTreeCtrl::PreSubclassWindow()
{
	Initialize();
}

void CColumnTreeCtrl::AssertValid( ) const
{
	// validate control state
#ifdef _DEBUG
	CStatic::AssertValid();
#endif

	ASSERT( m_Tree.m_hWnd ); 
	ASSERT( m_Header.m_hWnd );
	ASSERT( m_Header2.m_hWnd );
}

void CColumnTreeCtrl::Initialize()
{
	if (m_Tree.m_hWnd) 
		return; // do not initialize twice

	CRect rcClient;
	GetClientRect(&rcClient);
	
	// create tree and header controls as children
	m_Tree.Create(WS_CHILD | WS_VISIBLE  | TVS_NOHSCROLL | TVS_NOTOOLTIPS, CRect(), this, TreeID);
	m_Header.Create(WS_CHILD | HDS_BUTTONS | WS_VISIBLE | HDS_FULLDRAG  , rcClient, this, HeaderID);
	m_Header2.Create(WS_CHILD , rcClient, this, Header2ID);

	// create horisontal scroll bar
	m_horScroll.Create(SBS_HORZ|WS_CHILD|SBS_BOTTOMALIGN,rcClient,this,HScrollID);
	
	// set correct font for the header
	CFont* pFont = m_Tree.GetFont();
	m_Header.SetFont(pFont);
	m_Header2.SetFont(pFont);

	// check if the common controls library version 6.0 is available
	BOOL bIsComCtl6 = FALSE;

	HMODULE hComCtlDll = LoadLibrary(_T("comctl32.dll"));

	if (hComCtlDll)
	{
		typedef HRESULT (CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);

		PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION)GetProcAddress(hComCtlDll, "DllGetVersion");

		if (pfnDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			HRESULT hRes = (*pfnDllGetVersion)(&dvi);

			if (SUCCEEDED(hRes) && dvi.dwMajorVersion >= 6)
				bIsComCtl6 = TRUE;
		}

		FreeLibrary(hComCtlDll);
	}

	// get header layout
	WINDOWPOS wp;
	HDLAYOUT hdlayout;
	hdlayout.prc = &rcClient;
	hdlayout.pwpos = &wp;
	m_Header.Layout(&hdlayout);
	m_cyHeader = hdlayout.pwpos->cy;
	
	// offset from column start to text start
	m_xOffset = bIsComCtl6 ? 9 : 6;

	m_xPos = 0;

	UpdateColumns();
}


void CColumnTreeCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	m_Tree.SendMessage(WM_SETTINGCHANGE);
	m_horScroll.SendMessage(WM_SETTINGCHANGE);

	// set correct font for the header
	CRect rcClient;
	GetClientRect(&rcClient);
	
	//CFont* pFont = m_Tree.GetFont();
	//m_Header.SetFont(pFont);
	//m_Header2.SetFont(pFont);
	
	m_Header.SendMessage(WM_SETTINGCHANGE);
	m_Header2.SendMessage(WM_SETTINGCHANGE);
	
	m_Header.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
	m_Header2.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));

	// get header layout
	WINDOWPOS wp;
	HDLAYOUT hdlayout;
	hdlayout.prc = &rcClient;
	hdlayout.pwpos = &wp;
	m_Header.Layout(&hdlayout);
	m_cyHeader = hdlayout.pwpos->cy;

	RepositionControls();
}

void CColumnTreeCtrl::OnPaint()
{
	// do not draw entire background to avoid flickering
	// just fill right-bottom rectangle when it is visible
	
	CPaintDC dc(this);
	
	CRect rcClient;
	GetClientRect(&rcClient);
	
	int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	CBrush brush;
	brush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
	
	CRect rc;

	// determine if vertical scroll bar is visible
	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);
	m_Tree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);
	BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax?TRUE:FALSE;
	
	if(bVScrollVisible)
	{
		// fill the right-bottom rectangle
		rc.SetRect(rcClient.right-cxVScroll, rcClient.bottom-cyHScroll,
				rcClient.right, rcClient.bottom);
		dc.FillRect(rc,&brush);
	}
}

BOOL CColumnTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
	// do nothing, all work is done in OnPaint()
	return FALSE;
	
}

void CColumnTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
	RepositionControls();
}


void CColumnTreeCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);
	
	m_Tree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);
	
	BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax?TRUE:FALSE;
	
	// determine full header width
	int cxTotal = m_cxTotal+(bVScrollVisible?GetSystemMetrics(SM_CXVSCROLL):0);
	
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.Width();

	int xLast = m_xPos;

	switch (nSBCode)
	{
	case SB_LINELEFT:
		m_xPos -= 15;
		break;
	case SB_LINERIGHT:
		m_xPos += 15;
		break;
	case SB_PAGELEFT:
		m_xPos -= cx;
		break;
	case SB_PAGERIGHT:
		m_xPos += cx;
		break;
	case SB_LEFT:
		m_xPos = 0;
		break;
	case SB_RIGHT:
		m_xPos = cxTotal - cx;
		break;
	case SB_THUMBTRACK:
		m_xPos = nPos;
		break;
	}

	if (m_xPos < 0)
		m_xPos = 0;
	else if (m_xPos > cxTotal - cx)
		m_xPos = cxTotal - cx;

	if (xLast == m_xPos)
		return;

	m_Tree.m_nOffsetX = m_xPos;

	SetScrollPos(SB_HORZ, m_xPos);
	CWnd::OnHScroll(nSBCode,nPos,pScrollBar);
	RepositionControls();
	
}

void CColumnTreeCtrl::OnHeaderItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	// do not allow user to set zero width to the first column.
	// the minimum width is defined by m_uMinFirstColWidth;

	if(m_bHeaderChangesBlocked)
	{
		// do not allow change header size when moving it
		// but do not prevent changes the next time the header will be changed
		m_bHeaderChangesBlocked = FALSE;
		*pResult = TRUE; // prevent changes
		return;
	}

	*pResult = FALSE;

	LPNMHEADER pnm = (LPNMHEADER)pNMHDR;
	if(pnm->iItem==0)
	{
		CRect rc;
		m_Header.GetItemRect(0,&rc);
		if(pnm->pitem->cxy<m_uMinFirstColWidth)
		{
			// do not allow sizing of the first column 
			pnm->pitem->cxy=m_uMinFirstColWidth+1;
			*pResult = TRUE; // prevent changes
		}
		return;
	}
	
}

void CColumnTreeCtrl::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateColumns();

	m_Tree.Invalidate();
}


void CColumnTreeCtrl::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	// We use custom drawing to correctly display subitems.
	// Standard drawing code is used only for displaying icons and dotted lines
	// The rest of work is done here.

	NMCUSTOMDRAW* pNMCustomDraw = (NMCUSTOMDRAW*)pNMHDR;
	NMTVCUSTOMDRAW* pNMTVCustomDraw = (NMTVCUSTOMDRAW*)pNMHDR;

	switch (pNMCustomDraw->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
		break;

	case CDDS_ITEMPOSTPAINT:
		{
			HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
			CRect rcItem = pNMCustomDraw->rc;

			if (rcItem.IsRectEmpty())
			{
				// nothing to paint
				*pResult = CDRF_DODEFAULT;
				break;
			}

			CDC dc;
			dc.Attach(pNMCustomDraw->hdc);

			CRect rcLabel;
			m_Tree.GetItemRect(hItem, &rcLabel, TRUE);

			COLORREF crTextBk = pNMTVCustomDraw->clrTextBk;
			COLORREF crWnd = GetSysColor((IsWindowEnabled()?COLOR_WINDOW:COLOR_BTNFACE));
			

#ifndef _OWNER_DRAWN_TREE
			// clear the original label rectangle
			dc.FillSolidRect(&rcLabel, crWnd);
#endif //_OWNER_DRAWN_TREE

			int nColsCnt = m_Header.GetItemCount();

			// draw horizontal lines...
			int xOffset = 0;
			for (int i=0; i<nColsCnt; i++)
			{
				xOffset += m_arrColWidths[i];
				rcItem.right = xOffset-1;
				dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_RIGHT);
			}
			// ...and the vertical ones
			dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_BOTTOM);

			CString strText = m_Tree.GetItemText(hItem);
			CString strSub;
			AfxExtractSubString(strSub, strText, 0, '\t');

			// calculate main label's size
			CRect rcText(0,0,0,0);
			dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_CALCRECT);
			rcLabel.right = min(rcLabel.left + rcText.right + 4, m_arrColWidths[0] - 4);

			BOOL bFullRowSelect = m_Tree.GetStyle()&TVS_FULLROWSELECT;

			if (rcLabel.Width() < 0)
				crTextBk = crWnd;
			if (crTextBk != crWnd)	// draw label's background
			{
				CRect rcSelect =  rcLabel;
				if(bFullRowSelect) rcSelect.right = rcItem.right;
				
				dc.FillSolidRect(&rcSelect, crTextBk);

				// draw focus rectangle if necessary
				if (pNMCustomDraw->uItemState & CDIS_FOCUS)
					dc.DrawFocusRect(&rcSelect);

			}
			
			// draw main label

			CFont* pOldFont = NULL;
			if(m_Tree.GetStyle()&TVS_TRACKSELECT && pNMCustomDraw->uItemState && CDIS_HOT)
			{
				LOGFONT lf;
				pOldFont = m_Tree.GetFont();
				pOldFont->GetLogFont(&lf);
				lf.lfUnderline = 1;
				CFont newFont;
				newFont.CreateFontIndirect(&lf);
				dc.SelectObject(newFont);
			}

			rcText = rcLabel;
			rcText.DeflateRect(2, 1);
			dc.SetTextColor(pNMTVCustomDraw->clrText);
			dc.DrawText(strSub, &rcText, DT_VCENTER | DT_SINGLELINE | 
				DT_NOPREFIX | DT_END_ELLIPSIS);

			xOffset = m_arrColWidths[0];
			dc.SetBkMode(TRANSPARENT);

			if(IsWindowEnabled() && !bFullRowSelect)	
				dc.SetTextColor(::GetSysColor(COLOR_MENUTEXT));
			
			// set not underlined text for subitems
			if( pOldFont &&  !(m_Tree.GetStyle()& TVS_FULLROWSELECT)) 
				dc.SelectObject(pOldFont);

			// draw other columns text
			for (int i=1; i<nColsCnt; i++)
			{
				if (AfxExtractSubString(strSub, strText, i, '\t'))
				{
					rcText = rcLabel;
					rcText.left = xOffset+ COLUMN_MARGIN;
					rcText.right = xOffset + m_arrColWidths[i]-COLUMN_MARGIN;
					rcText.DeflateRect(m_xOffset, 1, 2, 1);
					if(rcText.left<0 || rcText.right<0 || rcText.left>=rcText.right)
					{
						xOffset += m_arrColWidths[i];
						continue;
					}
					DWORD dwFormat = m_arrColFormats[i]&HDF_RIGHT?
						DT_RIGHT:(m_arrColFormats[i]&HDF_CENTER?DT_CENTER:DT_LEFT);
					

					dc.DrawText(strSub, &rcText, DT_SINGLELINE |DT_VCENTER 
						| DT_NOPREFIX | DT_END_ELLIPSIS | dwFormat);
				}
				xOffset += m_arrColWidths[i];
			}

			if(pOldFont) dc.SelectObject(pOldFont);
			dc.Detach();
		}
		*pResult = CDRF_DODEFAULT;
		break;

	default:
		*pResult = CDRF_DODEFAULT;
	}
}


void CColumnTreeCtrl::UpdateColumns()
{
	m_cxTotal = 0;

	HDITEM hditem;
	hditem.mask = HDI_WIDTH;
	int nCnt = m_Header.GetItemCount();
	
	ASSERT(nCnt<=MAX_COLUMN_COUNT);
	
	// get column widths from the header control
	for (int i=0; i<nCnt; i++)
	{
		if (m_Header.GetItem(i, &hditem))
		{
			m_cxTotal += m_arrColWidths[i] = hditem.cxy;
			if (i==0)
				m_Tree.m_nFirstColumnWidth = hditem.cxy;
		}
	}
	m_bHeaderChangesBlocked = TRUE;
	RepositionControls();
}


void CColumnTreeCtrl::RepositionControls()
{
	// reposition child controls
	if (m_Tree.m_hWnd)
	{
	
		CRect rcClient;
		GetClientRect(&rcClient);
		int cx = rcClient.Width();
		int cy = rcClient.Height();

		// get dimensions of scroll bars
		int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
		int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	
		// determine if vertical scroll bar is visible
		SCROLLINFO scrinfo;
		scrinfo.cbSize = sizeof(scrinfo);
		m_Tree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);
		BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax?TRUE:FALSE;
	
		// determine full header width
		int cxTotal = m_cxTotal+(bVScrollVisible?cxVScroll:0);
	
		if (m_xPos > cxTotal - cx) m_xPos = cxTotal - cx;
		if (m_xPos < 0)	m_xPos = 0;
	
		scrinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrinfo.nPage = cx;
		scrinfo.nMin = 0;
		scrinfo.nMax = cxTotal;
		scrinfo.nPos = m_xPos;
		m_horScroll.SetScrollInfo(&scrinfo);

		CRect rcTree;
		m_Tree.GetClientRect(&rcTree);
		
		// move to a negative offset if scrolled horizontally
		int x = 0;
		if (cx < cxTotal)
		{
			x = m_horScroll.GetScrollPos();
			cx += x;
		}
	
		// show horisontal scroll only if total header width is greater
		// than the client rect width and cleint rect height is big enough
		BOOL bHScrollVisible = rcClient.Width()<cxTotal 
			&& rcClient.Height()>=cyHScroll+m_cyHeader;
			
		m_horScroll.ShowWindow(bHScrollVisible?SW_SHOW:SW_HIDE);
	
		m_Header.MoveWindow(-x, 0, cx  - (bVScrollVisible?cxVScroll:0), m_cyHeader);
		
		m_Header2.MoveWindow(rcClient.Width()-cxVScroll, 0, cxVScroll, m_cyHeader);

		m_Tree.MoveWindow(-x, m_cyHeader, cx, cy-m_cyHeader-(bHScrollVisible?cyHScroll:0));
		
		m_horScroll.MoveWindow(0, rcClient.Height()-cyHScroll,
			rcClient.Width() - (bVScrollVisible?cxVScroll:0), cyHScroll);

		
		// show the second header at the top-right corner 
		// only when vertical scroll bar is visible
		m_Header2.ShowWindow(bVScrollVisible?SW_SHOW:SW_HIDE);

		RedrawWindow();
	}
}


int CColumnTreeCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem)
{
	// update the header control in upper-right corner
	// to make it look the same way as main header

	CHeaderCtrl& header = GetHeaderCtrl();

	HDITEM hditem;
	hditem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hditem.fmt = nFormat;
	hditem.cxy = nWidth;
	hditem.pszText = (LPTSTR)lpszColumnHeading;
	m_arrColFormats[nCol] = nFormat;
	int indx =  header.InsertItem(nCol, &hditem);

	if(m_Header.GetItemCount()>0) 
	{
		// if the main header contains items, 
		// insert an item to m_Header2
		hditem.pszText = _T("");
		hditem.cxy = GetSystemMetrics(SM_CXVSCROLL)+5;
		m_Header2.InsertItem(0,&hditem);
	};
	UpdateColumns();
	
	return indx;
}

BOOL  CColumnTreeCtrl::DeleteColumn(int nCol)
{
	// update the header control in upper-right corner
	// to make it look the same way as main header

	BOOL bResult = m_Header.DeleteItem(nCol);
	if(m_Header.GetItemCount()==0) 
	{
		m_Header2.DeleteItem(0);
	}

	UpdateColumns();
	return bResult;
}

CString CColumnTreeCtrl::GetItemText(HTREEITEM hItem, int nColumn)
{
	// retreive and return the substring from tree item's text
	CString szText = m_Tree.GetItemText(hItem);
	CString szSubItem;
	AfxExtractSubString(szSubItem,szText,nColumn,'\t');
	return szSubItem;
}

void CColumnTreeCtrl::SetItemText(HTREEITEM hItem,int nColumn,LPCTSTR lpszItem)
{
	CString szText = m_Tree.GetItemText(hItem);
	CString szNewText, szSubItem;
	int i;
	for(i=0;i<m_Header.GetItemCount();i++)
	{
		AfxExtractSubString(szSubItem,szText,i,'\t');
		if(i!=nColumn) szNewText+=szSubItem+_T("\t");
		else szNewText+=CString(lpszItem)+_T("\t");
	}
	m_Tree.SetItemText(hItem,szNewText);
}

void CColumnTreeCtrl::SetFirstColumnMinWidth(UINT uMinWidth)
{
	// set minimum width value for the first column
	m_uMinFirstColWidth = uMinWidth;
}


// Call this function to determine the location of the specified point 
// relative to the client area of a tree view control.
HTREEITEM CColumnTreeCtrl::HitTest(CPoint pt, UINT* pFlags) const
{
	CTVHITTESTINFO htinfo = {pt, 0, NULL, 0};
	HTREEITEM hItem = HitTest(&htinfo);
	if(pFlags)
	{
		*pFlags = htinfo.flags;
	}
	return hItem;
}

// Call this function to determine the location of the specified point 
// relative to the client area of a tree view control.
HTREEITEM CColumnTreeCtrl::HitTest(CTVHITTESTINFO* pHitTestInfo, BOOL bRclick) const
{
	// We should use our own HitTest() method, because our custom tree
	// has different layout than the original CTreeCtrl.

	UINT uFlags = 0;
	HTREEITEM hItem = NULL;
	CRect rcItem, rcClient;
	
	CPoint point = pHitTestInfo->pt;
	if (bRclick)
	{
		point.x += m_xPos;
		point.y -= m_cyHeader;
	}
	
	hItem = m_Tree.HitTest(point, &uFlags);

	// Fill the CTVHITTESTINFO structure
	pHitTestInfo->hItem = hItem;
	pHitTestInfo->flags = uFlags;
	pHitTestInfo->iSubItem = 0;
		
	if (! (uFlags&TVHT_ONITEMLABEL || uFlags&TVHT_ONITEMRIGHT) )
		return hItem;

	// Additional check for item's label.
	// Determine correct subitem's index.

	int i;
	int x = 0;
	for(i=0; i<m_Header.GetItemCount(); i++)
	{
		if(point.x>=x && point.x<=x+m_arrColWidths[i])
		{
			pHitTestInfo->iSubItem = i;
			pHitTestInfo->flags = TVHT_ONITEMLABEL;
			return hItem;
		}
		x += m_arrColWidths[i];
	}	
	
	pHitTestInfo->hItem = NULL;
	pHitTestInfo->flags = TVHT_NOWHERE;
	return NULL;
}


BOOL CColumnTreeCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	// we need to forward all notifications to the parent window,
    // so use OnNotify() to handle all notifications in one step
	
	LPNMHDR pHdr = (LPNMHDR)lParam;
	
	// there are several notifications we need to precess
		
	if(pHdr->code==NM_CUSTOMDRAW && pHdr->idFrom == TreeID)
	{
		OnTreeCustomDraw(pHdr,pResult);
		return TRUE; // do not forward
	}
	
	if(pHdr->code==HDN_ITEMCHANGING && pHdr->idFrom == HeaderID)
	{
		OnHeaderItemChanging(pHdr,pResult);
		return TRUE; // do not forward
	}
	
	if(pHdr->code==HDN_ITEMCHANGED && pHdr->idFrom == HeaderID)
	{
		OnHeaderItemChanged(pHdr,pResult);
		return TRUE; // do not forward
	}

#ifdef _OWNER_DRAWN_TREE
	
	if(pHdr->code==TVN_ITEMEXPANDING && pHdr->idFrom == TreeID)
	{
		// avoid bitmap scrolling 
		Invalidate(); // ... and forward
	}

#endif //_OWNER_DRAWN_TREE

	if(pHdr->code==TVN_ITEMEXPANDED && pHdr->idFrom == TreeID)
	{
		RepositionControls(); // ... and forward
	}

	// forward notifications from children to the control owner
	pHdr->hwndFrom = GetSafeHwnd();
	pHdr->idFrom = GetWindowLong(GetSafeHwnd(),GWL_ID);
	return (BOOL)GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);
		
}

void CColumnTreeCtrl::OnCancelMode()
{
	m_Header.SendMessage(WM_CANCELMODE);
	m_Header2.SendMessage(WM_CANCELMODE);
	m_Tree.SendMessage(WM_CANCELMODE);
	m_horScroll.SendMessage(WM_CANCELMODE);
	
}
void CColumnTreeCtrl::OnEnable(BOOL bEnable)
{
	m_Header.EnableWindow(bEnable);
	m_Header2.EnableWindow(bEnable);
	m_Tree.EnableWindow(bEnable);
	m_horScroll.EnableWindow(bEnable);
}