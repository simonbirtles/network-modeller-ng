#pragma once
#include <map>
#include <tuple>
#include <string>
#include <functional>
#include <memory>

#include "..\Interfaces\IObjectDatabase.h"
//#include "..\DatabaseCoreElements\Value.h"
#include "..\Interfaces\ObjectDatabaseDefines.h"

namespace NM
{
	namespace ODB
	{
		
		class TriggerManager
		{
		public:
			TriggerManager();
			~TriggerManager();
			
			/*typedef ::std::function<bool(IObjectDatabase&, OBJECTUID, const Value& pValue, const Value& nValue)> TriggerFunc;
			enum class TriggerOperation { Insert, Update, Delete };*/

			TRIGGERID CreateTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType, TriggerFunc &f);
			bool DeleteTrigger(TRIGGERID TriggerID);
			size_t HasTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType);
			bool ExecuteTrigger(::std::wstring &TableName, OBJECTUID objectUID, const ::std::wstring &AttributeName, TriggerOperation OpType, const Value &PreviousValue, const Value &NewValue, IObjectDatabase &odb);

		private:
			typedef ::std::wstring TableName;
			typedef ::std::wstring AttributeName;
			
			// std::multimap key
			typedef ::std::tuple<TableName, AttributeName, TriggerOperation> mapKey;
			// std::multimap value
			typedef ::std::pair<TRIGGERID, TriggerFunc> mapValue;
			// std::multimap
			typedef ::std::multimap<mapKey, mapValue> TRIGGERMAP;
			//typedef ::std::multimap<mapKey, TriggerFunc> TRIGGERMAP;

			// std::pair for multimap equal_range find
			typedef std::pair <TRIGGERMAP::iterator, TRIGGERMAP::iterator> TRIGGERMAP_EQUALPAIRIT;

			

			TRIGGERMAP	mTrigger;

		};

	}
}

