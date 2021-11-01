#include "stdafx.h"
#include "GuiTest3.h"
#include "NetworkViewDoc.h"

IMPLEMENT_DYNCREATE(CNetworkViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CNetworkViewDoc, CDocument)
END_MESSAGE_MAP()

/**
*
*
*
*/
CNetworkViewDoc::CNetworkViewDoc()
{
}
/**
*
*
* This is called when GuiTest3::InitInstance (.. OpenDocumentFile(NULL) ) is called.
*/
//BOOL CNetworkViewDoc::OnNewDocument()
//{
//	if (!CDocument::OnNewDocument())
//		return FALSE;
//	return TRUE;
//}
/**
*
*
*
*/
CNetworkViewDoc::~CNetworkViewDoc()
{
}




#ifdef _DEBUG
void CNetworkViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CNetworkViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

//#ifndef _WIN32_WCE
//// CNetworkViewDoc serialization
//
///**
//* Not seen called yet.. not sure if we can remove ?
//*/
//void CNetworkViewDoc::Serialize(CArchive& ar)
//{
//	if (ar.IsStoring())
//	{
//		// TODO: add storing code here
//	}
//	else
//	{
//		// TODO: add loading code here
//	}
//}
//#endif
