#include "stdafx.h"
#include "IAttributeOptionList.h"
#include "AttributeOptionList.h"

namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		* optionList gets passed by value and 'moved' in to internal variable to take ownership
		*/
		IAttributeOptionList::IAttributeOptionList(::std::unique_ptr<AttributeOptionList> optionList) :
			_optionList(::std::move(optionList))
		{
			assert(_optionList);
		}
		/**
		*
		*
		*
		*/
		IAttributeOptionList::~IAttributeOptionList()
		{

		}
		/**
		*
		*
		*
		*/
		bool IAttributeOptionList::GetValueDescription(Value &value, ::std::wstring &strValueName)
		{
			return _optionList->GetValueDescription(value, strValueName);
		}
		/**
		*
		*
		*
		*/
		bool IAttributeOptionList::GetValue(const ::std::wstring &strValueName, ::std::unique_ptr<Value>& value)
		{
			return _optionList->GetValue(strValueName, value);
		}
		/**
		*
		*
		*
		*/
		bool IAttributeOptionList::GetOption(size_t iIndex, ::std::wstring &strValueName, ::std::unique_ptr<Value>& value)
		{
			return _optionList->GetOption(iIndex, strValueName, value);
		}
		/**
		*
		*
		*
		*/
		size_t IAttributeOptionList::GetOptionListCount()
		{
			return _optionList->GetOptionListCount();
		}




		// ns
	}
}