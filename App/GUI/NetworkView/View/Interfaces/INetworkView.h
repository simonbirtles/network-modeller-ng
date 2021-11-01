#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"
#include <vector>


namespace NM
{
	namespace NetGraph
	{
		class INetworkViewService;

		class INetworkView
		{
		public:
			INetworkView(INetworkViewService*);
			~INetworkView();

			void SetSpotlightOn(::std::vector<::NM::ODB::OBJECTUID>&);
			void SetSpotlightOff();
			bool SetGroupView(::NM::ODB::OBJECTUID groupUID);

		private:
			INetworkViewService* _pNetworkViewService;

		};


	}
}
