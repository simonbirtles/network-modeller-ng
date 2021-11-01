#pragma once

namespace NM
{
	namespace Registry
	{
		/**
		* ServiceBase
		*
		* Pure Virtual Abstract Base Class for any service that wishes to register with the ServiceRegistry and be a service
		*/
		class ServiceBase
		{
		public:
			virtual ~ServiceBase(){};
			virtual bool StartRegService() = 0;
			virtual bool StopRegService() = 0;
			virtual void* GetClientInterface() = 0;

		protected:
			ServiceBase(){};

		};

	}
}