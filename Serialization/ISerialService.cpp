#include "stdafx.h"
#include "ISerialService.h"
#include "ISerial.h"
#include "SerialContext.h"


namespace NM
{
	namespace Serial
	{
		/**
		*
		*
		*/
		ISerialService::ISerialService():
			_serialContext(nullptr),
			_serviceStarted(false)
		{
		}
		/**
		*
		*
		*/
		ISerialService::~ISerialService()
		{
			StopRegService();
		}
		/**
		*
		*
		*/
		bool ISerialService::StartRegService()
		{
			if(!_serviceStarted && (_serialContext == nullptr))
			{
				_serialContext = new SerialContext();
				_serviceStarted = true;
			}
			return true;
		}
		/**
		*
		*
		*/
		bool ISerialService::StopRegService()
		{
			if(_serviceStarted)
			{
				delete _serialContext;
				_serialContext = nullptr;
				_serviceStarted = false;
			}
			return true;			
		}
		/**
		*
		*
		*/
		void* ISerialService::GetClientInterface()
		{
			if( _serviceStarted && _serialContext )
			{
				return new ISerial(&_serialContext);
			}			
			return nullptr;
		}

		// ns

	}

}
