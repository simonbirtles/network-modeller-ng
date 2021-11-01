#pragma once
#include <queue>
#include "..\Interfaces\ObjectDatabaseDefines.h"
#include <memory>

namespace NM 
{
	namespace ODB 
	{
		class DatabaseUpdateRecord;
		class CDatabaseObserver;

		class DatabaseUpdateQueue
		{
		public:
			DatabaseUpdateQueue(CDatabaseObserver* clientObserver);										
			~DatabaseUpdateQueue();		

			typedef ::std::shared_ptr<DatabaseUpdateRecord>	SPDBUPDATERECORD;

			void				InsertUpdate(DatabaseUpdateRecord &update_record);
			size_t				QueueSize();			
			bool				Empty();
			SPDBUPDATERECORD	GetNextRecord();
			void				Lock();
			bool				IsLocked();
			void				Clear();
	

		
		private:
			DatabaseUpdateQueue &operator=(const DatabaseUpdateQueue &rhs);		// assignment not allowed
			DatabaseUpdateQueue(const DatabaseUpdateQueue& rhs) ;			// copy constructor	
		
			CDatabaseObserver*	_clientObserver;

			typedef ::std::queue<SPDBUPDATERECORD> UpdateQueue;
			UpdateQueue _updateQueue;

			bool _locked;

			void	EraseQueue();
			void	UnLock();
		};
 //ns
	}
}