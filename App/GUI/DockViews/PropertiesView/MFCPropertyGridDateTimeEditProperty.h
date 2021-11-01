#pragma once
#include "afxpropertygridctrl.h"

class CMFCPropertyGridDateTimeEditProperty;

/**
*
*
*
*/
class CPropDateTimeEditCtrl : public CMFCMaskedEdit
{
public:
	CPropDateTimeEditCtrl(CMFCPropertyGridDateTimeEditProperty *pProp, COLORREF clrBack);

protected:
	CBrush m_brBackground;
	COLORREF m_clrBack;
	CMFCPropertyGridDateTimeEditProperty *m_pProp;
	BOOL m_bSetFont;

public:
	void EnableSetFont(BOOL bSetFont = TRUE) { m_bSetFont = bSetFont; }
	DECLARE_MESSAGE_MAP()

	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void HScroll(UINT nSBCode, UINT nPos);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDestroy();
protected:
	//virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	//afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

/**
*
*
*
*/
class CMFCPropertyGridDateTimeEditProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridDateTimeEditProperty)

public:
	CMFCPropertyGridDateTimeEditProperty(
		const CString& strName, 
		COleDateTime &nValue, 
		LPCTSTR lpszDescr,
		DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL,
		LPCTSTR lpszEditTemplate = NULL,
		LPCTSTR lpszValidChars = NULL
		);

	virtual BOOL OnUpdateValue();
	virtual CString FormatProperty();
	virtual CString FormatOriginalProperty();

protected:
	virtual CWnd* CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat);
	virtual BOOL OnSetCursor() const { return FALSE; /* Use default */ }

private:
	LPCTSTR _lpszEditMask;
	LPCTSTR _lpszEditTemplate;
	LPCTSTR _lpszValidChars;
};
