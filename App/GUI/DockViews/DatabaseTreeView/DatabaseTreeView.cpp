#include "stdafx.h"
#include "MainFrm.h"
#include "DatabaseTreeView.h"
#include "Resource.h"
#include "GuiTest3.h"
#include <string>
#include <algorithm>

#include "IServiceRegistry.h"								// registry interface
#include "ISelectedObjects.h"								// holds references to currently selected objects application wide
#include "Interfaces\IObjectDatabase.h"						// for database client interface 
#include "IAdjacencyMatrix.h"								// node/link adj matrix interface
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"	// for client interface reqd for client database update notifications
#include "INetworkView.h"


using ::NM::ODB::real_uid;
using ::NM::ODB::real_string;
using ::NM::ODB::Value;

extern NM::Registry::IServiceRegistry* reg;
//#define OutputDebugString

#pragma region DatabaseTreeViewMenuButton
/**
* CDatabaseTreeViewMenuButton
*
* Provides class for the menu bar above the tree control
*/
class CDatabaseTreeViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CDatabaseTreeView;

	DECLARE_SERIAL(CDatabaseTreeViewMenuButton)
	//DECLARE_DYNAMIC(CDatabaseTreeViewMenuButton)

public:
	CDatabaseTreeViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};
/**
*
*
*
*/
IMPLEMENT_SERIAL(CDatabaseTreeViewMenuButton, CMFCToolBarMenuButton, 1)
//IMPLEMENT_DYNAMIC(CDatabaseTreeViewMenuButton, CMFCToolBarMenuButton)
#pragma endregion DatabaseTreeViewMenuButton

/**
*
*
*
*/
CDatabaseTreeView::CDatabaseTreeView() :
	_bLoading(true),
	_nextCacheID(1),		// do not start at zero - clashes with update item on parent nodes
	_bUpdatingSelected(false), 
	_updateCache(nullptr),
	_bIgnoreSelected(false),
	_bScrollSelect(false),
	_bStickySelect(false),
	_bRedrawSet(false),
	_objectMapHighlighted(false),
	_hNewTimer(NULL)
{
	//m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}
/**
*
*
*
*/
CDatabaseTreeView::~CDatabaseTreeView()
{
	::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
	_selectedObjects->DeRegisterClientObserver(this);
}
/**
*
*ID_TREE_DELETE
*
*/
BEGIN_MESSAGE_MAP(CDatabaseTreeView, CBCGPDockingControlBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()	
	
	// hmmm think old toolbar buttons
	ON_COMMAND(ID_CLASS_ADD_MEMBER_FUNCTION, OnClassAddMemberFunction)
	ON_COMMAND(ID_CLASS_ADD_MEMBER_VARIABLE, OnClassAddMemberVariable)
	ON_COMMAND(ID_CLASS_DEFINITION, OnClassDefinition)
	ON_COMMAND(ID_CLASS_PROPERTIES, OnClassProperties)

	// tool bar buttons
	ON_COMMAND(ID_REFRESH_TREE, OnRefreshDataCache)
	
	ON_COMMAND(ID_SCROLL_SELECT, OnScrollSelect)
	ON_UPDATE_COMMAND_UI(ID_SCROLL_SELECT, OnUpdateScrollSelect)
	
	ON_COMMAND(ID_STICKY_SELECT, OnStickySelect)
	ON_UPDATE_COMMAND_UI(ID_STICKY_SELECT, OnUpdateStickySelect)
	
	ON_COMMAND(ID_TREE_DELETE, OnDeleteObject)
	ON_UPDATE_COMMAND_UI(ID_TREE_DELETE, OnUpdateDeleteObject)
	
	
	// context menu
	ON_COMMAND(ID_OBJECTMENU_DELETE, OnDeleteObject)
	ON_COMMAND(ID_OBJECTMENU_SHOWONMAP, OnShowObjectOnMap)
	ON_COMMAND(ID_OBJECTMENU_OBJECT_INSERVICE, OnToggleObjectInService)

	// context menu for white space and think for toolbar dynamic button
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)


END_MESSAGE_MAP()
/**
*
*
*
*/
int CDatabaseTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	_bLoading = true;

	if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	// TVS_DISABLEDRAGDROP disables dragdrop functionality but as a result WM_LBUTTONDOWN and WM_LBUTTONUP etc messages are enabled with this set.
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_DISABLEDRAGDROP; // | TVS_CHECKBOXES | TVS_SHOWSELALWAYS;
	if (!m_wndDatabaseTreeView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	long exStyle = GetWindowLong(m_wndDatabaseTreeView, GWL_EXSTYLE);
	::SendMessage(m_wndDatabaseTreeView, TVM_SETEXTENDEDSTYLE, TVS_EX_DOUBLEBUFFER, (exStyle | TVS_EX_DOUBLEBUFFER));
	if (!SetWindowSubclass(m_wndDatabaseTreeView, &CDatabaseTreeView::TreeViewWndProc, 2, (DWORD_PTR)this))
	{
		TRACE0("Failed to create SubClass for tree\n");
		return -1;      // fail to create
	}

	//CUSTOMCOLORS
	//// change text color to white and background to off black
	//m_wndDatabaseTreeView.SetTextColor(RGB(255, 255, 255));	
	//m_wndDatabaseTreeView.SetBkColor(RGB(0.2 * 255, 0.2 * 255, 0.2 * 255));




	// Load images: original
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* Is locked */);
	OnChangeVisualStyle();
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);			// All commands will be routed via this control , not via the parent frame:

	// copy from properties
	//m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	//m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	//OnChangeVisualStyle();
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* Locked */);

	// original 
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);			// All commands will be routed via this control , not via the parent frame:

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CDatabaseTreeViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));
	CDatabaseTreeViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CDatabaseTreeViewMenuButton, m_wndToolBar.GetButton(0));
	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}


	_adjm.reset(static_cast<::NM::DataServices::IAdjacencyMatrix*>(reg->GetClientInterface(L"ADJMATRIX")));
	assert(_adjm);

	// Initialise local caches from DB and build tree
	RefreshDataCache();
	
	_bLoading = false;
	// Register this class with Database cache updates etc
	RegisterClientNotify();

	return 0;
}
/**
*
*
*
*/
void CDatabaseTreeView::OnSize(UINT nType, int cx, int cy)
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);
	AdjustLayout();
}
/**
* OnContextMenu
*
* Popup menu for HTREEITEM 
*/
void CDatabaseTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndDatabaseTreeView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CBCGPDockingControlBar::OnContextMenu(pWnd, point);
		return;
	}

	
	

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		UINT flags = 0;
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);		
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);

		menu.DestroyMenu();

		if( (hTreeItem != NULL) && (flags & TVHT_ONITEM))
		{
			// select the item in the tree
			//pWndTree->SelectItem(hTreeItem);
			// is this a rclick on a item
			long treeItemID = static_cast<long>(pWndTree->GetItemData(hTreeItem));
			IDUIDMAP::iterator mapIt = id_to_uid_map.find(treeItemID);
			if( (mapIt != id_to_uid_map.end()) && (treeItemID > -1))
			{
				pWndTree->SetFocus();	
				menu.LoadMenuW(IDR_DBVIEW_OBJECT_POPUP);
				CMenu* submenu = menu.GetSubMenu(0);

				::std::vector<::NM::ODB::OBJECTUID> objectList;
				objectList = GetSelectedObjects();
				size_t enabledCount = 0;
				size_t disabledCount = 0;
				::std::vector<::NM::ODB::OBJECTUID>::iterator objectit = objectList.begin();
				while (objectit != objectList.end())
				{
					::std::wstring objectType = _odb->GetObjectTypeName(*objectit);
					if ((objectType == L"vertex") || (objectType == L"edge"))
					{
						if (_odb->GetValue(*objectit, L"inservice")->Get<::NM::ODB::ODBBool>() == true)
						{
							++enabledCount;
						}
						else
						{
							++disabledCount;
						}
					}
					++objectit;
				}


				if ((enabledCount == 0) && (disabledCount > 0))
				{				
					MENUITEMINFO menuItemInfo;
					::std::memset(&menuItemInfo, 0, sizeof(MENUITEMINFO));
					menuItemInfo.cbSize = sizeof(MENUITEMINFO);
					submenu = menu.GetSubMenu(0);
					submenu->AppendMenuW(MF_STRING, ID_OBJECTMENU_OBJECT_INSERVICE, L"Enable");				
					menuItemInfo.fMask = MIIM_DATA;
					menuItemInfo.dwItemData = (ULONG_PTR)TRUE;
					submenu->SetMenuItemInfoW(ID_OBJECTMENU_OBJECT_INSERVICE, &menuItemInfo);
				}
				else if ((enabledCount > 0) && (disabledCount == 0))
				{
					MENUITEMINFO menuItemInfo;
					::std::memset(&menuItemInfo, 0, sizeof(MENUITEMINFO));
					menuItemInfo.cbSize = sizeof(MENUITEMINFO);
					submenu = menu.GetSubMenu(0);
					submenu->AppendMenuW(MF_STRING, ID_OBJECTMENU_OBJECT_INSERVICE, L"Disable");
					menuItemInfo.fMask = MIIM_DATA;
					menuItemInfo.dwItemData = (ULONG_PTR)FALSE;
					submenu->SetMenuItemInfoW(ID_OBJECTMENU_OBJECT_INSERVICE, &menuItemInfo);
				}

				

			}
			/* its a parent table node, not an object*/
			else
			{
				menu.LoadMenu(IDR_TREE_ROOT);
			}
		}
		// whitespace
		else
		{
			menu.LoadMenu(IDR_POPUP_SORT);
		}
	}
	
	pWndTree->SetFocus();
	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}
