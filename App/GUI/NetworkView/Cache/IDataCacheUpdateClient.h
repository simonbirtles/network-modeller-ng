#pragma once

namespace NM
{
	namespace NetGraph
	{
		class IDataCacheUpdateClient
		{
		public:
			IDataCacheUpdateClient();
			~IDataCacheUpdateClient();
			virtual void DataCacheUpdateNotification() = 0;
		};

	}
}

