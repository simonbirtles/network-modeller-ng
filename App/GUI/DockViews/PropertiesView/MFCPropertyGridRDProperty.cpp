#include "stdafx.h"
#include "MFCPropertyGridRDProperty.h"
#include "MFCRDEdit.h"

#include <locale.h>   


/**
*
*
*
*/
/////////////////////////////////////////////////////////////////////////////   
// CMFCPropertyGridUnsignedLongLongProperty   

IMPLEMENT_DYNAMIC(CMFCPropertyGridRDProperty, CMFCPropertyGridUnsignedLongLongProperty)

#define FORMAT_ULL   _T("%llu") //"%I64d"
#define FORMAT_USHORT _T("%u")
#define FORMAT_ULONG  _T("%u")

#define RDPAIRTOLONGT0(asn,value)		( (unsigned long long)          (   ((unsigned long long)(0) << 48)    +    (((unsigned long long)((asn) & 0xffff)) << 32)    +     ((unsigned long)((value) & 0xffffffff))   ) )
#define RDLONGTOTYPE(rd)				( (unsigned short)    (   ((unsigned long long)(rd) >> 48) & 0xffff					) )
#define RDLONGTOASNT0(rd)				( (unsigned short)    (   ((unsigned long long)(rd) >> 32) & 0xffff					) )
#define RDLONGTOVALUET0(rd)				( (unsigned long)     (   ((unsigned long long)(rd) & 0xffffffff)					) )

/**
*
*
*
*/
CMFCPropertyGridRDProperty::CMFCPropertyGridRDProperty(
	const CString& strName, 
	unsigned long long varValue, 
	LPCTSTR lpszDescr, 
	DWORD_PTR dwData, 
	LPCTSTR lpszEditMask, 
	LPCTSTR lpszEditTemplate, 
	LPCTSTR lpszValidChars):
	CMFCPropertyGridUnsignedLongLongProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
{

}
/**
* FormatProperty
*
* Value -> String, returns "aa..:vv.."
*
* http://packetlife.net/blog/2013/jun/10/route-distinguishers-and-route-targets/
*/
CString CMFCPropertyGridRDProperty::FormatProperty()
{
	OutputDebugString(L"\nCMFCPropertyGridRDProperty::FormatProperty()");
	ASSERT(m_varValue.vt == VT_UI8);
	COleVariant& var = m_varValue;	

	setlocale(LC_ALL, "");
	CString strVal;

	// switch type 
	switch (RDLONGTOTYPE(var.ullVal))
	{

	case 0:
	{
		OutputDebugString(L"\n\t Format Type 0(x:X)\t");

		CString strASN;
		CString strValue;
		/*if (m_bInPlaceEdit)
		{
			strASN.Format(_T("%05d"), RDLONGTOASNT0(var.ullVal));
			strValue.Format(_T("%010ld"), RDLONGTOVALUET0(var.ullVal));
		}
		else*/
		{
			strASN.Format(FORMAT_USHORT, RDLONGTOASNT0(var.ullVal));				// CHG: removed spce 5
			strValue.Format(FORMAT_ULONG, RDLONGTOVALUET0(var.ullVal));		// CHG: removed space 10
		}
		//strVal = strASN + strValue;
		strVal = strASN + L":" + strValue;
		OutputDebugString(strVal);
	}
	break;

	case 1:
		ASSERT(FALSE);
		break;

	case 2:
		ASSERT(FALSE);
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return strVal;
}
/**
* TextToVar
*
* String -> Value
*
*/

BOOL CMFCPropertyGridRDProperty::TextToVar(const CString& strText)
{
	OutputDebugString(L"\nCMFCPropertyGridRDProperty::TextToVar");

	ASSERT(m_varValue.vt == VT_UI8);
	CString strVal = strText;
	unsigned long long ullVal = 0;

	strVal.TrimLeft();
	strVal.TrimRight();
	
	int posDelim = strVal.Find(L":");
	if (posDelim == -1) return FALSE;

	CString strasn = strVal.Mid(0, posDelim);
	CString strvalue = strVal.Mid(posDelim+1);

	if (!strasn.IsEmpty() && !strvalue.IsEmpty())
	{
		unsigned long tmpASN;
		unsigned long long tmpVALUE;
		_stscanf_s(strasn.Mid(0, 5), FORMAT_ULONG, &tmpASN);
		_stscanf_s(strvalue, FORMAT_ULL, &tmpVALUE);

		if (tmpASN > 65535) return FALSE;
		if (tmpVALUE > pow(2, 32)) return FALSE;

		ullVal = RDPAIRTOLONGT0((unsigned short)tmpASN, (unsigned long)tmpVALUE);		
	}


	m_varValue = ullVal;
	return TRUE;
}
/**
*
*
*
*/
CWnd* CMFCPropertyGridRDProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	OutputDebugString(L"\nCMFCPropertyGridRDProperty::CreateInPlaceEdit");
	ASSERT(m_varValue.vt == VT_UI8);

	CMFCRDEdit* pWndEditMask = new CMFCRDEdit;
	//pWndEditMask->EnableSetMaskedCharsOnly(FALSE);
	//pWndEditMask->EnableGetMaskedCharsOnly(FALSE);
	//pWndEditMask->EnableMask(m_strEditMask, m_strEditTempl, _T(' '));		// CHG
	//pWndEditMask->SetValidChars(m_strValidChars);


	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL;
	if (!m_bEnabled || !m_bAllowEdit)
		dwStyle |= ES_READONLY;

	pWndEditMask->Create(dwStyle, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);

	CString text = FormatProperty();
	pWndEditMask->SetWindowText(text);

	bDefaultFormat = TRUE;
	return pWndEditMask;
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
BOOL CMFCPropertyGridRDProperty::IsValueChanged() const
{
	OutputDebugString(L"\nCMFCPropertyGridRDProperty::IsValueChanged()");
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