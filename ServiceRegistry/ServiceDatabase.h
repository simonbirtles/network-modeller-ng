#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "ServiceRegistryCommon.h"
//#include "ServiceClassWrapper.h"
#include "windows.h"

namespace NM
{
	namespace Registry
	{
		class BaseClassWrapper;

		class ServiceDatabase
		{
		public:

			class ServiceInfo
			{
			public:
				ServiceInfo(GRAPHID GraphID, const ::std::wstring &ServiceName ):
					graphID( GraphID),
					serviceName( ServiceName ),
					pBaseClassWrapper( nullptr ),
					serviceType( ServiceType::Invalid )
				{};

				~ServiceInfo();
				//{
				//	OutputDebugString(L"\n\tServiceInfo::dtor() for ");
				//	OutputDebugString( serviceName.c_str() );
				//	OutputDebugString(L"\n");
				//	delete pBaseClassWrapper;
				//	pBaseClassWrapper = nullptr;
		
				//};

				const GRAPHID			graphID;
				BaseClassWrapper*		pBaseClassWrapper;
				const ::std::wstring	serviceName;
				::std::wstring			serviceDisplayName;
				ServiceType				serviceType;

			private:
				ServiceInfo();
				ServiceInfo(const ServiceInfo&);
				ServiceInfo& operator=(const ServiceInfo&);
			};
	

			typedef ::std::shared_ptr<ServiceInfo>  SPSERVICEINFO ;
			typedef ::std::weak_ptr<ServiceInfo>	WPSERVICEINFO;
			typedef const ::std::wstring			CSERVICENAME;
		

			ServiceDatabase();
			~ServiceDatabase();
			WPSERVICEINFO FindService(const ::std::wstring &ServiceName);
			WPSERVICEINFO FindService(const GRAPHID GraphID);
			WPSERVICEINFO AddService(GRAPHID GraphID, const ::std::wstring &ServiceName, ServiceType SvcType);
			void		  RemoveService(const ::std::wstring &ServiceName);
			void		  RemoveService(const GRAPHID GraphID);
			void		  GetServicesList(::std::vector<::std::wstring>& ServicesList);

		private:
			::std::vector<SPSERVICEINFO>	_serviceDb;

		};
	}
}

