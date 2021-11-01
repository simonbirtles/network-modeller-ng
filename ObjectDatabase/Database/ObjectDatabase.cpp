#include "stdafx.h"
#include "ObjectDatabase.h"
#include ".\Factory\DatabaseObjectFactory.h"

#include "DatabaseCoreElements\Value.h"
#include "DatabaseCoreElements\Type.h"
#include "DatabaseCoreElements\attribute.h"
#include "DataObjects\DatabaseObject.h"

#include ".\Tables\Table.h"
#include ".\Tables\odbInsertTable.h"
#include ".\Tables\odbDeleteTable.h"
#include "CoreTriggers.h"
#include "TriggerManager.h"

#include "Interfaces\ITable.h"
#include "Interfaces\IAttribute.h"
#include "Interfaces\IObjectDatabase.h"	
#include "Interfaces\IDatabaseUpdate.h"									// to call insertupdate (DatabaseUpdateRecord&)
#include "Interfaces\ITrigger.h"
#include "..\Serialization\ISerial.h"
			
#include ".\DatabaseUpdateElements\DatabaseUpdateRecord.h"				// definition for insertupdate param

#include <sstream>
#include <time.h>
#include <list>
#include <memory>
#include <set>

extern void WCtMB(const ::std::wstring &ws, ::std::string &mbs);
extern void MBtWC(::std::wstring &ws, const ::std::string &mbs);


/****************************************************************************************************************************************************************************
* WORK TO BE DONE......
*
* 8. CopyVertex (object) needs work
******************************************************************************************************************************************************************************/

namespace NM 
{
	namespace ODB 
	{
		/**
		* 
		*
		* 
		*/
		CObjectDatabase::CObjectDatabase(IDatabaseUpdate* dbCache, NM::Serial::ISerial* SerialFile):
			_updateCache(dbCache),
			_databaseLoadInProgress(false),
			_serial(SerialFile)
		{
			// Private constructor to create data type prototypes 
			CType::InitDataTypes();

			ROOT_NODE		=	"odb";
			// no need for these as we have the table header above for these roots, so should just call each object a dbobject / DBO or Element ?!?!?!
			// but they are currently used in this file			
			OBJECT_ELEMENT  =   "tableobject";
			ATTRIBUTE_NODE  =   "attribute";

			// start the db trigger manager
			_triggerManager.reset( new TriggerManager() );

			// create app required default tables.
			CreateDefaultTables();

			// create core db triggers
			CoreTriggers::CreateCoreTriggers(_triggerManager);

			// register with file service
			if(_serial)
				_serial->RegisterClientSerializationService(this, ::std::wstring(L"odb") );

		}
		/**
		* 
		*
		* 
		*/
		CObjectDatabase::~CObjectDatabase(void)
		{  
			if (_serial)
			{
				_serial->UnregisterClientSerializationService(this);
				// we dont own this pointer, it gets passed to use from service - prob should use smart pointer ?
				//delete _serial;
				//_serial = nullptr;
			}

			// shutdown tables
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.begin();
			while (it != name_to_table.end())
			{
				delete it->second;
				it->second = nullptr;
				++it;
			}
			name_to_table.clear();

			CType::CleanUpDataTypes();
		}
		/****************************************************************************************************************************************
		* 
		*											Helper Functions
		*
		****************************************************************************************************************************************/
		/**
		* DeleteObjectFromMaps
		*
		* Removes the object from the ObjectDatabase lookup maps
		*/
		void CObjectDatabase::DeleteObjectFromMaps(OBJECTUID objectUID)
		{
			UID_TO_TABLE_MAP::iterator uid_to_db_it = uid_to_table.find(objectUID);
			if(uid_to_db_it != uid_to_table.end())
			{
				uid_to_table.erase(uid_to_db_it);
			}
		}
		/**
		*
		*
		*
		*
		*/
		Table* CObjectDatabase::GetTableHandle(OBJECTUID objectUID)
		{
			UID_TO_TABLE_MAP::const_iterator dit = uid_to_table.find(objectUID);
			if(dit != uid_to_table.end() )
			{
				return dit->second;
			}
			return nullptr;
		}
		/**
		*
		*
		*
		*/
		Table* CObjectDatabase::GetTableHandle(::std::string tableName)
		{
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.find(tableName);
			if (it != name_to_table.end())
				return it->second;

			return nullptr;
		}
		/**
		*
		*
		*
		*/
		Table* CObjectDatabase::GetTableHandle(::std::wstring tableName)
		{
			::std::string name;
			WCtMB(tableName, name);
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.find(name);
			if (it != name_to_table.end())
				return it->second;

			return nullptr;
		}
		/**
		* GetObjectType
		*
		* Returns the enum class ObjectType for a given object UID, i.e. Vertex, Edge, Group, layer etc
		*/
		ObjectType CObjectDatabase::GetObjectType(OBJECTUID objectUID)
		{
			OutputDebugString(L"\n ** CObjectDatabase::GetObjectType Called... **");
			Table* objectTable = GetTableHandle(objectUID);
			if( !objectTable )
			{
				return ObjectType::ObjectInvalid;
			}			
			return objectTable->GetObjectType();
		}
		/**
		*
		*
		*
		*/
		::std::wstring CObjectDatabase::GetObjectTypeName(OBJECTUID objectUID)
		{
			Table* objectTable = GetTableHandle(objectUID);
			if (!objectTable)
			{
				return L"";
			}

			return objectTable->GetObjectTypeName(objectUID);
		}
		/**
		*
		*
		*
		*/
		bool CObjectDatabase::IsValidObjectUID(OBJECTUID objectUID)
		{
			return( GetTableHandle(objectUID) != nullptr );
		}
		/**
		*
		*
		*
		*/
		bool CObjectDatabase::IsValidAttribute(OBJECTUID objectUID, AttributeName attributeName)
		{
			Table* objectTable = GetTableHandle(objectUID);
			if (!objectTable)
			{
				return false;
			}

			return objectTable->IsAttribute(objectUID, attributeName);
		}
		/**
		*
		*
		*
		*/
		void CObjectDatabase::Clear()
		{
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.begin();
			while (it != name_to_table.end())
			{
				it->second->Clear();
				++it;
			}
			return;
		}
		/**
		* GenerateRandomString()
		* 
		* 
		*
		*/
		::std::wstring CObjectDatabase::GenerateRandomString(const int len) 
		{
			::std::wstring s;
			for (int i = 0; i < len; ++i) 
			{
				int randomChar = rand()%(26+26+10);

				if (randomChar < 26)
				{
					s.push_back('a' + randomChar);
				}
				else if (randomChar < 26+26)
				{	
					s.push_back('A' + randomChar - 26);
				}
				else
				{
					s.push_back('b' + randomChar - 26 - 26);
				}
			}
			s.push_back(0);

			return ::std::move( s );
		}
	
