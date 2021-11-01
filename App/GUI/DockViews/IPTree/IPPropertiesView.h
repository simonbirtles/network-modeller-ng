#pragma once
#include "stdafx.h"
#include "IPRadixDef.h"
#include <vector>
#include <memory>


namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
	}
	namespace IPRTREE
	{
		class IIPService;
	}
}



class CIPPropertiesView : public CWnd
{
	DECLARE_DYNAMIC(CIPPropertiesView)

private:
	enum class PropertyValueType { None, Prefix, Node, InterfaceLink };
	CFont m_fntPropList;
	CMFCPropertyGridCtrl m_wndPropList;
	::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;
	::std::unique_ptr<::NM::IPRTREE::IIPService> _pTree;

	void SetPropListFont();
	void AdjustLayout();
	void DisplayDefault();

public:
	CIPPropertiesView();
	virtual ~CIPPropertiesView();

	void ShowProperties(::std::vector<::NM::IPRTREE::IPV4PREFIX>);
	void ShowProperties(::NM::IPRTREE::RD);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnShowOnMap();
	afx_msg void OnProperties();
};


