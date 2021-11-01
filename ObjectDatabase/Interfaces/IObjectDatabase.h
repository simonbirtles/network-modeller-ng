#pragma once
#include "ObjectDatabaseDefines.h"
#include <memory>
#include <list>

namespace NM 
{
	namespace ODB 
	{
		class CObjectDatabase;
		class ITrigger;
		class ITable;

		/**
		* class IObjectDatabase
		*					
		*/
		class IObjectDatabase
		{
		public:
			IObjectDatabase(CObjectDatabase**);
			~IObjectDatabase(void);

			ITable*		GetTableInterfaceHandle(::std::wstring tableName);
			size_t		GetTableObjectCount(::std::wstring &TableName);
			void		EnumerateTableNames(::std::vector<::std::wstring> &tableList);

			OBJECTUID	CreateVertex(OBJECTATTRIBUTES		*attribute_map);	
			OBJECTUID	CreateInterface(OBJECTATTRIBUTES	*attribute_map);
			OBJECTUID	CreateEdge(OBJECTATTRIBUTES			*attribute_map);
			OBJECTUID	CreateFlow(OBJECTATTRIBUTES			*attribute_map);	
			OBJECTUID	CreateDemand(OBJECTATTRIBUTES		*attribute_map);	
			OBJECTUID	CreatePath(OBJECTATTRIBUTES			*attribute_map);	
			OBJECTUID	CreateLayer(OBJECTATTRIBUTES		*attribute_map);	
			OBJECTUID	CopyVertex(OBJECTUID sourceObjectUID);												
			bool		DeleteObject(OBJECTUID objectUID);								

			bool		SetValue(OBJECTUID objectuid, const ::std::wstring &attrname, Value const &v);
			UPVALUE		GetValue(OBJECTUID objectuid, const ::std::wstring &attrname);
			bool		IsValidObjectUID(OBJECTUID objectUID);
			OBJECTUID	GetFirstObject(::std::wstring &TableName);
			OBJECTUID	GetNextObject(OBJECTUID objectUID);
			OBJECTUID	GetLastObject(::std::wstring &TableName);
			bool		IsValidAttribute(OBJECTUID, AttributeName);
			bool		GetAttributeList(OBJECTUID objectUID, int filterFlags, ATTRIBUTELIST& attrlist);
			UPIATTRIBUTE GetAttribute(OBJECTUID objectUID, ::std::wstring &attributeName);

			//IAdjacencyMatrix	GetAdjacencyMatrix();
			::std::wstring GetObjectTypeName(OBJECTUID objectUID);

			::std::shared_ptr<ITrigger>	GetTriggerInterface();

		private:
			CObjectDatabase** _odb;
		};

	// ns
	}
}