		/****************************************************************************************************************************************
		* 
		*												Lookup Functions
		*
		****************************************************************************************************************************************/
		/**
		*
		*
		*
		*/
		::std::shared_ptr<ITrigger> CObjectDatabase::GetTriggerInterface()
		{
			return ::std::shared_ptr<ITrigger>(new ITrigger(_triggerManager));
		}

#pragma region CRUDOBJECTS
		/****************************************************************************************************************************************
		* 
		*												Table Creation Function
		*
		****************************************************************************************************************************************/
		/**
		* CreateDefaultTables
		*
		* Create internal and default required tables.
		*/
		void CObjectDatabase::CreateDefaultTables()
		{			
			
			Table* newTable = nullptr;
			bool bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"vertextable"), ::std::wstring(L"vertex"), ObjectType::Vertex);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreateVertexTableSchema(newTable);
			assert(bSchemaCreated);

			newTable = nullptr;
			bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"interfacetable"), ::std::wstring(L"interface"), ObjectType::Interface);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreateInterfaceTableSchema(newTable);
			assert(bSchemaCreated);

			newTable = nullptr;
			bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"edgetable"), ::std::wstring(L"edge"), ObjectType::Edge);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreateEdgeTableSchema(newTable);
			assert(bSchemaCreated);

			newTable = nullptr;
			bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"flowtable"), ::std::wstring(L"flow"), ObjectType::Flow);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreateFlowTableSchema(newTable);
			assert(bSchemaCreated);

			newTable = nullptr;
			bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"demandtable"), ::std::wstring(L"demand"), ObjectType::Demand);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreateDemandTableSchema(newTable);
			assert(bSchemaCreated);

			newTable = nullptr;
			bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"pathtable"), ::std::wstring(L"path"), ObjectType::Path);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreatePathTableSchema(newTable);
			assert(bSchemaCreated);

			newTable = nullptr;
			bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"grouptable"), ::std::wstring(L"group"), ObjectType::Group);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreateGroupTableSchema(newTable);
			assert(bSchemaCreated);

			newTable = nullptr;
			bSchemaCreated = false;
			newTable = CreateTable(::std::wstring(L"layertable"), ::std::wstring(L"layer"), ObjectType::Layer);
			assert(newTable);
			bSchemaCreated = DefaultTables::CreateLayerTableSchema(newTable);
			assert(bSchemaCreated);

			// create internal db tables, these are derived tables overriding funcs not required in Table
			::std::string mbStr;

			odbInsertTable* _insertTable = new odbInsertTable();
			WCtMB(_insertTable->GetTableName(), mbStr);
			name_to_table[mbStr] = _insertTable;

			odbDeleteTable* _deleteTable = new odbDeleteTable();
			WCtMB(_deleteTable->GetTableName(), mbStr);
			name_to_table[mbStr] = _deleteTable;

			return;
		}
		/**
		*
		*
		*
		*/
		OBJECTUID CObjectDatabase::CreateNewObjectUID()
		{
			// create new UID
			OBJECTUID newUID = INVALID_OBJECT_UID;
			UuidCreate(&newUID);
			return newUID;
		}
		/**
		* CreateTable
		* Private
		* Generic table creation
		* defaultObject is a DatabaseObject with attributes created and assigned. No values set, used for new object creation in table
		*/
		Table* CObjectDatabase::CreateTable(::std::wstring& tableName, ::std::wstring& objectName, ObjectType objectType)
		{
			// create new table
			Table* newTable = new Table(tableName, objectName, objectType);
			// convert name to string and save name to table* reference in lookup
			::std::string mbStr;
			WCtMB(tableName, mbStr);
			name_to_table[mbStr] = newTable;

			return newTable;
		}
		/****************************************************************************************************************************************
		*
		*												Create/Delete/Copy Object Functions
		*
		****************************************************************************************************************************************/
		/**
		* CreateTableObject
		*
		* Public
		*
		*/
		OBJECTUID CObjectDatabase::CreateTableObject(::std::wstring tableName, OBJECTATTRIBUTES *attributeMap)
		{
			::std::string mbStr;
			WCtMB(tableName, mbStr);
			Table* table = GetTableHandle(mbStr); 
			if (!table) return INVALID_OBJECT_UID;
			return CreateTableObject(table, attributeMap);
		}
		/**
		* CreateTableObject
		*
		* Private
		*
		*/
		OBJECTUID CObjectDatabase::CreateTableObject(Table* table, OBJECTATTRIBUTES *attributeMap)
		{
			// Set New UID
			OBJECTUID newUID = CreateNewObjectUID();
			DatabaseObject *dbo = table->CreateObject();
			dbo->SetValue(L"uid", real_uid(newUID));

			// temp store in insert table until complete then move to owning table
			GetTableHandle("dboInsertTable")->InsertObject(dbo);
			uid_to_table[newUID] = GetTableHandle("dboInsertTable");

			// iterate through attributeMap setting values on dbo
			if (!ParseObjectAttributeMap(table, dbo, attributeMap))
			{
				// TODO LOG
				// will be left as orphan in deleted table - deleted table will delete all objects on app close or clear
				DeleteObjectFromMaps(newUID);
				return INVALID_OBJECT_UID;
			}

			// move object from insert table into the operational table
			if (!GetTableHandle("dboInsertTable")->MoveObjectToTable(table, newUID))
			{				
				// TODO LOG
				// will be left as orphan in deleted table - deleted table will delete all objects on app close or clear
				//delete dbo;
				//dbo = nullptr;
				DeleteObjectFromMaps(newUID);
				return INVALID_OBJECT_UID;
			}

			// update UID->Table mapping
			uid_to_table[newUID] = table;
			CreateObjectHouseKeeping(newUID);
			return newUID;
		}
		/**
		*
		*
		*
		*/
		void CObjectDatabase::EnumerateTableNames(::std::vector<::std::wstring> &tableList)
		{
			tableList.clear();
			::std::wstring ws;
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.begin();
			while (it != name_to_table.end())
			{
#ifndef DEBUG
				if (it->second->IsSystemTable())
				{
					++it;
					continue;
				}
#endif
				MBtWC(ws, it->first);
				tableList.push_back(ws);
				++it;
			}
			return;
		}
		/**
		* GetTableInterfaceHandle
		* Public
		*
		*/
		ITable*	CObjectDatabase::GetTableInterfaceHandle(::std::wstring tableName)
		{
			Table* table = GetTableHandle(tableName);
			return table ? new ITable(table) : nullptr;
		}
		/**
		* CreateObjectHouseKeeping
		*
		* Post Creation / Insertion Housekeeping
		*  send DBUpdate if reqd.
		* Specific stuff would be nice if from table registration or other dynamic way rather than static objecttypes could call soemthing..
		*/
		void CObjectDatabase::CreateObjectHouseKeeping(OBJECTUID uid)
		{		
			// if we are not loading from a file, i.e an individual creation task
			if( (!_databaseLoadInProgress) && _updateCache)
			{
				// create update record for the object creation
				DatabaseUpdateRecord update_record(
					DatabaseUpdateType::Create,
					uid,
					GetObjectType(uid),
					::std::wstring(L"create"),							// createString - attribute name.
					Value(real_bool(false)),							// dummy values
					Value(real_bool(true))								// dummy values
					);

				// insert update record
				_updateCache->InsertUpdate(update_record);
			}

			return;
		}
		/**
		*
		* DeleteObject
		* 
		* Deletes an object from the database, cleans up other infomation stores within the db too.
		* 
		*/
		bool CObjectDatabase::DeleteTableObject(OBJECTUID objectUID)
		{
			Table* dboTable = GetTableHandle(objectUID);
			if (!dboTable)
			{
				// log
				return false;
			}

			// lock update cache from sending out updates until we are done
			if(_updateCache)
				LockHandle hnd = _updateCache->GetLock();

			// if trigger exists, run it
			CObjectDatabase* me = this;
			bool bTrigger = _triggerManager->ExecuteTrigger(dboTable->GetTableName(), objectUID, ::std::wstring(L""), TriggerOperation::Delete, real_bool(true), real_bool(true), IObjectDatabase(&me));
			if (!bTrigger)
			{
				/* object delete not allowed due to... need log*/
				OutputDebugString(L"\n/* object delete not allowed due to... need log*/");
				return false;
			}	
						
			// delete from local lookup maps
			DeleteObjectFromMaps(objectUID);

			// request the table moves this object to the deleted table from current table
			if (!dboTable->MoveObjectToTable(GetTableHandle("dboDeleteTable"), objectUID))
			{
				// log
				// restore UID->Table mapping for valid table
				uid_to_table[objectUID] = dboTable;
				return false;
			}

			// update UID->Table mapping for object to deleted table
			uid_to_table[objectUID] = GetTableHandle("dboDeleteTable");
			
			if (_updateCache)
			{				
				// create delete record
				DatabaseUpdateRecord update_record(
					DatabaseUpdateType::Delete,
					objectUID,
					dboTable->GetObjectType(),
					::std::wstring(L"delete"),							// dummy attribute string, but registered by observers
					Value(real_bool(false)),							// dummy values
					Value(real_bool(true))								// dummy values
					);

				// send update record
				_updateCache->InsertUpdate(update_record);
			}
			return true;
		}
		/****************************************************************************************************************************************
		* 
		*												Get/Set Values Functions
		*
		****************************************************************************************************************************************/
		/**
		*
		* SetValue
		*
		* Public
		*
		* Is passed the object UID, a lookup is done to get the UID object pointer and then the object db owner i.e. VertexTable for a Vertex
		* Sets the value via the object owning database
		* Sets the serialisation service dirty flag for the ObjectDatabase.
		*
		*/
		bool CObjectDatabase::SetValue(OBJECTUID objectUID, const ::std::wstring &attrname, Value const &v)
		{
			// get the object owning table ptr
			Table* objectTable = GetTableHandle( objectUID );
			if( !objectTable )
			{
				// todo log
				return false;
			}
												
			// set the value
			if(!SetTableValue(objectTable, objectUID, attrname, v))
			{
				return false;
			}
					
			return true;
		}
		/**
		* SetTableValue
		*
		* Private
		*
		* Final ODB SetValue method, runs triggers and sends DB Update message
		*/
		bool CObjectDatabase::SetTableValue(Table* objectTable, OBJECTUID objectuid, const ::std::wstring &attributeName, const Value &newValue)
		{
			/* special case */
			if (attributeName == L"uid")
			{
				return objectTable->SetValue(objectuid, attributeName, newValue);
			}
			
			UPVALUE previousValue = ( GetValue(objectuid, attributeName) ) ;
			if (!previousValue)
			{
				// log or assert ?
				return false;
			}

			if (!objectTable->SetValue(objectuid, attributeName, newValue))
			{
				// log ?
				return false;
			}
						
			/* 
				this case is when we are loading, cant call a trigger on a dbo without UID set yet.. 
				not great but until we can prioritise UID set over all other objects this will have to do
				and XMLLoad methods cant add to Insert tmp table due to lack of ID when object is created
				And need to order table load based on dependencies
			*/
			if (!_databaseLoadInProgress)
			{
				// if trigger exists, run it
				CObjectDatabase* me = this;
				bool bTrigger = _triggerManager->ExecuteTrigger(objectTable->GetTableName(), objectuid, attributeName, TriggerOperation::Update, *previousValue, newValue, IObjectDatabase(&me));
				if (!bTrigger)
				{
					/* rollback set value */
					objectTable->SetValue(objectuid, attributeName, *previousValue);
					return false;
				}
			}

			// if this is an update in an existing object and we are not loading DB or creating a new object
			// create and send an update record
			if ((!_databaseLoadInProgress) && _updateCache)
			{
				// create the update record - use for Obs update and for Undo tracking.
				DatabaseUpdateRecord update_record(
					DatabaseUpdateType::Update,							// specify an update
					objectuid,												// this object uid that is being updated
					GetObjectType(objectuid),									// object type
					::std::wstring(attributeName),						// state the attribute textual name,
					*previousValue,										// previous value
					newValue);											// new value

				_updateCache->InsertUpdate(update_record);
			}

			// mark data changed - save reqd.
			if(_serial)
				_serial->SetClientDataDirtyState(this);

			return true;
		}
		/**
		* GetValue
		*
		* Public
		* 
		* functions call the DatabaseObject GetValue
		* Basically map the object UID to a pointer then call the pointer->GetValue(..)
		* 
		* ObjectUID, Attribute Name
		*
		*/
		UPVALUE CObjectDatabase::GetValue(OBJECTUID objectUID, const ::std::wstring &attrname)
		{
			// get the object owning table ptr
			Table* objectTable = GetTableHandle(objectUID);
			if (!objectTable)
			{
				// todo log
				return UPVALUE(nullptr);
			}

			// nasty conversion from BaseValue to Value 
			// .. release a BaseValue* from unique_ptr<BaseValue> to create a Value(BaseValue*) {copy ctor) into a unique_ptr<Value> ????
			return UPVALUE(
				new Value(*(objectTable->GetValue(objectUID, attrname)))
				);
		}
		/**
		* GetTableObjectCount
		*
		* Public
		*/
		size_t CObjectDatabase::GetTableObjectCount(::std::wstring &TableName)
		{
			::std::string wstr;
			WCtMB(TableName, wstr);
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.find(wstr);
			if (it == name_to_table.end())
			{
				return 0;
			}
			return it->second->GetTableObjectCount();
		}
		/**
		*
		* GetFirstObject
		*
		* Public
		*
		*/
		OBJECTUID CObjectDatabase::GetFirstObject(::std::wstring &TableName)
		{
			::std::string wstr;
			WCtMB(TableName, wstr);
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.find(wstr);
			if (it == name_to_table.end())
			{
				return INVALID_OBJECT_UID;
			}
			return it->second->GetFirstObject();
		}
		/**
		* GetNextObject
		*
		* Public
		*
		*/
		OBJECTUID CObjectDatabase::GetNextObject(OBJECTUID objectUID)
		{
			Table* pTable = GetTableHandle(objectUID);
			if (!pTable) return INVALID_OBJECT_UID;
			return GetValue(objectUID, L"nextsibling")->Get<ODBUID>();
		}
		/**
		*
		* GetLastObject
		*
		* Public
		*
		*/
		OBJECTUID CObjectDatabase::GetLastObject(::std::wstring &TableName)
		{
			::std::string wstr;
			WCtMB(TableName, wstr);
			TABLE_NAME_TO_PTABLE::iterator it = name_to_table.find(wstr);
			if (it == name_to_table.end())
			{
				return INVALID_OBJECT_UID;
			}
			return it->second->GetLastObject();
		}
		/**
		* GetAttributeList
		*
		* Public
		*/
		bool CObjectDatabase::GetAttributeList(OBJECTUID objectUID, int filterFlags, ::std::list<::std::wstring>& attrlist)
		{
			// get the object owning table ptr
			Table* objectTable = GetTableHandle(objectUID);
			if (!objectTable)
			{
				// todo log
				return false;
			}

			objectTable->GetAttributeList(objectUID, filterFlags, attrlist);
			return true;
		}
		/**
		* GetAttribute
		*
		* Public
		*/
		UPIATTRIBUTE CObjectDatabase::GetAttribute(OBJECTUID objectUID, ::std::wstring &attributeName)
		{
			// get the object owning table ptr
			Table* objectTable = GetTableHandle(objectUID);
			if (!objectTable)
			{
				// todo log
				return nullptr;
			}
			
			Attribute* attr = nullptr;
			if (objectTable->GetAttribute(objectUID, attributeName, &attr))
			{
				return UPIATTRIBUTE(new IAttribute(attr));
			}

			return nullptr;
		}
		


