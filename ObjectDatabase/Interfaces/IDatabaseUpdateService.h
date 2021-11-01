#pragma once
#include "..\ServiceRegistry\ServiceBase.h"


namespace NM
{
	namespace ODB
	{
		class DatabaseUpdateCache;

		class IDatabaseUpdateService : public NM::Registry::ServiceBase
		{
		public:
			IDatabaseUpdateService();
			~IDatabaseUpdateService();

			bool StartRegService();
			bool StopRegService();
			void* GetClientInterface();

		private:
			DatabaseUpdateCache* _updateCache;
			bool				 _serviceStarted;
		};



		// ns
	}
}

