#pragma once
#include "Table.h"
#include "..\Interfaces\ObjectDatabaseDefines.h"

namespace NM
{
	namespace ODB
	{
		class DatabaseObjectFactory;

		class odbDeleteTable : public Table
		{
		public:
			explicit odbDeleteTable();
			~odbDeleteTable();

			OBJECTUID		CreateObject(OBJECTATTRIBUTES *attributeMap IN, DatabaseObject** pObject OUT);
			bool			InsertObject(DatabaseObject* dbo);
			bool			DestroyObject(DatabaseObject* pObject, bool deleteObject = false);
			bool			RemoveObject(DatabaseObject* pObject);

			bool			SetValue(DatabaseObject*, size_t idx, Value const &v);
			bool			SetValue(DatabaseObject*, const ::std::wstring &attrname, Value const &v);
			bool			InsertAttribute(const Attribute& attribute) { return false; }

			bool			SerialiseTable();


		private:
			odbDeleteTable(const odbDeleteTable&) = delete;				// disable copying as we have not implemented these functions and compiler wont provide default ones
			odbDeleteTable& operator=(const odbDeleteTable&) = delete;
		};


	}
}
