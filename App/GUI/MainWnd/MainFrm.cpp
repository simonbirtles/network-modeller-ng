#include "stdafx.h"
#include "GuiTest3.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMainFrame, CBCGPMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CBCGPMDIFrameWnd)
	ON_WM_CREATE()

	/*ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)

	ON_COMMAND(ID_VIEW_CAPTION_BAR, &CMainFrame::OnViewCaptionBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &CMainFrame::OnUpdateViewCaptionBar)

	ON_COMMAND(ID_TOOLS_OPTIONS, &CMainFrame::OnOptions)

	ON_COMMAND(ID_VIEW_IPADDRVIEW, &CMainFrame::OnViewFileView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IPADDRVIEW, &CMainFrame::OnUpdateViewFileView)

	ON_COMMAND(ID_VIEW_CLASSVIEW, &CMainFrame::OnViewClassView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CLASSVIEW, &CMainFrame::OnUpdateViewClassView)

	ON_COMMAND(ID_VIEW_OUTPUTWND, &CMainFrame::OnViewOutputWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWND, &CMainFrame::OnUpdateViewOutputWindow)

	ON_COMMAND(ID_VIEW_PROPERTIESWND, &CMainFrame::OnViewPropertiesWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIESWND, &CMainFrame::OnUpdateViewPropertiesWindow)*/

	ON_WM_SETTINGCHANGE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/**
*
*
*
*/
CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
}

CMainFrame::~CMainFrame()
{
}
/**
*
*
*
*/
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPMDIFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;

	// Enable/Create document tab groups
	CBCGPMDITabParams mdiTabParams;
	mdiTabParams.m_style = CBCGPTabWnd::STYLE_3D_SCROLLED;			// STYLE_3D; //STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = FALSE;					// set to FALSE to remove close button from tab
	mdiTabParams.m_bTabCloseButton = FALSE;					// set to FALSE to remove close button from tab
	mdiTabParams.m_bTabIcons = FALSE;								// set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = FALSE;								// set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bFlatFrame = FALSE;
	mdiTabParams.m_nTabBorderSize = 0;
	mdiTabParams.m_bDocumentMenu = FALSE;							// enable the document menu at the right edge of the tab area	
	EnableMDITabbedGroups(TRUE, mdiTabParams);
	
	// Create the ribbon menu	
	/*
	if (!CreateRibbon()) {
		TRACE0("Failed to create ribbon\n");
		return -1;
	}
	*/

	// Create the Status Bar	
	/*
	if (!CreateStatusBar()) {
		TRACE0("Failed to create status bar\n");
		return -1;
	}
	*/

	// Create the docking windows	
	if (!CreateDockingWindows()){
		TRACE0("Failed to create docking windows\n");
		return -1;
	}
		
	m_wndDatabaseView.EnableDocking(CBRS_ALIGN_ANY);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	m_wndIPView.EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndIPAddressView.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndDBDockContainer.EnableDocking(CBRS_ALIGN_ANY);	
	EnableDocking(CBRS_ALIGN_ANY);

	DockControlBar(&m_wndDatabaseView);
	DockControlBar(&m_wndProperties);
	DockControlBar(&m_wndIPView);
	DockControlBar(&m_wndOutput);
	//DockControlBar(&m_wndIPAddressView);
	//DockControlBar(&m_wndDBDockContainer);

	// Load menu item image (not placed on any standard toolbars):
	////CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);
	
	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);

	// enable Visual Studio 2005 style docking window auto-hide behavior
	////EnableAutoHideBars(CBRS_ALIGN_ANY);

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
}
/**
* 
*
*
*/
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CBCGPMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU ;
	
	return TRUE;
}

void CMainFrame::OnClose()
{
	SaveMDIState(theApp.GetRegSectionPath());
	CBCGPMDIFrameWnd::OnClose();
}

/*
	https://www.bcgsoft.com/doc/ribbon.htm
*/
BOOL CMainFrame::CreateRibbon() {
	
	m_wndRibbonBar.Create(this);

	//m_PanelIcons.SetImageSize(CSize(16, 16));
	//m_PanelIcons.Load(IDB_PANEL_ICONS);

	// add catagory

	CBCGPRibbonCategory* pCategory = m_wndRibbonBar.AddCategory(_T("&Write"), 0, 0);

	/*
	CBCGPRibbonCategory* pCategory = m_wndRibbonBar.AddCategory(_T("&Write"),           // Category name
																IDB_WRITE,              // Category small images (16 x 16)
																IDB_WRITE_LARGE);   // Category large images (32 x 32)
	*/

	// add panel
	/*
	CBCGPRibbonPanel* pPanel = pCategory->AddPanel(_T("Clipboard"),                       // Panel name
													m_PanelIcons.ExtractIcon(0));  // Panel icon
	*/



	return TRUE;
}

BOOL CMainFrame::CreateStatusBar() {

	// Create Status Bar
	//if (!m_wndStatusBar.Create(this))
	//{
	//	TRACE0("Failed to create status bar\n");
	//	return -1;      // fail to create
	//}
	//m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, L"Pane1", TRUE), L"Pane1");
	//m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, L"Pane2", TRUE), L"Pane2");
	
	return TRUE;
}


