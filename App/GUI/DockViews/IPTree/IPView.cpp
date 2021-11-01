#include "stdafx.h"
//#include "IPRadixTree.h"
#include "IPView.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/***************************************************************************
*
* CPaneSplitter
*
****************************************************************************/

/**
*
*
*
*/
BOOL CPaneSplitter::AddWindow(int row, int col, CWnd* pWnd, CString clsName, DWORD dwStyle, DWORD dwStyleEx, SIZE sizeInit)
{
#ifdef _DEBUG
	ASSERT_VALID(this);
	ASSERT(row >= 0 && row < m_nRows);
	ASSERT(col >= 0 && col < m_nCols);
	ASSERT(pWnd);

	if (GetDlgItem(IdFromRowCol(row, col)) != NULL)
	{
		TRACE(traceAppMsg, 0, "Error: CreateView - pane already exists for row %d, col %d.\n", row, col);
		ASSERT(FALSE);
		return FALSE;
	}
#endif

	// set the initial size for that pane
	m_pColInfo[col].nIdealSize = sizeInit.cx;
	m_pRowInfo[row].nIdealSize = sizeInit.cy;
	ASSERT(pWnd->m_hWnd == NULL);       // not yet created
										// Create with the right size (wrong position)
	CRect rect(CPoint(0, 0), sizeInit);
	if (!pWnd->CreateEx(dwStyleEx, clsName, NULL, dwStyle, rect, this, IdFromRowCol(row, col)))
	{
		TRACE(traceAppMsg, 0, "Warning: couldn't create client pane for splitter.\n");
		return FALSE;
	}
	return TRUE;
}
/**
*
*
*
*/
void CPaneSplitter::GetInsideRect(CRect& rect) const
{
	CSplitterWndEx::GetInsideRect(rect);
	//rect.bottom -= m_cxSplitterGap;
}
/**
*
*
*
*/
void CPaneSplitter::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg)
{
	// splitBorder
	//CSplitterWndEx::OnDrawSplitter(pDC, splitBorder, rectArg);

	// if pDC == NULL, then just invalidate
	if (pDC == NULL)
	{
		RedrawWindow(rectArg, NULL, RDW_INVALIDATE | RDW_NOCHILDREN);
		return;
	}

	CRect rect = rectArg;

	switch (nType)
	{
	case splitBorder:
		CMFCVisualManager::GetInstance()->OnDrawSplitterBorder(pDC, this, rect);
		return;

	case splitBox:
		CMFCVisualManager::GetInstance()->OnDrawSplitterBox(pDC, this, rect);
		break;

	case splitIntersection:
	case splitBar:
		break;

	default:
		ASSERT(FALSE);  // unknown splitter type
	}

	// fill the middle	
	pDC->FillSolidRect(rect, GetGlobalData()->clrWindowFrame);  // hbrBtnShadow   GetafxGlobalData()->clrWindowFrame
	//CMFCVisualManager::GetInstance()->OnFillSplitterBackground(pDC, this, rect);   m_clrMainClientArea

	return;
};





/***************************************************************************
*
* IPView
*
****************************************************************************/

IMPLEMENT_DYNAMIC(IPView, CBCGPDockingControlBar)

BEGIN_MESSAGE_MAP(IPView, CBCGPDockingControlBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()
/**
*
*
*
*/
IPView::IPView()//:
	//splitRatio(.5)
{

}
/**
*
*
*
*/
IPView::~IPView()
{
}
/**
*
*
*
*/
int IPView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	if (!m_wndSplitter.CreateStatic(this, 2, 1))
	{
		OutputDebugString(L"Failed to create static splitter");
		return FALSE;
	}


	CRect rect;
	GetClientRect(&rect);
	DWORD dwStyle = WS_CHILD | WS_VISIBLE;



	LPCTSTR strIPprop = AfxRegisterWndClass(NULL);
	if (!m_wndSplitter.AddWindow(1, 0, &m_wndIPProp, strIPprop, WS_VISIBLE | WS_CHILD, 0, CSize(rect.Width(), 200)))
	{
		OutputDebugString(L"Failed AddWindow splitter");
		return FALSE;
	}

	LPCTSTR strIPview = AfxRegisterWndClass(NULL);
	if (!m_wndSplitter.AddWindow(0, 0, &m_wndIPTree, strIPview, dwStyle, 0, CSize(rect.Width(), rect.Height() - 200)))
	{
		OutputDebugString(L"Failed AddWindow splitter");
		return FALSE;
	}

	m_wndIPTree.EnableProperties(&m_wndIPProp);

	
	return 0;
}
/**
*
*
*
*/
void IPView::OnSize(UINT nType, int cx, int cy)
{	
	CBCGPDockingControlBar::OnSize(nType, cx, cy);

	// get current fraction for top window
	int cyCur = 0;
	int cyMin = 0;

	// turn on for static bottom pane
	//****************************************************
	m_wndSplitter.GetRowInfo(1, cyCur, cyMin);

	// turn on for ratio split
	// ****************************************************
	////m_wndSplitter.GetRowInfo(0, cyCur, cyMin);
	////if (cyCur >= 0)
	////{
	////	CRect insideRect;
	////	m_wndSplitter.GetInsideRect(insideRect);
	////	splitRatio = ((double)((double)cyCur / (double)(insideRect.bottom - insideRect.top -7)));
	////	splitRatio = std::floor(splitRatio * 100 + 0.5) / 100;
	////}
	////OutputDebugString(L"\nSplit Ratio\t");
	////OutputDebugString(::std::to_wstring(splitRatio).c_str());


	// always required
	//*****************************************************
	CRect rect;
	GetClientRect(rect);
	m_wndSplitter.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);


	// turn on for ratio split
	// ****************************************************
	////if (cyCur >= 0)
	////{
	////	CRect insideRect;
	////	m_wndSplitter.GetInsideRect(insideRect);
	////	int value = (int)(( (double)(splitRatio * (double)(insideRect.bottom - insideRect.top -7))));
	////	//value = std::floor(value * 100 + 0.0) / 100;
	////	m_wndSplitter.SetRowInfo(0, max(0, value), 2);
	////	m_wndSplitter.RecalcLayout();
	////}


	// turn on for static bottom pane
	//****************************************************
	if (cyCur >= 0)
	{
		CRect insideRect;
		m_wndSplitter.GetInsideRect(insideRect);
		m_wndSplitter.SetRowInfo(0, max(1, insideRect.bottom - insideRect.top - cyCur -7) , 2);
		m_wndSplitter.RecalcLayout();
	}
	return;
}
