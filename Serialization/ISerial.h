#pragma once
#include <string>

namespace NM {

	namespace Serial{

		class SerialContext;
		class SerialClient;

		class ISerial
		{
		public:
			ISerial(SerialContext** SerialCtxt);
			~ISerial();

			void RegisterClientSerializationService(SerialClient*, ::std::wstring&);
			void UnregisterClientSerializationService(SerialClient*);

			void SetClientDataDirtyState(SerialClient*);
			bool GetClientDataDirtyState(SerialClient*);
			bool GetGlobalDataDirtyState();

			bool LoadDatabase(::std::wstring&);
			bool SaveDatabase(::std::wstring&);
			void NewDatabase();
			::std::wstring GetOpenDatabaseName();


		private:
			SerialContext**	_serialContext;
		};

		// ns
	}

}

