#pragma once
#include "..\DatabaseUpdateElements\DatabaseUpdateRecord.h"
#include <map>
#include <memory>

namespace NM
{
	namespace ODB
	{

		class CDatabaseObserver;
		class DatabaseUpdateQueue;

		/**
		* IDatabaseUpdateCache
		*
		* Defines an interface into the Update Cache/Queue and records held for a particular client (CDatabaseObserver)
		*
		*/
		class IClientUpdateQueue
		{

		public:
			typedef ::std::shared_ptr<DatabaseUpdateRecord>	SPDBUPDATERECORD;

			IClientUpdateQueue(DatabaseUpdateQueue *updateQueue, CDatabaseObserver* clientPtr);		///> Construct with the DatabaseUpdateCache reference and the client CDatabaseObserver pointer

			~IClientUpdateQueue();

			size_t				QueueSize();
			bool				Empty();
			::std::shared_ptr<DatabaseUpdateRecord>	GetNextRecord();
			void				Clear();

		private:
			IClientUpdateQueue();
			DatabaseUpdateQueue*	_updateQueue;		// Client specific update queue
			CDatabaseObserver*		_clientPtr;			// Client class instance pointer - Not currently used but refers to the class that registered (handle)

			IClientUpdateQueue(const IClientUpdateQueue&) = delete;
			IClientUpdateQueue& operator=(const IClientUpdateQueue &rhs) = delete;	

		};
// ns
	}
}





//
//#pragma region iteratord
//		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		public:
//					
//			
//			class iterator
//			{
//			public:
//				typedef ::std::forward_iterator_tag						iterator_category;
//				typedef ::std::map<int, ObjectUpdate*>					objectmap;
//				typedef ::std::map<::std::wstring, AttributeUpdate*>	attr_map;
//				typedef objectmap										*pointer;
//				typedef objectmap										value_type;
//				typedef objectmap										&reference;
//				typedef int												difference_type;
//
//				// construct with a pointer to a DatabaseUpdateRecord
//				iterator(pointer ptr) : 
//					ptr_(ptr)
//				{ 
//					// get total number of updates  
//				}
//
//				iterator	operator++();
//				iterator	operator++(int ignore);
//				reference	operator*();
//				pointer		operator->();
//
//			private:
//				pointer ptr_;
//				objectmap::iterator it_object_map;			// save current location
//				attr_map::iterator	it_attribute_map;		// save current location
//
//			};
//
//			//////////////////////////////////////////////////////////////////////////////////////////////////////////
//			typedef int size_type;
//			// DatabaseUpdateQueue Class Iterator functions.
//
//			// create Class DatabaseUpdateRecord with first <object,attribute>
//			iterator  Begin()
//			{
//				 return iterator(&_db_update_cache);			// data_ is a ...
//			}
//
//			iterator  End();									// one beyond end - how ?
//
//			size_type Size() ;									// size of ?? -number of distinct objects?
//
//			//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#pragma endregion iteratord
//
