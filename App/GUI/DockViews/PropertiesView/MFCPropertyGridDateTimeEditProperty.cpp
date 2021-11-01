#include "stdafx.h"
#include "MFCPropertyGridDateTimeEditProperty.h"


// CMFCPropertyGridDateTimeEditProperty
///////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMFCPropertyGridDateTimeEditProperty, CMFCPropertyGridProperty)
/**
*
*
*
*/
CMFCPropertyGridDateTimeEditProperty::CMFCPropertyGridDateTimeEditProperty(
	const CString& strName,
	COleDateTime &nValue,
	LPCTSTR lpszDescr,
	DWORD_PTR dwData,
	LPCTSTR lpszEditMask,
	LPCTSTR lpszEditTemplate,
	LPCTSTR lpszValidChars
	) :
	CMFCPropertyGridProperty(strName, nValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars),
	_lpszEditMask(lpszEditMask),
	_lpszEditTemplate(lpszEditTemplate),
	_lpszValidChars(lpszValidChars)
{
}
/**
*
*
*
*/
CWnd* CMFCPropertyGridDateTimeEditProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	CPropDateTimeEditCtrl* pControlWnd = new CPropDateTimeEditCtrl(this, m_pWndList->GetBkColor());

	//rectEdit.InflateRect(4, 2, 0, 3);
	//pControlWnd->SetFont(m_pWndList->GetFont(), FALSE);
	//pControlWnd->EnableSetFont(FALSE);

	DWORD style = WS_VISIBLE | WS_CHILD;
	if (!m_bEnabled || !m_bAllowEdit)
	{
		style |= ES_READONLY;
	}


	pControlWnd->EnableSetMaskedCharsOnly(FALSE);
	pControlWnd->EnableGetMaskedCharsOnly(FALSE);
	TCHAR it = { L'_' };
	pControlWnd->EnableMask(_lpszEditMask, _lpszEditTemplate);
	pControlWnd->SetValidChars(_lpszValidChars);

	pControlWnd->Create(style, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);	

	LPCTSTR text = FormatProperty();
	pControlWnd->SetWindowText(text);
	




	bDefaultFormat = TRUE;
	return pControlWnd;
}
/**
*
*
*
*/
BOOL CMFCPropertyGridDateTimeEditProperty::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	CPropDateTimeEditCtrl* pProp = (CPropDateTimeEditCtrl*)m_pWndInPlace;

	// Save Current Value
	COleDateTime dtCurrent = m_varValue.date;

	// Get New Value into m_varValue.date
	CString strDateTime;
	COleDateTime dtNew;

	pProp->GetWindowTextW(strDateTime);
	if (!dtNew.ParseDateTime(strDateTime))
		return FALSE;

	m_varValue.date = dtNew;

	// if changed call OnUpdateProperty
	if (dtCurrent != dtNew)
		m_pWndList->OnPropertyChanged(this);

	return TRUE;
}
/**
*
*
*
*/
CString CMFCPropertyGridDateTimeEditProperty::FormatProperty()
{
	CString strVal;
	COleDateTime dt = m_varValue.date;
	strVal = dt.Format(L"%d/%m/%Y %H:%M:%S");
	return strVal;
}
/**
*
*
*
*/
CString CMFCPropertyGridDateTimeEditProperty::FormatOriginalProperty()
{
	CString strVal;
	COleDateTime dt = m_varValue.date;
	strVal = dt.Format(L"%d/%m/%Y %H:%M:%S");
	return strVal;
}


//
///////////////////////////////////////////////////////////////////////////////////

CPropDateTimeEditCtrl::CPropDateTimeEditCtrl(CMFCPropertyGridDateTimeEditProperty *pProp, COLORREF clrBack):
	CMFCMaskedEdit()
{
	m_clrBack = clrBack;
	m_brBackground.CreateSolidBrush(m_clrBack);
	m_pProp = pProp;
	m_bSetFont = TRUE;
}

BEGIN_MESSAGE_MAP(CPropDateTimeEditCtrl, CMFCMaskedEdit)
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_HSCROLL_REFLECT()
	//ON_WM_CREATE()
END_MESSAGE_MAP()

void CPropDateTimeEditCtrl::OnKillFocus(CWnd* pNewWnd)
{
	if (pNewWnd != NULL && IsChild(pNewWnd))
		return;
	if (pNewWnd == this)
		return;
	CMFCMaskedEdit::OnKillFocus(pNewWnd);
}

void CPropDateTimeEditCtrl::OnDestroy()
{
	m_bSetFont;
	SetFont(NULL, FALSE);
	//CWnd *editCtrl = NULL, *editTmp = NULL;
	//editCtrl = GetWindow(GW_CHILD);
	//while (editCtrl)
	//{
	//	editTmp = editCtrl;
	//	editCtrl = editCtrl->GetWindow(GW_HWNDNEXT);
	//	//      editTmp->UnsubclassWindow();   
	//	editTmp->DestroyWindow();
	//}

	CMFCMaskedEdit::OnDestroy();
}

HBRUSH CPropDateTimeEditCtrl::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetBkColor(m_clrBack);
	return m_brBackground;
}

void CPropDateTimeEditCtrl::HScroll(UINT /*nSBCode*/, UINT /*nPos*/)
{
	ASSERT_VALID(m_pProp);

	m_pProp->OnUpdateValue();
	m_pProp->Redraw();
}

//LRESULT CPropDateTimeEditCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
//{
//	//if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
//	//{
//	//	POINT pt;
//	//	GetCursorPos(&pt);
//	//	CWnd *edit = NULL;
//	//	CRect rect;
//	//	edit = GetWindow(GW_CHILD);
//	//	while (edit)
//	//	{
//	//		edit->GetWindowRect(rect);
//	//		if (rect.PtInRect(pt))
//	//		{
//	//			edit->SendMessage(message, wParam, lParam);
//	//			return TRUE;
//	//		}
//	//		edit = edit->GetWindow(GW_HWNDNEXT);
//	//	}
//	//}
//	//if (message == WM_SETFONT && !m_bSetFont)
//	//	return TRUE;
//
//	return CMFCMaskedEdit::WindowProc(message, wParam, lParam);
//}

//int CPropDateTimeEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	if (CMFCMaskedEdit::OnCreate(lpCreateStruct) == -1)
//		return -1;
//
//	//int i = 0;
//	//CWnd *edit = NULL;
//	//edit = GetWindow(GW_CHILD);
//	//while (edit && i<4)
//	//{
//	//	m_IPEdit[i++].SubclassWindow(edit->GetSafeHwnd());
//	//	edit = edit->GetWindow(GW_HWNDNEXT);
//	//}
//
//	return 0;
//}
