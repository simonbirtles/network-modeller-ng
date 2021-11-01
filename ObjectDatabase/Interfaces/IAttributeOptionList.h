#pragma once
#include <memory>
#include "Interfaces\ObjectDatabaseDefines.h"

namespace NM
{
	namespace ODB
	{
		class AttributeOptionList;

		class IAttributeOptionList
		{
		public:
			explicit IAttributeOptionList(::std::unique_ptr<AttributeOptionList> optionList);
			~IAttributeOptionList();

			bool GetValueDescription(Value &value, ::std::wstring &strValueName);
			bool GetValue(const ::std::wstring &strValueName, ::std::unique_ptr<Value>& value);
			bool GetOption(size_t iIndex, ::std::wstring &strValueName, ::std::unique_ptr<Value>& value);
			size_t GetOptionListCount();

		private:
			::std::unique_ptr<AttributeOptionList>		_optionList;

		};
	}
}

