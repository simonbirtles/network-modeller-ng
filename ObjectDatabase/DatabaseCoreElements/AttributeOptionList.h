#pragma once
#include "Type.h"
//#include "AttributeDefines.h"
#include <string>  
#include <vector>
#include <memory>

namespace NM 
{
	namespace ODB 
	{
		class BaseValue;
		
		class AttributeOptionList
		{
			public:
				typedef ::std::vector<::std::tuple<unsigned short, ::std::wstring, Value>> ATTR_OPTION_LIST;

				explicit AttributeOptionList(::std::wstring strAttributeListName);
				~AttributeOptionList();
				AttributeOptionList(const AttributeOptionList& rhs);					// copy ctor		
				AttributeOptionList& operator=(const AttributeOptionList &rhs);			// assignment
				
				::std::wstring GetOptionListName();
				bool GetValueDescription(Value &value, ::std::wstring &strValueName);
				bool GetValue(const ::std::wstring &strValueName, ::std::unique_ptr<Value>& value);
				bool GetOption(size_t iIndex, ::std::wstring &strValueName, ::std::unique_ptr<Value>& value);
				ATTR_OPTION_LIST GetOptionList();
				size_t GetOptionListCount();

				bool AddOption(unsigned short index, ::std::wstring optionText, Value value);
				bool RemoveOption(unsigned short index);
				bool RemoveOption(Value &value);
				void RemoveAll();

			private:
				ATTR_OPTION_LIST _optionList;
				::std::wstring	_listName;
		};
    }
}