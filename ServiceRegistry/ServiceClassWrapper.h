#pragma once
#include "ServiceRegistryCommon.h"
#include <string>
#include <assert.h>
#include <Windows.h>


namespace NM
{
	namespace Registry
	{

		class ServiceBase;

		/**
		* BaseClassWrapper
		*
		* Pure Virtual abstract class for each Service held in a ServiceClassWrapper<T> class.
		* 
		* This base class enables the storing of different types (T) of ServiceClassWrapper<T> in the same container. i.e. std::vertex<BaseClassWrapper*>
		*/
		class BaseClassWrapper
		{
		public:
			virtual ~BaseClassWrapper(){};
			virtual bool StartRegService() = 0;
			virtual void StopRegService() = 0;
			virtual ServiceStatus GetServiceStatus() = 0;
			virtual void* GetClientInterface() = 0;
	
		protected:
			BaseClassWrapper():
				_serviceStatus(ServiceStatus::Unknown)
			{};
	
			ServiceStatus _serviceStatus;

		private:
			BaseClassWrapper(const BaseClassWrapper&) = delete;
			BaseClassWrapper& operator=( const BaseClassWrapper& ) = delete;
		};

		/**
		* ServiceClassWrapper<T>
		*
		* Holds the ServiceClass Class Name (type) <T> and pointer to class when instansiated
		* base class BaseClassWrapper to enable holding on mutiple different <T> (serviceclasses) in a container
		*
		*/
		template<class T>
		class ServiceClassWrapper : public BaseClassWrapper
		{
		public:
			ServiceClassWrapper(const ::std::wstring &ServiceName):
				_svcName(ServiceName),
				_svc(nullptr)
			{
				_svcClassDebugName = L"\nServiceClassWrapper<";
				_svcClassDebugName += _svcName;
				_svcClassDebugName += L">";

				OutputDebugString( _svcClassDebugName.c_str() );
				OutputDebugString(L"::ctor");

				// compile time check that the passed class is derived from ServiceBase
				(void)static_cast<ServiceBase*>((T*)0);
			};

			~ServiceClassWrapper()
			{
				OutputDebugString( _svcClassDebugName.c_str() );
				OutputDebugString(L"::dtor");
				if(_svc)
				{
					_serviceStatus = ServiceStatus::Stopped;
					OutputDebugString(L"\n\tservice instance exists ... calling (delete) service dtor \n");
					delete _svc;
					_svc = nullptr;
				}
			};
			/**
			* StartRegService
			*
			* 
			*/
			bool StartRegService()
			{
				OutputDebugString( _svcClassDebugName.c_str() );
				OutputDebugString(L"::StartRegService \n");
		
				bool result = false;
				if( !_svc )
				{
					// creates new instance of T which is the user supplied service interface class dervied from ServiceBase
					try
					{
						_svc = new T;
					}
					catch(...)
					{
						// todo log something
						try
						{
							if( _svc )
							{
								delete _svc;
							}
						}
						catch(...)
						{
							// todo log something
						}
						_svc = nullptr;
						return false;
					}
				}

				if( _serviceStatus != ServiceStatus::Running )
				{
					result = static_cast<ServiceBase*>(_svc)->StartRegService();
					if( result )
					{
						_serviceStatus = ServiceStatus::Running;
					}
				}
				else
				{
					return true;
				}
				return result;
			};
			/**
			* StopRegService
			*
			* Calls StopRegService within the user defined service itself dervied from ServiceBase class...
			*
			*/
			void StopRegService()
			{
				OutputDebugString( _svcClassDebugName.c_str() );
				OutputDebugString(L"::StopRegService \n");
				// call new service dervived function StopRegService() before delete
				OutputDebugString(L"\t Calling Service Class : StopRegService func \n");

				if(  (_svc) && (_serviceStatus != ServiceStatus::Stopped) )
				{
					if( static_cast<ServiceBase*>(_svc)->StopRegService() )
					{
						_serviceStatus = ServiceStatus::Stopped;
					}
				}
			}
			/**
			* GetServiceStatus
			*
			* 
			*/
			ServiceStatus GetServiceStatus()
			{
				return _serviceStatus;
			}
			/**
			* GetServiceName
			*
			* 
			*/
			const ::std::wstring& GetServiceName()
			{
				return _svcName; 
			}
			/**
			* GetClientInterface
			*
			* 
			*/
			void* GetClientInterface()
			{
				return _svc->GetClientInterface();
			}

		private:
			T*											_svc;						/// a unassigned pointer to a service (ServiceBase), will be assigned on StartRegService
			const ::std::wstring						_svcName;					/// Service Name (Unique)
	
			::std::wstring	_svcClassDebugName; // OutputDebugString string for class name and template name T
		};

	}
}