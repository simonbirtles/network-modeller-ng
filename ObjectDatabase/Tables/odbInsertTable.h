#pragma once
#include "Table.h"
#include "..\Interfaces\ObjectDatabaseDefines.h"

namespace NM
{
	namespace ODB
	{
		class DatabaseObjectFactory;

		class odbInsertTable : public Table
		{
		public:
			explicit odbInsertTable();
			~odbInsertTable();

			OBJECTUID		CreateObject(OBJECTATTRIBUTES *attributeMap IN, DatabaseObject** pObject OUT);
			bool			InsertObject(DatabaseObject* dbo);
			bool			DestroyObject(DatabaseObject* pObject, bool deleteObject = false);
			bool			RemoveObject(DatabaseObject* pObject);

			bool			SerialiseTable();
			bool			InsertAttribute(const Attribute& attribute) { return false; }


		private:
			odbInsertTable(const odbInsertTable&) = delete;				// disable copying as we have not implemented these functions and compiler wont provide default ones
			odbInsertTable& operator=(const odbInsertTable&) = delete;
		};


	}
}
