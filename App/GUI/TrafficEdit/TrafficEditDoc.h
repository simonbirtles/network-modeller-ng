#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"

// CTrafficEditDoc document

class CTrafficEditDoc : public CDocument
{
	DECLARE_DYNCREATE(CTrafficEditDoc)

private:
	::NM::ODB::OBJECTUID	m_selectedFlowUID;
	::NM::ODB::OBJECTUID	m_selectedDemandUID;
	::NM::ODB::OBJECTUID	m_selectedPathUID;

public:
	CTrafficEditDoc();
	virtual ~CTrafficEditDoc();
#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif


	void SetSelectedFlow(::NM::ODB::OBJECTUID flowUID)
	{
		m_selectedFlowUID = flowUID;
		UpdateAllViews(NULL);
		SetModifiedFlag(FALSE);
	}
	void SetSelectedDemand(::NM::ODB::OBJECTUID demandUID)
	{
		m_selectedDemandUID = demandUID;
		UpdateAllViews(NULL);
		SetModifiedFlag(FALSE);
	}
	void SetSelectedPath(::NM::ODB::OBJECTUID pathUID)
	{
		m_selectedPathUID = pathUID;
		UpdateAllViews(NULL);
		SetModifiedFlag(FALSE);
	}



protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
};
