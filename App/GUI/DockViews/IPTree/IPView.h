#pragma once
#include "stdafx.h"
#include "IPTreeView.h"
#include "IPPropertiesView.h"

class CPaneSplitter : public CSplitterWndEx
{
public:
	void GetInsideRect(CRect& rect) const;
	BOOL AddWindow(int row, int col, CWnd* pWin, CString clsName, DWORD dwStyle, DWORD dwStyleEx, SIZE sizeInit);
	void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg);


};


// IPView

class IPView : public CBCGPDockingControlBar
{
	DECLARE_DYNAMIC(IPView)

private:
	//double splitRatio;

public:
	IPView();
	virtual ~IPView();

protected:
	CPaneSplitter		m_wndSplitter;
	CIPTreeView			m_wndIPTree;
	CIPPropertiesView	m_wndIPProp;

	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


