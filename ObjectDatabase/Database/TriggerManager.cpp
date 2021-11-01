#include "stdafx.h"
#include "TriggerManager.h"
#include "..\Interfaces\ObjectDatabaseDefines.h"


namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		*
		*/
		TriggerManager::TriggerManager()
		{
		}
		/**
		*
		*
		*
		*/
		TriggerManager::~TriggerManager()
		{
		}
		/**
		*
		*
		*
		*/
		TRIGGERID TriggerManager::CreateTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType, TriggerFunc &f)
		{
			OBJECTUID newUID = INVALID_OBJECT_UID;
			UuidCreate(&newUID);

			mapKey key = ::std::make_tuple(TableName, AttributeName, OpType);
			mapValue value = ::std::make_pair(newUID, f);

			mTrigger.insert( ::std::pair<mapKey, mapValue>(key, value) );

			return newUID;
		}
		/**
		*
		*
		*
		*/
		bool TriggerManager::DeleteTrigger(TRIGGERID TriggerID)
		{
			for (TRIGGERMAP::iterator mapIT = mTrigger.begin(); mapIT != mTrigger.end(); ++mapIT )
			{
				if (mapIT->second.first == TriggerID)
				{
					mTrigger.erase(mapIT);
					return true;
				}
			}
			return false;
		}
		/**
		*
		*
		*
		*/
		bool TriggerManager::ExecuteTrigger(
			::std::wstring &TableName, 
			OBJECTUID objectUID, 
			const ::std::wstring &AttributeName, 
			TriggerOperation OpType,
			const Value &PreviousValue, 
			const Value &NewValue, 
			IObjectDatabase &odb)
		{
			TriggerFunc func;
			mapKey key = ::std::make_tuple(TableName, AttributeName, OpType);			
			TRIGGERMAP_EQUALPAIRIT mapEqIT = mTrigger.equal_range(key);						
			for (TRIGGERMAP::iterator it = mapEqIT.first; it != mapEqIT.second; ++it)
			{
				/* the function is stored in the Value which is a std::pair<TriggerID, function>*/
				func = it->second.second;
				bool funcRet = false;
			/*	try
				{*/
				funcRet = func(odb, objectUID, PreviousValue, NewValue);
				if (!funcRet)
				{
					/* warning - this could leave things in an inconsistent state if previous triggers have modified values*/
					return false;
				}
			   /*}
				catch (...)
				{
					return false;
				}
				*/

			}		

			return true;
		}
		/**
		*
		*
		*
		*
		*/
		size_t TriggerManager::HasTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType)
		{
			mapKey key = ::std::make_tuple(TableName, AttributeName, OpType);
			TRIGGERMAP_EQUALPAIRIT mapEqIT = mTrigger.equal_range(key);
			return ( ::std::distance(mapEqIT.second, mapEqIT.first) );
		}
	}
}
