#include "stdafx.h"
#include "IDatabaseUpdateService.h"
#include "..\DatabaseUpdateElements\DatabaseUpdateCache.h"
#include "IDatabaseUpdate.h"


namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		*
		*/
		IDatabaseUpdateService::IDatabaseUpdateService():
			_updateCache(nullptr),
			_serviceStarted(false)
		{
		}
		/**
		*
		*
		*
		*/
		IDatabaseUpdateService::~IDatabaseUpdateService()
		{
			StopRegService();
		}
		/**
		*
		*
		*
		*/
		bool IDatabaseUpdateService::StartRegService()
		{
			if(!_serviceStarted && (_updateCache == nullptr))
			{
				_updateCache = new DatabaseUpdateCache();
				_serviceStarted = true;
			}
			return true;
		}
		/**
		*
		*
		*
		*/
		bool IDatabaseUpdateService::StopRegService()
		{
			if(_updateCache != nullptr)
			{
				delete _updateCache;
				_updateCache = nullptr;
				_serviceStarted = false;
			}
			return true;
		}
		/**
		*
		*
		*
		*/
		void* IDatabaseUpdateService::GetClientInterface()
		{
			if( _serviceStarted && _updateCache )
			{
				return new IDatabaseUpdate(&_updateCache);
			}			
			return nullptr;			
		}


	}
}
