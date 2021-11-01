#pragma once
#include "Value.h"
#include <functional>
#include <assert.h>
#include <string>

#pragma warning( push )
#pragma warning( disable : 4251 )

namespace tinyxml2
{
	class XMLNode;
}

namespace NM 
{
	namespace ODB 
	{

		/**
		* Class RealValue
		*
		* The RealValue class actually holds the assigned value;
		* Has no direct relationship to Attribute (or visa versa) other than mapped (linked) in each DatabaseObject(Vertex, Edge, etc) class Attribute<->Value
		* The Value class holds a pointer to a BaseValue (abstract class) of which RealValue is the concrete class holding the T datatype and actual assigned value
		*
		*/
		template <typename T> 
		class  RealValue : public BaseValue
		//class  RealValue : public BaseValue
		{
		public:
			 RealValue(T v):
				_val(v),
				_valueLocked(false)
			{}

			 virtual ~RealValue()
			{}

			RealValue*	Clone() const = 0;
			virtual ::std::wstring GetString() = 0;
			virtual bool operator==(BaseValue const &rhs) = 0;
			virtual bool operator<(BaseValue const &rhs)   = 0;
			virtual bool operator>(BaseValue const &rhs) = 0;	
			virtual bool SetXML(tinyxml2::XMLNode *thisAttributeNode) = 0;
			virtual bool GetXML(tinyxml2::XMLNode *thisAttributeNode) = 0;
			virtual bool Set(::std::wstring const &wStrValue) = 0;

			 operator T() const
			{
				return _val; 
			}			

			 T Get()
			{
				return _val;
			}

			 bool Set(Value const & value)
			{
				if (_valueLocked) return false;
				_val = value.Get<T>();
				return true;
			}			

			 bool Set(T const value)
			{
				if (_valueLocked) return false;
				_val = value;
				return true;
			}

			 void Lock()
			{
				_valueLocked = true;
			}

			 bool IsLocked()
			{
				return _valueLocked;
			}


		private:
			T		_val;
			bool	_valueLocked;

		};

	}
}

#pragma warning( pop )