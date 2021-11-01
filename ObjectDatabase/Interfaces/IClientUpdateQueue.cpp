#include "stdafx.h"
#include "IClientUpdateQueue.h"
#include "..\DatabaseUpdateElements\DatabaseUpdateQueue.h"


namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		*
		*/
		IClientUpdateQueue::IClientUpdateQueue(DatabaseUpdateQueue *updateQueue, CDatabaseObserver* clientPtr):
			_updateQueue(updateQueue),
			_clientPtr(clientPtr)

		{
		}
		/**
		*
		*
		*
		*/
		IClientUpdateQueue::~IClientUpdateQueue()
		{
		}
		/**
		*
		*
		*
		*/
		size_t IClientUpdateQueue::QueueSize()
		{
			if( (_updateQueue) == nullptr ) return 0;
			return (_updateQueue)->QueueSize();
		}
		/**
		*
		*
		*
		*/
		bool IClientUpdateQueue::Empty()
		{
			if( (_updateQueue) == nullptr ) return 0;
			return (_updateQueue)->Empty();
		}
		/**
		*
		*
		*
		*/
		IClientUpdateQueue::SPDBUPDATERECORD IClientUpdateQueue::GetNextRecord()
		{
			if( (_updateQueue) == nullptr ) return 0;
			return (_updateQueue)->GetNextRecord();
		}
		/**
		*
		*
		*
		*/
		void IClientUpdateQueue::Clear()
		{
			if ((_updateQueue) == nullptr) return;
			return (_updateQueue)->Clear();
		}


	// ns
	}

}