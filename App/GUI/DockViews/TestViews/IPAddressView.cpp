// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"
#include "mainfrm.h"
#include "IPAddressView.h"
#include "Resource.h"
#include "GuiTest3.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CIPAddressView

CIPAddressView::CIPAddressView()
{
}

CIPAddressView::~CIPAddressView()
{
}

BEGIN_MESSAGE_MAP(CIPAddressView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CIPAddressView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	//CUSTOMCOLORS
	//// change text color to white and background to off black
	//m_wndFileView.SetTextColor(RGB(255, 255, 255));
	//m_wndFileView.SetBkColor(RGB(0.2 * 255, 0.2 * 255, 0.2 * 255));

	// Load view images:
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 0));
	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// Fill in some static tree view data (dummy code, nothing magic here)
	FillFileView();
	AdjustLayout();

	return 0;
}

void CIPAddressView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CIPAddressView::FillFileView()
{
	HTREEITEM hRoot = m_wndFileView.InsertItem(_T("IPv4"), 0, 0);
	m_wndFileView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hSrc = m_wndFileView.InsertItem(_T("10.0.0.0"), 0, 0, hRoot);

	//
	HTREEITEM hInc = m_wndFileView.InsertItem(_T("10.1.0.0"), 0, 0, hSrc);
	HTREEITEM hInc2 = m_wndFileView.InsertItem(_T("10.1.1.0"), 0, 0, hInc);
	HTREEITEM hInc22 = m_wndFileView.InsertItem(_T("10.1.0.0/24"), 2, 2, hInc2);
	HTREEITEM hInc222 = m_wndFileView.InsertItem(_T("DTC-TBONE-PE1"), 3, 3, hInc22);

	//
	HTREEITEM hInc3 = m_wndFileView.InsertItem(_T("10.34.0.0"), 0, 0, hInc);
	HTREEITEM hInc33 = m_wndFileView.InsertItem(_T("10.34.2.0/24"), 2, 2, hInc3);
	HTREEITEM hInc333 = m_wndFileView.InsertItem(_T("UK2-TBONE-PE1"), 3, 3, hInc33);
	
	//
	HTREEITEM hInc4 = m_wndFileView.InsertItem(_T("10.100.0.0"), 0, 0, hInc);
	HTREEITEM hInc44 = m_wndFileView.InsertItem(_T("10.100.21.0/24"), 2, 2, hInc4);
	HTREEITEM hInc444 = m_wndFileView.InsertItem(_T("OCM-TBONE-PE1"), 3, 3, hInc44);
	
	//
	HTREEITEM hInc5 = m_wndFileView.InsertItem(_T("10.254.3.0"), 0, 0, hInc);

	HTREEITEM hInc55 = m_wndFileView.InsertItem(_T("10.254.3.0/30"), 2, 2, hInc5);
	HTREEITEM hInc55a = m_wndFileView.InsertItem(_T("CH4-TBONE-PE1"), 3, 3, hInc55);


	HTREEITEM hInc555 = m_wndFileView.InsertItem(_T("10.254.3.4/30"), 2, 2, hInc5);
	HTREEITEM hInc555a = m_wndFileView.InsertItem(_T("CH4-TBONE-PE2"), 3, 3, hInc555);


	HTREEITEM hInc5555 = m_wndFileView.InsertItem(_T("10.254.3.8/30"), 2, 2, hInc5);
	HTREEITEM hInc5555a = m_wndFileView.InsertItem(_T("CH4-TBONE-PE1"), 3, 3, hInc5555);

	//HTREEITEM hRes = m_wndFileView.InsertItem(_T("10.1.1.0"), 0, 0, hInc);

	//m_wndFileView.InsertItem(_T("10.1.1.1"), 2, 2, hRes);
	//m_wndFileView.InsertItem(_T("FakeApp.rc2"), 2, 2, hRes);
	//m_wndFileView.InsertItem(_T("FakeAppDoc.ico"), 2, 2, hRes);
	//m_wndFileView.InsertItem(_T("FakeToolbar.bmp"), 2, 2, hRes);

	m_wndFileView.Expand(hRoot, TVE_EXPAND);
	m_wndFileView.Expand(hSrc, TVE_EXPAND);
	m_wndFileView.Expand(hInc, TVE_EXPAND);
	m_wndFileView.Expand(hInc2, TVE_EXPAND);
	m_wndFileView.Expand(hInc3, TVE_EXPAND);
	m_wndFileView.Expand(hInc4, TVE_EXPAND);
	m_wndFileView.Expand(hInc5, TVE_EXPAND);

	m_wndFileView.Expand(hInc55, TVE_EXPAND);
	m_wndFileView.Expand(hInc55, TVE_EXPAND);
	m_wndFileView.Expand(hInc44, TVE_EXPAND);
	m_wndFileView.Expand(hInc33, TVE_EXPAND);
	m_wndFileView.Expand(hInc22, TVE_EXPAND);

	m_wndFileView.Expand(hInc555, TVE_EXPAND);
	m_wndFileView.Expand(hInc5555, TVE_EXPAND);

}

void CIPAddressView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndFileView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CIPAddressView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CIPAddressView::OnProperties()
{
	AfxMessageBox(_T("Properties...."));

}

void CIPAddressView::OnFileOpen()
{
	// TODO: Add your command handler code here
}

void CIPAddressView::OnFileOpenWith()
{
	// TODO: Add your command handler code here
}

void CIPAddressView::OnDummyCompile()
{
	// TODO: Add your command handler code here
}

void CIPAddressView::OnEditCut()
{
	// TODO: Add your command handler code here
}

void CIPAddressView::OnEditCopy()
{
	// TODO: Add your command handler code here
}

void CIPAddressView::OnEditClear()
{
	// TODO: Add your command handler code here
}

void CIPAddressView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CIPAddressView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CIPAddressView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}


