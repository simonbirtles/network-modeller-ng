#include "stdafx.h"
#include "IDatabaseUpdate.h"
#include "..\DatabaseUpdateElements\DatabaseUpdateCache.h"

namespace NM
{
	namespace ODB
	{

		IDatabaseUpdate::IDatabaseUpdate(DatabaseUpdateCache** UpdateCache):
			_updateCache(UpdateCache)
		{
		}
		/**
		*
		*
		*/
		IDatabaseUpdate::~IDatabaseUpdate(void)
		{
#pragma message("In dtor, call the update queue to de-register this client in case the client has not already done it, then ensure the IDatabaseUpdate pointer is passed in a unique/shared ptr for auto dtor?")
		}
		/**
		* RequestClientUpdatesQueue
		*
		* Returns a unique_ptr to the wrapper class IClientUpdateQueue, which contains pointers to the 
		* client specific update queue in the DatabaseUpdate service
		*
		*/
		IDatabaseUpdate::UpdateQueueHandle
			IDatabaseUpdate::RequestClientUpdatesQueue(CDatabaseObserver *pObserver, ObjectType object_type, ::std::vector<AttributeName> &attribute_list)
		{
			if( (*_updateCache) == nullptr) return UpdateQueueHandle();

			DatabaseUpdateQueue* dbq = (*_updateCache)->RequestClientUpdatesQueue(pObserver, object_type, attribute_list);

	
			::std::unique_ptr<IClientUpdateQueue> p (new IClientUpdateQueue(dbq, pObserver)) ;

			return ::std::move( p );

			//return new IClientUpdateQueue(dbq, pObserver);
			//return  p ;
		}
		/**
		*
		*
		*/					
		void IDatabaseUpdate::DeleteClientUpdatesQueue(CDatabaseObserver *pObserver)
		{
			if( (*_updateCache) == nullptr) return;
			(*_updateCache)->DeleteClientUpdatesQueue(pObserver);
		}
		/**
		*
		*
		*/			
		void IDatabaseUpdate::InsertUpdate(DatabaseUpdateRecord& updateRecord)
		{
			if( (*_updateCache) == nullptr) return;
			(*_updateCache)->InsertUpdate(updateRecord);
		}
		/**
		*
		*
		*/		
		LockHandle IDatabaseUpdate::GetLock()
		{
			if( (*_updateCache) == nullptr) return LockHandle();
			return (*_updateCache)->GetLock();
		}



		// ns
	}

}