#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"

namespace NM 
{
	namespace ODB 
	{
		class DatabaseObject;
		class Table;

		class Sibling
		{
		public:
			explicit Sibling(Table *pTable);
			~Sibling(void);

			void			AddSibling(DatabaseObject *graphObject);
			void			DeleteSibling(DatabaseObject *graphObject);
			DatabaseObject*	GetSibling(SiblingPosition position);

		private:
			Sibling(){};

			Table*			_table;							//> ObjectDatabase pointer
			DatabaseObject*		_pFirstObject;				//> Save the first object created as the start of the chain - Object UID
			DatabaseObject*		_pLastObject;					//> Save the last object created as the end of the chain - Object UID

		};


	}
}
