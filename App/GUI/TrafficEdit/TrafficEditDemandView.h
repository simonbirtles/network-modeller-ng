#pragma once


// CTrafficEditView view

class CTrafficEditDemandView : public CBCGPGridView //CView
{
	DECLARE_DYNCREATE(CTrafficEditDemandView)

protected:
	CTrafficEditDemandView();           // protected constructor used by dynamic creation
	virtual ~CTrafficEditDemandView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};


