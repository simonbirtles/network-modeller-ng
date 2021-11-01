#include "stdafx.h"
#include "DataCache.h"
#include "IServiceRegistry.h"							// registry interface
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\IAttribute.h"						// interface to an attribute copy
#include "Interfaces\IDatabaseUpdate.h"					// for client interface for core database update notifications

extern NM::Registry::IServiceRegistry* reg;

namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		DataCache::DataCache():
			_nextRecordID(1)
		{
			if(!reg)
				throw(::std::runtime_error("Service Registry Not Available!, Cannot Continue."));

			// Get The Database Handle
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				throw(::std::runtime_error("Core Database Not Available!, Cannot Continue."));

			// Database Update Notifications
			_updateCache.reset(reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache")));
			if (!_updateCache)
				throw(::std::runtime_error("Core Database Update Service Not Available!, Cannot Continue."));


		}
		/**
		*
		*
		*
		*/
		DataCache::~DataCache()
		{
		}
		/**
		*
		*
		*
		*/
		void DataCache::DatabaseUpdate()
		{

		}
		/**
		*
		*
		*
		*/
		DataCache::RecordNumber DataCache::GetNewRecordNumber()
		{
			return _nextRecordID++;
		}
		/**
		* RegisterForCacheUpdateNotifications
		*
		* Client Layer requests updates from this datacache, this cache needs to request updates from the main ODB.
		*/
		bool DataCache::RegisterForCacheUpdateNotifications(IDataCacheUpdateClient *pObserver, ObjectTableName objectTableName, ::std::vector<::NM::ODB::AttributeName> &attribute_list)
		{
			::NM::ODB::ObjectType objectTypeId = GetObjectTypeID(objectTableName);
			if (objectTypeId == ::NM::ODB::ObjectType::ObjectInvalid)
			{
				throw ::std::runtime_error("Invalid Object Table Name");
				return false;
			}

			// 1. fill cache data & request core ODB cache updates
			::std::vector<::NM::ODB::AttributeName>::iterator attribit;
			// check and add create/delete if not requested
			attribit = ::std::find(attribute_list.begin(), attribute_list.end(), L"create");
			if (attribit == attribute_list.end())
			{
				attribute_list.push_back(L"create");
			}
			attribit = ::std::find(attribute_list.begin(), attribute_list.end(), L"delete");
			if (attribit == attribute_list.end())
			{
				attribute_list.push_back(L"delete");
			}

			for each(::std::wstring attribute in attribute_list)
			{
				FillCache(objectTableName, attribute);
			}
						
			// request updates for above list
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, objectTypeId, attribute_list);


			// 2. register observer, it may already be registered and this could be an updated list - add only. to remove need to dereg then register
			OBSERVERS::iterator it = ::std::find(_observers.begin(), _observers.end(), pObserver);
			if (it == _observers.end())
			{
				_observers.push_back(pObserver);
			}
			return true;
		}
		/** 
		* FillCache
		*
		* fill the DATARECORD cache Current/Previous Values AND UID_TO_IDX_MAP with data from core database 
		* Actually cache data from the database, iterate through db table for all data
		*/
		bool DataCache::FillCache(ObjectTableName objectTableName, ::NM::ODB::AttributeName& attributeName)
		{
			::NM::ODB::OBJECTUID objectUID = _odb->GetFirstObject(objectTableName);
			while (objectUID != NM::ODB::INVALID_OBJECT_UID)
			{
				if (_odb->IsValidAttribute(objectUID, attributeName))
				{
					::NM::ODB::SPVALUE value = _odb->GetValue(objectUID, attributeName);
					UpdateCacheReferences(objectUID, objectTableName, attributeName, value);
				}
				objectUID = _odb->GetNextObject(objectUID);
			}
			return true;
		}
		/**
		* UpdateCache
		*
		* either updates existing objects value or creates a new object and values
		* Ensure that the cache request is saved for local lookups.
		*/
		/*
		::std::wstring typeName = _odb->GetObjectTypeName(objectUID);
		::NM::ODB::ObjectType typeId = GetObjectTypeID(typeName);
		if (typeId == ::NM::ODB::ObjectType::Path)
		return 0;
		*/
		DataCache::RecordNumber DataCache::UpdateCacheReferences(::NM::ODB::OBJECTUID objectUID, ObjectTableName objectTableName, ::NM::ODB::AttributeName& attributeName, ::NM::ODB::SPVALUE& value)
		{
			if (!_odb->IsValidObjectUID(objectUID)) return false;

			RecordNumber recNo = 0;
			// check to see if the <objecttype, attrname> pair already exists
			TYPEATTRNAME_TO_IDX_MAP::iterator typeattrit = _indexTypeName.find(::std::make_pair(objectTableName, attributeName));
			if (typeattrit == _indexTypeName.end())
			{
				recNo = GetNewRecordNumber();
				// nope so add it as a data record and into index
				_dataRecords[recNo] = ::std::make_tuple(objectTableName, attributeName, value, value);
				_indexTypeName[::std::make_pair(objectTableName, attributeName)] = recNo;
			}
			else
			{
				recNo = typeattrit->second;
			}
			
			// check for objectUID exists
			UID_TO_IDX_MAP::iterator objectit = _indexUID.find(objectUID);
			if (objectit == _indexUID.end())
			{
				// add this objectid to index
				_indexUID[objectUID] = recNo;
			}
		
			return true;
		}
		/**
		*
		*
		*
		*/
		::NM::ODB::ObjectType DataCache::GetObjectTypeID(ObjectTableName& typeName)
		{
			// resolve the objects type, look up here as its a temp fix, problem is 
			// we dont get access to the objects enum from outside the db, no method to request it,
			// but we can request the "name" of the type, we should be migrating to this anyway
			// but not had the time to change all code.. so we have to map it here.
			::NM::ODB::ObjectType typeId = ::NM::ODB::ObjectType::ObjectInvalid;

			if (typeName == L"verticies")
				typeId = ::NM::ODB::ObjectType::Vertex;
			else if (typeName == L"edges")
				typeId = ::NM::ODB::ObjectType::Edge;
			else if (typeName == L"groups")
				typeId = ::NM::ODB::ObjectType::Group;
			else if (typeName == L"layers")
				typeId = ::NM::ODB::ObjectType::Layer;
			else if (typeName == L"flows")
				typeId = ::NM::ODB::ObjectType::Flow;
			else if (typeName == L"demands")
				typeId = ::NM::ODB::ObjectType::Demand;
			else if (typeName == L"paths")
				typeId = ::NM::ODB::ObjectType::Path;

			return typeId;
		}
		/**
		*
		*
		*
		*/
		DataCache::ObjectTableName DataCache::GetObjectTypeName(::NM::ODB::ObjectType objectTypeId)
		{
			// resolve the typeid, look up here as its a temp fix, problem is 
			// we dont get access to the objects enum from outside the db, no method to request it,
			// but we can request the "name" of the type, we should be migrating to this anyway
			// but not had the time to change all code.. so we have to map it here.
			ObjectTableName typeName = L"";

			switch (objectTypeId)
			{				
			case ::NM::ODB::ObjectType::Vertex:
				typeName = L"verticies";
				break;				
			case::NM::ODB::ObjectType::Edge:
				typeName = L"edges";
				break;					
			case ::NM::ODB::ObjectType::Group:
				typeName = L"groups";
				break;					
			case ::NM::ODB::ObjectType::Layer:
				typeName = L"layers";
				break;
			case ::NM::ODB::ObjectType::Flow:
				typeName = L"flows";
				break;
			case ::NM::ODB::ObjectType::Demand:
				typeName = L"demands";
				break;
			case ::NM::ODB::ObjectType::Path:
				typeName = L"paths";
				break;
			}
			return typeName;
		}


		// ns
	}
}