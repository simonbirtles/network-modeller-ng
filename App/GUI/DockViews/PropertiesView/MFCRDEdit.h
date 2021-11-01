#pragma once
#include "afxcontrolbarutil.h"
#include <string>

class CMFCRDEdit : public CEdit
{
	DECLARE_DYNAMIC(CMFCRDEdit)
	
public:
	CMFCRDEdit();
	~CMFCRDEdit();
	
private:
	

private:
	::std::string m_str;                  // Initial value
	::std::string m_strValidNumbers;      // Valid string characters
	::std::string m_strValidDelim;        // Valid string characters
	//int		m_iDelimPos;			// delimi pos
	BOOL	m_bSetTextProcessing;

	void	GetGroupBounds(int &nUsed, int &nFree, int &nMax, int nPos);
	void	OnCharDelete(UINT nChar, UINT nRepCnt, UINT nFlags);
	void	OnCharBackspace(UINT nChar, UINT nRepCnt, UINT nFlags);
	bool	SetValue(::std::string newValue);
	bool	SetCharAt(UINT cChar, UINT iPos);
	::std::string GetValue();
	int		GetDelimPosition();
	int		GetASNLength();
	int		GetValueLength();


protected:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int	 OnSetText(LPCTSTR lpszText);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};

