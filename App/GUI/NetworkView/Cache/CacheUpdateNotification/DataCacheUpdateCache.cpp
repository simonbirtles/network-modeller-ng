#include "stdafx.h"
#include "IDataCacheUpdateClient.h"
#include "DataCacheUpdateQueue.h"
#include "DataCacheUpdateRecord.h"
#include "DataCacheUpdateCache.h"
//#include "..\Interfaces\Key.h"
#include <assert.h>

namespace NM
{
	namespace NetGraph 
	{		
		using namespace ::NM::ODB;

		/**
		*
		*
		*
		*/
		DataCacheUpdateCache::DataCacheUpdateCache()
		{
		}
		/**
		*
		*
		*/
		DataCacheUpdateCache::~DataCacheUpdateCache(void)
		{
			// clean up pointers to ::std::queues (observer specific queues)
			cachemap::iterator dbit = _clientQueueMap.begin();
			while( dbit != _clientQueueMap.end() )
			{
				// delete the DataCacheUpdateCache which was holding all the individual DataCacheUpdateCache per object
				delete dbit->second;		 
				dbit->second = nullptr;
				++dbit;
			}
			_clientQueueMap.clear();			// clear down map just to be clean
		}
		/**
		* CreateClientQueue
		*
		* Creates a client->queue map, If it does not already exist, creates a DataCacheUpdateCache instance relative to the clientObserve
		* saves it locally and passes back a reference to the client.
		*
		* locally saved in _clientQueueMap mapping --> DatabaseObserver*, DataCacheUpdateCache*
		*/
		DataCacheUpdateQueue* 
				DataCacheUpdateCache::CreateClientQueue(IDataCacheUpdateClient* clientObserver)
		{
			return GetObserverQueuePointer(clientObserver);
		}
		/**
		* DeleteClientQueue
		*
		* Deletes the DataCacheUpdateCache associated with the client IDataCacheUpdateClient - usually when client has de-registered.
		*
		*/
		void DataCacheUpdateCache::DeleteClientQueue(IDataCacheUpdateClient* clientObserver)
		{
			// do we have an existing queue for this clientObserver?
			cachemap::iterator it = _clientQueueMap.find( clientObserver );
			if(it != _clientQueueMap.end() )
			{
				// yes, delete the client queue
				 delete it->second;
				 _clientQueueMap.erase(it);
				 return;
			}
		}
		/**
		* InsertUpdate
		*
		* Gets a new update, finds the relevent client observers and inserts copies of the update into each client queue.
		*
		* can we create new (copy) object and use a shared_ptr to hold it, then pass sharedptr to each queue, this
		* should increment the cref counter, so this removes duplication and the actual object will be deleted
		* once the last ref to it it deleted when the last client reads the update and deletes then pops out of the queue
		*
		*/
		void DataCacheUpdateCache::InsertUpdate( DataCacheUpdateRecord &updateRecord) 
		{
			observers clients = GetObservers( updateRecord.GetObjectType(), updateRecord.GetObjectAttributeName() );
			observers::const_iterator it = clients.cbegin();
			while( it != clients.cend() )
			{
				// get observer queue pointer and update obs with update
				GetObserverQueuePointer( *it )->InsertUpdate( updateRecord );		
				++it;	// next client observer
			}

			// call clients to update
			NotifyClients();

			return;
		}
		/**
		* NotifyClients
		*
		* If updates are not locked, check each clients queue size and if updates available, call the update function
		*
		*/
		void DataCacheUpdateCache::NotifyClients()
		{
			if( Key::GetRefCnt() == 0 )
			{
				// check each clients queue size and if updates available, call the update function
				cachemap::iterator it = _clientQueueMap.begin();
				while( it != _clientQueueMap.end() )
				{
					// check this is not a nullptr to a client
					if( it->first )
					{
						// if the client queue has updates
						if( it->second->QueueSize() > 0 )
						{
							// call client update
							it->first->DataCacheUpdateNotification();
						}
					}
					++it;
				}
			}
		}
		/*
		* DataCacheUpdateCache::GetLock
		*
		* Provides a lock to prevent client updates until the calling routine is complete
		*/
		LockHandle DataCacheUpdateCache::GetLock()
		{
			return ::std::make_shared<Key>(std::bind(&DataCacheUpdateCache::NotifyClients, this) ); 
		}		
		/**
		* GetObservers
		*
		* Given an ObjectType and AttributeName; return CObserver*s that are registered for this type/name
		*/
		DataCacheUpdateCache::observers 
			DataCacheUpdateCache::GetObservers(ObjectType objectType, AttributeName attributeName)
		{
			observers clients;

			// do we have this object type registered for one or more clients
			object_map::iterator dbit = _database.find(objectType);
			if( dbit == _database.end() )
			{
				// return empty list
				return clients;
			}

			// do we have the attribute registered for the found objectType
			attribute_vector_map::iterator attribit = dbit->second.element.find( attributeName );
			if( attribit != dbit->second.element.end() )
			{
				// insert clients that have registered for this Attribute
				clients.insert(attribit->second.element.begin(), attribit->second.element.end() );
			}

			// check for clients that have registered for all attibutes for this objectType (special case)
			attribit = dbit->second.element.find(L"*");
			if (attribit != dbit->second.element.end())
			{
				// insert clients that have registered for this Attribute
				clients.insert(attribit->second.element.begin(), attribit->second.element.end());
			}
		
			// return copy of clients (if any) we have registered for both ObjectType and AttributeName
			return clients;
		}
		/**
		*
		*
		*
		*/
		DataCacheUpdateQueue* 
			DataCacheUpdateCache::GetObserverQueuePointer(IDataCacheUpdateClient* obs)
		{
			// do we have an existing queue for this obs?
			cachemap::iterator it = _clientQueueMap.find( obs );
			if(it != _clientQueueMap.end() )
			{
				// yes, so return the pointer to the queue
				return  it->second;
			}

			// no queue exists for this observer yet, so create one 
			queue_ptr qp = new queue(obs);
			// and add to map (_clientQueueMap)
			_clientQueueMap[obs] = qp;
			// return pointer to queue
			return qp;
		}
		/**
		* RequestClientUpdatesQueue
		*
		* Called from {interface} from client to register the client and the clients interests in updates
		*/
		DataCacheUpdateQueue*
			DataCacheUpdateCache::RequestClientUpdatesQueue(IDataCacheUpdateClient *pObserver, ObjectType object_type, ::std::vector<AttributeName> &attribute_list)
		{
			// do we have a obs->queue setup for this client?
			DataCacheUpdateQueue* dbq = CreateClientQueue(pObserver);

			// check to see if we have the ObjectType in the database already
			object_map::iterator object_it = _database.begin();
			object_it = _database.find( object_type );
			if( object_it == _database.end() )
			{
				// first addition for this object type, so create entry in database for this object
				attribute_vector_data av_map;
				_database[object_type] = av_map;
			}

			// get iterator to ObjectType, either was just added or already exists
			object_it = _database.find( object_type );
			assert(object_it != _database.end() );

			// get reference to the attributename-->observers* map for this object
			attribute_vector_map &av_map = object_it->second.element;

			// get iterator to the start of the attributename-->observers* map for this ObjectType
			attribute_vector_map::iterator av_map_it = av_map.begin();

			// get iterator to the attribute list we have been passed from the client for this ObjectType, 
			// this is the attribute list of the ObjectType the client observer wants updates for
			::std::vector<AttributeName>::iterator attr_it = attribute_list.begin();
			while( attr_it != attribute_list.end() )
			{
				// find the attribute name in the av maps
				av_map_it = av_map.find(*attr_it);	
				// does it already exist in the a/v map ?
				if( av_map_it != av_map.end() )
				{
					// attribute already in database, so add this observer to the list, as its a set as therefore unique values only, it wont add duplicates
					av_map_it->second.element.insert( pObserver );
				}
				else
				{
					// first request for this attribute as we dont have in DB, so create an observer list <set>, add the observer, then add the attribute and observer set to the av map
					database_observers obs;					// create observer list
					obs.element.insert( pObserver );		// add this observer
					av_map[*attr_it] = obs;					// add observer list to new map entry for this attribute
				}		
				++attr_it;
			}
			return dbq;
		}
		/**
		*
		*
		*
		*/
		void DataCacheUpdateCache::DeleteClientUpdatesQueue(IDataCacheUpdateClient *pObserver)
		{
			// check all ObjectType's
			object_map::iterator	object_it = _database.begin();
			while( object_it != _database.end() )
			{
				// get reference to the attribute/value map for this object
				attribute_vector_map &av_map = object_it->second.element;

				// get iterator to the attribute/value map for this object
				attribute_vector_map::iterator av_map_it = av_map.begin();

				while( av_map_it != av_map.end() )
				{
					// for this attribute, get reference to the <set> observer list
					observers &obs = av_map_it->second.element;
					observers::iterator obs_it = obs.begin();

					// if this observer exists in this attribute list
					obs_it = obs.find( pObserver );
					if( obs_it != obs.end() )
					{
						// erase observer from this attribute
						obs.erase( obs_it );
					}
					// next attribute
					++av_map_it;
				}
				// next object
				++object_it;
			}

			DeleteClientQueue( pObserver );
	
			return;
		}
// ns

	}
}
