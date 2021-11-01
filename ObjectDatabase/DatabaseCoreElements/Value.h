#pragma once
#include "BaseValue.h"
#include <string>
#include <stdexcept>

namespace NM 
{
	namespace ODB 
	{
		class BaseValue;

		// construct like:
		//
		// Value v(RealValue<int>(26));
		//
		class  Value
		{
		public:
			// constructor; we are init'd with a reference to a BaseValue, which would actually be a RealValue, 
			Value(BaseValue const &bv);

			// copy constructor
			Value(const Value& rhs);

			// assignment
			Value &operator=(const Value &rhs);

			// destructor
			~Value();
		
			/**
			* Get()
			* Returns the value as its native type i.e. int, double, float etc
			*/
			template <typename T> 
			T Get() const
			{
				if(_bv)
				{
					RealValue<T> const &rv =  reinterpret_cast<RealValue<T> const &>( *_bv );
					return rv;			 
				}
				throw std::invalid_argument( "Value.h T Get() - _bv is null" );
			}

			bool Set(const ::std::wstring &strValue)
			{
				if(_bv)
				{
					return _bv->Set(strValue);			 
				}
				throw std::invalid_argument( "Value.h Set() - _bv is null" );
			}

			::std::wstring GetStringValue()
			{
				if (_bv)
				{
					return _bv->GetString();
				}
				return L"";
			}

			// equality operator
			bool operator==(const Value &rhs);

			// less than operator
			bool operator< (const Value &rhs);

			// greater than operator
			bool operator> (const Value &rhs);


		private:
			BaseValue	*_bv;	

		};
//ns
	}
}