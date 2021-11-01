#pragma once

#ifdef _WIN32_WCE
#error "CMDIChildWnd is not supported for Windows CE."
#endif 

// CTrafficEditFrame frame with splitter

class CTrafficEditFrame : public CBCGPMDIChildWnd
{
private:
	BOOL m_bInitSplitter;

	DECLARE_DYNCREATE(CTrafficEditFrame)
protected:
	CTrafficEditFrame();           // protected constructor used by dynamic creation
	virtual ~CTrafficEditFrame();

	CSplitterWnd m_wndSplitter;

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	DECLARE_MESSAGE_MAP()
};


