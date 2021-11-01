/**
* IInternetProtocolService
*
* Provides the internal registry service interface used to register and service control the service
*/
#pragma once
#include "ServiceBase.h"
#include <memory>

namespace NM
{
	namespace IPRTREE
	{
		class PTree;

		class IInternetProtocolService : public ::NM::Registry::ServiceBase
		{
		public:
			IInternetProtocolService();
			~IInternetProtocolService();

			// servicebase overrides
			bool StartRegService();
			bool StopRegService();
			void* GetClientInterface();

		private:
			::std::shared_ptr<PTree> _pTreeSvc;

			// temp test
			void AddSampleIPTreeData();

		};


	}
}
