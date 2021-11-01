#pragma once
#include "AttributeDefines.h"
#include "Type.h"
#include <string>
#include <memory>

namespace NM 
{
	namespace ODB 
	{
		class IAttributeOptionList;
		class AttributeOptionList;
		class BaseValue;

		class Attribute
		{
		public: 

			// ctor without options list
			Attribute(
				const ::std::wstring					&name,								///< String name of this Attribute
				TypeT		        					dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
                int	         							flags,                              ///< Flags - See ATTR_* flags
				const   ::std::wstring					&description,						///< String description of this Attribute
				const 	BaseValue						*defaultValue = nullptr				///< Is the default value for attribute (optional)				
					);

			
			// ctor with options list
			Attribute(
				const ::std::wstring					&name,								///< String name of this Attribute
				TypeT		        					dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
				int	         							flags,                              ///< Flags - See ATTR_* flags
				const ::std::wstring					&description,						///< String description of this Attribute
				const BaseValue							*defaultValue,						///< Is the default value for attribute (optional)				
				const AttributeOptionList				&optionList							///< Options List - see ATTR_OPTIONS_LIST
				);

			~Attribute();

			Attribute(const Attribute& other);					// copy op
			Attribute& operator =(const Attribute& rhs);		// assignment op.

			::std::wstring	GetName() const;
			::std::wstring	GetDescription() const;
			TypeT			GetType() const;
			int				GetFlags();
			bool			GetUserModify() const;
			bool			IsUserAttribute() const;
			bool			IsRequiredAttribute() const;
			bool			IsSerializable() const;
			bool			IsSystemAttribute() const;
			bool			AttributeLocked() const;
			bool			AttributeLockedOnSet() const;
			bool			HasOptionList() const;
			::std::unique_ptr<BaseValue>			GetDefaultValue();
			::std::unique_ptr<IAttributeOptionList>	GetOptionListHandle();


		private:	
			::std::wstring								_name;						///< name of attribute - used for finding attributes must be unique(??) in object
			::std::wstring								_description;				///< description of this attribute
			CType										_type_class;				///< Instance of Class CType, created in constructor of TypeT which is integer value of enum describing type, i.e. int, float, string, bool etc...
			int			             					_flags;
			bool										_locked;					///< if _lockedOnceSet is true, then once set, this flag will be true to prevent value changes
			::std::unique_ptr<BaseValue>				_defaultValue;				///< Is the default value for attribute (optional)
			::std::unique_ptr<AttributeOptionList>		_optionList;
		};
// ns
	}
}


