#pragma once
#include "ObjectDatabaseDefines.h"
#include <memory>

namespace NM
{
	namespace ODB
	{
	
		class TriggerManager;

		class ITrigger
		{
		public:
			ITrigger(::std::shared_ptr<TriggerManager> pTriggerManager);
			~ITrigger();

			TRIGGERID CreateTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType, TriggerFunc &f);
			bool DeleteTrigger(TRIGGERID TriggerID);
			size_t HasTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType);

		private:
			::std::shared_ptr<TriggerManager> _pTriggerManager;
		};

	}
}