/**
*
*
*
*/
void CDatabaseTreeView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndDatabaseTreeView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}
/**
*
*
*
*/
BOOL CDatabaseTreeView::PreTranslateMessage(MSG* pMsg)
{
	return CBCGPDockingControlBar::PreTranslateMessage(pMsg);
}
/**
*
*
*
*/
void CDatabaseTreeView::OnSort(UINT id)
{
	/*
	* MFC Generated Code
	*/
	//if (m_nCurrSort == id)
	//{
	//	return;
	//}

	//m_nCurrSort = id;

	//CDatabaseTreeViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CDatabaseTreeViewMenuButton, m_wndToolBar.GetButton(0));

	//if (pButton != NULL)
	//{
	//	pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
	//	m_wndToolBar.Invalidate();
	//	m_wndToolBar.UpdateWindow();
	//}
}
/**
*
*
*
*/
void CDatabaseTreeView::OnUpdateSort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}
/**
*
*
*
*/
void CDatabaseTreeView::OnClassAddMemberFunction()
{
	AfxMessageBox(_T("Add member function..."));
}
/**
*
*
*
*/
void CDatabaseTreeView::OnClassAddMemberVariable()
{
	// TODO: Add your command handler code here
}
/**
*
*
*
*/
void CDatabaseTreeView::OnClassDefinition()
{
	// TODO: Add your command handler code here
}
/**
*
*
*
*/
void CDatabaseTreeView::OnClassProperties()
{
	// TODO: Add your command handler code here
}
/**
*
*
*
*/
void CDatabaseTreeView::OnScrollSelect()
{
	_bScrollSelect = (_bStickySelect ? false : !_bScrollSelect);
}
/**
*
*
*
*/
void CDatabaseTreeView::OnUpdateScrollSelect(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(!_bScrollSelect);
}
/**
*
*
*
*/
void CDatabaseTreeView::OnStickySelect()
{
	_bStickySelect = !_bStickySelect;
	_bScrollSelect = (_bStickySelect ? false : _bScrollSelect);
}
/**
*
*
*
*/
void  CDatabaseTreeView::OnUpdateStickySelect(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(_bStickySelect);

}

