#include "stdafx.h"
#include "ObjectUpdate.h"
#include "DatabaseUpdateCommon.h"
#include "DatabaseUpdateRecord.h"
#include "AttributeUpdate.h"

namespace NM {
	namespace ODB {


	ObjectUpdate::ObjectUpdate(void):
		_update_type(DatabaseUpdateType::Update)							// constructor
	{
	}

	ObjectUpdate::ObjectUpdate(const ObjectUpdate& rhs)						// copy constructor
	{
		// copy the _attribute_update_map, manually copy each AttributeUpdate
		attr_map::const_iterator it = rhs._attribute_update_map.begin();
		while( it != rhs._attribute_update_map.end())
		{
			_attribute_update_map[it->first] = new AttributeUpdate(*it->second);
			++it;
		}

		_update_type = rhs._update_type;
	}

	ObjectUpdate::~ObjectUpdate(void)										// destructor
	{
		attr_map::iterator it = _attribute_update_map.begin();
		while( it != _attribute_update_map.end() )
		{
			delete it->second;
			it->second = nullptr;
			++it;
		}
		_attribute_update_map.clear();

	}

	void ObjectUpdate::SetUpdateType(DatabaseUpdateType update_type)
	{
		_update_type = update_type;
	}

	DatabaseUpdateType ObjectUpdate::GetUpdateType()
	{
		return _update_type;
	}

	void ObjectUpdate::InsertUpdate(DatabaseUpdateRecord &update_record)
	{		
		// check for the attribute in the attr_map _attribute_update_map;
		attr_map::iterator it = _attribute_update_map.find( update_record.GetObjectAttributeName() );

		if( it != _attribute_update_map.end() )
		{
			// already exists, so update new value, leave _previous_value alone as the first one that was updated,
			// skipping out the changes in between
			it->second->SetNewValue( update_record.GetNewValue() );
		}
		else
		{
			// new attribute, create and add to map
			_attribute_update_map[update_record.GetObjectAttributeName()] = 
				new AttributeUpdate(
				update_record.GetPreviousValue(),
				update_record.GetNewValue() 
				);

		}

		return;
	}


}
}