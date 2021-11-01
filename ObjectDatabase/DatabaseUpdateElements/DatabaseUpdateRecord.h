#pragma once
#include <string>
#include "..\Interfaces\ObjectDatabaseDefines.h"

namespace NM 
{
	namespace ODB 
	{		
		class DatabaseUpdateRecord
		{
		public:	
			DatabaseUpdateRecord(DatabaseUpdateType update_type,
										OBJECTUID	object_uid,
										ObjectType	object_type,
										ODBWString	&object_attribute_name,
										Value		&previous_value,
										const Value &new_value );
			~DatabaseUpdateRecord();
			DatabaseUpdateRecord(const DatabaseUpdateRecord& rhs);

			DatabaseUpdateType	GetUpdateType() const;
			OBJECTUID			GetObjectUID() const;
			ObjectType			GetObjectType() const;
			ODBWString			GetObjectAttributeName() const;
			Value				GetPreviousValue() const;
			Value				GetNewValue() const;

		private:
		   	DatabaseUpdateType	_update_type;
			OBJECTUID			_object_uid;
			ObjectType			_object_type;
			ODBWString			_object_attribute_name;
			Value				_previous_value;
			Value				_new_value;

			void operator=(const DatabaseUpdateRecord&) = delete;

		};
// ns
	}
}