// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#pragma once

#include <map>
#include "Interfaces\ObjectDatabaseDefines.h"
#include "SelectedObjectsClientObs.h"
#include "SelectedObjectsDefines.h"
#include "..\ObjectDatabase\Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"					// base class for client listener (dervive from for update calls)


namespace NM
{
	class ISelectedObjects;

	namespace ODB
	{
		class IObjectDatabase;
	}
}



/**
*
*
*/
class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }

	//CUSTOMCOLORS
	//virtual void  OnFillBackground(CDC* pDC)
	//{
	//	CRect crect;
	//	GetClientRect(&crect);
	//	pDC->FillSolidRect(&crect, RGB(0.2*255, 0.2 * 255, 0.2 * 255));
	//}
};

/**
*
*
*/
class CPropertiesWnd : 
	public CBCGPDockingControlBar,
	public ::NM::SelectedObjectsClientObs, 
	public ::NM::ODB::CDatabaseObserver
{
// Construction
public:
	CPropertiesWnd();

	void AdjustLayout();

	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}
	void InitPropList();

	void SelectedObjectsUpdate();
	void MultiSelectChange(bool bMultiSelect);
	void DatabaseUpdate();

private:

	::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _updateCache;
	::NM::ODB::IDatabaseUpdate::UpdateQueueHandle	 _updateQ;

protected:
	CFont m_fntPropList;
	CComboBox m_wndObjectCombo;
	CPropertiesToolBar m_wndToolBar;
	CMFCPropertyGridCtrl m_wndPropList;

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties1();
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnProperties2();
	afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//afx_msg void OnPropertyClick(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg LRESULT OnPropertyChanged(__in WPARAM wparam, __in LPARAM lparam);
	

	DECLARE_MESSAGE_MAP()


	void SetPropListFont();

	int m_nComboHeight;


private:
	//int	 _iAttributeGroupIndex;
	bool _bIgnoreDatabaseUpdates;
	bool bResolveUIDtoName;
	::NM::ODB::OBJECTUID _currentObjectUID;
	::std::unique_ptr<NM::ODB::IObjectDatabase> _odb;
	::std::unique_ptr<NM::ISelectedObjects>		_selectedObjects;

	//void CreateAttributeHeader();
	void DisplayProperties(::NM::OBJECTVECTOR vecObjects);
	void AddObjectProperty(::NM::ODB::OBJECTUID, bool bClear = true);
	void DisplayDefault();
	//void ClearProperties();

	bool RegisterClientNotify();
	
};

