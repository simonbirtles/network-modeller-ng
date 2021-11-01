// DBViewDockContainer.cpp : implementation file
//

#include "stdafx.h"
#include "GuiTest3.h"
#include "DBViewDockContainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/******************************************************************/
BOOL CPaneSplitter::AddWindow(int row, int col, CWnd* pWnd, CString clsName, DWORD dwStyle, DWORD dwStyleEx, SIZE sizeInit)
{
#ifdef _DEBUG
	ASSERT_VALID(this);
	ASSERT(row >= 0 && row < m_nRows);
	ASSERT(col >= 0 && col < m_nCols);
	ASSERT(pWnd);

	if (GetDlgItem(IdFromRowCol(row, col)) != NULL)
	{
		TRACE(traceAppMsg, 0, "Error: CreateView - pane already exists for row %d, col %d.\n",
			row, col);
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
/****************************************************************************/

// DBViewDockContainer

IMPLEMENT_DYNAMIC(DBViewDockContainer, CDockablePane)

DBViewDockContainer::DBViewDockContainer()
{

}

DBViewDockContainer::~DBViewDockContainer()
{
}


BEGIN_MESSAGE_MAP(DBViewDockContainer, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// DBViewDockContainer message handlers


int DBViewDockContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	if (!m_wndSplitter.CreateStatic(this, 2, 1))
	{
		OutputDebugString(L"Failed to create static splitter");
		return FALSE;
	}


	CRect rect;
	GetClientRect(&rect);
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT
		| TVS_HASBUTTONS | TVS_EDITLABELS | TVS_SHOWSELALWAYS | TVS_FULLROWSELECT;


	if (!m_wndSplitter.AddWindow(0, 0, &m_wndTree, WC_TREEVIEW, dwStyle, 0, CSize(rect.Width(), rect.Height()/2)))
	{
		OutputDebugString(L"Failed  AddWindow splitter");
		return FALSE;
	}


	if (!m_wndSplitter.AddWindow(1, 0, &m_wndGrid, WC_TREEVIEW, WS_VISIBLE | WS_CHILD, 0, CSize(rect.Width(), rect.Height()/2)))
	{
		OutputDebugString(L"Failed  AddWindow splitter");
		return FALSE;
	}
	SetWindowLong(m_wndGrid, GWL_EXSTYLE, WS_EX_COMPOSITED | WS_EX_LAYERED);
	m_wndGrid.SetDescriptionRows(4);




	return 0;
}


void DBViewDockContainer::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(rect);
	m_wndSplitter.SetRowInfo(0, rect.Height()/2, 2);
	m_wndSplitter.SetWindowPos(NULL, rect.left , rect.top , rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}
