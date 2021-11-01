#include "stdafx.h"
#include "odbDeleteTable.h"
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
		odbDeleteTable::odbDeleteTable() :
			Table(::std::wstring(L"dboDeleteTable"), ::std::wstring(L"dboDeleteTable"), ObjectType::ObjectInvalid, true)
		{

		}
		/**
		*
		*
		* on Dtor, we cleanup anything thats been left and actually delete
		*/
		odbDeleteTable::~odbDeleteTable()
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
		bool odbDeleteTable::SerialiseTable()
		{
			return false;
		}
		/**
		*
		*
		*
		*/
		OBJECTUID odbDeleteTable::CreateObject(OBJECTATTRIBUTES *attributeMap IN, DatabaseObject** pObject OUT)
		{
			return INVALID_OBJECT_UID;
		}
		/**
		*
		*
		*
		*/
		bool odbDeleteTable::InsertObject(DatabaseObject* dbo)
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
		* We dont actually destroy the object in the delete table, just remove it from our local table as it may be going back into the original table
		*/
		bool odbDeleteTable::DestroyObject(DatabaseObject* pObject, bool deleteObject)
		{
			return RemoveObject(pObject);
		}
		/**
		*
		*
		* We dont actually destroy the object in the delete table, just remove it from our local table as it may be going back into the original table
		*/
		bool odbDeleteTable::RemoveObject(DatabaseObject* pObject)
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
		/**
		*
		*
		*
		*/
		bool odbDeleteTable::SetValue(DatabaseObject* dbo, size_t idx, Value const &v)
		{
			return false;
		}
		/**
		*
		*
		*
		*/
		bool odbDeleteTable::SetValue(DatabaseObject* dbo, const ::std::wstring &attrname, Value const &v)
		{
			return false;
		}



// ns

	}
}
