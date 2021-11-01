#include "stdafx.h"
#include "ITable.h"
#include "Tables\Table.h"
#include "DatabaseCoreElements\attribute.h"

namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		*
		*/
		ITable::ITable(Table* table) :
			_table(table)
		{
			assert(_table);
		}
		/**
		*
		*
		*
		*/
		ITable::~ITable()
		{
		}
		/**
		*
		*
		*
		*/
		bool ITable::CreateAttribute(
			const ::std::wstring						&name,								///< String name of this Attribute
			TypeT		        						dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
			int	         								flags,                              ///< Flags - See ATTR_* flags
			const ::std::wstring						&description,						///< String description of this Attribute
			const BaseValue								*defaultValue						///< Is the default value for attribute (optional)				
			)
		{
			// ensure user (non app core) created flag set
			return _table->InsertAttribute(Attribute(name, dataTypeID, flags | ATTR_USER_CREATE, description, defaultValue));
		}
		/**
		*
		*
		*
		*/
		bool ITable::CreateAttribute(
			const ::std::wstring						&name,								///< String name of this Attribute
			TypeT	        							dataTypeID,							///< DataType (TypeT) of this Attribute (Value)
			int	   										flags,                              ///< Flags - See ATTR_* flags
			const ::std::wstring						&description,						///< String description of this Attribute
			const BaseValue								*defaultValue,						///< Is the default value for attribute (optional)				
			const AttributeOptionList					&optionList							///< Options List - see ATTR_OPTIONS_LIST);
			)
		{
			// ensure user (non app core) created flag set
			return _table->InsertAttribute(Attribute(name, dataTypeID, flags | ATTR_USER_CREATE, description, defaultValue, optionList));
		}

	}
}
