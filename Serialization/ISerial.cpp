#include "stdafx.h"
#include "ISerial.h"
#include "SerialContext.h"

namespace NM {
	namespace Serial{

		/**
		*
		*
		*/
		ISerial::ISerial(SerialContext** SerialCtxt):
			_serialContext(SerialCtxt)
		{
		}
		/**
		*
		*
		*/
		ISerial::~ISerial(void)
		{
		}
		/**
		*
		*
		*/
		void ISerial::RegisterClientSerializationService(SerialClient* client, ::std::wstring& clientRoot)
		{
			if( (*_serialContext) == nullptr) return;
			(*_serialContext)->RegisterClientSerializationService(client, clientRoot);
		}
		/**
		*
		*
		*/
		void ISerial::UnregisterClientSerializationService(SerialClient* client)
		{
			if( (*_serialContext) == nullptr) return;
			(*_serialContext)->UnregisterClientSerializationService(client);
		}
		/**
		*
		*
		*/
		void ISerial::SetClientDataDirtyState(SerialClient* client )
		{
			if( (*_serialContext) == nullptr) return;
			(*_serialContext)->SetClientDataDirtyState(client);
		}
		/**
		*
		*
		*/
		bool ISerial::GetClientDataDirtyState(SerialClient* client)
		{
			if( (*_serialContext) == nullptr) return false;
			return (*_serialContext)->GetClientDataDirtyState(client);
		}
		/**
		*
		*
		*/
		bool ISerial::GetGlobalDataDirtyState()
		{
			if( (*_serialContext) == nullptr) return false;
			return (*_serialContext)->GetGlobalDataDirtyState();
		}
		/**
		*
		*
		*/
		bool ISerial::LoadDatabase(::std::wstring &filename)
		{
			if( (*_serialContext) == nullptr) return false;
			return (*_serialContext)->LoadState(filename);
		}
		/**
		*
		*
		*/
		bool ISerial::SaveDatabase(::std::wstring &filename)
		{
			if( (*_serialContext) == nullptr) return false;
			return (*_serialContext)->SaveState(filename);
		}
		/**
		*
		*
		*/
		void ISerial::NewDatabase()
		{
			if( (*_serialContext) == nullptr) return;
			(*_serialContext)->NewDatabase();
		}
		/**
		*
		*
		*/
		::std::wstring ISerial::GetOpenDatabaseName()
		{
			if( (*_serialContext) == nullptr) return ::std::wstring(L"");
			return ::std::move( (*_serialContext)->GetDatabaseName() );
		}

		//ns
	}
}
