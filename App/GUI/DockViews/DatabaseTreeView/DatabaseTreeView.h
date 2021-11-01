#pragma once

#include "ViewTree.h"
#include "Interfaces\ObjectDatabaseDefines.h"
#include "SelectedObjectsClientObs.h"
#include "..\ObjectDatabase\Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"	
#include <map>
#include <vector>
#include <memory>

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};



namespace NM
{
	class ISelectedObjects;

	namespace ODB
	{
		class IObjectDatabase;
	}

	namespace DataServices
	{
		class IAdjacencyMatrix;
	}
}

class CDatabaseTreeView : 
	public CBCGPDockingControlBar,
	public ::NM::SelectedObjectsClientObs,
	public ::NM::ODB::CDatabaseObserver
{
public:
	CDatabaseTreeView();
	virtual ~CDatabaseTreeView();

	void AdjustLayout();
	void OnChangeVisualStyle();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void SelectedObjectsUpdate();					// callback from SelectedObjects Service
	void MultiSelectChange(bool bMultiSelect);		// callback from SelectedObjects Service
	void DatabaseUpdate();							// callback from DatabaseUpdateCache service

	// treeview subclass WndProc
	static LRESULT CALLBACK TreeViewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	static VOID CALLBACK WaitOrTimerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);

protected:
	CClassToolBar m_wndToolBar;
	CViewTree m_wndDatabaseTreeView;
	CImageList m_ClassViewImages;
	UINT m_nCurrSort;
	
	// MFC Generated and not currently used.
	afx_msg void OnClassAddMemberFunction();		// button popup top dropdown menu
	afx_msg void OnClassAddMemberVariable();		// button popup top dropdown menu
	afx_msg void OnClassDefinition();				// button popup top dropdown menu
	afx_msg void OnClassProperties();				// button popup top dropdown menu
	afx_msg void OnRefreshDataCache();				// button top menu
	afx_msg void OnSort(UINT id);					// no corrosponding button?

	// real stuff
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);

	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);
	//afx_msg void OnTreeViewItemSelect(NMHDR *pNMHDR, LRESULT *pResult);

	// CViewTree virtual override for WM_NOTIFY notifications
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);	

	DECLARE_MESSAGE_MAP()

private:
	struct compareGuid
	{
		bool operator()(const ::NM::ODB::ODBUID& guid1, const ::NM::ODB::ODBUID& guid2) const
		{
			if (guid1.Data1 != guid2.Data1) {
				return guid1.Data1 < guid2.Data1;
			}
			if (guid1.Data2 != guid2.Data2) {
				return guid1.Data2 < guid2.Data2;
			}
			if (guid1.Data3 != guid2.Data3) {
				return guid1.Data3 < guid2.Data3;
			}
			for (int i = 0;i<8;i++) {
				if (guid1.Data4[i] != guid2.Data4[i]) {
					return guid1.Data4[i] < guid2.Data4[i];
				}
			}
			return false;
		}
	};

	CMenu menu;
	::std::unique_ptr<NM::ODB::IObjectDatabase> _odb;
	::std::unique_ptr<NM::ISelectedObjects> _selectedObjects;

	::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _updateCache;
	::NM::ODB::IDatabaseUpdate::UpdateQueueHandle	 _updateQ;

	// local cache ID -> OBJECTUID MAP
	typedef  ::std::map<long, ::NM::ODB::OBJECTUID> IDUIDMAP;
	IDUIDMAP id_to_uid_map;

	// OBJECTUID -> HTREEITEM MAP
	typedef  ::std::map<::NM::ODB::OBJECTUID, HTREEITEM, compareGuid> UIDHTREE;
	UIDHTREE uid_to_htree;
	
	// selected items in the tree syncd with selected objects service
	typedef ::std::vector<HTREEITEM> SELECTEDVEC;
	SELECTEDVEC _selectedHTreeItems;

	HANDLE		_hNewTimer;				// object show on map timer queue
	long		_nextCacheID;
	bool		_bLoading;					// inital loading flag to prevent any updates etc from running
	bool		_bUpdatingSelected;			// prevents us sending out updates to selection svc 
	bool		_bIgnoreSelectUpdates;		// when we select or deselect an item, we want to ignore the update
	bool		_bIgnoreSelected;			// for CustomDraw not to draw any selected items
	bool		_bScrollSelect;				// toggle if new item is selected during paging of scroll
	bool		_bStickySelect;				// toggle ctrl key emulation
	bool		_bRedrawSet;				// toggle WM_PAINT to allow draw or not, used for large updates to tree
	bool		_objectMapHighlighted;		// Have we enabled a map object(s) highlight

	void		AddItemToCache(long, ::NM::ODB::OBJECTUID, HTREEITEM);
	void		CreateGroupTree(HTREEITEM groupRoot, ::NM::ODB::OBJECTUID parentGroupUID = ::NM::ODB::INVALID_OBJECT_UID);
	long		GetNewCacheID();
	bool		RegisterClientNotify();

	void		UpdateGlobalSelect();
	void		SetUpdatingSelected(bool bUpdating);
	void		SelectItem(HTREEITEM hTreeItem, bool bAdditive = false);
	void		DeSelectItem(HTREEITEM hTreeItem);
	void		ClearSelected();
	bool		IsItemSelected(HTREEITEM hTreeItem);
	::std::vector<::NM::ODB::OBJECTUID> GetSelectedObjects();
	::std::unique_ptr<::NM::DataServices::IAdjacencyMatrix> _adjm;

	void		ReDrawSelected();
	size_t		GetValidSelectedCount();
	size_t		GetValidSelectedUID(::std::vector<::NM::ODB::ODBUID> &vecSelected);
	size_t		GetValidSelectedHTreeItems(SELECTEDVEC &vecSelected);
	HTREEITEM   GetLastSelectedItem();
	HTREEITEM	GetNextHTreeItem(HTREEITEM hTreeItem, UINT code);
	//void		OnTreeViewRightClick(NMHDR *pNMHDR, LRESULT *pResult);
	LRESULT		OnTreeViewLeftButtonDblClick(NMHDR *pNMHDR, LRESULT *pResult);
	void		OnTreeViewLeftButtonDown(NMHDR *pNMHDR, LRESULT *pResult);
	void		OnTreeViewRightButtonDown(NMHDR *pNMHDR, LRESULT *pResult);
	void		OnTreeViewKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
	void		OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	void		OnTreeViewSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void		OnTreeViewSelectionChanging(NMHDR* pNMHDR, LRESULT* pResult);
	void		SelectRange(HTREEITEM firstItem, HTREEITEM lastItem);
	void		OnLButtonDown(UINT, CPoint);
	// toolbar buttons
	void			RefreshDataCache();
	afx_msg void	OnScrollSelect();
	afx_msg void	OnUpdateScrollSelect(CCmdUI* pCmdUI);
	afx_msg void 	OnStickySelect();
	afx_msg void 	OnUpdateStickySelect(CCmdUI* pCmdUI);
	afx_msg void	OnDeleteObject();
	afx_msg void 	OnUpdateDeleteObject(CCmdUI* pCmdUI);
	afx_msg void	OnShowObjectOnMap();
	afx_msg void 	OnUpdateShowObjectOnMap(CCmdUI* pCmdUI);

	afx_msg void	OnToggleObjectInService();

};

