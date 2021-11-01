#pragma once
#include <string>
#include <vector>
#include "ServiceRegistryCommon.h"

namespace NM
{
	namespace Registry
	{

		class BaseClassWrapper;
		class ServiceDatabase;
		class ServiceDependencyGraph;	

		/**
		* ServiceRegistry
		*
		* The registry where service classes are un/registered and services started & stopped.
		*/
		class ServiceRegistry
		{
		public:
			ServiceRegistry();
			~ServiceRegistry();
	
			//template<class T>
			bool RegisterService(const ::std::wstring &ServiceName, const ::std::wstring &ServiceDisplayName, const ::std::vector<::std::wstring>& ServiceDependencies, BaseClassWrapper** pClass);
			bool UnRegisterService(const ::std::wstring &ServiceName);	
			bool StartRegService(const ::std::wstring &ServiceName);
			bool StopRegService(const ::std::wstring &ServiceName);

			void* GetClientInterface(const ::std::wstring &ServiceName);
			ServiceStatus GetServiceState(const ::std::wstring &ServiceName);
			::std::wstring GetServiceInfoText();

		private:
			ServiceDatabase*								_svcDb;
			ServiceDependencyGraph*							_dGraph;				// dependancy graph

			bool StartParentServices(const ::std::wstring &ServiceName);
			bool StartChildServices(const ::std::wstring &ServiceName);

		};

	}

}
