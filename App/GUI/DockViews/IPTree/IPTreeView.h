#pragma once
#include "ViewTreeCtrl.h"
#include "IPRadixDef.h"
#include "Interfaces\IDatabaseUpdate.h"
#include "Interfaces\DatabaseObserver.h"
#include <map>
#include <set>
#include <memory>

class CFileViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};


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

class CIPPropertiesView;

class CIPTreeView : 
	public CWnd, //CBCGPDockingControlBar
	public ::NM::ODB::CDatabaseObserver
{
	//struct compare_in_addr
	//{
	//	bool operator()(const in_addr& addr1, const in_addr& addr2) const
	//	{
	//		if (addr1.S_un.S_addr != addr2.S_un.S_addr) {
	//			return addr1.S_un.S_addr < addr2.S_un.S_addr;
	//		}
	//		return false;
	//	}
	//};

	struct compare_tuple
	{
		bool operator()(const ::NM::IPRTREE::IPV4PREFIX& prefix1, const ::NM::IPRTREE::IPV4PREFIX& prefix2) const
		{
			// ip address
			if ( ::std::get<0>(prefix1).S_un.S_addr != ::std::get<0>(prefix2).S_un.S_addr) {
				return  ::std::get<0>(prefix1).S_un.S_addr < ::std::get<0>(prefix2).S_un.S_addr;
			}

			// prefix length
			if (::std::get<1>(prefix1) != ::std::get<1>(prefix2)) {
				return  ::std::get<1>(prefix1) < ::std::get<1>(prefix2);
			}

			// interface UID <set>
			//if (::std::get<2>(prefix1) != ::std::get<2>(prefix2)) {
			//	return  ::std::get<2>(prefix1) < ::std::get<2>(prefix2);
			//}

			
				//::NM::IPRTREE::INTFLIST listA = ::std::get<2>(prefix1);
				//::NM::IPRTREE::INTFLIST listB = ::std::get<2>(prefix2);

				//if (listA != listB) 
				//{
				//	return listA < listB;
				//}

			

			return false;
		}
	};
	

public:
		
	CIPTreeView();
	virtual ~CIPTreeView();
	void EnableProperties(CIPPropertiesView*);
	
	//void AddSampleIPTreeData();
	void RefreshRDList();
	void SetSelectedRouteDistinguisher(::NM::IPRTREE::RD);
	::NM::IPRTREE::RD GetSelectedRouteDistinguisher();
	
	// db updates
	void DatabaseUpdate();

	// for edit dialog box
	struct ipinfo
	{
		::NM::IPRTREE::RDASN asn;
		::NM::IPRTREE::RDVALUE value;
		::NM::IPRTREE::IPV4ADDR addr;
		::NM::IPRTREE::IPV4PREFIXLENGTH prefixLength;
		::NM::ODB::OBJECTUID interfaceID;
		bool		ok;
	};
	ipinfo dialogInfo;
	static INT_PTR CALLBACK dialogProc(HWND   hwndDlg, UINT   uMsg, WPARAM wParam, LPARAM lParam);


protected:
	CFont				m_fntPropList;
	CComboBox			m_wndObjectCombo;
	CViewTreeCtrl		m_wndIPView;
	CImageList			m_imgIPView;
	CFileViewToolBar	m_wndToolBar;
	int					m_nComboHeight;	

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRefresh();
	afx_msg void OnAddPrefix();
	afx_msg void OnDeletePrefix();
	afx_msg void OnEditPrefix();
	afx_msg void OnCopyPrefix();
	afx_msg void OnExpandAll();
	afx_msg void OnCollapseAll();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnCbnSelchange();
	DECLARE_MESSAGE_MAP()


private:
	::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;
	::std::unique_ptr<::NM::IPRTREE::IIPService> _pTree;
	::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _pUpdateCache;
	::NM::ODB::IDatabaseUpdate::UpdateQueueHandle _pUpdateQueue;
	// combo box item index to RD map
	::std::map<int, ::NM::IPRTREE::RD> _indexRDMap;
	// treeview - ipprefix to tree hitem map
	typedef ::std::map<::NM::IPRTREE::IPV4PREFIX, HTREEITEM, compare_tuple> INDEXMAP;
	typedef INDEXMAP::iterator INDEXMAPIT;
	INDEXMAP _ipIndexMap;
	// properties box
	CIPPropertiesView	*m_wndProp;
	
	void AdjustLayout();
	void OnChangeVisualStyle();
	void SetPropListFont();		
	void RefreshIPData(::NM::IPRTREE::RD);
	void ExpandAll(HTREEITEM hItemParent);
	void CollapseAll(HTREEITEM hItemParent);
	INDEXMAPIT GetSelectedPrefix();
	::NM::IPRTREE::RD GetCurrentRDSelection();	
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

