#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"
#include <map>
#include <vector>
#include <string>

namespace NM 
{
	namespace ODB 
	{
		class Attribute;
		/**
		*
		* Class DatabaseObject
		*
		*/
		class DatabaseObject
		{
		public:	

			DatabaseObject(::std::wstring objectTypeName, ObjectType objectType);
			virtual ~DatabaseObject();	
			DatabaseObject(const DatabaseObject &object_to_copy); 				
			
			// Attributes
			bool			InsertAttribute(Attribute *attribute);
			bool			RemoveAttribute(const ::std::wstring &attributeName);
			Attribute*		GetAttribute(::std::wstring &attributeName);			
			size_t			GetAttributeCount();
			void			GetAttributeList(int filterFlags, ::std::list<::std::wstring> &attributeList);
			bool			IsAttribute(const ::std::wstring &attributeName);

			// misc
			bool			GetModifiedFlag();
			::std::wstring	GetObjectTypeName();
			ObjectType		GetObjectType();
			
			// Get/Set Values
			::std::unique_ptr<BaseValue>	GetValue(const ::std::wstring &attributeName);
			bool							SetValue(const ::std::wstring &attributeName, Value const &v);
			bool							SetValue(const ::std::wstring &attributeName, const ::std::wstring &strvalue);

		private:		

			ObjectType										_objectType;
			::std::wstring									_objectTypeName;				// textual name of the derived object, i.e. "Vertex" or "Edge"
			::std::map<const ::std::wstring, int>			_map;							// maps attr name to index
			::std::vector<Attribute*>						_attr;							// vector of attributes, index of _values
			::std::vector<BaseValue*>						_values;						// vector of the values of the attributes shoudl be in sync with _attr
			bool											_bmodified;						// has this been modified since loading, i.e. do we need to save

			bool									IsAttribute(const int attributeIndex);
			int										FindAttribute(const ::std::wstring &attributeName);
			::std::unique_ptr<BaseValue>			_Get_Value(size_t attributeIndex) const;			
			bool									_Set_Value(size_t attributeIndex, Value const &v);							// function that calls Set on the BaseValue::RealValue<T>::_val
			void									SetModifiedFlag(bool bmodified);

			DatabaseObject& operator= (const DatabaseObject &graphobject) = delete; // { return *this; }		// assignment constructor

		};

// ns
	}
}
