#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"
#include <string>
#include <vector>


namespace NM 
{
	namespace ODB 
	{		
		typedef ::std::unique_ptr<BaseValue> UPBASEVALUE;

		class DatabaseObject;
		class Value;
		class Sibling;
		class Attribute;

		class Table
		{
		public:
			Table(::std::wstring& tableName, ::std::wstring& objectName, ObjectType objectType, bool systemTable = false);
			virtual ~Table();

			bool					IsSystemTable(){ return _systemTable; };
			::std::wstring			GetTableName(){ return _tableName;}
			DatabaseObject*			CreateObject();
			virtual OBJECTUID		CreateObject(OBJECTATTRIBUTES *attributeMap IN, DatabaseObject** pObject OUT);
			bool					InsertObject(DatabaseObject* dbo);
			bool					DestroyObject(OBJECTUID objectUID, bool deleteObject = false);
			virtual bool			SetValue(OBJECTUID objectUID, const ::std::wstring &attrname, Value const &v);
			virtual UPBASEVALUE		GetValue(OBJECTUID objectUID, const ::std::wstring &attrname) ;
			void					Clear();
			virtual bool			SerialiseTable();
			virtual DatabaseObject* GetObjectPtr(OBJECTUID objectUID);
			OBJECTUID				GetFirstObject();
			OBJECTUID				GetLastObject();	

			virtual bool			InsertAttribute(const Attribute& attribute);
			virtual bool			IsAttribute(OBJECTUID objectUID, const ::std::wstring &attrname);
			virtual void			GetAttributeList(OBJECTUID objectUID, int filterFlags, ::std::list<::std::wstring> &attrlist);
			virtual bool			GetAttribute(OBJECTUID objectUID, ::std::wstring &attributeName, Attribute **attribute);
			virtual ::std::wstring	GetObjectTypeName(OBJECTUID objectUID);

			ObjectType				GetObjectType(){ return _objectType; };
			size_t					GetTableObjectCount() { return _objects.size(); };
			bool					MoveObjectToTable(Table* moveToTable, OBJECTUID objectUID);

			// move to protected ?
			

		protected:			
			virtual void	CreateDefaultAttributes();
			void			AddSibling(DatabaseObject *graphObject);
			void			DeleteSibling(DatabaseObject *graphObject);
			
			typedef ::std::vector<DatabaseObject*> OBJECT_VECTOR;
			OBJECT_VECTOR		_objects;		//  holds pointers to DatabaseObject instances	
			Sibling*			_siblings;

			// maps the UID (Unique ID of an object to the object pointer)
			typedef ::std::map<OBJECTUID, DatabaseObject*, compareGuid> UID_TO_POINTER_MAP;
			UID_TO_POINTER_MAP	uid_to_objectpointer;

		private:
			bool				_systemTable;
			::std::wstring		_tableName;
			::std::wstring		_objectName;
			ObjectType			_objectType;
			bool				_serialise;
			DatabaseObject*		_defaultObject;
			typedef ::std::map<::std::wstring, Attribute*> ATTR_MAP;
			ATTR_MAP _attributes;		// AttributeName -> Attribute*

			bool RecieveMovedObject(DatabaseObject* dbo);

			Table(const Table& other) = delete;
			Table& operator=(const Table& other) = delete;


		};
	}
}

