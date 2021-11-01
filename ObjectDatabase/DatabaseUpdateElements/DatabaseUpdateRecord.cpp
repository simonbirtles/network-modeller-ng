#include "stdafx.h"
#include "DatabaseUpdateRecord.h"

namespace NM 
{
	namespace ODB 
	{
			DatabaseUpdateRecord::DatabaseUpdateRecord(DatabaseUpdateType update_type,
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
			DatabaseUpdateRecord::DatabaseUpdateRecord(const DatabaseUpdateRecord& rhs):
				_update_type(rhs._update_type),
				_object_uid(rhs._object_uid),
				_object_type(rhs._object_type),
				_object_attribute_name(rhs._object_attribute_name),
				_previous_value(rhs._previous_value),
				_new_value(rhs._new_value)
			{
			}

			DatabaseUpdateRecord::~DatabaseUpdateRecord()
			{
			}

			DatabaseUpdateType	
				DatabaseUpdateRecord::GetUpdateType() const
			{
				return _update_type;
			}

			OBJECTUID	DatabaseUpdateRecord::GetObjectUID() const
			{
				return _object_uid;
			}

			ObjectType DatabaseUpdateRecord::GetObjectType() const
			{
				return _object_type;
			}

			::std::wstring DatabaseUpdateRecord::GetObjectAttributeName() const
			{
				return ::std::move( _object_attribute_name );
			}

			Value DatabaseUpdateRecord::GetPreviousValue() const
			{
				return _previous_value;
			}

			Value DatabaseUpdateRecord::GetNewValue() const
			{
				return _new_value;
			}

			// ns
	}
}