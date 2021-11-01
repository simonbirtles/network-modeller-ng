#pragma once


class CPaneSplitter : public CSplitterWndEx
{
public:
	BOOL AddWindow(int row, int col, CWnd* pWin, CString clsName, DWORD dwStyle, DWORD dwStyleEx, SIZE sizeInit);
};


// DBViewDockContainer

class DBViewDockContainer : public CDockablePane
{
	DECLARE_DYNAMIC(DBViewDockContainer)

public:
	DBViewDockContainer();
	virtual ~DBViewDockContainer();

protected:
	CPaneSplitter m_wndSplitter;

	CTreeCtrl m_wndTree;
	CMFCPropertyGridCtrl m_wndGrid;

	DECLARE_MESSAGE_MAP()
public:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


