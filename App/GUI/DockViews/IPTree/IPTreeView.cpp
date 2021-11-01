#include "stdafx.h"
#include "mainfrm.h"
#include "IPTreeView.h"
#include "Resource.h"
#include "GuiTest3.h"
#include "IIPService.h"
#include "IPPropertiesView.h"
#include "IServiceRegistry.h"
#include "Interfaces\IObjectDatabase.h"

#include <string>
#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// temp
static CIPTreeView::ipinfo* dialogInfo = nullptr;

extern NM::Registry::IServiceRegistry* reg;

BEGIN_MESSAGE_MAP(CIPTreeView, CWnd) //CBCGPDockingControlBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_REFRESH, OnRefresh)
	ON_COMMAND(ID_ADDPREFIX, OnAddPrefix)
	ON_COMMAND(ID_DELETEPREFIX, OnDeletePrefix)
	ON_COMMAND(ID_EDITPREFIX, OnEditPrefix)
	ON_COMMAND(ID_COPYPREFIX, OnCopyPrefix)
	ON_COMMAND(ID_EXPANDALL, OnExpandAll)
	ON_COMMAND(ID_COLLAPSEALL, OnCollapseAll)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_CBN_SELCHANGE(84, &CIPTreeView::OnCbnSelchange)
END_MESSAGE_MAP()
/**
*
*
*
*/
CIPTreeView::CIPTreeView():
	m_wndProp(nullptr)
{
	m_nComboHeight = 0;

	//pTree = new ::NM::IPRTREE::PTree;

	// Build IP Tree with sample state - temp as this will be in a service seperatly.
	//AddSampleIPTreeData();
}
/**
*
*
*
*/
CIPTreeView::~CIPTreeView()
{
	_pUpdateCache->DeleteClientUpdatesQueue(this);	
	//delete pTree;
	//pTree = nullptr;
}
/**
*
*
*
*/
void CIPTreeView::EnableProperties(CIPPropertiesView* pPropGrid)
{
	m_wndProp = pPropGrid;
}
/**
*
*
*
*/
int CIPTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	///if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	_pTree.reset(static_cast<::NM::IPRTREE::IIPService*>(reg->GetClientInterface(L"IPSERVICE")));
	ASSERT(_pTree);
	
	_pUpdateCache.reset(static_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"OBJECTUPDATECACHE")));
	ASSERT(_pUpdateCache);

	_odb.reset(static_cast<::NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	ASSERT(_odb);

	
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	/************************************************************************************************************************************************
	* Create combo box:
	************************************************************************************************************************************************/
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |  WS_VSCROLL;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 84))
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect(&rectCombo);
	m_nComboHeight = rectCombo.Height();

	/*************************************************************************************************************************************************
	* Create tree view:
	*************************************************************************************************************************************************/
	const DWORD dwViewStyle2 = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	if (!m_wndIPView.Create(dwViewStyle2, rectDummy, this, 277))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	/************************************************************************************************************************************************
	* Load TreeControl images:
	************************************************************************************************************************************************/
	m_imgIPView.Create(IDB_IPTREENODE_24, 16, 0, RGB(255, 0, 0));
	m_wndIPView.SetImageList(&m_imgIPView, TVSIL_NORMAL);

	/*************************************************************************************************************************************************
	* Create Toolbar & Load ToolBar Image List
	************************************************************************************************************************************************/
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_IPTREEVIEW);
	m_wndToolBar.LoadToolBar(IDR_IPTREEVIEW, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);
	
	SetPropListFont();
	// build from data
	RefreshRDList();
	SetSelectedRouteDistinguisher(RDPAIRTOLONGT0(65010,10));
	AdjustLayout();
	OnCbnSelchange();

	::std::vector<::std::wstring> attributeList;
	attributeList.push_back(L"create");
	attributeList.push_back(L"delete");
	attributeList.push_back(L"rd");
	attributeList.push_back(L"ipaddress");
	attributeList.push_back(L"ipprefixlength");
	_pUpdateQueue = _pUpdateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Interface, attributeList);
	attributeList.clear();
	attributeList.push_back(L"*");
	_pUpdateQueue = _pUpdateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Database, attributeList);  //DatabaseUpdateType::Refresh


	//::NM::IPRTREE::RD rd			= RDPAIRTOLONGT0(65535, 4294967295);
	//::NM::IPRTREE::RDTYPE type		= RDLONGTOTYPE(rd);
	//::NM::IPRTREE::RDASN asn		= RDLONGTOASNT0(rd);
	//::NM::IPRTREE::RDVALUE value	= RDLONGTOVALUET0(rd);
	return 0;
}
/**
*
*
*
*/
void CIPTreeView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	///CBCGPDockingControlBar::OnSize(nType, cx, cy);
	AdjustLayout();
}
/**
*
*
*
*/
void CIPTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndIPView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		///CBCGPDockingControlBar::OnContextMenu(pWnd, point);
		CWnd::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_IPTREEVIEW, point.x, point.y, this, TRUE);
}
/**
*
*
*
*/
void CIPTreeView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	//m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nComboHeight-1, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

	//m_wndIPView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndIPView.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), rectClient.Height() - (m_nComboHeight + cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}
