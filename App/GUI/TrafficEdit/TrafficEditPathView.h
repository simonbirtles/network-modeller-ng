#pragma once


// CTrafficEditView view

class CTrafficEditPathView : public CBCGPGridView //CView
{
	DECLARE_DYNCREATE(CTrafficEditPathView)

protected:
	CTrafficEditPathView();           // protected constructor used by dynamic creation
	virtual ~CTrafficEditPathView();

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


