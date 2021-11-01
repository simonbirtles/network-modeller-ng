#include "stdafx.h"
#include "ServiceRegistry.h"
#include "ServiceDatabase.h"
#include "ServiceClassWrapper.h"
#include "ServiceDependencyGraph.h"
#include "ServiceRegistryCommon.h"


#include <unordered_set>
#include <Windows.h>

#define OutputDebugString

namespace NM
{
	namespace Registry
	{

		/**
		*Implementation - there is a template function and do want to seperate all funcs between cpp and h
		*/

		/**
		* ctor
		*/
		ServiceRegistry::ServiceRegistry()
		{
			OutputDebugString(L"ServiceRegistry::ctor() \n");
			_svcDb = new ServiceDatabase();
			_dGraph = new ServiceDependencyGraph();

			assert(_svcDb);
			assert(_dGraph);
		};
		/**
		* dtor
		*/	
		ServiceRegistry::~ServiceRegistry()
		{
			OutputDebugString(L"ServiceRegistry::dtor() \n");
			// if any services are still registered (in any state)
			OutputDebugString(L"\tCleaning up any remaining services...\n");

			// Use ServiceDependancyGraph and work up the tree to shutdown and delete all services.
			// shutdown and dereg all leaves which are removed from graph then same again for the new leaves 
			// and so on until we get to the root(s)
			::std::unordered_set<GRAPHID> leafs = _dGraph->GetLeafServices();
			while( leafs.size() != 0)
			{
				// loop through current set of leafs
				::std::unordered_set<GRAPHID>::iterator leafIT = leafs.begin();
				while( leafIT != leafs.end() )
				{
					ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService( *leafIT );
					if( !wpSi.expired() )
					{
						::std::wstring strName;
						{
							auto pSi = wpSi.lock();
							strName = pSi->serviceName ;
						}
						UnRegisterService( strName );
					}
					++leafIT;
				}

				leafs = _dGraph->GetLeafServices();
			}
		

			OutputDebugString(L"ServiceRegistry::dtor() - Calling delete on ServiceDependencyGraph instance \n");
			delete _dGraph;
			_dGraph = nullptr;
			OutputDebugString(L"ServiceRegistry::dtor() - Calling delete on ServiceDatabase instance \n");
			delete _svcDb;
			_svcDb = nullptr;
	

		};
		/**
		* RegisterService
		*
		* Create Entry in ServiceDatabase and in ServiceDependancyGraph
		* Checks if a phantom service has been created based on a missing dependency and/or creates one for the same reson
		*
		*/
		//template<class T>
		bool ServiceRegistry::RegisterService(const ::std::wstring &ServiceName, const ::std::wstring &ServiceDisplayName, const ::std::vector<::std::wstring>& ServiceDependencies, BaseClassWrapper** pClass)
		{
			OutputDebugString(L"\nServiceRegistry::RegisterService( ");
			OutputDebugString( ServiceName.c_str() );
			OutputDebugString(L" )\n");
		
			GRAPHID nodeID = 0;
			ServiceDatabase::SPSERVICEINFO pSvcInfo;
			// Check to see if we already have this registered, could be a duplicate request or we may have a phantom node from a dependancy
			ServiceDatabase::WPSERVICEINFO ptrSvcInfo = _svcDb->FindService( ServiceName );
			if( !ptrSvcInfo.expired() )
			{	// service already exists
			
				// convert to sharedptr
				pSvcInfo = ptrSvcInfo.lock();

				// check service type
				if( pSvcInfo->serviceType == ServiceType::Real )
				{	
					// TODO LOG
					delete *pClass;
					pClass = nullptr;
					return false;	// this is a existing fully registered service...return failure to register
				}
				else if( pSvcInfo->serviceType == ServiceType::Phantom )
				{
					// this service has not been registered before but a phantom service has been created due to a service dependency awaiting this registration
					nodeID = pSvcInfo->graphID;				// get the nodeId of the existing phantom service which we will convert to real
				}
				else
				{
					delete *pClass;
					pClass = nullptr;
					// TODO LOG
					assert(false);			/// Invalid type.. ??? not sure of use case yet				
				}

			}
			else       // create new service
			{
				// create new node in the Dependancy Graph
				nodeID = _dGraph->CreateServiceNode();
				// create new service in database
				ptrSvcInfo = _svcDb->AddService(nodeID, ServiceName, ServiceType::Real);
				// convert to sharedptr
				pSvcInfo = ptrSvcInfo.lock();		
			}
		
			// Create Service (for new or old phantom service)
			pSvcInfo->pBaseClassWrapper = *pClass; //new ServiceClassWrapper<T>( ServiceName ) ;
			// Save service name for reference
			pSvcInfo->serviceDisplayName = ServiceDisplayName;
			// save service dependancy graphid for reference
			pSvcInfo->serviceType = ServiceType::Real;		

		
			/// convert dependencies names into graphids
			::std::vector<GRAPHID> dGraphIDs;																	
			::std::vector<::std::wstring>::const_iterator dIT = ServiceDependencies.begin();				/// have to use the servicename to baseclass ptr lookup
			while( dIT != ServiceDependencies.end() )
			{
				::std::wstring serviceDepName = *dIT;
				::std::transform(serviceDepName.begin(), serviceDepName.end(), serviceDepName.begin(), ::toupper);

				ServiceDatabase::WPSERVICEINFO si = _svcDb->FindService(serviceDepName);
				if( si.expired() )
				{
					OutputDebugString(L"\n\tDependant Service Not Found....Creating Phantom Service : \t");
					OutputDebugString(serviceDepName.c_str() );
					OutputDebugString( L"\n");
					// Dependancy Graph : create node in the Dependancy Graph
					GRAPHID GID = _dGraph->CreateServiceNode();
					// database: new service info class 
					si = _svcDb->AddService(GID, serviceDepName, ServiceType::Phantom);
				}
				auto pSI = si.lock();
				dGraphIDs.push_back( pSI->graphID );
				dIT++;
			}	
			// update graph with dependencies for this node
			_dGraph->SetServiceDependencies( nodeID, dGraphIDs );



			OutputDebugString(L"\nServiceRegistry::RegisterService( ");
			OutputDebugString( ServiceName.c_str() );
			OutputDebugString(L" )\t");
			OutputDebugString(L" GraphID : ");
			OutputDebugString(::std::to_wstring(nodeID).c_str());
			OutputDebugString(L"\n");

			return true;			
		};
		/**
		* UnRegisterService
		*
		* Shutdown Service, Delete from Graph and Database
		*/
		bool ServiceRegistry::UnRegisterService(const ::std::wstring &ServiceName)
		{
			OutputDebugString(L"\nServiceRegistry::UnRegisterService( ");
			OutputDebugString( ServiceName.c_str() );
			OutputDebugString(L" )\n");

			ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService(ServiceName);
		
			if( wpSi.expired() )
				return false;		// not found

			OutputDebugString(L"\n\t found service... \n");

			GRAPHID id = -1;
		
			auto pSi = wpSi.lock();
			id = pSi->graphID;


				// check direct dependant services (children of thisnode)
				::std::unordered_set<GRAPHID> children =_dGraph->GetChildServices(id);
				::std::unordered_set<GRAPHID>::iterator childIT = children.begin();
				while( childIT != children.end() )
				{
					ServiceDatabase::WPSERVICEINFO wpSi =_svcDb->FindService( *childIT );
					ServiceDatabase::SPSERVICEINFO pSi = wpSi.lock();
					// shut this child service, StopRegService will shut all dependant services too
					this->StopRegService(  pSi->serviceName );
					++childIT;
				}

				if( pSi->pBaseClassWrapper )	// check this is not a phantom service (could use graph fucntion but this works too)
				{			// stop the requested service now all desendants are stopped
					OutputDebugString(L"\n\t calling StopRegService... \n");
					pSi->pBaseClassWrapper->StopRegService();
				}
				pSi.reset();
	
				// if we have children, we make this a phantom node instead of deleting service
				if( children.size() > 0 )
				{
					ServiceDatabase::WPSERVICEINFO wpSi2 =_svcDb->FindService( id );
					ServiceDatabase::SPSERVICEINFO pSi = wpSi2.lock();
					// make this a phantom in the graph
					pSi->serviceType = ServiceType::Phantom;
					// clear display name 
					pSi->serviceDisplayName.clear();
					// call delete on the service class wrapper, which will call delete on the service itself
					delete pSi->pBaseClassWrapper;
					pSi->pBaseClassWrapper = nullptr;
					pSi.reset();
				}
				else  // no children, leaf node so delete
				{
					// remove from the dependancy graph
					OutputDebugString(L"\n\t Remove from graph\t ");
					OutputDebugString( ServiceName.c_str() );
					OutputDebugString(L"\n");
					_dGraph->DeleteServiceNode( id );
				
					// remove from the database
					OutputDebugString(L"\n\t Remove from Database\t ");
					OutputDebugString( ServiceName.c_str());
					OutputDebugString(L"\n");
					_svcDb->RemoveService( id );	
				}

			return true;
		}
		/**
		* StartRegService
		*
		*
		*/	
		bool ServiceRegistry::StartRegService(const ::std::wstring &ServiceName)
		{
			OutputDebugString(L"\nServiceRegistry::StartRegService( ");
			OutputDebugString( ServiceName.c_str() );
			OutputDebugString(L" )");
		
			ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService(ServiceName);
			if( wpSi.expired() )
				return false;

			auto pSi = wpSi.lock();
			if( pSi->serviceType != ServiceType::Real )
			{
				OutputDebugString(L"\nServiceType for this service is not 'Real' - Cannot start a non-real service...");
				return false;
			}

			if( pSi->pBaseClassWrapper->GetServiceStatus() == ServiceStatus::Running )
			{
				OutputDebugString(L"\nService already running...");
				return false;
			}


			// start parents (mandatory)
			if ( !StartParentServices( ServiceName ) )
			{
				OutputDebugString(L"\nFailed to start parent services...");
				return false;
			}

			// start this service (mandatory)
			bool result = pSi->pBaseClassWrapper->StartRegService();

			// start child services (optional)
			if( result )
			{
				// not a problem if they dont start... well not a problem to the service we have already started above !! :)
				if( !StartChildServices( ServiceName ) )
				{
					OutputDebugString(L"\nFailed to start a child service...( ");
					OutputDebugString(ServiceName.c_str() );
					OutputDebugString(L" )");
				}
			}

			return result;
		}
		/**
		* StartParentServices
		*
		*
		*/
		bool ServiceRegistry::StartParentServices(const ::std::wstring &ServiceName)
		{
			OutputDebugString(L"\nServiceRegistry::StartParentServices( ");
			OutputDebugString( ServiceName.c_str() );
			OutputDebugString(L" )");

			ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService(ServiceName);
			if( wpSi.expired() )
				return false;

			auto pSi = wpSi.lock();

			// are parent (depend on) services available ?
			::std::unordered_set<GRAPHID> dependantServices = 
				_dGraph->GetAncestorsServices( pSi->graphID );

			///bool result = true;
			// if there are services (parents) this service depends on...
			// check all started, if not try to start (recursive call) else fail
			::std::unordered_set<GRAPHID>::iterator dependIT = dependantServices.begin();
			while( dependIT != dependantServices.end() )
			{
				ServiceDatabase::WPSERVICEINFO wpSiDepend = _svcDb->FindService( *dependIT );
				assert(!wpSiDepend.expired());
				auto pSiDepend = wpSiDepend.lock();
				if( pSiDepend->serviceType == ServiceType::Real )
				{
					if( pSiDepend->pBaseClassWrapper->GetServiceStatus() != ServiceStatus::Running )
					{
						// recursive call to this func ...
						if( !StartParentServices( pSiDepend->serviceName ) )
						{
							// TODO log service start failed - already logged... debug string
							OutputDebugString(L"Failed to start parent service...( ");
							OutputDebugString( pSiDepend->serviceName.c_str() );
							OutputDebugString(L" ) - therefore cannot start this service");
							return false;
						}
					}					
				}
				else
				{
					OutputDebugString(L"ServiceType for ");
					OutputDebugString( pSiDepend->serviceName.c_str() );
					OutputDebugString(L" is not Real, which is a service that ");
					OutputDebugString( ServiceName.c_str() );
					OutputDebugString(L" is dependant on. - Cannot start a non-real service, therefore cannot start this service.");
					return false;
				}
				++dependIT;
			}

			// start this service
			bool result = pSi->pBaseClassWrapper->StartRegService();
			OutputDebugString( (result ? L"\t(Success)" :L"\t(Failed)" ) );

			return result;
		}
		/**
		* StartChildServices
		*
		*
		*/
		bool ServiceRegistry::StartChildServices(const ::std::wstring &ServiceName)
		{
			OutputDebugString(L"\nServiceRegistry::StartChildServices( ");
			OutputDebugString( ServiceName.c_str() );
			OutputDebugString(L" )");

			ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService(ServiceName);
			if( wpSi.expired() )
				return false;

			auto pSi = wpSi.lock();
			
			// are there service waiting for this to be started?
			// find any dependant services
			::std::unordered_set<GRAPHID> dependantChildren;
			dependantChildren = _dGraph->GetDescendantServices(pSi->graphID);
			// we now have a list of children of this node, see if we need to start any.
			::std::unordered_set<GRAPHID>::iterator it = dependantChildren.begin();
			while( it != dependantChildren.end() )
			{			
				ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService( *it );
				if( !wpSi.expired() )
				{
					ServiceDatabase::SPSERVICEINFO pSi = wpSi.lock();
					if( pSi->serviceType == ServiceType::Real )
					{
						if( pSi->pBaseClassWrapper->GetServiceStatus() != ServiceStatus::Running )
						{
							if( !StartChildServices( pSi->serviceName ) )
							{
								OutputDebugString(L"\nFailed to start child service...( ");
								OutputDebugString(ServiceName.c_str() );
								OutputDebugString(L" )");
								return false;
							}
						}
					}
				}
				++it;
			}
			// start this service
			bool result = pSi->pBaseClassWrapper->StartRegService();
			return result;
		}
		/**
		* StopRegService
		*
		*
		*/
		bool ServiceRegistry::StopRegService(const ::std::wstring &ServiceName)
		{
			OutputDebugString(L"\nServiceRegistry::StopRegService( ");
			OutputDebugString( ServiceName.c_str() );
			OutputDebugString(L")");

			ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService(ServiceName);
			if( wpSi.expired() )
				return false;

			auto pSi = wpSi.lock();

			if( ( pSi->serviceType == ServiceType::Phantom ) || ( pSi->pBaseClassWrapper == nullptr) )
			{
				OutputDebugString(L"\n\tService is Phantom state - Service already stopped...!");
				return true;
			}
		
			if( pSi->pBaseClassWrapper->GetServiceStatus() == ServiceStatus::Stopped )
			{
				OutputDebugString(L"\n\tService already stopped...!");
				return true;
			}

			OutputDebugString(L"\n\tShutting down dependant services...\n");
			// are child dependant services running ? GetDescendants
			// find any dependant services
			::std::unordered_set<GRAPHID> dependantChildren;
			dependantChildren = _dGraph->GetChildServices(pSi->graphID);
			// we now have a list of children of this node, see if we need to start any.
			::std::unordered_set<GRAPHID>::iterator it = dependantChildren.begin();
			while( it != dependantChildren.end() )
			{			
				ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService( *it );
				if( !wpSi.expired() )
				{
					ServiceDatabase::SPSERVICEINFO pSi = wpSi.lock();
					// recursive call
					StopRegService( pSi->serviceName );
				}
				++it;
			}
		
			// stop the passed service now all children are stopped if any existed
			pSi->pBaseClassWrapper->StopRegService();

			return true;
		}
		

