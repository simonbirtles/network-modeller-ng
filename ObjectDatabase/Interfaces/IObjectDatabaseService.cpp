#include "stdafx.h"
#include "IObjectDatabaseService.h"

#include ".\Database\ObjectDatabase.h"								// main object database

#include "Interfaces\IObjectDatabase.h"					// client interface to the objecty database
#include "..\ServiceRegistry\IServiceRegistry.h"
#include "Interfaces\IDatabaseUpdate.h"					// client interface to database updates
#include "..\Serialization\ISerial.h"					// client interface to serial/file service
#include "..\Services\GroupManager.h"


// temp until in main app then different way of getting global registry handle... 
extern NM::Registry::IServiceRegistry* reg;

namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		*/
		IObjectDatabaseService::IObjectDatabaseService() :
			_odb(nullptr),
			_updateCache(nullptr),
			_serialFile(nullptr),
			_serviceStarted(false),
			_groupManager(nullptr)
		{
		}
		/**
		*
		*
		*/
		IObjectDatabaseService::~IObjectDatabaseService(void)
		{
			StopRegService();
		}
		/**
		*
		*
		*/
		void* IObjectDatabaseService::GetClientInterface()
		{
			if( _serviceStarted && _odb )
			{
				return new IObjectDatabase(&_odb);
			}
			
			return nullptr;
		}
		/**
		*
		*
		*/
		bool IObjectDatabaseService::StartRegService()
		{
			if(_serviceStarted)
			{
				return true;
			}

			// these are client interface pointers to update and file service - we own them as they are per client so must manage/delete them
			_updateCache = reinterpret_cast<IDatabaseUpdate*>( ::reg->GetClientInterface(L"ObjectUpdateCache") );
			_serialFile = reinterpret_cast<NM::Serial::ISerial*>( ::reg->GetClientInterface(L"File") );
			if( _updateCache && _serialFile)
			{
				_odb = new CObjectDatabase(_updateCache, _serialFile);
				_serviceStarted = true;

				_groupManager = new GroupManager(_odb, _updateCache);
				return true;
			}	
			return false;
		}
		/**
		*
		*
		*/
		bool IObjectDatabaseService::StopRegService()
		{
			if(_serviceStarted)
			{
				delete _groupManager;
				_groupManager = nullptr;

				delete _odb;
				_odb = nullptr;

				delete _updateCache;
				_updateCache = nullptr;

				delete _serialFile;
				_serialFile = nullptr;

				_serviceStarted = false;
			}
			return true;
		}


		// ns
	}
}