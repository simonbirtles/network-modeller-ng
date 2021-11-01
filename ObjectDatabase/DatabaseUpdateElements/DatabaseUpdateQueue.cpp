#include "stdafx.h"
#include <assert.h>
#include "DatabaseUpdateQueue.h"
#include "DatabaseUpdateRecord.h"

namespace NM {
	namespace ODB {
		/**
		*
		*
		*
		*/
		DatabaseUpdateQueue::DatabaseUpdateQueue(CDatabaseObserver* clientObserver):
			_clientObserver(clientObserver)
		{
		}
		/**
		*
		*
		*
		*/
		DatabaseUpdateQueue::~DatabaseUpdateQueue()
		{
			EraseQueue();
		}
		/**
		*
		*
		*
		*/
		void DatabaseUpdateQueue::InsertUpdate(DatabaseUpdateRecord &updateRecord)
		{
			_updateQueue.push( ::std::make_shared<DatabaseUpdateRecord>( updateRecord) );
			return;
		}
		/**
		*
		*
		*
		*/
		DatabaseUpdateQueue::SPDBUPDATERECORD DatabaseUpdateQueue::GetNextRecord()
		{
			// return sharedptr of the DatabaseUpdateRecord
			::std::shared_ptr<DatabaseUpdateRecord> ptr = ::std::make_shared<DatabaseUpdateRecord>( *_updateQueue.front() );
			// remove the original update from the queue (this also calls dtor on the record)
			_updateQueue.pop();
			// return copied record
			return ptr;
		}
		/**
		*
		*
		*
		*/
		void DatabaseUpdateQueue::Lock()
		{
			_locked = true;
		}
		/**
		*
		*
		*
		*/
		void DatabaseUpdateQueue::UnLock()
		{
			_locked = false;
		}
		/**
		*
		*
		*
		*/
		bool DatabaseUpdateQueue::IsLocked()
		{
			return _locked;
		}
		/**
		*
		*
		*
		*/
		void DatabaseUpdateQueue::Clear()
		{
			EraseQueue();
			UnLock();
		}
		/**
		*
		*
		*
		*/
		void DatabaseUpdateQueue::EraseQueue()
		{
			while( !_updateQueue.empty() )
			{
				//delete _updateQueue.front();
				_updateQueue.pop();		// also calls element dtor
			}
		}
		/**
		*
		*
		*
		*/
		size_t DatabaseUpdateQueue::QueueSize()
		{
			return _updateQueue.size();
		}
		/**
		*
		*
		*
		*/
		bool DatabaseUpdateQueue::Empty()
		{
			return _updateQueue.empty();
		}

	//ns
	}
}