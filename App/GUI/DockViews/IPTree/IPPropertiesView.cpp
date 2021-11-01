#include "stdafx.h"
#include "GuiTest3.h"
#include "IPPropertiesView.h"
#include "Interfaces\ObjectDatabaseDefines.h"
#include "Interfaces\IObjectDatabase.h"
#include "IServiceRegistry.h"
#include "IIPService.h"
#include <string>
#include <tuple>
#include <algorithm>

extern NM::Registry::IServiceRegistry* reg;;

IMPLEMENT_DYNAMIC(CIPPropertiesView, CWnd)
/**
*
*
*
*/
BEGIN_MESSAGE_MAP(CIPPropertiesView, CWnd)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SHOWONMAP, &CIPPropertiesView::OnShowOnMap)
	ON_COMMAND(ID_PROPERTIES, &CIPPropertiesView::OnProperties)
END_MESSAGE_MAP()
/**
*
*
*
*/
CIPPropertiesView::CIPPropertiesView() 
{

}
/**
*
*
*
*/
CIPPropertiesView::~CIPPropertiesView()
{
}
/**
*
*
*
*/
int CIPPropertiesView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	_odb.reset(static_cast<::NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	ASSERT(_odb);

	_pTree.reset(static_cast<::NM::IPRTREE::IIPService*>(reg->GetClientInterface(L"IPSERVICE")));
	ASSERT(_pTree);

	// create properties grid
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 758))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}
	SetWindowLong(m_wndPropList, GWL_EXSTYLE, WS_EX_COMPOSITED | WS_EX_LAYERED);
	SetPropListFont();
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea(FALSE);
	m_wndPropList.SetDescriptionRows(0);
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties(FALSE);

	AdjustLayout();
	DisplayDefault();

	return 0;
}
/**
*
*
*
*/
void CIPPropertiesView::SetPropListFont()
{
	return;

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
	m_wndPropList.SetFont(&m_fntPropList);
	return;
}
/**
*
*
*
*/
void CIPPropertiesView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
		return;

	CRect rectClient;
	GetClientRect(rectClient);		
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	return;
}
/**
*
*
*
*/
void CIPPropertiesView::OnSize(UINT nType, int cx, int cy)
{
	//CBCGPDockingControlBar::OnSize(nType, cx, cy);
	AdjustLayout();
}
/**
* DisplayDefault
*
*
*/
void CIPPropertiesView::DisplayDefault()
{
	m_wndPropList.SetRedraw(FALSE);
	m_wndPropList.RemoveAll();
#pragma message("Need better stats from IPRTree on networks, subnets, etc")

	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(L"IP Statistics");

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"interfacetable"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Subnet Count", oleType, L"Subnets (requires proper stats from IPRTree");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	m_wndPropList.MarkModifiedProperties(FALSE);
	m_wndPropList.AddProperty(pGroup1);
	m_wndPropList.SetRedraw(TRUE);
	m_wndPropList.RedrawWindow();
	return;
}
/**
*
*
*
*/
void CIPPropertiesView::ShowProperties(::std::vector<::NM::IPRTREE::IPV4PREFIX> vecPrefixes)
{
	ASSERT(m_wndPropList);
	m_wndPropList.SetRedraw(FALSE);
	m_wndPropList.RemoveAll();

	//CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(L"Details");

	::std::vector<::NM::IPRTREE::IPV4PREFIX>::iterator vecit = vecPrefixes.begin();
	while (vecit != vecPrefixes.end())
	{

		// get prefix type 
		::NM::IPRTREE::PrefixType prefixType = _pTree->GetPrefixType(::std::get<0>(*vecit), ::std::get<1>(*vecit));

		// prefix as parent (and a prop group)
		::std::wstring strPrefix =
			::std::to_wstring(::std::get<0>(*vecit).S_un.S_un_b.s_b1) + L"." +
			::std::to_wstring(::std::get<0>(*vecit).S_un.S_un_b.s_b2) + L"." +
			::std::to_wstring(::std::get<0>(*vecit).S_un.S_un_b.s_b3) + L"." +
			::std::to_wstring(::std::get<0>(*vecit).S_un.S_un_b.s_b4) + L"\\" +
			::std::to_wstring(::std::get<1>(*vecit));
		
		switch (prefixType)
		{
		case ::NM::IPRTREE::PrefixType::Host:
			strPrefix += L" (Host)";
			break;

		case ::NM::IPRTREE::PrefixType::HostRoute:
			strPrefix += L" (Host Route)";
			break;

		case ::NM::IPRTREE::PrefixType::Network:
			strPrefix += L" (Network)";
			break;

		case ::NM::IPRTREE::PrefixType::Broadcast:
			strPrefix += L" (Broadcast)";
			break;

		default:
			strPrefix += L" (Unknown)";
			break;
		}
		
		// Create Top Level Group for Prefix
		CMFCPropertyGridProperty *pPrefix = new CMFCPropertyGridProperty(strPrefix.c_str(), (DWORD_PTR)PropertyValueType::Prefix);

		::std::map<::std::wstring, CMFCPropertyGridProperty*> nodePropertyGroupMap;

		// interface as nodes group child
		::NM::IPRTREE::INTFLIST::iterator intfit = ::std::get<2>(*vecit).begin();
		while( intfit != ::std::get<2>(*vecit).end())
		{
			ASSERT((*intfit) != ::NM::ODB::INVALID_OBJECT_UID);

			// get interface owning node name
			// first node objecuid
			::NM::ODB::OBJECTUID nodeObjectUID = _odb->GetValue(*intfit, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
			ASSERT(nodeObjectUID != ::NM::ODB::INVALID_OBJECT_UID);
			// get node name
			::std::wstring nodeName = _odb->GetValue(nodeObjectUID, L"shortname")->Get<::NM::ODB::ODBWString>();

			CMFCPropertyGridProperty *pNode = nullptr;
			// if we have this property group already, then use it, else create new child group for node to hold interfaces
			::std::map<::std::wstring, CMFCPropertyGridProperty*>::iterator nodeNameIt = nodePropertyGroupMap.find(nodeName);
			if (nodeNameIt != nodePropertyGroupMap.end())
			{
				// pNode is the node group
				pNode = nodeNameIt->second;
			}
			else
			{				
				// Create Child Group for connected/assigned node as prefix child
				pNode = new CMFCPropertyGridProperty(nodeName.c_str(), (DWORD_PTR)PropertyValueType::Node);
				// pNode is the node group
				pPrefix->AddSubItem(pNode);
				nodePropertyGroupMap[nodeName] = pNode;
			}
			

			// get connected edge name 
			::NM::ODB::OBJECTUID edgeObjectUID = _odb->GetValue(*intfit, L"edgeUID")->Get<::NM::ODB::OBJECTUID>();
			ASSERT(nodeObjectUID != ::NM::ODB::INVALID_OBJECT_UID);
			// get edge name
			::std::wstring edgeName = _odb->GetValue(edgeObjectUID, L"shortname")->Get<::NM::ODB::ODBWString>();
			COleVariant oleTypeInterface(edgeName.c_str());

			// get the interface name
			::std::wstring intfName = _odb->GetValue(*intfit, L"shortname")->Get<::NM::ODB::ODBWString>();
			CMFCPropertyGridProperty *pIntf = new CMFCPropertyGridProperty(intfName.c_str(), oleTypeInterface, L"Interface", (DWORD_PTR)PropertyValueType::InterfaceLink);


			pIntf->AllowEdit(FALSE);
			pNode->AddSubItem(pIntf);

			++intfit;
		}

		m_wndPropList.MarkModifiedProperties(FALSE);
		m_wndPropList.AddProperty(pPrefix);
		
		++vecit;
	}
	
	
	m_wndPropList.SetRedraw(TRUE);
	m_wndPropList.RedrawWindow();
	return;
}
/**
*
*
*
*/
void CIPPropertiesView::ShowProperties(::NM::IPRTREE::RD)
{
	m_wndPropList.SetRedraw(FALSE);
	m_wndPropList.RemoveAll();

	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(L"Details");


	m_wndPropList.MarkModifiedProperties(FALSE);
	m_wndPropList.AddProperty(pGroup1);
	m_wndPropList.SetRedraw(TRUE);
	m_wndPropList.RedrawWindow();
}

void CIPPropertiesView::OnContextMenu(CWnd* pCWND, CPoint pt)
{
	::std::wstring menutext;
	HWND hWnd = GetSafeHwnd();
	CMenu menu;
	CMenu submenu;
	
	CMFCPropertyGridProperty *pProp = nullptr;
	CMFCPropertyGridProperty::ClickArea pnArea;
	
	POINT p;
	p.x = pt.x;
	p.y = pt.y;
	pCWND->ScreenToClient(&p);
	pProp = m_wndPropList.HitTest(p, &pnArea);

	if (!pProp) return;

	// create menus
	menu.CreateMenu();
	submenu.CreatePopupMenu();
	menu.AppendMenuW(MF_POPUP | MF_STRING | MF_ENABLED, (UINT_PTR)submenu.m_hMenu, L"IPPropMenu");

	DWORD_PTR propType = pProp->GetData();
	switch ((PropertyValueType)propType)
	{
	case PropertyValueType::Prefix:
		//submenu.AppendMenuW(MF_STRING, 35002, L"Show Prefix Prop");
		return;
		break;

	case PropertyValueType::Node:		
		submenu.AppendMenuW(MF_STRING, ID_SHOWONMAP, L"Show On Map");
		submenu.AppendMenuW(MF_STRING, ID_PROPERTIES, L"Properties");
		break;

	case PropertyValueType::InterfaceLink:
		submenu.AppendMenuW(MF_STRING, ID_PROPERTIES, L"Properties");
		break;

	case PropertyValueType::None:
	default:
		return;
		break;
	}


	// show popup menu
	POINT screenPoint;
	GetCursorPos(&screenPoint);

	CWnd* pWnd = CWnd::FromHandle(hWnd);
	CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
	pPopupMenu->Create(pWnd, screenPoint.x, screenPoint.y, submenu.m_hMenu, TRUE, FALSE);


	return;
}


void CIPPropertiesView::OnShowOnMap()
{
	// TODO: Add your command handler code here
}


void CIPPropertiesView::OnProperties()
{
	// TODO: Add your command handler code here
}
