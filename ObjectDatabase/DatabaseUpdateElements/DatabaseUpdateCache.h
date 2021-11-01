#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"

#include <map>
#include <queue>
#include <set>
#include <vector>

namespace NM 
{
	namespace ODB 
	{
		class CDatabaseObserver;
		class DatabaseUpdateRecord;
		class DatabaseUpdateQueue;

		/**
		* class DatabaseUpdateCache
		*
		*
		*/
		class DatabaseUpdateCache
		{
		public:
			typedef ::std::set<CDatabaseObserver*> observers;	
			typedef DatabaseUpdateQueue queue;
			typedef queue* queue_ptr;

			DatabaseUpdateCache();												
			~DatabaseUpdateCache();
		
			DatabaseUpdateQueue*
						RequestClientUpdatesQueue(CDatabaseObserver *pObserver, ObjectType object_type, ::std::vector<AttributeName> &attribute_list);
			void		DeleteClientUpdatesQueue(CDatabaseObserver *pObserver);
			void		InsertUpdate(DatabaseUpdateRecord& updateRecord) ; 			
			LockHandle	GetLock();

		private:	
			DatabaseUpdateCache(const DatabaseUpdateCache& rhs) = delete;					// disable copy
			DatabaseUpdateCache &operator=(const DatabaseUpdateCache &rhs) = delete;			// disable assignment 
	
			DatabaseUpdateQueue* 
					GetObserverQueuePointer(CDatabaseObserver* obs);	
			DatabaseUpdateQueue* 
					CreateClientQueue(CDatabaseObserver*);
			void	DeleteClientQueue(CDatabaseObserver*);
			observers 
					GetObservers(ObjectType objectType, AttributeName attributeName);
			void	NotifyClients();

			::std::vector<CDatabaseObserver*>	_clientNotifyOrder;

			///> Map the Client Observer to its dedicated update queue.
			typedef ::std::map<CDatabaseObserver*, DatabaseUpdateQueue* > cachemap;
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
			*					|				|-------CDatabaseObserver*  (--> DatabaseUpdateQueue* via _dbcache)
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