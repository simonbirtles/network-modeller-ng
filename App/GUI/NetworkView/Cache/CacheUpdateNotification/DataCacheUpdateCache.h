#pragma once
#include <map>
#include <queue>
#include <set>
#include "..\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"

namespace NM
{
	namespace NetGraph
	{
		class IDataCacheUpdateClient;
		class DatabaseUpdateRecord;
		class DataCacheUpdateQueue;

		/**
		* class DataCacheUpdateCache
		*
		*
		*/
		class DataCacheUpdateCache
		{
		public:
			typedef ::std::set<IDataCacheUpdateClient*> observers;
			typedef DataCacheUpdateQueue queue;
			typedef queue* queue_ptr;

			DataCacheUpdateCache();
			~DataCacheUpdateCache();

			DataCacheUpdateQueue*
				RequestClientUpdatesQueue(IDataCacheUpdateClient *pObserver, ObjectType object_type, ::std::vector<AttributeName> &attribute_list);
			void		DeleteClientUpdatesQueue(IDataCacheUpdateClient *pObserver);
			void		InsertUpdate(DataCacheUpdateRecord& updateRecord);
			LockHandle	GetLock();

		private:
			DataCacheUpdateCache(const DataCacheUpdateCache& rhs) = delete;					// disable copy
			DataCacheUpdateCache &operator=(const DataCacheUpdateCache &rhs) = delete;			// disable assignment 

			DataCacheUpdateQueue*
				GetObserverQueuePointer(IDataCacheUpdateClient* obs);
			DataCacheUpdateQueue*
				CreateClientQueue(IDataCacheUpdateClient*);
			void	DeleteClientQueue(IDataCacheUpdateClient*);
			observers
				GetObservers(ObjectType objectType, AttributeName attributeName);
			void	NotifyClients();

			///> Map the Client Observer to its dedicated update queue.
			typedef ::std::map<IDataCacheUpdateClient*, DataCacheUpdateQueue* > cachemap;
			cachemap	_clientQueueMap;

			/**
			* _database maps the ObjectType(Router, Edge, Group, etc) to a map of ATTRNAME<->ClientDatabaseObservers
			*
			* MAP<ObjectType, attribute_vector_data -->
			*											MAP<ATTRNAME, database_observers -->
			*																					SET<CDatabaseObservers*>
			*
			* So given an ObjectType e.g ObjectVertex and an attribute name i.e. 'xpos', a list of interested observers can be found, which
			* can then be used to add the update to only those observer queues who are interested in a ObjectVertex::'xpos' update.
			*
			* e.g this maps out all the attributes of a Router that have been asked for as updates by each client observer.
			*
			*   ObjectVertex ---
			*					|
			*					|---- xpos -----
			*					|				|
			*					|				|-------CDatabaseObserver*  (--> DataCacheUpdateQueue* via _dbcache)
			*					|				|-------CDatabaseObserver*
			*					|
			*					|---- ypos -----
			*					|				|
			*					|				|-------CDatabaseObserver*
			*					|				|-------CDatabaseObserver*
			*					|
			*					|--- create ----
			*					|				|
			*					|				|-------CDatabaseObserver*
			*					|				|-------CDatabaseObserver*
			*					|				|-------CDatabaseObserver*
			*   ObjectEdge  -----
			*					|
			*					|---- vertexa ---
			*					|				|
			*					|				|-------CDatabaseObserver*
			*					|				|-------CDatabaseObserver*
			*					|
			*					|---- vertexb ---
			*					|				|
			*					|				|-------CDatabaseObserver*
			*					|				|-------CDatabaseObserver*
			*					|
			*					|--- create ----
			*					|				|
			*					|				|-------CDatabaseObserver*
			*					|				|-------CDatabaseObserver*
			*					|				|-------CDatabaseObserver*
			*/
			// unique set of observers, if we try to insert an existing, it does not add duplicate
			//typedef ::std::set<CDatabaseObserver*> observers;	
			struct database_observers
			{
				observers element;
			};

			//< attr_name, vector<CObserver*> >  maps single attrname to all CObservers that want notify on these updates
			typedef ::std::map<AttributeName, database_observers>	attribute_vector_map;
			struct attribute_vector_data
			{
				attribute_vector_map	element;
			};

			// Database wrapper object map
			typedef ::std::map<ObjectType, attribute_vector_data> object_map;
			object_map	_database;

		};

	}

}