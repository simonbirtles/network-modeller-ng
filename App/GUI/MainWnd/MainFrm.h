#pragma once
#include "IPAddressView.h"
#include "DatabaseTreeView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "IPView.h"

//#include "DBViewDockContainer.h"

class CMainFrame : public CBCGPMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	CPropertiesWnd* GetPropertiesWindow(){
		return &m_wndProperties;
	}

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	// ribbon
	CBCGPRibbonBar					m_wndRibbonBar;
	//CMFCRibbonApplicationButton		m_MainButton;
	CMFCToolBarImages				m_PanelIcons;
	// dock windows
	CBCGPRibbonStatusBar			m_wndStatusBar;
	CBCGPCaptionBar					m_wndCaptionBar;
	CDatabaseTreeView				m_wndDatabaseView;
	CPropertiesWnd					m_wndProperties;
	IPView							m_wndIPView;
	COutputWnd						m_wndOutput;

	// TestViews
	//CIPAddressView					m_wndIPAddressView;
	//DBViewDockContainer				m_wndDBDockContainer;
	

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnViewFileView();
	afx_msg void OnUpdateViewFileView(CCmdUI* pCmdUI);
	afx_msg void OnViewClassView();
	afx_msg void OnUpdateViewClassView(CCmdUI* pCmdUI);
	afx_msg void OnViewOutputWindow();
	afx_msg void OnUpdateViewOutputWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewPropertiesWindow();
	afx_msg void OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewCaptionBar();
	afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

	BOOL CreateRibbon();
	BOOL CreateStatusBar();
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