/**
* OnRefreshDataCache (Actually Refresh Tree from Database)
*
* For Debug purposes only... Tree refreshes itself as changes are made to the db hence this being debug only.
*/
void CDatabaseTreeView::OnRefreshDataCache()
{
	DWORD ret = ::MessageBox(NULL, L"For Debug purposes only... Tree refreshes itself as changes are made to the db hence this being debug only.\n\nClean & Refresh Tree From Database?", L"DEBUG FUNCTION ONLY", MB_YESNO);
	if (ret == IDNO) return;

	m_wndDatabaseTreeView.DeleteAllItems();
	id_to_uid_map.clear();
	uid_to_htree.clear();
	_selectedHTreeItems.clear();
	_nextCacheID = 1;
	RefreshDataCache();
}
/**
*
*
*
*/
void CDatabaseTreeView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndDatabaseTreeView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}
/**
*
*
*
*/
void CDatabaseTreeView::OnSetFocus(CWnd* pOldWnd)
{
	CBCGPDockingControlBar::OnSetFocus(pOldWnd);

	m_wndDatabaseTreeView.SetFocus();
}
/**
*
*
*
*/
void CDatabaseTreeView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndDatabaseTreeView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* Locked */);
}
/**
* RefreshDataCache
*
* Initialise local caches from DB and build tree
*/
void CDatabaseTreeView::RefreshDataCache()
{
	//OutputDebugString(L"\nRefreshDataCache");
	_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	_selectedObjects.reset(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));

	/*
	*	Create Root Items
	*/
	//size_t newID = GetNewCacheID();
	HTREEITEM hNode = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT),  L"Nodes" , 0, 0, NULL, NULL, (LPARAM)-1, NULL, NULL);
	m_wndDatabaseTreeView.SetItemState(hNode, TVIS_BOLD, TVIS_BOLD);
	AddItemToCache(-1, ::NM::ODB::INVALID_OBJECT_UID, hNode);

	HTREEITEM hIntf = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), L"Interfaces", 0, 0, NULL, NULL, (LPARAM)-1, NULL, NULL);
	m_wndDatabaseTreeView.SetItemState(hIntf, TVIS_BOLD, TVIS_BOLD);
	AddItemToCache(-1, ::NM::ODB::INVALID_OBJECT_UID, hIntf);

	HTREEITEM hGroup = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), L"Groups", 0, 0, NULL, NULL, (LPARAM)-1, NULL, NULL);
	m_wndDatabaseTreeView.SetItemState(hGroup, TVIS_BOLD, TVIS_BOLD);
	AddItemToCache(-1, ::NM::ODB::INVALID_OBJECT_UID, hGroup);

	//newID = GetNewCacheID();
	HTREEITEM hLink = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), L"Links", 0, 0, NULL, NULL, (LPARAM)-2, NULL, NULL);
	m_wndDatabaseTreeView.SetItemState(hLink, TVIS_BOLD, TVIS_BOLD);
	AddItemToCache(-2, ::NM::ODB::INVALID_OBJECT_UID, hLink);


	//newID = GetNewCacheID();
	HTREEITEM hFlow = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), L"Flows & Demands", 0, 0, NULL, NULL, (LPARAM)-3, NULL, NULL);
	m_wndDatabaseTreeView.SetItemState(hFlow, TVIS_BOLD, TVIS_BOLD);
	AddItemToCache(-3, ::NM::ODB::INVALID_OBJECT_UID, hFlow);

	//newID = GetNewCacheID();
	HTREEITEM hPath = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), L"Network Paths", 0, 0, NULL, NULL, (LPARAM)-4, NULL, NULL);
	m_wndDatabaseTreeView.SetItemState(hPath, TVIS_BOLD, TVIS_BOLD);
	AddItemToCache(-4, ::NM::ODB::INVALID_OBJECT_UID, hPath);


	/*
	* Create Items from DB Tables
	*/
	//NM::ODB::IAdjacencyMatrix adj = _odb->GetAdjacencyMatrix();
	::std::vector<NM::ODB::OBJECTUID> edgeList;
	::std::wstring text;

	// vertex
	::std::wstring table = L"vertextable";
	NM::ODB::OBJECTUID objectUID = _odb->GetFirstObject(table);
	while (objectUID != NM::ODB::INVALID_OBJECT_UID)
	{
		_adjm->GetVertexEdges(objectUID, edgeList);
		text = _odb->GetValue(objectUID, L"shortname")->Get<NM::ODB::ODBWString>();
		int imageid = (_odb->GetValue(objectUID, L"inservice")->Get<NM::ODB::ODBBool>() ? 7 : 8);

		size_t newID = GetNewCacheID();
		HTREEITEM hItem = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), text.c_str(), imageid, imageid, NULL, NULL, (LPARAM)newID, hNode, NULL);
		AddItemToCache(newID, objectUID, hItem);

		/*for each(auto edgeUID in edgeList)
		{
		::std::wcout << L"\t" << _odb->GetValue(edgeUID, L"shortname")->Get<NM::ODB::ODBWString>() << ::std::endl;
		}*/
		objectUID = _odb->GetNextObject(objectUID);
		//edgeList.clear();
	}

	// interfaces
	table = L"interfacetable";
	objectUID = _odb->GetFirstObject(table);
	while (objectUID != NM::ODB::INVALID_OBJECT_UID)
	{
		::NM::ODB::OBJECTUID vertexUID = _odb->GetValue(objectUID, L"vertexUID")->Get<::NM::ODB::ODBUID>();

		text = _odb->GetValue(vertexUID, L"shortname")->Get<NM::ODB::ODBWString>();
		text += L"::";
		text += _odb->GetValue(objectUID, L"shortname")->Get<NM::ODB::ODBWString>();
		int imageid = 3; //(_odb->GetValue(objectUID, L"inservice")->Get<NM::ODB::ODBBool>() ? 7 : 8);

		size_t newID = GetNewCacheID();
		HTREEITEM hItem = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), text.c_str(), imageid, imageid, NULL, NULL, (LPARAM)newID, hIntf, NULL);
		AddItemToCache(newID, objectUID, hItem);
		objectUID = _odb->GetNextObject(objectUID);
	}



	CreateGroupTree(hGroup);


	// edges
	table = L"edgetable";
	objectUID = _odb->GetFirstObject(table);
	while (objectUID != NM::ODB::INVALID_OBJECT_UID)
	{
		text = _odb->GetValue(objectUID, L"shortname")->Get<NM::ODB::ODBWString>();
		int imageid = (_odb->GetValue(objectUID, L"inservice")->Get<NM::ODB::ODBBool>() ? 7 : 8);

		size_t newID = GetNewCacheID();
		HTREEITEM hItem = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), text.c_str(), imageid, imageid, NULL, NULL, (LPARAM)newID, hLink, NULL);
		AddItemToCache(newID, objectUID, hItem);
		objectUID = _odb->GetNextObject(objectUID);

	}


	// flows
	table = L"flowtable";
	objectUID = _odb->GetFirstObject(table);
	while (objectUID != NM::ODB::INVALID_OBJECT_UID)
	{
		text = _odb->GetValue(objectUID, L"shortname")->Get<NM::ODB::ODBWString>();

		size_t newID = GetNewCacheID();
		HTREEITEM hItem = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), text.c_str(), 3, 3, NULL, NULL, (LPARAM)newID, hFlow, NULL);
		AddItemToCache(newID, objectUID, hItem);
		objectUID = _odb->GetNextObject(objectUID);

	}

	// demands, as each demand MUST have a flow parent, each demand gets added as a child to the parent flow HTREEITEM
	table = L"demandtable";
	objectUID = _odb->GetFirstObject(table);
	while (objectUID != NM::ODB::INVALID_OBJECT_UID)
	{
		text = _odb->GetValue(objectUID, L"shortname")->Get<NM::ODB::ODBWString>();

		::NM::ODB::ODBUID demandParent = _odb->GetValue(objectUID, L"flowuid")->Get<NM::ODB::ODBUID>();
		if (demandParent == ::NM::ODB::INVALID_OBJECT_UID)
		{
			continue;
		}

		// lookup flow HTREEITEM 
		UIDHTREE::iterator flowIT = uid_to_htree.find(demandParent);
		if (flowIT == uid_to_htree.end())
		{
			continue;
		}

		size_t newID = GetNewCacheID();
		HTREEITEM hItem = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), text.c_str(), 3, 3, NULL, NULL, (LPARAM)newID, flowIT->second, NULL);
		AddItemToCache(newID, objectUID, hItem);
		objectUID = _odb->GetNextObject(objectUID);

	}

	// paths
	table = L"pathtable";
	objectUID = _odb->GetFirstObject(table);
	while (objectUID != NM::ODB::INVALID_OBJECT_UID)
	{
		text = _odb->GetValue(objectUID, L"shortname")->Get<NM::ODB::ODBWString>();

		size_t newID = GetNewCacheID();
		HTREEITEM hItem = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), text.c_str(), 3, 3, NULL, NULL, (LPARAM)newID, hPath, NULL);
		AddItemToCache(newID, objectUID, hItem);
		objectUID = _odb->GetNextObject(objectUID);

	}

	m_wndDatabaseTreeView.SortChildren(hNode);
	m_wndDatabaseTreeView.SortChildren(hIntf);
	m_wndDatabaseTreeView.SortChildren(hLink);
	m_wndDatabaseTreeView.SortChildren(hFlow);
	m_wndDatabaseTreeView.SortChildren(hPath);
	m_wndDatabaseTreeView.Expand(hNode, TVE_EXPAND);
	if (_bLoading)
	{
		//SelectItem(m_wndDatabaseTreeView.GetChildItem(hNode));
		//m_wndDatabaseTreeView.EnsureVisible(hNode);
	}
		
	return;
}
/**
* CreateGroupTree
*
* Create the group tree hierarcy
*/
void CDatabaseTreeView::CreateGroupTree(HTREEITEM groupRoot, ::NM::ODB::OBJECTUID parentGroupUID)
{
	// groups
	::std::wstring table = L"grouptable";
	::std::wstring text;
	::NM::ODB::OBJECTUID objectUID = _odb->GetFirstObject(table);
	::std::map<::NM::ODB::OBJECTUID, ::NM::ODB::OBJECTUID, compareGuid> uid_to_parent;
	::std::map<::NM::ODB::OBJECTUID, HTREEITEM, compareGuid> group_to_htreeitem;

	while (objectUID != NM::ODB::INVALID_OBJECT_UID)
	{
		::NM::ODB::OBJECTUID parent = _odb->GetValue(objectUID, L"group")->Get<NM::ODB::ODBUID>();
		// if we find objects with invalid group which means they are at the global, root level, add these now
		if (parent == parentGroupUID)
		{
			text = _odb->GetValue(objectUID, L"shortname")->Get<NM::ODB::ODBWString>();
			size_t newID = GetNewCacheID();
			HTREEITEM hItem = m_wndDatabaseTreeView.InsertItem((TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT), text.c_str(), 1, 1, NULL, NULL, (LPARAM)newID, groupRoot, NULL);
			AddItemToCache(newID, objectUID, hItem);
			CreateGroupTree(hItem, objectUID);
		}
		objectUID = _odb->GetNextObject(objectUID);
	}
	
	return;
}
/**
*
*
*
*/
LRESULT CDatabaseTreeView::OnTreeViewLeftButtonDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	OutputDebugString(L"\nOnTreeViewLeftButtonDblClick::Select");

	return 1;

	/**********************************************************************************/
	TVHITTESTINFO ht = { 0 };
	bool isCurrentlySelected = false;
	bool isControlKeyDown = ((GetKeyState(VK_CONTROL) < 0) || _bStickySelect);
	bool isShiftKeyDown = (GetKeyState(VK_SHIFT) < 0);

	DWORD dwpos = GetMessagePos();
	ht.pt.x = GET_X_LPARAM(dwpos);
	ht.pt.y = GET_Y_LPARAM(dwpos);
	::MapWindowPoints(HWND_DESKTOP, m_wndDatabaseTreeView, &ht.pt, 1);

	// if we are currently highlighting on map, remove highlight
	if (_objectMapHighlighted)
	{
		// calling this will set spotlightoff
		OnShowObjectOnMap();
	}

	// if treeitem clicked 
	HTREEITEM hTreeItem = TreeView_HitTest(m_wndDatabaseTreeView, &ht);
	if (hTreeItem == NULL)
		return 1;

	if (!(TVHT_ONITEM & ht.flags))
		return 1;

	// select this item only, single select on dbl click
	SelectItem(hTreeItem, false);

	::std::vector<::NM::ODB::OBJECTUID> selected = GetSelectedObjects();

	// if size !=1 then the selection is invalid, such as a root node
	if (selected.size() == 1)
	{
		::std::wstring objecttypename = _odb->GetObjectTypeName(selected[0]);
		if (objecttypename == L"group")
		{
			::NM::NetGraph::INetworkView* pView = static_cast<::NM::NetGraph::INetworkView*>(reg->GetClientInterface(L"NETWORKVIEW"));
			pView->SetGroupView(selected[0]);
			return 0;
		}
	}


	return 1;
}
/**
* OnTreeViewLeftClick
*
* Clicked on the tree somewhere
*/
void CDatabaseTreeView::OnTreeViewLeftButtonDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	//OutputDebugString(L"\nOnTreeViewLeftClick");
	//NMTREEVIEW* nmTreeViewStruct = (NMTREEVIEW*)pNMHDR;
	TVHITTESTINFO ht = { 0 };
	bool isCurrentlySelected = false;
	bool isControlKeyDown = ((GetKeyState(VK_CONTROL) < 0) || _bStickySelect);
	bool isShiftKeyDown = (GetKeyState(VK_SHIFT) < 0);

	DWORD dwpos = GetMessagePos();
	ht.pt.x = GET_X_LPARAM(dwpos);
	ht.pt.y = GET_Y_LPARAM(dwpos);
	::MapWindowPoints(HWND_DESKTOP, m_wndDatabaseTreeView, &ht.pt, 1);

	// if we are currently highlighting on map, remove highlight
	if (_objectMapHighlighted)
	{
		// calling this will set spotlightoff
		OnShowObjectOnMap();
	}

	// if treeitem clicked 
	HTREEITEM hTreeItem = TreeView_HitTest(m_wndDatabaseTreeView, &ht);
	if (hTreeItem == NULL) return;

	if (!(TVHT_ONITEM & ht.flags))
		return;


	// act depending on current selected state
	isCurrentlySelected = IsItemSelected(hTreeItem);

	if ((isShiftKeyDown) && _selectedHTreeItems.size() != 0)
	{
		// get last selected with new selection
		SelectRange(GetLastSelectedItem(), hTreeItem);
	}
	else if (isCurrentlySelected & isControlKeyDown)
	{
		// deselect
		//OutputDebugString(L"\tDeselect one item");
		DeSelectItem(hTreeItem);
	}
	else if (!isCurrentlySelected & isControlKeyDown)
	{
		// add to current selection
		//OutputDebugString(L"\tAdd item to current selection");
		SelectItem(hTreeItem, true);
	}
	else if (!isCurrentlySelected & !isControlKeyDown)
	{
		//OutputDebugString(L"\tSelect only this item");
		SelectItem(hTreeItem, false);

	}
	else if (isCurrentlySelected & !isControlKeyDown)
	{
		//OutputDebugString(L"\tSelect only this item");
		if (GetValidSelectedCount() > 1)
		{
			SelectItem(hTreeItem, false);
		}
	}

	// Set Timer Event to show object on map
	//assert(_hNewTimer == NULL);
	//_hNewTimer = CreateTimerQueue();

	BOOL bSet = CreateTimerQueueTimer(
		&_hNewTimer,
		NULL,
		(WAITORTIMERCALLBACK)WaitOrTimerCallback,
		static_cast<PVOID>(this),
		500,
		0,
		WT_EXECUTEONLYONCE | WT_EXECUTEINTIMERTHREAD);

	assert(bSet == TRUE);

	//::SetCapture(m_wndDatabaseTreeView.GetSafeHwnd());
	
	return;
}
/**
*
*
*
*/
VOID CALLBACK CDatabaseTreeView::WaitOrTimerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{

	static_cast<CDatabaseTreeView*>(lpParameter)->OnShowObjectOnMap();
	//DeleteTimerQueueTimer(NULL, static_cast<CDatabaseTreeView*>(lpParameter)->_hNewTimer, NULL);
	//static_cast<CDatabaseTreeView*>(lpParameter)->_hNewTimer = NULL;
}

