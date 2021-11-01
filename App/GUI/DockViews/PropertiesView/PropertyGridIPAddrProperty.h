#pragma once
#include "afxpropertygridctrl.h"
#include <Winsock2.h> 



class CMFCPropertyGridIPAdressProperty;


class CNotifyEdit : public CEdit
{
	// Construction 
public:
	CNotifyEdit();
	virtual ~CNotifyEdit();
public:
	DECLARE_MESSAGE_MAP()
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class CPropIPAddressCtrl : public CIPAddressCtrl
{
	// Construction 
public:
	CPropIPAddressCtrl(CMFCPropertyGridIPAdressProperty *pProp, COLORREF clrBack);

protected:
	CNotifyEdit m_IPEdit[4];
	CBrush m_brBackground;
	COLORREF m_clrBack;
	CMFCPropertyGridIPAdressProperty *m_pProp;
	BOOL m_bSetFont;

public:
	void EnableSetFont(BOOL bSetFont = TRUE) { m_bSetFont = bSetFont; }
	DECLARE_MESSAGE_MAP()

	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void HScroll(UINT nSBCode, UINT nPos);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnNcPaint();
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

struct ULONG_VARIANT : public VARIANT
{
	ULONG_VARIANT(ULONG val)
	{
		vt = VT_UI4;
		ulVal = val;
	}
};

class CMFCPropertyGridIPAdressProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridIPAdressProperty)

public:
	CMFCPropertyGridIPAdressProperty(const CString& strName, in_addr &nValue, LPCTSTR lpszDescr = NULL, DWORD dwData = 0);

	virtual BOOL OnUpdateValue();
	virtual CString FormatProperty();
	virtual CString FormatOriginalProperty();

protected:
	virtual CWnd* CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat);
	virtual BOOL OnSetCursor() const { return FALSE; /* Use default */ }
};
