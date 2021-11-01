#pragma once
#include <string>

namespace tinyxml2
{
	class XMLDocument;
	class XMLNode;
}

namespace NM 
{
	namespace ODB 
	{		
		class Value;
		/**
		*
		* Class BaseValue
		*
		* abstract baseclass for class RealValue
		*
		*/
		class  BaseValue
		{
		public:
			virtual					~BaseValue()	{}
			virtual BaseValue*		Clone() const = 0;
			virtual bool			Set(Value const & v) = 0;
			virtual bool			Set(::std::wstring const &wStrValue) = 0;
			virtual ::std::wstring	GetString() = 0;
			virtual	bool			operator==(BaseValue const &rhs) = 0;
			virtual	bool			operator<(BaseValue  const &rhs)  = 0;
			virtual	bool			operator>(BaseValue const &rhs) = 0;
			virtual bool			SetXML(tinyxml2::XMLNode *thisAttributeNode) = 0;
			virtual bool			GetXML(tinyxml2::XMLNode *thisAttributeNode) = 0;
			virtual void			Lock() = 0;
			virtual bool			IsLocked() = 0;

		protected:
			BaseValue() {};

		private:

		};
	}
}