		::std::wstring ServiceRegistry::GetServiceInfoText()
		{
			::std::wstring text;

			::std::vector<::std::wstring>	serviceList;
			_svcDb->GetServicesList(serviceList);

			::std::vector<::std::wstring>::iterator serviceIT = serviceList.begin();
			while( serviceIT != serviceList.end() )
			{
				ServiceDatabase::WPSERVICEINFO wpSi =_svcDb->FindService( *serviceIT );
				if( !wpSi.expired() )
				{
					ServiceDatabase::SPSERVICEINFO pSi = wpSi.lock();
					::std::wstring ltext;
					ltext = pSi->serviceName;// +L"\t";
				
					ltext += L"\t";
					ltext += pSi->serviceDisplayName; //  +L"\t";
				
					switch( pSi->serviceType)
					{
					case ServiceType::Real:
						ltext += L"\tReal";
						break;

					case ServiceType::Phantom:
						ltext += L"\tPhantom";
						break;

					case ServiceType::Invalid:
						ltext += L"\tInvalid";
						break;


					}
					if( pSi->pBaseClassWrapper )
					{
						switch( pSi->pBaseClassWrapper->GetServiceStatus() )
						{
						case ServiceStatus::Running:
							ltext += L"\tRunning";
							break;
						case ServiceStatus::Stopped:
							ltext += L"\tStopped";
							break;
						case ServiceStatus::Unknown:
							ltext += L"\tUnknown";
							break;

						}
					}

					ltext += L"\n";
					text += ltext;
				}			
				++serviceIT;
			}

			return ::std::move(text);
		}


		void* ServiceRegistry::GetClientInterface(const ::std::wstring &ServiceName)
		{
			ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService(ServiceName);
			ServiceDatabase::SPSERVICEINFO pSi = wpSi.lock();
			
				if(pSi && pSi->pBaseClassWrapper) //|| pSi->serviceType == ServiceType::Real)
				{
					if (pSi->pBaseClassWrapper->GetServiceStatus() == ServiceStatus::Running)
					{
						return pSi->pBaseClassWrapper->GetClientInterface();
					}
				}
		
			return nullptr;
		}

		ServiceStatus ServiceRegistry::GetServiceState(const ::std::wstring &ServiceName)
		{
			ServiceDatabase::WPSERVICEINFO wpSi = _svcDb->FindService(ServiceName);
			ServiceDatabase::SPSERVICEINFO pSi = wpSi.lock();
			if(pSi->pBaseClassWrapper) //|| pSi->serviceType == ServiceType::Real)
			{
				return pSi->pBaseClassWrapper->GetServiceStatus();
			}
			return ServiceStatus::Unknown;
		}

	}

}