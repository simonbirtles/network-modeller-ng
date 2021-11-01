#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "GuiTest3.h"
#include <list>

#include "PropertyGridIPAddrProperty.h"
#include "MFCPropertyGridUIDProperty.h"
#include "MFCPropertyGridDateTimeEditProperty.h"
#include "MFCPropertyGridUnsignedLongLongProperty.h"
#include "MFCPropertyGridRDProperty.h"

#include "IServiceRegistry.h"						// registry interface

#include "ISelectedObjects.h"

#include "Interfaces\IObjectDatabase.h"					// for client interface 
#include "Interfaces\IAttribute.h"

// reqd for client database update notifications
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"					// for client interface 

using ::NM::ODB::real_bool;
using ::NM::ODB::real_uid;
using ::NM::ODB::real_string;
using ::NM::ODB::real_time;
using ::NM::ODB::real_short;
using ::NM::ODB::real_int;
using ::NM::ODB::real_color;
using ::NM::ODB::real_double;
using ::NM::ODB::real_byte;
using ::NM::ODB::real_longlong;
using ::NM::ODB::real_inaddr;

#define OutputDebugString

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern NM::Registry::IServiceRegistry* reg;

/**
*
*
*
*/
CPropertiesWnd::CPropertiesWnd() :
	_bIgnoreDatabaseUpdates(false),
	bResolveUIDtoName(true),
	_updateCache(nullptr),
	_currentObjectUID(::NM::ODB::INVALID_OBJECT_UID),
	m_nComboHeight(0)
{
}
/**
*
*
*
*/
CPropertiesWnd::~CPropertiesWnd()
{
	//::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
	_selectedObjects->DeRegisterClientObserver(this);
}
/**
*
*
*
*/
BEGIN_MESSAGE_MAP(CPropertiesWnd, CBCGPDockingControlBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	//ON_NOTIFY(LVN_KEYDOWN, 2, OnPropertyClick)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	
END_MESSAGE_MAP()
/**
*
*
*
*/
void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	//m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nComboHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), rectClient.Height() -(m_nComboHeight+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}
/**
*
*
*
*/
int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	//const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	//if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	//{
	//	TRACE0("Failed to create Properties Combo \n");
	//	return -1;      // fail to create
	//}
	//m_wndObjectCombo.AddString(_T("Application"));
	//m_wndObjectCombo.AddString(_T("Properties Window"));
	//m_wndObjectCombo.SetCurSel(0);
	//CRect rectCombo;
	//m_wndObjectCombo.GetClientRect (&rectCombo);
	//m_nComboHeight = rectCombo.Height();

	// create properties grid
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}
	SetWindowLong(m_wndPropList, GWL_EXSTYLE, WS_EX_COMPOSITED | WS_EX_LAYERED);
	m_wndPropList.SetDescriptionRows(4);

	//CUSTOMCOLORS
	//m_wndPropList.SetCustomColors(
	//	COLORREF(RGB(0.2*255, 0.2 * 255, 0.2 * 255)),				//The background color of property values.
	//	COLORREF(RGB(255,255,255)),				//The color of property names and property value text.
	//	COLORREF(RGB(0.2 * 255, 0.2 * 255, 0.2 * 255)),				//The background color of a property group.
	//	COLORREF(RGB(255, 255, 255)),				//The new text color of property group.
	//	COLORREF(RGB(0.2 * 255, 0.2 * 255, 0.2 * 255)),				//The background color of the description area.
	//	COLORREF(RGB(255, 255, 255)),				//The color of text in the description area.
	//	COLORREF(RGB(80, 80, 80))				//The color of lines that are drawn between properties.
	//	);


	// create tool bar
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);	// All commands will be routed via this control , not via the parent frame:

	
	_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	_selectedObjects.reset(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));

	InitPropList();			
	AdjustLayout();
	DisplayDefault();
	RegisterClientNotify();
	return 0;
}
/**
*
*
*
*/
void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);
	AdjustLayout();
}
/**
*
*
*
*/
void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}
/**
*
*
*
*/
void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}
/**
*
*
*
*/
void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}
/**
*
*
*
*/
void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}
/**
*
*
*
*/
void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
}
/**
*
*
*
*/
void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}
/**
*
*
*
*/
void CPropertiesWnd::OnProperties2()
{
	// TODO: Add your command handler code here
	OutputDebugString(L"\nOnProperties2");
	bResolveUIDtoName = !bResolveUIDtoName;
	SelectedObjectsUpdate();
}
/**
*
*
*
*/
void CPropertiesWnd::OnUpdateProperties2(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(bResolveUIDtoName);

}
/**
*
*
*
*/
void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CBCGPDockingControlBar::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}
/**
*
*
*
*/
void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CBCGPDockingControlBar::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}
/**
*
*
*
*/
void CPropertiesWnd::SetPropListFont()
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
//	m_wndObjectCombo.SetFont(&m_fntPropList);
}
/**
*
*
*
*/
//void CPropertiesWnd::OnPropertyClick(NMHDR * pNMHDR, LRESULT * pResult)
//{
//	OutputDebugString(L"\nOnPropertyClick");
//
//	*pResult = 0;
//	return;
//
//}
/**
*
*
*
*/
void CPropertiesWnd::SelectedObjectsUpdate()
{
	//::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
	::NM::OBJECTVECTOR vecObjects;
	size_t selectedCount = _selectedObjects->GetSelected(vecObjects);

	if (selectedCount > 0)
		DisplayProperties(vecObjects);
	else
	{
		m_wndPropList.RemoveAll();
		//ClearProperties();
		DisplayDefault();
		m_wndPropList.RedrawWindow();
	}

}
/**
*
*
*
*/
//void CPropertiesWnd::ClearProperties()
//{
//	m_wndPropList.RemoveAll();
//	_iAttributeGroupIndex = -1;
//	//OutputDebugString(L"\nClearProperties");
//	return;
//
//	CMFCPropertyGridProperty* pGroup1 = nullptr;
//	int iCount = m_wndPropList.GetPropertyCount();
//	//for (int i = iCount - 1; i > -1; --i)
//	//{
//	//	pGroup1 = m_wndPropList.GetProperty(i);
//	//	if (pGroup1)
//	//		m_wndPropList.DeleteProperty(pGroup1);
//	//}
//	for (int i = iCount - 1; i > -1; --i)
//	{
//		pGroup1 = m_wndPropList.GetProperty(i);
//		if (pGroup1)
//		{
//			int subItemCount = pGroup1->GetSubItemsCount();
//			for (int j = subItemCount - 1; j > -1; --j)
//			{
//				CMFCPropertyGridProperty* pSubItem = pGroup1->GetSubItem(j);
//				if (pSubItem)
//					pGroup1->RemoveSubItem(pSubItem);
//			}
//		}
//	}
//}
/**
*
*
*
*/
void CPropertiesWnd::MultiSelectChange(bool bMultiSelect)
{
}
//void CPropertiesWnd::CreateAttributeHeader()
//{
//	if (_iAttributeGroupIndex > -1) return;
//	CMFCPropertyGridProperty* pAttributeGroup = new CMFCPropertyGridProperty(L"Attributes");
//	_iAttributeGroupIndex = m_wndPropList.AddProperty(pAttributeGroup);
//	assert(_iAttributeGroupIndex != -1);
//}
/**
* DisplayProperties
*
* Only displays the last selected objects properties.
*/
void CPropertiesWnd::DisplayProperties(::NM::OBJECTVECTOR vecObjects)
{	
	//ClearProperties();

	if(vecObjects.size() > 0)
	{		
		real_uid uid = vecObjects[vecObjects.size() - 1];
		AddObjectProperty(uid);
		_currentObjectUID = uid;
	}
	return;
}
/**
*
*
*
*/
void CPropertiesWnd::DisplayDefault()
{
//	::std::unique_ptr<NM::ODB::IObjectDatabase> _odb(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));

	m_wndPropList.SetRedraw(FALSE);
	m_wndPropList.RemoveAll();

	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(L"Database Statistics");

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"vertextable")) ));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Node Count", oleType, L"Count of nodes in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"interfacetable"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Interface Count", oleType, L"Count of interfaces in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"edgetable"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Link Count", oleType, L"Count of links in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"grouptable"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Group Count", oleType, L"Count of groups in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"flowtable"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Flow Count", oleType, L"Count of flows in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"demandtable"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Demands Count", oleType, L"Count of demands in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"pathtable"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Path Count", oleType, L"Count of paths in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

