#pragma once
#include "ServiceBase.h"
#include "Interfaces\ObjectDatabaseDefines.h"

class CNetworkView;

namespace NM
{
	namespace NetGraph
	{
		class SpotLightLayer;

		class INetworkViewService : public ::NM::Registry::ServiceBase
		{
			friend class CNetworkView;

		public:
			INetworkViewService();
			~INetworkViewService();

			// virtuals from ServiceBase
			bool StartRegService();
			bool StopRegService();
			void* GetClientInterface();

			void SetSpotlightOn(::std::vector<::NM::ODB::OBJECTUID>&);
			void SetSpotlightOff();
			bool SetGroupView(::NM::ODB::OBJECTUID groupUID);



		private:
			static CNetworkView* _pNetworkView;

			::NM::NetGraph::SpotLightLayer* _pSpotlight;


		};


	}
}