/**
*
*
*
*/
void CIPTreeView::OnRefresh()
{
	RefreshRDList();
	OnCbnSelchange();
}
/**
*
*
*
*/
void CIPTreeView::OnDeletePrefix()
{
	INDEXMAPIT it = GetSelectedPrefix();
	if (it == _ipIndexMap.end())
		return;

	//INT if (::std::get<2>(it->first) == INVALID_OBJECT_UID)
	if (::std::get<2>(it->first).size() == 0)
		return;

	::NM::IPRTREE::RD rd = GetCurrentRDSelection();
	::NM::IPRTREE::RDTYPE rdType = static_cast<::NM::IPRTREE::RDTYPE>((rd) >> 48);
	::NM::IPRTREE::RDASN rdASN = static_cast<::NM::IPRTREE::RDASN>((rd) >> 32);
	::NM::IPRTREE::RDVALUE rdValue = static_cast<::NM::IPRTREE::RDVALUE>(rd);
	::NM::IPRTREE::IPV4ADDR ipv4addr = ::std::get<0>(it->first);
	::NM::IPRTREE::IPV4PREFIXLENGTH prefixLength = ::std::get<1>(it->first);

	int response = 0;
	HRESULT hr = TaskDialog(this->GetSafeHwnd(), NULL, L"Delete IP Prefix", L"Would you like to delete all child subnets too?", L"content", TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON, NULL, &response);
	if (!SUCCEEDED(hr)) return;
	if (response == IDCANCEL) return;

	OutputDebugString(L"\nto transform...");
	assert(false);
	///_pTree->Delete(rdASN, rdValue, ipv4addr, prefixLength, (response == IDYES ? true : false));

	OnRefresh();
	return;
}
/**
*
*
*
*/
void CIPTreeView::OnEditPrefix()
{
	INDEXMAPIT it = GetSelectedPrefix();
	if (it == _ipIndexMap.end())
		return;

	// INT if (::std::get<2>(it->first) == INVALID_OBJECT_UID)
	if (::std::get<2>(it->first).size() == 0)
		return;

	::NM::IPRTREE::RD rd = GetCurrentRDSelection();
	::NM::IPRTREE::RDTYPE rdType = static_cast<::NM::IPRTREE::RDTYPE>((rd) >> 48);
	::NM::IPRTREE::RDASN rdASN = static_cast<::NM::IPRTREE::RDASN>((rd) >> 32);
	::NM::IPRTREE::RDVALUE rdValue = static_cast<::NM::IPRTREE::RDVALUE>(rd);

	dialogInfo.asn = rdASN;
	dialogInfo.value = rdValue;
	// 0=ipaddr, 1=prefixlength, 2=intfid
	dialogInfo.addr = ::std::get<0>(it->first);
	dialogInfo.prefixLength = ::std::get<1>(it->first);
	//// TODO dialogInfo.interfaceID = ::std::get<2>(it->first).size(); //INT
	dialogInfo.ok = false;

	if (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_ADDPREFIX), this->GetSafeHwnd(), CIPTreeView::dialogProc, (LPARAM)this) == TRUE)
	{
		// edit prefix 
		//_pTree->Insert(dialogInfo.asn, dialogInfo.value, dialogInfo.addr, dialogInfo.prefixLength, dialogInfo.interfaceID);
		::MessageBox(this->GetSafeHwnd(), L"No impl.", L"no impl.", MB_ICONEXCLAMATION);
		OnRefresh();
	}
	return;
}
/**
*
*
*
*/
void CIPTreeView::OnCopyPrefix()
{
	INDEXMAPIT it = GetSelectedPrefix();
	if (it == _ipIndexMap.end())
		return;

	::std::string clipboardData;
	clipboardData += ::std::to_string(::std::get<0>(it->first).S_un.S_un_b.s_b1) + ".";
	clipboardData += ::std::to_string(::std::get<0>(it->first).S_un.S_un_b.s_b2) + ".";
	clipboardData += ::std::to_string(::std::get<0>(it->first).S_un.S_un_b.s_b3) + ".";
	clipboardData += ::std::to_string(::std::get<0>(it->first).S_un.S_un_b.s_b4) + "\\";
	clipboardData += ::std::to_string(::std::get<1>(it->first));

	// test copied
	//const char* output = "Test";
	//const size_t len = strlen(output) + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, clipboardData.size()+1);
	memcpy(GlobalLock(hMem), clipboardData.c_str(), clipboardData.size()+1);
	GlobalUnlock(hMem);
	::OpenClipboard(0);
	::EmptyClipboard();
	::SetClipboardData(CF_TEXT, hMem);
	::CloseClipboard();
	return;
}
/**
*
*
*
*/
void CIPTreeView::OnExpandAll()
{
	ExpandAll(m_wndIPView.GetRootItem());
}
/**
*
*
*
*/
void CIPTreeView::OnCollapseAll()
{
	m_wndIPView.SetRedraw(FALSE);
	CollapseAll(m_wndIPView.GetRootItem());
	m_wndIPView.Expand(m_wndIPView.GetRootItem(), TVE_EXPAND);
	m_wndIPView.SetRedraw(TRUE);
	m_wndIPView.RedrawWindow();
	return;
}
/**
*
*
*
*/
void CIPTreeView::OnAddPrefix()
{
	::NM::IPRTREE::RD rd = GetCurrentRDSelection();
	::NM::IPRTREE::RDTYPE rdType = static_cast<::NM::IPRTREE::RDTYPE>((rd) >> 48);
	::NM::IPRTREE::RDASN rdASN = static_cast<::NM::IPRTREE::RDASN>((rd) >> 32);
	::NM::IPRTREE::RDVALUE rdValue = static_cast<::NM::IPRTREE::RDVALUE>(rd);
	dialogInfo.asn = rdASN;
	dialogInfo.value = rdValue;

	// 0=ipaddr, 1=prefixlength, 2=intfid
	INDEXMAPIT it = GetSelectedPrefix();
	if (it == _ipIndexMap.end())
	{
		dialogInfo.addr.S_un.S_addr = 0;
		dialogInfo.prefixLength = 0;
	}
	else
	{
		dialogInfo.addr = ::std::get<0>(it->first);
		dialogInfo.prefixLength = ::std::get<1>(it->first);
	}
	//// TODO dialogInfo.interfaceID = rand() % 6000;
	dialogInfo.ok = false;

	if (DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_ADDPREFIX), this->GetSafeHwnd(), CIPTreeView::dialogProc, (LPARAM)this) == TRUE)
	{
		// add prefix 
		OutputDebugString(L"\n NewPrefix\t to transform");
		assert(false);
		///_pTree->Insert(dialogInfo.asn, dialogInfo.value, dialogInfo.addr, dialogInfo.prefixLength, dialogInfo.interfaceID);
		OnRefresh();
	}
	return;
}
/**
*
*
*
*/
INT_PTR CALLBACK CIPTreeView::dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CIPTreeView* pFileView;
	switch (uMsg)
	{

	case WM_INITDIALOG:
	{
		pFileView = (CIPTreeView*)lParam;
		::SetDlgItemInt(hwndDlg, IDC_ASN, pFileView->dialogInfo.asn, NULL);
		::SetDlgItemInt(hwndDlg, IDC_VALUE, pFileView->dialogInfo.value, NULL);

		LPARAM lp = MAKEIPADDRESS(
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b1,
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b2,
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b3,
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b4);
		::SendMessage(::GetDlgItem(hwndDlg, IDC_IPADDRESS), IPM_SETADDRESS, 0, (LPARAM)lp);

		::SetDlgItemInt(hwndDlg, IDC_PREFIXLENGTH, pFileView->dialogInfo.prefixLength, NULL);
		//// TODO ::SetDlgItemInt(hwndDlg, IDC_INTERFACEUID, pFileView->dialogInfo.interfaceID, NULL);
		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			int prefixlength = 0;
			BOOL check = FALSE;
			pFileView->dialogInfo.asn = ::GetDlgItemInt(hwndDlg, IDC_ASN, NULL, check);
			pFileView->dialogInfo.value = ::GetDlgItemInt(hwndDlg, IDC_VALUE, NULL, check);
			prefixlength = ::GetDlgItemInt(hwndDlg, IDC_PREFIXLENGTH, NULL, check);
			//// TODO pFileView->dialogInfo.interfaceID = ::GetDlgItemInt(hwndDlg, IDC_INTERFACEUID, NULL, check);

			pFileView->dialogInfo.prefixLength = (prefixlength <= 32 ? (__int8)prefixlength : 32);
			pFileView->dialogInfo.asn = (pFileView->dialogInfo.asn <= 65535 ? pFileView->dialogInfo.asn : 65535);
			pFileView->dialogInfo.value = (pFileView->dialogInfo.value <= 65535 ? pFileView->dialogInfo.value : 65535);

			// ip address control
			DWORD dwAddr;
			int iCount = (int)::SendMessage(::GetDlgItem(hwndDlg, IDC_IPADDRESS), IPM_GETADDRESS, 0, (LPARAM)&dwAddr);
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b1 = FIRST_IPADDRESS(dwAddr);
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b2 = SECOND_IPADDRESS(dwAddr);
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b3 = THIRD_IPADDRESS(dwAddr);
			pFileView->dialogInfo.addr.S_un.S_un_b.s_b4 = FOURTH_IPADDRESS(dwAddr);	

			pFileView->dialogInfo.ok = true;

			::EndDialog(hwndDlg, TRUE);
			return TRUE;
		}
			

		case IDCANCEL:
			pFileView->dialogInfo.ok = false;
			::EndDialog(hwndDlg, FALSE);
			return TRUE;
		}
		return 0L;
		
	}

	return FALSE;
}
/**
*
*
*
*/
void CIPTreeView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndIPView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}
/**
*
*
*
*/
void CIPTreeView::OnSetFocus(CWnd* pOldWnd)
{
	//CBCGPDockingControlBar::OnSetFocus(pOldWnd);
	CWnd::OnSetFocus(pOldWnd);

	m_wndIPView.SetFocus();
}
/**
*
*
*
*/
void CIPTreeView::OnChangeVisualStyle()
{
	// The Toolbar Image List
	//RemoveCaptionButtons();
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_IPTREE_VIEW_24 : IDB_IPTREE_VIEW, 0, 0, TRUE /* Locked */);

	// The Tree Control Image List
	m_imgIPView.DeleteImageList();
	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_IPTREENODE_24 : IDB_IPTREENODE_24;
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
	m_imgIPView.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_imgIPView.Add(&bmp, RGB(255, 0, 0));
	m_wndIPView.SetImageList(&m_imgIPView, TVSIL_NORMAL);
	//RemoveCaptionButtons();
}
/**
*
*
*
*/
void CIPTreeView::RefreshRDList()
{
	bool bChange = true;
	int cboCurrentSelected = m_wndObjectCombo.GetCurSel();
	::NM::IPRTREE::RD currentRD = 0;
	if (cboCurrentSelected != CB_ERR)
	{
		currentRD = GetSelectedRouteDistinguisher();
	}

	::NM::IPRTREE::RDLIST rdList = _pTree->EnumRouteDistinguishers();

	m_wndObjectCombo.ResetContent();

	OutputDebugString(L"\n\nPTree Route Distinguisher List...");
	::NM::IPRTREE::RDLIST::iterator it = rdList.begin();
	int iIndex = 0;
	while (it != rdList.end())
	{
		::NM::IPRTREE::RD rd = 0;
		rd = *it;
		// build inlines for following TYPE, ASN & VALUE ?
		// where type is 0;
		// RD[64Bit] = [TYPE[16bit],ASN[16Bit],VALUE[32Bit]] = 64Bit
		::NM::IPRTREE::RDTYPE rdType = static_cast<::NM::IPRTREE::RDTYPE>((*it) >> 48);
		::NM::IPRTREE::RDASN rdASN = static_cast<::NM::IPRTREE::RDASN>((*it) >> 32);
		::NM::IPRTREE::RDVALUE rdValue = static_cast<::NM::IPRTREE::RDVALUE>(*it);
		//::std::wstring strListItem = ::std::to_wstring(rdType) + L":" + ::std::to_wstring(rdASN) + L":" + ::std::to_wstring(rdValue);
		::std::wstring strListItem = ::std::to_wstring(rdASN) + L":" + ::std::to_wstring(rdValue);
		int itemIndex = m_wndObjectCombo.AddString(strListItem.c_str());
		m_wndObjectCombo.SetItemData(itemIndex, iIndex);
		_indexRDMap[iIndex] = (*it);

		if (rd == currentRD)
		{
			cboCurrentSelected = itemIndex;
			bChange = false;
		}

		//::std::wstring debugstr = L"\n(" + ::std::to_wstring(rd) + L")" + ::std::to_wstring(rdType) + L":" + ::std::to_wstring(rdASN) + L":" + ::std::to_wstring(rdValue);
		//OutputDebugString(debugstr.c_str());
		++iIndex;
		++it;
	}


	m_wndObjectCombo.SetCurSel(cboCurrentSelected);
	if( bChange) OnCbnSelchange();

	return;
}
/**
*
*
*
*/
void CIPTreeView::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	//CBCGPDockingControlBar::OnSettingChange(uFlags, lpszSection);
	CWnd::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}
