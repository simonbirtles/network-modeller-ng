#include "stdafx.h"
#include "..\ServiceRegistry\IServiceRegistry.h"								// registry interface
#include "SelectedObjects.h"
#include "SelectedObjectsClientObs.h"
#include "..\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"	// for client interface reqd for client database update notifications

extern NM::Registry::IServiceRegistry* reg;

namespace NM
{

	/**
	*
	*
	*
	*/
	SelectedObjects::SelectedObjects():
		_multiselect(false),
		_delayUpdates(false)
	{
		// Database Update Notifications
		// get client interafce to service
		::NM::ODB::IDatabaseUpdate* ptr = reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache"));
		_updateCache.reset(ptr);

		// create require updates list
		::std::vector<::std::wstring> attr_list;
		attr_list.push_back(L"delete");
		// request updates for above list
		_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Vertex, attr_list);
		_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Group, attr_list);
		_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Edge, attr_list);
	}
	/**
	*
	*
	*
	*/
	SelectedObjects::~SelectedObjects()
	{
		_updateCache->DeleteClientUpdatesQueue(this);

	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::Select(NM::ODB::OBJECTUID objectUID)
	{
		OutputDebugString(L"\nSelectedObjects::Select");
		if (!_multiselect)
		{
			_selectedObjects.clear();
		}		
		_selectedObjects.push_back(objectUID);
		SendUpdate();
	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::Select(OBJECTVECTOR& vecObjectUID)
	{
		OutputDebugString(L"\nSelectedObjects::Select");
		if (!_multiselect)
		{
			_selectedObjects.clear();
		}

		_delayUpdates = true;
		OBJECTVECTOR::iterator it = vecObjectUID.begin();
		while (it != vecObjectUID.end())
		{
			if (!IsSelected(*it))
			{
				_selectedObjects.push_back(*it);
			}
			++it;
		}
		_delayUpdates = false;
		SendUpdate();
	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::Deselect(NM::ODB::OBJECTUID objectUID)
	{
		OBJECTVECTOR::iterator it = ::std::find(_selectedObjects.begin(), _selectedObjects.end(), objectUID);
		if (it != _selectedObjects.end())
		{
			_selectedObjects.erase(it);
		}
		SendUpdate();
	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::Deselect(OBJECTVECTOR& vecObjectUID)
	{
		_delayUpdates = true;
		OBJECTVECTOR::iterator it = vecObjectUID.begin();
		while (it != vecObjectUID.end())
		{
			if (IsSelected(*it))
			{
				Deselect(*it);
			}
			++it;
		}
		_delayUpdates = false;
		SendUpdate();
	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::Clear()
	{
		_selectedObjects.clear();
		SendUpdate();
	}
	/**
	*
	*
	*
	*/
	bool SelectedObjects::IsSelected(NM::ODB::OBJECTUID objectUID)
	{
		OBJECTVECTOR::iterator it = ::std::find(_selectedObjects.begin(), _selectedObjects.end(), objectUID);
		return (it != _selectedObjects.end());
	}
	/**
	*
	*
	*
	*/
	size_t SelectedObjects::GetSelected(OBJECTVECTOR& objectVector)
	{
		objectVector = _selectedObjects;
		return _selectedObjects.size();
	}
	/**
	*
	*
	*
	*/
	size_t SelectedObjects::SelectedCount()
	{
		return _selectedObjects.size();
	}
	/**
	*
	*
	*
	*/
	NM::ODB::OBJECTUID SelectedObjects::GetSelectedPos(SelectedObjectPosition pos)
	{
		if (_selectedObjects.size() == 0)
			return ::NM::ODB::INVALID_OBJECT_UID;

		switch (pos)
		{
			case SelectedObjectPosition::First:
				return _selectedObjects[0];
			break;

			case SelectedObjectPosition::Last:
				return _selectedObjects[_selectedObjects.size()-1];
			break;
		}
		return ::NM::ODB::INVALID_OBJECT_UID;
	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::SetMultiSelect(bool bMultiSelect)
	{
		_multiselect = bMultiSelect;
		SendMultiSelectChangeUpdate();
	}
	/**
	*
	*
	*
	*/
	bool SelectedObjects::IsMultiSelect()
	{
		return (_multiselect);
	}
	/**
	*
	*
	*
	*/
	bool SelectedObjects::RegisterClientObserver(SelectedObjectsClientObs* client)
	{
		OBSERVERS::iterator it = ::std::find(_clientObs.begin(), _clientObs.end(), client);
		if (it == _clientObs.end())
		{
			_clientObs.push_back(client);
		}
		return true;
	}
	/**
	*
	*
	*
	*/
	bool SelectedObjects::DeRegisterClientObserver(SelectedObjectsClientObs* client)
	{
		_delayUpdates = true;
		OBSERVERS::iterator it = ::std::find(_clientObs.begin(), _clientObs.end(), client);
		if (it != _clientObs.end())
		{
			_clientObs.erase(it);
		}
		_delayUpdates = false;
		return true;
	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::SendUpdate()
	{
		if (!_delayUpdates)
		{
			OBSERVERS::iterator it = _clientObs.begin();
			while (it != _clientObs.end())
			{
				(*it)->SelectedObjectsUpdate();
				++it;
			}
		}
	}
	/**
	*
	*
	*
	*/
	void SelectedObjects::SendMultiSelectChangeUpdate()
	{
		OBSERVERS::iterator it = _clientObs.begin();
		while (it != _clientObs.end())
		{
			(*it)->MultiSelectChange(_multiselect);
			++it;
		}
	}
	/**
	* DatabaseUpdate
	*
	* Called from DatabaseUpdateCache for notifications requested
	*/
	void SelectedObjects::DatabaseUpdate()
	{
		bool bUpdateRequired = false;
		_delayUpdates = true;
		size_t qSize = _updateQ->QueueSize();
		while (!_updateQ->Empty())
		{
			::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _updateQ->GetNextRecord();
			// get the UID, attrname and new value of the update
			::NM::ODB::real_uid uid = record->GetObjectUID();
			::NM::ODB::real_string attrName = record->GetObjectAttributeName();
			::NM::ODB::Value newValue = record->GetNewValue();

			if (attrName.GetString() == L"delete")
			{
				// if deleted object was selectred -remove from select list
				if (IsSelected(record->GetObjectUID()))
				{
					Deselect(record->GetObjectUID());
					bUpdateRequired = true;
				}
			}
		}
		_delayUpdates = false;
		if(bUpdateRequired)
			SendUpdate();

		return;
	}

	// ns
}

