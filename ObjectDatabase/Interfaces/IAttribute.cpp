#include "stdafx.h"
#include "IAttribute.h"
#include "Attribute.h"
#include "IAttributeOptionList.h"
#include "ObjectDatabaseDefines.h"

namespace NM
{
	namespace ODB
	{


		IAttribute::IAttribute(Attribute* Attr) :
			_attribute(Attr)
		{
		}

		IAttribute::~IAttribute()
		{
		}

		/**
		*
		* Returns the string name of this Attribute
		*/
		::std::wstring IAttribute::GetName() const
		{
			if (!_attribute) return L"";
			return _attribute->GetName();
		}
		/**
		*
		* Returns the string description of this Attribute
		*/
		::std::wstring IAttribute::GetDescription() const
		{
			if (!_attribute) return L"";
			return _attribute->GetDescription();
		}
		/**
		*
		* Returns the TypeT datatype of this Attribute i.e. String, Int, etc TypeT
		*/
		TypeT IAttribute::GetType() const
		{
			if (!_attribute) return TypeT::TypeUnknown;
			return _attribute->GetType();		
		}
		/**
		* GetFlags
		* Returns integer value for set flags on attribute. Flags are ATTR_*
		*/
		int	IAttribute::GetFlags()
		{
			if (!_attribute) return 0;
			return _attribute->GetFlags();
		}
		/**
		*
		* Returns boolean based on if the attributes metadata can be changed or not
		*/
		bool IAttribute::GetUserModify() const
		{
			if (!_attribute) return false;
			return _attribute->GetUserModify();
		}
		/**
		*
		* If this returns True, then the value MUST be set when containing object is created and defaults will not be used.
		*/
		bool IAttribute::IsRequiredAttribute() 
		{
			if (!_attribute) return false;
			return _attribute->IsRequiredAttribute();
		}
		/**
		*
		*
		*
		*/
		bool IAttribute::IsSystemAttribute()
		{
			if (!_attribute) return false;
			return _attribute->IsSystemAttribute();
		}
		/**
		*
		* If this returns true, then this Attributes Value must be saved to the disk.
		*/
		bool IAttribute::IsSerializable() const
		{
			if (!_attribute) return false;
			return _attribute->IsSerializable();
		}
		/**
		* GetDefaultValue
		*
		* Returns a copy of the default Value 
		*/
		UPVALUE IAttribute::GetDefaultValue() const
		{
			if (!_attribute) return nullptr;
			return UPVALUE( new Value(*_attribute->GetDefaultValue() ) );
		}
		/**
		*
		*
		*
		*/
		bool IAttribute::DoesAttributeLockOnSet() const
		{
			if (!_attribute) return false;
			return _attribute->AttributeLockedOnSet();
		}
		/**
		*
		*
		*
		*/
		bool IAttribute::HasOptionList() const
		{
			if (!_attribute) return false;
			return _attribute->HasOptionList();
		}
		/**
		* GetOptionList
		*
		* Returns base class for copy of option list class. Must be cast to AttributeOptionList<T>
		*/
		::std::unique_ptr<IAttributeOptionList> IAttribute::GetOptionListHandle()
		{
			if (!_attribute) return false;
			return _attribute->GetOptionListHandle();
		}
		//ns
	}
}
