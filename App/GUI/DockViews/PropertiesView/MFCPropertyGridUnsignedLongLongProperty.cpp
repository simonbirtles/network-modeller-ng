#include "stdafx.h"
#include "MFCPropertyGridUnsignedLongLongProperty.h"

#include <locale.h>   


/**
*
*
*
*/
/////////////////////////////////////////////////////////////////////////////   
// CMFCPropertyGridUnsignedLongLongProperty   

IMPLEMENT_DYNAMIC(CMFCPropertyGridUnsignedLongLongProperty, CMFCPropertyGridProperty)

#define FORMAT_ULL   _T("%llu") //"%I64d"


/**
*
*
*
*/
CMFCPropertyGridUnsignedLongLongProperty::CMFCPropertyGridUnsignedLongLongProperty(
	const CString& strName, 
	unsigned long long varValue, 
	LPCTSTR lpszDescr, 
	DWORD_PTR dwData, 
	LPCTSTR lpszEditMask, 
	LPCTSTR lpszEditTemplate, 
	LPCTSTR lpszValidChars):
	CMFCPropertyGridProperty(strName, ULONGLONG_VARIANT(varValue), lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
{
}
/**
*
*
* 
*/
CString CMFCPropertyGridUnsignedLongLongProperty::FormatProperty()
{
	ASSERT(m_varValue.vt == VT_UI8);
	COleVariant& var = m_varValue;	
	setlocale(LC_ALL, "");
	CString strVal;
	strVal.Format(FORMAT_ULL, var.ullVal);
	return strVal;
}
/**
*
*
*
*/

BOOL CMFCPropertyGridUnsignedLongLongProperty::TextToVar(const CString& strText)
{
	ASSERT(m_varValue.vt == VT_UI8);
	CString strVal = strText;
	unsigned long long ullVal = 0;

	strVal.TrimLeft();
	strVal.TrimRight();

	if (!strVal.IsEmpty())
		_stscanf_s(strVal, FORMAT_ULL, &ullVal);

	m_varValue = ullVal;
	return TRUE;
}
/**
*
*
*
*/
CWnd* CMFCPropertyGridUnsignedLongLongProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	ASSERT(m_varValue.vt == VT_UI8);
	//switch (m_varValue.vt)
	//{
	//case VT_BSTR:
	//case VT_R4:
	//case VT_R8:
	//case VT_UI1:
	//case VT_I2:
	//case VT_INT:
	//case VT_UINT:
	//case VT_I4:
	//case VT_UI2:
	//case VT_UI4:
	//case VT_BOOL:
	//	break;
	//default:
	//	if (!m_bIsValueList)
	//	{
	//		return NULL;
	//	}
	//}

	CEdit* pWndEdit = NULL;

	if (!m_strEditMask.IsEmpty() || !m_strEditTempl.IsEmpty() || !m_strValidChars.IsEmpty())
	{
		CMFCMaskedEdit* pWndEditMask = new CMFCMaskedEdit;
		pWndEditMask->EnableSetMaskedCharsOnly(FALSE);
		pWndEditMask->EnableGetMaskedCharsOnly(FALSE);

		if (!m_strEditMask.IsEmpty() && !m_strEditTempl.IsEmpty())
		{
			pWndEditMask->EnableMask(m_strEditMask, m_strEditTempl, _T(' '));
		}

		if (!m_strValidChars.IsEmpty())
		{
			pWndEditMask->SetValidChars(m_strValidChars);
		}

		pWndEdit = pWndEditMask;
	}
	else
	{
		pWndEdit = new CEdit;
	}

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL;

	if (!m_bEnabled || !m_bAllowEdit)
	{
		dwStyle |= ES_READONLY;
	}

	pWndEdit->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);

	bDefaultFormat = TRUE;
	return pWndEdit;
}
/**
*
*
*
*/
//BOOL CMFCPropertyGridUnsignedLongLongProperty::OnUpdateValue()
//{
//	ASSERT_VALID(this);
//	ASSERT_VALID(m_pWndInPlace);
//	ASSERT_VALID(m_pWndList);
//	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));
//
//	COleDateTime lCurrValue = COleDateTime(m_varValue.date);
//
//	CDateTimeCtrl* pProp = (CDateTimeCtrl*)m_pWndInPlace;
//
//	COleDateTime datetime;
//	pProp->GetTime(datetime);
//	m_varValue.date = datetime;
//
//	if (lCurrValue != COleDateTime(m_varValue.date))
//		m_pWndList->OnPropertyChanged(this);
//
//	return TRUE;
//}
/**
*
*
*
*/
BOOL CMFCPropertyGridUnsignedLongLongProperty::IsValueChanged() const
{
	ASSERT_VALID(this);

	if (m_varValueOrig.vt != m_varValue.vt)
		return FALSE;

	const COleDateTime var(m_varValue);
	const COleDateTime var1(m_varValueOrig);

	if (m_varValue.vt == VT_DATE)
		return var != var1;

	return FALSE;
}

/**
*
*
*
*/
//CString CMFCPropertyGridUnsignedLongLongProperty::FormatOriginalProperty()
//{
//	CString strVal;
//	if (m_format.IsEmpty())
//	{
//		setlocale(LC_ALL, "");
//		strVal = COleDateTime(m_varValueOrig).Format(m_style ? _T("%x") : _T("%X"));
//	}
//	else
//		strVal = COleDateTime(m_varValueOrig).Format(m_format);
//
//	return strVal;
//}