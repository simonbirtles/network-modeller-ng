#include "stdafx.h"
#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#include "IServiceRegistry.h"			
extern NM::Registry::IServiceRegistry* reg;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************************************
* Class ClassCOutputWnd
*
* The main window for tabs, creates tab window, then tab child windows (actual tabs)
* Instantiated by CMainFrm
******************************************************************************************************/
#pragma region ClassCOutputWnd

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CBCGPDockingControlBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/**
*
*
*
*/
int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT/*STYLE_3D_SCROLLED*/, rectDummy, this, 1)) //STYLE_FLAT,   
	{
		TRACE0("Failed to create output tab window\n");
		return -1;      // fail to create
	}

	// Create output panes (to be tabs):
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndOutputServiceState.Create(dwStyle | LVS_REPORT, rectDummy, &m_wndTabs, 2) ||				// ServiceState
		!m_wndDatabaseLog.Create(dwStyle | LBS_USETABSTOPS, rectDummy, &m_wndTabs, 3) ||							// db change Log
		!m_wndOutputPath.Create(dwStyle, rectDummy, &m_wndTabs, 4) ||							// Path
		!m_wndOutputGrid.Create(dwStyle, rectDummy, &m_wndTabs, 5)								// Grid = no current use
		)
	{
		TRACE0("Failed to create output windows\n");
		return -1;      // fail to create
	}

	UpdateFonts();

	// Attach list windows to tab:
	m_wndTabs.AddTab(&m_wndOutputServiceState, L"Service State", (UINT)0);
	m_wndTabs.AddTab(&m_wndDatabaseLog, L"Database Notifications Log", (UINT)1);	
	m_wndTabs.AddTab(&m_wndOutputPath,  L"Path", (UINT)2);	
	m_wndTabs.AddTab(&m_wndOutputGrid,  L"Grid", (UINT)3);

	// Fill output tabs with some dummy text (nothing magic here)
	InitServiceStateWindow();
	//InitLogWindow();
	InitPathWindow();
	FillGridWindow();

	return 0;
}
/**
*
*
*
*/
void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}
/**
*
*
*
*/
void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}
/**
*
*
* Not used at present
*/
void COutputWnd::FillGridWindow()
{
	m_wndOutputGrid.EnableHeaderCtrl();
	//m_wndPropList.EnableDescriptionArea();
	m_wndOutputGrid.SetVSDotNetLook(TRUE);
	m_wndOutputGrid.MarkModifiedProperties(TRUE);
	m_wndOutputGrid.SetAlphabeticMode(!TRUE);
	m_wndOutputGrid.SetShowDragContext(TRUE);
}
/**
*
*
* Basic get state once... no periodic checking yet 
*/
void COutputWnd::InitServiceStateWindow()
{
	m_wndOutputServiceState.RefreshContent();
}

/**
*
*
*
*/
void COutputWnd::InitLogWindow()
{
	m_wndDatabaseLog.AddString(L"InitLogWindow");
}
/**
*
*
*
*/
void COutputWnd::InitPathWindow()
{
	m_wndOutputPath.AddString(L"InitPathWindow");
}
/**
*
*
*
*/
void COutputWnd::UpdateFonts()
{
	m_wndOutputServiceState.SetFont(&afxGlobalData.fontRegular);
	m_wndDatabaseLog.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputPath.SetFont(&afxGlobalData.fontRegular);
}

#pragma endregion ClassCOutputWnd
/*****************************************************************************************************
* Class COutputList
*
* Use by Path - does nothing
******************************************************************************************************/
#pragma region ClassCOutputList


/**
*
*
*
*/
COutputList::COutputList()
{
}
/**
*
*
*
*/
COutputList::~COutputList()
{
}
/**
*
*
*
*/
BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

