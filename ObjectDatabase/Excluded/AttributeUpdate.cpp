#include "stdafx.h"
#include "AttributeUpdate.h"

namespace NM {
	namespace ODB {

	// constructor
	AttributeUpdate::AttributeUpdate(Value previous_value, Value new_value):
		_previous_value(previous_value),
		_new_value(new_value)
	{
	}

	// copy constructor
	AttributeUpdate::AttributeUpdate(const AttributeUpdate& rhs):
		_previous_value(rhs._previous_value),
		_new_value(rhs._new_value)
	{
	}

	// destructor
	AttributeUpdate::~AttributeUpdate(void)
	{
	}
	
	Value AttributeUpdate::GetPreviousValue() const
	{
		return _previous_value;
	}

	Value AttributeUpdate::GetNewValue() const
	{
		return _new_value;
	}

	//void AttributeUpdate::SetPreviousValue(const Value &previous_value)
	//{
	//	_previous_value = previous_value;
	//}

	//void AttributeUpdate::SetNewValue(const Value &new_value)
	//{
	//	_new_value = new_value;
	//}


}
}