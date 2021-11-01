// TrafficEditView.cpp : implementation file
//

#include "stdafx.h"
#include "GuiTest3.h"
#include "TrafficEditFlowView.h"
#include "TrafficEditDoc.h"


// CTrafficEditView

IMPLEMENT_DYNCREATE(CTrafficEditFlowView, CBCGPGridView)

CTrafficEditFlowView::CTrafficEditFlowView()
{

}

CTrafficEditFlowView::~CTrafficEditFlowView()
{
}

BEGIN_MESSAGE_MAP(CTrafficEditFlowView, CBCGPGridView)
END_MESSAGE_MAP()


// CTrafficEditView drawing

void CTrafficEditFlowView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	//CTrafficEditDoc* pTDoc = (CTrafficEditDoc*)GetDocument();
	//pTDoc->SetSelectedFlow(::MNM)

}


// CTrafficEditView diagnostics

#ifdef _DEBUG
void CTrafficEditFlowView::AssertValid() const
{
	CBCGPGridView::AssertValid();
}

#ifndef _WIN32_WCE
void CTrafficEditFlowView::Dump(CDumpContext& dc) const
{
	CBCGPGridView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTrafficEditView message handlers


void CTrafficEditFlowView::OnInitialUpdate()
{
	CBCGPGridView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	CBCGPGridCtrl* m_wndGrid = GetGridCtrl();
	if(!m_wndGrid)
		m_wndGrid = CreateGrid();

	//CBCGPGridCtrl* m_wndGrid = GetGridCtrl();

	//// Create the Windows control and attach it to the Grid object
	//RECT rectGrid = {0,0,0,0};
	//
	//m_wndGrid->Create(WS_CHILD | WS_VISIBLE | WS_BORDER, rectGrid, this, 3737);
	// Insert columns:
	m_wndGrid->InsertColumn(0, _T("Source IP"), 300);
	m_wndGrid->InsertColumn(1, _T("Destination IP"), 300);
	m_wndGrid->InsertColumn(2, _T("Traffic Class"), 300);
	m_wndGrid->InsertColumn(3, _T("Active"), 300);
	const int nColumns = m_wndGrid->GetColumnCount();
	// Insert 10 rows:
	for (int nRow = 0; nRow < 100; nRow++)
	{
		// Create new row:
		CBCGPGridRow* pRow = m_wndGrid->CreateRow(nColumns);
		// Set each column data:
		for (int nColumn = 0; nColumn < nColumns; nColumn++)
		{
			long lValue = (nColumn + 1) * (nRow + 1);
			pRow->GetItem(nColumn)->SetValue(lValue);
		}
		// Add row to grid:
		m_wndGrid->AddRow(pRow, FALSE /* Don't recal. layout */);
	}
	m_wndGrid->AdjustLayout();

}


void CTrafficEditFlowView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: Add your specialized code here and/or call the base class
}
