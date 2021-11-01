// TrafficEditFrame.cpp : implementation file
//

#include "stdafx.h"
#include "GuiTest3.h"
#include "TrafficEditFrame.h"
#include "TrafficEditFlowView.h"
#include "TrafficEditDemandView.h"
#include "TrafficEditPathView.h"


// CTrafficEditFrame

IMPLEMENT_DYNCREATE(CTrafficEditFrame, CBCGPMDIChildWnd)

CTrafficEditFrame::CTrafficEditFrame():
	m_bInitSplitter(FALSE)
{

}

CTrafficEditFrame::~CTrafficEditFrame()
{
}

BOOL CTrafficEditFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	CRect cr;
	GetClientRect(&cr);

	if (!m_wndSplitter.CreateStatic(this, 3, 1))
		return FALSE;

	// Flows
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CTrafficEditFlowView), CSize(cr.Width(), cr.Height()/3), pContext))
	{
		MessageBox(L"Error setting up splitter frames!", L"Init Error!", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Demands
	if (!m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CTrafficEditDemandView), CSize(cr.Width(), cr.Height()/3), pContext))
	{
		MessageBox(L"Error setting up splitter frames!", L"Init Error!", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Paths
	if (!m_wndSplitter.CreateView(2, 0, RUNTIME_CLASS(CTrafficEditPathView), CSize(cr.Width(), cr.Height()/3), pContext))
	{
		MessageBox(L"Error setting up splitter frames!", L"Init Error!", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	m_bInitSplitter = TRUE;
	return TRUE;
}

BEGIN_MESSAGE_MAP(CTrafficEditFrame, CBCGPMDIChildWnd)
END_MESSAGE_MAP()


// CTrafficEditFrame message handlers
