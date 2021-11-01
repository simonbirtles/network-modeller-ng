#pragma once
#include "IClientUpdateQueue.h"
#include <vector>

namespace NM
{
	namespace ODB
	{
		class CDatabaseObserver;
		class DatabaseUpdateRecord;
		class DatabaseUpdateCache;

		class IDatabaseUpdate
		{
		public:
			typedef ::std::unique_ptr<IClientUpdateQueue>	UpdateQueueHandle;

			explicit IDatabaseUpdate(DatabaseUpdateCache** UpdateCache);
			~IDatabaseUpdate();

			UpdateQueueHandle	RequestClientUpdatesQueue(CDatabaseObserver *pObserver, ObjectType object_type, ::std::vector<AttributeName> &attribute_list);
			void				DeleteClientUpdatesQueue(CDatabaseObserver *pObserver);
			void				InsertUpdate(DatabaseUpdateRecord& updateRecord) ; 			
			LockHandle			GetLock();

		private:
			DatabaseUpdateCache** _updateCache;

		};


		// ns
	}

}
