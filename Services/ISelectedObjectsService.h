#pragma once
#include "..\ServiceRegistry\ServiceBase.h"

namespace NM
{
	/*
	* This class is passed as a template param into ServiceRegistry::RegisterService<T>(...)
	*
	*
	*/
	class SelectedObjects;

	class ISelectedObjectsService : public Registry::ServiceBase
	{
	public:
		ISelectedObjectsService();
		~ISelectedObjectsService();

		bool StartRegService();
		bool StopRegService();
		void* GetClientInterface();

	private:
		SelectedObjects* _selected;
		bool			_serviceStarted;
	};
}
