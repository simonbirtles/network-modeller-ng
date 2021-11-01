#pragma once
//#include "DatabaseCoreElements\Internal.h"
#include "Interfaces\ObjectDatabaseDefines.h"
#include "Interfaces\DatabaseObserver.h"
#include "Interfaces\IObjectDatabase.h"
#include "Interfaces\IDatabaseUpdate.h"

#include <map>
#include <vector>
#include <tuple>

namespace NM
{
	namespace NetGraph
	{
		class IDataCacheUpdateClient;

		class DataCache : public ::NM::ODB::CDatabaseObserver
		{
		public:
			DataCache();
			~DataCache();
			typedef ::std::wstring ObjectTableName;

			bool SetValue(::NM::ODB::OBJECTUID, ::NM::ODB::ATTRIBUTENAME, ::NM::ODB::Value);
			::NM::ODB::UPVALUE GetValue(::NM::ODB::OBJECTUID, ::NM::ODB::ATTRIBUTENAME);
			bool RegisterForCacheUpdateNotifications(IDataCacheUpdateClient*, ObjectTableName, ::std::vector<::NM::ODB::AttributeName>&);
			bool DeRegisterForCacheUpdateNotifications(IDataCacheUpdateClient *pObserver);

			bool LockOutgoingUpdates(bool bLock, IDataCacheUpdateClient*);

			void DatabaseUpdate();

		private:
			::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;
			::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _updateCache;
			::NM::ODB::IDatabaseUpdate::UpdateQueueHandle	 _updateQ;

			// client layers
			typedef ::std::vector<IDataCacheUpdateClient*>	OBSERVERS;
			OBSERVERS _observers;

			
			typedef ::NM::ODB::SPVALUE CurrentValue;			// current value can be a temp new value ie during dragging or from last db update
			typedef ::NM::ODB::SPVALUE PreviousValue;			// always from the last db update, when no changes occuring and no dragging etc, both current and prev will be the same
			typedef size_t RecordNumber;
			RecordNumber	_nextRecordID;

			// unique list of update types request by a client update request
			typedef ::std::map <RecordNumber,  ::std::tuple < ObjectTableName, ::NM::ODB::AttributeName, PreviousValue, CurrentValue> > DATARECORD;
			// map entries created when client requests updates - DATARECORD is filled at same time
			typedef ::std::pair<ObjectTableName, ::NM::ODB::AttributeName> PAIR_TYPENAME;
			typedef ::std::map< PAIR_TYPENAME, RecordNumber> TYPEATTRNAME_TO_IDX_MAP;	
			// fill when a client update request recieved
			typedef ::std::map<::NM::ODB::OBJECTUID, RecordNumber, ::NM::ODB::compareGuid> UID_TO_IDX_MAP;

			DATARECORD _dataRecords;
			UID_TO_IDX_MAP _indexUID;
			TYPEATTRNAME_TO_IDX_MAP _indexTypeName;

			RecordNumber GetNewRecordNumber();

			// fill the cache Current/Previous Values AND UID_TO_IDX_MAP with data from core database - actually cache data
			bool FillCache(ObjectTableName, ::NM::ODB::AttributeName&);

			// either updates existing objects value or creates a new object and values, creates a new data record if one doesnt exist
			RecordNumber UpdateCacheReferences(::NM::ODB::OBJECTUID, ObjectTableName, ::NM::ODB::AttributeName&, ::NM::ODB::SPVALUE&);

			::NM::ODB::ObjectType GetObjectTypeID(ObjectTableName&);
			ObjectTableName GetObjectTypeName(::NM::ODB::ObjectType);

		};


	}
}