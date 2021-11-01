#pragma once
#include "ObjectDatabaseDefines.h"


namespace NM
{
	namespace ODB
	{
		class Table;
		class AttributeOptionList;

		class ITable
		{
		public:
			explicit ITable(Table* table);
			~ITable();

			bool CreateAttribute(
				const ::std::wstring						&name,								///< String name of this Attribute
				TypeT		        						dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
				int	         								flags,                              ///< Flags - See ATTR_* flags
				const ::std::wstring						&description,						///< String description of this Attribute
				const BaseValue								*defaultValue = nullptr				///< Is the default value for attribute (optional)				
				);

			bool CreateAttribute(
				const ::std::wstring						&name,								///< String name of this Attribute
				TypeT	        							dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
				int	   										flags,                              ///< Flags - See ATTR_* flags
				const ::std::wstring						&description,						///< String description of this Attribute
				const BaseValue								*defaultValue,						///< Is the default value for attribute (optional)				
				const AttributeOptionList					&optionList							///< Options List - see ATTR_OPTIONS_LIST);
				);


			// Gets
			::std::wstring GetTableName();
			


		private:
			Table* _table;
		};

		// ns
	}
}


