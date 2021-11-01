#pragma once

// CNetworkViewDoc document

class CNetworkViewDoc : public CDocument
{
	

public:
	virtual ~CNetworkViewDoc();
	//rtual BOOL OnNewDocument();

#ifndef _WIN32_WCE
	//rtual void Serialize(CArchive& ar);   // overridden for document i/o
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	CNetworkViewDoc();
	DECLARE_DYNCREATE(CNetworkViewDoc)
	DECLARE_MESSAGE_MAP()
};