#ifdef DEBUG

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"dbodelete"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Deleted Count", oleType, L"Count of deleted items in database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

	{
		COleVariant oleType(static_cast<long>(_odb->GetTableObjectCount(::std::wstring(L"dboinsert"))));
		CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty(L"Insert Count", oleType, L"Count of items waiting to be inserted into database");
		p->Enable(FALSE);
		pGroup1->AddSubItem(p);
	}

#endif
	
	m_wndPropList.MarkModifiedProperties(FALSE);
	m_wndPropList.AddProperty(pGroup1);
	m_wndPropList.SetRedraw(TRUE);
	m_wndPropList.RedrawWindow();
}
/**
* 
*
* 
*/
void CPropertiesWnd::AddObjectProperty(::NM::ODB::OBJECTUID objectUID, bool bClear)
{	
	m_wndPropList.SetRedraw(FALSE);
	m_wndPropList.RemoveAll();

	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(L"Attributes");
	CMFCPropertyGridProperty* pProp = nullptr;

#pragma region AttributeListParse
	// Get The Database Handle
//	::std::unique_ptr<NM::ODB::IObjectDatabase> _odb(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	// Get this dbo's full attribute list
	NM::ODB::ATTRIBUTELIST attributeList;
	_odb->GetAttributeList(objectUID, ATTR_ALL, attributeList);
	
	// for each of these dbo attributes, get the type and add to properties display
	NM::ODB::ATTRIBUTELIST::iterator attribIT = attributeList.begin();
	while (attribIT != attributeList.end())
	{
		NM::ODB::UPVALUE val = _odb->GetValue(objectUID, *attribIT);
		if (val)
		{
			::NM::ODB::UPIATTRIBUTE attr = _odb->GetAttribute(objectUID, *attribIT);
			::std::wstring strRequiredAttribute = (attr->IsRequiredAttribute() ? L"\nRequired Attribute" : L"\nOptional Attribute");
			strRequiredAttribute = (attr->GetUserModify() ? strRequiredAttribute : L"\nSystem Attribute");

			/************************************************
			*	::NM::ODB::TypeT::TypeUID
			************************************************/
			if (attr->GetType() == ::NM::ODB::TypeT::TypeUID)
			{
				// get the shortname of the attributes UID (ie lokkup shortname)
				::NM::ODB::OBJECTUID lookupObjectUID = val->Get<::NM::ODB::ODBUID>();
				::std::wstring propDesc = attr->GetDescription() + L"\n(GUID)";
				propDesc += strRequiredAttribute;

				if (bResolveUIDtoName) /* resolve the UID of this atttriute value to the shortname of the object it points to*/
				{

					::std::wstring strLookupName;
					if (lookupObjectUID != ::NM::ODB::INVALID_OBJECT_UID)
					{
						::NM::ODB::UPVALUE vLookupName = _odb->GetValue(lookupObjectUID, L"shortname");
						if (vLookupName)
							strLookupName = vLookupName->Get<::NM::ODB::ODBWString>();
						else
							strLookupName = L"Not a Database Object UID";
					}

					pProp = new CMFCPropertyGridUIDProperty((*attribIT).c_str(), strLookupName.c_str(), /*uid*/ val->Get<::NM::ODB::ODBUID>(), propDesc.c_str());  // strattrname, strvalue, strdesc		
					pProp->AllowEdit(attr->GetUserModify());
					pGroup1->AddSubItem(pProp);
				}
				else /* do not resolve the UID to name- output as a string */
				{
					pProp = new CMFCPropertyGridUIDProperty((*attribIT).c_str(), val->GetStringValue().c_str(), /*uid*/ val->Get<::NM::ODB::ODBUID>(), propDesc.c_str());  // strattrname, strvalue, strdesc		
					pProp->AllowEdit(attr->GetUserModify());
					pGroup1->AddSubItem(pProp);
				}
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeColor
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeColor)
			{
				::std::wstring propDesc = attr->GetDescription() + L"\n(color)";
				propDesc += strRequiredAttribute;
				CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty((*attribIT).c_str(), val->Get<::NM::ODB::ODBColor>(), NULL, propDesc.c_str());
				pColorProp->EnableOtherButton(_T("Other..."));
				//pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
				pColorProp->AllowEdit(attr->GetUserModify());
				pGroup1->AddSubItem(pColorProp);
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeBool
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeBool)
			{
				::std::wstring propDesc = attr->GetDescription() + L"\n(bool)";
				propDesc += strRequiredAttribute;
				COleVariant oleBool((short)(val->Get<::NM::ODB::ODBBool>() ? VARIANT_TRUE : VARIANT_FALSE), VT_BOOL);
				CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty((*attribIT).c_str(), oleBool, propDesc.c_str());
				p->Enable(attr->GetUserModify() ? TRUE : FALSE);
				pGroup1->AddSubItem(p);
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeInt
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeInt)
			{
				LPCTSTR lpszValidChars = L"0123456789";
				::std::wstring propDesc = attr->GetDescription() + L"\n(int)";
				propDesc += strRequiredAttribute;
				COleVariant oleType((long)val->Get<::NM::ODB::ODBInt>(), VT_I4);
				CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty((*attribIT).c_str(), oleType, propDesc.c_str(), 0, 0, 0, lpszValidChars);
				p->Enable(attr->GetUserModify() ? TRUE : FALSE);
				pGroup1->AddSubItem(p);
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeLongLong
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeLongLong)
			{
				if (attr->GetName() == L"rd")
				{
					LPCTSTR lpszValidChars = L"0123456789";
					LPCTSTR lpszEditMask		= L"ddddd dddddddddd"; // The mask string
					LPCTSTR lpszInputTemplate	= L"_____:__________"; // A mask template string that specifies the literal characters that can appear at each position in the user input. Use the underscore character ('_') as a character placeholder. The length of the lpszInputTemplate and lpszMask parameter strings must be the same.

					::std::wstring propDesc = attr->GetDescription() + L"\n(unsigned long long [rd])";
					propDesc += strRequiredAttribute;
					unsigned long long ullValue = static_cast<unsigned long long>(val->Get<::NM::ODB::ODBLongLong>()); //, VT_I8); //VT_I8);		GridCtrl does not support longlong, ODBLongLong is now a long but named the same until poss updates
					CMFCPropertyGridRDProperty *p = new CMFCPropertyGridRDProperty((*attribIT).c_str(), ullValue, propDesc.c_str(), 0, lpszEditMask, lpszInputTemplate, lpszValidChars);
					p->Enable(attr->GetUserModify() ? TRUE : FALSE);
					pGroup1->AddSubItem(p);
				}
				else
				{
					LPCTSTR lpszValidChars = L"0123456789";
					::std::wstring propDesc = attr->GetDescription() + L"\n(unsigned long long)";
					propDesc += strRequiredAttribute;
					unsigned long long ullValue = static_cast<unsigned long long>(val->Get<::NM::ODB::ODBLongLong>()); //, VT_I8); //VT_I8);		GridCtrl does not support longlong, ODBLongLong is now a long but named the same until poss updates
					CMFCPropertyGridUnsignedLongLongProperty *p = new CMFCPropertyGridUnsignedLongLongProperty((*attribIT).c_str(), ullValue, propDesc.c_str(), 0, 0, 0, lpszValidChars);
					p->Enable(attr->GetUserModify() ? TRUE : FALSE);
					pGroup1->AddSubItem(p);
				}


			}
			/************************************************
			*	::NM::ODB::TypeT::TypeDouble
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeDouble)
			{
				LPCTSTR lpszValidChars = L"0123456789";
				::std::wstring propDesc = attr->GetDescription() + L"\n(double)";
				propDesc += strRequiredAttribute;
				COleVariant oleType((long)val->Get<::NM::ODB::ODBDouble>(), VT_I4);	//VT_R8); GridCtrl does not support double , 
				CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty((*attribIT).c_str(), oleType, propDesc.c_str(), 0, 0, 0, lpszValidChars);
				p->Enable(attr->GetUserModify() ? TRUE : FALSE);
				pGroup1->AddSubItem(p);
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeShort
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeShort)
			{
				LPCTSTR lpszValidChars = L"0123456789";
				::std::wstring propDesc = attr->GetDescription() + L"\n(short)";
				propDesc += strRequiredAttribute;
				COleVariant oleType((short)val->Get<::NM::ODB::ODBShort>(), VT_I2);
				CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty((*attribIT).c_str(), oleType, propDesc.c_str(), 0, 0, 0, lpszValidChars);
				p->Enable(attr->GetUserModify() ? TRUE : FALSE);
				pGroup1->AddSubItem(p);
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeByte
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeByte)
			{
				LPCTSTR lpszValidChars = L"0123456789";
				::std::wstring propDesc = attr->GetDescription() + L"\n(byte)";
				propDesc += strRequiredAttribute;
				COleVariant oleType((short)val->Get<::NM::ODB::ODBByte>(), VT_I2); // VT_UI1); GridCtrl does not support byte either  ,
				CMFCPropertyGridProperty *p = new CMFCPropertyGridProperty((*attribIT).c_str(), oleType, propDesc.c_str(), 0, 0, 0, lpszValidChars);
				p->Enable(attr->GetUserModify() ? TRUE : FALSE);
				pGroup1->AddSubItem(p);
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeIPv4
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeIPv4)
			{
				::std::wstring propDesc = attr->GetDescription() + L"\n(IPv4)";
				propDesc += strRequiredAttribute;
				in_addr ipv4Addr= val->Get<::NM::ODB::ODBINADDR>();
				CMFCPropertyGridIPAdressProperty *p = new CMFCPropertyGridIPAdressProperty((*attribIT).c_str(), ipv4Addr, propDesc.c_str());
				p->Enable(attr->GetUserModify() ? TRUE : FALSE);
				pGroup1->AddSubItem(p);
			}
			/************************************************
			*	::NM::ODB::TypeT::TypeTime
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeTime)
			{
				::std::wstring propDesc = attr->GetDescription() + L"\n(dd/mm/yyyy hh:mm:ss)";
				propDesc += strRequiredAttribute;
				LPCTSTR lpszValidChars		= L"0123456789";
				LPCTSTR lpszEditMask		= L"dd dd dddd dd dd dd"; // The mask string
				LPCTSTR lpszInputTemplate	= L"__/__/____ __:__:__"; // A mask template string that specifies the literal characters that can appear at each position in the user input. Use the underscore character ('_') as a character placeholder. The length of the lpszInputTemplate and lpszMask parameter strings must be the same.

				/* Own Custom Dervied from CMFCMaskedEdit */
				::NM::ODB::ODBTime tmTime = val->Get<::NM::ODB::ODBTime>();
				tmTime.tm_isdst = -1;
				//tmTime.tm_year = 70;
				time_t t = mktime(&tmTime);
				COleDateTime v1(t);
				assert(v1);
				CMFCPropertyGridDateTimeEditProperty *p = new CMFCPropertyGridDateTimeEditProperty((*attribIT).c_str(), v1, propDesc.c_str(), 0, lpszEditMask, lpszInputTemplate, lpszValidChars);
				p->Enable(attr->GetUserModify() ? TRUE : FALSE);
				pGroup1->AddSubItem(p);


			}
			/************************************************
			*	::NM::ODB::TypeT::TypeVector
			************************************************/
			else if (attr->GetType() == ::NM::ODB::TypeT::TypeVector)
			{
				CMFCPropertyGridProperty* pGroupVector = new CMFCPropertyGridProperty((*attribIT).c_str());


				::std::wstring propDesc = attr->GetDescription() + L"\n(vector)";
				propDesc += strRequiredAttribute;
				pGroupVector->SetDescription(propDesc.c_str());

				::NM::ODB::ODBVectorUID vec = val->Get<::NM::ODB::ODBVectorUID>();
				::NM::ODB::ODBVectorUID::iterator it = vec.begin();
				::NM::ODB::ODBWString strLookupName;
				while (it != vec.end())
				{
					if (bResolveUIDtoName)
					{
						if (*it != ::NM::ODB::INVALID_OBJECT_UID)
						{
							// get the shortname each of the vector elements (it = OBJECTUID) ::std::vector<ODBUID>
							::NM::ODB::UPVALUE elementValue = _odb->GetValue(*it, L"shortname");
							if (elementValue)
							{
								strLookupName = elementValue->Get<::NM::ODB::ODBWString>();
							}
							else
							{
								strLookupName = L"Not a Database Object UID";
							}
						}
						else
						{
							strLookupName = L"";		// invalid object uid
						}

						CMFCPropertyGridUIDProperty* pVecElement = new CMFCPropertyGridUIDProperty(L"", strLookupName.c_str(), *it, L"");
						pVecElement->AllowEdit(attr->GetUserModify());
						pGroupVector->AddSubItem(pVecElement);
					}					
					//	No UID Resolution Required - Output Value as string.
					else
					{
						// get attr as string
						CMFCPropertyGridUIDProperty* pVecElement = new CMFCPropertyGridUIDProperty(L"", ::NM::ODB::real_uid(*it).GetString().c_str(), *it, L"");
						pVecElement->AllowEdit(attr->GetUserModify());
						pGroupVector->AddSubItem(pVecElement);
					}

					++it;
				}
				pGroupVector->AllowEdit(attr->GetUserModify());
				pGroup1->AddSubItem(pGroupVector);

			}
			/************************************************************************
			*	::NM::ODB::TypeT::TypeString or if not, it will be shown as string.
			*************************************************************************/
			else
			{
				/*  TODO
				*	Should derive a class from CEdit to enable full selection of content on edit start, similar to UID, 
					prob just override OnEdit, call base class to do usual stuff, then do select all text and return.
				*/
				::std::wstring propDesc = attr->GetDescription() + L"\n(string)";
				propDesc += strRequiredAttribute;
				pProp = new CMFCPropertyGridProperty((*attribIT).c_str(), val->GetStringValue().c_str(), propDesc.c_str());
				pProp->AllowEdit(attr->GetUserModify());
				pGroup1->AddSubItem(pProp);
			}

		}

		++attribIT;
	}


