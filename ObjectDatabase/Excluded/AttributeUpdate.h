#pragma once
#include "..\DatabaseCoreElements\Value.h"

namespace NM {
	namespace ODB {

	class AttributeUpdate
	{
	public:
		AttributeUpdate(Value previous_value, Value new_value);		// constructor
		~AttributeUpdate(void);										// destructor
		AttributeUpdate(const AttributeUpdate& rhs) ;				// copy constructor

		Value GetPreviousValue() const;
		Value GetNewValue() const; 

		//void SetPreviousValue(const Value &previous_value);
		//void SetNewValue(const Value &new_value);

	private:
		AttributeUpdate &operator=(const AttributeUpdate &rhs);		// assignment		
		Value	_previous_value;
		Value	_new_value;
	};	

}
}