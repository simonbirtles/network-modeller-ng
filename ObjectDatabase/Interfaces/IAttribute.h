#pragma once
#include <memory>
#include <string>
#include "ObjectDatabaseDefines.h"

namespace NM
{
	namespace ODB
	{
		class Attribute;
		class IAttributeOptionList;

		class IAttribute
		{
		public:
			explicit IAttribute(Attribute* Attr);
			~IAttribute();

			::std::wstring GetName() const;
			::std::wstring GetDescription() const;
			TypeT GetType() const;
			int	 GetFlags();
			UPVALUE GetDefaultValue() const;
			bool GetUserModify() const;
			bool IsRequiredAttribute() ;
			bool IsSystemAttribute();
			bool IsSerializable() const;
			bool DoesAttributeLockOnSet() const;
			bool HasOptionList() const;
			::std::unique_ptr<IAttributeOptionList>	 GetOptionListHandle();




		private:
			Attribute*  _attribute;
			IAttribute(const IAttribute& rhs) = delete;
			IAttribute& IAttribute::operator=(const IAttribute &rhs) = delete;
		};


// ns

	}
}