#pragma endregion CRUDOBJECTS

#pragma region FILELOADSAVE
		/****************************************************************************************************************************************
		* 
		*												Load/Save Data Functions
		*
		****************************************************************************************************************************************/
		/**
		*
		* SaveXMLData
		*
		* Called from SerialContext when saving the application state 
		*
		*/
		bool CObjectDatabase::SaveXMLData(tinyxml2::XMLDocument *destDoc, tinyxml2::XMLNode *currentNode)
		{
			// insert our ObjectDatabase root into the document as a child 
			tinyxml2::XMLNode *odbRootNode = currentNode;
			tinyxml2::XMLElement *odb = destDoc->NewElement(ROOT_NODE.c_str());
			odbRootNode->InsertEndChild(odb);

			// for each table
			TABLE_NAME_TO_PTABLE::iterator tableIt = name_to_table.begin();
			while( tableIt != name_to_table.end())
			{
				// check if we need to save this table data 
				if (!tableIt->second->SerialiseTable())
				{
					++tableIt;
					continue;
				}

				// get table name and insert XMLNode for this table
				tinyxml2::XMLElement *tableRoot = destDoc->NewElement( tableIt->first.c_str());
				odb->InsertEndChild(tableRoot);

				// for each tableObject (DatabaseObject) in this table 
				UID_TO_TABLE_MAP::iterator tableObjectIt = uid_to_table.begin();
				while( tableObjectIt != uid_to_table.end() )
				{
					// if this UID is in table tableIt
					if( tableObjectIt->second == tableIt->second )
					{
						// get object pointer as its easy to work with
						DatabaseObject* dbo = tableIt->second->GetObjectPtr(tableObjectIt->first);
						if( dbo )
						{
							// insert this tableObject (OBJECT_ELEMENT)
							tinyxml2::XMLElement *tableObject = destDoc->NewElement( OBJECT_ELEMENT.c_str() );
							tableRoot->InsertEndChild(tableObject);

							// get object attributes list 
							::std::list<::std::wstring> attributeList;
							//dbo->GetAttributeList(AttributeFilter::Serialize, attributeList);
							dbo->GetAttributeList(ATTR_SERIALISE, attributeList);

							// get xml for each attribute
							::std::list<::std::wstring>::iterator attributeIt = attributeList.begin();
							while( attributeIt != attributeList.end() )
							{
								// get attribute name
								::std::wstring attributeName = *attributeIt;

								// insert attribute into document 
								tinyxml2::XMLElement *attribute = destDoc->NewElement( ATTRIBUTE_NODE.c_str() );
								::std::string strAttributeName;
								WCtMB(attributeName, strAttributeName);
								attribute->SetAttribute("name", strAttributeName.c_str());
								tableObject->InsertEndChild(attribute);
								
								// get attribute xml from datatype
								dbo->GetValue(*attributeIt)->GetXML(attribute)  ;
								
								++attributeIt;
							}							
						}
					}
					++tableObjectIt;
				}
				++tableIt;
			}

			return true;
		}
		/**
		*
		* LoadXMLData
		*
		* Called from SerialContext when a new document is loaded
		*
		* Iterate through the children _objectFactory our root ROOT_NODE and call relevent function to process that child i.e. VERTEX_NODE
		*
		* TODO - Sort out table loading order.. if reqd. does not matter at moment as triggers not called on db loading
		*
		*/
		bool CObjectDatabase::LoadXMLData(tinyxml2::XMLDocument *XMLdoc, tinyxml2::XMLNode *currentNode)
		{
			// if the current node (position) is not on our root _objectFactory 'odb' then quit.
			::std::string name = currentNode->Value();
			if(name != ROOT_NODE)
			{
				// TODO log 
				return false;
			}

			// set internal class flag we are loading data from file db
			_databaseLoadInProgress = true;
			// lock updates to clients while we do a bulk import
			if(_updateCache)
				LockHandle hnd = _updateCache->GetLock();

			Clear();
			OutputDebugString(L"TODO : CLEAR OUT EXISTING DATABASES and Flag to rest _objectFactory system ? - maybe ok via serialisation class !!!!!!!!!!");
			
			bool errorOccured = false;
			// loop through children of the root ROOT_NODE (odb), which are tables
			for( tinyxml2::XMLNode *xmlTableNode=currentNode->FirstChild(); xmlTableNode; xmlTableNode=xmlTableNode->NextSibling() )
			{
				if( xmlTableNode->NoChildren())
				{
					continue;
				}

				// for this table 
				::std::string tableName = xmlTableNode->Value();
				TABLE_NAME_TO_PTABLE::iterator tableIT = name_to_table.find(tableName);
				if( tableIT == name_to_table.end() )
				{
					// TODO Log invalid table name
					continue;
				}

				// check if we need to load table data, if we dont serialise then we dont load either
				if (!tableIT->second->SerialiseTable())	continue;

				// Parse the XML objects for this table
				errorOccured &= ParseObjectXML(xmlTableNode, tableIT->second) ;
			}	

			// set internal class flag we are no longer loading data from file db
			_databaseLoadInProgress = false;

			// send updates
			if (_updateCache)
			{
				// dbupdate global load ?????????????????????????????
				DatabaseUpdateRecord update_record(
					DatabaseUpdateType::Refresh,						// specify an update
					::NM::ODB::INVALID_OBJECT_UID,						// this object uid that is being updated
					::NM::ODB::ObjectType::Database,					// object type
					::std::wstring(L""),								// state the attribute textual name,
					Value(real_int(0)),										// previous value
					Value(real_int(0))											// new value
					);

				_updateCache->InsertUpdate(update_record);
			}

			return !errorOccured;
		}
		/**
		* ParseObjectXML
		*
		* Part of LoadXML tasks for Serial. Called from LoadXMLData
		*/
		bool CObjectDatabase::ParseObjectXML(tinyxml2::XMLNode *xmlTableNode, Table *dbTable)
		{
			bool loadError = false;
			// for this table, load each DatabaseObject
			for( tinyxml2::XMLNode *tableObject=xmlTableNode->FirstChild(); tableObject; tableObject=tableObject->NextSibling() )
			{
				// check we have an xml node named 'tableobject' i.e. DatabaseObject
				::std::string tableObjectname = tableObject->Value();
				if(tableObjectname != OBJECT_ELEMENT) 
				{
					// is not a valid DatabaseObject 
					// *** ERR LOG ***
					continue;
				}

				// create new object from relevent table
				DatabaseObject* dbo = dbTable->CreateObject();
				if (!dbo)
				{
					// log
					assert(false);
					return false;
				}

				// we have a valid table Object, i.e. DatabaseObject
				// for each DatabaseObject, process each attribute
				parseAttributes(tableObject, dbTable, dbo);

				// add object to owning table 
				if( dbTable->InsertObject( dbo ) )
				{
					// update uid/table maps
					Value v( *dbo->GetValue(L"uid") );
					OBJECTUID uid = v.Get<ODBUID>();
					assert(uid != INVALID_OBJECT_UID);
					uid_to_table[uid] = dbTable;	
					CreateObjectHouseKeeping(uid);
					// no db update
				}
				else
				{
					delete dbo;
					dbo = nullptr;
					loadError = true;
					OutputDebugString(L"\nLoad Error");
				}

			}

			return !loadError;
		}
		/**
		* parseAttributes
		*
		* Part of LoadXML tasks for Serial. Called from ParseObjectXML
		*/
		tinyxml2::XMLNode* CObjectDatabase::parseAttributes(tinyxml2::XMLNode *xmlTableNode, Table* dbTable, DatabaseObject* dbo)
		{
			// for each attribute of this tableObject
			for( tinyxml2::XMLNode *attribute=xmlTableNode->FirstChild(); attribute; attribute=attribute->NextSibling() )
			{
				::std::string nodename = attribute->Value();
				if((nodename != ATTRIBUTE_NODE) || (attribute->NoChildren()))
				{
					// *** ERR LOG *** node not a attribute node so log and ignore
					//OutputDebugString(L"LOG: TODO Not an Attribute node or no value given\n");
					continue;
				}
			
				// get the dbo attribute name
				::std::string attrName = (static_cast<tinyxml2::XMLElement*>(attribute))->Attribute("name");
				if(attrName.empty())
				{
					assert(false); // *** ERR LOG *** // remove when we have a log facility	// node a attribute node so log and ignore									
					continue;
				}

				if(!attribute->NoChildren())
				{
					::std::wstring wattrName;
					MBtWC(wattrName, attrName);
					if (dbo->IsAttribute(wattrName))
					{
						::std::unique_ptr<BaseValue> v = dbo->GetValue(wattrName);
						if (v->SetXML(attribute))
						{
							if (!dbo->SetValue(wattrName, *v))
							{
								assert(false);
								// error LOG TODO
							}
						}
					}
				}
			}
			return nullptr;
		}
		/**
		* ParseObjectAttributeMap
		*
		* Table neutral - Parse attribute/value pairs in attributeMaps, matches with object attribute and tries to set the value on the attr in dbo
		* Called by CreateTableObject(...)
		*/
		bool CObjectDatabase::ParseObjectAttributeMap(Table* dbTable, DatabaseObject* dbo, OBJECTATTRIBUTES* attributeMap)
		{
			const size_t ATTRIB_NAME_ELEMENT = 0;
			const size_t ATTRIB_VALUE_ELEMENT = 1;

			assert(attributeMap);
			
			// Strip UID From attribute map if is exists
			OBJECTATTRIBUTES::iterator attributeIter = attributeMap->find(L"uid");
			if (attributeIter != attributeMap->end())
			{
				// delete the UID element from the map so we dont process it as user not allowed to set on new object
				attributeMap->erase(attributeIter);
			}

			// get the list of attributes bound to the object template from the object itself
			::std::list<::std::wstring> factory_attribute_list;;
			//dbo->GetAttributeList(AttributeFilter::All, factory_attribute_list);
			dbo->GetAttributeList(ATTR_ALL, factory_attribute_list);

			// iterate through the list of current attributes in the object and save the 
			// attribute names that are required. As we add the new attributes from vertex_attribute_map
			// if they are required and therefore in the list, we will delete them.
			// Once the complete import is done, the list (required_attributes) should be empty
			// if not, then a required attribute has not been supplied and we fail the creation of this object
			// vector of required attributes
			::std::set<::std::wstring> required_attributes;
			::std::list<::std::wstring>::iterator it = factory_attribute_list.begin();
			while (it != factory_attribute_list.end())
			{
				Attribute* pAttribute = dbo->GetAttribute(*it);
				// check each attribute required flag
				if ((pAttribute) && (pAttribute->GetFlags() & ATTR_VALUE_REQUIRED_ON_CREATE))
				{
					required_attributes.insert(*it);
				}

				++it;
			}

			// Iterate through the passed attributes (vertex_attribute_map) and update the attributes in the newly created object
			OBJECTATTRIBUTES::iterator it_attrib = attributeMap->begin();
			while (it_attrib != attributeMap->end())
			{
				::std::wstring attrib_name = ::std::get<ATTRIB_NAME_ELEMENT>(*it_attrib);
				if (dbo->IsAttribute(attrib_name))
				{
					bool valueSet = false;
					::std::unique_ptr<BaseValue> upVal = dbo->GetValue(attrib_name);
					if (upVal)
					{
						Value v(*upVal);
						if (v.Set(::std::get<ATTRIB_VALUE_ELEMENT>(*it_attrib)))
						{
							if (dbo->SetValue(attrib_name, v))
							{
								// check to see if this is a required attribute
								::std::set<::std::wstring>::iterator it = required_attributes.find(attrib_name);
								if (it != required_attributes.end())
								{
									// matched, so delete from required_attributes sets as we have set the value on this one
									required_attributes.erase(it);
								}
								valueSet = true;
							}
						}
					}

					if (!valueSet)
					{
						// TODO LOG : Log (Failed to set value [] on [])						
						::std::wstring error =
							L"Failed to set <TABLE: "
							+ dbTable->GetTableName()
							+ L"> value ["
							+ ::std::get<ATTRIB_VALUE_ELEMENT>(*it_attrib)
							+ L"] on attribute ["
							+ attrib_name + L"]\n";
						OutputDebugString(error.c_str());
					}
				}
				++it_attrib;
			}

			// by this point we have set all values on this new vertex given by the passed param vertex_attribute_map.
			// we now check required_attributes and check if its empty meaning we have set values on all required attributes
			return required_attributes.empty();
		}

#pragma endregion FILELOADSAVE
		
		// ns
		}
}

