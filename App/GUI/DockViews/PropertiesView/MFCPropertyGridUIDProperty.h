#pragma once
#include "afxpropertygridctrl.h"
#include "Interfaces\ObjectDatabaseDefines.h"

class CMFCPropertyGridUIDProperty :	public CMFCPropertyGridProperty
{
public:
	
	//CMFCPropertyGridUIDProperty(const CString& strGroupName, DWORD_PTR dwData, BOOL bIsValueList );

	CMFCPropertyGridUIDProperty(
		const CString& strName,
		const COleVariant& varValue,
		::NM::ODB::ODBUID objectUID,
		LPCTSTR lpszDescr = NULL, 
		DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, 
		LPCTSTR lpszEditTemplate = NULL, 
		LPCTSTR lpszValidChars = NULL);

	virtual ~CMFCPropertyGridUIDProperty();

	BOOL OnDblClk(CPoint point);

private:
	::NM::ODB::ODBUID _objectUID;
};

