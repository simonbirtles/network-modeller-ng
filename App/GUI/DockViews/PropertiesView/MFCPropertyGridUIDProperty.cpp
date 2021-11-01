#include "stdafx.h"
#include "MFCPropertyGridUIDProperty.h"
#include "MainFrm.h"

#include "IServiceRegistry.h"						// registry interface
#include "ISelectedObjects.h"

extern NM::Registry::IServiceRegistry* reg;

/**
*
*
*
*/
//CMFCPropertyGridUIDProperty::CMFCPropertyGridUIDProperty(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE):
//	CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
//{
//}
CMFCPropertyGridUIDProperty::CMFCPropertyGridUIDProperty(
	const CString& strName, 
	const COleVariant& varValue, 
	::NM::ODB::ODBUID objectUID,
	LPCTSTR lpszDescr, 
	DWORD_PTR dwData,
	LPCTSTR lpszEditMask , 
	LPCTSTR lpszEditTemplate, 
	LPCTSTR lpszValidChars ):
	CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars),
	_objectUID(objectUID)
{

}
/**
* 
*
* 
*/
CMFCPropertyGridUIDProperty::~CMFCPropertyGridUIDProperty()
{
}
/**
* OnDblClk
*
* When user double clicks on a UID(or resolved UID), update selection to the UID clicked.
*/
BOOL CMFCPropertyGridUIDProperty::OnDblClk(CPoint point)
{
	if (_objectUID == ::NM::ODB::INVALID_OBJECT_UID) return TRUE;

	ClickArea pnArea = ClickArea::ClickDescription;
	HitTest(point, &pnArea);
	if (pnArea == ClickArea::ClickValue)
	{
		::std::unique_ptr<NM::ISelectedObjects> selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
		selectedObjects->SetMultiSelect(false);
		selectedObjects->Select(_objectUID);
	}
	return TRUE;
}