/**
* CreateDockingWindows
*
*
*
*/
BOOL CMainFrame::CreateDockingWindows()
{
	// Create class view ( Database Object Tree)
	//if (!m_wndDBDockContainer.Create(L"Database Dock View", this, CRect(0, 0, 200, 200), TRUE, ID_DB_DOCKVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	//{
	//	TRACE0("Failed to create Database Dock View window\n");
	//	return FALSE; // failed to create
	//}
	//m_wndIPView

	// Create Internet Protocol Tree/Prop View
	if (!m_wndIPView.Create(L"Internet Protocol View", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_INETP, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create InetP View window\n");
		return FALSE; // failed to create
	}
	

	// Create class view ( Database Object Tree)
	if (!m_wndDatabaseView.Create(L"Database View", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CLASSVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Database View window\n");
		return FALSE; // failed to create
	}	

	// Create properties window (Database Object Attributes)	
	if (!m_wndProperties.Create(L"Properties", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}
		
	// Create IPAddress Tree view
	//if (!m_wndIPAddressView.Create(L"Network IP", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_IPADDRVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
	//{
	//	TRACE0("Failed to create File View window\n");
	//	return FALSE; // failed to create
	//}

	// Create output window (tabs for ServiceState, Log, Path) 	
	if (!m_wndOutput.Create(L"Output", this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Information Output window\n");
		return FALSE; // failed to create
	}
	
	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}
/**
* SetDockingWindowIcons
*
*
*
*/
void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	//HICON hIPAddrViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	//m_wndIPAddressView.SetIcon(hIPAddrViewIcon, FALSE);

	HICON hIPAddrViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndIPView.SetIcon(hIPAddrViewIcon, FALSE);

	HICON hDatabaseViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndDatabaseView.SetIcon(hDatabaseViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

	//HICON hDBDockBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	//m_wndDBDockContainer.SetIcon(hDBDockBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();
}
/**
*
*
*
*/
//BOOL CMainFrame::CreateCaptionBar()
//{
//	if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
//	{
//		TRACE0("Failed to create caption bar\n");
//		return FALSE;
//	}
//
//	BOOL bNameValid;
//
//	CString strTemp, strTemp2;
//	bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON);
//	ASSERT(bNameValid);
//	m_wndCaptionBar.SetButton(strTemp, ID_TOOLS_OPTIONS, CBCGPCaptionBar::ALIGN_LEFT, FALSE);
//	bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON_TIP);
//	ASSERT(bNameValid);
//	m_wndCaptionBar.SetButtonToolTip(strTemp);
//
//	bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
//	ASSERT(bNameValid);
//	m_wndCaptionBar.SetText(strTemp, CBCGPCaptionBar::ALIGN_LEFT);
//
//	m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CBCGPCaptionBar::ALIGN_LEFT);
//	bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
//	ASSERT(bNameValid);
//	bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
//	ASSERT(bNameValid);
//	m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);
//
//	return TRUE;
//}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CBCGPMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CBCGPMDIFrameWnd::Dump(dc);
}
#endif //_DEBUG


BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return CBCGPMDIFrameWnd::OnEraseBkgnd(pDC);
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CBCGPMDIFrameWnd::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}

void CMainFrame::OnApplicationLook(UINT id)
{
	theApp.m_nAppLook = CBCGPWinApp::BCGP_VISUAL_THEME_VS_2019_DARK;;
	theApp.SetVisualTheme(CBCGPWinApp::BCGP_VISUAL_THEME_VS_2019_DARK);
	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
	//m_wndOutput.UpdateFonts();
	//RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}





void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCaptionBar()
{
	m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
	RecalcLayout(FALSE);
}

void CMainFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
}

void CMainFrame::OnOptions()
{
	CBCGPRibbonCustomize* pOptionsDlg = new CBCGPRibbonCustomize(this, &m_wndRibbonBar);
	ASSERT(pOptionsDlg != NULL);
	pOptionsDlg->DoModal();
	delete pOptionsDlg;
}

//void CMainFrame::OnViewFileView()
//{
//	// Show or activate the pane, depending on current state.  The
//	// pane can only be closed via the [x] button on the pane frame.
//	m_wndIPAddressView.ShowPane((m_wndIPAddressView.IsVisible() ? FALSE : TRUE), FALSE, (m_wndIPAddressView.IsVisible() ? FALSE : TRUE));
//	if(m_wndIPAddressView.IsVisible())
//		m_wndIPAddressView.SetFocus();
//}
//
//void CMainFrame::OnUpdateViewFileView(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable(TRUE);
//	pCmdUI->SetCheck(m_wndIPAddressView.IsVisible());
//}

void CMainFrame::OnViewClassView()
{
	m_wndDatabaseView.ShowControlBar((m_wndDatabaseView.IsVisible() ? FALSE : TRUE), FALSE, (m_wndDatabaseView.IsVisible() ? FALSE : TRUE));
	if (m_wndDatabaseView.IsVisible())
		m_wndDatabaseView.SetFocus();

	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	//m_wndDatabaseView.ShowPane(TRUE, FALSE, TRUE);
	//m_wndDatabaseView.SetFocus();
}

void CMainFrame::OnUpdateViewClassView(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_wndDatabaseView.IsVisible());
}

void CMainFrame::OnViewOutputWindow()
{
	m_wndOutput.ShowControlBar((m_wndOutput.IsVisible() ? FALSE : TRUE), FALSE, (m_wndOutput.IsVisible() ? FALSE : TRUE));
	if (m_wndOutput.IsVisible())
		m_wndOutput.SetFocus();

	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	//m_wndOutput.ShowPane(TRUE, FALSE, TRUE);
	//m_wndOutput.SetFocus();
}

void CMainFrame::OnUpdateViewOutputWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_wndOutput.IsVisible());
}

void CMainFrame::OnViewPropertiesWindow()
{
	m_wndProperties.ShowControlBar((m_wndProperties.IsVisible() ? FALSE : TRUE), FALSE, (m_wndProperties.IsVisible() ? FALSE : TRUE));
	if (m_wndProperties.IsVisible())
		m_wndProperties.SetFocus();

	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	//m_wndProperties.ShowPane(TRUE, FALSE, TRUE);
	//m_wndProperties.SetFocus();
}

void CMainFrame::OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_wndProperties.IsVisible());
}

