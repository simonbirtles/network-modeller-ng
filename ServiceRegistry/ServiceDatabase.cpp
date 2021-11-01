#include "stdafx.h"
#include "ServiceDatabase.h"
#include "ServiceClassWrapper.h"

namespace NM
{
	namespace Registry
	{
		ServiceDatabase::ServiceInfo::~ServiceInfo()
		{
			OutputDebugString(L"\n\tServiceInfo::dtor() for ");
			OutputDebugString(serviceName.c_str());
			OutputDebugString(L"\n");
			delete pBaseClassWrapper;
			pBaseClassWrapper = nullptr;
		}
		/**
		*
		*
		*
		*/
		ServiceDatabase::ServiceDatabase()
		{
		}
		/**
		*
		*
		*
		*/
		ServiceDatabase::~ServiceDatabase()
		{
		}
		/**
		*
		*
		*
		*/
		ServiceDatabase::WPSERVICEINFO 
			ServiceDatabase::FindService(const ::std::wstring &ServiceName)
		{
			::std::vector<SPSERVICEINFO>::iterator dbIT =_serviceDb.begin();;
			while( dbIT != _serviceDb.end() )
			{
				if( (*dbIT)->serviceName.compare( ServiceName ) == 0 )
				{
					return ::std::move( WPSERVICEINFO(*dbIT) );
				}
				++dbIT;
			}
			return ::std::move( WPSERVICEINFO() );
		}
		/**
		*
		*
		*
		*/
		ServiceDatabase::WPSERVICEINFO 
			ServiceDatabase::FindService(const GRAPHID GraphID)
		{
			::std::vector<SPSERVICEINFO>::iterator dbIT =_serviceDb.begin();;
			while( dbIT != _serviceDb.end() )
			{
				if( (*dbIT)->graphID == GraphID )
				{
					return ::std::move( WPSERVICEINFO(*dbIT) );
				}
				++dbIT;
			}
			return ::std::move( WPSERVICEINFO() );
		}
		/**
		*
		*
		*
		*/
		ServiceDatabase::WPSERVICEINFO 
			ServiceDatabase::AddService(GRAPHID GraphID, const ::std::wstring &ServiceName, ServiceType SvcType)
		{
	
			WPSERVICEINFO wpSi = FindService( ServiceName );
			if( !wpSi.expired() )
			{
				// service already exists
				return ::std::move( wpSi );
			}

			// new service
			SPSERVICEINFO pSi = ::std::make_shared<ServiceInfo>(GraphID, ServiceName);
			pSi->serviceType = SvcType;
			_serviceDb.push_back( pSi );
			wpSi = pSi;
			pSi.reset();

			return ::std::move( wpSi );
		}
		/**
		*
		*
		*
		*/
		void ServiceDatabase::RemoveService(const ::std::wstring &ServiceName)
		{
			::std::vector<SPSERVICEINFO>::iterator dbIT =_serviceDb.begin();;
			while( dbIT != _serviceDb.end() )
			{
				if( (*dbIT)->serviceName.compare( ServiceName ) == 0 )
				{
					(*dbIT).reset();
					_serviceDb.erase(dbIT);
					return;
				}
				++dbIT;
			}
			return;
		}
		/**
		*
		*
		*
		*/
		void ServiceDatabase::RemoveService(const GRAPHID GraphID)
		{
			::std::vector<SPSERVICEINFO>::iterator dbIT =_serviceDb.begin();;
			while( dbIT != _serviceDb.end() )
			{
				if( (*dbIT)->graphID == GraphID )
				{
					(*dbIT).reset();
					_serviceDb.erase(dbIT);
					return;
				}
				++dbIT;
			}
			return;
		}
		/**
		*
		*
		*
		*
		*/
		void ServiceDatabase::GetServicesList(::std::vector<::std::wstring>& ServicesList)
		{
			::std::vector<SPSERVICEINFO>::iterator it =	_serviceDb.begin();
			while( it != _serviceDb.end() )
			{
				ServicesList.push_back( (*it)->serviceName );
				++it;
			}
		}

	}
}