/**
*
*
*
*/
void CIPTreeView::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	//m_wndPropList.SetFont(&m_fntPropList);
	m_wndObjectCombo.SetFont(&m_fntPropList);
}
/**
*
*
*
*/
void CIPTreeView::RefreshIPData(::NM::IPRTREE::RD rd)
{
	HTREEITEM selectedItem = NULL;
	::NM::IPRTREE::IPV4PREFIX selectedPrefix;
	::std::get<1>(selectedPrefix) = 255;	// invalid, wont match anything
	INDEXMAPIT selectedPrefixIt = GetSelectedPrefix();
	if (selectedPrefixIt != _ipIndexMap.end())
	{
		selectedPrefix = ::std::get<0>(*selectedPrefixIt);
	}

	// RD[64Bit] = [TYPE[16bit],ASN[16Bit],VALUE[32Bit]] = 64Bit
	::NM::IPRTREE::RDTYPE rdType = static_cast<::NM::IPRTREE::RDTYPE>((rd) >> 48);
	::NM::IPRTREE::RDASN rdASN = static_cast<::NM::IPRTREE::RDASN>((rd) >> 32);
	::NM::IPRTREE::RDVALUE rdValue = static_cast<::NM::IPRTREE::RDVALUE>(rd);

	//int iIndex = 0;
	m_wndIPView.SetRedraw(FALSE);
	m_wndIPView.DeleteAllItems();
	_ipIndexMap.clear();

	// Route Distinguisher
	//unsigned long long rd = (static_cast<unsigned long long>(rdasn) << 32) + rdvalue;
	::NM::IPRTREE::RDPREFIXLIST prefixList = _pTree->EnumRouteDistinguisherIPPrefixes(rd);
	
	::NM::IPRTREE::RDPREFIXLIST::iterator ipit = prefixList.begin();
	HTREEITEM hRoot = m_wndIPView.InsertItem(_T("IPv4"), 0, 0);
	m_wndIPView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	while (ipit != prefixList.end())
	{
		HTREEITEM hParent = hRoot;
		// check to see if there is a aggregate for this
		::NM::IPRTREE::IPV4PREFIX ipAggregatePrefix;

		bool bAggregateExists = _pTree->GetAggregateIPv4(rdASN, rdValue, ::std::get<0>(*ipit), ::std::get<1>(*ipit), true, ipAggregatePrefix);

		if (bAggregateExists)
		{
			///::std::map<in_addr, HTREEITEM>::iterator lookupit = _ipIndexMap.find(::std::get<0>(ipAggregatePrefix));
			::std::map<::NM::IPRTREE::IPV4PREFIX, HTREEITEM>::iterator lookupit = _ipIndexMap.find(ipAggregatePrefix);
			if (lookupit != _ipIndexMap.end())
			{
				hParent = lookupit->second;
			}
			else
			{
				ASSERT(false);
				//return;
			}
		}

		// build node string
		::std::wstring str =
			::std::to_wstring(::std::get<0>(*ipit).S_un.S_un_b.s_b1) + L"." +
			::std::to_wstring(::std::get<0>(*ipit).S_un.S_un_b.s_b2) + L"." +
			::std::to_wstring(::std::get<0>(*ipit).S_un.S_un_b.s_b3) + L"." +
			::std::to_wstring(::std::get<0>(*ipit).S_un.S_un_b.s_b4) + L"\\" +
			::std::to_wstring(::std::get<1>(*ipit));

		// get prefix type (host, hostroute, network or broadcast)
		::NM::IPRTREE::PrefixType prefixType = _pTree->GetPrefixType(::std::get<0>(*ipit), ::std::get<1>(*ipit));
		int imageIndex = 0;
		switch (prefixType)
		{
		case ::NM::IPRTREE::PrefixType::Host:
		case ::NM::IPRTREE::PrefixType::HostRoute:
		{
			// get prefix network address (create func in ptree)
			::NM::IPRTREE::IPV4ADDR networkAddress;
			networkAddress.s_addr = 0;
			_pTree->GetNetworkAddress(::std::get<0>(*ipit), ::std::get<1>(*ipit), networkAddress);

			///::std::map<in_addr, HTREEITEM>::iterator lookupit = _ipIndexMap.find(networkAddress);
			::std::map<::NM::IPRTREE::IPV4PREFIX, HTREEITEM>::iterator lookupit = _ipIndexMap.begin();

			while (lookupit != _ipIndexMap.end())
			{
				// if the ip addresses and prefixlengths match
				if ((::std::get<0>(lookupit->first).S_un.S_addr == networkAddress.S_un.S_addr) &&
					(::std::get<1>(lookupit->first) == ::std::get<1>(*ipit)))
				{
					hParent = lookupit->second;
					break;
				}
				++lookupit;
			}
			//
			//if (lookupit != _ipIndexMap.end())
			//	hParent = lookupit->second;

			imageIndex = (prefixType == ::NM::IPRTREE::PrefixType::Host ? 4 : 3);

			str += L" [" + ::std::to_wstring(::std::get<2>(*ipit).size()) + L"]"; //INT
		}
		break;

		case ::NM::IPRTREE::PrefixType::Network:
		{
			// if this network prefix is just a summary not assigned to an 
			// interface, which his thhen just for organisational purposes, assign
			// different image
			//INT if (::std::get<2>(*ipit) == INVALID_OBJECT_UID)
			if (::std::get<2>(*ipit).size() == 0)
			{
				imageIndex = 6;
			}
			else
			{
				imageIndex = 5;
				str += L" [" + ::std::to_wstring(::std::get<2>(*ipit).size()) + L"]"; //INT
			}
		}
		break;

		case ::NM::IPRTREE::PrefixType::Broadcast:
		default:
			break;
		}

		// insert ip prefix into tree
		HTREEITEM hItem = m_wndIPView.InsertItem(str.c_str(), imageIndex, imageIndex, hParent);
		if ((::std::get<0>(*ipit).s_addr == ::std::get<0>(selectedPrefix).s_addr) &&
			(::std::get<1>(*ipit) == ::std::get<1>(selectedPrefix)))
		{
			selectedItem = hItem;
		}
		
		// save ipaddr to HTREEITEM lookup
		///_ipIndexMap[::std::get<0>(*ipit)] = hItem;
		_ipIndexMap[*ipit] = hItem;

		++ipit;
	}

	ExpandAll(hRoot);
	//CollapseAll(hRoot);
	m_wndIPView.Expand(hRoot, TVE_EXPAND);
	if (selectedItem == NULL)
	{
		m_wndIPView.EnsureVisible(hRoot);
	}
	else
	{
		m_wndIPView.SelectItem(selectedItem);
		m_wndIPView.EnsureVisible(selectedItem);
	}

	m_wndIPView.SetRedraw(TRUE);
	m_wndIPView.Invalidate();
	m_wndIPView.UpdateWindow();
	return;
}
/**
*
*
*
*/
void CIPTreeView::OnCbnSelchange()
{
	int cboCurSelIndex = m_wndObjectCombo.GetCurSel();
	if (cboCurSelIndex == CB_ERR)
	{
		m_wndIPView.DeleteAllItems();
		return;
	}
	DWORD_PTR dwPtr = m_wndObjectCombo.GetItemData(cboCurSelIndex);
	::std::map<int, ::NM::IPRTREE::RD>::iterator rdit = _indexRDMap.find(static_cast<int>(dwPtr));
	assert(rdit != _indexRDMap.end());
	RefreshIPData(rdit->second);
	return;
}
/**
*
*
*
*/
::NM::IPRTREE::RD CIPTreeView::GetSelectedRouteDistinguisher()
{
	int cboCurSelIndex = m_wndObjectCombo.GetCurSel();
	DWORD_PTR dwPtr = m_wndObjectCombo.GetItemData(cboCurSelIndex);
	::std::map<int, ::NM::IPRTREE::RD>::iterator rdit = _indexRDMap.find(static_cast<int>(dwPtr));
	assert(rdit != _indexRDMap.end());
	return rdit->second;
}
/**
*
*
*
*/
void CIPTreeView::SetSelectedRouteDistinguisher(::NM::IPRTREE::RD rd)
{
	::std::map<int, ::NM::IPRTREE::RD>::iterator rdit = _indexRDMap.begin(); 
	while (rdit != _indexRDMap.end())
	{
		if (rdit->second == rd)
		{
			m_wndObjectCombo.SetCurSel(rdit->first);
			return;
		}
		++rdit;
	}
	return;
}
/**
* 
*
*
*/
CIPTreeView::INDEXMAPIT CIPTreeView::GetSelectedPrefix()
{
	HTREEITEM hSelectedItem = m_wndIPView.GetSelectedItem();

	INDEXMAPIT it = _ipIndexMap.begin();
	while (it != _ipIndexMap.end())
	{
		if (it->second == hSelectedItem)
		{
			break;
		}
		++it;
	}
	return it;
}
/**
* 
*
*
*/
::NM::IPRTREE::RD CIPTreeView::GetCurrentRDSelection()
{
	int cboCurSelIndex = m_wndObjectCombo.GetCurSel();
	DWORD_PTR dwPtr = m_wndObjectCombo.GetItemData(cboCurSelIndex);
	::std::map<int, ::NM::IPRTREE::RD>::iterator rdit = _indexRDMap.find(static_cast<int>(dwPtr));
	if (rdit == _indexRDMap.end())
		return 0;;

	return rdit->second;
}
/**
* ExpandAll
*
*
*/
void CIPTreeView::ExpandAll(HTREEITEM hItemParent)
{
	HTREEITEM childItem = hItemParent;
	if (m_wndIPView.ItemHasChildren(hItemParent) )
	{
		m_wndIPView.Expand(hItemParent, TVE_EXPAND);

		for (
			childItem = m_wndIPView.GetChildItem(childItem); 
			childItem != NULL; 
			childItem = m_wndIPView.GetNextItem(childItem, TVGN_NEXT)
			)
		{
			ExpandAll(childItem);
		}
	}
	return;
}
/**
* CollapseAll
*
*
*/
void CIPTreeView::CollapseAll(HTREEITEM hItemParent)
{
	HTREEITEM childItem = hItemParent;
	if (m_wndIPView.ItemHasChildren(hItemParent))
	{
		m_wndIPView.Expand(hItemParent, TVE_COLLAPSE);

		for (
			childItem = m_wndIPView.GetChildItem(childItem);
			childItem != NULL;
			childItem = m_wndIPView.GetNextItem(childItem, TVGN_NEXT)
			)
		{
			CollapseAll(childItem);
		}
	}
	return;
}

