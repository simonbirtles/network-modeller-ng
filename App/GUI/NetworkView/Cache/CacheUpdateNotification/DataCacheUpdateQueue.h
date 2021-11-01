#pragma once
#include <queue>
#include "..\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"
#include <memory>

namespace NM 
{
	namespace NetGraph 
	{
		using namespace ::NM::ODB;

		class DataCacheUpdateRecord;
		class IDataCacheUpdateClient;

		class DataCacheUpdateQueue
		{
		public:
			DataCacheUpdateQueue(IDataCacheUpdateClient* clientObserver);
			~DataCacheUpdateQueue();		

			typedef ::std::shared_ptr<DataCacheUpdateRecord>	SPDBUPDATERECORD;

			void				InsertUpdate(DataCacheUpdateRecord &update_record);
			size_t				QueueSize();			
			bool				Empty();
			SPDBUPDATERECORD	GetNextRecord();
			void				Lock();
			bool				IsLocked();
			void				Clear();
	

		
		private:
			DataCacheUpdateQueue &operator=(const DataCacheUpdateQueue &rhs);		// assignment not allowed
			DataCacheUpdateQueue(const DataCacheUpdateQueue& rhs) ;			// copy constructor	
		
			IDataCacheUpdateClient*	_clientObserver;

			typedef ::std::queue<SPDBUPDATERECORD> UpdateQueue;
			UpdateQueue _updateQueue;

			bool _locked;

			void	EraseQueue();
			void	UnLock();
		};
 //ns
	}
}