#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"
#include <memory>
/*******************************************************************************************************************
* Class COutputList
*
* 
********************************************************************************************************************/
class COutputList : public CListBox
{
// Construction
public:
	COutputList();

// Implementation
public:
	virtual ~COutputList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};


/*******************************************************************************************************************
* Class CServiceState
*
* Gets a string of the services state and displays states
********************************************************************************************************************/
class CServiceState : public CListCtrl
{
public:
	CServiceState();
	virtual ~CServiceState();
	void RefreshContent();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRefresh();
	//afx_msg void OnEditClear();
	//afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};

/*******************************************************************************************************************
* Class COutputGrid
*
* 
********************************************************************************************************************/
class COutputGrid : public CMFCPropertyGridCtrl
{
	// Construction
public:
	COutputGrid();

	// Implementation
public:
	virtual ~COutputGrid();

protected:
	//afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//afx_msg void OnEditCopy();
	//afx_msg void OnEditClear();
	//afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};

/*******************************************************************************************************************
* Class CDatabaseNotificationsList
*
* Provides view on database notifications
********************************************************************************************************************/
#include "..\ObjectDatabase\Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"	
namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
		class IDatabaseUpdate;
	}
}
class CDatabaseNotificationsList :
	public CListBox,
	public ::NM::ODB::CDatabaseObserver
{
public:
	CDatabaseNotificationsList();
	virtual ~CDatabaseNotificationsList();

	void DatabaseUpdate();
	
protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()

private:
	::std::unique_ptr<NM::ODB::IObjectDatabase>		_odb;
	::std::unique_ptr<::NM::ODB::IDatabaseUpdate>	_updateCache;
	::NM::ODB::IDatabaseUpdate::UpdateQueueHandle	_updateQ;

};



/*******************************************************************************************************************
* Class COutputWnd
*
* Main output containing window
********************************************************************************************************************/
class COutputWnd : public CBCGPDockingControlBar
{
// Construction
public:
	COutputWnd();

	void UpdateFonts();


// Attributes
protected:
	CMFCTabCtrl	m_wndTabs;

	CServiceState m_wndOutputServiceState;
	CDatabaseNotificationsList m_wndDatabaseLog;
	COutputList m_wndOutputPath;
	COutputGrid m_wndOutputGrid;

protected:
	void InitServiceStateWindow();
	void InitLogWindow();
	void InitPathWindow();
	void FillGridWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

