#include "stdafx.h"
#include "IDatabaseUpdateCache.h"
#include "..\DatabaseUpdateElements\DatabaseUpdateCache.h"

#include <assert.h>
#include <vector>

namespace NM {
	namespace ODB {

		class DatabaseUpdateQueue;
/**
*
*
*
*/
IDatabaseUpdateCache::IDatabaseUpdateCache(DatabaseUpdateCache& updateCache):
	_updateCache(updateCache)
{
}
/**
*
*
*
*/
IDatabaseUpdateCache::~IDatabaseUpdateCache(void)
{
}
/**
*
*
*
*/
::std::shared_ptr<IOdbUpdateQueue> 
	IDatabaseUpdateCache::AddNotificationRequest(CDatabaseObserver *pObserver, ObjectType objectType,  ::std::vector<::std::wstring> &attribute_list)
{
	DatabaseUpdateQueue& dbq = _updateCache.AddNotificationRequest(pObserver, objectType, attribute_list);
	return ::std::make_shared<IOdbUpdateQueue>(dbq, pObserver);
}
/**
*
*
*
*/
void IDatabaseUpdateCache::RemoveNotificationRequest(CDatabaseObserver *pObserver)
{
	_updateCache.RemoveNotificationRequest(pObserver);
}
	
	
	//namespace
	}
}