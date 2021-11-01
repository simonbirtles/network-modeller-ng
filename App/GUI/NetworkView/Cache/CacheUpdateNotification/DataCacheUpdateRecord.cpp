#include "stdafx.h"
#include "DataCacheUpdateRecord.h"

namespace NM 
{
	namespace NetGraph 
	{
		using namespace ::NM::ODB;


			DataCacheUpdateRecord::DataCacheUpdateRecord(DatabaseUpdateType update_type,
									OBJECTUID object_uid,
									ObjectType object_type,
									ODBWString &object_attribute_name,
									Value &previous_value,
									const Value &new_value):
				_update_type(update_type),
				_object_uid(object_uid),
				_object_type(object_type),
				_object_attribute_name(object_attribute_name),
				_previous_value(previous_value),
				_new_value(new_value)
			{
			}

			// copy 
			DataCacheUpdateRecord::DataCacheUpdateRecord(const DataCacheUpdateRecord& rhs):
				_update_type(rhs._update_type),
				_object_uid(rhs._object_uid),
				_object_attribute_name(rhs._object_attribute_name),
				_previous_value(rhs._previous_value),
				_new_value(rhs._new_value)
			{
			}

			DataCacheUpdateRecord::~DataCacheUpdateRecord()
			{
			}

			DatabaseUpdateType	
				DataCacheUpdateRecord::GetUpdateType() const
			{
				return _update_type;
			}

			OBJECTUID	DataCacheUpdateRecord::GetObjectUID() const
			{
				return _object_uid;
			}

			ObjectType DataCacheUpdateRecord::GetObjectType() const
			{
				return _object_type;
			}

			::std::wstring DataCacheUpdateRecord::GetObjectAttributeName() const
			{
				return ::std::move( _object_attribute_name );
			}

			Value DataCacheUpdateRecord::GetPreviousValue() const
			{
				return _previous_value;
			}

			Value DataCacheUpdateRecord::GetNewValue() const
			{
				return _new_value;
			}

			// ns
	}
}