#pragma endregion AttributeListParse

	m_wndPropList.MarkModifiedProperties(FALSE);
	m_wndPropList.AddProperty(pGroup1);
	m_wndPropList.SetRedraw(TRUE);
	m_wndPropList.RedrawWindow();
}
/**
*
*
*
*/
void CPropertiesWnd::InitPropList()
{
	SetPropListFont();
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties(FALSE);
}
/**
*
*
*
*/
bool CPropertiesWnd::RegisterClientNotify()
{

	// Selected Objects Notifications
	//::std::unique_ptr<NM::ISelectedObjects> _selectedObjects(reinterpret_cast<::NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
	_selectedObjects->RegisterClientObserver(this);
	SelectedObjectsUpdate();

	// Database Update Notifications
	// get client interafce to service
	::NM::ODB::IDatabaseUpdate* ptr = reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache"));
	_updateCache.reset(ptr);	

	// create require updates list
	::std::vector<::std::wstring> attr_list;
	attr_list.push_back(L"*");
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
*
*
*
*/
void CPropertiesWnd::DatabaseUpdate()
{
	if (_bIgnoreDatabaseUpdates)
	{
		_updateQ->Clear();
		return;
	}

	size_t qSize = _updateQ->QueueSize();
	while (!_updateQ->Empty())
	{
		::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _updateQ->GetNextRecord();

		real_uid uid = record->GetObjectUID();

		if (uid != _currentObjectUID) continue;

		// prob better just to refresh current details rather than worry about trying to
		// just update the one attr in the properties dialog.. 

		// call update
		AddObjectProperty(_currentObjectUID);
		
		// no need to check anything else as we have updated the one and only object we display.
		_updateQ->Clear();
		return;

		//::std::wcout << record->GetObjectAttributeName().c_str() << L" :: objectuid " << real_uid(record->GetObjectUID()).GetString() << L"\n";

	}
	return;
}
/**
*
*
*
*/
LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM wparam, LPARAM lparam)
{
	OutputDebugString(L"\nCPropertiesWnd::OnPropertyChanged");

	// Cast the lparam to a property.
	CMFCPropertyGridProperty * pProperty = (CMFCPropertyGridProperty *)lparam;

	// Get The Database Handle
//	::std::unique_ptr<NM::ODB::IObjectDatabase> _odb(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));

	// Determine which property grid was changed (wparam is the control ID)
	if( wparam == 2)
	{
		// Determine which property was changed.
		::std::wstring attrName = pProperty->GetName();
		::NM::ODB::UPIATTRIBUTE pAttr = _odb->GetAttribute(_currentObjectUID, attrName);

		// Get the property's current value.
		COleVariant v = pProperty->GetValue();
		VARTYPE vt = v.vt;
		::NM::ODB::Value* newValue = nullptr;


		switch (vt)
		{
		case VT_UI4:			// punkVal (IPADDR)
			OutputDebugString(L"\nVT_UI4");
			switch (pAttr->GetType())
			{
			case ::NM::ODB::TypeT::TypeIPv4:
				{
					::NM::ODB::ODBINADDR ipv4Addr = { 0 };
					ipv4Addr.S_un.S_addr = v.ulVal;
					newValue = new  ::NM::ODB::Value(real_inaddr(ipv4Addr));;
				}
				break;
				
				default:
					throw ::std::logic_error("Unmapped Type for VT_UI4 (CPropertiesWnd::OnPropertyChanged)");
					break;
			}
			break;

		case VT_BOOL:	//(BOOL)		// ODBBool;			// Boolean value is in boolVal.
			OutputDebugString(L"\nVT_BOOL");
			newValue = new ::NM::ODB::Value(real_bool((v.boolVal == VARIANT_TRUE ? 1 : 0)));
			break;

		case VT_BSTR:	//(STRING)		// ODBUID; ODBWString;	// A string was passed; it is stored in bstrVal.		
			OutputDebugString(L"\nVT_BSTR");
			newValue = new ::NM::ODB::Value(real_string(v.bstrVal));
			break;

		case VT_I2:		//(SHORT)			// ODBShort
			switch (pAttr->GetType())
			{
				case ::NM::ODB::TypeT::TypeShort:
					OutputDebugString(L"\nVT_I2");
					newValue = new ::NM::ODB::Value(real_short(v.uiVal));
					break;

				case ::NM::ODB::TypeT::TypeByte:
					newValue = new ::NM::ODB::Value(real_byte(static_cast<::NM::ODB::ODBByte>(v.lVal)));
					break;

				default:
					break;
			}
			break;

		case VT_I4:		//(LONG)		// ODBInt; ODBColor; // A 4-byte integer value is stored in lVal.// LONG                lVal;            // VT_I4
			{								
				OutputDebugString(L"\nVT_I4");
				
				switch (pAttr->GetType())
				{
				case ::NM::ODB::TypeT::TypeInt:
					newValue = new ::NM::ODB::Value(real_int(v.lVal));
					break;

				case ::NM::ODB::TypeT::TypeColor:
					newValue = new ::NM::ODB::Value(real_color((::NM::ODB::ODBColor)v.lVal));
					break;

				case ::NM::ODB::TypeT::TypeLongLong:
					newValue = new ::NM::ODB::Value(real_longlong(v.lVal));
					break;

				case ::NM::ODB::TypeT::TypeDouble:
					newValue = new ::NM::ODB::Value(real_double(v.lVal));

				default:
					break;
				}
			}
			break;

		case VT_DATE:		// ODBTime;
		{
			OutputDebugString(L"\nVT_DATE");
			COleDateTime dt = v.date;
			::NM::ODB::ODBTime odbTime = { 0 };
			odbTime.tm_isdst = -1;
			odbTime.tm_year = dt.GetYear() - 1900;
			odbTime.tm_mon = dt.GetMonth() - 1;
			odbTime.tm_mday = dt.GetDay();
			odbTime.tm_hour = dt.GetHour();
			odbTime.tm_min = dt.GetMinute();
			odbTime.tm_sec = dt.GetSecond();
			// Date value is in date.
			newValue = new ::NM::ODB::Value(real_time(odbTime));
		}
			break;


		//case VT_R8:			// ODBDouble;
		//	OutputDebugString(L"\nVT_R8");
		//	newValue = new ::NM::ODB::Value(real_double(v.dblVal));
		//	break;

		//case VT_UI1:		// ODBByte;
		//	OutputDebugString(L"\nVT_UI1");
		//	newValue = new ::NM::ODB::Value(real_byte(v.bVal));
		//	break;

		case VT_UI8:		// ODBLongLong;;
			OutputDebugString(L"\nVT_UI8");
			newValue = new ::NM::ODB::Value(real_longlong(v.ullVal));
			break;

		default:
			throw ::std::logic_error("\nCPropertiesWnd::OnPropertyChanged: Unmapped Variant - should not be the case");
			break;
		}

		// Set the new value.
		if (newValue)
		{
			_bIgnoreDatabaseUpdates = true;
			if (!_odb->SetValue(_currentObjectUID, attrName, *newValue))
			{
				pProperty->ResetOriginalValue();
			}
			delete newValue;
			_bIgnoreDatabaseUpdates = false;
		}
		
	}

	return 1;
}
















































