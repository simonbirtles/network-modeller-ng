#include "stdafx.h"
#include "Value.h"
#include "RealValue.h"

namespace NM 
{
	namespace ODB 
	{

		// constructor; we should be init'd with a reference to a BaseValue, which would actually be a RealValue, 
		Value::Value(BaseValue const &bv):
			_bv(bv.Clone())
		{
		}

		// destructor
		Value::~Value()
		{
			delete _bv;
		}

		// copy constructor
		Value::Value(const Value& rhs) 
		{
			_bv = rhs._bv->Clone();
		}

		// assignment
		Value& Value::operator=(const Value &rhs)
		{
			if(_bv)
			{
				if(rhs._bv)
				{
					_bv->Set(rhs);
				}
			}

			return *this;
		}		

		// equality operator
		bool Value::operator==(const Value &rhs)
		{
			return (*_bv == *rhs._bv);
		}

		// less than operator
		bool Value::operator< (const Value &rhs)
		{
			return (*_bv < *rhs._bv);
		}

		// greater than operator
		bool Value::operator> (const Value &rhs)
		{
			return (*_bv > *rhs._bv);
		}

	}
}
