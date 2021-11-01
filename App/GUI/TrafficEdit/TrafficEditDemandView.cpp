// TrafficEditView.cpp : implementation file
//

#include "stdafx.h"
#include "GuiTest3.h"
#include "TrafficEditDemandView.h"
#include "TrafficEditDoc.h"


// CTrafficEditView

IMPLEMENT_DYNCREATE(CTrafficEditDemandView, CBCGPGridView)

CTrafficEditDemandView::CTrafficEditDemandView()
{

}

CTrafficEditDemandView::~CTrafficEditDemandView()
{
}

BEGIN_MESSAGE_MAP(CTrafficEditDemandView, CBCGPGridView)
END_MESSAGE_MAP()


// CTrafficEditView drawing

void CTrafficEditDemandView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	//CTrafficEditDoc* pTDoc = (CTrafficEditDoc*)GetDocument();
	//pTDoc->SetSelectedFlow(::MNM)

}


// CTrafficEditView diagnostics

#ifdef _DEBUG
void CTrafficEditDemandView::AssertValid() const
{
	CBCGPGridView::AssertValid();
}

#ifndef _WIN32_WCE
void CTrafficEditDemandView::Dump(CDumpContext& dc) const
{
	CBCGPGridView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTrafficEditView message handlers


void CTrafficEditDemandView::OnInitialUpdate()
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
	m_wndGrid->InsertColumn(0, _T("Demand Name"), 100);
	m_wndGrid->InsertColumn(1, _T("Mbps (Avg)"), 100);
	m_wndGrid->InsertColumn(2, _T("Mbps (Peak)"), 100);
	m_wndGrid->InsertColumn(3, _T("Frequency"), 100);
	m_wndGrid->InsertColumn(4, _T("Start"), 100);
	m_wndGrid->InsertColumn(5, _T("End"), 100);
	m_wndGrid->InsertColumn(6, _T("Latency (Max)"), 100);
	m_wndGrid->InsertColumn(7, _T("Jitter (Max)"), 100);
	m_wndGrid->InsertColumn(8, _T("Availability (Min)"), 100);
	m_wndGrid->InsertColumn(9, _T("Allow ECMP"), 100);
	m_wndGrid->InsertColumn(10, _T("Active"), 100);
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


void CTrafficEditDemandView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: Add your specialized code here and/or call the base class
}
