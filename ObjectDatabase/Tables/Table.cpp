#include "stdafx.h"
#include "Table.h"
#include "DataObjects\DatabaseObject.h"
#include "DatabaseCoreElements\attribute.h"
#include "Sibling.h"
#include <set>

namespace NM 
{
	namespace ODB 
	{

		Table::Table(::std::wstring& tableName, ::std::wstring& objectName, ObjectType objectType, bool systemTable) :
			_tableName(tableName),
			_objectType(objectType),
			_objectName(objectName),
			_serialise(true),
			_siblings(new Sibling(this)),
			_systemTable(systemTable)
		{
			CreateDefaultAttributes();
		}
		/**
		*
		*
		*
		*/
		Table::~Table()
		{
			delete _siblings;
			_siblings = nullptr;
			delete _defaultObject;
			_defaultObject = nullptr;
			Clear();

			// delete all attributes 
			ATTR_MAP::iterator it = _attributes.begin();
			while (it != _attributes.end())
			{
				delete it->second;
				it->second = nullptr;
				++it;
			}
			_attributes.clear();
		}
		/**
		* CreateDefaultAttributes
		*
		* Creates the mandatory default attributes required in every table. Can be overridden but attribute name and types must be the required ones
		* Creates the default object and adds default attributes
		*/
		void Table::CreateDefaultAttributes()
		{
			// create default attributes
			_attributes[L"uid"] = new Attribute(L"uid", TypeT::TypeUID, ATTR_SYSTEM | ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_LOCKED_ON_SET, L"Unique Object ID", &real_uid(INVALID_OBJECT_UID));
			_attributes[L"parent"] = new Attribute(L"parent", TypeT::TypeUID, ATTR_SYSTEM | ATTR_SERIALISE, L"Parent graph object (not used)", &real_uid(INVALID_OBJECT_UID));
			_attributes[L"previoussibling"] = new Attribute(L"previoussibling", TypeT::TypeUID, ATTR_SYSTEM | ATTR_SERIALISE, L"Previous table object", &real_uid(INVALID_OBJECT_UID));
			_attributes[L"nextsibling"] = new Attribute(L"nextsibling", TypeT::TypeUID, ATTR_SYSTEM | ATTR_SERIALISE, L"Next table object", &real_uid(INVALID_OBJECT_UID));
			_attributes[L"firstchild"] = new Attribute(L"firstchild", TypeT::TypeUID, ATTR_SYSTEM | ATTR_SERIALISE, L"First child graph object (not used)", &real_uid(INVALID_OBJECT_UID));
			_attributes[L"shortname"] = new Attribute(L"shortname", TypeT::TypeString, ATTR_USER_MODIFY | ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_SERIALISE, L"Shortname");
			_attributes[L"description"] = new Attribute(L"description", TypeT::TypeString, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Description");
			
			// create default object
			_defaultObject = new DatabaseObject(_objectName, _objectType);
			
			// add each attribute to the defaultobject.
			_defaultObject->InsertAttribute(_attributes[L"uid"]);
			_defaultObject->InsertAttribute(_attributes[L"parent"]);
			_defaultObject->InsertAttribute(_attributes[L"previoussibling"]);
			_defaultObject->InsertAttribute(_attributes[L"nextsibling"]);
			_defaultObject->InsertAttribute(_attributes[L"firstchild"]);
			_defaultObject->InsertAttribute(_attributes[L"shortname"]);
			_defaultObject->InsertAttribute(_attributes[L"description"]);

			return;
		}
		/**
		* InsertAttribute
		* Public
		* Inserts a new copy of the passed attribute into the table, and existing objects and default object.
		*/
		bool Table::InsertAttribute(const Attribute& attribute)
		{
			// get new attribute name, check not empty 
			::std::wstring attrName = attribute.GetName();
			if (attrName.empty()) return false;

			// check if the attribute (name) already exists
			ATTR_MAP::iterator it = _attributes.find(attrName);
			if (it != _attributes.end()) return false; // name already exists. cant have two attributes (e.g. columns) with same name

			// add to table store & lookup
			_attributes[attrName] = new Attribute(attribute);

			// add to default object
			_defaultObject->InsertAttribute(_attributes[attrName]);

			// add to all existing objects
			OBJECT_VECTOR::iterator existingObjectsIt = _objects.begin();
			while (existingObjectsIt != _objects.end())
			{
				(*existingObjectsIt)->InsertAttribute(_attributes[attrName]);
				++existingObjectsIt;
			}
			
			return true;
		}
		/**
		* CreateObject
		*
		* Returns a copy of the default object - does not store in local table, use InsertObject to add this object in.
		*/
		DatabaseObject* Table::CreateObject()
		{
			return new DatabaseObject(*_defaultObject);
		}
		/**
		* CreateObject
		*
		* Takes a list of attributes and creates a new object and adds to the database table
		*/
		OBJECTUID Table::CreateObject(OBJECTATTRIBUTES *attributeMap IN, DatabaseObject** pObject OUT)
		{
			const size_t ATTRIB_NAME_ELEMENT = 0;
			const size_t ATTRIB_VALUE_ELEMENT = 1;

			if (!attributeMap)
			{
				return INVALID_OBJECT_UID;
			}
			
			// GET UID From attribute map 
			OBJECTUID objectUID = INVALID_OBJECT_UID;
			// GET UID From attribute map or if not exist then create a new one for this flow
			OBJECTATTRIBUTES::iterator attributeIter = attributeMap->find(L"uid");
			if (attributeIter == attributeMap->end())
			{
				UuidCreate(&objectUID);
			}
			else
			{
				// convert UID string to OBJECTUID
				/*********************************************************************/
				real_uid uid(INVALID_OBJECT_UID);
				if (!uid.Set(attributeIter->second))
				{
					// todo log
					return INVALID_OBJECT_UID;
				}
				OBJECTUID objectUID = Value(uid).Get<ODBUID>();
				/*********************************************************************/
				// delete the UID element from the map so we dont process it.
				attributeMap->erase(attributeIter);
			}

			// Create New Object 
			DatabaseObject* dbo = CreateObject();
			if (!dbo)
			{
				// failed to create new object - error log reqd
				return INVALID_OBJECT_UID;
			}
			// set the UID 
			dbo->SetValue(L"uid", real_uid(objectUID));
			// copy pointer to return to ObjectDatabase
			*pObject = dbo;

			// get the list of attributes bound to the vertex template from the object itself
			::std::list<::std::wstring> factory_attribute_list;;
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
				if( (pAttribute) && (pAttribute->GetFlags() & ATTR_VALUE_REQUIRED_ON_CREATE))
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
					if (dbo->SetValue(attrib_name, ::std::get<ATTRIB_VALUE_ELEMENT>(*it_attrib)))
					{
						// check to see if this is a required attribute
						::std::set<::std::wstring>::iterator it = required_attributes.find(attrib_name);
						if (it != required_attributes.end())
						{
							// matched, so delete from required_attributes sets as we have set the value on this one
							required_attributes.erase(it);
						}
					}
					else
					{
						// failed to set value - TODO do something ?!?!
						// TODO LOG : Log (Failed to set value [] on [])
						//assert(false);
						::std::wstring error =
							L"Failed to set VERTEX value ["
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
			if (!required_attributes.empty())
			{
				// TODO - LOG this
				assert(false);
				*pObject = nullptr;
				delete dbo;
				dbo = nullptr;
				return INVALID_OBJECT_UID;
			}

			// all required attribs have been set and other optional attribs passed. Add the object to the table (vector)
			InsertObject(dbo);

			// return the uid of the newly created object
			Value v(*(dbo->GetValue(L"uid")));
			return v.Get<OBJECTUID>();
		}
		/**
		*
		*
		*
		*/
		bool Table::SetValue(OBJECTUID objectUID, const ::std::wstring &attrname, Value const &v)
		{
			DatabaseObject* dbo = GetObjectPtr(objectUID);
			if (!dbo) return false;
			if (!dbo->IsAttribute(attrname)) return false;
			return dbo->SetValue(attrname, v);
		}
		/**
		*
		*
		*
		*/
		UPBASEVALUE	 Table::GetValue(OBJECTUID objectUID, const ::std::wstring &attrname)
		{
			DatabaseObject* dbo = GetObjectPtr(objectUID);
			if (!dbo) return nullptr;
			if (!dbo->IsAttribute(attrname)) return nullptr;
			return dbo->GetValue(attrname);
		}
		/**
		*
		*
		*
		*/
		bool Table::SerialiseTable()
		{
			return _serialise;
		}
		/**
		* GetObjectPtr
		*
		* Returns the Object pointer from the Object UID
		*/
		DatabaseObject* Table::GetObjectPtr(OBJECTUID objectUID)
		{
			UID_TO_POINTER_MAP::iterator it = uid_to_objectpointer.find(objectUID);
			if( it != uid_to_objectpointer.end() )
			{
				return it->second;
			}
			return nullptr;
		}
		/**
		*
		*
		*
		*/
		void Table::AddSibling(DatabaseObject *graphObject)
		{
			_siblings->AddSibling(graphObject);
		}
		/**
		*
		*
		*
		*/
		void Table::DeleteSibling(DatabaseObject *graphObject)
		{
			_siblings->DeleteSibling(graphObject);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID Table::GetFirstObject()
		{
			DatabaseObject* dbo = _siblings->GetSibling(SiblingPosition::First);
			if (dbo)
			{
				return Value(*dbo->GetValue(L"uid")).Get<ODBUID>();
			}
			return INVALID_OBJECT_UID;
		}
		/**
		*
		*
		*
		*/
		OBJECTUID Table::GetLastObject()
		{
			DatabaseObject* dbo = _siblings->GetSibling(SiblingPosition::Last);
			if (dbo)
			{
				return Value(*dbo->GetValue(L"uid")).Get<ODBUID>();
			}
			return INVALID_OBJECT_UID;

		}
		/**
		*
		*
		*
		*/
		bool Table::InsertObject(DatabaseObject* dbo)
		{
			// get the UID value - bit messy until I sort this out.
			Value v = *dbo->GetValue(L"uid");
			OBJECTUID objectUID = v.Get<ODBUID>();
			assert(objectUID != INVALID_OBJECT_UID);			

			UID_TO_POINTER_MAP::iterator it = uid_to_objectpointer.find(objectUID);
			assert(it == uid_to_objectpointer.end());	// no duplicate OBJECTUID should ever be found in the map

			// all required attribs have been set and other optional attribs passed. Add the vertex to the database (vector)
			_objects.push_back(dbo);

			// maps the UID (Unique ID of an object to the object pointer)
			uid_to_objectpointer[objectUID] = dbo;

			// update sibling references
			AddSibling(dbo);

			return true;
		}
		/**
		*
		*
		*
		*/
		bool Table::DestroyObject(OBJECTUID objectUID, bool deleteObject)
		{
			DatabaseObject *dbo = GetObjectPtr(objectUID);
			if (!dbo) return false; // log

			DeleteSibling(dbo);

			// delete uid to instance ptr lookup
			UID_TO_POINTER_MAP::iterator pit = uid_to_objectpointer.find(objectUID);
			if (pit != uid_to_objectpointer.end())
			{
				pit->second = nullptr;
				uid_to_objectpointer.erase(pit);
			}

			// delete the object from the db
			OBJECT_VECTOR::iterator it = ::std::find(_objects.begin(), _objects.end(), dbo);
			if (it != _objects.end())
			{
				if (deleteObject)
					delete *it;
				*it = nullptr;
				_objects.erase(it);
				return true;
			}

			return false; // not found
		}
		/**
		*
		*
		*
		*/
		void Table::Clear()
		{
			// delete all Vertex objects 
			::std::vector<DatabaseObject*>::iterator it = _objects.begin();
			while (it != _objects.end())
			{
				delete *it;
				++it;
			}
			_objects.clear();
			return;
		}
		/**
		*
		*
		*
		*/
		bool Table::MoveObjectToTable(Table* moveToTable, OBJECTUID objectUID)
		{
			DatabaseObject *dbo = GetObjectPtr(objectUID);
			if (!dbo) return false; // log

			DestroyObject(objectUID, false);

			if (!moveToTable->RecieveMovedObject(dbo))
			{				
				InsertObject(dbo);
				return false;
			}			
			return true;			
		}
		/**
		*
		*
		*
		*/
		bool Table::RecieveMovedObject(DatabaseObject* dbo)
		{
			return InsertObject(dbo);
		}
		/**
		*
		*
		*
		*/
		void Table::GetAttributeList(OBJECTUID objectUID, int filterFlags, ::std::list<::std::wstring> &attrlist)
		{
			DatabaseObject *dbo = GetObjectPtr(objectUID);
			if (!dbo) return; // log

			dbo->GetAttributeList(filterFlags, attrlist);
		}
		/**
		* GetAttribute
		*
		* Needs adding for INVALID_OBJECT_UID will check the default object / local attrib store - not supported yet
		*/
		bool Table::GetAttribute(OBJECTUID objectUID, ::std::wstring &attributeName, Attribute **attribute)
		{
			DatabaseObject *dbo = GetObjectPtr(objectUID);
			if (!dbo) return false; // log

			Attribute* pAttribute = dbo->GetAttribute(attributeName);
			// check each attribute required flag
			if(!pAttribute)
			{
				return false;
			}

			*attribute = dbo->GetAttribute(attributeName);

			return (*attribute ? true : false);
		}
		/**
		*
		*
		*
		*/
		::std::wstring Table::GetObjectTypeName(OBJECTUID objectUID)
		{
			DatabaseObject *dbo = GetObjectPtr(objectUID);
			if (!dbo) return L""; // log

			return dbo->GetObjectTypeName();
		}
		/**
		*
		*
		*
		*/
		bool Table::IsAttribute(OBJECTUID objectUID, const ::std::wstring &attrname)
		{
			DatabaseObject *dbo = GetObjectPtr(objectUID);
			if (!dbo) return false; // log

			return dbo->IsAttribute(attrname);
		}

// ns
	}

}