/**
*
*
*
*/
void CDatabaseTreeView::OnTreeViewRightButtonDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	TVHITTESTINFO ht = { 0 };
	bool isCurrentlySelected = false;
	bool isControlKeyDown = ((GetKeyState(VK_CONTROL) < 0) || _bStickySelect);
	bool isShiftKeyDown = (GetKeyState(VK_SHIFT) < 0);

	DWORD dwpos = GetMessagePos();
	ht.pt.x = GET_X_LPARAM(dwpos);
	ht.pt.y = GET_Y_LPARAM(dwpos);
	::MapWindowPoints(HWND_DESKTOP, m_wndDatabaseTreeView, &ht.pt, 1);

	// if we are currently highlighting on map, remove highlight
	if (_objectMapHighlighted)
	{
		// calling this will set spotlightoff
		OnShowObjectOnMap();
	}

	HTREEITEM hTreeItem = TreeView_HitTest(m_wndDatabaseTreeView, &ht);
	if (hTreeItem == NULL) return;

	isCurrentlySelected = IsItemSelected(hTreeItem);

	if (TVHT_ONITEM & ht.flags)
	{
		if (isCurrentlySelected && !isControlKeyDown)
		{
			return;
		}
		else
		{
			SelectItem(hTreeItem, (isControlKeyDown ? true : false));
		}
	}
		
	return;
}
/**
*
*
*
*/
void CDatabaseTreeView::OnTreeViewKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

	switch (pTVKeyDown->wVKey)
	{
		case VK_DOWN:
		{
			HTREEITEM selectedItem = GetLastSelectedItem();
			if (selectedItem == NULL)
			{
				return;
			}
			HTREEITEM nextItem = GetNextHTreeItem(selectedItem, TVGN_NEXT);
			SelectItem(nextItem);
		}
		break;

		case VK_UP:
		{
			HTREEITEM selectedItem = GetLastSelectedItem();
			if (selectedItem == NULL)
			{
				return;
			}
			HTREEITEM nextItem = GetNextHTreeItem(selectedItem, TVGN_PREVIOUS);
			SelectItem(nextItem);
		}
		break;

		case VK_RIGHT:
		{
			HTREEITEM selectedItem = GetLastSelectedItem();
			if (m_wndDatabaseTreeView.ItemHasChildren(selectedItem))
			{
				m_wndDatabaseTreeView.Expand(selectedItem, TVE_EXPAND);
			}
		}
		break;

		case VK_LEFT:
		{
			HTREEITEM selectedItem = GetLastSelectedItem();
			if (m_wndDatabaseTreeView.ItemHasChildren(selectedItem))
			{
				m_wndDatabaseTreeView.Expand(selectedItem, TVE_COLLAPSE);
			}
		}
		break;

		case SB_PAGEDOWN:
		case VK_NEXT:
		{
			if (_bScrollSelect)
			{
				HTREEITEM selectedItem = GetLastSelectedItem();
				if (m_wndDatabaseTreeView.GetLastVisibleItem() == selectedItem)
					break;

				RECT rect = { 0 };
				m_wndDatabaseTreeView.GetItemRect(selectedItem, &rect, FALSE);
				_bIgnoreSelected = true;
				m_wndDatabaseTreeView.RedrawWindow(&rect);
				::SendMessage(m_wndDatabaseTreeView, WM_VSCROLL, SB_PAGEDOWN, 0);
				HTREEITEM lastVisibleItem = m_wndDatabaseTreeView.GetFirstVisibleItem();
				UINT maxItemCount = m_wndDatabaseTreeView.GetVisibleCount();
				for (UINT i = 0; i < maxItemCount+1; ++i)
				{
					lastVisibleItem = GetNextHTreeItem(lastVisibleItem, TVGN_NEXT);
				}
				_bIgnoreSelected = false;
				SelectItem((lastVisibleItem != NULL ? lastVisibleItem : m_wndDatabaseTreeView.GetLastVisibleItem()), false);
			}
			else
			{
				::SendMessage(m_wndDatabaseTreeView, WM_VSCROLL, SB_PAGEDOWN, 0);
			}
			ReDrawSelected();
		}
		break;

		case SB_PAGEUP:
		case VK_PRIOR:
		{
			if (_bScrollSelect)
			{
				HTREEITEM selectedItem = GetLastSelectedItem();
				if (selectedItem == m_wndDatabaseTreeView.GetRootItem())
					break;
				RECT rect = { 0 };
				m_wndDatabaseTreeView.GetItemRect(selectedItem, &rect, FALSE);
				_bIgnoreSelected = true;
				m_wndDatabaseTreeView.RedrawWindow(&rect);
				::SendMessage(m_wndDatabaseTreeView, WM_VSCROLL, SB_PAGEUP, 0);
				_bIgnoreSelected = false;
				SelectItem(m_wndDatabaseTreeView.GetFirstVisibleItem(), false);
				
			}
			else
			{
				::SendMessage(m_wndDatabaseTreeView, WM_VSCROLL, SB_PAGEUP, 0);
			}
			ReDrawSelected();
		}
		break;
	}


	*pResult = 0;

}
/**
*
*
* As we dont have a function to test if a particular item is in view, 
* we refresh the selected status of all after a scroll event, control
* seems to lose the paint created by custom draw during a scroll event.
*/
void CDatabaseTreeView::ReDrawSelected()
{
	for each(HTREEITEM item in _selectedHTreeItems)
	{
		RECT rect = { 0 };
		m_wndDatabaseTreeView.GetItemRect(item, &rect, FALSE);
		m_wndDatabaseTreeView.RedrawWindow(&rect);
	}
	return;
}
/**
*
*
*
*/
HTREEITEM CDatabaseTreeView::GetNextHTreeItem(HTREEITEM hTreeItem, UINT code)
{
	UINT nCode = (code == TVGN_NEXT ? TVGN_NEXTVISIBLE : TVGN_PREVIOUSVISIBLE);

	// check we are not at the root
	HTREEITEM rootItem = m_wndDatabaseTreeView.GetRootItem();
	//if (hTreeItem == rootItem) return rootItem;

	// see if there is a sibling for this hTreeItem
	HTREEITEM nextItem = m_wndDatabaseTreeView.GetNextItem(hTreeItem, nCode);
	// if no sibling, then go to parent of hTreeItem, get its sibling (next/prev)
	if (nextItem == NULL)
	{
		HTREEITEM hTreeItemParent = m_wndDatabaseTreeView.GetParentItem(hTreeItem);
		if (hTreeItemParent == rootItem)  // root of tree
			return rootItem;

		if (hTreeItemParent == NULL)		// a top level node, no parents, does not return root for other items on the top of tree 
			return hTreeItem;

		// return the items parent as we are going up the tree
		if (code == TVGN_PREVIOUS) return hTreeItemParent;

		// return the items parents next sibling as we are going down the tree
		HTREEITEM hTreeItemParentSibling = m_wndDatabaseTreeView.GetNextItem(hTreeItemParent, nCode);
		if (hTreeItemParentSibling == NULL)			// end of tree, last leaf
			return hTreeItem;
	}

	return nextItem;
}

