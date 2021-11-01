#include "stdafx.h"
#include <assert.h>
#include "DataCacheUpdateQueue.h"
#include "DataCacheUpdateRecord.h"

namespace NM {
	namespace NetGraph {
		using namespace ::NM::ODB;

		/**
		*
		*
		*
		*/
		DataCacheUpdateQueue::DataCacheUpdateQueue(IDataCacheUpdateClient* clientObserver):
			_clientObserver(clientObserver)
		{
		}
		/**
		*
		*
		*
		*/
		DataCacheUpdateQueue::~DataCacheUpdateQueue()
		{
			EraseQueue();
		}
		/**
		*
		*
		*
		*/
		void DataCacheUpdateQueue::InsertUpdate(DataCacheUpdateRecord &updateRecord)
		{
			_updateQueue.push( ::std::make_shared<DataCacheUpdateRecord>( updateRecord) );
			return;
		}
		/**
		*
		*
		*
		*/
		DataCacheUpdateQueue::SPDBUPDATERECORD DataCacheUpdateQueue::GetNextRecord()
		{
			// return sharedptr of the DataCacheUpdateRecord
			::std::shared_ptr<DataCacheUpdateRecord> ptr = ::std::make_shared<DataCacheUpdateRecord>( *_updateQueue.front() );
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
		void DataCacheUpdateQueue::Lock()
		{
			_locked = true;
		}
		/**
		*
		*
		*
		*/
		void DataCacheUpdateQueue::UnLock()
		{
			_locked = false;
		}
		/**
		*
		*
		*
		*/
		bool DataCacheUpdateQueue::IsLocked()
		{
			return _locked;
		}
		/**
		*
		*
		*
		*/
		void DataCacheUpdateQueue::Clear()
		{
			EraseQueue();
			UnLock();
		}
		/**
		*
		*
		*
		*/
		void DataCacheUpdateQueue::EraseQueue()
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
		size_t DataCacheUpdateQueue::QueueSize()
		{
			return _updateQueue.size();
		}
		/**
		*
		*
		*
		*/
		bool DataCacheUpdateQueue::Empty()
		{
			return _updateQueue.empty();
		}

	//ns
	}
}