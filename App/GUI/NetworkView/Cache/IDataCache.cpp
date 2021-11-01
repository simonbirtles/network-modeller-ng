#include "stdafx.h"
#include "IDataCache.h"
#include "DataCache.h"


namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		IDataCache::IDataCache(::std::shared_ptr<DataCache> datacache):
			_dataCache(datacache)
		{
		}
		/**
		*
		*
		*
		*/
		IDataCache::~IDataCache()
		{
		}
		/**
		*
		*
		*
		*
		*/
		bool IDataCache::RegisterForCacheUpdateNotifications(IDataCacheUpdateClient* Client, ObjectTableName objectTableName, ::std::vector<::NM::ODB::AttributeName>& attributeNames)
		{
			return _dataCache->RegisterForCacheUpdateNotifications(Client, objectTableName, attributeNames);
		}

	}
}