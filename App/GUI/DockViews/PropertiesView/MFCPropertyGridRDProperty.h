#pragma once
#include "MFCPropertyGridUnsignedLongLongProperty.h"

/**
*
*
*   *  VT_UI8                 [T][P]     unsigned 64-bit int
*/
class CMFCPropertyGridRDProperty : public CMFCPropertyGridUnsignedLongLongProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridRDProperty)

public:
	//CMFCPropertyGridUnsignedLongLongProperty(const CString& strName, unsigned long long &nValue, LPCTSTR lpszDescr = NULL, DWORD dwData = 0, BOOL style = TRUE, BOOL updown = FALSE, LPCTSTR setFormat = NULL, LPCTSTR format = NULL);
	CMFCPropertyGridRDProperty(const CString& strName, unsigned long long varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
														LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL);

	virtual CString FormatProperty();
	virtual BOOL TextToVar(const CString& strText);
	//virtual BOOL PushChar(UINT nChar);

	//virtual BOOL OnUpdateValue();
	//virtual CString FormatProperty();	
	//virtual CString FormatOriginalProperty();

protected:
	virtual CWnd* CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat);
	virtual BOOL OnSetCursor() const { return FALSE; /* Use default */ }
	BOOL IsValueChanged() const;
	//virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);

};