#pragma once
#include "afxpropertygridctrl.h"

struct ULONGLONG_VARIANT : public VARIANT
{
	ULONGLONG_VARIANT(unsigned long long val)
	{
		vt = VT_UI8;
		ullVal = val;
	}
};

/**
*
*
*   *  VT_UI8                 [T][P]     unsigned 64-bit int
*/
class CMFCPropertyGridUnsignedLongLongProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridUnsignedLongLongProperty)


public:
	//CMFCPropertyGridUnsignedLongLongProperty(const CString& strName, unsigned long long &nValue, LPCTSTR lpszDescr = NULL, DWORD dwData = 0, BOOL style = TRUE, BOOL updown = FALSE, LPCTSTR setFormat = NULL, LPCTSTR format = NULL);
	CMFCPropertyGridUnsignedLongLongProperty(const CString& strName, unsigned long long varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
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