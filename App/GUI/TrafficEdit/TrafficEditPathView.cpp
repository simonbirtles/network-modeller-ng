// TrafficEditView.cpp : implementation file
//

#include "stdafx.h"
#include "GuiTest3.h"
#include "TrafficEditPathView.h"
#include "TrafficEditDoc.h"


// CTrafficEditPathView

IMPLEMENT_DYNCREATE(CTrafficEditPathView, CBCGPGridView)

CTrafficEditPathView::CTrafficEditPathView()
{

}

CTrafficEditPathView::~CTrafficEditPathView()
{
}

BEGIN_MESSAGE_MAP(CTrafficEditPathView, CBCGPGridView)
END_MESSAGE_MAP()


// CTrafficEditPathView drawing

void CTrafficEditPathView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	//CTrafficEditDoc* pTDoc = (CTrafficEditDoc*)GetDocument();
	//pTDoc->SetSelectedFlow(::MNM)

}


// CTrafficEditPathView diagnostics

#ifdef _DEBUG
void CTrafficEditPathView::AssertValid() const
{
	CBCGPGridView::AssertValid();
}

#ifndef _WIN32_WCE
void CTrafficEditPathView::Dump(CDumpContext& dc) const
{
	CBCGPGridView::Dump(dc);
}
#endif
#endif //_DEBUG


// CTrafficEditPathView message handlers


void CTrafficEditPathView::OnInitialUpdate()
{
	CBCGPGridView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	CBCGPGridCtrl* m_wndGrid = GetGridCtrl();
	if(!m_wndGrid)
		m_wndGrid = CreateGrid();

	m_wndGrid->SetVisualManagerColorTheme();

	//CBCGPGridCtrl* m_wndGrid = GetGridCtrl();

	//// Create the Windows control and attach it to the Grid object
	//RECT rectGrid = {0,0,0,0};
	//
	//m_wndGrid->Create(WS_CHILD | WS_VISIBLE | WS_BORDER, rectGrid, this, 3737);
	// Insert columns:
	m_wndGrid->InsertColumn(0, _T("Start Node"), 100);
	m_wndGrid->InsertColumn(1, _T("End Node"), 100);
	m_wndGrid->InsertColumn(2, _T("Dynamic Path"), 100);
	m_wndGrid->InsertColumn(3, _T("IGP Cost Attribute"), 100);
	m_wndGrid->InsertColumn(4, _T("Exclude"), 100);
	m_wndGrid->InsertColumn(5, _T("Edge List"), 100);
	m_wndGrid->InsertColumn(6, _T("Active"), 100);
	m_wndGrid->InsertColumn(7, _T("Comments"), 100);

	const int nColumns = m_wndGrid->GetColumnCount();
	// Insert 10 rows:
	for (int nRow = 0; nRow < 1; nRow++)
	{
		// Create new row:
		CBCGPGridRow* pRow = m_wndGrid->CreateRow(nColumns);
		// Set each column data:
		/*for (int nColumn = 0; nColumn < nColumns; nColumn++)
		{
			long lValue = (nColumn + 1) * (nRow + 1);
			pRow->GetItem(nColumn)->SetValue(lValue);
		}*/
		// 
		pRow->GetItem(0)->SetValue(L"STC-TBONE-PE1");
		pRow->GetItem(1)->SetValue(L"NJH-TBONE-PE1");
		pRow->GetItem(2)->SetValue(L"weight");		
		pRow->GetItem(5)->SetValue(L"STC-R1;DTC-R1;OCL-R1;NJH-R1");		
		pRow->GetItem(7)->SetValue(L"STC-NJH Red Core (Dynamic)");		
		//Add row to grid:
		m_wndGrid->AddRow(pRow, FALSE /* Don't recal. layout */);

		pRow->ReplaceItem(3, new CBCGPGridCheckItem(TRUE));
		pRow->ReplaceItem(4, new CBCGPGridCheckItem(TRUE));
		pRow->ReplaceItem(6, new CBCGPGridCheckItem(TRUE));


	}
	m_wndGrid->AdjustLayout();

}


void CTrafficEditPathView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: Add your specialized code here and/or call the base class
}