/*pProp->AddOption(_T("None"));
pProp->AddOption(_T("Thin"));
pProp->AddOption(_T("Resizable"));
pProp->AddOption(_T("Dialog Frame"));
pProp->AllowEdit(FALSE);*/
/*CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("Window Size"), 0, TRUE);

pProp = new CMFCPropertyGridProperty(_T("Height"), (_variant_t) 250l, _T("Specifies the window's height"));
pProp->EnableSpinControl(TRUE, 50, 300);
pSize->AddSubItem(pProp);

pProp = new CMFCPropertyGridProperty( _T("Width"), (_variant_t) 150l, _T("Specifies the window's width"));
pProp->EnableSpinControl(TRUE, 50, 200);
pSize->AddSubItem(pProp);

m_wndPropList.AddProperty(pSize);

CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("Font"));

LOGFONT lf;
CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
font->GetLogFont(&lf);

_tcscpy_s(lf.lfFaceName, _T("Arial"));

pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(_T("Font"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("Specifies the default font for the window")));
pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("Use System Font"), (_variant_t) true, _T("Specifies that the window uses MS Shell Dlg font")));

m_wndPropList.AddProperty(pGroup2);

CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("Misc"));
pProp = new CMFCPropertyGridProperty(_T("(Name)"), _T("Application"));
pProp->Enable(FALSE);
pGroup3->AddSubItem(pProp);

CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Window Color"), RGB(210, 192, 254), NULL, _T("Specifies the default window color"));
pColorProp->EnableOtherButton(_T("Other..."));
pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
pGroup3->AddSubItem(pColorProp);

static const TCHAR szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));

pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));

m_wndPropList.AddProperty(pGroup3);

CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(_T("Hierarchy"));

CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("First sub-level"));
pGroup4->AddSubItem(pGroup41);

CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("Second sub-level"));
pGroup41->AddSubItem(pGroup411);

pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 1"), (_variant_t) _T("Value 1"), _T("This is a description")));
pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 2"), (_variant_t) _T("Value 2"), _T("This is a description")));
pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 3"), (_variant_t) _T("Value 3"), _T("This is a description")));

pGroup4->Expand(FALSE);
m_wndPropList.AddProperty(pGroup4);*/