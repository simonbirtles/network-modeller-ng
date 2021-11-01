#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"
#include "..\Interfaces\IOdbUpdateQueue.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <memory>

namespace NM 
{
	namespace ODB 
	{
		class CDatabaseObserver;
		class DatabaseUpdateCache;
		class IOdbUpdateQueue;

		class IDatabaseUpdateCache
		{
		public:
			IDatabaseUpdateCache(DatabaseUpdateCache& updateCache);
			~IDatabaseUpdateCache(void);
	
			::std::shared_ptr<IOdbUpdateQueue> 
				AddNotificationRequest(CDatabaseObserver *pObserver, ObjectType,  ::std::vector<::std::wstring> &attribute_list);
			void RemoveNotificationRequest(CDatabaseObserver *pObserver);
			
			
			///> Get set of registered observers for ObjectType/Attribute combination, return empty set if non and/or attr not registered
			//::std::set<CDatabaseObserver*>		
				// GetAttributeNotificationList(ObjectType object_type, ::std::wstring &attribute);
	
			///> Boolean function to state if ObjectType/Attribute combination is registered, not sure if we need this? 
			//bool IsObjectAttributeUpdateRegistered(ObjectType object_type, ::std::wstring &attribute);

		private:
			DatabaseUpdateCache& _updateCache;
		};

		// ns
	}
}