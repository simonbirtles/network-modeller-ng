#include "stdafx.h"
#include "attribute.h"
#include "AttributeOptionList.h"
#include "IAttributeOptionList.h"

namespace NM
{
	namespace ODB
	{

		Attribute::Attribute(
			const ::std::wstring				&name,								///< String name of this Attribute
			TypeT		        				dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
			int	         						flags,                              ///< Flags - See ATTR_* flags
			const ::std::wstring				&description,						///< String description of this Attribute
			const BaseValue						*defaultValue						///< Is the default value for attribute (optional)	 - takes a copy of the passed object			
			) :
			_name(name),
			_type_class(dataTypeID),
			_flags(flags),
			_description(description),
			_locked(false),
			_optionList(nullptr)
		{
			assert(!(flags & ATTR_OPTIONS_LIST));
			_defaultValue.reset(defaultValue ? defaultValue->Clone() : nullptr);
		};

		/**
		*
		* ctor with options list
		*
		*/
		Attribute::Attribute(
			const ::std::wstring						&name,								///< String name of this Attribute
			TypeT		        						dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
			int	         								flags,                              ///< Flags - See ATTR_* flags
			const ::std::wstring						&description,						///< String description of this Attribute
			const BaseValue								*defaultValue,						///< Is the default value for attribute (optional) - takes a copy of the passed object
			const AttributeOptionList					&optionList							///< Options List - see ATTR_OPTIONS_LIST - takes a copy of the passed object
			) :
			_name(name),
			_type_class(dataTypeID),
			_flags(flags),
			_description(description),
			_locked(false)//,
			//_optionList(optionList)
		{
			assert(flags & ATTR_OPTIONS_LIST);

			// store default value if passed
			_defaultValue.reset(defaultValue ? defaultValue->Clone() : nullptr);

			// create copy of passed option list and store copy
			_optionList.reset( new AttributeOptionList(optionList));
			
		};
		/**
		*
		*
		*/
		Attribute::~Attribute()
		{
		}
		/**
		* copy ctor
		*
		*/
		Attribute::Attribute(const Attribute& other) :
			_name(other._name),
			_type_class(other._type_class),
			_flags(other._flags),
			_description(other._description),
			_locked(other._locked),
			_optionList(other._optionList ? ::std::make_unique<AttributeOptionList>(*other._optionList) : nullptr),
			_defaultValue(other._defaultValue ? other._defaultValue->Clone() : nullptr)
		{

		}
		/**
		* assignment op
		*
		*
		*/
		Attribute& Attribute::operator =(const Attribute& rhs)
		{
			_name = rhs._name;
			_type_class = rhs._type_class;
			_flags = rhs._flags;
			_description =rhs._description;
			_locked = rhs._locked;
			_optionList.reset(new AttributeOptionList(*rhs._optionList));
			_defaultValue.reset(rhs._defaultValue ? rhs._defaultValue->Clone() : nullptr);
			return *this;
		}
		/**
		*
		* Returns the string name of this Attribute
		*/
		::std::wstring Attribute::GetName() const
		{
			return _name;
		}
		/**
		*
		* Returns the string description of this Attribute
		*/
		::std::wstring Attribute::GetDescription() const
		{
			return _description;
		}
		/**
		*
		* Returns the CType datatype of this Attribute i.e. String, Int, etc TypeT
		*/
		TypeT Attribute::GetType() const
		{
			return _type_class.GetType();			// return a reference to the CType class for this Attribute
		}
		/**
		*
		*
		* Return flags as set in _flags.
		*/
		int Attribute::GetFlags()
		{
			return _flags;
		}
		/**
		*
		*
		*/
		bool Attribute::IsUserAttribute() const
		{
			return (_flags & ATTR_USER_CREATE ? true : false);
		}
		/**
		*
		* Returns boolean based on if the attributes metadata can be changed or not
		*/
		bool Attribute::GetUserModify() const
		{
			return (_flags & ATTR_USER_MODIFY ? true : false); //_usermodify;
		}
		/**
		*
		* If this returns True, then the value MUST be set when containing object is created and defaults will not be used.
		*/
		bool Attribute::IsRequiredAttribute() const
		{
			return (_flags & ATTR_VALUE_REQUIRED_ON_CREATE ? true : false);// _required;
		}
		/**
		*
		* If this returns true, then this Attributes Value must be saved to the disk.
		*/
		bool Attribute::IsSerializable() const
		{
			return (_flags & ATTR_SERIALISE ? true : false);  //_serialize;
		}
		/**
		* GetDefaultValue
		*
		* Returns a copy of the default 'Value', if one was given when the attibute was saved, then use that else use the datatype default which is again a copy
		*/
		::std::unique_ptr<BaseValue> Attribute::GetDefaultValue() 
		{
			return ::std::unique_ptr<BaseValue>(_defaultValue ? _defaultValue->Clone() : _type_class.NewValue());
		}
		/**
		*
		*
		*
		*/
		bool Attribute::AttributeLocked() const
		{
			return _locked;
		}
		/**
		*
		*
		*
		*/
		bool Attribute::AttributeLockedOnSet() const
		{
			return (_flags & ATTR_LOCKED_ON_SET ? true : false); //_lockedOnceSet;
		}
		/**
		*
		*
		*
		*/
		bool Attribute::HasOptionList() const
		{
			return (_flags & ATTR_OPTIONS_LIST ? true : false);
		}
		/**
		* GetOptionList
		*
		* Returns a copy of the AttributeOptionList. - ** Should Return an IAttributeOptionList
		* Do we really need to pass back in a smart pointer ? pass back as a reference to a new heap instance instead ?
		*/
		::std::unique_ptr<IAttributeOptionList> Attribute::GetOptionListHandle()
		{
			if (!HasOptionList()) return nullptr;

			// 1. creates a copy of the local AttributeOptionList into a new unique_ptr
			// 2. creates (and returns) a unique_ptr of a IAttributeOptionList which is constructed with the unique_ptr from step 1.
			return ::std::make_unique<IAttributeOptionList>(::std::unique_ptr<AttributeOptionList>(::std::make_unique<AttributeOptionList>( *_optionList) ));
		}
		/**
		*
		*
		*/
		bool Attribute::IsSystemAttribute() const
		{
			return _flags & ATTR_SYSTEM;
		}



		// ns
	}
}