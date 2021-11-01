#include "stdafx.h"
#include "ServiceClassWrapper.h"


namespace NM
{
	namespace Registry
	{

		template<class T>
		ServiceClassWrapper::ServiceClassWrapper(const ::std::wstring &ServiceName) :
		_svcName(ServiceName),
			_svc(nullptr)
		{
			_svcClassDebugName = L"\nServiceClassWrapper<";
			_svcClassDebugName += _svcName;
			_svcClassDebugName += L">";

			OutputDebugString(_svcClassDebugName.c_str());
			OutputDebugString(L"::ctor");

			// compile time check that the passed class is derived from ServiceBase
			(void)static_cast<ServiceBase*>((T*)0);
		};

		template<class T>
		ServiceClassWrapper::~ServiceClassWrapper()
		{
			OutputDebugString(_svcClassDebugName.c_str());
			OutputDebugString(L"::dtor");
			if (_svc)
			{
				_serviceStatus = ServiceStatus::Stopped;
				OutputDebugString(L"\n\tservice instance exists ... calling (delete) service dtor \n");
				delete _svc;
				_svc = nullptr;
			}
		};










	} // ns
}
