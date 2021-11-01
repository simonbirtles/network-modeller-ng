#pragma once
#include "ServiceClassWrapper.h"
#include <string>
#include <vector>
#include <algorithm>

namespace NM
{
	namespace Registry
	{
		class ServiceRegistry;

		class IServiceRegistry
		{
		public:
			 IServiceRegistry();
			~IServiceRegistry();

			template<class T>
			bool RegisterService(const ::std::wstring &ServiceName, const ::std::wstring &ServiceDisplayName, const ::std::vector<::std::wstring>& ServiceDependencies);
			bool UnRegisterService(const ::std::wstring &ServiceName);	
			bool StartRegService(const ::std::wstring &ServiceName);
			bool StopRegService(const ::std::wstring &ServiceName);
			::std::wstring GetServiceInfoText();
			void* GetClientInterface(const ::std::wstring &ServiceName);
			ServiceStatus GetServiceState(const ::std::wstring &ServiceName);

		private:
			ServiceRegistry* _registry;
			bool RegisterService(const ::std::wstring &ServiceName, const ::std::wstring &ServiceDisplayName, const ::std::vector<::std::wstring>& ServiceDependencies, BaseClassWrapper** pClass);
		};



		template<class T>
		bool IServiceRegistry::RegisterService(const ::std::wstring &ServiceName, const ::std::wstring &ServiceDisplayName, const ::std::vector<::std::wstring>& ServiceDependencies)
		{
		#pragma message("need to add a flag for autostart service")
			::std::wstring serviceName = ServiceName;
			::std::transform(serviceName.begin(), serviceName.end(), serviceName.begin(), ::toupper);
			BaseClassWrapper* pClass = new ServiceClassWrapper<T>(serviceName);
			// calls internal private class RegisterService (in.cpp file)
			return RegisterService(serviceName, ServiceDisplayName, ServiceDependencies, &pClass);
		}
	
	}
}