#pragma once
#include "..\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"

#include <map>
#include <string>
#include <memory>

namespace NM
{
	namespace NetGraph
	{
		class IDataCacheUpdateClient;
		class DataCache;
		
		class IDataCache
		{
		public:
			IDataCache(::std::shared_ptr<DataCache>);
			~IDataCache();
			typedef ::std::wstring ObjectTableName;

			bool SetValue(::NM::ODB::OBJECTUID, ::NM::ODB::ATTRIBUTENAME, ::NM::ODB::Value);
			::NM::ODB::UPVALUE GetValue(::NM::ODB::OBJECTUID, ::NM::ODB::ATTRIBUTENAME);
			bool RegisterForCacheUpdateNotifications(IDataCacheUpdateClient*, ObjectTableName, ::std::vector<::NM::ODB::AttributeName>&);
			bool DeRegisterForCacheUpdateNotifications(IDataCacheUpdateClient *pObserver);

		private:
			::std::shared_ptr<DataCache> _dataCache;
		};


	}
}