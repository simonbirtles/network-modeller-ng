#include "stdafx.h"
#include "IServiceRegistry.h"
#include "ServiceRegistry.h"
#include <algorithm>

namespace NM
{
	namespace Registry
	{
		IServiceRegistry::IServiceRegistry()
		{
			_registry = new ServiceRegistry();
		}
	

		IServiceRegistry::~IServiceRegistry()
		{
			delete _registry;
			_registry = nullptr;
		}

//		template<class T>
//		bool IServiceRegistry::RegisterService(const ::std::wstring &ServiceName, const ::std::wstring &ServiceDisplayName, const ::std::vector<::std::wstring>& ServiceDependencies)
//		{
//#pragma message("need to add a flag for autostart service")
//			BaseClassWrapper* pClass = new ServiceClassWrapper<T>( ServiceName ) ;
//
//			return _registry->RegisterService(ServiceName, ServiceDisplayName, ServiceDependencies, &pClass);	 
//		}

		bool IServiceRegistry::UnRegisterService(const ::std::wstring &ServiceName)
		{
			::std::wstring serviceName = ServiceName;
			::std::transform(serviceName.begin(), serviceName.end(), serviceName.begin(), ::toupper);
			return _registry->UnRegisterService(serviceName);
		}

		bool IServiceRegistry::StartRegService(const ::std::wstring &ServiceName)
		{
			::std::wstring serviceName = ServiceName;
			::std::transform(serviceName.begin(), serviceName.end(), serviceName.begin(), ::toupper);
			return _registry->StartRegService(serviceName);
		}

		bool IServiceRegistry::StopRegService(const ::std::wstring &ServiceName)
		{
			::std::wstring serviceName = ServiceName;
			::std::transform(serviceName.begin(), serviceName.end(), serviceName.begin(), ::toupper);
			return _registry->StopRegService(serviceName);
		}

		::std::wstring IServiceRegistry::GetServiceInfoText()
		{
			return ::std::move( _registry->GetServiceInfoText() );
		}
		void* IServiceRegistry::GetClientInterface(const ::std::wstring &ServiceName)
		{
			::std::wstring serviceName = ServiceName;
			::std::transform(serviceName.begin(), serviceName.end(), serviceName.begin(), ::toupper);
			return _registry->GetClientInterface(serviceName);
		}
		ServiceStatus IServiceRegistry::GetServiceState(const ::std::wstring &ServiceName)
		{
			::std::wstring serviceName = ServiceName;
			::std::transform(serviceName.begin(), serviceName.end(), serviceName.begin(), ::toupper);
			return _registry->GetServiceState(serviceName);
		}
		bool IServiceRegistry::RegisterService(const ::std::wstring &ServiceName, const ::std::wstring &ServiceDisplayName, const ::std::vector<::std::wstring>& ServiceDependencies, BaseClassWrapper** pClass)
		{
			return _registry->RegisterService(ServiceName, ServiceDisplayName, ServiceDependencies, pClass);
		}

	}
}