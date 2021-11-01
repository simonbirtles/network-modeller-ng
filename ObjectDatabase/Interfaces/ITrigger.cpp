#include "stdafx.h"
#include "ITrigger.h"
#include "..\Database\TriggerManager.h"

namespace NM
{
	namespace ODB
	{		

		/**
		*
		*
		*
		*/
		ITrigger::ITrigger(::std::shared_ptr<TriggerManager> pTriggerManager):
			_pTriggerManager(pTriggerManager)
		{
		}
		/**
		*
		*
		*
		*/
		ITrigger::~ITrigger()
		{
		}
		/**
		*
		*
		*
		*/
		TRIGGERID ITrigger::CreateTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType, TriggerFunc &f)
		{
			return _pTriggerManager->CreateTrigger(TableName, AttributeName, OpType, f);
		}
		/**
		*
		*
		*
		*/
		bool ITrigger::DeleteTrigger(TRIGGERID TriggerID)
		{
			return _pTriggerManager->DeleteTrigger(TriggerID);
		}
		/**
		*
		*
		*
		*/
		size_t ITrigger::HasTrigger(::std::wstring &TableName, ::std::wstring &AttributeName, TriggerOperation OpType)
		{
			return _pTriggerManager->HasTrigger(TableName, AttributeName, OpType);
		}


		// ns
	}
}
