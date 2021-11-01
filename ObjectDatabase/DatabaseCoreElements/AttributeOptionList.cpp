#include "stdafx.h"
#include "AttributeOptionList.h"

#define ATTR_OPTION_LIST_INDEX			0x00
#define ATTR_OPTION_LIST_STRING         0x01
#define ATTR_OPTION_LIST_VALUE          0x02    

namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		* ctor
		*/
		AttributeOptionList::AttributeOptionList(::std::wstring strAttributeListName):
			_listName(strAttributeListName)
		{
			if (_listName.size() == 0)
			{
				throw ::std::logic_error("Attribute List Name Length = 0");
			}
		};
		/**
		*
		*
		* dtor
		*/
		AttributeOptionList::~AttributeOptionList()
		{
		}
		/**
		*
		*
		* copy ctor
		*/
		AttributeOptionList::AttributeOptionList(const AttributeOptionList& rhs) :
			_optionList(rhs._optionList)
		{
		}
		/**
		*
		*
		* assignment
		*/
		AttributeOptionList& AttributeOptionList::operator=(const AttributeOptionList &rhs)	// assignment
		{
			if (&rhs == this) return *this;
			_optionList = rhs._optionList;
			return *this;
		}
		/**
		* AddOption
		*
		* Adds option to option list - index currently ignored and replaced with internal index number
		*/
		bool AttributeOptionList::AddOption(unsigned short index, ::std::wstring optionText, Value value)
		{
			// check for existing index / option
			_optionList.push_back(
				::std::make_tuple(
					static_cast<unsigned short>(_optionList.size() + 1),
					optionText, 
					value
					));

			return true;
		}
		/**
		*
		*
		*
		*/
		bool AttributeOptionList::RemoveOption(unsigned short index)
		{
			return false;
		}
		/**
		*
		*
		*
		*/
		bool AttributeOptionList::RemoveOption(Value &value)
		{
			return false;
		}
		/**
		*
		*
		*
		*/
		void AttributeOptionList::RemoveAll()
		{
			_optionList.clear();
		}
		/**
		*
		*
		*
		*/
		::std::wstring AttributeOptionList::GetOptionListName()
		{
			return _listName;
		}
		/**
		* GetValueDescription
		*
		* search for matching BaseValue and return value description
		*/
		bool AttributeOptionList::GetValueDescription(Value &value, ::std::wstring &strValueName)
		{
			ATTR_OPTION_LIST::iterator it = _optionList.begin();
			while (it != _optionList.end())
			{
				if (::std::get<ATTR_OPTION_LIST_VALUE>(*it) == value)
				{
					strValueName = ::std::get<ATTR_OPTION_LIST_STRING>(*it);
					return true;
				}
				++it;
			}
			return false;
		}
		/**
		* GetValue
		*
		* Returns Option value given the option name
		*/
		bool AttributeOptionList::GetValue(const ::std::wstring &strValueName, ::std::unique_ptr<Value>& value)
		{
			ATTR_OPTION_LIST::iterator it = _optionList.begin();
			while (it != _optionList.end())
			{
				if (::std::get<ATTR_OPTION_LIST_STRING>(*it) == strValueName)
				{
					// create 'new' copy of stored value on the heap and pass pack pointer in unique_ptr
					value.reset( new Value( ::std::get<ATTR_OPTION_LIST_VALUE>(*it)) );
					return true;
				}
				++it;
			}
			return false;
		}
		/**
		* GetOption
		*
		* Given a valid index, both a copy of the string option name and value is passed back
		*/
		bool AttributeOptionList::GetOption(size_t iIndex, ::std::wstring &strValueName, ::std::unique_ptr<Value>& value)
		{
			if (iIndex >= _optionList.size())
			{
				strValueName = L"";
				value.reset();
				return  false;
			}

			strValueName = ::std::get<ATTR_OPTION_LIST_STRING>(_optionList.at(iIndex));
			value.reset(new Value(::std::get<ATTR_OPTION_LIST_VALUE>(_optionList.at(iIndex))));
			return true;
		}
		/**
		* GetOptionList
		*
		* Returns a copy of the option list
		*/
		AttributeOptionList::ATTR_OPTION_LIST AttributeOptionList::GetOptionList()
		{
			return _optionList;
		}
		/**
		* GetOptionListCount
		*
		* Returns number of options
		*/
		size_t AttributeOptionList::GetOptionListCount()
		{
			return _optionList.size();
		}
		
		// ns
	}
}