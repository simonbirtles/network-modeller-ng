#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"
//#include "..\DatabaseCoreElements\Internal.h"
#include "..\Serialization\SerialClient.h"
#include <string>
#include <memory>

namespace NM
{
	namespace Serial
	{
		class ISerial;
	}

	namespace ODB 
	{		
		class DatabaseObject;
		class Table;
		class ITable;
		class IDatabaseUpdate;
		class TriggerManager;
		class ITrigger;

		class CObjectDatabase:
			public Serial::SerialClient
		{
		public:	
			 explicit CObjectDatabase(IDatabaseUpdate* dbCache, NM::Serial::ISerial* SerialFile);
			~CObjectDatabase() ;
			
			// these are root nodes of the table tree in the xml file not actual objects
			::std::string ROOT_NODE;
			::std::string OBJECT_ELEMENT;
			::std::string ATTRIBUTE_NODE;

			OBJECTUID		CreateTableObject(::std::wstring tableName, OBJECTATTRIBUTES *attributeMap);
			bool			DeleteTableObject(OBJECTUID objectUID);	
			void			EnumerateTableNames(::std::vector<::std::wstring> &tableList);
			ITable*			GetTableInterfaceHandle(::std::wstring tableName);

			bool			SetValue(OBJECTUID objectUID, const ::std::wstring &attrname, Value const &v);
			UPVALUE			GetValue(OBJECTUID objectUID, const ::std::wstring &attrname);

			void			Clear();	// clears all tables managed by this db
			ObjectType		GetObjectType(OBJECTUID objectUID);
			::std::wstring	GetObjectTypeName(OBJECTUID objectUID);
			bool			IsValidObjectUID(OBJECTUID objectUID);
			bool			IsValidAttribute(OBJECTUID objectUID, AttributeName attributeName);

			OBJECTUID		GetFirstObject(::std::wstring &TableName);
			OBJECTUID		GetNextObject(OBJECTUID objectUID);
			OBJECTUID		GetLastObject(::std::wstring &TableName);
			bool			GetAttributeList(OBJECTUID objectUID, int filterFlags, ::std::list<::std::wstring> &attrlist);
			UPIATTRIBUTE	GetAttribute(OBJECTUID objectUID, ::std::wstring &attributeName);
			size_t			GetTableObjectCount(::std::wstring &TableName);			

			// virtuals from inheriting from Serial::SerialClient
			bool LoadXMLData(tinyxml2::XMLDocument *destDoc, tinyxml2::XMLNode *currentNode);
			bool SaveXMLData(tinyxml2::XMLDocument *destDoc, tinyxml2::XMLNode *currentNode);

			::std::shared_ptr<ITrigger>	GetTriggerInterface();
			
		private:   	
			bool								_databaseLoadInProgress;					// flag if we are opening a database and loading data from db file.	
			NM::Serial::ISerial					*_serial;
			IDatabaseUpdate						*_updateCache;
			::std::shared_ptr<TriggerManager>	_triggerManager;
				
			typedef ::std::map<OBJECTUID, Table*, compareGuid> UID_TO_TABLE_MAP;			// maps each object UID to a table (instance pointer)
			UID_TO_TABLE_MAP uid_to_table;

			typedef ::std::map<::std::string, Table*>	TABLE_NAME_TO_PTABLE;				// maps the table string name to the table instance pointer
			TABLE_NAME_TO_PTABLE name_to_table;

			OBJECTUID			CreateNewObjectUID();
			void				CreateDefaultTables();
			Table*				CreateTable(::std::wstring& tableName, ::std::wstring& objectName, ObjectType objectType);
			OBJECTUID			CreateTableObject(Table* table, OBJECTATTRIBUTES *attributeMap);
			Table*				GetTableHandle(OBJECTUID objectUID);
			Table*				GetTableHandle(::std::string tableName);
			Table*				GetTableHandle(::std::wstring tableName);
			bool				SetTableValue(Table* objectTable, OBJECTUID objectuid, const ::std::wstring &attributeName, const Value &NewValue);
			void				CreateObjectHouseKeeping(OBJECTUID uid);
			void				DeleteObjectFromMaps(OBJECTUID objectUID);					
			::std::wstring		GenerateRandomString(const int len);
			bool				ParseObjectAttributeMap(Table* dbTable, DatabaseObject* dbo, OBJECTATTRIBUTES* attributeMap);
			bool				ParseObjectXML(tinyxml2::XMLNode *xmlTableNode, Table *dbTable);
			tinyxml2::XMLNode*	parseAttributes(tinyxml2::XMLNode *xmlTableNode, Table *dbTable, DatabaseObject* dbo);

		};

// ns
	}
}
