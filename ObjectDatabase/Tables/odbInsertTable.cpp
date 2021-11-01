#include "stdafx.h"
#include "odbInsertTable.h"
#include "DataObjects\DatabaseObject.h"
namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		*
		*/
		odbInsertTable::odbInsertTable() :
			Table(::std::wstring(L"dboInsertTable"), ::std::wstring(L"dboInsertTable"), ObjectType::ObjectInvalid, true)
		{

		}
		/**
		*
		*
		* on Dtor, we cleanup anything thats been left and actually delete
		*/
		odbInsertTable::~odbInsertTable()
		{
			OBJECT_VECTOR::iterator it = _objects.begin();
			while (it != _objects.end())
			{
				delete *it;
				*it = nullptr;
				++it;
			}
		}
		/**
		*
		*
		*
		*/
		bool odbInsertTable::SerialiseTable()
		{
			return false;
		}
		/**
		*
		*
		*
		*/
		OBJECTUID odbInsertTable::CreateObject(OBJECTATTRIBUTES *attributeMap IN, DatabaseObject** pObject OUT)
		{
			return INVALID_OBJECT_UID;
		}
		/**
		*
		*
		*
		*/
		bool odbInsertTable::InsertObject(DatabaseObject* dbo)
		{
			Value v = *dbo->GetValue(L"uid");
			OBJECTUID objectUID = v.Get<ODBUID>();

			// all required attribs have been set and other optional attribs passed. Add the vertex to the database (vector)
			_objects.push_back(dbo);

			// maps the UID (Unique ID of an object to the object pointer)
			uid_to_objectpointer[objectUID] = dbo;
			

			return true;
		}
		/**
		*
		*
		* We dont actually destroy the object in the Insert table, just remove it from our local table
		*/
		bool odbInsertTable::DestroyObject(DatabaseObject* pObject, bool deleteObject)
		{
			return RemoveObject(pObject);
		}
		/**
		*
		*
		* We dont actually destroy the object in the Insert table, just remove it from our local table
		*/
		bool odbInsertTable::RemoveObject(DatabaseObject* pObject)
		{
			OBJECT_VECTOR::iterator it = ::std::find(_objects.begin(), _objects.end(), pObject);
			if (it != _objects.end())
			{
				*it = nullptr;
				_objects.erase(it);
				return true;
			}
			return false; // not found
		}



// ns

	}
}