/**
*
*
*
*/
void CDatabaseTreeView::SetUpdatingSelected(bool bUpdating)
{
	if (_bUpdatingSelected && !bUpdating)
	{
		_bUpdatingSelected = bUpdating;
		UpdateGlobalSelect();
	}
	else
	{
		_bUpdatingSelected = bUpdating;
	}
	return;	
}
/**
*
*
* 
*/
void CDatabaseTreeView::UpdateGlobalSelect()
{
	if (_bUpdatingSelected) return;

	::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));

	::std::vector<::NM::ODB::ODBUID> selectedUID;
	GetValidSelectedUID(selectedUID);
	_selectedObjects->Select(selectedUID);
}
/**
*
*
*
*/
void CDatabaseTreeView::SelectItem(HTREEITEM hTreeItem, bool bAdditive)
{
	assert(hTreeItem != NULL);

	// clear all selected if not control pressed or asked to aggregate
	if ((!bAdditive) && (!_bStickySelect))
		ClearSelected();

	// for each new selected
	SELECTEDVEC::iterator it = ::std::find(_selectedHTreeItems.begin(), _selectedHTreeItems.end(), hTreeItem);
	if (it == _selectedHTreeItems.end())
	{
		_selectedHTreeItems.push_back(hTreeItem);
		RECT rect = { 0 };
		m_wndDatabaseTreeView.GetItemRect(hTreeItem, &rect, FALSE);
		m_wndDatabaseTreeView.InvalidateRect(&rect);
	//	m_wndDatabaseTreeView.RedrawWindow(&rect);
		m_wndDatabaseTreeView.EnsureVisible(hTreeItem);
	}

	//m_wndDatabaseTreeView.Invalidate();
	m_wndDatabaseTreeView.UpdateWindow();
	// update the global select list
	UpdateGlobalSelect();

	return;
}
/**
*
*
*
*/
void CDatabaseTreeView::DeSelectItem(HTREEITEM hTreeItem) 
{
	assert(hTreeItem != NULL);

	SELECTEDVEC::iterator it = ::std::find(_selectedHTreeItems.begin(), _selectedHTreeItems.end(), hTreeItem);
	if (it != _selectedHTreeItems.end())
	{
		RECT rect = { 0 };
		m_wndDatabaseTreeView.GetItemRect(*it, &rect, FALSE);
		_selectedHTreeItems.erase(it);
		//m_wndDatabaseTreeView.RedrawWindow(&rect);
		m_wndDatabaseTreeView.InvalidateRect(&rect);
		// update the global select list
		UpdateGlobalSelect();
	}
	return;
}
/**
*
*
*
*/
void CDatabaseTreeView::ClearSelected()
{
	//	DeSelectItem(item);
	SELECTEDVEC::iterator it = _selectedHTreeItems.begin();
	while (it != _selectedHTreeItems.end())
	{
		RECT rect = { 0 };
		m_wndDatabaseTreeView.GetItemRect(*it, &rect, FALSE);
		m_wndDatabaseTreeView.InvalidateRect(&rect);			// invalidate rect ???? 
		++it;
	}
	_selectedHTreeItems.clear();
	return;
}
/**
*
*
*
*/
bool CDatabaseTreeView::IsItemSelected(HTREEITEM hTreeItem)
{
	if (hTreeItem == NULL) return false;

	SELECTEDVEC::iterator it = ::std::find(_selectedHTreeItems.begin(), _selectedHTreeItems.end(), hTreeItem);
	return (it != _selectedHTreeItems.end());
}
/**
*
*
* Counts all the valid (non root nodes) selected tree items
*/
size_t CDatabaseTreeView::GetValidSelectedCount()
{
	size_t count = 0;
	UIDHTREE::iterator uidit = uid_to_htree.begin();
	SELECTEDVEC::iterator selectit = _selectedHTreeItems.begin();
	while (selectit != _selectedHTreeItems.end())
	{
		uidit = uid_to_htree.begin();
		while (uidit != uid_to_htree.end())
		{
			if ((uidit->second == *selectit) && (uidit->first != ::NM::ODB::INVALID_OBJECT_UID))
			{
				++count;
			}
			++uidit;
		}
		++selectit;
	}
	return count;
}
/**
*
*
*
*/
::std::vector<::NM::ODB::OBJECTUID> CDatabaseTreeView::GetSelectedObjects()
{
	::std::vector<::NM::ODB::OBJECTUID> selectedObjectList;
	UIDHTREE::iterator uidit = uid_to_htree.begin();
	SELECTEDVEC::iterator selectit = _selectedHTreeItems.begin();
	while (selectit != _selectedHTreeItems.end())
	{
		uidit = uid_to_htree.begin();
		while (uidit != uid_to_htree.end())
		{
			if ((uidit->second == *selectit) && (uidit->first != ::NM::ODB::INVALID_OBJECT_UID))
			{
				selectedObjectList.push_back(uidit->first);
			}
			++uidit;
		}
		++selectit;
	}
	return selectedObjectList;
}
/**
*
*
*
*/
size_t CDatabaseTreeView::GetValidSelectedUID(::std::vector<::NM::ODB::ODBUID> &vecSelected)
{
	vecSelected.clear();
	size_t count = 0;
	UIDHTREE::iterator uidit = uid_to_htree.begin();
	SELECTEDVEC::iterator selectit = _selectedHTreeItems.begin();
	while (selectit != _selectedHTreeItems.end())
	{
		uidit = uid_to_htree.begin();
		while (uidit != uid_to_htree.end())
		{
			if ((uidit->second == *selectit) && (uidit->first != ::NM::ODB::INVALID_OBJECT_UID))
			{
				++count;
				vecSelected.push_back(uidit->first);
			}
			++uidit;
		}
		++selectit;
	}
	return count;
}
/**
*
*
*
*/
size_t CDatabaseTreeView::GetValidSelectedHTreeItems(SELECTEDVEC &vecSelected)
{
	vecSelected.clear();
	size_t count = 0;
	UIDHTREE::iterator uidit = uid_to_htree.begin();
	SELECTEDVEC::iterator selectit = _selectedHTreeItems.begin();
	while (selectit != _selectedHTreeItems.end())
	{
		uidit = uid_to_htree.begin();
		while (uidit != uid_to_htree.end())
		{
			if ((uidit->second == *selectit) && (uidit->first != ::NM::ODB::INVALID_OBJECT_UID))
			{
				++count;
				vecSelected.push_back(uidit->second);
			}
			++uidit;
		}
		++selectit;
	}
	return count;
}
/**
*
*
*
*/
HTREEITEM CDatabaseTreeView::GetLastSelectedItem()
{
	size_t size = _selectedHTreeItems.size();
	if (size > 0)
	{
		return _selectedHTreeItems[size - 1];
	}
	return NULL;
}
/**
*
*
*
*/
void CDatabaseTreeView::SelectRange(HTREEITEM firstItem, HTREEITEM lastItem)
{
	HTREEITEM newSelectedhTreeItem = m_wndDatabaseTreeView.GetParentItem(firstItem);
	HTREEITEM lastSelectedhTreeItem = m_wndDatabaseTreeView.GetParentItem(lastItem);
	// check both last selected and this selected have same root
	if (lastSelectedhTreeItem != newSelectedhTreeItem) return;
	// iterate through this branch until either first or last is found,
	// then note all passed until other one is found, 
	// then selectitem on each found
	size_t nodesFound = 0;

	_bRedrawSet = true;
	SetUpdatingSelected(true);
	SelectItem(firstItem, true);
	HTREEITEM rootChildhTreeItem = m_wndDatabaseTreeView.GetChildItem(lastSelectedhTreeItem);
	while (rootChildhTreeItem != NULL)
	{
		if ((rootChildhTreeItem == firstItem) || (rootChildhTreeItem == lastItem))
		{
			++nodesFound;
		}

		if (nodesFound > 0)
		{
			SelectItem(rootChildhTreeItem, true);
		}

		if (nodesFound == 2)
		{
			break;
		}
		rootChildhTreeItem = m_wndDatabaseTreeView.GetNextItem(rootChildhTreeItem, TVGN_NEXT);
	}

	SelectItem(lastItem, true);
	_bRedrawSet = false;
	SetUpdatingSelected(false);
	return;
}
/**
*
*
*
*/
void CDatabaseTreeView::OnTreeViewSelectionChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = TRUE;
	return;
}
/**
*
*
*
*/
void CDatabaseTreeView::OnTreeViewSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	assert(false);
}
/**
* SelectedObjectsUpdate
*
* Called from SelectedObjects Service (override from baseclass)
*/
void CDatabaseTreeView::SelectedObjectsUpdate()
{
	// if we are currently highlighting on map, remove highlight
	if (_objectMapHighlighted)
	{
		// calling this will set spotlightoff
		OnShowObjectOnMap();
	}


	// set this directly as we dont want to cause an update after calling false on SetUpdatingSelected
	// and will cause an infinite loop
	_bUpdatingSelected = true;

	// get the selected service 'selected' objects
	::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
	::NM::OBJECTVECTOR svcSelectedObjects;
	size_t selectedCount = _selectedObjects->GetSelected(svcSelectedObjects);

	// get a list of our valid objects from our local cache
	::std::vector<::NM::ODB::ODBUID> localCacheSelected;
	GetValidSelectedUID(localCacheSelected);

	// compare the svcObjects with localcache for any new selected objects
	::NM::OBJECTVECTOR::iterator vecSvcSelectedit = svcSelectedObjects.begin();
	while (vecSvcSelectedit != svcSelectedObjects.end())
	{
		::std::vector<::NM::ODB::ODBUID>::iterator localCacheit = ::std::find(localCacheSelected.begin(), localCacheSelected.end(), *vecSvcSelectedit);
		// if not found, the this is a new selection we need to add.
		if (localCacheit == localCacheSelected.end())
		{
			// get the HTREEITEM of the UID
			UIDHTREE::iterator hTreeItemit = uid_to_htree.find(*vecSvcSelectedit);
			if (hTreeItemit != uid_to_htree.end())
			{
				// add to our local cache
				SelectItem(hTreeItemit->second, true);
			}
		}
		++vecSvcSelectedit;
	}


	// compare our local cache with svcSelected to see if any require deselecting
	::std::vector<::NM::ODB::ODBUID>::iterator localCacheit = localCacheSelected.begin();
	while (localCacheit != localCacheSelected.end())
	{
		::NM::OBJECTVECTOR::iterator svcSelectedit = ::std::find(svcSelectedObjects.begin(), svcSelectedObjects.end(), *localCacheit);
		// if local cache object not found in svcSelectedObjects so deselect item from tree
		if (svcSelectedit == svcSelectedObjects.end())
		{			
			// get the HTREEITEM of the UID
			UIDHTREE::iterator hTreeItemit = uid_to_htree.find(*localCacheit);
			if (hTreeItemit != uid_to_htree.end())
			{
				// add to our local cache
				DeSelectItem(hTreeItemit->second);
			}
		}
		++localCacheit;
	}

	_bUpdatingSelected = false;

	return;
}
/**
* MultiSelectChange
*
* Called from SelectedObjects Service (override from baseclass)
*/
void CDatabaseTreeView::MultiSelectChange(bool bMultiSelect)
{
}
/**
* GetNewCacheID
*
* Returns a new cache ID which is bound to an HTREEITEM item
*/
long CDatabaseTreeView::GetNewCacheID()
{
	return _nextCacheID++;
}
/**
* AddItemToCache
*
* Adds HTREEITEM, ODBUID, local cache id to local caches
*/
void CDatabaseTreeView::AddItemToCache(long id, ::NM::ODB::OBJECTUID objectUID, HTREEITEM hItem)
{
	id_to_uid_map[id] = objectUID;
	uid_to_htree[objectUID] = hItem;
}
/**
* OnCustomDraw
*
* Provides logic for mutiple selections
*/
void CDatabaseTreeView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVCUSTOMDRAW *pcd = (NMTVCUSTOMDRAW*)pNMHDR;
	switch (pcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
		{
			SELECTEDVEC::iterator foundIT = ::std::find(_selectedHTreeItems.begin(), _selectedHTreeItems.end(), (HTREEITEM)pcd->nmcd.dwItemSpec);
			
			if ((foundIT != _selectedHTreeItems.end()) && !_bIgnoreSelected)
			{
				pcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
				pcd->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
			}
			else
			{
				pcd->clrText = GetSysColor(COLOR_WINDOWTEXT);
				pcd->clrTextBk = RGB(255,255,255);

				//CUSTOMCOLORS
				/*pcd->clrText = RGB(255, 255, 255);
				pcd->clrTextBk = RGB(0.2 * 255, 0.2 * 255,  0.2 * 255 );*/
			}
			*pResult = CDRF_DODEFAULT;// do not set *pResult = CDRF_SKIPDEFAULT
			break;
		}
	}
	return;
}
/**
* DatabaseUpdate
*
* Called from DatabaseUpdateCache for notifications requested
*/
void CDatabaseTreeView::DatabaseUpdate()
{
	size_t qSize = _updateQ->QueueSize();
	while (!_updateQ->Empty())
	{
		::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _updateQ->GetNextRecord();
		// get the UID, attrname and new value of the update
		real_uid uid = record->GetObjectUID();
		real_string attrName = record->GetObjectAttributeName();
		Value newValue = record->GetNewValue();

		// find the UID in out local cache
		UIDHTREE::iterator uidHtreeIt = uid_to_htree.find(uid);
		if (uidHtreeIt == uid_to_htree.end())
			continue;

		// get the HTREEITEM for the UID
		HTREEITEM hTreeItem = uidHtreeIt->second;

		/*
		*		** WARNING THIS ASSUMES THAT ITS A SHORTNAME SO WE NEED TO EXPAND POST TESTING.....
		*/
		if (attrName.GetString() == L"shortname")
		{
			// Update the HTREEITEM text with the new attribute 
			real_string strNewValue = newValue.Get<::NM::ODB::ODBWString>();
			m_wndDatabaseTreeView.SetItemText(hTreeItem, strNewValue.GetString().c_str());
		}
		else if (attrName.GetString() == L"inservice")
		{
			// Update the HTREEITEM text with the new attribute 
			int imageid = (newValue.Get<::NM::ODB::ODBBool>() ? 7 : 8);
			m_wndDatabaseTreeView.SetItemImage(hTreeItem, imageid, imageid);
		}
		/*
		* Object Has Been Deleted
		*/
		else if (attrName.GetString() == L"delete")
		{	
			// get id from htree and clear id_to_uid_map
#pragma message("CDatabaseTreeView::DatabaseUpdate:: get id from htree and clear id_to_uid_map")
			m_wndDatabaseTreeView.DeleteItem(hTreeItem);
			uid_to_htree.erase(uidHtreeIt);
		}
		//else if (attrName.GetString() == L"create")
		else
		{
			throw ::std::logic_error("code update required.");
		}
				
		
	/*	HTREEITEM hTreeRootItem = m_wndDatabaseTreeView.GetRootItem();
		HTREEITEM hTreeChildItem = m_wndDatabaseTreeView.GetChildItem(hTreeRootItem);
		while (hTreeChildItem != NULL)
		{
			m_wndDatabaseTreeView.SortChildren(m_wndDatabaseTreeView.GetParentItem(hTreeItem));
			hTreeChildItem = m_wndDatabaseTreeView.GetNextItem(hTreeChildItem, TVGN_NEXT);
		}*/
		
	}
	return;
}
/**
* RegisterClientNotify
*
* Registers for DB update notification 
*/
bool CDatabaseTreeView::RegisterClientNotify()
{

	// Selected Objects Notifications
	::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<::NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
	_selectedObjects->RegisterClientObserver(this);

	// Database Update Notifications
	// get client interafce to service
	::NM::ODB::IDatabaseUpdate* ptr = reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache"));
	_updateCache.reset(ptr);

	// create require updates list
	::std::vector<::std::wstring> attr_list;
	attr_list.push_back(L"shortname");
	attr_list.push_back(L"inservice");
	//attr_list.push_back(L"create");
	attr_list.push_back(L"delete");
	// request updates for above list
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Vertex, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Group, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Edge, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Flow, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Demand, attr_list);
	_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Path, attr_list);

	return true;
}
/**
* OnDeleteObject
*
* Called from HTREEITEM popup menu on a valid object(s) to delete
*/
void CDatabaseTreeView::OnDeleteObject()
{
	::std::unique_ptr<NM::ODB::IObjectDatabase> _odb(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
	::std::vector<::NM::ODB::ODBUID> vec, vecCopy;
	::std::vector<::NM::ODB::ODBUID> vecEmpty;

	if (!_selectedObjects) return;
	if (!_odb) return;


	// get current selected objects
	_selectedObjects->GetSelected(vec);
	vecCopy = vec;
	::std::wstring message = L"Confirm delete of ";
	message += ::std::to_wstring(vec.size());
	message += L" object(s).";

	
	DWORD questionAnswer = ::MessageBox(this->GetSafeHwnd(), message.c_str(), L"Delete", MB_YESNO);
	if (questionAnswer == IDNO) return;

	// if we are currently highlighting on map, remove highlight
	if (_objectMapHighlighted)
	{
		// calling this will set spotlightoff
		OnShowObjectOnMap();
	}

	// lock updates until we are done.
	::NM::ODB::LockHandle hLock =_updateCache->GetLock();

	// deselect all objects with empty vec
	_selectedObjects->Select(vecEmpty);
	// delete all objects
	for each(::NM::ODB::ODBUID object in vec)
	{
		if (_odb->DeleteObject(object))
		{
			::std::vector<::NM::ODB::ODBUID>::iterator it = ::std::find(vecCopy.begin(), vecCopy.end(), object);
			if (it != vecCopy.end())
				vecCopy.erase(it);
		}
	}

	if (!vecCopy.empty())
	{
		::MessageBox(NULL, L"Some objects could not be deleted", L"Problem", MB_ICONEXCLAMATION);
		_selectedObjects->Select(vecCopy);
	}
	return;
}
/**
*
*
*
*/
void CDatabaseTreeView::OnUpdateDeleteObject(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetValidSelectedCount());
}
/**
*
*
*
*/
void CDatabaseTreeView::OnShowObjectOnMap()
{
	::NM::NetGraph::INetworkView* pView = static_cast<::NM::NetGraph::INetworkView*>(reg->GetClientInterface(L"NETWORKVIEW"));
	if (!_objectMapHighlighted)
	{
		::std::vector<::NM::ODB::OBJECTUID> objectList;
		objectList = GetSelectedObjects();
		if (objectList.size() > 0)
		{
			pView->SetSpotlightOn(objectList);
			_objectMapHighlighted = true;
		}
	}
	else
	{
		pView->SetSpotlightOff();
		_objectMapHighlighted = false;
	}
	delete pView;
}
/**
* OnToggleObjectInService
*
* Dynamic Context Menu Item
*/
void CDatabaseTreeView::OnToggleObjectInService()
{
	//CMenu* menu = GetMenu();
	CMenu* submenu = nullptr;
	submenu = menu.GetSubMenu(0);

	MENUITEMINFO menuItemInfo;
	::std::memset(&menuItemInfo, 0, sizeof(MENUITEMINFO));
	menuItemInfo.cbSize = sizeof(MENUITEMINFO);
	menuItemInfo.fMask = MIIM_DATA;
	menu.GetMenuItemInfoW(ID_OBJECTMENU_OBJECT_INSERVICE, &menuItemInfo);

	::std::vector<::NM::ODB::OBJECTUID> objectList;
	objectList = GetSelectedObjects();
	::std::vector<::NM::ODB::OBJECTUID>::iterator objectit = objectList.begin();
	::NM::ODB::LockHandle lock = _updateCache->GetLock();
	while (objectit != objectList.end())
	{
		::std::wstring objectType = _odb->GetObjectTypeName(*objectit);
		if ((objectType == L"vertex") || (objectType == L"edge"))
		{
			_odb->SetValue(*objectit, L"inservice", ::NM::ODB::real_bool( (((BOOL)menuItemInfo.dwItemData) ? true : false)));
		}

		++objectit;
	}
	//::MessageBox(NULL, (((BOOL)menuItemInfo.dwItemData) ? L"enable" : L"disable"), L"Set Node InService to ....", MB_OK);
	return;
}
/**
*
*
*
*/
BOOL CDatabaseTreeView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{

	NMHDR *pNMHDR = (NMHDR*)lParam;

	switch (pNMHDR->code)
	{
	case TVN_ASYNCDRAW:
		break;
	case TVN_BEGINDRAG:
		return FALSE;
		break;
	case TVN_BEGINLABELEDIT:
		break;
	case TVN_BEGINRDRAG:
		break;
	case TVN_DELETEITEM:
		break;
	case TVN_ENDLABELEDIT:
		break;
	case TVN_GETDISPINFO:
		break;
	case TVN_GETINFOTIP:
		break;
	case TVN_ITEMCHANGED:
		break;
	case TVN_ITEMCHANGING:
		break;
	case TVN_ITEMEXPANDED:
		break;
	case TVN_ITEMEXPANDING:
		break;
	case TVN_KEYDOWN:
		//OutputDebugString(L"\n TVN_KEYDOWN***");
		OnTreeViewKeyDown(pNMHDR, pResult);
		break;
	case TVN_SELCHANGED:
		OutputDebugString(L"\n TVN_SELCHANGED***");
		OnTreeViewSelectionChanged(pNMHDR, pResult);		// not used
		break;
	case TVN_SELCHANGING:
		OnTreeViewSelectionChanging(pNMHDR, pResult); // returns TRUE to prevent change happening
		break;
	case TVN_SETDISPINFO:
		break;
	case TVN_SINGLEEXPAND:
		break;
	case NM_OUTOFMEMORY:
		OutputDebugString(L"\n NM_OUTOFMEMORY");
		break;
	case NM_CLICK:
		//OutputDebugString(L"\n NM_CLICK*** (LBUTTONUP");
		/*if (_hNewTimer != NULL)
		{
			if(_objectMapHighlighted)
				OnShowObjectOnMap();
			DeleteTimerQueueTimer(NULL, _hNewTimer, NULL);
			_hNewTimer = NULL;
		}*/
		//::ReleaseCapture();
		//OnTreeViewLeftButtonDown(pNMHDR, pResult);
		break;
	case NM_DBLCLK:
		OutputDebugString(L"\n NM_DBLCLK");
		break;
	case NM_RETURN:
		OutputDebugString(L"\n NM_RETURN");
		break;
	case NM_RCLICK:
		OutputDebugString(L"\n NM_RCLICK***");
		//OnTreeViewRightButtonDown(pNMHDR, pResult);
		break;
	case NM_RDBLCLK:
		OutputDebugString(L"\n NM_RDBLCLK");
		break;
	case NM_SETFOCUS:
		OutputDebugString(L"\n NM_SETFOCUS");
		break;
	case NM_KILLFOCUS:
		OutputDebugString(L"\n NM_KILLFOCUS");
		break;
	case NM_CUSTOMDRAW:
		//OutputDebugString(L"\n NM_CUSTOMDRAW**");
		 OnCustomDraw(pNMHDR, pResult);
		break;
	case NM_HOVER:
		OutputDebugString(L"\n NM_HOVER");
		break;
	case NM_NCHITTEST:
		OutputDebugString(L"\n NM_NCHITTEST");
		break;
	case NM_KEYDOWN:
		OutputDebugString(L"\n NM_KEYDOWN");
		break;
	case NM_RELEASEDCAPTURE:
		OutputDebugString(L"\n NM_RELEASEDCAPTURE");
		break;
	case NM_SETCURSOR:
		//OutputDebugString(L"\n NM_SETCURSOR");
		break;
	case NM_CHAR:
		OutputDebugString(L"\n NM_CHAR");
		break;
	case NM_TOOLTIPSCREATED:
		OutputDebugString(L"\n NM_TOOLTIPSCREATED");
		break;
	case NM_LDOWN:
		OutputDebugString(L"\n NM_LDOWN ***");
		OnTreeViewLeftButtonDown(pNMHDR, pResult);
		break;
	case NM_RDOWN:
		OutputDebugString(L"\n NM_RDOWN");
		break;
	case NM_THEMECHANGED:
		OutputDebugString(L"\n NM_THEMECHANGED");
		break;
	case NM_FONTCHANGED:
		OutputDebugString(L"\n NM_FONTCHANGED");
		break;
	case NM_CUSTOMTEXT:
		OutputDebugString(L"\n NM_CUSTOMTEXT");
		break;
	}


	return TRUE;
}
/**
*
*
*
*/
LRESULT CALLBACK CDatabaseTreeView::TreeViewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	CDatabaseTreeView* pClass = (CDatabaseTreeView*)dwRefData;

	switch (uMsg)
	{
	case WM_LBUTTONDBLCLK:
		if (pClass->OnTreeViewLeftButtonDblClick(nullptr, nullptr) == 0)
		{
			return 0;
		}
		break;

	case WM_LBUTTONDOWN:
		OutputDebugString(L"\nTreeViewWndProc\tWM_LBUTTONDOWN");
		::SetCapture(pClass->m_wndDatabaseTreeView);
		pClass->OnTreeViewLeftButtonDown(nullptr, nullptr);
		break;

	case WM_LBUTTONUP:
		OutputDebugString(L"\nTreeViewWndProc\tWM_LBUTTONUP");
		if (pClass->_hNewTimer != NULL)
		{
			if (pClass->_objectMapHighlighted)
				pClass->OnShowObjectOnMap();
			DeleteTimerQueueTimer(NULL, pClass->_hNewTimer, NULL);
			pClass->_hNewTimer = NULL;
		}
		::ReleaseCapture();
		break;

	case WM_RBUTTONDOWN:
		pClass->OnTreeViewRightButtonDown(nullptr, nullptr);
		break;

	case WM_VSCROLL:
		{
			/*pClass->_bIgnoreSelected = true;
			LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			pClass->_bIgnoreSelected = false;
			return result;*/
		}
		break;

	case WM_PAINT:
		if (pClass->_bRedrawSet)
			return 0;		// means we dealt with paiting
		break;

	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}