// COutputList message handlers
/**
*
*
*
*/
void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}
/**
*
*
*
*/
void COutputList::OnEditCopy()
{
	MessageBox(_T("Copy output"));
}
/**
*
*
*
*/
void COutputList::OnEditClear()
{
	MessageBox(_T("Clear output"));
}
/**
*
*
*
*/
void COutputList::OnViewOutput()
{
	CBCGPDockingControlBar* pParentBar = DYNAMIC_DOWNCAST(CBCGPDockingControlBar, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{

		pMainFrame->SetFocus();
		//pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}

#pragma endregion ClassCOutputList
/*****************************************************************************************************
* Class COutputGrid
*
* Used by OutputGrid which serves no current purpose
******************************************************************************************************/
#pragma region ClassCOutputGrid


/**
*
*
*
*/
COutputGrid::COutputGrid()
{
}
/**
*
*
*
*/
COutputGrid::~COutputGrid()
{
}
/**
*
*
*
*/
BEGIN_MESSAGE_MAP(COutputGrid, CMFCPropertyGridCtrl)
	//ON_WM_CONTEXTMENU()
	//ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	//ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	//ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

#pragma endregion ClassCOutputGrid
/*****************************************************************************************************
* Class CServiceState
*
* Gets a string of the services state and displays states
******************************************************************************************************/
#pragma region ClassCServiceState
/**
*
*
*
*/
BEGIN_MESSAGE_MAP(CServiceState, CListCtrl)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_REFRESH_SERVICES, OnRefresh)
	//ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	//ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/**
*
*
*
*/
CServiceState::CServiceState()
{
}
/**
*
*
*
*/
CServiceState::~CServiceState()
{
}
/**
*
*
*
*/
void CServiceState::OnRefresh()
{
	RefreshContent();
}
/**
*
*
*
*/
void CServiceState::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_SERVICES_STATE);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}
/**
*
*
*
*/
void CServiceState::RefreshContent()
{
	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	DeleteAllItems();
	DeleteColumn(3);
	DeleteColumn(2);
	DeleteColumn(1);
	DeleteColumn(0);

	LV_COLUMN col = { 0 };
	col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	col.pszText = L"Name (Handle)";
	col.cx = 200;
	InsertColumn(0, &col);

	col.pszText = L"Description";
	col.cx = 250;
	InsertColumn(1, &col);

	col.pszText = L"Type (Real/Phantom)";
	col.cx = 150;
	InsertColumn(2, &col);

	col.pszText = L"Operational State";
	col.cx = 150;
	InsertColumn(3, &col);

	// Get the service status text from the RegistryService
	::std::wstring infostr = reg->GetServiceInfoText();

	// take string and split into elements using \n as delimiter into a vector of wstrings
	std::vector<std::wstring> tokens;
	std::size_t start = 0, end = 0;
	while ((end = infostr.find('\n', start)) != std::string::npos)
	{
		tokens.push_back(infostr.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(infostr.substr(start));

	std::vector<std::wstring>::iterator it = tokens.begin();
	// for each wstring elment in the vector, create a new container object and call Set(wstring) method
	while (it != tokens.end())
	{
		// for each wstring elment in the vector,
		// take string and split into elements using \t as delimiter into a vector of wstrings
		std::vector<std::wstring> tokens2;
		std::size_t start2 = 0, end2 = 0;
		while ((end2 = (*it).find('\t', start2)) != std::string::npos)
		{
			tokens2.push_back((*it).substr(start2, end2 - start2));
			start2 = end2 + 1;
		}
		tokens2.push_back((*it).substr(start2));

		if ((tokens2.size() > 0) && (tokens2[0].size() > 0))
		{
			//insert the item (row)
			LV_ITEM item = { 0 };
			item.pszText = (LPWSTR)tokens2[0].c_str(); // Sends an LVN_GETDISPINFO message.
			item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_COLUMNS;
			item.stateMask = 0;
			item.iSubItem = 0;
			item.state = 0;
			int itemIndex = InsertItem(&item);

			// update text in each cell for this item
			int subItem = 0;
			for each(::std::wstring text in tokens2)
			{
				LV_ITEM item = { 0 };
				item.pszText = (LPWSTR)text.c_str(); // Sends an LVN_GETDISPINFO message.
				item.mask = LVIF_TEXT;
				item.iItem = itemIndex;
				item.iSubItem = subItem;
				SetItem(&item);
				++subItem;
			}
		}
		++it;
	}

	return;
}



#pragma endregion ClassCServiceState
/*****************************************************************************************************
* Class CDatabaseNotificationsList
*
* Provides view on database notifications
******************************************************************************************************/
#pragma region ClassCDatabaseNotificationsList
#include "Interfaces\IObjectDatabase.h"
#include "Interfaces\IDatabaseUpdate.h"	
/**
*
*
*
*/
BEGIN_MESSAGE_MAP(CDatabaseNotificationsList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/**
*
*
*
*/
CDatabaseNotificationsList::CDatabaseNotificationsList()
{
	_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	_updateCache.reset(reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache")));

	if ((!_odb) || (!_updateCache))
		throw ::std::runtime_error("Database Not Available");

	// create require updates list
	::std::vector<::std::wstring> attr_list;
	attr_list.push_back(L"create");
	attr_list.push_back(L"delete");
	attr_list.push_back(L"*");
	// request updates for above list
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Vertex, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Interface, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Group, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Edge, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Flow, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Demand, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Path, attr_list);
}
/**
*
*
*
*/
CDatabaseNotificationsList::~CDatabaseNotificationsList()
{
}
/**
*
*
*
*/
void CDatabaseNotificationsList::DatabaseUpdate()
{
	size_t qSize = _updateQ->QueueSize();
	while (!_updateQ->Empty())
	{
		::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _updateQ->GetNextRecord();
		// get the UID, attrname and new value of the update
		::NM::ODB::real_uid objectUID = record->GetObjectUID();
		::NM::ODB::real_string attrName = record->GetObjectAttributeName();
		::NM::ODB::Value newValue = record->GetNewValue();

		::std::wstring strMessage;
		strMessage += attrName.GetString();
		strMessage += L"\t\t" + _odb->GetObjectTypeName(objectUID);
		::std::wstring strName = _odb->GetValue(objectUID, L"shortname")->Get<::NM::ODB::ODBWString>();
		strMessage += L"\t" + strName;

		if ((attrName.GetString() != L"create") && (attrName.GetString() != L"delete"))
		{
			strMessage += L"\tPrevious Value: " + record->GetPreviousValue().GetStringValue();
			strMessage += L"\tNew Value: " + record->GetNewValue().GetStringValue();
		}

		AddString(strMessage.c_str());
	}
	return;
}
/**
*
*
*
*/
void CDatabaseNotificationsList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}
/**
*
*
*
*/
void CDatabaseNotificationsList::OnEditCopy()
{
	MessageBox(_T("Copy output"));
}
/**
*
*
*
*/
void CDatabaseNotificationsList::OnEditClear()
{
	MessageBox(_T("Clear output"));
}
/**
*
*
*
*/
void CDatabaseNotificationsList::OnViewOutput()
{
	CBCGPDockingControlBar* pParentBar = DYNAMIC_DOWNCAST(CBCGPDockingControlBar, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		//pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}

#pragma endregion ClassCDatabaseNotificationsList