BOOL CIPTreeView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class

	NMHDR* nmhdr = (NMHDR*)lParam;

	switch (wParam)
	{
	// tree view
	case 277:
		switch (nmhdr->code)
		{
		case TVN_SELCHANGED:
		{
			OutputDebugString(L"\nSelected Tree Item Changed...\t");
			INDEXMAPIT it = GetSelectedPrefix();
			if (it != _ipIndexMap.end())
			{
				
				::std::wstring str =
					::std::to_wstring(::std::get<0>(it->first).S_un.S_un_b.s_b1) + L"." +
					::std::to_wstring(::std::get<0>(it->first).S_un.S_un_b.s_b2) + L"." +
					::std::to_wstring(::std::get<0>(it->first).S_un.S_un_b.s_b3) + L"." +
					::std::to_wstring(::std::get<0>(it->first).S_un.S_un_b.s_b4) + L"\\" +
					::std::to_wstring(::std::get<1>(it->first));
				OutputDebugString(str.c_str());

				::std::vector<::NM::IPRTREE::IPV4PREFIX> prefixList;
				prefixList.push_back(it->first);
				m_wndProp->ShowProperties(prefixList);

			}
		}
			break;

		default:
			break;
		}
		// outer case 277
		break;

	default:
		break;
	}

	return CWnd::OnNotify(wParam, lParam, pResult);
}
/**
* DatabaseUpdate
*
* Called on Interface [RD,IPADDR,PREFIX] Create, Update, Delete and Database Refresh
*/
void CIPTreeView::DatabaseUpdate()
{

	while (!_pUpdateQueue->Empty())
	{
		::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _pUpdateQueue->GetNextRecord();
		::NM::ODB::DatabaseUpdateType updateType = record->GetUpdateType();
		::NM::ODB::OBJECTUID objectUID = record->GetObjectUID();

		::NM::IPRTREE::RD currentRD = GetCurrentRDSelection();
		::NM::ODB::ODBLongLong objectRD = _odb->GetValue(objectUID, L"rd")->Get<::NM::ODB::ODBLongLong>();
		// if this update is for not for the RD we are currently showing in the tree, process for new RD 
		// in cbo box, add if needed then skip ip,prefix as not displayed in tree
		//if (currentRD != objectRD)
		//{
		//	if (m_wndObjectCombo.FindStringExact(-1, ::std::to_wstring(objectRD).c_str()) == CB_ERR)
		//	{
		//		RefreshRDList();
		//	}			
		//	continue;
		//}

		switch (updateType)
		{

		case ::NM::ODB::DatabaseUpdateType::Create:
		{
			switch (record->GetObjectType())
			{
			case ::NM::ODB::ObjectType::Interface:
			{
				/*
				
				hmmm how to find the parent of new item, use Ptree to find aggregate, then see if we have it, else create
				but could have a higher level aggregate - recursive aggregate search?

				*/
				RefreshRDList();
				OnCbnSelchange();
				//assert(false);
			}
			break;

			default:
				break;
			}
		}
		break;  // case ::NM::ODB::DatabaseUpdateType::Create:



		case ::NM::ODB::DatabaseUpdateType::Delete:
		{
			switch (record->GetObjectType())
			{
			case ::NM::ODB::ObjectType::Interface:
			{
				RefreshRDList();
				OnCbnSelchange();

				//::NM::IPRTREE::IPV4PREFIX ipPrefix;
				//// inaddr
				//::std::get<0>(ipPrefix) = _odb->GetValue(objectUID, L"ipaddress")->Get<::NM::ODB::ODBINADDR>();
				//// prefixlength
				//::std::get<1>(ipPrefix) = _odb->GetValue(objectUID, L"ipprefixlength")->Get<::NM::ODB::ODBByte>();
				//// intf list is not compared in map tuple, so can leave default - empty
				//INDEXMAPIT indexIT = _ipIndexMap.find(ipPrefix);
				//if (indexIT != _ipIndexMap.end())
				//{
				//	// check to see if prefix still exists in IPTree as although its deleted, it could
				//	// be deleted from an interface but still active as a network object or still assigned 
				//	// to a further interfaces.
				//	::NM::IPRTREE::IPV4ADDR netPrefix;
				//	::NM::IPRTREE::IPV4PREFIXLENGTH netPrefixLength = 0;
				//	bool found = false;
				//	::NM::IPRTREE::INTFLIST intfList = _pTree->Find(RDLONGTOASNT0(currentRD), 
				//													RDLONGTOVALUET0(currentRD), 
				//													::std::get<0>(ipPrefix), 
				//													::std::get<1>(ipPrefix), 
				//													::NM::IPRTREE::MatchType::Exact, 
				//													netPrefix, 
				//													netPrefixLength,
				//													found);

				//	if (intfList.size > 0)
				//	{
				//		// dont delete it from tree
				//		break;
				//	}

				//	// if this is a aggregate for existing.. check htreeitem?

				//	m_wndIPView.DeleteItem(indexIT->second);
				//	_ipIndexMap.erase(indexIT);
				//}
			}
			break;

			default:
				break;
			}
		}
		break;  // case ::NM::ODB::DatabaseUpdateType::Delete:



		case ::NM::ODB::DatabaseUpdateType::Update:
		{
			switch (record->GetObjectType())
			{

			case ::NM::ODB::ObjectType::Interface:
			{
				/*
				use INDEXMAP _ipIndexMap; to find the old interface prefix, and htreeitem
				
				hmmm how to find the parent of new updated, use Ptree to find aggregate, then see if we have it, else create
				but could have a higher level aggregate - recursive aggregate search?

				*/
				RefreshRDList();
				OnCbnSelchange();
				//assert(false);
			}
			break;

			default:
				break;
			}
		}
		break;  // case ::NM::ODB::DatabaseUpdateType::Update:



		case ::NM::ODB::DatabaseUpdateType::Refresh:
		{
#pragma message("CIPTreeView::DatabaseUpdate::Refresh needs more detailed update code, this is a plaster")
			RefreshRDList();
			OnCbnSelchange();
			//assert(false);
		}
		break;	// case ::NM::ODB::DatabaseUpdateType::Refresh:


		default:
			break;
		}

	}


	return;
}






