#include "stdafx.h"
#include "PropertyGridIPAddrProperty.h"

#include <Ws2tcpip.h>
#include <string>


/////////////////////////////////////////////////////////////////////////////   
// CNotifyEdit   

CNotifyEdit::CNotifyEdit()
{
}

CNotifyEdit::~CNotifyEdit()
{
}

BEGIN_MESSAGE_MAP(CNotifyEdit, CEdit)
END_MESSAGE_MAP()

LRESULT CNotifyEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_KILLFOCUS)
		GetParent()->PostMessage(message, wParam, lParam);

	return CEdit::WindowProc(message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////   
// CMFCPropertyGridIPAdressProperty   

IMPLEMENT_DYNAMIC(CMFCPropertyGridIPAdressProperty, CMFCPropertyGridProperty)

CMFCPropertyGridIPAdressProperty::CMFCPropertyGridIPAdressProperty(const CString& strName, in_addr &nValue, LPCTSTR lpszDescr, DWORD dwData) :
	CMFCPropertyGridProperty(strName, ULONG_VARIANT(nValue.s_addr), lpszDescr, dwData)
{
}

CWnd* CMFCPropertyGridIPAdressProperty::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	CPropIPAddressCtrl* pWndIPAddress = new CPropIPAddressCtrl(this, m_pWndList->GetBkColor());

	
	rectEdit.InflateRect(4, 2, 0, 3);
	// only want 100 wide 
	rectEdit.right = (rectEdit.left + 100);
	//rectEdit.InflateRect(0, 0, 0, 0);

	DWORD style = WS_VISIBLE | WS_CHILD;

	pWndIPAddress->Create(style, rectEdit, m_pWndList, AFX_PROPLIST_ID_INPLACE);
	pWndIPAddress->SetFont(m_pWndList->GetFont(), FALSE);
	pWndIPAddress->EnableSetFont(FALSE);
	pWndIPAddress->SetAddress(m_varValue.ulVal);

	bDefaultFormat = TRUE;
	return pWndIPAddress;
}

BOOL CMFCPropertyGridIPAdressProperty::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	in_addr lCurrValue;
	lCurrValue.s_addr = m_varValue.ulVal;

	CPropIPAddressCtrl* pProp = (CPropIPAddressCtrl*)m_pWndInPlace;

	pProp->GetAddress(m_varValue.ulVal);
	m_varValue.ulVal = ntohl(m_varValue.ulVal);

	if (lCurrValue.s_addr != m_varValue.ulVal)
		m_pWndList->OnPropertyChanged(this);

	return TRUE;
}

CString CMFCPropertyGridIPAdressProperty::FormatProperty()
{
	CString strVal;
	::std::string strdefaultaddr = "0.0.0.0";

	in_addr address;
	address.s_addr = m_varValue.ulVal;

	wchar_t str[INET_ADDRSTRLEN];
	if (InetNtop(AF_INET, &(address.S_un.S_addr), str, INET_ADDRSTRLEN) != NULL)
	{
		strVal = str;
		return strVal;
	}

	return CString(strdefaultaddr.c_str());
}

CString CMFCPropertyGridIPAdressProperty::FormatOriginalProperty()
{
	CString strVal;
	::std::string strdefaultaddr = "0.0.0.0";

	in_addr address;
	address.s_addr = m_varValueOrig.ulVal;

	wchar_t str[INET_ADDRSTRLEN];
	if (InetNtop(AF_INET, &(address.S_un.S_addr), str, INET_ADDRSTRLEN) != NULL)
	{
		strVal = str;
		return strVal;
	}

	return CString(strdefaultaddr.c_str());
}

/////////////////////////////////////////////////////////////////////////////   
// CPropIPAddressCtrl   

CPropIPAddressCtrl::CPropIPAddressCtrl(CMFCPropertyGridIPAdressProperty *pProp, COLORREF clrBack)
{
	m_clrBack = clrBack;
	m_brBackground.CreateSolidBrush(m_clrBack);
	m_pProp = pProp;
	m_bSetFont = TRUE;
}

BEGIN_MESSAGE_MAP(CPropIPAddressCtrl, CIPAddressCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_HSCROLL_REFLECT()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

void CPropIPAddressCtrl::OnKillFocus(CWnd* pNewWnd)
{
	if (pNewWnd != NULL && IsChild(pNewWnd))
		return;
	if (pNewWnd == this)
		return;
	CIPAddressCtrl::OnKillFocus(pNewWnd);
}

void CPropIPAddressCtrl::OnDestroy()
{
	//m_bSetFont;
	//SetFont(NULL, FALSE);
	CWnd *editCtrl = NULL, *editTmp = NULL;
	editCtrl = GetWindow(GW_CHILD);
	while (editCtrl)
	{
		editTmp = editCtrl;
		editCtrl = editCtrl->GetWindow(GW_HWNDNEXT);
		//      editTmp->UnsubclassWindow();   
		editTmp->DestroyWindow();
	}

	CIPAddressCtrl::OnDestroy();
}

HBRUSH CPropIPAddressCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CIPAddressCtrl::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO: Change any attributes of the DC here

	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CPropIPAddressCtrl::OnNcPaint()    
{
	// we dont do anything here as we dont want any border so by ignoring 
	// no border gets drawn

	CPaintDC PaintDC(this);
}

HBRUSH CPropIPAddressCtrl::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetBkColor(m_clrBack);
	return m_brBackground;
}

void CPropIPAddressCtrl::HScroll(UINT /*nSBCode*/, UINT /*nPos*/)
{
	ASSERT_VALID(m_pProp);

	m_pProp->OnUpdateValue();
	m_pProp->Redraw();
}

LRESULT CPropIPAddressCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
	{
		POINT pt;
		GetCursorPos(&pt);
		CWnd *edit = NULL;
		CRect rect;
		edit = GetWindow(GW_CHILD);
		while (edit)
		{
			edit->GetWindowRect(rect);
			if (rect.PtInRect(pt))
			{
				edit->SendMessage(message, wParam, lParam);
				return TRUE;
			}
			edit = edit->GetWindow(GW_HWNDNEXT);
		}
	}
	if (message == WM_SETFONT && !m_bSetFont)
		return TRUE;

	return CIPAddressCtrl::WindowProc(message, wParam, lParam);
}

int CPropIPAddressCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CIPAddressCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	int i = 0;
	CWnd *edit = NULL;
	edit = GetWindow(GW_CHILD);
	while (edit && i<4)
	{
		m_IPEdit[i++].SubclassWindow(edit->GetSafeHwnd());
		edit = edit->GetWindow(GW_HWNDNEXT);
	}

	return 0;
}
