#pragma once
#include "..\ServiceRegistry\ServiceBase.h"

namespace NM
{
	namespace Serial
	{
		class SerialContext;

		class ISerialService : public NM::Registry::ServiceBase
		{
		public:
			ISerialService();
			~ISerialService();


			// pure virtuals from abstract base class ServiceBase
			bool	StartRegService();
			bool	StopRegService();
			void*	GetClientInterface();

		private:
			SerialContext*	_serialContext;
			bool			_serviceStarted;
		};

// ns
	}
}
