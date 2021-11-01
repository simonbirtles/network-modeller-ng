#include "StdAfx.h"
#include "DatabaseObject.h"
#include "..\DatabaseCoreElements\attribute.h"
#include <functional>

namespace NM 
{
	namespace ODB 
	{
		/**
		*	DatabaseObject
		*
		*
		*/
		DatabaseObject::DatabaseObject(::std::wstring objectTypeName, ObjectType objectType):
			_objectTypeName(objectTypeName),
			_bmodified(false),
			_objectType(objectType)
		{
		}
		/**
		*	~DatabaseObject
		*
		*
		*/
		DatabaseObject::~DatabaseObject()
		{
			// delete all values
			::std::vector<BaseValue*>::iterator it = _values.begin();
			while (it != _values.end())
			{
				if (*it != nullptr)
				{
					delete (*it);
					*it = nullptr;
				}
				++it;
			}

			// null all the attribute pointers - we dont own them, the table does.
			::std::vector<Attribute*>::iterator attribIT = _attr.begin();
			while (attribIT != _attr.end())
			{
				(*attribIT) = nullptr;
				++attribIT;
			}

		}
		/**
		*	DatabaseObject Copy Constructor
		*
		*
		*/
		DatabaseObject::DatabaseObject(const DatabaseObject &object_to_copy) //, OBJECTUID newUID)
		{ 
			_attr			= object_to_copy._attr;
			_map			= object_to_copy._map;
			_objectType		= object_to_copy._objectType;
			_objectTypeName = object_to_copy._objectTypeName;

			// dont copy _values (as all object will use same addresses for values) but create new value objects, ok to copy attribute names
			_values.clear();
			for(size_t t=0; t<object_to_copy._values.size(); ++t)
			{
				_values.push_back( object_to_copy._values[t]->Clone() );						// copy actual value
			}
		}								
		/**
		*
		*
		*
		*/
		ObjectType DatabaseObject::GetObjectType()
		{
			return _objectType;
		}
		/**
		*
		*
		*
		*/
		void DatabaseObject::SetModifiedFlag(bool bmodified)
		{
			_bmodified = bmodified;
		}
		/**
		*
		*
		*
		*/
		bool DatabaseObject::GetModifiedFlag()
		{
			return _bmodified;
		}
		/**
		*
		*
		*
		*/
		size_t DatabaseObject::GetAttributeCount()
		{
			return _attr.size();
		}
		/**
		* GetAttributeList
		*
		* Returns a List<wstring> of Attribute names for this tableObject with matching flags set
		*/
		void DatabaseObject::GetAttributeList(int filterFlags, ::std::list<::std::wstring> &attributeList)
		{
			attributeList.clear();
			for (size_t t = 0; t<_attr.size(); ++t)
			{
				if (_attr[t]->GetFlags() & filterFlags)
				{
					attributeList.push_back(_attr[t]->GetName());
				}
			}
			return;
		}
		/**
		*
		*
		*
		*/
		bool DatabaseObject::InsertAttribute(Attribute *attribute)
		{ 
			::std::wstring attrname = attribute->GetName();
			// if attribute already exists in this object, dont add a duplicate
			if( IsAttribute(attrname) )
			{
				return false;
			}
			// new one, so add and create attribute and add to _values
			_attr.push_back(attribute);										// inserts a copy of the passed attribute 
			_values.push_back( attribute->GetDefaultValue().release() );				// gets a new copy of a BaseValue from attribute and inserts BaseValue* into vector	
			_map[attrname] = (_attr.size() - 1);							// creates an index for attrname -> position in _attr/_values
			return true;
		}
		/**
		* DeleteAttribute
		*
		* find the attribute then delete if it exists
		*
		*/
		bool DatabaseObject::RemoveAttribute(const ::std::wstring &attributeName)		// 
		{
			// find attribute
			::std::map<const ::std::wstring, int>::iterator it = _map.find(attributeName);
			if( it == _map.end() )
			{
				return false;
			}
	
			// delete from table DB, lookups
			_map.erase(it);
			_attr.erase( _attr.begin() + it->second );
			_values.erase( _values.begin() + it->second );	
			return true;
		}
		/**
		* FindAttribute
		*
		* find the attribute string name then return index number of the attribute
		*
		*/
		int DatabaseObject::FindAttribute(const ::std::wstring &attributeName)		
		{
			::std::map<const ::std::wstring, int>::iterator it = _map.find(attributeName);

			if( it != _map.end() )
			{
				return it->second;		// attribute index
			}

			return INVALID_ATTRIBUTE;
		}
		/** 
		* IsAttribute
		*
		* Given an attribute string name, if is an attribute in this object then return true, else false.
		*
		*/
		bool DatabaseObject::IsAttribute(const ::std::wstring &attributeName)
		{
			::std::map<const ::std::wstring, int>::iterator it;
			it = _map.find(attributeName);
			if( it != _map.end() )
			{
				return true;		// attribute index
			}
			return false;
		}
		/** 
		* IsAttribute
		*
		* Given an attribute index name, if is an attribute in this object then return true, else false.
		*
		*/
		bool DatabaseObject::IsAttribute(const int attributeIndex)
		{
			if( (attributeIndex > static_cast<int>((_attr.size()-1))) ||
				attributeIndex < 0)
			{
				return false;
			}
			return true;
		}
		/**
		* GetAttribute
		*
		* Given an valid attribute index number, return a const pointer to the attribute object.
		*
		*/
		Attribute* DatabaseObject::GetAttribute(::std::wstring &attributeName)
		{
			int index = FindAttribute(attributeName);
			if (index == INVALID_ATTRIBUTE)
			{
				return nullptr;
			}

			return _attr[index];
		}
		/**
		* SetValue
		*
		* Sets the attribute value, given an valid attribute name (string) and Value
		*
		*/
		bool DatabaseObject::SetValue(const ::std::wstring &attributeName, Value const &v)
		{
			size_t idx = FindAttribute(attributeName);
			if(idx == INVALID_ATTRIBUTE)
			{
				return false;
			}

			return _Set_Value(idx, v);
		}
		/**
		* SetValue (wstr, wstr): 
		*
		* Given a valid Attribute name (string) and the value as a string,
		* will perform the value string to valid type conversion if possible
		* and sets the attribute value.
		*
		*/
		bool DatabaseObject::SetValue(const ::std::wstring &attributeName, const ::std::wstring &strValue)
		{
			size_t idx = FindAttribute(attributeName);
			if(idx == INVALID_ATTRIBUTE){ return false; }

			// get a COPY of the existing Value which will be of the correct type and current value
			Value v( *( GetValue(attributeName)) );
			// set the new value in the COPY value
			if( v.Set(strValue) )
			{
				return _Set_Value(idx, v);
			}

			return false;
		}
		/**
		* _Set_Value
		*
		* Private function - called from overloaded public functions SetValue(...)
		* Sets the value and creates an update record
		*
		*/
		bool DatabaseObject::_Set_Value(size_t attributeIndex, Value const &v)
		{
			if(_values[attributeIndex]->IsLocked())
			{
				return false;
			}

			// check prev and new values are not the same
			Value previous_value( *(_Get_Value(attributeIndex)) );
			if( previous_value == v)
			{
				return true;
			}

			// set the new value 
			if (!_values[attributeIndex]->Set(v))
			{
				assert(false);
				return false;	// due to being locked, although we should have caught that already
			}

			// if the attribute requires set and lock, then lock the value from change	
			if( _attr[attributeIndex]->AttributeLockedOnSet())
			{
				_values[attributeIndex]->Lock();
			}
		
			// set local DBO modified flag
			SetModifiedFlag(true);

			return true;
		}
		/**
		* _Get_Value
		*
		* (Private)
		* Given an attribute index, return a COPY of the Value
		*/
		::std::unique_ptr<BaseValue> DatabaseObject::_Get_Value(size_t attributeIndex) const
		{
			return ::std::unique_ptr<BaseValue>( _values[attributeIndex]->Clone() );
		}
		/**
		* GetValue
		*
		* Given an attribute name as a string, return a COPY of the Value
		*
		*/
		::std::unique_ptr<BaseValue> DatabaseObject::GetValue(const ::std::wstring &attributeName)
		{
			int idx = FindAttribute(attributeName);
			if( idx == -1)
			{
				return nullptr;
			}
			return _Get_Value(idx);
		}
		/**
		* GetObjectTypeName
		*
		* Returns this objects type as a string, i.e. Vertex, Edge etc
		*
		*/
		::std::wstring DatabaseObject::GetObjectTypeName()
		{
			return _objectTypeName;
		}
// ns
	}
}