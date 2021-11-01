#pragma once
#include <string>
#include "..\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"
#include "..\ObjectDatabase\DatabaseCoreElements\Value.h"

namespace NM 
{
	namespace NetGraph 
	{		
		using namespace ::NM::ODB;
		class DataCacheUpdateRecord
		{
		public:	
			DataCacheUpdateRecord(DatabaseUpdateType update_type,
										OBJECTUID	object_uid,
										ObjectType	object_type,
										ODBWString	&object_attribute_name,
										Value		&previous_value,
										const Value &new_value );
			~DataCacheUpdateRecord();
			DataCacheUpdateRecord(const DataCacheUpdateRecord& rhs);

			DatabaseUpdateType	GetUpdateType() const;
			OBJECTUID			GetObjectUID() const;
			ObjectType			GetObjectType() const;
			ODBWString			GetObjectAttributeName() const;
			Value				GetPreviousValue() const;
			Value				GetNewValue() const;

		private:
		   void operator=(const DataCacheUpdateRecord&);

			DatabaseUpdateType	_update_type;
			OBJECTUID			_object_uid;
			ObjectType			_object_type;
			ODBWString			_object_attribute_name;
			Value				_previous_value;
			Value				_new_value;
		};
// ns
	}
}