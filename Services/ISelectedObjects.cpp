#include "stdafx.h"
#include "ISelectedObjects.h"
#include "SelectedObjectsClientObs.h"
#include "SelectedObjects.h"

namespace NM
{
	/**
	*
	*
	*
	*/
	ISelectedObjects::ISelectedObjects(SelectedObjects** selectedObjects):
		_selectedObjects(selectedObjects)
	{
	}
	/**
	*
	*
	*
	*/
	ISelectedObjects::~ISelectedObjects()
	{
	}
	/**
	*
	*
	*
	*/
	void ISelectedObjects::Select(NM::ODB::OBJECTUID objectUID)
	{
		if (!(*_selectedObjects)) return;
		(*_selectedObjects)->Select(objectUID);
	}
	/**
	*
	*
	*
	*/
	void ISelectedObjects::Select(OBJECTVECTOR& objectVec)
	{
		if (!(*_selectedObjects)) return;
		(*_selectedObjects)->Select(objectVec);
	}
	/**
	*
	*
	*
	*/
	void ISelectedObjects::Deselect(NM::ODB::OBJECTUID objectUID)
	{
		if (!(*_selectedObjects)) return;
		(*_selectedObjects)->Deselect(objectUID);
	}
	/**
	*
	*
	*
	*/
	void ISelectedObjects::Deselect(OBJECTVECTOR& objectVec)
	{
		if (!(*_selectedObjects)) return;
		(*_selectedObjects)->Deselect(objectVec);
	}
	/**
	*
	*
	*
	*/
	void ISelectedObjects::Clear()
	{
		if (!(*_selectedObjects)) return;
		(*_selectedObjects)->Clear();
	}
	/**
	*
	*
	*
	*/
	bool ISelectedObjects::IsSelected(NM::ODB::OBJECTUID objectUID)
	{
		if (!(*_selectedObjects)) return false;
		return (*_selectedObjects)->IsSelected(objectUID);
	}
	/**
	*
	*
	*
	*/
	size_t ISelectedObjects::GetSelected(OBJECTVECTOR& objectVec)
	{
		if (!(*_selectedObjects)) return 0;
		return (*_selectedObjects)->GetSelected(objectVec);
	}
	/**
	*
	*
	*
	*/
	size_t ISelectedObjects::SelectedCount()
	{
		if (!(*_selectedObjects)) return 0;
		return (*_selectedObjects)->SelectedCount();
	}
	/**
	*
	*
	*
	*/
	NM::ODB::OBJECTUID	ISelectedObjects::GetSelectedPos(SelectedObjectPosition pos)
	{
		if (!(*_selectedObjects)) return NM::ODB::INVALID_OBJECT_UID;
		return (*_selectedObjects)->GetSelectedPos(pos);
	}
	/**
	*
	*
	*
	*/
	void ISelectedObjects::SetMultiSelect(bool bMultiSelect)
	{
		if (!(*_selectedObjects)) return;
		(*_selectedObjects)->SetMultiSelect(bMultiSelect);
	}
	/**
	*
	*
	*
	*/
	bool ISelectedObjects::IsMultiSelect()
	{
		if (!(*_selectedObjects)) return false;
		return (*_selectedObjects)->IsMultiSelect();
	}
	/**
	*
	*
	*
	*/
	bool ISelectedObjects::RegisterClientObserver(SelectedObjectsClientObs* client)
	{
		if (!(*_selectedObjects)) return false;
		return (*_selectedObjects)->RegisterClientObserver(client);
	}
	/**
	*
	*
	*
	*/
	bool ISelectedObjects::DeRegisterClientObserver(SelectedObjectsClientObs* client)
	{
		if (!(*_selectedObjects)) return false;
		return (*_selectedObjects)->DeRegisterClientObserver(client);
	}

	// ns
}
