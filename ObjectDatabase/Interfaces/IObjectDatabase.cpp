#include "stdafx.h"
#include "IObjectDatabase.h"
#include "..\Database\ObjectDatabase.h"
#include "ITrigger.h"
#include "Interfaces\IAttribute.h"
#include "Interfaces\ITable.h"

namespace NM {
	namespace ODB {
		/**
		*
		*
		*
		*/
		IObjectDatabase::IObjectDatabase(CObjectDatabase** db):
			_odb(db)
		{
		}
		/**
		*
		*
		*
		*/
		IObjectDatabase::~IObjectDatabase(void)
		{
			// we dont own the pointer to (*_odb) so we dont delete it and 
			// (**_odb) is just a ptr to a ptr so we dont delete this as nothing to delete.
		}
		/**
		*
		*
		*
		*/
		bool IObjectDatabase::DeleteObject(OBJECTUID objectUID)
		{
			if( (*_odb) == nullptr  ) return false;
			return (*_odb)->DeleteTableObject(objectUID);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CreateVertex(OBJECTATTRIBUTES *attribute_map)
		{
			if( (*_odb) == nullptr  ) return INVALID_OBJECT_UID;
			//return (*_odb)->CreateVertex(attribute_map);
			return (*_odb)->CreateTableObject(L"vertextable", attribute_map);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CreateInterface(OBJECTATTRIBUTES *attribute_map)
		{
			if ((*_odb) == nullptr) return INVALID_OBJECT_UID;
			//return (*_odb)->CreateInterface(attribute_map);
			return (*_odb)->CreateTableObject(L"interfacetable", attribute_map);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CreateEdge(OBJECTATTRIBUTES *attribute_map)
		{
			if( (*_odb) == nullptr  ) return INVALID_OBJECT_UID;
			//return (*_odb)->CreateEdge(attribute_map);
			return (*_odb)->CreateTableObject(L"edgetable", attribute_map);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CreateFlow(OBJECTATTRIBUTES *attribute_map)
		{
			if( (*_odb) == nullptr  ) return INVALID_OBJECT_UID;
			//return (*_odb)->CreateFlow(attribute_map);
			return (*_odb)->CreateTableObject(L"flowtable", attribute_map);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CreateDemand(OBJECTATTRIBUTES *attribute_map)
		{
			if( (*_odb) == nullptr  ) return INVALID_OBJECT_UID;
			//return (*_odb)->CreateDemand(attribute_map);
			return (*_odb)->CreateTableObject(L"demandtable", attribute_map);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CreatePath(OBJECTATTRIBUTES *attribute_map)
		{
			if( (*_odb) == nullptr  ) return INVALID_OBJECT_UID;
			//return (*_odb)->CreatePath(attribute_map);
			return (*_odb)->CreateTableObject(L"pathtable", attribute_map);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CreateLayer(OBJECTATTRIBUTES	*attribute_map)
		{
			if( (*_odb) == nullptr  ) return INVALID_OBJECT_UID;
			//return (*_odb)->CreateLayer(attribute_map);
			return (*_odb)->CreateTableObject(L"layertable", attribute_map);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::CopyVertex(OBJECTUID sourceObjectUID)
		{
			assert(false);
			return INVALID_OBJECT_UID;
			// should be copy object?
			//if( (*_odb) == nullptr  ) return INVALID_OBJECT_UID;
			//return (*_odb)->CopyVertex(sourceObjectUID);
		}
		/**
		*
		*
		*
		*/
		//bool IObjectDatabase::SetValue(OBJECTUID objectuid, size_t attribute_idx, Value const &v)
		//{
		//	if( (*_odb) == nullptr  ) return false;
		//	return (*_odb)->SetValue(objectuid, attribute_idx, v);
		//}
		/**
		*
		*
		*
		*/
		bool IObjectDatabase::SetValue(OBJECTUID objectuid, const ::std::wstring &attrname, Value const &v)
		{
			if( (*_odb) == nullptr  ) return false;
			return (*_odb)->SetValue(objectuid, attrname, v);
		}
		/**
		*
		*
		*
		*/
		//UPVALUE	IObjectDatabase::GetValue(OBJECTUID objectuid, size_t idx) const
		//{
		//	if( (*_odb) == nullptr  ) return UPVALUE();
		//	return (*_odb)->GetValue(objectuid, idx);
		//}
		/**
		*
		*
		*
		*/
		UPVALUE	IObjectDatabase::GetValue(OBJECTUID objectuid, const ::std::wstring &attrname)
		{
			if( (*_odb) == nullptr  ) return UPVALUE();
			return (*_odb)->GetValue(objectuid, attrname);
		}
		/**
		*
		*
		*
		*/
		bool IObjectDatabase::IsValidObjectUID(OBJECTUID objectUID)
		{
			if ((*_odb) == nullptr) return false;
			return (*_odb)->IsValidObjectUID(objectUID);
		}
		/**
		*
		*
		*
		*/
		bool IObjectDatabase::IsValidAttribute(OBJECTUID objectUID, AttributeName attributeName)
		{
			if ((*_odb) == nullptr) return false;
			return (*_odb)->IsValidAttribute(objectUID, attributeName);
		}
		/**
		*
		*
		*
		*/
		size_t IObjectDatabase::GetTableObjectCount(::std::wstring &TableName)
		{
			if ((*_odb) == nullptr) return 0;
			return (*_odb)->GetTableObjectCount(TableName);
		}
		/**
		*
		*
		*
		*/
		void IObjectDatabase::EnumerateTableNames(::std::vector<::std::wstring> &tableList)
		{
			if ((*_odb) == nullptr) return;
			(*_odb)->EnumerateTableNames(tableList);
			return;
		}
		/**
		*
		*
		*
		*/
		ITable*	IObjectDatabase::GetTableInterfaceHandle(::std::wstring tableName)
		{
			if ((*_odb) == nullptr) return 0;
			return (*_odb)->GetTableInterfaceHandle(tableName);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::GetFirstObject(::std::wstring &TableName)
		{
			if ((*_odb) == nullptr) return INVALID_OBJECT_UID;
			return (*_odb)->GetFirstObject(TableName);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::GetNextObject(OBJECTUID objectUID)
		{
			if ((*_odb) == nullptr) return INVALID_OBJECT_UID;
			if (objectUID == INVALID_OBJECT_UID) return INVALID_OBJECT_UID;
			return (*_odb)->GetNextObject(objectUID);
		}
		/**
		*
		*
		*
		*/
		OBJECTUID IObjectDatabase::GetLastObject(::std::wstring &TableName)
		{
			if ((*_odb) == nullptr) return INVALID_OBJECT_UID;
			return (*_odb)->GetLastObject(TableName);
		}
		/**
		*
		*
		*
		*/
		bool IObjectDatabase::GetAttributeList(OBJECTUID objectUID, int filterFlags, ATTRIBUTELIST& attrlist)
		{
			if ((*_odb) == nullptr) return false;
			return (*_odb)->GetAttributeList(objectUID, filterFlags, attrlist);
		}

		UPIATTRIBUTE IObjectDatabase::GetAttribute(OBJECTUID objectUID, ::std::wstring &attributeName)
		{
			if ((*_odb) == nullptr) return false;
			return (*_odb)->GetAttribute(objectUID, attributeName);
		}
		/**
		*
		*
		*
		*/
		//IAdjacencyMatrix IObjectDatabase::GetAdjacencyMatrix()
		//{
		//	//if ((*_odb) == nullptr) return IAdjacencyMatrix(;
		//	return (*_odb)->GetAdjacencyMatrix();
		//}
		/**
		*
		*
		*
		*/
		::std::wstring IObjectDatabase::GetObjectTypeName(OBJECTUID objectUID)
		{
			if ((*_odb) == nullptr) return  L"";
			return (*_odb)->GetObjectTypeName(objectUID);
		}
		/**
		*
		*
		*
		*/
		::std::shared_ptr<ITrigger>	IObjectDatabase::GetTriggerInterface()
		{
			if ((*_odb) == nullptr) return nullptr;
			return (*_odb)->GetTriggerInterface();
		}
		// ns
	}
}