//
///**
//* OnTreeViewClick
//*
//* i.e. Click on checkbox
//*/
//void CDatabaseTreeView::OnTreeViewClick(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	//OutputDebugString(L"\nOnTreeViewClick");
//
//	NMTREEVIEW* nmTreeViewStruct = (NMTREEVIEW*)pNMHDR;
//	//LPARAM lp = nmTreeViewStruct->hdr.hwndFrom;
//	//LPNMHDR lpnmh = (LPNMHDR)lp;
//
//	TVHITTESTINFO ht = { 0 };
//
//	if (pNMHDR->code == NM_CLICK)
//	{
//		DWORD dwpos = GetMessagePos();
//
//		// include <windowsx.h> and <windows.h> header files
//		ht.pt.x = GET_X_LPARAM(dwpos);
//		ht.pt.y = GET_Y_LPARAM(dwpos);
//		::MapWindowPoints(HWND_DESKTOP, nmTreeViewStruct->hdr.hwndFrom, &ht.pt, 1);
//
//		TreeView_HitTest(nmTreeViewStruct->hdr.hwndFrom, &ht);
//
//		if (TVHT_ONITEMSTATEICON & ht.flags)
//		{
//			//OutputDebugString(L"\tCheckbox Clicked");
//			HTREEITEM hItemChanged = ht.hItem;
//			TVITEM tvitem = { 0 };
//			tvitem.hItem = hItemChanged;
//			tvitem.mask = TVIF_HANDLE | TVIF_STATE;
//			tvitem.stateMask = TVIS_STATEIMAGEMASK;
//			if (m_wndDatabaseTreeView.GetItem(&tvitem))
//			{
//				LPARAM lp = tvitem.lParam;
//				IDUIDMAP::iterator it = id_to_uid_map.find(static_cast<long>(lp));
//				if (it != id_to_uid_map.end())
//				{
//					if ((tvitem.state >> 12) == 1) // 1 means checked, 2 means unchecked 
//					{
//						AddChecked(it->second);
//					}
//					else
//					{
//						RemoveChecked(it->second);
//					}
//					::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
//					_selectedObjects->Select(_selectedItems);
//				}
//			}
//		}
//		else
//		{
//			//OutputDebugString(L"\tNothing to do..");
//		}
//	}
//
//}