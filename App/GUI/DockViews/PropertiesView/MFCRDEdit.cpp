
#include "stdafx.h"
#include "afxcontrolbarutil.h"
#include "mfcrdedit.h"
#include "afxtagmanager.h"
#include "afxctrlcontainer.h"

extern void WCtMB(const ::std::wstring &ws, ::std::string &mbs);
extern void MBtWC(::std::wstring &ws, const ::std::string &mbs);


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCRDEdit

IMPLEMENT_DYNAMIC(CMFCRDEdit, CEdit)

BEGIN_MESSAGE_MAP(CMFCRDEdit, CEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SETTEXT()
	//ON_WM_LBUTTONUP()
	ON_WM_CREATE()
END_MESSAGE_MAP()

CMFCRDEdit::CMFCRDEdit()
{
	m_bSetTextProcessing = FALSE;
	m_strValidNumbers = "0123456789";
	m_strValidDelim = ":";
	//m_iDelimPos = -1;
}

CMFCRDEdit::~CMFCRDEdit()
{
}



//void CMFCRDEdit::OnLButtonUp(UINT nFlags, CPoint point)
//{
//	if (m_bSelectByGroup)
//	{
//		// Calc group bounds
//		int nGroupStart, nGroupEnd;
//		CEdit::GetSel(nGroupStart, nGroupEnd);
//		GetGroupBounds(nGroupStart, nGroupEnd, nGroupStart, TRUE);
//		if (nGroupStart == -1)
//		{
//			CEdit::GetSel(nGroupStart, nGroupEnd);
//			GetGroupBounds(nGroupStart, nGroupEnd, nGroupStart, FALSE);
//		}
//
//		// Correct selection
//		int nStart, nEnd;
//		CEdit::GetSel(nStart, nEnd);
//
//		int nNewStart = max(nStart, nGroupStart);
//		int nNewEnd = min(nEnd, nGroupEnd);
//		// additional
//		nNewStart = min(nNewStart, nGroupEnd);
//		nNewEnd = max(nNewEnd, nGroupStart);
//		if ((nNewEnd != nEnd) || (nNewStart != nStart))
//		{
//			CEdit::SetSel(nNewStart, nNewEnd);
//		}
//	}
//
//	CEdit::OnLButtonUp(nFlags, point);
//}

void CMFCRDEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int nStartChar, nEndChar;
	GetSel(nStartChar, nEndChar);

	if (nChar == VK_DELETE)
	{
		//OnCharDelete(nChar, nRepCnt, nFlags);
		//CEdit::OnChar(nChar, nRepCnt, nFlags);
		return;
	}
		
	if(nChar == VK_BACK)
	{
		//OnCharBackspace(nChar, nRepCnt, nFlags);
		//CEdit::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	if ((nStartChar > 16) || (m_str.size() >= 16))
	{
		return;
	}





	// single char selected
	if (nStartChar == nEndChar)
	{
		// if at max delim pos, and no delim present, put one in
		if ((nStartChar == 5) && (GetDelimPosition() == -1))
		{
			ReplaceSel(L":");
			SetCharAt(':', nStartChar);
			++nStartChar;
			++nEndChar;
		}

		// if on the delim, then hop over
		if((nStartChar == GetDelimPosition()) && GetDelimPosition() == 5)
		{
			++nStartChar; 
			++nEndChar;
			SetSel(nStartChar, nEndChar);
			// if user entered delim char do nothing
			if (m_strValidDelim.find(nChar) != -1)
			{
				return;
			}
		}

		int iUsed, iFree, iMax;
		GetGroupBounds(iUsed, iFree, iMax, nStartChar);
		OutputDebugString(L"\nBounds:Used\t");
		OutputDebugString(::std::to_wstring(iUsed).c_str());
		OutputDebugString(L"\tFree\t");
		OutputDebugString(::std::to_wstring(iFree).c_str());
		OutputDebugString(L"\tMax\t");
		OutputDebugString(::std::to_wstring(iMax).c_str());
		OutputDebugString(L"\tnPos\t");
		OutputDebugString(::std::to_wstring(nStartChar).c_str());

		if (m_strValidNumbers.find(nChar, 0) != -1)
		{
			// does current group have space for addtional char
			if (iFree == 0)
				return;
			
			
			if (!SetCharAt(nChar, nStartChar))
				return;


			CEdit::OnChar(nChar, nRepCnt, nFlags);
			OutputDebugString(L"\nOnChar\t");
			OutputDebugStringA(m_str.c_str());
			return;
		}
		
		if (m_strValidDelim.find(nChar, 0) != -1)
		{
			int delimPos = GetDelimPosition();
			if (delimPos != -1)
				return;
				//m_str.erase(delimPos, 1);

			SetCharAt(nChar, nStartChar);
			CEdit::OnChar(nChar, nRepCnt, nFlags);
			OutputDebugString(L"\nOnChar\t");
			OutputDebugStringA(m_str.c_str());
		}


	}
	/*if((m_strValidDelim.Find(nChar, 0) != -1) && (m_iDelimPos == -1))
	{
		if ((nStartChar > 0) && (nEndChar < 5))
		{
			m_iDelimPos = nStartChar;
			CEdit::OnChar(nChar, nRepCnt, nFlags);
		}
		return;
	}*/

}
/**
*
*
*
*/

void CMFCRDEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//MessageBeep((UINT)-1);

	switch (nChar)
	{
	//case VK_END:
	//	//CEdit::SetSel(nGroupEnd, nGroupEnd);
	//	//return;

	//case VK_HOME:
	//	//CEdit::SetSel(nGroupStart, nGroupStart);
	//	//return;

	//case VK_UP:
	//case VK_LEFT:
	//	//CEdit::SetSel(nStart - 1, nEnd);
	//	//return;

	//case VK_DOWN:
	//case VK_RIGHT:
	//	//break;

	case VK_BACK:
		OnCharBackspace(nChar, nRepCnt, nFlags);
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
		break;

	case VK_DELETE:
		if (::GetKeyState(VK_SHIFT) & 0x80)
		{
			break;
		}
		OnCharDelete(nChar, nRepCnt, nFlags);
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
		break;

	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
/**
*
*
*
*/
void CMFCRDEdit::OnCharBackspace(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_BACK)
	{
		int nStartChar, nEndChar;
		GetSel(nStartChar, nEndChar);
		if((nStartChar == 0) && nEndChar ==0) return;
		CString strText;
		GetWindowText(strText);
		//for (int iPos = nEndChar-1; iPos >= nStartChar; --iPos)
		//{
		//	if (m_strValidDelim.find(strText[iPos]) != -1)
		//	{
		//		m_iDelimPos = -1;
		//	}			
		//}


		// delim test after delete, if we deleted during this, put one back in between the none deleted pieces

		m_str.erase(max(0, nStartChar-1), (nEndChar - nStartChar)+1);
		SetSel(max(0, nStartChar-1), nEndChar);
		CEdit::ReplaceSel(L"");
		SetSel(max(0, nStartChar - 1), max(0, nStartChar - 1));
		OutputDebugString(L"\nDeleteBack\n\t");
		OutputDebugStringA(m_str.c_str());
		OutputDebugString(L"\t");
		GetWindowText(strText);
		OutputDebugString(strText);

	}

	return;
}
/**
*
*
*
*/
void CMFCRDEdit::OnCharDelete(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_DELETE)
	{
		if (m_str.size() == 0) return;
		int nStartChar, nEndChar;
		GetSel(nStartChar, nEndChar);
		CString strText;
		GetWindowText(strText);
		//for (int iPos = nEndChar-1; iPos >= nStartChar; --iPos)
		//{
		//	if (m_strValidDelim.find(strText[iPos]) != -1)
		//	{
		//		m_iDelimPos = -1;
		//	}
		//	//m_str.erase(m_str.begin() + iPos);
		//}

		// delim test after delete, if we deleted during this, put one back in between the none deleted pieces


		m_str.erase(nStartChar, (nEndChar - nStartChar)+1);
		CEdit::ReplaceSel(L"");
		OutputDebugString(L"\nDelete\t");
		OutputDebugStringA(m_str.c_str());
	}
	return;
}
/**
*
*
*
*/
void CMFCRDEdit::GetGroupBounds(int &nUsed, int &nFree, int &nMax, int nPos)
{
	ASSERT(nPos >= 0);

	int delimPos = GetDelimPosition();

	if ((delimPos == -1) && (m_str.size() > 5))
	{
		if (m_str.size() > 15)
		{
			nMax = 16;
			nUsed = 16;
			nFree = 0;
		}
		else
		{
			nMax = 16;
			nUsed = m_str.size();
			nFree = nMax - nUsed;
		}
		return;
	}

	// if no delim, ASN group
	if (delimPos == -1)
	{
		nMax = 5;
		nUsed = m_str.size();
		nFree = nMax - nUsed;
		ASSERT(nUsed <= 5);
		return;
	}

	// if delim, calc group ASN
	if(nPos <= delimPos)
	{
		nMax = 5;
		nFree = nMax - delimPos;
		nUsed = nMax - nFree;
		
		ASSERT(nUsed > -1);
		return;
	}

	// if delim, calc group VALUE
	if (nPos > delimPos)
	{
		nMax = 10;
		nUsed = (m_str.size()-1) - delimPos;
		nFree = nMax - nUsed;
		return;
	}
	

	ASSERT(FALSE);
}
/**
*
*
*
*/
int CMFCRDEdit::OnSetText(LPCTSTR lpszText)
{
	if (m_bSetTextProcessing || lpszText == NULL)
	{
		return (int)Default();
	}

	m_bSetTextProcessing = TRUE;
	::std::string strValue;
	WCtMB(lpszText, strValue);
	
	if (SetValue(strValue))
	{
		LRESULT lRes = FALSE;
		// validated new value should differ from lParam
		OutputDebugString(L"\nCMFCRDEdit::OnSetText");
		lRes = (LRESULT)::SetWindowText(GetSafeHwnd(), (LPCTSTR)lpszText);
		m_bSetTextProcessing = FALSE;
		return (int)lRes;
	}

	m_bSetTextProcessing = FALSE;
	return FALSE;
}
/**
*
*
*
*/
bool CMFCRDEdit::SetValue(::std::string newValue)
{
	//m_iDelimPos = newValue.find(m_strValidDelim);
	m_str = newValue;
	return true;
}
/**
*
*
*
*/
::std::string CMFCRDEdit::GetValue()
{
	return m_str;
}
/**
*
*
*
*/
int	CMFCRDEdit::GetDelimPosition()
{
	return m_str.find(m_strValidDelim);
}
/**
*
*
*
*/
int CMFCRDEdit::GetASNLength()
{
	int delimPos = GetDelimPosition();
	if (delimPos == -1) return m_str.size();
	if(delimPos  == 0 ) return 0;
	return delimPos - 1;
}
/**
*
*
*
*/
int CMFCRDEdit::GetValueLength()
{
	int delimPos = GetDelimPosition();
	return (delimPos == -1) ? 0 : (m_str.size() -delimPos) ;
}
/**
*
*
*
*/
bool CMFCRDEdit::SetCharAt(UINT cChar, UINT iPos)
{
	int iDiff = m_str.size() - iPos;

	if (iDiff > -1)
		m_str.insert(m_str.begin() + iPos, cChar);
	else if (iDiff == -1)
		m_str.push_back(cChar);
	else
		return false;


	return true;
}