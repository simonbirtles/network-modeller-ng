// TrafficEditDoc.cpp : implementation file
//

#include "stdafx.h"
#include "GuiTest3.h"
#include "TrafficEditDoc.h"


// CTrafficEditDoc

IMPLEMENT_DYNCREATE(CTrafficEditDoc, CDocument)

CTrafficEditDoc::CTrafficEditDoc()
{
}

BOOL CTrafficEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CTrafficEditDoc::~CTrafficEditDoc()
{
}


BEGIN_MESSAGE_MAP(CTrafficEditDoc, CDocument)
END_MESSAGE_MAP()


// CTrafficEditDoc diagnostics

#ifdef _DEBUG
void CTrafficEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CTrafficEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CTrafficEditDoc serialization

void CTrafficEditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}
#endif


// CTrafficEditDoc commands
